/********************************************************************************
*                                                                               *
*                             S l i d e r   O b j e c t s                       *
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
* $Id: FXSlider.h,v 1.10 1998/10/29 07:42:21 jeroen Exp $                        *
********************************************************************************/
#ifndef FXSLIDER_H
#define FXSLIDER_H


// Default sizes looks nice
#define SLIDERBAR_SIZE   5
#define SLIDERHEAD_SIZE  7


// Sliderbar options
enum FXSliderStyle {
 SLIDER_HORIZONTAL  = 0,
 SLIDER_VERTICAL    = 0x00008000,
 SLIDER_ARROW_UP    = 0x00010000,
 SLIDER_ARROW_DOWN  = 0x00020000,
 SLIDER_ARROW_LEFT  = SLIDER_ARROW_UP,
 SLIDER_ARROW_RIGHT = SLIDER_ARROW_DOWN,
 SLIDER_INSIDE_BAR  = 0x00040000,
 SLIDER_MASK        = SLIDER_VERTICAL|SLIDER_ARROW_UP|SLIDER_ARROW_DOWN|SLIDER_INSIDE_BAR
 };
 
 
  
// Slider head item
class FXSliderHead : public FXFrame {
  FXDECLARE(FXSliderHead)
protected:
  FXSliderHead(){}
  FXSliderHead(const FXSliderHead&){}
public:
  long onPaint(FXObject*,FXSelector,void*);
public:
  FXSliderHead(FXComposite* p,FXuint opts=0);
  virtual void create();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  };



// Scroll bar
class FXSlider : public FXComposite {
  FXDECLARE(FXSlider)
protected:
  FXSliderHead *head;
  FXTimer      *timer;
  FXint         range[2];
  FXPixel       slotColor;
  FXPixel       baseColor;
  FXPixel       hiliteColor;
  FXPixel       shadowColor;
  FXPixel       borderColor;
  FXint         headpos;
  FXint         headsize;
  FXint         slotsize;
  FXint         dragpoint;
  FXint         incr;
  FXint         pos;
protected:
  FXSlider(){}
  FXSlider(const FXSlider&){}
  virtual void layout();
  void drawDoubleSunkenRectangle(FXint x,FXint y,FXint w,FXint h);
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onLeftBtnPress(FXObject*,FXSelector,void*);
  long onMiddleBtnPress(FXObject*,FXSelector,void*);
  long onMotion(FXObject*,FXSelector,void*);
  long onAnyBtnRelease(FXObject*,FXSelector,void*);
  long onTimeInc(FXObject*,FXSelector,void*);
  long onTimeDec(FXObject*,FXSelector,void*);
  long onCmdSetValue(FXObject*,FXSelector,void*);
  long onCmdSetIntValue(FXObject*,FXSelector,void*);
  long onCmdGetIntValue(FXObject*,FXSelector,void*);
  long onCmdSetRealValue(FXObject*,FXSelector,void*);
  long onCmdGetRealValue(FXObject*,FXSelector,void*);
public:
  FXSlider(FXComposite* p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  virtual void create();
  virtual void enable();
  virtual void disable();
  virtual FXbool canFocus() const;
  FXSliderHead* sliderHead() const { return head; }
  void setRange(FXint lo,FXint hi);
  void getRange(FXint& lo,FXint& hi) const { lo=range[0]; hi=range[1]; }
  void setPosition(FXint p);
  FXint getPosition() const { return pos; }
  FXuint getSliderStyle() const;
  void setSliderStyle(FXuint opts);
  FXint getHeadSize() const { return headsize; }
  void setHeadSize(FXint hs);
  FXint getSlotSize() const { return slotsize; }
  void setSlotSize(FXint bs);
  FXPixel getSlotColor() const { return slotColor; }
  FXPixel getHiliteColor() const { return hiliteColor; }
  FXPixel getShadowColor() const { return shadowColor; }
  FXPixel getBorderColor() const { return borderColor; }
  FXPixel getBaseColor() const { return baseColor; }
  void setSlotColor(FXPixel clr);
  void setHiliteColor(FXPixel clr);
  void setShadowColor(FXPixel clr);
  void setBorderColor(FXPixel clr);
  void setBaseColor(FXPixel clr);
  void autoSlideInc();
  void autoSlideDec();
  void stopAutoSlide();
  FXbool isAutoSliding();
  virtual ~FXSlider();
  };

  
#endif
