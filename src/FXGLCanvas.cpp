/********************************************************************************
*                                                                               *
*                    O p e n G L   C a n v a s   O b j e c t                    *
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
* $Id: FXGLCanvas.cpp,v 1.13 1998/10/31 00:00:31 jvz Exp $                    *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "FXStream.h"
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

/*
  To do:
  - Better solution for onConfigure?
*/


#define ATTRIBLIST_SIZE 100

/*******************************************************************************/

// Map
FXDEFMAP(FXGLCanvas) FXGLCanvasMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXGLCanvas::onPaint),
  FXMAPFUNC(SEL_CONFIGURE,0,FXGLCanvas::onConfigure),
  FXMAPFUNC(SEL_MOTION,0,FXGLCanvas::onMotion),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXGLCanvas::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXGLCanvas::onLeftBtnRelease),
  FXMAPFUNC(SEL_MIDDLEBUTTONPRESS,0,FXGLCanvas::onMiddleBtnPress),
  FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,0,FXGLCanvas::onMiddleBtnRelease),
  FXMAPFUNC(SEL_RIGHTBUTTONPRESS,0,FXGLCanvas::onRightBtnPress),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,0,FXGLCanvas::onRightBtnRelease),
  FXMAPFUNC(SEL_KEYPRESS,0,FXGLCanvas::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXGLCanvas::onKeyRelease),
  };


// Object implementation
FXIMPLEMENT(FXGLCanvas,FXWindow,FXGLCanvasMap,ARRAYNUMBER(FXGLCanvasMap))



// Make a canvas
FXGLCanvas::FXGLCanvas(FXComposite* p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXWindow(p,opts,x,y,w,h){
  flags|=FLAG_ENABLED;
  target=tgt;
  message=sel;
  redSize=1;
  greenSize=1;
  blueSize=1;
  alphaSize=1;
  depthSize=12;
  stencilSize=0;
  accumRedSize=0;
  accumGreenSize=0;
  accumBlueSize=0;
  accumAlphaSize=0;
  visualInfo=NULL;
  colormap=0;
  ctx=NULL;
  }


// Canvas is an object drawn by another
long FXGLCanvas::onPaint(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_PAINT),ptr);
  }


// Handle configure notify
long FXGLCanvas::onConfigure(FXObject* sender,FXSelector sel,void* ptr){
  FXWindow::onConfigure(sender,sel,ptr);
  return target && target->handle(this,MKUINT(message,SEL_CONFIGURE),ptr);
  }



// Handle buttons if not handled in derived class
long FXGLCanvas::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr);
  }

long FXGLCanvas::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr);
  }

long FXGLCanvas::onMiddleBtnPress(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONPRESS),ptr);
  }

long FXGLCanvas::onMiddleBtnRelease(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONRELEASE),ptr);
  }

long FXGLCanvas::onRightBtnPress(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONPRESS),ptr);
  }

long FXGLCanvas::onRightBtnRelease(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONRELEASE),ptr);
  }


// Mouse moved
long FXGLCanvas::onMotion(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_MOTION),ptr);
  }


// Handle keyboard press/release 
long FXGLCanvas::onKeyPress(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_KEYPRESS),ptr);
  }

long FXGLCanvas::onKeyRelease(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_KEYRELEASE),ptr);
  }


// // Dump visual class
// static char* visualClass(int cls){
//   if(cls==TrueColor) return "TrueColor";
//   if(cls==DirectColor) return "DirectColor";
//   if(cls==PseudoColor) return "PseudoColor";
//   if(cls==StaticColor) return "StaticColor";
//   if(cls==GrayScale) return "GrayScale";
//   if(cls==StaticGray) return "StaticGray";
//   return "Unknown";
//   }


// Check for OpenGL extension
FXbool FXGLCanvas::checkOpenGL(){
#ifdef HAVE_OPENGL
  return glXQueryExtension(getApp()->display,NULL,NULL);
#else
  return FALSE;
#endif
  }


// Obtain visual 
Visual* FXGLCanvas::getDefaultVisual(){
  if(!visualInfo){
#ifdef HAVE_OPENGL
    if(checkOpenGL()){
      int attribList[ATTRIBLIST_SIZE];
      int *ptr=attribList;
      *ptr++=GLX_RGBA;
      if(options&GL_DOUBLE_BUFFER) *ptr++=GLX_DOUBLEBUFFER;
      if(options&GL_STEREO_BUFFER) *ptr++=GLX_STEREO;
      *ptr++=GLX_RED_SIZE;
      *ptr++=redSize;
      *ptr++=GLX_GREEN_SIZE;
      *ptr++=greenSize;
      *ptr++=GLX_BLUE_SIZE;
      *ptr++=blueSize;
      *ptr++=GLX_DEPTH_SIZE;
      *ptr++=depthSize;
//    *ptr++=GLX_BUFFER_SIZE;
//    *ptr++=bufferSize;
//    *ptr++=GLX_AUX_BUFFERS;
//    *ptr++=auxBuffers;
//    *ptr++=GLX_ALPHA_SIZE;
//    *ptr++=alphaSize;
//    *ptr++=GLX_STENCIL_SIZE;
//    *ptr++=stencilSize;
//    *ptr++=GLX_ACCUM_RED_SIZE;
//    *ptr++=accumRedSize;
//    *ptr++=GLX_ACCUM_GREEN_SIZE;
//    *ptr++=accumGreenSize;
//    *ptr++=GLX_ACCUM_BLUE_SIZE;
//    *ptr++=accumBlueSize;
//    *ptr++=GLX_ACCUM_ALPHA_SIZE;
//    *ptr++=accumAlphaSize;
      *ptr++=None;
      visualInfo=glXChooseVisual(getApp()->display,DefaultScreen(getApp()->display),attribList);
      if(visualInfo){
//      fprintf(stderr,"visualID=0x%lx\n",visualInfo->visualid);
//      fprintf(stderr,"  Screen: %d\n",visualInfo->screen);
//      fprintf(stderr,"  Depth: %d\n",visualInfo->depth);
//      fprintf(stderr,"  Bits: %d\n",visualInfo->bits_per_rgb);
//      fprintf(stderr,"  Class: %s\n",visualClass(visualInfo->c_class));
        return visualInfo->visual;
        }
      fxerror("%s::getDefaultVisual: requested OpenGL visual unavailable.\n",getClassName());
      }
#endif
    fxerror("%s::getDefaultVisual: display does not support OpenGL.\n",getClassName());
    }
  return visualInfo->visual;
  }


// Get depth from visual
FXint FXGLCanvas::getDefaultDepth(){
  return visualInfo->depth;
  }


// Obtain colormap
FXID FXGLCanvas::getDefaultColormap(){
  static struct CmapCache { Visual *visual; FXID cmap; } *cmapCache=NULL;
  static int cacheMalloced=0;
  static int cacheEntries=0;
  Visual* visual;
  
  if(!colormap){
    
    // Get visual for which to allocate colormap
    visual=getDefaultVisual();
    
    // Only make a new colormap if the visual is different from the default visual
    if((options&GL_PRIVATE_MAP) || (visual!=DefaultVisual(getDisplay(),DefaultScreen(getDisplay())))){
      
      // See if we can find it in the cache
      for(int i=0; i<cacheEntries; i++){
        if(cmapCache[i].visual==visual){
          colormap=cmapCache[i].cmap;
          return colormap;
          }
        }

      // not in the cache, create a new entry
      if(cacheEntries>=cacheMalloced){
        if(cacheMalloced==0){
          cacheMalloced=1;
          cmapCache=(CmapCache*)malloc(sizeof(CmapCache));
          }
        else{
          cacheMalloced<<=1;
          cmapCache=(CmapCache*)realloc(cmapCache,sizeof(CmapCache)*cacheMalloced);
          }
        }
 
      FXASSERT(cmapCache);
 
      // Add new entry
      colormap=XCreateColormap(getApp()->display,RootWindow(getApp()->display,0),visual,AllocNone);
      cmapCache[cacheEntries].cmap=colormap;
      cmapCache[cacheEntries].visual=visual;
      cacheEntries++;
      }
    
    // Get colormap for default visual
    else{
      FXASSERT(visual == DefaultVisual(getDisplay(),DefaultScreen(getDisplay())));
      colormap=DefaultColormap(getDisplay(),DefaultScreen(getDisplay())); 
      }
    }
  return colormap;
  }


// Create X window (GL CANVAS)
void FXGLCanvas::create(){
  FXWindow::create();
  if(options&GL_INSTALL_COLORMAP) addColormapWindows();
#ifdef HAVE_OPENGL
  ctx=glXCreateContext(getApp()->display,visualInfo,NULL,TRUE);
  if(!ctx){ fxerror("%s::create(): unable to create graphics rendering context.\n",getClassName()); }
  glXMakeCurrent(getApp()->display,xid,(GLXContext)ctx);
#endif
  show();
  }


// Destroy the GL Canvas
void FXGLCanvas::destroy(){
  if(options&GL_INSTALL_COLORMAP) removeColormapWindows();
  FXWindow::destroy();
  }


//  Make the rendering context of GL Canvas current
FXbool FXGLCanvas::makeCurrent(){
#ifdef HAVE_OPENGL
  glXMakeCurrent(getApp()->display,xid,(GLXContext)ctx);
#endif
  return TRUE;
  }
  
  

// Used by GL to swap the buffers in double buffer mode, or flush a single buffer
void FXGLCanvas::swapBuffers(){
#ifdef HAVE_OPENGL
  glXSwapBuffers(getApp()->display,xid);
#endif
  }


// This function only available on Mesa
void FXGLCanvas::swapSubBuffers(FXint x,FXint y,FXint w,FXint h){
#ifdef HAVE_OPENGL
#ifdef MESA
#ifdef GLX_MESA_copy_sub_buffer
  //fprintf(stderr,"swapSubBuffers: x=%d y=%d w=%d h=%d\n",x,height-y-h-1,w,h);
  glXCopySubBufferMESA(getApp()->display,xid,x,height-y-h-1,w,h);
#else
  glXSwapBuffers(getApp()->display,xid);
#endif
#endif
#endif
  }


// Delegate some messages to the target
long FXGLCanvas::handle(FXObject* sender,FXSelector key,void* data){
  
  // Try handle myself first
  if(FXWindow::handle(sender,key,data)) return 1;
  return 0;
  // Unknown messages are passed to the target
  ///return target && target->handle(sender,key,data);
  }


// Close and release any resources
FXGLCanvas::~FXGLCanvas(){
  if(options&GL_INSTALL_COLORMAP) removeColormapWindows();
  if(visualInfo) XFree(visualInfo);
  }
