/********************************************************************************
*                                                                               *
*                    Q u a t e r n i o n   F u n c t i o n s                    *
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
* $Id: FXQuat.cpp,v 1.3 1998/09/02 15:13:46 jvz Exp $                         *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXObject.h"
#include "FXVec.h"
#include "FXHVec.h"
#include "FXQuat.h"

/* Return the value val with the sign of s */
#define SIGN(val,s) (((s)<0)?-(val):(val))


/*******************************************************************************/


FXQuat::FXQuat(const FXVec& axis,const FXfloat phi){
  register double a=0.5*phi;
  register double s=sin(a)/len(axis);
  v[0]=axis[0]*s; v[1]=axis[1]*s; v[2]=axis[2]*s; v[3]=(FXdouble)cos(a);
  }


FXQuat& FXQuat::adjust(){
  register double abig,big=v[0];
  register int w=0;
  if(fabs(v[1])>fabs(big)){big=v[1];w=1;}
  if(fabs(v[2])>fabs(big)){big=v[2];w=2;}
  if(fabs(v[3])>fabs(big)){big=v[3];w=3;}
  v[w]=0.0;
  abig=sqrt(1.0-(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]+v[3]*v[3]));
  v[w]=SIGN(abig,big);
  return *this;
  }


FXQuat invert(const FXQuat& q){
  FXfloat n=q[0]*q[0]+q[1]*q[1]+q[2]*q[2]+q[3]*q[3];
  FXASSERT(n>0.0);
  return FXQuat(-q[0]/n,-q[1]/n,-q[2]/n,-q[3]/n);
  }


FXQuat conj(const FXQuat& q){
  return FXQuat(-q[0],-q[1],-q[2],q[3]);
  }


FXQuat operator*(const FXQuat& p,const FXQuat& q){
  return FXQuat(p[3]*q[0]+p[0]*q[3]+p[1]*q[2]-p[2]*q[1],
                p[3]*q[1]+p[1]*q[3]+p[2]*q[0]-p[0]*q[2],
                p[3]*q[2]+p[2]*q[3]+p[0]*q[1]-p[1]*q[0],
                p[3]*q[3]-p[0]*q[0]-p[1]*q[1]-p[2]*q[2]);
  }


FXQuat arc(const FXVec& f,const FXVec& t){
  FXQuat q;
  q.v[0] = f[1]*t[2]-f[2]*t[1];
  q.v[1] = f[2]*t[0]-f[0]*t[2];
  q.v[2] = f[0]*t[1]-f[1]*t[0];
  q.v[3] = f[0]*t[0]+f[1]*t[1]+f[2]*t[2];
  return q;
  }


FXQuat lerp(const FXQuat& u,const FXQuat& v,const FXfloat f){
  register double alpha,beta,theta,sin_t,cos_t;
  register int flip=0;
  cos_t = u[0]*v[0]+u[1]*v[1]+u[2]*v[2]+u[3]*v[3];
  if(cos_t<0.0){ cos_t = -cos_t; flip=1; }
  if((1.0-cos_t)<0.000001){
    beta = 1.0-f;
    alpha = f;
    }
  else{
    theta = acos(cos_t);
    sin_t = sin(theta);
    beta = sin(theta-f*theta)/sin_t;
    alpha = sin(f*theta)/sin_t;
    }
  if(flip) alpha = -alpha;
  return FXQuat(beta*u[0]+alpha*v[0],beta*u[1]+alpha*v[1],beta*u[2]+alpha*v[2],beta*u[3]+alpha*v[3]);
  }

