/********************************************************************************
*                                                                               *
*                         C u r s o r - O b j e c t                             *
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
* $Id: FXCursor.cpp,v 1.6 1998/09/18 22:07:14 jvz Exp $                       *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXObject.h"
#include "FXObjectList.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXCursor.h"

/*
  To do:
  - Redesign cursor class along the same lines as FXIcon/FXImage.
*/


/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXCursor,FXId,NULL,0)


// Initialize nicely
FXCursor::FXCursor(FXApp* a):FXId(a,None){
  hotx=-1;
  hoty=-1;
  // Need to acquire data here!
  }
  
  
// Create cursor
void FXCursor::create(){
  // Need to create cursor here!
  }


// Destroy cursor
void FXCursor::destroy(){
  if(xid){XFreeCursor(getApp()->display,xid);xid=0;}
  }


// Create from font
FXint FXCursor::createFromFont(FXint shape){
  xid=XCreateFontCursor(getApp()->display,shape);
  return xid!=None;
  }


// Create from bitmap
FXint FXCursor::createFromBitmapData(const FXchar* image,const FXchar* mask,FXint width,FXint height,FXint hx,FXint hy){
  Pixmap imagexpm,maskxpm;
  XColor color[2];
  color[0].pixel=BlackPixel(getApp()->display,DefaultScreen(getApp()->display));
  color[1].pixel=WhitePixel(getApp()->display,DefaultScreen(getApp()->display));
  color[0].flags=DoRed|DoGreen|DoBlue;
  color[1].flags=DoRed|DoGreen|DoBlue;
  XQueryColors(getDisplay(),DefaultColormap(getApp()->display,DefaultScreen(getApp()->display)),color,2);
  imagexpm=XCreateBitmapFromData(getApp()->display,XDefaultRootWindow(getApp()->display),(char*)image,width,height);
  maskxpm=None;
  if(mask){maskxpm=XCreateBitmapFromData(getApp()->display,XDefaultRootWindow(getApp()->display),(char*)mask,width,height);}
  xid=XCreatePixmapCursor(getApp()->display,imagexpm,maskxpm,&color[0],&color[1],hx,hy);
  return xid!=None;
  }


// Save object to stream
void FXCursor::save(FXStream& store) const {
  FXId::save(store);
  store << hotx;
  store << hoty;
  // More here!!!
  }

      

// Load object from stream
void FXCursor::load(FXStream& store){
  FXId::load(store);
  store >> hotx;
  store >> hoty;
  // More here!!!
  }  


// Clean up
FXCursor::~FXCursor(){
  destroy();
  }
  

