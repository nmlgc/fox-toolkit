/********************************************************************************
*                                                                               *
*            H o m o g e n e o u s   M a t r i x   O p e r a t i o n s          *
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
* $Id: FXHMat.h,v 1.9 2002/01/18 22:42:53 jeroen Exp $                          *
********************************************************************************/
#ifndef FXHMAT_H
#define FXHMAT_H


/// Homogeneous matrix (single-precision version)
class FXAPI FXHMat {
protected:
  FXHVec m[4];
public:
  /// Constructors
  FXHMat(){}
  FXHMat(FXfloat w);
  FXHMat(FXfloat a00,FXfloat a01,FXfloat a02,FXfloat a03,
         FXfloat a10,FXfloat a11,FXfloat a12,FXfloat a13,
         FXfloat a20,FXfloat a21,FXfloat a22,FXfloat a23,
         FXfloat a30,FXfloat a31,FXfloat a32,FXfloat a33);
  FXHMat(const FXHVec& a,const FXHVec& b,const FXHVec& c,const FXHVec& d);
  FXHMat(const FXHMat& other);

  /// Assignment operators
  FXHMat& operator=(const FXHMat& other);
  FXHMat& operator=(FXfloat w);
  FXHMat& operator+=(const FXHMat& w);
  FXHMat& operator-=(const FXHMat& w);
  FXHMat& operator*=(FXfloat w);
  FXHMat& operator*=(const FXHMat& w);
  FXHMat& operator/=(FXfloat w);

  /// Indexing
  FXHVec& operator[](FXint i){return m[i];}
  const FXHVec& operator[](FXint i) const {return m[i];}

  /// Conversion
  operator FXfloat*(){return m[0];}
  operator const FXfloat*() const {return m[0];}

  /// Other operators
  friend FXAPI FXHMat operator+(const FXHMat& a,const FXHMat& b);
  friend FXAPI FXHMat operator-(const FXHMat& a,const FXHMat& b);
  friend FXAPI FXHMat operator-(const FXHMat& a);
  friend FXAPI FXHMat operator*(const FXHMat& a,const FXHMat& b);
  friend FXAPI FXHMat operator*(FXfloat x,const FXHMat& a);
  friend FXAPI FXHMat operator*(const FXHMat& a,FXfloat x);
  friend FXAPI FXHMat operator/(const FXHMat& a,FXfloat x);
  friend FXAPI FXHMat operator/(FXfloat x,const FXHMat& a);

  /// Multiply matrix and vector
  friend FXAPI FXHVec operator*(const FXHVec& v,const FXHMat& m);
  friend FXAPI FXHVec operator*(const FXHMat& a,const FXHVec& v);

  /// Mutiply matrix and vector, for non-projective matrix
  friend FXAPI FXVec operator*(const FXVec& v,const FXHMat& m);
  friend FXAPI FXVec operator*(const FXHMat& a,const FXVec& v);

  /// Set identity matrix
  FXHMat& eye();

  /// Orthographic projection
  FXHMat& ortho(FXfloat left,FXfloat right,FXfloat bottom,FXfloat top,FXfloat hither,FXfloat yon);

  /// Perspective projection
  FXHMat& frustum(FXfloat left,FXfloat right,FXfloat bottom,FXfloat top,FXfloat hither,FXfloat yon);

  /// Multiply by left-hand matrix
  FXHMat& left();

  /// Multiply by rotation about unit-quaternion
  FXHMat& rot(const FXQuat& q);

  /// Multiply by rotation c,s about axis
  FXHMat& rot(const FXVec& v,FXfloat c,FXfloat s);

  /// Multiply by rotation of phi about axis
  FXHMat& rot(const FXVec& v,FXfloat phi);

  /// Multiply by x-rotation
  FXHMat& xrot(FXfloat c,FXfloat s);
  FXHMat& xrot(FXfloat phi);

  /// Multiply by y-rotation
  FXHMat& yrot(FXfloat c,FXfloat s);
  FXHMat& yrot(FXfloat phi);

  /// Multiply by z-rotation
  FXHMat& zrot(FXfloat c,FXfloat s);
  FXHMat& zrot(FXfloat phi);

  /// Look at
  FXHMat& look(const FXVec& eye,const FXVec& cntr,const FXVec& vup);

  /// Multiply by translation
  FXHMat& trans(FXfloat tx,FXfloat ty,FXfloat tz);
  FXHMat& trans(const FXVec& v);

  /// Multiply by scaling
  FXHMat& scale(FXfloat sx,FXfloat sy,FXfloat sz);
  FXHMat& scale(FXfloat s);
  FXHMat& scale(const FXVec& v);

  /// Determinant
  friend FXAPI FXfloat det(const FXHMat& m);

  /// Transpose
  friend FXAPI FXHMat transpose(const FXHMat& m);

  /// Invert
  friend FXAPI FXHMat invert(const FXHMat& m);

  /// Save to a stream
  friend FXAPI FXStream& operator<<(FXStream& store,const FXHMat& m);

  /// Load from a stream
  friend FXAPI FXStream& operator>>(FXStream& store,FXHMat& m);
  };

#endif
