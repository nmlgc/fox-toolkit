/********************************************************************************
*                                                                               *
*     H o m o g e n e o u s   F l o a t - V e c t o r   O p e r a t i o n s     *
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
* $Id: FXHVec.h,v 1.8 2002/01/18 22:42:53 jeroen Exp $                          *
********************************************************************************/
#ifndef FXHVEC_H
#define FXHVEC_H



/*********************  FXHVec Float Vector Class Definition  ******************/


/// Homogeneous vector (single-precision version)
class FXAPI FXHVec {
protected:
  FXfloat v[4];
public:

  /// Default constructor
  FXHVec(){}

  /// Copy constructor
  FXHVec(const FXHVec& w){v[0]=w.v[0];v[1]=w.v[1];v[2]=w.v[2];v[3]=w.v[3];}

  /// Initialize with 3-vector
  FXHVec(const FXVec& w){v[0]=w[0];v[1]=w[1];v[2]=w[2];v[3]=1.0f;}

  /// Initialize with components
  FXHVec(FXfloat x,FXfloat y,FXfloat z,FXfloat w=1.0f){v[0]=x;v[1]=y;v[2]=z;v[3]=w;}

  /// Initialize with color
  FXHVec(FXColor color);

  /// Return a non-const reference to the ith element
  FXfloat& operator[](FXint i){return v[i];}

  /// Return a const reference to the ith element
  const FXfloat& operator[](FXint i) const {return v[i];}

  /// Assign color
  FXHVec& operator=(FXColor color);

  /// Assignment
  FXHVec& operator=(const FXHVec& w){v[0]=w.v[0];v[1]=w.v[1];v[2]=w.v[2];v[3]=w.v[3];return *this;}
  FXHVec& operator=(const FXVec& w){v[0]=w[0];v[1]=w[1];v[2]=w[2];v[3]=1.0f;return *this;}

  /// Assigning operators
  FXHVec& operator+=(const FXHVec& a){v[0]+=a.v[0];v[1]+=a.v[1];v[2]+=a.v[2];v[3]+=a.v[3];return *this;}
  FXHVec& operator-=(const FXHVec& a){v[0]-=a.v[0];v[1]-=a.v[1];v[2]-=a.v[2];v[3]-=a.v[3];return *this;}
  FXHVec& operator*=(FXfloat n){v[0]*=n;v[1]*=n;v[2]*=n;v[3]*=n;return *this;}
  FXHVec& operator/=(FXfloat n){v[0]/=n;v[1]/=n;v[2]/=n;v[3]/=n;return *this;}

  /// Conversion to float array
  operator FXfloat*(){return v;}
  operator const FXfloat*() const {return v;}

  /// Convert to color
  operator FXColor() const;

  /// Other operators
  friend FXHVec operator-(const FXHVec& a){return FXHVec(-a.v[0],-a.v[1],-a.v[2],-a.v[3]);}
  friend FXHVec operator!(const FXHVec& a){return a.v[0]==0.0f && a.v[1]==0.0f && a.v[2]==0.0f && a.v[3]==0.0f;}
  friend FXHVec operator+(const FXHVec& a,const FXHVec& b){return FXHVec(a.v[0]+b.v[0],a.v[1]+b.v[1],a.v[2]+b.v[2],a.v[3]+b.v[3]);}
  friend FXHVec operator-(const FXHVec& a,const FXHVec& b){return FXHVec(a.v[0]-b.v[0],a.v[1]-b.v[1],a.v[2]-b.v[2],a.v[3]-b.v[3]);}
  friend FXHVec operator*(const FXHVec& a,FXfloat n){return FXHVec(a.v[0]*n,a.v[1]*n,a.v[2]*n,a.v[3]*n);}
  friend FXHVec operator*(FXfloat n,const FXHVec& a){return FXHVec(n*a.v[0],n*a.v[1],n*a.v[2],n*a.v[3]);}
  friend FXHVec operator/(const FXHVec& a,FXfloat n){return FXHVec(a.v[0]/n,a.v[1]/n,a.v[2]/n,a.v[3]/n);}
  friend FXHVec operator/(FXfloat n,const FXHVec& a){return FXHVec(n/a.v[0],n/a.v[1],n/a.v[2],n/a.v[3]);}

  /// Dot product
  friend FXfloat operator*(const FXHVec& a,const FXHVec& b){return a.v[0]*b.v[0]+a.v[1]*b.v[1]+a.v[2]*b.v[2]+a.v[3]*b.v[3];}

  /// Equality tests
  friend int operator==(const FXHVec& a,const FXHVec& b){return a.v[0]==b.v[0] && a.v[1]==b.v[1] && a.v[2]==b.v[2] && a.v[3]==b.v[3];}
  friend int operator==(const FXHVec& a,FXfloat n){return a.v[0]==n && a.v[1]==n && a.v[2]==n && a.v[3]==n;}
  friend int operator==(FXfloat n,const FXHVec& a){return n==a.v[0] && n==a.v[1] && n==a.v[2] && n==a.v[3];}
  friend int operator!=(const FXHVec& a,const FXHVec& b){return a.v[0]!=b.v[0] || a.v[1]!=b.v[1] || a.v[2]!=b.v[2] || a.v[3]!=b.v[3];}
  friend int operator!=(const FXHVec& a,FXfloat n){return a.v[0]!=n || a.v[1]!=n || a.v[2]!=n || a.v[3]!=n;}
  friend int operator!=(FXfloat n,const FXHVec& a){return n!=a.v[0] || n!=a.v[1] || n!=a.v[2] || n!=a.v[3];}

  /// Other functions
  friend FXAPI FXfloat len(const FXHVec& a);
  friend FXAPI FXHVec normalize(const FXHVec& a);
  friend FXAPI FXHVec lo(const FXHVec& a,const FXHVec& b);
  friend FXAPI FXHVec hi(const FXHVec& a,const FXHVec& b);

  /// Save to a stream
  friend FXAPI FXStream& operator<<(FXStream& store,const FXHVec& v);

  /// Load from a stream
  friend FXAPI FXStream& operator>>(FXStream& store,FXHVec& v);
  };



#endif
