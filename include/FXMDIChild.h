/********************************************************************************
*                                                                               *
*             M u l t i p l e  D o c u m e n t   C h i l d  W i n d o w         *
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
* $Id: FXMDIChild.h,v 1.19 1998/09/24 06:40:04 jeroen Exp $                     *
********************************************************************************/
#ifndef FXMDICHILD_H
#define FXMDICHILD_H

class FXMDIClient;


// MDI Child Window styles
enum FXMDIChildStyle {
  MDI_NORMAL    = 0,
  MDI_MAXIMIZED = 0x00001000,
  MDI_MINIMIZED = 0x00002000
  };

  

// Base composite
class FXMDIChild : public FXComposite {
  FXDECLARE(FXMDIChild)
protected:
  FXString      title;
  FXMenuButton *windowbtn;
  FXButton     *minimizebtn;
  FXButton     *restorebtn;
  FXButton     *maximizebtn;
  FXButton     *deletebtn;
  FXFont       *font;
  FXPixel       baseColor;
  FXPixel       hiliteColor;
  FXPixel       shadowColor;
  FXPixel       borderColor;
  FXPixel       titleColor;
  FXPixel       titleBackColor;
  FXint         xoff;
  FXint         yoff;
  FXint         oldx;
  FXint         oldy;
  FXint         oldw;
  FXint         oldh;
  FXint         iconPosX;
  FXint         iconPosY;
  FXint         iconWidth;
  FXint         iconHeight;
  FXint         normalPosX;
  FXint         normalPosY;
  FXint         normalWidth;
  FXint         normalHeight;
  FXuchar       action;
protected:
  FXMDIChild();
  FXMDIChild(const FXMDIChild&){}
  virtual void layout();
  void drawDoubleRaisedRectangle(FXint x,FXint y,FXint w,FXint h);
  void drawRaisedRectangle(FXint x,FXint y,FXint w,FXint h);
  void drawRubberBox(FXint x,FXint y,FXint w,FXint h);
  FXuchar where(FXint x,FXint y);
  void changeCursor(FXint x,FXint y);
  void revertCursor();
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onEnter(FXObject*,FXSelector,void*);
  long onLeave(FXObject*,FXSelector,void*);
  long onFocusIn(FXObject*,FXSelector,void*);
  long onFocusOut(FXObject*,FXSelector,void*);
  long onRightBtnPress(FXObject*,FXSelector,void*);
  long onRightBtnRelease(FXObject*,FXSelector,void*);
  long onLeftBtnPress(FXObject*,FXSelector,void*);
  long onLeftBtnRelease(FXObject*,FXSelector,void*);
  long onMotion(FXObject*,FXSelector,void*);
public:
  FXMDIChild(FXMDIClient* p,const char* name,FXIcon* ic=NULL,FXMenuPane* mn=NULL,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual void create();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  virtual FXbool canFocus() const;
  virtual void move(FXint x,FXint y);
  virtual void resize(FXint w,FXint h);
  virtual void position(FXint x,FXint y,FXint w,FXint h);
  virtual long handle(FXObject* sender,FXSelector key,void* data);
  FXWindow *contentWindow() const;
  void setTitle(const char* name);
  const char* getTitle() const { return title.text(); }
  FXint getTitleHeight() const;
  FXint getTitleWidth() const;
  FXPixel getHiliteColor() const { return hiliteColor; }
  FXPixel getShadowColor() const { return shadowColor; }
  FXPixel getBaseColor() const { return baseColor; }
  FXPixel getBorderColor() const { return borderColor; }
  void setHiliteColor(FXPixel clr);
  void setShadowColor(FXPixel clr);
  void setBaseColor(FXPixel clr);
  void setBorderColor(FXPixel clr);
  FXbool isMaximized() const;
  FXbool isMinimized() const;
  FXint getIconX() const { return iconPosX; }
  FXint getIconY() const { return iconPosY; }
  FXint getIconWidth() const { return iconWidth; }
  FXint getIconHeight() const { return iconHeight; }
  FXint getNormalX() const { return normalPosX; }
  FXint getNormalY() const { return normalPosY; }
  FXint getNormalWidth() const { return normalWidth; }
  FXint getNormalHeight() const { return normalHeight; }
  virtual void setFocus();
  void maximize();
  void minimize();
  void restore();
  void setActive();
  void setInactive();
  FXIcon *getWindowIcon() const;
  void setWindowIcon(FXIcon* icon);
  FXMenuPane* getWindowMenu() const;
  void setWindowMenu(FXMenuPane* menu);
  void setFont(FXFont *fnt);
  FXFont* getFont() const { return font; }
  virtual ~FXMDIChild();
  };

  
#endif
