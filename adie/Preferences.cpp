/********************************************************************************
*                                                                               *
*                        P r e f e r e n c e s   D i a l o g                    *
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
* $Id: Preferences.cpp,v 1.47.4.1 2003/06/20 19:02:07 fox Exp $                     *
********************************************************************************/
#include "fx.h"
#include "FXRex.h"
#include "icons.h"
#include "help.h"
#include <stdio.h>
#include <stdlib.h>
#include "Preferences.h"
#include "Hilite.h"
#include "Adie.h"
#include "icons.h"


/*******************************************************************************/

FXDEFMAP(Preferences) PreferencesMap[]={
  FXMAPFUNC(SEL_UPDATE,Preferences::ID_STYLE_NAME,Preferences::onUpdStyleName),
  FXMAPFUNC(SEL_COMMAND,Preferences::ID_STYLE_NAME,Preferences::onCmdStyleName),
  FXMAPFUNCS(SEL_UPDATE,Preferences::ID_STYLE_NORMAL_FG,Preferences::ID_STYLE_ACTIVE_BG,Preferences::onUpdStyleColor),
  FXMAPFUNCS(SEL_CHANGED,Preferences::ID_STYLE_NORMAL_FG,Preferences::ID_STYLE_ACTIVE_BG,Preferences::onCmdStyleColor),
  FXMAPFUNCS(SEL_COMMAND,Preferences::ID_STYLE_NORMAL_FG,Preferences::ID_STYLE_ACTIVE_BG,Preferences::onCmdStyleColor),
  FXMAPFUNCS(SEL_UPDATE,Preferences::ID_STYLE_UNDERLINE,Preferences::ID_STYLE_STRIKEOUT,Preferences::onUpdStyleFlags),
  FXMAPFUNCS(SEL_COMMAND,Preferences::ID_STYLE_UNDERLINE,Preferences::ID_STYLE_STRIKEOUT,Preferences::onCmdStyleFlags),
  FXMAPFUNC(SEL_UPDATE,Preferences::ID_STYLE_DELETE,Preferences::onUpdStyleDelete),
  FXMAPFUNC(SEL_COMMAND,Preferences::ID_STYLE_DELETE,Preferences::onCmdStyleDelete),
  FXMAPFUNC(SEL_UPDATE,Preferences::ID_STYLE_NEW,Preferences::onUpdStyleNew),
  FXMAPFUNC(SEL_COMMAND,Preferences::ID_STYLE_NEW,Preferences::onCmdStyleNew),

  FXMAPFUNC(SEL_COMMAND,Preferences::ID_LANG_INDEX,Preferences::onCmdLangIndex),
  FXMAPFUNC(SEL_COMMAND,Preferences::ID_LANG_NEW,Preferences::onCmdLangNew),
  FXMAPFUNC(SEL_COMMAND,Preferences::ID_LANG_DELETE,Preferences::onCmdLangDelete),
  FXMAPFUNC(SEL_UPDATE,Preferences::ID_LANG_DELETE,Preferences::onUpdLangSelected),
  FXMAPFUNC(SEL_COMMAND,Preferences::ID_LANG_CHANGED,Preferences::onCmdLangChanged),
  FXMAPFUNC(SEL_UPDATE,Preferences::ID_LANG_CHANGED,Preferences::onUpdLangSelected),

  FXMAPFUNC(SEL_COMMAND,Preferences::ID_SYNTAX_LANG,Preferences::onCmdSyntaxLang),
  FXMAPFUNC(SEL_UPDATE,Preferences::ID_SYNTAX_INDEX,Preferences::onUpdSyntaxChanged),
  FXMAPFUNC(SEL_CHANGED,Preferences::ID_SYNTAX_INDEX,Preferences::onCmdSyntaxIndex),
  FXMAPFUNC(SEL_COMMAND,Preferences::ID_SYNTAX_NEW,Preferences::onCmdSyntaxNew),
  FXMAPFUNC(SEL_UPDATE,Preferences::ID_SYNTAX_NEW,Preferences::onUpdSyntaxNew),
  FXMAPFUNC(SEL_COMMAND,Preferences::ID_SYNTAX_DELETE,Preferences::onCmdSyntaxDelete),
  FXMAPFUNC(SEL_UPDATE,Preferences::ID_SYNTAX_DELETE,Preferences::onUpdSyntaxChanged),
  FXMAPFUNC(SEL_COMMAND,Preferences::ID_SYNTAX_CHANGED,Preferences::onCmdSyntaxChanged),
  FXMAPFUNC(SEL_UPDATE,Preferences::ID_SYNTAX_CHANGED,Preferences::onUpdSyntaxChanged),
  FXMAPFUNC(SEL_CHANGED,Preferences::ID_SYNTAX_PATTERN,Preferences::onCmdSyntaxChanged),
  FXMAPFUNC(SEL_UPDATE,Preferences::ID_SYNTAX_PATTERN,Preferences::onUpdSyntaxChanged),
  };


// Object implementation
FXIMPLEMENT(Preferences,FXDialogBox,PreferencesMap,ARRAYNUMBER(PreferencesMap))



// Construct
Preferences::Preferences(TextWindow *owner):
  FXDialogBox(owner,"Adie Preferences",DECOR_TITLE|DECOR_BORDER|DECOR_RESIZE,0,0,0,0, 0,0,0,0, 4,4){
  FXchar key[10],language[200];
  register FXint i;

  FXVerticalFrame *vertical=new FXVerticalFrame(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXHorizontalFrame *horizontal=new FXHorizontalFrame(vertical,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  FXVerticalFrame *buttons=new FXVerticalFrame(horizontal,LAYOUT_LEFT|LAYOUT_FILL_Y|FRAME_SUNKEN|PACK_UNIFORM_WIDTH|PACK_UNIFORM_HEIGHT,0,0,0,0, 0,0,0,0, 0,0);
  FXSwitcher *switcher=new FXSwitcher(horizontal,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);

  // Icons
  pal=new FXGIFIcon(getApp(),palette_gif);
  ind=new FXGIFIcon(getApp(),indent_gif);
  pat=new FXGIFIcon(getApp(),pattern_gif);
  sty=new FXGIFIcon(getApp(),styles_gif);
  syn=new FXGIFIcon(getApp(),syntax_gif);
  lng=new FXGIFIcon(getApp(),lang_gif);

  /////////////////////////  Color settings pane  ///////////////////////////////
  FXVerticalFrame* colorspane=new FXVerticalFrame(switcher,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0, 0,0);
  new FXLabel(colorspane,"Color settings",NULL,LAYOUT_LEFT);
  new FXHorizontalSeparator(colorspane,SEPARATOR_LINE|LAYOUT_FILL_X);
  FXMatrix *matrix1=new FXMatrix(colorspane,8,MATRIX_BY_ROWS|PACK_UNIFORM_HEIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 4,4,4,4, 4, 4);

  new FXLabel(matrix1,"Background:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,"Text:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,"Sel. text background:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,"Sel. text:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,"Hilite text background:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,"Hilite text:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,"Act. text background:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,"Numbers background:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);

  new FXColorWell(matrix1,FXRGB(0,0,0),owner,TextWindow::ID_TEXT_BACK,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(matrix1,FXRGB(0,0,0),owner,TextWindow::ID_TEXT_FORE,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(matrix1,FXRGB(0,0,0),owner,TextWindow::ID_TEXT_SELBACK,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(matrix1,FXRGB(0,0,0),owner,TextWindow::ID_TEXT_SELFORE,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(matrix1,FXRGB(0,0,0),owner,TextWindow::ID_TEXT_HILITEBACK,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(matrix1,FXRGB(0,0,0),owner,TextWindow::ID_TEXT_HILITEFORE,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(matrix1,FXRGB(0,0,0),owner,TextWindow::ID_TEXT_ACTIVEBACK,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(matrix1,FXRGB(0,0,0),owner,TextWindow::ID_TEXT_NUMBACK,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);

  new FXFrame(matrix1,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix1,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix1,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix1,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix1,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix1,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix1,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix1,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);

  new FXLabel(matrix1,"Files background:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,"Files:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,"Sel. files background:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,"Sel. files:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,"Lines:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,"Cursor:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,"Active background:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix1,"Numbers:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);

  new FXColorWell(matrix1,FXRGB(0,0,0),owner,TextWindow::ID_DIR_BACK,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(matrix1,FXRGB(0,0,0),owner,TextWindow::ID_DIR_FORE,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(matrix1,FXRGB(0,0,0),owner,TextWindow::ID_DIR_SELBACK,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(matrix1,FXRGB(0,0,0),owner,TextWindow::ID_DIR_SELFORE,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(matrix1,FXRGB(0,0,0),owner,TextWindow::ID_DIR_LINES,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(matrix1,FXRGB(0,0,0),owner,TextWindow::ID_TEXT_CURSOR,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXCheckButton(matrix1,NULL,owner,TextWindow::ID_SHOWACTIVE,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 5,5,0,0);
  new FXColorWell(matrix1,FXRGB(0,0,0),owner,TextWindow::ID_TEXT_NUMFORE,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);

  //// Color settings button
  new FXButton(buttons,"Colors\tChange Colors\tChange text colors.",pal,switcher,FXSwitcher::ID_OPEN_FIRST,FRAME_RAISED|ICON_ABOVE_TEXT|LAYOUT_FILL_Y);


  ///////////////////////////  Editor settings pane  ////////////////////////////
  FXVerticalFrame* editorpane=new FXVerticalFrame(switcher,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0, 0,0);
  new FXLabel(editorpane,"Editor settings",NULL,LAYOUT_LEFT);
  new FXHorizontalSeparator(editorpane,SEPARATOR_LINE|LAYOUT_FILL_X);
  FXMatrix *matrix2=new FXMatrix(editorpane,7,MATRIX_BY_ROWS|PACK_UNIFORM_HEIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 4,4,4,4, 4, 4);

  new FXLabel(matrix2,"Word wrapping:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,"Auto indent:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,"Fixed wrap margin:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,"Strip carriage returns:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,"Strip trailing spaces:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,"Insert tab characters:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,"Warn if changed externally:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);

  new FXCheckButton(matrix2,NULL,owner,TextWindow::ID_TOGGLE_WRAP,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
  new FXCheckButton(matrix2,NULL,owner,TextWindow::ID_AUTOINDENT,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
  new FXCheckButton(matrix2,NULL,owner,TextWindow::ID_FIXED_WRAP,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
  new FXCheckButton(matrix2,NULL,owner,TextWindow::ID_STRIP_CR,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
  new FXCheckButton(matrix2,NULL,owner,TextWindow::ID_STRIP_SP,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
  new FXCheckButton(matrix2,NULL,owner,TextWindow::ID_INSERTTABS,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
  new FXCheckButton(matrix2,NULL,owner,TextWindow::ID_WARNCHANGED,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);

  new FXFrame(matrix2,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix2,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix2,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix2,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix2,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix2,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);
  new FXFrame(matrix2,LAYOUT_FILL_COLUMN|LAYOUT_FILL_ROW);

  new FXLabel(matrix2,"Wrap margin:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,"Tab columns:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,"Brace match time (ms):",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,"Mouse wheel lines:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,"Line number space:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,"Save view of file:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);
  new FXLabel(matrix2,"Save bookmarks:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW);

  new FXTextField(matrix2,6,owner,TextWindow::ID_WRAPCOLUMNS,JUSTIFY_RIGHT|FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW,0,0,0,0, 2,2,1,1);
  new FXTextField(matrix2,6,owner,TextWindow::ID_TABCOLUMNS,JUSTIFY_RIGHT|FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW,0,0,0,0, 2,2,1,1);
  new FXTextField(matrix2,6,owner,TextWindow::ID_BRACEMATCH,JUSTIFY_RIGHT|FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW,0,0,0,0, 2,2,1,1);
  FXSpinner* spinner=new FXSpinner(matrix2,3,owner,TextWindow::ID_WHEELADJUST,JUSTIFY_RIGHT|FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW,0,0,0,0, 2,2,1,1);
  spinner->setRange(1,100);
  FXSpinner* spinner1=new FXSpinner(matrix2,3,owner,TextWindow::ID_TEXT_LINENUMS,JUSTIFY_RIGHT|FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW,0,0,0,0, 2,2,1,1);
  spinner1->setRange(0,8);
  new FXCheckButton(matrix2,NULL,owner,TextWindow::ID_SAVEVIEWS,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
  new FXCheckButton(matrix2,NULL,owner,TextWindow::ID_SAVEMARKS,LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
  FXHorizontalFrame *worddelims=new FXHorizontalFrame(editorpane,LAYOUT_FILL_Y|LAYOUT_FILL_X,0,0,0,0, 4,4,4,4, 4, 4);
  new FXLabel(worddelims,"Word delimiters:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y);
  new FXTextField(worddelims,10,owner,TextWindow::ID_DELIMITERS,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X,0,0,0,0, 2,2,1,1);

  //// Editor settings button
  new FXButton(buttons,"Editor\tEditor settings\tChange editor settings and other things.",ind,switcher,FXSwitcher::ID_OPEN_SECOND,FRAME_RAISED|ICON_ABOVE_TEXT|LAYOUT_FILL_Y);


  ///////////////////////  File pattern settings pane  //////////////////////////
  FXVerticalFrame* filepatpane=new FXVerticalFrame(switcher,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0, 0,0);
  new FXLabel(filepatpane,"Filename patterns",NULL,LAYOUT_LEFT);
  new FXHorizontalSeparator(filepatpane,SEPARATOR_LINE|LAYOUT_FILL_X);
  FXVerticalFrame *sub3=new FXVerticalFrame(filepatpane,LAYOUT_FILL_Y|LAYOUT_FILL_X,0,0,0,0, 0,0,10,0, 0,0);
  new FXLabel(sub3,"Filename patterns, one per line:",NULL,JUSTIFY_LEFT);
  FXVerticalFrame* textwell=new FXVerticalFrame(sub3,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
  filepattext=new FXText(textwell,NULL,0,LAYOUT_FILL_X|LAYOUT_FILL_Y);

  //// File pattern settings button
  new FXButton(buttons,"Patterns\tFilename patterns\tChange wildcard patterns for filenames.",pat,switcher,FXSwitcher::ID_OPEN_THIRD,FRAME_RAISED|ICON_ABOVE_TEXT|LAYOUT_FILL_Y);


  /////////////////////  Highlight style settings pane  /////////////////////////
  FXVerticalFrame* highlightpane=new FXVerticalFrame(switcher,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0, 0,0);
  new FXLabel(highlightpane,"Highlight styles",NULL,LAYOUT_LEFT);
  new FXHorizontalSeparator(highlightpane,SEPARATOR_LINE|LAYOUT_FILL_X);
  FXHorizontalFrame *sub5=new FXHorizontalFrame(highlightpane,LAYOUT_FILL_Y|LAYOUT_FILL_X,0,0,0,0, 0,0,10,0);

  FXGroupBox* colorgroup=new FXGroupBox(sub5,"Style of item",FRAME_GROOVE|LAYOUT_RIGHT|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 4,4,4,4, 4,4);
  FXHorizontalFrame *stylenamegroup=new FXHorizontalFrame(colorgroup,LAYOUT_SIDE_TOP|LAYOUT_FILL_X,0,0,0,0, 0,0,0,0, 4, 4);
  new FXLabel(stylenamegroup,"Style name:",NULL,JUSTIFY_LEFT);
  stylename=new FXTextField(stylenamegroup,8,this,ID_STYLE_NAME,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X,0,0,0,0, 2,2,1,1);
  FXMatrix *colormatrix=new FXMatrix(colorgroup,3,MATRIX_BY_COLUMNS|PACK_UNIFORM_HEIGHT|LAYOUT_SIDE_TOP|LAYOUT_FILL_X,0,0,0,0, 0,0,0,0, 4, 4);
  new FXLabel(colormatrix,"Normal text color fg/bg:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN);
  new FXColorWell(colormatrix,FXRGB(0,0,0),this,ID_STYLE_NORMAL_FG,FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_X|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(colormatrix,FXRGB(0,0,0),this,ID_STYLE_NORMAL_BG,FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_X|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXLabel(colormatrix,"Selected text color fg/bg:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN);
  new FXColorWell(colormatrix,FXRGB(0,0,0),this,ID_STYLE_SELECT_FG,FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_X|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(colormatrix,FXRGB(0,0,0),this,ID_STYLE_SELECT_BG,FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_X|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXLabel(colormatrix,"Highlighted text color fg/bg:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN);
  new FXColorWell(colormatrix,FXRGB(0,0,0),this,ID_STYLE_HILITE_FG,FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_X|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXColorWell(colormatrix,FXRGB(0,0,0),this,ID_STYLE_HILITE_BG,FRAME_SUNKEN|FRAME_THICK|LAYOUT_CENTER_X|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXLabel(colormatrix,"Active line background color:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN);
  new FXColorWell(colormatrix,FXRGB(0,0,0),this,ID_STYLE_ACTIVE_BG,FRAME_SUNKEN|FRAME_THICK|LAYOUT_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT|LAYOUT_FILL_ROW,0,0,40,24);
  new FXFrame(colormatrix,LAYOUT_FILL_ROW);

  new FXLabel(colormatrix,"Underline text:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN);
  new FXCheckButton(colormatrix,NULL,this,ID_STYLE_UNDERLINE,LAYOUT_CENTER_X|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
  new FXFrame(colormatrix,LAYOUT_FILL_ROW);
  new FXLabel(colormatrix,"Strikeout text:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN);
  new FXCheckButton(colormatrix,NULL,this,ID_STYLE_STRIKEOUT,LAYOUT_CENTER_X|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
  new FXFrame(colormatrix,LAYOUT_FILL_ROW);
//   new FXLabel(colormatrix,"Bold:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN);
//   new FXCheckButton(colormatrix,NULL,NULL,0,LAYOUT_CENTER_X|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
//   new FXFrame(colormatrix,LAYOUT_FILL_ROW);
//   new FXLabel(colormatrix,"Italic:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN);
//   new FXCheckButton(colormatrix,NULL,NULL,0,LAYOUT_CENTER_X|LAYOUT_CENTER_Y|LAYOUT_FILL_ROW,0,0,0,0, 0,0,0,0);
//   new FXFrame(colormatrix,LAYOUT_FILL_ROW);

  FXGroupBox* stylegroup=new FXGroupBox(sub5,"Item name",FRAME_GROOVE|LAYOUT_FILL_Y,0,0,0,0, 4,4,4,4, 4,4);
  FXHorizontalFrame *stylebuttongroup=new FXHorizontalFrame(stylegroup,LAYOUT_SIDE_BOTTOM|PACK_UNIFORM_WIDTH|LAYOUT_FILL_X,0,0,0,0, 0,0,0,0, 4,4);
  new FXButton(stylebuttongroup,"New\tNew style",NULL,this,ID_STYLE_NEW,FRAME_RAISED|LAYOUT_LEFT);
  new FXButton(stylebuttongroup,"Delete\tDelete style",NULL,this,ID_STYLE_DELETE,FRAME_RAISED|LAYOUT_RIGHT);
  FXVerticalFrame* listframe=new FXVerticalFrame(stylegroup,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);
  stylelist=new FXList(listframe,0,NULL,0,LIST_BROWSESELECT|HSCROLLER_NEVER|LAYOUT_FIX_WIDTH|LAYOUT_FILL_Y, 0,0,110,0);

  //// Highlight style settings button
  new FXButton(buttons,"Styles\tHighlight styles\tChange highlight styles for syntax coloring.",sty,switcher,FXSwitcher::ID_OPEN_FOURTH,FRAME_RAISED|ICON_ABOVE_TEXT|LAYOUT_FILL_Y);


  /////////////////////////  Language mode settings  ////////////////////////////
  FXVerticalFrame* languagemodepane=new FXVerticalFrame(switcher,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0, 0,0);
  new FXLabel(languagemodepane,"Language mode",NULL,LAYOUT_LEFT);
  new FXHorizontalSeparator(languagemodepane,SEPARATOR_LINE|LAYOUT_FILL_X);
  FXHorizontalFrame *sub7=new FXHorizontalFrame(languagemodepane,LAYOUT_FILL_Y|LAYOUT_FILL_X,0,0,0,0, 0,0,10,0);

  // List of file types (languages)
  FXGroupBox* langgroup=new FXGroupBox(sub7,"Languages",FRAME_GROOVE|LAYOUT_FILL_Y,0,0,0,0, 4,4,4,4, 4,4);
  FXHorizontalFrame *langbuttongroup=new FXHorizontalFrame(langgroup,LAYOUT_SIDE_BOTTOM|PACK_UNIFORM_WIDTH|LAYOUT_FILL_X,0,0,0,0, 0,0,0,0, 4,4);
  new FXButton(langbuttongroup,"New\tNew language",NULL,this,ID_LANG_NEW,FRAME_RAISED|LAYOUT_LEFT);
  new FXButton(langbuttongroup,"Delete\tDelete language",NULL,this,ID_LANG_DELETE,FRAME_RAISED|LAYOUT_RIGHT);
  FXVerticalFrame* listframe1=new FXVerticalFrame(langgroup,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);
  langlist=new FXList(listframe1,0,this,ID_LANG_INDEX,LIST_BROWSESELECT|HSCROLLER_NEVER|LAYOUT_FIX_WIDTH|LAYOUT_FILL_Y, 0,0,110,0);

  FXGroupBox* langdefgroup=new FXGroupBox(sub7,"Language recognition",FRAME_GROOVE|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 4,4,4,4, 4,4);
  FXHorizontalFrame *langnamegroup=new FXHorizontalFrame(langdefgroup,LAYOUT_SIDE_TOP|LAYOUT_FILL_X,0,0,0,0, 0,0,0,0, 4, 4);
  new FXLabel(langnamegroup,"Language name:",NULL,JUSTIFY_LEFT);
  langname=new FXTextField(langnamegroup,8,this,ID_LANG_CHANGED,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X,0,0,0,0, 2,2,1,1);

  FXVerticalFrame *langextgroup=new FXVerticalFrame(langdefgroup,LAYOUT_SIDE_TOP|LAYOUT_FILL_X,0,0,0,0, 0,0,0,0, 0, 0);
  new FXLabel(langextgroup,"File extensions:",NULL,JUSTIFY_LEFT|LAYOUT_FILL_X);
  langext=new FXTextField(langextgroup,8,this,ID_LANG_CHANGED,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X,0,0,0,0, 2,2,1,1);

  //// Syntax settings button
  new FXButton(buttons,"Language\tLanguage modes\tChange language modes.",lng,switcher,FXSwitcher::ID_OPEN_FIFTH,FRAME_RAISED|ICON_ABOVE_TEXT|LAYOUT_FILL_Y);


  //////////////////////  Syntax pattern settings pane  /////////////////////////
  FXVerticalFrame* syntaxpane=new FXVerticalFrame(switcher,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0, 0,0);
  new FXLabel(syntaxpane,"Syntax patterns",NULL,LAYOUT_LEFT);
  new FXHorizontalSeparator(syntaxpane,SEPARATOR_LINE|LAYOUT_FILL_X);
  FXVerticalFrame *sub8=new FXVerticalFrame(syntaxpane,LAYOUT_FILL_Y|LAYOUT_FILL_X,0,0,0,0, 0,0,10,0);

  // Select language
  FXGroupBox* languagemodegroup=new FXGroupBox(sub8,"Language mode",FRAME_GROOVE|LAYOUT_FILL_X,0,0,0,0, 4,4,0,4, 4,4);
  new FXLabel(languagemodegroup,"Patterns for language mode:",NULL,JUSTIFY_LEFT|LAYOUT_SIDE_LEFT|LAYOUT_CENTER_Y);
  langbox=new FXListBox(languagemodegroup,6,this,ID_SYNTAX_LANG,FRAME_SUNKEN|FRAME_THICK|LAYOUT_SIDE_LEFT|LAYOUT_CENTER_Y|LAYOUT_FIX_WIDTH,0,0,100,0, 2,2,1,1);

  FXHorizontalFrame *sub6=new FXHorizontalFrame(sub8,LAYOUT_FILL_Y|LAYOUT_FILL_X,0,0,0,0, 0,0,0,0);
  FXGroupBox* patgroup=new FXGroupBox(sub6,"Pattern name",FRAME_GROOVE|LAYOUT_FILL_Y,0,0,0,0, 4,4,4,4, 4,4);
  FXHorizontalFrame *patbuttongroup=new FXHorizontalFrame(patgroup,LAYOUT_SIDE_BOTTOM|PACK_UNIFORM_WIDTH|LAYOUT_FILL_X,0,0,0,0, 0,0,0,0, 4,4);
  new FXButton(patbuttongroup,"New\tNew pattern",NULL,this,ID_SYNTAX_NEW,FRAME_RAISED|LAYOUT_LEFT);
  new FXButton(patbuttongroup,"Delete\tDelete pattern",NULL,this,ID_SYNTAX_DELETE,FRAME_RAISED|LAYOUT_RIGHT);
  FXVerticalFrame* listframe2=new FXVerticalFrame(patgroup,FRAME_SUNKEN|FRAME_THICK|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);
  patternlist=new FXList(listframe2,0,this,ID_SYNTAX_INDEX,LIST_BROWSESELECT|HSCROLLER_NEVER|LAYOUT_FIX_WIDTH|LAYOUT_FILL_Y, 0,0,110,0);

  FXGroupBox* patdefgroup=new FXGroupBox(sub6,"Pattern definition",FRAME_GROOVE|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 4,4,4,4, 4,4);

  FXHorizontalFrame *patnamegroup=new FXHorizontalFrame(patdefgroup,LAYOUT_SIDE_TOP|LAYOUT_FILL_X,0,0,0,0, 0,0,0,0, 4, 4);
  new FXLabel(patnamegroup,"Pattern name:",NULL,JUSTIFY_LEFT);
  patternname=new FXTextField(patnamegroup,8,this,ID_SYNTAX_CHANGED,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X,0,0,0,0, 2,2,1,1);

  FXMatrix *highlightmodegroup=new FXMatrix(patdefgroup,2,MATRIX_BY_ROWS|LAYOUT_SIDE_TOP|LAYOUT_FILL_X,0,0,0,0, 0,0,0,0, 8,0);
  new FXLabel(highlightmodegroup,"Parent:",NULL,JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_X);
  parentcombo=new FXComboBox(highlightmodegroup,6,6,this,ID_SYNTAX_CHANGED,COMBOBOX_STATIC|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_X,0,0,0,0, 2,2,1,1);
  new FXLabel(highlightmodegroup,"Style:",NULL,JUSTIFY_LEFT|LAYOUT_FILL_COLUMN|LAYOUT_FILL_X);
  stylecombo=new FXComboBox(highlightmodegroup,6,6,this,ID_SYNTAX_CHANGED,COMBOBOX_STATIC|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_COLUMN|LAYOUT_FILL_X,0,0,0,0, 2,2,1,1);
  new FXLabel(highlightmodegroup,"Lines:",NULL,JUSTIFY_LEFT|LAYOUT_FILL_X);
  contexttext=new FXTextField(highlightmodegroup,5,this,ID_SYNTAX_CHANGED,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X,0,0,0,0, 2,2,1,1);
  new FXLabel(highlightmodegroup,"Prio:",NULL,JUSTIFY_LEFT|LAYOUT_FILL_X);
  priospinner=new FXSpinner(highlightmodegroup,3,this,ID_SYNTAX_CHANGED,JUSTIFY_RIGHT|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 2,2,1,1);
  priospinner->setRange(0,999);

  // Three text entry boxes
  FXVerticalFrame* textboxes=new FXVerticalFrame(patdefgroup,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0, 0,0);

  new FXLabel(textboxes,"Regular expression:",NULL,JUSTIFY_LEFT|JUSTIFY_BOTTOM|LAYOUT_FILL_X);
  FXVerticalFrame *patternframe=new FXVerticalFrame(textboxes,LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
  patterntext=new FXText(patternframe,this,ID_SYNTAX_PATTERN,TEXT_WORDWRAP|HSCROLLER_NEVER|VSCROLLER_NEVER|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  patterntext->setVisRows(1);
  patterntext->setVisCols(40);


  //// Syntax settings button
  new FXButton(buttons,"Syntax\tSyntax rules\tChange syntax patterns.",syn,switcher,FXSwitcher::ID_OPEN_SIXTH,FRAME_RAISED|ICON_ABOVE_TEXT|LAYOUT_FILL_Y);


  // Bottom part
  new FXHorizontalSeparator(vertical,SEPARATOR_RIDGE|LAYOUT_FILL_X);
  FXHorizontalFrame *closebox=new FXHorizontalFrame(vertical,LAYOUT_BOTTOM|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH);
  new FXButton(closebox,"&Accept",NULL,this,FXDialogBox::ID_ACCEPT,BUTTON_INITIAL|BUTTON_DEFAULT|LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK,0,0,0,0, 20,20);
  new FXButton(closebox,"&Cancel",NULL,this,FXDialogBox::ID_CANCEL,BUTTON_DEFAULT|LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK,0,0,0,0, 20,20);

  // Fill styles
  stylecombo->appendItem("None");
  for(i=0; i<owner->getNumStyles(); i++){
    stylelist->appendItem(owner->getStyleName(i));
    stylecombo->appendItem(owner->getStyleName(i));
    }

  // Fill languages
  for(i=0; i<1000; i++){
    sprintf(key,"%d",i+1);
    if(getApp()->reg().readFormatEntry("LANGUAGES",key,"%[^,]",language)!=1) break;
    langlist->appendItem(language);
    langbox->appendItem(language);
    }
  onCmdSyntaxLang(NULL,0,NULL);
  onCmdLangIndex(NULL,0,NULL);
  }


/*******************************************************************************/


// Style name
long Preferences::onCmdStyleName(FXObject*,FXSelector,void*){
  FXint curstyle=stylelist->getCurrentItem();
  FXString name=stylename->getText();
  getOwner()->setStyleName(curstyle,name);
  stylelist->setItemText(curstyle,name);
  stylecombo->setItemText(curstyle+1,name);
  return 1;
  }


// Update style name
long Preferences::onUpdStyleName(FXObject* sender,FXSelector,void*){
  FXint curstyle=stylelist->getCurrentItem();
  if(0<=curstyle){
    FXString name=getOwner()->getStyleName(curstyle);
    sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&name);
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
    }
  else{
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
    }
  return 1;
  }


// Change style flags
long Preferences::onCmdStyleFlags(FXObject*,FXSelector sel,void*){
  FXint curstyle=stylelist->getCurrentItem();
  FXHiliteStyle style=getOwner()->getStyleColors(curstyle);
  switch(SELID(sel)){
    case ID_STYLE_UNDERLINE: style.style^=FXText::STYLE_UNDERLINE; break;
    case ID_STYLE_STRIKEOUT: style.style^=FXText::STYLE_STRIKEOUT; break;
    }
  getOwner()->setStyleColors(curstyle,style);
  return 1;
  }


// Update style flags
long Preferences::onUpdStyleFlags(FXObject* sender,FXSelector sel,void*){
  FXint curstyle=stylelist->getCurrentItem();
  if(0<=curstyle){
    FXHiliteStyle style=getOwner()->getStyleColors(curstyle);
    FXuint bit=0;
    switch(SELID(sel)){
      case ID_STYLE_UNDERLINE: bit=style.style&FXText::STYLE_UNDERLINE; break;
      case ID_STYLE_STRIKEOUT: bit=style.style&FXText::STYLE_STRIKEOUT; break;
      }
    sender->handle(this,bit?MKUINT(ID_CHECK,SEL_COMMAND):MKUINT(ID_UNCHECK,SEL_COMMAND),NULL);
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
    }
  else{
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
    }
  return 1;
  }


// Change style color
long Preferences::onCmdStyleColor(FXObject*,FXSelector sel,void* ptr){
  FXint curstyle=stylelist->getCurrentItem();
  FXHiliteStyle style=getOwner()->getStyleColors(curstyle);
  switch(SELID(sel)){
    case ID_STYLE_NORMAL_FG: style.normalForeColor=(FXColor)(FXuval)ptr; break;
    case ID_STYLE_NORMAL_BG: style.normalBackColor=(FXColor)(FXuval)ptr; break;
    case ID_STYLE_SELECT_FG: style.selectForeColor=(FXColor)(FXuval)ptr; break;
    case ID_STYLE_SELECT_BG: style.selectBackColor=(FXColor)(FXuval)ptr; break;
    case ID_STYLE_HILITE_FG: style.hiliteForeColor=(FXColor)(FXuval)ptr; break;
    case ID_STYLE_HILITE_BG: style.hiliteBackColor=(FXColor)(FXuval)ptr; break;
    case ID_STYLE_ACTIVE_BG: style.activeBackColor=(FXColor)(FXuval)ptr; break;
    }
  getOwner()->setStyleColors(curstyle,style);
  return 1;
  }


// Update style color
long Preferences::onUpdStyleColor(FXObject* sender,FXSelector sel,void*){
  FXint curstyle=stylelist->getCurrentItem();
  if(0<=curstyle){
    FXHiliteStyle style=getOwner()->getStyleColors(curstyle);
    FXColor color=0;
    switch(SELID(sel)){
      case ID_STYLE_NORMAL_FG: color=style.normalForeColor; break;
      case ID_STYLE_NORMAL_BG: color=style.normalBackColor; break;
      case ID_STYLE_SELECT_FG: color=style.selectForeColor; break;
      case ID_STYLE_SELECT_BG: color=style.selectBackColor; break;
      case ID_STYLE_HILITE_FG: color=style.hiliteForeColor; break;
      case ID_STYLE_HILITE_BG: color=style.hiliteBackColor; break;
      case ID_STYLE_ACTIVE_BG: color=style.activeBackColor; break;
      }
    sender->handle(this,MKUINT(ID_SETINTVALUE,SEL_COMMAND),(void*)&color);
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
    }
  else{
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
    }
  return 1;
  }



// New style
long Preferences::onCmdStyleNew(FXObject*,FXSelector,void*){
  FXHiliteStyle style;
  style.normalForeColor=FXRGB(0,0,0);
  style.normalBackColor=0;
  style.selectForeColor=0;
  style.selectBackColor=0;
  style.hiliteForeColor=0;
  style.hiliteBackColor=0;
  style.activeBackColor=0;
  style.style=0;
  getOwner()->appendStyle("New",style);
  stylelist->appendItem("New");
  stylecombo->appendItem("New");
  stylelist->setCurrentItem(stylelist->getNumItems()-1);
  return 1;
  }


// Update new style
long Preferences::onUpdStyleNew(FXObject* sender,FXSelector,void*){
  FXint nstyles=getOwner()->getNumStyles();
  sender->handle(this,(nstyles<MAXSTYLES)?MKUINT(ID_ENABLE,SEL_COMMAND):MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Delete style
long Preferences::onCmdStyleDelete(FXObject*,FXSelector,void*){
  FXint curstyle=stylelist->getCurrentItem();
  getOwner()->deleteStyle(curstyle);
  stylelist->removeItem(curstyle);
  stylecombo->removeItem(curstyle);
  return 1;
  }


// Update delete style
long Preferences::onUpdStyleDelete(FXObject* sender,FXSelector,void*){
  FXint curstyle=stylelist->getCurrentItem();
  sender->handle(this,(0<=curstyle)?MKUINT(ID_ENABLE,SEL_COMMAND):MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }



/*******************************************************************************/


// Selected language from language list
long Preferences::onCmdLangIndex(FXObject*,FXSelector,void*){
  FXchar key[10],name[200],extensions[2000];
  FXint index=langlist->getCurrentItem();
  sprintf(key,"%d",index+1);
  if(getApp()->reg().readFormatEntry("LANGUAGES",key,"%[^,],%[ -~]",name,extensions)==2){
    langname->setText(name);
    langext->setText(extensions);
    }
  else{
    langname->setText(FXString::null);
    langext->setText(FXString::null);
    }
  return 1;
  }


// Changed language name or extension
long Preferences::onCmdLangChanged(FXObject*,FXSelector,void*){
  FXint    index=langlist->getCurrentItem();
  FXString name=langname->getText();
  FXString extensions=langext->getText();
  FXchar   key[10];
  langlist->setItemText(index,name);
  langbox->setItemText(index,name);
  sprintf(key,"%d",index+1);
  getApp()->reg().writeFormatEntry("LANGUAGES",key,"%s,%s",name.text(),extensions.text());
  return 1;
  }


// New language name
long Preferences::onCmdLangNew(FXObject*,FXSelector,void*){
  FXchar okey[10],nkey[10],name[200],extensions[2000];
  FXint index=langlist->getCurrentItem()+1;
  for(FXint i=langlist->getNumItems()-1; index<=i; i--){
    sprintf(okey,"%d",i+1);
    sprintf(nkey,"%d",i+2);
    getApp()->reg().readFormatEntry("LANGUAGES",okey,"%[^,],%[ -~]",name,extensions);
    getApp()->reg().writeFormatEntry("LANGUAGES",nkey,"%s,%s",name,extensions);
    }
  sprintf(nkey,"%d",index+1);
  getApp()->reg().writeFormatEntry("LANGUAGES",nkey,"%s,%s","New","*");
  langlist->insertItem(index,"New");
  langlist->setCurrentItem(index);
  langbox->insertItem(index,"New");
  onCmdSyntaxLang(NULL,0,NULL);
  onCmdLangIndex(NULL,0,NULL);
  return 1;
  }


// Delete language name
long Preferences::onCmdLangDelete(FXObject*,FXSelector,void*){
  FXchar okey[10],nkey[10],name[200],extensions[2000];
  FXint index=langlist->getCurrentItem();
  for(FXint i=index; i<langlist->getNumItems()-1; i++){
    sprintf(okey,"%d",i+1);
    sprintf(nkey,"%d",i+2);
    getApp()->reg().readFormatEntry("LANGUAGES",nkey,"%[^,],%[ -~]",name,extensions);
    getApp()->reg().writeFormatEntry("LANGUAGES",okey,"%s,%s",name,extensions);
    }
  sprintf(nkey,"%d",langlist->getNumItems());
  getApp()->reg().deleteEntry("LANGUAGES",nkey);
  langlist->removeItem(index);
  langbox->removeItem(index);
  onCmdSyntaxLang(NULL,0,NULL);
  onCmdLangIndex(NULL,0,NULL);
  return 1;
  }


// Is a language selected
long Preferences::onUpdLangSelected(FXObject* sender,FXSelector,void*){
  if(0<=langlist->getCurrentItem())
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


/*******************************************************************************/


// Switch language
long Preferences::onCmdSyntaxLang(FXObject*,FXSelector,void*){
  FXchar key[10],name[200],pattern[2000];
  FXint style,priority,parent,context,i;
  FXString language;
  FXint index=langbox->getCurrentItem();
  patternlist->clearItems();
  if(index<0) return 1;
  language=langbox->getItemText(index);
  for(i=1; i<1000; i++){
    sprintf(key,"%d",i);
    if(getApp()->reg().readFormatEntry(language.text(),key,"%[^,],%d,%d,%d,%d,%[ -~]",name,&style,&priority,&parent,&context,pattern)){
      patternlist->appendItem(name);
      }
    }
  onCmdSyntaxIndex(NULL,0,NULL);
  return 1;
  }


// Switch syntax pattern
long Preferences::onCmdSyntaxIndex(FXObject*,FXSelector,void*){
  FXString language=langbox->getItemText(langbox->getCurrentItem());
  FXint index=patternlist->getCurrentItem();
  FXchar key[10],name[200],pattern[2000];
  FXint style,priority,parent,context,i;
  sprintf(key,"%d",index+1);
  name[0]=pattern[0]='\0';
  if(getApp()->reg().readFormatEntry(language.text(),key,"%[^,],%d,%d,%d,%d,%[ -~]",name,&style,&priority,&parent,&context,pattern)){
    FXTRACE((1,"name=\"%s\" sty=%d pri=%d par=%d ctx=%d rex=\"%s\"\n",name,style,priority,parent,context,pattern));
    contexttext->setText(FXStringVal(context));
    priospinner->setValue(priority);
    patternname->setText(name);
    parentcombo->clearItems();
    parentcombo->appendItem("None");
    for(i=0; i<index; i++){
      parentcombo->appendItem(patternlist->getItemText(i));
      }
    parentcombo->setCurrentItem(FXMIN(parent,parentcombo->getNumItems()-1));
    stylecombo->setCurrentItem(FXMIN(style,stylecombo->getNumItems()-1));
    patterntext->setText(pattern);
    }
  else{
    contexttext->setText(0);
    priospinner->setValue(0);
    patternname->setText(FXString::null);
    parentcombo->clearItems();
    parentcombo->appendItem("None");
    stylecombo->setCurrentItem(0);
    patterntext->setText(FXString::null);
    }
  return 1;
  }


// Insert pattern
long Preferences::onCmdSyntaxNew(FXObject*,FXSelector,void*){
  FXString language=langbox->getItemText(langbox->getCurrentItem());
  FXint index=patternlist->getCurrentItem()+1;
  FXchar okey[10],nkey[10],name[200],pattern[2000];
  FXint style,priority,parent,context;
  for(FXint i=patternlist->getNumItems()-1; index<=i; i--){
    sprintf(okey,"%d",i+1);
    sprintf(nkey,"%d",i+2);
    getApp()->reg().readFormatEntry(language.text(),okey,"%[^,],%d,%d,%d,%d,%[ -~]",name,&style,&priority,&parent,&context,pattern);
    if(parent>index) parent++;
    getApp()->reg().writeFormatEntry(language.text(),nkey,"%s,%d,%d,%d,%d,%s",name,style,priority,parent,context,pattern);
    }
  sprintf(nkey,"%d",index+1);
  getApp()->reg().writeFormatEntry(language.text(),nkey,"%s,%d,%d,%d,%d,%s","New",0,0,0,0,"pattern");
  patternlist->insertItem(index,"New");
  patternlist->setCurrentItem(index);
  onCmdSyntaxIndex(NULL,0,NULL);
  return 1;
  }


// Something about pattern changed
long Preferences::onUpdSyntaxNew(FXObject* sender,FXSelector,void*){
  if(0<=langbox->getCurrentItem())
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Delete pattern
long Preferences::onCmdSyntaxDelete(FXObject*,FXSelector,void*){
  FXString language=langbox->getItemText(langbox->getCurrentItem());
  FXint index=patternlist->getCurrentItem();
  FXchar okey[10],nkey[10],name[200],pattern[2000];
  FXint style,priority,parent,context;
  for(FXint i=index; i<patternlist->getNumItems()-1; i++){
    sprintf(okey,"%d",i+1);
    sprintf(nkey,"%d",i+2);
    getApp()->reg().readFormatEntry(language.text(),nkey,"%[^,],%d,%d,%d,%d,%[ -~]",name,&style,&priority,&parent,&context,pattern);
    if(parent-1>index) parent--; else if(parent-1==index) parent=0;
    getApp()->reg().writeFormatEntry(language.text(),okey,"%s,%d,%d,%d,%d,%s",name,style,priority,parent,context,pattern);
    }
  sprintf(nkey,"%d",patternlist->getNumItems());
  getApp()->reg().deleteEntry(language.text(),nkey);
  patternlist->removeItem(index,TRUE);
  onCmdSyntaxIndex(NULL,0,NULL);
  return 1;
  }


// Something about pattern changed
long Preferences::onCmdSyntaxChanged(FXObject*,FXSelector,void*){
  FXString language=langbox->getItemText(langbox->getCurrentItem());
  FXint index=patternlist->getCurrentItem();
  FXString name=patternname->getText();
  FXString pattern=patterntext->getText();
  FXint priority=priospinner->getValue();
  FXint context=FXIntVal(contexttext->getText());
  FXint style=stylecombo->getCurrentItem();
  FXint parent=parentcombo->getCurrentItem();
  FXchar key[10];
  patternlist->setItemText(index,name);
  sprintf(key,"%d",index+1);
  getApp()->reg().writeFormatEntry(language.text(),key,"%s,%d,%d,%d,%d,%s",name.text(),style,priority,parent,context,pattern.text());
  return 1;
  }


// Something about pattern changed
long Preferences::onUpdSyntaxChanged(FXObject* sender,FXSelector,void*){
  if(0<=patternlist->getCurrentItem() && 0<=langbox->getCurrentItem())
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


/*******************************************************************************/


// Change patterns, each pattern separated by newline
void Preferences::setPatterns(const FXString& patterns){
  filepattext->setText(patterns);
  }


// Return list of patterns
FXString Preferences::getPatterns() const {
  return filepattext->getText();
  }


// Clean up
Preferences::~Preferences(){
  delete pal;
  delete ind;
  delete pat;
  delete sty;
  delete syn;
  delete lng;
  }
