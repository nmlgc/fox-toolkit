/********************************************************************************
*                                                                               *
*                             O p t i o n   M e n u                             *
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
* $Id: FXOptionMenu.h,v 1.11 1998/09/09 17:13:58 jvz Exp $                   *
********************************************************************************/
#ifndef FXOPTIONMENU_H
#define FXOPTIONMENU_H


class FXPopup;


// Option
class FXOption : public FXLabel {
  FXDECLARE(FXOption)
protected:
  FXString tip;
  FXString help;
protected:
  FXOption(){}
  FXOption(const FXOption&){}
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onEnter(FXObject*,FXSelector,void*);
  long onLeave(FXObject*,FXSelector,void*);
  long onLeftBtnPress(FXObject*,FXSelector,void*);
  long onLeftBtnRelease(FXObject*,FXSelector,void*);
  long onActivate(FXObject*,FXSelector,void*);
  long onDeactivate(FXObject*,FXSelector,void*);
  long onQueryHelp(FXObject*,FXSelector,void*);
public:
  FXOption(FXComposite* p,const char* text=NULL,FXIcon* ic=NULL,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=JUSTIFY_NORMAL|ICON_BEFORE_TEXT|MENUBUTTON_DOWN,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD);
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  virtual FXbool canFocus() const;
  virtual void setFocus();
  virtual void killFocus();
  void setHelpText(const FXchar* text);
  const FXchar* getHelpText() const { return help; }
  void setTipText(const FXchar* text);
  const FXchar* getTipText() const { return tip; }
  virtual ~FXOption();
  };


  
// Option Menu button
class FXOptionMenu : public FXLabel {
  FXDECLARE(FXOptionMenu)
protected:
  FXPopup  *pane;
  FXOption *current;
protected:
  FXOptionMenu(){}
  FXOptionMenu(const FXOptionMenu&){}
  virtual void layout();
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onLeftBtnPress(FXObject*,FXSelector,void*);
  long onLeftBtnRelease(FXObject*,FXSelector,void*);
  long onFocusIn(FXObject*,FXSelector,void*);
  long onFocusOut(FXObject*,FXSelector,void*);
  long onMotion(FXObject*,FXSelector,void*);
  long onKeyPress(FXObject*,FXSelector,void*);
  long onKeyRelease(FXObject*,FXSelector,void*);
  long onActivate(FXObject*,FXSelector,void*);
  long onDeactivate(FXObject*,FXSelector,void*);
  long onCmdPost(FXObject*,FXSelector,void*);
  long onCmdUnpost(FXObject*,FXSelector,void*);
  long onQueryHelp(FXObject*,FXSelector,void*);
  long onQueryTip(FXObject*,FXSelector,void*);
public:
  FXOptionMenu(FXComposite* p,FXPopup* pup=NULL,FXuint opts=JUSTIFY_NORMAL|ICON_BEFORE_TEXT,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD);
  virtual void create();
  virtual void destroy();
  virtual void killFocus();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  virtual FXbool contains(FXint parentx,FXint parenty) const;
  void setCurrent(FXOption *win);
  FXOption* getCurrent() const { return current; }
  void setCurrentNo(FXint no);
  FXint getCurrentNo() const;
  void setPopup(FXPopup *pup);
  FXPopup* getPopup() const { return pane; }
  virtual FXbool canFocus() const;
  FXbool isPopped() const;
  virtual ~FXOptionMenu();
  };


#endif
