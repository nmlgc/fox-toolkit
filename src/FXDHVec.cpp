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
* $Id: FXDHVec.cpp,v 1.5 2002/01/18 22:42:59 jeroen Exp $                       *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXObject.h"
#include "FXDVec.h"
#include "FXDHVec.h"


FXDHVec::FXDHVec(FXColor color){
  v[0]=0.003921568627*FXREDVAL(color);
  v[1]=0.003921568627*FXGREENVAL(color);
  v[2]=0.003921568627*FXBLUEVAL(color);
  v[3]=0.003921568627*FXALPHAVAL(color);
  }


FXDHVec& FXDHVec::operator=(FXColor color){
  v[0]=0.003921568627*FXREDVAL(color);
  v[1]=0.003921568627*FXGREENVAL(color);
  v[2]=0.003921568627*FXBLUEVAL(color);
  v[3]=0.003921568627*FXALPHAVAL(color);
  return *this;
  }


FXDHVec::operator FXColor() const {
  return FXRGBA((v[0]*255.0),(v[1]*255.0),(v[2]*255.0),(v[3]*255.0));
  }


FXdouble len(const FXDHVec& a){
  return sqrt(a.v[0]*a.v[0]+a.v[1]*a.v[1]+a.v[2]*a.v[2]+a.v[3]*a.v[3]);
  }


FXDHVec normalize(const FXDHVec& a){
  register FXdouble t=sqrt((FXdouble)(a.v[0]*a.v[0]+a.v[1]*a.v[1]+a.v[2]*a.v[2]));
  if(t>1.0E-40){
    register FXfloat n=(FXfloat)(1.0/t);
    return FXDHVec(n*a.v[0],n*a.v[1],n*a.v[2],n*a.v[3]);
    }
  return FXDHVec(0.0,0.0,0.0,0.0);
  }


FXDHVec lo(const FXDHVec& a,const FXDHVec& b){
  return FXDHVec(FXMIN(a.v[0],b.v[0]),FXMIN(a.v[1],b.v[1]),FXMIN(a.v[2],b.v[2]),FXMIN(a.v[3],b.v[3]));
  }


FXDHVec hi(const FXDHVec& a,const FXDHVec& b){
  return FXDHVec(FXMAX(a.v[0],b.v[0]),FXMAX(a.v[1],b.v[1]),FXMAX(a.v[2],b.v[2]),FXMAX(a.v[3],b.v[3]));
  }


// Saving and loading
FXStream& operator<<(FXStream& store,const FXDHVec& v){
  store << v[0] << v[1] << v[2] << v[3];
  return store;
  }


FXStream& operator>>(FXStream& store,FXDHVec& v){
  store >> v[0] >> v[1] >> v[2] >> v[3];
  return store;
  }
