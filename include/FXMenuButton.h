/********************************************************************************
*                                                                               *
*                       M e n u    B u t t o n    O b j e c t                   *
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
* $Id: FXMenuButton.h,v 1.3 1998/10/30 04:49:05 jeroen Exp $                    *
********************************************************************************/
#ifndef FXMENUBUTTON_H
#define FXMENUBUTTON_H

class FXMenuPane;


// Menu button options
enum FXMenuButtonStyle {
  MENUBUTTON_DOWN   = 0,
  MENUBUTTON_UP     = 0x04000000,
  MENUBUTTON_WINDOW = 0x08000000,
  MENUBUTTON_MASK = MENUBUTTON_UP|MENUBUTTON_WINDOW
  };
  
  

// Menu button
class FXMenuButton : public FXButton {
  FXDECLARE(FXMenuButton)
protected:
  FXMenuPane *pane;
protected:
  FXMenuButton();
  FXMenuButton(const FXMenuButton&){}
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onMotion(FXObject*,FXSelector,void*);
  long onEnter(FXObject*,FXSelector,void*);
  long onLeave(FXObject*,FXSelector,void*);
  long onLeftBtnPress(FXObject*,FXSelector,void*);
  long onLeftBtnRelease(FXObject*,FXSelector,void*);
  long onKeyPress(FXObject*,FXSelector,void*);
  long onKeyRelease(FXObject*,FXSelector,void*);
  long onActivate(FXObject*,FXSelector,void*);
  long onDeactivate(FXObject*,FXSelector,void*);
  long onCmdPost(FXObject*,FXSelector,void*);
  long onCmdUnpost(FXObject*,FXSelector,void*);
public:
  FXMenuButton(FXComposite* p,const char* text=NULL,FXIcon* ic=NULL,FXMenuPane* pup=NULL,FXuint opts=JUSTIFY_NORMAL|ICON_BEFORE_TEXT|MENUBUTTON_DOWN,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD);
  virtual void create();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  virtual void killFocus();
  virtual FXbool contains(FXint parentx,FXint parenty) const;
  void setPopup(FXMenuPane *pup){ pane = pup; }
  FXMenuPane* getPopup() const { return pane; }
  void setPopupStyle(FXuint style);
  FXuint getPopupStyle() const;
  virtual ~FXMenuButton();
  };

#endif
