/********************************************************************************
*                                                                               *
*                    F l o a t - V e c t o r   O p e r a t i o n s              *
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
* $Id: FXVec.h,v 1.3 1998/07/06 22:44:40 jeroen Exp $                           *
********************************************************************************/
#ifndef FXVEC_H
#define FXVEC_H



/*********************  FXVec Float Vector Class Definition  *******************/

class FXVec {
protected:
  FXfloat v[3];
public:
  
  // Constructors
  FXVec(){}
  FXVec(const FXfloat n){v[0]=v[1]=v[2]=n;}
  FXVec(const FXfloat x,const FXfloat y,const FXfloat z){v[0]=x;v[1]=y;v[2]=z;}

  // Indexing
  FXfloat& operator[](const int i){return v[i];}
  const FXfloat& operator[](const int i) const {return v[i];}

  // Assignment operators
  FXVec& operator=(const FXfloat n){v[0]=v[1]=v[2]=n;return *this;}

  FXVec& operator+=(const FXVec& a){v[0]+=a.v[0];v[1]+=a.v[1];v[2]+=a.v[2];return *this;}
  FXVec& operator-=(const FXVec& a){v[0]-=a.v[0];v[1]-=a.v[1];v[2]-=a.v[2];return *this;}
  FXVec& operator*=(const FXfloat n){v[0]*=n;v[1]*=n;v[2]*=n;return *this;}
  FXVec& operator/=(const FXfloat n){v[0]/=n;v[1]/=n;v[2]/=n;return *this;}

  // Conversions
  operator FXfloat*(){return v;}
  operator const FXfloat*() const {return v;}
  operator FXColor() const;

  // Other operators
  friend FXVec operator-(const FXVec& a){return FXVec(-a.v[0],-a.v[1],-a.v[2]);}
  friend FXVec operator!(const FXVec& a){return a.v[0]==0.0 && a.v[1]==0.0 && a.v[2]==0.0;}
  friend FXVec operator+(const FXVec& a,const FXVec& b){return FXVec(a.v[0]+b.v[0],a.v[1]+b.v[1],a.v[2]+b.v[2]);}
  friend FXVec operator-(const FXVec& a,const FXVec& b){return FXVec(a.v[0]-b.v[0],a.v[1]-b.v[1],a.v[2]-b.v[2]);}
  friend FXVec operator*(const FXVec& a,const FXfloat n){return FXVec(a.v[0]*n,a.v[1]*n,a.v[2]*n);}
  friend FXVec operator*(const FXfloat n,const FXVec& a){return FXVec(n*a.v[0],n*a.v[1],n*a.v[2]);}
  friend FXVec operator/(const FXVec& a,const FXfloat n){return FXVec(a.v[0]/n,a.v[1]/n,a.v[2]/n);}
  friend FXVec operator/(const FXfloat n,const FXVec& a){return FXVec(n/a.v[0],n/a.v[1],n/a.v[2]);}

  // Dot and cross products
  friend FXfloat operator*(const FXVec& a,const FXVec& b){return a.v[0]*b.v[0]+a.v[1]*b.v[1]+a.v[2]*b.v[2];}
  friend FXVec operator^(const FXVec& a,const FXVec& b){return FXVec(a.v[1]*b.v[2]-a.v[2]*b.v[1], a.v[2]*b.v[0]-a.v[0]*b.v[2], a.v[0]*b.v[1]-a.v[1]*b.v[0]);}

  // Equality tests
  friend int operator==(const FXVec& a,const FXVec& b){return a.v[0]==b.v[0] && a.v[1]==b.v[1] && a.v[2]==b.v[2];}
  friend int operator==(const FXVec& a,const FXfloat n){return a.v[0]==n && a.v[1]==n && a.v[2]==n;}
  friend int operator==(const FXfloat n,const FXVec& a){return n==a.v[0] && n==a.v[1] && n==a.v[2];}
  friend int operator!=(const FXVec& a,const FXVec& b){return a.v[0]!=b.v[0] || a.v[1]!=b.v[1] || a.v[2]!=b.v[2];}
  friend int operator!=(const FXVec& a,const FXfloat n){return a.v[0]!=n || a.v[1]!=n || a.v[2]!=n;}
  friend int operator!=(const FXfloat n,const FXVec& a){return n!=a.v[0] || n!=a.v[1] || n!=a.v[2];}

  // Other functions
  friend FXfloat len(const FXVec& a);
  friend FXVec normalize(const FXVec& a);
  friend FXVec lo(const FXVec& a,const FXVec& b);
  friend FXVec hi(const FXVec& a,const FXVec& b);

  // Saving and loading
  friend FXStream& operator<<(FXStream& store,const FXVec& v);
  friend FXStream& operator>>(FXStream& store,FXVec& v);
  };



#endif
