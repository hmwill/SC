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

#ifndef _ROLLER_HEADER
#define _ROLLER_HEADER

#include "point.h"

#define ROLLER_NOERROR 0
#define ROLLER_GLWINDOWCREATION_ERROR 1

void ReSizeGLScene(int Width, int Height);
int InitializeRoller(char *filename, int do_intro, int width, int height, int stereoscopic, int cave_mode, float viewAngle, float eyeDist, float focalLength);
int DrawRoller();
int ExitRoller();
void ChangeRollerParameters(float viewAngle, float eyeDist, float focalLength);

/* those ones are exported to be initialized by the parser */

extern int nbPointControle;
extern point *ppointcontrole;

extern int startSegment;
extern int brakeSegment;

extern float averageSegmentLength;
extern float twistFactor;

#endif /* _ROLLER_HEADER */
