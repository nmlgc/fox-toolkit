/********************************************************************************
*                                                                               *
*                         T o p   W i n d o w   O b j e c t                     *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXTopWindow.cpp,v 1.72 2002/01/18 22:43:07 jeroen Exp $                  *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxpriv.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXRegistry.h"
#include "FXAccelTable.h"
#include "FXApp.h"
#include "FXDCWindow.h"
#include "FXCursor.h"
#include "FXIcon.h"
#include "FXTopWindow.h"
#include "FXToolbar.h"
#include "FXToolbarGrip.h"

/*
  Notes:
  - Need to trap & forward iconify messages.
  - Handle zero width/height case similar to FXWindow.
  - Pass Size Hints to Window Manager as per ICCCM.
  - Add padding options, as this is convenient for FXDialogBox subclasses;
    for FXTopWindow/FXMainWindow, padding should default to 0, for FXDialogBox,
    default to something easthetically pleasing...
  - Now observes LAYOUT_FIX_X and LAYOUT_FIX_Y hints.
  - LAYOUT_FIX_WIDTH and LAYOUT_FIX_HEIGHT take precedence over PACK_UNIFORM_WIDTH and
    PACK_UNIFORM_HEIGHT!
  - We should use some sort of SizeHints to let the WM know about our desired size.

    David Heath <dave@hipgraphics.com>

      I have something that appears to work under X/Motif, but even there I
      have not tested it under many window managers. Here is a little
      code snippet which sets the variables wmLeft, wmRight, wmTop, wmBottom
      to the border sizes.

            Window w, rw;
            unsigned int border, depth;
            unsigned int sx, sy, msx, msy;
            int ox, oy;
            Display *dpy = theApp->display ();

            w = XtWindow ((theApp->mainWindow ())->mainWindowWidget ());
            XGetGeometry (dpy, w, &rw, &ox, &oy, &msx, &msy, &border, &depth);

            wmLeft = wmRight = wmTop = wmBottom = 0;

            Window pw, c[100];
            unsigned int cn;

            do{
              XQueryTree (dpy, w, &rw, &pw, (Window **) &c, &cn);
              XGetGeometry (dpy, w, &rw, &ox, &oy, &sx, &sy, &border, &depth);
              printf ("Window 0x%p [%4dx%4d] +[%4d,%4d], border %4d\n", w, sx, sy, ox, oy, border);
              if(pw != rw){
                wmLeft += ox;
                wmTop += oy;
                }
              w = pw;
              }
            while(w != rw);
            wmRight  = (sx - msx - wmLeft);
            wmBottom = (sy - msy - wmTop);
            printf ("WM Borders l:%d r:%d t:%d b:%d\n", wmLeft, wmRight, wmTop, wmBottom);

      One thing that I would find useful, if you do eventually add code
      to fox to determine these sizes, that you make the values accessible
      to the API in the rare case that someone needs them.


    Theo Veenker <theo.veenker@let.uu.nl>

  - We need new toolbar layout modes:

      +--------+---+----------+----+
      |  bar1  |///|  bar2    |////|
      +--------+---+-------+--+----+
      |      bar3          |///////|
      +--------------------+-------+

  - Need to have some way to start window initially as "iconified".

*/


// Side layout modes
#define LAYOUT_SIDE_MASK (LAYOUT_SIDE_LEFT|LAYOUT_SIDE_RIGHT|LAYOUT_SIDE_TOP|LAYOUT_SIDE_BOTTOM)

// Layout modes
#define LAYOUT_MASK (LAYOUT_SIDE_MASK|LAYOUT_RIGHT|LAYOUT_CENTER_X|LAYOUT_BOTTOM|LAYOUT_CENTER_Y|LAYOUT_FIX_X|LAYOUT_FIX_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y)

#define DISPLAY(app) ((Display*)((app)->display))



/*******************************************************************************/

// Map
FXDEFMAP(FXTopWindow) FXTopWindowMap[]={
  FXMAPFUNC(SEL_CLOSE,0,FXTopWindow::onClose),
  FXMAPFUNC(SEL_FOCUS_UP,0,FXTopWindow::onFocusUp),
  FXMAPFUNC(SEL_FOCUS_DOWN,0,FXTopWindow::onFocusDown),
  FXMAPFUNC(SEL_FOCUS_LEFT,0,FXTopWindow::onFocusLeft),
  FXMAPFUNC(SEL_FOCUS_RIGHT,0,FXTopWindow::onFocusRight),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETSTRINGVALUE,FXTopWindow::onCmdSetStringValue),
  FXMAPFUNC(SEL_COMMAND,FXTopWindow::ID_ICONIFY,FXTopWindow::onCmdIconify),
  FXMAPFUNC(SEL_COMMAND,FXTopWindow::ID_DEICONIFY,FXTopWindow::onCmdDeiconify),
  };


// Object implementation
FXIMPLEMENT_ABSTRACT(FXTopWindow,FXShell,FXTopWindowMap,ARRAYNUMBER(FXTopWindowMap))


// Create toplevel window object & add to toplevel window list
FXTopWindow::FXTopWindow(FXApp* a,const FXString& name,FXIcon *ic,FXIcon *mi,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs):
  FXShell(a,opts,x,y,w,h){
  title=name;
  icon=ic;
  miniIcon=mi;
  accelTable=new FXAccelTable;
  padtop=pt;
  padbottom=pb;
  padleft=pl;
  padright=pr;
  hspacing=hs;
  vspacing=vs;
  }


// Create toplevel window object & add to toplevel window list
FXTopWindow::FXTopWindow(FXWindow* own,const FXString& name,FXIcon *ic,FXIcon *mi,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs):
  FXShell(own,opts,x,y,w,h){
  title=name;
  icon=ic;
  miniIcon=mi;
  accelTable=new FXAccelTable;
  padtop=pt;
  padbottom=pb;
  padleft=pl;
  padright=pr;
  hspacing=hs;
  vspacing=vs;
  }



#ifdef WIN32
const char* FXTopWindow::GetClass() const { return "FXTopWindow"; }
#endif



// Create window
void FXTopWindow::create(){
  FXShell::create();

  // Create icons
  if(icon) icon->create();
  if(miniIcon) miniIcon->create();

  if(xid){
    if(getApp()->initialized){
#ifndef WIN32
      // Catch delete window
      Atom protocols[2];
      protocols[0]=getApp()->wmDeleteWindow;
      protocols[1]=getApp()->wmTakeFocus;
      XSetWMProtocols(DISPLAY(getApp()),xid,protocols,2);

      // Set position for Window Manager
      XSizeHints size;

      size.flags=USPosition|PPosition|USSize|PSize|PWinGravity;
      size.min_width=0;
      size.min_height=0;
      size.max_width=0;
      size.max_height=0;
      size.x=xpos;
      size.y=ypos;
      size.width=width;
      size.height=height;
      size.width_inc=0;
      size.height_inc=0;
      size.min_aspect.x=0;
      size.min_aspect.y=0;
      size.max_aspect.x=0;
      size.max_aspect.y=0;
      size.base_width=0;
      size.base_height=0;
      size.win_gravity=StaticGravity;     // Account for border (ICCCM)
      size.win_gravity=NorthWestGravity;  // Tim Alexeevsky <realtim@mail.ru>

      // Set hints
      XSetWMNormalHints(DISPLAY(getApp()),xid,&size);
#endif

      // Set title
      settitle();

      // Set decorations
      setdecorations();

      // Set icon for X-Windows
      seticons();
      }
    }
  }


// Detach window
void FXTopWindow::detach(){
  FXShell::detach();
  if(icon) icon->detach();
  if(miniIcon) miniIcon->detach();
  }


// Focus to this toplevel window
void FXTopWindow::setFocus(){
  FXTRACE((100,"%s::setFocus %p\n",getClassName(),this));
  FXShell::setFocus();
  if(xid){
#ifndef WIN32
    XSetInputFocus(DISPLAY(getApp()),xid,RevertToPointerRoot,CurrentTime);
#else
    SetFocus((HWND)xid);
#endif
    }
  }


// Focus away from this toplevel window
void FXTopWindow::killFocus(){
  FXTRACE((100,"%s::killFocus %p\n",getClassName(),this));
  FXShell::killFocus();
  if(xid){
#ifndef WIN32
    Window win;
    int    dum;
    XGetInputFocus(DISPLAY(getApp()),&win,&dum);
    if(win==xid){
      if(getOwner() && getOwner()->id()){
        FXTRACE((100,"back to owner\n"));
        XSetInputFocus(DISPLAY(getApp()),getOwner()->id(),RevertToPointerRoot,CurrentTime);
        }
      else{
        FXTRACE((100,"back to NULL\n"));
        XSetInputFocus(DISPLAY(getApp()),PointerRoot,RevertToPointerRoot,CurrentTime);
        }
      }
#else
    if(GetFocus()==(HWND)xid){
      if(getOwner() && getOwner()->id()){
        FXTRACE((100,"back to owner\n"));
        SetFocus((HWND)getOwner()->id());
        }
      else{
        FXTRACE((100,"back to NULL\n"));
        SetFocus((HWND)NULL);
        }
      }
#endif
    }
  }


// Show and raise window
void FXTopWindow::show(){
  FXShell::show();
  raise();
  }



// Show and raise window, placed properly on the screen
void FXTopWindow::show(FXuint placement){
  place(placement);
  FXShell::show();
  raise();
  }



// Hide window
void FXTopWindow::hide(){
  if(flags&FLAG_SHOWN){
    killFocus();
    flags&=~FLAG_SHOWN;
    if(xid){
#ifndef WIN32
      XWithdrawWindow(DISPLAY(getApp()),xid,DefaultScreen(DISPLAY(getApp())));
#else
      ShowWindow((HWND)xid,SW_HIDE);
#endif
      }
    }
  }

// FIXME
// Unify placement API's under show, also perhaps
// move to FXWindow, and maybe same in FXMDIChild also
// for simplicity.

// void FXTopWindow::show(FXuint placement)
// {
//
// #ifdef WIN32
//   if (placement == PLACEMENT_MAXIMIZED)
//   {
//                 place(PLACEMENT_DEFAULT);
//                 //Replacement of FXShell::show()
//                 if(!(flags&FLAG_SHOWN))
//                 {
//                         flags|=FLAG_SHOWN;
//                 }
//                 if (xid)
//                 {
//                         ShowWindow((HWND)xid,SW_MAXIMIZE);
//                 }
//                 //end of replacement
//                 //FXShell::show();
//                 raise();
//
//         return;
//   }
//
//   //proposed minimized functionality...
//   if (placement == PLACEMENT_MINIMIZED)
//   {
//                 place(PLACEMENT_DEFAULT);
//                 //Replacement of FXShell::show()
//                 if(!(flags&FLAG_SHOWN))
//                 {
//                         flags|=FLAG_SHOWN;
//                 }
//                 if (xid)
//                 {
//                         ShowWindow((HWND)xid,SW_MINIMIZE);
//                 }
//                 //end of replacement
//                 //FXShell::show();
//                 raise();
//
//         return;
//   }
//
// #endif
//   place(placement);
//   FXShell::show();
//   raise();
// }

// void FXTopWindow::maximize(){
//   if(xid){
// #ifndef WIN32
//     place(PLACEMENT_MAXIMIZED); // I thought it works on Unix until this
// e-mail
// #else
//     ShowWindow((HWND)xid, SW_MAXIMIZE);
// #endif
//     }
//   }
//
// FXbool FXTopWindow::isMaximized() const {
//   FXbool maximized = false;
//   if(xid){
// #ifdef WIN32
//     maximized=IsZoomed((HWND)xid);
// #endif
//     }
//   return maximized;
//   }



// Position the window based on placement
void FXTopWindow::place(FXuint placement){
  FXint rw,rh,ox,oy,ow,oh,wx,wy,ww,wh,x,y;
  FXuint state;
  FXWindow *over;

  // Default placement:- leave it where it was
  wx=getX();
  wy=getY();
  ww=getWidth();
  wh=getHeight();

  // Get root window size
  rw=getRoot()->getWidth();
  rh=getRoot()->getHeight();

  // Placement policy
  switch(placement){

    // Place such that it contains the cursor
    case PLACEMENT_CURSOR:

      // Get dialog location in root coordinates
      translateCoordinatesTo(wx,wy,getRoot(),0,0);

      // Where's the mouse?
      getRoot()->getCursorPosition(x,y,state);

      // Place such that mouse in the middle, placing it as
      // close as possible in the center of the owner window.
      // Don't move the window unless the mouse is not inside.
      if((wx==0 && wy==0) || x<wx || y<wy || wx+ww<=x || wy+wh<=y){

        // Get the owner
        over=getOwner()?getOwner():getRoot();

        // Get owner window size
        ow=over->getWidth();
        oh=over->getHeight();

        // Owner's coordinates to root coordinates
        over->translateCoordinatesTo(ox,oy,getRoot(),0,0);

        // Adjust position
        wx=ox+(ow-ww)/2;
        wy=oy+(oh-wh)/2;

        // Move by the minimal amount
        if(x<wx) wx=x-20; else if(wx+ww<=x) wx=x-ww+20;
        if(y<wy) wy=y-20; else if(wy+wh<=y) wy=y-wh+20;
        }

      // Adjust so dialog is fully visible
      if(wx<0) wx=10;
      if(wy<0) wy=10;
      if(wx+ww>rw) wx=rw-ww-10;
      if(wy+wh>rh) wy=rh-wh-10;
      break;

    // Place centered over the owner
    case PLACEMENT_OWNER:

      // Get the owner
      over=getOwner()?getOwner():getRoot();

      // Get owner window size
      ow=over->getWidth();
      oh=over->getHeight();

      // Owner's coordinates to root coordinates
      over->translateCoordinatesTo(ox,oy,getRoot(),0,0);

      // Adjust position
      wx=ox+(ow-ww)/2;
      wy=oy+(oh-wh)/2;

      // Adjust so dialog is fully visible
      if(wx<0) wx=10;
      if(wy<0) wy=10;
      if(wx+ww>rw) wx=rw-ww-10;
      if(wy+wh>rh) wy=rh-wh-10;
      break;

    // Place centered on the screen
    case PLACEMENT_SCREEN:

      // Adjust position
      wx=(rw-ww)/2;
      wy=(rh-wh)/2;
      break;

    // Place to make it fully visible
    case PLACEMENT_VISIBLE:

      // Adjust so dialog is fully visible
      if(wx<0) wx=10;
      if(wy<0) wy=10;
      if(wx+ww>rw) wx=rw-ww-10;
      if(wy+wh>rh) wy=rh-wh-10;
      break;

    // Place maximized
    case PLACEMENT_MAXIMIZED:
      wx=0;
      wy=0;
      ww=rw;                // Yes, I know:- we should substract the borders;
      wh=rh;                // trouble is, no way to know how big those are....
      break;

    // Default placement
    case PLACEMENT_DEFAULT:
    default:
      break;
      }

  // Place it
  position(wx,wy,ww,wh);
  }


#ifdef WIN32


// Make HICON from FXIcon
void* FXTopWindow::makeicon(FXIcon* icon){
  ICONINFO iconinfo;
  iconinfo.fIcon=TRUE;
  iconinfo.xHotspot=0;
  iconinfo.yHotspot=0;
  iconinfo.hbmMask=(HBITMAP)icon->shape;
  iconinfo.hbmColor=(HBITMAP)icon->xid;
  void* wicon=CreateIconIndirect(&iconinfo);
  return wicon;
  }


#endif



// Set large icon(s)
void FXTopWindow::seticons(){

  // Set icon for X-Windows
#ifndef WIN32
  FXWindow *own=this;
  XWMHints  wmhints;
  wmhints.flags=InputHint|StateHint;
  wmhints.input=TRUE;       // True, but ICCCM says it should be FALSE....
  wmhints.initial_state=NormalState;
  if(icon){
    if(!icon->xid || !icon->shape){ fxerror("%s::setIcon: illegal icon specified.\n",getClassName()); }
    wmhints.flags|=IconPixmapHint|IconMaskHint;
    wmhints.icon_pixmap=icon->xid;
    wmhints.icon_mask=icon->shape;
    }
  else if(miniIcon){
    if(!miniIcon->xid || !miniIcon->shape){ fxerror("%s::setMiniIcon: illegal icon specified.\n",getClassName()); }
    wmhints.flags|=IconPixmapHint|IconMaskHint;
    wmhints.icon_pixmap=miniIcon->xid;
    wmhints.icon_mask=miniIcon->shape;
    }
  while(own->getOwner()){   // Find the ultimate owner of the whole chain
    own=own->getOwner();
    }
  if(own && own->id()){     // Set the window_group id; all windows in the group should be iconified together
    wmhints.flags|=WindowGroupHint;
    wmhints.window_group=own->id();
    }
  XSetWMHints(DISPLAY(getApp()),xid,&wmhints);

  // Set both large and mini icon for MS-Windows
#else
  if(icon){
    if(!icon->xid || !icon->shape){ fxerror("%s::setIcon: illegal icon specified.\n",getClassName()); }
    SendMessage((HWND)xid,WM_SETICON,ICON_BIG,(LPARAM)makeicon(icon));
    }
  if(miniIcon){
    if(!miniIcon->xid || !miniIcon->shape){ fxerror("%s::setMiniIcon: illegal icon specified.\n",getClassName()); }
    SendMessage((HWND)xid,WM_SETICON,ICON_SMALL,(LPARAM)makeicon(miniIcon));
    }
#endif
  }


// Set title
void FXTopWindow::settitle(){
  if(!title.empty()){
#ifndef WIN32
    XTextProperty t;
    char *s;
    s=(char*)title.text();
    if(XStringListToTextProperty((char**)&s,1,&t)){
      XSetWMIconName(DISPLAY(getApp()),xid,&t);
      XSetWMName(DISPLAY(getApp()),xid,&t);
      XFree(t.value);
      }
#else
    SetWindowText((HWND)xid,title.text());
#endif
    }
  }



// Set decorations
void FXTopWindow::setdecorations(){
#ifndef WIN32
  struct {
    long flags;
    long functions;
    long decorations;
    long inputmode;
    } prop;
  prop.flags=MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS|MWM_HINTS_INPUT_MODE;
  prop.decorations=0;
  prop.functions=MWM_FUNC_MOVE;
  prop.inputmode=MWM_INPUT_MODELESS;
  if(options&DECOR_TITLE){
    prop.decorations|=MWM_DECOR_TITLE;
    }
  if(options&DECOR_MINIMIZE){
    prop.decorations|=MWM_DECOR_MINIMIZE;
    prop.functions|=MWM_FUNC_MINIMIZE;
    }
  if(options&DECOR_MAXIMIZE){
    prop.decorations|=MWM_DECOR_MAXIMIZE;
    prop.functions|=MWM_FUNC_MAXIMIZE;
    }
  if(options&DECOR_CLOSE){
    prop.functions|=MWM_FUNC_CLOSE;
    }
  if(options&DECOR_BORDER){
    prop.decorations|=MWM_DECOR_BORDER;
    }
  if(options&DECOR_RESIZE){
    prop.decorations|=MWM_DECOR_RESIZEH;
    prop.functions|=MWM_FUNC_RESIZE;
    }
  if(options&DECOR_MENU){
    prop.decorations|=MWM_DECOR_MENU;
    prop.functions|=MWM_FUNC_RESIZE;
    }
  XChangeProperty(DISPLAY(getApp()),xid,getApp()->wmMotifHints,getApp()->wmMotifHints,32,PropModeReplace,(unsigned char*)&prop,4);
#else

  // Thanks to testing from Sander Jansen <sxj@cfdrc.com>

  // Get old style
  DWORD dwStyle=GetWindowLong((HWND)xid,GWL_STYLE);
  DWORD dwExStyle=GetWindowLong((HWND)xid,GWL_EXSTYLE);
  RECT rect;

  // Change style setting; note, under Windows, if we want a minimize,
  // maximize, or close button, we also need a window menu style as well.
  // Also, if you want a title, you will need a border.
  if(options&DECOR_BORDER) dwStyle|=WS_BORDER; else dwStyle&=~WS_BORDER;
  if(options&DECOR_TITLE) dwStyle|=WS_CAPTION; else dwStyle&=~WS_DLGFRAME;
  if(options&DECOR_RESIZE) dwStyle|=WS_THICKFRAME; else dwStyle&=~WS_THICKFRAME;
  if(options&DECOR_MENU) dwStyle|=WS_SYSMENU; else dwStyle&=~WS_SYSMENU;
  if(options&DECOR_CLOSE) dwStyle|=WS_SYSMENU;
  if(options&DECOR_MINIMIZE) dwStyle|=(WS_MINIMIZEBOX|WS_SYSMENU); else dwStyle&=~WS_MINIMIZEBOX;
  if(options&DECOR_MAXIMIZE) dwStyle|=(WS_MAXIMIZEBOX|WS_SYSMENU); else dwStyle&=~WS_MAXIMIZEBOX;

  // Set new style
  SetWindowLong((HWND)xid,GWL_STYLE,dwStyle);

  // Patch from Stephane Ancelot <sancelot@wanadoo.fr> and Sander Jansen <sxj@cfdrc.com>
  HMENU sysmenu=GetSystemMenu((HWND)xid,FALSE);
  if(sysmenu){
    if(options&DECOR_CLOSE)
      EnableMenuItem(sysmenu,SC_CLOSE,MF_ENABLED);
    else
      EnableMenuItem(sysmenu,SC_CLOSE,MF_GRAYED);
    }

  // Adjust non-client area size based on new style
  SetRect(&rect,0,0,width,height);
  AdjustWindowRectEx(&rect,dwStyle,FALSE,dwExStyle);
  SetWindowPos((HWND)xid,NULL,0,0,FXMAX(rect.right-rect.left,1),FXMAX(rect.bottom-rect.top,1),SWP_NOMOVE|SWP_NOZORDER);
  RedrawWindow((HWND)xid,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);
#endif
  }



// Change decorations
void FXTopWindow::setDecorations(FXuint decorations){
  FXuint opts=(decorations&~DECOR_ALL) | (decorations&DECOR_ALL);
  if(options!=opts){
    options=opts;
    if(xid) setdecorations();
    recalc();
    }
  }


// Get decorations
FXuint FXTopWindow::getDecorations() const {
  return options&DECOR_ALL;
  }


// Iconify window
void FXTopWindow::iconify(){
  if(xid){
#ifndef WIN32
    XIconifyWindow(DISPLAY(getApp()),xid,DefaultScreen(DISPLAY(getApp())));
#else
    ShowWindow((HWND)xid,SW_MINIMIZE);
#endif
    }
  }


// Deiconify window
void FXTopWindow::deiconify(){
  if(xid){
#ifndef WIN32
    XMapWindow(DISPLAY(getApp()),xid);
#else
    ShowWindow((HWND)xid,SW_RESTORE);
#endif
    }
  }


// Return TRUE if window has been iconified
FXbool FXTopWindow::isIconified() const {
  FXbool iconified=FALSE;
  if(xid){
#ifndef WIN32
    unsigned long length,after;
    unsigned char *data;
    Atom actualtype;
    int actualformat;
    if(Success==XGetWindowProperty(DISPLAY(getApp()),xid,getApp()->wmState,0,2,FALSE,AnyPropertyType,&actualtype,&actualformat,&length,&after,&data)){
      if(actualformat==32){
        iconified=(IconicState==*((FXuint*)data));
        }
      XFree((char*)data);
      }
#else
    iconified=IsIconic((HWND)xid);
#endif
    }
  return iconified;
  }


// Request for toplevel window move
void FXTopWindow::move(FXint x,FXint y){
  if((x!=xpos) || (y!=ypos)){
    xpos=x;
    ypos=y;
    if(xid){
#ifndef WIN32
      XWindowChanges cw;
      cw.x=xpos;
      cw.y=ypos;
      XReconfigureWMWindow(DISPLAY(getApp()),xid,DefaultScreen(DISPLAY(getApp())),CWX|CWY,&cw);
#else
      // Calculate the required window position based on the desired
      // position of the *client* rectangle.
      RECT rect;
      SetRect(&rect,xpos,ypos,0,0);
      DWORD dwStyle=GetWindowLong((HWND)xid,GWL_STYLE);
      BOOL bMenu=FALSE; // No, we always account for the menu bar ourselves
      DWORD dwExStyle=GetWindowLong((HWND)xid,GWL_EXSTYLE);
      AdjustWindowRectEx(&rect,dwStyle,bMenu,dwExStyle);
      SetWindowPos((HWND)xid,NULL,rect.left,rect.top,0,0,SWP_NOSIZE|SWP_NOZORDER);
#endif
      }
    }
  }


// Request for toplevel window resize
void FXTopWindow::resize(FXint w,FXint h){
  if((flags&FLAG_DIRTY) || (w!=width) || (h!=height)){
    width=FXMAX(w,1);
    height=FXMAX(h,1);
    if(xid){
#ifndef WIN32
      XWindowChanges cw;
      cw.width=width;
      cw.height=height;
      XReconfigureWMWindow(DISPLAY(getApp()),xid,DefaultScreen(DISPLAY(getApp())),CWWidth|CWHeight,&cw);
#else
      // Calculate the required window size based on the desired
      // size of the *client* rectangle.
      RECT rect;
      SetRect(&rect,0,0,width,height);
      DWORD dwStyle=GetWindowLong((HWND)xid,GWL_STYLE);
      BOOL bMenu=FALSE; // No, we always account for the menu bar ourselves
      DWORD dwExStyle=GetWindowLong((HWND)xid,GWL_EXSTYLE);
      AdjustWindowRectEx(&rect,dwStyle,bMenu,dwExStyle);
      SetWindowPos((HWND)xid,NULL,0,0,FXMAX(rect.right-rect.left,1),FXMAX(rect.bottom-rect.top,1),SWP_NOMOVE|SWP_NOZORDER);
#endif
      layout();
      }
    }
  }


// Request for toplevel window reposition
void FXTopWindow::position(FXint x,FXint y,FXint w,FXint h){
  if((flags&FLAG_DIRTY) || (x!=xpos) || (y!=ypos) || (w!=width) || (h!=height)){
    xpos=x;
    ypos=y;
    width=FXMAX(w,1);
    height=FXMAX(h,1);
    if(xid){
#ifndef WIN32
      XWindowChanges cw;
      cw.x=xpos;
      cw.y=ypos;
      cw.width=width;
      cw.height=height;
      XReconfigureWMWindow(DISPLAY(getApp()),xid,DefaultScreen(DISPLAY(getApp())),CWX|CWY|CWWidth|CWHeight,&cw);
#else
      // Calculate the required window position & size based on the desired
      // position & size of the *client* rectangle.
      RECT rect;
      SetRect(&rect,xpos,ypos,xpos+width,ypos+height);
      DWORD dwStyle=GetWindowLong((HWND)xid,GWL_STYLE);
      BOOL bMenu=FALSE; // No, we always account for the menu bar ourselves
      DWORD dwExStyle=GetWindowLong((HWND)xid,GWL_EXSTYLE);
      AdjustWindowRectEx(&rect,dwStyle,bMenu,dwExStyle);
      SetWindowPos((HWND)xid,NULL,rect.left,rect.top,FXMAX(rect.right-rect.left,1),FXMAX(rect.bottom-rect.top,1),SWP_NOZORDER);
#endif
      layout();
      }
    }
  }


// Compute minimum width based on child layout hints
int FXTopWindow::getDefaultWidth(){
  register FXint w,wcum,wmax,mw=0;
  register FXWindow* child;
  register FXuint hints;
  wmax=wcum=0;
  if(options&PACK_UNIFORM_WIDTH) mw=maxChildWidth();
  for(child=getLast(); child; child=child->getPrev()){
    if(child->shown()){
      hints=child->getLayoutHints();
      if(hints&LAYOUT_FIX_WIDTH) w=child->getWidth();
      else if(options&PACK_UNIFORM_WIDTH) w=mw;
      else w=child->getDefaultWidth();
      if((hints&LAYOUT_RIGHT)&&(hints&LAYOUT_CENTER_X)){    // Fixed X
        w=child->getX()+w;
        if(w>wmax) wmax=w;
        }
      else if(hints&LAYOUT_SIDE_LEFT){                      // Left or right
        if(child->getNext()) wcum+=hspacing;
        wcum+=w;
        }
      else{
        if(w>wcum) wcum=w;
        }
      }
    }
  return padleft+padright+FXMAX(wcum,wmax);
  }


// Compute minimum height based on child layout hints
int FXTopWindow::getDefaultHeight(){
  register FXint h,hcum,hmax,mh=0;
  register FXWindow* child;
  register FXuint hints;
  hmax=hcum=0;
  if(options&PACK_UNIFORM_HEIGHT) mh=maxChildHeight();
  for(child=getLast(); child; child=child->getPrev()){
    if(child->shown()){
      hints=child->getLayoutHints();
      if(hints&LAYOUT_FIX_HEIGHT) h=child->getHeight();
      else if(options&PACK_UNIFORM_HEIGHT) h=mh;
      else h=child->getDefaultHeight();
      if((hints&LAYOUT_BOTTOM)&&(hints&LAYOUT_CENTER_Y)){   // Fixed Y
        h=child->getY()+h;
        if(h>hmax) hmax=h;
        }
      else if(!(hints&LAYOUT_SIDE_LEFT)){                   // Top or bottom
        if(child->getNext()) hcum+=vspacing;
        hcum+=h;
        }
      else{
        if(h>hcum) hcum=h;
        }
      }
    }
  return padtop+padbottom+FXMAX(hcum,hmax);
  }


// Recalculate layout
void FXTopWindow::layout(){
  register FXint left,right,top,bottom,x,y,w,h;
  register FXint mw=0,mh=0;
  register FXWindow* child;
  register FXuint hints;

  // Placement rectangle; right/bottom non-inclusive
  left=padleft;
  right=width-padright;
  top=padtop;
  bottom=height-padbottom;

  // Get maximum child size
  if(options&PACK_UNIFORM_WIDTH) mw=maxChildWidth();
  if(options&PACK_UNIFORM_HEIGHT) mh=maxChildHeight();

  // Pack them in the cavity
  for(child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      hints=child->getLayoutHints();
      x=child->getX();
      y=child->getY();

      // Vertical
      if(hints&LAYOUT_SIDE_LEFT){

        // Height
        if(hints&LAYOUT_FIX_HEIGHT) h=child->getHeight();
        else if(options&PACK_UNIFORM_HEIGHT) h=mh;
        else if(hints&LAYOUT_FILL_Y) h=bottom-top;
        else h=child->getDefaultHeight();

        // Width
        if(hints&LAYOUT_FIX_WIDTH) w=child->getWidth();
        else if(options&PACK_UNIFORM_WIDTH) w=mw;
        else if(hints&LAYOUT_FILL_X) w=right-left;
        else w=child->getWidthForHeight(h);             // Width is a function of height!

        // Y
        if(!((hints&LAYOUT_BOTTOM)&&(hints&LAYOUT_CENTER_Y))){
          if(hints&LAYOUT_CENTER_Y) y=top+(bottom-top-h)/2;
          else if(hints&LAYOUT_BOTTOM) y=bottom-h;
          else y=top;
          }

        // X
        if(!((hints&LAYOUT_RIGHT)&&(hints&LAYOUT_CENTER_X))){
          if(hints&LAYOUT_CENTER_X) x=left+(right-left-w)/2;
          else if(hints&LAYOUT_SIDE_BOTTOM){            // Right
            x=right-w;
            right-=(w+hspacing);
            }
          else{                                         // Left
            x=left;
            left+=(w+hspacing);
            }
          }
        }

      // Horizontal
      else{

        // Width
        if(hints&LAYOUT_FIX_WIDTH) w=child->getWidth();
        else if(options&PACK_UNIFORM_WIDTH) w=mw;
        else if(hints&LAYOUT_FILL_X) w=right-left;
        else w=child->getDefaultWidth();

        // Height
        if(hints&LAYOUT_FIX_HEIGHT) h=child->getHeight();
        else if(options&PACK_UNIFORM_HEIGHT) h=mh;
        else if(hints&LAYOUT_FILL_Y) h=bottom-top;
        else h=child->getHeightForWidth(w);             // Height is a function of width!

        // X
        if(!((hints&LAYOUT_RIGHT)&&(hints&LAYOUT_CENTER_X))){
          if(hints&LAYOUT_CENTER_X) x=left+(right-left-w)/2;
          else if(hints&LAYOUT_RIGHT) x=right-w;
          else x=left;
          }

        // Y
        if(!((hints&LAYOUT_BOTTOM)&&(hints&LAYOUT_CENTER_Y))){
          if(hints&LAYOUT_CENTER_Y) y=top+(bottom-top-h)/2;
          else if(hints&LAYOUT_SIDE_BOTTOM){            // Bottom
            y=bottom-h;
            bottom-=(h+vspacing);
            }
          else{                                         // Top
            y=top;
            top+=(h+vspacing);
            }
          }
        }
      child->position(x,y,w,h);
      }
    }
  flags&=~FLAG_DIRTY;
  }


// Update value from a message
long FXTopWindow::onCmdSetStringValue(FXObject*,FXSelector,void* ptr){
  setTitle(*((FXString*)ptr));
  return 1;
  }


// Iconify the window
long FXTopWindow::onCmdIconify(FXObject*,FXSelector,void*){
  iconify();
  return 1;
  }


// Deiconify the window
long FXTopWindow::onCmdDeiconify(FXObject*,FXSelector,void*){
  deiconify();
  return 1;
  }


// Close window
long FXTopWindow::onClose(FXObject*,FXSelector,void*){

  // If handled, we're not closing the window after all
  if(target && target->handle(this,MKUINT(message,SEL_CLOSE),NULL)) return 1;

  // Otherwise close window by hiding it
  hide();
  return 1;
  }


// Focus moved up
long FXTopWindow::onFocusUp(FXObject*,FXSelector sel,void* ptr){
  FXWindow *child,*c;
  FXint cury,childy;
  if(getFocus()){
    cury=getFocus()->getY();
    while(1){
      child=NULL;
      childy=-10000000;
      for(c=getFirst(); c; c=c->getNext()){
        if(c->shown() && c->getY()<cury && childy<c->getY()){ childy=c->getY(); child=c; }
        }
      if(!child) return 0;
      if(child->isEnabled() && child->canFocus()){
        child->handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
        return 1;
        }
      if(child->isComposite() && child->handle(this,sel,ptr)) return 1;
      cury=childy;
      }
    }
  else{
    child=getLast();
    while(child){
      if(child->shown()){
        if(child->isEnabled() && child->canFocus()){
          child->handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
          return 1;
          }
        if(child->isComposite() && child->handle(this,sel,ptr)) return 1;
        }
      child=child->getPrev();
      }
    }
  return 0;
  }


// Focus moved down
long FXTopWindow::onFocusDown(FXObject*,FXSelector sel,void* ptr){
  FXWindow *child,*c;
  FXint cury,childy;
  if(getFocus()){
    cury=getFocus()->getY();
    while(1){
      child=NULL;
      childy=10000000;
      for(c=getFirst(); c; c=c->getNext()){
        if(c->shown() && cury<c->getY() && c->getY()<childy){ childy=c->getY(); child=c; }
        }
      if(!child) return 0;
      if(child->isEnabled() && child->canFocus()){
        child->handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
        return 1;
        }
      if(child->isComposite() && child->handle(this,sel,ptr)) return 1;
      cury=childy;
      }
    }
  else{
    child=getFirst();
    while(child){
      if(child->shown()){
        if(child->isEnabled() && child->canFocus()){
          child->handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
          return 1;
          }
        if(child->isComposite() && child->handle(this,sel,ptr)) return 1;
        }
      child=child->getNext();
      }
    }
  return 0;
  }


// Focus moved to left
long FXTopWindow::onFocusLeft(FXObject*,FXSelector sel,void* ptr){
  FXWindow *child,*c;
  FXint curx,childx;
  if(getFocus()){
    curx=getFocus()->getX();
    while(1){
      child=NULL;
      childx=-10000000;
      for(c=getFirst(); c; c=c->getNext()){
        if(c->shown() && c->getX()<curx && childx<c->getX()){ childx=c->getX(); child=c; }
        }
      if(!child) return 0;
      if(child->isEnabled() && child->canFocus()){
        child->handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
        return 1;
        }
      if(child->isComposite() && child->handle(this,sel,ptr)) return 1;
      curx=childx;
      }
    }
  else{
    child=getLast();
    while(child){
      if(child->shown()){
        if(child->isEnabled() && child->canFocus()){
          child->handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
          return 1;
          }
        if(child->isComposite() && child->handle(this,sel,ptr)) return 1;
        }
      child=child->getPrev();
      }
    }
  return 0;
  }


// Focus moved to right
long FXTopWindow::onFocusRight(FXObject*,FXSelector sel,void* ptr){
  FXWindow *child,*c;
  FXint curx,childx;
  if(getFocus()){
    curx=getFocus()->getX();
    while(1){
      child=NULL;
      childx=10000000;
      for(c=getFirst(); c; c=c->getNext()){
        if(c->shown() && curx<c->getX() && c->getX()<childx){ childx=c->getX(); child=c; }
        }
      if(!child) return 0;
      if(child->isEnabled() && child->canFocus()){
        child->handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
        return 1;
        }
      if(child->isComposite() && child->handle(this,sel,ptr)) return 1;
      curx=childx;
      }
    }
  else{
    child=getFirst();
    while(child){
      if(child->shown()){
        if(child->isEnabled() && child->canFocus()){
          child->handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
          return 1;
          }
        if(child->isComposite() && child->handle(this,sel,ptr)) return 1;
        }
      child=child->getNext();
      }
    }
  return 0;
  }



// Change regular icon
void FXTopWindow::setIcon(FXIcon* ic){
  if(icon!=ic){
    icon=ic;
    if(xid) seticons();
    }
  }


// Change mini icon
void FXTopWindow::setMiniIcon(FXIcon *ic){
  if(miniIcon!=ic){
    miniIcon=ic;
    if(xid) seticons();
    }
  }


// Set new window title
void FXTopWindow::setTitle(const FXString& name){
  if(title!=name){
    title=name;
    if(xid) settitle();
    }
  }


// Change packing hints
void FXTopWindow::setPackingHints(FXuint ph){
  FXuint opts=(options&~(PACK_UNIFORM_HEIGHT|PACK_UNIFORM_WIDTH)) | (ph&(PACK_UNIFORM_HEIGHT|PACK_UNIFORM_WIDTH));
  if(opts!=options){
    options=opts;
    recalc();
    update();
    }
  }


// Get packing hints
FXuint FXTopWindow::getPackingHints() const {
  return (options&(PACK_UNIFORM_HEIGHT|PACK_UNIFORM_WIDTH));
  }


// Change top padding
void FXTopWindow::setPadTop(FXint pt){
  if(padtop!=pt){
    padtop=pt;
    recalc();
    update();
    }
  }


// Change bottom padding
void FXTopWindow::setPadBottom(FXint pb){
  if(padbottom!=pb){
    padbottom=pb;
    recalc();
    update();
    }
  }


// Change left padding
void FXTopWindow::setPadLeft(FXint pl){
  if(padleft!=pl){
    padleft=pl;
    recalc();
    update();
    }
  }


// Change right padding
void FXTopWindow::setPadRight(FXint pr){
  if(padright!=pr){
    padright=pr;
    recalc();
    update();
    }
  }


// Change horizontal spacing
void FXTopWindow::setHSpacing(FXint hs){
  if(hspacing!=hs){
    hspacing=hs;
    recalc();
    update();
    }
  }


// Change vertical spacing
void FXTopWindow::setVSpacing(FXint vs){
  if(vspacing!=vs){
    vspacing=vs;
    recalc();
    update();
    }
  }

// Save object to stream
void FXTopWindow::save(FXStream& store) const {
  FXShell::save(store);
  store << title;
  store << icon;
  store << miniIcon;
  store << padtop << padbottom << padleft << padright;
  store << hspacing << vspacing;
  }


// Load object from stream
void FXTopWindow::load(FXStream& store){
  FXShell::load(store);
  store >> title;
  store >> icon;
  store >> miniIcon;
  store >> padtop >> padbottom >> padleft >> padright;
  store >> hspacing >> vspacing;
  }


// Remove this one from toplevel window list
FXTopWindow::~FXTopWindow(){
  icon=(FXIcon*)-1;
  miniIcon=(FXIcon*)-1;
  }
