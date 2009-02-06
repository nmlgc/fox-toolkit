/********************************************************************************
*                                                                               *
*       D o u b l e - P r e c i s i o n   3 - E l e m e n t   V e c t o r       *
*                                                                               *
*********************************************************************************
* Copyright (C) 1994,2009 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This library is free software; you can redistribute it and/or modify          *
* it under the terms of the GNU Lesser General Public License as published by   *
* the Free Software Foundation; either version 3 of the License, or             *
* (at your option) any later version.                                           *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
* GNU Lesser General Public License for more details.                           *
*                                                                               *
* You should have received a copy of the GNU Lesser General Public License      *
* along with this program.  If not, see <http://www.gnu.org/licenses/>          *
*********************************************************************************
* $Id: FXVec3d.cpp,v 1.19 2009/02/04 18:11:55 fox Exp $                         *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXHash.h"
#include "FXStream.h"
#include "FXObject.h"
#include "FXVec2d.h"
#include "FXVec3d.h"
#include "FXVec4d.h"
#include "FXMat3d.h"
#include "FXMat4d.h"


using namespace FX;

/*******************************************************************************/

namespace FX {


// Convert from vector to color
FXColor colorFromVec3d(const FXVec3d& vec){
  return FXRGB((vec.x*255.0+0.5),(vec.y*255.0+0.5),(vec.z*255.0+0.5));
  }


// Convert from color to vector
FXVec3d colorToVec3d(FXColor clr){
  return FXVec3d(0.003921568627*FXREDVAL(clr),0.003921568627*FXGREENVAL(clr),0.003921568627*FXBLUEVAL(clr));
  }
  
  
// Normalize vector
FXVec3d normalize(const FXVec3d& v){
  register FXdouble m=v.length2();
  FXVec3d result(v);
  if(m>0.0){ result/=sqrt(m); }
  return result;
  }


// Compute normal from three points a,b,c
FXVec3d normal(const FXVec3d& a,const FXVec3d& b,const FXVec3d& c){
  return normalize((b-a)^(c-a));
  }


// Compute approximate normal from four points a,b,c,d
FXVec3d normal(const FXVec3d& a,const FXVec3d& b,const FXVec3d& c,const FXVec3d& d){
  return normalize((c-a)^(d-b));
  }


// Vector times matrix
FXVec3d FXVec3d::operator*(const FXMat3d& m) const {
  return FXVec3d(x*m[0][0]+y*m[1][0]+z*m[2][0], x*m[0][1]+y*m[1][1]+z*m[2][1], x*m[0][2]+y*m[1][2]+z*m[2][2]);
  }


// Vector times matrix
FXVec3d FXVec3d::operator*(const FXMat4d& m) const {
  FXASSERT(m[0][3]==0.0 && m[1][3]==0.0 && m[2][3]==0.0 && m[3][3]==1.0);
  return FXVec3d(x*m[0][0]+y*m[1][0]+z*m[2][0]+m[3][0], x*m[0][1]+y*m[1][1]+z*m[2][1]+m[3][1], x*m[0][2]+y*m[1][2]+z*m[2][2]+m[3][2]);
  }


// Save vector to stream
FXStream& operator<<(FXStream& store,const FXVec3d& v){
  store << v.x << v.y << v.z;
  return store;
  }


// Load vector from stream
FXStream& operator>>(FXStream& store,FXVec3d& v){
  store >> v.x >> v.y >> v.z;
  return store;
  }

}
