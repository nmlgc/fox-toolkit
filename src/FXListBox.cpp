/********************************************************************************
*                                                                               *
*               D r o p - D o w n   L i s t   B o x   O b j e c t               *
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
* $Id: FXListBox.cpp,v 1.19 1998/10/30 05:29:50 jeroen Exp $                     *
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
#include "FXDrawable.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXLabel.h"
#include "FXTextField.h"
#include "FXButton.h"
#include "FXArrowButton.h"
#include "FXComposite.h"
#include "FXPacker.h"
#include "FXShell.h"
#include "FXPopup.h"
#include "FXScrollbar.h"
#include "FXScrollWindow.h"
#include "FXList.h"
#include "FXListBox.h"



/*
  Notes:
*/


/*******************************************************************************/

// Map
FXDEFMAP(FXListBox) FXListBoxMap[]={
  FXMAPFUNC(SEL_MOTION,0,FXListBox::onMotion),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXListBox::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXListBox::onLeftBtnRelease),
  FXMAPFUNC(SEL_COMMAND,FXListBox::ID_BUTTON,FXListBox::onCmdButton),
  FXMAPFUNC(SEL_COMMAND,FXListBox::ID_TEXT,FXListBox::onCmdText),
  FXMAPFUNC(SEL_COMMAND,FXListBox::ID_LIST,FXListBox::onCmdList),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_POST,FXListBox::onCmdPost),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_UNPOST,FXListBox::onCmdUnpost),
  };


// Object implementation
FXIMPLEMENT(FXListBox,FXPacker,FXListBoxMap,ARRAYNUMBER(FXListBoxMap))


// List box
FXListBox::FXListBox(FXComposite *p,FXint cols,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXPacker(p, opts, x,y,w,h, 0,0,0,0, 0,0){
  flags|=FLAG_ENABLED;
  target=tgt;
  message=sel;
  text=new FXTextField(this,cols,this,FXListBox::ID_TEXT,0, 0,0,0,0, pl,pr,pt,pb);
  button=new FXArrowButton(this,this,FXListBox::ID_BUTTON,FRAME_RAISED|FRAME_THICK|ARROW_DOWN, 0,0,0,0, 0,0,0,0);
  pane=new FXPopup(getApp(),this,FRAME_LINE);
  list=new FXList(pane,this,FXListBox::ID_LIST,LIST_BROWSESELECT|LIST_AUTOSELECT|LAYOUT_FILL_X|LAYOUT_FIX_Y|SCROLLERS_TRACK,0,0,0,400);
  dragCursor=getApp()->rarrowCursor;
  
  for(int i=0; i<40; i++){
    char name[50];
    sprintf(name,"%04d",i);
    list->addItemLast(name);
    }
  }


// Create X window
void FXListBox::create(){
  FXPacker::create();
  pane->create();
  }


// Destroy X window
void FXListBox::destroy(){
  pane->destroy();
  FXPacker::destroy();
  }


// Get default width
FXint FXListBox::getDefaultWidth(){
  FXint ww,pw;
  ww=text->getDefaultWidth()+button->getDefaultWidth();
  pw=pane->getDefaultWidth();
  return FXMAX(ww,pw)+(border<<1);
  }


// Get default height
FXint FXListBox::getDefaultHeight(){
  return text->getDefaultHeight()+(border<<1);
  }


// Recalculate layout
void FXListBox::layout(){
  FXint buttonWidth,textWidth,itemHeight;
  itemHeight=height-(border<<1);
  buttonWidth=button->getDefaultWidth();
  textWidth=width-buttonWidth-(border<<1);
  text->position(border,border,textWidth,itemHeight);
  button->position(border+textWidth,border,buttonWidth,itemHeight);
  flags&=~FLAG_DIRTY;
  }


// Post the menu
long FXListBox::onCmdPost(FXObject*,FXSelector,void*){
  if(pane && !pane->shown()){
    FXint x,y;
    translateCoordinatesTo(x,y,getRoot(),0,0);
    y=y+height;
    pane->popup(this,x,y,width,pane->getDefaultHeight());
    if(!grabbed()) grab();
    flags&=~FLAG_UPDATE;
    }
  return 1;
  }


// Unpost the menu
long FXListBox::onCmdUnpost(FXObject*,FXSelector,void*){
  if(pane && pane->shown()){
    pane->popdown();
    if(grabbed()) ungrab();
    flags|=FLAG_UPDATE;
    }
  return 1;
  }


// Pressed the button
long FXListBox::onCmdButton(FXObject* sender,FXSelector,void* ptr){
  if(!pane->shown()){
    handle(this,MKUINT(ID_POST,SEL_COMMAND),ptr);
    }
  else{
    handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),ptr);
    }
  return 1;
  }


// List has changed
long FXListBox::onCmdList(FXObject* sender,FXSelector,void* ptr){
  text->setText(list->getItemText(list->getCurrentItem()));
  handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),ptr);
  return 1;
  }


// Text has changed
long FXListBox::onCmdText(FXObject* sender,FXSelector,void* ptr){
  return 1;
  }


// Pressed left button
long FXListBox::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),ptr);
  return 1;
  }


// Released left button
long FXListBox::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  handle(this,MKUINT(ID_UNPOST,SEL_COMMAND),ptr);
  return 1;
  }


// Mousing around
long FXListBox::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  if(pane->shown()){
    if(pane){
      if(pane->contains(ev->root_x,ev->root_y)){
        if(grabbed()) ungrab(); 
        }
      else{
        if(!grabbed()) grab();
        }
      return 1;
      }
    }
  return 0;
  }


// Logically inside pane
FXbool FXListBox::contains(FXint parentx,FXint parenty) const {
  if(pane->shown() && pane->contains(parentx,parenty)) return 1;
  return 0;
  }


// Delete it
FXListBox::~FXListBox(){
  delete pane;
  pane=(FXPopup*)-1;
  text=(FXTextField*)-1;
  button=(FXArrowButton*)-1;
  list=(FXList*)-1;
  }

