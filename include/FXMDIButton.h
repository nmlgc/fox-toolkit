/********************************************************************************
*                                                                               *
*             M u l t i p l e  D o c u m e n t   B u t t o n                    *
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
* $Id: FXMDIButton.h,v 1.4 1998/10/30 04:49:05 jeroen Exp $                     *
********************************************************************************/
#ifndef FXMDIBUTTON_H
#define FXMDIBUTTON_H

// MDI Button styles
enum FXMDIButtonStyle {
  MDIBTN_DELETE   = 0,
  MDIBTN_MINIMIZE = 0x04000000,
  MDIBTN_MAXIMIZE = 0x08000000,
  MDIBTN_RESTORE  = 0x0C000000,
  MDIBTN_MASK     = MDIBTN_RESTORE
  };
  
  
// MDI button
class FXMDIButton : public FXButton {
  FXDECLARE(FXMDIButton)
protected:
  FXMDIButton(){}
  FXMDIButton(const FXMDIButton&){}
public:
  long onPaint(FXObject*,FXSelector,void*);
public:
  FXMDIButton(FXComposite* p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=FRAME_RAISED|FRAME_THICK,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  FXuint getMDIButtonStyle() const;
  void setMDIButtonStyle(FXuint style);
  };


// MDI Window Menu
class FXMDIMenu : public FXMenuPane {
  FXDECLARE(FXMDIMenu)
protected:
  FXMDIMenu(){}
  FXMDIMenu(const FXMDIMenu&){}
public:
  FXMDIMenu(FXApp* a,FXObject* tgt=NULL);
  };


#endif
