/********************************************************************************
*                                                                               *
*                             S p i n   B u t t o n                             *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998 by Jeroen van der Zijp.   All Rights Reserved.             *
*********************************************************************************
* Contributed by: Lyle Johnson                                                  *
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
* $Id: FXSpinner.cpp,v 1.11 1998/10/29 05:38:14 jeroen Exp $                     *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "fxkeys.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXObject.h"
#include "FXAccelTable.h"
#include "FXObjectList.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXDrawable.h"
#include "FXImage.h"
#include "FXIcon.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXLabel.h"
#include "FXTextField.h"
#include "FXButton.h"
#include "FXArrowButton.h"
#include "FXComposite.h"
#include "FXPacker.h"
#include "FXSpinner.h"


/*
  To do:
  - Should respond to up/down arrows.
  - should send SEL_COMMAND.
  - Should understand get/set messages.
*/

#define BUTTONWIDTH 14


/*******************************************************************************/

  
//  Message map
FXDEFMAP(FXSpinner) FXSpinnerMap[]={
  FXMAPFUNC(SEL_COMMAND,FXSpinner::ID_INCREMENT,FXSpinner::onIncrement),
  FXMAPFUNC(SEL_COMMAND,FXSpinner::ID_DECREMENT,FXSpinner::onDecrement),
  FXMAPFUNC(SEL_KEYPRESS,0,FXSpinner::onKeyPress),
  };

  
// Object implementation
FXIMPLEMENT(FXSpinner,FXPacker,FXSpinnerMap,ARRAYNUMBER(FXSpinnerMap))

  
// Construct spinner out of two buttons and a text field
FXSpinner::FXSpinner(){
  flags|=FLAG_ENABLED;
  textField=(FXTextField*)-1;
  upButton=(FXArrowButton*)-1;
  downButton=(FXArrowButton*)-1;
  minVal=-2147483648;
  maxVal=2147483647;
  incrementVal=1;
  currentVal=1;
  }


// Construct spinner out of two buttons and a text field
FXSpinner::FXSpinner(FXComposite *p,FXint cols,FXObject *tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXPacker(p, opts, x,y,w,h, 0,0,0,0, 0,0){
  flags|=FLAG_ENABLED;
  target=tgt;
  message=sel;
  textField=new FXTextField(this,cols,NULL,0, 0, 0,0,0,0,pl,pr,pt,pb);
  upButton=new FXArrowButton(this,this,FXSpinner::ID_INCREMENT,FRAME_RAISED|FRAME_THICK|ARROW_UP|ARROW_REPEAT, 0,0,0,0, 0,0,0,0);
  downButton=new FXArrowButton(this,this,FXSpinner::ID_DECREMENT,FRAME_RAISED|FRAME_THICK|ARROW_DOWN|ARROW_REPEAT, 0,0,0,0, 0,0,0,0);
  textField->disable();
  minVal=(options&SPIN_NOMIN) ? -2147483648 : 1;
  maxVal=(options&SPIN_NOMAX) ? 2147483647 : 10;
  incrementVal=1;
  currentVal=1;
  }


// Get default width
FXint FXSpinner::getDefaultWidth(){
  FXint tw=0;
  if(!(options&SPIN_NOTEXT)) tw=textField->getDefaultWidth();
//  return tw+upButton->getDefaultWidth()+(border<<1);
  return tw+BUTTONWIDTH+(border<<1);
  }


// Get default height
FXint FXSpinner::getDefaultHeight(){
//   FXint th=0;
//   if(!(options&SPIN_NOTEXT)) th=textField->getDefaultHeight();
//   return MAX(th,2*upButton->getDefaultHeight())+(border<<1);
  return textField->getDefaultHeight()+(border<<1);
  }


// Create window
void FXSpinner::create(){
  FXPacker::create();
  updateText();
  show();
  }


// Recompute layout
void FXSpinner::layout(){
  FXint buttonWidth,buttonHeight,textWidth,textHeight;

  textHeight=height-2*border;
  buttonHeight=textHeight>>1;
  
  // Buttons plus the text; buttons are default width, text stretches to fill the rest
  if(!(options&SPIN_NOTEXT)){
    buttonWidth=BUTTONWIDTH;
    textWidth=width-buttonWidth-2*border;
    textField->position(border,border,textWidth,textHeight);
    upButton->position(border+textWidth,border,buttonWidth,buttonHeight);
    downButton->position(border+textWidth,height-buttonHeight-border,buttonWidth,buttonHeight);
    }
  
  // Only the buttons:- place buttons to take up the whole space!
  else{
    buttonWidth=width-2*border;
    upButton->position(border,border,buttonWidth,buttonHeight);
    downButton->position(border,height-buttonHeight-border,buttonWidth,buttonHeight);
    }
  flags&=~FLAG_DIRTY;
  }


// Respond to increment message
long FXSpinner::onIncrement(FXObject*,FXSelector,void *ptr){
  if(!isEnabled()) return 0;
  increment();
  if(target) target->handle(this,MKUINT(message,SEL_CHANGED),ptr);
  return 1;
  }


// Responde to decrement message
long FXSpinner::onDecrement(FXObject*,FXSelector,void *ptr){
  if(!isEnabled()) return 0;
  decrement();
  if(target) target->handle(this,MKUINT(message,SEL_CHANGED),ptr);
  return 1;
  }


// Keyboard press
long FXSpinner::onKeyPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  if(!isEnabled()) return 0;
  
  // See if its one of the special keys
  switch(event->code){
    case KEY_Up:    
    case KEY_KP_Up:    
    case KEY_KP_Add:  
    case KEY_plus:
      increment();  
      if(target) target->handle(this,MKUINT(message,SEL_CHANGED),ptr);
      return 1;
    case KEY_Down:  
    case KEY_KP_Down:  
    case KEY_KP_Subtract: 
    case KEY_minus: 
      decrement();
      if(target) target->handle(this,MKUINT(message,SEL_CHANGED),ptr);
      return 1;
    }
  
  // Nope:- pretent we didn't handle it
  return 0;
  }


// Increment spinner
void FXSpinner::increment(){
  if(minVal<maxVal){
    FXint oldval=currentVal;
    currentVal+=incrementVal;
    if(options&SPIN_CYCLIC){
      currentVal=minVal+(currentVal-minVal)%(maxVal-minVal+1);  // Count around modulo (max-min+1)
      }
    else{
      if(currentVal>maxVal) currentVal=maxVal;
      }
    if(oldval!=currentVal) updateText();
    }
  }


// Decrement spinner
void FXSpinner::decrement(){
  if(minVal<maxVal){
    FXint oldval=currentVal;
    currentVal-=incrementVal;
    if(options&SPIN_CYCLIC){
      currentVal+=(maxVal-minVal+1);
      currentVal=minVal+(currentVal-minVal)%(maxVal-minVal+1);  // Count around modulo (max-min+1)
      }
    else{
      if(currentVal<minVal) currentVal=minVal;
      }
    if(oldval!=currentVal) updateText();
    }
  }


// Update text in the field
void FXSpinner::updateText(){
  FXchar s[20];
  sprintf(s,"%d",currentVal);
  setText(s);
  }


// Place text straight into the text field part
void FXSpinner::setText(const FXchar* text){
  textField->setText(text);
  }


// Return text in the text field part
const FXchar* FXSpinner::getText() const {
  return textField->getText();
  }


// True if spinner is cyclic
FXbool FXSpinner::isCyclic() const {
  return (options&SPIN_CYCLIC)!=0;
  }


// Set spinner cyclic mode
void FXSpinner::setCyclic(FXbool s){
  if(s) options|=SPIN_CYCLIC; else options&=~SPIN_CYCLIC;
  }


// Enable the widget
void FXSpinner::enable(){
  if(!(flags&FLAG_ENABLED)){
    upButton->enable();
    downButton->enable();
    }
  }


// Disable the widget
void FXSpinner::disable(){
  if(flags&FLAG_ENABLED){
    upButton->disable();
    downButton->disable();
    }
  }


// Set minimum and maximum; fix other stuff too
void FXSpinner::setMinMax(FXint mn,FXint mx){
  if(mn>mx){ fxerror("%s::setMinMax: minimum should be lower than maximum.\n",getClassName()); }
  if(currentVal<mn) currentVal=mn;
  if(currentVal>mx) currentVal=mx;
  minVal=mn;
  maxVal=mx;
  updateText();
  }


// Return the minimum and maximum
void FXSpinner::getMinMax(FXint& mn,FXint& mx) const {
  mn=minVal;
  mx=maxVal;
  }


// Set new value
void FXSpinner::setValue(FXint value){
  if(value<minVal) value=minVal;
  if(value>maxVal) value=maxVal;
  currentVal=value;
  updateText();
  }


// Change value increment
void FXSpinner::setIncrement(FXint inc){
  if(inc<1){ fxerror("%s::setIncrement: negative or zero increment specified.\n",getClassName()); }
  incrementVal=inc;
  }


// True if text supposed to be visible
FXbool FXSpinner::isTextVisible() const {
  return textField->shown();
  }


// Change text visibility
void FXSpinner::setTextVisible(FXbool s){
  FXuint opts=s?(options|SPIN_NOTEXT):(options&~SPIN_NOTEXT);
  if(options!=opts){
    s ? textField->show() : textField->hide();
    options=opts;
    recalc();
    }
  }


// Destruct spinner:- trash it!
FXSpinner::~FXSpinner(){
  textField=(FXTextField*)-1;
  upButton=(FXArrowButton*)-1;
  downButton=(FXArrowButton*)-1;
  }
