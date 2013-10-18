/* CAVE port addition
 * by Rodrigo Damazio <rdamazio@lsi.usp.br>
 */

#ifndef ROLLER_CAVE_H
#define ROLLER_CAVE_H

/* Viewport definitions
 * The cave works by splitting the window in 6 pieces, one for each sceen.
 * Here we define macros to address all of them.
 * Further changes would be required for a multi-pipe or cluster system.
 */

#define bottom_left \
   glViewport(0, 0, wndWidth, wndHeight);

#define bottom_center \
   glViewport(wndWidth, 0, wndWidth, wndHeight);

#define bottom_right \
   glViewport(wndWidth*2, 0, wndWidth, wndHeight);

#define top_left \
   glViewport(0, wndHeight, wndWidth, wndHeight);

#define top_center \
   glViewport(wndWidth, wndHeight, wndWidth, wndHeight);

#define top_right \
   glViewport(wndWidth*2, wndHeight, wndWidth, wndHeight);

/* Projection definitions
 * Here we define the rotation necessary for each projection.
 */
#define draw_front DrawGLScene(0,0)
#define draw_left DrawGLScene(M_PI/2,0)
#define draw_right DrawGLScene(-M_PI/2,0)
#define draw_back DrawGLScene(M_PI,0)
#define draw_bottom DrawGLScene(0,-M_PI/2)
#define draw_top DrawGLScene(0,M_PI/2)

#endif

