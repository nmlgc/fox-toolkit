/********************************************************************************
*                                                                               *
*     H o m o g e n e o u s   F l o a t - V e c t o r   O p e r a t i o n s     *
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
* $Id: FXHVec.h,v 1.2 1998/07/06 22:44:39 jeroen Exp $                          *
********************************************************************************/
#ifndef FXHVEC_H
#define FXHVEC_H



/*********************  FXHVec Float Vector Class Definition  ******************/

class FXHVec {
protected:
  FXfloat v[4];
public:
  
  // Constructors
  FXHVec(){}
  FXHVec(const FXfloat n){v[0]=v[1]=v[2]=v[3]=n;}
//  FXHVec(const FXColor clr);
  FXHVec(const FXfloat x,const FXfloat y,const FXfloat z,const FXfloat w=1.0F){v[0]=x;v[1]=y;v[2]=z;v[3]=w;}

  // Indexing
  FXfloat& operator[](const int i){return v[i];}
  const FXfloat& operator[](const int i) const {return v[i];}

  // Assignment operators
  FXHVec& operator=(const FXfloat n){v[0]=v[1]=v[2]=v[3]=n; return *this;}

  FXHVec& operator+=(const FXHVec& a){v[0]+=a.v[0];v[1]+=a.v[1];v[2]+=a.v[2];v[3]+=a.v[3];return *this;}
  FXHVec& operator-=(const FXHVec& a){v[0]-=a.v[0];v[1]-=a.v[1];v[2]-=a.v[2];v[3]-=a.v[3];return *this;}
  FXHVec& operator*=(const FXfloat n){v[0]*=n;v[1]*=n;v[2]*=n;v[3]*=n;return *this;}
  FXHVec& operator/=(const FXfloat n){v[0]/=n;v[1]/=n;v[2]/=n;v[3]/=n;return *this;}

  // Conversions
  operator FXfloat*(){return v;}
  operator const FXfloat*() const {return v;}
  operator FXColor() const;

  // Other operators
  friend FXHVec operator-(const FXHVec& a){return FXHVec(-a.v[0],-a.v[1],-a.v[2],-a.v[3]);}
  friend FXHVec operator!(const FXHVec& a){return a.v[0]==0.0 && a.v[1]==0.0 && a.v[2]==0.0 && a.v[3]==0.0;}
  friend FXHVec operator+(const FXHVec& a,const FXHVec& b){return FXHVec(a.v[0]+b.v[0],a.v[1]+b.v[1],a.v[2]+b.v[2],a.v[3]+b.v[3]);}
  friend FXHVec operator-(const FXHVec& a,const FXHVec& b){return FXHVec(a.v[0]-b.v[0],a.v[1]-b.v[1],a.v[2]-b.v[2],a.v[3]-b.v[3]);}
  friend FXHVec operator*(const FXHVec& a,const FXfloat n){return FXHVec(a.v[0]*n,a.v[1]*n,a.v[2]*n,a.v[3]*n);}
  friend FXHVec operator*(const FXfloat n,const FXHVec& a){return FXHVec(n*a.v[0],n*a.v[1],n*a.v[2],n*a.v[3]);}
  friend FXHVec operator/(const FXHVec& a,const FXfloat n){return FXHVec(a.v[0]/n,a.v[1]/n,a.v[2]/n,a.v[3]/n);}
  friend FXHVec operator/(const FXfloat n,const FXHVec& a){return FXHVec(n/a.v[0],n/a.v[1],n/a.v[2],n/a.v[3]);}

  // Dot product
  friend FXfloat operator*(const FXHVec& a,const FXHVec& b){return a.v[0]*b.v[0]+a.v[1]*b.v[1]+a.v[2]*b.v[2]+a.v[3]*b.v[3];}

  // Equality tests
  friend int operator==(const FXHVec& a,const FXHVec& b){return a.v[0]==b.v[0] && a.v[1]==b.v[1] && a.v[2]==b.v[2] && a.v[3]==b.v[3];}
  friend int operator==(const FXHVec& a,const FXfloat n){return a.v[0]==n && a.v[1]==n && a.v[2]==n && a.v[3]==n;}
  friend int operator==(const FXfloat n,const FXHVec& a){return n==a.v[0] && n==a.v[1] && n==a.v[2] && n==a.v[3];}
  friend int operator!=(const FXHVec& a,const FXHVec& b){return a.v[0]!=b.v[0] || a.v[1]!=b.v[1] || a.v[2]!=b.v[2] || a.v[3]!=b.v[3];}
  friend int operator!=(const FXHVec& a,const FXfloat n){return a.v[0]!=n || a.v[1]!=n || a.v[2]!=n || a.v[3]!=n;}
  friend int operator!=(const FXfloat n,const FXHVec& a){return n!=a.v[0] || n!=a.v[1] || n!=a.v[2] || n!=a.v[3];}

  // Other functions
  friend FXfloat len(const FXHVec& a);
  friend FXHVec normalize(const FXHVec& a);
  friend FXHVec lo(const FXHVec& a,const FXHVec& b);
  friend FXHVec hi(const FXHVec& a,const FXHVec& b);

  // Saving and loading
  friend FXStream& operator<<(FXStream& store,const FXHVec& v);
  friend FXStream& operator>>(FXStream& store,FXHVec& v);
  };



#endif
