/********************************************************************************
*                                                                               *
*                            T a b l e   W i d g e t                            *
*                                                                               *
*********************************************************************************
* Copyright (C) 1999,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXTable.h,v 1.79 2002/01/18 22:42:55 jeroen Exp $                        *
********************************************************************************/
#ifndef FXTABLE_H
#define FXTABLE_H

#ifndef FXSCROLLAREA_H
#include "FXScrollArea.h"
#endif



//////////////////////////////  UNDER DEVELOPMENT  //////////////////////////////

struct FXTimer;
class FXIcon;
class FXFont;
class FXTable;


/// Default cell margin
#define DEFAULT_MARGIN  2



/// Table options
enum {
  TABLE_COL_SIZABLE   = 0x00100000,   /// Columns are resizable
  TABLE_ROW_SIZABLE   = 0x00200000,   /// Rows are resizable
  TABLE_NO_COLSELECT  = 0x00400000,   /// Disallow column selections
  TABLE_NO_ROWSELECT  = 0x00800000    /// Disallow row selections
  };


/// Position in table
struct FXTablePos {
  FXint  row;
  FXint  col;
  };


/// Range of table cells
struct FXTableRange {
  FXTablePos fm;
  FXTablePos to;
  };


/// Item in table
class FXAPI FXTableItem : public FXObject {
  FXDECLARE(FXTableItem)
  friend class FXTable;
protected:
  FXString    label;
  FXIcon*     icon;
  void       *data;
  FXuint      state;
protected:
  FXTableItem():icon(NULL),data(NULL),state(0){}
  virtual void draw(const FXTable* table,FXDC& dc,FXint x,FXint y,FXint w,FXint h) const;
  virtual void drawButton(const FXTable* table,FXDC& dc,FXint x,FXint y,FXint w,FXint h) const;
  virtual void drawBorders(const FXTable* table,FXDC& dc,FXint x,FXint y,FXint w,FXint h) const;
  virtual void drawContent(const FXTable* table,FXDC& dc,FXint x,FXint y,FXint w,FXint h) const;
  virtual void drawPattern(const FXTable* table,FXDC& dc,FXint x,FXint y,FXint w,FXint h) const;
  virtual void drawBackground(const FXTable* table,FXDC& dc,FXint x,FXint y,FXint w,FXint h) const;
protected:
  enum{
    SELECTED   = 0x00000001,
    FOCUS      = 0x00000002,
    DISABLED   = 0x00000004,
    DRAGGABLE  = 0x00000008,
    BUTTON     = 0x00000010,
    PRESSED    = 0x00000020,
    ICONOWNED  = 0x00000040
    };
public:
  enum{
    RIGHT      = 0x00002000,      /// Align on right
    LEFT       = 0x00004000,      /// Align on left
    TOP        = 0x00008000,      /// Align on top
    BOTTOM     = 0x00010000,      /// Align on bottom
    BEFORE     = 0x00020000,      /// Icon before the text
    AFTER      = 0x00040000,      /// Icon after the text
    ABOVE      = 0x00080000,      /// Icon above the text
    BELOW      = 0x00100000,      /// Icon below the text
    LBORDER    = 0x00200000,      /// Draw left border
    RBORDER    = 0x00400000,      /// Draw right border
    TBORDER    = 0x00800000,      /// Draw top border
    BBORDER    = 0x01000000       /// Draw bottom border
    };
public:
  FXTableItem(const FXString& text,FXIcon* ic=NULL,void* ptr=NULL):label(text),icon(ic),data(ptr),state(FXTableItem::RIGHT){}
  virtual void setText(const FXString& txt){ label=txt; }
  FXString getText() const { return label; }
  virtual void setIcon(FXIcon* icn){ icon=icn; }
  FXIcon* getIcon() const { return icon; }
  void setData(void* ptr){ data=ptr; }
  void* getData() const { return data; }
  virtual void setFocus(FXbool focus);
  FXbool hasFocus() const { return (state&FOCUS)!=0; }
  virtual void setSelected(FXbool selected);
  FXbool isSelected() const { return (state&SELECTED)!=0; }
  virtual void setEnabled(FXbool enabled);
  FXbool isEnabled() const { return (state&DISABLED)==0; }
  virtual void setDraggable(FXbool draggable);
  FXbool isDraggable() const { return (state&DRAGGABLE)!=0; }
  void setJustify(FXuint justify);
  FXuint getJustify() const { return state&(RIGHT|LEFT|TOP|BOTTOM); }
  void setIconPosition(FXuint mode);
  FXuint getIconPosition() const { return state&(BEFORE|AFTER|ABOVE|BELOW); }
  void setBorders(FXuint borders);
  FXuint getBorders() const { return state&(LBORDER|RBORDER|TBORDER|BBORDER); }
  void setStipple(FXStipplePattern pat);
  FXStipplePattern getStipple() const;
  void setButton(FXbool button);
  FXbool isButton() const { return (state&BUTTON)!=0; }
  void setPressed(FXbool pressed);
  FXbool isPressed() const { return (state&PRESSED)!=0; }
  virtual void setIconOwned(FXuint owned=ICONOWNED);
  FXuint isIconOwned() const { return (state&ICONOWNED); }
  virtual FXint getWidth(const FXTable* table) const;
  virtual FXint getHeight(const FXTable* table) const;
  virtual void create();
  virtual void detach();
  virtual void destroy();
  virtual void save(FXStream& store) const;
  virtual void load(FXStream& store);
  virtual ~FXTableItem();
  };



/// Table Widget
class FXAPI FXTable : public FXScrollArea {
  FXDECLARE(FXTable)
protected:
  FXTableItem **cells;                  // Cells
  FXint        *col_x;                  // Vertical grid line positions
  FXint        *row_y;                  // Horizontal grid line positions
  FXFont       *font;                   // Font
  FXint         nrows;                  // Logically allocated rows
  FXint         ncols;                  // Logically allocated columns
  FXint         visiblerows;            // Visible rows
  FXint         visiblecols;            // Visible columns
  FXint         margintop;              // Margin top
  FXint         marginbottom;           // Margin bottom
  FXint         marginleft;             // Margin left
  FXint         marginright;            // Margin right
  FXColor       textColor;              // Normal text color
  FXColor       baseColor;              // Base color
  FXColor       hiliteColor;            // Highlight color
  FXColor       shadowColor;            // Shadow color
  FXColor       borderColor;            // Border color
  FXColor       selbackColor;           // Select background color
  FXColor       seltextColor;           // Select text color
  FXColor       gridColor;              // Grid line color
  FXColor       stippleColor;           // Stipple color
  FXColor       cellBorderColor;        // Cell border color
  FXint         cellBorderWidth;        // Cell border width
  FXColor       cellBackColor[2][2];    // Row/Column even/odd background color
  FXint         defColumnWidth;         // Default column width [if uniform columns]
  FXint         defRowHeight;           // Default row height [if uniform rows]
  FXint         leading_rows;           // Leading fixed rows
  FXint         leading_cols;           // Leading fixed columns
  FXint         scrolling_rows;         // Scrolling rows
  FXint         scrolling_cols;         // Scrolling columns
  FXint         trailing_rows;          // Trailing fixed rows
  FXint         trailing_cols;          // Trailing fixed columns
  FXint         scrollable_left;        // Left side of scrollable part of table
  FXint         scrollable_right;       // Right edge of scrollable part of table
  FXint         scrollable_top;         // Top side of scrollable part of table
  FXint         scrollable_bottom;      // Bottom side of scrollable part of table
  FXint         table_left;             // Left side of table
  FXint         table_right;            // Right edge of right side of table
  FXint         table_top;              // Top side of table
  FXint         table_bottom;           // Bottom side of bottom of table
  FXTableRange  selection;              // Selected cell range
  FXTablePos    current;                // Current position
  FXTablePos    anchor;                 // Anchor position
  FXTablePos    extent;                 // Extent position
  FXint         cellcursor;             // Cursor position in cell
  FXint         cellanchor;             // Anchor position in cell
  FXint         cellscroll;             // Scolled amount in cell
  FXbool        hgrid;                  // Horizontal grid lines shown
  FXbool        vgrid;                  // Vertical grid lines shown
  FXuchar       mode;                   // Mode we're in
  FXint         grabx;                  // Grab point x
  FXint         graby;                  // Grab point y
  FXint         rowcol;                 // Row or column being resized
  FXTimer      *blinker;                // Blink timer
  FXString      help;
public:
  static FXDragType csvType;
  static const FXchar csvTypeName[];
protected:
  FXTable();
  virtual void layout();
  void drawCursor(FXuint state);
  FXRectangle cellRect(FXint r,FXint c) const;
  virtual void drawCell(FXDC& dc,FXint xlo,FXint xhi,FXint ylo,FXint yhi,FXint xoff,FXint yoff,FXint sr,FXint er,FXint sc,FXint ec);
  virtual void drawRange(FXDC& dc,FXint xlo,FXint xhi,FXint ylo,FXint yhi,FXint xoff,FXint yoff,FXint rlo,FXint rhi,FXint clo,FXint chi);
  virtual FXTableItem* createItem(const FXString& text,FXIcon* icon,void* ptr);
  FXint startRow(FXint row,FXint col) const;
  FXint startCol(FXint row,FXint col) const;
  FXint endRow(FXint row,FXint col) const;
  FXint endCol(FXint row,FXint col) const;
  FXint nearestCol(FXint col,FXint x) const;
  FXint nearestRow(FXint row,FXint y) const;
protected:
  enum {
    MOUSE_NONE,
    MOUSE_SCROLL,
    MOUSE_DRAG,
    MOUSE_SELECT,
    MOUSE_COL_SELECT,
    MOUSE_ROW_SELECT,
    MOUSE_COL_SIZE,
    MOUSE_ROW_SIZE
    };
private:
  FXTable(const FXTable&);
  FXTable& operator=(const FXTable&);
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onFocusIn(FXObject*,FXSelector,void*);
  long onFocusOut(FXObject*,FXSelector,void*);
  long onMotion(FXObject*,FXSelector,void*);
  long onKeyPress(FXObject*,FXSelector,void*);
  long onKeyRelease(FXObject*,FXSelector,void*);
  long onLeftBtnPress(FXObject*,FXSelector,void*);
  long onLeftBtnRelease(FXObject*,FXSelector,void*);
  long onRightBtnPress(FXObject*,FXSelector,void*);
  long onRightBtnRelease(FXObject*,FXSelector,void*);
  long onUngrabbed(FXObject*,FXSelector,void*);
  long onBlink(FXObject*,FXSelector,void*);
  long onSelectionLost(FXObject*,FXSelector,void*);
  long onSelectionGained(FXObject*,FXSelector,void*);
  long onAutoScroll(FXObject*,FXSelector,void*);
  long onCommand(FXObject*,FXSelector,void*);
  long onClicked(FXObject*,FXSelector,void*);
  long onDoubleClicked(FXObject*,FXSelector,void*);
  long onTripleClicked(FXObject*,FXSelector,void*);

  // Visual characteristics
  long onCmdHorzGrid(FXObject*,FXSelector,void*);
  long onUpdHorzGrid(FXObject*,FXSelector,void*);
  long onCmdVertGrid(FXObject*,FXSelector,void*);
  long onUpdVertGrid(FXObject*,FXSelector,void*);

  // Row/Column manipulations
  long onCmdDeleteColumn(FXObject*,FXSelector,void*);
  long onUpdDeleteColumn(FXObject*,FXSelector,void*);
  long onCmdDeleteRow(FXObject*,FXSelector,void*);
  long onUpdDeleteRow(FXObject*,FXSelector,void*);
  long onCmdInsertColumn(FXObject*,FXSelector,void*);
  long onCmdInsertRow(FXObject*,FXSelector,void*);

  // Movement
  long onCmdMoveRight(FXObject*,FXSelector,void*);
  long onCmdMoveLeft(FXObject*,FXSelector,void*);
  long onCmdMoveUp(FXObject*,FXSelector,void*);
  long onCmdMoveDown(FXObject*,FXSelector,void*);
  long onCmdMoveHome(FXObject*,FXSelector,void*);
  long onCmdMoveEnd(FXObject*,FXSelector,void*);
  long onCmdMoveTop(FXObject*,FXSelector,void*);
  long onCmdMoveBottom(FXObject*,FXSelector,void*);
  long onCmdMovePageDown(FXObject*,FXSelector,void*);
  long onCmdMovePageUp(FXObject*,FXSelector,void*);

  // Mark and extend
  long onCmdMark(FXObject*,FXSelector,void*);
  long onCmdExtend(FXObject*,FXSelector,void*);

  // Changing Selection
  long onCmdSelectCell(FXObject*,FXSelector,void*);
  long onCmdSelectRow(FXObject*,FXSelector,void*);
  long onCmdSelectColumn(FXObject*,FXSelector,void*);
  long onCmdSelectAll(FXObject*,FXSelector,void*);
  long onCmdDeselectAll(FXObject*,FXSelector,void*);

public:

  enum {
    ID_HORZ_GRID=FXScrollArea::ID_LAST,
    ID_VERT_GRID,
    ID_DELETE_COLUMN,
    ID_DELETE_ROW,
    ID_INSERT_COLUMN,
    ID_INSERT_ROW,
    ID_SELECT_COLUMN,
    ID_SELECT_ROW,
    ID_SELECT_CELL,
    ID_SELECT_ALL,
    ID_DESELECT_ALL,
    ID_MOVE_LEFT,
    ID_MOVE_RIGHT,
    ID_MOVE_UP,
    ID_MOVE_DOWN,
    ID_MOVE_HOME,
    ID_MOVE_END,
    ID_MOVE_TOP,
    ID_MOVE_BOTTOM,
    ID_MOVE_PAGEDOWN,
    ID_MOVE_PAGEUP,
    ID_MARK,
    ID_EXTEND,
    ID_CUT_SEL,
    ID_COPY_SEL,
    ID_PASTE_SEL,
    ID_BLINK,
    ID_LAST
    };

public:

  /// Make new table with nr visible rows and nc visible columns; the table
  /// is initially empty, i.e. contains no cells (nrows=0, ncols=0)
  FXTable(FXComposite *p,FXint nr,FXint nc,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_MARGIN,FXint pr=DEFAULT_MARGIN,FXint pt=DEFAULT_MARGIN,FXint pb=DEFAULT_MARGIN);
  virtual void create();
  virtual void detach();
  virtual void recalc();

  /// Table widget can receive focus
  virtual FXbool canFocus() const;

  /// Move the focus to this window
  virtual void setFocus();

  /// Remove the focus from this window
  virtual void killFocus();

  /// Change visible rows/columns
  void setVisibleRows(FXint nvrows);
  FXint getVisibleRows() const { return visiblerows; }
  void setVisibleCols(FXint nvcols);
  FXint getVisibleCols() const { return visiblecols; }

  /// Is horizontal grid shown
  FXbool isHorzGridShown() const { return hgrid; }

  /// Is vertical grid shown
  FXbool isVertGridShown() const { return vgrid; }

  /// Show or hide horizontal grid
  void showHorzGrid(FXbool on=TRUE);

  /// Show or hide vertical grid
  void showVertGrid(FXbool on=TRUE);

  /// Report default size, which is determined based on the
  /// visible rows/columns and the default width/height
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();

  /// Compute content size
  virtual FXint getContentWidth();
  virtual FXint getContentHeight();

  /// Scroll contents
  virtual void moveContents(FXint x,FXint y);

  /// Resize the table content to nr rows and nc columns
  void setTableSize(FXint nr,FXint nc,FXbool notify=FALSE);

  /// Get number of rows
  FXint getNumRows() const { return nrows; }

  /// Get number of columns
  FXint getNumCols() const { return ncols; }

  /// Change cell margins
  void setMarginTop(FXint pt);
  FXint getMarginTop() const { return margintop; }
  void setMarginBottom(FXint pb);
  FXint getMarginBottom() const { return marginbottom; }
  void setMarginLeft(FXint pl);
  FXint getMarginLeft() const { return marginleft; }
  void setMarginRight(FXint pr);
  FXint getMarginRight() const { return marginright; }

  /// Change table style
  FXuint getTableStyle() const;
  void setTableStyle(FXuint style);

  /// Get/set leading rows
  virtual void setLeadingRows(FXint leadrows);
  FXint getLeadingRows() const { return leading_rows; }

  /// Get/set leading columns
  virtual void setLeadingCols(FXint leadcols);
  FXint getLeadingCols() const { return leading_cols; }

  /// Get/set trailing rows
  virtual void setTrailingRows(FXint trailrows);
  FXint getTrailingRows() const { return trailing_rows; }

  /// Get/set trailing columns
  virtual void setTrailingCols(FXint trailcols);
  FXint getTrailingCols() const { return trailing_cols; }

  /// Determine row containing y; returns -1 if y outside of table
  FXint rowAtY(FXint y) const;

  /// Determine column containing x; returns -1 if x outside of table
  FXint colAtX(FXint x) const;

  /// Return the item at the given index
  FXTableItem *getItem(FXint row,FXint col) const;

  /// Replace the item with a [possibly subclassed] item
  void setItem(FXint row,FXint col,FXTableItem* item);

  /// Insert new row
  virtual void insertRows(FXint row,FXint nr=1,FXbool notify=FALSE);

  /// Insert new column
  virtual void insertColumns(FXint col,FXint nc=1,FXbool notify=FALSE);

  /// Remove rows of cells
  virtual void removeRows(FXint row,FXint nr=1,FXbool notify=FALSE);

  /// Remove column of cells
  virtual void removeColumns(FXint col,FXint nc=1,FXbool notify=FALSE);

  /// Remove single cell
  virtual void removeItem(FXint row,FXint col,FXbool notify=FALSE);

  /// Scroll to make cell at r,c fully visible
  void makePositionVisible(FXint r,FXint c);

  /// Change column width
  void setColumnWidth(FXint col,FXint cwidth);
  FXint getColumnWidth(FXint col) const;

  /// Change row height
  void setRowHeight(FXint row,FXint rheight);
  FXint getRowHeight(FXint row) const;

  /// Change X coordinate of column c
  void setColumnX(FXint col,FXint x);
  FXint getColumnX(FXint col) const;

  /// Change Y coordinate of row r
  void setRowY(FXint row,FXint y);
  FXint getRowY(FXint row) const;

  /// Change default column width
  void setDefColumnWidth(FXint cwidth);
  FXint getDefColumnWidth() const { return defColumnWidth; }

  /// Change default row height
  void setDefRowHeight(FXint rheight);
  FXint getDefRowHeight() const { return defRowHeight; }

  /// Modify cell text
  void setItemText(FXint r,FXint c,const FXString& text);
  FXString getItemText(FXint r,FXint c) const;

  /// Modify cell icon
  void setItemIcon(FXint r,FXint c,FXIcon* icon);
  FXIcon* getItemIcon(FXint r,FXint c) const;

  /// Modify cell user-data
  void setItemData(FXint r,FXint c,void* ptr);
  void* getItemData(FXint r,FXint c) const;

  /// Is cell selected, current, visible, enabled
  FXbool isItemSelected(FXint r,FXint c) const;
  FXbool isItemCurrent(FXint r,FXint c) const;
  FXbool isItemVisible(FXint r,FXint c) const;
  FXbool isItemEnabled(FXint r,FXint c) const;

  /// Repaint cells between grid lines sr,er and grid lines sc,ec
  void updateRange(FXint sr,FXint er,FXint sc,FXint ec);

  /// Repaint cell at r,c
  void updateItem(FXint r,FXint c);

  /// Enable, disable, select, deselect, toggle cell
  FXbool enableItem(FXint r,FXint c);
  FXbool disableItem(FXint r,FXint c);
  FXbool selectItem(FXint r,FXint c,FXbool notify=FALSE);
  FXbool deselectItem(FXint r,FXint c,FXbool notify=FALSE);
  FXbool toggleItem(FXint r,FXint c,FXbool notify=FALSE);

  /// Change current cell
  void setCurrentItem(FXint r,FXint c,FXbool notify=FALSE);
  FXint getCurrentRow() const { return current.row; }
  FXint getCurrentColumn() const { return current.col; }

  /// Change anchored cell
  void setAnchorItem(FXint r,FXint c);
  FXint getAnchorRow() const { return anchor.row; }
  FXint getAnchorColumn() const { return anchor.col; }

  /// Select range
  FXbool selectRange(FXint sr,FXint er,FXint sc,FXint ec,FXbool notify=FALSE);

  /// Extend selection
  FXbool extendSelection(FXint r,FXint c,FXbool notify=FALSE);

  /// Kill selection
  FXbool killSelection(FXbool notify=FALSE);

  /// Change font
  void setFont(FXFont* fnt);
  FXFont* getFont() const { return font; }

  /// Obtain colors of various parts
  FXColor getTextColor() const { return textColor; }
  FXColor getBaseColor() const { return baseColor; }
  FXColor getHiliteColor() const { return hiliteColor; }
  FXColor getShadowColor() const { return shadowColor; }
  FXColor getBorderColor() const { return borderColor; }
  FXColor getSelBackColor() const { return selbackColor; }
  FXColor getSelTextColor() const { return seltextColor; }
  FXColor getGridColor() const { return gridColor; }
  FXColor getStippleColor() const { return stippleColor; }
  FXColor getCellBorderColor() const { return cellBorderColor; }

  /// Change colors of various parts
  void setTextColor(FXColor clr);
  void setBaseColor(FXColor clr);
  void setHiliteColor(FXColor clr);
  void setShadowColor(FXColor clr);
  void setBorderColor(FXColor clr);
  void setSelBackColor(FXColor clr);
  void setSelTextColor(FXColor clr);
  void setGridColor(FXColor clr);
  void setStippleColor(FXColor clr);
  void setCellBorderColor(FXColor clr);

  /// Change cell background color for even/odd rows/columns
  void setCellColor(FXint r,FXint c,FXColor clr);

  /// Obtain cell background color for even/odd rows/columns
  FXColor getCellColor(FXint r,FXint c) const;

  /// Change cell border width
  void setCellBorderWidth(FXint borderwidth);

  /// Return cell border width
  FXint getCellBorderWidth() const { return cellBorderWidth; }

  /// Change help text
  void setHelpText(const FXString& text);
  FXString getHelpText() const { return help; }

  /// Serialize
  virtual void save(FXStream& store) const;
  virtual void load(FXStream& store);

  virtual ~FXTable();
  };



#endif
