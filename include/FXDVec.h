/********************************************************************************
*                                                                               *
*                 D o u b l e - V e c t o r   O p e r a t i o n s               *
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
* $Id: FXDVec.h,v 1.2 1998/07/06 22:44:38 jeroen Exp $                          *
********************************************************************************/
#ifndef FXDVEC_H
#define FXDVEC_H



/********************  FXDVec Double Vector Class Definition  ******************/

class FXDVec {
protected:
  FXdouble v[3];
public:
  
  // Constructors
  FXDVec(){}
  FXDVec(const FXdouble n){v[0]=v[1]=v[2]=n;}
  FXDVec(const FXdouble x,const FXdouble y,const FXdouble z){v[0]=x;v[1]=y;v[2]=z;}

  // Indexing
  FXdouble& operator[](const int i){return v[i];}
  const FXdouble& operator[](const int i) const {return v[i];}

  // Assignment operators
  FXDVec& operator=(const FXdouble n){v[0]=v[1]=v[2]=n;return *this;}

  FXDVec& operator+=(const FXDVec& a){v[0]+=a.v[0];v[1]+=a.v[1];v[2]+=a.v[2];return *this;}
  FXDVec& operator-=(const FXDVec& a){v[0]-=a.v[0];v[1]-=a.v[1];v[2]-=a.v[2];return *this;}
  FXDVec& operator*=(const FXdouble n){v[0]*=n;v[1]*=n;v[2]*=n;return *this;}
  FXDVec& operator/=(const FXdouble n){v[0]/=n;v[1]/=n;v[2]/=n;return *this;}

  // Conversions
  operator FXdouble*(){return v;}
  operator const FXdouble*() const {return v;}
  operator FXColor() const;

  // Other operators
  friend FXDVec operator-(const FXDVec& a){return FXDVec(-a.v[0],-a.v[1],-a.v[2]);}
  friend FXDVec operator!(const FXDVec& a){return a.v[0]==0.0 && a.v[1]==0.0 && a.v[2]==0.0;}
  friend FXDVec operator+(const FXDVec& a,const FXDVec& b){return FXDVec(a.v[0]+b.v[0],a.v[1]+b.v[1],a.v[2]+b.v[2]);}
  friend FXDVec operator-(const FXDVec& a,const FXDVec& b){return FXDVec(a.v[0]-b.v[0],a.v[1]-b.v[1],a.v[2]-b.v[2]);}
  friend FXDVec operator*(const FXDVec& a,const FXdouble n){return FXDVec(a.v[0]*n,a.v[1]*n,a.v[2]*n);}
  friend FXDVec operator*(const FXdouble n,const FXDVec& a){return FXDVec(n*a.v[0],n*a.v[1],n*a.v[2]);}
  friend FXDVec operator/(const FXDVec& a,const FXdouble n){return FXDVec(a.v[0]/n,a.v[1]/n,a.v[2]/n);}
  friend FXDVec operator/(const FXdouble n,const FXDVec& a){return FXDVec(n/a.v[0],n/a.v[1],n/a.v[2]);}

  // Dot and cross products
  friend FXdouble operator*(const FXDVec& a,const FXDVec& b){return a.v[0]*b.v[0]+a.v[1]*b.v[1]+a.v[2]*b.v[2];}
  friend FXDVec operator^(const FXDVec& a,const FXDVec& b){return FXDVec(a.v[1]*b.v[2]-a.v[2]*b.v[1], a.v[2]*b.v[0]-a.v[0]*b.v[2], a.v[0]*b.v[1]-a.v[1]*b.v[0]);}

  // Equality tests
  friend int operator==(const FXDVec& a,const FXDVec& b){return a.v[0]==b.v[0] && a.v[1]==b.v[1] && a.v[2]==b.v[2];}
  friend int operator==(const FXDVec& a,const FXdouble n){return a.v[0]==n && a.v[1]==n && a.v[2]==n;}
  friend int operator==(const FXdouble n,const FXDVec& a){return n==a.v[0] && n==a.v[1] && n==a.v[2];}
  friend int operator!=(const FXDVec& a,const FXDVec& b){return a.v[0]!=b.v[0] || a.v[1]!=b.v[1] || a.v[2]!=b.v[2];}
  friend int operator!=(const FXDVec& a,const FXdouble n){return a.v[0]!=n || a.v[1]!=n || a.v[2]!=n;}
  friend int operator!=(const FXdouble n,const FXDVec& a){return n!=a.v[0] || n!=a.v[1] || n!=a.v[2];}

  // Other functions
  friend FXdouble len(const FXDVec& a);
  friend FXDVec normalize(const FXDVec& a);
  friend FXDVec lo(const FXDVec& a,const FXDVec& b);
  friend FXDVec hi(const FXDVec& a,const FXDVec& b);

  // Saving and loading
  friend FXStream& operator<<(FXStream& store,const FXDVec& v);
  friend FXStream& operator>>(FXStream& store,FXDVec& v);
  };



#endif
