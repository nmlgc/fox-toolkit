/********************************************************************************
*                                                                               *
*     H o m o g e n e o u s   D o u b l e - M a t r i x   O p e r a t i o n s   *
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
* $Id: FXDHMat.h,v 1.9 2002/01/18 22:42:51 jeroen Exp $                         *
********************************************************************************/
#ifndef FXDHMAT_H
#define FXDHMAT_H


/// Homogeneous matrix (double-precision version)
class FXAPI FXDHMat {
protected:
  FXDHVec m[4];
public:
  /// Constructors
  FXDHMat(){}
  FXDHMat(const FXDHMat& other);
  FXDHMat(FXdouble w);
  FXDHMat(FXdouble a00,FXdouble a01,FXdouble a02,FXdouble a03,
          FXdouble a10,FXdouble a11,FXdouble a12,FXdouble a13,
          FXdouble a20,FXdouble a21,FXdouble a22,FXdouble a23,
          FXdouble a30,FXdouble a31,FXdouble a32,FXdouble a33);
  FXDHMat(const FXDHVec& a,const FXDHVec& b,const FXDHVec& c,const FXDHVec& d);

  /// Assignment operators
  FXDHMat& operator=(const FXDHMat& other);
  FXDHMat& operator=(FXdouble w);
  FXDHMat& operator+=(const FXDHMat& w);
  FXDHMat& operator-=(const FXDHMat& w);
  FXDHMat& operator*=(FXdouble w);
  FXDHMat& operator*=(const FXDHMat& w);
  FXDHMat& operator/=(FXdouble w);

  /// Return a non-const reference to the ith row
  FXDHVec& operator[](FXint i){return m[i];}

  /// Return a const reference to the ith row
  const FXDHVec& operator[](FXint i) const {return m[i];}

  /// Conversion
  operator FXdouble*(){return m[0];}
  operator const FXdouble*() const {return m[0];}

  /// Other operators
  friend FXAPI FXDHMat operator+(const FXDHMat& a,const FXDHMat& b);
  friend FXAPI FXDHMat operator-(const FXDHMat& a,const FXDHMat& b);
  friend FXAPI FXDHMat operator-(const FXDHMat& a);
  friend FXAPI FXDHMat operator*(const FXDHMat& a,const FXDHMat& b);
  friend FXAPI FXDHMat operator*(FXdouble x,const FXDHMat& a);
  friend FXAPI FXDHMat operator*(const FXDHMat& a,FXdouble x);
  friend FXAPI FXDHMat operator/(const FXDHMat& a,FXdouble x);
  friend FXAPI FXDHMat operator/(FXdouble x,const FXDHMat& a);

  /// Multiply matrix and vector
  friend FXAPI FXDHVec operator*(const FXDHVec& v,const FXDHMat& m);
  friend FXAPI FXDHVec operator*(const FXDHMat& a,const FXDHVec& v);

  /// Mutiply matrix and vector, for projective matrix
  friend FXAPI FXDVec operator*(const FXDVec& v,const FXDHMat& m);
  friend FXAPI FXDVec operator*(const FXDHMat& a,const FXDVec& v);

  /// Set to identity matrix
  FXDHMat& eye();

  /// Orthographic projection
  FXDHMat& ortho(FXdouble left,FXdouble right,FXdouble bottom,FXdouble top,FXdouble hither,FXdouble yon);

  /// Perspective projection
  FXDHMat& frustum(FXdouble left,FXdouble right,FXdouble bottom,FXdouble top,FXdouble hither,FXdouble yon);

  /// Multiply by left-hand matrix
  FXDHMat& left();

  /// Multiply by rotation about unit-quaternion
  FXDHMat& rot(const FXDQuat& q);

  /// Multiply by rotation c,s about axis
  FXDHMat& rot(const FXDVec& v,FXdouble c,FXdouble s);

  /// Multiply by rotation of phi about axis
  FXDHMat& rot(const FXDVec& v,FXdouble phi);

  /// Multiply by x-rotation
  FXDHMat& xrot(FXdouble c,FXdouble s);
  FXDHMat& xrot(FXdouble phi);

  /// Multiply by y-rotation
  FXDHMat& yrot(FXdouble c,FXdouble s);
  FXDHMat& yrot(FXdouble phi);

  /// Multiply by z-rotation
  FXDHMat& zrot(FXdouble c,FXdouble s);
  FXDHMat& zrot(FXdouble phi);

  /// Look at
  FXDHMat& look(const FXDVec& eye,const FXDVec& cntr,const FXDVec& vup);

  /// Multiply by translation
  FXDHMat& trans(FXdouble tx,FXdouble ty,FXdouble tz);
  FXDHMat& trans(const FXDVec& v);

  /// Multiply by scaling
  FXDHMat& scale(FXdouble sx,FXdouble sy,FXdouble sz);
  FXDHMat& scale(FXdouble s);
  FXDHMat& scale(const FXDVec& v);

  /// Determinant
  friend FXAPI FXdouble det(const FXDHMat& m);

  /// Transpose
  friend FXAPI FXDHMat transpose(const FXDHMat& m);

  /// Invert
  friend FXAPI FXDHMat invert(const FXDHMat& m);

  /// Save to a stream
  friend FXAPI FXStream& operator<<(FXStream& store,const FXDHMat& m);

  /// Load from a stream
  friend FXAPI FXStream& operator>>(FXStream& store,FXDHMat& m);
  };

#endif
