/********************************************************************************
*                                                                               *
*                                 Test Dialog Box                               *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997 by Jeroen van der Zijp.   All Rights Reserved.             *
*********************************************************************************
* $Id: dialog.cpp,v 1.2 1998/09/23 22:54:32 jvz Exp $                      *
********************************************************************************/
#include "fx.h"
#include "FXDebugTarget.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/*******************************************************************************/

class FXTestDialog : public FXDialogBox {
  FXDECLARE(FXTestDialog)
protected:
  FXHorizontalFrame* contents;
  FXHorizontalFrame* buttons;
  FXMenuPane*        menu;
private:
  FXTestDialog(){}
public:
  FXTestDialog(FXApp *app);
  };
  
  
/*******************************************************************************/


// Mini application object
class FXDialogBoxApp : public FXApp {
  FXDECLARE(FXDialogBoxApp)
protected:
    
  // Member data
  FXMainWindow*      mainwindow;
  FXTooltip*         tooltip;
  FXMenuBar*         menubar;
  FXMenuPane*        filemenu;
  FXHorizontalFrame* contents;
  FXTestDialog*      dialog;
  
public:
  
  // Message handlers
  long onCmdShowDialog(FXObject*,FXSelector,void*);
  long onCmdShowDialogModal(FXObject*,FXSelector,void*);
  
public:
  
  // Messages
  enum {
    ID_SHOWDIALOG=FXApp::ID_LAST,
    ID_SHOWDIALOGMODAL,
    };
    
public:
  FXDialogBoxApp();
  void create();
  };


/*******************************************************************************/
  

// FXTestDialog implementation
FXIMPLEMENT(FXTestDialog,FXDialogBox,NULL,0)


// Construct a dialog box
FXTestDialog::FXTestDialog(FXApp *app):
  FXDialogBox(app,"Test of Dialog Box",DECOR_TITLE|DECOR_BORDER){
  
  // Bottom buttons
  buttons=new FXHorizontalFrame(this,LAYOUT_SIDE_BOTTOM|FRAME_NONE|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH,0,0,0,0,40,40,20,20);
  
  // Separator
  new FXHorizontalSeparator(this,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|SEPARATOR_GROOVE);
  
  // Contents
  contents=new FXHorizontalFrame(this,LAYOUT_SIDE_TOP|FRAME_NONE|LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);
  
  FXMenuPane *submenu=new FXMenuPane(getApp());
  new FXMenuCommand(submenu,"One");
  new FXMenuCommand(submenu,"Two");
  new FXMenuCommand(submenu,"Three");
    
  // Menu
  menu=new FXMenuPane(getApp());
  new FXMenuCommand(menu,"&Accept",this,ID_ACCEPT);
  new FXMenuCommand(menu,"&Cancel",this,ID_CANCEL);
  new FXMenuCascade(menu,"Submenu",submenu);
  new FXMenuCommand(menu,"&Quit",getApp(),FXApp::ID_QUIT);
  
  // Popup menu
  FXPopup *pop=new FXPopup(getApp());
  new FXOption(pop,"One",NULL,NULL,0,JUSTIFY_HZ_APART|ICON_AFTER_TEXT);
  new FXOption(pop,"Two",NULL,NULL,0,JUSTIFY_HZ_APART|ICON_AFTER_TEXT);
  new FXOption(pop,"Three",NULL,NULL,0,JUSTIFY_HZ_APART|ICON_AFTER_TEXT);
  new FXOption(pop,"Four",NULL,NULL,0,JUSTIFY_HZ_APART|ICON_AFTER_TEXT);
  new FXOption(pop,"Five",NULL,NULL,0,JUSTIFY_HZ_APART|ICON_AFTER_TEXT);
  new FXOption(pop,"Six",NULL,NULL,0,JUSTIFY_HZ_APART|ICON_AFTER_TEXT);
  new FXOption(pop,"Seven",NULL,NULL,0,JUSTIFY_HZ_APART|ICON_AFTER_TEXT);
  new FXOption(pop,"Eight",NULL,NULL,0,JUSTIFY_HZ_APART|ICON_AFTER_TEXT);
  new FXOption(pop,"Nine",NULL,NULL,0,JUSTIFY_HZ_APART|ICON_AFTER_TEXT);
  new FXOption(pop,"Ten",NULL,NULL,0,JUSTIFY_HZ_APART|ICON_AFTER_TEXT);
  
  // Option menu
  new FXOptionMenu(contents,pop,FRAME_RAISED|FRAME_THICK|JUSTIFY_HZ_APART|ICON_AFTER_TEXT|LAYOUT_CENTER_X|LAYOUT_CENTER_Y);

  // Button to pop menu
  new FXMenuButton(contents,"&Menu",NULL,menu,MENUBUTTON_DOWN|JUSTIFY_LEFT|LAYOUT_TOP|FRAME_RAISED|FRAME_THICK|ICON_AFTER_TEXT|LAYOUT_CENTER_X|LAYOUT_CENTER_Y);

  // Accept
  new FXButton(buttons,"&Accept",NULL,this,ID_ACCEPT,FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT|LAYOUT_CENTER_Y);
  
  // Cancel
  new FXButton(buttons,"&Cancel",NULL,this,ID_CANCEL,FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT|LAYOUT_CENTER_Y);
  }



/*******************************************************************************/
  
// Map
FXDEFMAP(FXDialogBoxApp) FXDialogBoxAppMap[]={
  FXMAPFUNC(SEL_COMMAND,  FXDialogBoxApp::ID_SHOWDIALOG,      FXDialogBoxApp::onCmdShowDialog),
  FXMAPFUNC(SEL_COMMAND,  FXDialogBoxApp::ID_SHOWDIALOGMODAL, FXDialogBoxApp::onCmdShowDialogModal),
  };


// FXDialogBoxApp implementation
FXIMPLEMENT(FXDialogBoxApp,FXApp,FXDialogBoxAppMap,ARRAYNUMBER(FXDialogBoxAppMap))

  
  
/*******************************************************************************/

  
// Make some windows
FXDialogBoxApp::FXDialogBoxApp(){

  // Main Window
  mainwindow=new FXMainWindow(this,"Group Box Test",DECOR_ALL,0,0,400,200);
  
  // Tooltip
  tooltip=new FXTooltip(this);
  
  // Menubar
  menubar=new FXMenuBar(mainwindow,LAYOUT_SIDE_TOP|LAYOUT_FILL_X);
  
  // Separator
  new FXHorizontalSeparator(mainwindow,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|SEPARATOR_GROOVE);

  // File Menu
  filemenu=new FXMenuPane(this);
  new FXMenuCommand(filemenu,"&Quit",this,ID_QUIT,0);
  new FXMenuTitle(menubar,"&File",filemenu);
  
  // Contents
  contents=new FXHorizontalFrame(mainwindow,LAYOUT_SIDE_TOP|FRAME_NONE|LAYOUT_FILL_X|LAYOUT_FILL_Y|PACK_UNIFORM_WIDTH);

  // Button to pop normal dialog
  new FXButton(contents,"&Non-Modal Dialog...\tDisplay normal dialog",NULL,this,ID_SHOWDIALOG,FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X|LAYOUT_CENTER_Y);
  
  // Button to pop modal dialog
  new FXButton(contents,"&Modal Dialog...\tDisplay modal dialog",NULL,this,ID_SHOWDIALOGMODAL,FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_X|LAYOUT_CENTER_Y);
  
  // Build a dialog box
  dialog=new FXTestDialog(this);
  }
  

// Open
long FXDialogBoxApp::onCmdShowDialog(FXObject*,FXSelector,void*){
  dialog->show();
  return 1;
  }


// Option
long FXDialogBoxApp::onCmdShowDialogModal(FXObject*,FXSelector sel,void*){
  dialog->execute();
  return 1;
  }


// Start
void FXDialogBoxApp::create(){
  FXApp::create();
  mainwindow->show();
  }


/*******************************************************************************/


// Start the whole thing
int main(int argc,char *argv[]){

  // Make application
  FXDialogBoxApp* application=new FXDialogBoxApp;
  
  // Open display
  application->init(argc,argv);
  
  // Create app
  application->create();
  
  // Run
  application->run();
  }


