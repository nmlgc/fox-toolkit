/********************************************************************************
*                                                                               *
*                        P r e f e r e n c e s   D i a l o g                    *
*                                                                               *
*********************************************************************************
* Copyright (C) 2003,2005 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This program is free software; you can redistribute it and/or modify          *
* it under the terms of the GNU General Public License as published by          *
* the Free Software Foundation; either version 2 of the License, or             *
* (at your option) any later version.                                           *
*                                                                               *
* This program is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
* GNU General Public License for more details.                                  *
*                                                                               *
* You should have received a copy of the GNU General Public License             *
* along with this program; if not, write to the Free Software                   *
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.    *
*********************************************************************************
* $Id: Preferences.cpp,v 1.5 2005/01/16 16:06:06 fox Exp $                     *
********************************************************************************/
#include "fx.h"
#include "PathFinder.h"
#include "Preferences.h"
#include "icons.h"


/*******************************************************************************/

// Map
FXDEFMAP(Preferences) PreferencesMap[]={
  FXMAPFUNC(SEL_COMMAND,Preferences::ID_EDITOR,Preferences::onCmdBrowseEditor),
  FXMAPFUNC(SEL_COMMAND,Preferences::ID_TERMINAL,Preferences::onCmdBrowseTerminal),
  };


// Object implementation
FXIMPLEMENT(Preferences,FXDialogBox,PreferencesMap,ARRAYNUMBER(PreferencesMap))


// Construct
Preferences::Preferences(PathFinderMain *owner):
  FXDialogBox(owner,"PathFinder Preferences",DECOR_TITLE|DECOR_BORDER|DECOR_RESIZE,0,0,500,300, 0,0,0,0, 4,4){

  FXVerticalFrame *vertical=new FXVerticalFrame(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXHorizontalFrame *horizontal=new FXHorizontalFrame(vertical,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXVerticalFrame *buttons=new FXVerticalFrame(horizontal,LAYOUT_LEFT|LAYOUT_FILL_Y|FRAME_SUNKEN|PACK_UNIFORM_WIDTH|PACK_UNIFORM_HEIGHT,0,0,0,0, 0,0,0,0, 0,0);
  FXSwitcher *switcher=new FXSwitcher(horizontal,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);

  // Icons
  pat=new FXGIFIcon(getApp(),pattern_gif);
  brw=new FXGIFIcon(getApp(),file_gif);
  icp=new FXGIFIcon(getApp(),iconpath);

  ///////////////////////////  Browser settings pane  ////////////////////////////
  FXVerticalFrame* browserpane=new FXVerticalFrame(switcher,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0, 0,0);
  new FXLabel(browserpane,"PathFinder settings",NULL,LAYOUT_LEFT);
  new FXHorizontalSeparator(browserpane,SEPARATOR_LINE|LAYOUT_FILL_X);
  FXMatrix *matrix2=new FXMatrix(browserpane,3,MATRIX_BY_COLUMNS|PACK_UNIFORM_HEIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,10,0, 6, 6);

  new FXLabel(matrix2,"Editor command:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X);
  editor=new FXTextField(matrix2,6,NULL,0,FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN,0,0,0,0, 2,2,2,2);
  new FXButton(matrix2,"Browse...",NULL,this,ID_EDITOR,FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_Y);

  new FXLabel(matrix2,"Terminal command:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X);
  terminal=new FXTextField(matrix2,6,NULL,0,FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_COLUMN,0,0,0,0, 2,2,2,2);
  new FXButton(matrix2,"Browse...",NULL,this,ID_TERMINAL,FRAME_RAISED|FRAME_THICK|LAYOUT_CENTER_Y);

  new FXLabel(matrix2,"Preview images:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X);
  preview=new FXCheckButton(matrix2,NULL,NULL,0,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN,0,0,0,0, 0,0,0,0);
  new FXFrame(matrix2,0);

  new FXLabel(matrix2,"Image blending:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X);
  blending=new FXCheckButton(matrix2,NULL,NULL,0,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_COLUMN,0,0,0,0, 0,0,0,0);
  new FXFrame(matrix2,0);

  //// Browser settings button
  new FXButton(buttons,"Browser\tFile browser settings\tChange browser settings and other things.",brw,switcher,FXSwitcher::ID_OPEN_FIRST,FRAME_RAISED|ICON_ABOVE_TEXT|LAYOUT_FILL_Y);


  ///////////////////////  File pattern settings pane  //////////////////////////
  FXVerticalFrame* filepatpane=new FXVerticalFrame(switcher,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0, 0,0);
  new FXLabel(filepatpane,"Filename patterns",NULL,LAYOUT_LEFT);
  new FXHorizontalSeparator(filepatpane,SEPARATOR_LINE|LAYOUT_FILL_X);
  FXVerticalFrame *sub3=new FXVerticalFrame(filepatpane,LAYOUT_FILL_Y|LAYOUT_FILL_X,0,0,0,0, 0,0,10,0, 0,0);
  new FXLabel(sub3,"Filename patterns, one per line:",NULL,JUSTIFY_LEFT);
  FXVerticalFrame* textwell=new FXVerticalFrame(sub3,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
  pattern=new FXText(textwell,NULL,0,LAYOUT_FILL_X|LAYOUT_FILL_Y);

  //// File pattern settings button
  new FXButton(buttons,"Patterns\tFilename patterns\tChange wildcard patterns for filenames.",pat,switcher,FXSwitcher::ID_OPEN_SECOND,FRAME_RAISED|ICON_ABOVE_TEXT|LAYOUT_FILL_Y);


  ///////////////////////  Icon path settings pane  //////////////////////////
  FXVerticalFrame* iconpathpane=new FXVerticalFrame(switcher,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0, 0,0);
  new FXLabel(iconpathpane,"Icon search path",NULL,LAYOUT_LEFT);
  new FXHorizontalSeparator(iconpathpane,SEPARATOR_LINE|LAYOUT_FILL_X);
  FXVerticalFrame *sub4=new FXVerticalFrame(iconpathpane,LAYOUT_FILL_Y|LAYOUT_FILL_X,0,0,0,0, 0,0,10,0, 0,0);
  new FXLabel(sub4,"Icon search folders, separated by '" PATHLISTSEPSTRING "', on one line:",NULL,JUSTIFY_LEFT);
  FXVerticalFrame* textbox=new FXVerticalFrame(sub4,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
  icondirs=new FXText(textbox,NULL,0,TEXT_WORDWRAP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

  //// File pattern settings button
  new FXButton(buttons,"Icon Path\tIcon search path\tChange folders searched for icons.",icp,switcher,FXSwitcher::ID_OPEN_THIRD,FRAME_RAISED|ICON_ABOVE_TEXT|LAYOUT_FILL_Y);


  // Bottom part
  new FXHorizontalSeparator(vertical,SEPARATOR_RIDGE|LAYOUT_FILL_X);
  FXHorizontalFrame *closebox=new FXHorizontalFrame(vertical,LAYOUT_BOTTOM|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH);
  new FXButton(closebox,"&Accept",NULL,this,FXDialogBox::ID_ACCEPT,BUTTON_INITIAL|BUTTON_DEFAULT|LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK,0,0,0,0, 20,20);
  new FXButton(closebox,"&Cancel",NULL,this,FXDialogBox::ID_CANCEL,BUTTON_DEFAULT|LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK,0,0,0,0, 20,20);
  }


// Set browser editor
long Preferences::onCmdBrowseEditor(FXObject*,FXSelector,void*){
  FXString neweditor=editor->getText();
  neweditor=FXFileDialog::getOpenFilename(this,"Editor Program",neweditor);
  if(!neweditor.empty()) editor->setText(neweditor);
  return 1;
  }


// Set terminal
long Preferences::onCmdBrowseTerminal(FXObject*,FXSelector,void*){
  FXString newterminal=terminal->getText();
  newterminal=FXFileDialog::getOpenFilename(this,"Terminal Program",newterminal);
  if(!newterminal.empty()) terminal->setText(newterminal);
  return 1;
  }


// Clean up
Preferences::~Preferences(){
  delete pat;
  delete brw;
  delete icp;
  }
