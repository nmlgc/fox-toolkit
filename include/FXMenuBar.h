/********************************************************************************
*                                                                               *
*                              M e n u  B a r                                   *
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
* $Id: FXMenuBar.h,v 1.3 1998/09/11 05:28:42 jeroen Exp $                       *
********************************************************************************/
#ifndef FXMENUBAR_H
#define FXMENUBAR_H


// MDI Style for menubar
enum FXMenuBarStyle {
  MENUBAR_NORMAL = 0,
  MENUBAR_MDI    = 0x00020000
  };


class FXMenuPane;
class FXMenuButton;


// Menu bar 
class FXMenuBar : public FXComposite {
  FXDECLARE(FXMenuBar)
protected:
  FXMenuButton *windowbtn;
  FXButton     *minimizebtn;
  FXButton     *restorebtn;
  FXButton     *deletebtn;
  FXPixel       borderColor;
protected:
  FXMenuBar(){}
  FXMenuBar(const FXMenuBar&){}
  virtual void layout();
public:
  long onFocusLeft(FXObject*,FXSelector,void*);
  long onFocusRight(FXObject*,FXSelector,void*);
  long onEnter(FXObject*,FXSelector,void*);
  long onLeave(FXObject*,FXSelector,void*);
  long onMotion(FXObject*,FXSelector,void*);
  long onBtnPress(FXObject*,FXSelector,void*);
  long onBtnRelease(FXObject*,FXSelector,void*);
  long onCmdUnpost(FXObject*,FXSelector,void*);
public:
  FXMenuBar(FXComposite* p,FXuint opts=LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_X|MENUBAR_NORMAL);
  virtual FXbool contains(FXint parentx,FXint parenty) const;
  virtual void create();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  void showMDIControls();
  void hideMDIControls();
  void setMDIClient(FXObject* client);
  FXObject* getMDIClient() const;
  FXIcon *getWindowIcon() const;
  void setWindowIcon(FXIcon* icon);
  FXMenuPane* getWindowMenu() const;
  void setWindowMenu(FXMenuPane* menu);
  virtual ~FXMenuBar();
  };


#endif
