/********************************************************************************
*                                                                               *
*                   M a t r i x   C o n t a i n e r   O b j e c t               *
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
* $Id: FXMatrix.cpp,v 1.14 1998/09/10 15:15:50 jvz Exp $                      *
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
#include "FXComposite.h"
#include "FXPacker.h"
#include "FXMatrix.h"
#include "FXShell.h"

  
/*
  Notes:
  - Need to observe FILL vs CENTER options.
  - Filled items should shrink as well as stretch.
  - Stretch should be proportional.
  - Center mode, non-stretch should be observed.
  - Row/Column stretchable iff all elements in row/column have row/column
    stretch hint.
*/

#define MAXNUM    512        // Maximum number of columns/rows


/*******************************************************************************/


// Map
FXDEFMAP(FXMatrix) FXMatrixMap[]={
  FXMAPFUNC(SEL_FOCUS_UP,0,FXMatrix::onFocusUp),
  FXMAPFUNC(SEL_FOCUS_DOWN,0,FXMatrix::onFocusDown),
  FXMAPFUNC(SEL_FOCUS_LEFT,0,FXMatrix::onFocusLeft),
  FXMAPFUNC(SEL_FOCUS_RIGHT,0,FXMatrix::onFocusRight),
  };


// Object implementation
FXIMPLEMENT(FXMatrix,FXPacker,FXMatrixMap,ARRAYNUMBER(FXMatrixMap))


// Make a vertical one
FXMatrix::FXMatrix(FXComposite* p,FXint n,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb,FXint hs,FXint vs):
  FXPacker(p,opts,x,y,w,h,pl,pr,pt,pb,hs,vs),num(n){
  if(num>MAXNUM) num=MAXNUM; else if(num<1) num=1;
  }


// Focus moved up
long FXMatrix::onFocusUp(FXObject*,FXSelector sel,void* ptr){
  FXWindow *child;
  FXint n;
  child=getFocus();
  if(child){
    n=(options&MATRIX_BY_COLUMNS)?num:1;
    while(child && n--) child=child->getPrev();
    }
  else 
    child=getLast();
  while(child){
    if(child->isEnabled() && child->canFocus()){
      child->setFocus();
      return 1;
      }
    if(child->isComposite() && child->handle(this,sel,ptr)) return 1;
    n=(options&MATRIX_BY_COLUMNS)?num:1;
    while(child && n--) child=child->getPrev();
    }
  return 0;
  }


// Focus moved down
long FXMatrix::onFocusDown(FXObject*,FXSelector sel,void* ptr){
  FXWindow *child;
  FXint n;
  child=getFocus();
  if(child){
    n=(options&MATRIX_BY_COLUMNS)?num:1;
    while(child && n--) child=child->getNext();
    }
  else 
    child=getFirst();
  while(child){
    if(child->isEnabled() && child->canFocus()){
      child->setFocus();
      return 1;
      }
    if(child->isComposite() && child->handle(this,sel,ptr)) return 1;
    n=(options&MATRIX_BY_COLUMNS)?num:1;
    while(child && n--) child=child->getNext();
    }
  return 0;
  }


// Focus moved to left
long FXMatrix::onFocusLeft(FXObject*,FXSelector sel,void* ptr){
  FXWindow *child;
  FXint n;
  child=getFocus();
  if(child){
    n=(options&MATRIX_BY_COLUMNS)?1:num;
    while(child && n--) child=child->getPrev();
    }
  else
    child=getLast();
  while(child){
    if(child->isEnabled() && child->canFocus()){
      child->setFocus();
      return 1;
      }
    if(child->isComposite() && child->handle(this,sel,ptr)) return 1;
    n=(options&MATRIX_BY_COLUMNS)?1:num;
    while(child && n--) child=child->getPrev();
    }
  return 0;
  }


// Focus moved to right
long FXMatrix::onFocusRight(FXObject*,FXSelector sel,void* ptr){
  FXWindow *child;
  FXint n;
  child=getFocus();
  if(child){
    n=(options&MATRIX_BY_COLUMNS)?1:num;
    while(child && n--) child=child->getNext();
    }
  else
    child=getFirst();
  while(child){
    if(child->isEnabled() && child->canFocus()){
      child->setFocus();
      return 1;
      }
    if(child->isComposite() && child->handle(this,sel,ptr)) return 1;
    n=(options&MATRIX_BY_COLUMNS)?1:num;
    while(child && n--) child=child->getNext();
    }
  return 0;
  }


// Compute minimum width based on child layout hints
FXint FXMatrix::getDefaultWidth(){
  register FXint ncol,wmax,c,n,w,mw=0;
  register FXWindow *child;
  register FXuint hints;
  FXint colw[MAXNUM];
  for(c=0; c<MAXNUM; c++) colw[c]=0;
  if(options&PACK_UNIFORM_WIDTH) mw=maxChildWidth();
  for(child=getFirst(),wmax=0,n=0; child; child=child->getNext()){
    if(child->shown()){
      hints=child->getLayoutHints();
      if(hints&LAYOUT_FIX_WIDTH) w=child->getWidth(); 
      else if(options&PACK_UNIFORM_WIDTH) w=mw;
      else w=child->getDefaultWidth();
      c=(options&MATRIX_BY_COLUMNS)?n%num:n/num;
      FXASSERT(c<MAXNUM);
      if(w>colw[c]){wmax+=w-colw[c];colw[c]=w;}
      n++;
      }
    }
  ncol=(options&MATRIX_BY_COLUMNS)?num:(n+num-1)/num;
  if(ncol>1) wmax+=(ncol-1)*hspacing;
  return padleft+padright+wmax+(border<<1);
  }
  
  

// Compute minimum height based on child layout hints
FXint FXMatrix::getDefaultHeight(){
  register FXint nrow,hmax,r,n,h,mh=0;
  register FXWindow *child;
  register FXuint hints;
  FXint rowh[MAXNUM];
  for(r=0; r<MAXNUM; r++) rowh[r]=0;
  if(options&PACK_UNIFORM_HEIGHT) mh=maxChildHeight();
  for(child=getFirst(),hmax=0,n=0; child; child=child->getNext()){
    if(child->shown()){
      hints=child->getLayoutHints();
      if(hints&LAYOUT_FIX_HEIGHT) h=child->getHeight(); 
      else if(options&PACK_UNIFORM_HEIGHT) h=mh;
      else h=child->getDefaultHeight();
      r=(options&MATRIX_BY_COLUMNS)?n/num:n%num;
      FXASSERT(r<MAXNUM);
      if(h>rowh[r]){hmax+=h-rowh[r];rowh[r]=h;}
      n++;
      }
    }
  nrow=(options&MATRIX_BY_COLUMNS)?(n+num-1)/num:num;
  if(nrow>1) hmax+=(nrow-1)*vspacing;
  return padtop+padbottom+hmax+(border<<1);
  }


// Recalculate layout
void FXMatrix::layout(){
  FXint ncol,nrow,r,c,x,y,w,h,n,e,t;
  FXint rowh[MAXNUM],colw[MAXNUM];
  FXbool srow[MAXNUM],scol[MAXNUM];
  FXint left,right,top,bottom,cw,rh;
  FXint mw=0,mh=0;
  FXint hremain,vremain,hsumexpand,vsumexpand;
  FXWindow *child;
  FXuint hints;
  
  flags&=~FLAG_DIRTY;
  
  // Done already?
  if(getFirst()==NULL) return;
  
  // Placement rectangle; right/bottom non-inclusive
  left=border+padleft;
  right=width-border-padright;
  top=border+padtop;
  bottom=height-border-padbottom;
  hremain=right-left;
  vremain=bottom-top;
  
  // Clear column/row sizes
  for(n=0; n<MAXNUM; n++){
    colw[n]=rowh[n]=1;      // Need at least 1 pixel
    srow[n]=scol[n]=1;
    }

  // Get maximum child size
  if(options&PACK_UNIFORM_WIDTH) mw=maxChildWidth();
  if(options&PACK_UNIFORM_HEIGHT) mh=maxChildHeight();
    
  // Find expandable columns and rows
  for(child=getFirst(),n=0; child; child=child->getNext()){
    if(child->shown()){
      hints=child->getLayoutHints();
      if(hints&LAYOUT_FIX_WIDTH) w=child->getWidth(); 
      else if(options&PACK_UNIFORM_WIDTH) w=mw;
      else w=child->getDefaultWidth();
      if(hints&LAYOUT_FIX_HEIGHT) h=child->getHeight(); 
      else if(options&PACK_UNIFORM_HEIGHT) h=mh;
      else h=child->getDefaultHeight();
      if(options&MATRIX_BY_COLUMNS){r=n/num;c=n%num;}else{r=n%num;c=n/num;}
      FXASSERT(r<MAXNUM && c<MAXNUM);
      if(w>colw[c]) colw[c]=w;
      if(h>rowh[r]) rowh[r]=h;
      if(!(hints&LAYOUT_FILL_COLUMN)) scol[c]=0;
      if(!(hints&LAYOUT_FILL_ROW)) srow[r]=0;
      n++;
      }
    }
      
  // Get number of rows and columns
  if(options&MATRIX_BY_COLUMNS){ncol=num;nrow=(n+num-1)/num;}else{ncol=(n+num-1)/num;nrow=num;}

  // Find stretch in columns
  for(c=hsumexpand=0; c<ncol; c++){ 
    if(scol[c]) hsumexpand+=colw[c]; else hremain-=colw[c]; 
    }

  // Find stretch in rows
  for(r=vsumexpand=0; r<nrow; r++){ 
    if(srow[r]) vsumexpand+=rowh[r]; else vremain-=rowh[r]; 
    }

  // Substract inter-child spacing
  if(ncol>1) hremain-=(ncol-1)*hspacing;
  if(nrow>1) vremain-=(nrow-1)*vspacing;
  
  // Disburse space horizontally
  for(c=e=0,x=border+padleft; c<ncol; c++){
    w=colw[c];
    if(scol[c]){
      t=w*hremain;
      FXASSERT(hsumexpand>0);
      w=t/hsumexpand;
      e+=t%hsumexpand;
      if(e>=hsumexpand){w++;e-=hsumexpand;}
      }
    colw[c]=x;
    x+=w+hspacing;
    }
  colw[ncol]=x;

  // Disburse space vertically
  for(r=e=0,y=border+padtop; r<nrow; r++){
    h=rowh[r];
    if(srow[r]){
      t=h*vremain;
      FXASSERT(vsumexpand>0);
      h=t/vsumexpand;
      e+=t%vsumexpand;
      if(e>=vsumexpand){w++;e-=vsumexpand;}
      }
    rowh[r]=y;
    y+=h+vspacing;
    }
  rowh[nrow]=y;
        
  // Do the layout
  for(child=getFirst(),n=0; child; child=child->getNext()){
    if(child->shown()){
      hints=child->getLayoutHints();
      if(options&MATRIX_BY_COLUMNS){r=n/num;c=n%num;}else{r=n%num;c=n/num;}
      cw=colw[c+1]-colw[c]-hspacing;
      rh=rowh[r+1]-rowh[r]-vspacing;
      
      if(hints&LAYOUT_FIX_WIDTH) w=child->getWidth(); 
      else if(options&PACK_UNIFORM_WIDTH) w=mw;
      else if(hints&LAYOUT_FILL_X) w=cw;
      else w=child->getDefaultWidth();
      
      if(hints&LAYOUT_CENTER_X) x=colw[c]+(cw-w)/2;
      else if(hints&LAYOUT_RIGHT) x=colw[c]+cw-w;
      else x=colw[c];
      
      if(hints&LAYOUT_FIX_HEIGHT) h=child->getHeight(); 
      else if(options&PACK_UNIFORM_HEIGHT) h=mh;
      else if(hints&LAYOUT_FILL_Y) h=rh;
      else h=child->getDefaultHeight();
    
      if(hints&LAYOUT_CENTER_Y) y=rowh[r]+(rh-h)/2;
      else if(hints&LAYOUT_BOTTOM) y=rowh[r]+rh-h;
      else y=rowh[r];
      
      child->position(x,y,w,h);
      n++;
      }
    }
  }
