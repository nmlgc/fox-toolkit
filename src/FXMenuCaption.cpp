/********************************************************************************
*                                                                               *
*                       M e n u   C a p t i o n   W i d g e t                   *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXMenuCaption.cpp,v 1.25 2002/01/18 22:43:01 jeroen Exp $                *
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
#include "FXIcon.h"
#include "FXMenuCaption.h"

/*
  Notes:
  - Accelerators.
  - Help text from constructor is third part; second part should be
    accelerator key combination.
  - When menu label changes, hotkey might have to be adjusted.
  - Fix it so menu stays up when after Alt-F, you press Alt-E.
  - Menu items should be derived from FXLabel.
  - FXMenuCascade should send ID_POST/ID_UNPOST to self.
  - Look into SEL_FOCUS_SELF some more:- menus should not
    get focus, or at least, return the focus to the original
    widget.
  - Want to support arbitrary large icons.
*/


#define LEADSPACE   22
#define TRAILSPACE  16

/*******************************************************************************/


// Map
FXDEFMAP(FXMenuCaption) FXMenuCaptionMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXMenuCaption::onPaint),
  FXMAPFUNC(SEL_UPDATE,0,FXMenuCaption::onUpdate),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_HELP,FXMenuCaption::onQueryHelp),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETSTRINGVALUE,FXMenuCaption::onCmdSetStringValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETSTRINGVALUE,FXMenuCaption::onCmdGetStringValue),
  };


// Object implementation
FXIMPLEMENT(FXMenuCaption,FXWindow,FXMenuCaptionMap,ARRAYNUMBER(FXMenuCaptionMap))


// Deserialization
FXMenuCaption::FXMenuCaption(){
  flags|=FLAG_SHOWN;
  }


// Menu entry
FXMenuCaption::FXMenuCaption(FXComposite* p,const FXString& text,FXIcon* ic,FXuint opts):
  FXWindow(p,opts,0,0,0,0){
  flags|=FLAG_SHOWN;
  label=text.extract(0,'\t','&');
  help=text.extract(2,'\t');
  icon=ic;
  font=getApp()->getNormalFont();
  hotkey=fxparsehotkey(text.text());
  hotoff=fxfindhotkeyoffset(text.text());
  addHotKey(hotkey);
  textColor=getApp()->getForeColor();
  seltextColor=getApp()->getSelforeColor();
  selbackColor=getApp()->getSelbackColor();
  hiliteColor=getApp()->getHiliteColor();
  shadowColor=getApp()->getShadowColor();
  }


// Create Window
void FXMenuCaption::create(){
  FXWindow::create();
  font->create();
  if(icon) icon->create();
  }


// Detach Window
void FXMenuCaption::detach(){
  FXWindow::detach();
  font->detach();
  if(icon) icon->detach();
  }


// Enable the menu entry
void FXMenuCaption::enable(){
  if(!(flags&FLAG_ENABLED)){
    FXWindow::enable();
    update();
    }
  }


// Disable the menu entry
void FXMenuCaption::disable(){
  if(flags&FLAG_ENABLED){
    FXWindow::disable();
    update();
    }
  }


// Get default width
FXint FXMenuCaption::getDefaultWidth(){
  FXint tw,iw;
  tw=iw=0;
  if(!label.empty()) tw=font->getTextWidth(label.text(),label.length());
  if(icon) iw=icon->getWidth()+5;
  return FXMAX(iw,LEADSPACE)+tw+TRAILSPACE;
  }


// Get default height
FXint FXMenuCaption::getDefaultHeight(){
  FXint th,ih,h;
  th=ih=h=0;
  if(!label.empty()) th=font->getFontHeight()+5;
  if(icon) ih=icon->getHeight()+5;
  return FXMAX(th,ih);
  }


// We were asked about status text
long FXMenuCaption::onQueryHelp(FXObject* sender,FXSelector,void*){
  if(!help.empty() && (flags&FLAG_HELP)){
    sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&help);
    return 1;
    }
  return 0;
  }


// Implement auto-hide or auto-gray modes
long FXMenuCaption::onUpdate(FXObject* sender,FXSelector sel,void* ptr){
  if(!FXWindow::onUpdate(sender,sel,ptr)){
    if(options&MENU_AUTOHIDE){if(shown()){hide();recalc();}}
    if(options&MENU_AUTOGRAY){disable();}
    }
  return 1;
  }


// Handle repaint
long FXMenuCaption::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent *ev=(FXEvent*)ptr;
  FXDCWindow dc(this,ev);
  FXint xx,yy;
  dc.setForeground(backColor);
  dc.fillRectangle(0,0,width,height);
  xx=LEADSPACE;
  if(icon){
    dc.drawIcon(icon,3,(height-icon->getHeight())/2);
    if(icon->getWidth()+5>xx) xx=icon->getWidth()+5;
    }
  if(!label.empty()){
    dc.setTextFont(font);
    dc.setForeground(textColor);
    yy=font->getFontAscent()+(height-font->getFontHeight())/2;
    dc.drawText(xx,yy,label.text(),label.length());
    if(0<=hotoff){
      dc.fillRectangle(xx+1+font->getTextWidth(&label[0],hotoff),yy+1,font->getTextWidth(&label[hotoff],1),1);
      }
    }
  return 1;
  }


// Update value from a message
long FXMenuCaption::onCmdSetStringValue(FXObject*,FXSelector,void* ptr){
  if(ptr==NULL){ fxerror("%s::onCmdSetStringValue: NULL pointer.\n",getClassName()); }
  setText(*((FXString*)ptr));
  return 1;
  }


// Obtain value from text field
long FXMenuCaption::onCmdGetStringValue(FXObject*,FXSelector,void* ptr){
  if(ptr==NULL){ fxerror("%s::onCmdGetStringValue: NULL pointer.\n",getClassName()); }
  *((FXString*)ptr)=getText();
  return 1;
  }


// Change help text
void FXMenuCaption::setHelpText(const FXString& text){
  help=text;
  }


// Change text, and scan this text to replace accelerators
void FXMenuCaption::setText(const FXString& text){
  FXString str=text.extract(0,'\t','&');
  if(label!=str){
    remHotKey(hotkey);
    hotkey=fxparsehotkey(text.text());
    hotoff=fxfindhotkeyoffset(text.text());
    addHotKey(hotkey);
    label=str;
    recalc();
    update();
    }
  }


// Change icon
void FXMenuCaption::setIcon(FXIcon* ic){
  if(icon!=ic){
    icon=ic;
    recalc();
    update();
    }
  }


// Change font
void FXMenuCaption::setFont(FXFont* fnt){
  if(!fnt){ fxerror("%s::setFont: NULL font specified.\n",getClassName()); }
  if(font!=fnt){
    font=fnt;
    recalc();
    update();
    }
  }


// Set text color
void FXMenuCaption::setTextColor(FXColor clr){
  if(clr!=textColor){
    textColor=clr;
    update();
    }
  }


// Set select background color
void FXMenuCaption::setSelBackColor(FXColor clr){
  if(clr!=selbackColor){
    selbackColor=clr;
    update();
    }
  }


// Set selected text color
void FXMenuCaption::setSelTextColor(FXColor clr){
  if(clr!=seltextColor){
    seltextColor=clr;
    update();
    }
  }


// Set highlight color
void FXMenuCaption::setHiliteColor(FXColor clr){
  if(clr!=hiliteColor){
    hiliteColor=clr;
    update();
    }
  }


// Set shadow color
void FXMenuCaption::setShadowColor(FXColor clr){
  if(clr!=shadowColor){
    shadowColor=clr;
    update();
    }
  }



// Save object to stream
void FXMenuCaption::save(FXStream& store) const {
  FXWindow::save(store);
  store << label;
  store << help;
  store << icon;
  store << font;
  store << hotoff;
  store << hotkey;
  store << textColor;
  store << selbackColor;
  store << seltextColor;
  store << hiliteColor;
  store << shadowColor;
  }


// Load object from stream
void FXMenuCaption::load(FXStream& store){
  FXWindow::load(store);
  store >> label;
  store >> help;
  store >> icon;
  store >> font;
  store >> hotoff;
  store >> hotkey;
  store >> textColor;
  store >> selbackColor;
  store >> seltextColor;
  store >> hiliteColor;
  store >> shadowColor;
  }


// Zap it
FXMenuCaption::~FXMenuCaption(){
  remHotKey(hotkey);
  font=(FXFont*)-1;
  icon=(FXIcon*)-1;
  }


