/********************************************************************************
*                                                                               *
*                             L a b e l   O b j e c t s                         *
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
* $Id: FXLabel.cpp,v 1.29 1998/10/27 04:57:43 jeroen Exp $                      *
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
#include "FXFont.h"
#include "FXDrawable.h"
#include "FXImage.h"
#include "FXIcon.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXComposite.h"
#include "FXLabel.h"


#define SEPARATOR_EXTRA 2

/*
  Notes:
  - When changing icon/font/etc, we should only recalc and update when it's different.
  - When text changes, do we delete the hot key, or parse it from the new label?
  - It makes sense for certain ``passive'' widgets such as labels to have onUpdate;
    for example, to show/hide/whatever based on changing data structures.
*/


/*******************************************************************************/

// Map
FXDEFMAP(FXHorizontalSeparator) FXHorizontalSeparatorMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXHorizontalSeparator::onPaint),
  };


// Object implementation
FXIMPLEMENT(FXHorizontalSeparator,FXFrame,FXHorizontalSeparatorMap,ARRAYNUMBER(FXHorizontalSeparatorMap))


// Construct and init
FXHorizontalSeparator::FXHorizontalSeparator(FXComposite* p,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXFrame(p,opts,x,y,w,h){
  }


// Create window
void FXHorizontalSeparator::create(){
  FXFrame::create();
  show();
  }


// Get default size
FXint FXHorizontalSeparator::getDefaultWidth(){ 
  return 1+(SEPARATOR_EXTRA<<1); 
  }


FXint FXHorizontalSeparator::getDefaultHeight(){ 
  return options&(SEPARATOR_GROOVE|SEPARATOR_RIDGE) ? 2 : 1; 
  }


// Handle repaint 
long FXHorizontalSeparator::onPaint(FXObject* sender,FXSelector sel,void* ptr){
  FXint yy=(height-1)/2;
  FXFrame::onPaint(sender,sel,ptr);
  if(options&SEPARATOR_GROOVE){
    setForeground(shadowColor);
    drawLine(SEPARATOR_EXTRA,yy,width-SEPARATOR_EXTRA-1,yy);
    setForeground(hiliteColor);
    drawLine(SEPARATOR_EXTRA,yy+1,width-SEPARATOR_EXTRA-1,yy+1);
    }
  else if(options&SEPARATOR_RIDGE){
    setForeground(hiliteColor);
    drawLine(SEPARATOR_EXTRA,yy,width-SEPARATOR_EXTRA-1,yy);
    setForeground(shadowColor);
    drawLine(SEPARATOR_EXTRA,yy+1,width-SEPARATOR_EXTRA-1,yy+1);
    }
  else if(options&SEPARATOR_LINE){
    setForeground(borderColor);
    drawLine(SEPARATOR_EXTRA,yy,width-SEPARATOR_EXTRA-1,yy);
    }
  return 1;
  }

  
/*******************************************************************************/


// Map
FXDEFMAP(FXVerticalSeparator) FXVerticalSeparatorMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXVerticalSeparator::onPaint),
  };


// Object implementation
FXIMPLEMENT(FXVerticalSeparator,FXFrame,FXVerticalSeparatorMap,ARRAYNUMBER(FXVerticalSeparatorMap))


// Construct and init
FXVerticalSeparator::FXVerticalSeparator(FXComposite* p,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXFrame(p,opts,x,y,w,h){
  }


// Create window
void FXVerticalSeparator::create(){
  FXFrame::create();
  show();
  }


// Get default size
FXint FXVerticalSeparator::getDefaultWidth(){ 
  return options&(SEPARATOR_GROOVE|SEPARATOR_RIDGE) ? 2 : 1; 
  }


FXint FXVerticalSeparator::getDefaultHeight(){ 
  return 1+(SEPARATOR_EXTRA<<1); 
  }


// Handle repaint 
long FXVerticalSeparator::onPaint(FXObject* sender,FXSelector sel,void* ptr){
  FXint xx=(width-1)/2;
  FXFrame::onPaint(sender,sel,ptr);
  if(options&SEPARATOR_GROOVE){
    setForeground(shadowColor);
    drawLine(xx,SEPARATOR_EXTRA,xx,height-SEPARATOR_EXTRA-1);
    setForeground(hiliteColor);
    drawLine(xx+1,SEPARATOR_EXTRA,xx+1,height-SEPARATOR_EXTRA-1);
    }
  else if(options&SEPARATOR_RIDGE){
    setForeground(hiliteColor);
    drawLine(xx,SEPARATOR_EXTRA,xx,height-SEPARATOR_EXTRA-1);
    setForeground(shadowColor);
    drawLine(xx+1,SEPARATOR_EXTRA,xx+1,height-SEPARATOR_EXTRA-1);
    }
  else if(options&SEPARATOR_LINE){
    setForeground(borderColor);
    drawLine(xx,SEPARATOR_EXTRA,xx,height-SEPARATOR_EXTRA-1);
    }
  return 1;
  }



/*******************************************************************************/


// Object implementation
FXIMPLEMENT(FXCell,FXFrame,NULL,0)

  
// Deserialization
FXCell::FXCell(){
  padtop=0;
  padbottom=0;
  padleft=0;
  padright=0;
  }


// Make a label
FXCell::FXCell(FXComposite* p,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXFrame(p,opts,x,y,w,h){
  padtop=pt;
  padbottom=pb;
  padleft=pl;
  padright=pr;
  }


// Create X window
void FXCell::create(){
  FXFrame::create();
  show();
  }


// Get default size
FXint FXCell::getDefaultWidth(){
  return 1+padleft+padright+(border<<1);
  }


FXint FXCell::getDefaultHeight(){
  return 1+padtop+padbottom+(border<<1);
  }


// Justify stuff in x-direction
void FXCell::just_x(FXint& tx,FXint& ix,FXint tw,FXint iw){
  FXint s=0;
  if(iw && tw) s=4;
  if((options&JUSTIFY_LEFT) && (options&JUSTIFY_RIGHT)){
    if(options&ICON_BEFORE_TEXT){ ix=padleft+border; tx=width-padright-border-tw; }
    else if(options&ICON_AFTER_TEXT){ tx=padleft+border; ix=width-padright-border-iw; }
    else{ ix=border+padleft; tx=border+padleft; }
    }
  else if(options&JUSTIFY_LEFT){
    if(options&ICON_BEFORE_TEXT){ ix=padleft+border; tx=ix+iw+s; }
    else if(options&ICON_AFTER_TEXT){ tx=padleft+border; ix=tx+tw+s; }
    else{ ix=border+padleft; tx=border+padleft; }
    }
  else if(options&JUSTIFY_RIGHT){
    if(options&ICON_BEFORE_TEXT){ tx=width-padright-border-tw; ix=tx-iw-s; }
    else if(options&ICON_AFTER_TEXT){ ix=width-padright-border-iw; tx=ix-tw-s; }
    else{ ix=width-padright-border-iw; tx=width-padright-border-tw; }
    }
  else{
    if(options&ICON_BEFORE_TEXT){ ix=border+padleft+(width-padleft-padright-(border<<1)-tw-iw-s)/2; tx=ix+iw+s; }
    else if(options&ICON_AFTER_TEXT){ tx=border+padleft+(width-padleft-padright-(border<<1)-tw-iw-s)/2; ix=tx+tw+s; }
    else{ ix=border+padleft+(width-padleft-padright-(border<<1)-iw)/2; tx=border+padleft+(width-padleft-padright-(border<<1)-tw)/2; }
    }
  }


// Justify stuff in y-direction
void FXCell::just_y(FXint& ty,FXint& iy,FXint th,FXint ih){
  if((options&JUSTIFY_TOP) && (options&JUSTIFY_BOTTOM)){
    if(options&ICON_ABOVE_TEXT){ iy=padtop+border; ty=height-padbottom-border-th; }
    else if(options&ICON_BELOW_TEXT){ ty=padtop+border; iy=height-padbottom-border-ih; }
    else{ iy=border+padtop; ty=border+padtop; }
    }
  else if(options&JUSTIFY_TOP){
    if(options&ICON_ABOVE_TEXT){ iy=padtop+border; ty=iy+ih; }
    else if(options&ICON_BELOW_TEXT){ ty=padtop+border; iy=ty+th; }
    else{ iy=border+padtop; ty=border+padtop; }
    }
  else if(options&JUSTIFY_BOTTOM){
    if(options&ICON_ABOVE_TEXT){ ty=height-padbottom-border-th; iy=ty-ih; }
    else if(options&ICON_BELOW_TEXT){ iy=height-padbottom-border-ih; ty=iy-th; }
    else{ iy=height-padbottom-border-ih; ty=height-padbottom-border-th; }
    }
  else{
    if(options&ICON_ABOVE_TEXT){ iy=border+padtop+(height-padbottom-padtop-(border<<1)-th-ih)/2; ty=iy+ih; }
    else if(options&ICON_BELOW_TEXT){ ty=border+padtop+(height-padbottom-padtop-(border<<1)-th-ih)/2; iy=ty+th; }
    else{ iy=border+padtop+(height-padbottom-padtop-(border<<1)-ih)/2; ty=border+padtop+(height-padbottom-padtop-(border<<1)-th)/2; }
    }
  }


// Change top padding
void FXCell::setPadTop(FXint pt){
  if(padtop!=pt){
    padtop=pt;
    recalc();
    update(0,0,width,height);
    }
  }


// Change bottom padding
void FXCell::setPadBottom(FXint pb){
  if(padbottom!=pb){
    padbottom=pb;
    recalc();
    update(0,0,width,height);
    }
  }


// Change left padding
void FXCell::setPadLeft(FXint pl){
  if(padleft!=pl){
    padleft=pl;
    recalc();
    update(0,0,width,height);
    }
  }


// Change right padding
void FXCell::setPadRight(FXint pr){
  if(padright!=pr){
    padright=pr;
    recalc();
    update(0,0,width,height);
    }
  }

// Save object to stream
void FXCell::save(FXStream& store) const {
  FXFrame::save(store);
  store << padtop << padbottom << padleft << padright;
  }


// Load object from stream
void FXCell::load(FXStream& store){
  FXFrame::load(store);
  store >> padtop >> padbottom >> padleft >> padright;
  }  


/*******************************************************************************/

// Map
FXDEFMAP(FXLabel) FXLabelMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXLabel::onPaint),
  FXMAPFUNC(SEL_KEYPRESS,FXWindow::ID_HOTKEY,FXLabel::onHotKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,FXWindow::ID_HOTKEY,FXLabel::onHotKeyRelease),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETSTRINGVALUE,FXLabel::onCmdSetStringValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETSTRINGVALUE,FXLabel::onCmdGetStringValue),
  };


// Object implementation
FXIMPLEMENT(FXLabel,FXCell,FXLabelMap,ARRAYNUMBER(FXLabelMap))

  
// Deserialization
FXLabel::FXLabel(){
  icon=(FXIcon*)-1;
  font=(FXFont*)-1;
  hotkey=0;
  hotoff=0;
  textColor=0;
  }


// Make a label
FXLabel::FXLabel(FXComposite* p,const char* text,FXIcon* ic,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXCell(p,opts,x,y,w,h,pl,pr,pt,pb),
  label(text,'\t','&',0),   // Copies text up till first tab, and strips & characters
  icon(ic){
  font=getApp()->normalFont;
  textColor=0;
  hotkey=fxparsehotkey(text);
  hotoff=fxfindhotkeyoffset(text);
  addHotKey(hotkey);
  }


// Create X window
void FXLabel::create(){
  FXCell::create();
  textColor=acquireColor(getApp()->foreColor);
  font->create();
  if(icon) icon->create();
  }


// Get height of multi-line label
FXint FXLabel::labelHeight() const {
  register FXuint beg,end;
  register FXint th=0;
  beg=0;
  do{
    end=beg;
    while(label[end] && label[end]!='\n') end++;
    th+=font->getFontHeight();
    beg=end+1;
    }
  while(label[end]);
  return th;
  }

  
// Get width of multi-line label
FXint FXLabel::labelWidth() const {
  register FXuint beg,end;
  register FXint w,tw=0;
  beg=0;
  do{
    end=beg;
    while(label[end] && label[end]!='\n') end++;
    if((w=font->getTextWidth(&label[beg],end-beg))>tw) tw=w;
    beg=end+1;
    }
  while(label[end]);
  return tw;
  }


// Draw multi-line label, with underline for hotkey
void FXLabel::drawLabel(FXint tx,FXint ty,FXint tw,FXint){
  register FXuint beg,end;
  register FXint xx,yy;
  yy=ty+font->getFontAscent();
  beg=0;
  do{
    end=beg;
    while(label[end] && label[end]!='\n') end++;
    if(options&JUSTIFY_LEFT) xx=tx;
    else if(options&JUSTIFY_RIGHT) xx=tx+tw-font->getTextWidth(&label[beg],end-beg);
    else xx=tx+(tw-font->getTextWidth(&label[beg],end-beg))/2;
    drawText(xx,yy,&label[beg],end-beg);
    if(beg<=hotoff && hotoff<end){
      drawLine(xx+font->getTextWidth(&label[beg],hotoff-beg),yy+1,xx+font->getTextWidth(&label[beg],hotoff-beg+1)-1,yy+1);
      }
    yy+=font->getFontHeight();
    beg=end+1;
    }
  while(label[end]);
  }


// Get default size
FXint FXLabel::getDefaultWidth(){
  FXint tw=0,iw=0,s=0,w;
  if(label.text()){
    tw=labelWidth();
    }
  if(icon){
    iw=icon->getWidth(); 
    }
  if(iw && tw) s=4;
  if(!(options&(ICON_AFTER_TEXT|ICON_BEFORE_TEXT))) w=FXMAX(tw,iw); else w=tw+iw+s;
  return w+padleft+padright+(border<<1);
  }


FXint FXLabel::getDefaultHeight(){
  FXint th=0,ih=0,h;
  if(label.text()){ 
    th=labelHeight();
    }
  if(icon){
    ih=icon->getHeight(); 
    }
  if(!(options&(ICON_ABOVE_TEXT|ICON_BELOW_TEXT))) h=FXMAX(th,ih); else h=th+ih;
  return h+padtop+padbottom+(border<<1);
  }


// Update value from a message
long FXLabel::onCmdSetStringValue(FXObject*,FXSelector,void* ptr){
  if(ptr){ setText(*((FXString*)ptr)); }
  return 1;
  }


// Obtain value from text field
long FXLabel::onCmdGetStringValue(FXObject*,FXSelector,void* ptr){
  if(ptr){ *((FXString*)ptr) = getText(); }
  return 1;
  }


// Handle repaint 
long FXLabel::onPaint(FXObject* sender,FXSelector sel,void* ptr){
  FXint tw=0,th=0,iw=0,ih=0,tx,ty,ix,iy;
  FXCell::onPaint(sender,sel,ptr);
  if(label.text()){
    tw=labelWidth();
    th=labelHeight();
    }
  if(icon){
    iw=icon->getWidth();
    ih=icon->getHeight();
    }
  just_x(tx,ix,tw,iw);
  just_y(ty,iy,th,ih);
  if(icon){
    drawIcon(icon,ix,iy);
    }
  if(label.text()){
    setForeground(textColor);
    setTextFont(font);
    drawLabel(tx,ty,tw,th);
    }
  return 1;
  }


// Move the focus to the next focusable child following the
// Label widget.  Thus, placing a label with accelerator in front
// of e.g. a TextField gives a convenient method for getting to it.
long FXLabel::onHotKeyPress(FXObject*,FXSelector,void*){
  FXWindow *window=getNext();
  while(window){
    if(window->isEnabled() && window->canFocus()){
      window->setFocus();
      return 1;
      }
    window=window->getNext();
    }
  return 1;
  }


// Nothing much happens here...
long FXLabel::onHotKeyRelease(FXObject*,FXSelector,void*){
  return 1;
  }
  

// Change text
void FXLabel::setText(const FXchar* text){
  if(label!=text){
    label=FXString(text,'\t','&',0);/// Don't like this...
    remHotKey(hotkey);
    hotkey=fxparsehotkey(text);
    hotoff=fxfindhotkeyoffset(text);
    addHotKey(hotkey);
    recalc();
    update(0,0,width,height);
    }
  }


// Change icon
void FXLabel::setIcon(FXIcon* ic){
  if(icon!=ic){
    icon=ic;
    recalc();
    update(0,0,width,height);
    }
  }


// Change the font
void FXLabel::setFont(FXFont *fnt){
  if(!fnt){ fxerror("%s::setFont: NULL font specified.\n",getClassName()); }
  if(font!=fnt){
    font=fnt;
    recalc();
    update(0,0,width,height);
    }
  }


// Set text color
void FXLabel::setTextColor(FXPixel clr){
  textColor=clr;
  update(0,0,width,height);
  }


// Save object to stream
void FXLabel::save(FXStream& store) const {
  FXCell::save(store);
  store << label;
  store << icon;
  store << font;
  store << hotkey;
  store << hotoff;
  }


// Load object from stream
void FXLabel::load(FXStream& store){
  FXCell::load(store);
  store >> label;
  store >> icon;
  store >> font;
  store >> hotkey;
  store >> hotoff;
  }  


// Destroy label
FXLabel::~FXLabel(){
  remHotKey(hotkey);
  icon=(FXIcon*)-1;
  font=(FXFont*)-1;
  }
