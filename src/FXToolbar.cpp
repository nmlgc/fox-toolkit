/********************************************************************************
*                                                                               *
*                        T o o l b a r   W i d g e t                            *
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
* $Id: FXToolbar.cpp,v 1.61.4.1 2002/10/03 06:08:48 fox Exp $                    *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXRegistry.h"
#include "FXAccelTable.h"
#include "FXApp.h"
#include "FXDCWindow.h"
#include "FXDrawable.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXComposite.h"
#include "FXPacker.h"
#include "FXToolbar.h"
#include "FXShell.h"
#include "FXTopWindow.h"
#include "FXToolbarGrip.h"


/*
  Notes:
  - Wrapping algorithm not OK yet; use D.P.
  - Divider widgets and special layout ideas:

      +---------+---------------+
      | 1 2 3 4 | 5 6 7         |
      +---------+---------------+

    after wrap:

      +-----+-----+
      | 1 2 | 5 6 |  Clue: use D.P.
      | 3 4 | 7   |
      +-----+-----+

    wrap:

      +-------------+
      | L L L L   R |
      +-------------+

    to:

      +---------+
      | L L L R |
      +---------+
      | L       |
      +---------+

  - Toolbars with no wetdock should still be redockable on different sides.
  - Want to support stretchable items.
  - Want to support non-equal galley heights.

*/

// How close to edge before considered docked
#define PROXIMITY    30
#define FUDGE        5

// Docking side
#define LAYOUT_SIDE_MASK (LAYOUT_SIDE_LEFT|LAYOUT_SIDE_RIGHT|LAYOUT_SIDE_TOP|LAYOUT_SIDE_BOTTOM)

// Horizontal placement options
#define LAYOUT_HORIZONTAL_MASK (LAYOUT_LEFT|LAYOUT_RIGHT|LAYOUT_CENTER_X|LAYOUT_FIX_X|LAYOUT_FILL_X)

// Vertical placement options
#define LAYOUT_VERTICAL_MASK   (LAYOUT_TOP|LAYOUT_BOTTOM|LAYOUT_CENTER_Y|LAYOUT_FIX_Y|LAYOUT_FILL_Y)

/*******************************************************************************/

// Map
FXDEFMAP(FXToolbar) FXToolbarMap[]={
  FXMAPFUNC(SEL_FOCUS_PREV,0,FXToolbar::onFocusLeft),
  FXMAPFUNC(SEL_FOCUS_NEXT,0,FXToolbar::onFocusRight),
  FXMAPFUNC(SEL_UPDATE,FXToolbar::ID_UNDOCK,FXToolbar::onUpdUndock),
  FXMAPFUNC(SEL_UPDATE,FXToolbar::ID_DOCK_TOP,FXToolbar::onUpdDockTop),
  FXMAPFUNC(SEL_UPDATE,FXToolbar::ID_DOCK_BOTTOM,FXToolbar::onUpdDockBottom),
  FXMAPFUNC(SEL_UPDATE,FXToolbar::ID_DOCK_LEFT,FXToolbar::onUpdDockLeft),
  FXMAPFUNC(SEL_UPDATE,FXToolbar::ID_DOCK_RIGHT,FXToolbar::onUpdDockRight),
  FXMAPFUNC(SEL_COMMAND,FXToolbar::ID_UNDOCK,FXToolbar::onCmdUndock),
  FXMAPFUNC(SEL_COMMAND,FXToolbar::ID_DOCK_TOP,FXToolbar::onCmdDockTop),
  FXMAPFUNC(SEL_COMMAND,FXToolbar::ID_DOCK_BOTTOM,FXToolbar::onCmdDockBottom),
  FXMAPFUNC(SEL_COMMAND,FXToolbar::ID_DOCK_LEFT,FXToolbar::onCmdDockLeft),
  FXMAPFUNC(SEL_COMMAND,FXToolbar::ID_DOCK_RIGHT,FXToolbar::onCmdDockRight),
  FXMAPFUNC(SEL_BEGINDRAG,FXToolbar::ID_TOOLBARGRIP,FXToolbar::onBeginDragGrip),
  FXMAPFUNC(SEL_ENDDRAG,FXToolbar::ID_TOOLBARGRIP,FXToolbar::onEndDragGrip),
  FXMAPFUNC(SEL_DRAGGED,FXToolbar::ID_TOOLBARGRIP,FXToolbar::onDraggedGrip),
  };


// Object implementation
FXIMPLEMENT(FXToolbar,FXPacker,FXToolbarMap,ARRAYNUMBER(FXToolbarMap))


// Deserialization
FXToolbar::FXToolbar(){
  drydock=NULL;
  wetdock=NULL;
  outline.x=0;
  outline.y=0;
  outline.w=0;
  outline.h=0;
  dockafter=NULL;
  dockside=0;
  docking=FALSE;
  }


// Make a dockable and, possibly, floatable toolbar
FXToolbar::FXToolbar(FXComposite* p,FXComposite* q,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs):
  FXPacker(p,opts,x,y,w,h,pl,pr,pt,pb,hs,vs){
  drydock=p;
  wetdock=q;
  outline.x=0;
  outline.y=0;
  outline.w=0;
  outline.h=0;
  dockafter=NULL;
  dockside=0;
  docking=FALSE;
  }


// Make a non-floatable toolbar
FXToolbar::FXToolbar(FXComposite* p,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs):
  FXPacker(p,opts,x,y,w,h,pl,pr,pt,pb,hs,vs){
  drydock=NULL;
  wetdock=NULL;
  outline.x=0;
  outline.y=0;
  outline.w=0;
  outline.h=0;
  dockafter=NULL;
  dockside=0;
  docking=FALSE;
  }


// Compute minimum width based on child layout hints
int FXToolbar::getDefaultWidth(){
  register FXint w,wcum,wmax,mw,n;
  register FXWindow* child;
  register FXuint hints;
  wcum=wmax=n=0;
  if(options&PACK_UNIFORM_WIDTH) mw=maxChildWidth();
  for(child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      hints=child->getLayoutHints();
      if(child->isMemberOf(FXMETACLASS(FXToolbarGrip))) w=child->getDefaultWidth();
      else if(hints&LAYOUT_FIX_WIDTH) w=child->getWidth();
      else if(options&PACK_UNIFORM_WIDTH) w=mw;
      else w=child->getDefaultWidth();
      if(wmax<w) wmax=w;
      wcum+=w;
      n++;
      }
    }
  if(!(options&LAYOUT_SIDE_LEFT)){      // Horizontal
    if(n>1) wcum+=(n-1)*hspacing;
    wmax=wcum;
    }
  return padleft+padright+wmax+(border<<1);
  }


// Compute minimum height based on child layout hints
int FXToolbar::getDefaultHeight(){
  register FXint h,hcum,hmax,mh,n;
  register FXWindow* child;
  register FXuint hints;
  hcum=hmax=n=0;
  if(options&PACK_UNIFORM_HEIGHT) mh=maxChildHeight();
  for(child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      hints=child->getLayoutHints();
      if(child->isMemberOf(FXMETACLASS(FXToolbarGrip))) h=child->getDefaultHeight();
      else if(hints&LAYOUT_FIX_HEIGHT) h=child->getHeight();
      else if(options&PACK_UNIFORM_HEIGHT) h=mh;
      else h=child->getDefaultHeight();
      if(hmax<h) hmax=h;
      hcum+=h;
      n++;
      }
    }
  if(options&LAYOUT_SIDE_LEFT){         // Vertical
    if(n>1) hcum+=(n-1)*vspacing;
    hmax=hcum;
    }
  return padtop+padbottom+hmax+(border<<1);
  }


// Return width for given height
FXint FXToolbar::getWidthForHeight(FXint givenheight){
  FXint wtot,wmax,hcum,w,h,mw,mh,space,ngalleys;
  FXWindow* child;
  FXuint hints;
  wtot=wmax=hcum=ngalleys=0;
  space=givenheight-padtop-padbottom-(border<<1);
  if(space<1) space=1;
  if(options&PACK_UNIFORM_WIDTH) mw=maxChildWidth();
  if(options&PACK_UNIFORM_HEIGHT) mh=maxChildHeight();
  for(child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      hints=child->getLayoutHints();
      if(child->isMemberOf(FXMETACLASS(FXToolbarGrip))) w=child->getDefaultWidth();
      else if(hints&LAYOUT_FIX_WIDTH) w=child->getWidth();
      else if(options&PACK_UNIFORM_WIDTH) w=mw;
      else w=child->getDefaultWidth();
      if(child->isMemberOf(FXMETACLASS(FXToolbarGrip))) h=child->getDefaultHeight();
      else if(hints&LAYOUT_FIX_HEIGHT) h=child->getHeight();
      else if(options&PACK_UNIFORM_HEIGHT) h=mh;
      else h=child->getDefaultHeight();
      if(hcum+h>space) hcum=0;
      if(hcum==0) ngalleys++;
      hcum+=h+vspacing;
      if(wmax<w) wmax=w;
      }
    }
  wtot=wmax*ngalleys;
  return padleft+padright+wtot+(border<<1);
  }


// Return height for given width
FXint FXToolbar::getHeightForWidth(FXint givenwidth){
  FXint htot,hmax,wcum,w,h,mw,mh,space,ngalleys;
  FXWindow* child;
  FXuint hints;
  htot=hmax=wcum=ngalleys=0;
  space=givenwidth-padleft-padright-(border<<1);
  if(space<1) space=1;
  if(options&PACK_UNIFORM_WIDTH) mw=maxChildWidth();
  if(options&PACK_UNIFORM_HEIGHT) mh=maxChildHeight();
  for(child=getFirst(); child; child=child->getNext()){
    if(child->shown()){
      hints=child->getLayoutHints();
      if(child->isMemberOf(FXMETACLASS(FXToolbarGrip))) w=child->getDefaultWidth();
      else if(hints&LAYOUT_FIX_WIDTH) w=child->getWidth();
      else if(options&PACK_UNIFORM_WIDTH) w=mw;
      else w=child->getDefaultWidth();
      if(child->isMemberOf(FXMETACLASS(FXToolbarGrip))) h=child->getDefaultHeight();
      else if(hints&LAYOUT_FIX_HEIGHT) h=child->getHeight();
      else if(options&PACK_UNIFORM_HEIGHT) h=mh;
      else h=child->getDefaultHeight();
      if(wcum+w>space) wcum=0;
      if(wcum==0) ngalleys++;
      wcum+=w+hspacing;
      if(hmax<h) hmax=h;
      }
    }
  htot=hmax*ngalleys;
  return padtop+padbottom+htot+(border<<1);
  }


// Recalculate layout
void FXToolbar::layout(){
  FXint galleyleft,galleyright,galleytop,galleybottom,galleywidth,galleyheight;
  FXint tleft,tright,ttop,bleft,bright,bbottom;
  FXint ltop,lbottom,lleft,rtop,rbottom,rright;
  FXWindow *child;
  FXint x,y,w,h,mw,mh;
  FXuint hints;

  //FXTRACE((150,"%s::layout\n",getClassName()));

  // Get maximum child size
  if(options&PACK_UNIFORM_WIDTH) mw=maxChildWidth();
  if(options&PACK_UNIFORM_HEIGHT) mh=maxChildHeight();

  // Vertical toolbar
  if(options&LAYOUT_SIDE_LEFT){
    galleywidth=0;
    for(child=getFirst(); child; child=child->getNext()){
      if(child->shown()){
        hints=child->getLayoutHints();
        if(child->isMemberOf(FXMETACLASS(FXToolbarGrip))) w=child->getDefaultWidth();
        else if(hints&LAYOUT_FIX_WIDTH) w=child->getWidth();
        else if(options&PACK_UNIFORM_WIDTH) w=mw;
        else w=child->getDefaultWidth();
        if(galleywidth<w) galleywidth=w;
        }
      }
    galleyleft=border+padleft;
    galleyright=width-border-padright;
    galleytop=border+padtop;
    galleybottom=height-border-padbottom;
    //FXTRACE((100,"vert galleyleft=%d galleyright=%d galleytop=%d galleybottom=%d galleywidth=%d\n",galleyleft,galleyright,galleytop,galleybottom,galleywidth));
    tleft=galleyleft;
    tright=galleyleft+galleywidth;
    ttop=galleytop;
    bright=galleyright;
    bleft=galleyright-galleywidth;
    bbottom=galleybottom;
    for(child=getFirst(); child; child=child->getNext()){
      if(child->shown()){
        hints=child->getLayoutHints();
        if(child->isMemberOf(FXMETACLASS(FXToolbarGrip))){
          w=galleywidth;
          h=child->getDefaultHeight();
          }
        else{
          if(hints&LAYOUT_FIX_WIDTH) w=child->getWidth();
          else if(options&PACK_UNIFORM_WIDTH) w=mw;
          else w=child->getDefaultWidth();
          if(hints&LAYOUT_FIX_HEIGHT) h=child->getHeight();
          else if(options&PACK_UNIFORM_HEIGHT) h=mh;
          else h=child->getDefaultHeight();
          }
        if(hints&LAYOUT_BOTTOM){
          if(bbottom-h<galleytop && bbottom!=galleybottom){
            bright=bleft;
            bleft-=galleywidth;
            bbottom=galleybottom;
            }
          y=bbottom-h;
          bbottom-=(h+vspacing);
          x=bleft+(galleywidth-w)/2;
          //FXTRACE((100,"vert bottom: x=%d y=%d w=%d h=%d\n",x,y,w,h));
          }
        else{
          if(ttop+h>galleybottom && ttop!=galleytop){
            tleft=tright;
            tright+=galleywidth;
            ttop=galleytop;
            }
          y=ttop;
          ttop+=(h+vspacing);
          x=tleft+(galleywidth-w)/2;
          //FXTRACE((100,"vert top   : x=%d y=%d w=%d h=%d\n",x,y,w,h));
          }
        child->position(x,y,w,h);
        }
      }
    }

  // Horizontal toolbar
  else{
    galleyheight=0;
    for(child=getFirst(); child; child=child->getNext()){
      if(child->shown()){
        hints=child->getLayoutHints();
        if(child->isMemberOf(FXMETACLASS(FXToolbarGrip))) h=child->getDefaultHeight();
        else if(hints&LAYOUT_FIX_HEIGHT) h=child->getHeight();
        else if(options&PACK_UNIFORM_HEIGHT) h=mh;
        else h=child->getDefaultHeight();
        if(galleyheight<h) galleyheight=h;
        }
      }
    galleyleft=border+padleft;
    galleyright=width-border-padright;
    galleytop=border+padtop;
    galleybottom=height-border-padbottom;
    //FXTRACE((100,"horz galleyleft=%d galleyright=%d galleytop=%d galleybottom=%d galleyheight=%d\n",galleyleft,galleyright,galleytop,galleybottom,galleyheight));
    ltop=galleytop;
    lbottom=galleytop+galleyheight;
    lleft=galleyleft;
    rbottom=galleybottom;
    rtop=galleybottom-galleyheight;
    rright=galleyright;
    for(child=getFirst(); child; child=child->getNext()){
      if(child->shown()){
        hints=child->getLayoutHints();
        if(child->isMemberOf(FXMETACLASS(FXToolbarGrip))){
          w=child->getDefaultWidth();
          h=galleyheight;
          }
        else{
          if(hints&LAYOUT_FIX_WIDTH) w=child->getWidth();
          else if(options&PACK_UNIFORM_WIDTH) w=mw;
          else w=child->getDefaultWidth();
          if(hints&LAYOUT_FIX_HEIGHT) h=child->getHeight();
          else if(options&PACK_UNIFORM_HEIGHT) h=mh;
          else h=child->getDefaultHeight();
          }
        if(hints&LAYOUT_RIGHT){
          if(rright-w<galleyleft && rright!=galleyright){
            rbottom=rtop;
            rtop-=galleyheight;
            rright=galleyright;
            }
          x=rright-w;
          rright-=(w+hspacing);
          y=rtop+(galleyheight-h)/2;
          //FXTRACE((100,"horz right : x=%d y=%d w=%d h=%d\n",x,y,w,h));
          }
        else{
          if(lleft+w>galleyright && lleft!=galleyleft){
            ltop=lbottom;
            lbottom+=galleyheight;
            lleft=galleyleft;
            }
          x=lleft;
          lleft+=(w+hspacing);
          y=ltop+(galleyheight-h)/2;
          //FXTRACE((100,"horz left  : x=%d y=%d w=%d h=%d\n",x,y,w,h));
          }
        child->position(x,y,w,h);
        }
      }
    }
  flags&=~FLAG_DIRTY;
  }


// Change toolbar orientation
void FXToolbar::setDockingSide(FXuint side){
  if((options&LAYOUT_SIDE_MASK)!=side){

    // New orientation is vertical
    if(side&LAYOUT_SIDE_LEFT){
      if(!(options&LAYOUT_SIDE_LEFT)){    // Was horizontal
        if((options&LAYOUT_RIGHT) && (options&LAYOUT_CENTER_X)) side|=LAYOUT_FIX_Y;
        else if(options&LAYOUT_RIGHT) side|=LAYOUT_BOTTOM;
        else if(options&LAYOUT_CENTER_X) side|=LAYOUT_CENTER_Y;
        if(options&LAYOUT_FILL_X) side|=LAYOUT_FILL_Y;
        }
      else{                               // Was vertical already
        side|=(options&(LAYOUT_BOTTOM|LAYOUT_CENTER_Y|LAYOUT_FILL_Y));
        }
      }

    // New orientation is horizontal
    else{
      if(options&LAYOUT_SIDE_LEFT){       // Was vertical
        if((options&LAYOUT_BOTTOM) && (options&LAYOUT_CENTER_Y)) side|=LAYOUT_FIX_X;
        else if(options&LAYOUT_BOTTOM) side|=LAYOUT_RIGHT;
        else if(options&LAYOUT_CENTER_Y) side|=LAYOUT_CENTER_X;
        if(options&LAYOUT_FILL_Y) side|=LAYOUT_FILL_X;
        }
      else{
        side|=(options&(LAYOUT_RIGHT|LAYOUT_CENTER_X|LAYOUT_FILL_X));
        }
      }

    // Simply preserve these options
    side|=(options&(LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT));

    // Update the layout
    setLayoutHints(side);
    }
  }


// Get toolbar orientation
FXuint FXToolbar::getDockingSide() const {
  return (options&LAYOUT_SIDE_MASK);
  }


// Return true if toolbar is docked
FXbool FXToolbar::isDocked() const {
  return (getParent()!=(FXWindow*)wetdock);
  }


// Set parent when docked, if it was docked it will remain docked
void FXToolbar::setDryDock(FXComposite* dry){
  if(dry && dry->id() && getParent()==(FXWindow*)drydock){
    reparent(dry);
    FXWindow* child=dry->getFirst();
    FXWindow* after=NULL;
    while(child){
      FXuint hints=child->getLayoutHints();
      if((hints&LAYOUT_FILL_X) && (hints&LAYOUT_FILL_Y)) break;
      after=child;
      child=child->getNext();
      }
    linkAfter(after);
    }
  drydock=dry;
  }


// Set parent when floating
void FXToolbar::setWetDock(FXComposite* wet){
  if(wet && wet->id() && getParent()==(FXWindow*)wetdock){
    reparent(wet);
    }
  wetdock=wet;
  }


// Dock the bar
void FXToolbar::dock(FXuint side,FXWindow* after){
  setDockingSide(side);
  if(drydock && !isDocked()){
    reparent(drydock);
    wetdock->hide();
    }
  if(after==(FXWindow*)-1){
    after=NULL;
    FXWindow* child=getParent()->getFirst();
    while(child){
      FXuint hints=child->getLayoutHints();
      if((hints&LAYOUT_FILL_X) && (hints&LAYOUT_FILL_Y)) break;
      after=child;
      child=child->getNext();
      }
    }
  linkAfter(after);
  }


// Undock the bar
void FXToolbar::undock(){
  if(wetdock && isDocked()){
    FXint rootx,rooty;
    translateCoordinatesTo(rootx,rooty,getRoot(),8,8);
    reparent(wetdock);
    wetdock->position(rootx,rooty,wetdock->getDefaultWidth(),wetdock->getDefaultHeight());
    wetdock->show();
    }
  }


// Undock
long FXToolbar::onCmdUndock(FXObject*,FXSelector,void*){
  undock();
  return 1;
  }

long FXToolbar::onUpdUndock(FXObject* sender,FXSelector,void*){
  if(isDocked())
    sender->handle(this,MKUINT(ID_UNCHECK,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_CHECK,SEL_COMMAND),NULL);
  if(wetdock)
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Redock on top
long FXToolbar::onCmdDockTop(FXObject*,FXSelector,void*){
  dock(LAYOUT_SIDE_TOP,(FXWindow*)-1);
  return 1;
  }

long FXToolbar::onUpdDockTop(FXObject* sender,FXSelector,void*){
  if(isDocked() && (options&LAYOUT_SIDE_MASK)==LAYOUT_SIDE_TOP)
    sender->handle(this,MKUINT(ID_CHECK,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_UNCHECK,SEL_COMMAND),NULL);
  return 1;
  }


// Redock on bottom
long FXToolbar::onCmdDockBottom(FXObject*,FXSelector,void*){
  dock(LAYOUT_SIDE_BOTTOM,(FXWindow*)-1);
  return 1;
  }

long FXToolbar::onUpdDockBottom(FXObject* sender,FXSelector,void*){
  if(isDocked() && (options&LAYOUT_SIDE_MASK)==LAYOUT_SIDE_BOTTOM)
    sender->handle(this,MKUINT(ID_CHECK,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_UNCHECK,SEL_COMMAND),NULL);
  return 1;
  }


// Redock on left
long FXToolbar::onCmdDockLeft(FXObject*,FXSelector,void*){
  dock(LAYOUT_SIDE_LEFT,(FXWindow*)-1);
  return 1;
  }

long FXToolbar::onUpdDockLeft(FXObject* sender,FXSelector,void*){
  if(isDocked() && (options&LAYOUT_SIDE_MASK)==LAYOUT_SIDE_LEFT)
    sender->handle(this,MKUINT(ID_CHECK,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_UNCHECK,SEL_COMMAND),NULL);
  return 1;
  }


// Redock on right
long FXToolbar::onCmdDockRight(FXObject*,FXSelector,void*){
  dock(LAYOUT_SIDE_RIGHT,(FXWindow*)-1);
  return 1;
  }

long FXToolbar::onUpdDockRight(FXObject* sender,FXSelector,void*){
  if(isDocked() && (options&LAYOUT_SIDE_MASK)==LAYOUT_SIDE_RIGHT)
    sender->handle(this,MKUINT(ID_CHECK,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_UNCHECK,SEL_COMMAND),NULL);
  return 1;
  }


// Toolbar grip drag started
long FXToolbar::onBeginDragGrip(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  FXint x,y;
  FXDCWindow dc(getRoot());
  FXTRACE((100,"FXToolbar::onBeginDragGrip(%d,%d)\n",event->root_x,event->root_y));
  translateCoordinatesTo(x,y,getRoot(),0,0);
  outline.x=x;
  outline.y=y;
  outline.w=width;
  outline.h=height;
  dockafter=getPrev();
  dockside=(options&LAYOUT_SIDE_MASK);
  docking=isDocked();
  dc.clipChildren(FALSE);
  dc.setFunction(BLT_SRC_XOR_DST);
  dc.setForeground(FXRGB(255,255,255));
  dc.setLineWidth(3);
  dc.drawRectangles(&outline,1);
  getApp()->flush();
  return 1;
  }


// Toolbar grip drag ended
long FXToolbar::onEndDragGrip(FXObject* sender,FXSelector,void* ptr){
  FXToolbarGrip *grip=(FXToolbarGrip*)sender;
  FXEvent *event=(FXEvent*)ptr;
  FXint x,y;
  FXDCWindow dc(getRoot());
  FXTRACE((100,"FXToolbar::onEndDragGrip(%d,%d)\n",event->root_x,event->root_y));
  dc.clipChildren(FALSE);
  dc.setFunction(BLT_SRC_XOR_DST);
  dc.setForeground(FXRGB(255,255,255));
  dc.setLineWidth(3);
  dc.drawRectangles(&outline,1);
  getApp()->flush();
  if(docking){
    dock(dockside,dockafter);
    }
  else{
    undock();
    x=event->root_x-event->click_x-grip->getX();
    y=event->root_y-event->click_y-grip->getY();
    wetdock->move(x,y);
    }
  return 1;
  }


// Toolbar grip dragged
long FXToolbar::onDraggedGrip(FXObject* sender,FXSelector,void* ptr){
  FXToolbarGrip *grip=(FXToolbarGrip*)sender;
  FXEvent *event=(FXEvent*)ptr;
  FXTRACE((100,"FXToolbar::onDraggedGrip(%d,%d)\n",event->root_x,event->root_y));
  FXint left,right,top,bottom,x,y,twx,twy;
  FXWindow *child,*after,*harbor;
  FXRectangle newoutline;
  FXuint hints,opts;

  // Current grip location
  x=event->root_x-event->click_x-grip->getX();
  y=event->root_y-event->click_y-grip->getY();

  // Move the highlight
  newoutline.x=x;
  newoutline.y=y;
  newoutline.w=width;
  newoutline.h=height;

  // Initialize
  if(drydock && wetdock){     // We can float if not close enough to docking spot
    //newoutline.w=outline.w;
    //newoutline.h=outline.h;
    harbor=drydock;
    dockafter=NULL;
    docking=FALSE;
    }
  else{                       // If too far from docking spot, snap back to original location
    //newoutline.w=width;
    //newoutline.h=height;
    harbor=getParent();
    dockside=(options&LAYOUT_SIDE_MASK);
    dockafter=getPrev();
    docking=TRUE;
    }

  // Drydock location in root coordinates
  harbor->translateCoordinatesTo(twx,twy,getRoot(),0,0);

  // Inner bounds
  left = twx; right  = twx + harbor->getWidth();
  top  = twy; bottom = twy + harbor->getHeight();

  // Find place where to dock
  after=NULL;
  child=harbor->getFirst();
  while(left<right && top<bottom){

    // Determine docking side
    if(top<=y && y<bottom){
      if(FXABS(x-left)<PROXIMITY){
        opts=options;
        options=(options&~LAYOUT_SIDE_MASK)|LAYOUT_SIDE_LEFT;
        if(options&LAYOUT_FIX_HEIGHT) newoutline.h=height;
        else if(options&(LAYOUT_FILL_Y|LAYOUT_FILL_X)){ newoutline.h=bottom-top; newoutline.y=top; }
        else newoutline.h=getDefaultHeight();
        if(options&LAYOUT_FIX_WIDTH) newoutline.w=width;
        else newoutline.w=getWidthForHeight(newoutline.h);
        options=opts;
        newoutline.x=left;
        dockside=LAYOUT_SIDE_LEFT;
        dockafter=after;
        docking=TRUE;
        }
      if(FXABS(x-right)<PROXIMITY){
        opts=options;
        options=(options&~LAYOUT_SIDE_MASK)|LAYOUT_SIDE_RIGHT;
        if(options&LAYOUT_FIX_HEIGHT) newoutline.h=height;
        else if(options&(LAYOUT_FILL_Y|LAYOUT_FILL_X)){ newoutline.h=bottom-top; newoutline.y=top; }
        else newoutline.h=getDefaultHeight();
        if(options&LAYOUT_FIX_WIDTH) newoutline.w=width;
        else newoutline.w=getWidthForHeight(newoutline.h);
        options=opts;
        newoutline.x=right-newoutline.w;
        dockside=LAYOUT_SIDE_RIGHT;
        dockafter=after;
        docking=TRUE;
        }
      }
    if(left<=x && x<right){
      if(FXABS(y-top)<PROXIMITY){
        opts=options;
        options=(options&~LAYOUT_SIDE_MASK)|LAYOUT_SIDE_TOP;
        if(options&LAYOUT_FIX_WIDTH) newoutline.w=width;
        else if(options&(LAYOUT_FILL_X|LAYOUT_FILL_Y)){ newoutline.w=right-left; newoutline.x=left; }
        else newoutline.w=getDefaultWidth();
        if(options&LAYOUT_FIX_HEIGHT) newoutline.h=height;
        else newoutline.h=getHeightForWidth(newoutline.w);
        options=opts;
        newoutline.y=top;
        dockside=LAYOUT_SIDE_TOP;
        dockafter=after;
        docking=TRUE;
        }
      if(FXABS(y-bottom)<PROXIMITY){
        opts=options;
        options=(options&~LAYOUT_SIDE_MASK)|LAYOUT_SIDE_BOTTOM;
        if(options&LAYOUT_FIX_WIDTH) newoutline.w=width;
        else if(options&(LAYOUT_FILL_X|LAYOUT_FILL_Y)){ newoutline.w=right-left; newoutline.x=left; }
        else newoutline.w=getDefaultWidth();
        if(options&LAYOUT_FIX_HEIGHT) newoutline.h=height;
        else newoutline.h=getHeightForWidth(newoutline.w);
        options=opts;
        newoutline.y=bottom-newoutline.h;
        dockside=LAYOUT_SIDE_BOTTOM;
        dockafter=after;
        docking=TRUE;
        }
      }

    // Done
    if(!child) break;

    // Get child hints
    hints=child->getLayoutHints();

    // Some final fully stretched child also marks the end
    if((hints&LAYOUT_FILL_X) && (hints&LAYOUT_FILL_Y)) break;

    // Advance inward
    if(child!=this){
      if(child->shown()){

        // Vertical
        if(hints&LAYOUT_SIDE_LEFT){
          if(!((hints&LAYOUT_RIGHT)&&(hints&LAYOUT_CENTER_X))){
            if(hints&LAYOUT_SIDE_BOTTOM){
              right=twx+child->getX();
              }
            else{
              left=twx+child->getX()+child->getWidth();
              }
            }
          }

        // Horizontal
        else{
          if(!((hints&LAYOUT_BOTTOM)&&(hints&LAYOUT_CENTER_Y))){
            if(hints&LAYOUT_SIDE_BOTTOM){
              bottom=twy+child->getY();
              }
            else{
              top=twy+child->getY()+child->getHeight();
              }
            }
          }
        }
      }
    after=child;

    // Next one
    child=child->getNext();
    }

  // Did rectangle move?
  if(newoutline.x!=outline.x || newoutline.y!=outline.y || newoutline.w!=outline.w || newoutline.h!=outline.h){
    FXDCWindow dc(getRoot());
    dc.clipChildren(FALSE);
    dc.setFunction(BLT_SRC_XOR_DST);
    dc.setForeground(FXRGB(255,255,255));
    dc.setLineWidth(3);
    dc.drawRectangles(&outline,1);
    outline=newoutline;
    dc.drawRectangles(&outline,1);
    getApp()->flush();
    }
  return 1;
  }


// Save data
void FXToolbar::save(FXStream& store) const {
  FXPacker::save(store);
  store << drydock;
  store << wetdock;
  }


// Load data
void FXToolbar::load(FXStream& store){
  FXPacker::load(store);
  store >> drydock;
  store >> wetdock;
  }


// Destroy
FXToolbar::~FXToolbar(){
  drydock=(FXComposite*)-1;
  wetdock=(FXComposite*)-1;
  dockafter=(FXWindow*)-1;
  }
