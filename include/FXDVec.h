/********************************************************************************
*                                                                               *
*                 D o u b l e - V e c t o r   O p e r a t i o n s               *
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
* $Id: FXDVec.h,v 1.7.4.1 2004/06/04 04:37:04 fox Exp $                          *
********************************************************************************/
#ifndef FXDVEC_H
#define FXDVEC_H



/// Double-precision vector class
class FXAPI FXDVec {
protected:
  FXdouble v[3];
public:

  /// Default constructor
  FXDVec(){}

  /// Copy constructor
  FXDVec(const FXDVec& w){v[0]=w.v[0];v[1]=w.v[1];v[2]=w.v[2];}

  /// Initialize with components
  FXDVec(FXdouble x,FXdouble y,FXdouble z){v[0]=x;v[1]=y;v[2]=z;}

  /// Initialize with color
  FXDVec(FXColor color);

  /// Return a non-const reference to the ith element
  FXdouble& operator[](FXint i){return v[i];}

  /// Return a const reference to the ith element
  const FXdouble& operator[](FXint i) const {return v[i];}

  /// Assign color
  FXDVec& operator=(FXColor color);

  /// Assignment
  FXDVec& operator=(const FXDVec& w){v[0]=w.v[0];v[1]=w.v[1];v[2]=w.v[2];return *this;}

  /// Assigning operators
  FXDVec& operator+=(const FXDVec& a){v[0]+=a.v[0];v[1]+=a.v[1];v[2]+=a.v[2];return *this;}
  FXDVec& operator-=(const FXDVec& a){v[0]-=a.v[0];v[1]-=a.v[1];v[2]-=a.v[2];return *this;}
  FXDVec& operator*=(FXdouble n){v[0]*=n;v[1]*=n;v[2]*=n;return *this;}
  FXDVec& operator/=(FXdouble n){v[0]/=n;v[1]/=n;v[2]/=n;return *this;}

  /// Conversions
  operator FXdouble*(){return v;}
  operator const FXdouble*() const {return v;}

  /// Convert to color
  operator FXColor() const;

  /// Other operators
  friend FXDVec operator-(const FXDVec& a){return FXDVec(-a.v[0],-a.v[1],-a.v[2]);}
  friend FXDVec operator+(const FXDVec& a,const FXDVec& b){return FXDVec(a.v[0]+b.v[0],a.v[1]+b.v[1],a.v[2]+b.v[2]);}
  friend FXDVec operator-(const FXDVec& a,const FXDVec& b){return FXDVec(a.v[0]-b.v[0],a.v[1]-b.v[1],a.v[2]-b.v[2]);}
  friend FXDVec operator*(const FXDVec& a,FXdouble n){return FXDVec(a.v[0]*n,a.v[1]*n,a.v[2]*n);}
  friend FXDVec operator*(FXdouble n,const FXDVec& a){return FXDVec(n*a.v[0],n*a.v[1],n*a.v[2]);}
  friend FXDVec operator/(const FXDVec& a,FXdouble n){return FXDVec(a.v[0]/n,a.v[1]/n,a.v[2]/n);}
  friend FXDVec operator/(FXdouble n,const FXDVec& a){return FXDVec(n/a.v[0],n/a.v[1],n/a.v[2]);}

  /// Dot and cross products
  friend FXdouble operator*(const FXDVec& a,const FXDVec& b){return a.v[0]*b.v[0]+a.v[1]*b.v[1]+a.v[2]*b.v[2];}
  friend FXDVec operator^(const FXDVec& a,const FXDVec& b){return FXDVec(a.v[1]*b.v[2]-a.v[2]*b.v[1], a.v[2]*b.v[0]-a.v[0]*b.v[2], a.v[0]*b.v[1]-a.v[1]*b.v[0]);}

  /// Test if zero
  friend int operator!(const FXDVec& a){return a.v[0]==0.0 && a.v[1]==0.0 && a.v[2]==0.0;}

  /// Equality tests
  friend int operator==(const FXDVec& a,const FXDVec& b){return a.v[0]==b.v[0] && a.v[1]==b.v[1] && a.v[2]==b.v[2];}
  friend int operator==(const FXDVec& a,FXdouble n){return a.v[0]==n && a.v[1]==n && a.v[2]==n;}
  friend int operator==(FXdouble n,const FXDVec& a){return n==a.v[0] && n==a.v[1] && n==a.v[2];}
  friend int operator!=(const FXDVec& a,const FXDVec& b){return a.v[0]!=b.v[0] || a.v[1]!=b.v[1] || a.v[2]!=b.v[2];}
  friend int operator!=(const FXDVec& a,FXdouble n){return a.v[0]!=n || a.v[1]!=n || a.v[2]!=n;}
  friend int operator!=(FXdouble n,const FXDVec& a){return n!=a.v[0] || n!=a.v[1] || n!=a.v[2];}

  /// Other functions
  friend FXAPI FXdouble len(const FXDVec& a);
  friend FXAPI FXDVec normalize(const FXDVec& a);
  friend FXAPI FXDVec lo(const FXDVec& a,const FXDVec& b);
  friend FXAPI FXDVec hi(const FXDVec& a,const FXDVec& b);

  /// Save to a stream
  friend FXAPI FXStream& operator<<(FXStream& store,const FXDVec& v);

  /// Load from a stream
  friend FXAPI FXStream& operator>>(FXStream& store,FXDVec& v);
  };



#endif
