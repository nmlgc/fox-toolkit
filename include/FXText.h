/********************************************************************************
*                                                                               *
*                    M u l t i - L i ne   T e x t   O b j e c t                 *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998 by Jeroen van der Zijp.   All Rights Reserved.             *
*********************************************************************************
* This library is free software; you can redistribute it and/or                 *
* modify it under the terms of the GNU Library General Public                   *
* License as published by the Free Software Foundation; either                  *
* version 2 of the License, or (at your option) any later version.              *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU             *
* Library General Public License for more details.                              *
*                                                                               *
* You should have received a copy of the GNU Library General Public             *
* License along with this library; if not, write to the Free                    *
* Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            *
*********************************************************************************
* $Id: FXText.h,v 1.39 1998/10/27 22:57:34 jvz Exp $                         *
********************************************************************************/
#ifndef FXTEXT_H
#define FXTEXT_H


// Text widget options
enum FXTextStyle {
  TEXT_READONLY      = 0x00100000,             // Text is NOT editable
  TEXT_TABPIXELS     = 0x00200000,             // Tabs are in terms of pixels
  TEXT_MASK          = TEXT_READONLY|TEXT_TABPIXELS
  };

  
// Selection modes
enum FXTextSelectionMode {
  SELECT_CHARS,
  SELECT_WORDS,
  SELECT_LINES
  };

  
// Base of list
class FXText : public FXScrollArea {
  FXDECLARE(FXText)
protected:
  FXchar       *buffer;               // Text buffer being edited
  FXint        *lines;                // Starts of lines in buffer
  FXint         length;               // Length of the actual text in the buffer.
  FXint         nlines;               // Number of lines
  FXint         nvislines;            // Number of visible lines
  FXint         gapstart;             // Start of the insertion point (the gap)
  FXint         gapend;               // End of the insertion point+1
  FXint         topline;              // Begin of first visible line
  FXint         toplineno;            // Line number of first line
  FXint         topy;                 // Y coordinate of first visible line
  FXint         selstartpos;          // Start of selection
  FXint         selendpos;            // End of selection
  FXint         anchorpos;            // Anchor position
  FXint         cursorpos;            // Cursor position
  FXint         cursorline;           // Cursor line start
  FXint         cursorlineno;         // Cursor line number
  FXint         cursorcol;            // Cursor column
  FXint         cursorx;              // X position of cursor
  FXint         cursory;              // Y position of cursor
  FXint         cursorprefx;          // Preferred cursor coordinate
  FXint         margintop;            // Margins top
  FXint         marginbottom;         // Margin bottom
  FXint         marginleft;           // Margin left
  FXint         marginright;          // Margin right
  FXFont       *font;                 // Text font
  FXPixel       textColor;            // Normal text color
  FXPixel       selbackColor;         // Select background color
  FXPixel       seltextColor;         // Select text color
  FXPixel       baseColor;            // Back color
  FXPixel       cursorColor;          // Cursor color
  FXTimer      *blinker;              // Timer to blick cursor
  FXint         textWidth;            // Total width of all text
  FXint         textHeight;           // Total height of all text
  FXint         tabdist;              // Tab distance
  FXString      help;                 // Status line help
  FXString      tip;                  // Tooltip
  FXbool        modified;             // User has modified text
  FXuint        dragmode;             // Drag mode
  FXint         grabx;                // Grab point x
  FXint         graby;                // Grab point y
  
protected:
  static const FXchar delimiters[];   // Word delimiters
  static FXDragType   textDragType;   // Drag type
  static FXDragType   textDeleteType; // Delete pseudo drag type
  
protected:
  FXText(){}
  FXText(const FXText&){}
  void recompute();
  void calcLines(FXint s,FXint e);
  void drawCursor(FXuint state);
  void drawBufString(FXint x,FXint y,FXint w,FXint h,FXint pos,FXint n,FXuint style);
  void drawTextLine(FXint line,FXint lclip,FXint rclip,FXint fc,FXint tc);
//   void drawTextRange(FXint beg,FXint end);
  void drawTextRectangle(FXint x,FXint y,FXint w,FXint h);
  FXint posToLine(FXint pos) const;
  FXbool posVisible(FXint pos) const;
  void updateRange(FXint beg,FXint end);
  void updateChanged(FXint pos,FXint ncinserted,FXint ncdeleted,FXint nlinserted,FXint nldeleted);
  void findTopLine();
  void movegap(FXint pos);
  void sizegap(FXint sz);
  FXchar getChar(FXint pos) const;
  void setChar(FXint pos,FXchar ch);
  void insert(FXint pos,const FXchar *text,FXint n);
  void remove(FXint pos,FXint n);
  void extract(FXchar *text,FXint pos,FXint n) const;
  FXint findf(FXchar c,FXint pos=0) const;
  FXint findb(FXchar c,FXint pos=0) const;
  FXuint getStyleAt(FXint linepos,FXint linelen,FXint column) const;
  FXint charWidth(FXchar ch,FXint indent) const;
  virtual void layout();
public:
  
  // System messages
  long onPaint(FXObject*,FXSelector,void*);
  long onFocusIn(FXObject*,FXSelector,void*);
  long onFocusOut(FXObject*,FXSelector,void*);
  long onLeftBtnPress(FXObject*,FXSelector,void*);
  long onLeftBtnRelease(FXObject*,FXSelector,void*);
  long onMiddleBtnPress(FXObject*,FXSelector,void*);
  long onMiddleBtnRelease(FXObject*,FXSelector,void*);
  long onRightBtnPress(FXObject*,FXSelector,void*);
  long onRightBtnRelease(FXObject*,FXSelector,void*);
  long onMotion(FXObject*,FXSelector,void*);
  long onDNDEnter(FXObject*,FXSelector,void*);
  long onDNDLeave(FXObject*,FXSelector,void*);
  long onDNDMotion(FXObject*,FXSelector,void*);
  long onDNDDrop(FXObject*,FXSelector,void*);
  long onSelectionLost(FXObject*,FXSelector,void*);
  long onSelectionGained(FXObject*,FXSelector,void*);
  long onSelectionRequest(FXObject*,FXSelector,void* ptr);
  long onKeyPress(FXObject*,FXSelector,void*);
  long onKeyRelease(FXObject*,FXSelector,void*);
  long onBlink(FXObject*,FXSelector,void*);
  long onAutoScroll(FXObject*,FXSelector,void*);
  long onQueryHelp(FXObject*,FXSelector,void*);
  long onQueryTip(FXObject*,FXSelector,void*);
  long onChanged(FXObject*,FXSelector,void*);
  long onSelected(FXObject*,FXSelector,void*);
  long onDeselected(FXObject*,FXSelector,void*);
  long onInserted(FXObject*,FXSelector,void*);
  long onDeleted(FXObject*,FXSelector,void*);
  long onCmdToggleEditable(FXObject*,FXSelector,void*);
  long onUpdToggleEditable(FXObject*,FXSelector,void*);
  long onCmdCursorLine(FXObject*,FXSelector,void*);
  long onUpdCursorLine(FXObject*,FXSelector,void*);
  long onCmdCursorColumn(FXObject*,FXSelector,void*);
  long onUpdCursorColumn(FXObject*,FXSelector,void*);
  long onUpdHaveSelection(FXObject*,FXSelector,void*);
  
  // Cursor movement
  long onCmdCursorTop(FXObject*,FXSelector,void*);
  long onCmdCursorBottom(FXObject*,FXSelector,void*);
  long onCmdCursorHome(FXObject*,FXSelector,void*);
  long onCmdCursorEnd(FXObject*,FXSelector,void*);
  long onCmdCursorRight(FXObject*,FXSelector,void*);
  long onCmdCursorLeft(FXObject*,FXSelector,void*);
  long onCmdCursorUp(FXObject*,FXSelector,void*);
  long onCmdCursorDown(FXObject*,FXSelector,void*);
  long onCmdCursorWordLeft(FXObject*,FXSelector,void*);
  long onCmdCursorWordRight(FXObject*,FXSelector,void*);
  long onCmdCursorPageDown(FXObject*,FXSelector,void*);
  long onCmdCursorPageUp(FXObject*,FXSelector,void*);
  long onCmdCursorScreenTop(FXObject*,FXSelector,void*);
  long onCmdCursorScreenBottom(FXObject*,FXSelector,void*);
  long onCmdCursorScreenCenter(FXObject*,FXSelector,void*);
  
  // Mark and extend
  long onCmdMark(FXObject*,FXSelector,void*);
  long onCmdExtend(FXObject*,FXSelector,void*);
  
  // Inserting
  long onCmdInsertChar(FXObject*,FXSelector,void*);
  long onCmdInsertString(FXObject*,FXSelector,void*);
  long onCmdInsertNewline(FXObject*,FXSelector,void*);
  
  // Manipulation Selection
  long onCmdCutSel(FXObject*,FXSelector,void*);
  long onCmdCopySel(FXObject*,FXSelector,void*);
  long onCmdPasteSel(FXObject*,FXSelector,void*);
  long onCmdDeleteSel(FXObject*,FXSelector,void*);
  
  // Changing Selection
  long onCmdSelectChar(FXObject*,FXSelector,void*);
  long onCmdSelectWord(FXObject*,FXSelector,void*);
  long onCmdSelectLine(FXObject*,FXSelector,void*);
  long onCmdSelectAll(FXObject*,FXSelector,void*);
  long onCmdDeselectAll(FXObject*,FXSelector,void*);
  
  // Deletion
  long onCmdBackspace(FXObject*,FXSelector,void*);
  long onCmdBackspaceWord(FXObject*,FXSelector,void*);
  long onCmdBackspaceBol(FXObject*,FXSelector,void*);
  long onCmdDelete(FXObject*,FXSelector,void*);
  long onCmdDeleteWord(FXObject*,FXSelector,void*);
  long onCmdDeleteEol(FXObject*,FXSelector,void*);
  long onCmdDeleteLine(FXObject*,FXSelector,void*);
public:
  enum{
    ID_CURSOR_TOP=FXScrollArea::ID_LAST,
    ID_CURSOR_BOTTOM,
    ID_CURSOR_HOME,
    ID_CURSOR_END,
    ID_CURSOR_RIGHT,
    ID_CURSOR_LEFT,
    ID_CURSOR_UP,
    ID_CURSOR_DOWN,
    ID_CURSOR_WORD_LEFT,
    ID_CURSOR_WORD_RIGHT,
    ID_CURSOR_PAGEDOWN,
    ID_CURSOR_PAGEUP,
    ID_CURSOR_SCRNTOP,
    ID_CURSOR_SCRNBTM,
    ID_CURSOR_SCRNCTR,
    ID_MARK,
    ID_EXTEND,
    ID_INSERT_CHAR,
    ID_INSERT_STRING,
    ID_INSERT_NEWLINE,
    ID_CUT_SEL,
    ID_COPY_SEL,
    ID_PASTE_SEL,
    ID_DELETE_SEL,
    ID_SELECT_CHAR,
    ID_SELECT_WORD,
    ID_SELECT_LINE,
    ID_SELECT_ALL,
    ID_DESELECT_ALL,
    ID_BACKSPACE,
    ID_BACKSPACE_WORD,
    ID_BACKSPACE_BOL,
    ID_DELETE,
    ID_DELETE_WORD,
    ID_DELETE_EOL,
    ID_DELETE_LINE,
    ID_TOGGLE_EDITABLE,
    ID_CURSOR_LINE,
    ID_CURSOR_COLUMN,
    ID_LAST
    };
public:
  static const FXchar textDragTypeName[];
  static const FXchar textDeleteTypeName[];
public:
  FXText(FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  static FXbool isdelim(FXchar ch);
  virtual void create();
  virtual void enable();
  virtual void disable();
  virtual void recalc();
  virtual FXint getContentWidth();
  virtual FXint getContentHeight();
  virtual FXbool canFocus() const;
  void moveContents(FXint x,FXint y);
  FXint getMarginTop() const { return margintop; }
  FXint getMarginBottom() const { return marginbottom; }
  FXint getMarginLeft() const { return marginleft; }
  FXint getMarginRight() const { return marginright; }
  FXint getLength() const { return length; }
  FXbool isModified() const { return modified; }
  void setModified(FXbool mod=TRUE){ modified=mod; }
  FXbool isEditable() const;
  void setEditable(FXbool edit=TRUE);
  void setMarginTop(FXint pt);
  void setMarginBottom(FXint pb);
  void setMarginLeft(FXint pl);
  void setMarginRight(FXint pr);
  FXint lineWidth(FXint pos,FXint n) const;
  FXint lineHeight(FXint pos,FXint n) const;
  void setFont(FXFont* fnt);
  FXFont* getFont() const { return font; }
  FXPixel getTextColor() const { return textColor; }
  void setTextColor(FXPixel clr);
  FXPixel getSelBackColor() const { return selbackColor; }
  void setSelBackColor(FXPixel clr);
  FXPixel getSelTextColor() const { return seltextColor; }
  void setSelTextColor(FXPixel clr);
  FXPixel getCursorColor() const { return cursorColor; }
  void setCursorColor(FXPixel clr);
  void setHelpText(const FXchar* text);
  const FXchar* getHelpText() const { return help; }
  void setTipText(const FXchar* text);
  const FXchar* getTipText() const { return tip; }
  void setText(const FXchar* text);
  const FXchar *getText();
  FXbool isPosSelected(FXint pos) const;
  FXint getPosAt(FXint x,FXint y) const;
  FXint getYOfPos(FXint pos) const;
  FXint getXOfPos(FXint pos) const;
  FXint getLineOfPos(FXint pos) const;
  FXint getColumnOfPos(FXint pos) const;
  FXint forwardNLines(FXint pos,FXint nlines=1) const;
  FXint backwardNLines(FXint pos,FXint nlines=1) const;
  FXint lineStart(FXint pos) const;
  FXint lineEnd(FXint pos) const;
  FXint leftWord(FXint pos) const;
  FXint rightWord(FXint pos) const;
  FXint wordStart(FXint pos) const;
  FXint wordEnd(FXint pos) const;
  FXint validPos(FXint pos) const;
  void setTopLine(FXint pos);
  void setBottomLine(FXint pos);
  void setCenterLine(FXint pos);
  FXint getTopLine() const;
  FXint getBottomLine() const;
  void setAnchorPos(FXint pos);
  FXint getAnchorPos() const { return anchorpos; }
  void setCursorPos(FXint pos);
  FXint getCursorPos() const { return cursorpos; }
  FXbool extendSelection(FXint pos,FXTextSelectionMode mode=SELECT_CHARS); 
  FXbool killSelection(); 
  void makePositionVisible(FXint pos);
  void setTextStyle(FXuint style);
  FXuint getTextStyle() const;
  virtual void save(FXStream& store) const;
  virtual void load(FXStream& store);
  virtual ~FXText();
  };



#endif
