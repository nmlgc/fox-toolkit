/********************************************************************************
*                                                                               *
*            H o m o g e n e o u s   M a t r i x   O p e r a t i o n s          *
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
* $Id: FXHMat.cpp,v 1.4 1998/10/21 15:31:12 jvz Exp $                         *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXObject.h"
#include "FXVec.h"
#include "FXHVec.h"
#include "FXQuat.h"
#include "FXHMat.h"


#define SWAP(a,b,tmp) ((tmp)=(a),(a)=(b),(b)=(tmp))

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
FXHMat::FXHMat(const FXfloat w){
  m[0][0]=w; m[0][1]=w; m[0][2]=w; m[0][3]=w;
  m[1][0]=w; m[1][1]=w; m[1][2]=w; m[1][3]=w;
  m[2][0]=w; m[2][1]=w; m[2][2]=w; m[2][3]=w;
  m[3][0]=w; m[3][1]=w; m[3][2]=w; m[3][3]=w;
  }


// Build matrix from scalars
FXHMat::FXHMat(const FXfloat a00,const FXfloat a01,const FXfloat a02,const FXfloat a03,
               const FXfloat a10,const FXfloat a11,const FXfloat a12,const FXfloat a13,
               const FXfloat a20,const FXfloat a21,const FXfloat a22,const FXfloat a23,
               const FXfloat a30,const FXfloat a31,const FXfloat a32,const FXfloat a33){
  m[0][0]=a00; m[0][1]=a01; m[0][2]=a02; m[0][3]=a03;
  m[1][0]=a10; m[1][1]=a11; m[1][2]=a12; m[1][3]=a13;
  m[2][0]=a20; m[2][1]=a21; m[2][2]=a22; m[2][3]=a23;
  m[3][0]=a30; m[3][1]=a31; m[3][2]=a32; m[3][3]=a33;
  }


// Build matrix from for vectors
FXHMat::FXHMat(const FXHVec& a,const FXHVec& b,const FXHVec& c,const FXHVec& d){
  m[0][0]=a[0]; m[0][1]=a[1]; m[0][2]=a[2]; m[0][3]=a[3];
  m[1][0]=b[0]; m[1][1]=b[1]; m[1][2]=b[2]; m[1][3]=b[3];
  m[2][0]=c[0]; m[2][1]=c[1]; m[2][2]=c[2]; m[2][3]=c[3];
  m[3][0]=d[0]; m[3][1]=d[1]; m[3][2]=d[2]; m[3][3]=d[3];
  }

  
// Set matrix to constant
FXHMat& FXHMat::operator=(const FXfloat w){
  m[0][0]=w; m[0][1]=w; m[0][2]=w; m[0][3]=w;
  m[1][0]=w; m[1][1]=w; m[1][2]=w; m[1][3]=w;
  m[2][0]=w; m[2][1]=w; m[2][2]=w; m[2][3]=w;
  m[3][0]=w; m[3][1]=w; m[3][2]=w; m[3][3]=w;
  return *this;
  }


// Add matrices
FXHMat& FXHMat::operator+=(const FXHMat& w){
  m[0][0]+=w.m[0][0]; m[0][1]+=w.m[0][1]; m[0][2]+=w.m[0][2]; m[0][3]+=w.m[0][3];
  m[1][0]+=w.m[1][0]; m[1][1]+=w.m[1][1]; m[1][2]+=w.m[1][2]; m[1][3]+=w.m[1][3];
  m[2][0]+=w.m[2][0]; m[2][1]+=w.m[2][1]; m[2][2]+=w.m[2][2]; m[2][3]+=w.m[2][3];
  m[3][0]+=w.m[3][0]; m[3][1]+=w.m[3][1]; m[3][2]+=w.m[3][2]; m[3][3]+=w.m[3][3];
  return *this;
  }


// Substract matrices
FXHMat& FXHMat::operator-=(const FXHMat& w){
  m[0][0]-=w.m[0][0]; m[0][1]-=w.m[0][1]; m[0][2]-=w.m[0][2]; m[0][3]-=w.m[0][3];
  m[1][0]-=w.m[1][0]; m[1][1]-=w.m[1][1]; m[1][2]-=w.m[1][2]; m[1][3]-=w.m[1][3];
  m[2][0]-=w.m[2][0]; m[2][1]-=w.m[2][1]; m[2][2]-=w.m[2][2]; m[2][3]-=w.m[2][3];
  m[3][0]-=w.m[3][0]; m[3][1]-=w.m[3][1]; m[3][2]-=w.m[3][2]; m[3][3]-=w.m[3][3];
  return *this;
  }


// Multiply matrix by scalar
FXHMat& FXHMat::operator*=(const FXfloat w){
  m[0][0]*=w; m[0][1]*=w; m[0][2]*=w; m[0][3]*=w;
  m[1][0]*=w; m[1][1]*=w; m[1][2]*=w; m[2][3]*=w;
  m[2][0]*=w; m[2][1]*=w; m[2][2]*=w; m[3][3]*=w;
  m[3][0]*=w; m[3][1]*=w; m[3][2]*=w; m[3][3]*=w;
  return *this;
  }


// Multiply matrix by matrix
FXHMat& FXHMat::operator*=(const FXHMat& w){
  register FXfloat x,y,z,h;
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
FXHMat& FXHMat::operator/=(const FXfloat w){
  m[0][0]/=w; m[0][1]/=w; m[0][2]/=w; m[0][3]/=w;
  m[1][0]/=w; m[1][1]/=w; m[1][2]/=w; m[1][3]/=w;
  m[2][0]/=w; m[2][1]/=w; m[2][2]/=w; m[2][3]/=w;
  m[3][0]/=w; m[3][1]/=w; m[3][2]/=w; m[3][3]/=w;
  return *this;
  }


// Add matrices
FXHMat operator+(const FXHMat& a,const FXHMat& b){
  return FXHMat(a.m[0][0]+b.m[0][0],a.m[0][1]+b.m[0][1],a.m[0][2]+b.m[0][2],a.m[0][3]+b.m[0][3],
                a.m[1][0]+b.m[1][0],a.m[1][1]+b.m[1][1],a.m[1][2]+b.m[1][2],a.m[1][3]+b.m[1][3],
                a.m[2][0]+b.m[2][0],a.m[2][1]+b.m[2][1],a.m[2][2]+b.m[2][2],a.m[2][3]+b.m[2][3],
                a.m[3][0]+b.m[3][0],a.m[3][1]+b.m[3][1],a.m[3][2]+b.m[3][2],a.m[3][3]+b.m[3][3]);
  }


// Substract matrices
FXHMat operator-(const FXHMat& a,const FXHMat& b){
  return FXHMat(a.m[0][0]-b.m[0][0],a.m[0][1]-b.m[0][1],a.m[0][2]-b.m[0][2],a.m[0][3]-b.m[0][3],
                a.m[1][0]-b.m[1][0],a.m[1][1]-b.m[1][1],a.m[1][2]-b.m[1][2],a.m[1][3]-b.m[1][3],
                a.m[2][0]-b.m[2][0],a.m[2][1]-b.m[2][1],a.m[2][2]-b.m[2][2],a.m[2][3]-b.m[2][3],
                a.m[3][0]-b.m[3][0],a.m[3][1]-b.m[3][1],a.m[3][2]-b.m[3][2],a.m[3][3]-b.m[3][3]);
  }


// Negate matrix
FXHMat operator-(const FXHMat& a){
  return FXHMat(-a.m[0][0],-a.m[0][1],-a.m[0][2],-a.m[0][3],
                -a.m[1][0],-a.m[1][1],-a.m[1][2],-a.m[1][3],
                -a.m[2][0],-a.m[2][1],-a.m[2][2],-a.m[2][3],
                -a.m[3][0],-a.m[3][1],-a.m[3][2],-a.m[3][3]);
  }



// Composite matrices
FXHMat operator*(const FXHMat& a,const FXHMat& b){
  FXHMat r;
  register FXfloat x,y,z,h;
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
FXHMat operator*(const FXfloat x,const FXHMat& a){
  return FXHMat(x*a.m[0][0],x*a.m[0][1],x*a.m[0][2],a.m[0][3],
                x*a.m[1][0],x*a.m[1][1],x*a.m[1][2],a.m[1][3],
                x*a.m[2][0],x*a.m[2][1],x*a.m[2][2],a.m[2][3],
                x*a.m[3][0],x*a.m[3][1],x*a.m[3][2],a.m[3][3]);
  }


// Multiply matrix by scalar
FXHMat operator*(const FXHMat& a,const FXfloat x){
  return FXHMat(a.m[0][0]*x,a.m[0][1]*x,a.m[0][2]*x,a.m[0][3],
                a.m[1][0]*x,a.m[1][1]*x,a.m[1][2]*x,a.m[1][3],
                a.m[2][0]*x,a.m[2][1]*x,a.m[2][2]*x,a.m[2][3],
                a.m[3][0]*x,a.m[3][1]*x,a.m[3][2]*x,a.m[3][3]);
  }


// Divide scalar by matrix
FXHMat operator/(const FXfloat x,const FXHMat& a){
  return FXHMat(x/a.m[0][0],x/a.m[0][1],x/a.m[0][2],a.m[0][3],
                x/a.m[1][0],x/a.m[1][1],x/a.m[1][2],a.m[1][3],
                x/a.m[2][0],x/a.m[2][1],x/a.m[2][2],a.m[2][3],
                x/a.m[3][0],x/a.m[3][1],x/a.m[3][2],a.m[3][3]);
  }


// Divide matrix by scalar
FXHMat operator/(const FXHMat& a,const FXfloat x){
  return FXHMat(a.m[0][0]/x,a.m[0][1]/x,a.m[0][2]/x,a.m[0][3],
                a.m[1][0]/x,a.m[1][1]/x,a.m[1][2]/x,a.m[1][3],
                a.m[2][0]/x,a.m[2][1]/x,a.m[2][2]/x,a.m[2][3],
                a.m[3][0]/x,a.m[3][1]/x,a.m[3][2]/x,a.m[3][3]);
  }


// Vector times matrix
FXHVec operator*(const FXHVec& v,const FXHMat& m){
  register FXfloat x=v[0],y=v[1],z=v[2],w=v[3];
  return FXHVec(x*m.m[0][0]+y*m.m[1][0]+z*m.m[2][0]+w*m.m[3][0],
                x*m.m[0][1]+y*m.m[1][1]+z*m.m[2][1]+w*m.m[3][1],
                x*m.m[0][2]+y*m.m[1][2]+z*m.m[2][2]+w*m.m[3][2],
                x*m.m[0][3]+y*m.m[1][3]+z*m.m[2][3]+w*m.m[3][3]);
  }


// Matrix times vector
FXHVec operator*(const FXHMat& m,const FXHVec& v){
  register FXfloat x=v[0],y=v[1],z=v[2],w=v[3];
  return FXHVec(x*m.m[0][0]+y*m.m[0][1]+z*m.m[0][2]+w*m.m[0][3],
                x*m.m[1][0]+y*m.m[1][1]+z*m.m[1][2]+w*m.m[1][3],
                x*m.m[2][0]+y*m.m[2][1]+z*m.m[2][2]+w*m.m[2][3],
                x*m.m[3][0]+y*m.m[3][1]+z*m.m[3][2]+w*m.m[3][3]);
  }


// Vector times matrix
FXVec operator*(const FXVec& v,const FXHMat& m){
  register FXfloat x=v[0],y=v[1],z=v[2];
  FXASSERT(m.m[0][3]==0.0 && m.m[1][3]==0.0 && m.m[2][3]==0.0 && m.m[3][3]==1.0);
  return FXVec(x*m.m[0][0]+y*m.m[1][0]+z*m.m[2][0]+m.m[3][0],
               x*m.m[0][1]+y*m.m[1][1]+z*m.m[2][1]+m.m[3][1],
               x*m.m[0][2]+y*m.m[1][2]+z*m.m[2][2]+m.m[3][2]);
  }


// Matrix times vector
FXVec operator*(const FXHMat& m,const FXVec& v){
  register FXfloat x=v[0],y=v[1],z=v[2];
  FXASSERT(m.m[0][3]==0.0 && m.m[1][3]==0.0 && m.m[2][3]==0.0 && m.m[3][3]==1.0);
  return FXVec(x*m.m[0][0]+y*m.m[0][1]+z*m.m[0][2]+m.m[0][3],
               x*m.m[1][0]+y*m.m[1][1]+z*m.m[1][2]+m.m[1][3],
               x*m.m[2][0]+y*m.m[2][1]+z*m.m[2][2]+m.m[2][3]);
  }


// Make unit matrix
FXHMat& FXHMat::eye(){
  m[0][0]=1.0; m[0][1]=0.0; m[0][2]=0.0; m[0][3]=0.0;
  m[1][0]=0.0; m[1][1]=1.0; m[1][2]=0.0; m[1][3]=0.0;
  m[2][0]=0.0; m[2][1]=0.0; m[2][2]=1.0; m[2][3]=0.0;
  m[3][0]=0.0; m[3][1]=0.0; m[3][2]=0.0; m[3][3]=1.0;
  return *this;
  }


// Make left hand matrix
FXHMat& FXHMat::left(){
  FXASSERT(m[0][3]==0.0 && m[1][3]==0.0 && m[2][3]==0.0 && m[3][3]==1.0);
  m[2][0]= -m[2][0];
  m[2][1]= -m[2][1];
  m[2][2]= -m[2][2];
  return *this;
  }



// Rotate using quaternion
FXHMat& FXHMat::rot(const FXQuat& q){
  register FXfloat r00,r01,r02,r10,r11,r12,r20,r21,r22;
  register FXfloat x2,y2,z2,xx2,yy2,zz2,xy2,xz2,yz2,wx2,wy2,wz2;
  register FXfloat x,y,z;
  FXASSERT(m[0][3]==0.0 && m[1][3]==0.0 && m[2][3]==0.0 && m[3][3]==1.0);

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
  x=m[0][0]; y=m[1][0]; z=m[2][0];
  m[0][0]=x*r00+y*r01+z*r02;
  m[1][0]=x*r10+y*r11+z*r12; 
  m[2][0]=x*r20+y*r21+z*r22;
  x=m[0][1]; y=m[1][1]; z=m[2][1];
  m[0][1]=x*r00+y*r01+z*r02; 
  m[1][1]=x*r10+y*r11+z*r12; 
  m[2][1]=x*r20+y*r21+z*r22;
  x=m[0][2]; y=m[1][2]; z=m[2][2];
  m[0][2]=x*r00+y*r01+z*r02; 
  m[1][2]=x*r10+y*r11+z*r12; 
  m[2][2]=x*r20+y*r21+z*r22;
  return *this;
  }


// Rotate by angle about arbitrary vector
FXHMat& FXHMat::rot(const FXVec& v,const FXfloat phi){
  FXASSERT((v*v-1.0)<0.000001);
  FXASSERT(m[0][3]==0.0 && m[1][3]==0.0 && m[2][3]==0.0 && m[3][3]==1.0);
  return rot(v,cos((double)phi),sin((double)phi));
  }


// Rotate about arbitrary vector
FXHMat& FXHMat::rot(const FXVec& v,const FXfloat c,const FXfloat s){
  register FXfloat r00,r01,r02,r10,r11,r12,r20,r21,r22;
  register FXfloat x,y,z,t;
  FXASSERT((v*v-1.0)<0.00001);
  FXASSERT(-1.00001<c && c<1.00001 && -1.00001<s && s<1.00001);
  FXASSERT(m[0][3]==0.0 && m[1][3]==0.0 && m[2][3]==0.0 && m[3][3]==1.0);

  // Rotation matrix
  t=1.0-c;
  r00=t*v[0]*v[0]+c; r01=t*v[0]*v[1]+s*v[2]; r02=t*v[0]*v[2]-s*v[1];
  r10=t*v[0]*v[1]-s*v[2]; r11=t*v[1]*v[1]+c; r12=t*v[1]*v[2]+s*v[0];
  r20=t*v[0]*v[2]+s*v[1]; r21=t*v[1]*v[2]-s*v[0]; r22=t*v[2]*v[2]+c;

  // Pre-multiply
  x=m[0][0]; y=m[1][0]; z=m[2][0];
  m[0][0]=x*r00+y*r01+z*r02;
  m[1][0]=x*r10+y*r11+z*r12; 
  m[2][0]=x*r20+y*r21+z*r22;
  x=m[0][1]; y=m[1][1]; z=m[2][1];
  m[0][1]=x*r00+y*r01+z*r02; 
  m[1][1]=x*r10+y*r11+z*r12; 
  m[2][1]=x*r20+y*r21+z*r22;
  x=m[0][2]; y=m[1][2]; z=m[2][2];
  return *this;
  }


// Rotate about x-axis
FXHMat& FXHMat::xrot(const FXfloat c,const FXfloat s){
  register FXfloat u,v;
  FXASSERT(m[0][3]==0.0 && m[1][3]==0.0 && m[2][3]==0.0 && m[3][3]==1.0);
  FXASSERT(-1.00001<c && c<1.00001 && -1.00001<s && s<1.00001);
  u=m[1][0]; v=m[2][0]; m[1][0]=c*u+s*v; m[2][0]=c*v-s*u;
  u=m[1][1]; v=m[2][1]; m[1][1]=c*u+s*v; m[2][1]=c*v-s*u;
  u=m[1][2]; v=m[2][2]; m[1][2]=c*u+s*v; m[2][2]=c*v-s*u;
  return *this;
  }


// Rotate by angle about x-axis
FXHMat& FXHMat::xrot(const FXfloat phi){ 
  register FXfloat c=cos(phi),s=sin(phi),u,v;
  FXASSERT(m[0][3]==0.0 && m[1][3]==0.0 && m[2][3]==0.0 && m[3][3]==1.0);
  u=m[1][0]; v=m[2][0]; m[1][0]=c*u+s*v; m[2][0]=c*v-s*u;
  u=m[1][1]; v=m[2][1]; m[1][1]=c*u+s*v; m[2][1]=c*v-s*u;
  u=m[1][2]; v=m[2][2]; m[1][2]=c*u+s*v; m[2][2]=c*v-s*u;
  return *this;
  }


// Rotate about y-axis
FXHMat& FXHMat::yrot(const FXfloat c,const FXfloat s){
  register FXfloat u,v;
  FXASSERT(m[0][3]==0.0 && m[1][3]==0.0 && m[2][3]==0.0 && m[3][3]==1.0);
  FXASSERT(-1.00001<c && c<1.00001 && -1.00001<s && s<1.00001);
  u=m[0][0]; v=m[2][0]; m[0][0]=c*u-s*v; m[2][0]=s*u+c*v;
  u=m[0][1]; v=m[2][1]; m[0][1]=c*u-s*v; m[2][1]=s*u+c*v;
  u=m[0][2]; v=m[2][2]; m[0][2]=c*u-s*v; m[2][2]=s*u+c*v;
  return *this;
  }


// Rotate by angle about y-axis
FXHMat& FXHMat::yrot(const FXfloat phi){
  register FXfloat c=cos(phi),s=sin(phi),u,v;
  FXASSERT(m[0][3]==0.0 && m[1][3]==0.0 && m[2][3]==0.0 && m[3][3]==1.0);
  u=m[0][0]; v=m[2][0]; m[0][0]=c*u-s*v; m[2][0]=s*u+c*v;
  u=m[0][1]; v=m[2][1]; m[0][1]=c*u-s*v; m[2][1]=s*u+c*v;
  u=m[0][2]; v=m[2][2]; m[0][2]=c*u-s*v; m[2][2]=s*u+c*v;
  return *this;
  }


// Rotate about z-axis
FXHMat& FXHMat::zrot(const FXfloat c,const FXfloat s){
  register FXfloat u,v;
  FXASSERT(m[0][3]==0.0 && m[1][3]==0.0 && m[2][3]==0.0 && m[3][3]==1.0);
  FXASSERT(-1.00001<c && c<1.00001 && -1.00001<s && s<1.00001);
  u=m[0][0]; v=m[1][0]; m[0][0]=c*u+s*v; m[1][0]=c*v-s*u;
  u=m[0][1]; v=m[1][1]; m[0][1]=c*u+s*v; m[1][1]=c*v-s*u;
  u=m[0][2]; v=m[1][2]; m[0][2]=c*u+s*v; m[1][2]=c*v-s*u;
  return *this;
  }


// Rotate by angle about z-axis
FXHMat& FXHMat::zrot(const FXfloat phi){
  register FXfloat c=cos(phi),s=sin(phi),u,v;
  FXASSERT(m[0][3]==0.0 && m[1][3]==0.0 && m[2][3]==0.0 && m[3][3]==1.0);
  u=m[0][0]; v=m[1][0]; m[0][0]=c*u+s*v; m[1][0]=c*v-s*u;
  u=m[0][1]; v=m[1][1]; m[0][1]=c*u+s*v; m[1][1]=c*v-s*u;
  u=m[0][2]; v=m[1][2]; m[0][2]=c*u+s*v; m[1][2]=c*v-s*u;
  return *this;
  }


// Translate
FXHMat& FXHMat::trans(const FXfloat tx,const FXfloat ty,const FXfloat tz){
  FXASSERT(m[0][3]==0.0 && m[1][3]==0.0 && m[2][3]==0.0 && m[3][3]==1.0);
  m[3][0]=m[3][0]+tx*m[0][0]+ty*m[1][0]+tz*m[2][0];
  m[3][1]=m[3][1]+tx*m[0][1]+ty*m[1][1]+tz*m[2][1];
  m[3][2]=m[3][2]+tx*m[0][2]+ty*m[1][2]+tz*m[2][2];
  return *this;
  }


// Translate over vector
FXHMat& FXHMat::trans(const FXVec& v){
  FXASSERT(m[0][3]==0.0 && m[1][3]==0.0 && m[2][3]==0.0 && m[3][3]==1.0);
  m[3][0]=m[3][0]+v[0]*m[0][0]+v[1]*m[1][0]+v[2]*m[2][0];
  m[3][1]=m[3][1]+v[0]*m[0][1]+v[1]*m[1][1]+v[2]*m[2][1];
  m[3][2]=m[3][2]+v[0]*m[0][2]+v[1]*m[1][2]+v[2]*m[2][2];
  return *this;
  }


// Scale unqual
FXHMat& FXHMat::scale(const FXfloat sx,const FXfloat sy,const FXfloat sz){
  FXASSERT(m[0][3]==0.0 && m[1][3]==0.0 && m[2][3]==0.0 && m[3][3]==1.0);
  m[0][0]*=sx; m[0][1]*=sx; m[0][2]*=sx;
  m[1][0]*=sy; m[1][1]*=sy; m[1][2]*=sy;
  m[2][0]*=sz; m[2][1]*=sz; m[2][2]*=sz;
  return *this;
  }


// Scale uniform
FXHMat& FXHMat::scale(const FXfloat s){
  FXASSERT(m[0][3]==0.0 && m[1][3]==0.0 && m[2][3]==0.0 && m[3][3]==1.0);
  m[0][0]*=s; m[0][1]*=s; m[0][2]*=s;
  m[1][0]*=s; m[1][1]*=s; m[1][2]*=s;
  m[2][0]*=s; m[2][1]*=s; m[2][2]*=s;
  return *this;
  }


// Scale matrix
FXHMat& FXHMat::scale(const FXVec& v){
  FXASSERT(m[0][3]==0.0 && m[1][3]==0.0 && m[2][3]==0.0 && m[3][3]==1.0);
  m[0][0]*=v[0]; m[0][1]*=v[0]; m[0][2]*=v[0];
  m[1][0]*=v[1]; m[1][1]*=v[1]; m[1][2]*=v[1];
  m[2][0]*=v[2]; m[2][1]*=v[2]; m[2][2]*=v[2];
  return *this;
  }


// Calculate determinant (not so good yet...)
FXfloat det(const FXHMat& a){
  return DET4(a.m[0][0],a.m[0][1],a.m[0][2],a.m[0][3],
              a.m[1][0],a.m[1][1],a.m[1][2],a.m[1][3],
              a.m[2][0],a.m[2][1],a.m[2][2],a.m[2][3],
              a.m[3][0],a.m[3][1],a.m[3][2],a.m[3][3]);
  }


// Transpose matrix
FXHMat transpose(const FXHMat& m){
  return FXHMat(m.m[0][0],m.m[1][0],m.m[2][0],m.m[3][0],
                m.m[0][1],m.m[1][1],m.m[2][1],m.m[3][1],
                m.m[0][2],m.m[1][2],m.m[2][2],m.m[3][2],
                m.m[0][3],m.m[1][3],m.m[2][3],m.m[3][3]);
  }


// Invert matrix
FXHMat invert(const FXHMat& s){
  FXHMat m(1.0, 0.0, 0.0, 0.0,
           0.0, 1.0, 0.0, 0.0,
           0.0, 0.0, 1.0, 0.0,
           0.0, 0.0, 0.0, 1.0);
  FXHMat x(s);
  register FXfloat pvv,t;
  register int i,j,pvi;
  for(i=0; i<4; i++){
    pvv=x[i][i];
    pvi=i;
    for(j=i+1; j<4; j++){   /* Find pivot (largest in column i) */
      if(fabs(x[j][i])>fabs(pvv)){
        pvi=j;
        pvv=x[j][i];
        }
      }
    FXASSERT(pvv != 0.0);                                 /* Should not be singular */
    if(pvi!=i){       /* Swap rows i and pvi */
      SWAP(m[i][0],m[pvi][0],t); SWAP(m[i][1],m[pvi][1],t);
      SWAP(m[i][2],m[pvi][2],t); SWAP(m[i][3],m[pvi][3],t);
      SWAP(x[i][0],x[pvi][0],t); SWAP(x[i][1],x[pvi][1],t);
      SWAP(x[i][2],x[pvi][2],t); SWAP(x[i][3],x[pvi][3],t);
      }
    x[i][0]/=pvv; x[i][1]/=pvv;   /* Normalize row i */
    x[i][2]/=pvv; x[i][3]/=pvv;
    m[i][0]/=pvv; m[i][1]/=pvv;
    m[i][2]/=pvv; m[i][3]/=pvv;
    for(j=0; j<4; j++){     /* Eliminate column i */
      if(j!=i){
        t=x[j][i];
        x[j][0]-=x[i][0]*t; x[j][1]-=x[i][1]*t;
        x[j][2]-=x[i][2]*t; x[j][3]-=x[i][3]*t;
        m[j][0]-=m[i][0]*t; m[j][1]-=m[i][1]*t;
        m[j][2]-=m[i][2]*t; m[j][3]-=m[i][3]*t;
        }
      }
    }
  return m;
  }


// Look at
FXHMat& FXHMat::look(FXVec& eye,FXVec& cntr,FXVec& vup){
  register FXfloat x0,x1,x2,tx,ty,tz; 
  FXVec rz,rx,ry;
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
FXStream& operator<<(FXStream& store,const FXHMat& m){
  store << m[0] << m[1] << m[2] << m[3];
  return store;
  }


// Load from archive
FXStream& operator>>(FXStream& store,FXHMat& m){
  store >> m[0] >> m[1] >> m[2] >> m[3];
  return store;
  }
 
