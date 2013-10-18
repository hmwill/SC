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

#ifndef _POTEAU_HEADER
#define _POTEAU_HEADER

int poteau_init();
void poteau_draw();
void drawtree();

/* those ones are exported to be initialized by the parser */

extern point *ppoteaucoord;
extern int *ppoteauabsc;
extern int nbPoteauCoord;
extern int nbPoteauAbsc;

extern point *ptree;
extern int nbTree;

extern float metalLength;
extern float metalAngle;
extern point metalPosition;

#endif /* _POTEAU_HEADER */
