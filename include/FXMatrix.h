/********************************************************************************
*                                                                               *
*                   M a t r i x   C o n t a i n e r   O b j e c t               *
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
* $Id: FXMatrix.h,v 1.4 1998/08/26 07:41:12 jeroen Exp $                        *
********************************************************************************/
#ifndef FXMATRIX_H
#define FXMATRIX_H


// Matrix packing options
enum FXMatrixStyle {
  MATRIX_BY_ROWS        = 0,
  MATRIX_BY_COLUMNS     = 0x00020000
  };
  
  

// Matrix packer
class FXMatrix : public FXPacker {
  FXDECLARE(FXMatrix)
protected:
  FXint  num;
protected:
  FXMatrix(){}
  FXMatrix(const FXMatrix&){}
  virtual void layout();
public:
  long onFocusUp(FXObject*,FXSelector,void*);
  long onFocusDown(FXObject*,FXSelector,void*);
  long onFocusLeft(FXObject*,FXSelector,void*);
  long onFocusRight(FXObject*,FXSelector,void*);
public:
  FXMatrix(FXComposite *p,FXint n=1,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_SPACING,FXint pr=DEFAULT_SPACING,FXint pt=DEFAULT_SPACING,FXint pb=DEFAULT_SPACING,FXint hs=DEFAULT_SPACING,FXint vs=DEFAULT_SPACING);
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  };


#endif
