/********************************************************************************
*                                                                               *
*                           O p e n G L   O b j e c t                           *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998 by Jeroen van der Zijp.   All Rights Reserved.             *
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
* $Id: FXGLObject.cpp,v 1.28 1998/10/28 05:14:11 jeroen Exp $                   *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXVec.h"
#include "FXHVec.h"
#include "FXQuat.h"
#include "FXHMat.h"
#include "FXRange.h"
#include "FXString.h"
#include "FXObject.h"
#include "FXAccelTable.h"
#include "FXObjectList.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXDrawable.h"
#include "FXWindow.h"
#include "FXCursor.h"
#include "FXGLCanvas.h"
#include "FXGLViewer.h"
#include "FXGLObject.h"
#include "FXComposite.h"
#include "FXShell.h"
#include "FXTooltip.h"


/*
  Notes:
  - Leaf objects don't push any names!
  - Group objects should do focus traversal.
  - Instead of returning TRUE/FALSE in select() etc, why not just call
    viewer->update(..)???
*/

/*******************************************************************************/


// Object implementation
FXIMPLEMENT(FXGLObject,FXObject,NULL,0)

  
// Construct
FXGLObject::FXGLObject(){
  }


// Get bounding box
void FXGLObject::bounds(FXRange& box){
  box[0][0]=box[0][1]=box[1][0]=box[1][1]=box[2][0]=box[2][1]=0.0;
  }


// Draw the GL scene
void FXGLObject::draw(FXGLViewer*){ }


// Hit objects
void FXGLObject::hit(FXGLViewer* viewer){ draw(viewer); }


// Identify object by its path
FXGLObject* FXGLObject::identify(FXuint*){ return this; }


// Return true if it can be dragged
FXbool FXGLObject::canDrag() const { return FALSE; }


// Drag the object
FXbool FXGLObject::drag(FXGLViewer*,FXint,FXint,FXint,FXint){ return FALSE; }


// Select object in certain viewer
FXbool FXGLObject::select(FXGLViewer*){ return TRUE; }


// Deselect object in certain viewer
FXbool FXGLObject::deselect(FXGLViewer*){ return TRUE; }


// Return stamp
FXuint FXGLObject::stamp(FXGLViewer*) const { return 0; }


// Destruct
FXGLObject::~FXGLObject(){ }


/*******************************************************************************/

// Drop
FXDEFMAP(FXGLGroup) FXGLGroupMap[]={
  FXMAPFUNC(SEL_FOCUSIN,0,FXGLGroup::onFocusIn),
  FXMAPFUNC(SEL_FOCUSOUT,0,FXGLGroup::onFocusOut),
  };

  
// Object implementation
FXIMPLEMENT(FXGLGroup,FXGLObject,FXGLGroupMap,ARRAYNUMBER(FXGLGroupMap))



// Contruct
FXGLGroup::FXGLGroup(){
  current=-1;
  }


// Get bounding box
void FXGLGroup::bounds(FXRange& box){
  if(list.no()==0){
    box[0][0]=box[0][1]=box[1][0]=box[1][1]=box[2][0]=box[2][1]=0.0;
    }
  else{
    box=FXRange(FLT_MAX,-FLT_MAX,FLT_MAX,-FLT_MAX,FLT_MAX,-FLT_MAX);
    for(FXuint i=0; i<list.no(); i++){
      FXRange r;
      list[i]->bounds(r);
      box.include(r);
      }
    }
  }


// Gained focus
long FXGLGroup::onFocusIn(FXObject*,FXSelector,void* ptr){
//fprintf(stderr,"%s::onFocusIn %08x\n",getClassName(),this);
  return (0<=current)&&(list[current]->handle(list[current],MKUINT(0,SEL_FOCUSIN),ptr));
  }


// Lost focus
long FXGLGroup::onFocusOut(FXObject*,FXSelector,void* ptr){
//fprintf(stderr,"%s::onFocusOut %08x\n",getClassName(),this);
  return (0<=current)&&(list[current]->handle(list[current],MKUINT(0,SEL_FOCUSOUT),ptr));
  }


// Draw
void FXGLGroup::draw(FXGLViewer* viewer){
  for(FXuint i=0; i<list.no(); i++) list[i]->draw(viewer);
  }


// Draw for hit
void FXGLGroup::hit(FXGLViewer* viewer){
#ifdef HAVE_OPENGL
  glPushName(0xffffffff);
  for(FXuint i=0; i<list.no(); i++){
    glLoadName(i);
    list[i]->hit(viewer);
    }
  glPopName();
#endif
  }


// Identify object by its path
FXGLObject* FXGLGroup::identify(FXuint* path){ 
  FXASSERT(path);
  FXASSERT(path[0]<list.no());
  return list[path[0]]->identify(path+1);
  }


// Return true if it can be dragged
FXbool FXGLGroup::canDrag() const { return TRUE; }


// Drag group object
FXbool FXGLGroup::drag(FXGLViewer* viewer,FXint fx,FXint fy,FXint tx,FXint ty){
  for(FXuint i=0; i<list.no(); i++){
    list[i]->drag(viewer,fx,fy,tx,ty);
    }
  return TRUE;
  }


// Save object to stream
void FXGLGroup::save(FXStream& store) const {
  FXGLObject::save(store);
  list.save(store);
  store << current;
  }

      
// Load object from stream
void FXGLGroup::load(FXStream& store){
  FXGLObject::load(store);
  list.load(store);
  store >> current;
  }


// Delete members of the group
FXGLGroup::~FXGLGroup(){
  for(FXuint i=0; i<list.no(); i++) delete list[i];
  }


/*******************************************************************************/

#define HANDLE_SIZE 4.0


// Object implementation
FXIMPLEMENT(FXGLPoint,FXGLObject,NULL,0)


// Create point
FXGLPoint::FXGLPoint():
  pos(0.0,0.0,0.0){
  }


// Create initialized point
FXGLPoint::FXGLPoint(FXfloat x,FXfloat y,FXfloat z):
  pos(x,y,z){
  }


// Get bounding box
void FXGLPoint::bounds(FXRange& box){
  box[0][0]=box[0][1]=pos[0];
  box[1][0]=box[1][1]=pos[0];
  box[2][0]=box[2][1]=pos[0];
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
FXGLLine::FXGLLine():fm(-0.5,0.0,0.0),to(0.5,0.0,0.0){
  }


// Create inittialized line
FXGLLine::FXGLLine(FXfloat fx,FXfloat fy,FXfloat fz,FXfloat tx,FXfloat ty,FXfloat tz):
  fm(fx,fy,fz),to(tx,ty,tz){
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


/*******************************************************************************/

// Drop
FXDEFMAP(FXGLCube) FXGLCubeMap[]={
  FXMAPFUNC(SEL_DND_DROP,0,FXGLCube::onDNDDrop),
  FXMAPFUNC(SEL_DND_MOTION,0,FXGLCube::onDNDMotion),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_TIP,FXGLCube::onQueryTip),
  FXMAPFUNC(SEL_COMMAND,FXGLCube::ID_SHADEOFF,FXGLCube::onCmdShadeOff),
  FXMAPFUNC(SEL_COMMAND,FXGLCube::ID_SHADEON,FXGLCube::onCmdShadeOn),
  FXMAPFUNC(SEL_COMMAND,FXGLCube::ID_SHADESMOOTH,FXGLCube::onCmdShadeSmooth),
  FXMAPFUNC(SEL_UPDATE,FXGLCube::ID_SHADEOFF,FXGLCube::onUpdShadeOff),
  FXMAPFUNC(SEL_UPDATE,FXGLCube::ID_SHADEON,FXGLCube::onUpdShadeOn),
  FXMAPFUNC(SEL_UPDATE,FXGLCube::ID_SHADESMOOTH,FXGLCube::onUpdShadeSmooth),
  };

  
// Object implementation
FXIMPLEMENT(FXGLCube,FXGLObject,FXGLCubeMap,ARRAYNUMBER(FXGLCubeMap))


// Create cube
FXGLCube::FXGLCube(){
  xmin=ymin=zmin=-0.5;
  xmax=ymax=zmax=0.5;
  color=FXVec(0.0,0.0,1.0);
  shading=2;
  }


// Create inittialized line
FXGLCube::FXGLCube(FXfloat xl,FXfloat xh,FXfloat yl,FXfloat yh,FXfloat zl,FXfloat zh){
  xmin=xl;
  xmax=xh;
  ymin=yl;
  ymax=yh;
  zmin=zl;
  zmax=zh;
  color=FXVec(0.0,0.0,1.0);
  shading=2;
  }


// Get bounding box
void FXGLCube::bounds(FXRange& box){
  box[0][0]=xmin; box[0][1]=xmax;
  box[1][0]=ymin; box[1][1]=ymax;
  box[2][0]=zmin; box[2][1]=zmax;
  }


// Handle drag-and-drop drop
long FXGLCube::onDNDDrop(FXObject* sender,FXSelector,void*){
  FXuchar *data; FXuint len,r,g,b,a;
  if(((FXWindow*)sender)->offeredDNDType(FXGLViewer::dragTypeColor)){
    if(((FXWindow*)sender)->getDNDData(FXGLViewer::dragTypeColor,data,len)){
      sscanf((char*)data,"#%02x%02x%02x%02x",&r,&g,&b,&a);
      color[0]=r*0.0039215686;
      color[1]=g*0.0039215686;
      color[2]=b*0.0039215686;
      FXFREE(&data);
      return 1;
      }
    }
  return 0;
  }


// Cursor got dragged over here.
long FXGLCube::onDNDMotion(FXObject*,FXSelector,void*){
  return 1;
  }


// We were asked about tip text
long FXGLCube::onQueryTip(FXObject* sender,FXSelector,void*){
  char buf[100];
  sprintf(buf,"Cube color (%g,%g,%g)",color[0],color[1],color[2]);
  ((FXTooltip*)sender)->setText(buf);
  return 1;
  }


long FXGLCube::onCmdShadeOff(FXObject*,FXSelector,void*){
  shading=0; // How to update the views??
  return 1;
  }

long FXGLCube::onCmdShadeOn(FXObject*,FXSelector,void*){
  shading=1;
  return 1;
  }

long FXGLCube::onCmdShadeSmooth(FXObject*,FXSelector,void*){
  shading=2;
  return 1;
  }

long FXGLCube::onUpdShadeOff(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=shading==0 ? FXWindow::ID_CHECK : FXWindow::ID_UNCHECK;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }

long FXGLCube::onUpdShadeOn(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=shading==1 ? FXWindow::ID_CHECK : FXWindow::ID_UNCHECK;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }

long FXGLCube::onUpdShadeSmooth(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=shading==2 ? FXWindow::ID_CHECK : FXWindow::ID_UNCHECK;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }


// Draw
void FXGLCube::draw(FXGLViewer* viewer){
#ifdef HAVE_OPENGL
  glPushAttrib(GL_ENABLE_BIT);
  if(shading){
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    }
  
  if(viewer->getSelection()==this) 
    glColor3f(1.0,0.0,0.0); 
  else
    glColor3fv(color); 
  
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

  glPopAttrib();
#endif
  }


// Draw for hit
void FXGLCube::hit(FXGLViewer* viewer){
  draw(viewer);
  }


// Return true if it can be dragged
FXbool FXGLCube::canDrag() const { return TRUE; }


// Drag cube around
FXbool FXGLCube::drag(FXGLViewer* viewer,FXint fx,FXint fy,FXint tx,FXint ty){
  FXVec c(0.5*(xmin+xmax),0.5*(ymin+ymax),0.5*(zmin+zmax));
  FXfloat zz=viewer->worldToEyeZ(c);
  FXVec wf=viewer->eyeToWorld(viewer->screenToEye(fx,fy,zz));
  FXVec wt=viewer->eyeToWorld(viewer->screenToEye(tx,ty,zz));
//fprintf(stderr,"eyez = %g distance=%g\n",zz,viewer->getDistance());
  xmin+=wt[0]-wf[0];
  xmax+=wt[0]-wf[0];
  ymin+=wt[1]-wf[1];
  ymax+=wt[1]-wf[1];
  zmin+=wt[2]-wf[2];
  zmax+=wt[2]-wf[2];
  return TRUE;
//           FXVec ea,eb,wa,wb,d;
//           ea=screenToTarget(event->win_x,event->win_y);
//           eb=screenToTarget(event->last_x,event->last_y);
//           wa=eyeToWorld(ea);
//           wb=eyeToWorld(eb);
//           d=wa-wb;
//           selection->drag(this,d);
//           update(0,0,width,height);
  }


// Save object to stream
void FXGLCube::save(FXStream& store) const {
  FXGLObject::save(store);
  store << xmin << xmax << ymin << ymax << zmin << zmax;
  store << color;
  store << shading;
  }

      
// Load object from stream
void FXGLCube::load(FXStream& store){
  FXGLObject::load(store);
  store >> xmin >> xmax >> ymin >> ymax >> zmin >> zmax;
  store >> color;
  store >> shading;
  }
