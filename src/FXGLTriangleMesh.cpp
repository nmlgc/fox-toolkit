/********************************************************************************
*                                                                               *
*               O p e n G L   T r i a n g l e   M e s h   O b j e c t           *
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
* $Id: FXGLTriangleMesh.cpp,v 1.12 2002/01/18 22:43:00 jeroen Exp $             *
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
#include "FXGLTriangleMesh.h"



/*
  Notes:

  - This is actually not a mesh at all but an just a bunch
    of individual triangles.

  - To do: make this into a true mesh with shared vertices,
    so we can actually see some speedup from using the vertex-
    array primitives.

  - Once we have a true mesh, revisit generatenormals() and make
    it spit out smooth normals based on mesh connectivity.
*/


/*******************************************************************************/


// Object implementation
FXIMPLEMENT(FXGLTriangleMesh,FXGLShape,NULL,0)


// Create cube
FXGLTriangleMesh::FXGLTriangleMesh():
  vertexBuffer(NULL),colorBuffer(NULL),normalBuffer(NULL),textureBuffer(NULL),vertexNumber(0){
  FXTRACE((100,"FXGLTriangleMesh::FXGLTriangleMesh\n"));
  range[0][0]=0.0; range[0][1]=0.0;
  range[1][0]=0.0; range[1][1]=0.0;
  range[2][0]=0.0; range[2][1]=0.0;
  }


// Create initialized tmesh
FXGLTriangleMesh::FXGLTriangleMesh(FXfloat x,FXfloat y,FXfloat z,FXint nv,FXfloat *v,FXfloat *n,FXfloat *c,FXfloat *t):
  FXGLShape(x,y,z,SHADING_SMOOTH|STYLE_SURFACE),
  vertexBuffer(v),colorBuffer(c),normalBuffer(n),textureBuffer(t),vertexNumber(nv){
  FXTRACE((100,"FXGLTriangleMesh::FXGLTriangleMesh\n"));
  if(!normalBuffer){ generatenormals(); }
  recomputerange();
  }


// Create initialized tmesh
FXGLTriangleMesh::FXGLTriangleMesh(FXfloat x,FXfloat y,FXfloat z,FXint nv,FXfloat *v,FXfloat *n,FXfloat *c,FXfloat *t,const FXMaterial& mtl):
  FXGLShape(x,y,z,SHADING_SMOOTH|STYLE_SURFACE,mtl,mtl),
  vertexBuffer(v),colorBuffer(c),normalBuffer(n),textureBuffer(t),vertexNumber(nv){
  FXTRACE((100,"FXGLTriangleMesh::FXGLTriangleMesh\n"));
  if(!normalBuffer){ generatenormals(); }
  recomputerange();
  }


// Copy constructor
FXGLTriangleMesh::FXGLTriangleMesh(const FXGLTriangleMesh& orig):FXGLShape(orig){
  FXTRACE((100,"FXGLTriangleMesh::FXGLTriangleMesh\n"));
  FXMEMDUP(&vertexBuffer,FXfloat,orig.vertexBuffer,3*orig.vertexNumber);
  FXMEMDUP(&colorBuffer,FXfloat,orig.colorBuffer,4*orig.vertexNumber);
  FXMEMDUP(&normalBuffer,FXfloat,orig.normalBuffer,3*orig.vertexNumber);
  FXMEMDUP(&textureBuffer,FXfloat,orig.textureBuffer,2*orig.vertexNumber);
  vertexNumber=orig.vertexNumber;
  }


// Change vertex buffer means recompute range
void FXGLTriangleMesh::setVertexBuffer(FXfloat *vertices){
  vertexBuffer=vertices;
  recomputerange();
  }


// Recompute range
void FXGLTriangleMesh::recomputerange(){
  register FXint i,n;
  register FXfloat t;
  range[0][0]=0.0; range[0][1]=0.0;
  range[1][0]=0.0; range[1][1]=0.0;
  range[2][0]=0.0; range[2][1]=0.0;
  if(vertexBuffer && vertexNumber>0){
    range[0][0]=range[0][1]=vertexBuffer[0];
    range[1][0]=range[1][1]=vertexBuffer[1];
    range[2][0]=range[2][1]=vertexBuffer[2];
    for(i=0,n=0; n<vertexNumber; n++){
      t=vertexBuffer[i++];
      if(t<range[0][0]) range[0][0]=t;
      if(t>range[0][1]) range[0][1]=t;
      t=vertexBuffer[i++];
      if(t<range[1][0]) range[1][0]=t;
      if(t>range[1][1]) range[1][1]=t;
      t=vertexBuffer[i++];
      if(t<range[2][0]) range[2][0]=t;
      if(t>range[2][1]) range[2][1]=t;
      }
    }
  }


// Draw
void FXGLTriangleMesh::drawshape(FXGLViewer*){
  if(!vertexBuffer || vertexNumber<=0) return;
#ifdef HAVE_OPENGL

#if !defined(GL_VERSION_1_1) && !defined(GL_VERSION_1_2)

  register FXint i;

  // Yes we know this is not efficient but hell, OpenGL 1.0 is old...
  glBegin(GL_TRIANGLES);
  for(i=0; i<vertexNumber; i++){
    if(textureBuffer) glTexCoord2fv(&textureBuffer[2*i]);
    if(normalBuffer) glNormal3fv(&normalBuffer[3*i]);
    if(colorBuffer) glColor4fv(&colorBuffer[4*i]);
    glVertex3fv(&vertexBuffer[3*i]);
    }
  glEnd();

#else

  // Setting buffers
  glVertexPointer(3,GL_FLOAT,0,vertexBuffer);
  glEnableClientState(GL_VERTEX_ARRAY);

  if(normalBuffer){
    glNormalPointer(GL_FLOAT,0,normalBuffer);
    glEnableClientState(GL_NORMAL_ARRAY);
    }
  else{
    glDisableClientState(GL_NORMAL_ARRAY);
    }

  if(colorBuffer){
    glColorPointer(4,GL_FLOAT,0,colorBuffer);
    glEnableClientState(GL_COLOR_ARRAY);
    }
  else{
    glDisableClientState(GL_COLOR_ARRAY);
    }

  if(textureBuffer){
    glTexCoordPointer(2,GL_FLOAT,0,textureBuffer);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
  else{
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

  // drawing
  glDrawArrays(GL_TRIANGLES,0,vertexNumber);

#endif
#endif
  }


// Routine to calculate FLAT normals at each triangle
void FXGLTriangleMesh::generatenormals(){
  register FXint i;
  FXASSERT(vertexBuffer);
  if(!normalBuffer) FXMALLOC(&normalBuffer,FXfloat,vertexNumber*3);

  for(i=0; i<vertexNumber*3; i+=9){
    FXVec a(vertexBuffer[i+0],
	    vertexBuffer[i+1],
	    vertexBuffer[i+2]);
    FXVec b(vertexBuffer[i+3],
	    vertexBuffer[i+4],
	    vertexBuffer[i+5]);
    FXVec c(vertexBuffer[i+6],
	    vertexBuffer[i+7],
	    vertexBuffer[i+8]);

    c-=b;
    b-=a;

    FXVec normal=b^c;

    normal=normalize(normal);

    normalBuffer[i+0]=normal[0];
    normalBuffer[i+1]=normal[1];
    normalBuffer[i+2]=normal[2];

    normalBuffer[i+3]=normal[0];
    normalBuffer[i+4]=normal[1];
    normalBuffer[i+5]=normal[2];

    normalBuffer[i+6]=normal[0];
    normalBuffer[i+7]=normal[1];
    normalBuffer[i+8]=normal[2];
    }
  }



// Copy this object
FXGLObject* FXGLTriangleMesh::copy(){
  return new FXGLTriangleMesh(*this);
  }


// Save object to stream
void FXGLTriangleMesh::save(FXStream& store) const {
  FXuchar hadvertices,hadcolors,hadnormals,hadtextures;
  FXGLShape::save(store);
  store << vertexNumber;
  hadvertices=(vertexBuffer!=NULL);
  hadcolors=(colorBuffer!=NULL);
  hadnormals=(normalBuffer!=NULL);
  hadtextures=(textureBuffer!=NULL);
  store << hadvertices << hadcolors << hadnormals << hadtextures;
  if(hadvertices){ store.save(vertexBuffer,vertexNumber); }
  if(hadcolors){ store.save(colorBuffer,vertexNumber); }
  if(hadnormals){ store.save(normalBuffer,vertexNumber); }
  if(hadtextures){ store.save(textureBuffer,vertexNumber); }
  }


// Load object from stream
void FXGLTriangleMesh::load(FXStream& store){
  FXuchar hadvertices,hadcolors,hadnormals,hadtextures;
  FXGLShape::load(store);
  store >> vertexNumber;
  store >> hadvertices >> hadcolors >> hadnormals >> hadtextures;
  if(hadvertices){ FXMALLOC(&vertexBuffer,FXfloat,3*vertexNumber); store.load(vertexBuffer,3*vertexNumber); }
  if(hadcolors){ FXMALLOC(&colorBuffer,FXfloat,4*vertexNumber); store.load(colorBuffer,4*vertexNumber); }
  if(hadnormals){ FXMALLOC(&normalBuffer,FXfloat,3*vertexNumber); store.load(normalBuffer,3*vertexNumber); }
  if(hadtextures){ FXMALLOC(&textureBuffer,FXfloat,2*vertexNumber); store.load(textureBuffer,2*vertexNumber); }
  }


// Zap object
FXGLTriangleMesh::~FXGLTriangleMesh(){
  FXTRACE((100,"FXGLTriangleMesh::~FXGLTriangleMesh\n"));
  FXFREE(&vertexBuffer);
  FXFREE(&colorBuffer);
  FXFREE(&normalBuffer);
  FXFREE(&textureBuffer);
  }
