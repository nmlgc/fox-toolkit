/********************************************************************************
*                                                                               *
*                           O p e n G L   O b j e c t                           *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXGLObject.cpp,v 1.18 2002/01/18 22:43:00 jeroen Exp $                   *
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
#include "FXGLObject.h"


// GLU versions prior to 1.1 have GLUquadric
#if !defined(GLU_VERSION_1_1) && !defined(GLU_VERSION_1_2) && !defined(GLU_VERSION_1_3)
#define GLUquadricObj GLUquadric
#endif

/*
  Notes:
  - Leaf objects don't push any names!
  - Group objects should do focus traversal.

  - Suggestion for transform nodes... "Mike Fletcher" <mcfletch@vrtelecom.com>
    class FXGLTransform( FXGLGroup ):

    def draw( self, viewer):
            # The transform node sets up a transformation matrix
            # then renders all children
            # then removes that transformation matrix
            # from the stack.
            GL.PushMatrix () # store previous
            if self.translation: # three tupple
                    apply (GL.glTranslatef, self.translation )
            if self.center: #three tupple
                    apply (GL.glTranslatef, self.center )
            # rotation of angle ra around axis rx,ry,rz
            if self.rotation: # 4 tupple
                    rx,ry,rz,ra = self.rotation
                    apply (GL.glRotatef, (saa*self.radtodeg, sax,say,saz) )
            if self.scale: # three tupple
                    if self.scaleOrientation: # 4 tupple
                            sax,say,saz,saa = self.scaleOrientation
                            apply (GL.glRotatef, (saa*self.radtodeg, sax,say,saz) )
                    apply (GL.glScalef, tuple( self.scale ) )
                    # reverse the scaleOrientation
                    if self.scaleOrientation:
                            apply (GL.glRotatef, (-saa*self.radtodeg, sax,say,saz) )
            # now reverse center offset
            if self.center:
                    cx,cy,cz = self.center
                    apply (GL.glTranslatef, ( -cx,-cy,-cz) )
            # draw children here...
            FXGLGroup.draw( self, viewer)
            # now remove the transformation matrix
            PopMatrix()

    Needs to do the same calculation for the pick pass as well...  Note that it
    is fairly easy to calculate the entire matrix at one go whenever it changes,
    thereby saving the system most of the work of calculation per-frame (a
    single matrix multiply with the current "surrounding" matrix
    GL.glGetFloatv(GL_MODELVIEW_MATRIX, ModelView) ).  I have not profiled the
    difference between them, but I would assume a cached matrix would be faster.


*/

/*******************************************************************************/


// Object implementation
FXIMPLEMENT(FXGLObject,FXObject,NULL,0)




// Get bounding box
void FXGLObject::bounds(FXRange& box){
  box[0][0]=box[0][1]=box[1][0]=box[1][1]=box[2][0]=box[2][1]=0.0;
  }


// Draw the GL scene
void FXGLObject::draw(FXGLViewer*){ }


// Hit objects
void FXGLObject::hit(FXGLViewer* viewer){ draw(viewer); }


// Copy
FXGLObject* FXGLObject::copy(){ return new FXGLObject(*this); }


// Identify object by its path
FXGLObject* FXGLObject::identify(FXuint*){ return this; }


// Return true if it can be dragged
FXbool FXGLObject::canDrag() const { return FALSE; }


// Return true if OK to delete object
FXbool FXGLObject::canDelete() const { return FALSE; }


// Drag the object
FXbool FXGLObject::drag(FXGLViewer*,FXint,FXint,FXint,FXint){ return FALSE; }



/*******************************************************************************/



// Object implementation
FXIMPLEMENT(FXGLGroup,FXGLObject,NULL,0)


// Get bounding box
void FXGLGroup::bounds(FXRange& box){
  if(list.no()==0){
    box[0][0]=box[0][1]=box[1][0]=box[1][1]=box[2][0]=box[2][1]=0.0;
    }
  else{
    box=FXRange(FLT_MAX,-FLT_MAX,FLT_MAX,-FLT_MAX,FLT_MAX,-FLT_MAX);
    for(FXint i=0; i<list.no(); i++){
      FXRange r;
      list[i]->bounds(r);
      box.include(r);
      }
    }
  }


// Draw
void FXGLGroup::draw(FXGLViewer* viewer){
  for(FXint i=0; i<list.no(); i++) list[i]->draw(viewer);
  }


// Draw for hit
void FXGLGroup::hit(FXGLViewer* viewer){
#ifdef HAVE_OPENGL
  glPushName(0xffffffff);
  for(FXint i=0; i<list.no(); i++){
    glLoadName(i);
    list[i]->hit(viewer);
    }
  glPopName();
#endif
  }


// Copy
FXGLObject* FXGLGroup::copy(){
  return new FXGLGroup(*this);
  }



// Identify object by its path
FXGLObject* FXGLGroup::identify(FXuint* path){
  FXASSERT(path);
  FXASSERT((FXint)path[0]<list.no());
  return list[path[0]]->identify(path+1);
  }


// Return true if it can be dragged
FXbool FXGLGroup::canDrag() const { return TRUE; }


// Drag group object
FXbool FXGLGroup::drag(FXGLViewer* viewer,FXint fx,FXint fy,FXint tx,FXint ty){
  for(FXint i=0; i<list.no(); i++){
    list[i]->drag(viewer,fx,fy,tx,ty);
    }
  return TRUE;
  }


// Save object to stream
void FXGLGroup::save(FXStream& store) const {
  FXGLObject::save(store);
  list.save(store);
  }


// Load object from stream
void FXGLGroup::load(FXStream& store){
  FXGLObject::load(store);
  list.load(store);
  }


// Delete members of the group
FXGLGroup::~FXGLGroup(){
  for(FXint i=0; i<list.no(); i++) delete list[i];
  }


/*******************************************************************************/

#define HANDLE_SIZE 4.0


// Object implementation
FXIMPLEMENT(FXGLPoint,FXGLObject,NULL,0)


// Create point
FXGLPoint::FXGLPoint():pos(0.0f,0.0f,0.0f){
  }

// Copy constructor
FXGLPoint::FXGLPoint(const FXGLPoint& orig):FXGLObject(orig),pos(orig.pos){
  }

// Create initialized point
FXGLPoint::FXGLPoint(FXfloat x,FXfloat y,FXfloat z):pos(x,y,z){
  }


// Get bounding box
void FXGLPoint::bounds(FXRange& box){
  box[0][0]=box[0][1]=pos[0];
  box[1][0]=box[1][1]=pos[1];
  box[2][0]=box[2][1]=pos[2];
  }


// Draw
void FXGLPoint::draw(FXGLViewer* ){
#ifdef HAVE_OPENGL
  glColor3f(0.0,0.0,1.0);
  glPointSize(HANDLE_SIZE);
  glBegin(GL_POINTS);
  glVertex3fv(pos);
  glEnd();
#endif
  }


// Draw for hit
void FXGLPoint::hit(FXGLViewer* ){
#ifdef HAVE_OPENGL
  glBegin(GL_POINTS);
  glVertex3fv(pos);
  glEnd();
#endif
  }


// Copy
FXGLObject* FXGLPoint::copy(){
  return new FXGLPoint(*this);
  }

// Save object to stream
void FXGLPoint::save(FXStream& store) const {
  FXGLObject::save(store);
  store << pos;
  }


// Load object from stream
void FXGLPoint::load(FXStream& store){
  FXGLObject::load(store);
  store >> pos;
  }


/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXGLLine,FXGLObject,NULL,0)


// Create line
FXGLLine::FXGLLine():fm(-0.5f,0.0f,0.0f),to(0.5f,0.0f,0.0f){
  }


// Copy constructor
FXGLLine::FXGLLine(const FXGLLine& orig):FXGLObject(orig),fm(orig.fm),to(orig.to){
  }


// Create inittialized line
FXGLLine::FXGLLine(FXfloat fx,FXfloat fy,FXfloat fz,FXfloat tx,FXfloat ty,FXfloat tz):fm(fx,fy,fz),to(tx,ty,tz){
  }


// Get bounding box
void FXGLLine::bounds(FXRange& box){
  FXMINMAX(box[0][0],box[0][1],fm.pos[0],to.pos[0]);
  FXMINMAX(box[1][0],box[1][1],fm.pos[1],to.pos[1]);
  FXMINMAX(box[2][0],box[2][1],fm.pos[2],to.pos[2]);
  }


// Draw
void FXGLLine::draw(FXGLViewer* ){
#ifdef HAVE_OPENGL
  glColor3f(1.0,0.0,0.0);
  glPointSize(HANDLE_SIZE);
  glBegin(GL_LINES);
  glVertex3fv(fm.pos);
  glVertex3fv(to.pos);
  glEnd();
#endif
  }


// Draw for hit
void FXGLLine::hit(FXGLViewer* ){
#ifdef HAVE_OPENGL
  glBegin(GL_LINES);
  glVertex3fv(fm.pos);
  glVertex3fv(to.pos);
  glEnd();
#endif
  }


// Copy
FXGLObject* FXGLLine::copy(){
  return new FXGLLine(*this);
  }


// Save object to stream
void FXGLLine::save(FXStream& store) const {
  FXGLObject::save(store);
  fm.save(store);
  to.save(store);
  }


// Load object from stream
void FXGLLine::load(FXStream& store){
  FXGLObject::load(store);
  fm.load(store);
  to.load(store);
  }

