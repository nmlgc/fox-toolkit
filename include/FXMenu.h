/********************************************************************************
*                                                                               *
*                             M e n u   O b j e c t s                           *
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
* $Id: FXMenu.h,v 1.23 1998/10/27 04:57:41 jeroen Exp $                         *
********************************************************************************/
#ifndef FXMENU_H
#define FXMENU_H


// Button masks for menu buttons
enum FXMenuStyle {
  MENU_DEFAULT  = 0x00008000,
  MENU_AUTOGRAY = 0x00010000,
  MENU_AUTOHIDE = 0x00020000,
  };


class FXMenuPane;
class FXMDIButton;
class FXMenuButton;


// Generic menu item
class FXMenuItem : public FXFrame {
  FXDECLARE(FXMenuItem)
protected:
  FXMenuItem(){}
  FXMenuItem(const FXMenuItem&){}
  FXMenuItem(FXComposite* p,FXuint opts=0);
public:
  virtual void create();
  };


// Menu item separator
class FXMenuSeparator : public FXMenuItem {
  FXDECLARE(FXMenuSeparator)
protected:
  FXMenuSeparator(){}
  FXMenuSeparator(const FXMenuSeparator&){}
public:
  long onPaint(FXObject*,FXSelector,void*);
public:
  FXMenuSeparator(FXComposite* p,FXuint opts=0);
  virtual FXint getDefaultHeight();
  };


// Text menu item
class FXMenuEntry : public FXMenuItem {
  FXDECLARE(FXMenuEntry)
protected:
  FXString     label;
  FXString     accel;
  FXString     help;
  FXFont*      font;
  FXint        hotoff;
  FXHotKey     hotkey;
  FXPixel      textColor;
  FXPixel      selbackColor;
  FXPixel      seltextColor;
protected:
  FXMenuEntry(){}
  FXMenuEntry(const FXMenuEntry&){}
  FXMenuEntry(FXComposite* p,const char* text,FXuint opts=0);
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onQueryHelp(FXObject*,FXSelector,void*);
  long onUpdate(FXObject*,FXSelector,void*);
public:
  virtual void create();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  void setText(const FXchar* text);
  const FXchar* getText() const { return label.text(); }
  void setFont(FXFont* fnt);
  FXFont* getFont() const { return font; }
  FXPixel getTextColor() const { return textColor; }
  void setTextColor(FXPixel clr);
  FXPixel getSelBackColor() const { return selbackColor; }
  void setSelBackColor(FXPixel clr);
  FXPixel getSelTextColor() const { return seltextColor; }
  void setSelTextColor(FXPixel clr);
  void setAccelText(const FXchar* text);
  const FXchar* getAccelText() const { return accel.text(); }
  void setHelpText(const FXchar* text);
  const FXchar* getHelpText() const { return help; }
  virtual ~FXMenuEntry();
  };


// Command menu item
class FXMenuCommand : public FXMenuEntry {
  FXDECLARE(FXMenuCommand)
private:
  void drawCheck(FXint l,FXint t,FXint r,FXint b);
  void drawBullit(FXint l,FXint t,FXint r,FXint b);
protected:
  FXMenuCommand(){}
  FXMenuCommand(const FXMenuCommand&){}
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onEnter(FXObject*,FXSelector,void*);
  long onLeave(FXObject*,FXSelector,void*);
  long onLeftBtnPress(FXObject*,FXSelector,void*);
  long onLeftBtnRelease(FXObject*,FXSelector,void*);
  long onActivate(FXObject*,FXSelector,void*);
  long onDeactivate(FXObject*,FXSelector,void*);
  long onCheck(FXObject*,FXSelector,void*);
  long onUncheck(FXObject*,FXSelector,void*);
public:
  FXMenuCommand(FXComposite* p,const char* text=NULL,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0);
  virtual void create();
  virtual FXbool canFocus() const;
  virtual void setFocus();
  virtual void killFocus();
  void setDefault();
  void setOther();
  FXint isDefault() const;
  virtual void enable();
  virtual void disable();
  void check();
  void uncheck();
  FXint isChecked() const;
  void checkRadio();
  void uncheckRadio();
  FXint isRadioChecked() const;
  };


// Cascade menu item
class FXMenuCascade : public FXMenuEntry {
  FXDECLARE(FXMenuCascade)
protected:
  FXMenuPane *pane;
  FXTimer*    timer;
private:
  void drawTriangle(FXint l,FXint t,FXint r,FXint b);
protected:
  FXMenuCascade(){}
  FXMenuCascade(const FXMenuCascade&){}
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onEnter(FXObject*,FXSelector,void*);
  long onLeave(FXObject*,FXSelector,void*);
  long onTimeout(FXObject*,FXSelector,void*);
  long onFocusRight(FXObject*,FXSelector,void*);
  long onFocusLeft(FXObject*,FXSelector,void*);
  long onLeftBtnPress(FXObject*,FXSelector,void*);
  long onLeftBtnRelease(FXObject*,FXSelector,void*);
  long onKeyPress(FXObject*,FXSelector,void*);
  long onKeyRelease(FXObject*,FXSelector,void*);
  long onActivate(FXObject*,FXSelector,void*);
  long onDeactivate(FXObject*,FXSelector,void*);
public:
  FXMenuCascade(FXComposite* p,const char* text,FXMenuPane* pup=NULL,FXuint opts=0);
  virtual void create();
  virtual void destroy();
  virtual FXbool canFocus() const;
  virtual void setFocus();
  virtual void killFocus();
  void setPopup(FXMenuPane *pup){ pane = pup; }
  FXMenuPane* getPopup() const { return pane; }
  virtual FXbool contains(FXint parentx,FXint parenty) const;
  virtual ~FXMenuCascade();
  };


// Menu title button
class FXMenuTitle : public FXMenuEntry {
  FXDECLARE(FXMenuTitle)
protected:
  FXMenuPane *pane;
protected:
  FXMenuTitle(){}
  FXMenuTitle(const FXMenuTitle&){}
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onEnter(FXObject*,FXSelector,void*);
  long onLeave(FXObject*,FXSelector,void*);
  long onLeftBtnPress(FXObject*,FXSelector,void*);
  long onLeftBtnRelease(FXObject*,FXSelector,void*);
  long onKeyPress(FXObject*,FXSelector,void*);
  long onKeyRelease(FXObject*,FXSelector,void*);
  long onActivate(FXObject*,FXSelector,void*);
  long onDeactivate(FXObject*,FXSelector,void*);
  long onFocusUp(FXObject*,FXSelector,void*);
  long onFocusDown(FXObject*,FXSelector,void*);
  long onFocusIn(FXObject*,FXSelector,void*);
  long onFocusOut(FXObject*,FXSelector,void*);
  long onCmdPost(FXObject*,FXSelector,void*);
  long onCmdUnpost(FXObject*,FXSelector,void*);
public:
  FXMenuTitle(FXComposite* p,const char* text=NULL,FXMenuPane* pup=NULL,FXuint opts=LAYOUT_LEFT);
  virtual void create();
  virtual FXbool canFocus() const;
  virtual void setFocus();
  virtual void killFocus();
  void setPopup(FXMenuPane *pup){ pane = pup; }
  FXMenuPane* getPopup() const { return pane; }
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  virtual FXbool contains(FXint parentx,FXint parenty) const;
  virtual ~FXMenuTitle();
  };


// Popup menu pane
class FXMenuPane : public FXPopup {
  FXDECLARE(FXMenuPane)
protected:
  FXMenuPane(){}
  FXMenuPane(const FXMenuPane&){}
public:
  FXMenuPane(FXApp* a,FXuint opts=0);
  virtual FXbool contains(FXint parentx,FXint parenty) const;
  };


#endif
