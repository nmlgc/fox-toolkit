/********************************************************************************
*                                                                               *
*                         T o p   W i n d o w   O b j e c t                     *
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
* $Id: FXTopWindow.h,v 1.6 1998/08/26 07:41:13 jeroen Exp $                     *
********************************************************************************/
#ifndef FXTOPWINDOW_H
#define FXTOPWINDOW_H


// Decorations
enum FXDecorations {
  DECOR_TITLE    = 0x00020000,        // Window title
  DECOR_MINIMIZE = 0x00040000,        // Minimize button
  DECOR_MAXIMIZE = 0x00080000,        // Maximize button
  DECOR_CLOSE    = 0x00100000,        // Close button
  DECOR_BORDER   = 0x00200000,        // Border
  DECOR_RESIZE   = 0x00400000,        // Resize handles
  DECOR_MENU     = 0x00800000,        // Window menu
  DECOR_ALL      = (DECOR_TITLE|DECOR_MINIMIZE|DECOR_MAXIMIZE|DECOR_CLOSE|DECOR_BORDER|DECOR_RESIZE|DECOR_MENU)
  };
  

// Top level window 
class FXTopWindow : public FXShell {
  FXDECLARE(FXTopWindow)
protected:
  FXString title;
  FXint    hspacing;
  FXint    vspacing;
public:
  long onEnter(FXObject*,FXSelector,void*);
  long onLeave(FXObject*,FXSelector,void*);
  long onClose(FXObject*,FXSelector,void*);
  long onCmdClose(FXObject*,FXSelector,void*);
protected:
  FXTopWindow(){}
  FXTopWindow(const FXTopWindow&){}
  virtual void layout();
  void settitle();
  void setdecorations();
  FXTopWindow(FXApp* a,const char* name,FXuint opts=DECOR_ALL,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint hs=4,FXint vs=4);
public:
  enum {
    ID_CLOSE=FXShell::ID_LAST,
    ID_LAST
    };  
public:
  virtual void create();
  virtual void show();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  virtual void move(FXint x,FXint y);
  virtual void resize(FXint w,FXint h);
  virtual void position(FXint x,FXint y,FXint w,FXint h);
  void setTitle(const char* name);
  const char* getTitle() const { return title.text(); }
  FXint getHSpacing() const { return hspacing; }
  FXint getVSpacing() const { return vspacing; }
  void setHSpacing(FXint hs);
  void setVSpacing(FXint vs);
  void setDecorations(FXDecorations decorations);
  FXDecorations getDecorations() const;
  virtual ~FXTopWindow();
  };


#endif
