/********************************************************************************
*                                                                               *
*              F O X   P r i v a t e   I n c l u d e   F i l e s                *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997 by Jeroen van der Zijp.   All Rights Reserved.             *
*********************************************************************************
* This library is free software; you can redistribute it and/or                 *
* modify it under the terms of the GNU Library General Public                   *
* License as published by the Free Software Foundation; either                  *
* version 2 of the License, or (at your option) any later version.              *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU             *
* Library General Public License for more details.                              *
*                                                                               *
* You should have received a copy of the GNU Library General Public             *
* License along with this library; if not, write to the Free                    *
* Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            *
*********************************************************************************
* $Id: xincs.h,v 1.15 1998/10/20 17:50:59 jvz Exp $                          *
********************************************************************************/
#ifndef XINCS_H
#define XINCS_H

// Basic includes
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <grp.h>
#include <pwd.h>
#else
#include <direct.h>
#define stat _stat
#define lstat _stat
#define getcwd _getcwd
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#if TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#if HAVE_DIRENT_H
#include <dirent.h>
#define NAMLEN(dirent) strlen((dirent)->d_name)
#else
#define dirent direct
#define NAMLEN(dirent) (dirent)->d_namlen
#if HAVE_SYS_NDIR_H
#include <sys/ndir.h>
#endif
#if HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif
#if HAVE_NDIR_H
#include <ndir.h>
#endif
#endif
#ifdef HAVE_XSHM
#include <sys/ipc.h>
#include <sys/shm.h>
#endif


// X windows includes
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xcms.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#ifdef HUMMINGBIRD
#include <X11/XlibXtra.h>
#endif

// Shared memory
#ifdef HAVE_XSHM
#include <X11/extensions/XShm.h>
#endif

// OpenGL includes
#ifdef HAVE_OPENGL
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#endif

#ifdef WIN32
#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif
#endif

/********************************  Definitions  ********************************/


// Definitions for FXMotifHints.flags
#define MWM_HINTS_FUNCTIONS	(1L << 0)
#define MWM_HINTS_DECORATIONS	(1L << 1)
#define MWM_HINTS_INPUT_MODE	(1L << 2)
#define MWM_HINTS_ALL           (MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS|MWM_HINTS_INPUT_MODE)

// Definitions for FXMotifHints.functions
#define MWM_FUNC_ALL		(1L << 0)
#define MWM_FUNC_RESIZE		(1L << 1)
#define MWM_FUNC_MOVE		(1L << 2)
#define MWM_FUNC_MINIMIZE	(1L << 3)
#define MWM_FUNC_MAXIMIZE	(1L << 4)
#define MWM_FUNC_CLOSE		(1L << 5)

// Definitions for FXMotifHints.decorations 
#define MWM_DECOR_ALL		(1L << 0)
#define MWM_DECOR_BORDER	(1L << 1)
#define MWM_DECOR_RESIZEH	(1L << 2)
#define MWM_DECOR_TITLE		(1L << 3)
#define MWM_DECOR_MENU		(1L << 4)
#define MWM_DECOR_MINIMIZE	(1L << 5)
#define MWM_DECOR_MAXIMIZE	(1L << 6)

// Values for FXMotifHints.inputmode
#define MWM_INPUT_MODELESS		    0
#define MWM_INPUT_PRIMARY_APPLICATION_MODAL 1
#define MWM_INPUT_SYSTEM_MODAL		    2
#define MWM_INPUT_FULL_APPLICATION_MODAL    3


// XDND Version
#define XDND_PROTOCOL_VERSION   3


/*********************************  Typedefs  **********************************/


// The _MWM_HINTS property.
struct FXMotifHints {
  long flags;
  long functions;
  long decorations;
  long inputmode;
  };



#endif
