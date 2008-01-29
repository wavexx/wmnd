/*
 * display: commod graphic functions
 * Copyright(c) 1997 by Alfredo K. Kojima
 * Copyright(c) 2001-2008 by wave++ "Yuri D'Elia" <wavexx@users.sf.net>
 * Distributed under GNU GPL (v2 or above) WITHOUT ANY WARRANTY.
 */

#ifndef DISPLAY_H
#define DISPLAY_H

/* local configuration */
#include "config.h"

/* we may need to define NULL on some compilers */
#ifndef NULL
#define NULL 0x0
#endif

/* usefull display macros */
#define copy_xpm_area(x, y, w, h, dx, dy) \
{ \
  XCopyArea(dockapp.d, dockapp.pixmap, dockapp.pixmap, dockapp.gc, \
	    x, y, w, h, dx, dy); \
  dockapp.update = 1; \
}

/* struct for drawing functions */
struct drwStruct
{
  char* funcName;
  void (*funcPtr)(unsigned long* hist, unsigned mIn, unsigned mOut,
      unsigned size, unsigned long long rx_max, unsigned long long tx_max);
};

/* function's structure */
extern struct drwStruct drwFuncs[];

#endif
