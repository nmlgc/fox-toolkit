/********************************************************************************
*                                                                               *
*                           O p e n G L   V i e w e r                           *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997 by Jeroen van der Zijp.   All Rights Reserved.             *
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
* $Id: FXGLViewer.cpp,v 1.67 1998/10/30 04:49:08 jeroen Exp $                   *
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
#include "FXFrame.h"
#include "FXComposite.h"
#include "FXStatusbar.h"
#include "FXShell.h"
#include "FXTooltip.h"
#include "FXLabel.h"
#include "FXColorWell.h"
#include "FXCursor.h"
#include "FXGLCanvas.h"
#include "FXGLViewer.h"
#include "FXGLObject.h"
#include "jitter.h"


/*
  To Do:
  - Initialize GL to fastest of everything for drawing lines.
  - Group object needs current element.
  - use app->dragDelta for motion tolerance
  - Default op to noop mode; all returns 0 in noop mode
  - GLuint unfortunately not always big enough to store a pointer...
  - The selection seems to get lost with multiple viewers into
    the same scene.  If you select a cube in one view, then select another
    cube in another view, both seem to get selected.  Can we push the
    "selection" member from the view to the scene object?
  - Instead of select/deselect, do focus gain/lost type deal.
  - Add methods for inquire of pick-ray.
  - Fix FXGLGroup to identify child or itself..
  - Need some way of updating ALL viewers.
  - Need a document/view type concept?
  - Load/Save need to save more...
  - Build mini display lists for offset/surface drawing.
  - Pass clicked/double-clicked/triple-clicked messages to object.
  - Distinguish between current object and selected ones.
    only one is current, but many may be selected.
*/

/*******************************************************************************/


// Clamp value x to range [lo..hi]
#define CLAMP(lo,x,hi) ((x)<(lo)?(lo):((x)>(hi)?(hi):(x)))

// Size of pick buffer
#define MAX_PICKBUF    1024      

// Maximum length of selection path
#define MAX_SELPATH    64

// Rotation tolerance
#define EPS            1.0E-3

/*******************************************************************************/


// Registered GL Viewer drag types
FXDragType FXGLViewer::dragTypeColor=0;


// Map
FXDEFMAP(FXGLViewer) FXGLViewerMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXGLViewer::onPaint),
  FXMAPFUNC(SEL_UPDATE,0,FXGLViewer::onUpdate),
  FXMAPFUNC(SEL_TIMEOUT,FXGLViewer::ID_TIPTIMER,FXGLViewer::onTimeout),
  FXMAPFUNC(SEL_CONFIGURE,0,FXGLViewer::onConfigure),
  FXMAPFUNC(SEL_MOTION,0,FXGLViewer::onMotion),
  FXMAPFUNC(SEL_DND_ENTER,0,FXGLViewer::onDNDEnter),
  FXMAPFUNC(SEL_DND_LEAVE,0,FXGLViewer::onDNDLeave),
  FXMAPFUNC(SEL_DND_DROP,0,FXGLViewer::onDNDDrop),
  FXMAPFUNC(SEL_DND_MOTION,0,FXGLViewer::onDNDMotion),
  FXMAPFUNC(SEL_ENTER,0,FXGLViewer::onEnter),
  FXMAPFUNC(SEL_LEAVE,0,FXGLViewer::onLeave),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXGLViewer::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXGLViewer::onLeftBtnRelease),
  FXMAPFUNC(SEL_MIDDLEBUTTONPRESS,0,FXGLViewer::onMiddleBtnPress),
  FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,0,FXGLViewer::onMiddleBtnRelease),
  FXMAPFUNC(SEL_RIGHTBUTTONPRESS,0,FXGLViewer::onRightBtnPress),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,0,FXGLViewer::onRightBtnRelease),
  FXMAPFUNC(SEL_KEYPRESS,0,FXGLViewer::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXGLViewer::onKeyRelease),
  FXMAPFUNC(SEL_FOCUSIN,0,FXGLViewer::onFocusIn),
  FXMAPFUNC(SEL_FOCUSOUT,0,FXGLViewer::onFocusOut),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_TIP,FXGLViewer::onQueryTip),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_HELP,FXGLViewer::onQueryHelp),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_PERSPECTIVE,FXGLViewer::onUpdPerspective),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_PARALLEL,FXGLViewer::onUpdParallel),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_FRONT,FXGLViewer::onUpdFront),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_BACK,FXGLViewer::onUpdBack),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_LEFT,FXGLViewer::onUpdLeft),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_RIGHT,FXGLViewer::onUpdRight),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_TOP,FXGLViewer::onUpdTop),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_BOTTOM,FXGLViewer::onUpdBottom),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_RESETVIEW,FXWindow::onUpdYes),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_FITVIEW,FXWindow::onUpdYes),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_PERSPECTIVE,FXGLViewer::onCmdPerspective),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_PARALLEL,FXGLViewer::onCmdParallel),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_FRONT,FXGLViewer::onCmdFront),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_BACK,FXGLViewer::onCmdBack),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_LEFT,FXGLViewer::onCmdLeft),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_RIGHT,FXGLViewer::onCmdRight),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_TOP,FXGLViewer::onCmdTop),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_BOTTOM,FXGLViewer::onCmdBottom),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_RESETVIEW,FXGLViewer::onCmdResetView),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_FITVIEW,FXGLViewer::onCmdFitView),
  };


// Object implementation
FXIMPLEMENT(FXGLViewer,FXGLCanvas,FXGLViewerMap,ARRAYNUMBER(FXGLViewerMap))


// Make a canvas
FXGLViewer::FXGLViewer(FXComposite* p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXGLCanvas(p,tgt,sel,opts,x,y,w,h){
  projection=PERSPECTIVE;                       // Projection
  zoom=1.0;                                     // Zoom factor
  fov=30.0;                                     // Field of view (1 to 90)
  aspect=1.0;                                   // Aspect ratio
  wvt.left=-1.0;                                // Init world box
  wvt.right=1.0;
  wvt.top=1.0;
  wvt.bottom=-1.0;
  wvt.hither=0.1;
  wvt.yon=1.0;
  wvt.w=100;                                    // Viewport width
  wvt.h=100;                                    // Viewport height
  screenmin=100.0;                              // Screen minimum
  screenmax=100.0;                              // Screen maximum
  diameter=2.0;                                 // Size of model
  distance=7.464116;                            // Distance of PRP to target
  rotation=FXQuat(0.0,0.0,0.0,1.0);             // Orientation
  center=FXVec(0.0,0.0,0.0);                    // Model center
  scale=FXVec(1.0,1.0,1.0);                     // Model scaling
  updateProjection();                           // Initial projection
  updateTransform();                            // Set transformation
  op=PICKING;                                   // Mouse operation
  background=FXHVec(1.0,1.0,1.0,1.0);           // Background color
  ambient=FXHVec(0.2,0.2,0.2,1.0);              // Scene ambient
  
  light.ambient=FXHVec(0.0,0.0,0.0,1.0);        // Light setup
  light.diffuse=FXHVec(1.0,1.0,1.0,1.0);
  light.specular=FXHVec(1.0,1.0,1.0,1.0);
  light.position=FXHVec(-2.0,2.0,3.0,0.0);
  light.direction=FXVec(0.0,0.0,-1.0);
  light.exponent=0.0;
  light.cutoff=180.0;
  light.c_attn=1.0;
  light.l_attn=0.0;
  light.q_attn=0.0;
  
  material.ambient=FXHVec(0.2,0.2,0.2,1.0);     // Material setup
  material.diffuse=FXHVec(0.8,0.8,0.8,1.0);
  material.specular=FXHVec(0.0,0.0,0.0,1.0);
  material.emission=FXHVec(0.0,0.0,0.0,1.0);
  material.shininess=0.0;
  
  stamp=0;                                      // Stamp when last updated
  
  timer=NULL;                                   // Motion timer
  dropped=NULL;                                 // Nobody being dropped on
  selection=NULL;                               // No initial selection
  scene=NULL;                                   // Scene to look at
  }


// Create window
void FXGLViewer::create(){
  FXRange r(-1.0,1.0,-1.0,1.0,-1.0,1.0);
  FXGLCanvas::create();
  glsetup();
  
  // Register drag type for color
  if(!FXGLViewer::dragTypeColor){
    FXGLViewer::dragTypeColor=getApp()->registerDragType(FXColorWell::colorDragTypeName);
    }
  
  // Viewer accepts any drop type
  dropEnable();
  
  // If have scene already, get correct bounds
  if(scene) scene->bounds(r);
  
  // Set initial viewing volume
  setBounds(r);
  }


// Render all the graphics into a world box
void FXGLViewer::drawWorld(FXViewport& wv){
#ifdef HAVE_OPENGL

  // Set viewport
  glViewport(0,0,wv.w,wv.h);

  // Clear background
  glClearDepth(1.0);
  glClearColor(background[0],background[1],background[2],1.0);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  // Set Projection Matrix
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  switch(projection){
    case PARALLEL:    
      glOrtho(wv.left,wv.right,wv.bottom,wv.top,wv.hither,wv.yon); 
      break;
    case PERSPECTIVE: 
      glFrustum(wv.left,wv.right,wv.bottom,wv.top,wv.hither,wv.yon); 
      break;
    }
  
  // Place the light
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  // Set light parameters
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0,GL_AMBIENT,light.ambient);
  glLightfv(GL_LIGHT0,GL_DIFFUSE,light.diffuse);
  glLightfv(GL_LIGHT0,GL_SPECULAR,light.specular);
  glLightfv(GL_LIGHT0,GL_POSITION,light.position);
  glLightfv(GL_LIGHT0,GL_SPOT_DIRECTION,light.direction);
  glLightf(GL_LIGHT0,GL_SPOT_EXPONENT,light.exponent);
  glLightf(GL_LIGHT0,GL_SPOT_CUTOFF,light.cutoff);
  glLightf(GL_LIGHT0,GL_CONSTANT_ATTENUATION,light.c_attn);
  glLightf(GL_LIGHT0,GL_LINEAR_ATTENUATION,light.l_attn);
  glLightf(GL_LIGHT0,GL_QUADRATIC_ATTENUATION,light.q_attn);
  
  // Set model matrix
  glLoadMatrixf(transform);
 
  // Material colors
  glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,material.ambient);
  glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,material.diffuse);
  glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,material.specular);
  glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,material.emission);
  glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,material.shininess);
  
  // Color commands change both
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  
  // Track material
  glDisable(GL_COLOR_MATERIAL);

  // Global ambient
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ambient);

  // Enable lighting
  glDisable(GL_LIGHTING);
  
  // Shade model
  glShadeModel(GL_FLAT);
  
  // Draw what's visible
  if(scene) scene->draw(this);
#endif
  }


// Render with anti-aliasing
void FXGLViewer::drawAnti(FXViewport& wv){
#ifdef HAVE_OPENGL
  FXViewport jt=wv;
  FXdouble d=0.5*worldpx;
  register FXuint i;
  glClearAccum(0.0,0.0,0.0,0.0);
  glClear(GL_ACCUM_BUFFER_BIT);
  for(i=0; i<ARRAYNUMBER(jitter); i++){
    jt.left = wv.left+jitter[i][0]*d;
    jt.right = wv.right+jitter[i][0]*d;
    jt.top = wv.top+jitter[i][1]*d;
    jt.bottom = wv.bottom+jitter[i][1]*d;
    drawWorld(jt);
    glAccum(GL_ACCUM,1.0/ARRAYNUMBER(jitter));
    }
  glAccum(GL_RETURN,1.0);
#endif
  }


// Process picks
FXGLObject* FXGLViewer::processHits(FXuint *pickbuffer,FXint nhits){
  register FXuint d1,d2,n,zmin,zmax;
  register int i,sel;
  sel=0;
  for(i=0,zmin=zmax=4294967295U; nhits>0; i+=n+3,nhits--){
    n=pickbuffer[i];
    d1=pickbuffer[1+i];
    d2=pickbuffer[2+i];
    if(d1<zmin || (d1==zmin && d2<=zmax)){
      zmin=d1;
      zmax=d2;
      sel=i;
      }
    }
  FXASSERT(0<=sel);
  FXASSERT(sizeof(FXuint)==sizeof(GLuint));
  return scene->identify((FXuint*)&pickbuffer[4+sel]);
  }


// Test if something was hit
FXGLObject* FXGLViewer::pick(FXint x,FXint y,FXint dw,FXint dh){
#ifdef HAVE_OPENGL
  FXuint list[MAX_PICKBUF];
  register float pickx,picky,pickw,pickh;
  register int nhits;
  if(scene){
    makeCurrent();
    FXASSERT(sizeof(FXuint)==sizeof(GLuint));
    pickx=(wvt.w-2.0*x)/((float)dw);
    picky=(2.0*y-wvt.h)/((float)dh);
    pickw=wvt.w/((float)dw);
    pickh=wvt.h/((float)dh);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glTranslatef(pickx,picky,0.0);
    glScalef(pickw,pickh,1.0);
    switch(projection){
      case PARALLEL: glOrtho(wvt.left,wvt.right,wvt.bottom,wvt.top,wvt.hither,wvt.yon); break;
      case PERSPECTIVE: glFrustum(wvt.left,wvt.right,wvt.bottom,wvt.top,wvt.hither,wvt.yon); break;
      }
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(transform);
    FXASSERT(sizeof(FXuint)==sizeof(GLuint));
    glSelectBuffer(MAX_PICKBUF,(GLuint*)list);
    glRenderMode(GL_SELECT);
    glInitNames();
    glPushName(0);
    scene->hit(this);
    glPopName();
    nhits=glRenderMode(GL_RENDER);
    if(nhits<0){ fxwarning("%s::pick: hit buffer overflowed\n",getClassName()); } 
    if(nhits>0) return processHits(list,nhits);
    }
#endif
  return NULL;
  }


// Handle expose event; if we're double buffering and using MESA
// we just blit the exposed section again from the back buffer;
// If its a synthesized expose event, or we're using true OpenGL,
// then we perform the regular OpenGL drawing to regenerate the
// whole window (this could be expensive!)
long FXGLViewer::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  FXASSERT(xid);
  //fprintf(stderr,"ev: x=%d y=%d w=%d h=%d synth=%d\n",event->rect.x,event->rect.y,event->rect.w,event->rect.h,event->synthetic);
  //fprintf(stderr,"width=%d height=%d\n",width,height);
  //fprintf(stderr,"flags&FLAG_DIRTY = %08x\n",flags&FLAG_DIRTY);
  makeCurrent();
#ifdef MESA
  if((options&GL_DOUBLE_BUFFER) && !event->synthetic){
    swapSubBuffers(event->rect.x,event->rect.y,event->rect.w,event->rect.h);
    return 1;
    }
#endif
  drawWorld(wvt);
  if(options&GL_DOUBLE_BUFFER) swapBuffers();
  return 1;
  }


// Normal expose events will not generate any OpenGL commands
// for MESA; thus, we need to catch this and force a synthesized
// expose event when the window gets resized.  
long FXGLViewer::onConfigure(FXObject* sender,FXSelector sel,void* ptr){
//fprintf(stderr,"%s::onConfigure w%d h%d\n",getClassName(),((FXEvent*)ptr)->rect.w,((FXEvent*)ptr)->rect.h);
  FXGLCanvas::onConfigure(sender,sel,ptr);
  update(0,0,width,height);
  return 1;
  }


// Start motion timer while in this window
long FXGLViewer::onEnter(FXObject* sender,FXSelector sel,void* ptr){
  FXGLCanvas::onEnter(sender,sel,ptr);
  if(isEnabled()){
    if(!timer){timer=getApp()->addTimeout(getApp()->menuPause,this,ID_TIPTIMER);}
    }
  return 1;
  }


// Stop motion timer when leaving window
long FXGLViewer::onLeave(FXObject* sender,FXSelector sel,void* ptr){
  FXGLCanvas::onLeave(sender,sel,ptr);
  if(isEnabled()){
    if(timer){getApp()->removeTimeout(timer);timer=NULL;}
    }
  return 1;
  }


// Gained focus
long FXGLViewer::onFocusIn(FXObject* sender,FXSelector sel,void* ptr){
  FXGLCanvas::onFocusIn(sender,sel,ptr);
  if(selection && selection->handle(this,MKUINT(0,SEL_FOCUSIN),ptr)){
    update(0,0,width,height);
    }
  return 1;
  }

  
// Lost focus
long FXGLViewer::onFocusOut(FXObject* sender,FXSelector sel,void* ptr){
  FXGLCanvas::onFocusOut(sender,sel,ptr);
  if(selection && selection->handle(this,MKUINT(0,SEL_FOCUSOUT),ptr)){
    update(0,0,width,height);
    }
  return 1;
  }


// Handle configure notify
void FXGLViewer::layout(){
  wvt.w=width;
  wvt.h=height;
  if(wvt.w>0 && wvt.h>0){
    aspect = (double)wvt.h / (double)wvt.w;
    if(wvt.w>wvt.h){
      screenmax=wvt.w;
      screenmin=wvt.h;
      }
    else{
      screenmax=wvt.h;
      screenmin=wvt.w;
      }
    updateProjection();
    }
  flags&=~FLAG_DIRTY;
  }


// Change scene
void FXGLViewer::setScene(FXGLObject* sc){
  scene=sc;
  update(0,0,width,height);
  }


/*********************  V i e w i n g   S e t u p  ***************************/


// Set up GL context
void FXGLViewer::glsetup(){
#ifdef HAVE_OPENGL

  // Make GL context current
  if(makeCurrent()){

    // Initialize GL context
    glRenderMode(GL_RENDER);

    // Fast hints
    glHint(GL_POLYGON_SMOOTH_HINT,GL_FASTEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_FASTEST);
    glHint(GL_FOG_HINT,GL_FASTEST);
    glHint(GL_LINE_SMOOTH_HINT,GL_FASTEST);
    glHint(GL_POINT_SMOOTH_HINT,GL_FASTEST);

    // Z-buffer test on
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthRange(0.0,1.0);
    glClearDepth(1.0);
    glClearColor(background[0],background[1],background[2],1.0);

    // No face culling
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Two sided lighting
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,TRUE);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,FALSE);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ambient);

    // Preferred blend over background
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    // Light on
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0,GL_AMBIENT,light.ambient);
    glLightfv(GL_LIGHT0,GL_DIFFUSE,light.diffuse);
    glLightfv(GL_LIGHT0,GL_SPECULAR,light.specular);
    glLightfv(GL_LIGHT0,GL_POSITION,light.position);
    glLightfv(GL_LIGHT0,GL_SPOT_DIRECTION,light.direction);
    glLightf(GL_LIGHT0,GL_SPOT_EXPONENT,light.exponent);
    glLightf(GL_LIGHT0,GL_SPOT_CUTOFF,light.cutoff);
    glLightf(GL_LIGHT0,GL_CONSTANT_ATTENUATION,light.c_attn);
    glLightf(GL_LIGHT0,GL_LINEAR_ATTENUATION,light.l_attn);
    glLightf(GL_LIGHT0,GL_QUADRATIC_ATTENUATION,light.q_attn);

    // Viewer is far away 
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,FALSE);

    // Material colors
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,material.ambient);
    glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,material.diffuse);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,material.specular);
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,material.emission);
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,material.shininess);

    // Vertex colors change both diffuse and ambient
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glDisable(GL_COLOR_MATERIAL);
    
    // Simplest and fastest drawing is default
    glShadeModel(GL_FLAT);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_COLOR_MATERIAL);
    
    // Lighting
    glDisable(GL_LIGHTING);
    
    // No normalization of normals (it's broken on some machines anyway)
    glDisable(GL_NORMALIZE);
    
    // Dithering if needed
    glDisable(GL_DITHER);
    }
#endif
  }


// Change field of view
void FXGLViewer::setFieldOfView(FXdouble fv){
  FXdouble tn;
  fov=CLAMP(1.0,fv,90.0);
  tn=tan(0.5*DTOR*fov);
  FXASSERT(tn>0.0);
  distance=diameter/tn;
  FXASSERT(distance>0.0);
  updateProjection();
  updateTransform();
  update(0,0,width,height);
  }


// Change eye distance
void FXGLViewer::setDistance(FXdouble ed){
  if(ed<diameter) ed=diameter;
  if(ed>114.0*diameter) ed=114.0*diameter;
  distance=ed;
  FXASSERT(distance>0.0);
  fov=2.0*RTOD*atan2(diameter,distance);
  updateProjection();
  updateTransform();
  update(0,0,width,height);
  }


// Change zoom factor
void FXGLViewer::setZoom(FXdouble zm){
  if(1.0E-30<zm){
   zoom=zm;
   updateProjection();
   update(0,0,width,height);
   }
 }



// Change orientation to new quaternion
void FXGLViewer::setOrientation(const FXQuat& rot){
  rotation=rot;
  rotation.adjust();
  updateTransform();
  update(0,0,width,height);
  }


// Change world projection
void FXGLViewer::updateProjection(){
  double hither_fac,r;

  // Get world box
  r=0.5*zoom*diameter;
  if(wvt.w<=wvt.h){
    wvt.left=-r;
    wvt.right=r;
    wvt.bottom=-r*aspect;
    wvt.top=r*aspect;
    }
  else{
    wvt.left=-r/aspect;
    wvt.right=r/aspect;
    wvt.bottom=-r;
    wvt.top=r;
    }
  FXASSERT(distance>0.0);
  FXASSERT(diameter>0.0);
  
  // A more intelligent method for setting the
  // clip planes might be interesting...
  wvt.yon=distance+diameter;
  wvt.hither=0.1*wvt.yon;
//  if(wvt.hither<distance-diameter) wvt.hither=distance-diameter;

  // Precalc stuff for view->world backmapping
  FXASSERT(wvt.w>0 && wvt.h>0);
  mx=(wvt.right-wvt.left)/wvt.w;
  my=(wvt.bottom-wvt.top)/wvt.h;
  ax=wvt.left;
  ay=wvt.bottom-my*wvt.h;

  // Correction for perspective
  if(projection==PERSPECTIVE){
    FXASSERT(distance>0.0);
    hither_fac=wvt.hither/distance;
    wvt.left*=hither_fac;
    wvt.right*=hither_fac;
    wvt.top*=hither_fac;
    wvt.bottom*=hither_fac;
    }

  // Size of a pixel in world and model
  worldpx=(wvt.right-wvt.left)/wvt.w;
  modelpx=worldpx/diameter;
  }

  
// Set model bounding box
FXbool FXGLViewer::setBounds(const FXRange& r){
  
  // Model center
  center=boxCenter(r);

  // Model size
  diameter=r.longest();

  // Fix zero size model
  if(diameter<1.0E-30) diameter=1.0;

  // Set equal scaling initially
  scale=FXVec(1.0,1.0,1.0);

  // Reset distance (and thus field of view)
  setDistance(2.0*diameter);

  return TRUE;
  }


// Fit view to new bounds
FXbool FXGLViewer::fitToBounds(const FXRange& box){
  FXRange r(FLT_MAX,-FLT_MAX,FLT_MAX,-FLT_MAX,FLT_MAX,-FLT_MAX);
  FXVec corner[8],v;
  FXHMat m;
  register int i;

  // Get corners
  boxCorners(corner,box);

  // Get rotation of model
  m.eye();
  m.rot(rotation);
  m.trans(-boxCenter(box));

  // Transform box
  for(i=0; i<8; i++){
    v=corner[i]*m;
    r.include(v[0],v[1],v[2]);
    }

  setBounds(r);
  return TRUE;
  }


// Change transformation matrix
void FXGLViewer::updateTransform(){
  transform.eye();
  transform.trans(0.0,0.0,-distance);
  transform.rot(rotation);
  transform.scale(scale);
  transform.trans(-center);
  itransform=invert(transform);
// fprintf(stderr,"itrans=%10.8f %10.8f %10.8f %10.8f\n",itransform[0][0],itransform[0][1],itransform[0][2],itransform[0][3]);
// fprintf(stderr,"       %10.8f %10.8f %10.8f %10.8f\n",itransform[1][0],itransform[1][1],itransform[1][2],itransform[1][3]);
// fprintf(stderr,"       %10.8f %10.8f %10.8f %10.8f\n",itransform[2][0],itransform[2][1],itransform[2][2],itransform[2][3]);
// fprintf(stderr,"       %10.8f %10.8f %10.8f %10.8f\n",itransform[3][0],itransform[3][1],itransform[3][2],itransform[3][3]);
// fprintf(stderr,"\n");
// fprintf(stderr," trans=%10.8f %10.8f %10.8f %10.8f\n",transform[0][0],transform[0][1],transform[0][2],transform[0][3]);
// fprintf(stderr,"       %10.8f %10.8f %10.8f %10.8f\n",transform[1][0],transform[1][1],transform[1][2],transform[1][3]);
// fprintf(stderr,"       %10.8f %10.8f %10.8f %10.8f\n",transform[2][0],transform[2][1],transform[2][2],transform[2][3]);
// fprintf(stderr,"       %10.8f %10.8f %10.8f %10.8f\n",transform[3][0],transform[3][1],transform[3][2],transform[3][3]);
// fprintf(stderr,"\n");
  }


// Get screen point from eye coordinate
void FXGLViewer::eyeToScreen(FXint& sx,FXint& sy,const FXVec& e){
  register double xp,yp;
  if(projection==PERSPECTIVE){
    if(e[2]==0.0){ fxerror("%s::eyeToScreen: cannot transform point.\n",getClassName()); }
    xp=-distance*e[0]/e[2];
    yp=-distance*e[1]/e[2];
    }
  else{
    xp=e[0];
    yp=e[1];
    }
  sx=(int)((xp-ax)/mx);
  sy=(int)((yp-ay)/my);
  }


// Convert screen point to eye coords
FXVec FXGLViewer::screenToEye(FXint sx,FXint sy,FXfloat eyez){
  register float xp,yp;
  FXVec e;
  xp=mx*sx+ax;
  yp=my*sy+ay;
  if(projection==PERSPECTIVE){
    FXASSERT(distance>0.0);
    e[0]=-eyez*xp/distance;
    e[1]=-eyez*yp/distance;
    e[2]=eyez;
    }
  else{
    e[0]=xp;
    e[1]=yp;
    e[2]=eyez;
    }
  return e;
  }


// Convert screen to eye, on projection plane
FXVec FXGLViewer::screenToTarget(FXint sx,FXint sy){
  return FXVec(mx*sx+ax, my*sy+ay, -distance);
  } 


// Convert world to eye coords
FXVec FXGLViewer::worldToEye(const FXVec& w){ 
  return w*transform;
  }


// Get eye Z-coordinate of world point
FXfloat FXGLViewer::worldToEyeZ(const FXVec& w){
  return w[0]*transform[0][2]+w[1]*transform[1][2]+w[2]*transform[2][2]+transform[3][2];
  }


// Convert eye to world coords
FXVec FXGLViewer::eyeToWorld(const FXVec& e){ 
  return e*itransform; 
  }


// Get world vector
FXVec FXGLViewer::worldVector(FXint fx,FXint fy,FXint tx,FXint ty){
  FXVec wfm,wto,vec;
  wfm=screenToTarget(fx,fy);
  wto=screenToTarget(tx,ty);
  vec=wto*itransform-wfm*itransform;
  return vec;
  }


// Get a bore vector
FXbool FXGLViewer::getBoreVector(FXint sx,FXint sy,FXVec& point,FXVec& dir){
  FXVec p;
  FXdouble d;
  p=eyeToWorld(screenToEye(sx,sy,diameter-distance));
  if(PARALLEL==projection)
    point=eyeToWorld(screenToEye(sx,sy,0));
  else
    point=eyeToWorld(FXVec(0,0,0));
  dir=p-point;
  d=len(dir);
  if(0.0<d) dir/=d;         // normalize
  return TRUE;
  }


// Get eye viewing direction (non-normalized)
FXVec FXGLViewer::getEyeVector() const{
  return FXVec(-distance*itransform[2][0],-distance*itransform[2][1],-distance*itransform[2][2]);
  }


// Get eye position
FXVec FXGLViewer::getEyePosition() const{
  return FXVec(0.0,0.0,0.0)*itransform;
  }


// Change model center
void FXGLViewer::setCenter(const FXVec& cntr){
  center=cntr;
  updateTransform();
  update(0,0,width,height);
  }


// Translate in world 
void FXGLViewer::translate(const FXVec& vec){
  center+=vec;
  updateTransform();
  update(0,0,width,height);
  }


// Change selection
void FXGLViewer::setSelection(FXGLObject* sel){
  FXbool change=FALSE;
  if(selection!=sel){
    if(selection) change|=selection->deselect(this);
    selection=sel;
    if(selection) change|=selection->select(this);
    if(change){
      update(0,0,width,height);
      }
    }
  }


// Change help text
void FXGLViewer::setHelpText(const FXchar* text){
  help=text;
  }


// Change tip text
void FXGLViewer::setTipText(const FXchar* text){
  tip=text;
  }


// Translate point into unit-sphere coordinate
FXVec FXGLViewer::spherePoint(FXint px,FXint py){
  FXdouble d,t;
  FXVec v;
  FXASSERT(screenmin>0);
  v[0]=2.0*(px-0.5*wvt.w)/screenmin;
  v[1]=2.0*(0.5*wvt.h-py)/screenmin;
  d=v[0]*v[0]+v[1]*v[1];
  if(d<0.75){
    v[2]=sqrt(1.0-d);
    }
  else if(d<3.0){ 
    d=1.7320508008-sqrt(d); 
    t=1.0-d*d;
    if(t<0.0) t=0.0;
    v[2]=1.0-sqrt(t); 
    }
  else{ 
    v[2]=0.0;
    }
  return normalize(v);
  }


// Turn camera
FXQuat FXGLViewer::turn(FXint fx,FXint fy,FXint tx,FXint ty){
  FXVec oldsp,newsp;
  FXQuat q;
  FXdouble t;
  oldsp=spherePoint(fx,fy);
  newsp=spherePoint(tx,ty);
  q=arc(oldsp,newsp);
  q[0]*=0.5; 
  q[1]*=0.5; 
  q[2]*=0.5; 
  t=1.0-(q[0]*q[0]+q[1]*q[1]+q[2]*q[2]);
  if(t<0.0) t=0.0;
  q[3]=sqrt(t);
  return q;
  }


// It can be focused on
FXbool FXGLViewer::canFocus() const { return TRUE; }


// We always redraw the entire window
void FXGLViewer::update(FXint,FXint,FXint,FXint){
  FXGLCanvas::update(0,0,width,height);
  }


// Draw non-destructive lasso box; drawing twice will erase it again
void FXGLViewer::drawLasso(FXint x0,FXint y0,FXint x1,FXint y1){
#ifdef HAVE_OPENGL
  if(makeCurrent()){
    
    // With Mesa, this is quite simple
#ifdef HAVE_MESA
    setFunction(BLT_NOT_DST);
    drawLine(x0,y0,x1,y0);
    drawLine(x1,y0,x1,y1);
    drawLine(x1,y1,x0,y1);
    drawLine(x0,y1,x0,y0);
    setFunction(BLT_SRC);
    
    // With OpenGL, first change back to 1:1 projection mode
#else
    glPushAttrib(GL_COLOR_BUFFER_BIT|GL_ENABLE_BIT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0,width-1.0,0.0,height-1.0,0.0,1.0); 
    if(options&GL_DOUBLE_BUFFER) glDrawBuffer(GL_FRONT);
    glLineWidth(1.0);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_LIGHTING);
    glShadeModel(GL_FLAT);
    glDepthMask(FALSE);
    
    // On SGI's we will probably have logic ops
#ifdef _SGI_
    glBlendEquationEXT(GL_LOGIC_OP);
    glLogicOp(GL_XOR);
    
    // On other machines we may not
#else
    glBlendFunc(GL_ONE_MINUS_DST_COLOR,GL_ONE_MINUS_SRC_COLOR);
#endif
    glBegin(GL_LINE_LOOP);
    glColor3f(1.0,1.0,1.0);
    glVertex2i(x0,wvt.h-y0-1);
    glVertex2i(x0,wvt.h-y1-1);
    glVertex2i(x1,wvt.h-y1-1);
    glVertex2i(x1,wvt.h-y0-1);
    glEnd();
    glFinish();
    if(options&GL_DOUBLE_BUFFER) glDrawBuffer(GL_BACK);
    glDepthMask(TRUE);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
#endif
    }
#endif
  }
  
/*************************  Mouse Actions in Viewer  ***************************/

/*
  Basic mouse actions:
  
  Button  Dir.   Modifier(s) Where              Moved? Action
  ----------------------------------------------------------------
  Left     Dn    None        Inside Selection   --     Start drag of selection
  Left     Dn    None        Outside Selection  --     Start rotate
  Left     Up    None        Inside Selection   No     End drag of selection
  Left     Up    None        Outside Selection  No     Deselect selection; select new selection; end rotate
  Left     Up    None        Anywhere           Yes    End rotate
  Left     Dn    Shift       Anywhere           --     Start lasso
  Left     Dn    Shift       Outside Selection  --     Start lasso
  Left     Up    Shift       Anywhere           No     End lasso; add new object(s) to selection
  Left     Up    Shift       Anywhere           Yes    End lasso; add lassoed to selection
  Left     Dn    Control     Anywhere           --     Start lasso
  Left     Dn    Control     Outside Selection  --     Start lasso
  Left     Up    Control     Anywhere           No     End lasso; toggle new object's selection status
  Left     Up    Control     Anywhere           Yes    End lasso; toggle lassoed objects selection status
  Left     Pr    X           Anywhere           --     Rotate about model X
  Left     Pr    Y           Anywhere           --     Rotate about model Y
  Left     Pr    Z           Anywhere           --     Rotate about model Z
  Left     Pr    U           Anywhere           --     Rotate about camera X
  Left     Pr    V           Anywhere           --     Rotate about camera Y
  Left     Pr    W           Anywhere           --     Rotate about camera Z
  Left   DblClk  None        Inside Selection   --     Edit object
  Left   DblClk  None        Outside Selection  --     Deselect selection
  -------------------------------------------------------------
  Middle   Dn    None        Anywhere           --     Start zoom
  Middle   Up    None        Anywhere           --     End zoom
  Middle   Dn    Shift       Anywhere           --     Start dolly
  Middle   Up    Shift       Anywhere           --     End dolly
  -------------------------------------------------------------
  Right    Dn    None        Selection          --     Popup menu for selection
  Right    Dn    None        Outside selection  --     Start translate
  Right    Up    None        Anywhere           --     End translate
  Right    Pr    X           Anywhere           --     Start translate in X=c (perhaps along X???)
  Right    Pr    Y           Anywhere           --     Start translate in Y=c
  Right    Pr    Z           Anywhere           --     Start translate in Z=c
  Right    Dn    Shift       Anywhere           --     Start FOV change
  Right    Up    Shift       Anywhere           --     End FOV change
  
  Selection:
  
  o  Selection depth is 0 for top level.
  o  If selection depth is N for group object:
     - it is N+1 for current child.
     - it is N for other children.
  o  FXGLViewer maintains selection depth.
*/


// Perform the usual mouse manipulation
long FXGLViewer::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  flags&=~FLAG_TIP;
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr)) return 1;
    if(event->click_count==1){
      
      // Move focus this widget
      if(canFocus()) setFocus();

      // Do the lasso
      if(event->state&(SHIFTMASK|CONTROLMASK)){
        drawLasso(event->click_x,event->click_y,event->win_x,event->win_y);
        // Change cursor to Lasso corner
        op=LASSOING;
        }

      // Hit current selection?
      else if(selection && (selection==pick(event->click_x,event->click_y))){
        if(selection->canDrag()){
          
          // Change cursor to Dragging
          op=DRAGGING;
          }
        }

      // Was no selection, just rotate
      else{
        // Change cursor to rotating
        op=ROTATING;
        }
      }
    flags&=~FLAG_UPDATE;
    }
  return 1;
  }


// Left mouse button released
long FXGLViewer::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  register FXuint operation=op;
  if(isEnabled()){
    flags|=FLAG_UPDATE;
    op=PICKING;
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr)) return 1;
    if(event->click_count==1){
      
      // We were lassoing
      if(operation==LASSOING){
        drawLasso(event->click_x,event->click_y,event->win_x,event->win_y);
        // Select stuff in the lasso
        }
      
      // We were dragging
      else if(operation==DRAGGING){
        }
      
      // We were rotating, but didn't move [too much]
      else if(!event->moved){
        
        // Select new selection
        setSelection(pick(event->click_x,event->click_y));
        }
      
      // We were rotating
      else{
        }
      }
    
    // We double-clicked
    else if(selection){
      // Should edit the selected object
      }
    }
  return 1;
  }


// Pressed middle mouse button
long FXGLViewer::onMiddleBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  flags&=~FLAG_TIP;
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONPRESS),ptr)) return 1;
    if(canFocus()) setFocus();
    if(event->state&(SHIFTMASK|CONTROLMASK)){
      // Change cursor to Dolly
      op=DOLLYING;
      }
    else{
      // Change cursor to Magnify
      op=MAGNIFYING;
      }
    flags&=~FLAG_UPDATE;
    }
  return 1;
  }


// Released middle mouse button
long FXGLViewer::onMiddleBtnRelease(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    flags|=FLAG_UPDATE;
    op=PICKING;
    if(target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONRELEASE),ptr)) return 1;
    }
  return 1;
  }


// Pressed right button
long FXGLViewer::onRightBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  flags&=~FLAG_TIP;
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONPRESS),ptr)) return 1;
    if(canFocus()) setFocus();
    if(event->state&SHIFTMASK){
      // Change cursor to gyrate
      op=GYRATING;
      }
    else if(event->state&CONTROLMASK){
      // Change cursor to FOV
      op=FOVING;
      }
    else{
      // Change cursor to translating
      op=TRANSLATING;
      }
    flags&=~FLAG_UPDATE;
    }
  return 1;
  }


// Released right button
long FXGLViewer::onRightBtnRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  register FXuint operation=op;
  FXGLObject *hit;
  if(isEnabled()){
    flags|=FLAG_UPDATE;
    op=PICKING;
    if(target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONRELEASE),ptr)) return 1;
    if(operation==TRANSLATING && !event->moved){
      if((hit=pick(event->click_x,event->click_y))!=NULL){
        hit->handle(this,MKUINT(ID_QUERY_MENU,SEL_COMMAND),ptr);
        }
      }
    }
  return 1;
  }


// We timed out, i.e. the user didn't move for a while
long FXGLViewer::onTimeout(FXObject*,FXSelector,void*){
//fprintf(stderr,"%s::onTimeout %08x\n",getClassName(),this);
  timer=NULL;
  flags|=FLAG_TIP;
  return 1;
  }


// We were asked about status text
long FXGLViewer::onQueryHelp(FXObject* sender,FXSelector,void*){
//fprintf(stderr,"%s::onQueryHelp %08x\n",getClassName(),this);
  if(help.text() && (flags&FLAG_HELP)){
    sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&help);
    return 1;
    }
  return 0;
  }


// We were asked about tip text
long FXGLViewer::onQueryTip(FXObject* sender,FXSelector sel,void* ptr){
//fprintf(stderr,"%s::onQueryTip %08x\n",getClassName(),this);
//fprintf(stderr,"tip shown=%d\n",((FXTooltip*)sender)->shown());
  FXint x,y; FXuint state;
  if(flags&FLAG_TIP){
    getCursorPosition(x,y,state);
    FXGLObject *hit=pick(x,y);
    return hit && hit->handle(sender,sel,ptr);
    }
  return 0;
  }


// Mouse moved
long FXGLViewer::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  register FXdouble delta;
  register FXfloat tmp;
  FXVec vec;
  FXQuat q;
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_MOTION),ptr)) return 1;
    if(event->last_x!=event->win_x || event->last_y!=event->win_y){
      if(timer){ getApp()->removeTimeout(timer); timer=NULL; }
      switch(op){
        case PICKING:           // Reset the timer each time we moved the cursor
          timer=getApp()->addTimeout(getApp()->menuPause,this,ID_TIPTIMER);
          break;
        case DOLLYING:          // Dollying camera forward or backward
          tmp=my*(event->last_y-event->win_y);
          vec=getEyeVector();
          translate(tmp*vec);
          break;
        case TRANSLATING:       // Translating camera
          vec=worldVector(event->last_x,event->last_y,event->win_x,event->win_y);
          translate(-vec);
          break;
        case MAGNIFYING:        // Zooming camera
          delta=0.005*(event->last_y-event->win_y);
          setZoom(getZoom()*pow(2.0,delta));
          break;
        case ROTATING:          // Rotating camera around target
          q=turn(event->last_x,event->last_y,event->win_x,event->win_y) * getOrientation();
          setOrientation(q);
          break;
        case GYRATING:          // Rotating camera around eye
          update(0,0,width,height);
          break;
        case LASSOING:          // Lasso an bunch of objects
          drawLasso(event->click_x,event->click_y,event->last_x,event->last_y);
          drawLasso(event->click_x,event->click_y,event->win_x,event->win_y);
          break;
        case FOVING:            // Change FOV
          setFieldOfView(getFieldOfView()+90.0*(event->win_y-event->last_y)/(double)wvt.h);
          break;
        case DRAGGING:          // Dragging a shape
          FXASSERT(selection);
          if(selection->drag(this,event->last_x,event->last_y,event->win_x,event->win_y)){
            update(0,0,width,height);
            }
          break;
        }
      }
    }
  flags&=~FLAG_TIP;
  return 1;
  }


// Update this widgets state, when it is not active
long FXGLViewer::onUpdate(FXObject* sender,FXSelector sel,void* ptr){
  FXWindow::onUpdate(sender,sel,ptr);
  
  ///// GRRRMBL@#&@%^$%^%# Need better way to do this!
  if(scene){
    FXuint newstamp=scene->stamp(this);
    if(getStamp()!=newstamp){
      update(0,0,width,height);
      setStamp(newstamp);
      }
    }
  return 1;
  }


/*****************************  Keyboard Input  ********************************/


// Handle keyboard press/release 
long FXGLViewer::onKeyPress(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_KEYPRESS),ptr)) return 1;
    }
  return 0;
  }


// Key release
long FXGLViewer::onKeyRelease(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_KEYRELEASE),ptr)) return 1;
    }
  return 0;
  }


/*****************************  Switch Projection  *****************************/


// Switch to perspective mode
long FXGLViewer::onCmdPerspective(FXObject*,FXSelector,void*){
  setProjection(PERSPECTIVE);
  return 1;
  }


// Update sender  
long FXGLViewer::onUpdPerspective(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=projection==PERSPECTIVE ? ID_CHECK : ID_UNCHECK;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }


// Switch to parallel mode
long FXGLViewer::onCmdParallel(FXObject*,FXSelector,void*){
  setProjection(PARALLEL);
  return 1;
  }


// Update sender
long FXGLViewer::onUpdParallel(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=projection==PARALLEL ? ID_CHECK : ID_UNCHECK;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }


/*****************************  Switch Viewpoints  *****************************/

// View front
long FXGLViewer::onCmdFront(FXObject*,FXSelector,void*){
  rotation=FXQuat(0.0,0.0,0.0,1.0);
  updateTransform();
  update(0,0,width,height);
  return 1;
  }


// Update sender
long FXGLViewer::onUpdFront(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=ID_UNCHECK;
  if(EPS>fabs(rotation[0]) && 
     EPS>fabs(rotation[1]) && 
     EPS>fabs(rotation[2]) && 
     EPS>fabs(rotation[3]-1.0)) msg=ID_CHECK;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }

// View back
long FXGLViewer::onCmdBack(FXObject*,FXSelector,void*){
  rotation=FXQuat(0.0,-1.0,0.0,0.0);
  updateTransform();
  update(0,0,width,height);
  return 1;
  }


// Update sender
long FXGLViewer::onUpdBack(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=ID_UNCHECK;
  if(EPS>fabs(rotation[0]) &&
     EPS>fabs(rotation[1]+1.0) &&
     EPS>fabs(rotation[2]) &&
     EPS>fabs(rotation[3])) msg=ID_CHECK;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }

// View left
long FXGLViewer::onCmdLeft(FXObject*,FXSelector,void*){
  rotation=FXQuat(0.0,0.7071067811865,0.0,0.7071067811865);
  updateTransform();
  update(0,0,width,height);
  return 1;
  }


// Update sender
long FXGLViewer::onUpdLeft(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=ID_UNCHECK;
  if(EPS>fabs(rotation[0]) &&
     EPS>fabs(rotation[1]-0.7071067811865) &&
     EPS>fabs(rotation[2]) &&
     EPS>fabs(rotation[3]-0.7071067811865)) msg=ID_CHECK;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }

// View right
long FXGLViewer::onCmdRight(FXObject*,FXSelector,void*){
  rotation=FXQuat(0.0,-0.7071067811865,0.0,0.7071067811865);
  updateTransform();
  update(0,0,width,height);
  return 1;
  }


// Update sender
long FXGLViewer::onUpdRight(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=ID_UNCHECK;
  if(EPS>fabs(rotation[0]) &&
     EPS>fabs(rotation[1]+0.7071067811865) &&
     EPS>fabs(rotation[2]) &&
     EPS>fabs(rotation[3]-0.7071067811865)) msg=ID_CHECK;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }

// View top
long FXGLViewer::onCmdTop(FXObject*,FXSelector,void*){
  rotation=FXQuat(0.7071067811865,0.0,0.0,0.7071067811865);
  updateTransform();
  update(0,0,width,height);
  return 1;
  }


// Update sender
long FXGLViewer::onUpdTop(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=ID_UNCHECK;
  if(EPS>fabs(rotation[0]-0.7071067811865) &&
     EPS>fabs(rotation[1]) &&
     EPS>fabs(rotation[2]) &&
     EPS>fabs(rotation[3]-0.7071067811865)) msg=ID_CHECK;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }

// View bottom
long FXGLViewer::onCmdBottom(FXObject*,FXSelector,void*){
  rotation=FXQuat(-0.7071067811865,0.0,0.0,0.7071067811865);
  updateTransform();
  update(0,0,width,height);
  return 1;
  }


// Update sender
long FXGLViewer::onUpdBottom(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=ID_UNCHECK;
  if(EPS>fabs(rotation[0]+0.7071067811865) &&
     EPS>fabs(rotation[1]) &&
     EPS>fabs(rotation[2]) &&
     EPS>fabs(rotation[3]-0.7071067811865)) msg=ID_CHECK;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }


// Reset view
long FXGLViewer::onCmdResetView(FXObject*,FXSelector,void*){
  FXRange r(-1.0,1.0,-1.0,1.0,-1.0,1.0);
  rotation=FXQuat(0.0,0.0,0.0,1.0);
  zoom=1.0;
  scale=FXVec(1.0,1.0,1.0);
  setDistance(2.0*diameter);
  if(scene) scene->bounds(r);
  setBounds(r);
  updateTransform();
  update(0,0,width,height);
  return 1;
  }


// Fit view
long FXGLViewer::onCmdFitView(FXObject*,FXSelector,void*){
  FXRange r(-1.0,1.0,-1.0,1.0,-1.0,1.0);
  if(scene) scene->bounds(r);
  setBounds(r);
  update(0,0,width,height);
  return 1;
  }


/*******************************  Drag and Drop  *******************************/


// Handle drag-and-drop enter
long FXGLViewer::onDNDEnter(FXObject* sender,FXSelector sel,void* ptr){
//fprintf(stderr,"%s::onDNDEnter win(%d)\n",getClassName(),xid);
  if(FXGLCanvas::onDNDEnter(sender,sel,ptr)) return 1;
  dropped=NULL;
  return 1;
  }

// Handle drag-and-drop leave
long FXGLViewer::onDNDLeave(FXObject* sender,FXSelector sel,void* ptr){
//fprintf(stderr,"%s::onDNDLeave win(%d)\n",getClassName(),xid);
  if(FXGLCanvas::onDNDLeave(sender,sel,ptr)) return 1;
  dropped=NULL;
  return 1;
  }


// Handle drag-and-drop motion
long FXGLViewer::onDNDMotion(FXObject* sender,FXSelector sel,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
//fprintf(stderr,"%s::onDNDMotion win(%d)\n",getClassName(),xid);
  
  // Handled elsewhere
  if(FXGLCanvas::onDNDMotion(sender,sel,ptr)) return 1;
  
  // Dropped on some object
  if((dropped=pick(event->win_x,event->win_y))!=NULL){
    
    // Object agrees with drop type
    if(dropped->handle(this,sel,ptr)){
      acceptDrop(DRAG_COPY);
      return 1;
      }
    
    // Forget about the whole thing
    dropped=NULL;
    return 0;
    }
  
  // Dropped in viewer background; hope its a color
  if(offeredDNDType(dragTypeColor)){
    acceptDrop(DRAG_COPY);
    return 1;
    }
  
  // Won't accept drop, dont know what it is
  return 0;
  }


// Handle drag-and-drop drop
long FXGLViewer::onDNDDrop(FXObject* sender,FXSelector sel,void* ptr){
  FXuint len,r,g,b,a;
  FXuchar *data; 
//fprintf(stderr,"%s::onDNDDrop win(%d)\n",getClassName(),xid);
  
  // Try base class first
  if(FXGLCanvas::onDNDDrop(sender,sel,ptr)) return 1;
  
  // Dropped on object?
  if(dropped){
    
    // Object handled drop; so probably want to repaint
    if(dropped->handle(this,sel,ptr)){
      update(0,0,width,height);
      return 1;
      }
    
    // We're done
    return 0;
    }
  
  // Dropped on viewer
  if(offeredDNDType(dragTypeColor)){
    if(getDNDData(FXGLViewer::dragTypeColor,data,len)){
      sscanf((char*)data,"#%02x%02x%02x%02x",&r,&g,&b,&a);
      background[0]=r*0.0039215686;
      background[1]=g*0.0039215686;
      background[2]=b*0.0039215686;
      FXFREE(&data);
      update(0,0,width,height);
      return 1;
      }
    }
  return 0;
  }


// Change projection
void FXGLViewer::setProjection(FXuint proj){
  projection=proj;
  updateProjection();
  update(0,0,width,height);
  }


// Delegate some messages to the GL Object
long FXGLViewer::handle(FXObject* sender,FXSelector key,void* data){
  
  // Filter out messages for the GL Viewer itself
  if(FXGLCanvas::handle(sender,key,data)) return 1;
  
  // Unknown messages are passed to the selected GL Object
  if(ID_LAST<=SELID(key) && selection && selection->handle(sender,key,data)){
    
    // Repaint:- there should be a better mechanism than this!
    if(SELTYPE(key)!=SEL_UPDATE) update(0,0,width,height);
    return 1;
    }
  return 0;
  }


// Save object to stream
void FXGLViewer::save(FXStream& store) const {
  FXGLCanvas::save(store);
  store << transform;
  store << itransform;
  store << projection;
  store << rotation;
  store << fov;
  store << zoom;
  store << center;
  store << scale;
  store << background;
  store << ambient;
  store << help;
  store << tip;
  }

      

// Load object from stream
void FXGLViewer::load(FXStream& store){
  FXGLCanvas::load(store);
  store >> transform;
  store >> itransform;
  store >> projection;
  store >> rotation;
  store >> fov;
  store >> zoom;
  store >> center;
  store >> scale;
  store >> background;
  store >> ambient;
  store >> help;
  store >> tip;
  }  


// Close and release any resources
FXGLViewer::~FXGLViewer(){
  if(timer) getApp()->removeTimeout(timer);
  timer=(FXTimer*)-1;
  dropped=(FXGLObject*)-1;
  selection=(FXGLObject*)-1;
  scene=(FXGLObject*)-1;
  }
