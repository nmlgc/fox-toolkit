/********************************************************************************
*                                                                               *
*            H o m o g e n e o u s   M a t r i x   O p e r a t i o n s          *
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
* $Id: FXHMat.h,v 1.1 1998/03/27 02:33:47 jeroen Exp $                          *
********************************************************************************/
#ifndef FXHMAT_H
#define FXHMAT_H


class FXHMat {
protected:
  FXHVec m[4];
public:
  // Constructors
  FXHMat(){}
  FXHMat(const FXfloat w);
  FXHMat(const FXfloat a00,const FXfloat a01,const FXfloat a02,const FXfloat a03,
         const FXfloat a10,const FXfloat a11,const FXfloat a12,const FXfloat a13,
         const FXfloat a20,const FXfloat a21,const FXfloat a22,const FXfloat a23,
         const FXfloat a30,const FXfloat a31,const FXfloat a32,const FXfloat a33);
  FXHMat(const FXHVec& a,const FXHVec& b,const FXHVec& c,const FXHVec& d);

  // Assignment operators
  FXHMat& operator=(const FXfloat w);
  FXHMat& operator+=(const FXHMat& w);
  FXHMat& operator-=(const FXHMat& w);
  FXHMat& operator*=(const FXfloat w);
  FXHMat& operator*=(const FXHMat& w);
  FXHMat& operator/=(const FXfloat w);

  // Indexing
  FXHVec& operator[](const int i){return m[i];}
  const FXHVec& operator[](const int i) const {return m[i];}

  // Conversion
  operator FXfloat*(){return m[0];}
  operator const FXfloat*() const {return m[0];}

  // Other operators
  friend FXHMat operator+(const FXHMat& a,const FXHMat& b);
  friend FXHMat operator-(const FXHMat& a,const FXHMat& b);
  friend FXHMat operator-(const FXHMat& a);
  friend FXHMat operator*(const FXHMat& a,const FXHMat& b);
  friend FXHMat operator*(const FXfloat x,const FXHMat& a);
  friend FXHMat operator*(const FXHMat& a,const FXfloat x);
  friend FXHMat operator/(const FXHMat& a,const FXfloat x);
  friend FXHMat operator/(const FXfloat x,const FXHMat& a);

  // Multiply matrix and vector
  friend FXHVec operator*(const FXHVec& v,const FXHMat& m);
  friend FXHVec operator*(const FXHMat& a,const FXHVec& v);

  // Mutiply matrix and vector, for non-projective matrix
  friend FXVec operator*(const FXVec& v,const FXHMat& m);
  friend FXVec operator*(const FXHMat& a,const FXVec& v);

  // Set identity matrix
  FXHMat& eye();

  // Multiply by left-hand matrix
  FXHMat& left();

  // Multiply by rotation about unit-quaternion
  FXHMat& rot(const FXQuat& q);

  // Multiply by rotation c,s about axis
  FXHMat& rot(const FXVec& v,const FXfloat c,const FXfloat s);

  // Multiply by rotation of phi about axis
  FXHMat& rot(const FXVec& v,const FXfloat phi);

  // Multiply by x-rotation
  FXHMat& xrot(const FXfloat c,const FXfloat s);
  FXHMat& xrot(const FXfloat phi);

  // Multiply by y-rotation
  FXHMat& yrot(const FXfloat c,const FXfloat s);
  FXHMat& yrot(const FXfloat phi);

  // Multiply by z-rotation
  FXHMat& zrot(const FXfloat c,const FXfloat s);
  FXHMat& zrot(const FXfloat phi);

  // Look at
  FXHMat& look(FXVec& eye,FXVec& cntr,FXVec& vup);

  // Multiply by translation
  FXHMat& trans(const FXfloat tx,const FXfloat ty,const FXfloat tz);
  FXHMat& trans(const FXVec& v);

  // Multiply by scaling
  FXHMat& scale(const FXfloat sx,const FXfloat sy,const FXfloat sz);
  FXHMat& scale(const FXfloat s);
  FXHMat& scale(const FXVec& v);

  // Determinant
  friend FXfloat det(const FXHMat& m);

  // Transpose
  friend FXHMat transpose(const FXHMat& m);

  // Invert
  friend FXHMat invert(const FXHMat& m);

  // Save and load
  friend FXStream& operator<<(FXStream& store,const FXHMat& m);
  friend FXStream& operator>>(FXStream& store,FXHMat& m);
  };

#endif
