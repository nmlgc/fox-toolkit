/********************************************************************************
*                                                                               *
*                    Q u a t e r n i o n   F u n c t i o n s                    *
*                                                                               *
*********************************************************************************
* Copyright (C) 1994,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This library is free software; you can redistribute it and/or                 *
* modify it under the terms of the GNU Lesser General Public                    *
* License as published by the Free Software Foundation; either                  *
* version 2.1 of the License, or (at your option) any later version.            *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU             *
* Lesser General Public License for more details.                               *
*                                                                               *
* You should have received a copy of the GNU Lesser General Public              *
* License along with this library; if not, write to the Free Software           *
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.    *
*********************************************************************************
* $Id: FXQuat.h,v 1.11 2002/01/18 22:55:03 jeroen Exp $                         *
********************************************************************************/
#ifndef FXQUAT_H
#define FXQUAT_H


/// Quaternion (single-precision version)
class FXAPI FXQuat : public FXHVec {
public:

  /// Constructors
  FXQuat(){}

  /// Construct from axis and angle
  FXQuat(const FXVec& axis,FXfloat phi=0.0);

  /// Construct from euler angles yaw (z), pitch (y), and roll (x)
  FXQuat(FXfloat roll,FXfloat pitch,FXfloat yaw);

  /// Construct from components
  FXQuat(FXfloat x,FXfloat y,FXfloat z,FXfloat w):FXHVec(x,y,z,w){}

  /// Adjust quaternion length
  FXQuat& adjust();

  /// Set quaternion from yaw (z), pitch (y), and roll (x)
  void setRollPitchYaw(FXfloat roll,FXfloat pitch,FXfloat yaw);

  /// Obtain yaw, pitch, and roll
  void getRollPitchYaw(FXfloat& roll,FXfloat& pitch,FXfloat& yaw);

  /// Exponentiate quaternion
  friend FXAPI FXQuat exp(const FXQuat& q);

  /// Take logarithm of quaternion
  friend FXAPI FXQuat log(const FXQuat& q);

  /// Invert quaternion
  friend FXAPI FXQuat invert(const FXQuat& q);

  /// Conjugate quaternion
  friend FXAPI FXQuat conj(const FXQuat& q);

  /// Multiply quaternions
  friend FXAPI FXQuat operator*(const FXQuat& p,const FXQuat& q);

  /// Construct quaternion from arc a->b on unit sphere
  friend FXAPI FXQuat arc(const FXVec& a,const FXVec& b);

  /// Spherical lerp
  friend FXAPI FXQuat lerp(const FXQuat& u,const FXQuat& v,FXfloat f);
  };

#endif
