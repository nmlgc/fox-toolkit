/********************************************************************************
*                                                                               *
*                S p l i t t e r   W i n d o w   O b j e c t                    *
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
* $Id: FXSplitter.h,v 1.4 1998/08/26 07:41:13 jeroen Exp $                      *
********************************************************************************/
#ifndef FXSPLITTER_H
#define FXSPLITTER_H



// Splitter options
enum FXSplitterStyle {
  SPLITTER_HORIZONTAL = 0,
  SPLITTER_VERTICAL   = 0x00008000,
  SPLITTER_REVERSED   = 0x00010000
  };



// Splitter window 
class FXSplitter : public FXComposite {
  FXDECLARE(FXSplitter)
private:
  FXWindow *win;
  FXint     split;
  FXint     off;
protected:
  FXSplitter();
  FXSplitter(const FXSplitter&){}
  FXWindow* findHSplit(FXint pos);
  FXWindow* findVSplit(FXint pos);
  void moveHSplit(FXint amount);
  void moveVSplit(FXint amount);
  void drawHSplit(FXint pos);
  void drawVSplit(FXint pos);
  void adjustHLayout();
  void adjustVLayout();
  virtual void layout();
public:
  long onLButtonPress(FXObject*,FXSelector,void*);
  long onLButtonRelease(FXObject*,FXSelector,void*);
  long onMotion(FXObject*,FXSelector,void*);
public:
  FXSplitter(FXComposite* p,FXuint opts=SPLITTER_HORIZONTAL,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual void create();
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  };


#endif
