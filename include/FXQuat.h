/********************************************************************************
*                                                                               *
*                    Q u a t e r n i o n   F u n c t i o n s                    *
*                                                                               *
*********************************************************************************
* Copyright (C) 1994 by Jeroen van der Zijp.   All Rights Reserved.             *
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
* $Id: FXQuat.h,v 1.2 1998/09/02 15:13:44 jvz Exp $                          *
********************************************************************************/
#ifndef FXQuat_H
#define FXQuat_H


class FXQuat : public FXHVec {
public:
  
  // Constructors
  FXQuat(){}
  FXQuat(const FXVec& axis,const FXfloat phi=0.0);
  FXQuat(const FXfloat x,const FXfloat y,const FXfloat z,const FXfloat w):FXHVec(x,y,z,w){}

  // Adjust quaternion length
  FXQuat& adjust();

  // Invert quaternion
  friend FXQuat invert(const FXQuat& q);

  // Conjugate quaternion
  friend FXQuat conj(const FXQuat& q);

  // Multiply quaternions
  friend FXQuat operator*(const FXQuat& p,const FXQuat& q);

  // Construct quaternion from arc a->b on unit sphere
  friend FXQuat arc(const FXVec& a,const FXVec& b);

  // Spherical lerp
  friend FXQuat lerp(const FXQuat& u,const FXQuat& v,const FXfloat f);
  };

#endif
