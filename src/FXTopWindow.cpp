/********************************************************************************
*                                                                               *
*                         T o p   W i n d o w   O b j e c t                     *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998 by Jeroen van der Zijp.   All Rights Reserved.             *
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
* $Id: FXTopWindow.cpp,v 1.15 1998/09/10 15:15:51 jvz Exp $                   *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXObject.h"
#include "FXAccelTable.h"
#include "FXObjectList.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXCursor.h"
#include "FXDrawable.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXComposite.h"
#include "FXRootWindow.h"
#include "FXShell.h"
#include "FXTopWindow.h"

/*

  To do:
  - allow resize option..
  - setting icons
  - Iconified/normal
  - FXApp should keep track of toplevel windows, and if last one is closed,
    end the application
*/


/*******************************************************************************/

// Map
FXDEFMAP(FXTopWindow) FXTopWindowMap[]={
  FXMAPFUNC(SEL_ENTER,0,FXTopWindow::onEnter),
  FXMAPFUNC(SEL_LEAVE,0,FXTopWindow::onLeave),
  FXMAPFUNC(SEL_CLOSE,0,FXTopWindow::onClose),
  FXMAPFUNC(SEL_COMMAND,FXTopWindow::ID_CLOSE,FXTopWindow::onCmdClose),
  };


// Object implementation
FXIMPLEMENT(FXTopWindow,FXShell,FXTopWindowMap,ARRAYNUMBER(FXTopWindowMap))


// Create toplevel window object & add to toplevel window list
FXTopWindow::FXTopWindow(FXApp* a,const char* name,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint hs,FXint vs):
  FXShell(a,opts,x,y,w,h),title(name){
  accelTable=new FXAccelTable;
  hspacing=hs;
  vspacing=vs;
  }


// Enter:- if modal dialog in effect, grab to the dialog
long FXTopWindow::onEnter(FXObject*,FXSelector,void*){
  //fprintf(stderr,"%s::onEnter\n",getClassName(),xid);   
  return 0;
  }


// Leave:- if modal dialog in effect, loose the grab
// We want normal behaviour in all windows except those of the app
long FXTopWindow::onLeave(FXObject*,FXSelector,void*){
  //fprintf(stderr,"%s::onLeave\n",getClassName(),xid);   
  return 0;
  }


// Catch client message
long FXTopWindow::onClose(FXObject*,FXSelector,void* ptr){
  handle(this,MKUINT(FXTopWindow::ID_CLOSE,SEL_COMMAND),ptr);
  return 1;
  }


// Catch client message
long FXTopWindow::onCmdClose(FXObject*,FXSelector,void*){
  // Keep counter of toplevel windows; when 0, we're done with the app
  hide();
  return 1;
  }


// Create a window
void FXTopWindow::create(){
  FXShell::create();
  
  // Catch delete window 
  XSetWMProtocols(getDisplay(),xid,(Atom*)&getApp()->wmDeleteWindow,1);
  
  // Set title
  settitle();
  
  // Set decorations
  setdecorations();
  }


// Display window and raise it
void FXTopWindow::show(){
  FXShell::show();
  raise();
  }


// Set title
void FXTopWindow::settitle(){
  XTextProperty t;
  char *s;
  s=(char*)title.text();
  if(XStringListToTextProperty((char**)&s,1,&t)){
    XSetWMIconName(getDisplay(),xid,&t);
    XSetWMName(getDisplay(),xid,&t);
    XFree(t.value);
    }
  }


// Set decorations
void FXTopWindow::setdecorations(){
  FXMotifHints prop;
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
  XChangeProperty(getDisplay(),xid,getApp()->wmMotifHints,getApp()->wmMotifHints,32,PropModeReplace,(unsigned char*)&prop,4);
  }


// Change decorations
void FXTopWindow::setDecorations(FXDecorations decorations){
  FXuint opts=(decorations&~DECOR_ALL) | (decorations&DECOR_ALL);
  if(options!=opts){
    options=opts;
    if(xid) setdecorations();
    recalc();
    }
  }


// Get decorations
FXDecorations FXTopWindow::getDecorations() const { 
  return (FXDecorations)(options&DECOR_ALL); 
  }


// Compute minimum width based on child layout hints
int FXTopWindow::getDefaultWidth(){
  register FXint t,wcum,wmax,mw=0;
  register FXWindow* child;
  register FXuint hints,side;
  wmax=wcum=0;
  if(options&PACK_UNIFORM_WIDTH) mw=maxChildWidth();
  for(child=getLast(); child; child=child->getPrev()){
    if(child->shown()){
      hints=child->getLayoutHints();
      side=hints&LAYOUT_SIDE_MASK;
      if(options&PACK_UNIFORM_WIDTH) t=mw;
      else if(hints&LAYOUT_FIX_WIDTH) t=child->getWidth(); 
      else t=child->getDefaultWidth();
      if(hints&LAYOUT_FIX_X){ 
        t=child->getX()+t;
        if(t>wmax) wmax=t;
        }
      else if(side==LAYOUT_SIDE_LEFT || side==LAYOUT_SIDE_RIGHT){
        if(child->getNext()) wcum+=hspacing;
        wcum+=t;
        }
      else{
        if(t>wcum) wcum=t;
        }
      }
    }
  return FXMAX(wcum,wmax);
  }
  

// Compute minimum height based on child layout hints
int FXTopWindow::getDefaultHeight(){
  register FXint t,hcum,hmax,mh=0;
  register FXWindow* child;
  register FXuint hints,side;
  hmax=hcum=0;
  if(options&PACK_UNIFORM_HEIGHT) mh=maxChildHeight();
  for(child=getLast(); child; child=child->getPrev()){
    if(child->shown()){
      hints=child->getLayoutHints();
      side=hints&LAYOUT_SIDE_MASK;
      if(options&PACK_UNIFORM_HEIGHT) t=mh;
      else if(hints&LAYOUT_FIX_HEIGHT) t=child->getHeight(); 
      else t=child->getDefaultHeight();
      if(hints&LAYOUT_FIX_Y){
        t=child->getY()+t;
        if(t>hmax) hmax=t;
        }
      else if(side==LAYOUT_SIDE_TOP || side==LAYOUT_SIDE_BOTTOM){
        if(child->getNext()) hcum+=vspacing;
        hcum+=t;
        }
      else{
        if(t>hcum) hcum=t;
        }
      }
    }
  return FXMAX(hcum,hmax);
  }


// Recalculate layout
void FXTopWindow::layout(){
  FXint left,right,top,bottom;
  FXint mw=0,mh=0;
  FXWindow* child;
  FXint x,y,w,h;
  FXint extra_space,total_space;
  FXuint hints,side;
  
  // Placement rectangle; right/bottom non-inclusive
  left=0;
  right=width;
  top=0;
  bottom=height;
  
  // Get maximum child size
  if(options&PACK_UNIFORM_WIDTH) mw=maxChildWidth();
  if(options&PACK_UNIFORM_HEIGHT) mh=maxChildHeight();
  
  // Pack them in the cavity
  for(child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      hints=child->getLayoutHints();
      side=hints&LAYOUT_SIDE_MASK;
      x=child->getX();
      y=child->getY();
      if(options&PACK_UNIFORM_WIDTH) w=mw;
      else if(hints&LAYOUT_FIX_WIDTH) w=child->getWidth(); 
      else w=child->getDefaultWidth();
      if(options&PACK_UNIFORM_HEIGHT) h=mh;
      else if(hints&LAYOUT_FIX_HEIGHT) h=child->getHeight(); 
      else h=child->getDefaultHeight();
      if(side==LAYOUT_SIDE_LEFT){           // Left side
        if(!(hints&LAYOUT_FIX_Y)){
          extra_space=0;
          if(hints&LAYOUT_FILL_Y){
            h=bottom-top;
            if(h<0) h=0;
            }
          else if(hints&LAYOUT_CENTER_Y){
            if(h<(bottom-top)) extra_space=(bottom-top-h)/2;
            }
          if(hints&LAYOUT_BOTTOM)
            y=bottom-extra_space-h;
          else /*hints&LAYOUT_TOP*/
            y=top+extra_space;
          }
        extra_space=0;
        total_space=0;
        if(hints&LAYOUT_FILL_X){
          w=right-left;
          if(w<0) w=0;
          }
        else if(hints&LAYOUT_CENTER_X){
          if(w<(right-left)){
            total_space=right-left-w;
            extra_space=total_space/2;
            }
          }
        x=left+extra_space;
        left+=(w+hspacing+total_space);
        }
      else if(side==LAYOUT_SIDE_RIGHT){     // Right side
        if(!(hints&LAYOUT_FIX_Y)){
          extra_space=0;
          if(hints&LAYOUT_FILL_Y){
            h=bottom-top;
            if(h<0) h=0;
            }
          else if(hints&LAYOUT_CENTER_Y){
            if(h<(bottom-top)) extra_space=(bottom-top-h)/2;
            }
          if(hints&LAYOUT_BOTTOM)
            y=bottom-extra_space-h;
          else /*hints&LAYOUT_TOP*/
            y=top+extra_space;
          }
        extra_space=0;
        total_space=0;
        if(hints&LAYOUT_FILL_X){
          w=right-left;
          if(w<0) w=0;
          }
        else if(hints&LAYOUT_CENTER_X){
          if(w<(right-left)){
            total_space=right-left-w;
            extra_space=total_space/2;
            }
          }
        x=right-w-extra_space;
        right-=(w+hspacing+total_space);
        }
      else if(side==LAYOUT_SIDE_BOTTOM){    // Bottom side
        if(!(hints&LAYOUT_FIX_X)){
          extra_space=0;
          if(hints&LAYOUT_FILL_X){
            w=right-left;
            if(w<0) w=0;
            }
          else if(hints&LAYOUT_CENTER_X){
            if(w<(right-left)) extra_space=(right-left-w)/2;
            }
          if(hints&LAYOUT_RIGHT)
            x=right-extra_space-w;
          else /*hints&LAYOUT_LEFT*/
            x=left+extra_space;
          }
        extra_space=0;
        total_space=0;
        if(hints&LAYOUT_FILL_Y){
          h=bottom-top;
          if(h<0) h=0;
          }
        else if(hints&LAYOUT_CENTER_Y){
          if(h<(bottom-top)){
            total_space=bottom-top-h;
            extra_space=total_space/2;
            }
          }
        y=bottom-h-extra_space;
        bottom-=(h+vspacing+total_space);
        }
      else if(side==LAYOUT_SIDE_TOP){       // Top side
        if(!(hints&LAYOUT_FIX_X)){
          extra_space=0;
          if(hints&LAYOUT_FILL_X){
            w=right-left;
            if(w<0) w=0;
            }
          else if(hints&LAYOUT_CENTER_X){
            if(w<(right-left)) extra_space=(right-left-w)/2;
            }
          if(hints&LAYOUT_RIGHT)
            x=right-extra_space-w;
          else /*hints&LAYOUT_LEFT*/
            x=left+extra_space;
          }
        extra_space=0;
        total_space=0;
        if(hints&LAYOUT_FILL_Y){
          h=bottom-top;
          if(h<0) h=0;
          }
        else if(hints&LAYOUT_CENTER_Y){
          if(h<(bottom-top)){
            total_space=bottom-top-h;
            extra_space=total_space/2;
            }
          }
        y=top+extra_space;
        top+=(h+vspacing+total_space);
        }
      child->position(x,y,w,h);
      }
    }
  
  // No more dirty
  flags&=~FLAG_DIRTY;
  }


// Request for toplevel window move
void FXTopWindow::move(FXint x,FXint y){
  if(xid){
    XWindowChanges cw;
    cw.x=x;
    cw.y=y;
    XReconfigureWMWindow(getApp()->display,xid,DefaultScreen(getApp()->display),CWX|CWY,&cw);
    recalc(); ////// perhaps this is unnecessary...
    }
  }
  
  
// Request for toplevel window resize
void FXTopWindow::resize(FXint w,FXint h){
  if(xid){
    XWindowChanges cw;
    cw.width=FXMAX(w,1);
    cw.height=FXMAX(h,1);
    XReconfigureWMWindow(getApp()->display,xid,DefaultScreen(getApp()->display),CWWidth|CWHeight,&cw);
    recalc();
    }
  }

  
// Request for toplevel window reposition
void FXTopWindow::position(FXint x,FXint y,FXint w,FXint h){
  if(xid){
    XWindowChanges cw;
    cw.x=x;
    cw.y=y;
    cw.width=FXMAX(w,1);
    cw.height=FXMAX(h,1);
    XReconfigureWMWindow(getApp()->display,xid,DefaultScreen(getApp()->display),CWX|CWY|CWWidth|CWHeight,&cw);
    recalc();
    }
  }


// Set new window title
void FXTopWindow::setTitle(const char* name){
  title=name;
  if(xid) settitle();
  }


// Change horizontal spacing
void FXTopWindow::setHSpacing(FXint hs){
  if(hspacing!=hs){
    hspacing=hs;
    recalc();
    update(0,0,width,height);
    }
  }


// Change vertical spacing
void FXTopWindow::setVSpacing(FXint vs){
  if(vspacing!=vs){
    vspacing=vs;
    recalc();
    update(0,0,width,height);
    }
  }


// Remove this one from toplevel window list
FXTopWindow::~FXTopWindow(){
  }


