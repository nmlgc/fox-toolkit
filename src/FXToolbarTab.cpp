/********************************************************************************
*                                                                               *
*                       T o o l b a r   T a b   O b j e c t                     *
*                                                                               *
*********************************************************************************
* Copyright (C) 1999,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXToolbarTab.cpp,v 1.17 2002/01/18 22:43:07 jeroen Exp $                 *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxkeys.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXRegistry.h"
#include "FXApp.h"
#include "FXDCWindow.h"
#include "FXFont.h"
#include "FXToolbarTab.h"


/*
  To do:
*/


// Size
#define TOOLBARTAB_WIDTH    9       // Width for horizontal toolbar tab
#define TOOLBARTAB_HEIGHT   24      // Height for horizontal toolbar tab

// Toolbar Tab styles
#define TOOLBARTAB_MASK        TOOLBARTAB_VERTICAL


/*******************************************************************************/

// Map
FXDEFMAP(FXToolbarTab) FXToolbarTabMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXToolbarTab::onPaint),
  FXMAPFUNC(SEL_UPDATE,0,FXToolbarTab::onUpdate),
  FXMAPFUNC(SEL_ENTER,0,FXToolbarTab::onEnter),
  FXMAPFUNC(SEL_LEAVE,0,FXToolbarTab::onLeave),
  FXMAPFUNC(SEL_UNGRABBED,0,FXToolbarTab::onUngrabbed),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXToolbarTab::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXToolbarTab::onLeftBtnRelease),
  FXMAPFUNC(SEL_KEYPRESS,0,FXToolbarTab::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXToolbarTab::onKeyRelease),
  FXMAPFUNC(SEL_UPDATE,FXToolbarTab::ID_COLLAPSE,FXToolbarTab::onUpdCollapse),
  FXMAPFUNC(SEL_UPDATE,FXToolbarTab::ID_UNCOLLAPSE,FXToolbarTab::onUpdUncollapse),
  FXMAPFUNC(SEL_COMMAND,FXToolbarTab::ID_COLLAPSE,FXToolbarTab::onCmdCollapse),
  FXMAPFUNC(SEL_COMMAND,FXToolbarTab::ID_UNCOLLAPSE,FXToolbarTab::onCmdUncollapse),
  };


// Object implementation
FXIMPLEMENT(FXToolbarTab,FXFrame,FXToolbarTabMap,ARRAYNUMBER(FXToolbarTabMap))


// Deserialization
FXToolbarTab::FXToolbarTab(){
  flags|=FLAG_ENABLED;
  activeColor=FXRGB(150,156,224);
  collapsed=FALSE;
  down=FALSE;
  }


// Construct and init
FXToolbarTab::FXToolbarTab(FXComposite* p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXFrame(p,opts,x,y,w,h){
  flags|=FLAG_ENABLED;
  activeColor=FXRGB(150,156,224);
  target=tgt;
  message=sel;
  collapsed=FALSE;
  down=FALSE;
  }


// If window can have focus
FXbool FXToolbarTab::canFocus() const { return 1; }


// Enable the window
void FXToolbarTab::enable(){
  if(!(flags&FLAG_ENABLED)){
    FXFrame::enable();
    update();
    }
  }


// Disable the window
void FXToolbarTab::disable(){
  if(flags&FLAG_ENABLED){
    FXFrame::disable();
    update();
    }
  }


// Get default width
FXint FXToolbarTab::getDefaultWidth(){
  FXWindow *sibling=getNext() ? getNext() : getPrev();
  FXint w;
  if(options&TOOLBARTAB_VERTICAL){          // Vertical
    if(collapsed){
      w=TOOLBARTAB_WIDTH;
      }
    else{
      w=TOOLBARTAB_HEIGHT;
      if(sibling) w=sibling->getDefaultWidth();
      }
    }
  else{                                     // Horizontal
    if(collapsed){
      w=TOOLBARTAB_HEIGHT;
      if(sibling) w=sibling->getDefaultHeight();
      }
    else{
      w=TOOLBARTAB_WIDTH;
      }
    }
  return w;
  }


// Get default height
FXint FXToolbarTab::getDefaultHeight(){
  FXWindow *sibling=getNext() ? getNext() : getPrev();
  FXint h;
  if(options&TOOLBARTAB_VERTICAL){          // Vertical
    if(collapsed){
      h=TOOLBARTAB_HEIGHT;
      if(sibling) h=sibling->getDefaultWidth();
      }
    else{
      h=TOOLBARTAB_WIDTH;
      }
    }
  else{                                     // Horizontal
    if(collapsed){
      h=TOOLBARTAB_WIDTH;
      }
    else{
      h=TOOLBARTAB_HEIGHT;
      if(sibling) h=sibling->getDefaultHeight();
      }
    }
  return h;
  }


// Collapse or uncollapse
void FXToolbarTab::collapse(FXbool c){
  FXWindow *sibling;
  if(c!=collapsed){
    sibling=getNext() ? getNext() : getPrev();
    if(sibling){
      if(c){
        sibling->hide();
        }
      else{
        sibling->show();
        }
      }
    collapsed=c;
    recalc();
    update();
    }
  }


// Update
long FXToolbarTab::onUpdate(FXObject* sender,FXSelector sel,void* ptr){
  FXWindow *sibling=getNext() ? getNext() : getPrev();
  FXFrame::onUpdate(sender,sel,ptr);
  if(sibling){
    if(sibling->shown() && collapsed){
      collapsed=FALSE;
      update();
      recalc();
      }
    else if(!sibling->shown() && !collapsed){
      collapsed=TRUE;
      update();
      recalc();
      }
    }
  return 1;
  }


// Entered button
long FXToolbarTab::onEnter(FXObject* sender,FXSelector sel,void* ptr){
  FXFrame::onEnter(sender,sel,ptr);
  if(isEnabled()){
    if(flags&FLAG_PRESSED) down=TRUE;
    update();
    }
  return 1;
  }


// Leave button
long FXToolbarTab::onLeave(FXObject* sender,FXSelector sel,void* ptr){
  FXFrame::onLeave(sender,sel,ptr);
  if(isEnabled()){
    if(flags&FLAG_PRESSED) down=FALSE;
    update();
    }
  return 1;
  }


// Pressed mouse button
long FXToolbarTab::onLeftBtnPress(FXObject* sender,FXSelector sel,void* ptr){
  if(!FXFrame::onLeftBtnPress(sender,sel,ptr)){
    if(isEnabled() && !(flags&FLAG_PRESSED)){
      flags|=FLAG_PRESSED;
      flags&=~FLAG_UPDATE;
      down=TRUE;
      update();
      return 1;
      }
    }
  return 0;
  }


// Released mouse button
long FXToolbarTab::onLeftBtnRelease(FXObject* sender,FXSelector sel,void* ptr){
  FXbool click=down;
  if(!FXFrame::onLeftBtnRelease(sender,sel,ptr)){
    if(isEnabled() && (flags&FLAG_PRESSED)){
      flags|=FLAG_UPDATE;
      flags&=~FLAG_PRESSED;
      down=FALSE;
      update();
      if(click){
        collapse(!collapsed);
        if(target) target->handle(this,MKUINT(message,SEL_COMMAND),(void*)(long)collapsed);
        }
      return 1;
      }
    }
  return 0;
  }


// The widget lost the grab for some reason
long FXToolbarTab::onUngrabbed(FXObject* sender,FXSelector sel,void* ptr){
  FXFrame::onUngrabbed(sender,sel,ptr);
  flags&=~FLAG_PRESSED;
  flags|=FLAG_UPDATE;
  down=FALSE;
  update();
  return 1;
  }


// Key Press
long FXToolbarTab::onKeyPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  flags&=~FLAG_TIP;
  if(isEnabled() && !(flags&FLAG_PRESSED)){
    if(target && target->handle(this,MKUINT(message,SEL_KEYPRESS),ptr)) return 1;
    if(event->code==KEY_space || event->code==KEY_KP_Space){
      down=TRUE;
      update();
      flags|=FLAG_PRESSED;
      flags&=~FLAG_UPDATE;
      return 1;
      }
    }
  return 0;
  }


// Key Release
long FXToolbarTab::onKeyRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  if(isEnabled() && (flags&FLAG_PRESSED)){
    if(target && target->handle(this,MKUINT(message,SEL_KEYRELEASE),ptr)) return 1;
    if(event->code==KEY_space || event->code==KEY_KP_Space){
      down=FALSE;
      update();
      flags|=FLAG_UPDATE;
      flags&=~FLAG_PRESSED;
      collapse(!collapsed);
      if(target) target->handle(this,MKUINT(message,SEL_COMMAND),(void*)(long)collapsed);
      return 1;
      }
    }
  return 0;
  }


// Collapse
long FXToolbarTab::onCmdCollapse(FXObject*,FXSelector,void*){
  collapse(TRUE);
  return 1;
  }


// Update collapse
long FXToolbarTab::onUpdCollapse(FXObject* sender,FXSelector,void*){
  if(collapsed)
    sender->handle(this,MKUINT(ID_CHECK,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_UNCHECK,SEL_COMMAND),NULL);
  return 1;
  }


// Uncollapse
long FXToolbarTab::onCmdUncollapse(FXObject*,FXSelector,void*){
  collapse(FALSE);
  return 1;
  }


// Update uncollapse
long FXToolbarTab::onUpdUncollapse(FXObject* sender,FXSelector,void*){
  if(!collapsed)
    sender->handle(this,MKUINT(ID_CHECK,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_UNCHECK,SEL_COMMAND),NULL);
  return 1;
  }


// Draw horizontal speckles
void FXToolbarTab::drawHSpeckles(FXDCWindow& dc,FXint x,FXint w){
  register FXint i;
  dc.setForeground(hiliteColor);
  for(i=0; i<w-5; i+=4){dc.drawPoint(x+i,2);dc.drawPoint(x+i+1,5);}
  dc.setForeground(shadowColor);
  for(i=0; i<w-5; i+=4){dc.drawPoint(x+i+1,3);dc.drawPoint(x+i+2,6);}
  }


// Draw vertical speckles
void FXToolbarTab::drawVSpeckles(FXDCWindow& dc,FXint y,FXint h){
  register FXint i;
  dc.setForeground(hiliteColor);
  for(i=0; i<h-5; i+=3){dc.drawPoint(2,y+i+1);dc.drawPoint(5,y+i);}
  dc.setForeground(shadowColor);
  for(i=0; i<h-5; i+=3){dc.drawPoint(6,y+i+1);dc.drawPoint(3,y+i+2);}
  }


// Draw up arrow
void FXToolbarTab::drawUpArrow(FXDCWindow& dc){
  dc.setForeground(borderColor);
  dc.drawLine(2,height-5,6,height-5);
  dc.drawPoint(3,height-6);
  dc.drawPoint(4,height-7);
  dc.drawPoint(5,height-6);
  dc.drawPoint(4,height-6);
  }


// Draw down arrow
void FXToolbarTab::drawDownArrow(FXDCWindow& dc){
  dc.setForeground(borderColor);
  dc.drawLine(2,4,6,4);
  dc.drawPoint(3,5);
  dc.drawPoint(4,6);
  dc.drawPoint(5,5);
  dc.drawPoint(4,5);
  }


// Draw left arrow
void FXToolbarTab::drawLeftArrow(FXDCWindow& dc){
  dc.setForeground(borderColor);
  dc.drawLine(width-5,2,width-5,6);
  dc.drawPoint(width-6,3);
  dc.drawPoint(width-7,4);
  dc.drawPoint(width-6,5);
  dc.drawPoint(width-6,4);
  }


// Draw right arrow
void FXToolbarTab::drawRightArrow(FXDCWindow& dc){
  dc.setForeground(borderColor);
  dc.drawLine(4,2,4,6);
  dc.drawPoint(5,3);
  dc.drawPoint(6,4);
  dc.drawPoint(5,5);
  dc.drawPoint(5,4);
  }


// Handle repaint
long FXToolbarTab::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent *ev=(FXEvent*)ptr;
  FXDCWindow dc(this,ev);

  // Got a border at all?
  if(options&(FRAME_RAISED|FRAME_SUNKEN)){

    // Draw sunken if enabled and either checked or pressed
    if(isEnabled() && down){
      if(down) dc.setForeground(hiliteColor); else dc.setForeground(backColor);
      dc.fillRectangle(border,border,width-border*2,height-border*2);
      if(options&FRAME_THICK) drawDoubleSunkenRectangle(dc,0,0,width,height);
      else drawSunkenRectangle(dc,0,0,width,height);
      }

    // Draw in up state if disabled or up
    else{
      if(underCursor())
        dc.setForeground(activeColor);
      else
        dc.setForeground(backColor);
      dc.fillRectangle(border,border,width-border*2,height-border*2);
      if(options&FRAME_THICK) drawDoubleRaisedRectangle(dc,0,0,width,height);
      else drawRaisedRectangle(dc,0,0,width,height);
      }
    }

  // No borders
  else{
    if(isEnabled() && down){
      dc.setForeground(hiliteColor);
      dc.fillRectangle(0,0,width,height);
      }
    else{
      if(underCursor())
        dc.setForeground(activeColor);
      else
        dc.setForeground(backColor);
      dc.fillRectangle(0,0,width,height);
      }
    }

  // Draw spickles
  if(options&TOOLBARTAB_VERTICAL){          // Vertical
    if(collapsed){
      if(options&LAYOUT_BOTTOM){
        drawVSpeckles(dc,3,height-10);
        drawUpArrow(dc);
        }
      else{
        drawVSpeckles(dc,10,height-10);
        drawDownArrow(dc);
        }
      }
    else{
      if(options&LAYOUT_RIGHT){
        drawHSpeckles(dc,3,width-10);
        drawLeftArrow(dc);
        }
      else{
        drawHSpeckles(dc,10,width-10);
        drawRightArrow(dc);
        }
      }
    }
  else{                                     // Horizontal
    if(collapsed){
      if(options&LAYOUT_RIGHT){
        drawHSpeckles(dc,3,width-10);
        drawLeftArrow(dc);
        }
      else{
        drawHSpeckles(dc,10,width-10);
        drawRightArrow(dc);
        }
      }
    else{
      if(options&LAYOUT_BOTTOM){
        drawVSpeckles(dc,3,height-10);
        drawUpArrow(dc);
        }
      else{
        drawVSpeckles(dc,10,height-10);
        drawDownArrow(dc);
        }
      }
    }
  return 1;
  }


// Change tab style
void FXToolbarTab::setTabStyle(FXuint style){
  FXuint opts=(options&~TOOLBARTAB_MASK) | (style&TOOLBARTAB_MASK);
  if(options!=opts){
    options=opts;
    update();
    }
  }


// Get tab style
FXuint FXToolbarTab::getTabStyle() const {
  return (options&TOOLBARTAB_MASK);
  }


// Set text color
void FXToolbarTab::setActiveColor(FXColor clr){
  if(clr!=activeColor){
    activeColor=clr;
    update();
    }
  }


// Save object to stream
void FXToolbarTab::save(FXStream& store) const {
  FXFrame::save(store);
  store << activeColor;
  }



// Load object from stream
void FXToolbarTab::load(FXStream& store){
  FXFrame::load(store);
  store >> activeColor;
  }
