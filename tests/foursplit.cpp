/********************************************************************************
*                                                                               *
*                          Test 4-Way  Splitter  Widget                         *
*                                                                               *
*********************************************************************************
* Copyright (C) 1999 by Jeroen van der Zijp.   All Rights Reserved.             *
*********************************************************************************
* $Id: foursplit.cpp,v 1.14 2002/06/11 06:10:09 fox Exp $                       *
********************************************************************************/
#include "fx.h"



/*******************************************************************************/


// Mini application object
class FourSplitWindow : public FXMainWindow {
  FXDECLARE(FourSplitWindow)
protected:
  FXMenuBar*         menubar;
  FXMenuPane*        filemenu;
  FXMenuPane*        expandmenu;
  FX4Splitter*       splitter;
  FX4Splitter*       subsplitter;
protected:
  FourSplitWindow(){}
public:
  FourSplitWindow(FXApp *a);
  virtual void create();
  virtual ~FourSplitWindow();
  };



/*******************************************************************************/


// Object implementation
FXIMPLEMENT(FourSplitWindow,FXMainWindow,NULL,0)


// Make some windows
FourSplitWindow::FourSplitWindow(FXApp *a):FXMainWindow(a,"4-Way Splitter Test",NULL,NULL,DECOR_ALL,0,0,800,600,0,0){
  FXButton *temp;

  // Menu bar
  menubar=new FXMenuBar(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X);

  // Status bar
  new FXStatusBar(this,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|STATUSBAR_WITH_DRAGCORNER);

  splitter=new FX4Splitter(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y|FOURSPLITTER_TRACKING);

  // File menu
  filemenu=new FXMenuPane(this);
  new FXMenuCommand(filemenu,"&Quit\tCtl-Q\tQuit the application.",NULL,getApp(),FXApp::ID_QUIT);
  new FXMenuTitle(menubar,"&File",NULL,filemenu);

  // Expand menu
  expandmenu=new FXMenuPane(this);
  new FXMenuCommand(expandmenu,"All four",NULL,splitter,FX4Splitter::ID_EXPAND_ALL);
  new FXMenuCommand(expandmenu,"Top/left",NULL,splitter,FX4Splitter::ID_EXPAND_TOPLEFT);
  new FXMenuCommand(expandmenu,"Top/right",NULL,splitter,FX4Splitter::ID_EXPAND_TOPRIGHT);
  new FXMenuCommand(expandmenu,"Bottom/left",NULL,splitter,FX4Splitter::ID_EXPAND_BOTTOMLEFT);
  new FXMenuCommand(expandmenu,"Bottom/right",NULL,splitter,FX4Splitter::ID_EXPAND_BOTTOMRIGHT);
  new FXMenuTitle(menubar,"&Expand",NULL,expandmenu);


  // Four widgets in the four splitter
  new FXButton(splitter,"Top &Left\tThis splitter tracks",NULL,NULL,0,FRAME_RAISED|FRAME_THICK);
  new FXButton(splitter,"Top &Right\tThis splitter tracks",NULL,NULL,0,FRAME_RAISED|FRAME_THICK);
  new FXButton(splitter,"&Bottom Left\tThis splitter tracks",NULL,NULL,0,FRAME_SUNKEN|FRAME_THICK);
  subsplitter=new FX4Splitter(splitter,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  temp=new FXButton(subsplitter,"&Of course\tThis splitter does NOT track",NULL,NULL,0,FRAME_SUNKEN|FRAME_THICK);
  temp->setBackColor(FXRGB(0,128,0));
  temp->setTextColor(FXRGB(255,255,255));
  temp=new FXButton(subsplitter,"the&y CAN\tThis splitter does NOT track",NULL,NULL,0,FRAME_SUNKEN|FRAME_THICK);
  temp->setBackColor(FXRGB(128,0,0));
  temp->setTextColor(FXRGB(255,255,255));
  temp=new FXButton(subsplitter,"be &NESTED\tThis splitter does NOT track",NULL,NULL,0,FRAME_SUNKEN|FRAME_THICK);
  temp->setBackColor(FXRGB(0,0,200));
  temp->setTextColor(FXRGB(255,255,255));
  temp=new FXButton(subsplitter,"&arbitrarily!\tThis splitter does NOT track",NULL,NULL,0,FRAME_SUNKEN|FRAME_THICK);
  temp->setBackColor(FXRGB(128,128,0));
  temp->setTextColor(FXRGB(255,255,255));

  new FXToolTip(getApp());
  }


// Clean up
FourSplitWindow::~FourSplitWindow(){
  delete filemenu;
  delete expandmenu;
  }


// Start
void FourSplitWindow::create(){
  FXMainWindow::create();
  show(PLACEMENT_SCREEN);
  }


/*******************************************************************************/


// Start the whole thing
int main(int argc,char *argv[]){
  FXApp application("FourSplit","FoxTest");
  application.init(argc,argv);
  new FourSplitWindow(&application);
  application.create();
  return application.run();
  }


