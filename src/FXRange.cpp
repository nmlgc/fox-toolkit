/********************************************************************************
*                                                                               *
*                             R a n g e    C l a s s                            *
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
* $Id: FXRange.cpp,v 1.7 2002/01/18 22:43:02 jeroen Exp $                       *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXVec.h"
#include "FXRange.h"



/**************************  R a n g e   C l a s s   *************************/


// Test if empty
FXbool FXRange::empty() const {
  return d[0][1]<=d[0][0] || d[1][1]<=d[1][0] || d[2][1]<=d[2][0];
  }

// Test if overlap
FXbool FXRange::overlap(const FXRange& box) const {
  return d[0][1]>box.d[0][0] && d[0][0]<box.d[0][1] &&
         d[1][1]>box.d[1][0] && d[1][0]<box.d[1][1] &&
         d[2][1]>box.d[2][0] && d[2][0]<box.d[2][1];
  }


// Test if box contains point i,j,k
FXbool FXRange::contains(FXfloat x,FXfloat y,FXfloat z) const {
  return d[0][0]<=x && x<=d[0][1] && d[1][0]<=y && y<=d[1][1] && d[2][0]<=z && z<=d[2][1];
  }


// Longest side
FXfloat FXRange::longest() const {
  register FXfloat l,len;
  len=d[0][1]-d[0][0];
  if((l=d[1][1]-d[1][0])>len) len=l;
  if((l=d[2][1]-d[2][0])>len) len=l;
  return len;
  }


// shortest side
FXfloat FXRange::shortest() const {
  register FXfloat l,len;
  len=d[0][1]-d[0][0];
  if((l=d[1][1]-d[1][0])<len) len=l;
  if((l=d[2][1]-d[2][0])<len) len=l;
  return len;
  }


// Include given box into box's range
FXRange& FXRange::include(const FXRange& box){
  if(box.d[0][0]<d[0][0]) d[0][0]=box.d[0][0];
  if(box.d[1][0]<d[1][0]) d[1][0]=box.d[1][0];
  if(box.d[2][0]<d[2][0]) d[2][0]=box.d[2][0];
  if(box.d[0][1]>d[0][1]) d[0][1]=box.d[0][1];
  if(box.d[1][1]>d[1][1]) d[1][1]=box.d[1][1];
  if(box.d[2][1]>d[2][1]) d[2][1]=box.d[2][1];
  return *this;
  }


// Include point into range
FXRange& FXRange::include(FXfloat x,FXfloat y,FXfloat z){
  if(x<d[0][0]) d[0][0]=x; if(x>d[0][1]) d[0][1]=x;
  if(y<d[1][0]) d[1][0]=y; if(y>d[1][1]) d[1][1]=y;
  if(z<d[2][0]) d[2][0]=z; if(z>d[2][1]) d[2][1]=z;
  return *this;
  }


// Include point into range
FXRange& FXRange::include(const FXVec& v){
  if(v[0]<d[0][0]) d[0][0]=v[0]; if(v[0]>d[0][1]) d[0][1]=v[0];
  if(v[1]<d[1][0]) d[1][0]=v[1]; if(v[1]>d[1][1]) d[1][1]=v[1];
  if(v[2]<d[2][0]) d[2][0]=v[2]; if(v[2]>d[2][1]) d[2][1]=v[2];
  return *this;
  }


// Clip domain against another
FXRange& FXRange::clipTo(const FXRange& box){
  if(d[0][0]<box.d[0][0]) d[0][0]=box.d[0][0];
  if(d[1][0]<box.d[1][0]) d[1][0]=box.d[1][0];
  if(d[2][0]<box.d[2][0]) d[2][0]=box.d[2][0];
  if(d[0][1]>box.d[0][1]) d[0][1]=box.d[0][1];
  if(d[1][1]>box.d[1][1]) d[1][1]=box.d[1][1];
  if(d[2][1]>box.d[2][1]) d[2][1]=box.d[2][1];
  return *this;
  }


// Get corners of box
void boxCorners(FXVec* points,const FXRange& box){
  points[0][0] = points[2][0] = points[4][0] = points[6][0] = box[0][0];
  points[1][0] = points[3][0] = points[5][0] = points[7][0] = box[0][1];
  points[0][1] = points[1][1] = points[4][1] = points[5][1] = box[1][0];
  points[2][1] = points[3][1] = points[6][1] = points[7][1] = box[1][1];
  points[0][2] = points[1][2] = points[2][2] = points[3][2] = box[2][0];
  points[4][2] = points[5][2] = points[6][2] = points[7][2] = box[2][1];
  }


// Ray intersection test
FXbool boxIntersect(const FXRange& box,const FXVec& u,const FXVec& v){
  FXfloat f,n,d,ni,fi,t;
  FXVec dir = v-u;
  int c;
  f = FLT_MAX;
  n = -FLT_MAX;
  for(c=0; c<3; c++){
    d = dir[c];
    if(d==0.0){
      if((box[c][1]<u[c])||(u[c]<box[c][0])) return FALSE;
      }
    else{
      ni = (box[c][0]-u[c])/d;
      fi = (box[c][1]-u[c])/d;
      if(ni>fi) FXSWAP(ni,fi,t);
      if(ni>n) n=ni;
      if(fi<f) f=fi;
      if(n>f) return FALSE;
      }
    }
  return TRUE;
  }


// Get center of box
FXVec boxCenter(const FXRange& box){
  return FXVec(0.5f*(box[0][0]+box[0][1]),0.5f*(box[1][0]+box[1][1]),0.5f*(box[2][0]+box[2][1]));
  }


// Get diagonal of box
FXfloat boxDiagonal(const FXRange& box){
  float dx=box.width();
  float dy=box.height();
  float dz=box.depth();
  return (FXfloat)sqrt(dx*dx+dy*dy+dz*dz);
  }


// Saving
FXStream& operator<<(FXStream& store,const FXRange& box){
  store << box.d[0][0] << box.d[0][1];
  store << box.d[1][0] << box.d[1][1];
  store << box.d[2][0] << box.d[2][1];
  return store;
  }


// Loading
FXStream& operator>>(FXStream& store,FXRange& box){
  store >> box.d[0][0] >> box.d[0][1];
  store >> box.d[1][0] >> box.d[1][1];
  store >> box.d[2][0] >> box.d[2][1];
  return store;
  }
