/********************************************************************************
*                                                                               *
*                Q u a t e r n i o n  D o u b l e -  F u n c t i o n s          *
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
* $Id: FXDQuat.cpp,v 1.12 2002/01/18 22:42:59 jeroen Exp $                      *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXObject.h"
#include "FXDVec.h"
#include "FXDHVec.h"
#include "FXDQuat.h"

/* Return the value val with the sign of s */
#define SIGN(val,s) (((s)<0)?-(val):(val))


/*******************************************************************************/


FXDQuat::FXDQuat(const FXDVec& axis,FXdouble phi){
  register FXdouble a=0.5*phi;
  register FXdouble s=sin(a)/len(axis);
  v[0]=axis[0]*s;
  v[1]=axis[1]*s;
  v[2]=axis[2]*s;
  v[3]=cos(a);
  }


FXDQuat::FXDQuat(FXdouble roll,FXdouble pitch,FXdouble yaw){
  setRollPitchYaw(roll,pitch,yaw);
  }


// Set quaternion from roll (x), pitch(y) yaw (z)
void FXDQuat::setRollPitchYaw(FXdouble roll,FXdouble pitch,FXdouble yaw){
  register FXdouble r,p,y,sr,cr,sp,cp,sy,cy;
  r=0.5*roll;
  p=0.5*pitch;
  y=0.5*yaw;
  sr=sin(r); cr=cos(r);
  sp=sin(p); cp=cos(p);
  sy=sin(y); cy=cos(y);
  v[0]=sr*cp*cy-cr*sp*sy;
  v[1]=cr*sp*cy+sr*cp*sy;
  v[2]=cr*cp*sy-sr*sp*cy;
  v[3]=cr*cp*cy+sr*sp*sy;
  }


// Obtain yaw, pitch, and roll
// Math is from "3D Game Engine Design" by David Eberly pp 19-20.
// However, instead of testing asin(Sy) against -PI/2 and PI/2, I
// test Sy against -1 and 1; this is numerically more stable, as
// asin doesn't like arguments outside [-1,1].
void FXDQuat::getRollPitchYaw(FXdouble& roll,FXdouble& pitch,FXdouble& yaw){
  register FXdouble x=v[0];
  register FXdouble y=v[1];
  register FXdouble z=v[2];
  register FXdouble w=v[3];
  register FXdouble s=2.0*(w*y-x*z);
  if(s<1.0){
    if(-1.0<s){
      roll=atan2(2.0*(y*z+w*x),1.0-2.0*(x*x+y*y));
      pitch=asin(s);
      yaw=atan2(2.0*(x*y+w*z),1.0-2.0*(y*y+z*z));
      }
    else{
      roll=-atan2(2.0*(x*y-w*z),1.0-2.0*(x*x+z*z));
      pitch=-1.57079632679489661923;
      yaw=0.0;
      }
    }
  else{
    roll=atan2(2.0*(x*y-w*z),1.0-2.0*(x*x+z*z));
    pitch=1.57079632679489661923;
    yaw=0.0;
    }
  }



// Make into unit quaternion
FXDQuat& FXDQuat::adjust(){
  register FXdouble len=v[0]*v[0]+v[1]*v[1]+v[2]*v[2]+v[3]*v[3];
  register FXdouble f;
  if(len>0.0){
    f=1.0/sqrt(len);
    v[0]*=f;
    v[1]*=f;
    v[2]*=f;
    v[3]*=f;
    }
  return *this;
  }


FXDQuat exp(const FXDQuat& q){
  register FXdouble theta=sqrt(q[0]*q[0]+q[1]*q[1]+q[2]);
  register FXdouble scale=1.0;
  FXDQuat result;
  if(theta>0.000001) scale=sin(theta)/theta;
  result[0]=scale*q[0];
  result[1]=scale*q[1];
  result[2]=scale*q[2];
  result[3]=cos(theta);
  return result;
  }


FXDQuat log(const FXDQuat& q){
  register FXdouble scale=sqrt(q[0]*q[0]+q[1]*q[1]+q[2]);
  register FXdouble theta=atan2(scale,q[3]);
  FXDQuat result;
  if(scale>0.0) scale=theta/scale;
  result[0]=scale*q[0];
  result[1]=scale*q[1];
  result[2]=scale*q[2];
  result[3]=0.0f;
  return result;
  }


FXDQuat invert(const FXDQuat& q){
  register FXdouble n=q[0]*q[0]+q[1]*q[1]+q[2]*q[2]+q[3]*q[3];
  FXASSERT(n>0.0);
  return FXDQuat(-q[0]/n,-q[1]/n,-q[2]/n,-q[3]/n);
  }


FXDQuat conj(const FXDQuat& q){
  return FXDQuat(-q[0],-q[1],-q[2],q[3]);
  }


FXDQuat operator*(const FXDQuat& p,const FXDQuat& q){
  return FXDQuat(p[3]*q[0]+p[0]*q[3]+p[1]*q[2]-p[2]*q[1],
                 p[3]*q[1]+p[1]*q[3]+p[2]*q[0]-p[0]*q[2],
                 p[3]*q[2]+p[2]*q[3]+p[0]*q[1]-p[1]*q[0],
                 p[3]*q[3]-p[0]*q[0]-p[1]*q[1]-p[2]*q[2]);
  }


FXDQuat arc(const FXDVec& f,const FXDVec& t){
  FXDQuat q;
  q.v[0]=f[1]*t[2]-f[2]*t[1];
  q.v[1]=f[2]*t[0]-f[0]*t[2];
  q.v[2]=f[0]*t[1]-f[1]*t[0];
  q.v[3]=f[0]*t[0]+f[1]*t[1]+f[2]*t[2];
  return q;
  }


FXDQuat lerp(const FXDQuat& u,const FXDQuat& v,FXdouble f){
  register FXdouble alpha,beta,theta,sin_t,cos_t;
  register FXint flip=0;
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
  return FXDQuat(beta*u[0]+alpha*v[0],beta*u[1]+alpha*v[1],beta*u[2]+alpha*v[2],beta*u[3]+alpha*v[3]);
  }

