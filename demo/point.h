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

#ifndef _POINT_HEADER
#define _POINT_HEADER

typedef struct{
	float x,y,z;
} point;


extern void mult(point*, float, point*);

extern void add_mult(point*, float, point*);

extern float norme2(point*);

extern float norme(point*);

extern void normalize(point*);

extern void sub(point*, point*, point*);

extern void add(point*, point*, point*);

extern float prod_scal(point*, point*);

extern void prod_vect(point*, point*, point*);

extern void rotate(point*, point*, float, point*);

extern int intersection(point*, point*, point*, point*, point*);

#endif /* _POINT_HEADER */
