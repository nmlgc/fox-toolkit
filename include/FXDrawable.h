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
* $Id: FXDrawable.h,v 1.6 1998/09/18 05:54:02 jeroen Exp $                      *
********************************************************************************/
#ifndef FXDRAWABLE_H
#define FXDRAWABLE_H


class FXImage;
class FXIcon;


class FXDrawable : public FXId {
  FXDECLARE(FXDrawable)
protected:
  FXint width;
  FXint height;
  FXint depth;
protected:
  FXDrawable();
  FXDrawable(const FXDrawable&){}
  virtual FXID getGrayBitmap() const;
  virtual GC getGC() const;
  FXDrawable(FXApp* a,FXint w,FXint h,FXint d);
public:
  // Width of drawable
  FXint getWidth() const { return width; }

  // Height of drawable
  FXint getHeight() const { return height; }

  // Depth of drawable
  FXuint getDepth() const { return depth; }

  // Create drawable surface
  virtual void create();

  // Destroy drawable surface
  virtual void destroy();

  // Draw points
  void drawPoint(FXint x,FXint y);
  void drawPoints(const FXPoint* points,FXuint npoints);
  void drawPointsRel(const FXPoint* points,FXuint npoints);

  // Draw lines
  void drawLine(FXint x1,FXint y1,FXint x2,FXint y2);
  void drawLines(const FXPoint* points,FXuint npoints);
  void drawLinesRel(const FXPoint* points,FXuint npoints);
  void drawLineSegments(const FXSegment* segments,FXuint nsegments);
  
  // Draw rectangles
  void drawRectangle(FXint x,FXint y,FXint w,FXint h);
  void drawRectangles(const FXRectangle* rectangles,FXuint nrectangles);

  // Draw arcs
  void drawArc(FXint x,FXint y,FXint w,FXint h,FXint ang1,FXint ang2);
  void drawArcs(const FXArc* arcs,FXuint narcs);

  // Filled rectangles
  void fillRectangle(FXint x,FXint y,FXint w,FXint h);
  void fillRectangles(const FXRectangle* rectangles,FXuint nrectangles);

  // Draw arcs
  void fillArc(FXint x,FXint y,FXint w,FXint h,FXint ang1,FXint ang2);
  void fillArcs(const FXArc* arcs,FXuint narcs);

  // Filled polygon
  void fillPolygon(const FXPoint* points,FXuint npoints);
  void fillConcavePolygon(const FXPoint* points,FXuint npoints);
  void fillComplexPolygon(const FXPoint* points,FXuint npoints);

  // Filled polygon with relative points
  void fillPolygonRel(const FXPoint* points,FXuint npoints);
  void fillConcavePolygonRel(const FXPoint* points,FXuint npoints);
  void fillComplexPolygonRel(const FXPoint* points,FXuint npoints);

  // Draw hashed box
  void drawHashBox(FXint x,FXint y,FXint w,FXint h,FXint b=1);
  
  // Draw area from source
  void drawArea(const FXDrawable* source,FXint sx,FXint sy,FXint sw,FXint sh,FXint dx,FXint dy);

  // Draw image 
  void drawImage(const FXImage* image,FXint dx,FXint dy);

  // Draw icon 
  void drawIcon(const FXIcon* icon,FXint dx,FXint dy);
  void drawIconShaded(const FXIcon* icon,FXint dx,FXint dy);

  // Draw string
  void drawText(FXint x,FXint y,const FXchar* string,FXuint length);
  void drawImageText(FXint x,FXint y,const FXchar* string,FXuint length);

  // Set foreground/background drawing color
  void setForeground(FXPixel clr);
  void setBackground(FXPixel clr);

  // Set dash pattern
  void setDashes(FXuint dashoffset,const FXchar *dashlist,FXuint n);
  
  // Set line attributes
  void setLineAttributes(FXuint linewidth=0,FXLineStyle linestyle=LINE_SOLID,FXCapStyle capstyle=CAP_BUTT,FXJoinStyle joinstyle=JOIN_MITER);
  
  // Set fill style
  void setFillStyle(FXFillStyle fillstyle=FILL_SOLID);
  
  // Set fill rule
  void setFillRule(FXFillRule fillrule=RULE_EVEN_ODD);
  
  // Set blit function
  void setFunction(FXFunction func=BLT_SRC);
  
  // Set clip rectangle
  void setClipRectangle(FXint x,FXint y,FXint w,FXint h);
  
  // Clear clipping
  void clearClipRectangle();
  
  // Set clip mask
  void setClipMask(FXImage* mask,FXint dx,FXint dy);
  
  // Clear clip mask
  void clearClipMask();
  
  // Set font to draw text with
  void setTextFont(FXFont *font);
  
  // Save object to stream
  virtual void save(FXStream& store) const;
  
  // Load object from stream
  virtual void load(FXStream& store);
  
  // Cleanup
  virtual ~FXDrawable();
  };
  
    
#endif
