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
* $Id: FXGLViewer.h,v 1.30 1998/10/28 05:14:09 jeroen Exp $                     *
********************************************************************************/
#ifndef FXGLVIEWER_H
#define FXGLVIEWER_H


// Pick tolerance
#define PICK_TOL  6


// Mouse actions when in viewing window
enum FXViewerAction {
  PICKING,                // Pick mode is default
  TRANSLATING,            // Translating camera
  ROTATING,               // Rotating camera around target
  MAGNIFYING,             // Magnifying
  FOVING,                 // Change field-of-view
  DRAGGING,               // Dragging objects
  DOLLYING,               // Dollying camera
  LASSOING,               // Lassoing rectangle
  GYRATING                // Rotation of camera around eye
  };

  
// Projection modes
enum FXViewerProjection {
  PARALLEL,               // Parallel projection
  PERSPECTIVE             // Perspective projection
  };


/************************  W h a t   t o   R e n d e r  **********************/

struct FXViewport {
  FXint      w,h;               // Viewport dimensions
  FXdouble   left,right;        // World box
  FXdouble   bottom,top;
  FXdouble   hither,yon;
  };

struct FXLight {
  FXHVec     ambient;           // Ambient light color
  FXHVec     diffuse;           // Diffuse light color
  FXHVec     specular;          // Specular light color
  FXHVec     position;          // Light position
  FXVec      direction;         // Spot direction
  FXfloat    exponent;          // Spotlight exponent
  FXfloat    cutoff;            // Spotlight cutoff angle
  FXfloat    c_attn;            // Constant attenuation factor
  FXfloat    l_attn;            // Linear attenuation factor
  FXfloat    q_attn;            // Quadratic attenuation factor
  };

struct FXMaterial {
  FXHVec     ambient;           // Ambient material color
  FXHVec     diffuse;           // Diffuse material color
  FXHVec     specular;          // Specular material color
  FXHVec     emission;          // Emissive material color
  FXfloat    shininess;         // Specular shininess
  };


/*******************************  Viewer  Class  *****************************/

  
// Canvas, an area drawn by another object
class FXGLViewer : public FXGLCanvas {
  FXDECLARE(FXGLViewer)
  friend class FXGLObject;
protected:
  FXViewport      wvt;              // Window viewport transform
  FXHMat          transform;        // Current transformation matrix
  FXHMat          itransform;       // Inverse of current transformation matrix
  FXdouble        screenmin;        // Minimum screen dimension
  FXdouble        screenmax;        // Maximum screen dimension
  FXuint          projection;       // Projection mode
  FXQuat          rotation;         // Viewer orientation
  FXdouble        aspect;           // Aspect ratio
  FXdouble        fov;              // Field of view
  FXdouble        zoom;             // Zoom factor
  FXVec           center;           // Model center
  FXVec           scale;            // Model scale
  FXdouble        worldpx;          // Pixel size in world
  FXdouble        modelpx;          // Pixel size in model
  FXuint          op;               // Operation being performed
  FXdouble        mx,ax,my,ay;      // Quick view->world coordinate mapping
  FXdouble        diameter;         // Size of model diameter ( always > 0)
  FXdouble        distance;         // Distance of PRP to target
  FXHVec          background;       // Background color
  FXHVec          ambient;          // Global ambient light
  FXLight         light;            // Light source
  FXMaterial      material;         // Base material properties
  FXuint          stamp;            // Last update stamp
  FXTimer        *timer;            // Motion timer
  FXString        help;             // Status help
  FXString        tip;              // Tooltip
  FXGLObject     *dropped;          // Object being dropped on
  FXGLObject     *selection;        // Selection path
  FXGLObject     *scene;            // What we're looking at
  
public:
  static FXDragType dragTypeColor;  // Drag types

private:
  void glsetup();
  void updateProjection();
  void updateTransform();
  FXVec spherePoint(FXint px,FXint py);
  FXQuat turn(FXint fx,FXint fy,FXint tx,FXint ty);
  void drawWorld(FXViewport& wv);
  void drawAnti(FXViewport& wv);
  void drawLasso(FXint x0,FXint y0,FXint x1,FXint y1);
  FXGLObject* pick(FXint x,FXint y,FXint dw=PICK_TOL,FXint dh=PICK_TOL);
protected:
  FXGLViewer(){}
  FXGLViewer(const FXGLViewer&){}
  virtual void layout();
  virtual FXGLObject* processHits(FXuint *pickbuffer,FXint nhits);
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onUpdate(FXObject*,FXSelector,void*);
  long onEnter(FXObject*,FXSelector,void*);
  long onLeave(FXObject*,FXSelector,void*);
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
  long onFocusIn(FXObject*,FXSelector,void*);
  long onFocusOut(FXObject*,FXSelector,void*);
  long onCmdPerspective(FXObject*,FXSelector,void*);
  long onUpdPerspective(FXObject*,FXSelector,void*);
  long onCmdParallel(FXObject*,FXSelector,void*);
  long onUpdParallel(FXObject*,FXSelector,void*);
  long onCmdFront(FXObject*,FXSelector,void*);
  long onUpdFront(FXObject*,FXSelector,void*);
  long onCmdBack(FXObject*,FXSelector,void*);
  long onUpdBack(FXObject*,FXSelector,void*);
  long onCmdLeft(FXObject*,FXSelector,void*);
  long onUpdLeft(FXObject*,FXSelector,void*);
  long onCmdRight(FXObject*,FXSelector,void*);
  long onUpdRight(FXObject*,FXSelector,void*);
  long onCmdTop(FXObject*,FXSelector,void*);
  long onUpdTop(FXObject*,FXSelector,void*);
  long onCmdBottom(FXObject*,FXSelector,void*);
  long onUpdBottom(FXObject*,FXSelector,void*);
  long onCmdResetView(FXObject*,FXSelector,void*);
  long onCmdFitView(FXObject*,FXSelector,void*);
  long onDNDEnter(FXObject*,FXSelector,void*);
  long onDNDLeave(FXObject*,FXSelector,void*);
  long onDNDMotion(FXObject*,FXSelector,void*);
  long onDNDDrop(FXObject*,FXSelector,void*);
  long onTimeout(FXObject*,FXSelector,void*);
  long onQueryHelp(FXObject*,FXSelector,void*);
  long onQueryTip(FXObject*,FXSelector,void*);
public:
  enum {
    ID_PERSPECTIVE=FXGLCanvas::ID_LAST,
    ID_PARALLEL,
    ID_FRONT,
    ID_BACK,
    ID_LEFT,
    ID_RIGHT,
    ID_TOP,
    ID_BOTTOM,
    ID_RESETVIEW,
    ID_FITVIEW,
    ID_TIPTIMER,
    ID_LAST
    };
public:
  FXGLViewer(FXComposite* p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual long handle(FXObject* sender,FXSelector key,void* data);
  virtual void create();
  FXdouble worldPix() const { return worldpx; }
  FXdouble modelPix() const { return modelpx; }
  FXbool setBounds(const FXRange& box);
  FXbool fitToBounds(const FXRange& box);
  void eyeToScreen(FXint& sx,FXint& sy,const FXVec& e);
  FXVec screenToEye(FXint sx,FXint sy,FXfloat eyez=0.0);
  FXVec screenToTarget(FXint sx,FXint sy);
  FXVec worldToEye(const FXVec& w);
  FXfloat worldToEyeZ(const FXVec& w);
  FXVec eyeToWorld(const FXVec& e); 
  FXVec worldVector(FXint fx,FXint fy,FXint tx,FXint ty);
  void setFieldOfView(FXdouble fv);
  FXdouble getFieldOfView() const { return fov; }
  void setZoom(FXdouble zm);
  FXdouble getZoom() const { return zoom; }
  void setDistance(FXdouble ed);
  FXdouble getDistance() const { return distance; }
  void setOrientation(const FXQuat& rot);
  const FXQuat& getOrientation() const { return rotation; }
  void setCenter(const FXVec& cntr);
  const FXVec& getCenter() const { return center; }
  FXuint getStamp() const { return stamp; }
  void setStamp(FXuint stmp){ stamp=stmp; }
  void translate(const FXVec& vec);
  FXbool getBoreVector(FXint sx,FXint sy,FXVec& point,FXVec& dir);
  FXVec getEyeVector() const;
  FXVec getEyePosition() const;
  virtual void update(FXint x,FXint y,FXint w,FXint h);
  virtual FXbool canFocus() const;
  void setHelpText(const FXchar* text);
  const FXchar* getHelpText() const { return help; }
  void setTipText(const FXchar* text);
  const FXchar* getTipText() const { return tip; }
  const FXHMat& getTransform() const { return transform; }
  const FXHMat& getInvTransform() const { return itransform; }
  void setScene(FXGLObject* sc);
  FXGLObject* getScene() const { return scene; }
  void setSelection(FXGLObject* sel);
  FXGLObject* getSelection() const { return selection; }
  void setProjection(FXuint proj);
  FXuint getProjection() const { return projection; }
  virtual void save(FXStream& store) const;
  virtual void load(FXStream& store);
  virtual ~FXGLViewer();
  };


#endif

