/********************************************************************************
*                                                                               *
*                              D e b u g   T a r g e t                          *
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
* $Id: FXDebugTarget.h,v 1.2 1998/09/04 04:58:52 jeroen Exp $                   *
********************************************************************************/
#ifndef FXDEBUGTARGET_H
#define FXDEBUGTARGET_H



class FXDebugTarget : public FXObject {
  FXDECLARE(FXDebugTarget)
  FXObject   *lastsender;
  FXSelector  lastsel;
  FXuint      count;
public:
  long onMessage(FXObject*,FXSelector,void*);
public:
  FXDebugTarget();
  };
  

extern const char *const messageTypeName[];


#endif
