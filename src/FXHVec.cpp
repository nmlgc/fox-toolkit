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
* $Id: FXHVec.cpp,v 1.3 1998/07/06 22:44:46 jeroen Exp $                         *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXObject.h"
#include "FXHVec.h"

// FXHVec::FXHVec(const FXColor clr){
//   v[0]=0.0039215686*FXREDVAL(rgba);
//   v[1]=0.0039215686*FXGREENVAL(rgba);
//   v[2]=0.0039215686*FXBLUEVAL(rgba);
//   v[3]=0.0039215686*FXALPHAVAL(rgba);
//   }


FXHVec::operator FXColor() const {
  return FXRGBA((v[0]*255.0),(v[1]*255.0),(v[2]*255.0),(v[3]*255.0));
  }


FXfloat len(const FXHVec& a){
  return (FXfloat)sqrt((double)(a.v[0]*a.v[0]+a.v[1]*a.v[1]+a.v[2]*a.v[2]+a.v[3]*a.v[3]));
  }


FXHVec normalize(const FXHVec& a){
  register FXfloat n=(FXfloat)(1.0/sqrt((double)(a.v[0]*a.v[0]+a.v[1]*a.v[1]+a.v[2]*a.v[2]+a.v[3]*a.v[3])));
  return FXHVec(n*a.v[0],n*a.v[1],n*a.v[2],n*a.v[3]);
  }


FXHVec lo(const FXHVec& a,const FXHVec& b){
  return FXHVec(FXMIN(a.v[0],b.v[0]),FXMIN(a.v[1],b.v[1]),FXMIN(a.v[2],b.v[2]),FXMIN(a.v[3],b.v[3]));
  }


FXHVec hi(const FXHVec& a,const FXHVec& b){
  return FXHVec(FXMAX(a.v[0],b.v[0]),FXMAX(a.v[1],b.v[1]),FXMAX(a.v[2],b.v[2]),FXMAX(a.v[3],b.v[3]));
  }


FXStream& operator<<(FXStream& store,const FXHVec& v){
  store << v[0] << v[1] << v[2] << v[3];
  return store;
  }

FXStream& operator>>(FXStream& store,FXHVec& v){
  store >> v[0] >> v[1] >> v[2] >> v[3];
  return store;
  }
