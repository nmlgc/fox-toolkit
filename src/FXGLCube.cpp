/********************************************************************************
*                                                                               *
*                      O p e n G L   C u b e   O b j e c t                      *
*                                                                               *
*********************************************************************************
* Copyright (C) 1999,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* Contributed by: Angel-Ventura Mendo Gomez <ventura@labri.u-bordeaux.fr>       *
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
* $Id: FXGLCube.cpp,v 1.11 2002/01/18 22:43:00 jeroen Exp $                     *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXVec.h"
#include "FXHVec.h"
#include "FXQuat.h"
#include "FXHMat.h"
#include "FXRange.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXRegistry.h"
#include "FXAccelTable.h"
#include "FXObjectList.h"
#include "FXApp.h"
#include "FXGLViewer.h"
#include "FXGLCube.h"


/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXGLCube,FXGLShape,NULL,0)


// Create cube
FXGLCube::FXGLCube():width(1.0),height(1.0),depth(1.0){
  FXTRACE((100,"FXGLCube::FXGLCube\n"));
  range[0][0]=-0.5f*width;  range[0][1]=0.5f*width;
  range[1][0]=-0.5f*height; range[1][1]=0.5f*height;
  range[2][0]=-0.5f*depth;  range[2][1]=0.5f*depth;
  }


// Create cube
FXGLCube::FXGLCube(FXfloat x,FXfloat y,FXfloat z,FXfloat w,FXfloat h,FXfloat d):
  FXGLShape(x,y,z,SHADING_SMOOTH|STYLE_SURFACE),width(w),height(h),depth(d){
  FXTRACE((100,"FXGLCube::FXGLCube\n"));
  range[0][0]=-0.5f*width;  range[0][1]=0.5f*width;
  range[1][0]=-0.5f*height; range[1][1]=0.5f*height;
  range[2][0]=-0.5f*depth;  range[2][1]=0.5f*depth;
  }


// Create initialized line
FXGLCube::FXGLCube(FXfloat x,FXfloat y,FXfloat z,FXfloat w,FXfloat h,FXfloat d,const FXMaterial& mtl):
  FXGLShape(x,y,z,SHADING_SMOOTH|STYLE_SURFACE,mtl,mtl),width(w),height(h),depth(d){
  FXTRACE((100,"FXGLCube::FXGLCube\n"));
  range[0][0]=-0.5f*width;  range[0][1]=0.5f*width;
  range[1][0]=-0.5f*height; range[1][1]=0.5f*height;
  range[2][0]=-0.5f*depth;  range[2][1]=0.5f*depth;
  }


// Copy constructor
FXGLCube::FXGLCube(const FXGLCube& orig):FXGLShape(orig){
  FXTRACE((100,"FXGLCube::FXGLCube\n"));
  width=orig.width;
  height=orig.height;
  depth=orig.depth;
  }


// Draw
void FXGLCube::drawshape(FXGLViewer*){
#ifdef HAVE_OPENGL
  FXfloat xmin =-0.5f*width;
  FXfloat xmax = 0.5f*width;
  FXfloat ymin =-0.5f*height;
  FXfloat ymax = 0.5f*height;
  FXfloat zmin =-0.5f*depth;
  FXfloat zmax = 0.5f*depth;

  glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(0.,0.,-1.);
    glVertex3f(xmin, ymin, zmin);
    glVertex3f(xmin, ymax, zmin);
    glVertex3f(xmax, ymin, zmin);
    glVertex3f(xmax, ymax, zmin);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(1.,0.,0.);
    glVertex3f(xmax, ymin, zmin);
    glVertex3f(xmax, ymax, zmin);
    glVertex3f(xmax, ymin, zmax);
    glVertex3f(xmax, ymax, zmax);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(0.,0.,1.);
    glVertex3f(xmax, ymin, zmax);
    glVertex3f(xmax, ymax, zmax);
    glVertex3f(xmin, ymin, zmax);
    glVertex3f(xmin, ymax, zmax);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(-1.,0.,0.);
    glVertex3f(xmin, ymin, zmax);
    glVertex3f(xmin, ymax, zmax);
    glVertex3f(xmin, ymin, zmin);
    glVertex3f(xmin, ymax, zmin);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(0.,1.,0.);
    glVertex3f(xmin, ymax, zmin);
    glVertex3f(xmin, ymax, zmax);
    glVertex3f(xmax, ymax, zmin);
    glVertex3f(xmax, ymax, zmax);
  glEnd();

  glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(0.,-1.,0.);
    glVertex3f(xmin, ymin, zmax);
    glVertex3f(xmin, ymin, zmin);
    glVertex3f(xmax, ymin, zmax);
    glVertex3f(xmax, ymin, zmin);
  glEnd();
#endif
  }


// Copy this object
FXGLObject* FXGLCube::copy(){
  return new FXGLCube(*this);
  }


// Save object to stream
void FXGLCube::save(FXStream& store) const {
  FXGLShape::save(store);
  store << width << height << depth;
  }


// Load object from stream
void FXGLCube::load(FXStream& store){
  FXGLShape::load(store);
  store >> width >> height >> depth;
  }


// Destroy
FXGLCube::~FXGLCube(){
  FXTRACE((100,"FXGLCube::~FXGLCube\n"));
  }
