/********************************************************************************
*                                                                               *
*                               T a b   O b j e c t                             *
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
* $Id: FXTab.h,v 1.5 1998/08/26 07:41:13 jeroen Exp $                           *
********************************************************************************/
#ifndef FXTAB_H
#define FXTAB_H


// Tab Item orientations which affect border 
enum FXTabStyle {
  TAB_TOP          = 0,           // Top side tabs
  TAB_LEFT         = 0x00800000,  // Left side tabs
  TAB_RIGHT        = 0x01000000,  // Right side tabs
  TAB_BOTTOM       = 0x01800000,  // Bottom side tabs
  TAB_ORIENT_MASK  = TAB_TOP|TAB_LEFT|TAB_RIGHT|TAB_BOTTOM,
  TAB_TOP_NORMAL   = JUSTIFY_NORMAL|ICON_BEFORE_TEXT|TAB_TOP,
  TAB_BOTTOM_NORMAL= JUSTIFY_NORMAL|ICON_BEFORE_TEXT|TAB_BOTTOM,
  TAB_LEFT_NORMAL  = JUSTIFY_LEFT|JUSTIFY_CENTER_Y|ICON_BEFORE_TEXT|TAB_LEFT,
  TAB_RIGHT_NORMAL = JUSTIFY_LEFT|JUSTIFY_CENTER_Y|ICON_BEFORE_TEXT|TAB_RIGHT
  };

  
// Tab Book options
enum FXTabBookStyle {
  TABBOOK_TOPTABS    = 0x00008000,
  TABBOOK_BOTTOMTABS = 0x00010000,
  TABBOOK_LEFTTABS   = 0x00020000,
  TABBOOK_RIGHTTABS  = 0x00040000
  };


// Generic tab item
class FXTabItem : public FXLabel {
  FXDECLARE(FXTabItem)
protected:
  FXString tip;
  FXString help;
protected:
  FXTabItem(){}
  FXTabItem(const FXTabItem&){}
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onQueryHelp(FXObject*,FXSelector,void*);
  long onQueryTip(FXObject*,FXSelector,void*);
public:
  FXTabItem(FXComposite* p,const char* text=NULL,FXIcon* ic=0,FXuint opts=TAB_TOP_NORMAL);
  void setHelpText(const FXchar* text);
  const FXchar* getHelpText() const { return help; }
  void setTipText(const FXchar* text);
  const FXchar* getTipText() const { return tip; }
  };


// Switcher widget aka sWitch :-)
class FXSwitcher : public FXComposite {
  FXDECLARE(FXSwitcher)
protected:
  FXPixel baseColor;
  FXPixel hiliteColor;
  FXPixel shadowColor;
  FXPixel borderColor;
  FXuint  current;  
protected:
  FXSwitcher(){}
  FXSwitcher(const FXSwitcher&){}
  virtual void layout();
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onCmdMakeTop(FXObject*,FXSelector,void*);
  long onUpdMakeTop(FXObject*,FXSelector,void*);
public:
  enum {
    ID_MAKETOP_FIRST=FXComposite::ID_LAST,
    ID_MAKETOP_LAST=ID_MAKETOP_FIRST+999,
    ID_LAST
    };
public:
  FXSwitcher(FXComposite *p,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual void create();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  virtual void setCurrent(FXuint f);
  FXuint getCurrent() const { return current; }
  FXPixel getHiliteColor() const { return hiliteColor; }
  FXPixel getShadowColor() const { return shadowColor; }
  FXPixel getBorderColor() const { return borderColor; }
  FXPixel getBaseColor() const { return baseColor; }
  void setHiliteColor(FXPixel clr);
  void setShadowColor(FXPixel clr);
  void setBorderColor(FXPixel clr);
  void setBaseColor(FXPixel clr);
  };


// Tab bar 
class FXTabBar : public FXComposite {
  FXDECLARE(FXTabBar)
protected:
  FXint active;
public:
  long onButtonPress(FXObject*,FXSelector,void*);
  long onButtonRelease(FXObject*,FXSelector,void*);
  long onFocusIn(FXObject*,FXSelector,void*);
  long onFocusOut(FXObject*,FXSelector,void*);
protected:
  FXTabBar(){}
  FXTabBar(const FXTabBar&){}
  virtual void layout();
public:
  FXTabBar(FXComposite* p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual void create();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  virtual void setActive(FXint f);
  FXint getActive() const { return active; }
  };


// Tab bar 
class FXTabBook : public FXTabBar {
  FXDECLARE(FXTabBook)
protected:
  FXSwitcher* switcher;
protected:
  FXTabBook(){}
  FXTabBook(const FXTabBook&){}
  FXint tabwidth() const;
  FXint tabheight() const;
  virtual void layout();
public:
  long onButtonPress(FXObject*,FXSelector,void*);
  long onButtonRelease(FXObject*,FXSelector,void*);
public:
  FXTabBook(FXComposite* p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  virtual void setActive(FXint f);
  FXSwitcher* getContents() const { return switcher; }
  };



#endif
