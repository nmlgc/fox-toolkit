/********************************************************************************
*                                                                               *
*                    M u l t i - L i ne   T e x t   O b j e c t                 *
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
* $Id: FXText.cpp,v 1.214.4.7 2003/09/13 03:37:02 fox Exp $                         *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxkeys.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXRex.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXObject.h"
#include "FXCharset.h"
#include "FXRegistry.h"
#include "FXAccelTable.h"
#include "FXApp.h"
#include "FXDCWindow.h"
#include "FXFont.h"
#include "FXGIFIcon.h"
#include "FXScrollbar.h"
#include "FXText.h"
#include "FXInputDialog.h"
#include "FXReplaceDialog.h"
#include "FXSearchDialog.h"



/*
  Notes:
  - Line start array is one longer than number of visible lines.
  - We want both tab translated to spaces as well as tab-stops array.
  - Control characters in the buffer are OK (e.g. ^L)
  - Drag cursor should be same as normal one until drag starts!
  - Change of cursor only implies makePositionVisible() if done by user.
  - Breaking:
    Soft-hyphen     173  \xAD
    No break space  240  \xF0
  - Buffer layout:

    Content  :  A  B  C  .  .  .  .  .  .  .  .  D  E  F  G
    Position :  0  1  2 			 3  4  5  6    length=7
    Addresss :  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14    buffersize=7+11-3=15
             		 ^			 ^
	     		 |			 |
	     	      gapstart=3	       gapend=11       gaplen=11-3=8

    The gap is moved around the buffer so newly added text
    can be entered into the gap; when the gap becomes too small,
    the buffer is resized.  This gapped-buffer technique minimizes
    the number of resizes of the buffer, and minimizes the number
    of block moves.

    The tail end of the visrows array will look like:

    visrow[0]= 0: "Q R S T U V W \n"
    visrow[1]= 8: "X Y Z"
    visrow[2]=11: <no text>
    visrow[3]=11: <no text>            length = 11

    The last legal position is length = 11.

  - While resizing window, keep track of a position which should remain visible,
    i.e. toppos=rowStart(position).  The position is changed same as toppos, except during
    resize.
  - When changing text, if we're looking at the tail end of the buffer, avoid jumping
    the top lines when the content hight shrinks.
  - Add undo capability. (First undo will turn mod flag back off).
  - Add incremental search, search/replace, selection search.
  - Style table stuff.
  - Need to allow for one single routine to update style buffer same as text buffer
  - Suggested additional bindings:
    Ctl-F   Find
    Ctl-R   Replace
    Ctl-G   Find again (Shift-Ctl-G Find again backward)
    Ctl-T   Replace again (Shift-Ctl-G Replace again backward)
    Ctl-L   Goto line number
    Ctl-E   Goto selected
    Ctl-I   Indent (shift 1 character right)
    Ctl-U   Unindent (shift 1 character left)
    Ctl-Z   undo
    Ctl-Y   redo
    Ctl-M   Goto matching (Shift-Ctl-M backward)
    Insert  toggle overstrike mode
    Brace matching
  - Maybe put all keyboard bindings into accelerator table.
  - Variable cursorcol should take tabcolumns into account.
  - Italic fonts are bit problematic on border between selected/unselected text
    due to kerning.
  - Tab should work as tabcolumns columns when computing a column.
  - Need rectangular selection capability.
  - Perhaps split off buffer management into separate text buffer class (allows for multiple views).
  - Need to implement regex search/replace.
  - Add better support for subclassing (syntax coloring e.g.).
  - Add support for line numbers.
  - Improve book keeping based on line/column numbers, not rows/characters.
  - If there is a style table, the style buffer is used as index into the style table,
    allowing for up to 255 styles (style index==0 is the default style).
    The style member in the FXHiliteStyle struct is used for underlining, strikeouts,
    and other effects.
    If there is NO style table but there is a style buffer, the style buffer can still
    be used for underlining, strikeouts, and other effects.
  - Sending SEL_CHANGED is pretty useless; should only be sent AFTER text change,
    and void* should contain some sensible info.
*/


#define MINSIZE   80                  // Minimum gap size
#define NVISROWS  20                  // Initial visible rows

#define TEXT_MASK   (TEXT_FIXEDWRAP|TEXT_WORDWRAP|TEXT_OVERSTRIKE|TEXT_READONLY|TEXT_NO_TABS|TEXT_AUTOINDENT|TEXT_SHOWACTIVE)

/*******************************************************************************/



// Map
FXDEFMAP(FXText) FXTextMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXText::onPaint),
  FXMAPFUNC(SEL_UPDATE,0,FXText::onUpdate),
  FXMAPFUNC(SEL_MOTION,0,FXText::onMotion),
  FXMAPFUNC(SEL_DRAGGED,0,FXText::onDragged),
  FXMAPFUNC(SEL_TIMEOUT,FXText::ID_BLINK,FXText::onBlink),
  FXMAPFUNC(SEL_TIMEOUT,FXWindow::ID_AUTOSCROLL,FXText::onAutoScroll),
  FXMAPFUNC(SEL_TIMEOUT,FXText::ID_FLASH,FXText::onFlash),
  FXMAPFUNC(SEL_FOCUSIN,0,FXText::onFocusIn),
  FXMAPFUNC(SEL_FOCUSOUT,0,FXText::onFocusOut),
  FXMAPFUNC(SEL_BEGINDRAG,0,FXText::onBeginDrag),
  FXMAPFUNC(SEL_ENDDRAG,0,FXText::onEndDrag),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXText::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXText::onLeftBtnRelease),
  FXMAPFUNC(SEL_MIDDLEBUTTONPRESS,0,FXText::onMiddleBtnPress),
  FXMAPFUNC(SEL_MIDDLEBUTTONRELEASE,0,FXText::onMiddleBtnRelease),
  FXMAPFUNC(SEL_RIGHTBUTTONPRESS,0,FXText::onRightBtnPress),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,0,FXText::onRightBtnRelease),
  FXMAPFUNC(SEL_UNGRABBED,0,FXText::onUngrabbed),
  FXMAPFUNC(SEL_DND_ENTER,0,FXText::onDNDEnter),
  FXMAPFUNC(SEL_DND_LEAVE,0,FXText::onDNDLeave),
  FXMAPFUNC(SEL_DND_DROP,0,FXText::onDNDDrop),
  FXMAPFUNC(SEL_DND_MOTION,0,FXText::onDNDMotion),
  FXMAPFUNC(SEL_DND_REQUEST,0,FXText::onDNDRequest),
  FXMAPFUNC(SEL_SELECTION_LOST,0,FXText::onSelectionLost),
  FXMAPFUNC(SEL_SELECTION_GAINED,0,FXText::onSelectionGained),
  FXMAPFUNC(SEL_SELECTION_REQUEST,0,FXText::onSelectionRequest),
  FXMAPFUNC(SEL_CLIPBOARD_LOST,0,FXText::onClipboardLost),
  FXMAPFUNC(SEL_CLIPBOARD_GAINED,0,FXText::onClipboardGained),
  FXMAPFUNC(SEL_CLIPBOARD_REQUEST,0,FXText::onClipboardRequest),
  FXMAPFUNC(SEL_KEYPRESS,0,FXText::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXText::onKeyRelease),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_HELP,FXText::onQueryHelp),
  FXMAPFUNC(SEL_UPDATE,FXWindow::ID_QUERY_TIP,FXText::onQueryTip),
  FXMAPFUNC(SEL_UPDATE,FXText::ID_TOGGLE_EDITABLE,FXText::onUpdToggleEditable),
  FXMAPFUNC(SEL_UPDATE,FXText::ID_TOGGLE_OVERSTRIKE,FXText::onUpdToggleOverstrike),
  FXMAPFUNC(SEL_UPDATE,FXText::ID_CURSOR_ROW,FXText::onUpdCursorRow),
  FXMAPFUNC(SEL_UPDATE,FXText::ID_CURSOR_COLUMN,FXText::onUpdCursorColumn),
  FXMAPFUNC(SEL_UPDATE,FXText::ID_CUT_SEL,FXText::onUpdHaveSelection),
  FXMAPFUNC(SEL_UPDATE,FXText::ID_COPY_SEL,FXText::onUpdHaveSelection),
  FXMAPFUNC(SEL_UPDATE,FXText::ID_PASTE_SEL,FXText::onUpdYes),
  FXMAPFUNC(SEL_UPDATE,FXText::ID_DELETE_SEL,FXText::onUpdHaveSelection),
  FXMAPFUNC(SEL_UPDATE,FXText::ID_UPPER_CASE,FXText::onUpdHaveSelection),
  FXMAPFUNC(SEL_UPDATE,FXText::ID_LOWER_CASE,FXText::onUpdHaveSelection),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_TOP,FXText::onCmdCursorTop),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_BOTTOM,FXText::onCmdCursorBottom),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_HOME,FXText::onCmdCursorHome),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_END,FXText::onCmdCursorEnd),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_RIGHT,FXText::onCmdCursorRight),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_LEFT,FXText::onCmdCursorLeft),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_UP,FXText::onCmdCursorUp),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_DOWN,FXText::onCmdCursorDown),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_WORD_LEFT,FXText::onCmdCursorWordLeft),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_WORD_RIGHT,FXText::onCmdCursorWordRight),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_PAGEDOWN,FXText::onCmdCursorPageDown),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_PAGEUP,FXText::onCmdCursorPageUp),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_SCRNTOP,FXText::onCmdCursorScreenTop),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_SCRNBTM,FXText::onCmdCursorScreenBottom),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_SCRNCTR,FXText::onCmdCursorScreenCenter),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_PAR_HOME,FXText::onCmdCursorParHome),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_PAR_END,FXText::onCmdCursorParEnd),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_SCROLL_UP,FXText::onCmdScrollUp),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_SCROLL_DOWN,FXText::onCmdScrollDown),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_MARK,FXText::onCmdMark),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_EXTEND,FXText::onCmdExtend),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_OVERST_STRING,FXText::onCmdOverstString),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_INSERT_STRING,FXText::onCmdInsertString),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_INSERT_NEWLINE,FXText::onCmdInsertNewline),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_INSERT_TAB,FXText::onCmdInsertTab),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CUT_SEL,FXText::onCmdCutSel),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_COPY_SEL,FXText::onCmdCopySel),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_PASTE_SEL,FXText::onCmdPasteSel),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_DELETE_SEL,FXText::onCmdDeleteSel),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_SELECT_CHAR,FXText::onCmdSelectChar),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_SELECT_WORD,FXText::onCmdSelectWord),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_SELECT_LINE,FXText::onCmdSelectLine),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_SELECT_ALL,FXText::onCmdSelectAll),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_DESELECT_ALL,FXText::onCmdDeselectAll),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_BACKSPACE,FXText::onCmdBackspace),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_BACKSPACE_WORD,FXText::onCmdBackspaceWord),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_BACKSPACE_BOL,FXText::onCmdBackspaceBol),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_DELETE,FXText::onCmdDelete),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_DELETE_WORD,FXText::onCmdDeleteWord),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_DELETE_EOL,FXText::onCmdDeleteEol),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_DELETE_LINE,FXText::onCmdDeleteLine),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_TOGGLE_EDITABLE,FXText::onCmdToggleEditable),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_TOGGLE_OVERSTRIKE,FXText::onCmdToggleOverstrike),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_ROW,FXText::onCmdCursorRow),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_CURSOR_COLUMN,FXText::onCmdCursorColumn),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETSTRINGVALUE,FXText::onCmdSetStringValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETSTRINGVALUE,FXText::onCmdGetStringValue),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_UPPER_CASE,FXText::onCmdChangeCase),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_LOWER_CASE,FXText::onCmdChangeCase),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_GOTO_MATCHING,FXText::onCmdGotoMatching),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_GOTO_SELECTED,FXText::onCmdGotoSelected),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_GOTO_LINE,FXText::onCmdGotoLine),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_SELECT_MATCHING,FXText::onCmdSelectMatching),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_SEARCH_FORW_SEL,FXText::onCmdSearchSel),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_SEARCH_BACK_SEL,FXText::onCmdSearchSel),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_SEARCH,FXText::onCmdSearch),
  FXMAPFUNC(SEL_COMMAND,FXText::ID_REPLACE,FXText::onCmdReplace),
  FXMAPFUNCS(SEL_COMMAND,FXText::ID_SELECT_BRACE,FXText::ID_SELECT_ANG,FXText::onCmdSelectBlock),
  FXMAPFUNCS(SEL_COMMAND,FXText::ID_LEFT_BRACE,FXText::ID_LEFT_ANG,FXText::onCmdBlockBeg),
  FXMAPFUNCS(SEL_COMMAND,FXText::ID_RIGHT_BRACE,FXText::ID_RIGHT_ANG,FXText::onCmdBlockEnd),
  FXMAPFUNCS(SEL_COMMAND,FXText::ID_CLEAN_INDENT,FXText::ID_SHIFT_TABRIGHT,FXText::onCmdShiftText),
  };


// Object implementation
FXIMPLEMENT(FXText,FXScrollArea,FXTextMap,ARRAYNUMBER(FXTextMap))

/*******************************************************************************/

// Search icon
const unsigned char searchicon[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x20,0x00,0x20,0x00,0xf3,0x00,0x00,0xb2,0xc0,0xdc,
  0x00,0x00,0x00,0x94,0x99,0x99,0x12,0x12,0x12,0x00,0x00,0xff,0xff,0x00,0x00,0x3f,
  0x3f,0x40,0x70,0x70,0x71,0x40,0x40,0x41,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2c,0x00,0x00,
  0x00,0x00,0x20,0x00,0x20,0x00,0x00,0x04,0x98,0x10,0xc8,0x49,0xab,0xbd,0x38,0xeb,
  0xcd,0xbb,0xff,0x5f,0x20,0x8e,0x64,0x69,0x8a,0xd9,0xa9,0xaa,0x60,0x0b,0x96,0x2e,
  0x40,0x4a,0xa2,0x60,0xdb,0xc3,0x40,0x8f,0x1b,0x2f,0x07,0x82,0x8a,0x20,0xa0,0xfb,
  0x05,0x42,0xc0,0xcb,0xb0,0xd8,0x4a,0x62,0x04,0x4c,0x0e,0x81,0x20,0x0b,0x02,0x0a,
  0xd8,0x6c,0x41,0x02,0x05,0x4c,0xa9,0x99,0x69,0x55,0xa2,0xcd,0x72,0x75,0xdf,0x8e,
  0x33,0xd3,0x7d,0x59,0xd9,0x51,0x0d,0x6a,0x0d,0x3b,0x7b,0x50,0x80,0x81,0xb5,0xde,
  0x6e,0x0d,0xd6,0x14,0x4b,0x31,0x79,0x80,0x00,0x4b,0x43,0x77,0x3e,0x39,0x37,0x38,
  0x03,0x06,0x07,0x7d,0x72,0x33,0x12,0x39,0x95,0x34,0x08,0x90,0x71,0x83,0x13,0x03,
  0x98,0x91,0x9b,0x9c,0x9e,0x9a,0x14,0x2b,0x26,0xa1,0x99,0x29,0xa5,0x75,0x79,0xa2,
  0xa0,0x16,0x9d,0x07,0xa3,0xa0,0x39,0xa9,0xaa,0xa6,0x1c,0xb6,0x2c,0xae,0xbb,0x9b,
  0x11,0x00,0x3b
  };

// Goto icon
const unsigned char gotoicon[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x20,0x00,0x20,0x00,0xf1,0x00,0x00,0xb2,0xc0,0xdc,
  0x00,0x00,0x00,0xff,0x00,0x00,0x00,0x00,0xff,0x2c,0x00,0x00,0x00,0x00,0x20,0x00,
  0x20,0x00,0x00,0x02,0x4e,0x84,0x8f,0xa9,0xcb,0xed,0x0f,0xa3,0x9c,0x33,0xd8,0x8b,
  0xb3,0xb6,0x6d,0x7b,0x4f,0x85,0xe2,0x78,0x7c,0x26,0xe7,0x9c,0xe6,0x22,0x90,0x8f,
  0xd0,0xba,0x09,0x4c,0xb7,0xc3,0x8d,0xe7,0xfa,0x3e,0x1c,0x35,0x0c,0xe0,0x09,0x79,
  0x0a,0xa0,0x8c,0x75,0x84,0xa8,0x34,0x92,0x65,0x26,0x09,0x8d,0x96,0x9c,0xcf,0x07,
  0xb5,0x2a,0xcd,0x8e,0xae,0x58,0x06,0x17,0xa3,0x0d,0x53,0xbe,0xdd,0x04,0x19,0x2c,
  0x4e,0x1b,0x0a,0x00,0x3b
  };

/*******************************************************************************/


// Helper
static inline FXint fxabs(FXint a){ return a<0?-a:a; }


// For deserialization
FXText::FXText(){
  flags|=FLAG_ENABLED|FLAG_DROPTARGET;
  buffer=NULL;
  sbuffer=NULL;
  visrows=NULL;
  length=0;
  nrows=1;
  nvisrows=0;
  gapstart=0;
  gapend=0;
  toppos=0;
  keeppos=0;
  toprow=0;
  selstartpos=0;
  selendpos=0;
  hilitestartpos=0;
  hiliteendpos=0;
  anchorpos=0;
  cursorpos=0;
  cursorstart=0;
  cursorend=0;
  cursorrow=0;
  cursorcol=0;
  prefcol=-1;
  wrapwidth=80;
  wrapcolumns=80;
  tabwidth=8;
  tabcolumns=8;
  barwidth=0;
  barcolumns=0;
  hilitestyles=NULL;
  blinker=NULL;
  flasher=NULL;
  textWidth=0;
  textHeight=0;
  clipbuffer=NULL;
  cliplength=0;
  vrows=0;
  vcols=0;
  matchtime=0;
  modified=FALSE;
  mode=MOUSE_NONE;
  grabx=0;
  graby=0;
  }


// Text widget
FXText::FXText(FXComposite *p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXScrollArea(p,opts,x,y,w,h),
  delimiters("~.,/\\`'!@#$%^&*()-=+{}|[]\":;<>?"){  // I *swear*, these are delimiters :-)
  flags|=FLAG_ENABLED|FLAG_DROPTARGET;
  target=tgt;
  message=sel;
  FXCALLOC(&buffer,FXchar,MINSIZE);
  sbuffer=NULL;
  FXCALLOC(&visrows,FXint,NVISROWS+1);
  length=0;
  nrows=1;
  nvisrows=NVISROWS;
  gapstart=0;
  gapend=MINSIZE;
  toppos=0;
  keeppos=0;
  toprow=0;
  selstartpos=0;
  selendpos=0;
  hilitestartpos=0;
  hiliteendpos=0;
  anchorpos=0;
  cursorpos=0;
  cursorstart=0;
  cursorend=0;
  cursorrow=0;
  cursorcol=0;
  prefcol=-1;
  margintop=2;
  marginbottom=2;
  marginleft=3;
  marginright=3;
  wrapwidth=80;
  wrapcolumns=80;
  tabwidth=8;
  tabcolumns=8;
  barwidth=0;
  barcolumns=0;
  font=getApp()->getNormalFont();
  hilitestyles=NULL;
  defaultCursor=getApp()->getDefaultCursor(DEF_TEXT_CURSOR);
  dragCursor=getApp()->getDefaultCursor(DEF_TEXT_CURSOR);
  textColor=getApp()->getForeColor();
  selbackColor=getApp()->getSelbackColor();
  seltextColor=getApp()->getSelforeColor();
  hilitebackColor=FXRGB(255,128,128);
  hilitetextColor=getApp()->getForeColor();
  activebackColor=backColor;
  cursorColor=FXRGB(192,0,0);
  numberColor=textColor;
  barColor=backColor;
  blinker=NULL;
  flasher=NULL;
  textWidth=0;
  textHeight=0;
  clipbuffer=NULL;
  cliplength=0;
  vrows=0;
  vcols=0;
  matchtime=0;
  modified=FALSE;
  mode=MOUSE_NONE;
  grabx=0;
  graby=0;
  }


// Create window
void FXText::create(){
  FXScrollArea::create();
  font->create();
  if(!deleteType){ deleteType=getApp()->registerDragType(deleteTypeName); }
  if(!textType){ textType=getApp()->registerDragType(textTypeName); }
  if(options&TEXT_FIXEDWRAP){ wrapwidth=wrapcolumns*font->getTextWidth(" ",1); }
  tabwidth=tabcolumns*font->getTextWidth(" ",1);
  barwidth=barcolumns*font->getTextWidth("8",1);
  recalc();// FIXME:- ugly because replaceText resets the flags...
  }


// Detach window
void FXText::detach(){
  FXScrollArea::detach();
  font->detach();
  deleteType=0;
  textType=0;
  }


// If window can have focus
FXbool FXText::canFocus() const { return 1; }


// Into focus chain
void FXText::setFocus(){
  FXScrollArea::setFocus();
  setDefault(TRUE);
  }


// Out of focus chain
void FXText::killFocus(){
  FXScrollArea::killFocus();
  setDefault(MAYBE);
  }


// Make a valid position
FXint FXText::validPos(FXint pos) const {
  return pos<0 ? 0 : pos>length ? length : pos;
  }


// Get default width
FXint FXText::getDefaultWidth(){
  if(0<vcols){ return marginleft+barwidth+marginright+vcols*font->getTextWidth("8",1); }
  return FXScrollArea::getDefaultWidth();
  }


// Get default height
FXint FXText::getDefaultHeight(){
  if(0<vrows){ return margintop+marginbottom+vrows*font->getFontHeight(); }
  return FXScrollArea::getDefaultHeight();
  }


// Enable the window
void FXText::enable(){
  if(!(flags&FLAG_ENABLED)){
    FXScrollArea::enable();
    update(0,0,viewport_w,viewport_h);
    }
  }


// Disable the window
void FXText::disable(){
  if(flags&FLAG_ENABLED){
    FXScrollArea::disable();
    update(0,0,viewport_w,viewport_h);
    }
  }


// Propagate size change
void FXText::recalc(){
  FXScrollArea::recalc();
  flags|=FLAG_RECALC;
  }


/*******************************************************************************/


// Get character
FXint FXText::getChar(FXint pos) const {
  FXASSERT(0<=pos && pos<length);
  return (FXuchar)buffer[pos<gapstart ? pos : pos-gapstart+gapend];
  }


// Get style
FXint FXText::getStyle(FXint pos) const {
  FXASSERT(0<=pos && pos<length);
  return (FXuchar)sbuffer[pos<gapstart ? pos : pos-gapstart+gapend];
  }


// Move the gap
void FXText::movegap(FXint pos){
  register FXint gaplen=gapend-gapstart;
  FXASSERT(0<=pos && pos<=length);
  FXASSERT(0<=gapstart && gapstart<=length);
  if(gapstart<pos){
    memmove(&buffer[gapstart],&buffer[gapend],pos-gapstart);
    if(sbuffer){memmove(&sbuffer[gapstart],&sbuffer[gapend],pos-gapstart);}
    gapend=pos+gaplen;
    gapstart=pos;
    }
  else if(pos<gapstart){
    memmove(&buffer[pos+gaplen],&buffer[pos],gapstart-pos);
    if(sbuffer){memmove(&sbuffer[pos+gaplen],&sbuffer[pos],gapstart-pos);}
    gapend=pos+gaplen;
    gapstart=pos;
    }
  }


// Size gap
void FXText::sizegap(FXint sz){
  register FXint gaplen=gapend-gapstart;
  FXASSERT(0<=gapstart && gapstart<=length);
  if(sz>=gaplen){
    sz+=MINSIZE;
    if(!FXRESIZE(&buffer,FXchar,length+sz)){
      fxerror("%s::sizegap: out of memory.\n",getClassName());
      }
    memmove(&buffer[gapstart+sz],&buffer[gapend],length-gapstart);
    if(sbuffer){
      if(!FXRESIZE(&sbuffer,FXchar,length+sz)){
        fxerror("%s::sizegap: out of memory.\n",getClassName());
        }
      memmove(&sbuffer[gapstart+sz],&sbuffer[gapend],length-gapstart);
      }
    gapend=gapstart+sz;
    }
  }


// Squeeze out the gap by moving it to the end of the buffer
void FXText::squeezegap(){
  if(gapstart!=length){
    memmove(&buffer[gapstart],&buffer[gapend],length-gapstart);
    if(sbuffer){memmove(&sbuffer[gapstart],&sbuffer[gapend],length-gapstart);}
    gapend=length+gapend-gapstart;
    gapstart=length;
    }
  }


/*******************************************************************************/


// Character width
FXint FXText::charWidth(FXchar ch,FXint indent) const {
  if(' ' <= ((FXuchar)ch)) return font->getTextWidth(&ch,1);
  if(ch == '\t') return (tabwidth-indent%tabwidth);
  ch|=0x40;
  return font->getTextWidth("^",1)+font->getTextWidth(&ch,1);
  }


// Start of next wrapped line
FXint FXText::wrap(FXint start) const {
  register FXint lw,cw,p,s,c;
  FXASSERT(0<=start && start<=length);
  lw=0;
  p=s=start;
  while(p<length){
    c=getChar(p);
    if(c=='\n') return p+1;     // Newline always breaks
    cw=charWidth(c,lw);
    if(lw+cw>wrapwidth){        // Technically, a tab-before-wrap should be as wide as space!
      if(s>start) return s;     // We remembered the last space we encountered; break there!
      if(p==start) p++;         // Always at least one character on each line!
      return p;
      }
    lw+=cw;
    p++;
    if(isspace(c)) s=p;         // Remember potential break point!
    }
  return length;
  }


// Count number of newlines
FXint FXText::countLines(FXint start,FXint end) const {
  register FXint p,nl=0;
  FXASSERT(0<=start && end<=length+1);
  p=start;
  while(p<end){
    if(p>=length) return nl+1;
    if(getChar(p)=='\n') nl++;
    p++;
    }
  return nl;
  }


// Count number of rows; start and end should be on a row start
FXint FXText::countRows(FXint start,FXint end) const {
  register FXint p,q,s,w=0,c,cw,nr=0;
  FXASSERT(0<=start && end<=length+1);
  if(options&TEXT_WORDWRAP){
    p=q=s=start;
    while(q<end){
      if(p>=length) return nr+1;
      c=getChar(p);
      if(c=='\n'){
        nr++;
        w=0;
        p=q=s=p+1;
        }
      else{
        cw=charWidth(c,w);
        if(w+cw>wrapwidth){
          nr++;
          if(s>q){
            p=q=s;
            }
          else{
            if(p==q) p++;
            q=s=p;
            }
          w=0;
          }
        else{
          w+=cw;
          p++;
          if(isspace(c)) s=p;
          }
        }
      }
    }
  else{
    p=start;
    while(p<end){
      if(p>=length) return nr+1;
      c=getChar(p);
      if(c=='\n') nr++;
      p++;
      }
    }
  return nr;
  }


// Count number of columns
FXint FXText::countCols(FXint start,FXint end) const {
  register FXint nc=0,in=0,ch;
  FXASSERT(0<=start && end<=length);
  while(start<end){
    ch=getChar(start);
    if(ch=='\n'){
      if(in>nc) nc=in;
      in=0;
      }
    else if(ch=='\t'){
      in+=(tabcolumns-nc%tabcolumns);
      }
    else{
      in++;
      }
    start++;
    }
  if(in>nc) nc=in;
  return nc;
  }


// Measure lines; start and end should be on a row start
FXint FXText::measureText(FXint start,FXint end,FXint& wmax,FXint& hmax) const {
  register FXint nr=0,w=0,c,cw,p,q,s;
  FXASSERT(0<=start && end<=length+1);
  if(options&TEXT_WORDWRAP){
    wmax=wrapwidth;
    p=q=s=start;
    while(q<end){
      if(p>=length){
        nr++;
        break;
        }
      c=getChar(p);
      if(c=='\n'){
        nr++;
        w=0;
        p=q=s=p+1;
        }
      else{
        cw=charWidth(c,w);
        if(w+cw>wrapwidth){
          nr++;
          if(s>q){
            p=q=s;
            }
          else{
            if(p==q) p++;
            q=s=p;
            }
          w=0;
          }
        else{
          w+=cw;
          p++;
          if(isspace(c)) s=p;
          }
        }
      }
    }
  else{
    wmax=0;
    p=start;
    while(p<end){
      if(p>=length){
        if(w>wmax) wmax=w;
        nr++;
        break;
        }
      c=getChar(p);
      if(c=='\n'){
        if(w>wmax) wmax=w;
        nr++;
        w=0;
        }
      else{
        w+=charWidth(c,w);
        }
      p++;
      }
    }
  hmax=nr*font->getFontHeight();
  return nr;
  }


// Find end of previous word
FXint FXText::leftWord(FXint pos) const {
  register FXchar ch;
  if(pos>length) pos=length;
  if(0<pos){
    ch=getChar(pos-1);
    if(delimiters.has(ch)) return pos-1;
    }
  while(0<pos){
    ch=getChar(pos-1);
    if(delimiters.has(ch)) return pos;
    if(isspace(ch)) break;
    pos--;
    }
  while(0<pos){
    ch=getChar(pos-1);
    if(!isspace(ch)) return pos;
    pos--;
    }
  return 0;
  }


// Find begin of next word
FXint FXText::rightWord(FXint pos) const {
  register FXchar ch;
  if(pos<0) pos=0;
  if(pos<length){
    ch=getChar(pos);
    if(delimiters.has(ch)) return pos+1;
    }
  while(pos<length){
    ch=getChar(pos);
    if(delimiters.has(ch)) return pos;
    if(isspace(ch)) break;
    pos++;
    }
  while(pos<length){
    ch=getChar(pos);
    if(!isspace(ch)) return pos;
    pos++;
    }
  return length;
  }


// Find begin of a word
FXint FXText::wordStart(FXint pos) const {
  register FXchar c=' ';
  if(pos<=0) return 0;
  if(pos<length) c=getChar(pos); else pos=length;
  if(c==' ' || c=='\t'){
    while(0<pos){
      c=getChar(pos-1);
      if(c!=' ' && c!='\t') return pos;
      pos--;
      }
    }
  else if(delimiters.has(c)){
    while(0<pos){
      c=getChar(pos-1);
      if(!delimiters.has(c)) return pos;
      pos--;
      }
    }
  else{
    while(0<pos){
      c=getChar(pos-1);
      if(delimiters.has(c) || isspace(c)) return pos;
      pos--;
      }
    }
  return 0;
  }


// Find end of word
FXint FXText::wordEnd(FXint pos) const {
  register FXchar c=' ';
  if(pos>=length) return length;
  if(0<=pos) c=getChar(pos); else pos=0;
  if(c==' ' || c=='\t'){
    while(pos<length){
      c=getChar(pos);
      if(c!=' ' && c!='\t') return pos;
      pos++;
      }
    }
  else if(delimiters.has(c)){
    while(pos<length){
      c=getChar(pos);
      if(!delimiters.has(c)) return pos;
      pos++;
      }
    }
  else{
    while(pos<length){
      c=getChar(pos);
      if(delimiters.has(c) || isspace(c)) return pos;
      pos++;
      }
    }
  return length;
  }


// Return position of begin of paragraph
FXint FXText::lineStart(FXint pos) const {
  FXASSERT(0<=pos && pos<=length);
  while(0<pos){ if(getChar(pos-1)=='\n') return pos; pos--; }
  return 0;
  }


// Return position of end of paragraph
FXint FXText::lineEnd(FXint pos) const {
  FXASSERT(0<=pos && pos<=length);
  while(pos<length){ if(getChar(pos)=='\n') return pos; pos++; }
  return length;
  }


// Return start of next line
FXint FXText::nextLine(FXint pos,FXint nl) const {
  FXASSERT(0<=pos && pos<=length);
  if(nl<=0) return pos;
  while(pos<length){
    if(getChar(pos)=='\n'){
      if(--nl==0) return pos+1;
      }
    pos++;
    }
  return length;
  }


// Return start of previous line
FXint FXText::prevLine(FXint pos,FXint nl) const {
  FXASSERT(0<=pos && pos<=length);
  if(nl<=0) return pos;
  while(0<pos){
    if(getChar(pos-1)=='\n'){
      if(nl--==0) return pos;
      }
    pos--;
    }
  return 0;
  }


// Return row start
FXint FXText::rowStart(FXint pos) const {
  register FXint p,t;
  FXASSERT(0<=pos && pos<=length);
  p=lineStart(pos);
  if(!(options&TEXT_WORDWRAP)) return p;
  while(p<pos && (t=wrap(p))<=pos && t<length) p=t;
  FXASSERT(0<=p && p<=pos);
  return p;
  }


// Return row end
FXint FXText::rowEnd(FXint pos) const {
  register FXint p;
  FXASSERT(0<=pos && pos<=length);
  if(!(options&TEXT_WORDWRAP)) return lineEnd(pos);
  p=lineStart(pos);
  while(p<length && p<=pos) p=wrap(p);
  FXASSERT(0<=p && p<=length);
  if(pos<p && isspace(getChar(p-1))) p--;
  FXASSERT(pos<=p && p<=length);
  return p;
  }


// Move to next row given start of line
FXint FXText::nextRow(FXint pos,FXint nr) const {
  register FXint p;
  FXASSERT(0<=pos && pos<=length);
  if(!(options&TEXT_WORDWRAP)) return nextLine(pos,nr);
  if(nr<=0) return pos;
  p=rowStart(pos);
  while(p<length && 0<nr){ p=wrap(p); nr--; }
  FXASSERT(0<=p && p<=length);
  return p;
  }


// Move to previous row given start of line
FXint FXText::prevRow(FXint pos,FXint nr) const {
  register FXint p,q,t;
  FXASSERT(0<=pos && pos<=length);
  if(!(options&TEXT_WORDWRAP)) return prevLine(pos,nr);
  if(nr<=0) return pos;
  while(0<pos){
    p=lineStart(pos);
    for(q=p; q<pos && (t=wrap(q))<=pos && t<length; q=t) nr--;
    if(nr==0) return p;
    if(nr<0){
      do{p=wrap(p);}while(++nr);
      FXASSERT(0<=p && p<=length);
      return p;
      }
    pos=p-1;
    nr--;
    }
  return 0;
  }


// Backs up to the begin of the line preceding the line containing pos, or the
// start of the line containing pos if the preceding line terminated in a newline
FXint FXText::changeBeg(FXint pos) const {
  register FXint p1,p2,t;
  FXASSERT(0<=pos && pos<=length);
  p1=p2=lineStart(pos);
  if(!(options&TEXT_WORDWRAP)) return p1;
  while(p2<pos && (t=wrap(p2))<=pos){
    p1=p2;
    p2=t;
    }
  FXASSERT(0<=p1 && p1<=length);
  return p1;
  }


// Scan forward to the end of affected area, which is the start of the next
// paragraph; a change can cause the rest of the paragraph to reflow.
FXint FXText::changeEnd(FXint pos) const {
  FXASSERT(0<=pos && pos<=length);
  while(pos<length){
    if(getChar(pos)=='\n') return pos+1;
    pos++;
    }
  return length+1;  // YES, one more!
  }


// Calculate line width
FXint FXText::lineWidth(FXint pos,FXint n) const {
  register FXint end=pos+n,w=0;
  FXASSERT(0<=pos && end<=length);
  while(pos<end){ w+=charWidth(getChar(pos),w); pos++; }
  return w;
  }


// Determine indent of position pos relative to start
FXint FXText::indentFromPos(FXint start,FXint pos) const {
  register FXint in=0,ch;
  FXASSERT(0<=start && pos<=length);
  while(start<pos){
    ch=getChar(start);
    if(ch=='\n'){
      in=0;
      }
    else if(ch=='\t'){
      in+=(tabcolumns-in%tabcolumns);
      }
    else{
      in+=1;
      }
    start++;
    }
  return in;
  }


// Determine position of indent relative to start
FXint FXText::posFromIndent(FXint start,FXint indent) const {
  register FXint in,pos,ch;
  FXASSERT(0<=start && start<=length);
  in=0;
  pos=start;
  while(in<indent && pos<length){
    ch=getChar(pos);
    if(ch=='\n')
      break;
    else if(ch=='\t')
      in+=(tabcolumns-in%tabcolumns);
    else
      in+=1;
    pos++;
    }
  return pos;
  }



// Search forward for match
FXint FXText::matchForward(FXint pos,FXint end,FXchar l,FXchar r,FXint level) const {
  register FXchar c;
  FXASSERT(0<=end && end<=length);
  FXASSERT(0<=pos && pos<=length);
  while(pos<end){
    c=getChar(pos);
    if(c==r){
      level--;
      if(level<=0) return pos;
      }
    else if(c==l){
      level++;
      }
    pos++;
    }
  return -1;
  }


// Search backward for match
FXint FXText::matchBackward(FXint pos,FXint beg,FXchar l,FXchar r,FXint level) const {
  register FXchar c;
  FXASSERT(0<=beg && beg<=length);
  FXASSERT(0<=pos && pos<=length);
  while(beg<=pos){
    c=getChar(pos);
    if(c==l){
      level--;
      if(level<=0) return pos;
      }
    else if(c==r){
      level++;
      }
    pos--;
    }
  return -1;
  }


// Search for matching character
FXint FXText::findMatching(FXint pos,FXint beg,FXint end,FXchar ch,FXint level) const {
  FXASSERT(0<=level);
  FXASSERT(0<=pos && pos<=length);
  switch(ch){
    case '{': return matchForward(pos+1,end,'{','}',level);
    case '}': return matchBackward(pos-1,beg,'{','}',level);
    case '[': return matchForward(pos+1,end,'[',']',level);
    case ']': return matchBackward(pos-1,beg,'[',']',level);
    case '(': return matchForward(pos+1,end,'(',')',level);
    case ')': return matchBackward(pos-1,beg,'(',')',level);
    }
  return -1;
  }


// Flash matching braces or parentheses, if within visible part of buffer
void FXText::flashMatching(){
  FXint matchpos;
  killHighlight();
  if(flasher) flasher=getApp()->removeTimeout(flasher);
  if(matchtime && 0<cursorpos){
    matchpos=findMatching(cursorpos-1,visrows[0],visrows[nvisrows],getChar(cursorpos-1),1);
    if(0<=matchpos){
      flasher=getApp()->addTimeout(matchtime,this,ID_FLASH);
      setHighlight(matchpos,1);
      /*
      if(matchpos<cursorpos){
        setHighlight(matchpos,cursorpos-matchpos);
        }
      else{
        setHighlight(cursorpos-1,matchpos-cursorpos+2);
        }
      */
      }
    }
  }


// Search for text
FXbool FXText::findText(const FXString& string,FXint* beg,FXint* end,FXint start,FXuint flags,FXint npar){
  register FXint mode;
  FXRex rex;

  // Compile flags
  mode=REX_VERBATIM;
  if(1<npar) mode|=REX_CAPTURE;
  if(flags&SEARCH_REGEX) mode&=~REX_VERBATIM;
  if(flags&SEARCH_IGNORECASE) mode|=REX_ICASE;

  // Try parse the regex
  if(rex.parse(string,mode)==REGERR_OK){

    // Make all characters contiguous in the buffer
    squeezegap();

    // Search backward
    if(flags&SEARCH_BACKWARD){

      // Search from start to begin of buffer
      if(rex.match(buffer,length,beg,end,REX_BACKWARD,npar,0,start)) return TRUE;

      if(!(flags&SEARCH_WRAP)) return FALSE;

      // Search from end of buffer backwards
      if(rex.match(buffer,length,beg,end,REX_BACKWARD,npar,start,length)) return TRUE;
      }

    // Search forward
    else{

      // Search from start to end of buffer
      if(rex.match(buffer,length,beg,end,REX_FORWARD,npar,start,length)) return TRUE;

      if(!(flags&SEARCH_WRAP)) return FALSE;

      // Search from begin of buffer forwards
      if(rex.match(buffer,length,beg,end,REX_FORWARD,npar,0,start)) return TRUE;
      }
    }
  return FALSE;
  }


/*******************************************************************************/


// See if pos is a visible position
FXbool FXText::posVisible(FXint pos) const {
  return visrows[0]<=pos && pos<=visrows[nvisrows];
  }


// See if position is in the selection, and the selection is non-empty
FXbool FXText::isPosSelected(FXint pos) const {
  return selstartpos<selendpos && selstartpos<=pos && pos<=selendpos;
  }


// Find line number from visible pos
FXint FXText::posToLine(FXint pos,FXint ln) const {
  FXASSERT(0<=ln && ln<nvisrows);
  FXASSERT(visrows[ln]<=pos && pos<=visrows[nvisrows]);
  while(ln<nvisrows-1 && visrows[ln+1]<=pos && visrows[ln]<visrows[ln+1]) ln++;
  FXASSERT(0<=ln && ln<nvisrows);
  FXASSERT(visrows[ln]<=pos && pos<=visrows[ln+1]);
  return ln;
  }


// Localize position at x,y
FXint FXText::getPosAt(FXint x,FXint y) const {
  register FXint row,ls,le,cx,cw;
  register FXchar ch;
  y=y-pos_y-margintop;
  row=y/font->getFontHeight();
  if(row<0) return 0;               // Before first row
  if(row>=nrows) return length;     // Below last row
  if(row<toprow){                   // Above visible area
    ls=prevRow(toppos,toprow-row);
    le=nextRow(ls,1);
    }
  else if(row>=toprow+nvisrows){    // Below visible area
    ls=nextRow(toppos,row-toprow);
    le=nextRow(ls,1);
    }
  else{                             // Inside visible area
    ls=visrows[row-toprow];
    le=visrows[row-toprow+1];
    }
  x=x-pos_x-marginleft-barwidth;    // Before begin of line
  if(x<0) return ls;
  FXASSERT(0<=ls);
  FXASSERT(ls<=le);
  FXASSERT(le<=length);
  if(ls<le && (((ch=getChar(le-1))=='\n') || (le<length && isspace(ch)))) le--;
  cx=0;
  while(ls<le){
    ch=getChar(ls);
    cw=charWidth(ch,cx);
    if(x<=(cx+(cw>>1))) return ls;
    cx+=cw;
    ls+=1;
    }
  return le;
  }


// Determine x,y from position pos
FXint FXText::getYOfPos(FXint pos) const {
  register FXint h=font->getFontHeight();
  register FXint n,y;
  if(pos>length) pos=length;
  if(pos<0) pos=0;

  // Above visible part of buffer
  if(pos<visrows[0]){
    n=countRows(rowStart(pos),visrows[0]);
    y=(toprow-n)*h;
    FXTRACE((150,"getYOfPos(%d < visrows[0]=%d) = %d\n",pos,visrows[0],margintop+y));
    }

  // Below visible part of buffer
  else if(pos>visrows[nvisrows]){
    n=countRows(visrows[nvisrows-1],pos);
    y=(toprow+nvisrows-1+n)*h;
    FXTRACE((150,"getYOfPos(%d > visrows[%d]=%d) = %d\n",pos,nvisrows,visrows[nvisrows],margintop+y));
    }

  // In visible part of buffer
  else{
    n=posToLine(pos,0);
    y=(toprow+n)*h;
    FXTRACE((150,"getYOfPos(visrows[0]=%d <= %d <= visrows[%d]=%d) = %d\n",visrows[0],pos,nvisrows,visrows[nvisrows],margintop+y));
    }
  return margintop+y;
  }


// Calculate X position of pos
FXint FXText::getXOfPos(FXint pos) const {
  register FXint base=rowStart(pos);
  return marginleft+barwidth+lineWidth(base,pos-base);
  }


// Force position to become fully visible
void FXText::makePositionVisible(FXint pos){
  register FXint x,y,nx,ny;

  // Get coordinates of position
  x=getXOfPos(pos);
  y=getYOfPos(pos);

  // Old scroll position
  ny=pos_y;
  nx=pos_x;

  // Check vertical visibility
  if(pos_y+y<margintop){
    ny=margintop-y;
    nx=0;
    }
  else if(pos_y+y+font->getFontHeight()>viewport_h-marginbottom){
    ny=viewport_h-font->getFontHeight()-marginbottom-y;
    nx=0;
    }

  // Check Horizontal visibility
  if(pos_x+x<marginleft+barwidth){
    nx=marginleft+barwidth-x;
    }
  else if(pos_x+x>viewport_w-marginright){
    nx=viewport_w-marginright-x;
    }

  // If needed, scroll
  if(nx!=pos_x || ny!=pos_y){
    setPosition(nx,ny);
    }
  }


// Return TRUE if position is visible
FXbool FXText::isPosVisible(FXint pos) const {
  if(visrows[0]<=pos && pos<=visrows[nvisrows]){
    register FXint h=font->getFontHeight();
    register FXint y=pos_y+margintop+(toprow+posToLine(pos,0))*h;
    return margintop<=y && y+h<viewport_h-marginbottom;
    }
  return FALSE;
  }


// Make line containing pos the top visible line
void FXText::setTopLine(FXint pos){
  setPosition(pos_x,margintop-getYOfPos(pos));
  }


// Make line containing pos the bottom visible line
void FXText::setBottomLine(FXint pos){
  setPosition(pos_x,viewport_h-font->getFontHeight()-marginbottom-getYOfPos(pos));
  }


// Center line of pos in the middle of the screen
void FXText::setCenterLine(FXint pos){
  setPosition(pos_x,viewport_h/2+font->getFontHeight()/2-getYOfPos(pos));
  }


// Get top line
FXint FXText::getTopLine() const {
  return visrows[0];
  }


// Get bottom line
FXint FXText::getBottomLine() const {
  return visrows[nvisrows-1];
  }


// Move content
void FXText::moveContents(FXint x,FXint y){
  register FXint delta,i,dx,dy;

  // Erase fragments of cursor overhanging margins
  eraseCursorOverhang();

  // Number of lines scrolled
  delta=-y/font->getFontHeight() - toprow;

  // Scrolled up one or more lines
  if(delta<0){
    if(toprow+delta<=0){
      toppos=0;
      toprow=0;
      }
    else{
      toppos=prevRow(toppos,-delta);
      toprow=toprow+delta;
      }
    if(-delta<nvisrows){
      for(i=nvisrows; i>=-delta; i--) visrows[i]=visrows[delta+i];
      calcVisRows(0,-delta);
      }
    else{
      calcVisRows(0,nvisrows);
      }
    }

  // Scrolled down one or more lines
  else if(delta>0){
    if(toprow+delta>=nrows-1){
      toppos=rowStart(length);
      toprow=nrows-1;
      }
    else{
      toppos=nextRow(toppos,delta);
      toprow=toprow+delta;
      }
    if(delta<nvisrows){
      for(i=0; i<=nvisrows-delta; i++) visrows[i]=visrows[delta+i];
      calcVisRows(nvisrows-delta,nvisrows);
      }
    else{
      calcVisRows(0,nvisrows);
      }
    }

  // This is now the new keep position
  keeppos=toppos;

  // Hopefully, all is still in range
  FXASSERT(0<=toprow && toprow<=nrows-1);
  FXASSERT(0<=toppos && toppos<=length);

  // Scroll the contents
  dx=x-pos_x;
  dy=y-pos_y;
  pos_x=x;
  pos_y=y;

  // Scroll stuff in the bar only vertically
  scroll(0,0,barwidth,viewport_h,0,dy);

  // Scroll the text
  scroll(marginleft+barwidth,margintop,viewport_w-marginleft-barwidth-marginright,viewport_h-margintop-marginbottom,dx,dy);
  }


/*******************************************************************************/


// Recalculate line starts
void FXText::calcVisRows(FXint startline,FXint endline){
  register FXint line,pos;
  FXASSERT(nvisrows>0);
  if(startline<0)
    startline=0;
  else if(startline>nvisrows)
    startline=nvisrows;
  if(endline<0)
    endline=0;
  else if(endline>nvisrows)
    endline=nvisrows;
  if(startline<=endline){
    if(startline==0){
      FXASSERT(0<=toppos && toppos<=length);
      visrows[0]=toppos;
      startline=1;
      }
    pos=visrows[startline-1];
    line=startline;
    if(options&TEXT_WORDWRAP){
      while(line<=endline && pos<length){
        pos=wrap(pos);
        FXASSERT(0<=pos && pos<=length);
        visrows[line++]=pos;
        }
      }
    else{
      while(line<=endline && pos<length){
        pos=nextLine(pos);
        FXASSERT(0<=pos && pos<=length);
        visrows[line++]=pos;
        }
      }
    while(line<=endline){
      visrows[line++]=length;
      }
    }
  }


// There has been a mutation in the buffer
void FXText::mutation(FXint pos,FXint ncins,FXint ncdel,FXint nrins,FXint nrdel){
  register FXint ncdelta=ncins-ncdel;
  register FXint nrdelta=nrins-nrdel;
  register FXint line,i,x,y;

  FXTRACE((150,"BEFORE: pos=%d ncins=%d ncdel=%d nrins=%d nrdel=%d toppos=%d toprow=%d nrows=%d nvisrows=%d\n",pos,ncins,ncdel,nrins,nrdel,toppos,toprow,nrows,nvisrows));

  // All of the change is below the last visible line
  if(visrows[nvisrows]<pos){
    FXTRACE((150,"change below visible\n"));
    nrows+=nrdelta;
    }

  // All change above first visible line
  else if(pos+ncdel<=visrows[0]){
    FXTRACE((150,"change above visible\n"));
    nrows+=nrdelta;
    toprow+=nrdelta;
    toppos+=ncdelta;
    keeppos=toppos;
    for(i=0; i<=nvisrows; i++) visrows[i]+=ncdelta;
    pos_y-=nrdelta*font->getFontHeight();
    FXASSERT(0<=toppos && toppos<=length);
    if(nrdelta) update(0,0,barwidth,height);
    }

  // Top visible part unchanged
  else if(visrows[0]<=pos){
    line=posToLine(pos,0);
    FXTRACE((150,"change below visible line %d\n",line));

    // More lines means paint the bottom half
    if(nrdelta>0){
      FXTRACE((150,"inserted %d rows\n",nrdelta));
      nrows+=nrdelta;
      for(i=nvisrows; i>line+nrdelta; i--) visrows[i]=visrows[i-nrdelta]+ncdelta;
      calcVisRows(line+1,line+nrins);
      FXASSERT(0<=toppos && toppos<=length);
      y=pos_y+margintop+(toprow+line)*font->getFontHeight();
      update(barwidth,y,width-barwidth,height-y);
      }

    // Less lines means paint bottom half also
    else if(nrdelta<0){
      FXTRACE((150,"deleted %d rows\n",-nrdelta));
      nrows+=nrdelta;
      for(i=line+1; i<=nvisrows+nrdelta; i++) visrows[i]=visrows[i-nrdelta]+ncdelta;
      calcVisRows(nvisrows+nrdelta,nvisrows);
      calcVisRows(line+1,line+nrins);
      FXASSERT(0<=toppos && toppos<=length);
      y=pos_y+margintop+(toprow+line)*font->getFontHeight();
      update(barwidth,y,width-barwidth,height-y);
      }

    // Same lines means paint the changed area only
    else{
      FXTRACE((150,"same number of rows\n"));
      for(i=line+1; i<=nvisrows; i++) visrows[i]=visrows[i]+ncdelta;
      calcVisRows(line+1,line+nrins);
      FXASSERT(0<=toppos && toppos<=length);
      if(nrins==0){
        x=pos_x+marginleft+barwidth+lineWidth(visrows[line],pos-visrows[line]);
        y=pos_y+margintop+(toprow+line)*font->getFontHeight();
        update(x,y,width-x,font->getFontHeight());
        FXTRACE((150,"update(%d,%d,%d,%d)\n",x,y,width-x,font->getFontHeight()));
        }
      else{
        y=pos_y+margintop+(toprow+line)*font->getFontHeight();
        update(barwidth,y,width-barwidth,nrins*font->getFontHeight());
        FXTRACE((150,"update(%d,%d,%d,%d)\n",0,y,width,nrins*font->getFontHeight()));
        }
      }
    }

  // Bottom visible part unchanged
  else if(pos+ncdel<visrows[nvisrows-1]){
    nrows+=nrdelta;
    line=1+posToLine(pos+ncdel,0);
    FXASSERT(0<=line && line<nvisrows);
    FXASSERT(pos+ncdel<=visrows[line]);
    FXTRACE((150,"change above visible line %d\n",line));

    // Too few lines left to display
    if(toprow+nrdelta<=line){
      FXTRACE((150,"reset to top\n"));
      toprow=0;
      toppos=0;
      keeppos=0;
      pos_y=0;
      calcVisRows(0,nvisrows);
      FXASSERT(0<=toppos && toppos<=length);
      update();
      }

    // Redisplay only the top
    else{
      FXTRACE((150,"redraw top %d lines\n",line));
      toprow+=nrdelta;
      toppos=prevRow(visrows[line]+ncdelta,line);
      keeppos=toppos;
      pos_y-=nrdelta*font->getFontHeight();
      calcVisRows(0,nvisrows);
      FXASSERT(0<=toppos && toppos<=length);
      update(barwidth,0,width-barwidth,pos_y+margintop+(toprow+line)*font->getFontHeight());
      if(nrdelta) update(0,0,barwidth,height);
      }
    }

  // All visible text changed
  else{
    FXTRACE((150,"change all visible lines\n"));
    nrows+=nrdelta;

    // Reset to top because too few lines left
    if(toprow>=nrows){
      FXTRACE((150,"reset to top\n"));
      toprow=0;
      toppos=0;
      keeppos=0;
      FXASSERT(0<=toppos && toppos<=length);
      pos_y=0;
      }

    // Maintain same row as before
    else{
      FXTRACE((150,"set to same row %d\n",toprow));
      toppos=nextRow(0,toprow);
      keeppos=toppos;
      FXASSERT(0<=toppos && toppos<=length);
      }
    calcVisRows(0,nvisrows);
    update();
    }

  FXTRACE((150,"AFTER : pos=%d ncins=%d ncdel=%d nrins=%d nrdel=%d toppos=%d toprow=%d nrows=%d\n",pos,ncins,ncdel,nrins,nrdel,toppos,toprow,nrows));
  }


// Replace m characters at pos by n characters
void FXText::replace(FXint pos,FXint m,const FXchar *text,FXint n,FXint style){
  register FXint nrdel,nrins,ncdel,ncins,wbeg,wend,del;
  FXint wdel,hdel,wins,hins;
  showCursor(0);    // FIXME can we do without this?

  FXTRACE((150,"pos=%d mdel=%d nins=%d\n",pos,m,n));

  // Delta in characters
  del=n-m;

  // Bracket potentially affected character range for wrapping purposes
  wbeg=changeBeg(pos);
  wend=changeEnd(pos+m);

  // Measure stuff prior to change
  nrdel=measureText(wbeg,wend,wdel,hdel);
  ncdel=wend-wbeg;

  FXTRACE((150,"wbeg=%d wend=%d nrdel=%d ncdel=%d length=%d wdel=%d hdel=%d\n",wbeg,wend,nrdel,ncdel,length,wdel,hdel));

  // Modify the buffer
  sizegap(del);
  movegap(pos);
  memcpy(&buffer[pos],text,n);
  if(sbuffer){memset(&sbuffer[pos],style,n);}
  gapstart+=n;
  gapend+=m;
  length+=del;

  // Measure stuff after change
  nrins=measureText(wbeg,wend+n-m,wins,hins);
  ncins=wend+n-m-wbeg;

  FXTRACE((150,"wbeg=%d wend+n-m=%d nrins=%d ncins=%d length=%d wins=%d hins=%d\n",wbeg,wend+n-m,nrins,ncins,length,wins,hins));

  // Update stuff
  mutation(wbeg,ncins,ncdel,nrins,nrdel);

  // Fix text metrics
  textHeight=textHeight+hins-hdel;
  textWidth=FXMAX(textWidth,wins);

  // Fix selection end
  if(pos+m<=selendpos) selendpos+=del;
  else if(pos<=selendpos) selendpos=pos+n;

  // Fix selection start
  if(pos+m<=selstartpos) selstartpos+=del;
  else if(pos<=selstartpos) selstartpos=pos+n;

  // Fix highlight end
  if(pos+m<=hiliteendpos) hiliteendpos+=del;
  else if(pos<=hiliteendpos) hiliteendpos=pos+n;

  // Fix highlight start
  if(pos+m<=hilitestartpos) hilitestartpos+=del;
  else if(pos<=hilitestartpos) hilitestartpos=pos+n;

  // Fix anchor position
  if(pos+m<=anchorpos) anchorpos+=del;
  else if(pos<=anchorpos) anchorpos=pos+n;

  // Update cursor position variables
  if(wend<=cursorpos){
    cursorpos+=del;
    cursorstart+=del;
    cursorend+=del;
    cursorrow+=nrins-nrdel;
    }
  else if(wbeg<=cursorpos){
    if(pos+m<=cursorpos) cursorpos+=del;
    else if(pos<=cursorpos) cursorpos=pos+n;
    cursorstart=rowStart(cursorpos);
    cursorend=nextRow(cursorstart);
    cursorcol=indentFromPos(cursorstart,cursorpos);
    if(cursorstart<toppos){
      cursorrow=toprow-countRows(cursorstart,toppos);
      }
    else{
      cursorrow=toprow+countRows(toppos,cursorstart);
      }
    }

  // Reconcile scrollbars
  FXScrollArea::layout();     // FIXME:- scrollbars, but no layout

  // Forget preferred column
  prefcol=-1;
  }


// Replace m characters at pos by n characters
void FXText::replaceStyledText(FXint pos,FXint m,const FXchar *text,FXint n,FXint style,FXbool notify){
  FXint what[2];
  if(n<0 || m<0 || pos<0 || length<pos+m){ fxerror("%s::replaceStyledText: bad argument range.\n",getClassName()); }
  if(notify && target){
    what[0]=pos;
    what[1]=m;
    target->handle(this,MKUINT(message,SEL_DELETED),(void*)what);
    }
  FXTRACE((130,"replaceStyledText(%d,%d,text,%d)\n",pos,m,n));
  replace(pos,m,text,n,style);
  if(notify && target){
    what[0]=pos;
    what[1]=n;
    target->handle(this,MKUINT(message,SEL_INSERTED),(void*)what);
    target->handle(this,MKUINT(message,SEL_CHANGED),(void*)(FXival)cursorpos);
    }
  }


// Replace text by other text
void FXText::replaceText(FXint pos,FXint m,const FXchar *text,FXint n,FXbool notify){
  replaceStyledText(pos,m,text,n,0,notify);
  }


// Add text at the end
void FXText::appendStyledText(const FXchar *text,FXint n,FXint style,FXbool notify){
  FXint what[2];
  if(n<0){ fxerror("%s::appendStyledText: bad argument range.\n",getClassName()); }
  FXTRACE((130,"appendStyledText(text,%d)\n",n));
  replace(length,0,text,n,style);
  if(notify && target){
    what[0]=length-n;   // Place where added; thanks to Sander Jansen <sxj@cfdrc.com>
    what[1]=n;
    target->handle(this,MKUINT(message,SEL_INSERTED),(void*)what);
    target->handle(this,MKUINT(message,SEL_CHANGED),(void*)(FXival)cursorpos);
    }
  }


// Add text at the end
void FXText::appendText(const FXchar *text,FXint n,FXbool notify){
  appendStyledText(text,n,0,notify);
  }


// Insert some text at pos
void FXText::insertStyledText(FXint pos,const FXchar *text,FXint n,FXint style,FXbool notify){
  FXint what[2];
  if(n<0 || pos<0 || length<pos){ fxerror("%s::insertStyledText: bad argument range.\n",getClassName()); }
  FXTRACE((130,"insertStyledText(%d,text,%d)\n",pos,n));
  replace(pos,0,text,n,style);
  if(notify && target){
    what[0]=pos;
    what[1]=n;
    target->handle(this,MKUINT(message,SEL_INSERTED),(void*)what);
    target->handle(this,MKUINT(message,SEL_CHANGED),(void*)(FXival)cursorpos);
    }
  }


// Insert some text at pos
void FXText::insertText(FXint pos,const FXchar *text,FXint n,FXbool notify){
  insertStyledText(pos,text,n,0,notify);
  }


// Remove some text at pos
void FXText::removeText(FXint pos,FXint n,FXbool notify){
  FXint what[2];
  if(n<0 || pos<0 || length<pos+n){ fxerror("%s::removeText: bad argument range.\n",getClassName()); }
  if(notify && target){
    what[0]=pos;
    what[1]=n;
    target->handle(this,MKUINT(message,SEL_DELETED),(void*)what);
    }
  FXTRACE((130,"removeText(%d,%d)\n",pos,n));
  replace(pos,n,NULL,0,0);
  if(notify && target){
    target->handle(this,MKUINT(message,SEL_CHANGED),(void*)(FXival)cursorpos);
    }
  }


// Grab range of text
void FXText::extractText(FXchar *text,FXint pos,FXint n) const {
  if(n<0 || pos<0 || length<pos+n){ fxerror("%s::extractText: bad argument.\n",getClassName()); }
  FXASSERT(0<=n && 0<=pos && pos+n<=length);
  if(pos+n<=gapstart){
    memcpy(text,&buffer[pos],n);
    }
  else if(pos>=gapstart){
    memcpy(text,&buffer[pos-gapstart+gapend],n);
    }
  else{
    memcpy(text,&buffer[pos],gapstart-pos);
    memcpy(&text[gapstart-pos],&buffer[gapend],pos+n-gapstart);
    }
  }


// Grab range of style
void FXText::extractStyle(FXchar *style,FXint pos,FXint n) const {
  if(n<0 || pos<0 || length<pos+n){ fxerror("%s::extractStyle: bad argument.\n",getClassName()); }
  FXASSERT(0<=n && 0<=pos && pos+n<=length);
  if(sbuffer){
    if(pos+n<=gapstart){
      memcpy(style,&sbuffer[pos],n);
      }
    else if(pos>=gapstart){
      memcpy(style,&sbuffer[pos-gapstart+gapend],n);
      }
    else{
      memcpy(style,&sbuffer[pos],gapstart-pos);
      memcpy(&style[gapstart-pos],&sbuffer[gapend],pos+n-gapstart);
      }
    }
  }


// Change style of text range
void FXText::changeStyle(FXint pos,FXint n,FXint style){
  if(n<0 || pos<0 || length<pos+n){ fxerror("%s::changeStyle: bad argument range.\n",getClassName()); }
  if(sbuffer){
    if(pos+n<=gapstart){
      memset(&sbuffer[pos],style,n);
      }
    else if(pos>=gapstart){
      memset(&sbuffer[pos-gapstart+gapend],style,n);
      }
    else{
      memset(&sbuffer[pos],style,gapstart-pos);
      memset(&sbuffer[gapend],style,pos+n-gapstart);
      }
    updateRange(pos,pos+n);
    }
  }


// Change style of text range from style-array
void FXText::changeStyle(FXint pos,FXint n,const FXchar* style){
  if(n<0 || pos<0 || length<pos+n){ fxerror("%s::changeStyle: bad argument range.\n",getClassName()); }
  if(sbuffer && style){
    if(pos+n<=gapstart){
      memcpy(&sbuffer[pos],style,n);
      }
    else if(pos>=gapstart){
      memcpy(&sbuffer[pos-gapstart+gapend],style,n);
      }
    else{
      memcpy(&sbuffer[pos],style,gapstart-pos);
      memcpy(&sbuffer[gapend],&style[gapstart-pos],pos+n-gapstart);
      }
    updateRange(pos,pos+n);
    }
  }


// Change the text in the buffer to new text
void FXText::setStyledText(const FXchar* text,FXint n,FXint style,FXbool notify){
  FXint what[2];
  if(n<0){ fxerror("%s::setText: bad argument range.\n",getClassName()); }
  if(!FXRESIZE(&buffer,FXchar,n+MINSIZE)){
    fxerror("%s::setText: out of memory.\n",getClassName());
    }
  memcpy(buffer,text,n);
  if(sbuffer){
    if(!FXRESIZE(&sbuffer,FXchar,n+MINSIZE)){
      fxerror("%s::setText: out of memory.\n",getClassName());
      }
    memset(sbuffer,style,n);
    }
  gapstart=n;
  gapend=gapstart+MINSIZE;
  length=n;
  toppos=0;
  toprow=0;
  keeppos=0;
  selstartpos=0;
  selendpos=0;
  hilitestartpos=0;
  hiliteendpos=0;
  anchorpos=0;
  cursorpos=0;
  cursorstart=0;
  cursorend=0;
  cursorrow=0;
  cursorcol=0;
  prefcol=-1;
  pos_x=0;
  pos_y=0;
  if(notify && target){
    what[0]=0;
    what[1]=n;
    target->handle(this,MKUINT(message,SEL_INSERTED),(void*)what);
    target->handle(this,MKUINT(message,SEL_CHANGED),(void*)(FXival)cursorpos);
    }
  recalc();
  layout();
  update();
  }


// Change the text in the buffer to new text
void FXText::setText(const FXchar* text,FXint n,FXbool notify){
  setStyledText(text,n,0,notify);
  }


// Retrieve text into buffer
void FXText::getText(FXchar* text,FXint n) const {
  extractText(text,0,n);
  }


// Change all of the text
void FXText::setStyledText(const FXString& text,FXint style,FXbool notify){
  setStyledText(text.text(),text.length(),style,notify);
  }


// Change all of the text
void FXText::setText(const FXString& text,FXbool notify){
  setStyledText(text.text(),text.length(),0,notify);
  }


// We return a constant copy of the buffer
FXString FXText::getText() const {
  FXString value;
  FXASSERT(0<=gapstart && gapstart<=length);
  value.append(buffer,gapstart);
  value.append(&buffer[gapend],length-gapstart);
  return value;
  }


// Perform belated layout
long FXText::onUpdate(FXObject* sender,FXSelector sel,void* ptr){
  FXScrollArea::onUpdate(sender,sel,ptr);
  // FIXME full text reflow should be done by delayed layout,
  // rather than immediately.
  return 1;
  }


// Completely reflow the text, because font, wrapwidth, or all of the
// text may have changed and everything needs to be recomputed
void FXText::recompute(){
  FXint hh=font->getFontHeight();
  FXint ww1,ww2,ww3,hh1,hh2,hh3;

  // Major recalc
  if(flags&FLAG_RECALC){

    // Make it point somewhere sensible
    if(keeppos<0) keeppos=0;
    if(keeppos>length) keeppos=length;

    // Make sure we're pointing to the start of a row again
    toppos=rowStart(keeppos);

    // Get start
    cursorstart=rowStart(cursorpos);
    cursorend=nextRow(cursorstart);
    cursorcol=indentFromPos(cursorstart,cursorpos);

    // Avoid measuring huge chunks of text twice!
    if(cursorstart<toprow){
      cursorrow=measureText(0,cursorstart,ww1,hh1);
      toprow=cursorrow+measureText(cursorstart,toppos,ww2,hh2);
      nrows=toprow+measureText(toppos,length+1,ww3,hh3);
      }
    else{
      toprow=measureText(0,toppos,ww1,hh1);
      cursorrow=toprow+measureText(toppos,cursorstart,ww2,hh2);
      nrows=cursorrow+measureText(cursorstart,length+1,ww3,hh3);
      }

    textWidth=FXMAX3(ww1,ww2,ww3);
    textHeight=hh1+hh2+hh3;

    // Adjust position; we keep the same fractional position
    pos_y=-toprow*hh-(-pos_y%hh);
    }

  // Number of visible lines may have changed
  nvisrows=(height-margintop-marginbottom+hh+hh-1)/hh;
  if(nvisrows<1) nvisrows=1;

  // Number of visible lines changed; lines is 1 longer than nvisrows,
  // so we can find the end of a line faster for every visible line
  FXRESIZE(&visrows,FXint,nvisrows+1);

  // Recompute line starts
  calcVisRows(0,nvisrows);

  FXTRACE((150,"recompute : toprow=%d toppos=%d nrows=%d nvisrows=%d textWidth=%d textHeight=%d length=%d cursorrow=%d cursorcol=%d\n",toprow,toppos,nrows,nvisrows,textWidth,textHeight,length,cursorrow,cursorcol));

  // Done with that
  flags&=~(FLAG_RECALC|FLAG_DIRTY);
  }


/*******************************************************************************/


// Determine content width of scroll area
FXint FXText::getContentWidth(){
  if(flags&FLAG_DIRTY) recompute();
  return marginleft+barwidth+marginright+textWidth;
  }


// Determine content height of scroll area
FXint FXText::getContentHeight(){
  if(flags&FLAG_DIRTY) recompute();
  return margintop+marginbottom+textHeight;
  }


// Recalculate layout
void FXText::layout(){

  // Compute new wrap width
  if(!(options&TEXT_FIXEDWRAP)){
    wrapwidth=width-marginleft-barwidth-marginright;
    if(!(options&VSCROLLER_NEVER)) wrapwidth-=vertical->getDefaultWidth();
    }
  else{
    wrapwidth=wrapcolumns*font->getTextWidth(" ",1);
    }

  // Scrollbars adjusted
  FXScrollArea::layout();

  // Set line size based on font
  vertical->setLine(font->getFontHeight());
  horizontal->setLine(font->getTextWidth(" ",1));

  // Force repaint
  update();

  // Done
  flags&=~FLAG_DIRTY;
  }


// The widget is resized
void FXText::resize(FXint w,FXint h){
  FXint hh=font->getFontHeight();
  FXint nv=(h-margintop-marginbottom+hh+hh-1)/hh;
  if(nv<1) nv=1;

  // In wrap mode, a width change causes a content recalculation
  if((options&TEXT_WORDWRAP) && !(options&TEXT_FIXEDWRAP) && (width!=w)) flags|=(FLAG_RECALC|FLAG_DIRTY);

  // Need to redo line starts
  if(nv!=nvisrows) flags|=FLAG_DIRTY;

  // Resize the window, and do layout
  FXScrollArea::resize(w,h);
  }


// The widget is moved and possibly resized
void FXText::position(FXint x,FXint y,FXint w,FXint h){
  FXint hh=font->getFontHeight();
  FXint nv=(h-margintop-marginbottom+hh+hh-1)/hh;
  if(nv<1) nv=1;
  //FXTRACE((100,"FXText::position width=%d height=%d w=%d h=%d hh=%d nv=%d\n",width,height,w,h,hh,nv));

  // In wrap mode, a width change causes a content recalculation
  if((options&TEXT_WORDWRAP) && !(options&TEXT_FIXEDWRAP) && (width!=w)) flags|=(FLAG_RECALC|FLAG_DIRTY);

  // Need to redo line starts
  if(nv!=nvisrows) flags|=FLAG_DIRTY;

  // Place the window, and do layout
  FXScrollArea::position(x,y,w,h);
  }


/*******************************************************************************/


// Gained focus
long FXText::onFocusIn(FXObject* sender,FXSelector sel,void* ptr){
  FXScrollArea::onFocusIn(sender,sel,ptr);
  if(!blinker){blinker=getApp()->addTimeout(getApp()->getBlinkSpeed(),this,ID_BLINK);}
  showCursor(FLAG_CARET);
  return 1;
  }


// Lost focus
long FXText::onFocusOut(FXObject* sender,FXSelector sel,void* ptr){
  FXScrollArea::onFocusOut(sender,sel,ptr);
  if(blinker){blinker=getApp()->removeTimeout(blinker);}
  showCursor(0);
  flags|=FLAG_UPDATE;
  return 1;
  }



// We were asked about status text
long FXText::onQueryHelp(FXObject* sender,FXSelector,void*){
  if(!help.empty() && (flags&FLAG_HELP)){
    sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&help);
    return 1;
    }
  return 0;
  }


// We were asked about tip text
long FXText::onQueryTip(FXObject* sender,FXSelector,void*){
  if(!tip.empty() && (flags&FLAG_TIP)){
    sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),&tip);
    return 1;
    }
  return 0;
  }


// Blink the cursor
long FXText::onBlink(FXObject*,FXSelector,void*){
  showCursor(flags^FLAG_CARET);
  blinker=getApp()->addTimeout(getApp()->getBlinkSpeed(),this,ID_BLINK);
  return 0;
  }


// Flash matching brace
long FXText::onFlash(FXObject*,FXSelector,void*){
  killHighlight();
  flasher=NULL;
  return 0;
  }


// Pressed left button
long FXText::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  FXint pos;
  flags&=~FLAG_TIP;
  handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
  if(isEnabled()){
    grab();
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr)) return 1;
    flags&=~FLAG_UPDATE;
    if(ev->click_count==1){
      pos=getPosAt(ev->win_x,ev->win_y);
      FXTRACE((150,"getPosAt(%d,%d) = %d getYOfPos(%d) = %d getXOfPos(%d)=%d\n",ev->win_x,ev->win_y,pos,pos,getYOfPos(pos),pos,getXOfPos(pos)));
      setCursorPos(pos,TRUE);
      makePositionVisible(pos);
      if(ev->state&SHIFTMASK){
        extendSelection(pos,SELECT_CHARS,TRUE);
        }
      else{
        killSelection(TRUE);
        setAnchorPos(pos);
        flashMatching();
        }
      mode=MOUSE_CHARS;
      }

    // Select word by double click
    else if(ev->click_count==2){
      setAnchorPos(cursorpos);
      extendSelection(cursorpos,SELECT_WORDS,TRUE);
      mode=MOUSE_WORDS;
      }

    // Select line by triple click
    else{
      setAnchorPos(cursorpos);
      extendSelection(cursorpos,SELECT_LINES,TRUE);
      mode=MOUSE_LINES;
      }
    return 1;
    }
  return 0;
  }


// Released left button
long FXText::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    ungrab();
    flags|=FLAG_UPDATE;
    mode=MOUSE_NONE;
    stopAutoScroll();
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr)) return 1;
    return 1;
    }
  return 0;
  }


// Pressed middle button
long FXText::onMiddleBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  FXint pos;
  flags&=~FLAG_TIP;
  handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
  if(isEnabled()){
    grab();
    if(target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONPRESS),ptr)) return 1;
    pos=getPosAt(ev->win_x,ev->win_y);

    // Move over
    setCursorPos(pos,TRUE);
    makePositionVisible(pos);

    // Start text drag
    if(isPosSelected(pos)){
      mode=MOUSE_TRYDRAG;
      flags&=~FLAG_UPDATE;
      }

    // Place cursor
    else if(isEditable()){
      mode=MOUSE_PASTE;
      flags&=~FLAG_UPDATE;
      }

    // Beep
    else{
      getApp()->beep();
      }
    return 1;
    }
  return 0;
  }


// Released middle button
long FXText::onMiddleBtnRelease(FXObject*,FXSelector,void* ptr){
  FXuchar *data; FXuint len;
  if(isEnabled()){
    ungrab();
    flags|=FLAG_UPDATE;
    stopAutoScroll();
    if(target && target->handle(this,MKUINT(message,SEL_MIDDLEBUTTONRELEASE),ptr)) return 1;

    // Drop text somewhere
    if(mode==MOUSE_DRAG){
      handle(this,MKUINT(0,SEL_ENDDRAG),ptr);
      }

    // Paste text from selection
    else if(mode==MOUSE_PASTE){
      if(getDNDData(FROM_SELECTION,stringType,data,len)){
        FXRESIZE(&data,FXchar,len+1); data[len]='\0';
        handle(this,MKUINT(ID_INSERT_STRING,SEL_COMMAND),(void*)data);
        FXFREE(&data);
        }
      }
    mode=MOUSE_NONE;
    return 1;
    }
  return 0;
  }


// Pressed right button
long FXText::onRightBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  flags&=~FLAG_TIP;
  handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
  if(isEnabled()){
    grab();
    if(target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONPRESS),ptr)) return 1;
    mode=MOUSE_SCROLL;
    grabx=ev->win_x-pos_x;
    graby=ev->win_y-pos_y;
    flags&=~FLAG_UPDATE;
    return 1;
    }
  return 0;
  }


// Released right button
long FXText::onRightBtnRelease(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    ungrab();
    mode=MOUSE_NONE;
    flags|=FLAG_UPDATE;
    if(target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONRELEASE),ptr)) return 1;
    return 1;
    }
  return 0;
  }


// The widget lost the grab for some reason
long FXText::onUngrabbed(FXObject* sender,FXSelector sel,void* ptr){
  FXScrollArea::onUngrabbed(sender,sel,ptr);
  mode=MOUSE_NONE;
  flags|=FLAG_UPDATE;
  stopAutoScroll();
  return 1;
  }


// Autoscroll timer fired
long FXText::onAutoScroll(FXObject* sender,FXSelector sel,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  FXint pos;
  FXScrollArea::onAutoScroll(sender,sel,ptr);
  switch(mode){
    case MOUSE_CHARS:
      if((fxabs(ev->win_x-ev->click_x)>getApp()->getDragDelta())||(fxabs(ev->win_y-ev->click_y)>getApp()->getDragDelta())){
        pos=getPosAt(ev->win_x,ev->win_y);
        extendSelection(pos,SELECT_CHARS,TRUE);
        setCursorPos(pos,TRUE);
        //makePositionVisible(pos);
        }
      return 1;
    case MOUSE_WORDS:
      if((fxabs(ev->win_x-ev->click_x)>getApp()->getDragDelta())||(fxabs(ev->win_y-ev->click_y)>getApp()->getDragDelta())){
        pos=getPosAt(ev->win_x,ev->win_y);
        extendSelection(pos,SELECT_WORDS,TRUE);
        setCursorPos(pos,TRUE);
        //makePositionVisible(pos);
        }
      return 1;
    case MOUSE_LINES:
      if((fxabs(ev->win_x-ev->click_x)>getApp()->getDragDelta())||(fxabs(ev->win_y-ev->click_y)>getApp()->getDragDelta())){
        pos=getPosAt(ev->win_x,ev->win_y);
        extendSelection(pos,SELECT_LINES,TRUE);
        setCursorPos(pos,TRUE);
        //makePositionVisible(pos);
        }
      return 1;
    }
  return 0;
  }


// Handle real or simulated mouse motion
long FXText::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  FXint pos;
  switch(mode){
    case MOUSE_CHARS:
      if(startAutoScroll(ev->win_x,ev->win_y,FALSE)) return 1;
      if((fxabs(ev->win_x-ev->click_x)>getApp()->getDragDelta())||(fxabs(ev->win_y-ev->click_y)>getApp()->getDragDelta())){
        pos=getPosAt(ev->win_x,ev->win_y);
        extendSelection(pos,SELECT_CHARS,TRUE);
        setCursorPos(pos,TRUE);
        }
      return 1;
    case MOUSE_WORDS:
      if(startAutoScroll(ev->win_x,ev->win_y,FALSE)) return 1;
      if((fxabs(ev->win_x-ev->click_x)>getApp()->getDragDelta())||(fxabs(ev->win_y-ev->click_y)>getApp()->getDragDelta())){
        pos=getPosAt(ev->win_x,ev->win_y);
        extendSelection(pos,SELECT_WORDS,TRUE);
        setCursorPos(pos,TRUE);
        }
      return 1;
    case MOUSE_LINES:
      if(startAutoScroll(ev->win_x,ev->win_y,FALSE)) return 1;
      if((fxabs(ev->win_x-ev->click_x)>getApp()->getDragDelta())||(fxabs(ev->win_y-ev->click_y)>getApp()->getDragDelta())){
        pos=getPosAt(ev->win_x,ev->win_y);
        extendSelection(pos,SELECT_LINES,TRUE);
        setCursorPos(pos,TRUE);
        }
      return 1;
    case MOUSE_SCROLL:
      setPosition(ev->win_x-grabx,ev->win_y-graby);
      return 1;
    case MOUSE_DRAG:
      handle(this,MKUINT(0,SEL_DRAGGED),ptr);
      return 1;
    case MOUSE_TRYDRAG:
      if(ev->moved){
        mode=MOUSE_NONE;
        if(handle(this,MKUINT(0,SEL_BEGINDRAG),ptr)){
          mode=MOUSE_DRAG;
          }
        }
      return 1;
    case MOUSE_PASTE:
      if(startAutoScroll(ev->win_x,ev->win_y,FALSE)) return 1;
      if((fxabs(ev->win_x-ev->click_x)>getApp()->getDragDelta())||(fxabs(ev->win_y-ev->click_y)>getApp()->getDragDelta())){
        pos=getPosAt(ev->win_x,ev->win_y);
        setCursorPos(pos,TRUE);
        }
      return 1;
    }
  return 0;
  }


/*******************************************************************************/


// Start a drag operation
long FXText::onBeginDrag(FXObject* sender,FXSelector sel,void* ptr){
  if(FXScrollArea::onBeginDrag(sender,sel,ptr)) return 1;
  beginDrag(&textType,1);
  setDragCursor(getApp()->getDefaultCursor(DEF_DNDSTOP_CURSOR));
  return 1;
  }


// End drag operation
long FXText::onEndDrag(FXObject* sender,FXSelector sel,void* ptr){
  if(FXScrollArea::onEndDrag(sender,sel,ptr)) return 1;
  endDrag((didAccept()!=DRAG_REJECT));
  setDragCursor(getApp()->getDefaultCursor(DEF_TEXT_CURSOR));
  return 1;
  }


// Dragged stuff around
long FXText::onDragged(FXObject* sender,FXSelector sel,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXDragAction action;
  if(FXScrollArea::onDragged(sender,sel,ptr)) return 1;
  action=DRAG_COPY;
  if(isEditable()){
    if(isDropTarget()) action=DRAG_MOVE;
    if(event->state&CONTROLMASK) action=DRAG_COPY;
    if(event->state&SHIFTMASK) action=DRAG_MOVE;
    }
  handleDrag(event->root_x,event->root_y,action);
  if(didAccept()!=DRAG_REJECT){
    if(action==DRAG_MOVE)
      setDragCursor(getApp()->getDefaultCursor(DEF_DNDMOVE_CURSOR));
    else
      setDragCursor(getApp()->getDefaultCursor(DEF_DNDCOPY_CURSOR));
    }
  else{
    setDragCursor(getApp()->getDefaultCursor(DEF_DNDSTOP_CURSOR));
    }
  return 1;
  }


// Handle drag-and-drop enter
long FXText::onDNDEnter(FXObject* sender,FXSelector sel,void* ptr){
  FXScrollArea::onDNDEnter(sender,sel,ptr);
  showCursor(FLAG_CARET);
  return 1;
  }


// Handle drag-and-drop leave
long FXText::onDNDLeave(FXObject* sender,FXSelector sel,void* ptr){
  FXScrollArea::onDNDLeave(sender,sel,ptr);
  stopAutoScroll();
  showCursor(0);
  return 1;
  }


// Handle drag-and-drop motion
long FXText::onDNDMotion(FXObject* sender,FXSelector sel,void* ptr){
  FXEvent* ev=(FXEvent*)ptr;
  FXDragAction action;
  FXint pos;

  // Scroll into view
  if(startAutoScroll(ev->win_x,ev->win_y,TRUE)) return 1;

  // Handled elsewhere
  if(FXScrollArea::onDNDMotion(sender,sel,ptr)) return 1;

  // Correct drop type
  if(offeredDNDType(FROM_DRAGNDROP,textType)){

    // Is target editable?
    if(isEditable()){
      action=inquireDNDAction();

      // Check for legal DND action
      if(action==DRAG_COPY || action==DRAG_MOVE){

        // Get the suggested drop position
        pos=getPosAt(ev->win_x,ev->win_y);

        // Move cursor to new position
        setCursorPos(pos,TRUE);
        makePositionVisible(pos);

        // We don't accept a drop on the selection
        if(!isPosSelected(pos)){
          acceptDrop(DRAG_ACCEPT);
          }
        }
      }
    return 1;
    }

  // Didn't handle it here
  return 0;
  }


// Handle drag-and-drop drop
long FXText::onDNDDrop(FXObject* sender,FXSelector sel,void* ptr){
  FXuchar *data,*junk; FXuint len,dum;

  // Stop scrolling
  stopAutoScroll();
  showCursor(0);

  // Try handling it in base class first
  if(FXScrollArea::onDNDDrop(sender,sel,ptr)) return 1;

  // Should really not have gotten this if non-editable
  if(isEditable()){

    // Try handle here
    if(getDNDData(FROM_DRAGNDROP,textType,data,len)){
      FXRESIZE(&data,FXchar,len+1); data[len]='\0';

      // Need to ask the source to delete his copy
      if(inquireDNDAction()==DRAG_MOVE){
        getDNDData(FROM_DRAGNDROP,deleteType,junk,dum);
        FXASSERT(!junk);
        }

      // Insert the new text
      handle(this,MKUINT(ID_INSERT_STRING,SEL_COMMAND),(void*)data);
      FXFREE(&data);
      }
    return 1;
    }
  return 0;
  }


// Service requested DND data
long FXText::onDNDRequest(FXObject* sender,FXSelector sel,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  FXuchar *data; FXuint len;

  // Perhaps the target wants to supply its own data
  if(FXScrollArea::onDNDRequest(sender,sel,ptr)) return 1;

  // Return dragged text
  if(event->target==textType){
    len=selendpos-selstartpos;
    FXMALLOC(&data,FXuchar,len);
    extractText((FXchar*)data,selstartpos,len);
    setDNDData(FROM_DRAGNDROP,stringType,data,len);
    return 1;
    }

  // Delete dragged text
  if(event->target==deleteType){
    if(isEditable()){
      handle(this,MKUINT(ID_DELETE_SEL,SEL_COMMAND),NULL);
      }
    return 1;
    }

  return 0;
  }


/*******************************************************************************/


// We now really do have the selection
long FXText::onSelectionGained(FXObject* sender,FXSelector sel,void* ptr){
  FXScrollArea::onSelectionGained(sender,sel,ptr);
  return 1;
  }


// We lost the selection somehow
long FXText::onSelectionLost(FXObject* sender,FXSelector sel,void* ptr){
  FXint what[2];
  FXScrollArea::onSelectionLost(sender,sel,ptr);
  if(target){
    what[0]=selstartpos;
    what[1]=selendpos-selstartpos;
    target->handle(this,MKUINT(message,SEL_DESELECTED),(void*)what);
    }
  updateRange(selstartpos,selendpos);
  selstartpos=0;
  selendpos=0;
  return 1;
  }


// Somebody wants our selection
long FXText::onSelectionRequest(FXObject* sender,FXSelector sel,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  FXuchar *data; FXuint len;

  // Perhaps the target wants to supply its own data for the selection
  if(FXScrollArea::onSelectionRequest(sender,sel,ptr)) return 1;

  // Return text of the selection
  if(event->target==stringType || event->target==textType){
    FXASSERT(selstartpos<=selendpos);
    len=selendpos-selstartpos;
    FXMALLOC(&data,FXuchar,len);
    extractText((FXchar*)data,selstartpos,len);
    setDNDData(FROM_SELECTION,event->target,data,len);
    return 1;
    }

  return 0;
  }


/*******************************************************************************/


// We now really do have the selection
long FXText::onClipboardGained(FXObject* sender,FXSelector sel,void* ptr){
  FXScrollArea::onClipboardGained(sender,sel,ptr);
  return 1;
  }


// We lost the selection somehow
long FXText::onClipboardLost(FXObject* sender,FXSelector sel,void* ptr){
  FXScrollArea::onClipboardLost(sender,sel,ptr);
  FXFREE(&clipbuffer);
  clipbuffer=NULL;
  cliplength=0;
  return 1;
  }


// Somebody wants our selection
long FXText::onClipboardRequest(FXObject* sender,FXSelector sel,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  FXuchar *data;

  // Try handling it in base class first
  if(FXScrollArea::onClipboardRequest(sender,sel,ptr)) return 1;

  // Requested data from clipboard
  if(event->target==stringType || event->target==textType){
    FXASSERT(cliplength);
#ifndef WIN32
    FXMALLOC(&data,FXuchar,cliplength);
    memcpy(data,clipbuffer,cliplength);
    setDNDData(FROM_CLIPBOARD,stringType,data,cliplength);
#else
    FXint i,j,c;
    FXCALLOC(&data,FXuchar,2*cliplength+1);
    for(i=j=0; j<cliplength; j++){
      c=clipbuffer[j];
      if(c=='\n'){data[i++]='\r';data[i++]='\n';}else{data[i++]=c;}
      }
    setDNDData(FROM_CLIPBOARD,event->target,data,i+1);
#endif
    return 1;
    }

  return 0;
  }

/*******************************************************************************/

// Keyboard press
long FXText::onKeyPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  flags&=~FLAG_TIP;
  if(!isEnabled()) return 0;
  FXTRACE((200,"%s::onKeyPress keysym=0x%04x state=%04x\n",getClassName(),event->code,event->state));
  if(target && target->handle(this,MKUINT(message,SEL_KEYPRESS),ptr)) return 1;
  switch(event->code){
    case KEY_Shift_L:
    case KEY_Shift_R:
    case KEY_Control_L:
    case KEY_Control_R:
      if(mode==MOUSE_DRAG){handle(this,MKUINT(0,SEL_DRAGGED),ptr);}
      return 1;
    case KEY_Up:
    case KEY_KP_Up:
      if(event->state&CONTROLMASK){
        handle(this,MKUINT(ID_SCROLL_UP,SEL_COMMAND),NULL);
        }
      else{
        if(!(event->state&SHIFTMASK)){
          handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
          }
        handle(this,MKUINT(ID_CURSOR_UP,SEL_COMMAND),NULL);
        if(event->state&SHIFTMASK){
          handle(this,MKUINT(ID_EXTEND,SEL_COMMAND),NULL);
          }
        else{
          handle(this,MKUINT(ID_MARK,SEL_COMMAND),NULL);
          }
        }
      return 1;
    case KEY_Down:
    case KEY_KP_Down:
      if(event->state&CONTROLMASK){
        handle(this,MKUINT(ID_SCROLL_DOWN,SEL_COMMAND),NULL);
        }
      else{
        if(!(event->state&SHIFTMASK)){
          handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
          }
        handle(this,MKUINT(ID_CURSOR_DOWN,SEL_COMMAND),NULL);
        if(event->state&SHIFTMASK){
          handle(this,MKUINT(ID_EXTEND,SEL_COMMAND),NULL);
          }
        else{
          handle(this,MKUINT(ID_MARK,SEL_COMMAND),NULL);
          }
        }
      return 1;
    case KEY_Left:
    case KEY_KP_Left:
      if(!(event->state&SHIFTMASK)){
        handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
        }
      if(event->state&CONTROLMASK){
        handle(this,MKUINT(ID_CURSOR_WORD_LEFT,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_CURSOR_LEFT,SEL_COMMAND),NULL);
        }
      if(event->state&SHIFTMASK){
        handle(this,MKUINT(ID_EXTEND,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_MARK,SEL_COMMAND),NULL);
        }
      return 1;
    case KEY_Right:
    case KEY_KP_Right:
      if(!(event->state&SHIFTMASK)){
        handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
        }
      if(event->state&CONTROLMASK){
        handle(this,MKUINT(ID_CURSOR_WORD_RIGHT,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_CURSOR_RIGHT,SEL_COMMAND),NULL);
        }
      if(event->state&SHIFTMASK){
        handle(this,MKUINT(ID_EXTEND,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_MARK,SEL_COMMAND),NULL);
        }
      return 1;
    case KEY_Home:
    case KEY_KP_Home:
      if(!(event->state&SHIFTMASK)){
        handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
        }
      if(event->state&CONTROLMASK){
        handle(this,MKUINT(ID_CURSOR_TOP,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_CURSOR_HOME,SEL_COMMAND),NULL);
        }
      if(event->state&SHIFTMASK){
        handle(this,MKUINT(ID_EXTEND,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_MARK,SEL_COMMAND),NULL);
        }
      return 1;
    case KEY_End:
    case KEY_KP_End:
      if(!(event->state&SHIFTMASK)){
        handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
        }
      if(event->state&CONTROLMASK){
        handle(this,MKUINT(ID_CURSOR_BOTTOM,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_CURSOR_END,SEL_COMMAND),NULL);
        }
      if(event->state&SHIFTMASK){
        handle(this,MKUINT(ID_EXTEND,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_MARK,SEL_COMMAND),NULL);
        }
      return 1;
    case KEY_Page_Up:
    case KEY_KP_Page_Up:
      if(!(event->state&SHIFTMASK)){
        handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
        }
      handle(this,MKUINT(ID_CURSOR_PAGEUP,SEL_COMMAND),NULL);
      if(event->state&SHIFTMASK){
        handle(this,MKUINT(ID_EXTEND,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_MARK,SEL_COMMAND),NULL);
        }
      return 1;
    case KEY_Page_Down:
    case KEY_KP_Page_Down:
      if(!(event->state&SHIFTMASK)){
        handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
        }
      handle(this,MKUINT(ID_CURSOR_PAGEDOWN,SEL_COMMAND),NULL);
      if(event->state&SHIFTMASK){
        handle(this,MKUINT(ID_EXTEND,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_MARK,SEL_COMMAND),NULL);
        }
      return 1;
    case KEY_Insert:
    case KEY_KP_Insert:
      if(event->state&CONTROLMASK){
        handle(this,MKUINT(ID_COPY_SEL,SEL_COMMAND),NULL);
        }
      else if(event->state&SHIFTMASK){
        if(isEditable()){
          handle(this,MKUINT(ID_PASTE_SEL,SEL_COMMAND),NULL);
          }
        else{
          getApp()->beep();
          }
        }
      else{
        handle(this,MKUINT(ID_TOGGLE_OVERSTRIKE,SEL_COMMAND),NULL);
        }
      return 1;
    case KEY_Delete:
    case KEY_KP_Delete:
      if(isEditable()){
        if(isPosSelected(cursorpos)){
          handle(this,MKUINT(ID_DELETE_SEL,SEL_COMMAND),NULL);
          }
        else{
          handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
          if(event->state&CONTROLMASK){
            handle(this,MKUINT(ID_DELETE_WORD,SEL_COMMAND),NULL);
            }
          else if(event->state&SHIFTMASK){
            handle(this,MKUINT(ID_DELETE_EOL,SEL_COMMAND),NULL);
            }
          else{
            handle(this,MKUINT(ID_DELETE,SEL_COMMAND),NULL);
            }
          }
        }
      else{
        getApp()->beep();
        }
      return 1;
    case KEY_BackSpace:
      if(isEditable()){
        if(isPosSelected(cursorpos)){
          handle(this,MKUINT(ID_DELETE_SEL,SEL_COMMAND),NULL);
          }
        else{
         handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
         if(event->state&CONTROLMASK){
            handle(this,MKUINT(ID_BACKSPACE_WORD,SEL_COMMAND),NULL);
            }
          else if(event->state&SHIFTMASK){
            handle(this,MKUINT(ID_BACKSPACE_BOL,SEL_COMMAND),NULL);
            }
          else{
            handle(this,MKUINT(ID_BACKSPACE,SEL_COMMAND),NULL);
            }
          }
        }
      else{
        getApp()->beep();
        }
      return 1;
    case KEY_Return:
    case KEY_KP_Enter:
      if(isEditable()){
        if(isPosSelected(cursorpos)){
          handle(this,MKUINT(ID_DELETE_SEL,SEL_COMMAND),NULL);
          }
        else{
          handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
          }
        handle(this,MKUINT(ID_INSERT_NEWLINE,SEL_COMMAND),NULL);
        }
      else{
        getApp()->beep();
        }
      return 1;
    case KEY_Tab:
    case KEY_KP_Tab:
      if(isEditable()){
        if(isPosSelected(cursorpos)){
          handle(this,MKUINT(ID_DELETE_SEL,SEL_COMMAND),NULL);
          }
        else{
          handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
          }
        if(event->state&CONTROLMASK){
          handle(this,MKUINT(ID_INSERT_STRING,SEL_COMMAND),(void*)"\t");
          }
        else{
          handle(this,MKUINT(ID_INSERT_TAB,SEL_COMMAND),NULL);
          }
        }
      else{
        getApp()->beep();
        }
      return 1;
    case KEY_a:
      if(!(event->state&CONTROLMASK)) goto ins;
      handle(this,MKUINT(ID_SELECT_ALL,SEL_COMMAND),NULL);
      return 1;
    case KEY_x:
      if(!(event->state&CONTROLMASK)) goto ins;
      if(isEditable()){
        handle(this,MKUINT(ID_CUT_SEL,SEL_COMMAND),NULL);
        }
      else{
        getApp()->beep();
        }
      return 1;
    case KEY_c:
      if(!(event->state&CONTROLMASK)) goto ins;
      handle(this,MKUINT(ID_COPY_SEL,SEL_COMMAND),NULL);
      return 1;
    case KEY_v:
      if(!(event->state&CONTROLMASK)) goto ins;
      if(isEditable()){
        handle(this,MKUINT(ID_PASTE_SEL,SEL_COMMAND),NULL);
        }
      else{
        getApp()->beep();
        }
      return 1;
    default:
ins:  if((event->state&(CONTROLMASK|ALTMASK)) || ((FXuchar)event->text[0]<32)) return 0;
      if(isEditable()){
        if(isPosSelected(cursorpos)){
          handle(this,MKUINT(ID_DELETE_SEL,SEL_COMMAND),NULL);
          }
        else{
          handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
          }
        if(options&TEXT_OVERSTRIKE){
          handle(this,MKUINT(ID_OVERST_STRING,SEL_COMMAND),(void*)event->text.text());
          }
        else{
          handle(this,MKUINT(ID_INSERT_STRING,SEL_COMMAND),(void*)event->text.text());
          }
        }
      else{
        getApp()->beep();
        }
      return 1;
    }
  return 0;
  }


// Keyboard release
long FXText::onKeyRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  if(!isEnabled()) return 0;
  FXTRACE((200,"%s::onKeyRelease keysym=0x%04x state=%04x\n",getClassName(),event->code,event->state));
  if(target && target->handle(this,MKUINT(message,SEL_KEYRELEASE),ptr)) return 1;
  switch(event->code){
    case KEY_Shift_L:
    case KEY_Shift_R:
    case KEY_Control_L:
    case KEY_Control_R:
      if(mode==MOUSE_DRAG){handle(this,MKUINT(0,SEL_DRAGGED),ptr);}
      return 1;
    }
  return 0;
  }


/*******************************************************************************/

// Move cursor to top of buffer
long FXText::onCmdCursorTop(FXObject*,FXSelector,void*){
  setCursorPos(0,TRUE);
  makePositionVisible(cursorpos);
  flashMatching();
  return 1;
  }


// Move cursor to bottom of buffer
long FXText::onCmdCursorBottom(FXObject*,FXSelector,void*){
  setCursorPos(length,TRUE);
  makePositionVisible(cursorpos);
  flashMatching();
  return 1;
  }


// Move cursor to begin of line
long FXText::onCmdCursorHome(FXObject*,FXSelector,void*){
  setCursorPos(rowStart(cursorpos),TRUE);
  makePositionVisible(cursorpos);
  flashMatching();
  return 1;
  }


// Move cursor to end of line
long FXText::onCmdCursorEnd(FXObject*,FXSelector,void*){
  setCursorPos(rowEnd(cursorpos),TRUE);
  makePositionVisible(cursorpos);
  flashMatching();
  return 1;
  }


// Move cursor right
long FXText::onCmdCursorRight(FXObject*,FXSelector,void*){
  if(cursorpos>=length) return 1;
  setCursorPos(cursorpos+1,TRUE);
  makePositionVisible(cursorpos);
  flashMatching();
  return 1;
  }


// Move cursor left
long FXText::onCmdCursorLeft(FXObject*,FXSelector,void*){
  if(cursorpos<=0) return 1;
  setCursorPos(cursorpos-1,TRUE);
  makePositionVisible(cursorpos);
  flashMatching();
  return 1;
  }


// Move cursor to previous line
long FXText::onCmdCursorUp(FXObject*,FXSelector,void*){
  FXint newrow,newpos,col;
  col=(0<=prefcol) ? prefcol : cursorcol;
  newrow=prevRow(cursorpos);
  newpos=posFromIndent(newrow,col);
  setCursorPos(newpos,TRUE);
  makePositionVisible(cursorpos);
  flashMatching();
  prefcol=col;
  return 1;
  }


// Move cursor to next line
long FXText::onCmdCursorDown(FXObject*,FXSelector,void*){
  FXint newrow,newpos,col;
  col=(0<=prefcol) ? prefcol : cursorcol;
  newrow=nextRow(cursorpos);
  newpos=posFromIndent(newrow,col);
  setCursorPos(newpos,TRUE);
  makePositionVisible(cursorpos);
  flashMatching();
  prefcol=col;
  return 1;
  }


// Page down
long FXText::onCmdCursorPageDown(FXObject*,FXSelector,void*){
  FXint newrow,newpos,col;
  col=(0<=prefcol) ? prefcol : cursorcol;
  newrow=nextRow(cursorpos,(viewport_h)/font->getFontHeight());
  newpos=posFromIndent(newrow,col);
  setTopLine(nextRow(toppos,viewport_h/font->getFontHeight()));
  setCursorPos(newpos,TRUE);
  makePositionVisible(cursorpos);
  prefcol=col;
  return 1;
  }


// Page up
long FXText::onCmdCursorPageUp(FXObject*,FXSelector,void*){
  FXint newrow,newpos,col;
  col=(0<=prefcol) ? prefcol : cursorcol;
  newrow=prevRow(cursorpos,(viewport_h)/font->getFontHeight());
  newpos=posFromIndent(newrow,col);
  setTopLine(prevRow(toppos,viewport_h/font->getFontHeight()));
  setCursorPos(newpos,TRUE);
  makePositionVisible(cursorpos);
  prefcol=col;
  return 1;
  }


// Word Left
long FXText::onCmdCursorWordLeft(FXObject*,FXSelector,void*){
  setCursorPos(leftWord(cursorpos),TRUE);
  makePositionVisible(cursorpos);
  flashMatching();
  return 1;
  }


// Word Right
long FXText::onCmdCursorWordRight(FXObject*,FXSelector,void*){
  setCursorPos(rightWord(cursorpos),TRUE);
  makePositionVisible(cursorpos);
  flashMatching();
  return 1;
  }


// Cursor pos to top of screen
long FXText::onCmdCursorScreenTop(FXObject*,FXSelector,void*){
  setTopLine(cursorpos);
  return 1;
  }


// Cursor pos to bottom of screen
long FXText::onCmdCursorScreenBottom(FXObject*,FXSelector,void*){
  setBottomLine(cursorpos);
  return 1;
  }


// Cursor pos to center of screen
long FXText::onCmdCursorScreenCenter(FXObject*,FXSelector,void*){
  setCenterLine(cursorpos);
  return 1;
  }


// Scroll up one line, leaving cursor in place
long FXText::onCmdScrollUp(FXObject*,FXSelector,void*){
  setTopLine(prevRow(toppos,1));
  return 1;
  }


// Scroll down one line, leaving cursor in place
long FXText::onCmdScrollDown(FXObject*,FXSelector,void*){
  setTopLine(nextRow(toppos,1));
  return 1;
  }


// Move cursor to begin of paragraph
long FXText::onCmdCursorParHome(FXObject*,FXSelector,void*){
  setCursorPos(lineStart(cursorpos),TRUE);
  makePositionVisible(cursorpos);
  return 1;
  }


// Move cursor to end of paragraph
long FXText::onCmdCursorParEnd(FXObject*,FXSelector,void*){
  setCursorPos(lineEnd(cursorpos),TRUE);
  makePositionVisible(cursorpos);
  return 1;
  }


// Mark
long FXText::onCmdMark(FXObject*,FXSelector,void*){
  setAnchorPos(cursorpos);
  return 1;
  }


// Extend
long FXText::onCmdExtend(FXObject*,FXSelector,void*){
  extendSelection(cursorpos,SELECT_CHARS,TRUE);
  return 1;
  }


// Overstrike a string
long FXText::onCmdOverstString(FXObject*,FXSelector,void* ptr){
  register FXint sindent,oindent,nindent,pos,ch,len;
  register FXchar* string;
  string=(FXchar*)ptr;
  len=strlen(string);
  if(!isEditable()) return 1;
  sindent=0;
  pos=lineStart(cursorpos);
  while(pos<cursorpos){                         // Measure indent of cursorpos
    if(getChar(pos)=='\t')
      sindent+=(tabcolumns-sindent%tabcolumns);
    else
      sindent+=1;
    pos++;
    }
  nindent=sindent;
  pos=0;
  while(pos<len){                               // Measure indent of new string
    if(string[pos]=='\t')
      nindent+=(tabcolumns-nindent%tabcolumns);
    else
      nindent+=1;
    pos++;
    }
  oindent=sindent;
  pos=cursorpos;
  while(pos<length && (ch=getChar(pos))!='\n'){ // Measure indent of old string
    if(ch=='\t')
      oindent+=(tabcolumns-oindent%tabcolumns);
    else
      oindent+=1;
    if(oindent==nindent){                       // Same indent
      pos++;                                    // Include last character
      break;
      }
    if(oindent>nindent){                        // Greater indent
      if(ch!='\t') pos++;                       // Don't include last character if it was a tab
      break;
      }
    pos++;
    }
  //FXTRACE((100,"onCmdOverstString: sindent=%d oindent=%d nindent=%d cursorpos=%d pos=%d len=%d pad=%d\n",sindent,oindent,nindent,cursorpos,pos,len,pad));
  replaceText(cursorpos,pos-cursorpos,string,len,TRUE);
  setCursorPos(cursorpos,TRUE);
  makePositionVisible(cursorpos);
  flashMatching();
  flags|=FLAG_CHANGED;
  modified=TRUE;
  return 1;
  }


// Insert a string
long FXText::onCmdInsertString(FXObject*,FXSelector,void* ptr){
  FXchar* string=(FXchar*)ptr;
  FXint len=strlen(string);
  if(!isEditable()) return 1;
  insertText(cursorpos,string,len,TRUE);
  setCursorPos(cursorpos,TRUE);
  makePositionVisible(cursorpos);
  flashMatching();
  flags|=FLAG_CHANGED;
  modified=TRUE;
  return 1;
  }


// Insert a character
long FXText::onCmdInsertNewline(FXObject*,FXSelector,void*){
  FXint start,end,n;
  FXchar *text;
  if(!isEditable()) return 1;
  if(options&TEXT_AUTOINDENT){
    start=lineStart(cursorpos);
    end=start;
    while(end<cursorpos){
      if(!isspace(getChar(end))) break;
      end++;
      }
    n=end-start+1;
    FXMALLOC(&text,FXchar,n);
    text[0]='\n';
    extractText(&text[1],start,end-start);
    insertText(cursorpos,text,n,TRUE);
    FXFREE(&text);
    }
  else{
    insertText(cursorpos,"\n",1,TRUE);
    }
  setCursorPos(cursorpos,TRUE);
  makePositionVisible(cursorpos);
  flags|=FLAG_CHANGED;
  modified=TRUE;
  return 1;
  }



// Insert a character
long FXText::onCmdInsertTab(FXObject*,FXSelector,void*){
  FXint start,indent,n;
  FXchar *text;
  if(!isEditable()) return 1;
  if(options&TEXT_NO_TABS){
    start=lineStart(cursorpos);
    indent=0;
    while(start<cursorpos){
      if(getChar(start)=='\t')
        indent+=(tabcolumns-indent%tabcolumns);
      else
        indent+=1;
      start++;
      }
    n=tabcolumns-indent%tabcolumns;
    FXMALLOC(&text,FXchar,n);
    memset(text,' ',n);
    insertText(cursorpos,text,n,TRUE);
    FXFREE(&text);
    }
  else{
    insertText(cursorpos,"\t",1,TRUE);
    }
  setCursorPos(cursorpos,TRUE);
  makePositionVisible(cursorpos);
  flags|=FLAG_CHANGED;
  modified=TRUE;
  return 1;
  }


// Cut
long FXText::onCmdCutSel(FXObject*,FXSelector,void*){
  FXDragType types[2];
  if(selstartpos<selendpos){
    if(isEditable()){
      types[0]=stringType;
      types[1]=textType;
      if(acquireClipboard(types,2)){
        FXFREE(&clipbuffer);
        FXASSERT(selstartpos<=selendpos);
        cliplength=selendpos-selstartpos;
        FXCALLOC(&clipbuffer,FXchar,cliplength+1);
        if(!clipbuffer){
          fxwarning("%s::onCmdCutSel: out of memory\n",getClassName());
          cliplength=0;
          }
        else{
          extractText(clipbuffer,selstartpos,cliplength);
          handle(this,MKUINT(ID_DELETE_SEL,SEL_COMMAND),NULL);
          }
        }
      }
    else{
      getApp()->beep();
      }
    }
  return 1;
  }


// Copy
long FXText::onCmdCopySel(FXObject*,FXSelector,void*){
  FXDragType types[2];
  if(selstartpos<selendpos){
    types[0]=stringType;
    types[1]=textType;
    if(acquireClipboard(types,2)){
      FXFREE(&clipbuffer);
      FXASSERT(selstartpos<=selendpos);
      cliplength=selendpos-selstartpos;
      FXCALLOC(&clipbuffer,FXchar,cliplength+1);
      if(!clipbuffer){
        fxwarning("%s::onCmdCopySel: out of memory\n",getClassName());
        cliplength=0;
        }
      else{
        extractText(clipbuffer,selstartpos,cliplength);
        }
      }
    }
  return 1;
  }


// Paste
long FXText::onCmdPasteSel(FXObject*,FXSelector,void*){
  FXuchar *data; FXuint len;
  if(isEditable()){
    if(selstartpos<selendpos){
      handle(this,MKUINT(ID_DELETE_SEL,SEL_COMMAND),NULL);
      }
    if(getDNDData(FROM_CLIPBOARD,stringType,data,len)){
      FXRESIZE(&data,FXuchar,len+1); data[len]='\0';
#ifdef WIN32
      FXuint i,j;
      FXuchar c;
      for(i=j=0; j<len; j++){if((c=data[j])!='\r'){data[i++]=c;}}
      data[i]='\0';
#endif
      handle(this,MKUINT(ID_INSERT_STRING,SEL_COMMAND),(void*)data);
      FXFREE(&data);
      return 1;
      }
    }
  else{
    getApp()->beep();
    }
  return 1;
  }


// Delete selection
long FXText::onCmdDeleteSel(FXObject*,FXSelector,void*){
  if(selstartpos>=selendpos) return 1;
  removeText(selstartpos,selendpos-selstartpos,TRUE);
  killSelection(TRUE);
  setCursorPos(cursorpos,TRUE);
  makePositionVisible(cursorpos);
  flags|=FLAG_CHANGED;
  modified=TRUE;
  return 1;
  }


// Select character
long FXText::onCmdSelectChar(FXObject*,FXSelector,void*){
  setAnchorPos(cursorpos);
  extendSelection(cursorpos+1,SELECT_CHARS,TRUE);
  return 1;
  }


// Select Word
long FXText::onCmdSelectWord(FXObject*,FXSelector,void*){
  setAnchorPos(cursorpos);
  extendSelection(cursorpos,SELECT_WORDS,TRUE);
  return 1;
  }


// Select Line
long FXText::onCmdSelectLine(FXObject*,FXSelector,void*){
  setAnchorPos(cursorpos);
  extendSelection(cursorpos,SELECT_LINES,TRUE);
  return 1;
  }


// Select All
long FXText::onCmdSelectAll(FXObject*,FXSelector,void*){
  setAnchorPos(0);
  extendSelection(length,SELECT_CHARS,TRUE);
  return 1;
  }


// Deselect All
long FXText::onCmdDeselectAll(FXObject*,FXSelector,void*){
  killSelection(TRUE);
  return 1;
  }


// Backspace character
long FXText::onCmdBackspace(FXObject*,FXSelector,void*){
  if(!isEditable()) return 1;
  if(cursorpos==0){ getApp()->beep(); return 1; }
  removeText(cursorpos-1,1,TRUE);
  setCursorPos(cursorpos,TRUE);
  makePositionVisible(cursorpos);
  flags|=FLAG_CHANGED;
  modified=TRUE;
  return 1;
  }


// Backspace word
long FXText::onCmdBackspaceWord(FXObject*,FXSelector,void*){
  if(!isEditable()) return 1;
  FXint pos=leftWord(cursorpos);
  removeText(pos,cursorpos-pos,TRUE);
  setCursorPos(cursorpos,TRUE);
  makePositionVisible(cursorpos);
  flags|=FLAG_CHANGED;
  modified=TRUE;
  return 1;
  }


// Backspace bol
long FXText::onCmdBackspaceBol(FXObject*,FXSelector,void*){
  if(!isEditable()) return 1;
  FXint pos=rowStart(cursorpos);
  removeText(pos,cursorpos-pos,TRUE);
  setCursorPos(cursorpos,TRUE);
  makePositionVisible(cursorpos);
  flags|=FLAG_CHANGED;
  modified=TRUE;
  return 1;
  }


// Delete character
long FXText::onCmdDelete(FXObject*,FXSelector,void*){
  if(!isEditable()) return 1;
  if(cursorpos==length){ getApp()->beep(); return 1; }
  removeText(cursorpos,1,TRUE);
  setCursorPos(cursorpos,TRUE);
  makePositionVisible(cursorpos);
  flags|=FLAG_CHANGED;
  modified=TRUE;
  return 1;
  }


// Delete word
long FXText::onCmdDeleteWord(FXObject*,FXSelector,void*){
  if(!isEditable()) return 1;
  FXint num=rightWord(cursorpos)-cursorpos;
  removeText(cursorpos,num,TRUE);
  setCursorPos(cursorpos,TRUE);
  makePositionVisible(cursorpos);
  flags|=FLAG_CHANGED;
  modified=TRUE;
  return 1;
  }


// Delete end of line
long FXText::onCmdDeleteEol(FXObject*,FXSelector,void*){
  if(!isEditable()) return 1;
  FXint num=rowEnd(cursorpos)-cursorpos;
  removeText(cursorpos,num,TRUE);
  setCursorPos(cursorpos,TRUE);
  makePositionVisible(cursorpos);
  flags|=FLAG_CHANGED;
  modified=TRUE;
  return 1;
  }


// Delete line
long FXText::onCmdDeleteLine(FXObject*,FXSelector,void*){
  if(!isEditable()) return 1;
  FXint pos=rowStart(cursorpos);
  FXint num=nextRow(cursorpos)-pos;
  removeText(pos,num,TRUE);
  setCursorPos(cursorpos,TRUE);
  makePositionVisible(cursorpos);
  flags|=FLAG_CHANGED;
  modified=TRUE;
  return 1;
  }


// Make selected text upper case
long FXText::onCmdChangeCase(FXObject*,FXSelector sel,void*){
  register FXint i,pos,num;
  FXchar *text;
  if(!isEditable()) return 1;
  pos=selstartpos;
  num=selendpos-selstartpos;
  FXMALLOC(&text,FXchar,num);
  extractText(text,pos,num);
  if(SELID(sel)==ID_UPPER_CASE){
    for(i=0; i<num; i++) text[i]=toupper((FXuchar)text[i]);
    }
  else{
    for(i=0; i<num; i++) text[i]=tolower((FXuchar)text[i]);
    }
  replaceText(pos,num,text,num,TRUE);
  setCursorPos(cursorpos,TRUE);
  makePositionVisible(cursorpos);
  setSelection(pos,num,TRUE);
  FXFREE(&text);
  flags|=FLAG_CHANGED;
  modified=TRUE;
  return 1;
  }


// Shift text by certain amount
FXint FXText::shiftText(FXint start,FXint end,FXint amount,FXbool notify){
  FXint white,p,len,size,c;
  FXchar *text;
  if(start<0) start=0;
  if(end>length) end=length;
  FXASSERT(0<tabcolumns);
  if(start<end){
    p=start;
    white=0;
    size=0;
    while(p<end){
      c=getChar(p++);
      if(c==' '){
        white++;
        }
      else if(c=='\t'){
        white+=(tabcolumns-white%tabcolumns);
        }
      else if(c=='\n'){
        size++; white=0;
        }
      else{
        white+=amount;
        if(white<0) white=0;
        if(!(options&TEXT_NO_TABS)){ size+=(white/tabcolumns+white%tabcolumns); } else { size+=white; }
        size++;
        while(p<end){
          c=getChar(p++);
          size++;
          if(c=='\n') break;
          }
        white=0;
        }
      }
    FXMALLOC(&text,FXchar,size);
    p=start;
    white=0;
    len=0;
    while(p<end){
      c=getChar(p++);
      if(c==' '){
        white++;
        }
      else if(c=='\t'){
        white+=(tabcolumns-white%tabcolumns);
        }
      else if(c=='\n'){
        text[len++]='\n'; white=0;
        }
      else{
        white+=amount;
        if(white<0) white=0;
        if(!(options&TEXT_NO_TABS)){ while(white>=tabcolumns){ text[len++]='\t'; white-=tabcolumns;} }
        while(white>0){ text[len++]=' '; white--; }
        text[len++]=c;
        while(p<end){
          c=getChar(p++);
          text[len++]=c;
          if(c=='\n') break;
          }
        white=0;
        }
      }
    FXASSERT(len<=size);
    replaceText(start,end-start,text,len,notify);
    FXFREE(&text);
    return len;
    }
  return 0;
  }


// Shift selected lines left or right
long FXText::onCmdShiftText(FXObject*,FXSelector sel,void*){
  FXint start,end,len,amount;
  if(!isEditable()) return 1;
  amount=0;
  switch(SELID(sel)){
    case ID_SHIFT_LEFT: amount=-1; break;
    case ID_SHIFT_RIGHT: amount=1; break;
    case ID_SHIFT_TABLEFT: amount=-tabcolumns; break;
    case ID_SHIFT_TABRIGHT: amount=tabcolumns; break;
    }
  if(selstartpos<selendpos){
    FXASSERT(0<=selstartpos && selstartpos<=length);
    FXASSERT(0<=selendpos && selendpos<=length);
    start=lineStart(selstartpos);
    end=selendpos;
    if(0<end && getChar(end-1)!='\n') end=nextLine(end);
    }
  else{
    start=lineStart(cursorpos);
    end=lineEnd(cursorpos);
    if(end<length) end++;
    }
  len=shiftText(start,end,amount,TRUE);
  setAnchorPos(start);
  extendSelection(start+len,SELECT_CHARS,TRUE);
  setCursorPos(start,TRUE);
  flags|=FLAG_CHANGED;
  modified=TRUE;
  return 1;
  }


// Goto matching character
long FXText::onCmdGotoMatching(FXObject*,FXSelector,void*){
  if(0<cursorpos){
    FXchar ch=getChar(cursorpos-1);
    FXint pos=findMatching(cursorpos-1,0,length,ch,1);
    if(0<=pos){
      setCursorPos(pos+1);
      makePositionVisible(cursorpos);
      return 1;
      }
    }
  getApp()->beep();
  return 1;
  }


// Select text till matching character
long FXText::onCmdSelectMatching(FXObject*,FXSelector,void*){
  if(0<cursorpos){
    FXchar ch=getChar(cursorpos-1);
    FXint pos=findMatching(cursorpos-1,0,length,ch,1);
    if(0<=pos){
      if(pos>cursorpos){
        setAnchorPos(cursorpos-1);
        extendSelection(pos+1,SELECT_CHARS,TRUE);
        }
      else{
        setAnchorPos(pos);
        extendSelection(cursorpos,SELECT_CHARS,TRUE);
        }
      return 1;
      }
    }
  getApp()->beep();
  return 1;
  }


static const FXchar righthand[]="}])>";
static const FXchar lefthand[]="{[(<";


// Select entire enclosing block
long FXText::onCmdSelectBlock(FXObject*,FXSelector sel,void*){
  FXint beg,end,what,level=1;
  while(1){
    what=SELID(sel)-ID_SELECT_BRACE;
    beg=matchBackward(cursorpos-1,0,lefthand[what],righthand[what],level);
    end=matchForward(cursorpos,length,lefthand[what],righthand[what],level);
    if(0<=beg && beg<end){
      if(isPosSelected(beg) && isPosSelected(end+1)){ level++; continue; }
      setAnchorPos(beg);
      extendSelection(end+1,SELECT_CHARS,TRUE);
      return 1;
      }
    getApp()->beep();
    break;
    }
  return 1;
  }


// Goto start of enclosing block
long FXText::onCmdBlockBeg(FXObject*,FXSelector sel,void*){
  FXint what=SELID(sel)-ID_LEFT_BRACE;
  FXint beg=cursorpos-1;
  if(0<beg){
    if(getChar(beg)==lefthand[what]) beg--;
    FXint pos=matchBackward(beg,0,lefthand[what],righthand[what],1);
    if(0<=pos){
      setCursorPos(pos+1);
      makePositionVisible(cursorpos);
      return 1;
      }
    }
  getApp()->beep();
  return 1;
  }


// Goto end of enclosing block
long FXText::onCmdBlockEnd(FXObject*,FXSelector sel,void*){
  FXint what=SELID(sel)-ID_RIGHT_BRACE;
  FXint start=cursorpos;
  if(start<length){
    if(getChar(start)==righthand[what]) start++;
    FXint pos=matchForward(start,length,lefthand[what],righthand[what],1);
    if(0<=pos){
      setCursorPos(pos);
      makePositionVisible(cursorpos);
      return 1;
      }
    }
  getApp()->beep();
  return 1;
  }


// Search for selected text
long FXText::onCmdSearchSel(FXObject*,FXSelector sel,void*){
  FXchar *data; FXuint len;
  if(getDNDData(FROM_SELECTION,stringType,(FXuchar*&)data,len)){
    FXString searchstring(data,len);
    FXuint   searchflags;
    FXint    beg,end,pos;
    pos=cursorpos;
    FXFREE(&data);
    if(SELID(sel)==ID_SEARCH_FORW_SEL){
      if(isPosSelected(pos)) pos=selendpos;
      searchflags=SEARCH_WRAP|SEARCH_FORWARD;
      }
    else{
      if(isPosSelected(pos)) pos=selstartpos-1;
      searchflags=SEARCH_WRAP|SEARCH_BACKWARD;
      }
    if(findText(searchstring,&beg,&end,pos,searchflags)){
      if(beg!=selstartpos || end!=selendpos){
        setAnchorPos(beg);
        extendSelection(end,SELECT_CHARS,TRUE);
        setCursorPos(end);
        makePositionVisible(beg);
        makePositionVisible(end);
        return 1;
        }
      }
    }
  getApp()->beep();
  return 1;
  }


// Goto selected line number
long FXText::onCmdGotoSelected(FXObject*,FXSelector,void*){
  FXchar *data; FXuint len; FXint row,num;
  if(getDNDData(FROM_SELECTION,stringType,(FXuchar*&)data,len)){
    FXRESIZE(&data,FXchar,len+1); data[len]='\0';
    num=sscanf(data,"%d",&row);
    FXFREE(&data);
    if(num==1){
      setCursorRow(row-1,TRUE);
      makePositionVisible(cursorpos);
      return 1;
      }
    }
  getApp()->beep();
  return 1;
  }


// Search text
long FXText::onCmdSearch(FXObject*,FXSelector,void*){
  FXGIFIcon icon(getApp(),searchicon);
  FXSearchDialog searchdialog(this,"Search",&icon);
  FXint beg[10],end[10],pos;
  FXuint searchflags,code;
  FXString searchstring;
  do{
    code=searchdialog.execute();
    if(code==FXSearchDialog::DONE) return 1;
    searchflags=searchdialog.getSearchMode();
    searchstring=searchdialog.getSearchText();
    pos=isPosSelected(cursorpos) ? (searchflags&SEARCH_BACKWARD) ? selstartpos-1 : selendpos : cursorpos;
    if(findText(searchstring,beg,end,pos,searchflags|SEARCH_WRAP,10)){
      setAnchorPos(beg[0]);
      extendSelection(end[0],SELECT_CHARS,TRUE);
      setCursorPos(end[0],TRUE);
      makePositionVisible(beg[0]);
      makePositionVisible(end[0]);
      }
    else{
      getApp()->beep();
      }
    }
  while(code==FXSearchDialog::SEARCH_NEXT);
  return 1;
  }


// Replace text; we assume that findText has called squeezegap()!
long FXText::onCmdReplace(FXObject*,FXSelector,void*){
  FXGIFIcon icon(getApp(),searchicon);
  FXReplaceDialog replacedialog(this,"Replace",&icon);
  FXint beg[10],end[10],fm,to,len,pos;
  FXuint searchflags,code;
  FXString searchstring;
  FXString replacestring;
  FXString replacevalue;
  do{
    code=replacedialog.execute();
    if(code==FXReplaceDialog::DONE) return 1;
    searchflags=replacedialog.getSearchMode();
    searchstring=replacedialog.getSearchText();
    replacestring=replacedialog.getReplaceText();
    replacevalue=FXString::null;
    fm=-1;
    to=-1;
    if(code==FXReplaceDialog::REPLACE_ALL){
      searchflags&=~SEARCH_BACKWARD;
      pos=0;
      while(findText(searchstring,beg,end,pos,searchflags,10)){
        if(0<=fm) replacevalue.append(&buffer[pos],beg[0]-pos);
        replacevalue.append(FXRex::substitute(buffer,length,beg,end,replacestring,10));
        if(fm<0) fm=beg[0];
        to=end[0];
        pos=end[0];
        if(beg[0]==end[0]) pos++;
        }
      }
    else{
      pos=isPosSelected(cursorpos) ? (searchflags&SEARCH_BACKWARD) ? selstartpos-1 : selendpos : cursorpos;
      if(findText(searchstring,beg,end,pos,searchflags|SEARCH_WRAP,10)){
        replacevalue=FXRex::substitute(buffer,length,beg,end,replacestring,10);
        fm=beg[0];
        to=end[0];
        }
      }
    if(0<=fm){
      len=replacevalue.length();
      replaceText(fm,to-fm,replacevalue.text(),len,TRUE);
      setCursorPos(fm+len,TRUE);
      makePositionVisible(getCursorPos());
      modified=TRUE;
      }
    else{
      getApp()->beep();
      }
    }
  while(code==FXReplaceDialog::REPLACE_NEXT);
  return 1;
  }


// Goto line number
long FXText::onCmdGotoLine(FXObject*,FXSelector,void*){
  FXGIFIcon icon(getApp(),gotoicon);
  FXint row=cursorrow+1;
  if(FXInputDialog::getInteger(row,this,"Goto Line","&Goto line number:",&icon,1,2147483647)){
    update();
    setCursorRow(row-1,TRUE);
    makePositionVisible(cursorpos);
    }
  return 1;
  }


/*******************************************************************************/

// Editable toggle
long FXText::onCmdToggleEditable(FXObject*,FXSelector,void*){
  options^=TEXT_READONLY;
  return 1;
  }


// Update editable toggle
long FXText::onUpdToggleEditable(FXObject* sender,FXSelector,void*){
  FXuint msg=(options&TEXT_READONLY) ? ID_UNCHECK : ID_CHECK;
  sender->handle(this,MKUINT(ID_SHOW,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(msg,SEL_COMMAND),NULL);
  return 1;
  }


// Overstrike toggle
long FXText::onCmdToggleOverstrike(FXObject*,FXSelector,void*){
  options^=TEXT_OVERSTRIKE;
  return 1;
  }


// Update overstrike toggle
long FXText::onUpdToggleOverstrike(FXObject* sender,FXSelector,void*){
  FXuint msg=(options&TEXT_OVERSTRIKE) ? ID_CHECK : ID_UNCHECK;
  sender->handle(this,MKUINT(ID_SHOW,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  sender->handle(this,MKUINT(msg,SEL_COMMAND),NULL);
  return 1;
  }


// Move cursor to indicated row
long FXText::onCmdCursorRow(FXObject* sender,FXSelector,void*){
  FXint row=cursorrow+1;
  sender->handle(this,MKUINT(FXWindow::ID_GETINTVALUE,SEL_COMMAND),(void*)&row);
  setCursorRow(row-1,TRUE);
  makePositionVisible(cursorpos);
  return 1;
  }


// Being asked about current row number
long FXText::onUpdCursorRow(FXObject* sender,FXSelector,void*){
  FXint row=cursorrow+1;
  sender->handle(this,MKUINT(FXWindow::ID_SETINTVALUE,SEL_COMMAND),(void*)&row);
  return 1;
  }


// Move cursor to indicated column
long FXText::onCmdCursorColumn(FXObject* sender,FXSelector,void*){
  FXint col=cursorcol;
  sender->handle(this,MKUINT(FXWindow::ID_GETINTVALUE,SEL_COMMAND),(void*)&col);
  setCursorCol(col,TRUE);
  makePositionVisible(cursorpos);
  return 1;
  }


// Being asked about current column
long FXText::onUpdCursorColumn(FXObject* sender,FXSelector,void*){
  sender->handle(this,MKUINT(FXWindow::ID_SETINTVALUE,SEL_COMMAND),(void*)&cursorcol);
  return 1;
  }


// Update somebody who works on the selection
long FXText::onUpdHaveSelection(FXObject* sender,FXSelector,void* ptr){
  FXuint msg=(selstartpos<selendpos) ? ID_ENABLE : ID_DISABLE;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),ptr);
  return 1;
  }


/*******************************************************************************/


// Draw fragment of text in given style
void FXText::drawBufferText(FXDCWindow& dc,FXint x,FXint y,FXint,FXint,FXint pos,FXint n,FXuint style) const {
  register FXuint index=(style&STYLE_MASK);
//register FXuint usedstyle=style;                                              // Style flags from style buffer
  register FXColor color;
  FXchar str[2];
  color=0;
  if(hilitestyles && index){                                                    // Get colors from style table
//  usedstyle=hilitestyles[index-1].style;                                      // Style flags now from style table
    if(style&STYLE_SELECTED) color=hilitestyles[index-1].selectForeColor;
    else if(style&STYLE_HILITE) color=hilitestyles[index-1].hiliteForeColor;
    if(color==0) color=hilitestyles[index-1].normalForeColor;                   // Fall back on normal foreground color
    }
  if(color==0){                                                                 // Fall back to default style
    if(style&STYLE_SELECTED) color=seltextColor;
    else if(style&STYLE_HILITE) color=hilitetextColor;
    if(color==0) color=textColor;                                               // Fall back to normal text color
    }
  dc.setForeground(color);
  if(style&STYLE_CONTROL){
    y+=font->getFontAscent();
    str[0]='^';
    while(pos<gapstart && 0<n){
      str[1]=buffer[pos]|0x40;
      dc.drawText(x,y,str,2);
      x+=font->getTextWidth(str,2);
      pos++;
      n--;
      }
    while(0<n){
      str[1]=buffer[pos-gapstart+gapend]|0x40;
      dc.drawText(x,y,str,2);
      x+=font->getTextWidth(str,2);
      pos++;
      n--;
      }
    }
  else{
    y+=font->getFontAscent();
    if(pos+n<=gapstart){
      dc.drawText(x,y,&buffer[pos],n);
      }
    else if(pos>=gapstart){
      dc.drawText(x,y,&buffer[pos-gapstart+gapend],n);
      }
    else{
      dc.drawText(x,y,&buffer[pos],gapstart-pos);
      x+=font->getTextWidth(&buffer[pos],gapstart-pos);
      dc.drawText(x,y,&buffer[gapend],pos+n-gapstart);
      }
    }
  }


// Fill fragment of background in given style
void FXText::fillBufferRect(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h,FXuint style) const {
  register FXuint index=(style&STYLE_MASK);
  register FXuint usedstyle=style;                                              // Style flags from style buffer
  register FXColor bgcolor,fgcolor;
  bgcolor=fgcolor=0;
  if(hilitestyles && index){                                                    // Get colors from style table
    usedstyle=hilitestyles[index-1].style;                                      // Style flags now from style table
    if(style&STYLE_SELECTED){
      bgcolor=hilitestyles[index-1].selectBackColor;
      fgcolor=hilitestyles[index-1].selectForeColor;
      }
    else if(style&STYLE_HILITE){
      bgcolor=hilitestyles[index-1].hiliteBackColor;
      fgcolor=hilitestyles[index-1].hiliteForeColor;
      }
    else if(style&STYLE_ACTIVE){
      bgcolor=hilitestyles[index-1].activeBackColor;
      }
    else{
      bgcolor=hilitestyles[index-1].normalBackColor;
      }
    if(fgcolor==0){                                                             // Fall back to normal foreground color
      fgcolor=hilitestyles[index-1].normalForeColor;
      }
    }
  if(bgcolor==0){                                                               // Fall back to default background colors
    if(style&STYLE_SELECTED) bgcolor=selbackColor;
    else if(style&STYLE_HILITE) bgcolor=hilitebackColor;
    else if(style&STYLE_ACTIVE) bgcolor=activebackColor;
    else bgcolor=backColor;
    }
  if(fgcolor==0){                                                               // Fall back to default foreground colors
    if(style&STYLE_SELECTED) fgcolor=seltextColor;
    else if(style&STYLE_HILITE) fgcolor=hilitetextColor;
    if(fgcolor==0) fgcolor=textColor;                                           // Fall back to text color
    }
  dc.setForeground(bgcolor);
  dc.fillRectangle(x,y,w,h);
  if(usedstyle&STYLE_UNDERLINE){
    dc.setForeground(fgcolor);
    dc.fillRectangle(x,y+font->getFontAscent()+1,w,1);
    }
  if(usedstyle&STYLE_STRIKEOUT){
    dc.setForeground(fgcolor);
    dc.fillRectangle(x,y+font->getFontAscent()/2,w,1);
    }
  }


// Obtain text style at position pos; note pos may be outside of text
// to allow for rectangular selections!
FXuint FXText::style(FXint row,FXint,FXint end,FXint pos) const {
  register FXuint s=0;
  register FXchar ch;

  // Selected part of text
  if(selstartpos<=pos && pos<selendpos) s|=STYLE_SELECTED;

  // Highlighted part of text
  if(hilitestartpos<=pos && pos<hiliteendpos) s|=STYLE_HILITE;

  // Current active line
  if((row==cursorrow)&&(options&TEXT_SHOWACTIVE)) s|=STYLE_ACTIVE;

  // Blank part of line
  if(pos>=end) return s;

  // Special style for control characters
  ch=getChar(pos);

  // Get value from style buffer
  if(sbuffer) s|=getStyle(pos);

  // Tabs are just fill
  if(ch == '\t') return s;

  // Spaces are just fill
  if(ch == ' ') return s;

  // Newlines are just fill
  if(ch == '\n') return s;

  // Get special style for control codes
  if((FXuchar)ch < ' ') return s|STYLE_CONTROL|STYLE_TEXT;

  return s|STYLE_TEXT;
  }


// Draw partial text line with correct style
void FXText::drawTextRow(FXDCWindow& dc,FXint line,FXint left,FXint right) const {
  register FXint x,y,w,h,linebeg,lineend,truelineend,cw,sp,ep,row,edge;
  register FXuint curstyle,newstyle;
  linebeg=visrows[line];
  lineend=truelineend=visrows[line+1];
  if(linebeg<lineend && isspace(getChar(lineend-1))) lineend--;         // Back off last space
  x=0;
  w=0;
  h=font->getFontHeight();
  y=pos_y+margintop+(toprow+line)*h;
  edge=pos_x+marginleft+barwidth;
  row=toprow+line;

  // Scan ahead till until we hit the end or the left edge
  for(sp=linebeg; sp<lineend; sp++){
    cw=charWidth(getChar(sp),x);
    if(x+edge+cw>=left) break;
    x+=cw;
    }

  // First style to display
  curstyle=style(row,linebeg,lineend,sp);

  // Draw until we hit the end or the right edge
  for(ep=sp; ep<lineend; ep++){
    newstyle=style(row,linebeg,truelineend,ep);
    if(newstyle!=curstyle){
      fillBufferRect(dc,edge+x,y,w,h,curstyle);
      if(curstyle&STYLE_TEXT) drawBufferText(dc,edge+x,y,w,h,sp,ep-sp,curstyle);
      curstyle=newstyle;
      sp=ep;
      x+=w;
      w=0;
      }
    cw=charWidth(getChar(ep),x+w);
    if(x+edge+w>=right) break;
    w+=cw;
    }

  // Draw unfinished fragment
  fillBufferRect(dc,edge+x,y,w,h,curstyle);
  if(curstyle&STYLE_TEXT) drawBufferText(dc,edge+x,y,w,h,sp,ep-sp,curstyle);
  x+=w;

  // Fill any left-overs outside of text
  if(x+edge<right){
    curstyle=style(row,linebeg,truelineend,ep);
    fillBufferRect(dc,edge+x,y,right-edge-x,h,curstyle);
    }
  }


// Show or hide cursor
void FXText::showCursor(FXuint state){
  if((state^flags)&FLAG_CARET){
    flags^=FLAG_CARET;
    drawCursor(flags&FLAG_CARET);
    }
  }


// Draw the cursor
void FXText::drawCursor(FXuint state){
  register FXint xx,yt,yb,fh;
  if(!id()) return;
  FXASSERT(0<=cursorpos && cursorpos<=length);
  FXASSERT(0<=cursorrow && cursorrow<=nrows);
  if(toprow<=cursorrow && cursorrow<toprow+nvisrows){
    xx=pos_x+marginleft+barwidth+lineWidth(cursorstart,cursorpos-cursorstart)-1;
    if(barwidth<=xx+3 && xx-2<viewport_w){
      FXDCWindow dc(this);
      fh=font->getFontHeight();
      yt=pos_y+margintop+cursorrow*fh;
      yb=yt+fh-1;
      dc.setClipRectangle(barwidth,0,viewport_w-barwidth,viewport_h);
      if(!(state&FLAG_CARET)){

        // Repaint cursor in background to erase it
        dc.setForeground(backColor);
        dc.fillRectangle(xx,yt,2,yb-yt);
        dc.fillRectangle(xx-2,yt,6,1);
        dc.fillRectangle(xx-2,yb,6,1);

        // Clip repaint of the text to the margins
        dc.setClipRectangle(marginleft+barwidth,margintop,viewport_w-marginleft-barwidth-marginright,viewport_h-margintop-marginbottom);

        // Restore text
        dc.setTextFont(font);
        drawTextRow(dc,cursorrow-toprow,xx-3,xx+3);
        }
      else{
        dc.setForeground(cursorColor);
        dc.fillRectangle(xx,yt,2,yb-yt);
        dc.fillRectangle(xx-2,yt,6,1);
        dc.fillRectangle(xx-2,yb,6,1);
        }
      }
    }
  }


// Erase cursor overhang outside of margins
void FXText::eraseCursorOverhang(){
  register FXint xx,yt,yb,fh;
  //FXTRACE((1,"cursorpos=%d cursorrow=%d length=%d nrows=%d\n",cursorpos,cursorrow,length,nrows));
  FXASSERT(0<=cursorpos && cursorpos<=length);
  FXASSERT(0<=cursorrow && cursorrow<=nrows);
  if(toprow<=cursorrow && cursorrow<toprow+nvisrows){
    xx=pos_x+marginleft+barwidth+lineWidth(cursorstart,cursorpos-cursorstart)-1;
    if(barwidth<=xx+3 && xx-2<viewport_w){
      FXDCWindow dc(this);
      fh=font->getFontHeight();
      yt=pos_y+margintop+cursorrow*fh;
      yb=yt+fh-1;
      dc.setClipRectangle(barwidth,0,viewport_w-barwidth,viewport_h);
      if(xx-2<=marginleft+barwidth && barwidth<=xx+3){
        dc.setForeground(backColor);
        dc.fillRectangle(barwidth,yt,marginleft,fh);
        }
      if(viewport_w-marginright<=xx+3 && xx-2<=viewport_w){
        dc.setForeground(backColor);
        dc.fillRectangle(viewport_w-marginright,yt,marginright,fh);
        }
      if(yt<=margintop && 0<=yb){
        dc.setForeground(backColor);
        dc.fillRectangle(xx-2,0,5,margintop);
        }
      if(viewport_h-marginbottom<=yb && yt<viewport_h){
        dc.setForeground(backColor);
        dc.fillRectangle(xx-2,viewport_h-marginbottom,5,marginbottom);
        }
      }
    }
  }


// Repaint lines of text
void FXText::drawContents(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h) const {
  register FXint hh=font->getFontHeight();
  register FXint yy=pos_y+margintop+toprow*hh;
  register FXint tl=(y-yy)/hh;
  register FXint bl=(y+h-yy)/hh;
  register FXint ln;
  if(tl<0) tl=0;
  if(bl>=nvisrows) bl=nvisrows-1;
  for(ln=tl; ln<=bl; ln++){
    drawTextRow(dc,ln,x,x+w);
    }
  }


// Repaint line numbers
void FXText::drawNumbers(FXDCWindow& dc,FXint x,FXint y,FXint w,FXint h) const {
  register FXint hh=font->getFontHeight();
  register FXint yy=pos_y+margintop+toprow*hh;
  register FXint tl=(y-yy)/hh;
  register FXint bl=(y+h-yy)/hh;
  register FXint ln,n,tw;
  FXchar lineno[20];
  if(tl<0) tl=0;
  if(bl>=nvisrows) bl=nvisrows-1;
  dc.setForeground(barColor);
  dc.fillRectangle(x,y,w,h);
  dc.setForeground(numberColor);
  for(ln=tl; ln<=bl; ln++){
    n=sprintf(lineno,"%d",toprow+ln+1);
    tw=font->getTextWidth(lineno,n);
    dc.drawText(barwidth-tw,yy+ln*hh+font->getFontAscent(),lineno,n);
    }
  }


// Repaint text range
void FXText::updateRange(FXint beg,FXint end){
  register FXint tl,bl,fc,lc,ty,by,lx,rx,t;
  if(beg>end){t=beg;beg=end;end=t;}
  if(beg<visrows[nvisrows] && visrows[0]<end && beg<end){
    if(beg<visrows[0]) beg=visrows[0];
    if(end>visrows[nvisrows]) end=visrows[nvisrows];
    tl=posToLine(beg,0);
    bl=posToLine(end,tl);
    if(tl==bl){
      fc=beg-visrows[tl];
      lc=end-visrows[tl];
      ty=pos_y+margintop+(toprow+tl)*font->getFontHeight();
      by=ty+font->getFontHeight();
      lx=pos_x+marginleft+barwidth+lineWidth(visrows[tl],fc);
      if(end<=(visrows[tl+1]-1)) rx=pos_x+marginleft+barwidth+lineWidth(visrows[tl],lc); else rx=width;
      }
    else{
      ty=pos_y+margintop+(toprow+tl)*font->getFontHeight();
      by=pos_y+margintop+(toprow+bl+1)*font->getFontHeight();
      lx=barwidth;
      rx=width;
      }
    update(lx,ty,rx-lx,by-ty);
    }
  }


// Draw item list
long FXText::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXDCWindow dc(this,event);
  dc.setTextFont(font);
//dc.setForeground(FXRGB(255,0,0));
//dc.fillRectangle(event->rect.x,event->rect.y,event->rect.w,event->rect.h);

  // Paint top margin
  if(event->rect.y<=margintop){
    dc.setForeground(backColor);
    dc.fillRectangle(barwidth,0,viewport_w-barwidth,margintop);
    }

  // Paint bottom margin
  if(event->rect.y+event->rect.h>=viewport_h-marginbottom){
    dc.setForeground(backColor);
    dc.fillRectangle(barwidth,viewport_h-marginbottom,viewport_w-barwidth,marginbottom);
    }

  // Paint left margin
  if(event->rect.x<barwidth+marginleft){
    dc.setForeground(backColor);
    dc.fillRectangle(barwidth,margintop,marginleft,viewport_h-margintop-marginbottom);
    }

  // Paint right margin
  if(event->rect.x+event->rect.w>=viewport_w-marginright){
    dc.setForeground(backColor);
    dc.fillRectangle(viewport_w-marginright,margintop,marginright,viewport_h-margintop-marginbottom);
    }

  // Paint line numbers
  if(event->rect.x<barwidth){
    dc.setClipRectangle(0,0,barwidth,height);
    drawNumbers(dc,event->rect.x,event->rect.y,event->rect.w,event->rect.h);
    }

  // Paint text
  dc.setClipRectangle(marginleft+barwidth,margintop,viewport_w-marginright-marginleft-barwidth,viewport_h-margintop-marginbottom);
  drawContents(dc,event->rect.x,event->rect.y,event->rect.w,event->rect.h);

  drawCursor(flags);
  return 1;
  }


/*******************************************************************************/


// Move the cursor
void FXText::setCursorPos(FXint pos,FXbool notify){
  register FXint cursorstartold,cursorendold;
  if(pos>length) pos=length;
  if(pos<0) pos=0;
  if(cursorpos!=pos){
    showCursor(0);
    if(pos<cursorstart || cursorend<=pos){    // Move to other line?
      cursorstartold=cursorstart;
      cursorendold=cursorend;
      cursorstart=rowStart(pos);
      cursorend=nextRow(cursorstart);
      if(cursorstart<cursorstartold){
        cursorrow=cursorrow-countRows(cursorstart,cursorstartold);
        }
      else{
        cursorrow=cursorrow+countRows(cursorstartold,cursorstart);
        }
      if(options&TEXT_SHOWACTIVE){
        updateRange(cursorstartold,cursorendold);
        updateRange(cursorstart,cursorend);
        }
      }
    cursorcol=indentFromPos(cursorstart,pos);
    cursorpos=pos;
    FXTRACE((150,"setCursorPos(%d): cursorpos=%d cursorrow=%d cursorcol=%d cursorstartold=%d cursorstart=%d\n",pos,cursorpos,cursorrow,cursorcol,cursorstartold,cursorstart));
    showCursor(FLAG_CARET);
    prefcol=-1;
    if(target && notify){
      target->handle(this,MKUINT(message,SEL_CHANGED),(void*)(FXival)cursorpos);
      }
    }
  }


// Set cursor row
void FXText::setCursorRow(FXint row,FXbool notify){
  FXint col,newrow,newpos;
  if(row!=cursorrow){
    if(row<0) row=0;
    if(row>=nrows) row=nrows-1;
    col=(0<=prefcol) ? prefcol : cursorcol;
    if(row>cursorrow){
      newrow=nextRow(cursorpos,row-cursorrow);
      }
    else{
      newrow=prevRow(cursorpos,cursorrow-row);
      }
    newpos=posFromIndent(newrow,col);
    setCursorPos(newpos,notify);
    prefcol=col;
    }
  }


// Set cursor column
void FXText::setCursorCol(FXint col,FXbool notify){
  FXint newpos;
  if(cursorcol!=col){
    newpos=posFromIndent(cursorstart,col);
    setCursorPos(newpos,notify);
    }
  }


// Set anchor position
void FXText::setAnchorPos(FXint pos){
  if(pos>length) pos=length;
  if(pos<0) pos=0;
  anchorpos=pos;
  }


// Select all text
FXbool FXText::selectAll(FXbool notify){
  return setSelection(0,length,notify);
  }


// Extend selection
FXbool FXText::extendSelection(FXint pos,FXTextSelectionMode select,FXbool notify){
  FXint sp,ep;

  // Validate position
  if(pos<0) pos=0;
  if(pos>length) pos=length;

  // Did position change?
  switch(select){

    // Selecting words
    case SELECT_WORDS:
      if(pos<=anchorpos){
        sp=wordStart(pos);
        ep=wordEnd(anchorpos);
        }
      else{
        sp=wordStart(anchorpos);
        ep=wordEnd(pos);
        }
      break;

    // Selecting lines
    case SELECT_LINES:
      if(pos<=anchorpos){
        sp=rowStart(pos);
        ep=nextRow(anchorpos);
        }
      else{
        sp=rowStart(anchorpos);
        ep=nextRow(pos);
        }
      break;

    // Selecting characters
    default:
      if(pos<=anchorpos){
        sp=pos;
        ep=anchorpos;
        }
      else{
        sp=anchorpos;
        ep=pos;
        }
      break;
    }

  // Select the new range
  return setSelection(sp,ep-sp,notify);
  }


// Set selection
FXbool FXText::setSelection(FXint pos,FXint len,FXbool notify){
  FXDragType types[2];
  FXint what[2];
  FXint ep=pos+len;
  FXint sp=pos;

  // Validate position
  if(sp<0) sp=0;
  if(ep<0) ep=0;
  if(sp>length) sp=length;
  if(ep>length) ep=length;

  // Something changed?
  if(selstartpos!=sp || selendpos!=ep){

    // Release selection
    if(sp==ep){
      if(notify && target){
        what[0]=selstartpos;
        what[1]=selendpos-selstartpos;
        target->handle(this,MKUINT(message,SEL_DESELECTED),(void*)what);
        }
      if(hasSelection()) releaseSelection();
      }

    // Minimally update
    if(ep<=selstartpos || selendpos<=sp){
      updateRange(selstartpos,selendpos);
      updateRange(sp,ep);
      }
    else{
      updateRange(sp,selstartpos);
      updateRange(selendpos,ep);
      }

    selstartpos=sp;
    selendpos=ep;

    // Acquire selection
    if(sp!=ep){
      types[0]=stringType;
      types[1]=textType;
      if(!hasSelection()) acquireSelection(types,2);
      if(notify && target){
        what[0]=selstartpos;
        what[1]=selendpos-selstartpos;
        target->handle(this,MKUINT(message,SEL_SELECTED),(void*)what);
        }
      }
    return TRUE;
    }
  return FALSE;
  }


// Kill the selection
FXbool FXText::killSelection(FXbool notify){
  FXint what[2];
  if(selstartpos<selendpos){
    if(notify && target){
      what[0]=selstartpos;
      what[1]=selendpos-selstartpos;
      target->handle(this,MKUINT(message,SEL_DESELECTED),(void*)what);
      }
    if(hasSelection()) releaseSelection();
    updateRange(selstartpos,selendpos);
    selstartpos=0;
    selendpos=0;
    return TRUE;
    }
  return FALSE;
  }


// Set highlight
FXbool FXText::setHighlight(FXint pos,FXint len){
  FXint he=pos+len;
  FXint hs=pos;

  // Validate
  if(hs<0) hs=0;
  if(he<0) he=0;
  if(hs>length) hs=length;
  if(he>length) he=length;

  // Anything changed?
  if(hs!=hilitestartpos || he!=hiliteendpos){

    // Minimally update
    if(he<=hilitestartpos || hiliteendpos<=hs){
      updateRange(hilitestartpos,hiliteendpos);
      updateRange(hs,he);
      }
    else{
      updateRange(hs,hilitestartpos);
      updateRange(hiliteendpos,he);
      }

    // Keep new range
    hilitestartpos=hs;
    hiliteendpos=he;

    return TRUE;
    }
  return FALSE;
  }


// Unhighlight the text
FXbool FXText::killHighlight(){
  if(hilitestartpos<hiliteendpos){
    updateRange(hilitestartpos,hiliteendpos);
    hilitestartpos=0;
    hiliteendpos=0;
    return TRUE;
    }
  return FALSE;
  }


/*******************************************************************************/


// Change top margin
void FXText::setMarginTop(FXint mt){
  if(margintop!=mt){
    margintop=mt;
    recalc();
    update();
    }
  }

// Change bottom margin
void FXText::setMarginBottom(FXint mb){
  if(marginbottom!=mb){
    marginbottom=mb;
    recalc();
    update();
    }
  }


// Change left margin
void FXText::setMarginLeft(FXint ml){
  if(marginleft!=ml){
    marginleft=ml;
    recalc();
    update();
    }
  }


// Change right margin
void FXText::setMarginRight(FXint mr){
  if(marginright!=mr){
    marginright=mr;
    recalc();
    update();
    }
  }


// Change the font
void FXText::setFont(FXFont* fnt){
  if(!fnt){ fxerror("%s::setFont: NULL font specified.\n",getClassName()); }
  if(font!=fnt){
    font=fnt;
    recalc();
    tabwidth=tabcolumns*font->getTextWidth(" ",1);
    barwidth=barcolumns*font->getTextWidth("8",1);
    if(options&TEXT_FIXEDWRAP){ wrapwidth=wrapcolumns*font->getTextWidth(" ",1); }
    layout();
    update();
    }
  }


// Set wrap columns
void FXText::setWrapColumns(FXint cols){
  if(cols<=0) cols=1;
  if(cols!=wrapcolumns){
    wrapcolumns=cols;
    if(options&TEXT_FIXEDWRAP){ wrapwidth=wrapcolumns*font->getTextWidth(" ",1); }
    recalc();
    update();
    }
  }


// Set tab columns
void FXText::setTabColumns(FXint cols){
  if(cols<=0) cols=1;
  if(cols!=tabcolumns){
    tabcolumns=cols;
    tabwidth=tabcolumns*font->getTextWidth(" ",1);
    recalc();
    update();
    }
  }

// Change number of columns used for line numbers
void FXText::setBarColumns(FXint cols){
  if(cols<=0) cols=0;
  if(cols!=barcolumns){
    barcolumns=cols;
    barwidth=barcolumns*font->getTextWidth("8",1);
    recalc();
    update();
    }
  }

// Set text color
void FXText::setTextColor(FXColor clr){
  if(clr!=textColor){
    textColor=clr;
    update(barwidth,0,width-barwidth,height);
    }
  }


// Set select background color
void FXText::setSelBackColor(FXColor clr){
  if(clr!=selbackColor){
    selbackColor=clr;
    updateRange(selstartpos,selendpos);
    }
  }


// Set selected text color
void FXText::setSelTextColor(FXColor clr){
  if(clr!=seltextColor){
    seltextColor=clr;
    updateRange(selstartpos,selendpos);
    }
  }


// Change highlighted text color
void FXText::setHiliteTextColor(FXColor clr){
  if(clr!=hilitetextColor){
    hilitetextColor=clr;
    updateRange(hilitestartpos,hiliteendpos);
    }
  }


// Change highlighted background color
void FXText::setHiliteBackColor(FXColor clr){
  if(clr!=hilitebackColor){
    hilitebackColor=clr;
    updateRange(hilitestartpos,hiliteendpos);
    }
  }


// Change active background color
void FXText::setActiveBackColor(FXColor clr){
  if(clr!=activebackColor){
    activebackColor=clr;
    update(barwidth,0,width-barwidth,height);
    }
  }

// Change line number color
void FXText::setNumberColor(FXColor clr){
  if(clr!=numberColor){
    numberColor=clr;
    update(0,0,barwidth,height);
    }
  }


// Change bar color
void FXText::setBarColor(FXColor clr){
  if(clr!=barColor){
    barColor=clr;
    update(0,0,barwidth,height);
    }
  }


// Set cursor color
void FXText::setCursorColor(FXColor clr){
  if(clr!=cursorColor){
    showCursor(0);
    cursorColor=clr;
    showCursor(FLAG_CARET);
    }
  }


// Change text style
void FXText::setTextStyle(FXuint style){
  FXuint opts=(options&~TEXT_MASK) | (style&TEXT_MASK);
  if(options!=opts){
    options=opts;
    if(options&TEXT_FIXEDWRAP){ wrapwidth=wrapcolumns*font->getTextWidth(" ",1); }
    recalc();
    update();
    }
  }


// Get text style
FXuint FXText::getTextStyle() const {
  return (options&TEXT_MASK);
  }


// Return true if editable
FXbool FXText::isEditable() const {
  return (options&TEXT_READONLY)==0;
  }


// Set widget is editable or not
void FXText::setEditable(FXbool edit){
  if(edit) options&=~TEXT_READONLY; else options|=TEXT_READONLY;
  }


// Set styled text mode
void FXText::setStyled(FXbool styled){
  if(styled && !sbuffer){
    if(!FXCALLOC(&sbuffer,FXchar,length+gapend-gapstart)){fxerror("%s::setStyled: out of memory.\n",getClassName());}
    update();
    }
  if(!styled && sbuffer){
    FXFREE(&sbuffer);
    update();
    }
  }


// Set highlight styles
void FXText::setHiliteStyles(const FXHiliteStyle* styles){
  hilitestyles=styles;
  update();
  }


// Change number of visible rows
void FXText::setVisRows(FXint rows){
  if(rows<0) rows=0;
  if(vrows!=rows){
    vrows=rows;
    recalc();
    }
  }


// Change number of visible columns
void FXText::setVisCols(FXint cols){
  if(cols<0) cols=0;
  if(vcols!=cols){
    vcols=cols;
    recalc();
    }
  }


/*******************************************************************************/


// Update value from a message
long FXText::onCmdSetStringValue(FXObject*,FXSelector,void* ptr){
  if(ptr==NULL){ fxerror("%s::onCmdSetStringValue: NULL pointer.\n",getClassName()); }
  setText(*((FXString*)ptr));
  return 1;
  }


// Obtain value from text
long FXText::onCmdGetStringValue(FXObject*,FXSelector,void* ptr){
  if(ptr==NULL){ fxerror("%s::onCmdGetStringValue: NULL pointer.\n",getClassName()); }
  *((FXString*)ptr)=getText();
  return 1;
  }


/*******************************************************************************/


// Save object to stream
void FXText::save(FXStream& store) const {
  FXScrollArea::save(store);
  store << length;
  store.save(buffer,gapstart);
  store.save(buffer+gapend,length-gapstart);
  store << nvisrows;
  store.save(visrows,nvisrows+1);
  store << wrapcolumns;
  store << tabcolumns;
  store << margintop;
  store << marginbottom;
  store << marginleft;
  store << marginright;
  store << delimiters;
  store << font;
  store << textColor;
  store << selbackColor;
  store << seltextColor;
  store << hilitebackColor;
  store << hilitetextColor;
  store << cursorColor;
  store << help;
  store << tip;
  store << matchtime;
  }


// Load object from stream
void FXText::load(FXStream& store){
  FXScrollArea::load(store);
  store >> length;
  FXMALLOC(&buffer,FXchar,length+MINSIZE);    // FIXME should we save text&style?
  store.load(buffer,length);
  gapstart=length;
  gapend=length+MINSIZE;
  store >> nvisrows;
  FXMALLOC(&visrows,FXint,nvisrows+1);
  store.load(visrows,nvisrows+1);
  store >> wrapcolumns;
  store >> tabcolumns;
  store >> margintop;
  store >> marginbottom;
  store >> marginleft;
  store >> marginright;
  store >> delimiters;
  store >> font;
  store >> textColor;
  store >> selbackColor;
  store >> seltextColor;
  store >> hilitebackColor;
  store >> hilitetextColor;
  store >> cursorColor;
  store >> help;
  store >> tip;
  store >> matchtime;
  }


// Clean up
FXText::~FXText(){
  if(blinker) getApp()->removeTimeout(blinker);
  if(flasher) getApp()->removeTimeout(flasher);
  FXFREE(&buffer);
  FXFREE(&sbuffer);
  FXFREE(&visrows);
  FXFREE(&clipbuffer);
  buffer=(FXchar*)-1;
  sbuffer=(FXchar*)-1;
  clipbuffer=(FXchar*)-1;
  visrows=(FXint*)-1;
  font=(FXFont*)-1;
  hilitestyles=(FXHiliteStyle*)-1;
  blinker=(FXTimer*)-1;
  flasher=(FXTimer*)-1;
  }
