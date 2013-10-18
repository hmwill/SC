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

//#include "ug.h"
#include <SDL.h>
#include "gl.h"
#include "vgl.h"
//#include "ug.h"

//#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "roller.h"

#define	WINDOW_WIDTH	400
#define WINDOW_HEIGHT	300

/* Re-introduce 32-bit word encoding of 32-bit RGBA values */
#define GL_UNSIGNED_INT_8_8_8_8           0x8035
#define GL_UNSIGNED_INT_8_8_8_8_REV       0x8367

/* The number of our GLUT window */
int window;
SDL_Surface *sdlSurface;
VGL_Surface glSurface;

int width = WINDOW_WIDTH;
int height = WINDOW_HEIGHT;
int fullscreen = 0;
int caveMode = 1;
static int stereoFlag = 0;
float viewAngle = 60.0f;
static float eyeDist = 0.005f;
static float focalLength = 0.05f;
char *trackName = "freestyle.trk"; /* name of the file containing track description */
int intro = 0;


void display(void)
{
	DrawRoller();
	vglSwapBuffers(sdlSurface, glSurface);
}


void resize(int x, int y)
{
	if(caveMode==5 || caveMode==6)
	{
		x /= 3;
		y /= 2;
	}
	ReSizeGLScene(x,y);
	glScissor(WINDOW_WIDTH/10, WINDOW_HEIGHT/10, WINDOW_WIDTH*7/10, WINDOW_HEIGHT*7/10);
	glEnable(GL_SCISSOR_TEST);
	display();
}

#define ESCAPE 27

#if 0
void keyboard(unsigned char key, int x, int y)
{
	switch(key)
	{
		case '+':
			eyeDist += 0.0001f;
			ChangeRollerParameters(viewAngle,eyeDist,focalLength);
			break;
		case '-':
			if(eyeDist > 0.0f) eyeDist -= 0.0001f;
			ChangeRollerParameters(viewAngle,eyeDist,focalLength);
			break;
		case 'q' :
		case ESCAPE :
			glutDestroyWindow(window);
			exit(0);
	}
}


void specialKeyboard(int key, int x, int y)
{
	switch (key) {
		case GLUT_KEY_LEFT:
			if(viewAngle > 20.0f) viewAngle -= 1.0f;
			ChangeRollerParameters(viewAngle,eyeDist,focalLength);
			break;
		case GLUT_KEY_RIGHT:
			if(viewAngle < 90.0f) viewAngle += 1.0f;
			ChangeRollerParameters(viewAngle,eyeDist,focalLength);
			break;
		case GLUT_KEY_UP:
			focalLength += 0.005f;
			ChangeRollerParameters(viewAngle,eyeDist,focalLength);
			break;
		case GLUT_KEY_DOWN:
			if(focalLength > 0.01f) focalLength -= 0.005f;
			ChangeRollerParameters(viewAngle,eyeDist,focalLength);
			break;
	}
}

#endif

void mouse(int button, int state, int x, int y)
{
}


void mousemotion(int x, int y)
{
}

#if 0
void usage()
{
	char msg[800];

	snprintf(msg,800,"RollerCoaster2000 by Plusplus\nUsage: roller [options]\nOptions:\n\
 -h            print this help\n\
 -f            select fullscreen mode\n\
 -s            select stereoscopic mode (for use with a stereo display device)\n\
 -c {1|5|6}    select C.A.V.E. mode(5 or 6 screens, or 1 for normal mode)\n\
 -t <filename> select a file containing a track description (default rc2k.trk)\n\
 -x <width>    select window width (default 800)\n\
 -y <height>   select window height (default 600)\n\
 -i            disables intro(skip straight to ride)\n\n");
#ifdef _WIN32
	MessageBoxA(NULL,msg,"RollerCoaster2000",MB_OK);
#else
	printf(msg);
#endif
}
#endif

#if 0
int main(int argc, char* argv[])
{
	int i;

	glutInit(&argc,argv);
	i = 1;
	while(i<argc)
	{
		if(!strcmp(argv[i],"-h"))
		{
			usage();
			return 0;
		}
		if(!strcmp(argv[i],"-f"))
		{
			fullscreen = 1;
			i++;
			continue;
		}
		if(!strcmp(argv[i],"-s"))
		{
			stereoFlag = 1;
			i++;
			caveMode = 1;
			continue;
		}
		if(!strcmp(argv[i],"-i"))
		{
			intro = 0;
			i++;
			continue;
		}
		if(!strcmp(argv[i],"-c"))
		{
			if(++i < argc)
			{
				caveMode = atoi(argv[i++]);
				stereoFlag = 0;
			}
			if(caveMode!=1 && caveMode!=5 && caveMode!=6)
			{
				fprintf(stderr, "Invalid C.A.V.E. mode parameter - can be 1(normal mode), 5(five-screen CAVE), or 6(six-screen CAVE).\n");
				exit(-1);
			}
			continue;
		}
		if(!strcmp(argv[i],"-t"))
		{
			if(++i < argc)
			{
				trackName = argv[i];
			}
			i++;
			continue;
		}
		if(!strcmp(argv[i],"-x"))
		{
			if(++i < argc)
			{
				int value = atoi(argv[i]);
				if(value) width = value;
			}
			i++;
			continue;
		}
		if(!strcmp(argv[i],"-y"))
		{
			if(++i < argc)
			{
				int value = atoi(argv[i]);
				if(value) height = value;
			}
			i++;
			continue;
		}
		i++;
	}
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	/* glutInitWindowPosition(200,200); */
	if(caveMode!=1)
	{
		glutInitWindowSize(width*3,height*2);
		viewAngle = 90.0f;
		intro = 0;
	}
	else
	{
		glutInitWindowSize(width,height);
	}

	window = glutCreateWindow("RollerCoaster2000");
	if(fullscreen) glutFullScreen();

	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKeyboard);
	glutReshapeFunc(resize);
	glutMouseFunc(mouse);
	glutMotionFunc(mousemotion);
	glutSetCursor(GLUT_CURSOR_NONE);

	glutMainLoop();
	return 0;
}
#endif

void init() {
	InitializeRoller(trackName,intro,width,height,0,caveMode,viewAngle,eyeDist,focalLength);
}

int main(int argc, char* argv[]) 
{
	SDL_Event event;

	SDL_Init(SDL_INIT_VIDEO);
	atexit(SDL_Quit);
	vglInitialize();

	sdlSurface = SDL_SetVideoMode(width, height, 0, SDL_ANYFORMAT);
	glSurface = vglCreateSurface(width, height, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, 0);
	vglMakeCurrent(glSurface, glSurface);

	init();
	resize(width, height);
	display();

	for (;;) {
		SDL_PollEvent(&event);

		if (event.type == SDL_QUIT) {
			break;
		}

		display();
	}

	return 0;
}