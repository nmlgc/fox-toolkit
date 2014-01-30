/********************************************************************************
*                                                                               *
*       D o u b l e - P r e c i s i o n   2 - E l e m e n t   V e c t o r       *
*                                                                               *
*********************************************************************************
* Copyright (C) 1994,2014 by Jeroen van der Zijp.   All Rights Reserved.        *
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
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXArray.h"
#include "FXHash.h"
#include "FXStream.h"
#include "FXObject.h"
#include "FXVec2d.h"
#include "FXVec3d.h"


using namespace FX;

/*******************************************************************************/

namespace FX {


// Normalize vector
FXVec2d normalize(const FXVec2d& v){
  register FXdouble m=v.length2();
  FXVec2d result(v);
  if(__likely(0.0<m)){ result/=sqrt(m); }
  return result;
  }


// Linearly interpolate
FXVec2d lerp(const FXVec2d& u,const FXVec2d& v,FXdouble f){
#if defined(FOX_HAS_SSE2)
  register __m128d u0=_mm_loadu_pd(&u[0]);
  register __m128d v0=_mm_loadu_pd(&v[0]);
  register __m128d ff=_mm_set1_pd(f);
  FXVec2d r;
  _mm_storeu_pd(&r[0],_mm_add_pd(u0,_mm_mul_pd(_mm_sub_pd(v0,u0),ff)));
  return r;
#else
  return FXVec2d(u.x+(v.x-u.x)*f,u.y+(v.y-u.y)*f);
#endif
  }


// Save vector to a stream
FXStream& operator<<(FXStream& store,const FXVec2d& v){
  store << v.x << v.y;
  return store;
  }


// Load vector from a stream
FXStream& operator>>(FXStream& store,FXVec2d& v){
  store >> v.x >> v.y;
  return store;
  }

}
