/********************************************************************************
*                                                                               *
*    H o m o g e n e o u s   D o u b l e - V e c t o r   O p e r a t i o n s    *
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
* $Id: FXDHVec.h,v 1.8 2002/01/18 22:42:51 jeroen Exp $                         *
********************************************************************************/
#ifndef FXDHVEC_H
#define FXDHVEC_H



/// Homogeneous vector (double-precision version)
class FXAPI FXDHVec {
protected:
  FXdouble v[4];
public:

  /// Default constructor
  FXDHVec(){}

  /// Copy constructor
  FXDHVec(const FXDHVec& w){v[0]=w.v[0];v[1]=w.v[1];v[2]=w.v[2];v[3]=w.v[3];}

  /// Initialize with components
  FXDHVec(FXdouble x,FXdouble y,FXdouble z,FXdouble w=1.0){v[0]=x;v[1]=y;v[2]=z;v[3]=w;}

  /// Initialize with 3-vector
  FXDHVec(const FXDVec& w){v[0]=w[0];v[1]=w[1];v[2]=w[2];v[3]=1.0;}


  /// Initialize with color
  FXDHVec(FXColor color);

  /// Return a non-const reference to the ith element
  FXdouble& operator[](FXint i){return v[i];}

  /// Return a const reference to the ith element
  const FXdouble& operator[](FXint i) const {return v[i];}

  /// Assign color
  FXDHVec& operator=(FXColor color);

  /// Assignment
  FXDHVec& operator=(const FXDHVec& w){v[0]=w.v[0];v[1]=w.v[1];v[2]=w.v[2];v[3]=w.v[3];return *this;}
  FXDHVec& operator=(const FXDVec& w){v[0]=w[0];v[1]=w[1];v[2]=w[2];v[3]=1.0F;return *this;}


  /// Assigning operators
  FXDHVec& operator+=(const FXDHVec& a){v[0]+=a.v[0];v[1]+=a.v[1];v[2]+=a.v[2];v[3]+=a.v[3];return *this;}
  FXDHVec& operator-=(const FXDHVec& a){v[0]-=a.v[0];v[1]-=a.v[1];v[2]-=a.v[2];v[3]-=a.v[3];return *this;}
  FXDHVec& operator*=(FXdouble n){v[0]*=n;v[1]*=n;v[2]*=n;v[3]*=n;return *this;}
  FXDHVec& operator/=(FXdouble n){v[0]/=n;v[1]/=n;v[2]/=n;v[3]/=n;return *this;}

  /// Conversions
  operator FXdouble*(){return v;}
  operator const FXdouble*() const {return v;}

  /// Convert to color
  operator FXColor() const;

  /// Other operators
  friend FXDHVec operator-(const FXDHVec& a){return FXDHVec(-a.v[0],-a.v[1],-a.v[2],-a.v[3]);}
  friend FXDHVec operator!(const FXDHVec& a){return a.v[0]==0.0 && a.v[1]==0.0 && a.v[2]==0.0 && a.v[3]==0.0;}
  friend FXDHVec operator+(const FXDHVec& a,const FXDHVec& b){return FXDHVec(a.v[0]+b.v[0],a.v[1]+b.v[1],a.v[2]+b.v[2],a.v[3]+b.v[3]);}
  friend FXDHVec operator-(const FXDHVec& a,const FXDHVec& b){return FXDHVec(a.v[0]-b.v[0],a.v[1]-b.v[1],a.v[2]-b.v[2],a.v[3]-b.v[3]);}
  friend FXDHVec operator*(const FXDHVec& a,FXdouble n){return FXDHVec(a.v[0]*n,a.v[1]*n,a.v[2]*n,a.v[3]*n);}
  friend FXDHVec operator*(FXdouble n,const FXDHVec& a){return FXDHVec(n*a.v[0],n*a.v[1],n*a.v[2],n*a.v[3]);}
  friend FXDHVec operator/(const FXDHVec& a,FXdouble n){return FXDHVec(a.v[0]/n,a.v[1]/n,a.v[2]/n,a.v[3]/n);}
  friend FXDHVec operator/(FXdouble n,const FXDHVec& a){return FXDHVec(n/a.v[0],n/a.v[1],n/a.v[2],n/a.v[3]);}

  /// Dot product
  friend FXdouble operator*(const FXDHVec& a,const FXDHVec& b){return a.v[0]*b.v[0]+a.v[1]*b.v[1]+a.v[2]*b.v[2]+a.v[3]*b.v[3];}

  /// Equality tests
  friend int operator==(const FXDHVec& a,const FXDHVec& b){return a.v[0]==b.v[0] && a.v[1]==b.v[1] && a.v[2]==b.v[2] && a.v[3]==b.v[3];}
  friend int operator==(const FXDHVec& a,FXdouble n){return a.v[0]==n && a.v[1]==n && a.v[2]==n && a.v[3]==n;}
  friend int operator==(FXdouble n,const FXDHVec& a){return n==a.v[0] && n==a.v[1] && n==a.v[2] && n==a.v[3];}
  friend int operator!=(const FXDHVec& a,const FXDHVec& b){return a.v[0]!=b.v[0] || a.v[1]!=b.v[1] || a.v[2]!=b.v[2] || a.v[3]!=b.v[3];}
  friend int operator!=(const FXDHVec& a,FXdouble n){return a.v[0]!=n || a.v[1]!=n || a.v[2]!=n || a.v[3]!=n;}
  friend int operator!=(FXdouble n,const FXDHVec& a){return n!=a.v[0] || n!=a.v[1] || n!=a.v[2] || n!=a.v[3];}

  /// Other functions
  friend FXAPI FXdouble len(const FXDHVec& a);
  friend FXAPI FXDHVec normalize(const FXDHVec& a);
  friend FXAPI FXDHVec lo(const FXDHVec& a,const FXDHVec& b);
  friend FXAPI FXDHVec hi(const FXDHVec& a,const FXDHVec& b);

  /// Save to a stream
  friend FXAPI FXStream& operator<<(FXStream& store,const FXDHVec& v);

  /// Load from a stream
  friend FXAPI FXStream& operator>>(FXStream& store,FXDHVec& v);
  };



#endif
