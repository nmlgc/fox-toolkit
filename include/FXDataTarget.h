/********************************************************************************
*                                                                               *
*                              D a t a   T a r g e t                            *
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
* $Id: FXDataTarget.h,v 1.8 1998/10/29 07:42:21 jeroen Exp $                    *
********************************************************************************/
#ifndef FXDATATARGET_H
#define FXDATATARGET_H



class FXDataTarget : public FXObject {
  FXDECLARE(FXDataTarget)
protected:
  FXuint  type;
  void   *data;
public:
  long onCmdValue(FXObject*,FXSelector,void*);
  long onUpdValue(FXObject*,FXSelector,void*);
public:
  enum {
    DT_VOID = 0,
    DT_CHAR,
    DT_UCHAR,
    DT_SHORT,
    DT_USHORT,
    DT_INT,
    DT_UINT,
    DT_FLOAT,
    DT_DOUBLE,
    DT_STRING,
    DT_LAST
    };
public:
  enum {
    ID_VALUE=1,
    ID_LAST
    };
public:
  FXDataTarget():type(DT_VOID),data(NULL){}
  FXDataTarget(FXchar& value):type(DT_CHAR),data(&value){}
  FXDataTarget(FXuchar& value):type(DT_UCHAR),data(&value){}
  FXDataTarget(FXshort& value):type(DT_SHORT),data(&value){}
  FXDataTarget(FXushort& value):type(DT_USHORT),data(&value){}
  FXDataTarget(FXint& value):type(DT_INT),data(&value){}
  FXDataTarget(FXuint& value):type(DT_UINT),data(&value){}
  FXDataTarget(FXfloat& value):type(DT_FLOAT),data(&value){}
  FXDataTarget(FXdouble& value):type(DT_DOUBLE),data(&value){}
  FXDataTarget(FXString& value):type(DT_STRING),data(&value){}
  };
  


#endif
