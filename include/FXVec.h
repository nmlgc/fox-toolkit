/********************************************************************************
*                                                                               *
*                    F l o a t - V e c t o r   O p e r a t i o n s              *
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
* $Id: FXVec.h,v 1.7.4.1 2004/06/04 04:28:57 fox Exp $                           *
********************************************************************************/
#ifndef FXVEC_H
#define FXVEC_H



/*********************  FXVec Float Vector Class Definition  *******************/

/// Single-precision vector class
class FXAPI FXVec {
protected:
  FXfloat v[3];
public:

  /// Default constructor
  FXVec(){}

  /// Copy constructor
  FXVec(const FXVec& w){v[0]=w.v[0];v[1]=w.v[1];v[2]=w.v[2];}

  /// Initialize with components
  FXVec(FXfloat x,FXfloat y,FXfloat z){v[0]=x;v[1]=y;v[2]=z;}

  /// Initialize with color
  FXVec(FXColor color);

  /// Return a non-const reference to the ith element
  FXfloat& operator[](FXint i){return v[i];}

  /// Return a const reference to the ith element
  const FXfloat& operator[](FXint i) const {return v[i];}

  /// Assign color
  FXVec& operator=(FXColor color);

  /// Assignment
  FXVec& operator=(const FXVec& w){v[0]=w.v[0];v[1]=w.v[1];v[2]=w.v[2];return *this;}

  /// Assigning operators
  FXVec& operator+=(const FXVec& a){v[0]+=a.v[0];v[1]+=a.v[1];v[2]+=a.v[2];return *this;}
  FXVec& operator-=(const FXVec& a){v[0]-=a.v[0];v[1]-=a.v[1];v[2]-=a.v[2];return *this;}
  FXVec& operator*=(FXfloat n){v[0]*=n;v[1]*=n;v[2]*=n;return *this;}
  FXVec& operator/=(FXfloat n){v[0]/=n;v[1]/=n;v[2]/=n;return *this;}

  /// Conversions
  operator FXfloat*(){return v;}
  operator const FXfloat*() const {return v;}

  /// Convert to color
  operator FXColor() const;

  /// Other operators
  friend FXAPI FXVec operator-(const FXVec& a){return FXVec(-a.v[0],-a.v[1],-a.v[2]);}
  friend FXAPI FXVec operator+(const FXVec& a,const FXVec& b){return FXVec(a.v[0]+b.v[0],a.v[1]+b.v[1],a.v[2]+b.v[2]);}
  friend FXAPI FXVec operator-(const FXVec& a,const FXVec& b){return FXVec(a.v[0]-b.v[0],a.v[1]-b.v[1],a.v[2]-b.v[2]);}
  friend FXAPI FXVec operator*(const FXVec& a,FXfloat n){return FXVec(a.v[0]*n,a.v[1]*n,a.v[2]*n);}
  friend FXAPI FXVec operator*(FXfloat n,const FXVec& a){return FXVec(n*a.v[0],n*a.v[1],n*a.v[2]);}
  friend FXAPI FXVec operator/(const FXVec& a,FXfloat n){return FXVec(a.v[0]/n,a.v[1]/n,a.v[2]/n);}
  friend FXAPI FXVec operator/(FXfloat n,const FXVec& a){return FXVec(n/a.v[0],n/a.v[1],n/a.v[2]);}

  /// Dot and cross products
  friend FXAPI FXfloat operator*(const FXVec& a,const FXVec& b){return a.v[0]*b.v[0]+a.v[1]*b.v[1]+a.v[2]*b.v[2];}
  friend FXAPI FXVec operator^(const FXVec& a,const FXVec& b){return FXVec(a.v[1]*b.v[2]-a.v[2]*b.v[1], a.v[2]*b.v[0]-a.v[0]*b.v[2], a.v[0]*b.v[1]-a.v[1]*b.v[0]);}

  /// Test if zero
  friend FXAPI int operator!(const FXVec& a){return a.v[0]==0.0f && a.v[1]==0.0f && a.v[2]==0.0f;}

  /// Equality tests
  friend FXAPI int operator==(const FXVec& a,const FXVec& b){return a.v[0]==b.v[0] && a.v[1]==b.v[1] && a.v[2]==b.v[2];}
  friend FXAPI int operator==(const FXVec& a,FXfloat n){return a.v[0]==n && a.v[1]==n && a.v[2]==n;}
  friend FXAPI int operator==(FXfloat n,const FXVec& a){return n==a.v[0] && n==a.v[1] && n==a.v[2];}
  friend FXAPI int operator!=(const FXVec& a,const FXVec& b){return a.v[0]!=b.v[0] || a.v[1]!=b.v[1] || a.v[2]!=b.v[2];}
  friend FXAPI int operator!=(const FXVec& a,FXfloat n){return a.v[0]!=n || a.v[1]!=n || a.v[2]!=n;}
  friend FXAPI int operator!=(FXfloat n,const FXVec& a){return n!=a.v[0] || n!=a.v[1] || n!=a.v[2];}

  /// Other functions
  friend FXAPI FXfloat len(const FXVec& a);
  friend FXAPI FXVec normalize(const FXVec& a);
  friend FXAPI FXVec lo(const FXVec& a,const FXVec& b);
  friend FXAPI FXVec hi(const FXVec& a,const FXVec& b);

  /// Save vector to a stream
  friend FXAPI FXStream& operator<<(FXStream& store,const FXVec& v);

  /// Load vector from a stream
  friend FXAPI FXStream& operator>>(FXStream& store,FXVec& v);
  };



#endif
