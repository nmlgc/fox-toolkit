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
* $Id: FXVec.cpp,v 1.5 2002/01/18 22:43:07 jeroen Exp $                         *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXObject.h"
#include "FXVec.h"


FXVec::FXVec(FXColor color){
  v[0]=0.003921568627f*FXREDVAL(color);
  v[1]=0.003921568627f*FXGREENVAL(color);
  v[2]=0.003921568627f*FXBLUEVAL(color);
  }


FXVec& FXVec::operator=(FXColor color){
  v[0]=0.003921568627f*FXREDVAL(color);
  v[1]=0.003921568627f*FXGREENVAL(color);
  v[2]=0.003921568627f*FXBLUEVAL(color);
  return *this;
  }


FXVec::operator FXColor() const {
  return FXRGB((v[0]*255.0f),(v[1]*255.0f),(v[2]*255.0f));
  }


FXfloat len(const FXVec& a){
  return (FXfloat)sqrt((FXdouble)(a.v[0]*a.v[0]+a.v[1]*a.v[1]+a.v[2]*a.v[2]));
  }


FXVec normalize(const FXVec& a){
  register FXdouble t=sqrt((FXdouble)(a.v[0]*a.v[0]+a.v[1]*a.v[1]+a.v[2]*a.v[2]));
  if(t>1.0E-40){
    register FXfloat n=(FXfloat)(1.0/t);
    return FXVec(n*a.v[0],n*a.v[1],n*a.v[2]);
    }
  return FXVec(0.0f,0.0f,0.0f);
  }


FXVec lo(const FXVec& a,const FXVec& b){
  return FXVec(FXMIN(a.v[0],b.v[0]),FXMIN(a.v[1],b.v[1]),FXMIN(a.v[2],b.v[2]));
  }


FXVec hi(const FXVec& a,const FXVec& b){
  return FXVec(FXMAX(a.v[0],b.v[0]),FXMAX(a.v[1],b.v[1]),FXMAX(a.v[2],b.v[2]));
  }


FXStream& operator<<(FXStream& store,const FXVec& v){
  store << v[0] << v[1] << v[2];
  return store;
  }


FXStream& operator>>(FXStream& store,FXVec& v){
  store >> v[0] >> v[1] >> v[2];
  return store;
  }


