/********************************************************************************
*                                                                               *
*    H o m o g e n e o u s   D o u b l e - V e c t o r   O p e r a t i o n s    *
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
* $Id: FXDHVec.h,v 1.2 1998/07/06 22:44:38 jeroen Exp $                         *
********************************************************************************/
#ifndef FXDHVEC_H
#define FXDHVEC_H



/***********************  FXDHVec Vector Class Definition  **********************/

class FXDHVec {
protected:
  FXdouble v[4];
public:
  
  // Constructors
  FXDHVec(){}
  FXDHVec(const FXdouble n){v[0]=v[1]=v[2]=v[3]=n;}
  FXDHVec(const FXdouble x,const FXdouble y,const FXdouble z,const FXdouble w=1.0F){v[0]=x;v[1]=y;v[2]=z;v[3]=w;}

  // Indexing
  FXdouble& operator[](const int i){return v[i];}
  const FXdouble& operator[](const int i) const {return v[i];}

  // Assignment operators
  FXDHVec& operator=(const FXdouble n){v[0]=v[1]=v[2]=v[3]=n; return *this;}

  FXDHVec& operator+=(const FXDHVec& a){v[0]+=a.v[0];v[1]+=a.v[1];v[2]+=a.v[2];v[3]+=a.v[3];return *this;}
  FXDHVec& operator-=(const FXDHVec& a){v[0]-=a.v[0];v[1]-=a.v[1];v[2]-=a.v[2];v[3]-=a.v[3];return *this;}
  FXDHVec& operator*=(const FXdouble n){v[0]*=n;v[1]*=n;v[2]*=n;v[3]*=n;return *this;}
  FXDHVec& operator/=(const FXdouble n){v[0]/=n;v[1]/=n;v[2]/=n;v[3]/=n;return *this;}

  // Conversions
  operator FXdouble*(){return v;}
  operator const FXdouble*() const {return v;}
  operator FXColor() const;

  // Other operators
  friend FXDHVec operator-(const FXDHVec& a){return FXDHVec(-a.v[0],-a.v[1],-a.v[2],-a.v[3]);}
  friend FXDHVec operator!(const FXDHVec& a){return a.v[0]==0.0 && a.v[1]==0.0 && a.v[2]==0.0 && a.v[3]==0.0;}
  friend FXDHVec operator+(const FXDHVec& a,const FXDHVec& b){return FXDHVec(a.v[0]+b.v[0],a.v[1]+b.v[1],a.v[2]+b.v[2],a.v[3]+b.v[3]);}
  friend FXDHVec operator-(const FXDHVec& a,const FXDHVec& b){return FXDHVec(a.v[0]-b.v[0],a.v[1]-b.v[1],a.v[2]-b.v[2],a.v[3]-b.v[3]);}
  friend FXDHVec operator*(const FXDHVec& a,const FXdouble n){return FXDHVec(a.v[0]*n,a.v[1]*n,a.v[2]*n,a.v[3]*n);}
  friend FXDHVec operator*(const FXdouble n,const FXDHVec& a){return FXDHVec(n*a.v[0],n*a.v[1],n*a.v[2],n*a.v[3]);}
  friend FXDHVec operator/(const FXDHVec& a,const FXdouble n){return FXDHVec(a.v[0]/n,a.v[1]/n,a.v[2]/n,a.v[3]/n);}
  friend FXDHVec operator/(const FXdouble n,const FXDHVec& a){return FXDHVec(n/a.v[0],n/a.v[1],n/a.v[2],n/a.v[3]);}

  // Dot product
  friend FXdouble operator*(const FXDHVec& a,const FXDHVec& b){return a.v[0]*b.v[0]+a.v[1]*b.v[1]+a.v[2]*b.v[2]+a.v[3]*b.v[3];}

  // Equality tests
  friend int operator==(const FXDHVec& a,const FXDHVec& b){return a.v[0]==b.v[0] && a.v[1]==b.v[1] && a.v[2]==b.v[2] && a.v[3]==b.v[3];}
  friend int operator==(const FXDHVec& a,const FXdouble n){return a.v[0]==n && a.v[1]==n && a.v[2]==n && a.v[3]==n;}
  friend int operator==(const FXdouble n,const FXDHVec& a){return n==a.v[0] && n==a.v[1] && n==a.v[2] && n==a.v[3];}
  friend int operator!=(const FXDHVec& a,const FXDHVec& b){return a.v[0]!=b.v[0] || a.v[1]!=b.v[1] || a.v[2]!=b.v[2] || a.v[3]!=b.v[3];}
  friend int operator!=(const FXDHVec& a,const FXdouble n){return a.v[0]!=n || a.v[1]!=n || a.v[2]!=n || a.v[3]!=n;}
  friend int operator!=(const FXdouble n,const FXDHVec& a){return n!=a.v[0] || n!=a.v[1] || n!=a.v[2] || n!=a.v[3];}

  // Other functions
  friend FXdouble len(const FXDHVec& a);
  friend FXDHVec normalize(const FXDHVec& a);
  friend FXDHVec lo(const FXDHVec& a,const FXDHVec& b);
  friend FXDHVec hi(const FXDHVec& a,const FXDHVec& b);

  // Saving and loading
  friend FXStream& operator<<(FXStream& store,const FXDHVec& v);
  friend FXStream& operator>>(FXStream& store,FXDHVec& v);
  };



#endif
