/********************************************************************************
*                                                                               *
*                Q u a t e r n i o n  D o u b l e -  F u n c t i o n s          *
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
* $Id: FXDQuat.h,v 1.9 2002/01/18 22:42:52 jeroen Exp $                         *
********************************************************************************/
#ifndef FXDQUAT_H
#define FXDQUAT_H


/// Quaternion (double-precision version)
class FXAPI FXDQuat : public FXDHVec {
public:

  /// Default constructor
  FXDQuat(){}

  /// Construct from axis and angle
  FXDQuat(const FXDVec& axis,FXdouble phi=0.0);

  /// Construct from euler angles yaw (z), pitch (y), and roll (x)
  FXDQuat(FXdouble roll,FXdouble pitch,FXdouble yaw);

  /// Construct from components
  FXDQuat(FXdouble x,FXdouble y,FXdouble z,FXdouble w):FXDHVec(x,y,z,w){}

  /// Adjust quaternion length
  FXDQuat& adjust();

  /// Set quaternion from yaw (z), pitch (y), and roll (x)
  void setRollPitchYaw(FXdouble roll,FXdouble pitch,FXdouble yaw);

  /// Obtain yaw, pitch, and roll
  void getRollPitchYaw(FXdouble& roll,FXdouble& pitch,FXdouble& yaw);

  /// Exponentiate quaternion
  friend FXAPI FXDQuat exp(const FXDQuat& q);

  /// Take logarithm of quaternion
  friend FXAPI FXDQuat log(const FXDQuat& q);

  /// Invert quaternion
  friend FXAPI FXDQuat invert(const FXDQuat& q);

  /// Conjugate quaternion
  friend FXAPI FXDQuat conj(const FXDQuat& q);

  /// Multiply quaternions
  friend FXAPI FXDQuat operator*(const FXDQuat& p,const FXDQuat& q);

  /// Construct quaternion from arc a->b on unit sphere
  friend FXAPI FXDQuat arc(const FXDVec& a,const FXDVec& b);

  /// Spherical lerp
  friend FXAPI FXDQuat lerp(const FXDQuat& u,const FXDQuat& v,FXdouble f);
  };

#endif
