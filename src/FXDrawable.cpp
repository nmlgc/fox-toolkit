/********************************************************************************
*                                                                               *
*                             D r a w a b l e   A r e a                         *
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
* $Id: FXDrawable.cpp,v 1.12 1998/10/28 07:58:58 jeroen Exp $                   *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXObject.h"
#include "FXObjectList.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXFont.h"
#include "FXDrawable.h"
#include "FXImage.h"
#include "FXIcon.h"

/*
  To do:
  
  - Use getGC() virtual instead of getGC().  Not all windows may have
    same visual!
    
  - Add virtuals for colors also!
  
*/


/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXDrawable,FXObject,NULL,0)


// For deserialization
FXDrawable::FXDrawable(){
  width=1;
  height=1;
  depth=0;
  }


// Initialize nicely
FXDrawable::FXDrawable(FXApp* a,FXint w,FXint h,FXint d):FXId(a,0){
  width=FXMAX(w,1);
  height=FXMAX(h,1);
  depth=d;
  }


// Gray bitmap
FXID FXDrawable::getGrayBitmap() const { return getApp()->graybitmap; }


// Obtain common graphic context
GC FXDrawable::getGC() const { return getApp()->gc; }


// Create drawable surface
void FXDrawable::create(){ }
 

// Destroy drawable surface
void FXDrawable::destroy(){ }


// Draw points
void FXDrawable::drawPoint(FXint x,FXint y){ 
  if(!xid){ fxerror("%s::drawPoint: drawable has not yet been created\n.",getClassName()); }
  XDrawPoint(getApp()->display,xid,getGC(),x,y); 
  }

void FXDrawable::drawPoints(const FXPoint* points,FXuint npoints){ 
  if(!xid){ fxerror("%s::drawPoints: drawable has not yet been created\n.",getClassName()); }
  XDrawPoints(getApp()->display,xid,getGC(),(XPoint*)points,npoints,CoordModeOrigin); 
  }

void FXDrawable::drawPointsRel(const FXPoint* points,FXuint npoints){
  if(!xid){ fxerror("%s::drawPointsRel: drawable has not yet been created\n.",getClassName()); }
  XDrawPoints(getApp()->display,xid,getGC(),(XPoint*)points,npoints,CoordModePrevious); 
  }


// Draw lines
void FXDrawable::drawLine(FXint x1,FXint y1,FXint x2,FXint y2){
  if(!xid){ fxerror("%s::drawLine: drawable has not yet been created\n.",getClassName()); }
  XDrawLine(getApp()->display,xid,getGC(),x1,y1,x2,y2);
  }

void FXDrawable::drawLines(const FXPoint* points,FXuint npoints){
  if(!xid){ fxerror("%s::drawLines: drawable has not yet been created\n.",getClassName()); }
  XDrawLines(getApp()->display,xid,getGC(),(XPoint*)points,npoints,CoordModeOrigin); 
  }

void FXDrawable::drawLinesRel(const FXPoint* points,FXuint npoints){
  if(!xid){ fxerror("%s::drawLinesRel: drawable has not yet been created\n.",getClassName()); }
  XDrawLines(getApp()->display,xid,getGC(),(XPoint*)points,npoints,CoordModePrevious); 
  }

void FXDrawable::drawLineSegments(const FXSegment* segments,FXuint nsegments){
  if(!xid){ fxerror("%s::drawLineSegments: drawable has not yet been created\n.",getClassName()); }
  XDrawSegments(getApp()->display,xid,getGC(),(XSegment*)segments,nsegments); 
  }
 

// Draw rectangles
void FXDrawable::drawRectangle(FXint x,FXint y,FXint w,FXint h){
  if(!xid){ fxerror("%s::drawRectangle: drawable has not yet been created\n.",getClassName()); }
  XDrawRectangle(getApp()->display,xid,getGC(),x,y,w,h);
  }

void FXDrawable::drawRectangles(const FXRectangle* rectangles,FXuint nrectangles){
  if(!xid){ fxerror("%s::drawRectangles: drawable has not yet been created\n.",getClassName()); }
  XDrawRectangles(getApp()->display,xid,getGC(),(XRectangle*)rectangles,nrectangles);
  }


// Draw arcs
void FXDrawable::drawArc(FXint x,FXint y,FXint w,FXint h,FXint ang1,FXint ang2){
  if(!xid){ fxerror("%s::drawArc: drawable has not yet been created\n.",getClassName()); }
  XDrawArc(getApp()->display,xid,getGC(),x,y,w,h,ang1,ang2);
  }

void FXDrawable::drawArcs(const FXArc* arcs,FXuint narcs){
  if(!xid){ fxerror("%s::drawArcs: drawable has not yet been created\n.",getClassName()); }
  XDrawArcs(getApp()->display,xid,getGC(),(XArc*)arcs,narcs);
  }


// Filled rectangles
void FXDrawable::fillRectangle(FXint x,FXint y,FXint w,FXint h){
  if(!xid){ fxerror("%s::fillRectangle: drawable has not yet been created\n.",getClassName()); }
  XFillRectangle(getApp()->display,xid,getGC(),x,y,w,h);
  }

void FXDrawable::fillRectangles(const FXRectangle* rectangles,FXuint nrectangles){
  if(!xid){ fxerror("%s::fillRectangles: drawable has not yet been created\n.",getClassName()); }
  XFillRectangles(getApp()->display,xid,getGC(),(XRectangle*)rectangles,nrectangles);
  }


// Draw arcs
void FXDrawable::fillArc(FXint x,FXint y,FXint w,FXint h,FXint ang1,FXint ang2){
  if(!xid){ fxerror("%s::fillArc: drawable has not yet been created\n.",getClassName()); }
  XFillArc(getApp()->display,xid,getGC(),x,y,w,h,ang1,ang2);
  }

void FXDrawable::fillArcs(const FXArc* arcs,FXuint narcs){
  if(!xid){ fxerror("%s::fillArcs: drawable has not yet been created\n.",getClassName()); }
  XFillArcs(getApp()->display,xid,getGC(),(XArc*)arcs,narcs);
  }


// Filled simple polygon
void FXDrawable::fillPolygon(const FXPoint* points,FXuint npoints){
  if(!xid){ fxerror("%s::fillPolygon: drawable has not yet been created\n.",getClassName()); }
  XFillPolygon(getApp()->display,xid,getGC(),(XPoint*)points,npoints,Convex,CoordModeOrigin);
  }

void FXDrawable::fillConcavePolygon(const FXPoint* points,FXuint npoints){
  if(!xid){ fxerror("%s::fillConcavePolygon: drawable has not yet been created\n.",getClassName()); }
  XFillPolygon(getApp()->display,xid,getGC(),(XPoint*)points,npoints,Nonconvex,CoordModeOrigin);
  }

void FXDrawable::fillComplexPolygon(const FXPoint* points,FXuint npoints){
  if(!xid){ fxerror("%s::fillComplexPolygon: drawable has not yet been created\n.",getClassName()); }
  XFillPolygon(getApp()->display,xid,getGC(),(XPoint*)points,npoints,Complex,CoordModeOrigin);
  }


// Filled simple polygon with relative points
void FXDrawable::fillPolygonRel(const FXPoint* points,FXuint npoints){
  if(!xid){ fxerror("%s::fillPolygonRel: drawable has not yet been created\n.",getClassName()); }
  XFillPolygon(getApp()->display,xid,getGC(),(XPoint*)points,npoints,Convex,CoordModePrevious);
  }

void FXDrawable::fillConcavePolygonRel(const FXPoint* points,FXuint npoints){
  if(!xid){ fxerror("%s::fillConcavePolygonRel: drawable has not yet been created\n.",getClassName()); }
  XFillPolygon(getApp()->display,xid,getGC(),(XPoint*)points,npoints,Nonconvex,CoordModePrevious);
  }

void FXDrawable::fillComplexPolygonRel(const FXPoint* points,FXuint npoints){
  if(!xid){ fxerror("%s::fillComplexPolygonRel: drawable has not yet been created\n.",getClassName()); }
  XFillPolygon(getApp()->display,xid,getGC(),(XPoint*)points,npoints,Complex,CoordModePrevious);
  }


// Draw string
void FXDrawable::drawText(FXint x,FXint y,const FXchar* string,FXuint length){
  if(!xid){ fxerror("%s::drawText: drawable has not yet been created\n.",getClassName()); }
  XDrawString(getApp()->display,xid,getGC(),x,y,(char*)string,length);
  }


void FXDrawable::drawImageText(FXint x,FXint y,const FXchar* string,FXuint length){
  if(!xid){ fxerror("%s::drawImageText: drawable has not yet been created\n.",getClassName()); }
  XDrawImageString(getApp()->display,xid,getGC(),x,y,(char*)string,length);
  }


// Draw area from source
void FXDrawable::drawArea(const FXDrawable* source,FXint sx,FXint sy,FXint sw,FXint sh,FXint dx,FXint dy){
  if(!xid){ fxerror("%s::drawArea: drawable has not yet been created\n.",getClassName()); }
  if(!source || !source->xid){ fxerror("%s::drawArea: illegal source specified.\n",getClassName()); }
  XCopyArea(getApp()->display,source->xid,xid,getGC(),sx,sy,sw,sh,dx,dy);
  }


// Draw image
void FXDrawable::drawImage(const FXImage* image,FXint dx,FXint dy){
  if(!xid){ fxerror("%s::drawImage: drawable has not yet been created\n.",getClassName()); }
  if(!image || !image->xid){ fxerror("%s::drawImage: illegal image specified.\n",getClassName()); }
  XCopyArea(getApp()->display,image->xid,xid,getGC(),0,0,image->width,image->height,dx,dy);
  }


// Draw icon
void FXDrawable::drawIcon(const FXIcon* icon,FXint dx,FXint dy){
  XGCValues gcv;  
  if(!xid){ fxerror("%s::drawIcon: drawable has not yet been created\n.",getClassName()); }
  if(!icon || !icon->xid || !icon->shape){ fxerror("%s::drawIcon: illegal icon specified.\n",getClassName()); }
  gcv.clip_mask=icon->shape;
  gcv.clip_x_origin=dx;
  gcv.clip_y_origin=dy;
  XChangeGC(getApp()->display,getGC(),GCClipMask|GCClipXOrigin|GCClipYOrigin,&gcv);
  XCopyArea(getApp()->display,icon->xid,xid,getGC(),0,0,icon->width,icon->height,dx,dy);
  gcv.clip_mask=None;
  XChangeGC(getApp()->display,getGC(),GCClipMask,&gcv);
  }


// Draw icon shaded
void FXDrawable::drawIconShaded(const FXIcon* icon,FXint dx,FXint dy){
  XGCValues gcv;  
  if(!xid){ fxerror("%s::drawIconShaded: drawable has not yet been created\n.",getClassName()); }
  if(!icon || !icon->xid || !icon->shape){ fxerror("%s::drawIconShaded: illegal icon specified.\n",getClassName()); }
  gcv.clip_mask=icon->shape;
  gcv.clip_x_origin=dx;
  gcv.clip_y_origin=dy;
  XChangeGC(getApp()->display,getGC(),GCClipMask|GCClipXOrigin|GCClipYOrigin,&gcv);
  XCopyArea(getApp()->display,icon->xid,xid,getGC(),0,0,icon->width,icon->height,dx,dy);
  gcv.function=BLT_SRC_AND_DST;
  gcv.tile=getGrayBitmap();
  gcv.fill_style=FILL_TILED;
  XChangeGC(getApp()->display,getGC(),GCFunction|GCTile|GCFillStyle,&gcv);
  XFillRectangle(getApp()->display,xid,getGC(),dx,dy,icon->width,icon->height);
  gcv.clip_mask=None;
  gcv.function=BLT_SRC;
  gcv.fill_style=FILL_SOLID;
  XChangeGC(getApp()->display,getGC(),GCClipMask|GCFunction|GCFillStyle,&gcv);
  }


// Draw hashed box
void FXDrawable::drawHashBox(FXint x,FXint y,FXint w,FXint h,FXint b){
  XGCValues gcv;  
  if(!xid){ fxerror("%s::drawHashBox: drawable has not yet been created\n.",getClassName()); }
  gcv.stipple=getApp()->graystipple;
  gcv.fill_style=FILL_STIPPLED;
  XChangeGC(getApp()->display,getGC(),GCStipple|GCFillStyle,&gcv);
  XFillRectangle(getApp()->display,xid,getGC(),x,y,w-b,b);
  XFillRectangle(getApp()->display,xid,getGC(),x+w-b,y,b,h-b);
  XFillRectangle(getApp()->display,xid,getGC(),x+b,y+h-b,w-b,b);
  XFillRectangle(getApp()->display,xid,getGC(),x,y+b,b,h-b);
  gcv.stipple=getApp()->blackstipple;   // Needed for IRIX6.4 bug workaround!
  gcv.fill_style=FILL_SOLID;
  XChangeGC(getApp()->display,getGC(),GCStipple|GCFillStyle,&gcv);
  }


// Set foreground/background drawing color
void FXDrawable::setForeground(FXPixel clr){
  XSetForeground(getApp()->display,getGC(),clr);
  }


void FXDrawable::setBackground(FXPixel clr){
  XSetBackground(getApp()->display,getGC(),clr);
  }


// Set dash pattern
void FXDrawable::setDashes(FXuint dashoffset,const FXchar *dashlist,FXuint n){
  XSetDashes(getApp()->display,getGC(),dashoffset,(char*)dashlist,n);
  }


// Set line attributes
void FXDrawable::setLineAttributes(FXuint linewidth,FXLineStyle linestyle,FXCapStyle capstyle,FXJoinStyle joinstyle){
  XSetLineAttributes(getApp()->display,getGC(),linewidth,linestyle,capstyle,joinstyle);
  }


// Set fill style
void FXDrawable::setFillStyle(FXFillStyle fillstyle){
  XSetFillStyle(getApp()->display,getGC(),fillstyle);
  }
  
  
// Set fill rule
void FXDrawable::setFillRule(FXFillRule fillrule){
  XSetFillRule(getApp()->display,getGC(),fillrule);
  }


// Set blit function
void FXDrawable::setFunction(FXFunction func){
  XSetFunction(getApp()->display,getGC(),func);
  }


// Set clip rectangle
void FXDrawable::setClipRectangle(FXint x,FXint y,FXint w,FXint h){
  XRectangle c;
  c.x=x; c.y=y; c.width=w; c.height=h;
  XSetClipRectangles(getApp()->display,getGC(),0,0,&c,1,Unsorted);
  }
 

// Clear clipping
void FXDrawable::clearClipRectangle(){
  XSetClipMask(getApp()->display,getGC(),None);
  }


// Set clip mask
void FXDrawable::setClipMask(FXImage* mask,FXint dx,FXint dy){
  XGCValues gcv;  
  if(!mask || !mask->xid){ fxerror("%s::setClipMask: illegal mask specified.\n",getClassName()); }
  gcv.clip_mask=mask->xid;
  gcv.clip_x_origin=dx;
  gcv.clip_y_origin=dy;
  XChangeGC(getApp()->display,getGC(),GCClipMask|GCClipXOrigin|GCClipYOrigin,&gcv);
  }


// Clear clip mask
void FXDrawable::clearClipMask(){
  XSetClipMask(getApp()->display,getGC(),None);
  }
  

// Set font to draw text with
void FXDrawable::setTextFont(FXFont *font){
  if(!font || !font->xid){ fxerror("%s::setTextFont: illegal or NULL font specified.\n",getClassName()); }
  XSetFont(getApp()->display,getGC(),font->id());
  }


// Save data
void FXDrawable::save(FXStream& store) const {
  FXId::save(store);
  store << width;
  store << height;
  }


// Load data
void FXDrawable::load(FXStream& store){ 
  FXId::load(store);
  store >> width;
  store >> height;
  }


// Clean up
FXDrawable::~FXDrawable(){
  xid=0;
  }
  

