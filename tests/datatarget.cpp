/********************************************************************************
*                                                                               *
*                                 Data Target Test                              *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997 by Jeroen van der Zijp.   All Rights Reserved.             *
*********************************************************************************
* $Id: datatarget.cpp,v 1.5 1998/10/27 04:57:46 jeroen Exp $                      *
********************************************************************************/
#include "fx.h"
#include "FXDebugTarget.h"
#include "FXDataTarget.h"
#include <stdio.h>
#include <stdlib.h>


/*******************************************************************************/


// Mini application object
class FXDataTargetApp : public FXApp {
  FXDECLARE(FXDataTargetApp)
protected:
  FXMainWindow*      mainwindow;
  FXMenuBar*         menubar;
  FXMenuPane*        filemenu;
  FXMatrix*          matrix;
  FXint              some_int;
  FXdouble           some_double;
  FXString           some_string;
  FXDataTarget*      int_target;
  FXDataTarget*      double_target;
  FXDataTarget*      string_target;
public:
  FXDataTargetApp();
  void create();
  };


  
/*******************************************************************************/
  
// Object implementation
FXIMPLEMENT(FXDataTargetApp,FXApp,NULL,0)


// Make some windows
FXDataTargetApp::FXDataTargetApp(){
  
  mainwindow=new FXMainWindow(this,"Data Target Test",DECOR_ALL,0,0,0,0);
  
  // Menubar
  menubar=new FXMenuBar(mainwindow,LAYOUT_SIDE_TOP|LAYOUT_FILL_X);
  filemenu=new FXMenuPane(this);
    new FXMenuCommand(filemenu,"&Quit\tCtl-Q",this,FXApp::ID_QUIT,0);
  new FXMenuTitle(menubar,"&File",filemenu);
  
  new FXHorizontalSeparator(mainwindow,LAYOUT_SIDE_TOP|SEPARATOR_GROOVE|LAYOUT_FILL_X);
  
  // Arange nicely
  matrix=new FXMatrix(mainwindow,5,MATRIX_BY_COLUMNS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  
  some_int = 10;
  some_double = 3.1415927;
  some_string = "FOX";
  
  
  // Make INTEGER target
  int_target = new FXDataTarget(some_int);
  
  // Make DOUBLE target
  double_target = new FXDataTarget(some_double);
  
  // Make STRING target
  string_target = new FXDataTarget(some_string);
  
  // First row
  new FXLabel(matrix,"Integer",NULL,LAYOUT_CENTER_Y|LAYOUT_CENTER_X|JUSTIFY_RIGHT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  
  new FXTextField(matrix,10,int_target,FXDataTarget::ID_VALUE,LAYOUT_CENTER_Y|LAYOUT_CENTER_X|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  
  new FXTextField(matrix,10,int_target,FXDataTarget::ID_VALUE,LAYOUT_CENTER_Y|LAYOUT_CENTER_X|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  
  new FXSlider(matrix,int_target,FXDataTarget::ID_VALUE,LAYOUT_CENTER_Y|LAYOUT_CENTER_X|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FIX_WIDTH,0,0,100);
  
  new FXDial(matrix,int_target,FXDataTarget::ID_VALUE,LAYOUT_CENTER_Y|LAYOUT_CENTER_X|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FIX_WIDTH|DIAL_HORIZONTAL|DIAL_HAS_NOTCH,0,0,100);
  
  // Second row
  new FXLabel(matrix,"Real",NULL,LAYOUT_CENTER_Y|LAYOUT_CENTER_X|JUSTIFY_RIGHT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  
  new FXTextField(matrix,10,double_target,FXDataTarget::ID_VALUE,LAYOUT_CENTER_Y|LAYOUT_CENTER_X|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  
  new FXTextField(matrix,10,double_target,FXDataTarget::ID_VALUE,LAYOUT_CENTER_Y|LAYOUT_CENTER_X|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  
  new FXSlider(matrix,double_target,FXDataTarget::ID_VALUE,LAYOUT_CENTER_Y|LAYOUT_CENTER_X|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FIX_WIDTH,0,0,100);
  
  new FXDial(matrix,double_target,FXDataTarget::ID_VALUE,LAYOUT_CENTER_Y|LAYOUT_CENTER_X|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW|LAYOUT_FIX_WIDTH|DIAL_HORIZONTAL|DIAL_HAS_NOTCH,0,0,100);

  // Third row
  new FXLabel(matrix,"String",NULL,LAYOUT_CENTER_Y|LAYOUT_CENTER_X|JUSTIFY_RIGHT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  
  new FXTextField(matrix,10,string_target,FXDataTarget::ID_VALUE,LAYOUT_CENTER_Y|LAYOUT_CENTER_X|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  
  new FXTextField(matrix,10,string_target,FXDataTarget::ID_VALUE,LAYOUT_CENTER_Y|LAYOUT_CENTER_X|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  
  // Install an accelerator
  mainwindow->getAccelTable()->addAccel(fxparseaccel("Ctl-Q"),this,MKUINT(FXApp::ID_QUIT,SEL_COMMAND));
  }
  

// Start
void FXDataTargetApp::create(){
  FXApp::create();
  mainwindow->show();
  }


/*******************************************************************************/


// Start the whole thing
int main(int argc,char *argv[]){

  // Make application
  FXDataTargetApp* application=new FXDataTargetApp;
  
  // Open display
  application->init(argc,argv);
  
  // Create app
  application->create();
  
  // Run
  application->run();
  }
