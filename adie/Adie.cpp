/********************************************************************************
*                                                                               *
*                     T h e   A d i e   T e x t   E d i t o r                   *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: Adie.cpp,v 1.67.4.1 2003/06/20 19:02:07 fox Exp $                            *
********************************************************************************/
#include "fx.h"
#include "fxkeys.h"
#include "FXRex.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <signal.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <ctype.h>
#include "HelpWindow.h"
#include "Preferences.h"
#include "Commands.h"
#include "Hilite.h"
#include "Adie.h"
#include "icons.h"

/*
  Note:
  - Regular expression search/replace.
  - Multiple toplevel windows.
  - Commenting/uncommenting source code (various programming languages, of course).
  - Block select (and block operations).
  - Better icons.
  - Syntax highlighting (programmable).
  - Shell commands.
  - Each style has optional parent; colors with value FXRGBA(0,0,0,0) are
    inherited from the parent; this way, sub-styles are possible.
  - Bookmarks should save top and current line number, not character position.
*/

#define CLOCKTIMER 500
#define FILETIMER  1000


#define TEST 1

/*******************************************************************************/

// Map
FXDEFMAP(TextWindow) TextWindowMap[]={
  FXMAPFUNC(SEL_TIMEOUT,           TextWindow::ID_FILETIME,        TextWindow::onCheckFile),
  FXMAPFUNC(SEL_TIMEOUT,           TextWindow::ID_CLOCKTIME,       TextWindow::onClock),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_ABOUT,           TextWindow::onCmdAbout),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_REOPEN,          TextWindow::onCmdReopen),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_REOPEN,          TextWindow::onUpdReopen),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_OPEN,            TextWindow::onCmdOpen),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_OPEN_SELECTED,   TextWindow::onCmdOpenSelected),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_SAVE,            TextWindow::onCmdSave),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_SAVE,            TextWindow::onUpdSave),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_SAVEAS,          TextWindow::onCmdSaveAs),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_NEW,             TextWindow::onCmdNew),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_TITLE,           TextWindow::onUpdTitle),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_FONT,            TextWindow::onCmdFont),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_QUIT,            TextWindow::onCmdQuit),
  FXMAPFUNC(SEL_SIGNAL,            TextWindow::ID_QUIT,            TextWindow::onCmdQuit),
  FXMAPFUNC(SEL_CLOSE,             TextWindow::ID_TITLE,           TextWindow::onCmdQuit),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_PRINT,           TextWindow::onCmdPrint),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_TREELIST,        TextWindow::onCmdTreeList),

  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_TEXT_BACK,       TextWindow::onCmdTextBackColor),
  FXMAPFUNC(SEL_CHANGED,           TextWindow::ID_TEXT_BACK,       TextWindow::onCmdTextBackColor),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_TEXT_BACK,       TextWindow::onUpdTextBackColor),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_TEXT_FORE,       TextWindow::onCmdTextForeColor),
  FXMAPFUNC(SEL_CHANGED,           TextWindow::ID_TEXT_FORE,       TextWindow::onCmdTextForeColor),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_TEXT_FORE,       TextWindow::onUpdTextForeColor),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_TEXT_SELBACK,    TextWindow::onCmdTextSelBackColor),
  FXMAPFUNC(SEL_CHANGED,           TextWindow::ID_TEXT_SELBACK,    TextWindow::onCmdTextSelBackColor),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_TEXT_SELBACK,    TextWindow::onUpdTextSelBackColor),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_TEXT_SELFORE,    TextWindow::onCmdTextSelForeColor),
  FXMAPFUNC(SEL_CHANGED,           TextWindow::ID_TEXT_SELFORE,    TextWindow::onCmdTextSelForeColor),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_TEXT_SELFORE,    TextWindow::onUpdTextSelForeColor),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_TEXT_HILITEBACK, TextWindow::onCmdTextHiliteBackColor),
  FXMAPFUNC(SEL_CHANGED,           TextWindow::ID_TEXT_HILITEBACK, TextWindow::onCmdTextHiliteBackColor),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_TEXT_HILITEBACK, TextWindow::onUpdTextHiliteBackColor),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_TEXT_HILITEFORE, TextWindow::onCmdTextHiliteForeColor),
  FXMAPFUNC(SEL_CHANGED,           TextWindow::ID_TEXT_HILITEFORE, TextWindow::onCmdTextHiliteForeColor),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_TEXT_HILITEFORE, TextWindow::onUpdTextHiliteForeColor),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_TEXT_CURSOR,     TextWindow::onCmdTextCursorColor),
  FXMAPFUNC(SEL_CHANGED,           TextWindow::ID_TEXT_CURSOR,     TextWindow::onCmdTextCursorColor),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_TEXT_CURSOR,     TextWindow::onUpdTextCursorColor),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_TEXT_ACTIVEBACK, TextWindow::onCmdTextActBackColor),
  FXMAPFUNC(SEL_CHANGED,           TextWindow::ID_TEXT_ACTIVEBACK, TextWindow::onCmdTextActBackColor),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_TEXT_ACTIVEBACK, TextWindow::onUpdTextActBackColor),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_TEXT_NUMBACK,    TextWindow::onCmdTextBarColor),
  FXMAPFUNC(SEL_CHANGED,           TextWindow::ID_TEXT_NUMBACK,    TextWindow::onCmdTextBarColor),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_TEXT_NUMBACK,    TextWindow::onUpdTextBarColor),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_TEXT_NUMFORE,    TextWindow::onCmdTextNumberColor),
  FXMAPFUNC(SEL_CHANGED,           TextWindow::ID_TEXT_NUMFORE,    TextWindow::onCmdTextNumberColor),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_TEXT_NUMFORE,    TextWindow::onUpdTextNumberColor),

  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_DIR_BACK,        TextWindow::onCmdDirBackColor),
  FXMAPFUNC(SEL_CHANGED,           TextWindow::ID_DIR_BACK,        TextWindow::onCmdDirBackColor),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_DIR_BACK,        TextWindow::onUpdDirBackColor),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_DIR_FORE,        TextWindow::onCmdDirForeColor),
  FXMAPFUNC(SEL_CHANGED,           TextWindow::ID_DIR_FORE,        TextWindow::onCmdDirForeColor),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_DIR_FORE,        TextWindow::onUpdDirForeColor),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_DIR_SELBACK,     TextWindow::onCmdDirSelBackColor),
  FXMAPFUNC(SEL_CHANGED,           TextWindow::ID_DIR_SELBACK,     TextWindow::onCmdDirSelBackColor),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_DIR_SELBACK,     TextWindow::onUpdDirSelBackColor),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_DIR_SELFORE,     TextWindow::onCmdDirSelForeColor),
  FXMAPFUNC(SEL_CHANGED,           TextWindow::ID_DIR_SELFORE,     TextWindow::onCmdDirSelForeColor),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_DIR_SELFORE,     TextWindow::onUpdDirSelForeColor),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_DIR_LINES,       TextWindow::onCmdDirLineColor),
  FXMAPFUNC(SEL_CHANGED,           TextWindow::ID_DIR_LINES,       TextWindow::onCmdDirLineColor),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_DIR_LINES,       TextWindow::onUpdDirLineColor),

  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_RECENTFILE,      TextWindow::onCmdRecentFile),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_TOGGLE_WRAP,     TextWindow::onUpdWrap),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_TOGGLE_WRAP,     TextWindow::onCmdWrap),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_SAVE_SETTINGS,   TextWindow::onCmdSaveSettings),
  FXMAPFUNC(SEL_CHANGED,           TextWindow::ID_TEXT,            TextWindow::onTextChanged),
  FXMAPFUNC(SEL_INSERTED,          TextWindow::ID_TEXT,            TextWindow::onTextInserted),
  FXMAPFUNC(SEL_DELETED,           TextWindow::ID_TEXT,            TextWindow::onTextDeleted),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,TextWindow::ID_TEXT,            TextWindow::onTextRightMouse),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_FIXED_WRAP,      TextWindow::onUpdWrapFixed),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_FIXED_WRAP,      TextWindow::onCmdWrapFixed),
  FXMAPFUNC(SEL_DND_MOTION,        TextWindow::ID_TEXT,            TextWindow::onEditDNDMotion),
  FXMAPFUNC(SEL_DND_DROP,          TextWindow::ID_TEXT,            TextWindow::onEditDNDDrop),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_STRIP_CR,        TextWindow::onUpdStripReturns),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_STRIP_CR,        TextWindow::onCmdStripReturns),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_STRIP_SP,        TextWindow::onUpdStripSpaces),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_STRIP_SP,        TextWindow::onCmdStripSpaces),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_INCLUDE_PATH,    TextWindow::onCmdIncludePaths),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_SHOW_HELP,       TextWindow::onCmdShowHelp),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_FILEFILTER,      TextWindow::onCmdFilter),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_OVERSTRIKE,      TextWindow::onUpdOverstrike),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_READONLY,        TextWindow::onUpdReadOnly),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_NUMCHARS,        TextWindow::onUpdNumChars),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_PREFERENCES,     TextWindow::onCmdPreferences),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_TABCOLUMNS,      TextWindow::onCmdTabColumns),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_TABCOLUMNS,      TextWindow::onUpdTabColumns),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_DELIMITERS,      TextWindow::onCmdDelimiters),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_DELIMITERS,      TextWindow::onUpdDelimiters),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_WRAPCOLUMNS,     TextWindow::onCmdWrapColumns),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_WRAPCOLUMNS,     TextWindow::onUpdWrapColumns),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_AUTOINDENT,      TextWindow::onCmdAutoIndent),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_AUTOINDENT,      TextWindow::onUpdAutoIndent),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_INSERTTABS,      TextWindow::onCmdInsertTabs),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_INSERTTABS,      TextWindow::onUpdInsertTabs),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_BRACEMATCH,      TextWindow::onCmdBraceMatch),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_BRACEMATCH,      TextWindow::onUpdBraceMatch),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_INSERT_FILE,     TextWindow::onUpdInsertFile),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_INSERT_FILE,     TextWindow::onCmdInsertFile),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_EXTRACT_FILE,    TextWindow::onUpdExtractFile),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_EXTRACT_FILE,    TextWindow::onCmdExtractFile),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_WHEELADJUST,     TextWindow::onUpdWheelAdjust),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_WHEELADJUST,     TextWindow::onCmdWheelAdjust),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_NEXT_MARK,       TextWindow::onUpdNextMark),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_NEXT_MARK,       TextWindow::onCmdNextMark),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_PREV_MARK,       TextWindow::onUpdPrevMark),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_PREV_MARK,       TextWindow::onCmdPrevMark),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_SET_MARK,        TextWindow::onUpdSetMark),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_SET_MARK,        TextWindow::onCmdSetMark),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_CLEAR_MARKS,     TextWindow::onCmdClearMarks),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_SAVEMARKS,       TextWindow::onUpdSaveMarks),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_SAVEMARKS,       TextWindow::onCmdSaveMarks),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_SAVEVIEWS,       TextWindow::onUpdSaveViews),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_SAVEVIEWS,       TextWindow::onCmdSaveViews),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_SHOWACTIVE,      TextWindow::onUpdShowActive),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_SHOWACTIVE,      TextWindow::onCmdShowActive),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_TEXT_LINENUMS,   TextWindow::onUpdLineNumbers),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_TEXT_LINENUMS,   TextWindow::onCmdLineNumbers),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_WARNCHANGED,     TextWindow::onUpdWarnChanged),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_WARNCHANGED,     TextWindow::onCmdWarnChanged),

  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_SYNTAX,          TextWindow::onUpdSyntax),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_SYNTAX,          TextWindow::onCmdSyntax),
  FXMAPFUNC(SEL_UPDATE,            TextWindow::ID_RESTYLE,         TextWindow::onUpdRestyle),
  FXMAPFUNC(SEL_COMMAND,           TextWindow::ID_RESTYLE,         TextWindow::onCmdRestyle),
  };


// Object implementation
FXIMPLEMENT(TextWindow,FXMainWindow,TextWindowMap,ARRAYNUMBER(TextWindowMap))


FXbool restyle=FALSE;

/*******************************************************************************/

// Make some windows
TextWindow::TextWindow(FXApp* a):FXMainWindow(a,"Adie: - untitled",NULL,NULL,DECOR_ALL,0,0,850,600,0,0){

  // Default font
  font=NULL;

  // Make some icons
  bigicon=new FXGIFIcon(getApp(),big_gif);
  smallicon=new FXGIFIcon(getApp(),small_gif);
  newicon=new FXGIFIcon(getApp(),new_gif,0,IMAGE_ALPHAGUESS);
  reloadicon=new FXGIFIcon(getApp(),reload_gif);
  openicon=new FXGIFIcon(getApp(),open_gif);
  saveicon=new FXGIFIcon(getApp(),save_gif);
  saveasicon=new FXGIFIcon(getApp(),saveas_gif,0,IMAGE_ALPHAGUESS);
  printicon=new FXGIFIcon(getApp(),print_gif);
  cuticon=new FXGIFIcon(getApp(),cut_gif);
  copyicon=new FXGIFIcon(getApp(),copy_gif);
  pasteicon=new FXGIFIcon(getApp(),paste_gif);
  deleteicon=new FXGIFIcon(getApp(),delete_gif);
  undoicon=new FXGIFIcon(getApp(),undo_gif);
  redoicon=new FXGIFIcon(getApp(),redo_gif);
  fontsicon=new FXGIFIcon(getApp(),fonts_gif);
  helpicon=new FXGIFIcon(getApp(),help_gif);
  quiticon=new FXGIFIcon(getApp(),quit_gif);
  shiftlefticon=new FXGIFIcon(getApp(),shiftleft_gif);
  shiftrighticon=new FXGIFIcon(getApp(),shiftright_gif);

  searchicon=new FXGIFIcon(getApp(),search_gif,0,IMAGE_ALPHAGUESS);
  searchnexticon=new FXGIFIcon(getApp(),searchnext_gif,0,IMAGE_ALPHAGUESS);
  searchprevicon=new FXGIFIcon(getApp(),searchprev_gif,0,IMAGE_ALPHAGUESS);
  bookseticon=new FXGIFIcon(getApp(),bookset_gif);
  booknexticon=new FXGIFIcon(getApp(),booknext_gif);
  bookprevicon=new FXGIFIcon(getApp(),bookprev_gif);
  bookdelicon=new FXGIFIcon(getApp(),bookdel_gif);

  // Application icons
  setIcon(bigicon);
  setMiniIcon(smallicon);

  // Make main window; set myself as the target
  setTarget(this);
  setSelector(ID_TITLE);

  // Help window
  helpwindow=new HelpWindow(this);

  // Make menu bar
  FXToolbarShell* dragshell1=new FXToolbarShell(this,FRAME_RAISED);
  menubar=new FXMenubar(this,dragshell1,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|FRAME_RAISED);
  new FXToolbarGrip(menubar,menubar,FXMenubar::ID_TOOLBARGRIP,TOOLBARGRIP_DOUBLE);

  // Tool bar
  FXToolbarShell* dragshell2=new FXToolbarShell(this,FRAME_RAISED);
  toolbar=new FXToolbar(this,dragshell2,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|FRAME_RAISED);
  new FXToolbarGrip(toolbar,toolbar,FXToolbar::ID_TOOLBARGRIP,TOOLBARGRIP_DOUBLE);

  // Status bar
  statusbar=new FXStatusbar(this,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|STATUSBAR_WITH_DRAGCORNER|FRAME_RAISED);

  // Info about the editor
  new FXButton(statusbar,"\tAbout Adie\tAbout the Adie text editor.",smallicon,this,ID_ABOUT,LAYOUT_FILL_Y|LAYOUT_RIGHT);

  // File menu
  filemenu=new FXMenuPane(this);
  new FXMenuTitle(menubar,"&File",NULL,filemenu);

  // Edit Menu
  editmenu=new FXMenuPane(this);
  new FXMenuTitle(menubar,"&Edit",NULL,editmenu);

  // Goto Menu
  gotomenu=new FXMenuPane(this);
  new FXMenuTitle(menubar,"&Goto",NULL,gotomenu);

  // Search Menu
  searchmenu=new FXMenuPane(this);
  new FXMenuTitle(menubar,"&Search",NULL,searchmenu);

  // Options Menu
  optionmenu=new FXMenuPane(this);
  new FXMenuTitle(menubar,"&Options",NULL,optionmenu);

  // View menu
  viewmenu=new FXMenuPane(this);
  new FXMenuTitle(menubar,"&View",NULL,viewmenu);

  // Help menu
  helpmenu=new FXMenuPane(this);
  new FXMenuTitle(menubar,"&Help",NULL,helpmenu,LAYOUT_RIGHT);

  // Splitter
  FXSplitter* splitter=new FXSplitter(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y|SPLITTER_TRACKING);

  // Sunken border for tree
  treebox=new FXVerticalFrame(splitter,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);

  // Make tree
  FXHorizontalFrame* treeframe=new FXHorizontalFrame(treebox,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);
  dirlist=new FXDirList(treeframe,0,this,ID_TREELIST,DIRLIST_SHOWFILES|TREELIST_BROWSESELECT|TREELIST_SHOWS_LINES|TREELIST_SHOWS_BOXES|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXHorizontalFrame* filterframe=new FXHorizontalFrame(treebox,LAYOUT_FILL_X);
  new FXLabel(filterframe,"Filter:");
  filter=new FXComboBox(filterframe,25,4,this,ID_FILEFILTER,COMBOBOX_STATIC|LAYOUT_FILL_X|FRAME_SUNKEN|FRAME_THICK);

  // Sunken border for text widget
  FXHorizontalFrame *textbox=new FXHorizontalFrame(splitter,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);

  // Make editor window
  editor=new FXText(textbox,this,ID_TEXT,LAYOUT_FILL_X|LAYOUT_FILL_Y|TEXT_SHOWACTIVE);
  editor->setHiliteMatchTime(300000);
  editor->setBarColumns(6);
  editor->setHiliteStyles(stylecolor);

  // Show clock on status bar
  clock=new FXTextField(statusbar,8,NULL,0,FRAME_SUNKEN|JUSTIFY_RIGHT|LAYOUT_RIGHT|LAYOUT_CENTER_Y|TEXTFIELD_READONLY,0,0,0,0,2,2,1,1);
  clock->setBackColor(statusbar->getBackColor());

  // Show readonly state in status bar
  FXLabel* readonly=new FXLabel(statusbar,NULL,NULL,FRAME_SUNKEN|JUSTIFY_RIGHT|LAYOUT_RIGHT|LAYOUT_CENTER_Y,0,0,0,0,2,2,1,1);
  readonly->setTarget(this);
  readonly->setSelector(ID_READONLY);

  // Show insert mode in status bar
  FXLabel* overstrike=new FXLabel(statusbar,NULL,NULL,FRAME_SUNKEN|LAYOUT_RIGHT|LAYOUT_CENTER_Y,0,0,0,0,2,2,1,1);
  overstrike->setTarget(this);
  overstrike->setSelector(ID_OVERSTRIKE);

  // Show size of text in status bar
  FXTextField* numchars=new FXTextField(statusbar,7,this,ID_NUMCHARS,FRAME_SUNKEN|JUSTIFY_RIGHT|LAYOUT_RIGHT|LAYOUT_CENTER_Y,0,0,0,0,2,2,1,1);
  numchars->setBackColor(statusbar->getBackColor());

  // Caption before number
  new FXLabel(statusbar,"  Size:",NULL,LAYOUT_RIGHT|LAYOUT_CENTER_Y);

  // Show column number in status bar
  FXTextField* columnno=new FXTextField(statusbar,7,editor,FXText::ID_CURSOR_COLUMN,FRAME_SUNKEN|JUSTIFY_RIGHT|LAYOUT_RIGHT|LAYOUT_CENTER_Y,0,0,0,0,2,2,1,1);
  columnno->setBackColor(statusbar->getBackColor());

  // Caption before number
  new FXLabel(statusbar,"  Col:",NULL,LAYOUT_RIGHT|LAYOUT_CENTER_Y);

  // Show line number in status bar
  FXTextField* rowno=new FXTextField(statusbar,7,editor,FXText::ID_CURSOR_ROW,FRAME_SUNKEN|JUSTIFY_RIGHT|LAYOUT_RIGHT|LAYOUT_CENTER_Y,0,0,0,0,2,2,1,1);
  rowno->setBackColor(statusbar->getBackColor());

  // Caption before number
  new FXLabel(statusbar,"  Line:",NULL,LAYOUT_RIGHT|LAYOUT_CENTER_Y);

  // Toobar buttons: File manipulation
  new FXButton(toolbar,"\tNew\tCreate new document.",newicon,this,ID_NEW,ICON_ABOVE_TEXT|BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  new FXButton(toolbar,"\tOpen\tOpen document file.",openicon,this,ID_OPEN,ICON_ABOVE_TEXT|BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  new FXButton(toolbar,"\tSave\tSave document.",saveicon,this,ID_SAVE,ICON_ABOVE_TEXT|BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  new FXButton(toolbar,"\tSave As\tSave document to another file.",saveasicon,this,ID_SAVEAS,ICON_ABOVE_TEXT|BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);

  // Spacer
  new FXFrame(toolbar,LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FIX_WIDTH,0,0,4,0);

  // Toobar buttons: Print
  new FXButton(toolbar,"\tPrint\tPrint document.",printicon,this,ID_PRINT,ICON_ABOVE_TEXT|BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);

  // Spacer
  new FXFrame(toolbar,FRAME_SUNKEN|LAYOUT_CENTER_Y|LAYOUT_FIX_HEIGHT|LAYOUT_FIX_WIDTH,0,0,2,22);

  // Toobar buttons: Editing
  new FXButton(toolbar,"\tCut\tCut selection to clipboard.",cuticon,editor,FXText::ID_CUT_SEL,ICON_ABOVE_TEXT|BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  new FXButton(toolbar,"\tCopy\tCopy selection to clipboard.",copyicon,editor,FXText::ID_COPY_SEL,ICON_ABOVE_TEXT|BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  new FXButton(toolbar,"\tPaste\tPaste clipboard.",pasteicon,editor,FXText::ID_PASTE_SEL,ICON_ABOVE_TEXT|BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  new FXButton(toolbar,"\tDelete\t\tDelete selection.",deleteicon,editor,FXText::ID_DELETE_SEL,ICON_ABOVE_TEXT|BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);

  // Spacer
  new FXFrame(toolbar,FRAME_SUNKEN|LAYOUT_CENTER_Y|LAYOUT_FIX_HEIGHT|LAYOUT_FIX_WIDTH,0,0,2,22);

  // Undo/redo
  new FXButton(toolbar,"\tUndo\tUndo last change.",undoicon,&undolist,FXUndoList::ID_UNDO,ICON_ABOVE_TEXT|BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  new FXButton(toolbar,"\tRedo\tRedo last undo.",redoicon,&undolist,FXUndoList::ID_REDO,ICON_ABOVE_TEXT|BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);

  // Spacer
  new FXFrame(toolbar,FRAME_SUNKEN|LAYOUT_CENTER_Y|LAYOUT_FIX_HEIGHT|LAYOUT_FIX_WIDTH,0,0,2,22);

  // Search
  new FXButton(toolbar,"\tSearch\tSearch text.",searchicon,editor,FXText::ID_SEARCH,ICON_ABOVE_TEXT|BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  new FXButton(toolbar,"\tSearch Next Selected\tSearch next occurrence of selected text.",searchnexticon,editor,FXText::ID_SEARCH_FORW_SEL,ICON_ABOVE_TEXT|BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  new FXButton(toolbar,"\tSearch Previous Selected\tSearch previous occurrence of selected text.",searchprevicon,editor,FXText::ID_SEARCH_BACK_SEL,ICON_ABOVE_TEXT|BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);

  // Spacer
  new FXFrame(toolbar,FRAME_SUNKEN|LAYOUT_CENTER_Y|LAYOUT_FIX_HEIGHT|LAYOUT_FIX_WIDTH,0,0,2,22);

  // Bookmarks
  new FXButton(toolbar,"\tBookmark\tSet bookmark.",bookseticon,this,ID_SET_MARK,ICON_ABOVE_TEXT|BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  new FXButton(toolbar,"\tNext Bookmark\tGoto next bookmark.",booknexticon,this,ID_NEXT_MARK,ICON_ABOVE_TEXT|BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  new FXButton(toolbar,"\tPrevious Bookmark\tGoto previous bookmark.",bookprevicon,this,ID_PREV_MARK,ICON_ABOVE_TEXT|BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  new FXButton(toolbar,"\tDelete Bookmarks\tDelete all bookmarks.",bookdelicon,this,ID_CLEAR_MARKS,ICON_ABOVE_TEXT|BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);

  // Spacer
  new FXFrame(toolbar,FRAME_SUNKEN|LAYOUT_CENTER_Y|LAYOUT_FIX_HEIGHT|LAYOUT_FIX_WIDTH,0,0,2,22);

  new FXButton(toolbar,"\tShift left\tShift text left by one.",shiftlefticon,editor,FXText::ID_SHIFT_LEFT,ICON_ABOVE_TEXT|BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  new FXButton(toolbar,"\tShift right\tShift text right by one.",shiftrighticon,editor,FXText::ID_SHIFT_RIGHT,ICON_ABOVE_TEXT|BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);

  // Spacer
  new FXFrame(toolbar,FRAME_SUNKEN|LAYOUT_CENTER_Y|LAYOUT_FIX_HEIGHT|LAYOUT_FIX_WIDTH,0,0,2,22);

  // Color
  new FXButton(toolbar,"\tFonts\tDisplay font dialog.",fontsicon,this,ID_FONT,ICON_ABOVE_TEXT|BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);

  // Help
  new FXButton(toolbar,"\tDisplay help\tDisplay online help information.",helpicon,this,ID_SHOW_HELP,ICON_ABOVE_TEXT|BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_RIGHT);

  // File Menu entries
  new FXMenuCommand(filemenu,"&Open...        \tCtl-O\tOpen document file.",openicon,this,ID_OPEN);
  new FXMenuCommand(filemenu,"Open Selected...   \tCtl-E\tOpen highlighted document file.",NULL,this,ID_OPEN_SELECTED);
  new FXMenuCommand(filemenu,"&Reopen...\t\tReopen file.",reloadicon,this,ID_REOPEN);
  new FXMenuCommand(filemenu,"&New...\tCtl-N\tCreate new document.",newicon,this,ID_NEW);
  new FXMenuCommand(filemenu,"&Save\tCtl-S\tSave changes to file.",saveicon,this,ID_SAVE);
  new FXMenuCommand(filemenu,"Save &As...\t\tSave document to another file.",saveasicon,this,ID_SAVEAS);
  new FXMenuSeparator(filemenu);
  new FXMenuCommand(filemenu,"Insert from file...\t\tInsert text from file.",NULL,this,ID_INSERT_FILE);
  new FXMenuCommand(filemenu,"Extract to file...\t\tExtract text to file.",NULL,this,ID_EXTRACT_FILE);
  new FXMenuCommand(filemenu,"&Print...\tCtl-P\tPrint document.",printicon,this,ID_PRINT);
  new FXMenuCommand(filemenu,"&Editable\t\tDocument editable.",NULL,editor,FXText::ID_TOGGLE_EDITABLE);

  // Recent file menu; this automatically hides if there are no files
  FXMenuSeparator* sep1=new FXMenuSeparator(filemenu);
  sep1->setTarget(&mrufiles);
  sep1->setSelector(FXRecentFiles::ID_ANYFILES);
  new FXMenuCommand(filemenu,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_1);
  new FXMenuCommand(filemenu,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_2);
  new FXMenuCommand(filemenu,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_3);
  new FXMenuCommand(filemenu,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_4);
  new FXMenuCommand(filemenu,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_5);
  new FXMenuCommand(filemenu,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_6);
  new FXMenuCommand(filemenu,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_7);
  new FXMenuCommand(filemenu,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_8);
  new FXMenuCommand(filemenu,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_9);
  new FXMenuCommand(filemenu,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_10);
  new FXMenuCommand(filemenu,"&Clear Recent Files",NULL,&mrufiles,FXRecentFiles::ID_CLEAR);
  FXMenuSeparator* sep2=new FXMenuSeparator(filemenu);
  sep2->setTarget(&mrufiles);
  sep2->setSelector(FXRecentFiles::ID_ANYFILES);
  new FXMenuCommand(filemenu,"&Quit\tCtl-Q",quiticon,this,ID_QUIT);

  // Edit Menu entries
  new FXMenuCommand(editmenu,"&Undo\tCtl-Z\tUndo last change.",undoicon,&undolist,FXUndoList::ID_UNDO);
  new FXMenuCommand(editmenu,"&Redo\tCtl-Y\tRedo last undo.",redoicon,&undolist,FXUndoList::ID_REDO);
  new FXMenuCommand(editmenu,"&Undo all\t\tUndo all.",NULL,&undolist,FXUndoList::ID_UNDO_ALL);
  new FXMenuCommand(editmenu,"&Redo all\t\tRedo all.",NULL,&undolist,FXUndoList::ID_REDO_ALL);
  new FXMenuCommand(editmenu,"&Revert to saved\t\tRevert to saved.",NULL,&undolist,FXUndoList::ID_REVERT);
  new FXMenuSeparator(editmenu);
  new FXMenuCommand(editmenu,"&Copy\tCtl-C\tCopy selection to clipboard.",copyicon,editor,FXText::ID_COPY_SEL);
  new FXMenuCommand(editmenu,"Cu&t\tCtl-X\tCut selection to clipboard.",cuticon,editor,FXText::ID_CUT_SEL);
  new FXMenuCommand(editmenu,"&Paste\tCtl-V\tPaste from clipboard.",pasteicon,editor,FXText::ID_PASTE_SEL);
  new FXMenuCommand(editmenu,"&Delete\t\tDelete selection.",deleteicon,editor,FXText::ID_DELETE_SEL);
  new FXMenuSeparator(editmenu);
  new FXMenuCommand(editmenu,"Lo&wer-case\tCtl-6\tChange to lower case.",NULL,editor,FXText::ID_LOWER_CASE);
  new FXMenuCommand(editmenu,"Upp&er-case\tShift-Ctl-^\tChange to upper case.",NULL,editor,FXText::ID_UPPER_CASE);
  new FXMenuCommand(editmenu,"Clean indent\t\tClean indentation to either all tabs or all spaces.",NULL,editor,FXText::ID_CLEAN_INDENT);
  new FXMenuCommand(editmenu,"Shift left\tCtl-9\tShift text left.",shiftlefticon,editor,FXText::ID_SHIFT_LEFT);
  new FXMenuCommand(editmenu,"Shift right\tCtl-0\tShift text right.",shiftrighticon,editor,FXText::ID_SHIFT_RIGHT);
  new FXMenuCommand(editmenu,"Shift tab left\t\tShift text left one tab position.",shiftlefticon,editor,FXText::ID_SHIFT_TABLEFT);
  new FXMenuCommand(editmenu,"Shift tab right\t\tShift text right one tab position.",shiftrighticon,editor,FXText::ID_SHIFT_TABRIGHT);

  // Right mouse popup
  popupmenu=new FXMenuPane(this);
  new FXMenuCommand(popupmenu,"Undo",undoicon,&undolist,FXUndoList::ID_UNDO);
  new FXMenuCommand(popupmenu,"Redo",redoicon,&undolist,FXUndoList::ID_REDO);
  new FXMenuSeparator(popupmenu);
  new FXMenuCommand(popupmenu,"Cut",cuticon,editor,FXText::ID_CUT_SEL);
  new FXMenuCommand(popupmenu,"Copy",copyicon,editor,FXText::ID_COPY_SEL);
  new FXMenuCommand(popupmenu,"Paste",pasteicon,editor,FXText::ID_PASTE_SEL);
  new FXMenuCommand(popupmenu,"Select All",NULL,editor,FXText::ID_SELECT_ALL);
  new FXMenuSeparator(popupmenu);
  new FXMenuCommand(popupmenu,"Set bookmark",bookseticon,this,ID_SET_MARK);
  new FXMenuCommand(popupmenu,"Next bookmark",booknexticon,this,ID_NEXT_MARK);
  new FXMenuCommand(popupmenu,"Previous bookmark",bookprevicon,this,ID_PREV_MARK);
  new FXMenuCommand(popupmenu,"Clear bookmarks",bookdelicon,this,ID_CLEAR_MARKS);

  // Goto Menu entries
  new FXMenuCommand(gotomenu,"&Goto...\tCtl-G\tGoto line number.",NULL,editor,FXText::ID_GOTO_LINE);
  new FXMenuCommand(gotomenu,"Goto selected...\tCtl-L\tGoto selected line number.",NULL,editor,FXText::ID_GOTO_SELECTED);
  new FXMenuSeparator(gotomenu);
  new FXMenuCommand(gotomenu,"Goto {..\tShift-Ctl-{\tGoto start of enclosing block.",NULL,editor,FXText::ID_LEFT_BRACE);
  new FXMenuCommand(gotomenu,"Goto ..}\tShift-Ctl-}\tGoto end of enclosing block.",NULL,editor,FXText::ID_RIGHT_BRACE);
  new FXMenuCommand(gotomenu,"Goto (..\tShift-Ctl-(\tGoto start of enclosing expression.",NULL,editor,FXText::ID_LEFT_PAREN);
  new FXMenuCommand(gotomenu,"Goto ..)\tShift-Ctl-)\tGoto end of enclosing expression.",NULL,editor,FXText::ID_RIGHT_PAREN);
  new FXMenuSeparator(gotomenu);
  new FXMenuCommand(gotomenu,"Goto matching      (..)\tCtl-M\tGoto matching brace or parenthesis.",NULL,editor,FXText::ID_GOTO_MATCHING);
  new FXMenuSeparator(gotomenu);
  new FXMenuCommand(gotomenu,"&Set bookmark\tAlt-B",bookseticon,this,ID_SET_MARK);
  new FXMenuCommand(gotomenu,"&Next bookmark\tAlt-N",booknexticon,this,ID_NEXT_MARK);
  new FXMenuCommand(gotomenu,"&Previous bookmark\tAlt-P",bookprevicon,this,ID_PREV_MARK);
  new FXMenuCommand(gotomenu,"&Clear bookmarks\tAlt-C",bookdelicon,this,ID_CLEAR_MARKS);

  // Search Menu entries
  new FXMenuCommand(searchmenu,"Select matching (..)\tShift-Ctl-M\tSelect matching brace or parenthesis.",NULL,editor,FXText::ID_SELECT_MATCHING);
  new FXMenuCommand(searchmenu,"Select block {..}\tShift-Alt-{\tSelect enclosing block.",NULL,editor,FXText::ID_SELECT_BRACE);
  new FXMenuCommand(searchmenu,"Select block {..}\tShift-Alt-}\tSelect enclosing block.",NULL,editor,FXText::ID_SELECT_BRACE);
  new FXMenuCommand(searchmenu,"Select expression (..)\tShift-Alt-(\tSelect enclosing parentheses.",NULL,editor,FXText::ID_SELECT_PAREN);
  new FXMenuCommand(searchmenu,"Select expression (..)\tShift-Alt-)\tSelect enclosing parentheses.",NULL,editor,FXText::ID_SELECT_PAREN);
  new FXMenuSeparator(searchmenu);
  new FXMenuCommand(searchmenu,"&Search sel. fwd\tCtl-H\tSearch for selection.",searchnexticon,editor,FXText::ID_SEARCH_FORW_SEL);
  new FXMenuCommand(searchmenu,"&Search sel. bck\tShift-Ctl-H\tSearch for selection.",searchprevicon,editor,FXText::ID_SEARCH_BACK_SEL);
  new FXMenuCommand(searchmenu,"&Search...\tCtl-F\tSearch for a string.",searchicon,editor,FXText::ID_SEARCH);
  new FXMenuCommand(searchmenu,"R&eplace...\tCtl-R\tSearch for a string.",NULL,editor,FXText::ID_REPLACE);

  // Options menu
  new FXMenuCommand(optionmenu,"Preferences...\t\tChange preferences.",NULL,this,TextWindow::ID_PREFERENCES);
  new FXMenuCommand(optionmenu,"Font...\t\tChange text font.",fontsicon,this,ID_FONT);
  new FXMenuCommand(optionmenu,"Overstrike\t\tToggle overstrike mode.",NULL,editor,FXText::ID_TOGGLE_OVERSTRIKE);
  new FXMenuCommand(optionmenu,"Syntax coloring\t\tToggle syntax coloring.",NULL,this,TextWindow::ID_SYNTAX);
  new FXMenuCommand(optionmenu,"Res&tyle\tCtl-T\tToggle syntax coloring.",NULL,this,TextWindow::ID_RESTYLE);
  new FXMenuCommand(optionmenu,"Include path...\t\tDirectories to search for include files.",NULL,this,TextWindow::ID_INCLUDE_PATH);
  new FXMenuSeparator(optionmenu);
  new FXMenuCommand(optionmenu,"Save Settings\t\tSave settings now.",NULL,this,ID_SAVE_SETTINGS);

  // View Menu entries
  new FXMenuCommand(viewmenu,"Hidden files\t\tShow hidden files and directories.",NULL,dirlist,FXDirList::ID_TOGGLE_HIDDEN);
  new FXMenuCommand(viewmenu,"File Browser\t\tDisplay file list.",NULL,treebox,FXWindow::ID_TOGGLESHOWN);
  new FXMenuCommand(viewmenu,"Toolbar\t\tDisplay toolbar.",NULL,toolbar,FXWindow::ID_TOGGLESHOWN);
  new FXMenuCommand(viewmenu,"Status line\t\tDisplay status line.",NULL,statusbar,FXWindow::ID_TOGGLESHOWN);
  new FXMenuCommand(viewmenu,"Clock\t\tShow clock on status line.",NULL,clock,FXWindow::ID_TOGGLESHOWN);

  // Help Menu entries
  new FXMenuCommand(helpmenu,"&Help...\t\tDisplay help information.",helpicon,this,ID_SHOW_HELP,0);
  new FXMenuSeparator(helpmenu);
  new FXMenuCommand(helpmenu,"&About Adie...\t\tDisplay about panel.",smallicon,this,ID_ABOUT,0);

  // Make a tool tip
  new FXTooltip(getApp(),0);

  // Recent files
  mrufiles.setTarget(this);
  mrufiles.setSelector(ID_RECENTFILE);

  // Add some alternative accelerators
  if(getAccelTable()){
    getAccelTable()->addAccel(MKUINT(KEY_Z,CONTROLMASK|SHIFTMASK),&undolist,MKUINT(FXUndoList::ID_REDO,SEL_COMMAND));
    }

  // Initialize bookmarks
  clearBookmarks();

  // Initialize file name
  filename="untitled";
  filetime=0;
  filenameset=FALSE;

  // Initialize other stuff
  nstyles=0;
  searchpath="/usr/include";
  setPatterns("All Files (*)");
  setCurrentPattern(0);
  filetimer=NULL;
  clocktimer=NULL;
  stripcr=TRUE;
  stripsp=FALSE;
  saveviews=FALSE;
  savemarks=FALSE;
  warnchanged=FALSE;
  undolist.mark();
  }


// Create and show window
void TextWindow::create(){
  readRegistry();
  FXMainWindow::create();
  if(!urilistType){urilistType=getApp()->registerDragType(urilistTypeName);}
  if(warnchanged){filetimer=getApp()->addTimeout(1,this,ID_FILETIME);}
  clocktimer=getApp()->addTimeout(1,this,ID_CLOCKTIME);
  show(PLACEMENT_DEFAULT);
  }


// Clean up the mess
TextWindow::~TextWindow(){
  if(filetimer) getApp()->removeTimeout(filetimer);
  if(clocktimer) getApp()->removeTimeout(clocktimer);
  delete font;
  delete helpwindow;
  delete filemenu;
  delete editmenu;
  delete gotomenu;
  delete searchmenu;
  delete optionmenu;
  delete viewmenu;
  delete helpmenu;
  delete popupmenu;
  delete bigicon;
  delete smallicon;
  delete newicon;
  delete reloadicon;
  delete openicon;
  delete saveicon;
  delete saveasicon;
  delete printicon;
  delete cuticon;
  delete copyicon;
  delete pasteicon;
  delete deleteicon;
  delete undoicon;
  delete redoicon;
  delete fontsicon;
  delete helpicon;
  delete quiticon;
  delete shiftlefticon;
  delete shiftrighticon;
  delete searchicon;
  delete searchnexticon;
  delete searchprevicon;
  delete bookseticon;
  delete booknexticon;
  delete bookprevicon;
  delete bookdelicon;
  }


/*******************************************************************************/


// Load file
FXbool TextWindow::loadFile(const FXString& file){
  FXuint size,n,i,j,k,c;
  FXchar *text;
  FILE *fp;

  FXTRACE((100,"loadFile(%s)\n",file.text()));

  // Open file
  fp=fopen(file.text(),"r");
  if(!fp){
    FXMessageBox::error(this,MBOX_OK,"Error Loading File","Unable to open file: %s",file.text());
    return FALSE;
    }

  // Get file size
  size=FXFile::size(file);

  // Make buffer to load file
  if(!FXCALLOC(&text,FXchar,size+1)){
    FXMessageBox::error(this,MBOX_OK,"Error Loading File","File is too big: %s",file.text());
    fclose(fp);
    return FALSE;
    }

  // Set wait cursor
  getApp()->beginWaitCursor();

  // Read the file
  n=fread(text,1,size,fp);

  // Close file
  fclose(fp);

  // Strip carriage returns
  if(stripcr){
    for(i=j=0; j<n; j++){
      c=text[j];
      if(c!='\r'){
        text[i++]=c;
        }
      }
    text[i]='\0';
    n=i;
    }

  // Strip trailing spaces
  if(stripsp){
    for(i=j=k=0; j<n; i++,j++){
      c=text[j];
      if(c=='\n'){
        i=k;
        k++;
        }
      else if(!isspace(c)){
        k=i+1;
        }
      text[i]=c;
      }
    text[i]='\0';
    n=i;
    }

  // Set text, and kick the style engine
//#ifdef TEST
//  editor->setText(text,n,TRUE);
//#else
  editor->setText(text,n);
//#endif
  FXFREE(&text);

  // Kill wait cursor
  getApp()->endWaitCursor();

  // Set stuff
  editor->setModified(FALSE);
  editor->setEditable(FXFile::isWritable(file));
  dirlist->setCurrentFile(file);
  mrufiles.appendFile(file);
  filetime=FXFile::modified(file);
  filename=file;
  filenameset=TRUE;

  // TEST
  language=getLanguage(filename);
  readSyntax(language);
  FXTRACE((1,"language=%s\n",language.text()));

  // Clear undo records
  undolist.clear();

  // Mark undo state as clean (saved)
  undolist.mark();
#ifdef TEST
restyle=TRUE;
#endif
  return TRUE;
  }


// Insert file
FXbool TextWindow::insertFile(const FXString& file){
  FXuint size,n,i,j,k,c;
  FXchar *text;
  FILE *fp;

  FXTRACE((100,"insertFile(%s)\n",file.text()));

  // Open file
  fp=fopen(file.text(),"r");
  if(!fp){
    FXMessageBox::error(this,MBOX_OK,"Error Inserting File","Unable to open file: %s",file.text());
    return FALSE;
    }

  // Get file size
  size=FXFile::size(file);

  // Make buffer to load file
  if(!FXCALLOC(&text,FXchar,size+1)){
    FXMessageBox::error(this,MBOX_OK,"Error Inserting File","File is too big: %s",file.text());
    fclose(fp);
    return FALSE;
    }

  // Set wait cursor
  getApp()->beginWaitCursor();

  // Read the file
  n=fread(text,1,size,fp);

  // Strip carriage returns
  if(stripcr){
    for(i=j=0; j<n; j++){
      c=text[j];
      if(c!='\r'){
        text[i++]=c;
        }
      }
    text[i]='\0';
    n=i;
    }

  // Strip trailing spaces
  if(stripsp){
    for(i=j=k=0; j<n; i++,j++){
      c=text[j];
      if(c=='\n'){
        i=k;
        k++;
        }
      else if(!isspace(c)){
        k=i+1;
        }
      text[i]=c;
      }
    text[i]='\0';
    n=i;
    }

  // Set text
  editor->insertText(editor->getCursorPos(),text,n,TRUE);
  editor->setModified(TRUE);
  FXFREE(&text);

  // Kill wait cursor
  getApp()->endWaitCursor();

  // Close file
  fclose(fp);

  // Set stuff
  return TRUE;
  }


// Save file
FXbool TextWindow::saveFile(const FXString& file){
  FXuint size,n;
  FXchar *text;
  FILE *fp;

  FXTRACE((100,"saveFile(%s)\n",file.text()));

  // Open file
  fp=fopen(file.text(),"w");
  if(!fp){
    FXMessageBox::error(this,MBOX_OK,"Error Saving File","Unable to open file: %s",file.text());
    return FALSE;
    }

  // Get size
  size=editor->getLength();

  // Alloc buffer
  if(!FXCALLOC(&text,FXchar,size)){
    FXMessageBox::error(this,MBOX_OK,"Error Saving File","File is too big: %s",file.text());
    fclose(fp);
    return FALSE;
    }

  // Set wait cursor
  getApp()->beginWaitCursor();

  // Get text from editor
  editor->getText(text,size);

  // Write the file
  n=fwrite(text,1,size,fp);
  FXFREE(&text);

  // Kill wait cursor
  getApp()->endWaitCursor();

  // Close file
  fclose(fp);

  // Were we able to write it all?
  if(n!=size){
    FXMessageBox::error(this,MBOX_OK,"Error Saving File","File: %s truncated.",file.text());
    return FALSE;
    }

  // Set stuff
  editor->setModified(FALSE);
  editor->setEditable(TRUE);
  dirlist->setCurrentFile(file);
  mrufiles.appendFile(file);
  filetime=FXFile::modified(file);
  filename=file;
  filenameset=TRUE;
  undolist.mark();
  return TRUE;
  }


// Extract file
FXbool TextWindow::extractFile(const FXString& file){
  FXuint size,n;
  FXchar *text;
  FILE *fp;

  FXTRACE((100,"extractFile(%s)\n",file.text()));

  // Open file
  fp=fopen(file.text(),"w");
  if(!fp){
    FXMessageBox::error(this,MBOX_OK,"Error Extracting File","Unable to open file: %s",file.text());
    return FALSE;
    }

  // Get size
  size=editor->getSelEndPos()-editor->getSelStartPos();

  // Alloc buffer
  if(!FXCALLOC(&text,FXchar,size)){
    FXMessageBox::error(this,MBOX_OK,"Error Extracting File","File is too big: %s",file.text());
    fclose(fp);
    return FALSE;
    }

  // Set wait cursor
  getApp()->beginWaitCursor();

  // Get text from editor
  editor->extractText(text,editor->getSelStartPos(),size);

  // Write the file
  n=fwrite(text,1,size,fp);
  FXFREE(&text);

  // Kill wait cursor
  getApp()->endWaitCursor();

  // Close file
  fclose(fp);

  // Were we able to write it all?
  if(n!=size){
    FXMessageBox::error(this,MBOX_OK,"Error Extracting File","File: %s truncated.",file.text());
    return FALSE;
    }

  return TRUE;
  }

/*******************************************************************************/

// Read settings from registry
void TextWindow::readRegistry(){
  FXColor textback,textfore,textselback,textselfore,textcursor,texthilitefore,texthiliteback;
  FXColor dirback,dirfore,dirselback,dirselfore,dirlines,textactiveback,textbar,textnumber;
  FXint ww,hh,xx,yy,treewidth,hidetree,hiddenfiles,wrapping,wrapcols,fixedwrap,tabcols;
  FXint autoindent,hardtabs,hideclock,hidestatus,hidetoolbar,hilitematchtime,barcols,syntax;
  const FXchar* delimiters;
  FXFontDesc fontdesc;
  const FXchar *fontspec;

  // Text colors
  textback=getApp()->reg().readColorEntry("SETTINGS","textbackground",editor->getBackColor());
  textfore=getApp()->reg().readColorEntry("SETTINGS","textforeground",editor->getTextColor());
  textselback=getApp()->reg().readColorEntry("SETTINGS","textselbackground",editor->getSelBackColor());
  textselfore=getApp()->reg().readColorEntry("SETTINGS","textselforeground",editor->getSelTextColor());
  textcursor=getApp()->reg().readColorEntry("SETTINGS","textcursor",editor->getCursorColor());
  texthiliteback=getApp()->reg().readColorEntry("SETTINGS","texthilitebackground",editor->getHiliteBackColor());
  texthilitefore=getApp()->reg().readColorEntry("SETTINGS","texthiliteforeground",editor->getHiliteTextColor());
  textactiveback=getApp()->reg().readColorEntry("SETTINGS","textactivebackground",editor->getActiveBackColor());
  textbar=getApp()->reg().readColorEntry("SETTINGS","textnumberbackground",editor->getBarColor());
  textnumber=getApp()->reg().readColorEntry("SETTINGS","textnumberforeground",editor->getNumberColor());

  // Directory colors
  dirback=getApp()->reg().readColorEntry("SETTINGS","browserbackground",dirlist->getBackColor());
  dirfore=getApp()->reg().readColorEntry("SETTINGS","browserforeground",dirlist->getTextColor());
  dirselback=getApp()->reg().readColorEntry("SETTINGS","browserselbackground",dirlist->getSelBackColor());
  dirselfore=getApp()->reg().readColorEntry("SETTINGS","browserselforeground",dirlist->getSelTextColor());
  dirlines=getApp()->reg().readColorEntry("SETTINGS","browserlines",dirlist->getLineColor());

  // Delimiters
  delimiters=getApp()->reg().readStringEntry("SETTINGS","delimiters","~.,/\\`'!@#$%^&*()-=+{}|[]\":;<>?");

  // Font
  fontspec=getApp()->reg().readStringEntry("SETTINGS","font",NULL);
  if(fontspec && fxparsefontdesc(fontdesc,fontspec)){
    font=new FXFont(getApp(),fontdesc);
    editor->setFont(font);
    }

  // Get size
  xx=getApp()->reg().readIntEntry("SETTINGS","x",5);
  yy=getApp()->reg().readIntEntry("SETTINGS","y",5);
  ww=getApp()->reg().readIntEntry("SETTINGS","width",600);
  hh=getApp()->reg().readIntEntry("SETTINGS","height",400);

  // Hidden files shown
  hiddenfiles=getApp()->reg().readIntEntry("SETTINGS","showhiddenfiles",FALSE);
  dirlist->showHiddenFiles(hiddenfiles);

  // Showing the tree?
  hidetree=getApp()->reg().readIntEntry("SETTINGS","hidetree",TRUE);

  // Showing the clock?
  hideclock=getApp()->reg().readIntEntry("SETTINGS","hideclock",FALSE);

  // Showing the status line?
  hidestatus=getApp()->reg().readIntEntry("SETTINGS","hidestatus",FALSE);

  // Showing the tool bar?
  hidetoolbar=getApp()->reg().readIntEntry("SETTINGS","hidetoolbar",FALSE);

  // Highlight match time
  hilitematchtime=getApp()->reg().readIntEntry("SETTINGS","hilitematchtime",3000);

  // Width of tree
  treewidth=getApp()->reg().readIntEntry("SETTINGS","treewidth",100);

  // Word wrapping
  wrapping=getApp()->reg().readIntEntry("SETTINGS","wordwrap",0);
  wrapcols=getApp()->reg().readIntEntry("SETTINGS","wrapcols",80);
  fixedwrap=getApp()->reg().readIntEntry("SETTINGS","fixedwrap",1);

  // Tab settings, autoindent
  autoindent=getApp()->reg().readIntEntry("SETTINGS","autoindent",0);
  hardtabs=getApp()->reg().readIntEntry("SETTINGS","hardtabs",1);
  tabcols=getApp()->reg().readIntEntry("SETTINGS","tabcols",8);

  // Space for line numbers
  barcols=getApp()->reg().readIntEntry("SETTINGS","barcols",0);

  // Various flags
  stripcr=getApp()->reg().readIntEntry("SETTINGS","stripreturn",FALSE);
  stripsp=getApp()->reg().readIntEntry("SETTINGS","stripspaces",FALSE);
  saveviews=getApp()->reg().readIntEntry("SETTINGS","saveviews",FALSE);
  savemarks=getApp()->reg().readIntEntry("SETTINGS","savebookmarks",FALSE);
  warnchanged=getApp()->reg().readIntEntry("SETTINGS","warnchanged",TRUE);
  syntax=getApp()->reg().readIntEntry("SETTINGS","syntax",FALSE);

  // File patterns
  setPatterns(getApp()->reg().readStringEntry("SETTINGS","filepatterns","All Files (*)"));
  setCurrentPattern(getApp()->reg().readIntEntry("SETTINGS","filepatternno",0));

  // Search path
  searchpath=getApp()->reg().readStringEntry("SETTINGS","searchpath","/usr/include");

  // Change the colors
  editor->setTextColor(textfore);
  editor->setBackColor(textback);
  editor->setSelBackColor(textselback);
  editor->setSelTextColor(textselfore);
  editor->setCursorColor(textcursor);
  editor->setHiliteBackColor(texthiliteback);
  editor->setHiliteTextColor(texthilitefore);
  editor->setActiveBackColor(textactiveback);
  editor->setBarColor(textbar);
  editor->setNumberColor(textnumber);

  dirlist->setTextColor(dirfore);
  dirlist->setBackColor(dirback);
  dirlist->setSelBackColor(dirselback);
  dirlist->setSelTextColor(dirselfore);
  dirlist->setLineColor(dirlines);

  // Change delimiters
  FXCharset set(delimiters);
  editor->setDelimiters(set);

  // Hide tree if asked for
  if(hidetree) treebox->hide();

  // Hide clock
  if(hideclock) clock->hide();

  // Hide statusline
  if(hidestatus) statusbar->hide();

  // Hide toolbar
  if(hidetoolbar) toolbar->hide();

  // Set tree width
  treebox->setWidth(treewidth);

  // Open toward file
  dirlist->setCurrentFile(filename);

  // Wrap mode
  if(wrapping)
    editor->setTextStyle(editor->getTextStyle()|TEXT_WORDWRAP);
  else
    editor->setTextStyle(editor->getTextStyle()&~TEXT_WORDWRAP);

  // Wrap fixed mode
  if(fixedwrap)
    editor->setTextStyle(editor->getTextStyle()|TEXT_FIXEDWRAP);
  else
    editor->setTextStyle(editor->getTextStyle()&~TEXT_FIXEDWRAP);

  // Autoindent
  if(autoindent)
    editor->setTextStyle(editor->getTextStyle()|TEXT_AUTOINDENT);
  else
    editor->setTextStyle(editor->getTextStyle()&~TEXT_AUTOINDENT);

  // Hard tabs
  if(hardtabs)
    editor->setTextStyle(editor->getTextStyle()&~TEXT_NO_TABS);
  else
    editor->setTextStyle(editor->getTextStyle()|TEXT_NO_TABS);

  // Wrap and tab columns
  editor->setWrapColumns(wrapcols);
  editor->setTabColumns(tabcols);
  editor->setBarColumns(barcols);

  // Highlight match time
  editor->setHiliteMatchTime(hilitematchtime);

  // Read styles
  readStyles();

  // Enable style buffer, iff we have styles
  editor->setStyled(syntax);

  // Reposition window
  position(xx,yy,ww,hh);
  }


/*******************************************************************************/


// Save settings to registry
void TextWindow::writeRegistry(){
  FXFontDesc fontdesc;
  FXchar fontspec[200];

  // Colors of text
  getApp()->reg().writeColorEntry("SETTINGS","textbackground",editor->getBackColor());
  getApp()->reg().writeColorEntry("SETTINGS","textforeground",editor->getTextColor());
  getApp()->reg().writeColorEntry("SETTINGS","textselbackground",editor->getSelBackColor());
  getApp()->reg().writeColorEntry("SETTINGS","textselforeground",editor->getSelTextColor());
  getApp()->reg().writeColorEntry("SETTINGS","textcursor",editor->getCursorColor());
  getApp()->reg().writeColorEntry("SETTINGS","texthilitebackground",editor->getHiliteBackColor());
  getApp()->reg().writeColorEntry("SETTINGS","texthiliteforeground",editor->getHiliteTextColor());
  getApp()->reg().writeColorEntry("SETTINGS","textactivebackground",editor->getActiveBackColor());
  getApp()->reg().writeColorEntry("SETTINGS","textnumberbackground",editor->getBarColor());
  getApp()->reg().writeColorEntry("SETTINGS","textnumberforeground",editor->getNumberColor());

  // Colors of directory
  getApp()->reg().writeColorEntry("SETTINGS","browserbackground",dirlist->getBackColor());
  getApp()->reg().writeColorEntry("SETTINGS","browserforeground",dirlist->getTextColor());
  getApp()->reg().writeColorEntry("SETTINGS","browserselbackground",dirlist->getSelBackColor());
  getApp()->reg().writeColorEntry("SETTINGS","browserselforeground",dirlist->getSelTextColor());
  getApp()->reg().writeColorEntry("SETTINGS","browserlines",dirlist->getLineColor());

  // Delimiters
  FXString delimiters=editor->getDelimiters();
  getApp()->reg().writeStringEntry("SETTINGS","delimiters",delimiters.text());

  // Write new window size back to registry
  getApp()->reg().writeIntEntry("SETTINGS","x",getX());
  getApp()->reg().writeIntEntry("SETTINGS","y",getY());
  getApp()->reg().writeIntEntry("SETTINGS","width",getWidth());
  getApp()->reg().writeIntEntry("SETTINGS","height",getHeight());

  // Were showing hidden files
  getApp()->reg().writeIntEntry("SETTINGS","showhiddenfiles",dirlist->showHiddenFiles());

  // Was tree shown
  getApp()->reg().writeIntEntry("SETTINGS","hidetree",!treebox->shown());

  // Was status line shown
  getApp()->reg().writeIntEntry("SETTINGS","hidestatus",!statusbar->shown());

  // Was clock shown
  getApp()->reg().writeIntEntry("SETTINGS","hideclock",!clock->shown());

  // Was toolbar shown
  getApp()->reg().writeIntEntry("SETTINGS","hidetoolbar",!toolbar->shown());

  // Highlight match time
  getApp()->reg().writeIntEntry("SETTINGS","hilitematchtime",editor->getHiliteMatchTime());

  // Width of tree
  getApp()->reg().writeIntEntry("SETTINGS","treewidth",treebox->getWidth());

  // Wrap mode
  getApp()->reg().writeIntEntry("SETTINGS","wordwrap",(editor->getTextStyle()&TEXT_WORDWRAP)!=0);
  getApp()->reg().writeIntEntry("SETTINGS","fixedwrap",(editor->getTextStyle()&TEXT_FIXEDWRAP)!=0);
  getApp()->reg().writeIntEntry("SETTINGS","wrapcols",editor->getWrapColumns());

  // Bar columns
  getApp()->reg().writeIntEntry("SETTINGS","barcols",editor->getBarColumns());

  // Tab settings, autoindent
  getApp()->reg().writeIntEntry("SETTINGS","autoindent",(editor->getTextStyle()&TEXT_AUTOINDENT)!=0);
  getApp()->reg().writeIntEntry("SETTINGS","hardtabs",(editor->getTextStyle()&TEXT_NO_TABS)==0);
  getApp()->reg().writeIntEntry("SETTINGS","tabcols",editor->getTabColumns());

  // Strip returns
  getApp()->reg().writeIntEntry("SETTINGS","stripreturn",stripcr);
  getApp()->reg().writeIntEntry("SETTINGS","stripspaces",stripsp);
  getApp()->reg().writeIntEntry("SETTINGS","saveviews",saveviews);
  getApp()->reg().writeIntEntry("SETTINGS","savebookmarks",savemarks);
  getApp()->reg().writeIntEntry("SETTINGS","warnchanged",warnchanged);
  getApp()->reg().writeIntEntry("SETTINGS","syntax",editor->isStyled());

  // File patterns
  getApp()->reg().writeIntEntry("SETTINGS","filepatternno",getCurrentPattern());
  getApp()->reg().writeStringEntry("SETTINGS","filepatterns",getPatterns().text());

  // Search path
  getApp()->reg().writeStringEntry("SETTINGS","searchpath",searchpath.text());

  // Write highlight styles
  writeStyles();

  // Font
  editor->getFont()->getFontDesc(fontdesc);
  fxunparsefontdesc(fontspec,fontdesc);
  getApp()->reg().writeStringEntry("SETTINGS","font",fontspec);

  }


/*******************************************************************************/


// About box
long TextWindow::onCmdAbout(FXObject*,FXSelector,void*){
  FXDialogBox about(this,"About Adie",DECOR_TITLE|DECOR_BORDER,0,0,0,0, 0,0,0,0, 0,0);
  FXGIFIcon picture(getApp(),adie_gif);
  new FXLabel(&about,FXString::null,&picture,FRAME_GROOVE|LAYOUT_SIDE_LEFT|LAYOUT_CENTER_Y|JUSTIFY_CENTER_X|JUSTIFY_CENTER_Y,0,0,0,0, 0,0,0,0);
  FXVerticalFrame* side=new FXVerticalFrame(&about,LAYOUT_SIDE_RIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 10,10,10,10, 0,0);
  new FXLabel(side,"A . d . i . e",NULL,JUSTIFY_LEFT|ICON_BEFORE_TEXT|LAYOUT_FILL_X);
  new FXHorizontalSeparator(side,SEPARATOR_LINE|LAYOUT_FILL_X);
  new FXLabel(side,FXStringFormat("\nThe Adie ADvanced Interactive Editor, version 1.3.0.\n\nAdie is a fast and convenient programming text editor and text\nfile viewer with an integrated file browser.\nAdie uses the FOX Toolkit version %d.%d.%d.\nCopyright (C) 2000,2002 Jeroen van der Zijp (jeroen@fox-toolkit.org).\n ",FOX_MAJOR,FOX_MINOR,FOX_LEVEL),NULL,JUSTIFY_LEFT|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXButton *button=new FXButton(side,"&OK",NULL,&about,FXDialogBox::ID_ACCEPT,BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT,0,0,0,0,32,32,2,2);
  button->setFocus();
  about.execute(PLACEMENT_OWNER);
  return 1;
  }


// Show help window
long TextWindow::onCmdShowHelp(FXObject*,FXSelector,void*){
  helpwindow->show(PLACEMENT_CURSOR);
  return 1;
  }


// Show preferences dialog
long TextWindow::onCmdPreferences(FXObject*,FXSelector,void*){
  Preferences preferences(this);
  preferences.setPatterns(getPatterns());
  if(preferences.execute()){
    setPatterns(preferences.getPatterns());
    }
  return 1;
  }


// Change font
long TextWindow::onCmdFont(FXObject*,FXSelector,void*){
  FXFontDialog fontdlg(this,"Change Font",DECOR_BORDER|DECOR_TITLE);
  FXFontDesc fontdesc;
  editor->getFont()->getFontDesc(fontdesc);
  fontdlg.setFontSelection(fontdesc);
  if(fontdlg.execute()){
    FXFont *oldfont=font;
    fontdlg.getFontSelection(fontdesc);
    font=new FXFont(getApp(),fontdesc);
    font->create();
    editor->setFont(font);
    delete oldfont;
    }
  return 1;
  }


/*******************************************************************************/


// Reopen file
long TextWindow::onCmdReopen(FXObject*,FXSelector,void*){
  if(!undolist.marked()){
    if(FXMessageBox::question(this,MBOX_YES_NO,"Document was changed","Discard changes to this document?")==MBOX_CLICKED_NO) return 1;
    }
  loadFile(filename);
  return 1;
  }


// Update reopen file
long TextWindow::onUpdReopen(FXObject* sender,FXSelector,void* ptr){
  if(filenameset)
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),ptr);
  else
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),ptr);
  return 1;
  }


// Save changes, prompt for new filename
FXbool TextWindow::saveChanges(){
  FXuint answer;
  FXString file;
  writeBookmarks(filename);
  writeView(filename);
  if(!undolist.marked()){
    answer=FXMessageBox::question(this,MBOX_YES_NO_CANCEL,"Unsaved Document","Save current document to file?");
    if(answer==MBOX_CLICKED_CANCEL) return FALSE;
    if(answer==MBOX_CLICKED_YES){
      file=filename;
      if(!filenameset){
        FXFileDialog savedialog(this,"Save Document");
        savedialog.setSelectMode(SELECTFILE_ANY);
        savedialog.setPatternList(getPatterns());
        savedialog.setCurrentPattern(getCurrentPattern());
        savedialog.setFilename(file);
        if(!savedialog.execute()) return FALSE;
        setCurrentPattern(savedialog.getCurrentPattern());
        file=savedialog.getFilename();
        if(FXFile::exists(file)){
          if(MBOX_CLICKED_NO==FXMessageBox::question(this,MBOX_YES_NO,"Overwrite Document","Overwrite existing document: %s?",file.text())) return FALSE;
          }
        file=savedialog.getFilename();
        }
      saveFile(file);
      }
    }
  return TRUE;
  }


// Open
long TextWindow::onCmdOpen(FXObject*,FXSelector,void*){
  FXString file;
  if(!saveChanges()) return 1;
  FXFileDialog opendialog(this,"Open Document");
  opendialog.setSelectMode(SELECTFILE_EXISTING);
  opendialog.setPatternList(getPatterns());
  opendialog.setCurrentPattern(getCurrentPattern());
  opendialog.setFilename(filename);
  if(opendialog.execute()){
    setCurrentPattern(opendialog.getCurrentPattern());
    file=opendialog.getFilename();
    loadFile(file);
    readBookmarks(file);
    readView(file);
    }
  return 1;
  }


// Insert file into buffer
long TextWindow::onCmdInsertFile(FXObject*,FXSelector,void*){
  FXString file;
  FXFileDialog opendialog(this,"Open Document");
  opendialog.setSelectMode(SELECTFILE_EXISTING);
  opendialog.setPatternList(getPatterns());
  opendialog.setCurrentPattern(getCurrentPattern());
  if(opendialog.execute()){
    setCurrentPattern(opendialog.getCurrentPattern());
    file=opendialog.getFilename();
    insertFile(file);
    }
  return 1;
  }


// Update insert file
long TextWindow::onUpdInsertFile(FXObject* sender,FXSelector,void*){
  if(editor->isEditable())
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Extract selection to file
long TextWindow::onCmdExtractFile(FXObject*,FXSelector,void*){
  FXFileDialog savedialog(this,"Save Document");
  FXString file="untitled";
  savedialog.setSelectMode(SELECTFILE_ANY);
  savedialog.setPatternList(getPatterns());
  savedialog.setCurrentPattern(getCurrentPattern());
  savedialog.setFilename(file);
  if(savedialog.execute()){
    setCurrentPattern(savedialog.getCurrentPattern());
    file=savedialog.getFilename();
    if(FXFile::exists(file)){
      if(MBOX_CLICKED_NO==FXMessageBox::question(this,MBOX_YES_NO,"Overwrite Document","Overwrite existing document: %s?",file.text())) return 1;
      }
    extractFile(file);
    }
  return 1;
  }


// Update extract file
long TextWindow::onUpdExtractFile(FXObject* sender,FXSelector,void*){
  if(editor->hasSelection())
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Open Selected
long TextWindow::onCmdOpenSelected(FXObject*,FXSelector,void*){
  FXchar name[1024],*data;
  FXuint len;
  FXint pos=0,lineno=0;

  // Get selection
  if(getDNDData(FROM_SELECTION,stringType,(FXuchar*&)data,len)){
    FXString string(data,len);
    FXFREE(&data);

    // Its too big, most likely not a file name
    if(len<1024){
      FXString file=FXString::null;
      FXString dir=FXFile::getCurrentDirectory();

      // Base off currently loaded file
      if(!filename.empty()) dir=FXFile::directory(filename);

      // Strip leading/trailing space
      string.trim();

      // Extract name from #include syntax
      if(sscanf(string.text(),"#include \"%[^\"]\"",name)==1){
        file=FXFile::absolute(dir,name);
        if(!FXFile::exists(file)){
          file=FXFile::search(searchpath,name);
          }
        }
      else if(sscanf(string.text(),"#include <%[^>]>",name)==1){
        file=FXFile::absolute(dir,name);
        if(!FXFile::exists(file)){
          file=FXFile::search(searchpath,name);
          }
        }

      // Compiler output in the form filename:lineno: Error message
      else if(sscanf(string.text(),"%[^:]:%d:",name,&lineno)==2){
        file=FXFile::absolute(dir,name);
        if(!FXFile::exists(file)){
          file=FXFile::absolute(dir,string);
          }
        }

      // Compiler output in the form filename: Other stuff
      else if(sscanf(string.text(),"%[^:]:",name)==1){
        file=FXFile::absolute(dir,name);
        if(!FXFile::exists(file)){
          file=FXFile::absolute(dir,string);
          }
        }

      // Try whole selection
      else{
        file=FXFile::absolute(dir,string);
        }

      // Not a file name
      if(FXFile::exists(file)){

        // Different from current file?
        if(file!=filename){

          // Save old file
          if(!saveChanges()) return 1;

          // Open it
          loadFile(file);
          }

        // Switch line number only
        if(lineno){
          pos=editor->nextLine(0,lineno-1);
          editor->setCursorPos(pos);
          editor->setCenterLine(pos);
          }
        return 1;
        }
      }
    getApp()->beep();
    }
  return 1;
  }


// Open recent file
long TextWindow::onCmdRecentFile(FXObject*,FXSelector,void* ptr){
  FXString filename=(const char*)ptr;
  if(!saveChanges()) return 1;
  loadFile(filename);
  readBookmarks(filename);
  readView(filename);
  return 1;
  }


// Save
long TextWindow::onCmdSave(FXObject* sender,FXSelector sel,void* ptr){
  if(!filenameset) return onCmdSaveAs(sender,sel,ptr);
  saveFile(filename);
  return 1;
  }


// Save Update
long TextWindow::onUpdSave(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=!undolist.marked() ? FXWindow::ID_ENABLE : FXWindow::ID_DISABLE;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }


// Save As
long TextWindow::onCmdSaveAs(FXObject*,FXSelector,void*){
  FXFileDialog savedialog(this,"Save Document");
  FXString file=filename;
  savedialog.setSelectMode(SELECTFILE_ANY);
  savedialog.setPatternList(getPatterns());
  savedialog.setCurrentPattern(getCurrentPattern());
  savedialog.setFilename(file);
  if(savedialog.execute()){
    setCurrentPattern(savedialog.getCurrentPattern());
    file=savedialog.getFilename();
    if(FXFile::exists(file)){
      if(MBOX_CLICKED_NO==FXMessageBox::question(this,MBOX_YES_NO,"Overwrite Document","Overwrite existing document: %s?",file.text())) return 1;
      }
    saveFile(file);
    }
  return 1;
  }


// New
long TextWindow::onCmdNew(FXObject*,FXSelector,void*){
  if(!saveChanges()) return 1;
  filename="untitled";
  filetime=0;
  filenameset=FALSE;
  editor->setText(NULL,0);
  editor->setModified(FALSE);
  editor->setEditable(TRUE);
  undolist.clear();
  undolist.mark();
  clearBookmarks();
  return 1;
  }


// Quit
long TextWindow::onCmdQuit(FXObject*,FXSelector,void*){
  if(!saveChanges()) return 1;
  writeRegistry();
  getApp()->exit(0);
  return 1;
  }


// Update title
long TextWindow::onUpdTitle(FXObject* sender,FXSelector,void*){
  FXString title="Adie:- " + filename;
  if(!undolist.marked()) title+="*";
  sender->handle(this,MKUINT(FXWindow::ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&title);
  return 1;
  }


// Print the text
long TextWindow::onCmdPrint(FXObject*,FXSelector,void*){
  FXPrintDialog dlg(this,"Print File");
  FXPrinter printer;
  if(dlg.execute()){
    dlg.getPrinter(printer);
    FXTRACE((100,"Printer = %s\n",printer.name.text()));
    }
  return 1;
  }


// Command from the tree list
long TextWindow::onCmdTreeList(FXObject*,FXSelector,void* ptr){
  FXTreeItem *item=(FXTreeItem*)ptr;
  FXString file;
  if(!item || !dirlist->isItemFile(item)) return 1;
  if(!saveChanges()) return 1;
  file=dirlist->getItemPathname(item);
  loadFile(file);
  readBookmarks(file);
  readView(file);
  return 1;
  }


// See if we can get it as a filename
long TextWindow::onEditDNDDrop(FXObject*,FXSelector,void*){
  FXchar *data; FXuint len;
  if(getDNDData(FROM_DRAGNDROP,urilistType,(FXuchar*&)data,len)){
    FXString urilist(data,len);
    FXString file=FXURL::fileFromURL(urilist.before('\r'));
    FXFREE(&data);
    if(file.empty()) return 1;
    if(!saveChanges()) return 1;
    loadFile(file);
    readBookmarks(file);
    readView(file);
    return 1;
    }
  return 0;
  }


// See if a filename is being dragged over the window
long TextWindow::onEditDNDMotion(FXObject*,FXSelector,void*){
  if(offeredDNDType(FROM_DRAGNDROP,urilistType)){
    acceptDrop(DRAG_COPY);
    return 1;
    }
  return 0;
  }


/*******************************************************************************/

// Save settings
long TextWindow::onCmdSaveSettings(FXObject*,FXSelector,void*){
  writeRegistry();
  getApp()->reg().write();
  return 1;
  }


// Toggle wrap mode
long TextWindow::onCmdWrap(FXObject*,FXSelector,void*){
  editor->setTextStyle(editor->getTextStyle()^TEXT_WORDWRAP);
  return 1;
  }


// Update toggle wrap mode
long TextWindow::onUpdWrap(FXObject* sender,FXSelector,void*){
  if(editor->getTextStyle()&TEXT_WORDWRAP)
    sender->handle(this,MKUINT(ID_CHECK,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_UNCHECK,SEL_COMMAND),NULL);
  return 1;
  }


// Toggle fixed wrap mode
long TextWindow::onCmdWrapFixed(FXObject*,FXSelector,void*){
  editor->setTextStyle(editor->getTextStyle()^TEXT_FIXEDWRAP);
  return 1;
  }


// Update toggle fixed wrap mode
long TextWindow::onUpdWrapFixed(FXObject* sender,FXSelector,void*){
  if(editor->getTextStyle()&TEXT_FIXEDWRAP)
    sender->handle(this,MKUINT(ID_CHECK,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_UNCHECK,SEL_COMMAND),NULL);
  return 1;
  }

// Toggle show active background mode
long TextWindow::onCmdShowActive(FXObject*,FXSelector,void*){
  editor->setTextStyle(editor->getTextStyle()^TEXT_SHOWACTIVE);
  return 1;
  }


// Update show active background mode
long TextWindow::onUpdShowActive(FXObject* sender,FXSelector,void*){
  if(editor->getTextStyle()&TEXT_SHOWACTIVE)
    sender->handle(this,MKUINT(ID_CHECK,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_UNCHECK,SEL_COMMAND),NULL);
  return 1;
  }

// Toggle strip returns mode
long TextWindow::onCmdStripReturns(FXObject*,FXSelector,void*){
  stripcr=!stripcr;
  return 1;
  }


// Update toggle strip returns mode
long TextWindow::onUpdStripReturns(FXObject* sender,FXSelector,void*){
  if(stripcr)
    sender->handle(this,MKUINT(ID_CHECK,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_UNCHECK,SEL_COMMAND),NULL);
  return 1;
  }


// Enable warning if file changed externally
long TextWindow::onCmdWarnChanged(FXObject*,FXSelector,void*){
  warnchanged=!warnchanged;
  if(warnchanged && !filetimer) filetimer=getApp()->addTimeout(FILETIMER,this,ID_FILETIME);
  if(!warnchanged && filetimer) filetimer=getApp()->removeTimeout(filetimer);
  return 1;
  }


// Update check button for warning
long TextWindow::onUpdWarnChanged(FXObject* sender,FXSelector,void*){
  if(warnchanged)
    sender->handle(this,MKUINT(ID_CHECK,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_UNCHECK,SEL_COMMAND),NULL);
  return 1;
  }


// Toggle strip spaces mode
long TextWindow::onCmdStripSpaces(FXObject*,FXSelector,void*){
  stripsp=!stripsp;
  return 1;
  }


// Update toggle strip spaces mode
long TextWindow::onUpdStripSpaces(FXObject* sender,FXSelector,void*){
  if(stripsp)
    sender->handle(this,MKUINT(ID_CHECK,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_UNCHECK,SEL_COMMAND),NULL);
  return 1;
  }


// Change tab columns
long TextWindow::onCmdTabColumns(FXObject* sender,FXSelector,void*){
  FXint tabs;
  sender->handle(this,MKUINT(ID_GETINTVALUE,SEL_COMMAND),(void*)&tabs);
  editor->setTabColumns(tabs);
  return 1;
  }


// Update tab columns
long TextWindow::onUpdTabColumns(FXObject* sender,FXSelector,void*){
  FXint tabs=editor->getTabColumns();
  sender->handle(this,MKUINT(ID_SETINTVALUE,SEL_COMMAND),(void*)&tabs);
  return 1;
  }


// Change wrap columns
long TextWindow::onCmdWrapColumns(FXObject* sender,FXSelector,void*){
  FXint wrap;
  sender->handle(this,MKUINT(ID_GETINTVALUE,SEL_COMMAND),(void*)&wrap);
  editor->setWrapColumns(wrap);
  return 1;
  }


// Update wrap columns
long TextWindow::onUpdWrapColumns(FXObject* sender,FXSelector,void*){
  FXint wrap=editor->getWrapColumns();
  sender->handle(this,MKUINT(ID_SETINTVALUE,SEL_COMMAND),(void*)&wrap);
  return 1;
  }


// Change line number columna
long TextWindow::onCmdLineNumbers(FXObject* sender,FXSelector,void*){
  FXint cols;
  sender->handle(this,MKUINT(ID_GETINTVALUE,SEL_COMMAND),(void*)&cols);
  editor->setBarColumns(cols);
  return 1;
  }


// Update line number columna
long TextWindow::onUpdLineNumbers(FXObject* sender,FXSelector,void*){
  FXint cols=editor->getBarColumns();
  sender->handle(this,MKUINT(ID_SETINTVALUE,SEL_COMMAND),(void*)&cols);
  return 1;
  }


// Toggle insertion of tabs
long TextWindow::onCmdInsertTabs(FXObject*,FXSelector,void*){
  editor->setTextStyle(editor->getTextStyle()^TEXT_NO_TABS);
  return 1;
  }


// Update insertion of tabs
long TextWindow::onUpdInsertTabs(FXObject* sender,FXSelector,void*){
  sender->handle(this,(editor->getTextStyle()&TEXT_NO_TABS)?MKUINT(ID_UNCHECK,SEL_COMMAND):MKUINT(ID_CHECK,SEL_COMMAND),NULL);
  return 1;
  }


// Toggle autoindent
long TextWindow::onCmdAutoIndent(FXObject*,FXSelector,void*){
  editor->setTextStyle(editor->getTextStyle()^TEXT_AUTOINDENT);
  return 1;
  }


// Update autoindent
long TextWindow::onUpdAutoIndent(FXObject* sender,FXSelector,void*){
  sender->handle(this,(editor->getTextStyle()&TEXT_AUTOINDENT)?MKUINT(ID_CHECK,SEL_COMMAND):MKUINT(ID_UNCHECK,SEL_COMMAND),NULL);
  return 1;
  }


// Set brace match time
long TextWindow::onCmdBraceMatch(FXObject* sender,FXSelector,void*){
  FXuint value;
  sender->handle(this,MKUINT(ID_GETINTVALUE,SEL_COMMAND),(void*)&value);
  editor->setHiliteMatchTime(value);
  return 1;
  }


// Update brace match time
long TextWindow::onUpdBraceMatch(FXObject* sender,FXSelector,void*){
  FXuint value=editor->getHiliteMatchTime();
  sender->handle(this,MKUINT(ID_SETINTVALUE,SEL_COMMAND),(void*)&value);
  return 1;
  }


// Change word delimiters
long TextWindow::onCmdDelimiters(FXObject* sender,FXSelector,void*){
  FXString string;
  sender->handle(this,MKUINT(ID_GETSTRINGVALUE,SEL_COMMAND),(void*)&string);
  editor->setDelimiters(string);
  return 1;
  }


// Update word delimiters
long TextWindow::onUpdDelimiters(FXObject* sender,FXSelector,void*){
  FXString string=editor->getDelimiters();
  sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&string);
  return 1;
  }


// Update box for overstrike mode display
long TextWindow::onUpdOverstrike(FXObject* sender,FXSelector,void*){
  FXString mode((editor->getTextStyle()&TEXT_OVERSTRIKE)?"OVR":"INS");
  sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&mode);
  return 1;
  }


// Update box for readonly display
long TextWindow::onUpdReadOnly(FXObject* sender,FXSelector,void*){
  FXString rw((editor->getTextStyle()&TEXT_READONLY)?"RO":"RW");
  sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&rw);
  return 1;
  }


// Update box for size display
long TextWindow::onUpdNumChars(FXObject* sender,FXSelector,void*){
  FXuint size=editor->getLength();
  sender->handle(this,MKUINT(ID_SETINTVALUE,SEL_COMMAND),(void*)&size);
  return 1;
  }


// Set TextWindow path
long TextWindow::onCmdIncludePaths(FXObject*,FXSelector,void*){
  FXInputDialog::getString(searchpath,this,"Change include file search path","Specify a list of directories separated by a `" PATHLISTSEPSTRING "' where include files are to be found.\nFor example:\n\n  /usr/include" PATHLISTSEPSTRING "/usr/local/include\n\nThis list will be used to locate the selected file name.");
  return 1;
  }


// Set scroll wheel lines (Mathew Robertson <mathew@optushome.com.au>)
long TextWindow::onCmdWheelAdjust(FXObject* sender,FXSelector,void*){
  FXuint value;
  sender->handle(this,MKUINT(ID_GETINTVALUE,SEL_COMMAND),(void*)&value);
  getApp()->setWheelLines(value);
  return 1;
  }


// Update brace match time
long TextWindow::onUpdWheelAdjust(FXObject* sender,FXSelector,void*){
  FXuint value=getApp()->getWheelLines();
  sender->handle(this,MKUINT(ID_SETINTVALUE,SEL_COMMAND),(void*)&value);
  return 1;
  }


/*******************************************************************************/


// Change text color
long TextWindow::onCmdTextForeColor(FXObject*,FXSelector,void* ptr){
  editor->setTextColor((FXColor)(FXuval)ptr);
  return 1;
  }

// Update text color
long TextWindow::onUpdTextForeColor(FXObject* sender,FXSelector,void*){
  FXColor color=editor->getTextColor();
  sender->handle(this,MKUINT(ID_SETINTVALUE,SEL_COMMAND),(void*)&color);
  return 1;
  }


// Change text background color
long TextWindow::onCmdTextBackColor(FXObject*,FXSelector,void* ptr){
  editor->setBackColor((FXColor)(FXuval)ptr);
  return 1;
  }

// Update background color
long TextWindow::onUpdTextBackColor(FXObject* sender,FXSelector,void*){
  FXColor color=editor->getBackColor();
  sender->handle(this,MKUINT(ID_SETINTVALUE,SEL_COMMAND),(void*)&color);
  return 1;
  }


// Change selected text foreground color
long TextWindow::onCmdTextSelForeColor(FXObject*,FXSelector,void* ptr){
  editor->setSelTextColor((FXColor)(FXuval)ptr);
  return 1;
  }


// Update selected text foregoround color
long TextWindow::onUpdTextSelForeColor(FXObject* sender,FXSelector,void*){
  FXColor color=editor->getSelTextColor();
  sender->handle(this,MKUINT(ID_SETINTVALUE,SEL_COMMAND),(void*)&color);
  return 1;
  }


// Change selected text background color
long TextWindow::onCmdTextSelBackColor(FXObject*,FXSelector,void* ptr){
  editor->setSelBackColor((FXColor)(FXuval)ptr);
  return 1;
  }

// Update selected text background color
long TextWindow::onUpdTextSelBackColor(FXObject* sender,FXSelector,void*){
  FXColor color=editor->getSelBackColor();
  sender->handle(this,MKUINT(ID_SETINTVALUE,SEL_COMMAND),(void*)&color);
  return 1;
  }


// Change hilight text color
long TextWindow::onCmdTextHiliteForeColor(FXObject*,FXSelector,void* ptr){
  editor->setHiliteTextColor((FXColor)(FXuval)ptr);
  return 1;
  }

// Update hilight text color
long TextWindow::onUpdTextHiliteForeColor(FXObject* sender,FXSelector,void*){
  FXColor color=editor->getHiliteTextColor();
  sender->handle(this,MKUINT(ID_SETINTVALUE,SEL_COMMAND),(void*)&color);
  return 1;
  }


// Change hilight text background color
long TextWindow::onCmdTextHiliteBackColor(FXObject*,FXSelector,void* ptr){
  editor->setHiliteBackColor((FXColor)(FXuval)ptr);
  return 1;
  }

// Update hilight text background color
long TextWindow::onUpdTextHiliteBackColor(FXObject* sender,FXSelector,void*){
  FXColor color=editor->getHiliteBackColor();
  sender->handle(this,MKUINT(ID_SETINTVALUE,SEL_COMMAND),(void*)&color);
  return 1;
  }


// Change active text background color
long TextWindow::onCmdTextActBackColor(FXObject*,FXSelector,void* ptr){
  editor->setActiveBackColor((FXColor)(FXuval)ptr);
  return 1;
  }

// Update active text background color
long TextWindow::onUpdTextActBackColor(FXObject* sender,FXSelector,void*){
  FXColor color=editor->getActiveBackColor();
  sender->handle(this,MKUINT(ID_SETINTVALUE,SEL_COMMAND),(void*)&color);
  return 1;
  }


// Change cursor color
long TextWindow::onCmdTextCursorColor(FXObject*,FXSelector,void* ptr){
  editor->setCursorColor((FXColor)(FXuval)ptr);
  return 1;
  }

// Update cursor color
long TextWindow::onUpdTextCursorColor(FXObject* sender,FXSelector,void*){
  FXColor color=editor->getCursorColor();
  sender->handle(sender,MKUINT(FXWindow::ID_SETINTVALUE,SEL_COMMAND),(void*)&color);
  return 1;
  }


// Change line numbers background color
long TextWindow::onCmdTextBarColor(FXObject*,FXSelector,void* ptr){
  editor->setBarColor((FXColor)(FXuval)ptr);
  return 1;
  }


// Update line numbers background color
long TextWindow::onUpdTextBarColor(FXObject* sender,FXSelector,void*){
  FXColor color=editor->getBarColor();
  sender->handle(this,MKUINT(ID_SETINTVALUE,SEL_COMMAND),(void*)&color);
  return 1;
  }

// Change line numbers color
long TextWindow::onCmdTextNumberColor(FXObject*,FXSelector,void* ptr){
  editor->setNumberColor((FXColor)(FXuval)ptr);
  return 1;
  }


// Update line numbers color
long TextWindow::onUpdTextNumberColor(FXObject* sender,FXSelector,void*){
  FXColor color=editor->getNumberColor();
  sender->handle(this,MKUINT(ID_SETINTVALUE,SEL_COMMAND),(void*)&color);
  return 1;
  }


// Change both tree background color
long TextWindow::onCmdDirBackColor(FXObject*,FXSelector,void* ptr){
  dirlist->setBackColor((FXColor)(FXuval)ptr);
  return 1;
  }


// Update background color
long TextWindow::onUpdDirBackColor(FXObject* sender,FXSelector,void*){
  FXColor color=dirlist->getBackColor();
  sender->handle(this,MKUINT(ID_SETINTVALUE,SEL_COMMAND),(void*)&color);
  return 1;
  }


// Change both text and tree selected background color
long TextWindow::onCmdDirSelBackColor(FXObject*,FXSelector,void* ptr){
  dirlist->setSelBackColor((FXColor)(FXuval)ptr);
  return 1;
  }

// Update selected background color
long TextWindow::onUpdDirSelBackColor(FXObject* sender,FXSelector,void*){
  FXColor color=dirlist->getSelBackColor();
  sender->handle(this,MKUINT(ID_SETINTVALUE,SEL_COMMAND),(void*)&color);
  return 1;
  }


// Change both text and tree text color
long TextWindow::onCmdDirForeColor(FXObject*,FXSelector,void* ptr){
  dirlist->setTextColor((FXColor)(FXuval)ptr);
  return 1;
  }

// Forward GUI update to text widget
long TextWindow::onUpdDirForeColor(FXObject* sender,FXSelector,void*){
  FXColor color=dirlist->getTextColor();
  sender->handle(this,MKUINT(ID_SETINTVALUE,SEL_COMMAND),(void*)&color);
  return 1;
  }


// Change both text and tree
long TextWindow::onCmdDirSelForeColor(FXObject*,FXSelector,void* ptr){
  dirlist->setSelTextColor((FXColor)(FXuval)ptr);
  return 1;
  }


// Forward GUI update to text widget
long TextWindow::onUpdDirSelForeColor(FXObject* sender,FXSelector,void*){
  FXColor color=dirlist->getSelTextColor();
  sender->handle(sender,MKUINT(FXWindow::ID_SETINTVALUE,SEL_COMMAND),(void*)&color);
  return 1;
  }


// Change both text and tree
long TextWindow::onCmdDirLineColor(FXObject*,FXSelector,void* ptr){
  dirlist->setLineColor((FXColor)(FXuval)ptr);
  return 1;
  }


// Forward GUI update to text widget
long TextWindow::onUpdDirLineColor(FXObject* sender,FXSelector,void*){
  FXColor color=dirlist->getLineColor();
  sender->handle(sender,MKUINT(FXWindow::ID_SETINTVALUE,SEL_COMMAND),(void*)&color);
  return 1;
  }


/*******************************************************************************/


// Text inserted
long TextWindow::onTextInserted(FXObject*,FXSelector,void* ptr){
  FXint *what=(FXint*)ptr;
  FXTRACE((140,"Inserted: %d %d\n",what[0],what[1]));
  undolist.add(new FXTextInsert(editor,what[0],what[1]));


// #ifdef TEST
//   FXint sta=what[0];
//   FXint fin=what[0]+what[1];
//   FXint beg=editor->lineStart(sta);
//   //FXint end=editor->nextLine(fin);
//   FXint end=editor->getLength();
//   FXint len=end-beg;
//   FXchar *text,*style;
//   FXMALLOC(&text,FXchar,len+len);
//   style=text+len;
//   editor->extractText(text,beg,len);
//   hilite.stylize(text,style,0,len);
//   editor->changeStyle(beg,len,style);
//   FXFREE(&text);
// #endif

#ifdef TEST
restyle=TRUE;
#endif

  // Keep the undo list in check by trimming it down to KEEPUNDOSIZE
  // whenever the amount of undo buffering exceeds MAXUNDOSIZE.
  if(undolist.undoSize()>MAXUNDOSIZE) undolist.trimSize(KEEPUNDOSIZE);
  return 1;
  }


// Text deleted
long TextWindow::onTextDeleted(FXObject*,FXSelector,void* ptr){
  FXint *what=(FXint*)ptr;
  FXTRACE((140,"Deleted: %d %d\n",what[0],what[1]));
  undolist.add(new FXTextDelete(editor,what[0],what[1]));

// #ifdef TEST
//   FXint sta=what[0];
//   FXint fin=what[0]+what[1];
//   FXint beg=editor->lineStart(sta);
//   //FXint end=editor->lineEnd(fin);
//   //FXint end=editor->nextLine(fin);
//   FXint end=editor->getLength();
//   FXint len=(sta-beg)+(end-fin);
//   FXchar *text,*style;
//   FXMALLOC(&text,FXchar,len+len);
//   style=text+len;
//   editor->extractText(text,beg,sta-beg);
//   editor->extractText(&text[sta-beg],fin,end-fin);
//   hilite.stylize(text,style,0,len);
//   editor->changeStyle(beg,sta-beg,style);
//   editor->changeStyle(fin,end-fin,style);
//   FXFREE(&text);
// #endif
#ifdef TEST
restyle=TRUE;
#endif

  // Keep the undo list in check by trimming it down to KEEPUNDOSIZE
  // whenever the amount of undo buffering exceeds MAXUNDOSIZE.
  if(undolist.undoSize()>MAXUNDOSIZE) undolist.trimSize(KEEPUNDOSIZE);
  return 1;
  }


// Text changed
long TextWindow::onTextChanged(FXObject*,FXSelector,void*){
  FXTRACE((10,"onTextChanged\n"));
#ifdef TEST
//restyle=TRUE;
#endif
  return 1;
  }


// Released right button
long TextWindow::onTextRightMouse(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  if(!event->moved){
    popupmenu->popup(NULL,event->root_x,event->root_y);
    getApp()->runModalWhileShown(popupmenu);
    }
  return 1;
  }


/*******************************************************************************/


// Change patterns, each pattern separated by newline
void TextWindow::setPatterns(const FXString& patterns){
  FXString pat; FXint i;
  filter->clearItems();
  for(i=0; !(pat=patterns.extract(i,'\n')).empty(); i++){
    filter->appendItem(pat);
    }
  if(!filter->getNumItems()) filter->appendItem("All Files (*)");
  setCurrentPattern(0);
  }


// Return list of patterns
FXString TextWindow::getPatterns() const {
  FXString pat; FXint i;
  for(i=0; i<filter->getNumItems(); i++){
    if(!pat.empty()) pat+='\n';
    pat+=filter->getItemText(i);
    }
  return pat;
  }


// Strip pattern from text if present
static FXString patternFromText(const FXString& pattern){
  FXint beg,end;
  end=pattern.findb(')');
  beg=pattern.findb('(',end-1);
  if(0<=beg && beg<end) return pattern.mid(beg+1,end-beg-1);
  return pattern;
  }


// Set current pattern
void TextWindow::setCurrentPattern(FXint n){
  n=FXCLAMP(0,n,filter->getNumItems()-1);
  filter->setCurrentItem(n);
  dirlist->setPattern(patternFromText(filter->getItemText(n)));
  }


// Return current pattern
FXint TextWindow::getCurrentPattern() const {
  return filter->getCurrentItem();
  }


// Change the pattern
long TextWindow::onCmdFilter(FXObject*,FXSelector,void* ptr){
  dirlist->setPattern(patternFromText((FXchar*)ptr));
  return 1;
  }


// Update box for readonly display
long TextWindow::onCheckFile(FXObject*,FXSelector,void*){
  register long t;
#ifdef TEST
  if(restyle){
    onCmdRestyle(NULL,0,NULL);
    restyle=FALSE;
    }
#endif
  if(filetime!=0){
    t=FXFile::modified(filename);
    if(t && t!=filetime){
      filetime=t;
      if(MBOX_CLICKED_OK==FXMessageBox::warning(this,MBOX_OK_CANCEL,"File Was Changed","The file was changed by another program\nReload this file from disk?")){
        FXint top=editor->getTopLine();
        FXint pos=editor->getCursorPos();
        loadFile(filename);
        editor->setTopLine(top);
        editor->setCursorPos(pos);
        }
      }
    }
  filetimer=getApp()->addTimeout(FILETIMER,this,ID_FILETIME);
  return 1;
  }


// Update clock
long TextWindow::onClock(FXObject*,FXSelector,void*){
  clock->setText(FXFile::time("%H:%M:%S",FXFile::now()));
  clocktimer=getApp()->addTimeout(CLOCKTIMER,this,ID_CLOCKTIME);
  return 1;
  }


/*******************************************************************************/


// Next bookmarked place
long TextWindow::onCmdNextMark(FXObject*,FXSelector,void*){
  register FXint b;
  if(bookmark[0].pos){
    FXint pos=editor->getCursorPos();
    for(b=0; b<=9; b++){
      if(bookmark[b].pos==0) break;
      if(bookmark[b].pos>pos){ gotoBookmark(b); break; }
      }
    }
  return 1;
  }


// Sensitize if bookmark beyond cursor pos
long TextWindow::onUpdNextMark(FXObject* sender,FXSelector,void*){
  register FXint b;
  if(bookmark[0].pos){
    FXint pos=editor->getCursorPos();
    for(b=0; b<=9; b++){
      if(bookmark[b].pos==0) break;
      if(bookmark[b].pos>pos){ sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL); return 1; }
      }
    }
  sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Previous bookmarked place
long TextWindow::onCmdPrevMark(FXObject*,FXSelector,void*){
  register FXint b;
  if(bookmark[0].pos){
    FXint pos=editor->getCursorPos();
    for(b=9; 0<=b; b--){
      if(bookmark[b].pos==0) continue;
      if(bookmark[b].pos<pos){ gotoBookmark(b); break; }
      }
    }
  return 1;
  }


// Sensitize if bookmark before cursor pos
long TextWindow::onUpdPrevMark(FXObject* sender,FXSelector,void*){
  register FXint b;
  if(bookmark[0].pos){
    FXint pos=editor->getCursorPos();
    for(b=9; 0<=b; b--){
      if(bookmark[b].pos==0) continue;
      if(bookmark[b].pos<pos){ sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL); return 1; }
      }
    }
  sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Set bookmark
long TextWindow::onCmdSetMark(FXObject*,FXSelector,void*){
  setBookmark(editor->getCursorPos(),editor->getTopLine());
  return 1;
  }


// Update set bookmark
long TextWindow::onUpdSetMark(FXObject* sender,FXSelector,void*){
  if(bookmark[9].pos==0)
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Clear bookmarks
long TextWindow::onCmdClearMarks(FXObject*,FXSelector,void*){
  clearBookmarks();
  return 1;
  }


// Add bookmark at current cursor position; we force the cursor
// position to be somewhere in the currently visible text.
void TextWindow::setBookmark(FXint pos,FXint top){
  register FXint b;
  if(!bookmark[9].pos){
    for(b=9; 0<b && (bookmark[b-1].pos==0 || bookmark[b-1].pos>pos); b--){
      bookmark[b]=bookmark[b-1];
      }
    bookmark[b].top=top;
    bookmark[b].pos=pos;
    }
  }


// Goto bookmark
void TextWindow::gotoBookmark(FXint b){
  if(bookmark[b].pos){
    if(!editor->isPosVisible(bookmark[b].pos)){
      editor->setTopLine(bookmark[b].top);
      }
    editor->setCursorPos(bookmark[b].pos);
    }
  }


// Clear bookmarks
void TextWindow::clearBookmarks(){
  bookmark[0].top=bookmark[0].pos=0;
  bookmark[1].top=bookmark[1].pos=0;
  bookmark[2].top=bookmark[2].pos=0;
  bookmark[3].top=bookmark[3].pos=0;
  bookmark[4].top=bookmark[4].pos=0;
  bookmark[5].top=bookmark[5].pos=0;
  bookmark[6].top=bookmark[6].pos=0;
  bookmark[7].top=bookmark[7].pos=0;
  bookmark[8].top=bookmark[8].pos=0;
  bookmark[9].top=bookmark[9].pos=0;
  }


// Read bookmarks associated with file
void TextWindow::readBookmarks(const FXString& file){
  const FXchar *marks=getApp()->reg().readStringEntry("BOOKMARKS",FXFile::name(file).text(),"0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
  sscanf(marks,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",&bookmark[0].top,&bookmark[0].pos,&bookmark[1].top,&bookmark[1].pos,&bookmark[2].top,&bookmark[2].pos,&bookmark[3].top,&bookmark[3].pos,&bookmark[4].top,&bookmark[4].pos,&bookmark[5].top,&bookmark[5].pos,&bookmark[6].top,&bookmark[6].pos,&bookmark[7].top,&bookmark[7].pos,&bookmark[8].top,&bookmark[8].pos,&bookmark[9].top,&bookmark[9].pos);
  }


// Write bookmarks associated with file, if any were set
void TextWindow::writeBookmarks(const FXString& file){
  if(savemarks){
    if(bookmark[0].pos || bookmark[1].pos || bookmark[2].pos || bookmark[3].pos || bookmark[4].pos || bookmark[5].pos || bookmark[6].pos || bookmark[7].pos || bookmark[8].pos || bookmark[9].pos){
      FXchar marks[1000];
      sprintf(marks,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",bookmark[0].top,bookmark[0].pos,bookmark[1].top,bookmark[1].pos,bookmark[2].top,bookmark[2].pos,bookmark[3].top,bookmark[3].pos,bookmark[4].top,bookmark[4].pos,bookmark[5].top,bookmark[5].pos,bookmark[6].top,bookmark[6].pos,bookmark[7].top,bookmark[7].pos,bookmark[8].top,bookmark[8].pos,bookmark[9].top,bookmark[9].pos);
      getApp()->reg().writeStringEntry("BOOKMARKS",FXFile::name(file).text(),marks);
      }
    else{
      getApp()->reg().deleteEntry("BOOKMARKS",FXFile::name(file).text());
      }
    }
  }


// Toggle saving of bookmarks
long TextWindow::onCmdSaveMarks(FXObject*,FXSelector,void*){
  savemarks=!savemarks;
  if(!savemarks) getApp()->reg().deleteSection("BOOKMARKS");
  return 1;
  }


// Update saving bookmarks
long TextWindow::onUpdSaveMarks(FXObject* sender,FXSelector,void*){
  sender->handle(this,savemarks?MKUINT(ID_CHECK,SEL_COMMAND):MKUINT(ID_UNCHECK,SEL_COMMAND),NULL);
  return 1;
  }


// Toggle saving of views
long TextWindow::onCmdSaveViews(FXObject*,FXSelector,void*){
  saveviews=!saveviews;
  if(!saveviews) getApp()->reg().deleteSection("VIEW");
  return 1;
  }


// Update saving views
long TextWindow::onUpdSaveViews(FXObject* sender,FXSelector,void*){
  sender->handle(this,saveviews?MKUINT(ID_CHECK,SEL_COMMAND):MKUINT(ID_UNCHECK,SEL_COMMAND),NULL);
  return 1;
  }


// Read view of the file
void TextWindow::readView(const FXString& file){
  FXTRACE((1,"file=%s\n",file.text()));
  editor->setTopLine(getApp()->reg().readIntEntry("VIEW",FXFile::name(file).text(),0));
  }


// Write current view of the file
void TextWindow::writeView(const FXString& file){
  if(saveviews){
    if(editor->getTopLine()){
      getApp()->reg().writeIntEntry("VIEW",FXFile::name(file).text(),editor->getTopLine());
      }
    else{
      getApp()->reg().deleteEntry("VIEW",FXFile::name(file).text());
      }
    }
  }


/*******************************************************************************/

// Determine language from filename
FXString TextWindow::getLanguage(const FXString& filename){
  FXchar key[10],name[200],extensions[2000];
  FXString file=FXFile::name(filename);
  for(int i=0; i<1000; i++){
    sprintf(key,"%d",i+1);
    if(!getApp()->reg().readFormatEntry("LANGUAGES",key,"%[^,],%[ -~]",name,extensions)) break;
    FXTRACE((1,"trying file=%s and pattern=%s\n",file.text(),extensions));
    if(FXFile::match(extensions,file)) return name;
    }
  return FXString::null;
  }


// Toggle syntax coloring
long TextWindow::onCmdSyntax(FXObject*,FXSelector,void*){
  FXint syntax=!editor->isStyled();
  editor->setStyled(syntax);
  if(syntax){
    // FIXME
    }
  return 1;
  }


// Update syntax coloring
long TextWindow::onUpdSyntax(FXObject* sender,FXSelector,void*){
  FXint syntax=editor->isStyled();
  sender->handle(this,MKUINT(ID_SETINTVALUE,SEL_COMMAND),(void*)&syntax);
  return 1;
  }


// Restyle text
long TextWindow::onCmdRestyle(FXObject*,FXSelector,void*){
#ifdef TEST
  FXint len=editor->getLength();
  FXchar *text,*style;
  FXTRACE((1,"onCmdRestyle\n"));
  FXMALLOC(&text,FXchar,len+len);
  style=text+len;
  editor->extractText(text,0,len);
  hilite.stylize(text,style,0,len);
  editor->changeStyle(0,len,style);
  FXFREE(&text);
#endif
  return 1;
  }


// Update restyle text
long TextWindow::onUpdRestyle(FXObject* sender,FXSelector,void*){
  FXint syntax=editor->isStyled();
  sender->handle(this,syntax?MKUINT(ID_ENABLE,SEL_COMMAND):MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Read syntax rules
void TextWindow::readSyntax(const FXString& lang){
  FXchar index[10],name[200],pattern[2000];
  HLNode *nodes[512];
  FXint sty,pri,par,ctx,i;
  hilite.clear();
  nodes[0]=NULL;
  for(i=1; i<ARRAYNUMBER(nodes); i++){
    sprintf(index,"%d",i);
    if(!getApp()->reg().readFormatEntry(lang.text(),index,"%[^,],%d,%d,%d,%d,%[ -~]",name,&sty,&pri,&par,&ctx,pattern)) break;
    FXTRACE((1,"name=\"%s\" sty=%d pri=%d par=%d ctx=%d rex=\"%s\"\n",name,sty,pri,par,ctx,pattern));
    if(sty>nstyles || par>=i) break;
    nodes[i]=hilite.append(pattern,sty,pri,ctx,nodes[par]);
    if(!nodes[i]) break;
    }
  }


/*******************************************************************************/

// Delete style
void TextWindow::deleteStyle(FXint index){
  register FXint i;
  FXASSERT(index<nstyles);
  for(i=index+1; i<nstyles; i++){
    stylename[i-1]=stylename[i];
    stylecolor[i-1]=stylecolor[i];
    }
  editor->update();
  nstyles--;
  }


// Append style
void TextWindow::appendStyle(const FXString& name,const FXHiliteStyle& style){
  FXASSERT(nstyles<MAXSTYLES);
  stylename[nstyles]=name;
  stylecolor[nstyles]=style;
  editor->update();
  nstyles++;
  }


// Change style colors
void TextWindow::setStyleColors(FXint index,const FXHiliteStyle& style){
  stylecolor[index]=style;
  editor->update();
  }


// Read styles
void TextWindow::readStyles(){
  FXchar nfg[100],nbg[100],sfg[100],sbg[100],hfg[100],hbg[100],abg[100],index[10],name[200];
  FXint  sty,i;
  nstyles=0;
  for(i=0; i<MAXSTYLES; i++){
    sprintf(index,"%d",i+1);
    if(getApp()->reg().readFormatEntry("STYLES",index,"%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%d",name,nfg,nbg,sfg,sbg,hfg,hbg,abg,&sty)!=9) break;
    FXTRACE((1,"name=\"%s\" nfg=%s nbg=%s sfg=%s sbg=%s hfg=%s hbg=%s abg=%s sty=%d\n",name,nfg,nbg,sfg,sbg,hfg,hbg,abg,sty));
    stylename[i]=name;
    stylecolor[i].normalForeColor=fxcolorfromname(nfg);
    stylecolor[i].normalBackColor=fxcolorfromname(nbg);
    stylecolor[i].selectForeColor=fxcolorfromname(sfg);
    stylecolor[i].selectBackColor=fxcolorfromname(sbg);
    stylecolor[i].hiliteForeColor=fxcolorfromname(hfg);
    stylecolor[i].hiliteBackColor=fxcolorfromname(hbg);
    stylecolor[i].activeBackColor=fxcolorfromname(abg);
    stylecolor[i].style=sty;
    nstyles++;
    }
  }


// Write styles
void TextWindow::writeStyles(){
  FXchar nfg[100],nbg[100],sfg[100],sbg[100],hfg[100],hbg[100],abg[100],name[10];
  FXint  i;
  getApp()->reg().deleteSection("STYLES");
  for(i=0; i<nstyles; i++){
    fxnamefromcolor(nfg,stylecolor[i].normalForeColor);
    fxnamefromcolor(nbg,stylecolor[i].normalBackColor);
    fxnamefromcolor(sfg,stylecolor[i].selectForeColor);
    fxnamefromcolor(sbg,stylecolor[i].selectBackColor);
    fxnamefromcolor(hfg,stylecolor[i].hiliteForeColor);
    fxnamefromcolor(hbg,stylecolor[i].hiliteBackColor);
    fxnamefromcolor(abg,stylecolor[i].activeBackColor);
    sprintf(name,"%d",i+1);
    getApp()->reg().writeFormatEntry("STYLES",name,"%s,%s,%s,%s,%s,%s,%s,%s,%d",stylename[i].text(),nfg,nbg,sfg,sbg,hfg,hbg,abg,stylecolor[i].style);
    }
  }


/*******************************************************************************/


// Start the whole thing
int main(int argc,char *argv[]){

  // Make application
  FXApp application("Adie",FXString::null);

  // Open display
  application.init(argc,argv);

  // Make window
  TextWindow* window=new TextWindow(&application);

  // Handle interrupt to save stuff nicely
  application.addSignal(SIGINT,window,TextWindow::ID_QUIT);

  // Create it
  application.create();

  // Start
  if(argc>1){
    window->loadFile(argv[1]);
    window->readBookmarks(argv[1]);
    window->readView(argv[1]);
    }

  // Run
  return application.run();
  }


