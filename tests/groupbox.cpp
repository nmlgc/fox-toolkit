/********************************************************************************
*                                                                               *
*                                 Test Group Box                                *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997 by Jeroen van der Zijp.   All Rights Reserved.             *
*********************************************************************************
* $Id: groupbox.cpp,v 1.51 1998/10/29 05:38:16 jeroen Exp $                     *
********************************************************************************/
#include "fx.h"
#include "FXDebugTarget.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/*******************************************************************************/


// Mini application object
class FXGroupBoxApp : public FXApp {
  FXDECLARE(FXGroupBoxApp)
protected:
    
  // Member data
  FXMainWindow*      mainwindow;
  FXTooltip*         tooltip;
  FXMenuBar*         menubar;
  FXMenuPane*        filemenu;
  FXMenuPane*        helpmenu;
  FXHorizontalFrame* contents;
  FXPacker*          group1;
  FXGroupBox*        group2;
  FXGroupBox*        group3;
  FXuint             choice;
  
public:
  
  // Message handlers
  long onCmdOpen(FXObject*,FXSelector,void*);
  long onCmdDownSize(FXObject*,FXSelector,void*);
  long onCmdDelete(FXObject*,FXSelector,void*);
  long onCmdAbout(FXObject*,FXSelector,void*);
  long onCmdRadio(FXObject*,FXSelector,void*);
  long onCmdPopup(FXObject*,FXSelector,void*);
  long onUpdRadio(FXObject*,FXSelector,void*);
  long onCmdOption(FXObject*,FXSelector,void*);
  
public:
  
  // Messages
  enum {
    ID_DOWNSIZE=FXApp::ID_LAST,
    ID_POPUP,
    ID_ABOUT,
    ID_DELETE,
    ID_OPEN,
    ID_OPTION1,
    ID_OPTION2,
    ID_OPTION3,
    ID_OPTION4,
    ID_RADIO1,
    ID_RADIO2,
    ID_RADIO3
    };
    
public:
  FXGroupBoxApp();
  void create();
  };


  
/*******************************************************************************/
  
// Map
FXDEFMAP(FXGroupBoxApp) FXGroupBoxAppMap[]={

  //__Message_Type__________ID___________________________________________________Message_Handler_____
  FXMAPFUNC(SEL_COMMAND,  FXGroupBoxApp::ID_DOWNSIZE,                          FXGroupBoxApp::onCmdDownSize),
  FXMAPFUNC(SEL_COMMAND,  FXGroupBoxApp::ID_DELETE,                            FXGroupBoxApp::onCmdDelete),
  FXMAPFUNCS(SEL_COMMAND, FXGroupBoxApp::ID_RADIO1,FXGroupBoxApp::ID_RADIO3,   FXGroupBoxApp::onCmdRadio),
  FXMAPFUNCS(SEL_UPDATE,  FXGroupBoxApp::ID_RADIO1,FXGroupBoxApp::ID_RADIO3,   FXGroupBoxApp::onUpdRadio),
  FXMAPFUNC(SEL_COMMAND,  FXGroupBoxApp::ID_POPUP,                             FXGroupBoxApp::onCmdPopup),
  FXMAPFUNC(SEL_COMMAND,  FXGroupBoxApp::ID_ABOUT,                             FXGroupBoxApp::onCmdAbout),
  FXMAPFUNC(SEL_COMMAND,  FXGroupBoxApp::ID_OPEN,                              FXGroupBoxApp::onCmdOpen),
  FXMAPFUNCS(SEL_COMMAND, FXGroupBoxApp::ID_OPTION1,FXGroupBoxApp::ID_OPTION4, FXGroupBoxApp::onCmdOption),
  };


// Object implementation
FXIMPLEMENT(FXGroupBoxApp,FXApp,FXGroupBoxAppMap,ARRAYNUMBER(FXGroupBoxAppMap))

  
  
/*******************************************************************************/

  
// Make some windows
FXGroupBoxApp::FXGroupBoxApp(){
  mainwindow=new FXMainWindow(this,"Group Box Test",DECOR_ALL,0,0,800,600);
  tooltip=new FXTooltip(this,0,100,100);
  
  // Menubar
  menubar=new FXMenuBar(mainwindow,LAYOUT_SIDE_TOP|LAYOUT_FILL_X);
  filemenu=new FXMenuPane(this);
    new FXMenuCommand(filemenu,"&Open\tCtl-O\tOpen a file.",this,ID_OPEN,MENU_DEFAULT);
    new FXMenuCommand(filemenu,"&Show Tooltip\tAlt-S",tooltip,FXWindow::ID_SHOW,0);
    new FXMenuCommand(filemenu,"&Hide Tooltip\tAlt-H",tooltip,FXWindow::ID_HIDE,0);
    new FXMenuCommand(filemenu,"Delete\tCtl-X",this,ID_DELETE,0);
    new FXMenuCommand(filemenu,"Radio1",this,ID_RADIO1,0);
    new FXMenuCommand(filemenu,"Radio2",this,ID_RADIO2,0);
    new FXMenuCommand(filemenu,"Radio3",this,ID_RADIO3,0);
    new FXMenuCommand(filemenu,"Downsize",this,ID_DOWNSIZE,0);
    
    // Make edit popup menu
    FXMenuPane *editmenu=new FXMenuPane(this);
      new FXMenuCommand(editmenu,"Undo");
      new FXMenuCommand(editmenu,"Cut");
        FXMenuPane *submenu1=new FXMenuPane(this);
          new FXMenuCommand(submenu1,"One");
          new FXMenuCommand(submenu1,"Two",NULL,0);
          new FXMenuCommand(submenu1,"Three",NULL,0);
          new FXMenuCommand(submenu1,"Four");
          new FXMenuCommand(submenu1,"Four");
          new FXMenuCommand(submenu1,"Four");
      new FXMenuCascade(editmenu,"Submenu1",submenu1);
    new FXMenuCascade(filemenu,"&Edit",editmenu);
    new FXMenuCommand(filemenu,"&Quit",this,ID_QUIT,0);
  new FXMenuTitle(menubar,"&File",filemenu);
  
  helpmenu=new FXMenuPane(this);
    new FXMenuCommand(helpmenu,"&About FOX...",this,ID_ABOUT,0);
  new FXMenuTitle(menubar,"&Help",helpmenu,LAYOUT_RIGHT);

  
  
  // Status bar
  FXStatusbar *status=new FXStatusbar(mainwindow,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|STATUSBAR_WITH_DRAGCORNER);
  
  new FXLabel(status,"10:15 PM",NULL,LAYOUT_FILL_Y|LAYOUT_RIGHT|FRAME_SUNKEN);

  // Content
  contents=new FXHorizontalFrame(mainwindow,LAYOUT_SIDE_TOP|FRAME_NONE|LAYOUT_FILL_X|LAYOUT_FILL_Y);

  group1=new FXGroupBox(contents,"Title Left",GROUPBOX_TITLE_LEFT|FRAME_RIDGE|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  group2=new FXGroupBox(contents,"Slider Tests",GROUPBOX_TITLE_CENTER|FRAME_RIDGE|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  group3=new FXGroupBox(contents,"Title Right",GROUPBOX_TITLE_RIGHT|FRAME_RIDGE|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  
  new FXLabel(group1,"&Now is the time\nfor all good men\nto come to the aid\nof their country",NULL,LAYOUT_CENTER_X|JUSTIFY_CENTER_X|FRAME_RAISED);
  new FXButton(group1,"Small &Button",NULL,NULL,0,LAYOUT_BOTTOM|FRAME_RAISED|FRAME_THICK);
  new FXButton(group1,"Downsize",NULL,this,ID_DOWNSIZE,FRAME_RAISED|FRAME_THICK);
  new FXButton(group1,"Big Fat Wide Button\nComprising\nthree lines",NULL,NULL,0,FRAME_RAISED|FRAME_THICK);

  FXPopup *pop=new FXPopup(this);
  
  new FXOption(pop,"First\tTip #1\tHelp first",NULL,this,ID_OPTION1,JUSTIFY_HZ_APART|ICON_AFTER_TEXT);
  new FXOption(pop,"Second\tTip #2\tHelp second",NULL,this,ID_OPTION2,JUSTIFY_HZ_APART|ICON_AFTER_TEXT);
  new FXOption(pop,"Third\tTip #3\tHelp third",NULL,this,ID_OPTION3,JUSTIFY_HZ_APART|ICON_AFTER_TEXT);
  new FXOption(pop,"Fourth\tTip #4\tHelp fourth",NULL,this,ID_OPTION4,JUSTIFY_HZ_APART|ICON_AFTER_TEXT);
  
  new FXOptionMenu(group1,pop,LAYOUT_TOP|FRAME_RAISED|FRAME_THICK|JUSTIFY_HZ_APART|ICON_AFTER_TEXT);

  new FXLabel(group1,"Te&kstje",NULL,LAYOUT_TOP|JUSTIFY_LEFT);
  new FXButton(group1,"Add an `&&' by doubling\tTooltip\tHelp text for status",NULL,NULL,0,LAYOUT_TOP|FRAME_RAISED|FRAME_THICK);
  new FXButton(group1,"Te&kstje",NULL,this,ID_POPUP,LAYOUT_TOP|FRAME_RAISED|FRAME_THICK);
  
  new FXMenuButton(group1,"&Menu",NULL,filemenu,MENUBUTTON_DOWN|JUSTIFY_LEFT|LAYOUT_TOP|FRAME_RAISED|FRAME_THICK|ICON_AFTER_TEXT);
  new FXMenuButton(group1,"&Menu",NULL,filemenu,MENUBUTTON_UP|LAYOUT_TOP|FRAME_RAISED|FRAME_THICK|ICON_AFTER_TEXT);

  FXMatrix* matrix=new FXMatrix(group1,3,FRAME_RAISED|LAYOUT_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  
  new FXButton(matrix,"A",NULL,NULL,0,FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_ROW);
  new FXButton(matrix,"AAAAAAAAAA",NULL,NULL,0,FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_X);
  new FXButton(matrix,"A",NULL,NULL,0,FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_X);
  
  new FXButton(matrix,"BB",NULL,NULL,0,FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN);
  new FXButton(matrix,"B",NULL,NULL,0,FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN);
  new FXButton(matrix,"BB",NULL,NULL,0,FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN);
  
  new FXButton(matrix,"C",NULL,NULL,0,FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_CENTER_X|LAYOUT_FILL_ROW);
  new FXButton(matrix,"CCCC",NULL,NULL,0,FRAME_RAISED|FRAME_THICK);
  new FXButton(matrix,"CC",NULL,NULL,0,FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT);
  
  new FXLabel(group2,"No Arrow");
  new FXSlider(group2,NULL,0,LAYOUT_TOP|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|SLIDER_HORIZONTAL,0,0,200,20);
  
  new FXLabel(group2,"Up Arrow");
  new FXSlider(group2,NULL,0,LAYOUT_TOP|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|SLIDER_HORIZONTAL|SLIDER_ARROW_UP,0,0,200,20);
  
  new FXLabel(group2,"Down Arrow");
  new FXSlider(group2,NULL,0,LAYOUT_TOP|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|SLIDER_HORIZONTAL|SLIDER_ARROW_DOWN,0,0,200,20);
  
  new FXLabel(group2,"Inside Bar");
  new FXSlider(group2,NULL,0,LAYOUT_TOP|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|SLIDER_HORIZONTAL|SLIDER_INSIDE_BAR,0,0,200,14);  
  
  FXHorizontalFrame *frame=new FXHorizontalFrame(group2,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  
  new FXSlider(frame,NULL,0,LAYOUT_FIX_HEIGHT|SLIDER_VERTICAL,0,0,20,200);
  new FXSlider(frame,NULL,0,LAYOUT_FIX_HEIGHT|SLIDER_VERTICAL|SLIDER_ARROW_RIGHT,0,0,20,200);  
  new FXSlider(frame,NULL,0,LAYOUT_FIX_HEIGHT|SLIDER_VERTICAL|SLIDER_ARROW_LEFT,0,0,20,200);
  new FXSlider(frame,NULL,0,LAYOUT_FIX_HEIGHT|SLIDER_VERTICAL|SLIDER_INSIDE_BAR,0,0,0,200);

  FXGroupBox *gp=new FXGroupBox(group3,"Group Box",LAYOUT_SIDE_TOP|FRAME_GROOVE|LAYOUT_FILL_X, 0,0,0,0);
  new FXRadioButton(gp,"Hilversum 1",NULL,0,ICON_BEFORE_TEXT|LAYOUT_SIDE_TOP);
  new FXRadioButton(gp,"Hilversum 2",NULL,0,ICON_BEFORE_TEXT|LAYOUT_SIDE_TOP);
  new FXRadioButton(gp,"Hilversum 3",NULL,0,ICON_BEFORE_TEXT|LAYOUT_SIDE_TOP);
  new FXRadioButton(gp,"Radio Stad Amsterdam",NULL,0,ICON_BEFORE_TEXT|LAYOUT_SIDE_TOP);
  
  FXPacker *vv=new FXGroupBox(group3,"Group Box",LAYOUT_SIDE_TOP|FRAME_GROOVE|LAYOUT_FILL_X, 0,0,0,0);
  new FXCheckButton(vv,"Hilversum 1",NULL,0,ICON_BEFORE_TEXT|LAYOUT_SIDE_TOP);
  new FXCheckButton(vv,"Hilversum 2",NULL,0,ICON_BEFORE_TEXT|LAYOUT_SIDE_TOP);
  new FXCheckButton(vv,"Hilversum 3",NULL,0,ICON_BEFORE_TEXT|LAYOUT_SIDE_TOP);
  new FXCheckButton(vv,"Radio Stad Amsterdam",NULL,0,ICON_BEFORE_TEXT|LAYOUT_SIDE_TOP);
  
  FXSpinner *spinner=new FXSpinner(group3,20,NULL,0,SPIN_NORMAL|FRAME_SUNKEN|FRAME_THICK|LAYOUT_SIDE_TOP);
  spinner->setMinMax(0,1000);
  
  new FXListBox(group3,20,NULL,0,FRAME_SUNKEN|FRAME_THICK|LAYOUT_SIDE_TOP);
  
  new FXLabel(group3,"H&it the hotkey",NULL,LAYOUT_CENTER_X|JUSTIFY_CENTER_X|FRAME_RAISED);
  new FXTextField(group3,20,NULL,0,FRAME_SUNKEN|FRAME_THICK|LAYOUT_SIDE_TOP);
  
  new FXDial(group3,NULL,0,DIAL_CYCLIC|DIAL_HAS_NOTCH|DIAL_VERTICAL|LAYOUT_FIX_HEIGHT|FRAME_RAISED|FRAME_THICK,0,0,0,120);
  new FXDial(group3,NULL,0,DIAL_CYCLIC|DIAL_HAS_NOTCH|DIAL_HORIZONTAL|LAYOUT_FILL_X|FRAME_RAISED|FRAME_THICK,0,0,120,0);
  
  FXProgressBar *pbar=new FXProgressBar(group3,NULL,0,LAYOUT_FILL_X|FRAME_SUNKEN|FRAME_THICK|PROGRESSBAR_PERCENTAGE);
  pbar->setProgress(48);
  FXProgressBar *pbar2=new FXProgressBar(group3,NULL,0,LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK|PROGRESSBAR_VERTICAL|PROGRESSBAR_PERCENTAGE);
  pbar2->setProgress(48);
  choice=0;
  }
  

// Open
long FXGroupBoxApp::onCmdOpen(FXObject*,FXSelector,void*){
  FXFileDialog open(this,"Open some file");
  if(open.execute()){
    char filename[1000];
    strcpy(filename,open.getFilename());
    }
  return 1;
  }


// Option
long FXGroupBoxApp::onCmdOption(FXObject*,FXSelector sel,void*){
  fprintf(stderr,"Chose option %d\n",SELID(sel)-ID_OPTION1+1);
  return 1;
  }


// Test something
long FXGroupBoxApp::onCmdDownSize(FXObject*,FXSelector,void*){
  mainwindow->resize(mainwindow->getDefaultWidth(),mainwindow->getDefaultHeight());
  return 1;
  }


// Test delete
long FXGroupBoxApp::onCmdDelete(FXObject*,FXSelector,void*){
  delete group2;
  group2=NULL;
  return 1;
  }


// Pop up menu
long FXGroupBoxApp::onCmdPopup(FXObject*,FXSelector,void* ptr){
  FXint x,y; FXuint buttons;
  getRoot()->getCursorPosition(x,y,buttons);
  filemenu->popup(NULL,x,y);
  return 1;
  }


// Set choice
long FXGroupBoxApp::onCmdRadio(FXObject*,FXSelector sel,void*){
  choice=SELID(sel);
  return 1;
  }


// Update menu
long FXGroupBoxApp::onUpdRadio(FXObject* sender,FXSelector sel,void*){
  FXMenuCommand *cmd=(FXMenuCommand*)sender;
  (SELID(sel)==choice) ? cmd->checkRadio() : cmd->uncheckRadio();
  return 1;
  }


// About
long FXGroupBoxApp::onCmdAbout(FXObject*,FXSelector,void*){
  showModalInformationBox(MBOX_OK,"About FOX","FOX is a really, really cool C++ library!\nExample written by Jeroen");
  return 1;
  }


// Start
void FXGroupBoxApp::create(){
  FXApp::create();
  mainwindow->show();
  }


/*******************************************************************************/


// Start the whole thing
int main(int argc,char *argv[]){

  // Make application
  FXGroupBoxApp* application=new FXGroupBoxApp;
  
  // Open display
  application->init(argc,argv);
  
  // Create app
  application->create();
  
  // Run
  application->run();
  }


