/********************************************************************************
*                                                                               *
*                               F o n t   O b j e c t                           *
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
* $Id: FXFont.cpp,v 1.7 1998/09/18 22:07:14 jvz Exp $                         *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXObject.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXFont.h"


/*
  To do:
  - Instead of a font name only, should be taking different params.
*/


/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXFont,FXId,NULL,0)


// Deserialization
FXFont::FXFont(){
  font=(XFontStruct*)-1;
  }


// Initialize
FXFont::FXFont(FXApp* a,const char* nm):FXId(a),name(nm){
  font=NULL;
  }


// Create font
void FXFont::create(){
  if(!xid){

    // Gotta have display open!
    if(!getApp()->display){ fxerror("%s::create: trying to create font before opening display.\n",getClassName()); }
  
    // Should have non-NULL font name
    if(!name.text()){ fxerror("%s::create: font name should not be NULL.\n",getClassName()); }

    // Try load the font
    font=XLoadQueryFont(getApp()->display,name.text());

    // Test if it worked    
    if(font){
      xid=font->fid;
      }
    }
  }


// Destroy font
void FXFont::destroy(){
  if(xid){
    FXASSERT(font);
    XFreeFont(getApp()->display,font);
    font=NULL;
    xid=0;
    }
  }


// Is it a mono space font
FXbool FXFont::isFontMono() const {
  if(!font){ fxerror("%s::isFontMono: font has not been created.\n",getClassName()); }
  return font->per_char==NULL;
  }


// Get font ascent
FXint FXFont::getFontWidth() const {
  if(!font){ fxerror("%s::getFontWidth: font has not been created.\n",getClassName()); }
  return font->max_bounds.width;
  }


// Get font descent
FXint FXFont::getFontHeight() const {
  if(!font){ fxerror("%s::getFontHeight: font has not been created.\n",getClassName()); }
  return font->max_bounds.ascent+font->max_bounds.descent;
  }


// Get font ascent
FXint FXFont::getFontAscent() const {
  if(!font){ fxerror("%s::getFontAscent: font has not been created.\n",getClassName()); }
  return font->max_bounds.ascent;
  }


// Get font descent
FXint FXFont::getFontDescent() const {
  if(!font){ fxerror("%s::getFontDescent: font has not been created.\n",getClassName()); }
  return font->max_bounds.descent;
  }


// Text width
FXint FXFont::getTextWidth(const FXchar *text,FXuint n) const {
  if(!font){ fxerror("%s::getTextWidth: font has not been created.\n",getClassName()); }
  if(!text && n){ fxerror("%s::getTextWidth: NULL string argument\n",getClassName()); }
  return XTextWidth(font,text,n);
  }


// Text height
FXint FXFont::getTextHeight(const FXchar *text,FXuint n) const {
  XCharStruct chst; int dir,asc,desc;
  if(!font){ fxerror("%s::getTextHeight: font has not been created.\n",getClassName()); }
  if(!text && n){ fxerror("%s::getTextHeight: NULL string argument\n",getClassName()); }
  XTextExtents(font,text,n,&dir,&asc,&desc,&chst);
  return asc+desc;
  }


// Save font to stream
void FXFont::save(FXStream& store) const {
  FXId::save(store);
  store << name;
  }


// Load font from stream; create() should be called later
void FXFont::load(FXStream& store){
  FXId::load(store);
  store >> name;
  if(font){XFreeFont(getApp()->display,font);}
  font=NULL;
  }  


// Clean up
FXFont::~FXFont(){
  if(font){XFreeFont(getApp()->display,font);}
  font=(XFontStruct*)-1;
  }
  

