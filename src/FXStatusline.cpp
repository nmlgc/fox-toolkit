/********************************************************************************
*                                                                               *
*                       S t a t u s l i n e   W i d g e t                       *
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
* $Id: FXStatusline.cpp,v 1.14 2002/01/18 22:43:04 jeroen Exp $                 *
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
#include "FXFont.h"
#include "FXHorizontalFrame.h"
#include "FXDragCorner.h"
#include "FXStatusline.h"



/*
  Notes:
  - Fallback text is displayed when neither cursor window or
    target object supply temporary help string.
*/


/*******************************************************************************/

// Map
FXDEFMAP(FXStatusline) FXStatuslineMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXStatusline::onPaint),
  FXMAPFUNC(SEL_UPDATE,0,FXStatusline::onUpdate),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETSTRINGVALUE,FXStatusline::onCmdSetStringValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETSTRINGVALUE,FXStatusline::onCmdGetStringValue),
  };


// Default message
const FXchar FXStatusline::defaultMessage[]="Ready.";


// Object implementation
FXIMPLEMENT(FXStatusline,FXFrame,FXStatuslineMap,ARRAYNUMBER(FXStatuslineMap))


// Deserialization
FXStatusline::FXStatusline(){
  flags|=FLAG_SHOWN;
  }


// Construct and init
FXStatusline::FXStatusline(FXComposite* p,FXObject* tgt,FXSelector sel):
  FXFrame(p,FRAME_SUNKEN|LAYOUT_LEFT|LAYOUT_FILL_Y|LAYOUT_FILL_X,0,0,0,0, 4,4,2,2){
  flags|=FLAG_SHOWN;
  status=defaultMessage;
  normal=defaultMessage;
  font=getApp()->getNormalFont();
  textColor=getApp()->getForeColor();
  textHighlightColor=getApp()->getForeColor();
  target=tgt;
  message=sel;
  }


// Create Window
void FXStatusline::create(){
  FXFrame::create();
  font->create();
  }


// Detach Window
void FXStatusline::detach(){
  FXFrame::detach();
  font->detach();
  }


// Get default width; as text changes often, exact content does not matter
FXint FXStatusline::getDefaultWidth(){
  return padleft+padright+(border<<1)+8;
  }


// Get default height; just care about font height
FXint FXStatusline::getDefaultHeight(){
  return font->getFontHeight()+padtop+padbottom+(border<<1);
  }


// Slightly different from Frame border
long FXStatusline::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent *ev=(FXEvent*)ptr;
  FXDCWindow dc(this,ev);
  FXint ty=padtop+(height-padtop-padbottom-font->getFontHeight())/2;
  FXint pos,len;
  dc.setForeground(backColor);
  dc.fillRectangle(ev->rect.x,ev->rect.y,ev->rect.w,ev->rect.h);
  if(!status.empty()){
    dc.setTextFont(font);
    pos=status.findf('\n');
    len=status.length();
    if(pos>=0){
      dc.setForeground(textHighlightColor);
      dc.drawText(padleft,ty+font->getFontAscent(),status.text(),pos);
      dc.setForeground(textColor);
      dc.drawText(padleft+font->getTextWidth(status.text(),pos),ty+font->getFontAscent(),status.text()+pos+1,len-pos-1);
      }
    else{
      dc.setForeground(textColor);
      dc.drawText(padleft,ty+font->getFontAscent(),status.text(),len);
      }
    }
  drawFrame(dc,0,0,width,height);
  return 1;
  }


// If the cursor is inside a widget, flash its help text;
// Otherwise, unflash back to the regular status message.
long FXStatusline::onUpdate(FXObject* sender,FXSelector sel,void* ptr){
  FXWindow *helpsource;

  // Regular GUI update
  FXFrame::onUpdate(sender,sel,ptr);

  // Ask the help source for a new status text first
  helpsource=getApp()->getCursorWindow();
  if(helpsource){
    if(helpsource->handle(this,MKUINT(FXWindow::ID_QUERY_HELP,SEL_UPDATE),NULL)){
      return 1;
      }
    }

  // Ask target; this should be the normal help text
  // indicating the state the program is in currently.
  if(target){
    if(target->handle(this,MKUINT(message,SEL_UPDATE),NULL)){
      return 1;
      }
    }

  // Otherwise, supply normal message
  setText(normal);
  return 1;
  }


// Update value from a message
long FXStatusline::onCmdSetStringValue(FXObject*,FXSelector,void* ptr){
  if(ptr==NULL){ fxerror("%s::onCmdSetStringValue: NULL pointer.\n",getClassName()); }
  setText(*((FXString*)ptr));
  return 1;
  }


// Obtain value from text field
long FXStatusline::onCmdGetStringValue(FXObject*,FXSelector,void* ptr){
  if(ptr==NULL){ fxerror("%s::onCmdGetStringValue: NULL pointer.\n",getClassName()); }
  *((FXString*)ptr)=getText();
  return 1;
  }


// Set currently displayed message
void FXStatusline::setText(const FXString& text){
  if(status!=text){
    status=text;
    update(border,border,width-(border<<1),height-(border<<1));
    repaint(border,border,width-(border<<1),height-(border<<1));
    getApp()->flush();
    }
  }


// Set permanently displayed message
void FXStatusline::setNormalText(const FXString& text){
  if(normal!=text){
    normal=text;
    update(border,border,width-(border<<1),height-(border<<1));
    repaint(border,border,width-(border<<1),height-(border<<1));
    getApp()->flush();
    }
  }


// Change the font
void FXStatusline::setFont(FXFont* fnt){
  if(!fnt){ fxerror("%s::setFont: NULL font specified.\n",getClassName()); }
  if(font!=fnt){
    font=fnt;
    recalc();
    update();
    }
  }


// Set text color
void FXStatusline::setTextColor(FXColor clr){
  textColor=clr;
  update(border,border,width-(border<<1),height-(border<<1));
  }


// Set text highlight color
void FXStatusline::setTextHighlightColor(FXColor clr){
  textHighlightColor=clr;
  update(border,border,width-(border<<1),height-(border<<1));
  }


// Save object to stream
void FXStatusline::save(FXStream& store) const {
  FXFrame::save(store);
  store << status;
  store << normal;
  store << font;
  store << textColor;
  store << textHighlightColor;
  }


// Load object from stream
void FXStatusline::load(FXStream& store){
  FXFrame::load(store);
  store >> status;
  store >> normal;
  store >> font;
  store >> textColor;
  store >> textHighlightColor;
  }


// Destruct
FXStatusline::~FXStatusline(){
  font=(FXFont*)-1;
  }

