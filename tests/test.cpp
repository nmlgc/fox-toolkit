/********************************************************************************
*                                                                               *
*                                   T e s t                                     *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997 by Jeroen van der Zijp.   All Rights Reserved.             *
*********************************************************************************
* $Id: test.cpp,v 1.1.1.1 1998/02/19 17:50:11 jeroen Exp $                        *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXEditBuffer.h"
#include "FXObject.h"
#include "FXObjectList.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXDrawable.h"
#include "FXImage.h"
#include "FXIcon.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXLabel.h"
#include "FXTextField.h"
#include "FXButton.h"
#include "FXComposite.h"
#include "FXShell.h"
#include "FXMenu.h"
#include "FXTab.h"
#include "FXScrollbar.h"
#include "FXScrollWindow.h"
#include "FXList.h"
#include "FXTreeList.h"
#include "FXMessageBox.h"

FXApp* application;

FXWindow *killitem=NULL;


#define file_width 42
#define file_height 32
#define file_x_hot 1
#define file_y_hot 1
static char file_bits[] = {
   0x00, 0xc0, 0xff, 0xff, 0x07, 0x00, 0x02, 0x40, 0x00, 0x00, 0x0a, 0x00,
   0x06, 0x40, 0x00, 0x00, 0x12, 0x00, 0x0e, 0x40, 0x00, 0x00, 0x22, 0x00,
   0x1e, 0x40, 0x00, 0x00, 0x3e, 0x00, 0x3e, 0x40, 0x00, 0x00, 0x20, 0x00,
   0x7e, 0x40, 0x00, 0x00, 0x20, 0x00, 0xfe, 0x40, 0x00, 0x00, 0x20, 0x00,
   0xfe, 0x41, 0x00, 0x00, 0x20, 0x00, 0x3e, 0x40, 0x00, 0x00, 0x20, 0x00,
   0x36, 0x40, 0x00, 0x00, 0x20, 0x00, 0x62, 0x40, 0x00, 0x00, 0x20, 0x00,
   0x60, 0x40, 0x00, 0x00, 0x20, 0x00, 0xc0, 0x40, 0x00, 0x00, 0x20, 0x00,
   0xc0, 0x40, 0x00, 0x00, 0x20, 0x00, 0x00, 0x40, 0x00, 0x00, 0x20, 0x00,
   0x00, 0x40, 0x00, 0x00, 0x20, 0x00, 0x00, 0x40, 0x00, 0x00, 0x20, 0x00,
   0x00, 0x40, 0x00, 0x00, 0x20, 0x00, 0x00, 0x40, 0x00, 0x00, 0x20, 0x00,
   0x00, 0x40, 0x00, 0x00, 0x20, 0x00, 0x00, 0x40, 0x00, 0x00, 0x20, 0x00,
   0x00, 0x40, 0x00, 0x00, 0x20, 0x00, 0x00, 0x40, 0x00, 0x00, 0x20, 0x00,
   0x00, 0x40, 0x00, 0x00, 0x20, 0x00, 0x00, 0x40, 0x00, 0x00, 0x20, 0x00,
   0x00, 0x40, 0x00, 0x00, 0x20, 0x00, 0x00, 0x40, 0x00, 0x00, 0x20, 0x00,
   0x00, 0x40, 0x00, 0x00, 0x20, 0x00, 0x00, 0x40, 0x00, 0x00, 0x20, 0x00,
   0x00, 0xc0, 0xff, 0xff, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


#define file_mask_width 42
#define file_mask_height 32
#define file_mask_x_hot 1
#define file_mask_y_hot 1
static char file_mask_bits[] = {
   0x03, 0xc0, 0xff, 0xff, 0x07, 0x00, 0x07, 0xc0, 0xff, 0xff, 0x0f, 0x00,
   0x0f, 0xc0, 0xff, 0xff, 0x1f, 0x00, 0x1f, 0xc0, 0xff, 0xff, 0x3f, 0x00,
   0x3f, 0xc0, 0xff, 0xff, 0x3f, 0x00, 0x7f, 0xc0, 0xff, 0xff, 0x3f, 0x00,
   0xff, 0xc0, 0xff, 0xff, 0x3f, 0x00, 0xff, 0xc1, 0xff, 0xff, 0x3f, 0x00,
   0xff, 0xc3, 0xff, 0xff, 0x3f, 0x00, 0xff, 0xc3, 0xff, 0xff, 0x3f, 0x00,
   0x7f, 0xc0, 0xff, 0xff, 0x3f, 0x00, 0xf7, 0xc0, 0xff, 0xff, 0x3f, 0x00,
   0xf3, 0xc0, 0xff, 0xff, 0x3f, 0x00, 0xe0, 0xc1, 0xff, 0xff, 0x3f, 0x00,
   0xe0, 0xc1, 0xff, 0xff, 0x3f, 0x00, 0xc0, 0xc0, 0xff, 0xff, 0x3f, 0x00,
   0x00, 0xc0, 0xff, 0xff, 0x3f, 0x00, 0x00, 0xc0, 0xff, 0xff, 0x3f, 0x00,
   0x00, 0xc0, 0xff, 0xff, 0x3f, 0x00, 0x00, 0xc0, 0xff, 0xff, 0x3f, 0x00,
   0x00, 0xc0, 0xff, 0xff, 0x3f, 0x00, 0x00, 0xc0, 0xff, 0xff, 0x3f, 0x00,
   0x00, 0xc0, 0xff, 0xff, 0x3f, 0x00, 0x00, 0xc0, 0xff, 0xff, 0x3f, 0x00,
   0x00, 0xc0, 0xff, 0xff, 0x3f, 0x00, 0x00, 0xc0, 0xff, 0xff, 0x3f, 0x00,
   0x00, 0xc0, 0xff, 0xff, 0x3f, 0x00, 0x00, 0xc0, 0xff, 0xff, 0x3f, 0x00,
   0x00, 0xc0, 0xff, 0xff, 0x3f, 0x00, 0x00, 0xc0, 0xff, 0xff, 0x3f, 0x00,
   0x00, 0xc0, 0xff, 0xff, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


// long FXApp::onTest(FXObject* sender,FXSelector sel,void* ptr){
//   if(SELTYPE(sel)==SEL_DRAG){
//     fprintf(stderr,"Drag\n");
//     setDNDData((FXuchar*)"Hello\n",7);
//     FXCursor *cursor=new FXCursor(this);
//     cursor->createFromBitmapData(file_bits,file_mask_bits,file_width,file_height,file_x_hot,file_y_hot);
//     runDND((FXWindow*)sender,cursor,DND_RAWDATA);
//     delete cursor;
//     }
//   else if(SELTYPE(sel)==SEL_DROP){
//     FXEvent *ev=(FXEvent*)ptr;
//     FXuchar *buf;
//     FXuint size;
//     int dropx=(int)((short)(ev->event.xclient.data.l[3]&0xffff));
//     int dropy=(int)((short)(ev->event.xclient.data.l[3]>>16));
//     fprintf(stderr,"Drop type=%d btns=%x source=%d x=%d y=%d\n",ev->event.xclient.data.l[0],ev->event.xclient.data.l[1],ev->event.xclient.data.l[2],dropx,dropy);
//     if(getDNDData(buf,size)){
//       fprintf(stderr,"Got %s\n",buf);
//       freeDNDData(buf);
//       }
//     }
//   else if(SELTYPE(sel)==SEL_COMMAND){
//   extern FXMenuPane *popuppane;
//     popuppane->popUp(200,200);
//     popuppane->grab(ButtonPressMask|ButtonReleaseMask|PointerMotionMask);
//     }
//   return 1;
//   }



class FXTestController : public FXObject {
  FXDECLARE(FXTestController)
public:
  long onTest(FXObject*,FXSelector sel,void* ptr);
  long onQuit(FXObject*,FXSelector sel,void* ptr);
  };



// Map
FXDEFMAP(FXTestController) FXTestControllerMap[]={
  FXMAPFUNC(SEL_COMMAND,1,FXTestController::onTest),
  FXMAPFUNC(SEL_COMMAND,2,FXTestController::onQuit),
  };


// Implementation
FXIMPLEMENT(FXTestController,FXObject,FXTestControllerMap,ARRAYNUMBER(FXTestControllerMap))


long FXTestController::onTest(FXObject*,FXSelector sel,void* ptr){
fprintf(stderr,"hello\n");
  FXMessageBox box(application,"Title","Text",NULL,MBOX_OK);
  box.create();
  box.layout();
  box.show();
  }

long FXTestController::onQuit(FXObject*,FXSelector sel,void* ptr){
  ::exit(0);
  }



FXTestController controller;


int main(int argc,char *argv[]){


  // Make application
  application=new FXApp;
  
  // Open display
  application->openDisplay(":0.0");
    
  // Make my own main window
  FXMainWindow* main=new FXMainWindow(application,"Periodic Table",PACKING_VERTICAL,0,0,800,600);

  // Make menubar
  FXMenuBar* menubar=new FXMenuBar(main);

  // Make file popup menu
  FXMenuPane *filemenu=new FXMenuPane(application);
  new FXMenuCommand(filemenu,"Kill",&controller,1,MENU_ENABLED|MENU_DEFAULT);
  new FXMenuCommand(filemenu,"Exit",&controller,2);
//  filemenu->layout();

  // Hang it under menubar
  new FXMenuTitle(menubar,"File",filemenu);
    
//   // Make Periodic Table
//   FXHorizontalFrame *parent=new FXHorizontalFrame(main,FRAME_LINE|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_LEFT);
// 
// 
//   // Buttons
//   new FXLabel(parent,"D",NULL,FRAME_LINE|JUSTIFY_CENTER_X|LAYOUT_FILL_X|LAYOUT_CENTER_X|JUSTIFY_CENTER_Y);
//   new FXLabel(parent,"XXXXXXXXXXXX",NULL,FRAME_LINE|JUSTIFY_CENTER_X|LAYOUT_FILL_X|LAYOUT_CENTER_X|JUSTIFY_CENTER_Y);
//   new FXLabel(parent,"Big",NULL,FRAME_LINE|JUSTIFY_CENTER_X|LAYOUT_FILL_X|JUSTIFY_CENTER_Y);
//   new FXLabel(parent,"XXXXX",NULL,FRAME_LINE|JUSTIFY_CENTER_X|LAYOUT_CENTER_X|JUSTIFY_CENTER_Y);
//   new FXLabel(parent,"A",NULL,FRAME_LINE|JUSTIFY_CENTER_X|LAYOUT_FILL_X|LAYOUT_CENTER_X|JUSTIFY_CENTER_Y);
//   new FXLabel(parent,"D",NULL,FRAME_LINE|JUSTIFY_CENTER_X|LAYOUT_FILL_X|LAYOUT_CENTER_X|JUSTIFY_CENTER_Y);

  // Make Periodic Table
  FXGroupBox *parent=new FXGroupBox(main,"Group Box",PACKING_HORIZONTAL|FRAME_LINE|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_LEFT);

//  new FXLabel(parent,"D",NULL,FRAME_LINE|JUSTIFY_CENTER_X|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_CENTER_X|JUSTIFY_CENTER_Y);
  new FXLabel(parent,"Q",NULL,FRAME_LINE|JUSTIFY_CENTER_X|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_CENTER_X|JUSTIFY_CENTER_Y);
//  new FXLabel(parent,"Big",NULL,FRAME_LINE|JUSTIFY_CENTER_X|LAYOUT_FILL_X|LAYOUT_FILL_Y|JUSTIFY_CENTER_Y);
//  new FXLabel(parent,"XXXXX",NULL,FRAME_LINE|JUSTIFY_CENTER_X|LAYOUT_CENTER_X|LAYOUT_FILL_Y|JUSTIFY_CENTER_Y);
//  new FXLabel(parent,"A",NULL,FRAME_LINE|JUSTIFY_CENTER_X|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_CENTER_X|JUSTIFY_CENTER_Y);
//  new FXLabel(parent,"D",NULL,FRAME_LINE|JUSTIFY_CENTER_X|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_CENTER_X|JUSTIFY_CENTER_Y);

  // Buttons
//   new FXLabel(parent,"D",NULL,FRAME_LINE|JUSTIFY_CENTER_X|LAYOUT_FILL_Y|LAYOUT_CENTER_Y|JUSTIFY_CENTER_Y);
//   new FXLabel(parent,"XXXXXXXXXXXX",NULL,FRAME_LINE|JUSTIFY_CENTER_X|LAYOUT_FILL_Y|LAYOUT_CENTER_Y|JUSTIFY_CENTER_Y);
//   new FXLabel(parent,"Big",NULL,FRAME_LINE|JUSTIFY_CENTER_X|LAYOUT_FILL_Y|JUSTIFY_CENTER_X);
//   new FXLabel(parent,"XXXXX",NULL,FRAME_LINE|JUSTIFY_CENTER_X|LAYOUT_LEFT|JUSTIFY_CENTER_Y);
//   new FXLabel(parent,"A",NULL,FRAME_LINE|JUSTIFY_CENTER_X|LAYOUT_FILL_Y|LAYOUT_LEFT|JUSTIFY_CENTER_Y);
//   new FXLabel(parent,"D",NULL,FRAME_LINE|JUSTIFY_CENTER_X|LAYOUT_FILL_Y|LAYOUT_LEFT|JUSTIFY_CENTER_Y);


  // Resize and display main window
  application->create();
  main->layout();
  main->show();

  // Run
  application->run();
  }


