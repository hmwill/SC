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
#endif
#include "gl.h"
//#include "ug.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI  /* some math header doesn't define it */
#define M_PI 3.14159265359f
#endif

#include "point.h"
#include "glbmp.h"

point* ppoteau;
point* plien;
point* ppoteau_coul;
point* plien_coul;
int nbPoteau;
int nbLien;

int nbCotePoteau = 6;
int nbCoteLien = 4;

float rayonPoteau = 0.02f;
static float rayonLien = 0.015f;

float distPoteau = 0.4f;
float hauteurLien = 0.1f;

point light_dir = {1.0f,-0.5f,2.0f};
point lightAmbient = {0.4f,0.4f,0.4f};
point lightDiffuse = {1.0f,0.9f,1.0f};
point light = {1.0f,0.5f,1.0f};

FILE *file;

__inline static void glColor3fv(const GLfloat *v) {
	glColor4f(v[0], v[1], v[2], 1.0f);
}

__inline static void glColor3f(GLfloat red, GLfloat green, GLfloat blue) {
	glColor4f(red, green, blue, 1.0f);
}

void cylinder(point* a, point* b, point* v, int nbcote, float rayon, point *buf, point* bufn){
	int i,k;
	point p1,p2,p4;
	sub(&p1,a,b);
	normalize(&p1);
	p4 = *v;
	add_mult(&p4,prod_scal(v,&p1)/norme2(&p1),&p1);
	normalize(&p4);
	mult(&p4,rayon,&p4);
	k = 0;
	for(i=0 ; i<nbcote ; i++){
		rotate(&p2,&p1,(float)i*2.0f*M_PI/(float)nbcote,&p4);
		add(&buf[k++],a,&p2);
		add(&buf[k++],b,&p2);
	}
	k = 0;
	for(i=0 ; i<nbcote ; i++){
		sub(&p1,&buf[k],a);
		normalize(&p1);
		bufn[k++] = p1;
		sub(&p1,&buf[k],b);
		normalize(&p1);
		bufn[k++] = p1;
	}
}


void calc_coul(int nb, point* r, point* amb, point* dif, point* lig, point* normale){
	int i;
	float f;
	for(i=0 ; i<nb ; i++){
		f = prod_scal(&light_dir,&normale[i]);
		if(f<=0.0f) f = 0.0f;
		r[i].x = dif->x * f + lig->x * amb->x;
		r[i].y = dif->y * f + lig->y * amb->y;
		r[i].z = dif->z * f + lig->z * amb->z;
	}
}


// p est un point de la trajectoire
// v est la tangente à la trajectoire en ce point
void create_poteau(point* p1, point* p2, point* v, point* buf, point* bufc){
	point a,b;
	int i;
	a = *p1;
	a.z = 0.0f;
	b = *p2;
	b.z = 0.0f;
	if(v->z < 0.0f)
	{
		a.z = b.z = -v->z;
		v->z = 0.0f;
	}
	cylinder(p1,&a,v,nbCotePoteau,rayonPoteau,buf,bufc);
	i = nbCotePoteau * 2;
	bufc[i].x = 0.0f; bufc[i].y = 0.0f; bufc[i].z = 1.0f;
	buf[i] = *p1;
	buf[i++].z += rayonPoteau;
	cylinder(p2,&b,v,nbCotePoteau,rayonPoteau,&buf[i],&bufc[i]);
	i += nbCotePoteau * 2;
	bufc[i].x = 0.0f; bufc[i].y = 0.0f; bufc[i].z = 1.0f;
	buf[i] = *p2;
	buf[i++].z += rayonPoteau;
	calc_coul(4*nbCotePoteau+2,bufc,&lightAmbient,&lightDiffuse,&light,bufc);
}


void create_lien(point* p, point* p1, point* p2, point* buf, point* bufc){
	point a;
	float z1, z2;
	z1 = p1->z;
	z2 = p2->z;
	p1->z = p->z - hauteurLien;
	p2->z = p->z - hauteurLien;
	a.x = 0.0f; a.y = 0.0f; a.z = 1.0f;
	cylinder(p,p1,&a,nbCoteLien,rayonLien,buf,bufc);
	cylinder(p,p2,&a,nbCoteLien,rayonLien,&buf[nbCoteLien*2],&bufc[nbCoteLien*2]);
	calc_coul(4*nbCoteLien,bufc,&lightAmbient,&lightDiffuse,&light,bufc);
	p1->z = z1;
	p2->z = z2;
}


void draw_poteau(int nb, point *buf, point *bufc){
	int i,j;
	nb *= 2;
	for(i=0 ; i<nb ; i++){
		glBegin(GL_TRIANGLE_STRIP);
		for(j=0 ; j<2*nbCotePoteau ; j++){
			glColor3fv((float*)&bufc[j+i*(2*nbCotePoteau+1)]);
			glVertex3fv((float*)&buf[j+i*(2*nbCotePoteau+1)]);
		}
		glColor3fv((float*)&bufc[i*(2*nbCotePoteau+1)]);
		glVertex3fv((float*)&buf[i*(2*nbCotePoteau+1)]);
		glColor3fv((float*)&bufc[i*(2*nbCotePoteau+1)+1]);
		glVertex3fv((float*)&buf[i*(2*nbCotePoteau+1)+1]);
		glEnd();
		glBegin(GL_TRIANGLE_FAN);
		glColor3fv((float*)&bufc[(i+1)*(2*nbCotePoteau+1)-1]);
		glVertex3fv((float*)&buf[(i+1)*(2*nbCotePoteau+1)-1]);
		for(j=0 ; j<2*nbCotePoteau ; j+=2){
			glColor3fv((float*)&bufc[j+i*(2*nbCotePoteau+1)]);
			glVertex3fv((float*)&buf[j+i*(2*nbCotePoteau+1)]);
		}
		glColor3fv((float*)&bufc[i*(2*nbCotePoteau+1)]);
		glVertex3fv((float*)&buf[i*(2*nbCotePoteau+1)]);
		glEnd();
	}
}


void draw_lien(int nb, point *buf, point *bufc){
	int i,j;
	nb *= 2;
	for(i=0 ; i<nb ; i++){
		glBegin(GL_TRIANGLE_STRIP);
		for(j=0 ; j<2*nbCoteLien ; j++){
			glColor3fv((float*)&bufc[j+i*(2*nbCoteLien)]);
			glVertex3fv((float*)&buf[j+i*(2*nbCoteLien)]);
		}
		glColor3fv((float*)&bufc[i*(2*nbCoteLien)]);
		glVertex3fv((float*)&buf[i*(2*nbCoteLien)]);
		glColor3fv((float*)&bufc[i*(2*nbCoteLien)+1]);
		glVertex3fv((float*)&buf[i*(2*nbCoteLien)+1]);
		glEnd();
	}
}


int est_relie(point* r, point* p, point* p1, point* p2, int i, point* pline, int nbLine){
	point v,n,z;
	float t;
	sub(&v,p1,p2);
	z.x = 0.0f; z.y = 0.0f; z.z = 1.0f;
	prod_vect(&n,&v,&z);
	sub(&v,&pline[(i+1)%nbLine],&pline[i]);
	if(!intersection(r,&pline[i],&v,p1,&n)) return 0;
	sub(&z,p1,&pline[i]);
	t = prod_scal(&z,&n) / prod_scal(&v,&n);
	if(t<0.0f || t>=1.0f) return 0;
	sub(&v,r,p);
	v.z = 0.0f;
	if(norme(&v)>0.3f) return 0;
	return 1;
}


point panneauPosition = {0.0f,0.0f,0.0f};
point panneauDirection;


void create_support(point* p, point* v, point* pline, int nbLine){
	point a,b,r;
	point p1, p2;
	int i;
	int flag_last_i=0;
	float zmax;
	a = *v;
	a.z = 0.0f;
	normalize(&a);
	if(a.x*a.x>0.0001f){
		a.x = -a.y / a.x;
		a.y = 1.0f;
	}
	else{ //a.y non nul
		a.y = -a.x / a.y;
		a.x = 1.0f;
	}
	normalize(&a);
	mult(&a,distPoteau/2.0f,&a);
	b = *p;
	add(&p1,&b,&a);
	sub(&p2,&b,&a);
	zmax = -1.0f;
	for(i=0 ; i<nbLine ; i++){
		if(est_relie(&r,p,&p1,&p2,i,pline,nbLine)){
			if(!flag_last_i && ((p->z*p->z)<0.0001f || p->z > r.z) && r.z-hauteurLien>-v->z){
				create_lien(&r,&p1,&p2,&plien[nbLien*4*nbCoteLien],&plien_coul[nbLien*4*nbCoteLien]);
				nbLien++;
				flag_last_i = 1;
				if(r.z > zmax) zmax = r.z;
			}
			else flag_last_i = 0;
		}
		else flag_last_i = 0;
	}
	if(zmax > -0.5f){
		p1.z = p2.z = zmax;
		if(v->z > 0.001f){
			panneauPosition = *p;
			panneauPosition.z = v->z;
			p1.z = p2.z = v->z; v->z = 0.0f;
			panneauDirection = *v;
		}
		create_poteau(&p1,&p2,v,&ppoteau[nbPoteau*(4*nbCotePoteau+2)],&ppoteau_coul[nbPoteau*(4*nbCotePoteau+2)]);
		nbPoteau++;
	}
}


glBmpImage panneau;
glBmpImage metal;
glBmpImage arbre;

int init_texture(){
	glBmpInit(&panneau);
	if(!glBmpLoadImage(&panneau,"panneau.tga"))
	{
#ifdef _WIN32
		MessageBox(NULL,_T("Error: Can't open file panneau.tga\n"),_T("RollerCoaster2000"),MB_OK | MB_ICONWARNING);
#else
		fprintf(stderr,"Error: Can't open file panneau.tga\n");
#endif
		exit(5);
	}
	glBmpSetFilter(&panneau,GL_LINEAR_MIPMAP_LINEAR,GL_LINEAR);
	glBmpSetTextureWrap(&panneau,GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE);
	glBmpGenTextureMipMap(&panneau);

	glBmpInit(&metal);
	if(!glBmpLoadImage(&metal,"metal.tga"))
	{
#ifdef _WIN32
		MessageBox(NULL,_T("Error: Can't open file metal.tga\n"),_T("RollerCoaster2000"),MB_OK | MB_ICONWARNING);
#else
		fprintf(stderr,"Error: Can't open file metal.tga\n");
#endif
		exit(5);
	}
	glBmpSetFilter(&metal,GL_LINEAR_MIPMAP_LINEAR,GL_LINEAR);
	glBmpSetTextureWrap(&metal,GL_REPEAT,GL_REPEAT);
	glBmpGenTextureMipMap(&metal);

	glBmpInit(&arbre);
	if(!glBmpLoadImage(&arbre,"tree.tga"))
	{
#ifdef _WIN32
		MessageBox(NULL,_T("Error: Can't open file tree.tga\n"),_T("RollerCoaster2000"),MB_OK | MB_ICONWARNING);
#else
		fprintf(stderr,"Error: Can't open file tree.tga\n");
#endif
		exit(5);
	}
	glBmpSetFilter(&arbre,GL_LINEAR,GL_LINEAR);
	glBmpSetTextureWrap(&arbre,GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE);
	glBmpGenTexture(&arbre);
	return 1;
}


//****************** fonctions exportées *******************


point *ppoteaucoord; /* coordinates were pillars have to be placed */
int *ppoteauabsc;    /* abscisses on the track from the start were pillars have to be placed */
int nbPoteauCoord = 0;
int nbPoteauAbsc = 0;

point *ptree;
int nbTree;

int poteau_init(point *pline, int nbLine, point* ptang){
	point p,v;
	int i;
	int taille_tab;
	point *tab;
	int taille_tab1;
	int *tab1;

	taille_tab = nbPoteauCoord;
	tab = ppoteaucoord;
	taille_tab1 = nbPoteauAbsc;
	tab1 = ppoteauabsc;
	nbPoteau = taille_tab + taille_tab1;
	nbLien = nbPoteau*10;  /******** Here we assume that the maximum is 10 liens for 1 poteau ********/
	normalize(&light_dir);
	ppoteau = (point*)malloc(nbPoteau*(4*nbCotePoteau+2)*sizeof(point));
	ppoteau_coul = (point*)malloc(nbPoteau*(4*nbCotePoteau+2)*sizeof(point));
	plien = (point*)malloc(nbLien*(4*nbCoteLien)*sizeof(point));
	plien_coul = (point*)malloc(nbLien*(4*nbCoteLien)*sizeof(point));
	nbPoteau = nbLien = 0;
	for(i=0 ; i<taille_tab ; i++) create_support(&tab[2*i],&tab[2*i+1],pline,nbLine);
	for(i=0 ; i<taille_tab1 ; i++){
		p = pline[tab1[i]];
		v = ptang[tab1[i]];
		p.z = v.z = 0.0f;
		create_support(&p,&v,pline,nbLine);
	}
	return init_texture();
}


void drawtree()
{
	glEnable(GL_TEXTURE_2D);
	glBmpBind(&arbre);
	//glEnable(GL_ALPHA_TEST);
	//glAlphaFunc(GL_NOTEQUAL,0.0f);
	glColor3f(1.0f,1.0f,1.0f);
	glBegin(GL_TRIANGLE_FAN);
		glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 1.0f);
		glVertex3f(-0.5f, 0.0f, 1.5f);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 1.0f);
		glVertex3f(0.5f, 0.0f, 1.5f);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
		glVertex3f(0.5f, 0.0f, 0.0f);
		glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 0.0f);
		glVertex3f(-0.5f, 0.0f, 0.0f);
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
		glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 0.0f);
		glVertex3f(-0.5f, 0.0f, 0.0f);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
		glVertex3f(0.5f, 0.0f, 0.0f);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 1.0f);
		glVertex3f(0.5f, 0.0f, 1.5f);
		glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 1.0f);
		glVertex3f(-0.5f, 0.0f, 1.5f);
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
		glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 1.0f);
		glVertex3f(0.0f, -0.5f, 1.5f);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 1.0f);
		glVertex3f(0.0f, 0.5f, 1.5f);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.5f, 0.0f);
		glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 0.0f);
		glVertex3f(0.0f, -0.5f, 0.0f);
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
		glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 0.0f);
		glVertex3f(0.0f, -0.5f, 0.0f);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.5f, 0.0f);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 1.0f);
		glVertex3f(0.0f, 0.5f, 1.5f);
		glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 1.0f);
		glVertex3f(0.0f, -0.5f, 1.5f);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}


float metalLength = 2.0f;
float metalAngle = 0.0f;
point metalPosition = {-1.0f,-1.0f,0.5f};


void drawmetal()
{
	typedef struct{
		float x,y;
	} coord2d;
	static coord2d tabt[12];
	static point tabc[] = {
		{-1.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 1.0f},
		{1.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 1.0f},
		{1.0f, 0.0f, 0.0f}
	};
	static point tab1v[] = {
		{-0.25f, 0.0f, -0.02f},
		{-0.25f, 0.0f, 0.04f},
		{-0.5f, 0.0f, 0.04f},
		{-0.5f, 0.0f, 0.0f},
		{0.25f, 0.0f, 0.0f},
		{0.25f, 0.0f, -0.02f}
	};
	static coord2d tab1t[6];
	static point tab1c = {0.0f, -1.0f, 0.0f};
	static point tab2v[6];
	static coord2d tab2t[6];
	static point tab2c = {0.0f, 1.0f, 0.0f};
	int i;
	static int first = 1;
	point p;
	point z = {0.0f, 0.0f, 1.0f};

	if(metalLength == 0.0f) return;
	if(first)
	{
		point lightAmbient = {0.4f,0.4f,0.4f};
		point lightDiffuse = {1.0f,1.0f,1.0f};
		point light = {1.0f,1.0f,1.0f};
		float angle = metalAngle * M_PI / 180;

		for(i=0 ; i<6 ; i++)
		{
			tab1t[i].x = tab1v[i].x * 20.0f;
			tab1t[i].y = (tab1v[i].z + (i!=3 && i!=4 ? metalPosition.z : 0.0f)) * 20.0f;
			tabt[i*2].x = tabt[i*2+1].x = (tab1v[(9-i)%6].x + tab1v[(9-i)%6].z + (i!=0 && i!=5 ? metalPosition.z : 0.0f)) * 20.0f;
			tabt[i*2].y = 0.0f;
			tabt[i*2+1].y = metalLength * 20.0f;
		}
		tab2t[0] = tab1t[0];
		for(i=1 ; i<6 ; i++) tab2t[i] = tab1t[6-i];
		tab2v[0] = tab1v[0];
		for(i=1 ; i<6 ; i++) tab2v[i] = tab1v[6-i];
		for(i=0 ; i<6 ; i++)
		{
			tab1v[i].y -= metalLength / 2.0f;
			rotate(&p,&z,angle,&tab1v[i]);
			add(&tab1v[i],&metalPosition,&p);
			tab2v[i].y += metalLength / 2.0f;
			rotate(&p,&z,angle,&tab2v[i]);
			add(&tab2v[i],&metalPosition,&p);
		}
		tab1v[3].z = tab1v[4].z = 0.0f;
		tab2v[2].z = tab2v[3].z = 0.0f;
		rotate(&p,&z,angle,&tab1c);
		tab1c = p;
		rotate(&p,&z,angle,&tab2c);
		tab2c = p;
		for(i=0 ; i<5 ; i++)
		{
			rotate(&p,&z,angle,&tabc[i]);
			tabc[i] = p;
		}
		calc_coul(5,tabc,&lightAmbient,&lightDiffuse,&light,tabc);
		calc_coul(1,&tab1c,&lightAmbient,&lightDiffuse,&light,&tab1c);
		calc_coul(1,&tab2c,&lightAmbient,&lightDiffuse,&light,&tab2c);
		first = 0;
	}
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_FLAT);
	glBmpBind(&metal);
	glBegin(GL_TRIANGLE_STRIP);
	for(i=0 ; i<6 ; i++)
	{
		glColor3fv((float*)&tabc[(i+4)%5]);
		glMultiTexCoord2f(GL_TEXTURE0, tabt[2*i].x, tabt[2*i].y);
		glVertex3fv((float*)&tab2v[(3+i)%6]);
		glMultiTexCoord2f(GL_TEXTURE0, tabt[2*i+1].x, tabt[2*i+1].y);
		glVertex3fv((float*)&tab1v[(9-i)%6]);
	}
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
	glColor3fv((float*)&tab1c);
	for(i=0 ; i < sizeof(tab1v)/sizeof(point) ; i++)
	{
		glMultiTexCoord2f(GL_TEXTURE0, tab1t[i].x, tab1t[i].y);
		glVertex3fv((float*)&tab1v[i]);
	}
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
	glColor3fv((float*)&tab2c);
	for(i=0 ; i < sizeof(tab2v)/sizeof(point) ; i++)
	{
		glMultiTexCoord2f(GL_TEXTURE0, tab2t[i].x, tab2t[i].y);
		glVertex3fv((float*)&tab2v[i]);
	}
	glEnd();
	glShadeModel(GL_SMOOTH);
	glDisable(GL_TEXTURE_2D);
}


void drawpanneau()
{
	typedef struct{
		float x,y;
	} coord2d;
	static point tabv[] = {
		{-0.4f, -0.03f, -0.0f},
		{ 0.4f, -0.03f, -0.0f},
		{-0.4f,  0.03f, -0.0f},
		{ 0.4f,  0.03f, -0.0f},
		{-0.4f,  0.03f, -0.7f},
		{ 0.4f,  0.03f, -0.7f},
		{-0.4f, -0.03f, -0.7f},
		{ 0.4f, -0.03f, -0.7f}
	};
	static coord2d tabt[] = {
		{0.0f, 0.0f},
		{16.0f, 0.0f},
		{0.0f, 1.2f},
		{16.0f, 1.2f},
		{0.0f, 11.2f},
		{16.0f, 11.2f},
		{0.0f, 12.4f},
		{16.0f, 12.4f}
	};
	static point tabc[] = {
		{0.0f, 0.0f, 1.0f},
		{0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, -1.0f},
		{1.0f, 0.0f, 0.0f},
		{-1.0f, 0.0f, 0.0f},
		{0.0f, -1.0f, 0.0f}
	};
	static int first_call = 1;
	int i;

	if(panneauPosition.z == 0.0f) return;
	if( first_call )
	{
		point lightAmbient = {0.4f,0.4f,0.4f};
		point lightDiffuse = {1.0f,1.0f,1.0f};
		point light = {1.0f,1.0f,1.0f};
		point a, z={0.0f,0.0f,1.0f};

		normalize(&panneauDirection);
		a = panneauDirection;
		if(a.x*a.x>0.0001f){
			a.x = -a.y / a.x;
			a.y = 1.0f;
		}
		else{
			a.y = -a.x / a.y;
			a.x = 1.0f;
		}
		if(panneauDirection.x * a.y - panneauDirection.y * a.x > 0.0f)
		{
			a.x = -a.x; a.y = -a.y; a.z = -a.z;
		}
		for(i=0 ; i<6 ; i++)
		{
			point v = tabc[i];
			tabc[i].x = a.x * v.x + panneauDirection.x * v.y + z.x * v.z;
			tabc[i].y = a.y * v.x + panneauDirection.y * v.y + z.y * v.z;
			tabc[i].z = a.z * v.x + panneauDirection.z * v.y + z.z * v.z;
		}
		for(i=0 ; i<8 ; i++)
		{
			point v = tabv[i];
			tabv[i].x = a.x * v.x + panneauDirection.x * v.y + z.x * v.z;
			tabv[i].y = a.y * v.x + panneauDirection.y * v.y + z.y * v.z;
			tabv[i].z = a.z * v.x + panneauDirection.z * v.y + z.z * v.z;
			add(&tabv[i],&panneauPosition,&tabv[i]);
		}
		calc_coul(sizeof(tabc)/sizeof(point),tabc,&lightAmbient,&lightDiffuse,&light,tabc);
		first_call = 0;
	}
	glShadeModel(GL_FLAT);
	glEnable(GL_TEXTURE_2D);
	glBmpBind(&panneau);
	glBegin(GL_TRIANGLE_FAN);
		glColor3fv((float*)&tabc[5]);
		glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 1.0f);
		glVertex3fv((float*)&tabv[1]);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 1.0f);
		glVertex3fv((float*)&tabv[0]);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
		glVertex3fv((float*)&tabv[6]);
		glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 0.0f);
		glVertex3fv((float*)&tabv[7]);
	glEnd();
	glBmpBind(&metal);
	glBegin(GL_TRIANGLE_STRIP);
	for(i=0 ; i < sizeof(tabv)/sizeof(point) ; i++)
	{
		glColor3fv((float*)&tabc[i>1?(i-2)/2:0]);
		glMultiTexCoord2f(GL_TEXTURE0, tabt[i].x, tabt[i].y);
		glVertex3fv((float*)&tabv[i]);
	}
	glEnd();
	glBegin(GL_TRIANGLE_FAN);
		glColor3fv((float*)&tabc[3]);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
		glVertex3fv((float*)&tabv[1]);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 10.0f);
		glVertex3fv((float*)&tabv[7]);
		glMultiTexCoord2f(GL_TEXTURE0, 1.2f, 10.0f);
		glVertex3fv((float*)&tabv[5]);
		glMultiTexCoord2f(GL_TEXTURE0, 1.2f, 0.0f);
		glVertex3fv((float*)&tabv[3]);
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
		glColor3fv((float*)&tabc[4]);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
		glVertex3fv((float*)&tabv[0]);
		glMultiTexCoord2f(GL_TEXTURE0, 1.2f, 0.0f);
		glVertex3fv((float*)&tabv[2]);
		glMultiTexCoord2f(GL_TEXTURE0, 1.2f, 10.0f);
		glVertex3fv((float*)&tabv[4]);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 10.0f);
		glVertex3fv((float*)&tabv[6]);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
}


void poteau_draw()
{
	int i;
	point v;

	drawpanneau();
	v.x = v.y = v.z = 0.0f;
	glPushMatrix();
	for(i=0 ; i<nbTree ; i++)
	{
		glTranslatef(ptree[i].x - v.x,ptree[i].y - v.y,ptree[i].z - v.z);
		v = ptree[i];
		//drawtree();
	}
	glPopMatrix();
	draw_poteau(nbPoteau,ppoteau,ppoteau_coul);
	draw_lien(nbLien,plien,plien_coul);
	drawmetal();
}
