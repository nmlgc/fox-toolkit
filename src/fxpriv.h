/********************************************************************************
*                                                                               *
*              P r i v a t e   I n t e r n a l   F u n c t i o n s              *
*                                                                               *
*********************************************************************************
* Copyright (C) 2000,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This library is free software; you can redistribute it and/or                 *
* modify it under the terms of the GNU Lesser General Public                    *
* License as published by the Free Software Foundation; either                  *
* version 2.1 of the License, or (at your option) any later version.            *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU             *
* Lesser General Public License for more details.                               *
*                                                                               *
* You should have received a copy of the GNU Lesser General Public              *
* License along with this library; if not, write to the Free Software           *
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.    *
*********************************************************************************
* $Id: fxpriv.h,v 1.16 2002/01/18 22:43:08 jeroen Exp $                         *
********************************************************************************/


// DND protocol version
#define XDND_PROTOCOL_VERSION   4

// Definitions for DND messages for Windows
#ifdef WIN32
#define WM_DND_REQUEST          (WM_APP+1)
#define WM_DND_ENTER            (WM_APP+2)
#define WM_DND_LEAVE            (WM_APP+3)
#define WM_DND_POSITION_REJECT  (WM_APP+4)
#define WM_DND_POSITION_COPY    (WM_APP+5)
#define WM_DND_POSITION_MOVE    (WM_APP+6)
#define WM_DND_POSITION_LINK    (WM_APP+7)
#define WM_DND_POSITION_PRIVATE (WM_APP+8)
#define WM_DND_STATUS_REJECT    (WM_APP+9)
#define WM_DND_STATUS_COPY      (WM_APP+10)
#define WM_DND_STATUS_MOVE      (WM_APP+11)
#define WM_DND_STATUS_LINK      (WM_APP+12)
#define WM_DND_STATUS_PRIVATE   (WM_APP+13)
#define WM_DND_DROP             (WM_APP+14)
#define WM_DND_FINISH           (WM_APP+15)
#define WM_DND_REPLY            (WM_APP+16)
#endif

// Definitions for Motif-style WM Hints.
#ifndef WIN32
#define MWM_HINTS_FUNCTIONS	(1L << 0)       // Definitions for FXMotifHints.flags
#define MWM_HINTS_DECORATIONS	(1L << 1)
#define MWM_HINTS_INPUT_MODE	(1L << 2)
#define MWM_HINTS_ALL           (MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS|MWM_HINTS_INPUT_MODE)

#define MWM_FUNC_ALL		(1L << 0)       // Definitions for FXMotifHints.functions
#define MWM_FUNC_RESIZE		(1L << 1)
#define MWM_FUNC_MOVE		(1L << 2)
#define MWM_FUNC_MINIMIZE	(1L << 3)
#define MWM_FUNC_MAXIMIZE	(1L << 4)
#define MWM_FUNC_CLOSE		(1L << 5)

#define MWM_DECOR_ALL		(1L << 0)       // Definitions for FXMotifHints.decorations
#define MWM_DECOR_BORDER	(1L << 1)
#define MWM_DECOR_RESIZEH	(1L << 2)
#define MWM_DECOR_TITLE		(1L << 3)
#define MWM_DECOR_MENU		(1L << 4)
#define MWM_DECOR_MINIMIZE	(1L << 5)
#define MWM_DECOR_MAXIMIZE	(1L << 6)

#define MWM_INPUT_MODELESS		    0   // Values for FXMotifHints.inputmode
#define MWM_INPUT_PRIMARY_APPLICATION_MODAL 1
#define MWM_INPUT_SYSTEM_MODAL		    2
#define MWM_INPUT_FULL_APPLICATION_MODAL    3
#endif

// Named color
struct FXNamedColor {
  const FXchar *name;
  FXColor       color;
  };



// X11 helpers
#ifndef WIN32
extern FXAPI Atom fxsendrequest(Display *display,Window window,Atom selection,Atom prop,Atom type,FXuint time);
extern FXAPI Atom fxsendreply(Display *display,Window window,Atom selection,Atom prop,Atom target,FXuint time);
extern FXAPI Atom fxsendtypes(Display *display,Window window,Atom prop,FXDragType* types,FXuint numtypes);
extern FXAPI Atom fxrecvtypes(Display *display,Window window,Atom prop,FXDragType*& types,FXuint& numtypes);
extern FXAPI Atom fxsenddata(Display *display,Window window,Atom prop,Atom type,FXuchar* data,FXuint size);
extern FXAPI Atom fxrecvdata(Display *display,Window window,Atom prop,Atom,FXuchar*& data,FXuint& size);
#endif

// Windows helpers
#ifdef WIN32
extern FXAPI HANDLE fxsendrequest(HWND window,HWND requestor,WPARAM type);
extern FXAPI HANDLE fxsenddata(HWND window,FXuchar* data,FXuint size);
extern FXAPI HANDLE fxrecvdata(HANDLE hMap,FXuchar*& data,FXuint& size);
extern FXAPI unsigned int fxmodifierkeys();
extern FXAPI int fxpointsize_to_height(HDC hdc,unsigned size);
extern FXAPI unsigned fxheight_to_pointsize(HDC hdc,int height);
extern FXAPI FXbool fxisconsole(const FXchar *path);
extern UINT wkbGetCodePage();
extern FXuint wkbMapKeyCode(HWND hWnd, UINT iMsg, WPARAM uVirtKey, LPARAM lParam);
extern FXAPI FXbool wkbTranslateMessage(HWND hWnd, UINT iMsg, WPARAM wParam,LPARAM lParam);
#endif


// List of color names
extern FXAPI const FXNamedColor fxcolornames[];


// Number of color names
extern FXAPI const FXuint fxnumcolornames;


// Floyd-Steinberg quantization full 24 bpp to less than or equal to 256 colors
extern FXbool fxfsquantize(FXuchar* p8,const FXuchar* p24,FXuchar* rmap,FXuchar* gmap,FXuchar* bmap,FXint& actualcolors,FXint w,FXint h,FXint maxcolors);

// EZ quantization may be used if w*h<=maxcolors, or if the actual colors used is
// less than maxcolors; using fxezquantize assures that no loss of data occurs
// repeatedly loading and saving the same file!
extern FXbool fxezquantize(FXuchar* p8,const FXuchar* p24,FXuchar* rmap,FXuchar* gmap,FXuchar* bmap,FXint& actualcolors,FXint w,FXint h,FXint maxcolors);

