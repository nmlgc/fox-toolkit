/********************************************************************************
*                                                                               *
*                G L  C a n v a s   W i n d o w   O b j e c t                   *
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
* $Id: FXGLCanvas.h,v 1.8 1998/10/31 00:00:29 jvz Exp $                      *
********************************************************************************/
#ifndef FXGLCANVAS_H
#define FXGLCANVAS_H


// OpenGL options
enum FXGLOption {
  GL_INSTALL_COLORMAP = 0x00008000,   // Install colormaps
  GL_DOUBLE_BUFFER    = 0x00010000,   // Double buffer mode
  GL_STEREO_BUFFER    = 0x00020000,   // Stereo buffers
  GL_PRIVATE_MAP      = 0x00040000    // Private colormap for GL
  };


// Canvas, an area drawn by another object
class FXGLCanvas : public FXWindow {
  FXDECLARE(FXGLCanvas)
protected:
  FXint        redSize;
  FXint        greenSize;
  FXint        blueSize;
  FXint        alphaSize;
  FXint        depthSize;
  FXint        stencilSize;
  FXint        accumRedSize;
  FXint        accumGreenSize;
  FXint        accumBlueSize;
  FXint        accumAlphaSize;
private:
  XVisualInfo *visualInfo;
  FXID         colormap;
  void*        ctx;
protected:
  FXGLCanvas(){}
  FXGLCanvas(const FXGLCanvas&){}
  virtual Visual* getDefaultVisual();
  virtual FXint getDefaultDepth();
  virtual FXID getDefaultColormap();
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onConfigure(FXObject*,FXSelector,void*);
  long onMotion(FXObject*,FXSelector,void*);
  long onLeftBtnPress(FXObject*,FXSelector,void*);
  long onLeftBtnRelease(FXObject*,FXSelector,void*);
  long onMiddleBtnPress(FXObject*,FXSelector,void*);
  long onMiddleBtnRelease(FXObject*,FXSelector,void*);
  long onRightBtnPress(FXObject*,FXSelector,void*);
  long onRightBtnRelease(FXObject*,FXSelector,void*);
  long onKeyPress(FXObject*,FXSelector,void*);
  long onKeyRelease(FXObject*,FXSelector,void*);
public:
  FXGLCanvas(FXComposite* p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual long handle(FXObject* sender,FXSelector key,void* data);
  virtual void create();
  virtual void destroy();
  FXbool checkOpenGL();
  FXbool makeCurrent();
  void swapBuffers();
  void swapSubBuffers(FXint x,FXint y,FXint w,FXint h);
  virtual ~FXGLCanvas();
  };


#endif

