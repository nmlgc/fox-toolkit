/********************************************************************************
*                                                                               *
*                           O p e n G L   V i e w e r                           *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXGLViewer.cpp,v 1.104.4.3 2003/06/20 19:02:07 fox Exp $                  *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxkeys.h"
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
#include "FXSettings.h"
#include "FXRegistry.h"
#include "FXAccelTable.h"
#include "FXObjectList.h"
#include "FXApp.h"
#include "FXVisual.h"
#include "FXGLVisual.h"
#include "FXDC.h"
#include "FXDCWindow.h"
#include "FXDCPrint.h"
#include "FXMessageBox.h"
#include "FXTooltip.h"
#include "FXCursor.h"
#include "FXGLContext.h"
#include "FXGLViewer.h"
#include "FXGLObject.h"
#include "FXPrintDialog.h"
#include "jitter.h"


/*
  To Do:
  - Initialize GL to fastest of everything for drawing lines.
  - Group object needs current element.
  - use app->getDragDelta() for motion tolerance.
  - Default op to noop mode; all returns 0 in noop mode.
  - GLuint unfortunately not always big enough to store a pointer...
  - The selection seems to get lost with multiple viewers into.
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
  - When object(s) deleted, need to fix up selection...
  - GLViewer should source some messages to its target for important events.
  - Zoom-lasso feature.
  - Basic mouse actions:

    State     Event      Modifiers         Where           State          Meaning
    --------------------------------------------------------------------------------------------
    HOVERING  Left       --                outside         PICKING        Pick an object if no move
    PICKING   Motion     --                --              ROTATING       Rotate camera about target point
    HOVERING  Left       --                inside object   DRAGGING       Drag object
    HOVERING  Left       Shift             --              LASSOSELECT    Select
    HOVERING  Left       Control           --              LASSOSELECT    Toggle selection
    HOVERING  Left       Right             --              ZOOMING        Zoom in
    HOVERING  Left       Right + Shift     --              TRUCKING       Trucking camera
    HOVERING  Middle     --                --              ZOOMING        Zoom in/out
    HOVERING  Middle     Shift             --              TRUCKING       Trucking camera
    HOVERING  Right      --                --              POSTING        Post popup menu if no move
    POSTING   Motion     --                --              TRANSLATING    Translate camera
    HOVERING  Right      Shift             --              GYRATING       Rotate object about camera
    HOVERING  Right      Control           --              FOVING         Change field of view
    HOVERING  Right      Left              --              ZOOMING        Zoom in
    HOVERING  Right      Left +Shift       --              TRUCKING       Trucking camera

*/

/*******************************************************************************/


// Size of pick buffer
#define MAX_PICKBUF    1024

// Maximum length of selection path
#define MAX_SELPATH    64

// Rotation tolerance
#define EPS            1.0E-2


/*******************************************************************************/


// Map
FXDEFMAP(FXGLViewer) FXGLViewerMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXGLViewer::onPaint),
  FXMAPFUNC(SEL_MOTION,0,FXGLViewer::onMotion),
  FXMAPFUNC(SEL_MOUSEWHEEL,0,FXGLViewer::onMouseWheel),
  FXMAPFUNC(SEL_TIMEOUT,FXGLViewer::ID_TIPTIMER,FXGLViewer::onTipTimer),
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
  FXMAPFUNC(SEL_UNGRABBED,0,FXGLViewer::onUngrabbed),
  FXMAPFUNC(SEL_KEYPRESS,0,FXGLViewer::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXGLViewer::onKeyRelease),
  FXMAPFUNC(SEL_FOCUSIN,0,FXGLViewer::onFocusIn),
  FXMAPFUNC(SEL_FOCUSOUT,0,FXGLViewer::onFocusOut),
  FXMAPFUNC(SEL_CHANGED,0,FXGLViewer::onChanged),
  FXMAPFUNC(SEL_CLICKED,0,FXGLViewer::onClicked),
  FXMAPFUNC(SEL_DOUBLECLICKED,0,FXGLViewer::onDoubleClicked),
  FXMAPFUNC(SEL_TRIPLECLICKED,0,FXGLViewer::onTripleClicked),
  FXMAPFUNC(SEL_LASSOED,0,FXGLViewer::onLassoed),
  FXMAPFUNC(SEL_SELECTED,0,FXGLViewer::onSelected),
  FXMAPFUNC(SEL_DESELECTED,0,FXGLViewer::onDeselected),
  FXMAPFUNC(SEL_INSERTED,0,FXGLViewer::onInserted),
  FXMAPFUNC(SEL_DELETED,0,FXGLViewer::onDeleted),
  FXMAPFUNC(SEL_PICKED,0,FXGLViewer::onPick),
  FXMAPFUNC(SEL_CLIPBOARD_LOST,0,FXGLViewer::onClipboardLost),
  FXMAPFUNC(SEL_CLIPBOARD_GAINED,0,FXGLViewer::onClipboardGained),
  FXMAPFUNC(SEL_CLIPBOARD_REQUEST,0,FXGLViewer::onClipboardRequest),
  FXMAPFUNCS(SEL_UPDATE,FXGLViewer::ID_DIAL_X,FXGLViewer::ID_DIAL_Z,FXGLViewer::onUpdXYZDial),
  FXMAPFUNCS(SEL_CHANGED,FXGLViewer::ID_DIAL_X,FXGLViewer::ID_DIAL_Z,FXGLViewer::onCmdXYZDial),
  FXMAPFUNCS(SEL_COMMAND,FXGLViewer::ID_DIAL_X,FXGLViewer::ID_DIAL_Z,FXGLViewer::onCmdXYZDial),
  FXMAPFUNCS(SEL_UPDATE,FXGLViewer::ID_ROLL,FXGLViewer::ID_YAW,FXGLViewer::onUpdRollPitchYaw),
  FXMAPFUNCS(SEL_COMMAND,FXGLViewer::ID_ROLL,FXGLViewer::ID_YAW,FXGLViewer::onCmdRollPitchYaw),
  FXMAPFUNCS(SEL_CHANGED,FXGLViewer::ID_ROLL,FXGLViewer::ID_YAW,FXGLViewer::onCmdRollPitchYaw),
  FXMAPFUNCS(SEL_UPDATE,FXGLViewer::ID_SCALE_X,FXGLViewer::ID_SCALE_Z,FXGLViewer::onUpdXYZScale),
  FXMAPFUNCS(SEL_COMMAND,FXGLViewer::ID_SCALE_X,FXGLViewer::ID_SCALE_Z,FXGLViewer::onCmdXYZScale),
  FXMAPFUNCS(SEL_CHANGED,FXGLViewer::ID_SCALE_X,FXGLViewer::ID_SCALE_Z,FXGLViewer::onCmdXYZScale),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_TIP,FXGLViewer::onQueryTip),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_HELP,FXGLViewer::onQueryHelp),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_PERSPECTIVE,FXGLViewer::onUpdPerspective),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_PERSPECTIVE,FXGLViewer::onCmdPerspective),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_PARALLEL,FXGLViewer::onUpdParallel),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_PARALLEL,FXGLViewer::onCmdParallel),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_FRONT,FXGLViewer::onUpdFront),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_FRONT,FXGLViewer::onCmdFront),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_BACK,FXGLViewer::onUpdBack),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_BACK,FXGLViewer::onCmdBack),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_LEFT,FXGLViewer::onUpdLeft),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_LEFT,FXGLViewer::onCmdLeft),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_RIGHT,FXGLViewer::onUpdRight),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_RIGHT,FXGLViewer::onCmdRight),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_TOP,FXGLViewer::onUpdTop),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_TOP,FXGLViewer::onCmdTop),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_BOTTOM,FXGLViewer::onUpdBottom),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_BOTTOM,FXGLViewer::onCmdBottom),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_RESETVIEW,FXWindow::onUpdYes),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_RESETVIEW,FXGLViewer::onCmdResetView),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_FITVIEW,FXWindow::onUpdYes),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_FITVIEW,FXGLViewer::onCmdFitView),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_CUT_SEL,FXGLViewer::onUpdDeleteSel),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_CUT_SEL,FXGLViewer::onCmdCutSel),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_COPY_SEL,FXGLViewer::onUpdCurrent),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_COPY_SEL,FXGLViewer::onCmdCopySel),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_PASTE_SEL,FXGLViewer::onUpdYes),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_PASTE_SEL,FXGLViewer::onCmdPasteSel),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_DELETE_SEL,FXGLViewer::onUpdDeleteSel),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_DELETE_SEL,FXGLViewer::onCmdDeleteSel),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_BACK_COLOR,FXGLViewer::onUpdBackColor),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_BACK_COLOR,FXGLViewer::onCmdBackColor),
  FXMAPFUNC(SEL_CHANGED,FXGLViewer::ID_BACK_COLOR,FXGLViewer::onCmdBackColor),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_AMBIENT_COLOR,FXGLViewer::onUpdAmbientColor),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_AMBIENT_COLOR,FXGLViewer::onCmdAmbientColor),
  FXMAPFUNC(SEL_CHANGED,FXGLViewer::ID_AMBIENT_COLOR,FXGLViewer::onCmdAmbientColor),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_LOCK,FXGLViewer::onUpdLock),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_LOCK,FXGLViewer::onCmdLock),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_LIGHTING,FXGLViewer::onUpdLighting),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_LIGHTING,FXGLViewer::onCmdLighting),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_FOG,FXGLViewer::onUpdFog),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_DITHER,FXGLViewer::onUpdDither),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_LIGHT_AMBIENT,FXGLViewer::onUpdLightAmbient),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_LIGHT_AMBIENT,FXGLViewer::onCmdLightAmbient),
  FXMAPFUNC(SEL_CHANGED,FXGLViewer::ID_LIGHT_AMBIENT,FXGLViewer::onCmdLightAmbient),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_LIGHT_DIFFUSE,FXGLViewer::onUpdLightDiffuse),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_LIGHT_DIFFUSE,FXGLViewer::onCmdLightDiffuse),
  FXMAPFUNC(SEL_CHANGED,FXGLViewer::ID_LIGHT_DIFFUSE,FXGLViewer::onCmdLightDiffuse),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_LIGHT_SPECULAR,FXGLViewer::onUpdLightSpecular),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_LIGHT_SPECULAR,FXGLViewer::onCmdLightSpecular),
  FXMAPFUNC(SEL_CHANGED,FXGLViewer::ID_LIGHT_SPECULAR,FXGLViewer::onCmdLightSpecular),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_TURBO,FXGLViewer::onUpdTurbo),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_TURBO,FXGLViewer::onCmdTurbo),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_PRINT_IMAGE,FXGLViewer::onUpdYes),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_PRINT_VECTOR,FXGLViewer::onUpdYes),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_LASSO_ZOOM,FXGLViewer::onUpdYes),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_LASSO_ZOOM,FXGLViewer::onCmdLassoZoom),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_LASSO_SELECT,FXGLViewer::onUpdYes),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_LASSO_SELECT,FXGLViewer::onCmdLassoSelect),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_FOG,FXGLViewer::onCmdFog),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_DITHER,FXGLViewer::onCmdDither),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_FOV,FXGLViewer::onUpdFov),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_FOV,FXGLViewer::onCmdFov),
  FXMAPFUNC(SEL_CHANGED,FXGLViewer::ID_FOV,FXGLViewer::onCmdFov),
  FXMAPFUNC(SEL_UPDATE,FXGLViewer::ID_ZOOM,FXGLViewer::onUpdZoom),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_ZOOM,FXGLViewer::onCmdZoom),
  FXMAPFUNC(SEL_CHANGED,FXGLViewer::ID_ZOOM,FXGLViewer::onCmdZoom),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_PRINT_IMAGE,FXGLViewer::onCmdPrintImage),
  FXMAPFUNC(SEL_COMMAND,FXGLViewer::ID_PRINT_VECTOR,FXGLViewer::onCmdPrintVector),
  };


// Object implementation
FXIMPLEMENT(FXGLViewer,FXGLCanvas,FXGLViewerMap,ARRAYNUMBER(FXGLViewerMap))


/*******************************************************************************/

// Drag type names for generic object
const FXchar FXGLViewer::objectTypeName[]="application/x-globject";


// Drag type for generic object
FXDragType FXGLViewer::objectType=0;


/*******************************************************************************/


// For deserialization
FXGLViewer::FXGLViewer(){
  flags|=FLAG_ENABLED|FLAG_DROPTARGET;
  dial[0]=0;
  dial[1]=0;
  dial[2]=0;
  timer=NULL;
  dropped=NULL;
  selection=NULL;
  zsortfunc=NULL;
  doesturbo=FALSE;
  op=HOVERING;
  }


// Construct GL viewer widget with private display list
FXGLViewer::FXGLViewer(FXComposite* p,FXGLVisual *vis,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXGLCanvas(p,vis,NULL,tgt,sel,opts,x,y,w,h){
  initialize();
  }


// Construct GL viewer widget with shared display list
FXGLViewer::FXGLViewer(FXComposite* p,FXGLVisual *vis,FXGLViewer* sharegroup,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXGLCanvas(p,vis,sharegroup,tgt,sel,opts,x,y,w,h){
  initialize();
  }


// Common initialization for constructor
void FXGLViewer::initialize(){
  flags|=FLAG_ENABLED|FLAG_DROPTARGET;
  defaultCursor=getApp()->getDefaultCursor(DEF_CROSSHAIR_CURSOR);
  dragCursor=getApp()->getDefaultCursor(DEF_CROSSHAIR_CURSOR);
  projection=PERSPECTIVE;                       // Projection
  zoom=1.0;                                     // Zoom factor
  offset=0.004;                                 // Offset for lines on surfaces
  fov=30.0;                                     // Field of view (1 to 90)
  wvt.left=-1.0;                                // Init world box
  wvt.right=1.0;
  wvt.top=1.0;
  wvt.bottom=-1.0;
  wvt.hither=0.1;
  wvt.yon=1.0;
  wvt.w=100;                                    // Viewport width
  wvt.h=100;                                    // Viewport height
  diameter=2.0;                                 // Size of model
  distance=7.464116;                            // Distance of PRP to target
  rotation=FXQuat(0.0f,0.0f,0.0f,1.0f);         // Orientation
  center=FXVec(0.0f,0.0f,0.0f);                 // Model center
  scale=FXVec(1.0f,1.0f,1.0f);                  // Model scaling
  updateProjection();                           // Initial projection
  updateTransform();                            // Set transformation
  op=HOVERING;                                  // Mouse operation
  maxhits=512;                                  // Maximum hit buffer size
  background=getApp()->getBackColor();          // Background copied from GUI background
  ambient=FXHVec(0.2f,0.2f,0.2f,1.0);           // Scene ambient

  light.ambient=FXHVec(0.0f,0.0f,0.0f,1.0f);    // Light setup
  light.diffuse=FXHVec(1.0f,1.0f,1.0f,1.0f);
  light.specular=FXHVec(0.0f,0.0f,0.0f,1.0f);
  light.position=FXHVec(-2.0f,2.0f,3.0f,0.0f);
  light.direction=FXVec(0.0f,0.0f,-1.0f);
  light.exponent=0.0f;
  light.cutoff=180.0f;
  light.c_attn=1.0f;
  light.l_attn=0.0f;
  light.q_attn=0.0f;

  material.ambient=FXHVec(0.2f,0.2f,0.2f,1.0f); // Material setup
  material.diffuse=FXHVec(0.8f,0.8f,0.8f,1.0f);
  material.specular=FXHVec(1.0f,1.0f,1.0f,1.0f);
  material.emission=FXHVec(0.0f,0.0f,0.0f,1.0f);
  material.shininess=30.0f;

  dial[0]=0;                                    // Old dial position
  dial[1]=0;                                    // Old dial position
  dial[2]=0;                                    // Old dial position

  doesturbo=FALSE;                              // In interaction
  turbomode=FALSE;                              // Turbo mode
  timer=NULL;                                   // Motion timer
  dropped=NULL;                                 // Nobody being dropped on
  selection=NULL;                               // No initial selection
  zsortfunc=NULL;                               // Routine to sort feedback buffer
  scene=NULL;                                   // Scene to look at
  }


// Create window
void FXGLViewer::create(){
  FXRange r(-1.0f,1.0f,-1.0f,1.0f,-1.0f,1.0f);

  // We would like to have this be true
#ifdef HAVE_OPENGL
  FXASSERT(sizeof(FXuint)==sizeof(GLuint));
#endif

  // Create Window
  FXGLCanvas::create();

  // Set up OpenGL environment
  glsetup();

  // Register drag type for color
  if(!colorType){colorType=getApp()->registerDragType(colorTypeName);}
  if(!objectType){objectType=getApp()->registerDragType(objectTypeName);}

  // If have scene already, get correct bounds
  if(scene) scene->bounds(r);

  // Set initial viewing volume
  setBounds(r);
  }


// Detach window
void FXGLViewer::detach(){
  FXGLCanvas::detach();
  colorType=0;
  objectType=0;
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

    // Viewer is close
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,TRUE);

    // Material colors
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,material.ambient);
    glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,material.diffuse);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,material.specular);
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,material.emission);
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,material.shininess);

    // Vertex colors change both diffuse and ambient
    glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
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
    makeNonCurrent();
    }
#endif
  }


// Render all the graphics into a world box
void FXGLViewer::drawWorld(FXViewport& wv){
#ifdef HAVE_OPENGL

  // Set viewport
  glViewport(0,0,wv.w,wv.h);

  // Clear background
  glClearDepth(1.0);
  glClearColor(background[0],background[1],background[2],1.0f);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  // Set OFFSET Projection Matrix
  glNewList(FXGLViewer::OFFSETPROJECTION,GL_COMPILE);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glTranslatef(0.0f,0.0f,(GLfloat)(-offset/zoom));    // FIXME when fov becomes very small this needs to be corrected!!!
  switch(projection){
    case PARALLEL: glOrtho(wv.left,wv.right,wv.bottom,wv.top,wv.hither,wv.yon); break;
    case PERSPECTIVE: glFrustum(wv.left,wv.right,wv.bottom,wv.top,wv.hither,wv.yon); break;
    }
  glMatrixMode(GL_MODELVIEW);
  glEndList();

  // Set SURFACE Projection Matrix
  glNewList(FXGLViewer::SURFACEPROJECTION,GL_COMPILE);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  switch(projection){
    case PARALLEL: glOrtho(wv.left,wv.right,wv.bottom,wv.top,wv.hither,wv.yon); break;
    case PERSPECTIVE: glFrustum(wv.left,wv.right,wv.bottom,wv.top,wv.hither,wv.yon); break;
    }
  glMatrixMode(GL_MODELVIEW);
  glEndList();

  // Set SURFACE Projection Matrix
  glCallList(FXGLViewer::SURFACEPROJECTION);

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

  // Default material colors
  glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,material.ambient);
  glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,material.diffuse);
  glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,material.specular);
  glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,material.emission);
  glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,material.shininess);

  // Color commands change both
  glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);

  // Track material
  glDisable(GL_COLOR_MATERIAL);

  // Global ambient light
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ambient);

  // Enable lighting
  if(options&VIEWER_LIGHTING)
    glEnable(GL_LIGHTING);
  else
    glDisable(GL_LIGHTING);

  // Enable fog
  if(options&VIEWER_FOG){
    glEnable(GL_FOG);
    glFogfv(GL_FOG_COLOR,background);         // Disappear into the background
    //glFogf(GL_FOG_DENSITY,1.0f);
    glFogf(GL_FOG_START,(GLfloat)(distance-diameter));   // Range tight around model position
    glFogf(GL_FOG_END,(GLfloat)(distance+diameter));     // Far place same as clip plane:- clipped stuff is in the mist!
    glFogi(GL_FOG_MODE,GL_LINEAR);	// Simple linear depth cueing
    }
  else{
    glDisable(GL_FOG);
    }

  // Dithering
  if(options&VIEWER_DITHER)
    glEnable(GL_DITHER);
  else
    glDisable(GL_DITHER);

  // Shade model
  glShadeModel(GL_SMOOTH);

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
  glClearAccum(0.0f,0.0f,0.0f,0.0f);
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


// Fill select buffer with hits in rectangle
FXint FXGLViewer::selectHits(FXuint*& hits,FXint& nhits,FXint x,FXint y,FXint w,FXint h){
  register FXfloat pickx,picky,pickw,pickh;
  register FXint mh=maxhits;
  hits=NULL;
  nhits=0;
#ifdef HAVE_OPENGL
  if(makeCurrent()){

    // Where to pick
    pickx=(wvt.w-2.0f*x-w)/((FXfloat)w);
    picky=(2.0f*y+h-wvt.h)/((FXfloat)h);
    pickw=wvt.w/((FXfloat)w);
    pickh=wvt.h/((FXfloat)h);

    // Set OFFSET Pick Matrix
    glNewList(FXGLViewer::OFFSETPROJECTION,GL_COMPILE);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glTranslatef(pickx,picky,(GLfloat)(-offset/zoom));   // Slightly toward eye; one wants to pick lines first!
    glScalef(pickw,pickh,1.0f);
    switch(projection){
      case PARALLEL: glOrtho(wvt.left,wvt.right,wvt.bottom,wvt.top,wvt.hither,wvt.yon); break;
      case PERSPECTIVE: glFrustum(wvt.left,wvt.right,wvt.bottom,wvt.top,wvt.hither,wvt.yon); break;
      }
    glMatrixMode(GL_MODELVIEW);
    glEndList();

    // Set SURFACE Pick Matrix
    glNewList(FXGLViewer::SURFACEPROJECTION,GL_COMPILE);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glTranslatef(pickx,picky,0.0f);
    glScalef(pickw,pickh,1.0f);
    switch(projection){
      case PARALLEL: glOrtho(wvt.left,wvt.right,wvt.bottom,wvt.top,wvt.hither,wvt.yon); break;
      case PERSPECTIVE: glFrustum(wvt.left,wvt.right,wvt.bottom,wvt.top,wvt.hither,wvt.yon); break;
      }
    glMatrixMode(GL_MODELVIEW);
    glEndList();

    // Set SURFACE Pick Matrix
    glCallList(FXGLViewer::SURFACEPROJECTION);

    // Model Matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(transform);

    // Loop until room enough to fit
    do{
      nhits=0;
      if(!FXRESIZE(&hits,FXuint,mh)) break;
      glSelectBuffer(mh,(GLuint*)hits);
      glRenderMode(GL_SELECT);
      glInitNames();
      glPushName(0);
      if(scene) scene->hit(this);
      glPopName();
      nhits=glRenderMode(GL_RENDER);
      mh<<=1;
      }
    while(nhits<0);
    makeNonCurrent();
    if(!nhits) FXFREE(&hits);
    return nhits;
    }
#endif
  return 0;
  }


// Process picks
FXGLObject* FXGLViewer::processHits(FXuint *pickbuffer,FXint nhits){
  FXuint d1,d2,i,n,zmin,zmax,sel=0;
  if(0<=nhits){
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
    return scene->identify(&pickbuffer[4+sel]);
    }
  return NULL;
  }


// Build NULL-terminated list of ALL picked objects overlapping rectangle
FXGLObject** FXGLViewer::select(FXint x,FXint y,FXint w,FXint h){
  FXGLObject *obj,**objects=NULL;
  FXint nhits,i,j;
  FXuint *hits;
  if(scene && maxhits){
    if(selectHits(hits,nhits,x,y,w,h)){     // FIXME leak
      FXMALLOC(&objects,FXGLObject*,nhits+1);
      for(i=j=0; nhits>0; i+=hits[i]+3,nhits--){
        if((obj=scene->identify(&hits[4+i]))!=NULL) objects[j++]=obj;
        }
      objects[j]=NULL;
      FXFREE(&hits);
      }
    }
  return objects;
  }


// Lasso objects
FXGLObject** FXGLViewer::lasso(FXint x1,FXint y1,FXint x2,FXint y2){
  FXint xlo,xhi,ylo,yhi;
  FXMINMAX(xlo,xhi,x1,x2);
  FXMINMAX(ylo,yhi,y1,y2);
  return select(xlo,ylo,xhi-xlo+1,yhi-ylo+1);
  }


// Pick ONE object at x,y
FXGLObject* FXGLViewer::pick(FXint x,FXint y){
  FXGLObject *obj=NULL;
  FXuint *hits;
  FXint nhits;
  if(scene && maxhits){
    if(selectHits(hits,nhits,x-PICK_TOL,y-PICK_TOL,PICK_TOL*2,PICK_TOL*2)){     // FIXME leak
      obj=processHits(hits,nhits);
      FXFREE(&hits);
      }
    }
  return obj;
  }


// Repaint the GL window
long FXGLViewer::onPaint(FXObject*,FXSelector,void*){
#ifdef HAVE_OPENGL
  FXGLVisual *vis=(FXGLVisual*)getVisual();
  FXASSERT(xid);
  if(makeCurrent()){
    drawWorld(wvt);
    if(vis->isDoubleBuffer()) swapBuffers();
    makeNonCurrent();
    }
#endif
  return 1;
  }


// Handle configure notify
void FXGLViewer::layout(){
  wvt.w=width;
  wvt.h=height;
  updateProjection();
  flags&=~FLAG_DIRTY;
  }


// Start motion timer while in this window
long FXGLViewer::onEnter(FXObject* sender,FXSelector sel,void* ptr){
  FXGLCanvas::onEnter(sender,sel,ptr);
  if(isEnabled()){
    if(!timer){timer=getApp()->addTimeout(getApp()->getMenuPause(),this,ID_TIPTIMER);}
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
    update();
    }
  return 1;
  }


// Lost focus
long FXGLViewer::onFocusOut(FXObject* sender,FXSelector sel,void* ptr){
  FXGLCanvas::onFocusOut(sender,sel,ptr);
  if(selection && selection->handle(this,MKUINT(0,SEL_FOCUSOUT),ptr)){
    update();
    }
  return 1;
  }


// Change scene
void FXGLViewer::setScene(FXGLObject* sc){
  scene=sc;
  update();
  }


// Change field of view
void FXGLViewer::setFieldOfView(FXdouble fv){
  FXdouble tn;
  fov=FXCLAMP(2.0,fv,90.0);
  tn=tan(0.5*DTOR*fov);
  FXASSERT(tn>0.0);
  distance=diameter/tn;
  FXASSERT(distance>0.0);
  updateProjection();
  updateTransform();
  update();
  }


// Change eye distance
void FXGLViewer::setDistance(FXdouble d){
  if(d<diameter) d=diameter;
  if(d>114.0*diameter) d=114.0*diameter;
  if(d!=distance){
    distance=d;
    FXASSERT(distance>0.0);
    fov=2.0*RTOD*atan2(diameter,distance);
    updateProjection();
    updateTransform();
    update();
    }
  }


// Change zoom factor
void FXGLViewer::setZoom(FXdouble zm){
  if(zm<1.0E-30) zm=1.0E-30;
  if(zm!=zoom){
    zoom=zm;
    updateProjection();
    update();
    }
  }


// Change scale factors
void FXGLViewer::setScale(FXVec s){
  if(s[0]<=0.000001f) s[0]=0.000001f;
  if(s[1]<=0.000001f) s[1]=0.000001f;
  if(s[2]<=0.000001f) s[2]=0.000001f;
  if(scale!=s){
    scale=s;
    updateTransform();
    update();
    }
  }


// Change orientation to new quaternion
void FXGLViewer::setOrientation(FXQuat rot){
  if(rot!=rotation){
    rotation=rot;
    rotation.adjust();
    updateTransform();
    update();
    }
  }


// Change world projection
void FXGLViewer::updateProjection(){
  FXdouble hither_fac,r,aspect;

  // Should be non-0 size viewport
  if(wvt.w>0 && wvt.h>0){

    // Aspect ratio of viewer
    aspect = (FXdouble)wvt.h / (FXdouble)wvt.w;

    // Get world box
    r=0.5*diameter/zoom;
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
    //wvt.hither=distance-diameter;
  //  if(wvt.hither<distance-diameter) wvt.hither=distance-diameter;

    // New window
    FXTRACE((100,"wvt.left=%g wvt.right=%g wvt.top=%g wvt.bottom=%g wvt.hither=%g wvt.yon=%g\n",wvt.left,wvt.right,wvt.top,wvt.bottom,wvt.hither,wvt.yon));

    // Size of a pixel in world and model
    worldpx=(wvt.right-wvt.left)/wvt.w;
    modelpx=worldpx*diameter;

    // Precalc stuff for view->world backmapping
    ax=wvt.left;
    ay=wvt.top-worldpx;

    // Report factors
    FXTRACE((100,"worldpx=%g modelpx=%g\n",worldpx,modelpx));

    // Correction for perspective
    if(projection==PERSPECTIVE){
      FXASSERT(distance>0.0);
      hither_fac=wvt.hither/distance;
      wvt.left*=hither_fac;
      wvt.right*=hither_fac;
      wvt.top*=hither_fac;
      wvt.bottom*=hither_fac;
      }
    }
  }


// Change transformation matrix
void FXGLViewer::updateTransform(){
  transform.eye();
  transform.trans(0.0f,0.0f,(FXfloat)-distance);
  transform.rot(rotation);
  transform.scale(scale);
  transform.trans(-center);
  itransform=invert(transform);
//   FXTRACE((150,"itrans=%11.8f %11.8f %11.8f %11.8f\n",itransform[0][0],itransform[0][1],itransform[0][2],itransform[0][3]));
//   FXTRACE((150,"       %11.8f %11.8f %11.8f %11.8f\n",itransform[1][0],itransform[1][1],itransform[1][2],itransform[1][3]));
//   FXTRACE((150,"       %11.8f %11.8f %11.8f %11.8f\n",itransform[2][0],itransform[2][1],itransform[2][2],itransform[2][3]));
//   FXTRACE((150,"       %11.8f %11.8f %11.8f %11.8f\n",itransform[3][0],itransform[3][1],itransform[3][2],itransform[3][3]));
//   FXTRACE((150,"\n"));
//   FXTRACE((150," trans=%11.8f %11.8f %11.8f %11.8f\n",transform[0][0],transform[0][1],transform[0][2],transform[0][3]));
//   FXTRACE((150,"       %11.8f %11.8f %11.8f %11.8f\n",transform[1][0],transform[1][1],transform[1][2],transform[1][3]));
//   FXTRACE((150,"       %11.8f %11.8f %11.8f %11.8f\n",transform[2][0],transform[2][1],transform[2][2],transform[2][3]));
//   FXTRACE((150,"       %11.8f %11.8f %11.8f %11.8f\n",transform[3][0],transform[3][1],transform[3][2],transform[3][3]));
//   FXTRACE((150,"\n"));
//   FXHMat check=itransform*transform;
//   FXTRACE((150," check=%11.8f %11.8f %11.8f %11.8f\n",check[0][0],check[0][1],check[0][2],check[0][3]));
//   FXTRACE((150,"       %11.8f %11.8f %11.8f %11.8f\n",check[1][0],check[1][1],check[1][2],check[1][3]));
//   FXTRACE((150,"       %11.8f %11.8f %11.8f %11.8f\n",check[2][0],check[2][1],check[2][2],check[2][3]));
//   FXTRACE((150,"       %11.8f %11.8f %11.8f %11.8f\n",check[3][0],check[3][1],check[3][2],check[3][3]));
//   FXTRACE((150,"\n"));
  }


// Set model bounding box
FXbool FXGLViewer::setBounds(const FXRange& r){
//   FXTRACE((100,"xlo=%g xhi=%g ylo=%g yhi=%g zlo=%g zhi=%g\n",r[0][0],r[0][1],r[1][0],r[1][1],r[2][0],r[2][1]));

  // Model center
  center=boxCenter(r);

  // Model size
  diameter=r.longest();

  // Fix zero size model
  if(diameter<1.0E-30) diameter=1.0;

  // Set equal scaling initially
  scale=FXVec(1.0f,1.0f,1.0f);

  // Reset distance (and thus field of view)
  setDistance(1.1*diameter);

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


// Obtain viewport
void FXGLViewer::getViewport(FXViewport& v) const {
  v=wvt;
  }


// Set material
void FXGLViewer::setMaterial(const FXMaterial &mtl){
  material=mtl;
  update();
  }


// Get material
void FXGLViewer::getMaterial(FXMaterial &mtl) const {
  mtl=material;
  }


// Get screen point from eye coordinate
void FXGLViewer::eyeToScreen(FXint& sx,FXint& sy,FXVec e){
  register double xp,yp;
  if(projection==PERSPECTIVE){
    if(e[2]==0.0f){ fxerror("%s::eyeToScreen: cannot transform point.\n",getClassName()); }
    xp=-distance*e[0]/e[2];
    yp=-distance*e[1]/e[2];
    }
  else{
    xp=e[0];
    yp=e[1];
    }
  sx=(int)((xp-ax)/worldpx);
  sy=(int)((ay-yp)/worldpx);
  }


// Convert screen point to eye coords
FXVec FXGLViewer::screenToEye(FXint sx,FXint sy,FXfloat eyez){
  register float xp,yp;
  FXVec e;
  xp=(float)(worldpx*sx+ax);
  yp=(float)(ay-worldpx*sy);
  if(projection==PERSPECTIVE){
    FXASSERT(distance>0.0);
    e[0]=(FXfloat)(-eyez*xp/distance);
    e[1]=(FXfloat)(-eyez*yp/distance);
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
  return FXVec((FXfloat)(worldpx*sx+ax), (FXfloat)(ay-worldpx*sy), (FXfloat)-distance);
  }


// Convert world to eye coords
FXVec FXGLViewer::worldToEye(FXVec w){
  return w*transform;
  }


// Get eye Z-coordinate of world point
FXfloat FXGLViewer::worldToEyeZ(FXVec w){
  return w[0]*transform[0][2]+w[1]*transform[1][2]+w[2]*transform[2][2]+transform[3][2];
  }


// Convert eye to world coords
FXVec FXGLViewer::eyeToWorld(FXVec e){
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
  p=eyeToWorld(screenToEye(sx,sy,(FXfloat)(diameter-distance)));
  if(PARALLEL==projection)
    point=eyeToWorld(screenToEye(sx,sy,0.0f));
  else
    point=eyeToWorld(FXVec(0.0f,0.0f,0.0f));
  dir=p-point;
  d=len(dir);
  if(0.0<d) dir/=(FXfloat)d;         // normalize
  return TRUE;
  }


// Get eye viewing direction (non-normalized)
FXVec FXGLViewer::getEyeVector() const {
  return FXVec(-itransform[2][0],-itransform[2][1],-itransform[2][2]);
  }


// Get eye position
FXVec FXGLViewer::getEyePosition() const{
  return FXVec(itransform[3][0],itransform[3][1],itransform[3][2]);
  }


// Change model center
void FXGLViewer::setCenter(FXVec cntr){
  if(center!=cntr){
    center=cntr;
    updateTransform();
    update();
    }
  }


// Translate in world
void FXGLViewer::translate(FXVec vec){
  center+=vec;
  updateTransform();
  update();
  }


// Change selection
void FXGLViewer::setSelection(FXGLObject* sel){
  if(selection!=sel){
    selection=sel;
    update();
    }
  }


// Change help text
void FXGLViewer::setHelpText(const FXString& text){
  help=text;
  }


// Change tip text
void FXGLViewer::setTipText(const FXString& text){
  tip=text;
  }


// Translate point into unit-sphere coordinate
FXVec FXGLViewer::spherePoint(FXint px,FXint py){
  FXdouble d,t,screenmin;
  FXVec v;
  if(wvt.w>wvt.h)
    screenmin=wvt.h;
  else
    screenmin=wvt.w;
  FXASSERT(screenmin>0.0);
  v[0]=(FXfloat)(2.0*(px-0.5*wvt.w)/screenmin);
  v[1]=(FXfloat)(2.0*(0.5*wvt.h-py)/screenmin);
  d=v[0]*v[0]+v[1]*v[1];
  if(d<0.75){
    v[2]=(FXfloat)sqrt(1.0-d);
    }
  else if(d<3.0){
    d=1.7320508008-sqrt(d);
    t=1.0-d*d;
    if(t<0.0) t=0.0;
    v[2]=(FXfloat)(1.0-sqrt(t));
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
  q[3]=(FXfloat)sqrt(t);
  return q;
  }


// Draw non-destructive lasso box; drawing twice will erase it again
void FXGLViewer::drawLasso(FXint x0,FXint y0,FXint x1,FXint y1){
#ifdef HAVE_OPENGL
  FXGLVisual *vis=(FXGLVisual*)getVisual();

  // With OpenGL, first change back to 1:1 projection mode
  if(makeCurrent()){

    // Save state
    glPushAttrib(GL_COLOR_BUFFER_BIT|GL_ENABLE_BIT|GL_DEPTH_BUFFER_BIT|GL_LINE_BIT);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();

    // Fix xform
    glLoadIdentity();
    glOrtho(0.0,width-1.0,0.0,height-1.0,0.0,1.0);

    // Draw in front buffer, so we see it w/o blowing
    // away the drawing by calling swapBuffers.
    if(vis->isDoubleBuffer()) glDrawBuffer(GL_FRONT);

    // Fancy stuff off
    glLineWidth(1.0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_LIGHTING);
    glShadeModel(GL_FLAT);
    glDepthMask(FALSE);
    glDisable(GL_DITHER);

    // Windows
#ifdef WIN32

#ifndef _ALPHA_
    // MS-Windows has logic ops, and they seem to work:- at least
    // with the unaccelerated software rendering they seem to...
    glEnable(GL_COLOR_LOGIC_OP);
    glLogicOp(GL_INVERT);

#else
    // ALPHA CPU's don't have logic ops; or at least they're broken :-(
    glBlendFunc(GL_ONE_MINUS_DST_COLOR,GL_ZERO);
    glEnable(GL_BLEND);

#endif

    // UNIX
#else
#if !defined(GL_VERSION_1_1) || !defined(GL_VERSION_1_2)

    // If you don't have OpenGL 1.1 or better, blending
    // to invert the lasso is your only choice...
    glBlendFunc(GL_ONE_MINUS_DST_COLOR,GL_ZERO);
    glEnable(GL_BLEND);

#else

    // You have OpenGL 1.1 or better, but chances are it
    // still doesn't work, because you may have an incomplete
    // implementation [DEC], or perhaps broken hardware.

    // If it works for you, uncomment the lines below,
    // and comment the ones above...
    glEnable(GL_COLOR_LOGIC_OP);
    glLogicOp(GL_INVERT);
#endif
#endif

    glBegin(GL_LINE_LOOP);
    glColor4ub(255,255,255,255);
    glVertex2i(x0,wvt.h-y0-1);
    glVertex2i(x0,wvt.h-y1-1);
    glVertex2i(x1,wvt.h-y1-1);
    glVertex2i(x1,wvt.h-y0-1);
    glEnd();
    glFinish();

    // Restore to old state
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
    makeNonCurrent();
    }
#endif
  }

/*************************  Mouse Actions in Viewer  ***************************/


// Current object changed
long FXGLViewer::onChanged(FXObject*,FXSelector,void* ptr){
  setSelection((FXGLObject*)ptr);
  if(target) target->handle(this,MKUINT(message,SEL_CHANGED),ptr);
  return 1;
  }


// Message that indicates where user picked
long FXGLViewer::onPick(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_PICKED),ptr);
  }


// Clicked in widget
long FXGLViewer::onClicked(FXObject*,FXSelector,void* ptr){
  if(target){
    if(target->handle(this,MKUINT(message,SEL_CLICKED),ptr)) return 1;
    if(ptr && target->handle(this,MKUINT(message,SEL_COMMAND),ptr)) return 1;
    }
  return 1;
  }


// Double clicked in widget; ptr may or may not point to an object
long FXGLViewer::onDoubleClicked(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_DOUBLECLICKED),ptr);
  }


// Triple clicked in widget; ptr may or may not point to an object
long FXGLViewer::onTripleClicked(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_TRIPLECLICKED),ptr);
  }


// Lassoed object(s)
long FXGLViewer::onLassoed(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXGLObject **objlist;

  // Notify target of lasso
  if(target && target->handle(this,MKUINT(message,SEL_LASSOED),ptr)) return 1;

  // Grab all objects in lasso
  objlist=lasso(event->click_x,event->click_y,event->win_x,event->win_y);

  // Add selection mode
  if(event->state&SHIFTMASK){
    handle(this,MKUINT(0,SEL_SELECTED),(void*)objlist);
    }

  // Toggle selection mode
  else if(event->state&CONTROLMASK){
    handle(this,MKUINT(0,SEL_DESELECTED),(void*)objlist);
    }

  // Free list
  FXFREE(&objlist);

  return 1;
  }


// Selected object(s)
long FXGLViewer::onSelected(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_SELECTED),ptr);
  }


// Deselected object(s)
long FXGLViewer::onDeselected(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_DESELECTED),ptr);
  }


// Inserted object(s)
long FXGLViewer::onInserted(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_INSERTED),ptr);
  }


// Deleted object(s)
long FXGLViewer::onDeleted(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_DELETED),ptr);
  }


// Change operation
void FXGLViewer::setOp(FXuint o){
  if(op!=o){
    switch(o){
      case HOVERING:
        setDragCursor(getDefaultCursor());
        FXTRACE((100,"HOVERING\n"));
        if(doesturbo) update();
        doesturbo=FALSE;
        break;
      case PICKING:
        FXTRACE((100,"PICKING\n"));
        setDragCursor(getDefaultCursor());
        break;
      case ROTATING:
        FXTRACE((100,"ROTATING\n"));
        doesturbo=turbomode;
        setDragCursor(getApp()->getDefaultCursor(DEF_ROTATE_CURSOR));
        break;
      case POSTING:
        FXTRACE((100,"POSTING\n"));
        setDragCursor(getDefaultCursor());
        break;
      case TRANSLATING:
        FXTRACE((100,"TRANSLATING\n"));
        doesturbo=turbomode;
        setDragCursor(getApp()->getDefaultCursor(DEF_MOVE_CURSOR));
        break;
      case ZOOMING:
        FXTRACE((100,"ZOOMING\n"));
        doesturbo=turbomode;
        setDragCursor(getApp()->getDefaultCursor(DEF_DRAGH_CURSOR));
        break;
      case FOVING:
        FXTRACE((100,"FOVING\n"));
        doesturbo=turbomode;
        setDragCursor(getApp()->getDefaultCursor(DEF_DRAGH_CURSOR));
        break;
      case DRAGGING:
        FXTRACE((100,"DRAGGING\n"));
        doesturbo=turbomode;
        setDragCursor(getApp()->getDefaultCursor(DEF_MOVE_CURSOR));
        break;
      case TRUCKING:
        FXTRACE((100,"TRUCKING\n"));
        doesturbo=turbomode;
        setDragCursor(getApp()->getDefaultCursor(DEF_DRAGH_CURSOR));
        break;
      case GYRATING:
        FXTRACE((100,"GYRATING\n"));
        doesturbo=turbomode;
        setDragCursor(getApp()->getDefaultCursor(DEF_ROTATE_CURSOR));
        break;
      case DO_LASSOSELECT:
        if(op==LASSOSELECT) return;
        FXTRACE((100,"LASSOSELECT\n"));
        setDefaultCursor(getApp()->getDefaultCursor(DEF_CORNERNW_CURSOR));
        break;
      case LASSOSELECT:
        FXTRACE((100,"LASSOSELECT\n"));
        setDefaultCursor(getDragCursor());
        setDragCursor(getApp()->getDefaultCursor(DEF_CORNERNW_CURSOR));
        break;
      case DO_LASSOZOOM:
        if(op==LASSOZOOM) return;
        FXTRACE((100,"LASSOZOOM\n"));
        setDefaultCursor(getApp()->getDefaultCursor(DEF_CORNERNW_CURSOR));
        break;
      case LASSOZOOM:
        FXTRACE((100,"LASSOZOOM\n"));
        setDefaultCursor(getDragCursor());
        setDragCursor(getApp()->getDefaultCursor(DEF_CORNERNW_CURSOR));
        break;
      }
    op=o;
    }
  }


// Perform the usual mouse manipulation
long FXGLViewer::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXGLObject *objects[2];
  flags&=~FLAG_TIP;
  FXTRACE((100,"onLeftBtnPress Mask=%08x\n",event->state));
  handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
  if(isEnabled()){
    grab();
    flags&=~FLAG_UPDATE;
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr)) return 1;
    if(options&VIEWER_LOCKED){
      if(!handle(this,MKUINT(0,SEL_PICKED),ptr)){
        objects[0]=pick(event->click_x,event->click_y);
        objects[1]=NULL;
        handle(this,MKUINT(0,SEL_CHANGED),(void*)objects[0]);
        handle(this,MKUINT(0,SEL_SELECTED),(void*)objects);
        if(objects[0] && objects[0]->canDrag()){
          setOp(DRAGGING);
          }
        }
      }
    else{
      if(event->state&RIGHTBUTTONMASK){
        if(event->state&SHIFTMASK)
          setOp(TRUCKING);
        else
          setOp(ZOOMING);
        }
      else if(event->state&MIDDLEBUTTONMASK){
        setOp(ROTATING);
        }
      else{
        if(op==DO_LASSOZOOM){
          drawLasso(event->click_x,event->click_y,event->win_x,event->win_y);
          setOp(LASSOZOOM);
          }
        else if((op==DO_LASSOSELECT) || (event->state&(SHIFTMASK|CONTROLMASK))){
          drawLasso(event->click_x,event->click_y,event->win_x,event->win_y);
          setOp(LASSOSELECT);
          }
        else if(selection && selection->canDrag() && selection==pick(event->click_x,event->click_y)){
          setOp(DRAGGING);
          }
        else{
          setOp(PICKING);
          }
        }
      }
    }
  return 1;
  }


// Left mouse button released
long FXGLViewer::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXGLObject *objects[2];
  FXint new_x,new_y,cx,cy,xl,xh,yl,yh;
  FXVec vec;
  FXTRACE((100,"onLeftBtnRelease Mask=%08x\n",event->state));
  if(isEnabled()){
    ungrab();
    flags|=FLAG_UPDATE;
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr)) return 1;
    if(options&VIEWER_LOCKED){
      if(op==DRAGGING){
        if(target) target->handle(this,MKUINT(message,SEL_DRAGGED),selection);
        setOp(HOVERING);
        }
      }
    else{
      if(event->state&RIGHTBUTTONMASK){
        if(event->state&SHIFTMASK)
          setOp(GYRATING);
        else if(event->state&CONTROLMASK)
          setOp(FOVING);
        else
          setOp(TRANSLATING);
        grab();
        }
      else if(event->state&MIDDLEBUTTONMASK){
        if(event->state&SHIFTMASK)
          setOp(TRUCKING);
        else
          setOp(ZOOMING);
        grab();
        }
      else{
        if(op==LASSOZOOM){
          new_x=FXCLAMP(0,event->win_x,(width-1));
          new_y=FXCLAMP(0,event->win_y,(height-1));
          drawLasso(event->click_x,event->click_y,new_x,new_y);
          xl=FXMIN(new_x,event->click_x);
          xh=FXMAX(new_x,event->click_x);
          yl=FXMIN(new_y,event->click_y);
          yh=FXMAX(new_y,event->click_y);
          if(xh>xl && yh>yl){
            cx=(getWidth()-(xl+xh))/2;
            cy=(getHeight()-(yl+yh))/2;
            vec=worldVector(0,0,cx,cy);
            translate(-vec);
            setZoom(zoom*getWidth()/(xh-xl));
            }
          setOp(HOVERING);
          }
        else if(op==LASSOSELECT){
          new_x=FXCLAMP(0,event->win_x,(width-1));
          new_y=FXCLAMP(0,event->win_y,(height-1));
          drawLasso(event->click_x,event->click_y,new_x,new_y);
          handle(this,MKUINT(0,SEL_LASSOED),ptr);
          setOp(HOVERING);
          }
        else if(op==PICKING){
          setOp(HOVERING);
          if(!handle(this,MKUINT(0,SEL_PICKED),ptr)){
            objects[0]=pick(event->click_x,event->click_y);
            objects[1]=NULL;
            handle(this,MKUINT(0,SEL_CHANGED),(void*)objects[0]);
            handle(this,MKUINT(0,SEL_SELECTED),(void*)objects);
            }
          }
        else if(op==DRAGGING){
          if(target) target->handle(this,MKUINT(message,SEL_DRAGGED),selection);
          setOp(HOVERING);
          }
        else{
          setOp(HOVERING);
          }
        }
      }
    if(event->click_count==1){
      handle(this,MKUINT(0,SEL_CLICKED),(void*)selection);
      }
    else if(event->click_count==2){
      handle(this,MKUINT(0,SEL_DOUBLECLICKED),(void*)selection);
      }
    else if(event->click_count==3){
      handle(this,MKUINT(0,SEL_TRIPLECLICKED),(void*)selection);
      }
    }
  return 1;
  }


// Pressed middle mouse button
long FXGLViewer::onMiddleBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  flags&=~FLAG_TIP;
  FXTRACE((100,"onMiddleBtnPress Mask=%08x\n",event->state));
  handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
  if(isEnabled()){
    grab();
    flags&=~FLAG_UPDATE;
    if(target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONPRESS),ptr)) return 1;
    if(!(options&VIEWER_LOCKED)){
      if(event->state&SHIFTMASK)
        setOp(TRUCKING);
      else
        setOp(ZOOMING);
      }
    }
  return 1;
  }


// Released middle mouse button
long FXGLViewer::onMiddleBtnRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXTRACE((100,"onMiddleBtnRelease Mask=%08x\n",event->state));
  if(isEnabled()){
    ungrab();
    flags|=FLAG_UPDATE;
    if(target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONRELEASE),ptr)) return 1;
    if(!(options&VIEWER_LOCKED)){
      if(event->state&LEFTBUTTONMASK){
        setOp(ROTATING);
        grab();
        }
      else if(event->state&RIGHTBUTTONMASK){
        if(event->state&SHIFTMASK)
          setOp(GYRATING);
        else if(event->state&CONTROLMASK)
          setOp(FOVING);
        else
          setOp(TRANSLATING);
        grab();
        }
      else{
        setOp(HOVERING);
        }
      }
    }
  return 1;
  }


// Pressed right button
long FXGLViewer::onRightBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  flags&=~FLAG_TIP;
  FXTRACE((100,"onRightBtnPress Mask=%08x\n",event->state));
  handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
  if(isEnabled()){
    grab();
    flags&=~FLAG_UPDATE;
    if(target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONPRESS),ptr)) return 1;
    if(options&VIEWER_LOCKED){
      setOp(POSTING);
      }
    else{
      if(event->state&LEFTBUTTONMASK){
        if(event->state&SHIFTMASK)
          setOp(TRUCKING);
        else
          setOp(ZOOMING);
        }
      else if(event->state&MIDDLEBUTTONMASK){
        if(event->state&SHIFTMASK)
          setOp(GYRATING);
        else if(event->state&CONTROLMASK)
          setOp(FOVING);
        else
          setOp(TRANSLATING);
        }
      else{
        if(event->state&SHIFTMASK)
          setOp(GYRATING);
        else if(event->state&CONTROLMASK)
          setOp(FOVING);
        else
          setOp(POSTING);
        }
      }
    }
  return 1;
  }


// Microsoft Visual C++: Disable compiler warnings for empty "if"
// statements below in FXGLViewer::onRightBtnRelease()
#ifdef _MSC_VER
#pragma warning( disable : 4390 )
#endif


// Released right button
long FXGLViewer::onRightBtnRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXGLObject *hit;
  FXTRACE((100,"onRightBtnRelease Mask=%08x\n",event->state));
  if(isEnabled()){
    ungrab();
    flags|=FLAG_UPDATE;
    if(target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONRELEASE),ptr)) return 1;
    if(options&VIEWER_LOCKED){
      if(op==POSTING){
        setOp(HOVERING);
        hit=pick(event->click_x,event->click_y);
        if(hit && hit->handle(this,MKUINT(ID_QUERY_MENU,SEL_COMMAND),ptr))
          ;
        else if(target && target->handle(this,MKUINT(ID_QUERY_MENU,SEL_COMMAND),ptr))
          ;
        }
      setOp(HOVERING);
      }
    else{
      if(event->state&LEFTBUTTONMASK){
        setOp(ROTATING);
        grab();
        }
      else if(event->state&MIDDLEBUTTONMASK){
        if(event->state&SHIFTMASK)
          setOp(TRUCKING);
        else
          setOp(ZOOMING);
        grab();
        }
      else{
        if(op==POSTING){
          setOp(HOVERING);
          hit=pick(event->click_x,event->click_y);
          if(hit && hit->handle(this,MKUINT(ID_QUERY_MENU,SEL_COMMAND),ptr))
            ;
          else if(target && target->handle(this,MKUINT(ID_QUERY_MENU,SEL_COMMAND),ptr))
            ;
          }
        setOp(HOVERING);
        }
      }
    }
  return 1;
  }


// Mouse moved
long FXGLViewer::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXint new_x,new_y,old_x,old_y;
  long changed=(flags&FLAG_TIP)!=0;
  FXdouble delta;
  FXfloat tmp;
  FXVec vec;
  FXQuat q;
  flags&=~FLAG_TIP;
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_MOTION),ptr)) return 1;
    if(timer){ getApp()->removeTimeout(timer); timer=NULL; }
    switch(op){
      case HOVERING:          // Reset the timer each time we moved the cursor
        timer=getApp()->addTimeout(getApp()->getMenuPause(),this,ID_TIPTIMER);
        break;
      case PICKING:           // Picking, but we moved; if moving more than a delta, we go to rotate mode
        if(!event->moved || (options&VIEWER_LOCKED)) break;
        setOp(ROTATING);
      case ROTATING:          // Rotating camera around target
        q=turn(event->last_x,event->last_y,event->win_x,event->win_y) * getOrientation();
        setOrientation(q);
        changed=1;
        break;
      case POSTING:           // Posting right-mouse menu; if moving more than delta, we go to translate mode
        if(!event->moved || (options&VIEWER_LOCKED)) break;
        setOp(TRANSLATING);
      case TRANSLATING:       // Translating camera
        vec=worldVector(event->last_x,event->last_y,event->win_x,event->win_y);
        translate(-vec);
        changed=1;
        break;
      case ZOOMING:           // Zooming camera
        delta=0.005*(event->win_y-event->last_y);
        setZoom(getZoom()*pow(2.0,delta));
        changed=1;
        break;
      case FOVING:            // Change FOV
        setFieldOfView(getFieldOfView()+90.0*(event->win_y-event->last_y)/(double)wvt.h);
        changed=1;
        break;
      case DRAGGING:          // Dragging a shape
        if(selection && selection->drag(this,event->last_x,event->last_y,event->win_x,event->win_y)){
          //// Perhaps callback here for the target to be notified of the new object position
          update();
          }
        changed=1;
        break;
      case TRUCKING:          // Trucking camera forward or backward
        tmp=(FXfloat)(worldpx*(event->win_y-event->last_y));
        vec=normalize(getEyeVector());
        translate(tmp*vec);
        changed=1;
        break;
      case GYRATING:          // Rotating camera around eye
        {
          FXHMat mm;
          FXQuat qq;
          qq=turn(event->win_x,event->win_y,event->last_x,event->last_y);
          mm.eye();
          mm.trans(0.0f,0.0f,(FXfloat)-distance); // FIXME This aint it yet...
          mm.rot(qq);
          mm.trans(0.0f,0.0f,(FXfloat)distance);
          center=center*mm;
          q=qq * getOrientation();
          setOrientation(q);
          update();
          changed=1;
        }
        break;
      case LASSOSELECT:          // Dragging a lasso
      case LASSOZOOM:
        old_x=FXCLAMP(0,event->last_x,(width-1));
        old_y=FXCLAMP(0,event->last_y,(height-1));
        new_x=FXCLAMP(0,event->win_x,(width-1));
        new_y=FXCLAMP(0,event->win_y,(height-1));
        drawLasso(event->click_x,event->click_y,old_x,old_y);
        drawLasso(event->click_x,event->click_y,new_x,new_y);
        if(new_x>event->click_x){
          if(new_y>event->click_y)
            setDragCursor(getApp()->getDefaultCursor(DEF_CORNERSE_CURSOR));
          else
            setDragCursor(getApp()->getDefaultCursor(DEF_CORNERNE_CURSOR));
          }
        else{
          if(new_y>event->click_y)
            setDragCursor(getApp()->getDefaultCursor(DEF_CORNERSW_CURSOR));
          else
            setDragCursor(getApp()->getDefaultCursor(DEF_CORNERNW_CURSOR));
          }
        changed=1;
        break;
      }
    }
  return changed;
  }


// Mouse wheel
long FXGLViewer::onMouseWheel(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  setZoom(getZoom()*pow(2.0,-0.1*event->code/120.0));
  return 1;
  }


// Handle keyboard press/release
long FXGLViewer::onKeyPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  flags&=~FLAG_TIP;
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_KEYPRESS),ptr)) return 1;
    switch(event->code){
      case KEY_Shift_L:
      case KEY_Shift_R:

        // We do not switch modes unless something was going on already
        if(op!=HOVERING){
          if((event->state&MIDDLEBUTTONMASK) || ((event->state&LEFTBUTTONMASK) && (event->state&RIGHTBUTTONMASK))){
            setOp(TRUCKING);
            }
          else if(event->state&RIGHTBUTTONMASK){
            setOp(GYRATING);
            }
          }
        return 1;
      case KEY_Control_L:
      case KEY_Control_R:

        // We do not switch modes unless something was going on already
        if(op!=HOVERING){
          if(event->state&RIGHTBUTTONMASK){
            setOp(FOVING);
            }
          }
        return 1;
      }
    }
  return 0;
  }


// Key release
long FXGLViewer::onKeyRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  if(isEnabled()){
    if(target && target->handle(this,MKUINT(message,SEL_KEYRELEASE),ptr)) return 1;
    switch(event->code){
      case KEY_Shift_L:
      case KEY_Shift_R:

        // We do not switch modes unless something was going on already
        if(op!=HOVERING){
          if((event->state&MIDDLEBUTTONMASK) || ((event->state&LEFTBUTTONMASK) && (event->state&RIGHTBUTTONMASK))){
            setOp(ZOOMING);
            }
          else if(event->state&RIGHTBUTTONMASK){
            setOp(TRANSLATING);
            }
          }
        return 1;
      case KEY_Control_L:
      case KEY_Control_R:

        // We do not switch modes unless something was going on already
        if(op!=HOVERING){
          if(event->state&RIGHTBUTTONMASK){
            setOp(TRANSLATING);
            }
          }
        return 1;
      }
    }
  return 0;
  }


// The widget lost the grab for some reason
long FXGLViewer::onUngrabbed(FXObject* sender,FXSelector sel,void* ptr){
  FXGLCanvas::onUngrabbed(sender,sel,ptr);
  flags&=~FLAG_PRESSED;
  flags&=~FLAG_CHANGED;
  flags|=FLAG_UPDATE;
  setOp(HOVERING);
  doesturbo=FALSE;
  return 1;
  }


// We timed out, i.e. the user didn't move for a while
long FXGLViewer::onTipTimer(FXObject*,FXSelector,void*){
  FXTRACE((250,"%s::onTipTimer %p\n",getClassName(),this));
  timer=NULL;
  flags|=FLAG_TIP;
  return 1;
  }


// We were asked about status text
long FXGLViewer::onQueryHelp(FXObject* sender,FXSelector,void*){
  if((flags&FLAG_HELP)){
    FXTRACE((250,"%s::onQueryHelp %p\n",getClassName(),this));
    if(!help.empty()){
      sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&help);
      return 1;
      }
    }
  return 0;
  }


// We were asked about tip text
long FXGLViewer::onQueryTip(FXObject* sender,FXSelector sel,void* ptr){
  FXGLObject *hit;
  FXuint state;
  FXint x,y;
  if(flags&FLAG_TIP){
    getCursorPosition(x,y,state);
    FXTRACE((250,"%s::onQueryTip %p (%d,%d)\n",getClassName(),this,x,y));
    hit=pick(x,y);
    if(hit && hit->handle(sender,sel,ptr)) return 1;
    if(!tip.empty()){
      sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&tip);
      return 1;
      }
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
long FXGLViewer::onUpdPerspective(FXObject* sender,FXSelector,void*){
  FXuint msg=projection==PERSPECTIVE ? ID_CHECK : ID_UNCHECK;
  sender->handle(this,MKUINT(ID_SHOW,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(msg,SEL_COMMAND),NULL);
  return 1;
  }


// Switch to parallel mode
long FXGLViewer::onCmdParallel(FXObject*,FXSelector,void*){
  setProjection(PARALLEL);
  return 1;
  }


// Update sender
long FXGLViewer::onUpdParallel(FXObject* sender,FXSelector,void*){
  FXuint msg=projection==PARALLEL ? ID_CHECK : ID_UNCHECK;
  sender->handle(this,MKUINT(ID_SHOW,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(msg,SEL_COMMAND),NULL);
  return 1;
  }


/*****************************  Switch Viewpoints  *****************************/

// View front
long FXGLViewer::onCmdFront(FXObject*,FXSelector,void*){
  rotation=FXQuat(0.0f,0.0f,0.0f,1.0f);
  updateTransform();
  update();
  return 1;
  }


// Update sender
long FXGLViewer::onUpdFront(FXObject* sender,FXSelector,void*){
  FXuint msg=ID_UNCHECK;
  if(EPS>fabs(rotation[0]) &&
     EPS>fabs(rotation[1]) &&
     EPS>fabs(rotation[2]) &&
     EPS>fabs(rotation[3]-1.0)) msg=ID_CHECK;
  sender->handle(this,MKUINT(ID_SHOW,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(msg,SEL_COMMAND),NULL);
  return 1;
  }

// View back
long FXGLViewer::onCmdBack(FXObject*,FXSelector,void*){
  rotation=FXQuat(0.0f,-1.0f,0.0f,0.0f);
  updateTransform();
  update();
  return 1;
  }


// Update sender
long FXGLViewer::onUpdBack(FXObject* sender,FXSelector,void*){
  FXuint msg=ID_UNCHECK;
  if(EPS>fabs(rotation[0]) &&
     EPS>fabs(rotation[1]+1.0) &&
     EPS>fabs(rotation[2]) &&
     EPS>fabs(rotation[3])) msg=ID_CHECK;
  sender->handle(this,MKUINT(ID_SHOW,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(msg,SEL_COMMAND),NULL);
  return 1;
  }

// View left
long FXGLViewer::onCmdLeft(FXObject*,FXSelector,void*){
  rotation=FXQuat(0.0f,0.7071067811865f,0.0f,0.7071067811865f);
  updateTransform();
  update();
  return 1;
  }


// Update sender
long FXGLViewer::onUpdLeft(FXObject* sender,FXSelector,void*){
  FXuint msg=ID_UNCHECK;
  if(EPS>fabs(rotation[0]) &&
     EPS>fabs(rotation[1]-0.7071067811865) &&
     EPS>fabs(rotation[2]) &&
     EPS>fabs(rotation[3]-0.7071067811865)) msg=ID_CHECK;
  sender->handle(this,MKUINT(ID_SHOW,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(msg,SEL_COMMAND),NULL);
  return 1;
  }

// View right
long FXGLViewer::onCmdRight(FXObject*,FXSelector,void*){
  rotation=FXQuat(0.0f,-0.7071067811865f,0.0f,0.7071067811865f);
  updateTransform();
  update();
  return 1;
  }


// Update sender
long FXGLViewer::onUpdRight(FXObject* sender,FXSelector,void*){
  FXuint msg=ID_UNCHECK;
  if(EPS>fabs(rotation[0]) &&
     EPS>fabs(rotation[1]+0.7071067811865) &&
     EPS>fabs(rotation[2]) &&
     EPS>fabs(rotation[3]-0.7071067811865)) msg=ID_CHECK;
  sender->handle(this,MKUINT(ID_SHOW,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(msg,SEL_COMMAND),NULL);
  return 1;
  }

// View top
long FXGLViewer::onCmdTop(FXObject*,FXSelector,void*){
  rotation=FXQuat(0.7071067811865f,0.0f,0.0f,0.7071067811865f);
  updateTransform();
  update();
  return 1;
  }


// Update sender
long FXGLViewer::onUpdTop(FXObject* sender,FXSelector,void*){
  FXuint msg=ID_UNCHECK;
  if(EPS>fabs(rotation[0]-0.7071067811865) &&
     EPS>fabs(rotation[1]) &&
     EPS>fabs(rotation[2]) &&
     EPS>fabs(rotation[3]-0.7071067811865)) msg=ID_CHECK;
  sender->handle(this,MKUINT(ID_SHOW,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(msg,SEL_COMMAND),NULL);
  return 1;
  }

// View bottom
long FXGLViewer::onCmdBottom(FXObject*,FXSelector,void*){
  rotation=FXQuat(-0.7071067811865f,0.0f,0.0f,0.7071067811865f);
  updateTransform();
  update();
  return 1;
  }


// Update sender
long FXGLViewer::onUpdBottom(FXObject* sender,FXSelector,void*){
  FXuint msg=ID_UNCHECK;
  if(EPS>fabs(rotation[0]+0.7071067811865) &&
     EPS>fabs(rotation[1]) &&
     EPS>fabs(rotation[2]) &&
     EPS>fabs(rotation[3]-0.7071067811865)) msg=ID_CHECK;
  sender->handle(this,MKUINT(ID_SHOW,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(msg,SEL_COMMAND),NULL);
  return 1;
  }


// Reset view
long FXGLViewer::onCmdResetView(FXObject*,FXSelector,void*){
  FXRange r(-1.0f,1.0f,-1.0f,1.0f,-1.0f,1.0f);
  rotation=FXQuat(0.0f,0.0f,0.0f,1.0f);
  zoom=1.0;
  scale=FXVec(1.0f,1.0f,1.0f);
  if(scene) scene->bounds(r);
  setBounds(r);
  updateProjection();
  updateTransform();
  update();
  return 1;
  }


// Fit view
long FXGLViewer::onCmdFitView(FXObject*,FXSelector,void*){
  FXRange r(-1.0f,1.0f,-1.0f,1.0f,-1.0f,1.0f);
  if(scene) scene->bounds(r);
  setBounds(r);
  update();
  return 1;
  }


// Update zoom
long FXGLViewer::onUpdZoom(FXObject* sender,FXSelector,void*){
  sender->handle(this,MKUINT(FXWindow::ID_SETREALVALUE,SEL_COMMAND),(void*)&zoom);
  return 1;
  }


// Change zoom
long FXGLViewer::onCmdZoom(FXObject* sender,FXSelector sel,void*){
  FXdouble z=zoom;
  sender->handle(this,MKUINT(FXWindow::ID_GETREALVALUE,SEL_COMMAND),(void*)&z);
  doesturbo=(SELTYPE(sel)==SEL_CHANGED)?turbomode:FALSE;
  setZoom(z);
  return 1;
  }


// Update field of view
long FXGLViewer::onUpdFov(FXObject* sender,FXSelector,void*){
  sender->handle(this,MKUINT(FXWindow::ID_SETREALVALUE,SEL_COMMAND),(void*)&fov);
  return 1;
  }


// Change field of view
long FXGLViewer::onCmdFov(FXObject* sender,FXSelector sel,void*){
  FXdouble f=fov;
  sender->handle(this,MKUINT(FXWindow::ID_GETREALVALUE,SEL_COMMAND),(void*)&f);
  doesturbo=(SELTYPE(sel)==SEL_CHANGED)?turbomode:FALSE;
  setFieldOfView(f);
  return 1;
  }


// Scale model
long FXGLViewer::onCmdXYZScale(FXObject* sender,FXSelector sel,void*){
  FXVec s=scale;
  FXdouble value;
  sender->handle(this,MKUINT(FXWindow::ID_GETREALVALUE,SEL_COMMAND),&value);
  s[SELID(sel)-ID_SCALE_X]=value;
  doesturbo=(SELTYPE(sel)==SEL_CHANGED)?turbomode:FALSE;
  setScale(s);
  return 1;
  }


// Update scale value
long FXGLViewer::onUpdXYZScale(FXObject* sender,FXSelector sel,void*){
  FXdouble value=scale[SELID(sel)-ID_SCALE_X];
  sender->handle(this,MKUINT(FXWindow::ID_SETREALVALUE,SEL_COMMAND),(void*)&value);
  return 1;
  }


// Rotate camera about model by means of dials
long FXGLViewer::onCmdXYZDial(FXObject*,FXSelector sel,void* ptr){
  const FXVec xaxis(1.0f,0.0f,0.0f);
  const FXVec yaxis(0.0f,1.0f,0.0f);
  const FXVec zaxis(0.0f,0.0f,1.0f);
  FXint dialnew=(FXint)(FXival)ptr;
  FXfloat ang;
  FXQuat q;
  if(SELTYPE(sel)==SEL_CHANGED){
    doesturbo=turbomode;
    FXASSERT(ID_DIAL_X<=SELID(sel) && SELID(sel)<=ID_DIAL_Z);
    switch(SELID(sel)){
      case ID_DIAL_X:
        ang=(FXfloat)(DTOR*(dialnew-dial[0]));
        q=FXQuat(xaxis,-ang);
        dial[0]=dialnew;
        break;
      case ID_DIAL_Y:
        ang=(FXfloat)(DTOR*(dialnew-dial[1]));
        q=FXQuat(yaxis, ang);
        dial[1]=dialnew;
        break;
      case ID_DIAL_Z:
        ang=(FXfloat)(DTOR*(dialnew-dial[2]));
        q=FXQuat(zaxis, ang);
        dial[2]=dialnew;
        break;
      }
    setOrientation(q*getOrientation());
    }
  else if(doesturbo){
    doesturbo=FALSE;
    update();
    }
  return 1;
  }


// Update dial value
long FXGLViewer::onUpdXYZDial(FXObject* sender,FXSelector sel,void*){
  FXASSERT(ID_DIAL_X<=SELID(sel) && SELID(sel)<=ID_DIAL_Z);
  sender->handle(this,MKUINT(FXWindow::ID_SETINTVALUE,SEL_COMMAND),(void*)&dial[SELID(sel)-ID_DIAL_X]);
  return 1;
  }


// Update roll pitch yaw
long FXGLViewer::onCmdRollPitchYaw(FXObject* sender,FXSelector sel,void*){
  FXASSERT(ID_ROLL<=SELID(sel) && SELID(sel)<=ID_YAW);
  FXfloat rpy[3];
  FXdouble ang;
  rotation.getRollPitchYaw(rpy[0],rpy[1],rpy[2]);
  sender->handle(this,MKUINT(FXWindow::ID_GETREALVALUE,SEL_COMMAND),(void*)&ang);
  rpy[SELID(sel)-ID_ROLL]=DTOR*ang;
  doesturbo=(SELTYPE(sel)==SEL_CHANGED)?turbomode:FALSE;
  setOrientation(FXQuat(rpy[0],rpy[1],rpy[2]));
  update();
  return 1;
  }


// Update roll pitch yaw
long FXGLViewer::onUpdRollPitchYaw(FXObject* sender,FXSelector sel,void*){
  FXASSERT(ID_ROLL<=SELID(sel) && SELID(sel)<=ID_YAW);
  FXfloat rpy[3];
  rotation.getRollPitchYaw(rpy[0],rpy[1],rpy[2]);
  FXdouble ang=RTOD*rpy[SELID(sel)-ID_ROLL];
  sender->handle(this,MKUINT(FXWindow::ID_SETREALVALUE,SEL_COMMAND),(void*)&ang);
  return 1;
  }


/******************************  Printing Support  *****************************/


// Read back pixels
// Derived from code contributed by <sancelot@crosswinds.net>
FXbool FXGLViewer::readPixels(FXuchar*& buffer,FXint x,FXint y,FXint w,FXint h){
#ifdef HAVE_OPENGL
  GLint swapbytes,lsbfirst,rowlength,oldbuf;
  GLint skiprows,skippixels,alignment;
  GLuint size=w*h*3;

  // Try allocate buffer
  if(!FXMALLOC(&buffer,FXuchar,size)) return FALSE;

  makeCurrent();

  // Save old pixel formats
  glGetIntegerv(GL_PACK_SWAP_BYTES,&swapbytes);
  glGetIntegerv(GL_PACK_LSB_FIRST,&lsbfirst);
  glGetIntegerv(GL_PACK_ROW_LENGTH,&rowlength);
  glGetIntegerv(GL_PACK_SKIP_ROWS,&skiprows);
  glGetIntegerv(GL_PACK_SKIP_PIXELS,&skippixels);
  glGetIntegerv(GL_PACK_ALIGNMENT,&alignment);
  glGetIntegerv(GL_READ_BUFFER,&oldbuf);

  // Set pixel readback formats
  glPixelStorei(GL_PACK_SWAP_BYTES,GL_FALSE);
  glPixelStorei(GL_PACK_LSB_FIRST,GL_FALSE);
  glPixelStorei(GL_PACK_ROW_LENGTH,0);
  glPixelStorei(GL_PACK_SKIP_ROWS,0);
  glPixelStorei(GL_PACK_SKIP_PIXELS,0);
  glPixelStorei(GL_PACK_ALIGNMENT,1);

  // Read from the right buffer
  glReadBuffer((GLenum)GL_FRONT);

  // Read the pixels
  glReadPixels(x,y,w,h,GL_RGB,GL_UNSIGNED_BYTE,(GLvoid*)buffer);

  // Restore old formats
  glPixelStorei(GL_PACK_SWAP_BYTES,swapbytes);
  glPixelStorei(GL_PACK_LSB_FIRST,lsbfirst);
  glPixelStorei(GL_PACK_ROW_LENGTH,rowlength);
  glPixelStorei(GL_PACK_SKIP_ROWS,skiprows);
  glPixelStorei(GL_PACK_SKIP_PIXELS,skippixels);
  glPixelStorei(GL_PACK_ALIGNMENT,alignment);
  glReadBuffer((GLenum)oldbuf);

  makeNonCurrent();

  return TRUE;
#else
  return FALSE;
#endif
  }


// Print the window by grabbing pixels
long FXGLViewer::onCmdPrintImage(FXObject*,FXSelector,void*){
  FXPrintDialog dlg(this,"Print Scene");
  FXuint red,green,blue;
  FXPrinter printer;
  FXuchar *buffer;
  FXint i;

  // Run dialog
  if(dlg.execute()){
    dlg.getPrinter(printer);
    FXDCPrint pdc(getApp());
    if(!pdc.beginPrint(printer)){
      FXMessageBox::error(this,MBOX_OK,"Printer Error","Unable to print");
      return 1;
      }

    // Repaint now
    repaint();

    // Flush commands
    getApp()->flush(TRUE);

    // Page header
    pdc.beginPage(1);

    // Grab pixels
    if(readPixels(buffer,0,0,width,height)){

      pdc.outf("/picstr %d string def\n",width*3);
      pdc.outf("%d %d translate\n",0,0+height);
      pdc.outf("%d %d scale\n",width,-height);
      pdc.outf("%d %d %d\n",width,height,8);
      pdc.outf("[%d 0 0 -%d 0 %d]\n",width,height,height);
      pdc.outf("{currentfile picstr readhexstring pop}\n");
      pdc.outf("false %d\n",3);
      pdc.outf("colorimage\n");

      for(i=0; i<width*height; i++){
        red=buffer[3*i];
        green=buffer[3*i+1];
        blue=buffer[3*i+2];
        pdc.outhex(red);
        pdc.outhex(green);
        pdc.outhex(blue);
        }
      pdc.outf("\n");

      FXFREE(&buffer);
      }

    // Page trailer
    pdc.endPage();
    pdc.endPrint();
    }
  return 1;
  }


// Render
FXint FXGLViewer::renderFeedback(FXfloat *buffer,FXint x,FXint y,FXint w,FXint h,FXint maxbuffer){
#ifdef HAVE_OPENGL
  FXint used;
  makeCurrent();
  glFeedbackBuffer(maxbuffer,GL_3D_COLOR,buffer);
  glRenderMode(GL_FEEDBACK);
  drawWorld(wvt);
  used=glRenderMode(GL_RENDER);
  makeNonCurrent();
  return used;
#else
  return -1;
#endif
  }


// Read feedback buffer
FXbool FXGLViewer::readFeedback(FXfloat*& buffer,FXint& used,FXint& size,FXint x,FXint y,FXint w,FXint h){
  FXbool ok=FALSE;
  buffer=NULL;
  used=0;
  size=10000;
  while(1){

    // Allocate buffer
    FXMALLOC(&buffer,FXfloat,size);

    // It got too big, give up
    if(!buffer) break;

    // Try to render scene into it
    used=renderFeedback(buffer,x,y,w,h,size);

    // No errors, got our stuff
    if(0<used){
      ok=TRUE;
      break;
      }

    // It didn't fit, lets double the buffer and try again
    FXFREE(&buffer);
    size*=2;
    continue;
    }
  return ok;
  }


// Draw feedback buffer into dc
void FXGLViewer::drawFeedback(FXDCPrint& pdc,const FXfloat* buffer,FXint used){
#ifdef HAVE_OPENGL
  FXint nvertices,smooth,token,i,p;

  // Draw background
  pdc.outf("%g %g %g C\n",background[0],background[1],background[2]);
  pdc.outf("newpath\n");
  pdc.outf("%g %g moveto\n",0.0,0.0);
  pdc.outf("%g %g lineto\n",0.0,(double)height);
  pdc.outf("%g %g lineto\n",(double)width,(double)height);
  pdc.outf("%g %g lineto\n",(double)width,0.0);
  pdc.outf("closepath fill\n");

  pdc.outf("1 setlinewidth\n");

  // Crank out primitives
  p=0;
  while(p<used){
    token=(FXint)buffer[p++];
    switch(token){

      // Point primitive
      case GL_POINT_TOKEN:
        pdc.outf("%g %g %g %g %g P\n",buffer[p+0],buffer[p+1],buffer[p+3],buffer[p+4],buffer[p+5]);
        p+=7;             // Each vertex element in the feedback buffer is 7 floats
        break;

      // Line primitive
      case GL_LINE_RESET_TOKEN:
      case GL_LINE_TOKEN:
        if(fabs(buffer[p+3]-buffer[p+7+3])<1E-4 || fabs(buffer[p+4]-buffer[p+7+4])<1E-4 || fabs(buffer[p+5]-buffer[p+7+5])<1E-4){
          pdc.outf("%g %g %g %g %g %g %g %g %g %g SL\n",buffer[p+0],buffer[p+1],buffer[p+3],buffer[p+4],buffer[p+5], buffer[p+7+0],buffer[p+7+1],buffer[p+7+3],buffer[p+7+4],buffer[p+7+5]);
          }
        else{
          pdc.outf("%g %g %g %g %g %g %g L\n",buffer[p+0],buffer[p+1],buffer[p+7+0],buffer[p+7+1],buffer[p+3],buffer[p+4],buffer[p+5]);
          }
        p+=14;            // Each vertex element in the feedback buffer is 7 GLfloats
        break;

      // Polygon primitive
      case GL_POLYGON_TOKEN:
        nvertices = (FXint)buffer[p++];
        if(nvertices==3){ // We assume polybusting has taken place already!
          smooth=0;
          for(i=1; i<nvertices; i++){
            if(fabs(buffer[p+3]-buffer[p+i*7+3])<1E-4 || fabs(buffer[p+4]-buffer[p+i*7+4])<1E-4 || fabs(buffer[p+5]-buffer[p+i*7+5])<1E-4){ smooth=1; break; }
            }
          if(smooth){
            pdc.outf("%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g ST\n",buffer[p+0],buffer[p+1],buffer[p+3],buffer[p+4],buffer[p+5], buffer[p+7+0],buffer[p+7+1],buffer[p+7+3],buffer[p+7+4],buffer[p+7+5], buffer[p+14+0],buffer[p+14+1],buffer[p+14+3],buffer[p+14+4],buffer[p+14+5]);
            }
          else{
            pdc.outf("%g %g %g %g %g %g %g %g %g T\n",buffer[p+0],buffer[p+1], buffer[p+7+0],buffer[p+7+1], buffer[p+14+0],buffer[p+14+1], buffer[p+3],buffer[p+4],buffer[p+5]);
            }
          }
        p+=nvertices*7;   // Each vertex element in the feedback buffer is 7 GLfloats
        break;

      // Skip these, don't deal with it here
      case GL_BITMAP_TOKEN:
      case GL_DRAW_PIXEL_TOKEN:
      case GL_COPY_PIXEL_TOKEN:
        p+=7;
        break;

      // Skip passthrough tokens
      case GL_PASS_THROUGH_TOKEN:
        p++;
        break;

      // Bad token, this is the end
      default:
        return;
      }
    }
#endif
  }





// Print the window by means of feedback buffer
long FXGLViewer::onCmdPrintVector(FXObject*,FXSelector,void*){
  FXPrintDialog dlg(this,"Print Scene");
  FXPrinter printer;
  FXfloat *buffer;
  FXint used,size;

  // Run dialog
  if(dlg.execute()){
    dlg.getPrinter(printer);
    FXDCPrint pdc(getApp());
    if(!pdc.beginPrint(printer)){
      FXMessageBox::error(this,MBOX_OK,"Printer Error","Unable to print");
      return 1;
      }

    // Repaint now
    repaint();

    // Flush commands
    getApp()->flush(TRUE);

    // Page header
    pdc.beginPage(1);

    // Read feedback
    if(readFeedback(buffer,used,size,0,0,width,height)){
      if(zsortfunc) (*zsortfunc)(buffer,used,size);   // FIXME:- may throw exception
      drawFeedback(pdc,buffer,used);
      }

    // Page trailer
    pdc.endPage();
    pdc.endPrint();
    }
  return 1;
  }


// Zoom into lasso rectangle
long FXGLViewer::onCmdLassoZoom(FXObject*,FXSelector,void*){
  setOp(DO_LASSOZOOM);
  return 1;
  }

// Select objects in lasso rectangle
long FXGLViewer::onCmdLassoSelect(FXObject*,FXSelector,void*){
  setOp(DO_LASSOSELECT);
  return 1;
  }

/*****************************  Selection Support  *****************************/

// We now really do have the selection
long FXGLViewer::onClipboardGained(FXObject* sender,FXSelector sel,void* ptr){
  FXGLCanvas::onClipboardGained(sender,sel,ptr);
  return 1;
  }


// We lost the selection somehow
long FXGLViewer::onClipboardLost(FXObject* sender,FXSelector sel,void* ptr){
  FXGLCanvas::onClipboardLost(sender,sel,ptr);
  return 1;
  }


// Somebody wants our selection
long FXGLViewer::onClipboardRequest(FXObject* sender,FXSelector sel,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  FXuchar *data; FXuint len;

  // Try handling it in base class first
  if(FXGLCanvas::onClipboardRequest(sender,sel,ptr)) return 1;

  // Requested data from clipboard
  if(event->target==objectType){
    FXTRACE((1,"requested objectType\n"));
//    FXMemoryStream stream;
//    stream.open(NULL,0,FXStreamSave);
//    stream.takeBuffer(data,len);
//    stream.close();
//    setDNDData(FROM_CLIPBOARD,objectType,data,len);
    return 1;
    }

  return 0;
  }

// Cut selected object
long FXGLViewer::onCmdCutSel(FXObject*,FXSelector,void*){
  // Serialize object into temp buffer
  // Delete object, tell target it was deleted
  //fxwarning("%s::onCmdCutSel: unimplemented.\n",getClassName());
  return 1;
  }


// Copy selected object
long FXGLViewer::onCmdCopySel(FXObject*,FXSelector,void*){
  // Serialize object into buffer
  //fxwarning("%s::onCmdCopySel: unimplemented.\n",getClassName());
  return 1;
  }


// Paste object
long FXGLViewer::onCmdPasteSel(FXObject*,FXSelector,void*){
  // Ask clipboard for object data [What type?]
  // Deserialize data [type?]
  // Tell target about the data?
  //fxwarning("%s::onCmdPasteSel: unimplemented.\n",getClassName());
  return 1;
  }


// Delete selected object
long FXGLViewer::onCmdDeleteSel(FXObject*,FXSelector,void*){
  FXGLObject *obj[2];
  obj[0]=selection;
  obj[1]=NULL;
  if(obj[0] && obj[0]->canDelete()){
    handle(this,MKUINT(0,SEL_CHANGED),NULL);
    handle(this,MKUINT(0,SEL_DELETED),(void*)obj);
    //delete obj[0];
    }
  else{
    getApp()->beep();
    }
  return 1;
  }


// Update delete object
long FXGLViewer::onUpdDeleteSel(FXObject* sender,FXSelector,void*){
  if(selection && selection->canDelete()){
    sender->handle(this,MKUINT(ID_SHOW,SEL_COMMAND),NULL);
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
    return 1;
    }
  return 0;
  }


// Update for current object
long FXGLViewer::onUpdCurrent(FXObject* sender,FXSelector,void*){
  if(selection){
    sender->handle(this,MKUINT(ID_SHOW,SEL_COMMAND),NULL);
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
    return 1;
    }
  return 0;
  }


// Set background color
long FXGLViewer::onCmdBackColor(FXObject*,FXSelector sel,void* ptr){
  FXColor color=(FXColor)(FXuval)ptr;
  background=color;
  if(SELTYPE(sel)==SEL_COMMAND || !turbomode){
    update();
    }
  return 1;
  }


// Update background color
long FXGLViewer::onUpdBackColor(FXObject* sender,FXSelector,void*){
  FXColor clr=background;
  sender->handle(this,MKUINT(FXWindow::ID_SETVALUE,SEL_COMMAND),(void*)(FXuval)clr);
  return 1;
  }


// Set ambient light color
long FXGLViewer::onCmdAmbientColor(FXObject*,FXSelector sel,void* ptr){
  FXColor color=(FXColor)(FXuval)ptr;
  ambient=color;
  if(SELTYPE(sel)==SEL_COMMAND || !turbomode){
    update();
    }
  return 1;
  }


// Update ambient light color
long FXGLViewer::onUpdAmbientColor(FXObject* sender,FXSelector,void*){
  FXColor clr=ambient;
  sender->handle(this,MKUINT(FXWindow::ID_SETVALUE,SEL_COMMAND),(void*)(FXuval)clr);
  return 1;
  }


// Set ambient light color
long FXGLViewer::onCmdLightAmbient(FXObject*,FXSelector sel,void* ptr){
  FXColor color=(FXColor)(FXuval)ptr;
  light.ambient=color;
  if(SELTYPE(sel)==SEL_COMMAND || !turbomode){
    update();
    }
  return 1;
  }


// Update ambient light color
long FXGLViewer::onUpdLightAmbient(FXObject* sender,FXSelector,void*){
  FXColor clr=light.ambient;
  sender->handle(this,MKUINT(FXWindow::ID_SETVALUE,SEL_COMMAND),(void*)(FXuval)clr);
  return 1;
  }


// Set diffuse light color
long FXGLViewer::onCmdLightDiffuse(FXObject*,FXSelector sel,void* ptr){
  FXColor color=(FXColor)(FXuval)ptr;
  light.diffuse=color;
  if(SELTYPE(sel)==SEL_COMMAND || !turbomode){
    update();
    }
  return 1;
  }


// Update diffuse light color
long FXGLViewer::onUpdLightDiffuse(FXObject* sender,FXSelector,void*){
  FXColor clr=light.diffuse;
  sender->handle(this,MKUINT(FXWindow::ID_SETVALUE,SEL_COMMAND),(void*)(FXuval)clr);
  return 1;
  }


// Set specular light color
long FXGLViewer::onCmdLightSpecular(FXObject*,FXSelector sel,void* ptr){
  FXColor color=(FXColor)(FXuval)ptr;
  light.specular=color;
  if(SELTYPE(sel)==SEL_COMMAND || !turbomode){
    update();
    }
  return 1;
  }


// Update specular light color
long FXGLViewer::onUpdLightSpecular(FXObject* sender,FXSelector,void*){
  FXColor clr=light.specular;
  sender->handle(this,MKUINT(FXWindow::ID_SETVALUE,SEL_COMMAND),(void*)(FXuval)clr);
  return 1;
  }


// Toggle Lock view
long FXGLViewer::onCmdLock(FXObject*,FXSelector,void*){
  setViewLock(!getViewLock());
  return 1;
  }



// Update lock view
long FXGLViewer::onUpdLock(FXObject* sender,FXSelector,void*){
  FXuint msg=getViewLock() ? ID_CHECK : ID_UNCHECK;
  sender->handle(this,MKUINT(ID_SHOW,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(msg,SEL_COMMAND),NULL);
  return 1;
  }


// Toggle Turbo Mode
long FXGLViewer::onCmdTurbo(FXObject*,FXSelector,void*){
  setTurboMode(!getTurboMode());
  return 1;
  }



// Update Turbo Mode
long FXGLViewer::onUpdTurbo(FXObject* sender,FXSelector,void*){
  FXuint msg=getTurboMode() ? ID_CHECK : ID_UNCHECK;
  sender->handle(this,MKUINT(ID_SHOW,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(msg,SEL_COMMAND),NULL);
  return 1;
  }


// Toggle lighting
long FXGLViewer::onCmdLighting(FXObject*,FXSelector,void*){
  options^=VIEWER_LIGHTING;
  update();
  return 1;
  }


// Update lighting
long FXGLViewer::onUpdLighting(FXObject* sender,FXSelector,void*){
  FXuint msg=(options&VIEWER_LIGHTING) ? ID_CHECK : ID_UNCHECK;
  sender->handle(this,MKUINT(ID_SHOW,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(msg,SEL_COMMAND),NULL);
  return 1;
  }


// Toggle fog
long FXGLViewer::onCmdFog(FXObject*,FXSelector,void*){
  options^=VIEWER_FOG;
  update();
  return 1;
  }


// Update fog
long FXGLViewer::onUpdFog(FXObject* sender,FXSelector,void*){
  FXuint msg=(options&VIEWER_FOG) ? ID_CHECK : ID_UNCHECK;
  sender->handle(this,MKUINT(ID_SHOW,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(msg,SEL_COMMAND),NULL);
  return 1;
  }


// Toggle dithering
long FXGLViewer::onCmdDither(FXObject*,FXSelector,void*){
  options^=VIEWER_DITHER;
  update();
  return 1;
  }


// Update dithering
long FXGLViewer::onUpdDither(FXObject* sender,FXSelector,void*){
  FXuint msg=(options&VIEWER_DITHER) ? ID_CHECK : ID_UNCHECK;
  sender->handle(this,MKUINT(ID_SHOW,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(msg,SEL_COMMAND),NULL);
  return 1;
  }


/*******************************  Drag and Drop  *******************************/


// Handle drag-and-drop enter
long FXGLViewer::onDNDEnter(FXObject* sender,FXSelector sel,void* ptr){
  if(FXGLCanvas::onDNDEnter(sender,sel,ptr)) return 1;
  dropped=NULL;
  return 1;
  }

// Handle drag-and-drop leave
long FXGLViewer::onDNDLeave(FXObject* sender,FXSelector sel,void* ptr){
  if(FXGLCanvas::onDNDLeave(sender,sel,ptr)) return 1;
  dropped=NULL;
  return 1;
  }


// Handle drag-and-drop motion
long FXGLViewer::onDNDMotion(FXObject* sender,FXSelector sel,void* ptr){
  FXEvent* event=(FXEvent*)ptr;

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
  if(offeredDNDType(FROM_DRAGNDROP,colorType)){
    acceptDrop(DRAG_COPY);
    return 1;
    }

  // Won't accept drop, dont know what it is
  return 0;
  }


// Handle drag-and-drop drop
long FXGLViewer::onDNDDrop(FXObject* sender,FXSelector sel,void* ptr){
  FXushort *clr; FXuint len;

  // Try base class first
  if(FXGLCanvas::onDNDDrop(sender,sel,ptr)) return 1;

  // Dropped on object?
  if(dropped){

    // Object handled drop; so probably want to repaint
    if(dropped->handle(this,sel,ptr)){
      update();
      return 1;
      }

    // We're done
    return 0;
    }

  // Dropped on viewer
  if(getDNDData(FROM_DRAGNDROP,FXGLViewer::colorType,(FXuchar*&)clr,len)){
    background[0]=clr[0]/65535.0f;
    background[1]=clr[1]/65535.0f;
    background[2]=clr[2]/65535.0f;
    background[3]=1.0f;
    FXFREE(&clr);
    update();
    return 1;
    }
  return 0;
  }


// Change projection
void FXGLViewer::setProjection(FXuint proj){
  projection=proj;
  updateProjection();
  update();
  }


// Lock viewpoint
void FXGLViewer::setViewLock(FXbool lock){
  options=lock?(options|VIEWER_LOCKED):(options&~VIEWER_LOCKED);
  }


// Get locked
FXbool FXGLViewer::getViewLock() const {
  return (options&VIEWER_LOCKED)!=0;
  }


// Set background
void FXGLViewer::setBackgroundColor(const FXHVec& clr){
  background=clr;
  update();
  }


// Set ambient color
void FXGLViewer::setAmbientColor(const FXHVec& clr){
  ambient=clr;
  update();
  }


// Delegate all other messages to the GL Object
long FXGLViewer::onDefault(FXObject* sender,FXSelector key,void* data){
  return selection && selection->handle(sender,key,data);
  }


// Change surface/lines offset
void FXGLViewer::setOffset(FXdouble offs){
  if(offset!=offs){
    offset=offs;
    update();
    }
  }


// Change turbo mode
void FXGLViewer::setTurboMode(FXbool turbo){
  if(!turbo) doesturbo=FALSE;
  turbomode=turbo;
  }


// Return light settings
void FXGLViewer::getLight(FXLight& lite) const {
  lite=light;
  }


// Change light settings
void FXGLViewer::setLight(const FXLight& lite) {
  light=lite;
  update();
  }


// Save object to stream
void FXGLViewer::save(FXStream& store) const {
  FXGLCanvas::save(store);
  store << wvt.w;
  store << wvt.h;
  store << wvt.left;
  store << wvt.right;
  store << wvt.bottom;
  store << wvt.top;
  store << wvt.hither;
  store << wvt.yon;
  store << transform;
  store << itransform;
  store << projection;
  store << rotation;
  store << fov;
  store << zoom;
  store << offset;
  store << center;
  store << scale;
  store << worldpx;
  store << modelpx;
  store << maxhits;
  store << diameter;
  store << distance;
  store << background;
  store << ambient;
  store << turbomode;
  store << help;
  }



// Load object from stream
void FXGLViewer::load(FXStream& store){
  FXGLCanvas::load(store);
  store >> wvt.w;
  store >> wvt.h;
  store >> wvt.left;
  store >> wvt.right;
  store >> wvt.bottom;
  store >> wvt.top;
  store >> wvt.hither;
  store >> wvt.yon;
  store >> transform;
  store >> itransform;
  store >> projection;
  store >> rotation;
  store >> fov;
  store >> zoom;
  store >> offset;
  store >> center;
  store >> scale;
  store >> worldpx;
  store >> modelpx;
  store >> maxhits;
  store >> diameter;
  store >> distance;
  store >> background;
  store >> ambient;
  store >> turbomode;
  store >> help;
  }


// Close and release any resources
FXGLViewer::~FXGLViewer(){
  if(timer) getApp()->removeTimeout(timer);
  timer=(FXTimer*)-1;
  dropped=(FXGLObject*)-1;
  selection=(FXGLObject*)-1;
  scene=(FXGLObject*)-1;
  }
