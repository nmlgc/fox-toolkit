/********************************************************************************
*                                                                               *
*     H o m o g e n e o u s   D o u b l e - M a t r i x   O p e r a t i o n s   *
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
* $Id: FXDHMat.cpp,v 1.10 2002/01/18 22:42:59 jeroen Exp $                       *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXObject.h"
#include "FXDVec.h"
#include "FXDHVec.h"
#include "FXDQuat.h"
#include "FXDHMat.h"



/*
  Notes:
  - Transformations pre-multiply.
  - Goal is same effect as OpenGL.
*/

#define DET2(a00,a01, \
             a10,a11) ((a00)*(a11)-(a10)*(a01))

#define DET3(a00,a01,a02, \
             a10,a11,a12, \
             a20,a21,a22) ((a00)*DET2(a11,a12,a21,a22) - \
                           (a10)*DET2(a01,a02,a21,a22) + \
                           (a20)*DET2(a01,a02,a11,a12))

#define DET4(a00,a01,a02,a03, \
             a10,a11,a12,a13, \
             a20,a21,a22,a23, \
             a30,a31,a32,a33) ((a00)*DET3(a11,a12,a13,a21,a22,a23,a31,a32,a33) - \
                               (a10)*DET3(a01,a02,a03,a21,a22,a23,a31,a32,a33) + \
                               (a20)*DET3(a01,a02,a03,a11,a12,a13,a31,a32,a33) - \
                               (a30)*DET3(a01,a02,a03,a11,a12,a13,a21,a22,a23))

/*******************************************************************************/

// Build matrix from constant
FXDHMat::FXDHMat(FXdouble w){
  m[0][0]=w; m[0][1]=w; m[0][2]=w; m[0][3]=w;
  m[1][0]=w; m[1][1]=w; m[1][2]=w; m[1][3]=w;
  m[2][0]=w; m[2][1]=w; m[2][2]=w; m[2][3]=w;
  m[3][0]=w; m[3][1]=w; m[3][2]=w; m[3][3]=w;
  }


// Build matrix from scalars
FXDHMat::FXDHMat(FXdouble a00,FXdouble a01,FXdouble a02,FXdouble a03,
                 FXdouble a10,FXdouble a11,FXdouble a12,FXdouble a13,
                 FXdouble a20,FXdouble a21,FXdouble a22,FXdouble a23,
                 FXdouble a30,FXdouble a31,FXdouble a32,FXdouble a33){
  m[0][0]=a00; m[0][1]=a01; m[0][2]=a02; m[0][3]=a03;
  m[1][0]=a10; m[1][1]=a11; m[1][2]=a12; m[1][3]=a13;
  m[2][0]=a20; m[2][1]=a21; m[2][2]=a22; m[2][3]=a23;
  m[3][0]=a30; m[3][1]=a31; m[3][2]=a32; m[3][3]=a33;
  }


// Build matrix from four vectors
FXDHMat::FXDHMat(const FXDHVec& a,const FXDHVec& b,const FXDHVec& c,const FXDHVec& d){
  m[0][0]=a[0]; m[0][1]=a[1]; m[0][2]=a[2]; m[0][3]=a[3];
  m[1][0]=b[0]; m[1][1]=b[1]; m[1][2]=b[2]; m[1][3]=b[3];
  m[2][0]=c[0]; m[2][1]=c[1]; m[2][2]=c[2]; m[2][3]=c[3];
  m[3][0]=d[0]; m[3][1]=d[1]; m[3][2]=d[2]; m[3][3]=d[3];
  }


// Copy constructor
FXDHMat::FXDHMat(const FXDHMat& other){
  m[0]=other.m[0];
  m[1]=other.m[1];
  m[2]=other.m[2];
  m[3]=other.m[3];
  }


// Assignment operator
FXDHMat& FXDHMat::operator=(const FXDHMat& other){
  if(&other!=this){
    m[0]=other.m[0];
    m[1]=other.m[1];
    m[2]=other.m[2];
    m[3]=other.m[3];
    }
  return *this;
  }


// Set matrix to constant
FXDHMat& FXDHMat::operator=(FXdouble w){
  m[0][0]=w; m[0][1]=w; m[0][2]=w; m[0][3]=w;
  m[1][0]=w; m[1][1]=w; m[1][2]=w; m[1][3]=w;
  m[2][0]=w; m[2][1]=w; m[2][2]=w; m[2][3]=w;
  m[3][0]=w; m[3][1]=w; m[3][2]=w; m[3][3]=w;
  return *this;
  }


// Add matrices
FXDHMat& FXDHMat::operator+=(const FXDHMat& w){
  m[0][0]+=w.m[0][0]; m[0][1]+=w.m[0][1]; m[0][2]+=w.m[0][2]; m[0][3]+=w.m[0][3];
  m[1][0]+=w.m[1][0]; m[1][1]+=w.m[1][1]; m[1][2]+=w.m[1][2]; m[1][3]+=w.m[1][3];
  m[2][0]+=w.m[2][0]; m[2][1]+=w.m[2][1]; m[2][2]+=w.m[2][2]; m[2][3]+=w.m[2][3];
  m[3][0]+=w.m[3][0]; m[3][1]+=w.m[3][1]; m[3][2]+=w.m[3][2]; m[3][3]+=w.m[3][3];
  return *this;
  }


// Substract matrices
FXDHMat& FXDHMat::operator-=(const FXDHMat& w){
  m[0][0]-=w.m[0][0]; m[0][1]-=w.m[0][1]; m[0][2]-=w.m[0][2]; m[0][3]-=w.m[0][3];
  m[1][0]-=w.m[1][0]; m[1][1]-=w.m[1][1]; m[1][2]-=w.m[1][2]; m[1][3]-=w.m[1][3];
  m[2][0]-=w.m[2][0]; m[2][1]-=w.m[2][1]; m[2][2]-=w.m[2][2]; m[2][3]-=w.m[2][3];
  m[3][0]-=w.m[3][0]; m[3][1]-=w.m[3][1]; m[3][2]-=w.m[3][2]; m[3][3]-=w.m[3][3];
  return *this;
  }


// Multiply matrix by scalar
FXDHMat& FXDHMat::operator*=(FXdouble w){
  m[0][0]*=w; m[0][1]*=w; m[0][2]*=w; m[0][3]*=w;
  m[1][0]*=w; m[1][1]*=w; m[1][2]*=w; m[2][3]*=w;
  m[2][0]*=w; m[2][1]*=w; m[2][2]*=w; m[3][3]*=w;
  m[3][0]*=w; m[3][1]*=w; m[3][2]*=w; m[3][3]*=w;
  return *this;
  }


// Multiply matrix by matrix
FXDHMat& FXDHMat::operator*=(const FXDHMat& w){
  register FXdouble x,y,z,h;
  x=m[0][0]; y=m[0][1]; z=m[0][2]; h=m[0][3];
  m[0][0]=x*w.m[0][0]+y*w.m[1][0]+z*w.m[2][0]+h*w.m[3][0];
  m[0][1]=x*w.m[0][1]+y*w.m[1][1]+z*w.m[2][1]+h*w.m[3][1];
  m[0][2]=x*w.m[0][2]+y*w.m[1][2]+z*w.m[2][2]+h*w.m[3][2];
  m[0][3]=x*w.m[0][3]+y*w.m[1][3]+z*w.m[2][3]+h*w.m[3][3];
  x=m[1][0]; y=m[1][1]; z=m[1][2]; h=m[1][3];
  m[1][0]=x*w.m[0][0]+y*w.m[1][0]+z*w.m[2][0]+h*w.m[3][0];
  m[1][1]=x*w.m[0][1]+y*w.m[1][1]+z*w.m[2][1]+h*w.m[3][1];
  m[1][2]=x*w.m[0][2]+y*w.m[1][2]+z*w.m[2][2]+h*w.m[3][2];
  m[1][3]=x*w.m[0][3]+y*w.m[1][3]+z*w.m[2][3]+h*w.m[3][3];
  x=m[2][0]; y=m[2][1]; z=m[2][2]; h=m[2][3];
  m[2][0]=x*w.m[0][0]+y*w.m[1][0]+z*w.m[2][0]+h*w.m[3][0];
  m[2][1]=x*w.m[0][1]+y*w.m[1][1]+z*w.m[2][1]+h*w.m[3][1];
  m[2][2]=x*w.m[0][2]+y*w.m[1][2]+z*w.m[2][2]+h*w.m[3][2];
  m[2][3]=x*w.m[0][3]+y*w.m[1][3]+z*w.m[2][3]+h*w.m[3][3];
  x=m[3][0]; y=m[3][1]; z=m[3][2]; h=m[3][3];
  m[3][0]=x*w.m[0][0]+y*w.m[1][0]+z*w.m[2][0]+h*w.m[3][0];
  m[3][1]=x*w.m[0][1]+y*w.m[1][1]+z*w.m[2][1]+h*w.m[3][1];
  m[3][2]=x*w.m[0][2]+y*w.m[1][2]+z*w.m[2][2]+h*w.m[3][2];
  m[3][3]=x*w.m[0][3]+y*w.m[1][3]+z*w.m[2][3]+h*w.m[3][3];
  return *this;
  }


// Divide matric by scalar
FXDHMat& FXDHMat::operator/=(FXdouble w){
  m[0][0]/=w; m[0][1]/=w; m[0][2]/=w; m[0][3]/=w;
  m[1][0]/=w; m[1][1]/=w; m[1][2]/=w; m[1][3]/=w;
  m[2][0]/=w; m[2][1]/=w; m[2][2]/=w; m[2][3]/=w;
  m[3][0]/=w; m[3][1]/=w; m[3][2]/=w; m[3][3]/=w;
  return *this;
  }


// Add matrices
FXDHMat operator+(const FXDHMat& a,const FXDHMat& b){
  return FXDHMat(a.m[0][0]+b.m[0][0],a.m[0][1]+b.m[0][1],a.m[0][2]+b.m[0][2],a.m[0][3]+b.m[0][3],
                 a.m[1][0]+b.m[1][0],a.m[1][1]+b.m[1][1],a.m[1][2]+b.m[1][2],a.m[1][3]+b.m[1][3],
                 a.m[2][0]+b.m[2][0],a.m[2][1]+b.m[2][1],a.m[2][2]+b.m[2][2],a.m[2][3]+b.m[2][3],
                 a.m[3][0]+b.m[3][0],a.m[3][1]+b.m[3][1],a.m[3][2]+b.m[3][2],a.m[3][3]+b.m[3][3]);
  }


// Substract matrices
FXDHMat operator-(const FXDHMat& a,const FXDHMat& b){
  return FXDHMat(a.m[0][0]-b.m[0][0],a.m[0][1]-b.m[0][1],a.m[0][2]-b.m[0][2],a.m[0][3]-b.m[0][3],
                 a.m[1][0]-b.m[1][0],a.m[1][1]-b.m[1][1],a.m[1][2]-b.m[1][2],a.m[1][3]-b.m[1][3],
                 a.m[2][0]-b.m[2][0],a.m[2][1]-b.m[2][1],a.m[2][2]-b.m[2][2],a.m[2][3]-b.m[2][3],
                 a.m[3][0]-b.m[3][0],a.m[3][1]-b.m[3][1],a.m[3][2]-b.m[3][2],a.m[3][3]-b.m[3][3]);
  }


// Negate matrix
FXDHMat operator-(const FXDHMat& a){
  return FXDHMat(-a.m[0][0],-a.m[0][1],-a.m[0][2],-a.m[0][3],
                 -a.m[1][0],-a.m[1][1],-a.m[1][2],-a.m[1][3],
                 -a.m[2][0],-a.m[2][1],-a.m[2][2],-a.m[2][3],
                 -a.m[3][0],-a.m[3][1],-a.m[3][2],-a.m[3][3]);
  }



// Composite matrices
FXDHMat operator*(const FXDHMat& a,const FXDHMat& b){
  FXDHMat r;
  register FXdouble x,y,z,h;
  x=a.m[0][0]; y=a.m[0][1]; z=a.m[0][2]; h=a.m[0][3];
  r.m[0][0]=x*b.m[0][0]+y*b.m[1][0]+z*b.m[2][0]+h*b.m[3][0];
  r.m[0][1]=x*b.m[0][1]+y*b.m[1][1]+z*b.m[2][1]+h*b.m[3][1];
  r.m[0][2]=x*b.m[0][2]+y*b.m[1][2]+z*b.m[2][2]+h*b.m[3][2];
  r.m[0][3]=x*b.m[0][3]+y*b.m[1][3]+z*b.m[2][3]+h*b.m[3][3];
  x=a.m[1][0]; y=a.m[1][1]; z=a.m[1][2]; h=a.m[1][3];
  r.m[1][0]=x*b.m[0][0]+y*b.m[1][0]+z*b.m[2][0]+h*b.m[3][0];
  r.m[1][1]=x*b.m[0][1]+y*b.m[1][1]+z*b.m[2][1]+h*b.m[3][1];
  r.m[1][2]=x*b.m[0][2]+y*b.m[1][2]+z*b.m[2][2]+h*b.m[3][2];
  r.m[1][3]=x*b.m[0][3]+y*b.m[1][3]+z*b.m[2][3]+h*b.m[3][3];
  x=a.m[2][0]; y=a.m[2][1]; z=a.m[2][2]; h=a.m[2][3];
  r.m[2][0]=x*b.m[0][0]+y*b.m[1][0]+z*b.m[2][0]+h*b.m[3][0];
  r.m[2][1]=x*b.m[0][1]+y*b.m[1][1]+z*b.m[2][1]+h*b.m[3][1];
  r.m[2][2]=x*b.m[0][2]+y*b.m[1][2]+z*b.m[2][2]+h*b.m[3][2];
  r.m[2][3]=x*b.m[0][3]+y*b.m[1][3]+z*b.m[2][3]+h*b.m[3][3];
  x=a.m[3][0]; y=a.m[3][1]; z=a.m[3][2]; h=a.m[3][3];
  r.m[3][0]=x*b.m[0][0]+y*b.m[1][0]+z*b.m[2][0]+h*b.m[3][0];
  r.m[3][1]=x*b.m[0][1]+y*b.m[1][1]+z*b.m[2][1]+h*b.m[3][1];
  r.m[3][2]=x*b.m[0][2]+y*b.m[1][2]+z*b.m[2][2]+h*b.m[3][2];
  r.m[3][3]=x*b.m[0][3]+y*b.m[1][3]+z*b.m[2][3]+h*b.m[3][3];
  return r;
  }


// Multiply scalar by matrix
FXDHMat operator*(FXdouble x,const FXDHMat& a){
  return FXDHMat(x*a.m[0][0],x*a.m[0][1],x*a.m[0][2],a.m[0][3],
                 x*a.m[1][0],x*a.m[1][1],x*a.m[1][2],a.m[1][3],
                 x*a.m[2][0],x*a.m[2][1],x*a.m[2][2],a.m[2][3],
                 x*a.m[3][0],x*a.m[3][1],x*a.m[3][2],a.m[3][3]);
  }


// Multiply matrix by scalar
FXDHMat operator*(const FXDHMat& a,FXdouble x){
  return FXDHMat(a.m[0][0]*x,a.m[0][1]*x,a.m[0][2]*x,a.m[0][3],
                 a.m[1][0]*x,a.m[1][1]*x,a.m[1][2]*x,a.m[1][3],
                 a.m[2][0]*x,a.m[2][1]*x,a.m[2][2]*x,a.m[2][3],
                 a.m[3][0]*x,a.m[3][1]*x,a.m[3][2]*x,a.m[3][3]);
  }


// Divide scalar by matrix
FXDHMat operator/(FXdouble x,const FXDHMat& a){
  return FXDHMat(x/a.m[0][0],x/a.m[0][1],x/a.m[0][2],a.m[0][3],
                 x/a.m[1][0],x/a.m[1][1],x/a.m[1][2],a.m[1][3],
                 x/a.m[2][0],x/a.m[2][1],x/a.m[2][2],a.m[2][3],
                 x/a.m[3][0],x/a.m[3][1],x/a.m[3][2],a.m[3][3]);
  }


// Divide matrix by scalar
FXDHMat operator/(const FXDHMat& a,FXdouble x){
  return FXDHMat(a.m[0][0]/x,a.m[0][1]/x,a.m[0][2]/x,a.m[0][3],
                 a.m[1][0]/x,a.m[1][1]/x,a.m[1][2]/x,a.m[1][3],
                 a.m[2][0]/x,a.m[2][1]/x,a.m[2][2]/x,a.m[2][3],
                 a.m[3][0]/x,a.m[3][1]/x,a.m[3][2]/x,a.m[3][3]);
  }


// Vector times matrix
FXDHVec operator*(const FXDHVec& v,const FXDHMat& m){
  register FXdouble x=v[0],y=v[1],z=v[2],w=v[3];
  return FXDHVec(x*m.m[0][0]+y*m.m[1][0]+z*m.m[2][0]+w*m.m[3][0],
                 x*m.m[0][1]+y*m.m[1][1]+z*m.m[2][1]+w*m.m[3][1],
                 x*m.m[0][2]+y*m.m[1][2]+z*m.m[2][2]+w*m.m[3][2],
                 x*m.m[0][3]+y*m.m[1][3]+z*m.m[2][3]+w*m.m[3][3]);
  }


// Matrix times vector
FXDHVec operator*(const FXDHMat& m,const FXDHVec& v){
  register FXdouble x=v[0],y=v[1],z=v[2],w=v[3];
  return FXDHVec(x*m.m[0][0]+y*m.m[0][1]+z*m.m[0][2]+w*m.m[0][3],
                 x*m.m[1][0]+y*m.m[1][1]+z*m.m[1][2]+w*m.m[1][3],
                 x*m.m[2][0]+y*m.m[2][1]+z*m.m[2][2]+w*m.m[2][3],
                 x*m.m[3][0]+y*m.m[3][1]+z*m.m[3][2]+w*m.m[3][3]);
  }


// Vector times matrix
FXDVec operator*(const FXDVec& v,const FXDHMat& m){
  register FXdouble x=v[0],y=v[1],z=v[2];
  FXASSERT(m.m[0][3]==0.0 && m.m[1][3]==0.0 && m.m[2][3]==0.0 && m.m[3][3]==1.0);
  return FXDVec(x*m.m[0][0]+y*m.m[1][0]+z*m.m[2][0]+m.m[3][0],
                x*m.m[0][1]+y*m.m[1][1]+z*m.m[2][1]+m.m[3][1],
                x*m.m[0][2]+y*m.m[1][2]+z*m.m[2][2]+m.m[3][2]);
  }


// Matrix times vector
FXDVec operator*(const FXDHMat& m,const FXDVec& v){
  register FXdouble x=v[0],y=v[1],z=v[2];
  FXASSERT(m.m[0][3]==0.0 && m.m[1][3]==0.0 && m.m[2][3]==0.0 && m.m[3][3]==1.0);
  return FXDVec(x*m.m[0][0]+y*m.m[0][1]+z*m.m[0][2]+m.m[0][3],
                x*m.m[1][0]+y*m.m[1][1]+z*m.m[1][2]+m.m[1][3],
                x*m.m[2][0]+y*m.m[2][1]+z*m.m[2][2]+m.m[2][3]);
  }


// Make unit matrix
FXDHMat& FXDHMat::eye(){
  m[0][0]=1.0; m[0][1]=0.0; m[0][2]=0.0; m[0][3]=0.0;
  m[1][0]=0.0; m[1][1]=1.0; m[1][2]=0.0; m[1][3]=0.0;
  m[2][0]=0.0; m[2][1]=0.0; m[2][2]=1.0; m[2][3]=0.0;
  m[3][0]=0.0; m[3][1]=0.0; m[3][2]=0.0; m[3][3]=1.0;
  return *this;
  }


// Orthographic projection
FXDHMat& FXDHMat::ortho(FXdouble left,FXdouble right,FXdouble bottom,FXdouble top,FXdouble hither,FXdouble yon){
  register FXdouble x,y,z,tx,ty,tz,rl,tb,yh,r0,r1,r2,r3;
  rl=right-left;
  tb=top-bottom;
  yh=yon-hither;
  FXASSERT(rl && tb && yh);         // Throw exception in future
  x= 2.0/rl;
  y= 2.0/tb;
  z=-2.0/yh;
  tx=-(right+left)/rl;
  ty=-(top+bottom)/tb;
  tz=-(yon+hither)/yh;
  r0=m[0][0];
  r1=m[1][0];
  r2=m[2][0];
  r3=m[3][0];
  m[0][0]=x*r0;
  m[1][0]=y*r1;
  m[2][0]=z*r2;
  m[3][0]=tx*r0+ty*r1+tz*r2+r3;
  r0=m[0][1];
  r1=m[1][1];
  r2=m[2][1];
  r3=m[3][1];
  m[0][1]=x*r0;
  m[1][1]=y*r1;
  m[2][1]=z*r2;
  m[3][1]=tx*r0+ty*r1+tz*r2+r3;
  r0=m[0][2];
  r1=m[1][2];
  r2=m[2][2];
  r3=m[3][2];
  m[0][2]=x*r0;
  m[1][2]=y*r1;
  m[2][2]=z*r2;
  m[3][2]=tx*r0+ty*r1+tz*r2+r3;
  r0=m[0][3];
  r1=m[1][3];
  r2=m[2][3];
  r3=m[3][3];
  m[0][3]=x*r0;
  m[1][3]=y*r1;
  m[2][3]=z*r2;
  m[3][3]=tx*r0+ty*r1+tz*r2+r3;
  return *this;
  }


// Perspective projection
FXDHMat& FXDHMat::frustum(FXdouble left,FXdouble right,FXdouble bottom,FXdouble top,FXdouble hither,FXdouble yon){
  register FXdouble x,y,a,b,c,d,rl,tb,yh,r0,r1,r2,r3;
  FXASSERT(0.0<hither && hither<yon);   // Throw exception in future
  rl=right-left;
  tb=top-bottom;
  yh=yon-hither;
  FXASSERT(rl && tb);                   // Throw exception in future
  x= 2.0*hither/rl;
  y= 2.0*hither/tb;
  a= (right+left)/rl;
  b= (top+bottom)/tb;
  c=-(yon+hither)/yh;
  d=-(2.0*yon*hither)/yh;
  r0=m[0][0];
  r1=m[1][0];
  r2=m[2][0];
  r3=m[3][0];
  m[0][0]=x*r0;
  m[1][0]=y*r1;
  m[2][0]=a*r0+b*r1+c*r2-r3;
  m[3][0]=d*r2;
  r0=m[0][1];
  r1=m[1][1];
  r2=m[2][1];
  r3=m[3][1];
  m[0][1]=x*r0;
  m[1][1]=y*r1;
  m[2][1]=a*r0+b*r1+c*r2-r3;
  m[3][1]=d*r2;
  r0=m[0][2];
  r1=m[1][2];
  r2=m[2][2];
  r3=m[3][2];
  m[0][2]=x*r0;
  m[1][2]=y*r1;
  m[2][2]=a*r0+b*r1+c*r2-r3;
  m[3][2]=d*r2;
  r0=m[0][3];
  r1=m[1][3];
  r2=m[2][3];
  r3=m[3][3];
  m[0][3]=x*r0;
  m[1][3]=y*r1;
  m[2][3]=a*r0+b*r1+c*r2-r3;
  m[3][3]=d*r2;
  return *this;
  }


// Make left hand matrix
FXDHMat& FXDHMat::left(){
  m[2][0]= -m[2][0];
  m[2][1]= -m[2][1];
  m[2][2]= -m[2][2];
  m[2][3]= -m[2][3];
  return *this;
  }



// Rotate using quaternion
FXDHMat& FXDHMat::rot(const FXDQuat& q){
  register FXdouble r00,r01,r02,r10,r11,r12,r20,r21,r22;
  register FXdouble x2,y2,z2,xx2,yy2,zz2,xy2,xz2,yz2,wx2,wy2,wz2;
  register FXdouble x,y,z;

  // Pre-calculate some stuff
  x2 =  q[0]*2.0; y2 = q[1]*2.0; z2 = q[2]*2.0;
  xx2 = q[0]*x2; yy2 = q[1]*y2; zz2 = q[2]*z2;
  xy2 = q[0]*y2; xz2 = q[0]*z2; yz2 = q[1]*z2;
  wx2 = q[3]*x2; wy2 = q[3]*y2; wz2 = q[3]*z2;

  // Rotation matrix
  r00 = 1.0-yy2-zz2; r01 = xy2+wz2; r02 = xz2-wy2;
  r10 = xy2-wz2; r11 = 1.0-xx2-zz2; r12 = yz2+wx2;
  r20 = xz2+wy2; r21 = yz2-wx2; r22 = 1.0-xx2-yy2;

  // Pre-multiply
  x=m[0][0];
  y=m[1][0];
  z=m[2][0];
  m[0][0]=x*r00+y*r01+z*r02;
  m[1][0]=x*r10+y*r11+z*r12;
  m[2][0]=x*r20+y*r21+z*r22;
  x=m[0][1];
  y=m[1][1];
  z=m[2][1];
  m[0][1]=x*r00+y*r01+z*r02;
  m[1][1]=x*r10+y*r11+z*r12;
  m[2][1]=x*r20+y*r21+z*r22;
  x=m[0][2];
  y=m[1][2];
  z=m[2][2];
  m[0][2]=x*r00+y*r01+z*r02;
  m[1][2]=x*r10+y*r11+z*r12;
  m[2][2]=x*r20+y*r21+z*r22;
  x=m[0][3];
  y=m[1][3];
  z=m[2][3];
  m[0][3]=x*r00+y*r01+z*r02;
  m[1][3]=x*r10+y*r11+z*r12;
  m[2][3]=x*r20+y*r21+z*r22;
  return *this;
  }


// Rotate about arbitrary vector
FXDHMat& FXDHMat::rot(const FXDVec& v,FXdouble c,FXdouble s){
  register FXdouble xx,yy,zz,xy,yz,zx,xs,ys,zs,t;
  register FXdouble r00,r01,r02,r10,r11,r12,r20,r21,r22;
  register FXdouble x=v[0];
  register FXdouble y=v[1];
  register FXdouble z=v[2];
  register FXdouble mag=x*x+y*y+z*z;
  FXASSERT(-1.00001<c && c<1.00001 && -1.00001<s && s<1.00001);
  if(mag<=1.0E-30) return *this;         // Rotation about 0-length axis
  mag=sqrt(mag);
  x/=mag;
  y/=mag;
  z/=mag;
  xx=x*x;
  yy=y*y;
  zz=z*z;
  xy=x*y;
  yz=y*z;
  zx=z*x;
  xs=x*s;
  ys=y*s;
  zs=z*s;
  t=1.0f-c;
  r00=t*xx+c;  r10=t*xy-zs; r20=t*zx+ys;
  r01=t*xy+zs; r11=t*yy+c;  r21=t*yz-xs;
  r02=t*zx-ys; r12=t*yz+xs; r22=t*zz+c;
  x=m[0][0];
  y=m[1][0];
  z=m[2][0];
  m[0][0]=x*r00+y*r01+z*r02;
  m[1][0]=x*r10+y*r11+z*r12;
  m[2][0]=x*r20+y*r21+z*r22;
  x=m[0][1];
  y=m[1][1];
  z=m[2][1];
  m[0][1]=x*r00+y*r01+z*r02;
  m[1][1]=x*r10+y*r11+z*r12;
  m[2][1]=x*r20+y*r21+z*r22;
  x=m[0][2];
  y=m[1][2];
  z=m[2][2];
  m[0][2]=x*r00+y*r01+z*r02;
  m[1][2]=x*r10+y*r11+z*r12;
  m[2][2]=x*r20+y*r21+z*r22;
  x=m[0][3];
  y=m[1][3];
  z=m[2][3];
  m[0][3]=x*r00+y*r01+z*r02;
  m[1][3]=x*r10+y*r11+z*r12;
  m[2][3]=x*r20+y*r21+z*r22;
  return *this;
  }


// Rotate by angle about arbitrary vector
FXDHMat& FXDHMat::rot(const FXDVec& v,FXdouble phi){
  return rot(v,cos(phi),sin(phi));
  }


// Rotate about x-axis
FXDHMat& FXDHMat::xrot(FXdouble c,FXdouble s){
  register FXdouble u,v;
  FXASSERT(-1.00001<c && c<1.00001 && -1.00001<s && s<1.00001);
  u=m[1][0]; v=m[2][0]; m[1][0]=c*u+s*v; m[2][0]=c*v-s*u;
  u=m[1][1]; v=m[2][1]; m[1][1]=c*u+s*v; m[2][1]=c*v-s*u;
  u=m[1][2]; v=m[2][2]; m[1][2]=c*u+s*v; m[2][2]=c*v-s*u;
  u=m[1][3]; v=m[2][3]; m[1][3]=c*u+s*v; m[2][3]=c*v-s*u;
  return *this;
  }


// Rotate by angle about x-axis
FXDHMat& FXDHMat::xrot(FXdouble phi){
  return xrot(cos(phi),sin(phi));
  }


// Rotate about y-axis
FXDHMat& FXDHMat::yrot(FXdouble c,FXdouble s){
  register FXdouble u,v;
  FXASSERT(-1.00001<c && c<1.00001 && -1.00001<s && s<1.00001);
  u=m[0][0]; v=m[2][0]; m[0][0]=c*u-s*v; m[2][0]=c*v+s*u;
  u=m[0][1]; v=m[2][1]; m[0][1]=c*u-s*v; m[2][1]=c*v+s*u;
  u=m[0][2]; v=m[2][2]; m[0][2]=c*u-s*v; m[2][2]=c*v+s*u;
  u=m[0][3]; v=m[2][3]; m[0][3]=c*u-s*v; m[2][3]=c*v+s*u;
  return *this;
  }


// Rotate by angle about y-axis
FXDHMat& FXDHMat::yrot(FXdouble phi){
  return yrot(cos(phi),sin(phi));
  }


// Rotate about z-axis
FXDHMat& FXDHMat::zrot(FXdouble c,FXdouble s){
  register FXdouble u,v;
  FXASSERT(-1.00001<c && c<1.00001 && -1.00001<s && s<1.00001);
  u=m[0][0]; v=m[1][0]; m[0][0]=c*u+s*v; m[1][0]=c*v-s*u;
  u=m[0][1]; v=m[1][1]; m[0][1]=c*u+s*v; m[1][1]=c*v-s*u;
  u=m[0][2]; v=m[1][2]; m[0][2]=c*u+s*v; m[1][2]=c*v-s*u;
  u=m[0][3]; v=m[1][3]; m[0][3]=c*u+s*v; m[1][3]=c*v-s*u;
  return *this;
  }


// Rotate by angle about z-axis
FXDHMat& FXDHMat::zrot(FXdouble phi){
  return zrot(cos(phi),sin(phi));
  }


// Translate
FXDHMat& FXDHMat::trans(FXdouble tx,FXdouble ty,FXdouble tz){
  m[3][0]=m[3][0]+tx*m[0][0]+ty*m[1][0]+tz*m[2][0];
  m[3][1]=m[3][1]+tx*m[0][1]+ty*m[1][1]+tz*m[2][1];
  m[3][2]=m[3][2]+tx*m[0][2]+ty*m[1][2]+tz*m[2][2];
  m[3][3]=m[3][3]+tx*m[0][3]+ty*m[1][3]+tz*m[2][3];
  return *this;
  }


// Translate over vector
FXDHMat& FXDHMat::trans(const FXDVec& v){
  return trans(v[0],v[1],v[2]);
  }


// Scale unqual
FXDHMat& FXDHMat::scale(FXdouble sx,FXdouble sy,FXdouble sz){
  m[0][0]*=sx; m[0][1]*=sx; m[0][2]*=sx; m[0][3]*=sx;
  m[1][0]*=sy; m[1][1]*=sy; m[1][2]*=sy; m[1][3]*=sy;
  m[2][0]*=sz; m[2][1]*=sz; m[2][2]*=sz; m[2][3]*=sz;
  return *this;
  }


// Scale uniform
FXDHMat& FXDHMat::scale(FXdouble s){
  return scale(s,s,s);
  }


// Scale matrix
FXDHMat& FXDHMat::scale(const FXDVec& v){
  return scale(v[0],v[1],v[2]);
  }


// Calculate determinant (not so good yet...)
FXdouble det(const FXDHMat& a){
  return DET4(a.m[0][0],a.m[0][1],a.m[0][2],a.m[0][3],
              a.m[1][0],a.m[1][1],a.m[1][2],a.m[1][3],
              a.m[2][0],a.m[2][1],a.m[2][2],a.m[2][3],
              a.m[3][0],a.m[3][1],a.m[3][2],a.m[3][3]);
  }


// Transpose matrix
FXDHMat transpose(const FXDHMat& m){
  return FXDHMat(m.m[0][0],m.m[1][0],m.m[2][0],m.m[3][0],
                m.m[0][1],m.m[1][1],m.m[2][1],m.m[3][1],
                m.m[0][2],m.m[1][2],m.m[2][2],m.m[3][2],
                m.m[0][3],m.m[1][3],m.m[2][3],m.m[3][3]);
  }


// Invert matrix
FXDHMat invert(const FXDHMat& s){
  FXDHMat m(1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0);
  FXDHMat x(s);
  register FXdouble pvv,t;
  register int i,j,pvi;
  for(i=0; i<4; i++){
    pvv=x[i][i];
    pvi=i;
    for(j=i+1; j<4; j++){   // Find pivot (largest in column i)
      if(fabs(x[j][i])>fabs(pvv)){
        pvi=j;
        pvv=x[j][i];
        }
      }
    FXASSERT(pvv != 0.0);   // Should not be singular
    if(pvi!=i){             // Swap rows i and pvi
      FXSWAP(m[i][0],m[pvi][0],t); FXSWAP(m[i][1],m[pvi][1],t); FXSWAP(m[i][2],m[pvi][2],t); FXSWAP(m[i][3],m[pvi][3],t);
      FXSWAP(x[i][0],x[pvi][0],t); FXSWAP(x[i][1],x[pvi][1],t); FXSWAP(x[i][2],x[pvi][2],t); FXSWAP(x[i][3],x[pvi][3],t);
      }
    x[i][0]/=pvv; x[i][1]/=pvv; x[i][2]/=pvv; x[i][3]/=pvv;
    m[i][0]/=pvv; m[i][1]/=pvv; m[i][2]/=pvv; m[i][3]/=pvv;
    for(j=0; j<4; j++){     // Eliminate column i
      if(j!=i){
        t=x[j][i];
        x[j][0]-=x[i][0]*t; x[j][1]-=x[i][1]*t; x[j][2]-=x[i][2]*t; x[j][3]-=x[i][3]*t;
        m[j][0]-=m[i][0]*t; m[j][1]-=m[i][1]*t; m[j][2]-=m[i][2]*t; m[j][3]-=m[i][3]*t;
        }
      }
    }
  return m;
  }


// Look at
FXDHMat& FXDHMat::look(const FXDVec& eye,const FXDVec& cntr,const FXDVec& vup){
  register FXdouble x0,x1,x2,tx,ty,tz;
  FXDVec rz,rx,ry;
  rz=normalize(eye-cntr);
  rx=normalize(vup^rz);
  ry=normalize(rz^rx);
  tx= -eye[0]*rx[0]-eye[1]*rx[1]-eye[2]*rx[2];
  ty= -eye[0]*ry[0]-eye[1]*ry[1]-eye[2]*ry[2];
  tz= -eye[0]*rz[0]-eye[1]*rz[1]-eye[2]*rz[2];
  x0=m[0][0]; x1=m[0][1]; x2=m[0][2];
  m[0][0]=rx[0]*x0+rx[1]*x1+rx[2]*x2+tx*m[0][3];
  m[0][1]=ry[0]*x0+ry[1]*x1+ry[2]*x2+ty*m[0][3];
  m[0][2]=rz[0]*x0+rz[1]*x1+rz[2]*x2+tz*m[0][3];
  x0=m[1][0]; x1=m[1][1]; x2=m[1][2];
  m[1][0]=rx[0]*x0+rx[1]*x1+rx[2]*x2+tx*m[1][3];
  m[1][1]=ry[0]*x0+ry[1]*x1+ry[2]*x2+ty*m[1][3];
  m[1][2]=rz[0]*x0+rz[1]*x1+rz[2]*x2+tz*m[1][3];
  x0=m[2][0]; x1=m[2][1]; x2=m[2][2];
  m[2][0]=rx[0]*x0+rx[1]*x1+rx[2]*x2+tx*m[2][3];
  m[2][1]=ry[0]*x0+ry[1]*x1+ry[2]*x2+ty*m[2][3];
  m[2][2]=rz[0]*x0+rz[1]*x1+rz[2]*x2+tz*m[2][3];
  x0=m[3][0]; x1=m[3][1]; x2=m[3][2];
  m[3][0]=rx[0]*x0+rx[1]*x1+rx[2]*x2+tx*m[3][3];
  m[3][1]=ry[0]*x0+ry[1]*x1+ry[2]*x2+ty*m[3][3];
  m[3][2]=rz[0]*x0+rz[1]*x1+rz[2]*x2+tz*m[3][3];
  return *this;
  }



// Save to archive
FXStream& operator<<(FXStream& store,const FXDHMat& m){
  store << m[0] << m[1] << m[2] << m[3];
  return store;
  }


// Load from archive
FXStream& operator>>(FXStream& store,FXDHMat& m){
  store >> m[0] >> m[1] >> m[2] >> m[3];
  return store;
  }

