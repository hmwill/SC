/*
 *	RollerCoaster2000
 *	Copyright (C) 2003 Plusplus (plusplus@free.fr)
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* under windows, gl.h needs windows.h to be included */
#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#include <mmsystem.h>
#define snprintf _snprintf
#endif

#include "gl.h"
//#include "ug.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef M_PI  /* some math header doesn't define it */
#define M_PI 3.14159265359f
#endif

#ifndef _WIN32
#include <sys/time.h>
#endif

#include "point.h"
#include "poteau.h"
#include "glbmp.h"
#include "parser.h"
#include "roller.h"
#include "cave.h"


#if defined(_WIN32_WCE)
unsigned long timeGetTime()
{
	return GetTickCount();
}
#endif

#ifdef _WIN32_WCE
static __inline float sinf(float value) {
	return (float) sin(value);
}

static __inline float cosf(float value) {
	return (float) cos(value);
}

static __inline float asinf(float value) {
	return (float) asin(value);
}

static __inline float tanf(float value) {
	return (float) tan(value);
}


#endif

float aperture = 60.0f;  /* field of view angle in degree */
float near_clip = 0.01f; /* near clip plane coord */
float far_clip = 60.0f;  /* far clip plane coord */

int startSegment = -5;              /* index of segment where the train starts */
int brakeSegment = -20;             /* index of segment where the train starts braking */
float averageSegmentLength = 0.15f; /* length of segments that the track is made of */
float twistFactor = 5.0f;           /* amount of tilt when the track tilt in turns */

int stereo = 0;            /* stereoscopic mode is on if non zero */
int cave = 1;              /* multi-screen mode if non zero(5 or 6 screens) */
float eye_sep = 0.005f;    /* distance between eyes */
float focallength = 0.05f;
int wndWidth, wndHeight;
int doIntro = 1;          /* Show coaster before starting the ride. */

int nbPointControle;      // points par lesquelles passe la courbe de Bezier
int nbLine;               // nombre total de segment du circuit
int nbCote = 6;           // nombre de cotes du cylindre
int nbLiens;
point *ppointcontrole;    /* tableau des points de controle tels qu'ils sont dans le fichier */
point *pcontrole;         /* tableau des points des courbes de Bezier */
float *pcourbure;         // courbure de la courbe projetée sur le plan xy
point *pline;             // tableau des extremites des segments
point *ppos;              // tableau des vecteurs position par rapport a la courbe
point *ptraj;
point *pcyl;              // tableau des coordonnees des vertex du tuyau
point *pline1,*pline2;
point *prail1, *prail2;
point *prail1n, *prail2n, *pcyln; // tableau des normales
point *pliens;
point *pnliens;
point *pliensn;
point *ptang;             // tangente à la trajectoire en chacun des points
float *pnorme;            // longueurs des segments
float rayonCyl = 0.02f;
float rayonLien = 0.01f;
float longueurLien = 0.1f;

float maxZ = 0.0f;      /* maximum z coordinate of the curve             */
float maxDist = 0.0f;   /* maximum distance of the curve from the origin */

glBmpImage grass;

__inline static void glColor3fv(const GLfloat *v) {
	glColor4f(v[0], v[1], v[2], 1.0f);
}

__inline static void glColor3f(GLfloat red, GLfloat green, GLfloat blue) {
	glColor4f(red, green, blue, 1.0f);
}

__inline static void glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
	glBegin(GL_TRIANGLE_FAN);
		glVertex2f(x1, y1);
		glVertex2f(x2, y1);
		glVertex2f(x2, y2);
		glVertex2f(x1, y2);
	glEnd();
}

// Load Bitmaps And Convert To Textures
GLvoid LoadGLTextures()
{
	/* char *imgPtr; */

	// Load Texture
	glBmpInit(&grass);
	if(!glBmpLoadImage(&grass,"grass.tga"))
	{
#ifdef _WIN32
		MessageBox(NULL,_T("Error: Can't open file grass.tga\n"),_T("RollerCoaster2000"),MB_OK | MB_ICONWARNING);
#else
		fprintf(stderr,"Error: Can't open file grass.tga\n");
#endif
		exit(5);
	}
	glBmpSetFilter(&grass,GL_LINEAR_MIPMAP_LINEAR,GL_LINEAR);
	glBmpSetTextureWrap(&grass,GL_REPEAT,GL_REPEAT);
	glBmpGenTextureMipMap(&grass);
}


int InitGL()  /* This Will Be Called Right After The GL Window Is Created */
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearColor(0.3f, 0.3f, 0.7f, 0.0f); /* This will clear the background color to "la couleur du ciel" */
	glClearDepthf(1.0f);                    /* Enables clearing of the depth buffer */
	glDisable(GL_LINE_SMOOTH);
	glDisable(GL_POINT_SMOOTH);
	//glDisable(GL_POLYGON_SMOOTH);
	glShadeModel(GL_SMOOTH);              /* Enables Gouraud shading */
	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_NORMALIZE);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);

	//glDisable(GL_DITHER);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	//glPolygonMode(GL_FRONT,GL_FILL);
	glFrontFace(GL_CCW);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
/*
	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, LightSpecular);
	glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);
	glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER ,1.0f);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(4.0f);
*/
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	return 1;
}


void ReSizeGLScene(int Width, int Height)
{
	float ratio, wd2;

	if(Height == 0)   /* Prevent a divide by zero if the window is too small */
		Height=1;

	wndWidth = Width;
	wndHeight = Height;
//	glViewport(0, 0, Width, Height);   /* Reset the current viewport and perspective transformation */

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	ratio = Width / (float)Height;
	wd2 = near_clip * tanf(0.5f * M_PI * aperture / 180.0f);
	glFrustumf(-ratio*wd2,  /* left   */
              ratio*wd2,   /* right  */
              -wd2,        /* bottom */
              wd2,         /* top    */
              near_clip, far_clip);

	glMatrixMode(GL_MODELVIEW);
}


void matrix_mult(float *r, float* a, float *b){
	int i,j;
	for(i=0 ; i<16 ; i++) r[i]=0.0f;
	for(i=0 ; i<4 ; i++){
		for(j=0 ; j<4 ; j++){
			r[i+4*j] = a[i]*b[4*j] + a[i+4]*b[4*j+1] + a[i+8]*b[4*j+2] + a[i+12]*b[4*j+3];
		}
	}
}


void mult_vect(point *r, float *m, point *v){
	float w;
	w = m[3]*v->x + m[7]*v->y + m[11]*v->z + m[15];
	r->x = (m[0]*v->x + m[4]*v->y + m[8]*v->z + m[12]) / w;
	r->y = (m[1]*v->x + m[5]*v->y + m[9]*v->z + m[13]) / w;
	r->z = m[2]*v->x + m[6]*v->y + m[10]*v->z + m[14];
}


// calcul la longueur d'un patch de Bézier en le discrétisant avec 1000 segments
float longueur_bezier(point *p1, point *p2, point *p3, point *p4){
	point a,b;
	float t,f;
	float d = 1.0f/1000.0f;
	a = *p1;
	f = 0.0f;
	for(t=d ; t<=1.0f ; t+=d){
		mult(&b, (1-t)*(1-t)*(1-t), p1);
		add_mult(&b, 3*t*(1-t)*(1-t), p2);
		add_mult(&b, 3*t*t*(1-t), p3);
		add_mult(&b, t*t*t, p4);
		sub(&a,&b,&a);
		f += norme(&a);
		a = b;
	}
	return f;
}


point *pvitesse;
point *paccel;


/* Calcul l'inverse du rayon de courbure de la courbe de Bézier i (projeté sur le plan xy) à l'abscisse t */
float rayonDeCourbure(int i, float t)
{
	point v,g; /* vitesse et accélération */

	v.z = g.z = 0.0f;
	mult(&v, -3*t*t+6*t-3, &pcontrole[3*i]);
	add_mult(&v, 9*t*t-12*t+3, &pcontrole[3*i+1]);
	add_mult(&v, -9*t*t+6*t, &pcontrole[3*i+2]);
	add_mult(&v, 3*t*t, &pcontrole[3*i+3]);
	mult(&g, -6*t+6, &pcontrole[3*i]);
	add_mult(&g, 18*t-12, &pcontrole[3*i+1]);
	add_mult(&g, -18*t+6, &pcontrole[3*i+2]);
	add_mult(&g, 6*t, &pcontrole[3*i+3]);
	v.z = g.z = 0.0f;


	/* formula : 1/R = det(v,g)/|v|^3 */
	if(norme2(&v) < 0.5f)  /* cas la trajectoire est proche de la verticale */
	{
		return 0.0f;
	}
	return (v.x * g.y - v.y * g.x) / norme2(&v) / norme(&v);
	/* autre façon de calculer l'inverse du rayon de courbure:
	 * projeter le vecteur accélération sur un vecteur unitaire orthoganal
	 * à la trajectoire (à l'aide d'un produit scalaire). Diviser le résultat
	 * par le carré de la longueur du vecteur vitesse.
	 * 1 / R = prod_scal(&n,&g) / norme2(&v); où n est orthogonale à v
	 */
}


/* longueur_seg est la longueur d'un segment de la courbe */
void InitCurve(float longueur_seg){
	int i,j,k;
	point a,b,c;
	float t,d;
	float *pnbsegment;

	/* initialise pcontrole à partir de ppointcontrole */
	pcontrole = (point*) malloc((3*nbPointControle+1)*sizeof(point));
	for(i=0 ; i<nbPointControle ; i++) {
		pcontrole[3*i] = ppointcontrole[2*i];
		pcontrole[3*i+1] = ppointcontrole[2*i+1];
	}
	pcontrole[3*nbPointControle] = pcontrole[0];
	/* calculer les points intermédiaires */
	for(i=0 ; i<3*nbPointControle ; i+=3){
		b = pcontrole[i+1];
		if(i>0) j = i-3; else j = 3*nbPointControle-3;
		if(norme(&b)<=0.1f){
			sub(&b,&pcontrole[i+3],&pcontrole[j]);
		}
		normalize(&b);
		sub(&c,&pcontrole[i+3],&pcontrole[i]);
		if(prod_scal(&b,&c)<0.0f){ b.x = -b.x; b.y = -b.y; b.z = -b.z; }
		mult(&a,norme(&c)*M_PI/5.5f/1.41421f,&b);
		add(&pcontrole[i+1],&pcontrole[i],&a);
		b.x = -b.x; b.y = -b.y; b.z = -b.z;
		sub(&c,&pcontrole[i],&pcontrole[j]);
		mult(&a,norme(&c)*M_PI/5.5f/1.41421f,&b);
		if(i>0) k = i-1; else k = 3*nbPointControle-1;
		add(&pcontrole[k],&pcontrole[i],&a);
	}
	/* calculer le nombre de segments pour chaque patch de Bézier */
	pnbsegment = (float*)malloc(nbPointControle*sizeof(float));
	nbLine = 0;
	for(i=0 ; i<nbPointControle ; i++){
		j = i * 3;
		t = longueur_bezier(&pcontrole[j],&pcontrole[j+1],&pcontrole[j+2],&pcontrole[j+3]);
		pnbsegment[i] = t / longueur_seg;
		if(!pnbsegment[i]) pnbsegment[i] = 1;
		nbLine += pnbsegment[i] + 1;
	}
	/* faire la discrétisation pour tout les patchs */
	pline = (point*)malloc(nbLine*sizeof(point));
/*	pcourbure = (float*)malloc(nbLine*sizeof(float)); */
	k = 0;
	for(i=0 ; i<nbPointControle ; i++){
		a = pcontrole[3*i];
		pline[k++] = a;
		d = 1.0f / (pnbsegment[i]+1.0f);
		for(j=1,t=d ; j<pnbsegment[i] ; j++,t+=d){
			mult(&b, (1-t)*(1-t)*(1-t), &pcontrole[3*i]);
			add_mult(&b, 3*t*(1-t)*(1-t), &pcontrole[3*i+1]);
			add_mult(&b, 3*t*t*(1-t), &pcontrole[3*i+2]);
			add_mult(&b, t*t*t, &pcontrole[3*i+3]);
			pline[k] = b;
			if(b.z > maxZ) maxZ = b.z;
			if(b.x * b.x + b.y * b.y > maxDist) maxDist = b.x * b.x + b.y * b.y;
/*			pcourbure[k] = - rayonDeCourbure(i,t); */
			k++;
		}
	}
	maxDist = sqrtf(maxDist);
	free(pnbsegment);
}


/* intersection du cylindre d'axe v avec le plan passant par p de normal n */
void calcCercle(point *p, point *z, point *v, int nbCote, float rayon, float angle, point *n, point *pmesh){
	point r;
	int i;
	for(i=0 ; i<nbCote ; i++){
		rotate(&r, v, (angle+(float)i)*2.0f*M_PI/(float)nbCote, z);
		normalize(&r);
		mult(&r,rayon,&r);
		add(&r,p,&r);
		intersection(&r,&r,v,p,n);
		pmesh[i] = r;
	}
}


void calcTuyau(point *pline, int nbLine, int nbCote, float rayon, float angle, point *pmesh){
	point a,b,c;
	point v,p;
	int i;
	a = pline[nbLine-1];
	for(i=0 ; i<nbLine ; i++){
		b = pline[i];
		c = pline[(i+1)%nbLine];
		sub(&v,&b,&a);
		normalize(&v);
		sub(&c,&c,&b);
		normalize(&c);
		add(&p,&v,&c);
		c = ppos[i];
		calcCercle(&b, &c, &v, nbCote, rayon, angle+pcourbure[i], &p, &pmesh[i*nbCote]);
		a = b;
	}
}


void moyenne(float *tab, int nbelem, int nbmoy_pred, int nbmoy_succ){
	float *buf;
	int i,j,k;
	float tot,f,m;
	buf = (float*)malloc(nbelem*sizeof(float));
	for(i=0 ; i<nbelem ; i++) buf[i] = tab[i];
	for(i=0 ; i<nbelem ; i++){
		k = i;
		tot = 0.0f;
		m = 0.0f;
		for(j=0 ; j<nbmoy_pred ; j++){
			f = (float)j / nbmoy_pred;
			f = 1.0f - f*f;
			m += f;
			tot += f * buf[k];
			k = (k-1+nbelem) % nbelem;
		}
		k = (i+1) % nbelem;
		for(j=1 ; j<nbmoy_succ ; j++){
			f = (float)j / nbmoy_succ;
			f = 1.0f - f*f;
			m += f;
			tot += f * buf[k];
			k = (k+1) % nbelem;
		}
		tab[i] = tot / m;
	}
	free(buf);
}


void InitLines(){
	int i,j,k;
	float t;
	point a,b,c;
	point p,v;
	/* initialisation de pcourbure et pnorme */
	pcourbure = (float*)malloc(nbLine*sizeof(float));
	pnorme = (float*)malloc(nbLine*sizeof(float));
	k=0;
	for(i=0 ; i<nbLine ; i++){
		sub(&p,&pline[i],&pline[(i-1+nbLine)%nbLine]);
		sub(&v,&pline[(i+1)%nbLine],&pline[i]);
		pnorme[i] = norme(&p);
		normalize(&p);
		normalize(&v);
		p.z = v.z = 0;
		prod_vect(&a,&v,&p);
		pcourbure[i] = twistFactor * asinf(a.z);
/*		pcourbure[i] = pcourbure[i] / 1.8f; */
	}
	moyenne(pcourbure,nbLine,7,5);
	/* initialisation de ppos */
	ppos = (point*) malloc(nbLine*sizeof(point));
	c.x = 0.0f; c.y = 0.0f; c.z = 1.0f;
	/************** at point j, curve's upward vector must be +z ************/
	j = 0;
	for(i=0 ; i<nbLine ; i++){
		a = pline[(j+i-1+nbLine)%nbLine];
		b = pline[(j+i)%nbLine];
		sub(&p,&b,&a);
		normalize(&p);
		t = -prod_scal(&p,&c)/* / prod_scal(&p,&p)*/;
		add_mult(&c,t,&p);
		normalize(&c);
		prod_vect(&a,&p,&c);
		if(a.z != 0.0f){
			a.z = 0.0f;
			prod_vect(&c,&a,&p);
			normalize(&c);
		}
		ppos[(i+j)%nbLine] = c;
	}
	/* initialisation de ptraj et ptang */
	ptraj = (point*) malloc(nbLine*sizeof(point));
	ptang = (point*) malloc(nbLine*sizeof(point));
	a = pline[nbLine-1];
	for(i=0 ; i<nbLine ; i++){
		b = pline[i];
		c = pline[(i+1)%nbLine];
		sub(&p,&b,&a);
		normalize(&p);
		sub(&c,&c,&b);
		normalize(&c);
		add(&v,&p,&c);
		ptang[i] = v;
		normalize(&ptang[i]);
		add(&c,&ppos[i],&b);
		intersection(&a,&c,&p,&b,&v);
		sub(&a,&a,&b);
		normalize(&a);
		ptraj[i] = a;
		a = b;
	}
	/* initialisation de pcyl */
	pcyl = (point*) malloc(nbLine*4*sizeof(point));
	calcTuyau(pline,nbLine,4,rayonCyl,0.0f,pcyl);
	/* initialisation de pline1 et pline2 */
	pline1 = (point*) malloc(nbLine*sizeof(point));
	pline2 = (point*) malloc(nbLine*sizeof(point));
	a = pline[nbLine-1];
	for(i=0 ; i<nbLine ; i++){
		b = pline[i];
		c = pline[(i+1)%nbLine];
		sub(&p,&b,&a);
		normalize(&p);
		sub(&v,&c,&b);
		normalize(&v);
		t = pcourbure[i];
		add(&p,&v,&p);
		normalize(&p);
		v = ptraj[i];
		rotate(&c,&p,t+1.0f*M_PI/3.0f,&v);
		mult(&c,longueurLien,&c);
		add(&pline1[i],&c,&b);
		rotate(&c,&p,t-1.0f*M_PI/3.0f,&v);
		mult(&c,longueurLien,&c);
		add(&pline2[i],&c,&b);
		a = b;
	}
	/* initialisation de prail1 */
	prail1 = (point*) malloc(nbLine*nbCote*sizeof(point));
	calcTuyau(pline1,nbLine,nbCote,rayonCyl,0.0f,prail1);
	/* initialisation de prail2 */
	prail2 = (point*) malloc(nbLine*nbCote*sizeof(point));
	calcTuyau(pline2,nbLine,nbCote,rayonCyl,0.0f,prail2);
}


void InitLiens(float f){ // f est la distance entre 2 liens consécutif souhaitée
	point a,a1,a2,p,p1,p2,b,v,n;
	float longueur,l;
	int i;
	int ia,ib;
	float distLien;
	float t,t1,t2;
	longueur = 0.0f;
	for(i=0 ; i<nbLine ; i++){
		longueur += pnorme[i];
	}
	nbLiens = (int)floor(longueur / f);
	pliens = (point*)malloc(nbLiens*12*sizeof(point));
	pnliens = (point*)malloc(nbLiens*3*sizeof(point));
	distLien = longueur / (float)nbLiens;
	l = 0.0f;
	ia = nbLine-1;
	ib = 0;
	sub(&p,&pline[ib],&pline[ia]);
	sub(&p1,&pline1[ib],&pline1[ia]);
	sub(&p2,&pline2[ib],&pline2[ia]);
	for(i=0 ; i<nbLiens ; i++){
		while(l > pnorme[ib]){
			l -= pnorme[ib];
			ia = (ia + 1) % nbLine;
			ib = (ib + 1) % nbLine;
			sub(&p,&pline[ib],&pline[ia]);
			sub(&p1,&pline1[ib],&pline1[ia]);
			sub(&p2,&pline2[ib],&pline2[ia]);
		}
		t = l / pnorme[ib];
		a = pline[ia];
		add_mult(&a,t,&p);
		pnliens[i*3] = a;
		a1 = pline1[ia];
		sub(&v,&a,&a1);
		t1 = prod_scal(&v,&p) / prod_scal(&p1,&p);
		add_mult(&a1,t1,&p1);
		pnliens[i*3+1] = a1;
		a2 = pline2[ia];
		sub(&v,&a,&a2);
		t2 = prod_scal(&v,&p) / prod_scal(&p2,&p);
		add_mult(&a2,t2,&p2);
		pnliens[i*3+2] = a2;
		sub(&n,&a1,&a2);
		normalize(&n);
		sub(&v,&a1,&a);
		normalize(&v);
		prod_vect(&b,&v,&p);
		normalize(&b);
		calcCercle(&a,&b,&v,4,rayonLien,0.0f,&n,&pliens[i*12]);       // lien avec le rail central
		calcCercle(&a1,&b,&v,4,rayonLien,0.0f,&n,&pliens[i*12+4]);
		sub(&v,&a,&a2);
		normalize(&v);
		prod_vect(&b,&v,&p);
		normalize(&b);
		calcCercle(&a2,&b,&v,4,rayonLien,0.0f,&n,&pliens[i*12+8]);
		l += distLien;
	}
}


void InitNormales(){
	int i;
	prail1n = (point*)malloc(nbLine*nbCote*sizeof(point));
	for(i=0 ; i<nbLine*nbCote ; i++){
		sub(&prail1n[i],&prail1[i],&pline1[i/nbCote]);
		normalize(&prail1n[i]);
	}
	prail2n = (point*)malloc(nbLine*nbCote*sizeof(point));
	for(i=0 ; i<nbLine*nbCote ; i++){
		sub(&prail2n[i],&prail2[i],&pline2[i/nbCote]);
		normalize(&prail2n[i]);
	}
	pcyln = (point*)malloc(nbLine*4*sizeof(point));
	for(i=0 ; i<nbLine*4 ; i++){
		sub(&pcyln[i],&pcyl[i],&pline[i/4]);
		normalize(&pcyln[i]);
	}
	pliensn = (point*)malloc(nbLiens*12*sizeof(point));
	for(i=0 ; i<nbLiens*3 ; i++){
		sub(&pliensn[i*4],&pliens[i*4],&pnliens[i]);
		normalize(&pliensn[i*4]);
		sub(&pliensn[i*4+1],&pliens[i*4+1],&pnliens[i]);
		normalize(&pliensn[i*4+1]);
		sub(&pliensn[i*4+2],&pliens[i*4+2],&pnliens[i]);
		normalize(&pliensn[i*4+2]);
		sub(&pliensn[i*4+3],&pliens[i*4+3],&pnliens[i]);
		normalize(&pliensn[i*4+3]);
	}
}


void InitCouleurs(point *light_dir){
	int i;
	float f;
	static GLfloat LightAmbient[] = {0.4f,0.4f,0.4f,1.0f};
	static GLfloat LightDiffuse[] = {1.1f,0.8f,1.1f,1.0f};
	/* static GLfloat LightSpecular[] = {1.0f,1.0f,1.0f,1.0f}; */
	/* static GLfloat LightPosition[] = {-1.0f,1.0f,0.0f,1.0f}; */

	normalize(light_dir);
	for(i=0 ; i<nbLine*nbCote ; i++){
		f = prod_scal(light_dir,&prail1n[i]);
		if(f<=0.0f) f = 0.0f;
		prail1n[i].x = LightDiffuse[0] * f + LightAmbient[0];
		prail1n[i].y = LightDiffuse[1] * f + 0.0f * LightAmbient[1];
		prail1n[i].z = LightDiffuse[2] * f + LightAmbient[2];
		f = prod_scal(light_dir,&prail2n[i]);
		if(f<=0.0f) f = 0.0f;
		prail2n[i].x = LightDiffuse[0] * f + LightAmbient[0];
		prail2n[i].y = LightDiffuse[1] * f + 0.0f * LightAmbient[1];
		prail2n[i].z = LightDiffuse[2] * f + LightAmbient[2];
	}
	for(i=0 ; i<nbLine*4 ; i++){
		f = prod_scal(light_dir,&pcyln[i]);
		if(f<=0.0f) f = 0.0f;
		pcyln[i].x = LightDiffuse[0] * f + LightAmbient[0];
		pcyln[i].y = LightDiffuse[1] * f + 0.0f * LightAmbient[1];
		pcyln[i].z = LightDiffuse[2] * f + LightAmbient[2];
	}
	for(i=0 ; i<nbLiens*12 ; i++){
		f = prod_scal(light_dir,&pliensn[i]);
		if(f<=0.0f) f = 0.0f;
		pliensn[i].x = LightDiffuse[0] * f + LightAmbient[0];
		pliensn[i].y = LightDiffuse[1] * f + 0.0f * LightAmbient[1];
		pliensn[i].z = LightDiffuse[2] * f + LightAmbient[2];
	}
}


void drawTuyau0(point *ptuyau, point *pnorm, int nbLine, int nbCote, float *mat, point *pline){
	int i0,i,j,k;
	point v;
	int flag1,flag2;
	int index;
	mult_vect(&v,mat,&pline[0]);
	flag1 = (v.x<-1.1f || v.x>1.1f || v.y<-1.2f || v.y>1.1f || v.z>0.0f);
	i0 = 0;
	for(i=0 ; i<nbLine ; i++){
		mult_vect(&v,mat,&pline[(i+1)%nbLine]);
		flag2 = (v.x<-1.1f || v.x>1.1f || v.y<-1.2f || v.y>1.1f || v.z>0.0f);
		if(flag1 && flag2){i0 = (i+1)%nbLine; continue;}
		glBegin(GL_TRIANGLE_STRIP);
		for(j=0 ; j<=nbCote ; j++){
			k = j % nbCote;
			index = ((i+1)%nbLine)*nbCote+k;
			glColor3fv((float*)&pnorm[index]);
			glVertex3fv((float*)&ptuyau[index]);
			index = i0*nbCote+k;
			glColor3fv((float*)&pnorm[index]);
			glVertex3fv((float*)&ptuyau[index]);
		}
		glEnd();
		i0 = (i+1)%nbLine;
		flag1 = flag2;
		if((v.z < -2.0f) && i<(nbLine-2) && (i%2)==0) i++;
	}
}


void drawTuyau(point *ptuyau, point *pnorm, int nbLine, int nbCote, float *mat, point *pline){
	int i0,i,j,k;
	point v;
	int flag1,flag2;
	int shl,shl0;
	int index;
	mult_vect(&v,mat,&pline[0]);
	flag1 = (v.x<-1.1f || v.x>1.1f || v.y<-1.2f || v.y>1.1f || v.z>0.0f);
	i0 = 0;
	shl0 = 0;
	for(i=0 ; i<nbLine ; i++){
		mult_vect(&v,mat,&pline[(i+1)%nbLine]);
		flag2 = (v.x<-1.1f || v.x>1.1f || v.y<-1.2f || v.y>1.1f || v.z>0.0f);
		if((v.z >= -2.0f)||(i==nbLine-1)) shl = 0; else shl = 1;
		if(flag1 && flag2){
			i0 = (i+1)%nbLine;
			shl0 = shl;
			flag1 = flag2;
			continue;
		}
		if(shl && shl0){
			glBegin(GL_TRIANGLE_STRIP);
			for(j=0 ; j<=nbCote ; j+=2){
				k = j % nbCote;
				index = ((i+1)%nbLine)*nbCote+k;
				glColor3fv((float*)&pnorm[index]);
				glVertex3fv((float*)&ptuyau[index]);
				index = i0*nbCote+k;
				glColor3fv((float*)&pnorm[index]);
				glVertex3fv((float*)&ptuyau[index]);
			}
			glEnd();
		}
		else{
			if(shl && !shl0){
				for(j=0 ; j<nbCote ; j+=2){
					glBegin(GL_TRIANGLE_STRIP);
					k = (j+2) % nbCote;
					index = i0*nbCote+k;
					glColor3fv((float*)&pnorm[index]);
					glVertex3fv((float*)&ptuyau[index]);
					index = ((i+1)%nbLine)*nbCote+k;
					glColor3fv((float*)&pnorm[index]);
					glVertex3fv((float*)&ptuyau[index]);
					k = j+1;
					index = i0*nbCote+k;
					glColor3fv((float*)&pnorm[index]);
					glVertex3fv((float*)&ptuyau[index]);
					k = j;
					index = ((i+1)%nbLine)*nbCote+k;
					glColor3fv((float*)&pnorm[index]);
					glVertex3fv((float*)&ptuyau[index]);
					index = i0*nbCote+k;
					glColor3fv((float*)&pnorm[index]);
					glVertex3fv((float*)&ptuyau[index]);
					glEnd();
				}
			}
			else{
				if(!shl && shl0){
					for(j=0 ; j<nbCote ; j+=2){
						glBegin(GL_TRIANGLE_STRIP);
						k = j;
						index = ((i+1)%nbLine)*nbCote+k;
						glColor3fv((float*)&pnorm[index]);
						glVertex3fv((float*)&ptuyau[index]);
						index = i0*nbCote+k;
						glColor3fv((float*)&pnorm[index]);
						glVertex3fv((float*)&ptuyau[index]);
						k = j+1;
						index = ((i+1)%nbLine)*nbCote+k;
						glColor3fv((float*)&pnorm[index]);
						glVertex3fv((float*)&ptuyau[index]);
						k = (j+2) % nbCote;
						index = i0*nbCote+k;
						glColor3fv((float*)&pnorm[index]);
						glVertex3fv((float*)&ptuyau[index]);
						index = ((i+1)%nbLine)*nbCote+k;
						glColor3fv((float*)&pnorm[index]);
						glVertex3fv((float*)&ptuyau[index]);
						glEnd();
					}
				}
				else{
					glBegin(GL_TRIANGLE_STRIP);
					for(j=0 ; j<=nbCote ; j++){
						k = j % nbCote;
						index = ((i+1)%nbLine)*nbCote+k;
						glColor3fv((float*)&pnorm[index]);
						glVertex3fv((float*)&ptuyau[index]);
						index = i0*nbCote+k;
						glColor3fv((float*)&pnorm[index]);
						glVertex3fv((float*)&ptuyau[index]);
					}
					glEnd();
				}
			}
		}
		i0 = (i+1)%nbLine;
		shl0 = shl;
		flag1 = flag2;
		if((v.z < -2.0f) && i<(nbLine-2) && (i%2)==0) i++;
	}
}


void drawLiens(point *pliens, point *pliensn, int nbLiens, float *mat, point *pnliens){
	int i,j,k;
	point v;
	int flag1,flag2,flag3;
	int loin;
	for(i=0 ; i<nbLiens ; i++){
		mult_vect(&v,mat,&pnliens[i*3]);
		flag1 = (v.x<-1.0f || v.x>1.0f || v.y<-1.0f || v.y>1.0f || v.z>0.0f);
		loin = (v.z < -2.0f);
		mult_vect(&v,mat,&pnliens[i*3+1]);
		flag2 = (v.x<-1.0f || v.x>1.0f || v.y<-1.0f || v.y>1.0f || v.z>0.0f);
		mult_vect(&v,mat,&pnliens[i*3+2]);
		flag3 = (v.x<-1.0f || v.x>1.0f || v.y<-1.0f || v.y>1.0f || v.z>0.0f);
		if(!(flag1&&flag2)){
			glBegin(GL_TRIANGLE_STRIP);
			for(j=0 ; j<5 ; j++){
				if(loin && j==3) j++;
				k = j % 4;
				glColor3fv((float*)&pliensn[i*12+4+k]);
				glVertex3fv((float*)&pliens[i*12+4+k]);
				glColor3fv((float*)&pliensn[i*12+k]);
				glVertex3fv((float*)&pliens[i*12+k]);
			}
			glEnd();
		}
		if(!(flag1&&flag3)){
			glBegin(GL_TRIANGLE_STRIP);
			for(j=0 ; j<5 ; j++){
				if(loin && j==3) j++;
				k = j % 4;
				glColor3fv((float*)&pliensn[i*12+k]);
				glVertex3fv((float*)&pliens[i*12+k]);
				glColor3fv((float*)&pliensn[i*12+8+k]);
				glVertex3fv((float*)&pliens[i*12+8+k]);
			}
			glEnd();
		}
	}
}


void drawSkyAndGround(int nbCote, float size, float texsize,float bas, float haut){ //nbCote = nombre de cotés du cylindre
	int i;
	float x,y;
	float c,s;

	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glBegin(GL_TRIANGLE_STRIP);
	glColor3f(0.5f,0.5f,0.7f);
	glVertex3f(size,0.0f,bas);
	glColor3f(0.3f,0.3f,0.7f);
	glVertex3f(size,0.0f,haut);
	for(i=1 ; i<nbCote ; i++){
		x = size * cos((float)i*2.0f*M_PI/(float)nbCote);
		y = size * sin((float)i*2.0f*M_PI/(float)nbCote);
		glColor3f(0.5f,0.5f,0.7f);
		glVertex3f(x,y,bas);
		glColor3f(0.3f,0.3f,0.7f);
		glVertex3f(x,y,haut);
	}
	glColor3f(0.5f,0.5f,0.7f);
	glVertex3f(size,0.0f,bas);
	glColor3f(0.3f,0.3f,0.7f);
	glVertex3f(size,0.0f,haut);
	glEnd();

	/* Draw the sky */
/*	glBegin(GL_POLYGON);
	glColor3f(0.3f,0.3f,0.7f);
	glVertex3f(size,0.0f,haut);
	for(i=nbCote-1 ; i>0 ; i--){
		x = size * cos((float)i*2.0f*M_PI/(float)nbCote);
		y = size * sin((float)i*2.0f*M_PI/(float)nbCote);
		glColor3f(0.3f,0.3f,0.7f);
		glVertex3f(x,y,haut);
	}
	glEnd();
*/
	/* Draw the ground with one single big polygon*/
/*	glEnable(GL_TEXTURE_2D);
	glBmpBind(&grass);
	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_POLYGON);
	glMultiTexCoord2f(GL_TEXTURE0, texsize,0.0f);
	glVertex3f(size,0.0f,bas);
	for(i=1 ; i<nbCote ; i++){
		c = cos((float)i*2.0f*M_PI/(float)nbCote);
		s = sin((float)i*2.0f*M_PI/(float)nbCote);
		glMultiTexCoord2f(GL_TEXTURE0, texsize*c,texsize*s);
		glVertex3f(size*c,size*s,bas);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
*/
	/* Draw the ground with many triangles*/
	glEnable(GL_TEXTURE_2D);
	glBmpBind(&grass);
	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_TRIANGLE_FAN);
	glMultiTexCoord2f(GL_TEXTURE0, 0.0f,0.0f);
	glVertex3f(0.0f,0.0f,bas);
	for(i=0 ; i<nbCote ; i++){
		c = cos((float)i*2.0f*M_PI/(float)nbCote);
		s = sin((float)i*2.0f*M_PI/(float)nbCote);
		glMultiTexCoord2f(GL_TEXTURE0, texsize*c,texsize*s);
		glVertex3f(size*c,size*s,bas);
	}
	glMultiTexCoord2f(GL_TEXTURE0, texsize,0.0f);
	glVertex3f(size,0.0f,bas);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
}


void drawEverything()
{
	float m[16];
	float modelmat[16], projmat[16];

	glGetFloatv(GL_MODELVIEW_MATRIX,modelmat);
	glGetFloatv(GL_PROJECTION_MATRIX,projmat);
	matrix_mult(m,projmat,modelmat);
	m[2] = modelmat[2];
	m[6] = modelmat[6];
	m[10] = modelmat[10];
	m[14] = modelmat[14];
	drawSkyAndGround(8,50.0f,60.0f,0.0f,10.0f);
	drawTuyau0(pcyl,pcyln,nbLine,4,m,pline);
	drawTuyau(prail1,prail1n,nbLine,nbCote,m,pline1);
	drawTuyau(prail2,prail2n,nbLine,nbCote,m,pline2);
	drawLiens(pliens,pliensn,nbLiens,m,pnliens);
	poteau_draw();
}


void fade(float opacity)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.0f,0.0f,0.0f,opacity);
	glRectf(-1.0f, -1.0f, 1.0f, 1.0f);
	glDisable(GL_BLEND);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

static void __identf(GLfloat m[])
{
	memset(m, 0, sizeof m[0] * 16);
	m[0] = m[5] = m[10] = m[15] = 1.0f;
}

static void normalizef(float v[3])
{
	float r;

	r = (float) sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	if (r == 0.0)
		return;
	r = 1.f / r;

	v[0] *= r;
	v[1] *= r;
	v[2] *= r;
}

static void crossf(float v0[3], float v1[3], float r[3])
{
	r[0] = v0[1] * v1[2] - v0[2] * v1[1];
	r[1] = v0[2] * v1[0] - v0[0] * v1[2];
	r[2] = v0[0] * v1[1] - v0[1] * v1[0];
}

void APIENTRY
ugluLookAtf(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx,
			GLfloat centery, GLfloat centerz, GLfloat upx, GLfloat upy, GLfloat upz)
{
	GLfloat forward[3], side[3], up[3];
	GLfloat m[4][4];

	forward[0] = centerx - eyex;
	forward[1] = centery - eyey;
	forward[2] = centerz - eyez;

	normalizef(forward);

	up[0] = upx;
	up[1] = upy;
	up[2] = upz;

	crossf(forward, up, side);

	normalizef(side);
	crossf(side, forward, up);

	__identf(&m[0][0]);
	m[0][0] = side[0];
	m[1][0] = side[1];
	m[2][0] = side[2];

	m[0][1] = up[0];
	m[1][1] = up[1];
	m[2][1] = up[2];

	m[0][2] = -forward[0];
	m[1][2] = -forward[1];
	m[2][2] = -forward[2];

	glMultMatrixf(&m[0][0]);
	glTranslatef(-eyex, -eyey, -eyez);
}

GLvoid DrawGLScene0()
{
	static unsigned long begin_time; /* first time thisfunction is called */
	static unsigned long time, diftime, oldtimeframe; /* last one used to compute frame per sec */
	static int nbframe = 0;
	float fps;
	static int flag_firstcall = 1;
	static float angle = 0.0f;
	char wndtitle[256];
	float dist; /* camera distance from origin */

	if( flag_firstcall )
	{
		oldtimeframe = timeGetTime();
		flag_firstcall = 0;
		begin_time = oldtimeframe;
		//glutSetWindowTitle("RollerCoaster2000");
	}
	time = timeGetTime();

	angle = (float)(timeGetTime() - begin_time) / 100; /* angle in degree */
	angle = (angle + 180.0f) / 180.0f * M_PI;          /* angle in radian */
	/* compute camera distance from the origin */
	if(maxDist*tan(aperture*M_PI/360.0f)*2.0f < maxZ)
		dist = maxZ / tan(aperture*M_PI/360.0f) / 2.0f;
	else
		dist = 0.5f + maxDist;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/* glMatrixMode(GL_MODELVIEW); */
	glLoadIdentity();
	ugluLookAtf(dist*cosf(angle),dist*sinf(angle),maxZ,                /* eye position */
        	  0.0f,0.0f,maxZ+1.0f-dist*tan(aperture*M_PI/360.0f),  /* target       */
        	  0.0f,0.0f,1.0f);                                     /* up vector    */
	drawEverything();
	/* fade in at the beginning */
	if(time - begin_time < 2000)
	{
		diftime = time - begin_time;
		fade(1.0f-(float)diftime / 2000.0f);
	}
	/* fade out at the end */
	if(time - begin_time > 34000)
	{
		diftime = time - begin_time - 34000;
		fade((float)diftime / 2000.0f);
	}

	if(time-oldtimeframe > 200.0f)
	{
		fps=(float)nbframe*1000.0f/(float)(time-oldtimeframe);
		oldtimeframe=time;
		nbframe=0;
		//snprintf(wndtitle,256,"RollerCoaster2000 %10.2f",fps);
		//glutSetWindowTitle(wndtitle);
		/* SetWindowText(hWnd,wndtitle); */
	}
	nbframe++;
}


GLvoid DrawGLScene(GLfloat roll_angle, GLfloat pitch_angle)
{
	static int index = 0;
	static float abscisse = 0.0f;
	static float oldz;
	static float vitesse = 0.0f;
	static int flag = 1; /* passe à zéro quand il n'y a plus de montée */
	static int flag_firstcall = 1;
	static int flag_stopping = 0;
	static unsigned long time, oldtime;
	float fps;
	static unsigned long oldtimeframe;
	static unsigned long stoptime; /* used to wait 5 sec before starting again */
	static int nbframe = 0;
	float t;
	point p;
	point position, eye;
	point ddv,ddv1,ddv2;
	//char wndtitle[256];
	static unsigned long begin_time; /* premier appel de timeGetTime au debut de l'anim */

	if(flag_firstcall){
		oldtimeframe = timeGetTime();
		index = startSegment;
		time = begin_time = oldtimeframe;
	}
	while(abscisse > pnorme[(index+1)%nbLine]){
		abscisse -= pnorme[(index+1)%nbLine];
		index++;
		index %= nbLine;
	}

	t = abscisse / pnorme[(index+1)%nbLine];
	position = pline[index];
	sub(&p,&pline[(index+1)%nbLine],&pline[index]);
	add_mult(&position,t,&p);
	ddv1 = ptraj[index];
	sub(&p,&ptraj[(index+1)%nbLine],&ptraj[index]);
	add_mult(&ddv1,t,&p);
	normalize(&ddv1);                /**** ddv1 vector points upward ****/
	ddv = ptang[index];
	sub(&p,&ptang[(index+1)%nbLine],&ptang[index]);
	add_mult(&ddv,t,&p);
	normalize(&ddv);                 /**** ddv vector points forward ****/
	p = ddv1;
	rotate(&ddv1, &ddv, pcourbure[index]+t*(pcourbure[(index+1)%nbLine]-pcourbure[index]), &p);
	prod_vect(&ddv2,&ddv,&ddv1);     /**** ddv2 vector points rightward ****/
	/* compute camera position */
	mult(&eye,0.2f,&ddv1);
	add(&eye,&eye,&position);

	/* rotate camera */
	if (roll_angle!=0.0f) { /* Rotate the front and side vectors around the up vector */
	    p = ddv; /* Front vector */
	    rotate(&ddv, &ddv1, roll_angle, &p);
	    p = ddv2; /* Side vector */
	    rotate(&ddv2, &ddv1, roll_angle, &p);
	} else if (pitch_angle!=0.0f) { /* Rotate the up and front vectors around the side vector. */
	    p = ddv; /* Front vector */
	    rotate (&ddv, &ddv2, pitch_angle, &p);
	    p = ddv1; /* Up vector */
	    rotate (&ddv1, &ddv2, pitch_angle, &p);
	}

//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/* glMatrixMode(GL_MODELVIEW); */
	glLoadIdentity();
	ugluLookAtf(eye.x, eye.y, eye.z,                   /* eye position */
	    	  eye.x+ddv.x, eye.y+ddv.y, eye.z+ddv.z, /* target       */
	    	  ddv1.x, ddv1.y, ddv1.z);               /* up vector    */
/*
	m[3] = m[7] = m[11] = m[12] = m[13] = m[14] = 0.0f;
	m[15] = 1.0f;
	m[0] = ddv2.x;  m[4] = ddv2.y;  m[8] = ddv2.z;
	m[1] = ddv1.x;  m[5] = ddv1.y;  m[9] = ddv1.z;
	m[2] = -ddv.x;  m[6] = -ddv.y;  m[10] = -ddv.z;
	glLoadMatrixf(m);
	mult(&ddv1,0.2f,&ddv1);
	glTranslatef(-position.x - ddv1.x, -position.y - ddv1.y, -position.z - ddv1.z);
*/
	drawEverything();
	/* fade in at the beginning of the ride */
	if(time - begin_time < 1500)
	{
		fade(1.0f-(float)(time - begin_time) / 1500.0f);
	}

	time = timeGetTime();
	if(flag){
		if(oldz-position.z > 0.0f) flag = 0;
		else vitesse = 0.3f;
	}
	else{
		if(index >= brakeSegment || flag_stopping){
			flag_stopping = 1;
			if(vitesse > 0.0f){
				vitesse = vitesse*vitesse - 5.6f * (float)(time - oldtime) / 1000.0f;
				if(vitesse <= 0.0f){
					vitesse = 0.0f;
					stoptime = time;
				}
				else vitesse = sqrt(vitesse);
			}
			else{
				if(time - stoptime > 5000){
					flag_stopping = 0;
					flag = 1;
				}
			}
		}
		else{
			vitesse = vitesse*vitesse + 3.0f*(oldz-position.z);
			if(vitesse <= 0.0f) vitesse = 0.0f;
			else vitesse = sqrt(vitesse);
		}
	}
	glFlush();
	if(flag_firstcall){
		flag_firstcall = 0;
		oldtime = time-1;
	}
	abscisse += vitesse * (float)(time - oldtime) / 1000.0f;
	if(time-oldtimeframe > 200.0f)
	{
		fps=(float)nbframe*1000.0f/(float)(time-oldtimeframe);
		oldtimeframe=time;
		nbframe=0;
//		snprintf(wndtitle,256,"RollerCoaster2000 %10.2f",fps);
		//snprintf(wndtitle,256,"RollerCoaster2000 %10.2f",(fps/cave));
		//glutSetWindowTitle(wndtitle);
		/* SetWindowText(hWnd,wndtitle); */
	}
	nbframe++;
	oldz = position.z;
	oldtime = time;
}


int InitializeRoller(char *filename, int do_intro, int width, int height, int stereoscopic, int cave_mode, float viewAngle, float eyeDist, float focalLength)
{
	point light_dir={1.0f,-0.5f,2.0f};
	stereo = stereoscopic;
	cave = cave_mode;
	doIntro = do_intro;
	aperture = viewAngle;
	eye_sep = eyeDist;
	focallength = focalLength;
	InitGL();
	ReSizeGLScene(width,height);
	if(openfile(filename))
	{
		parsefile();
		closefile();
	}
	InitCurve(averageSegmentLength);
	if(startSegment < 0) startSegment = nbLine + startSegment;
	if(brakeSegment < 0) brakeSegment = nbLine + brakeSegment;
	InitLines();
	InitLiens(0.3f);
	InitNormales();
	InitCouleurs(&light_dir);
	poteau_init(pline,nbLine,ptang);
	LoadGLTextures();
	return 1;
}


void ChangeRollerParameters(float viewAngle, float eyeDist, float focalLength)
{
	if(aperture != viewAngle && !stereo)
	{
		aperture = viewAngle;
		ReSizeGLScene(wndWidth,wndHeight);
	}
	aperture = viewAngle;
	eye_sep = eyeDist;
	focallength = focalLength;
}


int DrawRoller()
{
	static int flag_firstcall = 1;
	static unsigned long firsttime;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(flag_firstcall)
	{
		flag_firstcall = 0;
		firsttime = timeGetTime();
	}
	if(timeGetTime() - firsttime < 36000 && doIntro)
	{
		glViewport(0, 0, wndWidth, wndHeight);
		DrawGLScene0();
		//glutSwapBuffers();
	}
	else
	{
		if(cave==1)
		{
			glViewport(0, 0, wndWidth, wndHeight);
			DrawGLScene(0, 0);
		}
		else if(cave>=5)
		{
			top_center;
			draw_front;
			top_left;
			draw_left;
			top_right;
			draw_right;
			bottom_center;
			draw_bottom;
			bottom_left;
			draw_back;
		}
		if(cave==6)
		{ /* Sixth screen(roof screen), optional. */
			bottom_right;
			draw_top;
		}
		//glutSwapBuffers();
	}
	return ROLLER_NOERROR;
}
