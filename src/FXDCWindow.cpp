/********************************************************************************
*                                                                               *
*  D e v i c e   C o n t e x t   F o r   W i n d o w s   a n d   I m a g e s    *
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
* $Id: FXDCWindow.cpp,v 1.86 2002/01/18 22:42:59 jeroen Exp $                   *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxkeys.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXObject.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXSettings.h"
#include "FXRegistry.h"
#include "FXAccelTable.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXVisual.h"
#include "FXFont.h"
#include "FXCursor.h"
#include "FXDrawable.h"
#include "FXImage.h"
#include "FXBitmap.h"
#include "FXIcon.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXComposite.h"
#include "FXRootWindow.h"
#include "FXShell.h"
#include "FXRegion.h"
#include "FXDC.h"
#include "FXDCWindow.h"

/*
  Notes:

  - Associate a DC with a surface before you begin using it:

     long SomeWidget::onPaint(FXObject*,FXSelector,void* ptr){
       FXDCWindow dc(this,ptr);
       dc.drawLine(...);
       ... jadajadajada ...
       return 1;
       }

    The association is automatically broken when you go out of scope; the
    destructor of the FXDCWindow does this for you.

  - Optimizations: only perform style/attribute changes just before an actual
    drawing command takes place:- X-Windows apparently already does this;
    MS-Windows also?

  - We assume the following initial state:

    BLIT Function:        BLT_SRC
    Foreground:           black (0)
    Background:           white (1)
    Line Width:           0 (meaning thinnest/fastest, no guaranteed pixelation)
    Cap Style:            CAP_BUTT
    Join Style:           JOIN_MITER
    Line Style:           LINE_SOLID
    Fill Style:           FILL_SOLID
    Fill Rule:            RULE_EVEN_ODD
    Font:                 None
    Other Paremeters:     To Be Determined

  - Under X-Windows, end() will restore the GC to the state above; flags
    keeps track of which changes have been made to minimize the necessary
    updating.

  - Under X, graphics_exposures should be OFF:- at least some SGI IRIX machines
    have broken implementations of graphics_exposures.

  - Try suggestions from "Kevin Radke" <kmradke@isualum.com> below:

    Sorry about the huge delay with this code.  Work has been horribly busy
    and I wasn't able to dig up the CD with the original code.  However, if I
    remember
    correctly (and this code snippet I found was from that test), I changed from
    using
    cosmetic pens to use geometric pens and specifying a line join style of
    PS_JOIN_BEVEL.

    I.E.

        LOGBRUSH logBrush;
        logBrush.lbStyle = BS_SOLID;
        logBrush.lbColor = RGB(red, green, blue);
        logBrush.lbHatch = HS_CROSS;  // Not used

        // NYI Only solid lines are valid on Windows 95.  style is ignored
        return ExtCreatePen (PS_GEOMETRIC | PS_JOIN_BEVEL | style,
                                        width, &logBrush, 0, NULL);


    this returns an HPEN.

    I remember this being significantly slower than cosmetic pens (under NT4)
    and at the time that I scrapped the code, and dealt with the drawing
    differences
    between X and Win32 at a higher level.

    This won't work on Win95/Win98 (without using paths), and after a closer
    look at
    the docs here it makes more sense to specify PS_ENDCAP_SQUARE to draw
    the last pixel instead of the line join style.  I remember experimenting
    with both, so
    the code I found may have been in intermediate (unworking) version.

    In any case, it isn't too hard to experiment to see which has the required
    behavior.  I've unfortunately been away from FOX work for a few months
    or I'd try it myself.

  - Device caps for DirectX:
    http://www.molybdenium.de/devicecaps/e_index.html
*/

#define DISPLAY(app) ((Display*)((app)->display))


/********************************************************************************
*                                    X-Windows                                  *
********************************************************************************/

#ifndef WIN32


// Construct for expose event painting
FXDCWindow::FXDCWindow(FXDrawable* drawable,FXEvent* event):FXDC(drawable->getApp()){
  begin(drawable);
  rect.x=clip.x=event->rect.x;
  rect.y=clip.y=event->rect.y;
  rect.w=clip.w=event->rect.w;
  rect.h=clip.h=event->rect.h;
  XSetClipRectangles(DISPLAY(getApp()),(GC)gc,0,0,(XRectangle*)&clip,1,Unsorted);
  flags|=GCClipMask;
  }


// Construct for normal painting
FXDCWindow::FXDCWindow(FXDrawable* drawable):FXDC(drawable->getApp()){
  begin(drawable);
  }


// Destruct
FXDCWindow::~FXDCWindow(){
  end();
  }


// Begin locks in a drawable surface
void FXDCWindow::begin(FXDrawable *drawable){
  if(!drawable){ fxerror("FXDCWindow::begin: NULL drawable.\n"); }
  if(!drawable->id()){ fxerror("FXDCWindow::begin: drawable not created yet.\n"); }
  surface=drawable;
  visual=drawable->getVisual();
  rect.x=clip.x=0;
  rect.y=clip.y=0;
  rect.w=clip.w=drawable->getWidth();
  rect.h=clip.h=drawable->getHeight();
  devfg=~0;
  devbg=0;
  gc=visual->gc;
  flags=0;
  }


// End unlock the drawable surface; restore it
void FXDCWindow::end(){
  if(flags){
    XGCValues gcv;
    if(flags&GCFunction) gcv.function=BLT_SRC;
    if(flags&GCForeground) gcv.foreground=BlackPixel(DISPLAY(getApp()),DefaultScreen(DISPLAY(getApp())));
    if(flags&GCBackground) gcv.background=WhitePixel(DISPLAY(getApp()),DefaultScreen(DISPLAY(getApp())));
    if(flags&GCLineWidth) gcv.line_width=0;
    if(flags&GCCapStyle) gcv.cap_style=CAP_BUTT;
    if(flags&GCJoinStyle) gcv.join_style=JOIN_MITER;
    if(flags&GCLineStyle) gcv.line_style=LINE_SOLID;
    if(flags&GCFillStyle) gcv.fill_style=FILL_SOLID;
    if(flags&GCStipple) gcv.stipple=getApp()->stipples[STIPPLE_WHITE];    // Needed for IRIX6.4 bug workaround!
    if(flags&GCFillRule) gcv.fill_rule=RULE_EVEN_ODD;
    if(flags&GCFont) gcv.font=getApp()->getNormalFont()->id();
    if(flags&GCClipMask) gcv.clip_mask=None;
    if(flags&GCClipXOrigin) gcv.clip_x_origin=0;
    if(flags&GCClipYOrigin) gcv.clip_y_origin=0;
    if(flags&GCDashOffset) gcv.dash_offset=0;
    if(flags&GCDashList) gcv.dashes=4;
    if(flags&GCTileStipXOrigin) gcv.ts_x_origin=0;
    if(flags&GCTileStipYOrigin) gcv.ts_y_origin=0;
    if(flags&GCGraphicsExposures) gcv.graphics_exposures=True;
    if(flags&GCSubwindowMode) gcv.subwindow_mode=ClipByChildren;
    XChangeGC(DISPLAY(getApp()),(GC)gc,flags,&gcv);
    flags=0;
    }
  surface=NULL;
  }


// Read back pixel
FXColor FXDCWindow::readPixel(FXint x,FXint y){
  FXColor color=FXRGBA(0,0,0,0);
  if(!surface){ fxerror("FXDCWindow::readPixel: DC not connected to drawable.\n"); }
  if(0<=x && 0<=y && x<surface->getWidth() && y<surface->getHeight()){
    XImage* xim=XGetImage(DISPLAY(getApp()),surface->id(),x,y,1,1,AllPlanes,ZPixmap);
    if(xim && xim->data){
      color=visual->getColor(XGetPixel(xim,0,0));
      XDestroyImage(xim);
      }
    }
  return color;
  }


void FXDCWindow::drawPoint(FXint x,FXint y){
  if(!surface){ fxerror("FXDCWindow::drawPoint: DC not connected to drawable.\n"); }
  XDrawPoint(DISPLAY(getApp()),surface->id(),(GC)gc,x,y);
  }


void FXDCWindow::drawPoints(const FXPoint* points,FXuint npoints){
  if(!surface){ fxerror("FXDCWindow::drawPoints: DC not connected to drawable.\n"); }
  XDrawPoints(DISPLAY(getApp()),surface->id(),(GC)gc,(XPoint*)points,npoints,CoordModeOrigin);
  }


void FXDCWindow::drawPointsRel(const FXPoint* points,FXuint npoints){
  if(!surface){ fxerror("FXDCWindow::drawPointsRel: DC not connected to drawable.\n"); }
  XDrawPoints(DISPLAY(getApp()),surface->id(),(GC)gc,(XPoint*)points,npoints,CoordModePrevious);
  }


void FXDCWindow::drawLine(FXint x1,FXint y1,FXint x2,FXint y2){
  if(!surface){ fxerror("FXDCWindow::drawLine: DC not connected to drawable.\n"); }
  XDrawLine(DISPLAY(getApp()),surface->id(),(GC)gc,x1,y1,x2,y2);
  }


void FXDCWindow::drawLines(const FXPoint* points,FXuint npoints){
  if(!surface){ fxerror("FXDCWindow::drawLines: DC not connected to drawable.\n"); }
  XDrawLines(DISPLAY(getApp()),surface->id(),(GC)gc,(XPoint*)points,npoints,CoordModeOrigin);
  }


void FXDCWindow::drawLinesRel(const FXPoint* points,FXuint npoints){
  if(!surface){ fxerror("FXDCWindow::drawLinesRel: DC not connected to drawable.\n"); }
  XDrawLines(DISPLAY(getApp()),surface->id(),(GC)gc,(XPoint*)points,npoints,CoordModePrevious);
  }


void FXDCWindow::drawLineSegments(const FXSegment* segments,FXuint nsegments){
  if(!surface){ fxerror("FXDCWindow::drawLineSegments: DC not connected to drawable.\n"); }
  XDrawSegments(DISPLAY(getApp()),surface->id(),(GC)gc,(XSegment*)segments,nsegments);
  }


void FXDCWindow::drawRectangle(FXint x,FXint y,FXint w,FXint h){
  if(!surface){ fxerror("FXDCWindow::drawRectangle: DC not connected to drawable.\n"); }
  XDrawRectangle(DISPLAY(getApp()),surface->id(),(GC)gc,x,y,w,h);
  }


void FXDCWindow::drawRectangles(const FXRectangle* rectangles,FXuint nrectangles){
  if(!surface){ fxerror("FXDCWindow::drawRectangles: DC not connected to drawable.\n"); }
  XDrawRectangles(DISPLAY(getApp()),surface->id(),(GC)gc,(XRectangle*)rectangles,nrectangles);
  }


void FXDCWindow::drawArc(FXint x,FXint y,FXint w,FXint h,FXint ang1,FXint ang2){
  if(!surface){ fxerror("FXDCWindow::drawArc: DC not connected to drawable.\n"); }
  XDrawArc(DISPLAY(getApp()),surface->id(),(GC)gc,x,y,w,h,ang1,ang2);
  }


void FXDCWindow::drawArcs(const FXArc* arcs,FXuint narcs){
  if(!surface){ fxerror("FXDCWindow::drawArcs: DC not connected to drawable.\n"); }
  XDrawArcs(DISPLAY(getApp()),surface->id(),(GC)gc,(XArc*)arcs,narcs);
  }


void FXDCWindow::fillRectangle(FXint x,FXint y,FXint w,FXint h){
  if(!surface){ fxerror("FXDCWindow::fillRectangle: DC not connected to drawable.\n"); }
  XFillRectangle(DISPLAY(getApp()),surface->id(),(GC)gc,x,y,w,h);
  }


void FXDCWindow::fillRectangles(const FXRectangle* rectangles,FXuint nrectangles){
  if(!surface){ fxerror("FXDCWindow::fillRectangles: DC not connected to drawable.\n"); }
  XFillRectangles(DISPLAY(getApp()),surface->id(),(GC)gc,(XRectangle*)rectangles,nrectangles);
  }


void FXDCWindow::fillArc(FXint x,FXint y,FXint w,FXint h,FXint ang1,FXint ang2){
  if(!surface){ fxerror("FXDCWindow::fillArc: DC not connected to drawable.\n"); }
  XFillArc(DISPLAY(getApp()),surface->id(),(GC)gc,x,y,w,h,ang1,ang2);
  }


void FXDCWindow::fillArcs(const FXArc* arcs,FXuint narcs){
  if(!surface){ fxerror("FXDCWindow::fillArcs: DC not connected to drawable.\n"); }
  XFillArcs(DISPLAY(getApp()),surface->id(),(GC)gc,(XArc*)arcs,narcs);
  }


void FXDCWindow::fillPolygon(const FXPoint* points,FXuint npoints){
  if(!surface){ fxerror("FXDCWindow::fillArcs: DC not connected to drawable.\n"); }
  XFillPolygon(DISPLAY(getApp()),surface->id(),(GC)gc,(XPoint*)points,npoints,Convex,CoordModeOrigin);
  }


void FXDCWindow::fillConcavePolygon(const FXPoint* points,FXuint npoints){
  if(!surface){ fxerror("FXDCWindow::fillConcavePolygon: DC not connected to drawable.\n"); }
  XFillPolygon(DISPLAY(getApp()),surface->id(),(GC)gc,(XPoint*)points,npoints,Nonconvex,CoordModeOrigin);
  }


void FXDCWindow::fillComplexPolygon(const FXPoint* points,FXuint npoints){
  if(!surface){ fxerror("FXDCWindow::fillComplexPolygon: DC not connected to drawable.\n"); }
  XFillPolygon(DISPLAY(getApp()),surface->id(),(GC)gc,(XPoint*)points,npoints,Complex,CoordModeOrigin);
  }


void FXDCWindow::fillPolygonRel(const FXPoint* points,FXuint npoints){
  if(!surface){ fxerror("FXDCWindow::fillPolygonRel: DC not connected to drawable.\n"); }
  XFillPolygon(DISPLAY(getApp()),surface->id(),(GC)gc,(XPoint*)points,npoints,Convex,CoordModePrevious);
  }


void FXDCWindow::fillConcavePolygonRel(const FXPoint* points,FXuint npoints){
  if(!surface){ fxerror("FXDCWindow::fillConcavePolygonRel: DC not connected to drawable.\n"); }
  XFillPolygon(DISPLAY(getApp()),surface->id(),(GC)gc,(XPoint*)points,npoints,Nonconvex,CoordModePrevious);
  }


void FXDCWindow::fillComplexPolygonRel(const FXPoint* points,FXuint npoints){
  if(!surface){ fxerror("FXDCWindow::fillComplexPolygonRel: DC not connected to drawable.\n"); }
  XFillPolygon(DISPLAY(getApp()),surface->id(),(GC)gc,(XPoint*)points,npoints,Complex,CoordModePrevious);
  }


void FXDCWindow::drawText(FXint x,FXint y,const FXchar* string,FXuint length){
  if(!surface){ fxerror("FXDCWindow::drawText: DC not connected to drawable.\n"); }
  XDrawString(DISPLAY(getApp()),surface->id(),(GC)gc,x,y,(char*)string,length);
  }


void FXDCWindow::drawImageText(FXint x,FXint y,const FXchar* string,FXuint length){
  if(!surface){ fxerror("FXDCWindow::drawImageText: DC not connected to drawable.\n"); }
  XDrawImageString(DISPLAY(getApp()),surface->id(),(GC)gc,x,y,(char*)string,length);
  }


void FXDCWindow::drawArea(const FXDrawable* source,FXint sx,FXint sy,FXint sw,FXint sh,FXint dx,FXint dy){
  if(!surface){ fxerror("FXDCWindow::drawArea: DC not connected to drawable.\n"); }
  if(!source || !source->id()){ fxerror("FXDCWindow::drawArea: illegal source specified.\n"); }
  XCopyArea(DISPLAY(getApp()),source->id(),surface->id(),(GC)gc,sx,sy,sw,sh,dx,dy);
  }


void FXDCWindow::drawImage(const FXImage* image,FXint dx,FXint dy){
  if(!surface){ fxerror("FXDCWindow::drawImage: DC not connected to drawable.\n"); }
  if(!image || !image->id()){ fxerror("FXDCWindow::drawImage: illegal image specified.\n"); }
  XCopyArea(DISPLAY(getApp()),image->id(),surface->id(),(GC)gc,0,0,image->width,image->height,dx,dy);
  }


void FXDCWindow::drawBitmap(const FXBitmap* bitmap,FXint dx,FXint dy) {
  if(!surface) fxerror("FXDCWindow::drawBitmap: DC not connected to drawable.\n");
  if(!bitmap || !bitmap->id()) fxerror("FXDCWindow::drawBitmap: illegal bitmap specified.\n");
  XCopyPlane(DISPLAY(getApp()),bitmap->id(),surface->id(),(GC)gc,0,0,bitmap->width,bitmap->height,dx,dy,1);
  }


// Draw a vanilla icon
void FXDCWindow::drawIcon(const FXIcon* icon,FXint dx,FXint dy){
  if(!surface){ fxerror("FXDCWindow::drawIcon: DC not connected to drawable.\n"); }
  if(!icon || !icon->id() || !icon->shape){ fxerror("FXDCWindow::drawIcon: illegal icon specified.\n"); }
  FXRectangle d=clip*FXRectangle(dx,dy,icon->width,icon->height);
  if(d.w>0 && d.h>0){
    if(icon->getOptions()&IMAGE_OPAQUE){
      XCopyArea(DISPLAY(getApp()),icon->id(),surface->id(),(GC)gc,d.x-dx,d.y-dy,d.w,d.h,d.x,d.y);
      }
    else{
      XGCValues gcv;
      gcv.clip_mask=icon->shape;
      gcv.clip_x_origin=dx;
      gcv.clip_y_origin=dy;
      XChangeGC(DISPLAY(getApp()),(GC)gc,GCClipMask|GCClipXOrigin|GCClipYOrigin,&gcv);
      XCopyArea(DISPLAY(getApp()),icon->id(),surface->id(),(GC)gc,d.x-dx,d.y-dy,d.w,d.h,d.x,d.y);
      XSetClipRectangles(DISPLAY(getApp()),(GC)gc,0,0,(XRectangle*)&clip,1,Unsorted); // Restore old clip rectangle
      flags|=GCClipMask;
      }
    }
  }


// Draw a shaded icon, like when it is selected
void FXDCWindow::drawIconShaded(const FXIcon* icon,FXint dx,FXint dy){
  if(!surface){ fxerror("FXDCWindow::drawIconShaded: DC not connected to drawable.\n"); }
  if(!icon || !icon->id() || !icon->shape){ fxerror("FXDCWindow::drawIconShaded: illegal icon specified.\n"); }
  FXRectangle d=clip*FXRectangle(dx,dy,icon->width,icon->height);
  if(d.w>0 && d.h>0){
    XGCValues gcv;
    gcv.clip_mask=icon->shape;
    gcv.clip_x_origin=dx;
    gcv.clip_y_origin=dy;
    XChangeGC(DISPLAY(getApp()),(GC)gc,GCClipMask|GCClipXOrigin|GCClipYOrigin,&gcv);
    XCopyArea(DISPLAY(getApp()),icon->id(),surface->id(),(GC)gc,d.x-dx,d.y-dy,d.w,d.h,d.x,d.y);
    gcv.function=BLT_SRC;
    gcv.stipple=getApp()->stipples[STIPPLE_GRAY];
    gcv.fill_style=FILL_STIPPLED;
    gcv.ts_x_origin=dx;
    gcv.ts_y_origin=dy;
    gcv.foreground=visual->getPixel(getApp()->getSelbackColor());
    XChangeGC(DISPLAY(getApp()),(GC)gc,GCForeground|GCFunction|GCTileStipXOrigin|GCTileStipYOrigin|GCStipple|GCFillStyle,&gcv);
    XFillRectangle(DISPLAY(getApp()),surface->id(),(GC)gc,d.x,d.y,d.w,d.h);
    gcv.function=rop;
    gcv.fill_style=fill;
    gcv.ts_x_origin=tx;
    gcv.ts_y_origin=ty;
    XChangeGC(DISPLAY(getApp()),(GC)gc,GCTileStipXOrigin|GCTileStipYOrigin|GCFunction|GCFillStyle,&gcv);  // Restore old raster op function and fill style
    XSetClipRectangles(DISPLAY(getApp()),(GC)gc,0,0,(XRectangle*)&clip,1,Unsorted); // Restore old clip rectangle
    flags|=GCClipMask;
    }
  }


// This draws a sunken icon
void FXDCWindow::drawIconSunken(const FXIcon* icon,FXint dx,FXint dy){
  if(!surface){ fxerror("FXDCWindow::drawIconSunken: DC not connected to drawable.\n"); }
  if(!icon || !icon->id() || !icon->etch){ fxerror("FXDCWindow::drawIconSunken: illegal icon specified.\n"); }
  XGCValues gcv;
  FXColor base=getApp()->getBaseColor();
  FXColor clr=FXRGB((85*FXREDVAL(base))/100,(85*FXGREENVAL(base))/100,(85*FXBLUEVAL(base))/100);

  // Erase to black
  gcv.background=0;
  gcv.foreground=0xffffffff;
  gcv.function=BLT_NOT_SRC_AND_DST;
  XChangeGC(DISPLAY(getApp()),(GC)gc,GCForeground|GCBackground|GCFunction,&gcv);
  XCopyPlane(DISPLAY(getApp()),icon->etch,surface->id(),(GC)gc,0,0,icon->width,icon->height,dx+1,dy+1,1);

  // Paint highlight part
  gcv.function=BLT_SRC_OR_DST;
  gcv.foreground=visual->getPixel(getApp()->getHiliteColor());
  XChangeGC(DISPLAY(getApp()),(GC)gc,GCForeground|GCFunction,&gcv);
  XCopyPlane(DISPLAY(getApp()),icon->etch,surface->id(),(GC)gc,0,0,icon->width,icon->height,dx+1,dy+1,1);

  // Erase to black
  gcv.foreground=0xffffffff;
  gcv.function=BLT_NOT_SRC_AND_DST;
  XChangeGC(DISPLAY(getApp()),(GC)gc,GCForeground|GCFunction,&gcv);
  XCopyPlane(DISPLAY(getApp()),icon->etch,surface->id(),(GC)gc,0,0,icon->width,icon->height,dx,dy,1);

  // Paint shadow part
  gcv.function=BLT_SRC_OR_DST;
  gcv.foreground=visual->getPixel(clr);
  XChangeGC(DISPLAY(getApp()),(GC)gc,GCForeground|GCFunction,&gcv);
  XCopyPlane(DISPLAY(getApp()),icon->etch,surface->id(),(GC)gc,0,0,icon->width,icon->height,dx,dy,1);

  // Restore stuff
  gcv.foreground=devfg;
  gcv.background=devbg;
  gcv.function=rop;
  XChangeGC(DISPLAY(getApp()),(GC)gc,GCForeground|GCBackground|GCFunction,&gcv);
  }


void FXDCWindow::drawHashBox(FXint x,FXint y,FXint w,FXint h,FXint b){
  XGCValues gcv;
  if(!surface){ fxerror("FXDCWindow::drawHashBox: DC not connected to drawable.\n"); }
  gcv.stipple=getApp()->stipples[STIPPLE_GRAY];
  gcv.fill_style=FILL_STIPPLED;
  XChangeGC(DISPLAY(getApp()),(GC)gc,GCStipple|GCFillStyle,&gcv);
  XFillRectangle(DISPLAY(getApp()),surface->id(),(GC)gc,x,y,w-b,b);
  XFillRectangle(DISPLAY(getApp()),surface->id(),(GC)gc,x+w-b,y,b,h-b);
  XFillRectangle(DISPLAY(getApp()),surface->id(),(GC)gc,x+b,y+h-b,w-b,b);
  XFillRectangle(DISPLAY(getApp()),surface->id(),(GC)gc,x,y+b,b,h-b);
  gcv.stipple=getApp()->stipples[STIPPLE_WHITE];    // Needed for IRIX6.4 bug workaround!
  gcv.fill_style=fill;
  XChangeGC(DISPLAY(getApp()),(GC)gc,GCStipple|GCFillStyle,&gcv);
  }


void FXDCWindow::drawFocusRectangle(FXint x,FXint y,FXint w,FXint h){
  XGCValues gcv;
  if(!surface){ fxerror("FXDCWindow::drawFocusRectangle: DC not connected to drawable.\n"); }
  gcv.stipple=getApp()->stipples[STIPPLE_GRAY];
  gcv.fill_style=FILL_STIPPLED;
  gcv.background=0;
  gcv.foreground=0xffffffff;    // Maybe should use FILL_OPAQUESTIPPLED and current fg/bg color and BLT_SRC
  gcv.function=BLT_SRC_XOR_DST; // This would be more flexible
  gcv.ts_x_origin=x;
  gcv.ts_y_origin=y;
  XChangeGC(DISPLAY(getApp()),(GC)gc,GCTileStipXOrigin|GCTileStipYOrigin|GCForeground|GCBackground|GCFunction|GCStipple|GCFillStyle,&gcv);
  XFillRectangle(DISPLAY(getApp()),surface->id(),(GC)gc,x,y,w-1,1);
  XFillRectangle(DISPLAY(getApp()),surface->id(),(GC)gc,x+w-1,y,1,h-1);
  XFillRectangle(DISPLAY(getApp()),surface->id(),(GC)gc,x+1,y+h-1,w-1,1);
  XFillRectangle(DISPLAY(getApp()),surface->id(),(GC)gc,x,y+1,1,h-1);
  gcv.stipple=getApp()->stipples[STIPPLE_WHITE];    // Needed for IRIX6.4 bug workaround!
  gcv.fill_style=fill;
  gcv.background=devbg;
  gcv.foreground=devfg;
  gcv.function=rop;
  gcv.ts_x_origin=tx;
  gcv.ts_y_origin=ty;
  XChangeGC(DISPLAY(getApp()),(GC)gc,GCTileStipXOrigin|GCTileStipYOrigin|GCForeground|GCBackground|GCFunction|GCStipple|GCFillStyle,&gcv);
  }


void FXDCWindow::setForeground(FXColor clr){
  if(!surface){ fxerror("FXDCWindow::setForeground: DC not connected to drawable.\n"); }
  devfg=visual->getPixel(clr);
  XSetForeground(DISPLAY(getApp()),(GC)gc,devfg);
  flags|=GCForeground;
  fg=clr;
  }


void FXDCWindow::setBackground(FXColor clr){
  if(!surface){ fxerror("FXDCWindow::setBackground: DC not connected to drawable.\n"); }
  devbg=visual->getPixel(clr);
  XSetBackground(DISPLAY(getApp()),(GC)gc,devbg);
  flags|=GCBackground;
  bg=clr;
  }


void FXDCWindow::setDashes(FXuint dashoffset,const FXchar *dashpattern,FXuint dashlength){
  register FXuint len,i;
  if(!surface){ fxerror("FXDCWindow::setDashes: DC not connected to drawable.\n"); }
  for(i=len=0; i<dashlength; i++){
    dashpat[i]=dashpattern[i];
    len+=(FXuint)dashpattern[i];
    }
  dashlen=dashlength;
  dashoff=dashoffset%len;
  XSetDashes(DISPLAY(getApp()),(GC)gc,dashoff,(char*)dashpat,dashlen);
  flags|=(GCDashList|GCDashOffset);
  }


void FXDCWindow::setLineWidth(FXuint linewidth){
  XGCValues gcv;
  if(!surface){ fxerror("FXDCWindow::setLineWidth: DC not connected to drawable.\n"); }
  gcv.line_width=linewidth;
  XChangeGC(DISPLAY(getApp()),(GC)gc,GCLineWidth,&gcv);
  flags|=GCLineWidth;
  width=linewidth;
  }


void FXDCWindow::setLineCap(FXCapStyle capstyle){
  XGCValues gcv;
  if(!surface){ fxerror("FXDCWindow::setLineCap: DC not connected to drawable.\n"); }
  gcv.cap_style=capstyle;
  XChangeGC(DISPLAY(getApp()),(GC)gc,GCCapStyle,&gcv);
  flags|=GCCapStyle;
  cap=capstyle;
  }


void FXDCWindow::setLineJoin(FXJoinStyle joinstyle){
  XGCValues gcv;
  if(!surface){ fxerror("FXDCWindow::setLineJoin: DC not connected to drawable.\n"); }
  gcv.join_style=joinstyle;
  XChangeGC(DISPLAY(getApp()),(GC)gc,GCJoinStyle,&gcv);
  flags|=GCJoinStyle;
  join=joinstyle;
  }


void FXDCWindow::setLineStyle(FXLineStyle linestyle){
  XGCValues gcv;
  if(!surface){ fxerror("FXDCWindow::setLineStyle: DC not connected to drawable.\n"); }
  gcv.line_style=linestyle;
  XChangeGC(DISPLAY(getApp()),(GC)gc,GCLineStyle,&gcv);
  flags|=GCLineStyle;
  style=linestyle;
  }


void FXDCWindow::setFillStyle(FXFillStyle fillstyle){
  if(!surface){ fxerror("FXDCWindow::setFillStyle: DC not connected to drawable.\n"); }
  XSetFillStyle(DISPLAY(getApp()),(GC)gc,fillstyle);
  flags|=GCFillStyle;
  fill=fillstyle;
  }


void FXDCWindow::setFillRule(FXFillRule fillrule){
  if(!surface){ fxerror("FXDCWindow::setFillRule: DC not connected to drawable.\n"); }
  XSetFillRule(DISPLAY(getApp()),(GC)gc,fillrule);
  flags|=GCFillRule;
  rule=fillrule;
  }


void FXDCWindow::setFunction(FXFunction func){
  if(!surface){ fxerror("FXDCWindow::setFunction: DC not connected to drawable.\n"); }
  XSetFunction(DISPLAY(getApp()),(GC)gc,func);
  flags|=GCFunction;
  rop=func;
  }


void FXDCWindow::setTile(FXImage* image,FXint dx,FXint dy){
  XGCValues gcv;
  if(!surface){ fxerror("FXDCWindow::setTile: DC not connected to drawable.\n"); }
  if(!image || !image->id()){ fxerror("FXDCWindow::setTile: illegal image specified.\n"); }
  gcv.tile=image->id();
  gcv.ts_x_origin=dx;
  gcv.ts_y_origin=dy;
  XChangeGC(DISPLAY(getApp()),(GC)gc,GCTileStipXOrigin|GCTileStipYOrigin|GCTile,&gcv);
  if(dx) flags|=GCTileStipXOrigin;
  if(dy) flags|=GCTileStipYOrigin;
  tile=image;
  tx=dx;
  ty=dy;
  }


void FXDCWindow::setStipple(FXBitmap* bitmap,FXint dx,FXint dy){
  XGCValues gcv;
  if(!surface){ fxerror("FXDCWindow::setStipple: DC not connected to drawable.\n"); }
  if(!bitmap || !bitmap->id()){ fxerror("FXDCWindow::setStipple: illegal image specified.\n"); }
  gcv.stipple=bitmap->id();
  gcv.ts_x_origin=dx;
  gcv.ts_y_origin=dy;
  XChangeGC(DISPLAY(getApp()),(GC)gc,GCTileStipXOrigin|GCTileStipYOrigin|GCStipple,&gcv);
  if(dx) flags|=GCTileStipXOrigin;
  if(dy) flags|=GCTileStipYOrigin;
  flags|=GCStipple;
  stipple=bitmap;
  pattern=STIPPLE_NONE;
  tx=dx;
  ty=dy;
  }


void FXDCWindow::setStipple(FXStipplePattern pat,FXint dx,FXint dy){
  XGCValues gcv;
  if(!surface){ fxerror("FXDCWindow::setStipple: DC not connected to drawable.\n"); }
  if(pat>STIPPLE_CROSSDIAG) pat=STIPPLE_CROSSDIAG;
  FXASSERT(getApp()->stipples[pat]);
  gcv.stipple=getApp()->stipples[pat];
  gcv.ts_x_origin=dx;
  gcv.ts_y_origin=dy;
  XChangeGC(DISPLAY(getApp()),(GC)gc,GCTileStipXOrigin|GCTileStipYOrigin|GCStipple,&gcv);
  if(dx) flags|=GCTileStipXOrigin;
  if(dy) flags|=GCTileStipYOrigin;
  stipple=NULL;
  pattern=pat;
  flags|=GCStipple;
  tx=dx;
  ty=dy;
  }


void FXDCWindow::setClipRegion(const FXRegion& region){
  if(!surface){ fxerror("FXDCWindow::setClipRegion: DC not connected to drawable.\n"); }
  XSetRegion(DISPLAY(getApp()),(GC)gc,(Region)region.region);///// Should intersect region and rect??
  flags|=GCClipMask;
  }


void FXDCWindow::setClipRectangle(FXint x,FXint y,FXint w,FXint h){
  if(!surface){ fxerror("FXDCWindow::setClipRectangle: DC not connected to drawable.\n"); }
  clip.x=FXMAX(x,rect.x);
  clip.y=FXMAX(y,rect.y);
  clip.w=FXMIN(x+w,rect.x+rect.w)-clip.x;
  clip.h=FXMIN(y+h,rect.y+rect.h)-clip.y;
  if(clip.w<=0) clip.w=0;
  if(clip.h<=0) clip.h=0;
  XSetClipRectangles(DISPLAY(getApp()),(GC)gc,0,0,(XRectangle*)&clip,1,Unsorted);
  flags|=GCClipMask;
  }


void FXDCWindow::setClipRectangle(const FXRectangle& rectangle){
  if(!surface){ fxerror("FXDCWindow::setClipRectangle: DC not connected to drawable.\n"); }
  clip.x=FXMAX(rectangle.x,rect.x);
  clip.y=FXMAX(rectangle.y,rect.y);
  clip.w=FXMIN(rectangle.x+rectangle.w,rect.x+rect.w)-clip.x;
  clip.h=FXMIN(rectangle.y+rectangle.h,rect.y+rect.h)-clip.y;
  if(clip.w<=0) clip.w=0;
  if(clip.h<=0) clip.h=0;
  XSetClipRectangles(DISPLAY(getApp()),(GC)gc,0,0,(XRectangle*)&clip,1,Unsorted);
  flags|=GCClipMask;
  }


void FXDCWindow::clearClipRectangle(){
  if(!surface){ fxerror("FXDCWindow::clearClipRectangle: DC not connected to drawable.\n"); }
  clip=rect;
  XSetClipRectangles(DISPLAY(getApp()),(GC)gc,0,0,(XRectangle*)&clip,1,Unsorted);
  flags|=GCClipMask;
  }


void FXDCWindow::setClipMask(FXBitmap* bitmap,FXint dx,FXint dy){
  XGCValues gcv;
  if(!surface){ fxerror("FXDCWindow::setClipMask: DC not connected to drawable.\n"); }
  if(!bitmap || !bitmap->id()){ fxerror("FXDCWindow::setClipMask: illegal mask specified.\n"); }
  gcv.clip_mask=bitmap->id();
  gcv.clip_x_origin=dx;
  gcv.clip_y_origin=dy;
  XChangeGC(DISPLAY(getApp()),(GC)gc,GCClipMask|GCClipXOrigin|GCClipYOrigin,&gcv);
  if(dx) flags|=GCClipXOrigin;
  if(dy) flags|=GCClipYOrigin;
  flags|=GCClipMask;
  mask=bitmap;
  cx=dx;
  cy=dy;
  }


void FXDCWindow::clearClipMask(){
  if(!surface){ fxerror("FXDCWindow::clearClipMask: DC not connected to drawable.\n"); }
  clip=rect;
  XSetClipRectangles(DISPLAY(getApp()),(GC)gc,0,0,(XRectangle*)&clip,1,Unsorted);
  flags|=GCClipMask;
  mask=NULL;
  cx=0;
  cy=0;
  }


void FXDCWindow::setTextFont(FXFont *fnt){
  if(!surface){ fxerror("FXDCWindow::setTextFont: DC not connected to drawable.\n"); }
  if(!fnt || !fnt->id()){ fxerror("FXDCWindow::setTextFont: illegal or NULL font specified.\n"); }
  XSetFont(DISPLAY(getApp()),(GC)gc,fnt->id());
  flags|=GCFont;
  font=fnt;
  }


void FXDCWindow::clipChildren(FXbool yes){
  if(!surface){ fxerror("FXDCWindow::clipChildren: window has not yet been created.\n"); }
  if(yes){
    XSetSubwindowMode(DISPLAY(getApp()),(GC)gc,ClipByChildren);
    flags&=~GCSubwindowMode;
    }
  else{
    XSetSubwindowMode(DISPLAY(getApp()),(GC)gc,IncludeInferiors);
    flags|=GCSubwindowMode;
    }
  }


/********************************************************************************
*                                   MS-Windows                                  *
********************************************************************************/

#else

// This one is not defined in the Cygwin header files
#ifndef PS_JOIN_MASK
#define PS_JOIN_MASK 0x0000F000
#endif

// Construct for expose event painting
FXDCWindow::FXDCWindow(FXDrawable* drawable,FXEvent* event):FXDC(drawable->getApp()){
  oldpalette=NULL;
  oldbrush=NULL;
  oldpen=NULL;
  needsNewBrush=FALSE;
  needsNewPen=FALSE;
  needsPath=FALSE;
  begin(drawable);
  rect.x=clip.x=event->rect.x;
  rect.y=clip.y=event->rect.y;
  rect.w=clip.w=event->rect.w;
  rect.h=clip.h=event->rect.h;
  HRGN hrgn=CreateRectRgn(clip.x,clip.y,clip.x+clip.w,clip.y+clip.h);
  SelectClipRgn((HDC)dc,hrgn);
  DeleteObject(hrgn);
  }


// Construct for normal painting
FXDCWindow::FXDCWindow(FXDrawable* drawable):FXDC(drawable->getApp()){
  oldpalette=NULL;
  oldbrush=NULL;
  oldpen=NULL;
  needsNewBrush=FALSE;
  needsNewPen=FALSE;
  needsPath=FALSE;
  begin(drawable);
  }


// Destruct
FXDCWindow::~FXDCWindow(){
  end();
  }


// Begin locks in a drawable surface
void FXDCWindow::begin(FXDrawable *drawable){
  if(!drawable){ fxerror("FXDCWindow::begin: NULL drawable.\n"); }
  if(!drawable->id()){ fxerror("FXDCWindow::begin: drawable not created yet.\n"); }

  surface=drawable;// Careful:- surface->id() can be HWND or HBITMAP depending on drawable
  visual=drawable->getVisual();
  dc=drawable->GetDC();
  rect.x=clip.x=0;
  rect.y=clip.y=0;
  rect.w=clip.w=drawable->getWidth();
  rect.h=clip.h=drawable->getHeight();

  // Select and realize palette, if necessary
  if(visual->hPalette){oldpalette=SelectPalette((HDC)dc,(HPALETTE)visual->hPalette,FALSE);RealizePalette((HDC)dc);}

  devfg=~0;
  devbg=0;

  // Create our default pen (black, solid, one pixel wide)
  LOGBRUSH lb;
  lb.lbStyle=BS_SOLID;
  lb.lbColor=PALETTERGB(0,0,0);
  lb.lbHatch=0;
  oldpen=SelectObject((HDC)dc,ExtCreatePen(PS_GEOMETRIC|PS_SOLID|PS_ENDCAP_FLAT|PS_JOIN_MITER,1,&lb,0,NULL));

  // Create our default brush (solid white, for fills)
  lb.lbStyle=BS_SOLID;
  lb.lbColor=PALETTERGB(255,255,255);
  lb.lbHatch=0;
  oldbrush=SelectObject((HDC)dc,CreateBrushIndirect(&lb));

  // Text alignment
  SetTextAlign((HDC)dc,TA_BASELINE|TA_LEFT);

  // Polygon fill mode
  SetPolyFillMode((HDC)dc,ALTERNATE);

  }


// End unlocks the drawable surface
void FXDCWindow::end(){
  if(dc){
    DeleteObject(SelectObject((HDC)dc,oldpen));
    DeleteObject(SelectObject((HDC)dc,oldbrush));
    if(visual->hPalette){SelectPalette((HDC)dc,(HPALETTE)oldpalette,FALSE);}
    surface->ReleaseDC((HDC)dc);
    DWORD dwFlags=GetWindowLong((HWND)surface->id(),GWL_STYLE);
    SetWindowLong((HWND)surface->id(),GWL_STYLE,dwFlags|WS_CLIPCHILDREN);
    dc=0;
    }
  surface=NULL;
  }


// Read back pixel
FXColor FXDCWindow::readPixel(FXint x,FXint y){
  FXColor color=FXRGBA(0,0,0,0);
  if(!surface){ fxerror("FXDCWindow::readPixel: DC not connected to drawable.\n"); }
  if(0<=x && 0<=y && x<surface->getWidth() && y<surface->getHeight()){
    COLORREF clr=GetPixel((HDC)dc,x,y);
    color=FXRGB(GetRValue(clr),GetGValue(clr),GetBValue(clr));
    }
  return color;
  }


// Draw pixel in current foreground color
void FXDCWindow::drawPoint(FXint x,FXint y){
  if(!surface){ fxerror("FXDCWindow::drawPoint: DC not connected to drawable.\n"); }
  SetPixel((HDC)dc,x,y,devfg);
  }


void FXDCWindow::drawPoints(const FXPoint* points,FXuint npoints){
  register FXuint i;
  if(!surface){ fxerror("FXDCWindow::drawPoints: DC not connected to drawable.\n"); }
  for(i=0; i<npoints; i++){
    SetPixel((HDC)dc,points[i].x,points[i].y,devfg);
    }
  }


void FXDCWindow::drawPointsRel(const FXPoint* points,FXuint npoints){
  register int x=0,y=0;
  register FXuint i;
  if(!surface){ fxerror("FXDCWindow::drawPointsRel: DC not connected to drawable.\n"); }
  for(i=0; i<npoints; i++){
    x+=points[i].x;
    y+=points[i].y;
    SetPixel((HDC)dc,x,y,devfg);
    }
  }


void FXDCWindow::drawLine(FXint x1,FXint y1,FXint x2,FXint y2){
//  register FXint dx,dy,adx,ady;
  if(!surface){ fxerror("FXDCWindow::drawLine: DC not connected to drawable.\n"); }
  if(needsNewPen) updatePen();
  if(needsPath){
    BeginPath((HDC)dc);
    }
  POINT pts[2];
  pts[0].x=x1; pts[0].y=y1;
  pts[1].x=x2; pts[1].y=y2;
  Polyline((HDC)dc,pts,2);

/*
  MoveToEx((HDC)dc,x1,y1,NULL);
  adx=dx=x2-x1; if(adx<0) adx=-adx;
  ady=dy=y2-y1; if(ady<0) ady=-ady;
  if(adx>ady){
    if(dx<0) x2--; else x2++;
    }
  else if(adx<ady){
    if(dy<0) y2--; else y2++;
    }
  else{
    if(dx<0) x2--; else if(dx>0) x2++;
    if(dy<0) y2--; else if(dy>0) y2++;
    }
  LineTo((HDC)dc,x2,y2);
*/
  if(needsPath){
    EndPath((HDC)dc);
    StrokePath((HDC)dc);
    }
  }


void FXDCWindow::drawLines(const FXPoint* points,FXuint npoints){
  register FXuint i;
  if(!surface){ fxerror("FXDCWindow::drawLines: DC not connected to drawable.\n"); }
  if(needsNewPen) updatePen();
  MoveToEx((HDC)dc,points[0].x,points[0].y,NULL);
  if(needsPath){
    BeginPath((HDC)dc);
    }
  if((npoints>8192) || (npoints<4)){
    MoveToEx((HDC)dc,points[0].x,points[0].y,NULL);
    for(i=1; i<npoints; i++) LineTo((HDC)dc,points[i].x,points[i].y);
    }
  else{
    POINT longPoints[8192];
    for(i=0; i<npoints; i++){
      longPoints[i].x = points[i].x;
      longPoints[i].y = points[i].y;
      }
    Polyline((HDC)dc,longPoints,npoints);
    //MoveToEx((HDC)dc,longPoints[npoints-1].x,longPoints[npoints-1].y,NULL);
    }
  //for(FXuint i=1; i<npoints; i++) LineTo((HDC)dc,points[i].x,points[i].y);
  if(needsPath){
    EndPath((HDC)dc);
    StrokePath((HDC)dc);
    }
  }


void FXDCWindow::drawLinesRel(const FXPoint* points,FXuint npoints){
  register FXuint i;
  register int x=0,y=0;
  POINT pts[2048];
  if(!surface){ fxerror("FXDCWindow::drawLinesRel: DC not connected to drawable.\n"); }
  if(needsNewPen) updatePen();
//  int x=points[0].x,y=points[0].y;
  if(needsPath){
    BeginPath((HDC)dc);
    }
  for(i=0; i<npoints; i++){
    x+=points[i].x; pts[i].x=x;
    y+=points[i].y; pts[i].y=y;
    }
  Polyline((HDC)dc,pts,npoints);
/*
  MoveToEx((HDC)dc,x,y,NULL);
  for(i=1; i < npoints; i++){
    x+=points[i].x;
    y+=points[i].y;
    LineTo((HDC)dc,x,y);
    }
*/
  if(needsPath){
    EndPath((HDC)dc);
    StrokePath((HDC)dc);
    }
  }


void FXDCWindow::drawLineSegments(const FXSegment* segments,FXuint nsegments){
  register FXuint i;
  POINT pts[2];
  if(!surface){ fxerror("FXDCWindow::drawLineSegments: DC not connected to drawable.\n"); }
  if(needsNewPen) updatePen();
  if(needsPath){
    BeginPath((HDC)dc);
    }
  for(i=0; i<nsegments; i++){
    pts[0].x=segments[i].x1; pts[0].y=segments[i].y1;
    pts[1].x=segments[i].x2; pts[1].y=segments[i].y2;
    Polyline((HDC)dc,pts,2);
    }
  if(needsPath){
    EndPath((HDC)dc);
    StrokePath((HDC)dc);
    }
  }


// Unfilled rectangle
void FXDCWindow::drawRectangle(FXint x,FXint y,FXint w,FXint h){
  if(!surface){ fxerror("FXDCWindow::drawRectangle: DC not connected to drawable.\n"); }
  if(needsNewPen) updatePen();
  HBRUSH hBrush=(HBRUSH)SelectObject((HDC)dc,(HBRUSH)GetStockObject(NULL_BRUSH));
  Rectangle((HDC)dc,x,y,x+w+1,y+h+1);
  SelectObject((HDC)dc,hBrush);
  }


void FXDCWindow::drawRectangles(const FXRectangle* rectangles,FXuint nrectangles){
  register FXuint i;
  if(!surface){ fxerror("FXDCWindow::drawRectangles: DC not connected to drawable.\n"); }
  if(needsNewPen) updatePen();
  HBRUSH hBrush=(HBRUSH)SelectObject((HDC)dc,(HBRUSH)GetStockObject(NULL_BRUSH));
  for(i=0; i<nrectangles; i++){
    Rectangle((HDC)dc,rectangles[i].x,rectangles[i].y,rectangles[i].x+rectangles[i].w+1,rectangles[i].y+rectangles[i].h+1);
    }
  SelectObject((HDC)dc,hBrush);
  }


// Draw arc; angles in degrees*64, ang2 relative to ang1
// If angle is negative flip the start and end; also, if ang2 is zero,
// don't draw anything at all (patch: Sander Jansen <sxj@cfdrc.com>).
void FXDCWindow::drawArc(FXint x,FXint y,FXint w,FXint h,FXint ang1,FXint ang2){
  register FXbool reversed=(ang2<0);
  if(!surface){ fxerror("FXDCWindow::drawArc: DC not connected to drawable.\n"); }
  if(ang2==0) return;
  if(needsNewPen) updatePen();
  ang2+=ang1;
  w+=1;
  h+=1;
  int xStart=int(x+0.5*w+w*cos(ang1*PI/(180.0*64.0)));
  int yStart=int(y+0.5*h-h*sin(ang1*PI/(180.0*64.0)));
  int xEnd=int(x+0.5*w+w*cos(ang2*PI/(180.0*64.0)));
  int yEnd=int(y+0.5*h-h*sin(ang2*PI/(180.0*64.0)));
  if(needsPath){
    BeginPath((HDC)dc);
    }
  if(reversed)
    Arc((HDC)dc,x,y,x+w,y+h,xEnd,yEnd,xStart,yStart);
  else
    Arc((HDC)dc,x,y,x+w,y+h,xStart,yStart,xEnd,yEnd);
  if(needsPath){
    EndPath((HDC)dc);
    StrokePath((HDC)dc);
    }
  }


void FXDCWindow::drawArcs(const FXArc* arcs,FXuint narcs){
  register FXuint i;
  if(!surface){ fxerror("FXDCWindow::drawArcs: DC not connected to drawable.\n"); }
  for(i=0; i<narcs; i++){
    drawArc(arcs[i].x,arcs[i].y,arcs[i].w,arcs[i].h,arcs[i].a,arcs[i].b);
    }
  }


// Fill using currently selected ROP code
void FXDCWindow::fillRectangle(FXint x,FXint y,FXint w,FXint h){
  if(!surface){ fxerror("FXDCWindow::fillRectangle: DC not connected to drawable.\n"); }
  if(needsNewBrush) updateBrush();
  HPEN hPen=(HPEN)SelectObject((HDC)dc,GetStockObject(NULL_PEN));
  Rectangle((HDC)dc,x,y,x+w+1,y+h+1);
  SelectObject((HDC)dc,hPen);
  }


// Fill using currently selected ROP code
void FXDCWindow::fillRectangles(const FXRectangle* rectangles,FXuint nrectangles){
  register FXuint i;
  if(!surface){ fxerror("FXDCWindow::fillRectangles: DC not connected to drawable.\n"); }
  if(needsNewBrush) updateBrush();
  HPEN hPen=(HPEN)SelectObject((HDC)dc,GetStockObject(NULL_PEN));
  for(i=0; i<nrectangles; i++){
    Rectangle((HDC)dc,rectangles[i].x,rectangles[i].y,rectangles[i].x+rectangles[i].w+1,rectangles[i].y+rectangles[i].h+1);
    }
  SelectObject((HDC)dc,hPen);
  }


// Draw filled arc; angles are in degrees*64; ang2 is relative from ang1
// If angle is negative flip the start and end; also, if ang2 is zero,
// don't draw anything at all (patch: Sander Jansen <sxj@cfdrc.com>).
void FXDCWindow::fillArc(FXint x,FXint y,FXint w,FXint h,FXint ang1,FXint ang2){
  register FXbool reversed=(ang2<0);
  if(!surface){ fxerror("FXDCWindow::fillArc: DC not connected to drawable.\n"); }
  if(ang2==0) return;
  if(needsNewBrush) updateBrush();
  ang2+=ang1;
  w+=1;
  h+=1;
  int xStart=int(x+0.5*w+w*cos(ang1*PI/(180.0*64.0)));
  int yStart=int(y+0.5*h-h*sin(ang1*PI/(180.0*64.0)));
  int xEnd=int(x+0.5*w+w*cos(ang2*PI/(180.0*64.0)));
  int yEnd=int(y+0.5*h-h*sin(ang2*PI/(180.0*64.0)));
  HPEN hPen=(HPEN)SelectObject((HDC)dc,GetStockObject(NULL_PEN));
  if(reversed)
    Pie((HDC)dc,x,y,x+w,y+h,xEnd,yEnd,xStart,yStart);
  else
    Pie((HDC)dc,x,y,x+w,y+h,xStart,yStart,xEnd,yEnd);
  SelectObject((HDC)dc,hPen);
  }


void FXDCWindow::fillArcs(const FXArc* arcs,FXuint narcs){
  register FXuint i;
  if(!surface){ fxerror("FXDCWindow::fillArcs: DC not connected to drawable.\n"); }
  for(i=0; i<narcs; i++){
    fillArc(arcs[i].x,arcs[i].y,arcs[i].w,arcs[i].h,arcs[i].a,arcs[i].b);
    }
  }


// Filled simple polygon
void FXDCWindow::fillPolygon(const FXPoint* points,FXuint npoints){
  register FXuint i;
  POINT pts[1024];
  if(!surface){ fxerror("FXDCWindow::fillPolygon: DC not connected to drawable.\n"); }
  if(needsNewBrush) updateBrush();
  HPEN hPen=(HPEN)SelectObject((HDC)dc,GetStockObject(NULL_PEN));
  for(i=0; i<npoints; i++){
    pts[i].x=points[i].x;
    pts[i].y=points[i].y;
    }
  Polygon((HDC)dc,pts,npoints);
  SelectObject((HDC)dc,hPen);
  }


void FXDCWindow::fillConcavePolygon(const FXPoint* points,FXuint npoints){
  register FXuint i;
  POINT pts[1024];
  if(!surface){ fxerror("FXDCWindow::fillConcavePolygon: DC not connected to drawable.\n"); }
  if(needsNewBrush) updateBrush();
  HPEN hPen=(HPEN)SelectObject((HDC)dc,GetStockObject(NULL_PEN));
  for(i=0; i<npoints; i++){
    pts[i].x=points[i].x;
    pts[i].y=points[i].y;
    }
  Polygon((HDC)dc,pts,npoints);
  SelectObject((HDC)dc,hPen);
  }


void FXDCWindow::fillComplexPolygon(const FXPoint* points,FXuint npoints){
  register FXuint i;
  POINT pts[1024];
  if(!surface){ fxerror("FXDCWindow::fillComplexPolygon: DC not connected to drawable.\n"); }
  if(needsNewBrush) updateBrush();
  HPEN hPen=(HPEN)SelectObject((HDC)dc,GetStockObject(NULL_PEN));
  for(i=0; i<npoints; i++){
    pts[i].x=points[i].x;
    pts[i].y=points[i].y;
    }
  Polygon((HDC)dc,pts,npoints);
  SelectObject((HDC)dc,hPen);
  }


// Filled simple polygon with relative points
void FXDCWindow::fillPolygonRel(const FXPoint* points,FXuint npoints){
  register int x=0,y=0;
  register FXuint i;
  POINT pts[1024];
  if(!surface){ fxerror("FXDCWindow::fillPolygonRel: DC not connected to drawable.\n"); }
  if(needsNewBrush) updateBrush();
  HPEN hPen=(HPEN)SelectObject((HDC)dc,GetStockObject(NULL_PEN));
  for(i=0; i<npoints; i++){
    x+=points[i].x; pts[i].x=x;
    y+=points[i].y; pts[i].y=y;
    }
  Polygon((HDC)dc,pts,npoints);
  SelectObject((HDC)dc,hPen);
  }


void FXDCWindow::fillConcavePolygonRel(const FXPoint* points,FXuint npoints){
  register int x=0,y=0;
  register FXuint i;
  POINT pts[1024];
  if(!surface){ fxerror("FXDCWindow::fillConcavePolygonRel: DC not connected to drawable.\n"); }
  if(needsNewBrush) updateBrush();
  HPEN hPen=(HPEN)SelectObject((HDC)dc,GetStockObject(NULL_PEN));
  for(i=0; i<npoints; i++){
    x+=points[i].x; pts[i].x=x;
    y+=points[i].y; pts[i].y=y;
    }
  Polygon((HDC)dc,pts,npoints);
  SelectObject((HDC)dc,hPen);
  }


void FXDCWindow::fillComplexPolygonRel(const FXPoint* points,FXuint npoints){
  register int x=0,y=0;
  register FXuint i;
  POINT pts[1024];
  if(!surface){ fxerror("FXDCWindow::fillComplexPolygonRel: DC not connected to drawable.\n"); }
  if(needsNewBrush) updateBrush();
  HPEN hPen=(HPEN)SelectObject((HDC)dc,GetStockObject(NULL_PEN));
  for(i=0; i<npoints; i++){
    x+=points[i].x; pts[i].x=x;
    y+=points[i].y; pts[i].y=y;
    }
  Polygon((HDC)dc,pts,npoints);
  SelectObject((HDC)dc,hPen);
  }


// Draw string (only foreground bits)
void FXDCWindow::drawText(FXint x,FXint y,const FXchar* string,FXuint length){
  if(!surface){ fxerror("FXDCWindow::drawText: DC not connected to drawable.\n"); }
  int iBkMode=SetBkMode((HDC)dc,TRANSPARENT);
  TextOut((HDC)dc,x,y,string,length);
  SetBkMode((HDC)dc,iBkMode);
  }


// Draw string (both foreground and background bits)
void FXDCWindow::drawImageText(FXint x,FXint y,const FXchar* string,FXuint length){
  if(!surface){ fxerror("FXDCWindow::drawImageText: DC not connected to drawable.\n"); }
  int iBkMode=SetBkMode((HDC)dc,OPAQUE);
  TextOut((HDC)dc,x,y,string,length);
  SetBkMode((HDC)dc,iBkMode);
  }


// Draw area from source
// Some of these ROP codes do not have names; the full list can be found in the MSDN docs
// at Platform SDK/Reference/Appendixes/Win32 Appendixes/Raster Operation Codes/Ternary Raster Operations
void FXDCWindow::drawArea(const FXDrawable* source,FXint sx,FXint sy,FXint sw,FXint sh,FXint dx,FXint dy){
  if(!surface){ fxerror("FXDCWindow::drawArea: DC not connected to drawable.\n"); }
  if(!source || !source->id()){ fxerror("FXDCWindow::drawArea: illegal source specified.\n"); }
  HDC shdc=(HDC)source->GetDC();
  switch(rop){
    case BLT_CLR:                     // D := 0
      BitBlt((HDC)dc,dx,dy,sw,sh,shdc,sx,sy,BLACKNESS);
      break;
    case BLT_SRC_AND_DST:             // D := S & D
      BitBlt((HDC)dc,dx,dy,sw,sh,shdc,sx,sy,SRCAND);
      break;
    case BLT_SRC_AND_NOT_DST:         // D := S & ~D
      BitBlt((HDC)dc,dx,dy,sw,sh,shdc,sx,sy,SRCERASE);
      break;
    case BLT_SRC:                     // D := S
      BitBlt((HDC)dc,dx,dy,sw,sh,shdc,sx,sy,SRCCOPY);
      break;
    case BLT_NOT_SRC_AND_DST:         // D := ~S & D
      BitBlt((HDC)dc,dx,dy,sw,sh,shdc,sx,sy,0x220326);
      break;
    case BLT_DST:                     // D := D
      break;
    case BLT_SRC_XOR_DST:             // D := S ^ D
      BitBlt((HDC)dc,dx,dy,sw,sh,shdc,sx,sy,SRCINVERT);
      break;
    case BLT_SRC_OR_DST:              // D := S | D
      BitBlt((HDC)dc,dx,dy,sw,sh,shdc,sx,sy,SRCPAINT);
      break;
    case BLT_NOT_SRC_AND_NOT_DST:     // D := ~S & ~D  ==  D := ~(S | D)
      BitBlt((HDC)dc,dx,dy,sw,sh,shdc,sx,sy,NOTSRCERASE);
      break;
    case BLT_NOT_SRC_XOR_DST:         // D := ~S ^ D
      BitBlt((HDC)dc,dx,dy,sw,sh,shdc,sx,sy,0x990066); // Not sure about this one
      break;
    case BLT_NOT_DST:                 // D := ~D
      BitBlt((HDC)dc,dx,dy,sw,sh,shdc,sx,sy,DSTINVERT);
      break;
    case BLT_SRC_OR_NOT_DST:          // D := S | ~D
      BitBlt((HDC)dc,dx,dy,sw,sh,shdc,sx,sy,0xDD0228);
      break;
    case BLT_NOT_SRC:                 // D := ~S
      BitBlt((HDC)dc,dx,dy,sw,sh,shdc,sx,sy,NOTSRCCOPY);
      break;
    case BLT_NOT_SRC_OR_DST:          // D := ~S | D
      BitBlt((HDC)dc,dx,dy,sw,sh,shdc,sx,sy,MERGEPAINT);
      break;
    case BLT_NOT_SRC_OR_NOT_DST:      // D := ~S | ~D  ==  ~(S & D)
      BitBlt((HDC)dc,dx,dy,sw,sh,shdc,sx,sy,0x7700E6);
      break;
    case BLT_SET:                     // D := 1
      BitBlt((HDC)dc,dx,dy,sw,sh,shdc,sx,sy,WHITENESS);
      break;
    }
  source->ReleaseDC(shdc);
  }


// Draw image
void FXDCWindow::drawImage(const FXImage* image,FXint dx,FXint dy){
  if(!surface){ fxerror("FXDCWindow::drawImage: DC not connected to drawable.\n"); }
  if(!image || !image->id()){ fxerror("FXDCWindow::drawImage: illegal image specified.\n"); }
  HDC dcMem=(HDC)image->GetDC();
  switch(rop){
    case BLT_CLR:                     // D := 0
      BitBlt((HDC)dc,dx,dy,image->width,image->height,dcMem,0,0,BLACKNESS);
      break;
    case BLT_SRC_AND_DST:             // D := S & D
      BitBlt((HDC)dc,dx,dy,image->width,image->height,dcMem,0,0,SRCAND);
      break;
    case BLT_SRC_AND_NOT_DST:         // D := S & ~D
      BitBlt((HDC)dc,dx,dy,image->width,image->height,dcMem,0,0,SRCERASE);
      break;
    case BLT_SRC:                     // D := S
      BitBlt((HDC)dc,dx,dy,image->width,image->height,dcMem,0,0,SRCCOPY);
      break;
    case BLT_NOT_SRC_AND_DST:         // D := ~S & D
      BitBlt((HDC)dc,dx,dy,image->width,image->height,dcMem,0,0,0x220326);
      break;
    case BLT_DST:                     // D := D
      break;
    case BLT_SRC_XOR_DST:             // D := S ^ D
      BitBlt((HDC)dc,dx,dy,image->width,image->height,dcMem,0,0,SRCINVERT);
      break;
    case BLT_SRC_OR_DST:              // D := S | D
      BitBlt((HDC)dc,dx,dy,image->width,image->height,dcMem,0,0,SRCPAINT);
      break;
    case BLT_NOT_SRC_AND_NOT_DST:     // D := ~S & ~D  ==  D := ~(S | D)
      BitBlt((HDC)dc,dx,dy,image->width,image->height,dcMem,0,0,NOTSRCERASE);
      break;
    case BLT_NOT_SRC_XOR_DST:         // D := ~S ^ D
      BitBlt((HDC)dc,dx,dy,image->width,image->height,dcMem,0,0,0x990066); // Not sure about this one
      break;
    case BLT_NOT_DST:                 // D := ~D
      BitBlt((HDC)dc,dx,dy,image->width,image->height,dcMem,0,0,DSTINVERT);
      break;
    case BLT_SRC_OR_NOT_DST:          // D := S | ~D
      BitBlt((HDC)dc,dx,dy,image->width,image->height,dcMem,0,0,0xDD0228);
      break;
    case BLT_NOT_SRC:                 // D := ~S
      BitBlt((HDC)dc,dx,dy,image->width,image->height,dcMem,0,0,NOTSRCCOPY);
      break;
    case BLT_NOT_SRC_OR_DST:          // D := ~S | D
      BitBlt((HDC)dc,dx,dy,image->width,image->height,dcMem,0,0,MERGEPAINT);
      break;
    case BLT_NOT_SRC_OR_NOT_DST:      // D := ~S | ~D  ==  ~(S & D)
      BitBlt((HDC)dc,dx,dy,image->width,image->height,dcMem,0,0,0x7700E6);
      break;
    case BLT_SET:                     // D := 1
      BitBlt((HDC)dc,dx,dy,image->width,image->height,dcMem,0,0,WHITENESS);
      break;
    }
  image->ReleaseDC(dcMem);
  }


// Draw bitmap (Contributed by Michal Furmanczyk <mf@cfdrc.com>)
void FXDCWindow::drawBitmap(const FXBitmap* bitmap,FXint dx,FXint dy) {
  if(!surface) fxerror("FXDCWindow::drawBitmap: DC not connected to drawable.\n");
  if(!bitmap || !bitmap->id()) fxerror("FXDCWindow::drawBitmap: illegal bitmap specified.\n");
  HDC dcMem=(HDC)bitmap->GetDC();
  switch(rop){
    case BLT_CLR:                     // D := 0
      BitBlt((HDC)dc,dx,dy,bitmap->width,bitmap->height,dcMem,0,0,BLACKNESS);
      break;
    case BLT_SRC_AND_DST:             // D := S & D
      BitBlt((HDC)dc,dx,dy,bitmap->width,bitmap->height,dcMem,0,0,SRCAND);
      break;
    case BLT_SRC_AND_NOT_DST:         // D := S & ~D
      BitBlt((HDC)dc,dx,dy,bitmap->width,bitmap->height,dcMem,0,0,SRCERASE);
      break;
    case BLT_SRC:                     // D := S
      BitBlt((HDC)dc,dx,dy,bitmap->width,bitmap->height,dcMem,0,0,SRCCOPY);
      break;
    case BLT_NOT_SRC_AND_DST:         // D := ~S & D
      BitBlt((HDC)dc,dx,dy,bitmap->width,bitmap->height,dcMem,0,0,0x220326);
      break;
    case BLT_DST:                     // D := D
      break;
    case BLT_SRC_XOR_DST:             // D := S ^ D
      BitBlt((HDC)dc,dx,dy,bitmap->width,bitmap->height,dcMem,0,0,SRCINVERT);
      break;
    case BLT_SRC_OR_DST:              // D := S | D
      BitBlt((HDC)dc,dx,dy,bitmap->width,bitmap->height,dcMem,0,0,SRCPAINT);
      break;
    case BLT_NOT_SRC_AND_NOT_DST:     // D := ~S & ~D  ==  D := ~(S | D)
      BitBlt((HDC)dc,dx,dy,bitmap->width,bitmap->height,dcMem,0,0,NOTSRCERASE);
      break;
    case BLT_NOT_SRC_XOR_DST:         // D := ~S ^ D
      BitBlt((HDC)dc,dx,dy,bitmap->width,bitmap->height,dcMem,0,0,0x990066); // Not sure about this one
      break;
    case BLT_NOT_DST:                 // D := ~D
      BitBlt((HDC)dc,dx,dy,bitmap->width,bitmap->height,dcMem,0,0,DSTINVERT);
      break;
    case BLT_SRC_OR_NOT_DST:          // D := S | ~D
      BitBlt((HDC)dc,dx,dy,bitmap->width,bitmap->height,dcMem,0,0,0xDD0228);
      break;
    case BLT_NOT_SRC:                 // D := ~S
      BitBlt((HDC)dc,dx,dy,bitmap->width,bitmap->height,dcMem,0,0,NOTSRCCOPY);
      break;
    case BLT_NOT_SRC_OR_DST:          // D := ~S | D
      BitBlt((HDC)dc,dx,dy,bitmap->width,bitmap->height,dcMem,0,0,MERGEPAINT);
      break;
    case BLT_NOT_SRC_OR_NOT_DST:      // D := ~S | ~D  ==  ~(S & D)
      BitBlt((HDC)dc,dx,dy,bitmap->width,bitmap->height,dcMem,0,0,0x7700E6);
      break;
    case BLT_SET:                     // D := 1
      BitBlt((HDC)dc,dx,dy,bitmap->width,bitmap->height,dcMem,0,0,WHITENESS);
      break;
    }
  bitmap->ReleaseDC(dcMem);
  }


// Draw icon
void FXDCWindow::drawIcon(const FXIcon* icon,FXint dx,FXint dy){
  if(!surface){ fxerror("FXDCWindow::drawIcon: DC not connected to drawable.\n"); }
  if(!icon || !icon->id() || !icon->shape){ fxerror("FXDCWindow::drawIcon: illegal icon specified.\n"); }
  HDC hdcSrc=(HDC)icon->GetDC();
  if(icon->getOptions()&IMAGE_OPAQUE){
    BitBlt((HDC)dc,dx,dy,icon->getWidth(),icon->getHeight(),hdcSrc,0,0,SRCCOPY);
    }
  else{
    HDC hdcMsk=CreateCompatibleDC((HDC)dc);
    SelectObject(hdcMsk,icon->shape);
    COLORREF crOldBack=SetBkColor((HDC)dc,RGB(255,255,255));
    COLORREF crOldText=SetTextColor((HDC)dc,RGB(0,0,0));
    BitBlt((HDC)dc,dx,dy,icon->getWidth(),icon->getHeight(),hdcMsk,0,0,SRCAND);
    BitBlt((HDC)dc,dx,dy,icon->getWidth(),icon->getHeight(),hdcSrc,0,0,SRCPAINT);
    DeleteDC(hdcMsk);
    SetBkColor((HDC)dc,crOldBack);
    SetTextColor((HDC)dc,crOldText);
    }
  icon->ReleaseDC(hdcSrc);
  }


// This may be done faster, I suspect; but I'm tired of looking at this now;
// at least it's correct as it stands..
void FXDCWindow::drawIconShaded(const FXIcon* icon,FXint dx,FXint dy){
  if(!surface){ fxerror("FXDCWindow::drawIconShaded: DC not connected to drawable.\n"); }
  if(!icon || !icon->id() || !icon->shape){ fxerror("FXDCWindow::drawIconShaded: illegal icon specified.\n"); }
  HDC hdcSrc=(HDC)icon->GetDC();
  HDC hdcMsk=CreateCompatibleDC((HDC)dc);
  FXColor selbackColor=getApp()->getSelbackColor();
  SelectObject(hdcMsk,icon->shape);
  COLORREF crOldBack=SetBkColor((HDC)dc,RGB(255,255,255));
  COLORREF crOldText=SetTextColor((HDC)dc,RGB(0,0,0));

  // Paint icon
  BitBlt((HDC)dc,dx,dy,icon->getWidth(),icon->getHeight(),hdcMsk,0,0,SRCAND);
  BitBlt((HDC)dc,dx,dy,icon->getWidth(),icon->getHeight(),hdcSrc,0,0,SRCPAINT);

  HBRUSH hBrush=CreatePatternBrush((HBITMAP)getApp()->stipples[STIPPLE_GRAY]);
  HBRUSH hOldBrush=(HBRUSH)SelectObject((HDC)dc,hBrush);
  SetBrushOrgEx((HDC)dc,dx,dy,NULL);

  // Make black where pattern is 0 and shape is 0 [DPSoa]
  BitBlt((HDC)dc,dx,dy,icon->getWidth(),icon->getHeight(),hdcMsk,0,0,0x00A803A9);

  SetTextColor((HDC)dc,RGB(FXREDVAL(selbackColor),FXGREENVAL(selbackColor),FXBLUEVAL(selbackColor)));
  SetBkColor((HDC)dc,RGB(0,0,0));

  // Make selbackcolor where pattern is 0 and shape is 0 [DPSoo]
  BitBlt((HDC)dc,dx,dy,icon->getWidth(),icon->getHeight(),hdcMsk,0,0,0x00FE02A9);

  SelectObject((HDC)dc,hOldBrush);
  DeleteObject(hBrush);
  DeleteDC(hdcMsk);
  SetBkColor((HDC)dc,crOldBack);
  SetTextColor((HDC)dc,crOldText);
  SetBrushOrgEx((HDC)dc,tx,ty,NULL);
  icon->ReleaseDC(hdcSrc);
  }



// Draw a sunken or etched-in icon.
void FXDCWindow::drawIconSunken(const FXIcon* icon,FXint dx,FXint dy){
  if(!surface){ fxerror("FXDCWindow::drawIconSunken: DC not connected to drawable.\n"); }
  if(!icon || !icon->id() || !icon->shape){ fxerror("FXDCWindow::drawIconSunken: illegal icon specified.\n"); }
  HDC hdcSrc=(HDC)icon->GetDC();
  HDC hdcMono=CreateCompatibleDC((HDC)dc);
  SelectObject(hdcMono,icon->etch);

  // Apparently, BkColor and TextColor apply to the source
  COLORREF crOldBack=SetBkColor((HDC)dc,RGB(255,255,255));
  COLORREF crOldText=SetTextColor((HDC)dc,RGB(0,0,0));

  // While brush colors apply to the pattern
  FXColor hiliteColor=getApp()->getHiliteColor();
  HBRUSH hbrHilite=CreateSolidBrush(RGB(FXREDVAL(hiliteColor),FXGREENVAL(hiliteColor),FXBLUEVAL(hiliteColor)));
  HBRUSH oldBrush=(HBRUSH)SelectObject((HDC)dc,hbrHilite);

  // BitBlt the black bits in the monochrome bitmap into highlight colors
  // in the destination DC (offset a bit). This BitBlt(), and the next one,
  // use an unnamed raster op (0xB8074a) whose effect is D := ((D ^ P) & S) ^ P.
  // Or at least I think it is ;) The code = PSDPxax, so that's correct JVZ
  BitBlt((HDC)dc,dx+1,dy+1,icon->getWidth(),icon->getHeight(),hdcMono,0,0,0xB8074A);
  FXColor shadowColor=getApp()->getShadowColor();
  HBRUSH hbrShadow=CreateSolidBrush(RGB(FXREDVAL(shadowColor),FXGREENVAL(shadowColor),FXBLUEVAL(shadowColor)));
  SelectObject((HDC)dc,hbrShadow);

  // Now BitBlt the black bits in the monochrome bitmap into the
  // shadow color on the destination DC.
  BitBlt((HDC)dc,dx,dy,icon->getWidth(),icon->getHeight(),hdcMono,0,0,0xB8074A);
  DeleteDC(hdcMono);
  SelectObject((HDC)dc,oldBrush);
  DeleteObject(hbrHilite);
  DeleteObject(hbrShadow);
  SetBkColor((HDC)dc,crOldBack);
  SetTextColor((HDC)dc,crOldText);
  icon->ReleaseDC(hdcSrc);
  }


void FXDCWindow::drawHashBox(FXint x,FXint y,FXint w,FXint h,FXint b){
  if(!surface){ fxerror("FXDCWindow::drawHashBox: DC not connected to drawable.\n"); }
  HBRUSH hBrush=(HBRUSH)SelectObject((HDC)dc,CreatePatternBrush((HBITMAP)getApp()->stipples[STIPPLE_GRAY]));
  COLORREF crOldBack=SetBkColor((HDC)dc,RGB(255,255,255));
  COLORREF crOldText=SetTextColor((HDC)dc,RGB(0,0,0));
  PatBlt((HDC)dc,x,y,w-b,b,PATINVERT);
  PatBlt((HDC)dc,x+w-b,y,b,h-b,PATINVERT);
  PatBlt((HDC)dc,x+b,y+h-b,w-b,b,PATINVERT);
  PatBlt((HDC)dc,x,y+b,b,h-b,PATINVERT);
  DeleteObject(SelectObject((HDC)dc,hBrush));
  SetBkColor((HDC)dc,crOldBack);
  SetTextColor((HDC)dc,crOldText);
  }


void FXDCWindow::drawFocusRectangle(FXint x,FXint y,FXint w,FXint h){
  if(!surface){ fxerror("FXDCWindow::drawFocusRectangle: DC not connected to drawable.\n"); }
  HBRUSH hBrush=(HBRUSH)SelectObject((HDC)dc,CreatePatternBrush((HBITMAP)getApp()->stipples[STIPPLE_GRAY]));
  COLORREF crOldBack=SetBkColor((HDC)dc,RGB(255,255,255));
  COLORREF crOldText=SetTextColor((HDC)dc,RGB(0,0,0));
  SetBrushOrgEx((HDC)dc,x,y,NULL);
  PatBlt((HDC)dc,x,y,w-1,1,PATINVERT);
  PatBlt((HDC)dc,x+w-1,y,1,h-1,PATINVERT);
  PatBlt((HDC)dc,x+1,y+h-1,w-1,1,PATINVERT);
  PatBlt((HDC)dc,x,y+1,1,h-1,PATINVERT);
  DeleteObject(SelectObject((HDC)dc,hBrush));
  SetBkColor((HDC)dc,crOldBack);
  SetTextColor((HDC)dc,crOldText);
  SetBrushOrgEx((HDC)dc,tx,ty,NULL);
  }


static DWORD FXStipplePattern2Hatch(FXStipplePattern pat){
  switch(pat){
    case STIPPLE_HORZ: return HS_HORIZONTAL;
    case STIPPLE_VERT: return HS_VERTICAL;
    case STIPPLE_CROSS: return HS_CROSS;
    case STIPPLE_DIAG: return HS_BDIAGONAL;
    case STIPPLE_REVDIAG: return HS_FDIAGONAL;
    case STIPPLE_CROSSDIAG: return HS_DIAGCROSS;
    default: return HS_CROSS;
    }
  }


void FXDCWindow::updatePen(){
  DWORD dashes[32];
  DWORD penstyle,i;
  LOGBRUSH lb;

  // Setup brush of this pen
  switch(fill){
    case FILL_SOLID:
      lb.lbStyle=BS_SOLID;
      lb.lbColor=devfg;
      lb.lbHatch=0;
      break;
    case FILL_TILED:
      FXASSERT(FALSE);
      break;
    case FILL_STIPPLED:
      if(stipple){
        lb.lbStyle=BS_PATTERN;
        lb.lbColor=devfg;
        lb.lbHatch=(LONG)stipple->id();    // This should be a HBITMAP
        }
      else if(pattern>=STIPPLE_0 && pattern<=STIPPLE_16){
        lb.lbStyle=BS_PATTERN;
        lb.lbColor=devfg;
        lb.lbHatch=(LONG)getApp()->stipples[pattern];
        }
      else{
        lb.lbStyle=BS_HATCHED;
        lb.lbColor=devfg;
        lb.lbHatch=FXStipplePattern2Hatch(pattern);
        }
      break;
    case FILL_OPAQUESTIPPLED:
      if(stipple){
        lb.lbStyle=BS_PATTERN;
        lb.lbColor=devfg;
        lb.lbHatch=(LONG)stipple->id();    // This should be a HBITMAP
        }
      else if(pattern>=STIPPLE_0 && pattern<=STIPPLE_16){
        lb.lbStyle=BS_PATTERN;
        lb.lbColor=devfg;
        lb.lbHatch=(LONG)getApp()->stipples[pattern];
        }
      else{
        lb.lbStyle=BS_HATCHED;
        lb.lbColor=devfg;
        lb.lbHatch=FXStipplePattern2Hatch(pattern);
        }
      break;
    }

  penstyle=0;

  // Cap style
  if(cap==CAP_ROUND)
    penstyle|=PS_JOIN_ROUND;
  else if(cap==CAP_PROJECTING)
    penstyle|=PS_ENDCAP_SQUARE;
  else
    penstyle|=PS_ENDCAP_FLAT;

  // Join style
  if(join==JOIN_MITER)
    penstyle|=PS_JOIN_MITER;
  else if(join==JOIN_ROUND)
    penstyle|=PS_JOIN_ROUND;
  else
    penstyle|=PS_JOIN_BEVEL;

  // Kind of pen
  //if(width<=1 && fill==FILL_SOLID)
  //  penstyle|=PS_COSMETIC;
  //else
    penstyle|=PS_GEOMETRIC;

  // Line style
  if(style==LINE_SOLID){
    penstyle|=PS_SOLID;
    DeleteObject(SelectObject((HDC)dc,ExtCreatePen(penstyle,width,&lb,0,NULL)));
    }
  else if(dashoff==0 && dashlen==2 && dashpat[0]==1 && dashpat[1]==1){
    penstyle|=PS_DOT;
    DeleteObject(SelectObject((HDC)dc,ExtCreatePen(penstyle,width,&lb,0,NULL)));
    }
  else if(dashoff==0 && dashlen==2 && dashpat[0]==3 && dashpat[1]==1){
    penstyle|=PS_DASH;
    DeleteObject(SelectObject((HDC)dc,ExtCreatePen(penstyle,width,&lb,0,NULL)));
    }
  else if(dashoff==0 && dashlen==4 && dashpat[0]==3 && dashpat[1]==1 && dashpat[2]==1 && dashpat[3]==1){
    penstyle|=PS_DASHDOT;
    DeleteObject(SelectObject((HDC)dc,ExtCreatePen(penstyle,width,&lb,0,NULL)));
    }
  else if(dashoff==0 && dashlen==6 && dashpat[0]==3 && dashpat[1]==1 && dashpat[2]==1 && dashpat[3]==1 && dashpat[4]==1 && dashpat[5]==1){
    penstyle|=PS_DASHDOTDOT;
    DeleteObject(SelectObject((HDC)dc,ExtCreatePen(penstyle,width,&lb,0,NULL)));
    }
  else{
    penstyle|=PS_USERSTYLE;
    for(i=0; i<dashlen; i++){ dashes[i]=dashpat[(i+dashoff)%dashlen]; }
    DeleteObject(SelectObject((HDC)dc,ExtCreatePen(penstyle,width,&lb,dashlen,dashes)));
    }
  if(fill==FILL_STIPPLED){
    SetBkMode((HDC)dc,TRANSPARENT);         // Alas, only works for BS_HATCHED...
    }
  else{
    SetBkMode((HDC)dc,OPAQUE);
    }
  if(fill!=FILL_SOLID){
    SetBrushOrgEx((HDC)dc,tx,ty,NULL);
    }
  needsPath=(width>1);
  needsNewPen=FALSE;
  }


void FXDCWindow::updateBrush(){
  LOGBRUSH lb;
  switch(fill){
    case FILL_SOLID:
      lb.lbStyle=BS_SOLID;
      lb.lbColor=devfg;
      lb.lbHatch=0;
      break;
    case FILL_TILED:
      FXASSERT(FALSE);
      lb.lbColor=devfg;
      break;
    case FILL_STIPPLED:
      if(stipple){
        lb.lbStyle=BS_PATTERN;
        lb.lbColor=devfg;
        lb.lbHatch=(LONG)stipple->id();     // This should be a HBITMAP
        }
      else if(pattern>=STIPPLE_0 && pattern<=STIPPLE_16){
        lb.lbStyle=BS_PATTERN;
        lb.lbColor=devfg;
        lb.lbHatch=(LONG)getApp()->stipples[pattern];
        }
      else{
        lb.lbStyle=BS_HATCHED;
        lb.lbColor=devfg;
        lb.lbHatch=FXStipplePattern2Hatch(pattern);
        }
      break;
    case FILL_OPAQUESTIPPLED:
      if(stipple){
        lb.lbStyle=BS_PATTERN;
        lb.lbColor=devfg;
        lb.lbHatch=(LONG)stipple->id();     // This should be a HBITMAP
        }
      else if(pattern>=STIPPLE_0 && pattern<=STIPPLE_16){
        lb.lbStyle=BS_PATTERN;
        lb.lbColor=devfg;
        lb.lbHatch=(LONG)getApp()->stipples[pattern];
        }
      else{
        lb.lbStyle=BS_HATCHED;
        lb.lbColor=devfg;
        lb.lbHatch=FXStipplePattern2Hatch(pattern);
        }
      break;
    }
  DeleteObject(SelectObject((HDC)dc,CreateBrushIndirect(&lb)));
  if(fill==FILL_STIPPLED){
    SetBkMode((HDC)dc,TRANSPARENT);         // Alas, only works for BS_HATCHED...
    }
  else{
    SetBkMode((HDC)dc,OPAQUE);
    }
  if(fill!=FILL_SOLID){
    SetBrushOrgEx((HDC)dc,tx,ty,NULL);
    }
  needsNewBrush=FALSE;
  }


void FXDCWindow::setForeground(FXColor clr){
  if(!surface){ fxerror("FXDCWindow::setForeground: DC not connected to drawable.\n"); }
  devfg=visual->getPixel(clr);
  needsNewPen=TRUE;
  needsNewBrush=TRUE;
  SetTextColor((HDC)dc,devfg);
  fg=clr;
  }


void FXDCWindow::setBackground(FXColor clr){
  if(!surface){ fxerror("FXDCWindow::setBackground: DC not connected to drawable.\n"); }
  devbg=visual->getPixel(clr);
  SetBkColor((HDC)dc,devbg);
  bg=clr;
  }


// Set dash pattern (for the LINE_ONOFF_DASH line style)
void FXDCWindow::setDashes(FXuint dashoffset,const FXchar *dashpattern,FXuint dashlength){
  register FXuint len,i;
  if(!surface){ fxerror("FXDCWindow::setDashes: DC not connected to drawable.\n"); }
  for(i=len=0; i<dashlength; i++){
    dashpat[i]=dashpattern[i];
    len+=(FXuint)dashpattern[i];
    }
  dashlen=dashlength;
  dashoff=dashoffset%len;
  needsNewPen=TRUE;
  }


void FXDCWindow::setLineWidth(FXuint linewidth){
  if(!surface){ fxerror("FXDCWindow::setLineWidth: DC not connected to drawable.\n"); }
  width=linewidth;
  needsNewPen=TRUE;
  }


void FXDCWindow::setLineCap(FXCapStyle capstyle){
  if(!surface){ fxerror("FXDCWindow::setLineCap: DC not connected to drawable.\n"); }
  cap=capstyle;
  needsNewPen=TRUE;
  }


void FXDCWindow::setLineJoin(FXJoinStyle joinstyle){
  if(!surface){ fxerror("FXDCWindow::setLineJoin: DC not connected to drawable.\n"); }
  join=joinstyle;
  needsNewPen=TRUE;
  }


void FXDCWindow::setLineStyle(FXLineStyle linestyle){
  if(!surface){ fxerror("FXDCWindow::setLineStyle: DC not connected to drawable.\n"); }
  style=linestyle;
  needsNewPen=TRUE;
  }


void FXDCWindow::setFillStyle(FXFillStyle fillstyle){
  if(!surface){ fxerror("FXDCWindow::setFillStyle: DC not connected to drawable.\n"); }
  fill=fillstyle;
  needsNewBrush=TRUE;
  needsNewPen=TRUE;
  }


// Set fill rule
void FXDCWindow::setFillRule(FXFillRule fillrule){
  if(!surface){ fxerror("FXDCWindow::setFillRule: DC not connected to drawable.\n"); }
  if(fillrule==RULE_EVEN_ODD)
    SetPolyFillMode((HDC)dc,ALTERNATE);
  else
    SetPolyFillMode((HDC)dc,WINDING);
  rule=fillrule;
  }


// Set blit function
void FXDCWindow::setFunction(FXFunction func){
  if(!surface){ fxerror("FXDCWindow::setFunction: DC not connected to drawable.\n"); }
  rop=func;

  // Also set ROP2 code for lines
  switch(rop){
    case BLT_CLR:                     // D := 0
      SetROP2((HDC)dc,R2_BLACK);
      break;
    case BLT_SRC_AND_DST:             // D := S & D
      SetROP2((HDC)dc,R2_MASKPEN);
      break;
    case BLT_SRC_AND_NOT_DST:         // D := S & ~D
      SetROP2((HDC)dc,R2_MASKPENNOT);
      break;
    case BLT_SRC:                     // D := S
      SetROP2((HDC)dc,R2_COPYPEN);
      break;
    case BLT_NOT_SRC_AND_DST:         // D := ~S & D
      SetROP2((HDC)dc,R2_MASKNOTPEN);
      break;
    case BLT_DST:                     // D := D
      break;
    case BLT_SRC_XOR_DST:             // D := S ^ D
      SetROP2((HDC)dc,R2_XORPEN);
      break;
    case BLT_SRC_OR_DST:              // D := S | D
      SetROP2((HDC)dc,R2_MERGEPEN);
      break;
    case BLT_NOT_SRC_AND_NOT_DST:     // D := ~S & ~D  ==  D := ~(S | D)
      SetROP2((HDC)dc,R2_NOTMERGEPEN);
      break;
    case BLT_NOT_SRC_XOR_DST:         // D := ~S ^ D
      SetROP2((HDC)dc,R2_NOTXORPEN); // Is this the right one?
      break;
    case BLT_NOT_DST:                 // D := ~D
      SetROP2((HDC)dc,R2_NOT);
      break;
    case BLT_SRC_OR_NOT_DST:          // D := S | ~D
      SetROP2((HDC)dc,R2_MERGEPENNOT);
      break;
    case BLT_NOT_SRC:                 // D := ~S
      SetROP2((HDC)dc,R2_NOTCOPYPEN);
      break;
    case BLT_NOT_SRC_OR_DST:          // D := ~S | D
      SetROP2((HDC)dc,R2_MERGENOTPEN);
      break;
    case BLT_NOT_SRC_OR_NOT_DST:      // D := ~S | ~D  ==  ~(S & D)
      SetROP2((HDC)dc,R2_NOTMASKPEN);
      break;
    case BLT_SET:                     // D := 1
      SetROP2((HDC)dc,R2_WHITE);
      break;
    }
  }


// Set tile image
void FXDCWindow::setTile(FXImage* image,FXint dx,FXint dy){
  if(!surface){ fxerror("FXDCWindow::setTile: DC not connected to drawable.\n"); }
  tile=image;
  tx=dx;
  ty=dy;
  }


// Set stipple pattern
void FXDCWindow::setStipple(FXBitmap* bitmap,FXint dx,FXint dy){
  if(!surface){ fxerror("FXDCWindow::setStipple: DC not connected to drawable.\n"); }
  stipple=bitmap;
  pattern=STIPPLE_NONE;
  needsNewBrush=TRUE;
  needsNewPen=TRUE;
  tx=dx;
  ty=dy;
  }


void FXDCWindow::setStipple(FXStipplePattern pat,FXint dx,FXint dy){
  if(!surface){ fxerror("FXDCWindow::setStipple: DC not connected to drawable.\n"); }
  stipple=NULL;
  pattern=pat;
  needsNewBrush=TRUE;
  needsNewPen=TRUE;
  tx=dx;
  ty=dy;
  }


void FXDCWindow::setClipRegion(const FXRegion& region){
  if(!surface){ fxerror("FXDCWindow::setClipRegion: DC not connected to drawable.\n"); }
  SelectClipRgn((HDC)dc,(HRGN)region.region);
  }


void FXDCWindow::setClipRectangle(FXint x,FXint y,FXint w,FXint h){
  if(!surface){ fxerror("FXDCWindow::setClipRectangle: DC not connected to drawable.\n"); }
  clip.x=FXMAX(x,rect.x);
  clip.y=FXMAX(y,rect.y);
  clip.w=FXMIN(x+w,rect.x+rect.w)-clip.x;
  clip.h=FXMIN(y+h,rect.y+rect.h)-clip.y;
  if(clip.w<=0) clip.w=0;
  if(clip.h<=0) clip.h=0;
  HRGN hrgn=CreateRectRgn(clip.x,clip.y,clip.x+clip.w,clip.y+clip.h);
  SelectClipRgn((HDC)dc,hrgn);
  DeleteObject(hrgn);
  }


void FXDCWindow::setClipRectangle(const FXRectangle& rectangle){
  if(!surface){ fxerror("FXDCWindow::setClipRectangle: DC not connected to drawable.\n"); }
  clip.x=FXMAX(rectangle.x,rect.x);
  clip.y=FXMAX(rectangle.y,rect.y);
  clip.w=FXMIN(rectangle.x+rectangle.w,rect.x+rect.w)-clip.x;
  clip.h=FXMIN(rectangle.y+rectangle.h,rect.y+rect.h)-clip.y;
  if(clip.w<=0) clip.w=0;
  if(clip.h<=0) clip.h=0;
  HRGN hrgn=CreateRectRgn(clip.x,clip.y,clip.x+clip.w,clip.y+clip.h);
  SelectClipRgn((HDC)dc,hrgn);
  DeleteObject(hrgn);
  }


void FXDCWindow::clearClipRectangle(){
  if(!surface){ fxerror("FXDCWindow::clearClipRectangle: DC not connected to drawable.\n"); }
  clip=rect;
  HRGN hrgn=CreateRectRgn(clip.x,clip.y,clip.x+clip.w,clip.y+clip.h);
  SelectClipRgn((HDC)dc,hrgn);
  DeleteObject(hrgn);
  }


void FXDCWindow::setClipMask(FXBitmap* bitmap,FXint dx,FXint dy){
  if(!surface){ fxerror("FXDCWindow::setClipMask: DC not connected to drawable.\n"); }
  FXASSERT(FALSE);
  mask=bitmap;
  cx=dx;
  cy=dy;
  }


void FXDCWindow::clearClipMask(){
  if(!surface){ fxerror("FXDCWindow::clearClipMask: DC not connected to drawable.\n"); }
  FXASSERT(FALSE);
  mask=NULL;
  cx=0;
  cy=0;
  }


void FXDCWindow::setTextFont(FXFont *fnt){
  if(!surface){ fxerror("FXDCWindow::setTextFont: DC not connected to drawable.\n"); }
  if(!fnt || !fnt->id()){ fxerror("FXDCWindow::setTextFont: illegal or NULL font specified.\n"); }
  SelectObject((HDC)dc,fnt->id());
  font=fnt;
  }



// Window will clip against child windows
void FXDCWindow::clipChildren(FXbool yes){
  if(!surface){ fxerror("FXDCWindow::clipChildren: window has not yet been created.\n"); }
  DWORD    dwFlags=GetWindowLong((HWND)surface->id(),GWL_STYLE);
  HPEN     hPen;
  HBRUSH   hBrush;
  HFONT    hFont;
  COLORREF textcolor;
  COLORREF backcolor;
  FXint    fillmode;
  if(yes){
    if(!(dwFlags&WS_CLIPCHILDREN)){
      hPen=(HPEN)SelectObject((HDC)dc,GetStockObject(NULL_PEN));
      hBrush=(HBRUSH)SelectObject((HDC)dc,GetStockObject(NULL_BRUSH));
      hFont=(HFONT)SelectObject((HDC)dc,GetStockObject(SYSTEM_FONT));
      textcolor=GetTextColor((HDC)dc);
      backcolor=GetBkColor((HDC)dc);
      fillmode=GetPolyFillMode((HDC)dc);

      ReleaseDC((HWND)surface->id(),(HDC)dc);
      SetWindowLong((HWND)surface->id(),GWL_STYLE,dwFlags|WS_CLIPCHILDREN);
      dc=GetDC((HWND)surface->id());

      SelectObject((HDC)dc,hFont);
      SelectObject((HDC)dc,hPen);
      SelectObject((HDC)dc,hBrush);

      if(visual->hPalette){
        SelectPalette((HDC)dc,(HPALETTE)visual->hPalette,FALSE);
        RealizePalette((HDC)dc);
        }
      SetTextAlign((HDC)dc,TA_BASELINE|TA_LEFT);
      SetTextColor((HDC)dc,textcolor);
      SetBkColor((HDC)dc,backcolor);
      SetPolyFillMode((HDC)dc,fillmode);
      }
    }
  else{
//  if(dwFlags&WS_CLIPCHILDREN){
    if(dwFlags&WS_CLIPCHILDREN && (HWND)surface->id()!=GetDesktopWindow()){
      hPen=(HPEN)SelectObject((HDC)dc,GetStockObject(NULL_PEN));
      hBrush=(HBRUSH)SelectObject((HDC)dc,GetStockObject(NULL_BRUSH));
      hFont=(HFONT)SelectObject((HDC)dc,GetStockObject(SYSTEM_FONT));
      textcolor=GetTextColor((HDC)dc);
      backcolor=GetBkColor((HDC)dc);
      fillmode=GetPolyFillMode((HDC)dc);

      ReleaseDC((HWND)surface->id(),(HDC)dc);
      SetWindowLong((HWND)surface->id(),GWL_STYLE,dwFlags & ~WS_CLIPCHILDREN);
      dc=GetDC((HWND)surface->id());

      SelectObject((HDC)dc,hFont);
      SelectObject((HDC)dc,hPen);
      SelectObject((HDC)dc,hBrush);

      if(visual->hPalette){
        SelectPalette((HDC)dc,(HPALETTE)visual->hPalette,FALSE);
        RealizePalette((HDC)dc);
        }
      SetTextAlign((HDC)dc,TA_BASELINE|TA_LEFT);
      SetTextColor((HDC)dc,textcolor);
      SetBkColor((HDC)dc,backcolor);
      SetPolyFillMode((HDC)dc,fillmode);
      }
    }
  }

#endif
