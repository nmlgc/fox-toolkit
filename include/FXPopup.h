/********************************************************************************
*                                                                               *
*                     P o p u p   W i n d o w   O b j e c t                     *
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
* $Id: FXPopup.h,v 1.9 1998/09/24 06:40:04 jeroen Exp $                         *
********************************************************************************/
#ifndef FXPOPUP_H
#define FXPOPUP_H


// Popup internal orientation
enum FXPopupStyle {
  POPUP_VERTICAL   = 0,               // Vertical orientation
  POPUP_HORIZONTAL = 0x00020000,      // Horizontal orientation
  POPUP_MASK       = POPUP_HORIZONTAL
  };

  

// Popup window
class FXPopup : public FXShell {
  FXDECLARE(FXPopup)
protected:
  FXWindow *owner;
  FXPixel   baseColor;
  FXPixel   hiliteColor;
  FXPixel   shadowColor;
  FXPixel   borderColor;
  FXint     border;
public:
  long onFocusDown(FXObject*,FXSelector,void*);
  long onFocusUp(FXObject*,FXSelector,void*);
  long onEnter(FXObject*,FXSelector,void*);
  long onLeave(FXObject*,FXSelector,void*);
  long onMotion(FXObject*,FXSelector,void*);
  long onMap(FXObject*,FXSelector,void*);
  long onButtonPress(FXObject*,FXSelector,void*);
  long onButtonRelease(FXObject*,FXSelector,void*);
  long onCmdUnpost(FXObject*,FXSelector,void*);
  long onKeyPress(FXObject*,FXSelector,void*);
  long onKeyRelease(FXObject*,FXSelector,void*);
protected:
  FXPopup(){}
  FXPopup(const FXPopup&){}
  virtual FXbool doesOverrideRedirect() const;
  virtual FXbool doesSaveUnder() const;
  virtual void layout();
  virtual void show();
  virtual void hide();
public:
  long onPaint(FXObject*,FXSelector,void*);
public:
  FXPopup(FXApp* a,FXWindow* own=NULL,FXuint opts=POPUP_VERTICAL|FRAME_RAISED|FRAME_THICK,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual void create();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  void drawBorderRectangle(FXint x,FXint y,FXint w,FXint h);
  void drawRaisedRectangle(FXint x,FXint y,FXint w,FXint h);
  void drawSunkenRectangle(FXint x,FXint y,FXint w,FXint h);
  void drawRidgeRectangle(FXint x,FXint y,FXint w,FXint h);
  void drawGrooveRectangle(FXint x,FXint y,FXint w,FXint h);
  void drawDoubleRaisedRectangle(FXint x,FXint y,FXint w,FXint h);
  void drawDoubleSunkenRectangle(FXint x,FXint y,FXint w,FXint h);
  void drawFrame(FXint x,FXint y,FXint w,FXint h);
  void setFrameStyle(FXuint style);
  FXuint getFrameStyle() const;
  FXPixel getHiliteColor() const { return hiliteColor; }
  FXPixel getShadowColor() const { return shadowColor; }
  FXPixel getBorderColor() const { return borderColor; }
  FXPixel getBaseColor() const { return baseColor; }
  FXint getBorderWidth() const { return border; }
  void setHiliteColor(FXPixel clr);
  void setShadowColor(FXPixel clr);
  void setBorderColor(FXPixel clr);
  void setBaseColor(FXPixel clr);
  virtual void popup(FXWindow* own,FXint x,FXint y,FXint w=0,FXint h=0);
  virtual void popdown();
  FXWindow* getOwner() const;
  FXuint getOrientation() const;
  void setOrientation(FXuint orient);
  virtual ~FXPopup();
  };


#endif
