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

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "point.h"
#include "roller.h"
#include "poteau.h"
#include "parser.h"


#define LF 0x0A  /* line feed      */
#define CR 0x0D  /* chariot return */


FILE *file;
static int line_number = 1;

#define snprintf _snprintf
#define vsnprintf _vsnprintf

void printerror(char *str, ...)
{
	va_list ap;
	char buf1[1024];
	char buf2[512];

#if 0
	wsnprintf(buf1,1024,"Error line %d: ",line_number);
	va_start(ap,str);
	vsnprintf(buf2,512,str,ap);
	va_end(ap);
	strcat(buf1,buf2);
	strcat(buf1,"\n");
#endif
#ifdef _WIN32
	MessageBox(NULL,_T("Parser Error"),_T("RollerCoaster2000 Parser Error"),MB_OK | MB_ICONWARNING);
#else
	fprintf(stderr,buf1);
#endif
}


/*
 * this function makes many translation
 * upper case -> lower case : so we only read lower case letters
 * chariot return -> space : to be compatible with dos/windows file format
 * we also increment line number when needed
 */
int readchar()
{
	int c;
	c = getc(file);
	if(c >= 'A' && c<='Z')
	{
		return c - 'A' + 'a';
	}
	if(c == CR)
	{
		return ' ';
	}
	if(c == LF)
	{
		line_number++;
	}
	return c;
}


int ungetchar(int c)
{
	if(c == LF) line_number--;
	if(c == EOF) return EOF;
	else return ungetc(c,file);
}


void skipwhite()
{
	int c;

	while((c = readchar()) != EOF)
	{
		if(c != ' ' && c != '\t')
		{
			ungetchar(c);
			return;
		}
	}
}


void skipcomment() /* actually it skips comments and white and new line */
{
	int c;

	skipwhite();
	if((c = readchar()) == EOF) return;
	while(c == LF || c=='#')
	{
		if(c == '#')
		{
			c = readchar();
			while(c != EOF && c != LF) c = readchar();
		}
		skipwhite();
		c = readchar();
	}
	ungetchar(c);
}


int readfloat(float *f)
{
	int c;
	int minus = 0;
	float d = 0.0f;

	*f = 0.0f;
	if((c = readchar()) == EOF) return 0;
	if(c == '-') minus = 1;
	else
	{
		if(c != '+') ungetchar(c);
	}
	c = readchar();
	if((c < '0' || c > '9') && c != '.')
	{
		ungetchar(c);
		return 0;
	}
	ungetchar(c);
	while((c = readchar()) != EOF)
	{
		if(c == '.')
		{
			if(d == 0.0f)
			{
				d = 1.0f;
				continue;
			}
			else
			{
				ungetchar(c);
				return 1;
			}
		}
		if(c >= '0' && c <= '9')
		{
			if(d == 0.0f) *f = *f * 10.0f + (float)(c - '0');
			else *f += (float)(c - '0') / (d *= 10.0f);
		 	continue;
		}
		ungetchar(c);
		if(d != 1.0f)
		{
			if(minus) *f = -*f;
			return 1;
		}
		else return 0;
	}
	if(d != 1.0f)
	{
			if(minus) *f = -*f;
			return 1;
	}
	else return 0;
}


int readpositiveinteger(int *i)
{
	int c;

	*i = 0;
	if((c = readchar()) == EOF) return 0;
	if(c < '0' || c > '9')
	{
		ungetchar(c);
		return 0;
	}
	ungetchar(c);
	while((c = readchar()) != EOF)
	{
		if(c >= '0' && c <= '9')
		{
			*i = *i * 10 + c - '0';
		 	continue;
		}
		ungetchar(c);
		return 1;
	}
	return 1;
}


int readname(char *str, int length)
{
	int c;
	int i = 0;

	str[i] = 0;
	if((c = readchar()) == EOF) return 0;
	ungetchar(c);
	if(c < 'a' || c > 'z')
	{
		return 0;
	}
	while((c = readchar()) != EOF)
	{
		if(c >= 'a' && c <= 'z')
		{
			if(i < length - 1) str[i++] = c;
			else str[i] = 0;
			continue;
		}
		str[i] = 0;
		ungetchar(c);
		return 1;
	}
	str[i] = 0;
	return 1;
}


int skipnewline()
{
	int old = line_number;

	skipcomment();
	if(line_number - old > 0)
	{
		return 1;
	}
	else
	{
		printerror("Newline expected");
		return 0;
	}
}


int readsignedint(int *i)
{
	int c, minus = 0;

	if((c = readchar()) == EOF)
	{
		printerror("Integer expected");
		return 0;
	}
	switch(c)
	{
		case '-':
			minus = 1;
			break;
		case '+':
			break;
		default :
			ungetchar(c);
	}
	if(!readpositiveinteger(i))
	{
		printerror("Integer expected");
		return 0;
	}
	if(minus) *i = -*i;
	return 1;
}


int readunsignedint(int *i)
{
	if(!readpositiveinteger(i))
	{
		printerror("Positive integer expected");
		return 0;
	}
	return 1;
}


int readpoint(point *p)
{
	if(!readfloat(&p->x))
	{
		printerror("Number expected");
		return 0;
	}
	skipwhite();
	if(!readfloat(&p->y))
	{
		printerror("Number expected");
		return 0;
	}
	skipwhite();
	if(!readfloat(&p->z))
	{
		printerror("Number expected");
		return 0;
	}
	return 1;
}


int readseparator()
{
	int c;

	skipwhite();
	c = readchar();
	if(c == ';' || c==',')
	{
		skipwhite();
		return 1;
	}
	else
	{
		ungetchar(c);
		printerror("Separator expected");
		return 0;
	}
}


int readfile()
{
	char name[256];
	int nameok;
	int i;

	skipcomment();
	while(!feof(file))
	{
		if(!readname(name,256))
		{
			printerror("Section name expected");
			return 0;
		}
		nameok = 0;
		if(!strcmp(name,"track"))
		{
			nameok = 1;
			if(!skipnewline()) return 0;
			if(!readunsignedint(&nbPointControle)) return 0;
			ppointcontrole = (point*)malloc(nbPointControle*2*sizeof(point));
			for(i=0 ; i<nbPointControle ; i++)
			{
				if(!skipnewline()) return 0;
				if(!readpoint(&ppointcontrole[i*2])) return 0;
				if(!readseparator()) return 0;
				if(!readpoint(&ppointcontrole[i*2+1])) return 0;
			}
		}
		if(!strcmp(name,"supportcoordinate"))
		{
			nameok = 1;
			if(!skipnewline()) return 0;
			if(!readunsignedint(&nbPoteauCoord)) return 0;
			ppoteaucoord = (point*)malloc(nbPoteauCoord*2*sizeof(point));
			for(i=0 ; i<nbPoteauCoord ; i++)
			{
				if(!skipnewline()) return 0;
				if(!readpoint(&ppoteaucoord[i*2])) return 0;
				if(!readseparator()) return 0;
				if(!readpoint(&ppoteaucoord[i*2+1])) return 0;
			}
		}
		if(!strcmp(name,"supportabscisse"))
		{
			nameok = 1;
			if(!skipnewline()) return 0;
			if(!readunsignedint(&nbPoteauAbsc)) return 0;
			ppoteauabsc = (int*)malloc(nbPoteauAbsc*sizeof(int));
			for(i=0 ; i<nbPoteauAbsc ; i++)
			{
				if(!skipnewline()) return 0;
				if(!readunsignedint(&ppoteauabsc[i])) return 0;
			}
		}
		if(!strcmp(name,"tree"))
		{
			nameok = 1;
			if(!skipnewline()) return 0;
			if(!readunsignedint(&nbTree)) return 0;
			ptree = (point*)malloc(nbTree*sizeof(point));
			for(i=0 ; i<nbTree ; i++)
			{
				if(!skipnewline()) return 0;
				if(!readpoint(&ptree[i])) return 0;
				ptree[i].z = 0.0f;
			}
		}
		if(!strcmp(name,"box"))
		{
			nameok = 1;
			if(!skipnewline()) return 0;
			if(!readpoint(&metalPosition)) return 0;
			if(!skipnewline()) return 0;
			if(!readfloat(&metalLength))
			{
				printerror("Number expected");
				return 0;
			}
			if(!skipnewline()) return 0;
			if(!readfloat(&metalAngle))
			{
				printerror("Number expected");
				return 0;
			}
		}
		if(!strcmp(name,"startsegment"))
		{
			nameok = 1;
			if(!skipnewline()) return 0;
			if(!readsignedint(&startSegment)) return 0;
		}
		if(!strcmp(name,"brakesegment"))
		{
			nameok = 1;
			if(!skipnewline()) return 0;
			if(!readsignedint(&brakeSegment)) return 0;
		}
		if(!strcmp(name,"averagesegmentlength"))
		{
			nameok = 1;
			if(!skipnewline()) return 0;
			if(!readfloat(&averageSegmentLength))
			{
				printerror("Number expected");
				return 0;
			}
		}
		if(!strcmp(name,"tiltfactor"))
		{
			nameok = 1;
			if(!skipnewline()) return 0;
			if(!readfloat(&twistFactor))
			{
				printerror("Number expected");
				return 0;
			}
		}
		if(!nameok) printerror("Unknown section name \"%s\"",name);
		/* here we don't want to print an error if there is no */
		/* new line at the end of the file, so we don't directly */
		/* call skipnewline */
		i = line_number;
		skipcomment();
		if((line_number - i == 0) && !feof(file))
			printerror("Newline expected");
	}
	return 1;
}


void initinfo()
{
	nbPointControle = 0;
	nbPoteauCoord = 0;
	nbPoteauAbsc = 0;
	nbTree = 0;
	metalLength = 0.0f;
	startSegment = 0;
	brakeSegment = -20;
	twistFactor = 0.0f;
	averageSegmentLength = 0.15f;
}


void initdefault()
{
	point tab1[] = {
		{-1.0f, 0.0f, 0.50f}, {0.0f, 1.0f, 0.0f},
		{-1.0f, 3.0f, 3.00f}, {0.0f, 1.0f, 0.0f},
		{0.0f, 4.0f, 2.99f}, {0.0f, 0.0f, 0.0f},
		{1.0f, 3.0f, 2.98f}, {0.0f, 1.0f, 0.0f},
		{1.0f, 2.5f, 2.975f}, {0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 0.50f}, {-0.5f, -0.7f, 0.0f},
		{-1.0f, -2.5f, 2.905f}, {0.0f, 1.0f, 0.0f},
		{-1.0f, -3.0f, 2.90f}, {0.0f, 1.0f, 0.0f},
		{0.0f, -4.0f, 2.85f}, {0.0f, 0.0f, 0.0f},
		{1.0f, -3.0f, 2.80f}, {0.0f, 1.0f, 0.0f},
		{1.0f, 0.0f, 0.70f}, {0.0f, 1.0f, 0.0f},
		{1.0f, 3.0f, 2.50f}, {0.0f, 1.0f, 0.0f},
		{0.0f, 4.0f, 2.35f}, {0.0f, 0.0f, 0.0f},
		{-1.0f, 3.0f, 2.20f}, {0.0f, 0.0f, 0.0f},
		{0.0f, 2.0f, 2.05f}, {0.0f, 0.0f, 0.0f},
		{1.0f, 3.0f, 1.90f}, {0.0f, 1.0f, 0.0f},
		{0.0f, 4.0f, 1.75f}, {0.0f, 0.0f, 0.0f},
		{-1.0f, 3.0f, 1.60f}, {0.0f, 1.0f, 0.0f},
		{-0.2f, 0.0f, 1.00f}, {0.0f, 0.0f, 0.0f},
		{-0.1f, -1.0f, 1.70f}, {0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 2.50f}, {0.0f, 0.0f, 0.0f},
		{0.1f, 1.0f, 1.70f}, {0.0f, 0.0f, 0.0f},
		{0.2f, 0.0f, 1.00f}, {0.0f, 0.0f, 0.0f},
		{1.0f, -3.0f, 2.00f}, {0.0f, 1.0f, 0.0f},
		{0.0f, -4.0f, 1.85f}, {0.0f, 0.0f, 0.0f},
		{-1.0f, -3.0f, 1.70f}, {0.0f, 0.0f, 0.0f},
		{0.0f, -2.0f, 1.55f}, {0.0f, 0.0f, 0.0f},
		{1.0f, -3.0f, 1.40f}, {0.0f, 0.0f, 0.0f},
		{0.0f, -4.0f, 1.25f}, {0.0f, 0.0f, 0.0f},
		{-1.0f, -3.0f, 1.10f}, {0.0f, 0.0f, 0.0f},
		{0.0f, -2.0f, 0.95f}, {0.0f, 0.0f, 0.0f},
		{1.0f, -3.0f, 0.80f}, {0.0f, 0.0f, 0.0f},
		{0.0f, -4.0f, 0.65f}, {0.0f, 0.0f, 0.0f},
		{-1.0f, -3.0f, 0.50f}, {0.0f, 1.0f, 0.0f},
	};
	point tab2[] = {
		{0.0f, 4.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
		{0.0f, 2.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
		{1.0f, 3.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
		{-1.0f, 3.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
		{0.0f, -4.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
		{0.0f, -2.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
		{1.0f, -3.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
		{-1.0f, -3.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
		{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 0.6f}, {-0.5f, -0.7f, 0.0f},
		{-1.0f, -1.5f, 0.0f}, {0.0f, 1.0f, -0.48f},
		{-1.0f, 1.2f, 0.0f}, {0.0f, 1.0f, 3.5f}
	};
	int tab3[]={267, 321, 285, 300};
	int i;

	nbPointControle = 34;
	nbPoteauCoord = 12;
	nbPoteauAbsc = 4;
	ppointcontrole = (point*)malloc(nbPointControle*2*sizeof(point));
	for(i=0 ; i<2*nbPointControle ; i++)
		ppointcontrole[i] = tab1[i];
	ppoteaucoord = (point*)malloc(nbPoteauCoord*2*sizeof(point));
	for(i=0 ; i<2*nbPoteauCoord ; i++)
		ppoteaucoord[i] = tab2[i];
	ppoteauabsc = (int*)malloc(nbPoteauAbsc*sizeof(int));
	for(i=0 ; i<nbPoteauAbsc ; i++)
		ppoteauabsc[i] = tab3[i];
	nbTree = 3;
	ptree = (point*)malloc(nbTree*sizeof(point));
	ptree[0].x = 3.5f; ptree[0].y = 0.0f; ptree[0].z = 0.0f;
	ptree[1].x = 3.5f; ptree[1].y = 1.0f; ptree[1].z = 0.0f;
	ptree[2].x = 3.5f; ptree[2].y = -1.0f; ptree[2].z = 0.0f;
	metalLength = 2.0f;
	metalAngle = 0.0f;
	metalPosition.x = -1.0f;
	metalPosition.y = -1.0f;
	metalPosition.z = 0.5f;
	startSegment = -5;
	brakeSegment = -20;
	twistFactor = 5.0f;
	averageSegmentLength = 0.15f;
}


int parsefile()
{
	initinfo();
	if(readfile())
	{
		if(nbPointControle != 0) return 1;
	}
	initdefault();
	return 0;
}


int openfile(char *str)
{
	file = fopen(str,"r");
	if(file == NULL)
	{
		initdefault();
		return 0;
	}
	return 1;
}


int closefile()
{
	return fclose(file);
}

/*
int main(int argc, char *argv[])
{
	float f;
	int res;

	file = stdin;
	return parsefile();
}
*/
