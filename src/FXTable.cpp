/********************************************************************************
*                                                                               *
*                            T a b l e   W i d g e t                            *
*                                                                               *
*********************************************************************************
* Copyright (C) 1999,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* Contributions from: Pierre Cyr <pcyr@po-box.mcgill.ca>                        *
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
* $Id: FXTable.cpp,v 1.103 2002/01/18 22:43:05 jeroen Exp $                     *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxkeys.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXRegistry.h"
#include "FXAccelTable.h"
#include "FXApp.h"
#include "FXDCWindow.h"
#include "FXFont.h"
#include "FXIcon.h"
#include "FXScrollbar.h"
#include "FXTable.h"


/*
  Notes:

  - Table looks like:

    +--------+--------+--------+--------+
    |        | ColHdr | ColHdr | ColHdr |
    +--------+--------+--------+--------+
    | RowHdr |    3.14|        |Pi      |
    +--------+--------+--------+--------+
    | RowHdr |        |        |        |
    +--------+--------+--------+--------+
    | RowHdr |        |        |        |
    +--------+--------+--------+--------+

  - Grid lines horizontal should be optional.
  - Grid lines vertictal should be optional also, independent of horizontal ones.
  - Grid line have different styles [Similar to frame styles]; normally dotted lines or light grey.
  - Cells have margins around the text.
  - Column headers are optional.
  - Row headers are optional.
  - Headers stay in place, i.e. column headers stay on top, row headers stay on right.
  - Cells can contain string or icon or number.
  - Justification and formatting [for numbers]:

      - Format same for whole table
      - Format same for column
      - Format same for row
      - Format different for each cell

  - Resizing columns [same for rows]:

      - Off, no resizing allowed.
      - Column bounds.
      - Adjustment of subsequent columns (proportional to old sizes).
      - Adjustment of prior columns (proportional to old sizes).
      - Adjustment of all columns (proportional to old sizes).
      - Adjustment of first/last column.
      - Uniform column width, or per-column defined width.

  - Selection:

      - Disabled.
      - Select rows only.
      - Select columns only.
      - Select rows and columns.
      - Select cells only.

  - Selection ranges:

      - Single entity (i.e. single row, column, etc.)
      - Range of entities (multiple rows, multiple columns, blocks)

  - Reordering:

      - Disabled.
      - Reordering of columns allowed.
      - Reordering of rows allowed.
      - Both.

  - Alternating colors:

      - All the same color
      - Alternate background/foreground every other row
      - Alternate background/foreground every other column

  - Header buttons:

      - Column header button to select whole column.
      - Row header button to select whole row.

  - Fixed columns or rows:

      - First n columns and last m columns. (e.g. to show totals).
      - First n rows and last m rows.

  - Virtual storage capability for HUGE arrays:

    o When exposing virtual cells, we ask to supply content
      for the exposed cells.

    o Keep track of part of table which is visible; this is
      the actual table.

    o The actual table keeps REAL cells for those virtual cells
      which are visible [so you can manipulate them, and for
      quick repainting].

    o When scrolling, we roll over the cells as follows:

      +---------+      +---------+      +---------+
      |XXXXXXXBB|      |BBXXXXXXX|      |DDCCCCCCC|
      |XXXXXXXBB|      |BBXXXXXXX|      |BBXXXXXXX|
      |XXXXXXXBB|  ->  |BBXXXXXXX|  ->  |BBXXXXXXX|
      |XXXXXXXBB|      |BBXXXXXXX|      |BBXXXXXXX|
      |CCCCCCCDD|      |DDCCCCCCC|      |BBXXXXXXX|
      +---------+      +---------+      +---------+

      Then of course we ask to refill the cells marked B, D,
      and C.

    o When resizing, we resize the actual table, and ask to
      refill the cells on the bottom/right [or top/left, if
      we're at the maximum scrolled position and new cells are
      introduced at the top!]

    o Virtual cell from actual one:

       vr = ar+firstrow (0<=ar<visiblerows)
       vc = ac+firstcol (0<=ac<visiblecols)

    o Actual cell from virtual one:

       ar = vr-firstrow (firstrow<=vr<firstrow+visiblerows)
       ac = vc-firstcol (colstart<=vr<firstcol+visiblecols)

      In virtual mode, virtual cells outside the actual table should probably
      return NULL.

      Perhaps we can do it as follows:

      ar = (vr-firstrow)%visiblerows
      ac = (vc-firstcol)%visiblecols

      and just update nrows and ncols during scrolling.

    - Need cells which could span multiple rows/columns

    - Need multi-line cells.

    - Set cell width/height in terms of character width/font height.

  - Allow for NULL in cell[].

  - Selection modes:
    Browse, single, multiple, extended. (like FXList).
    Column only, Row only, Both rows and columns, no selectability.
*/


#define DEFAULTCOLUMNWIDTH  100     // Initial value for defColumnWidth
#define DEFAULTROWHEIGHT    20      // Initial value for defRowHeight
#define FUDGE               1

#define TABLE_MASK          (TABLE_COL_SIZABLE|TABLE_ROW_SIZABLE)


/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXTableItem,FXObject,NULL,0)


// Draw background behind the cell
void FXTableItem::drawBackground(const FXTable* table,FXDC& dc,FXint x,FXint y,FXint w,FXint h) const {
  register FXbool hg=table->isHorzGridShown();
  register FXbool vg=table->isVertGridShown();
  dc.fillRectangle(x+vg,y+hg,w-vg,h-hg);
  }


// Draw a button
void FXTableItem::drawButton(const FXTable* table,FXDC& dc,FXint x,FXint y,FXint w,FXint h) const {
  register FXbool hg=table->isHorzGridShown();
  register FXbool vg=table->isVertGridShown();

  // Adjust to stay inside grid lines
  x+=vg;
  y+=hg;
  w-=vg;
  h-=hg;

  // Background
  dc.setForeground(table->getBaseColor());
  dc.fillRectangle(x,y,w,h);

  // Button is pressed
  if(state&PRESSED){
    dc.setForeground(table->getShadowColor());
    dc.fillRectangle(x,y,w-1,1);
    dc.fillRectangle(x,y,1,h-1);
    dc.setForeground(table->getBorderColor());
    dc.fillRectangle(x+1,y+1,w-3,1);
    dc.fillRectangle(x+1,y+1,1,h-3);
    dc.setForeground(table->getHiliteColor());
    dc.fillRectangle(x,y+h-1,w,1);
    dc.fillRectangle(x+w-1,y,1,h);
    dc.setForeground(table->getBaseColor());
    dc.fillRectangle(x+1,y+h-2,w-2,1);
    dc.fillRectangle(x+w-2,y+1,1,h-2);
    }

  // Button is up
  else{
    dc.setForeground(table->getHiliteColor());
    dc.fillRectangle(x,y,w-1,1);
    dc.fillRectangle(x,y,1,h-1);
    dc.setForeground(table->getBaseColor());
    dc.fillRectangle(x+1,y+1,w-2,1);
    dc.fillRectangle(x+1,y+1,1,h-2);
    dc.setForeground(table->getShadowColor());
    dc.fillRectangle(x+1,y+h-2,w-2,1);
    dc.fillRectangle(x+w-2,y+1,1,h-1);
    dc.setForeground(table->getBorderColor());
    dc.fillRectangle(x,y+h-1,w,1);
    dc.fillRectangle(x+w-1,y,1,h);
    }
  }


// Draw hatch pattern
void FXTableItem::drawPattern(const FXTable* table,FXDC& dc,FXint x,FXint y,FXint w,FXint h) const {
  if(state&0x1f00){
    register FXbool hg=table->isHorzGridShown();
    register FXbool vg=table->isVertGridShown();
    dc.setStipple((FXStipplePattern)((state&0x1f00)>>8),x,y);
    dc.setFillStyle(FILL_STIPPLED);
    dc.setForeground(table->getStippleColor());
    dc.fillRectangle(x+vg,y+hg,w-vg,h-hg);
    dc.setFillStyle(FILL_SOLID);
    }
  }


// Draw borders
void FXTableItem::drawBorders(const FXTable* table,FXDC& dc,FXint x,FXint y,FXint w,FXint h) const {
  if(state&(LBORDER|RBORDER|TBORDER|BBORDER)){
    register FXint bb=table->getCellBorderWidth();
    register FXbool hg=table->isHorzGridShown();
    register FXbool vg=table->isVertGridShown();
    dc.setForeground(table->getCellBorderColor());
    if(state&LBORDER) dc.fillRectangle(x,y,bb,h+hg);
    if(state&RBORDER) dc.fillRectangle(x+w+vg-bb,y,bb,h+hg);
    if(state&TBORDER) dc.fillRectangle(x,y,w+vg,bb);
    if(state&BBORDER) dc.fillRectangle(x,y+h+hg-bb,w+vg,bb);
    }
  }


// Draw content
void FXTableItem::drawContent(const FXTable* table,FXDC& dc,FXint x,FXint y,FXint w,FXint h) const {
  register FXint tx,ty,tw,th,ix,iy,iw,ih,s,ml,mr,mt,mb,beg,end,t,xx,yy;
  register FXFont *font=dc.getTextFont();

  // Get margins
  ml=table->getMarginLeft();
  mr=table->getMarginRight();
  mt=table->getMarginTop();
  mb=table->getMarginBottom();

  // Text width and height
  beg=tw=th=0;
  do{
    end=beg;
    while(label[end] && label[end]!='\n') end++;
    if((t=font->getTextWidth(&label[beg],end-beg))>tw) tw=t;
    th+=font->getFontHeight();
    beg=end+1;
    }
  while(label[end]);

  // Icon size
  iw=ih=0;
  if(icon){
    iw=icon->getWidth();
    ih=icon->getHeight();
    }

  // Icon-text spacing
  s=0;
  if(iw && tw) s=4;

  // Fix x coordinate
  if(state&LEFT){
    if(state&BEFORE){ ix=x+ml; tx=ix+iw+s; }
    else if(state&AFTER){ tx=x+ml; ix=tx+tw+s; }
    else{ ix=x+ml; tx=x+ml; }
    }
  else if(state&RIGHT){
    if(state&BEFORE){ tx=x+w-mr-tw; ix=tx-iw-s; }
    else if(state&AFTER){ ix=x+w-mr-iw; tx=ix-tw-s; }
    else{ ix=x+w-mr-iw; tx=x+w-mr-tw; }
    }
  else{
    if(state&BEFORE){ ix=x+ml+(w-ml-mr-tw-iw-s)/2; tx=ix+iw+s; }
    else if(state&AFTER){ tx=x+ml+(w-ml-mr-tw-iw-s)/2; ix=tx+tw+s; }
    else{ ix=x+ml+(w-ml-mr-iw)/2; tx=x+ml+(w-ml-mr-tw)/2; }
    }

  // Fix y coordinate
  if(state&TOP){
    if(state&ABOVE){ iy=y+mt; ty=iy+ih; }
    else if(state&BELOW){ ty=y+mt; iy=ty+th; }
    else{ iy=y+mt; ty=y+mt; }
    }
  else if(state&BOTTOM){
    if(state&ABOVE){ ty=y+h-mb-th; iy=ty-ih; }
    else if(state&BELOW){ iy=y+h-mb-ih; ty=iy-th; }
    else{ iy=y+h-mb-ih; ty=y+h-mb-th; }
    }
  else{
    if(state&ABOVE){ iy=y+mt+(h-mb-mt-th-ih)/2; ty=iy+ih; }
    else if(state&BELOW){ ty=y+mt+(h-mb-mt-th-ih)/2; iy=ty+th; }
    else{ iy=y+mt+(h-mb-mt-ih)/2; ty=y+mt+(h-mb-mt-th)/2; }
    }

  // Offset a bit when pressed
  if(state&PRESSED){ tx++; ty++; ix++; iy++; }

  // Paint icon
  if(icon){
    dc.drawIcon(icon,ix,iy);
    }

  // Text color
  if(state&BUTTON)
    dc.setForeground(table->getTextColor());
  else if(state&SELECTED)
    dc.setForeground(table->getSelTextColor());
  else
    dc.setForeground(table->getTextColor());

  // Draw text
  yy=ty+font->getFontAscent();
  beg=0;
  do{
    end=beg;
    while(label[end] && label[end]!='\n') end++;
    if(state&LEFT) xx=tx;
    else if(state&RIGHT) xx=tx+tw-font->getTextWidth(&label[beg],end-beg);
    else xx=tx+(tw-font->getTextWidth(&label[beg],end-beg))/2;
    dc.drawText(xx,yy,&label[beg],end-beg);
    yy+=font->getFontHeight();
    beg=end+1;
    }
  while(label[end]);
  }


// Draw item
void FXTableItem::draw(const FXTable* table,FXDC& dc,FXint x,FXint y,FXint w,FXint h) const {

  // Draw as a button
  if(state&BUTTON){

    // Draw button frame
    drawButton(table,dc,x,y,w,h);

    // Draw cell content
    drawContent(table,dc,x,y,w,h);
    }

  // Draw as a normal cell
  else{

    // Draw background
    drawBackground(table,dc,x,y,w,h);

    // Draw hatch pattern
    drawPattern(table,dc,x,y,w,h);

    // Draw cell content
    drawContent(table,dc,x,y,w,h);

    // Draw borders
    drawBorders(table,dc,x,y,w,h);
    }
  }


// Set or kill focus
void FXTableItem::setFocus(FXbool focus){
  if(focus) state|=FOCUS; else state&=~FOCUS;
  }

// Select or deselect item
void FXTableItem::setSelected(FXbool selected){
  if(selected) state|=SELECTED; else state&=~SELECTED;
  }


// Enable or disable the item
void FXTableItem::setEnabled(FXbool enabled){
  if(enabled) state&=~DISABLED; else state|=DISABLED;
  }


// Icon is draggable
void FXTableItem::setDraggable(FXbool draggable){
  if(draggable) state|=DRAGGABLE; else state&=~DRAGGABLE;
  }


// Icons owner by item
void FXTableItem::setIconOwned(FXuint owned){
  state=(state&~ICONOWNED)|(owned&ICONOWNED);
  }


// Change justify mode
void FXTableItem::setJustify(FXuint justify){
  state=(state&~(RIGHT|LEFT|TOP|BOTTOM)) | (justify&(RIGHT|LEFT|TOP|BOTTOM));
  }

// Change icon positioning
void FXTableItem::setIconPosition(FXuint mode){
  state=(state&~(BEFORE|AFTER|ABOVE|BELOW)) | (mode&(BEFORE|AFTER|ABOVE|BELOW));
  }


// Change border mode
void FXTableItem::setBorders(FXuint borders){
  state=(state&~(LBORDER|RBORDER|TBORDER|BBORDER)) | (borders&(LBORDER|RBORDER|TBORDER|BBORDER));
  }


// Set stipple pattern
void FXTableItem::setStipple(FXStipplePattern pat) {
  state=(state&0xffffe0ff)|((pat<<8));
  }


// Get stipple pattern
FXStipplePattern FXTableItem::getStipple() const {
  return (FXStipplePattern)((state>>8)&0x1f);
  }


// Change button mode
void FXTableItem::setButton(FXbool button){
  if(button) state|=BUTTON; else state&=~BUTTON;
  }


// Set button state
void FXTableItem::setPressed(FXbool pressed){
  if(pressed) state|=PRESSED; else state&=~PRESSED;
  }


// Create icon
void FXTableItem::create(){
  if(icon) icon->create();
  }


// Destroy icon
void FXTableItem::destroy(){
  if((state&ICONOWNED) && icon) icon->destroy();
  }


// Detach from icon resource
void FXTableItem::detach(){
  if(icon) icon->detach();
  }


// Get width of item
FXint FXTableItem::getWidth(const FXTable*) const { return 1; }


// Get height of item
FXint FXTableItem::getHeight(const FXTable*) const { return 1; }


// Save data
void FXTableItem::save(FXStream& store) const {
  FXObject::save(store);
  store << label;
  store << icon;
  store << state;
  }


// Load data
void FXTableItem::load(FXStream& store){
  FXObject::load(store);
  store >> label;
  store >> icon;
  store >> state;
  }


// Delete icon if owned
FXTableItem::~FXTableItem(){
  if(state&ICONOWNED) delete icon;
  }


/*******************************************************************************/

// Map
FXDEFMAP(FXTable) FXTableMap[]={
  FXMAPFUNC(SEL_PAINT,0,FXTable::onPaint),
  FXMAPFUNC(SEL_TIMEOUT,FXTable::ID_BLINK,FXTable::onBlink),
  FXMAPFUNC(SEL_MOTION,0,FXTable::onMotion),
  FXMAPFUNC(SEL_TIMEOUT,FXWindow::ID_AUTOSCROLL,FXTable::onAutoScroll),
  FXMAPFUNC(SEL_UNGRABBED,0,FXTable::onUngrabbed),
  FXMAPFUNC(SEL_LEFTBUTTONPRESS,0,FXTable::onLeftBtnPress),
  FXMAPFUNC(SEL_LEFTBUTTONRELEASE,0,FXTable::onLeftBtnRelease),
  FXMAPFUNC(SEL_RIGHTBUTTONPRESS,0,FXTable::onRightBtnPress),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,0,FXTable::onRightBtnRelease),
  FXMAPFUNC(SEL_KEYPRESS,0,FXTable::onKeyPress),
  FXMAPFUNC(SEL_KEYRELEASE,0,FXTable::onKeyRelease),
  FXMAPFUNC(SEL_FOCUSIN,0,FXTable::onFocusIn),
  FXMAPFUNC(SEL_FOCUSOUT,0,FXTable::onFocusOut),
  FXMAPFUNC(SEL_SELECTION_LOST,0,FXTable::onSelectionLost),
  FXMAPFUNC(SEL_SELECTION_GAINED,0,FXTable::onSelectionGained),
  FXMAPFUNC(SEL_CLICKED,0,FXTable::onClicked),
  FXMAPFUNC(SEL_DOUBLECLICKED,0,FXTable::onDoubleClicked),
  FXMAPFUNC(SEL_TRIPLECLICKED,0,FXTable::onTripleClicked),
  FXMAPFUNC(SEL_COMMAND,0,FXTable::onCommand),
  FXMAPFUNC(SEL_UPDATE,FXTable::ID_HORZ_GRID,FXTable::onUpdHorzGrid),
  FXMAPFUNC(SEL_UPDATE,FXTable::ID_VERT_GRID,FXTable::onUpdVertGrid),
  FXMAPFUNC(SEL_COMMAND,FXTable::ID_HORZ_GRID,FXTable::onCmdHorzGrid),
  FXMAPFUNC(SEL_COMMAND,FXTable::ID_VERT_GRID,FXTable::onCmdVertGrid),
  FXMAPFUNC(SEL_COMMAND,FXTable::ID_DELETE_COLUMN,FXTable::onCmdDeleteColumn),
  FXMAPFUNC(SEL_UPDATE,FXTable::ID_DELETE_COLUMN,FXTable::onUpdDeleteColumn),
  FXMAPFUNC(SEL_COMMAND,FXTable::ID_DELETE_ROW,FXTable::onCmdDeleteRow),
  FXMAPFUNC(SEL_UPDATE,FXTable::ID_DELETE_ROW,FXTable::onUpdDeleteRow),
  FXMAPFUNC(SEL_COMMAND,FXTable::ID_INSERT_COLUMN,FXTable::onCmdInsertColumn),
  FXMAPFUNC(SEL_COMMAND,FXTable::ID_INSERT_ROW,FXTable::onCmdInsertRow),

  FXMAPFUNC(SEL_COMMAND,FXTable::ID_MOVE_LEFT,FXTable::onCmdMoveLeft),
  FXMAPFUNC(SEL_COMMAND,FXTable::ID_MOVE_RIGHT,FXTable::onCmdMoveRight),
  FXMAPFUNC(SEL_COMMAND,FXTable::ID_MOVE_UP,FXTable::onCmdMoveUp),
  FXMAPFUNC(SEL_COMMAND,FXTable::ID_MOVE_DOWN,FXTable::onCmdMoveDown),
  FXMAPFUNC(SEL_COMMAND,FXTable::ID_MOVE_HOME,FXTable::onCmdMoveHome),
  FXMAPFUNC(SEL_COMMAND,FXTable::ID_MOVE_END,FXTable::onCmdMoveEnd),
  FXMAPFUNC(SEL_COMMAND,FXTable::ID_MOVE_TOP,FXTable::onCmdMoveTop),
  FXMAPFUNC(SEL_COMMAND,FXTable::ID_MOVE_BOTTOM,FXTable::onCmdMoveBottom),
  FXMAPFUNC(SEL_COMMAND,FXTable::ID_MOVE_PAGEDOWN,FXTable::onCmdMovePageDown),
  FXMAPFUNC(SEL_COMMAND,FXTable::ID_MOVE_PAGEUP,FXTable::onCmdMovePageUp),

  FXMAPFUNC(SEL_COMMAND,FXTable::ID_SELECT_COLUMN,FXTable::onCmdSelectColumn),
  FXMAPFUNC(SEL_COMMAND,FXTable::ID_SELECT_ROW,FXTable::onCmdSelectRow),
  FXMAPFUNC(SEL_COMMAND,FXTable::ID_SELECT_CELL,FXTable::onCmdSelectCell),
  FXMAPFUNC(SEL_COMMAND,FXTable::ID_SELECT_ALL,FXTable::onCmdSelectAll),
  FXMAPFUNC(SEL_COMMAND,FXTable::ID_DESELECT_ALL,FXTable::onCmdDeselectAll),
  FXMAPFUNC(SEL_COMMAND,FXTable::ID_MARK,FXTable::onCmdMark),
  FXMAPFUNC(SEL_COMMAND,FXTable::ID_EXTEND,FXTable::onCmdExtend),
  };


// Object implementation
FXIMPLEMENT(FXTable,FXScrollArea,FXTableMap,ARRAYNUMBER(FXTableMap))

// So we can cut and paste into MS EXCEL
const FXchar FXTable::csvTypeName[]="Csv";

// Drag types
FXDragType FXTable::csvType=0;


/*******************************************************************************/

// Serialization
FXTable::FXTable(){
  flags|=FLAG_ENABLED;
  cells=(FXTableItem**)-1;
  col_x=(FXint*)-1;
  row_y=(FXint*)-1;
  font=(FXFont*)-1;
  nrows=0;
  ncols=0;
  visiblerows=0;
  visiblecols=0;
  margintop=0;
  marginbottom=0;
  marginleft=0;
  marginright=0;
  textColor=0;
  baseColor=0;
  hiliteColor=0;
  shadowColor=0;
  borderColor=0;
  selbackColor=0;
  seltextColor=0;
  gridColor=0;
  stippleColor=0;
  cellBorderColor=0;
  cellBorderWidth=0;
  defColumnWidth=DEFAULTCOLUMNWIDTH;
  defRowHeight=DEFAULTROWHEIGHT;
  leading_rows=0;
  leading_cols=0;
  scrolling_rows=0;
  scrolling_cols=0;
  trailing_rows=0;
  trailing_cols=0;
  scrollable_left=0;
  scrollable_right=0;
  scrollable_top=0;
  scrollable_bottom=0;
  table_left=0;
  table_right=0;
  table_top=0;
  table_bottom=0;
  selection.fm.row=0;
  selection.fm.col=0;
  selection.to.row=-1;
  selection.to.col=-1;
  current.row=0;
  current.col=0;
  anchor.row=0;
  anchor.col=0;
  extent.row=0;
  extent.col=0;
  cellcursor=0;
  cellanchor=0;
  cellscroll=0;
  mode=MOUSE_NONE;
  vgrid=TRUE;
  hgrid=TRUE;
  grabx=0;
  graby=0;
  rowcol=0;
  blinker=NULL;
  }


// Build table
FXTable::FXTable(FXComposite *p,FXint nr,FXint nc,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h,FXint pl,FXint pr,FXint pt,FXint pb):
  FXScrollArea(p,opts,x,y,w,h){
  flags|=FLAG_ENABLED;
  target=tgt;
  message=sel;
  FXCALLOC(&cells,FXTableItem*,1);
  FXCALLOC(&col_x,FXint,1);
  FXCALLOC(&row_y,FXint,1);
  font=getApp()->getNormalFont();
  nrows=0;
  ncols=0;
  visiblerows=FXMAX(nr,0);
  visiblecols=FXMAX(nc,0);
  margintop=pt;
  marginbottom=pb;
  marginleft=pl;
  marginright=pr;
  textColor=getApp()->getForeColor();
  baseColor=getApp()->getBaseColor();
  hiliteColor=getApp()->getHiliteColor();
  shadowColor=getApp()->getShadowColor();
  borderColor=getApp()->getBorderColor();
  selbackColor=getApp()->getSelbackColor();
  seltextColor=getApp()->getSelforeColor();
  gridColor=getApp()->getBaseColor();
  stippleColor=FXRGB(255,0,0);
  cellBorderColor=getApp()->getBorderColor();
  cellBorderWidth=2;
//   cellBackColor[0][0]=FXRGB(255,255,255);     // Even row, even column
//   cellBackColor[0][1]=FXRGB(255,255,255);     // Even row, odd column
//   cellBackColor[1][0]=FXRGB(255,240,240);     // Odd row, even column
//   cellBackColor[1][1]=FXRGB(255,240,240);     // Odd row, odd column
  cellBackColor[0][0]=FXRGB(255,255,255);     // Even row, even column
  cellBackColor[0][1]=FXRGB(255,240,240);     // Even row, odd column
  cellBackColor[1][0]=FXRGB(240,255,240);     // Odd row, even column
  cellBackColor[1][1]=FXRGB(240,240,255);     // Odd row, odd column
  defColumnWidth=DEFAULTCOLUMNWIDTH;
  defRowHeight=DEFAULTROWHEIGHT;
  leading_rows=0;
  leading_cols=0;
  scrolling_rows=0;
  scrolling_cols=0;
  trailing_rows=0;
  trailing_cols=0;
  scrollable_left=0;
  scrollable_right=0;
  scrollable_top=0;
  scrollable_bottom=0;
  table_left=0;
  table_right=0;
  table_top=0;
  table_bottom=0;
  selection.fm.row=0;
  selection.fm.col=0;
  selection.to.row=-1;
  selection.to.col=-1;
  current.row=0;
  current.col=0;
  anchor.row=0;
  anchor.col=0;
  extent.row=0;
  extent.col=0;
  cellcursor=0;
  cellanchor=0;
  cellscroll=0;
  vgrid=TRUE;
  hgrid=TRUE;
  mode=MOUSE_NONE;
  grabx=0;
  graby=0;
  rowcol=0;
  blinker=NULL;
  }


// Create window
void FXTable::create(){
  register FXint n=nrows*ncols;
  register FXint i;
  FXScrollArea::create();
  if(!textType) textType=getApp()->registerDragType(textTypeName);
  if(!csvType) csvType=getApp()->registerDragType(csvTypeName);
  for(i=0; i<n; i++){ if(cells[i]) cells[i]->create(); }
  font->create();
  }


// Detach window
void FXTable::detach(){
  register FXint n=nrows*ncols;
  register FXint i;
  FXScrollArea::detach();
  for(i=0; i<n; i++){ if(cells[i]) cells[i]->detach(); }
  font->detach();
  }


// Can have focus
FXbool FXTable::canFocus() const { return TRUE; }


// Into focus chain
void FXTable::setFocus(){
  FXScrollArea::setFocus();
  setDefault(TRUE);
  }


// Out of focus chain
void FXTable::killFocus(){
  FXScrollArea::killFocus();
  setDefault(MAYBE);
  }


// Get default width
FXint FXTable::getDefaultWidth(){
  return visiblecols ? visiblecols*defColumnWidth : FXScrollArea::getDefaultWidth();
  }


// Get default height
FXint FXTable::getDefaultHeight(){
  return visiblerows ? visiblerows*defRowHeight : FXScrollArea::getDefaultHeight();
  }


// Create item
FXTableItem* FXTable::createItem(const FXString& text,FXIcon* icon,void* ptr){
  return new FXTableItem(text,icon,ptr);
  }


// Propagate size change
void FXTable::recalc(){
  FXScrollArea::recalc();
  flags|=FLAG_RECALC;
  }



// Move content
void FXTable::moveContents(FXint x,FXint y){
  FXint dx=x-pos_x;
  FXint dy=y-pos_y;

  // Hide cursor
  drawCursor(0);

  // Update position
  pos_x=x;
  pos_y=y;

  // Scroll leading fixed rows
  if(leading_rows){
    scroll(scrollable_left+vgrid,table_top,scrollable_right-scrollable_left-vgrid,scrollable_top+hgrid-table_top,dx,0);
    }

  // Scroll leading fixed columns
  if(leading_cols){
    scroll(table_left,scrollable_top+hgrid,scrollable_left+vgrid-table_left,scrollable_bottom-scrollable_top-hgrid,0,dy);
    }

  // Scroll trailing fixed rows
  if(trailing_rows){
    scroll(scrollable_left+vgrid,scrollable_bottom,scrollable_right-scrollable_left-vgrid,table_bottom+hgrid-scrollable_bottom,dx,0);
    }

  // Scroll trailing fixed columns
  if(trailing_cols){
    scroll(scrollable_right,scrollable_top+hgrid,table_right+vgrid-scrollable_right,scrollable_bottom-scrollable_top-hgrid,0,dy);
    }

  // Scroll table
  scroll(scrollable_left+vgrid,scrollable_top+hgrid,scrollable_right-scrollable_left-vgrid,scrollable_bottom-scrollable_top-hgrid,dx,dy);
  }


// Starting row of multi-column cell
FXint FXTable::startRow(FXint row,FXint col) const {
  register FXTableItem *item=cells[row*ncols+col];
  while(0<row && cells[(row-1)*ncols+col]==item) row--;
  return row;
  }


// Starting column of multi-column cell
FXint FXTable::startCol(FXint row,FXint col) const {
  register FXTableItem *item=cells[row*ncols+col];
  while(0<col && cells[row*ncols+col-1]==item) col--;
  return col;
  }


// Ending row of multi-column cell
FXint FXTable::endRow(FXint row,FXint col) const {
  register FXTableItem *item=cells[row*ncols+col];
  while(row<nrows && cells[row*ncols+col]==item) row++;
  return row;
  }


// Ending column of multi-column cell
FXint FXTable::endCol(FXint row,FXint col) const {
  register FXTableItem *item=cells[row*ncols+col];
  while(col<ncols && cells[row*ncols+col]==item) col++;
  return col;
  }


// Find value in array within [l,h]
static FXint bsearch(FXint *array,FXint l,FXint h,FXint value){
  register FXint m;
  do{
    m=(h+l)/2;
    if(array[m+1]<=value) l=m+1;
    else if(value<array[m]) h=m-1;
    else break;
    }
  while(h>=l);
  return m;
  }


// Get column containing x, -1 if outside table
FXint FXTable::colAtX(FXint x) const {
  if(x<table_left || table_right<x) return -1;
  if(x<scrollable_left) return bsearch(col_x,0,leading_cols-1,x);
  if(scrollable_right<=x) return bsearch(col_x,ncols-trailing_cols,ncols-1,x-scrollable_right+col_x[ncols-trailing_cols]);
  return bsearch(col_x,leading_cols,ncols-trailing_cols-1,x-pos_x);
  }


// Get row containing y
FXint FXTable::rowAtY(FXint y) const {
  if(y<table_top || table_bottom<y) return -1;
  if(y<scrollable_top) return bsearch(row_y,0,leading_rows-1,y);
  if(scrollable_bottom<=y) return bsearch(row_y,nrows-trailing_rows,nrows-1,y-scrollable_bottom+row_y[nrows-trailing_rows]);
  return bsearch(row_y,leading_rows,nrows-trailing_rows-1,y-pos_y);
  }


// Return column that x is near
FXint FXTable::nearestCol(FXint col,FXint x) const {
  register FXint t;
  if(0<=col){
    t=getColumnX(col);
    if(t-FUDGE<=x && x<=t+FUDGE) return col;
    t+=getColumnWidth(col);
    if(t-FUDGE<=x && x<=t+FUDGE) return col+1;
    }
  return -1;
  }


// Return row that y is near
FXint FXTable::nearestRow(FXint row,FXint y) const {
  register FXint t;
  if(0<=row){
    t=getRowY(row);
    if(t-FUDGE<=y && y<=t+FUDGE) return row;
    t+=getRowHeight(row);
    if(t-FUDGE<=y && y<=t+FUDGE) return row+1;
    }
  return -1;
  }


// Rectangle of cell ar r,c
FXRectangle FXTable::cellRect(FXint r,FXint c) const {
  FXRectangle rect;

  // Check inputs
  FXASSERT(0<=r && r<=nrows);
  FXASSERT(0<=c && c<=ncols);

  // Cell span
  FXint sr=startRow(r,c);
  FXint er=endRow(r,c);
  FXint sc=startCol(r,c);
  FXint ec=endCol(r,c);

  // X coordinate
  if(ec<=leading_cols)
    rect.x=col_x[sc];
  else if(ncols-trailing_cols<=sc)
    rect.x=scrollable_right+col_x[sc]-col_x[ncols-trailing_cols];
  else
    rect.x=pos_x+col_x[sc];

  // Y coordinate
  if(er<=leading_rows)
    rect.y=row_y[sr];
  else if(nrows-trailing_rows<=sr)
    rect.y=scrollable_bottom+row_y[sr]-row_y[nrows-trailing_rows];
  else
    rect.y=pos_y+row_y[sr];

  // Size, including next grid line if any
  rect.w=col_x[ec]-col_x[sc]+vgrid;
  rect.h=row_y[er]-row_y[sr]+hgrid;

  return rect;
  }


// Force position to become fully visible
void FXTable::makePositionVisible(FXint r,FXint c){
  register FXint xlo,xhi,ylo,yhi,px,py;
  if(xid){
    px=pos_x;
    py=pos_y;
    if(leading_cols<=c && c<ncols-trailing_cols){
      xlo=col_x[c];
      xhi=col_x[c+1];
      if(px+xhi >= scrollable_right) px=scrollable_right-xhi;
      if(px+xlo <= scrollable_left) px=scrollable_left-xlo;
      }
    if(leading_rows<=r && r<nrows-trailing_rows){
      ylo=row_y[r];
      yhi=row_y[r+1];
      if(py+yhi >= scrollable_bottom) py=scrollable_bottom-yhi;
      if(py+ylo <= scrollable_top) py=scrollable_top-ylo;
      }
    setPosition(px,py);
    }
  }


// Repaint cells between grid lines sr,er and grid lines sc,ec
void FXTable::updateRange(FXint sr,FXint er,FXint sc,FXint ec){
  FXint xl,xr,yt,yb;
  if(sr<0 || sc<0 || nrows<er || ncols<ec){ fxerror("%s::updateItems: index out of range.\n",getClassName()); }
  if(sr<=er && sc<=ec){
    if(ec<=leading_cols){
      xl=col_x[sc];
      xr=col_x[ec];
      }
    else if(ncols-trailing_cols<=sc){
      xl=scrollable_right+col_x[sc]-col_x[ncols-trailing_cols];
      xr=scrollable_right+col_x[ec]-col_x[ncols-trailing_cols];
      }
    else{
      xl=pos_x+col_x[sc];
      xr=pos_x+col_x[ec];
      }
    if(er<=leading_rows){
      yt=row_y[sr];
      yb=row_y[er+1];
      }
    else if(nrows-trailing_rows<=sr){
      yt=scrollable_bottom+row_y[sr]-row_y[nrows-trailing_rows];
      yb=scrollable_bottom+row_y[er]-row_y[nrows-trailing_rows];
      }
    else{
      yt=pos_y+row_y[sr];
      yb=pos_y+row_y[er];
      }
    update(xl,yt,xr-xl+vgrid,yb-yt+hgrid);
    }
  }


// Repaint
void FXTable::updateItem(FXint r,FXint c){
  if(r<0 || c<0 || nrows<=r || ncols<=c){ fxerror("%s::updateItem: index out of range.\n",getClassName()); }
  updateRange(startRow(r,c),endRow(r,c),startCol(r,c),endCol(r,c));
  }


// Return the item at the given index
FXTableItem *FXTable::getItem(FXint row,FXint col) const {
  if(row<0 || col<0 || nrows<row || ncols<=col){ fxerror("%s::getItem: index out of range.\n",getClassName()); }
  return cells[row*ncols+col];
  }


// Replace item with another (may be NULL)
void FXTable::setItem(FXint row,FXint col,FXTableItem* item){
  if(row<0 || col<0 || nrows<row || ncols<=col){ fxerror("%s::setItem: index out of range.\n",getClassName()); }
  cells[row*ncols+col]=item;
  recalc();
  }


// Change item text
void FXTable::setItemText(FXint r,FXint c,const FXString& text){
  if(r<0 || nrows<=r || c<0 || ncols<=c){ fxerror("%s::setItemText: index out of range.\n",getClassName()); }
  if(cells[r*ncols+c]==NULL) cells[r*ncols+c]=createItem(NULL,NULL,NULL);
  cells[r*ncols+c]->setText(text);
  updateItem(r,c);
  }


// Get item text
FXString FXTable::getItemText(FXint r,FXint c) const {
  if(r<0 || c<0 || nrows<=r || ncols<=c){ fxerror("%s::getItemText: index out of range.\n",getClassName()); }
  if(cells[r*ncols+c]==NULL) return FXString::null;
  return cells[r*ncols+c]->getText();
  }


// Set item icon
void FXTable::setItemIcon(FXint r,FXint c,FXIcon* icon){
  if(r<0 || c<0 || nrows<=r || ncols<=c){ fxerror("%s::setItemIcon: index out of range.\n",getClassName()); }
  if(cells[r*ncols+c]==NULL) cells[r*ncols+c]=createItem(NULL,NULL,NULL);
  cells[r*ncols+c]->setIcon(icon);
  updateItem(r,c);
  }

// Get item icon
FXIcon* FXTable::getItemIcon(FXint r,FXint c) const {
  if(r<0 || c<0 || nrows<=r || ncols<=c){ fxerror("%s::getItemIcon: index out of range.\n",getClassName()); }
  if(cells[r*ncols+c]==NULL) return NULL;
  return cells[r*ncols+c]->getIcon();
  }


// Set item data
void FXTable::setItemData(FXint r,FXint c,void* ptr){
  if(r<0 || c<0 || nrows<=r || ncols<=c){ fxerror("%s::setItemData: index out of range.\n",getClassName()); }
  if(cells[r*ncols+c]==NULL) cells[r*ncols+c]=createItem(NULL,NULL,NULL);
  cells[r*ncols+c]->setData(ptr);
  }


// Get item data
void* FXTable::getItemData(FXint r,FXint c) const {
  if(r<0 || c<0 || nrows<=r || ncols<=c){ fxerror("%s::getItemData: index out of range.\n",getClassName()); }
  if(cells[r*ncols+c]==NULL) return NULL;
  return cells[r*ncols+c]->getData();
  }


// True if item is selected
FXbool FXTable::isItemSelected(FXint r,FXint c) const {
  if(r<0 || c<0 || nrows<=r || ncols<=c){ fxerror("%s::isItemSelected: index out of range.\n",getClassName()); }
  return cells[r*ncols+c] && cells[r*ncols+c]->isSelected();
  }


// True if item is current
FXbool FXTable::isItemCurrent(FXint r,FXint c) const {
  if(r<0 || c<0 || nrows<=r || ncols<=c){ fxerror("%s::isItemCurrent: index out of range.\n",getClassName()); }
  return current.row==r && current.col==c;
  }


// True if item (partially) visible
FXbool FXTable::isItemVisible(FXint r,FXint c) const {
  FXbool vis=TRUE;
  if(r<0 || c<0 || nrows<=r || ncols<=c){ fxerror("%s::isItemVisible: index out of range.\n",getClassName()); }
  if(c<leading_cols){
    if(table_right<=col_x[c]) vis=FALSE;
    }
  else if(ncols-trailing_cols<=c){
    if(scrollable_right+col_x[c+1]-col_x[ncols-trailing_cols]<table_left) vis=FALSE;
    }
  else{
    if(pos_x+col_x[c+1]<scrollable_left || scrollable_right<=pos_x+col_x[c]) vis=FALSE;
    }
  if(r<leading_rows){
    if(table_bottom<=row_y[r]) vis=FALSE;
    }
  else if(nrows-trailing_rows<=r){
    if(scrollable_bottom+row_y[r+1]-row_y[nrows-trailing_rows]<table_top) vis=FALSE;
    }
  else{
    if(pos_y+row_y[r+1]<scrollable_top || scrollable_bottom<=pos_y+row_y[r]) vis=FALSE;
    }
  return vis;
  }


// True if item is enabled
FXbool FXTable::isItemEnabled(FXint r,FXint c) const {
  if(r<0 || nrows<=r || c<0 || ncols<=c){ fxerror("%s::isItemEnabled: index out of range.\n",getClassName()); }
  FXTableItem *item=cells[r*ncols+c];
  return item && item->isEnabled();
  }


// Enable one item
FXbool FXTable::enableItem(FXint r,FXint c){
  if(r<0 || nrows<=r || c<0 || ncols<=c){ fxerror("%s::enableItem: index out of range.\n",getClassName()); }
  FXTableItem *item=cells[r*ncols+c];
  if(item && !item->isEnabled()){
    item->setEnabled(TRUE);
    updateItem(r,c);
    return TRUE;
    }
  return FALSE;
  }


// Disable one item
FXbool FXTable::disableItem(FXint r,FXint c){
  if(r<0 || nrows<=r || c<0 || ncols<=c){ fxerror("%s::disableItem: index out of range.\n",getClassName()); }
  FXTableItem *item=cells[r*ncols+c];
  if(item && item->isEnabled()){
    item->setEnabled(FALSE);
    updateItem(r,c);
    return TRUE;
    }
  return FALSE;
  }


// Select one item
FXbool FXTable::selectItem(FXint r,FXint c,FXbool notify){
  if(r<0 || nrows<=r || c<0 || ncols<=c){ fxerror("%s::selectItem: index out of range.\n",getClassName()); }
  FXTableItem *item=cells[r*ncols+c];
  FXTablePos tablepos;
  if(item && !item->isSelected()){
    item->setSelected(TRUE);
    updateItem(r,c);
    if(notify && target){
      tablepos.row=r;
      tablepos.col=c;
      target->handle(this,MKUINT(message,SEL_SELECTED),(void*)&tablepos);
      }
    return TRUE;
    }
  return FALSE;
  }


// Deselect one item
FXbool FXTable::deselectItem(FXint r,FXint c,FXbool notify){
  if(r<0 || nrows<=r || c<0 || ncols<=c){ fxerror("%s::deselectItem: index out of range.\n",getClassName()); }
  FXTableItem *item=cells[r*ncols+c];
  FXTablePos tablepos;
  if(item && item->isSelected()){
    item->setSelected(FALSE);
    updateItem(r,c);
    if(notify && target){
      tablepos.row=r;
      tablepos.col=c;
      target->handle(this,MKUINT(message,SEL_DESELECTED),(void*)&tablepos);
      }
    return TRUE;
    }
  return FALSE;
  }


// Toggle one item
FXbool FXTable::toggleItem(FXint r,FXint c,FXbool notify){
  if(r<0 || nrows<=r || c<0 || ncols<=c){ fxerror("%s::toggleItem: index out of range.\n",getClassName()); }
  FXTableItem *item=cells[r*ncols+c];
  FXTablePos tablepos;
  if(item && item->isSelected()){
    item->setSelected(FALSE);
    updateItem(r,c);
    if(notify && target){
      tablepos.row=r;
      tablepos.col=c;
      target->handle(this,MKUINT(message,SEL_DESELECTED),(void*)&tablepos);
      }
    return TRUE;
    }
  if(item && !item->isSelected()){
    item->setSelected(TRUE);
    updateItem(r,c);
    if(notify && target){
      tablepos.row=r;
      tablepos.col=c;
      target->handle(this,MKUINT(message,SEL_SELECTED),(void*)&tablepos);
      }
    return TRUE;
    }
  return FALSE;
  }


// Set current item
void FXTable::setCurrentItem(FXint r,FXint c,FXbool notify){
  if(r<0) r=0;
  if(c<0) c=0;
  if(r>=nrows) r=nrows-1;
  if(c>=ncols) c=ncols-1;
  if(r!=current.row || c!=current.col){

    // Deactivate old item
    if(0<=current.row && 0<=current.col){

      // No visible change if it doen't have the focus
      if(hasFocus()){
        FXASSERT(current.row<nrows);
        FXASSERT(current.col<ncols);
        if(cells[current.row*ncols+current.col]){
          cells[current.row*ncols+current.col]->setFocus(FALSE);
          }
        updateItem(current.row,current.col);
        }
      }

    current.row=r;
    current.col=c;

    // Activate new item
    if(0<=current.row && 0<=current.col){

      // No visible change if it doen't have the focus
      if(hasFocus()){
        FXASSERT(current.row<nrows);
        FXASSERT(current.col<ncols);
        if(cells[current.row*ncols+current.col]){
          cells[current.row*ncols+current.col]->setFocus(TRUE);
          }
        updateItem(current.row,current.col);
        }
      }

    // Notify item change
    if(notify && target){
      target->handle(this,MKUINT(message,SEL_CHANGED),(void*)&current);
      }
    }
  }


// Set anchor item
void FXTable::setAnchorItem(FXint r,FXint c){
  if(r<0) r=0;
  if(c<0) c=0;
  if(r>=nrows) r=nrows-1;
  if(c>=ncols) c=ncols-1;
  anchor.row=r;
  anchor.col=c;
  extent.row=r;
  extent.col=c;
  }


// Select range
FXbool FXTable::selectRange(FXint sr,FXint er,FXint sc,FXint ec,FXbool notify){
  register FXbool changes=FALSE;
  register FXint rr,cc;
  for(rr=sr; rr<=er; rr++){
    for(cc=sc; cc<=ec; cc++){
      changes|=selectItem(rr,cc,notify);
      }
    }
  return changes;
  }


// Extend selection
FXbool FXTable::extendSelection(FXint r,FXint c,FXbool notify){
  register FXint orlo,orhi,oclo,ochi;
  register FXint nrlo,nrhi,nclo,nchi;
  register FXint rlo,rhi,clo,chi,rr,cc;
  register FXbool inold,innew;
  register FXbool changes=FALSE;
  if(0<=r && 0<=c && 0<=anchor.row && 0<=anchor.col){
    FXMINMAX(orlo,orhi,anchor.row,extent.row);
    FXMINMAX(oclo,ochi,anchor.col,extent.col);
    FXMINMAX(nrlo,nrhi,anchor.row,r);
    FXMINMAX(nclo,nchi,anchor.col,c);
    rlo=FXMIN(orlo,nrlo);
    rhi=FXMAX(orhi,nrhi);
    clo=FXMIN(oclo,nclo);
    chi=FXMAX(ochi,nchi);
    for(rr=rlo; rr<=rhi; rr++){
      for(cc=clo; cc<=chi; cc++){
        inold=(orlo<=rr && rr<=orhi && oclo<=cc && cc<=ochi);
        innew=(nrlo<=rr && rr<=nrhi && nclo<=cc && cc<=nchi);
        if(inold && !innew){
          changes|=deselectItem(rr,cc,notify);
          }
        else if(!inold && innew){
          changes|=selectItem(rr,cc,notify);
          }
        }
      }
    extent.row=r;
    extent.col=c;
    }
  return changes;
  }


// Kill selection
FXbool FXTable::killSelection(FXbool notify){
  register FXbool changes=FALSE;
  register FXint r,c;
  for(r=0; r<nrows; r++){
    for(c=0; c<ncols; c++){
      changes|=deselectItem(r,c,notify);
      }
    }
  extent=anchor;
  return changes;
  }


// Determine scrollable content width
FXint FXTable::getContentWidth(){
  return col_x[ncols]+vgrid;
  }


// Determine scrollable content height
FXint FXTable::getContentHeight(){
  return row_y[nrows]+hgrid;
  }


// Recalculate layout determines item locations and sizes
void FXTable::layout(){

  // Calculate contents
  FXScrollArea::layout();

  // Whole table placement
  table_left=col_x[0];
  table_top=row_y[0];
  table_right=col_x[ncols];
  table_bottom=row_y[nrows];

  // Adjust right and bottom
  if(table_right>=viewport_w) table_right=viewport_w-vgrid;
  if(table_bottom>=viewport_h) table_bottom=viewport_h-hgrid;

  // Scrollable part of table
  scrollable_left=col_x[leading_cols];
  scrollable_top=row_y[leading_rows];
  scrollable_right=table_right-col_x[ncols]+col_x[ncols-trailing_cols];
  scrollable_bottom=table_bottom-row_y[nrows]+row_y[nrows-trailing_rows];

  FXTRACE((200,"     table_left=%d      table_right=%d      table_top=%d table_bottom=%d\n",table_left,table_right,table_top,table_bottom));
  FXTRACE((200,"scrollable_left=%d scrollable_right=%d scrollable_top=%d table_bottom=%d\n",scrollable_left,scrollable_right,scrollable_top,scrollable_bottom));

  // Determine line size for scroll bars
  vertical->setLine(defRowHeight);
  horizontal->setLine(defColumnWidth);

  // Force repaint
  update();

  // No more dirty
  flags&=~FLAG_DIRTY;
  }


// Gained focus
long FXTable::onFocusIn(FXObject* sender,FXSelector sel,void* ptr){
  register FXTableItem *item;
  FXScrollArea::onFocusIn(sender,sel,ptr);
  if(!blinker){ blinker=getApp()->addTimeout(getApp()->getBlinkSpeed(),this,ID_BLINK); }
  drawCursor(FLAG_CARET);
  if(0<=current.row && 0<=current.col){
    FXASSERT(current.row<nrows);
    FXASSERT(current.col<ncols);
    item=cells[current.row*ncols+current.col];
    if(item) item->setFocus(TRUE);
    updateItem(current.row,current.col);
    }
  return 1;
  }


// Lost focus
long FXTable::onFocusOut(FXObject* sender,FXSelector sel,void* ptr){
  register FXTableItem *item;
  FXScrollArea::onFocusOut(sender,sel,ptr);
  if(blinker){ blinker=getApp()->removeTimeout(blinker); }
  drawCursor(0);
  if(0<=current.row && 0<=current.col){
    FXASSERT(current.row<nrows);
    FXASSERT(current.col<ncols);
    item=cells[current.row*ncols+current.col];
    if(item) item->setFocus(FALSE);
    updateItem(current.row,current.col);
    }
  return 1;
  }


// We have the selection
long FXTable::onSelectionGained(FXObject* sender,FXSelector sel,void* ptr){
  FXScrollArea::onSelectionGained(sender,sel,ptr);
  ////
  return 1;
  }


// We lost the selection
long FXTable::onSelectionLost(FXObject* sender,FXSelector sel,void* ptr){
  FXScrollArea::onSelectionLost(sender,sel,ptr);
  ////
  return 1;
  }


// Blink the cursor
long FXTable::onBlink(FXObject*,FXSelector,void*){
  drawCursor(flags^FLAG_CARET);
  blinker=getApp()->addTimeout(getApp()->getBlinkSpeed(),this,ID_BLINK);
  return 0;
  }


// Draw the cursor
void FXTable::drawCursor(FXuint state){
  //register FXint xx,yt,yb,cl;
  if((state^flags)&FLAG_CARET){
    FXDCWindow dc(this);
//     xx=pos_x+marginleft+cursorx;
//     yt=pos_y+margintop+cursory;
//     yb=yt+font->getFontHeight()-1;
//     dc.begin(this);
//     dc.setClipRectangle(marginleft,margintop,viewport_w-marginleft-marginright,viewport_h-margintop-marginbottom);
//     if(flags&FLAG_CARET){
//       if(lines[0]<=cursorpos && cursorpos<=lines[nvislines]){
//
//         // Cursor may be in the selection
//         if(hasSelection() && selstartpos<=cursorpos && cursorpos<selendpos){
//           dc.setForeground(selbackColor);
//           }
//         else{
//           dc.setForeground(backColor);
//           }
//
//         // Repaint cursor in background to erase it
//         dc.drawLine(xx,yt,xx,yb);
//         dc.drawLine(xx+1,yt,xx+1,yb);
//         dc.drawLine(xx-2,yt,xx+3,yt);
//         dc.drawLine(xx-2,yb,xx+3,yb);
//
//         // Restore text
//         dc.setTextFont(font);
//         cl=posToLine(cursorpos);
//         drawTextLine(cl,xx-3,xx+3,0,10000);
//         }
//       flags&=~FLAG_CARET;
//       }
//     else{
//       if(lines[0]<=cursorpos && cursorpos<=lines[nvislines]){
//         dc.setForeground(cursorColor);
//         dc.drawLine(xx,yt,xx,yb);
//         dc.drawLine(xx+1,yt,xx+1,yb);
//         dc.drawLine(xx-2,yt,xx+3,yt);
//         dc.drawLine(xx-2,yb,xx+3,yb);
//         flags|=FLAG_CARET;
//         }
//       }
//     dc.clearClipRectangle();
//     dc.end();
    }
  }


// Draw single cell, possibly spanning multiple rows,columns
void FXTable::drawCell(FXDC& dc,FXint xlo,FXint xhi,FXint ylo,FXint yhi,FXint xoff,FXint yoff,FXint sr,FXint er,FXint sc,FXint ec){
  register FXTableItem *item=cells[sr*ncols+sc];
  register FXint xl,xr,yt,yb;

  // Verify some stuff
  FXASSERT(0<=sc && sc<=ec && ec<=ncols);
  FXASSERT(0<=sr && sr<=er && er<=nrows);

  // Get cell bounds
  yt=yoff+row_y[sr];
  yb=yoff+row_y[er];
  xl=xoff+col_x[sc];
  xr=xoff+col_x[ec];

  // All drawing is clipped against cell bounds AND the table
  // fragment bounds so we don't paint over the neighboring cells
  dc.setClipRectangle(FXMAX(xlo,xl),FXMAX(ylo,yt),FXMIN(xhi,xr+vgrid)-FXMAX(xlo,xl),FXMIN(yhi,yb+hgrid)-FXMAX(ylo,yt));

  // Set background color
  if(isItemSelected(sr,sc)){
    dc.setForeground(selbackColor);                 // Selected item
    }
  else if(sr+1==er && sc+1==ec){
    dc.setForeground(cellBackColor[sr&1][sc&1]);    // Singular item
    }
  else{
    dc.setForeground(backColor);                    // Spanning item
    }

  // Draw the item, if there is one
  if(!item){
    dc.fillRectangle(xl+vgrid,yt+hgrid,xr-xl-vgrid,yb-yt-hgrid);
    }
  else{
    item->draw(this,dc,xl,yt,xr-xl,yb-yt);
    }

  // If focus in current cell, draw the focus
  if(hasFocus()){
    if(sr<=current.row && current.row<er && sc<=current.col && current.col<ec){
      dc.drawFocusRectangle(xl+2,yt+2,xr-xl-3,yb-yt-3);
      }
    }
  }


// Draw table fragment [rlo:rhi, clo:chi] clipping against box
void FXTable::drawRange(FXDC& dc,FXint xlo,FXint xhi,FXint ylo,FXint yhi,FXint xoff,FXint yoff,FXint rlo,FXint rhi,FXint clo,FXint chi){
  register FXint r,c,fr,lr,fc,lc;
  register FXTableItem *item,*meti;
  if(xlo<xhi && ylo<yhi){

    //FXTRACE((300,"drawTableRange: rlo=%d rhi=%d clo=%d chi=%d\n",rlo,rhi,clo,chi));

    // Find dirty part of table
    for(fc=clo; fc<chi && col_x[fc+1]<xlo-xoff; fc++);
    for(lc=fc; lc<chi && col_x[lc]<=xhi-xoff; lc++);
    for(fr=rlo; fr<rhi && row_y[fr+1]<ylo-yoff; fr++);
    for(lr=fr; lr<rhi && row_y[lr]<=yhi-yoff; lr++);

    //FXTRACE((300,"fr=%d lr=%d fc=%d lc=%d\n",fr,lr,fc,lc));
    //FXTRACE((300,"xlo=%d xhi=%d col_x[%d]=%d col_x[%d]=%d\n",xlo-xoff,xhi-xoff,fc,col_x[fc],lc,col_x[lc]));
    //FXTRACE((300,"ylo=%d yhi=%d row_y[%d]=%d row_y[%d]=%d\n",ylo-yoff,yhi-yoff,fr,row_y[fr],lr,row_y[lr]));

    FXASSERT(0<=fc && lc<=ncols);
    FXASSERT(0<=fr && lr<=nrows);

    // Clip against table fragment
    dc.setClipRectangle(xlo,ylo,xhi-xlo,yhi-ylo);

    // Draw horizontal grid lines
    if(hgrid){
      dc.setForeground(gridColor);
      for(c=fc; c<lc; c++){
        meti=item=NULL;
        for(r=fr; r<=lr; r++){

          // Take care not to access outside of cell array!
          if(r==0 || r==nrows || (item=cells[r*ncols+c])==NULL || item!=meti){
            dc.fillRectangle(xoff+col_x[c],yoff+row_y[r],col_x[c+1]-col_x[c],1);
            }
          meti=item;
          }
        }
      }

    // Draw vertical grid lines
    if(vgrid){
      dc.setForeground(gridColor);
      for(r=fr; r<lr; r++){
        meti=item=NULL;
        for(c=fc; c<=lc; c++){

          // Take care not to access outside of cell array!
          if(c==0 || c==ncols || (item=cells[r*ncols+c])==NULL || item!=meti){
            dc.fillRectangle(xoff+col_x[c],yoff+row_y[r],1,row_y[r+1]-row_y[r]);
            }
          meti=item;
          }
        }
      }

    // Draw the cells
    for(r=fr; r<lr; r++){
      for(c=fc; c<lc; c++){
        item=cells[r*ncols+c];
        if(item){
          if((r!=fr && cells[(r-1)*ncols+c]==item) || (c!=fc && cells[r*ncols+c-1]==item)) continue;
          drawCell(dc,xlo,xhi,ylo,yhi,xoff,yoff,startRow(r,c),endRow(r,c),startCol(r,c),endCol(r,c));
          }
        else{
          drawCell(dc,xlo,xhi,ylo,yhi,xoff,yoff,r,r+1,c,c+1);
          }
        }
      }
    }
  }


// Draw exposed part of table
long FXTable::onPaint(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXDCWindow dc(this,event);
  FXint llx,lhx,mlx,mhx,rlx,rhx;
  FXint tly,thy,mly,mhy,bly,bhy;
  FXint xlo,xhi,ylo,yhi;

  dc.setTextFont(font);

//dc.setForeground(FXRGB(255,0,0));
//dc.fillRectangle(event->rect.x,event->rect.y,event->rect.w,event->rect.h);

  // Fill background right of the table
  if(width>col_x[ncols]){
    dc.setForeground(backColor);
    dc.fillRectangle(col_x[ncols],0,width-col_x[ncols],height);
    }

  // Fill background below the table
  if(height>row_y[nrows]){
    dc.setForeground(backColor);
    dc.fillRectangle(0,row_y[nrows],col_x[ncols],height-row_y[nrows]);
    }

  // Exposed area
  xlo=event->rect.x;
  xhi=event->rect.x+event->rect.w;
  ylo=event->rect.y;
  yhi=event->rect.y+event->rect.h;

  // Overlap with columns
  llx=FXMAX(xlo,table_left);
  lhx=FXMIN(xhi,scrollable_left+vgrid);
  mlx=FXMAX(xlo,scrollable_left+vgrid);
  mhx=FXMIN(xhi,scrollable_right);
  rlx=FXMAX(xlo,scrollable_right);
  rhx=FXMIN(xhi,table_right+vgrid);

  // Overlap with rows
  tly=FXMAX(ylo,table_top);
  thy=FXMIN(yhi,scrollable_top+hgrid);
  mly=FXMAX(ylo,scrollable_top+hgrid);
  mhy=FXMIN(yhi,scrollable_bottom);
  bly=FXMAX(ylo,scrollable_bottom);
  bhy=FXMIN(yhi,table_bottom+hgrid);

  //FXTRACE((300,"llx=%d lhx=%d mlx=%d mhx=%d rlx=%d rhx=%d\n",llx,lhx,mlx,mhx,rlx,rhx));
  //FXTRACE((300,"tly=%d thy=%d mly=%d mhy=%d bly=%d bhy=%d\n",tly,thy,mly,mhy,bly,bhy));

  // Draw main part
  drawRange(dc,mlx,mhx,mly,mhy,pos_x,pos_y,leading_rows,nrows-trailing_rows,leading_cols,ncols-trailing_cols);

  // Vertically scrollable parts
  drawRange(dc,llx,lhx,mly,mhy,0,pos_y,leading_rows,nrows-trailing_rows, 0,leading_cols);
  drawRange(dc,rlx,rhx,mly,mhy,scrollable_right-col_x[ncols-trailing_cols],pos_y,leading_rows,nrows-trailing_rows,ncols-trailing_cols,ncols);

  // Horizontally scrollable parts
  drawRange(dc,mlx,mhx,tly,thy,pos_x,0,0,leading_rows,leading_cols,ncols-trailing_cols);
  drawRange(dc,mlx,mhx,bly,bhy,pos_x,scrollable_bottom-row_y[nrows-trailing_rows],nrows-trailing_rows,nrows,leading_cols,ncols-trailing_cols);

  // Corner parts
  drawRange(dc,llx,lhx,tly,thy,0,0,0,leading_rows,0,leading_cols);
  drawRange(dc,rlx,rhx,tly,thy,scrollable_right-col_x[ncols-trailing_cols],0,0,leading_rows,ncols-trailing_cols,ncols);
  drawRange(dc,llx,lhx,bly,bhy,0,scrollable_bottom-row_y[nrows-trailing_rows],nrows-trailing_rows,nrows,0,leading_cols);
  drawRange(dc,rlx,rhx,bly,bhy,scrollable_right-col_x[ncols-trailing_cols],scrollable_bottom-row_y[nrows-trailing_rows],nrows-trailing_rows,nrows,ncols-trailing_cols,ncols);

  return 1;
  }


// Key Press
long FXTable::onKeyPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  flags&=~FLAG_TIP;
  if(!isEnabled()) return 0;
  if(target && target->handle(this,MKUINT(message,SEL_KEYPRESS),ptr)) return 1;
  switch(event->code){
    case KEY_Home:
    case KEY_KP_Home:
      if(!(event->state&SHIFTMASK)){
        handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
        }
      if(event->state&CONTROLMASK){
        handle(this,MKUINT(ID_MOVE_TOP,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_MOVE_HOME,SEL_COMMAND),NULL);
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
        handle(this,MKUINT(ID_MOVE_BOTTOM,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_MOVE_END,SEL_COMMAND),NULL);
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
      handle(this,MKUINT(ID_MOVE_PAGEUP,SEL_COMMAND),NULL);
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
      handle(this,MKUINT(ID_MOVE_PAGEDOWN,SEL_COMMAND),NULL);
      if(event->state&SHIFTMASK){
        handle(this,MKUINT(ID_EXTEND,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_MARK,SEL_COMMAND),NULL);
        }
      return 1;
    case KEY_Up:
    case KEY_KP_Up:
      if(!(event->state&SHIFTMASK)){
        handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
        }
      handle(this,MKUINT(ID_MOVE_UP,SEL_COMMAND),NULL);
      if(event->state&SHIFTMASK){
        handle(this,MKUINT(ID_EXTEND,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_MARK,SEL_COMMAND),NULL);
        }
      return 1;
    case KEY_Down:
    case KEY_KP_Down:
      if(!(event->state&SHIFTMASK)){
        handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
        }
      handle(this,MKUINT(ID_MOVE_DOWN,SEL_COMMAND),NULL);
      if(event->state&SHIFTMASK){
        handle(this,MKUINT(ID_EXTEND,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_MARK,SEL_COMMAND),NULL);
        }
      return 1;
    case KEY_Left:
    case KEY_KP_Left:
      if(!(event->state&SHIFTMASK)){
        handle(this,MKUINT(ID_DESELECT_ALL,SEL_COMMAND),NULL);
        }
      handle(this,MKUINT(ID_MOVE_LEFT,SEL_COMMAND),NULL);
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
      handle(this,MKUINT(ID_MOVE_RIGHT,SEL_COMMAND),NULL);
      if(event->state&SHIFTMASK){
        handle(this,MKUINT(ID_EXTEND,SEL_COMMAND),NULL);
        }
      else{
        handle(this,MKUINT(ID_MARK,SEL_COMMAND),NULL);
        }
      return 1;
    case KEY_space:
    case KEY_KP_Space:
      flags&=~FLAG_UPDATE;
      return 1;
    case KEY_Return:
    case KEY_KP_Enter:
      return 1;
    case KEY_Control_L:
    case KEY_Control_R:
    case KEY_Shift_L:
    case KEY_Shift_R:
    case KEY_Alt_L:
    case KEY_Alt_R:
      //if(flags&FLAG_DODRAG){handle(this,MKUINT(0,SEL_DRAGGED),ptr);}
      return 1;
    }
  return 0;
  }


// Key Release
long FXTable::onKeyRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  if(!isEnabled()) return 0;
  flags|=FLAG_UPDATE;
  if(target && target->handle(this,MKUINT(message,SEL_KEYRELEASE),ptr)) return 1;
  switch(event->code){
    case KEY_Home:
    case KEY_KP_Home:
    case KEY_End:
    case KEY_KP_End:
    case KEY_Page_Up:
    case KEY_KP_Page_Up:
    case KEY_Page_Down:
    case KEY_KP_Page_Down:
    case KEY_Left:
    case KEY_KP_Left:
    case KEY_Right:
    case KEY_KP_Right:
    case KEY_Up:
    case KEY_KP_Up:
    case KEY_Down:
    case KEY_KP_Down:
      return 1;
    case KEY_space:
    case KEY_KP_Space:
      flags|=FLAG_UPDATE;
      return 1;
    case KEY_Return:
    case KEY_KP_Enter:
      handle(this,MKUINT(0,SEL_DOUBLECLICKED),(void*)&current);
      //if(0<=current && items[current]->isEnabled()){
      //  handle(this,MKUINT(0,SEL_COMMAND),(void*)&current);
      //  }
      return 1;
    case KEY_Shift_L:
    case KEY_Shift_R:
    case KEY_Control_L:
    case KEY_Control_R:
    case KEY_Alt_L:
    case KEY_Alt_R:
      //if(flags&FLAG_DODRAG){handle(this,MKUINT(0,SEL_DRAGGED),ptr);}
      return 1;
    }
  return 0;
  }


// Automatic scroll
long FXTable::onAutoScroll(FXObject* sender,FXSelector sel,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXint r,c;
  FXScrollArea::onAutoScroll(sender,sel,ptr);
  switch(mode){
    case MOUSE_SELECT:
      c=colAtX(event->win_x);
      r=rowAtY(event->win_y);
      if(0<=r && 0<=c && (current.row!=r || current.col!=c)){
        extendSelection(r,c,TRUE);
        setCurrentItem(r,c,TRUE);
        }
      return 1;
    case MOUSE_DRAG:
      return 1;
    }
  return 1;
  }


// Mouse moved
long FXTable::onMotion(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXint r,c;
  FXCursor *cursor;
  switch(mode){
    case MOUSE_NONE:
      cursor=getApp()->getDefaultCursor(DEF_ARROW_CURSOR);
      if(options&TABLE_ROW_SIZABLE){
        r=nearestRow(rowAtY(event->win_y),event->win_y);
        if(0<r) cursor=getApp()->getDefaultCursor(DEF_VSPLIT_CURSOR);
        }
      if(options&TABLE_COL_SIZABLE){
        c=nearestCol(colAtX(event->win_x),event->win_x);
        if(0<c) cursor=getApp()->getDefaultCursor(DEF_HSPLIT_CURSOR);
        }
      setDefaultCursor(cursor);
      return 0;
    case MOUSE_SCROLL:
      setPosition(event->win_x-grabx,event->win_y-graby);
      return 1;
    case MOUSE_DRAG:
      return 1;
    case MOUSE_SELECT:
      if(startAutoScroll(event->win_x,event->win_y,FALSE)) return 1;  // FIXME scroll when near edge of scrollable area
      c=colAtX(event->win_x);
      r=rowAtY(event->win_y);
      if(0<=r && 0<=c && (current.row!=r || current.col!=c)){
        extendSelection(r,c,TRUE);
        setCurrentItem(r,c,TRUE);
        }
      return 1;
    case MOUSE_COL_SELECT:
      return 1;
    case MOUSE_ROW_SELECT:
      return 1;
    case MOUSE_COL_SIZE:
      setColumnX(rowcol,event->win_x);
      return 1;
    case MOUSE_ROW_SIZE:
      setRowY(rowcol,event->win_y);
      return 1;
    }
  return 0;
  }


// Pressed button
long FXTable::onLeftBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXTablePos tablepos;
  FXint r,c;
  flags&=~FLAG_TIP;
  handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
  if(isEnabled()){
    grab();
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONPRESS),ptr)) return 1;

    // Cell being clicked on
    tablepos.row=rowAtY(event->win_y);
    tablepos.col=colAtX(event->win_x);

    // Outside table
    if(tablepos.row<0 || tablepos.row>=nrows || tablepos.col<0 || tablepos.col>=ncols) return 0;

    // Resizeable column
    if(options&TABLE_COL_SIZABLE){
      c=nearestCol(tablepos.col,event->win_x);
      if(0<c){
        setDragCursor(getApp()->getDefaultCursor(DEF_HSPLIT_CURSOR));
        rowcol=c;
        mode=MOUSE_COL_SIZE;
        flags&=~FLAG_UPDATE;
        return 1;
        }
      }

    // Resizeable row
    if(options&TABLE_ROW_SIZABLE){
      r=nearestRow(tablepos.row,event->win_y);
      if(0<r){
        setDragCursor(getApp()->getDefaultCursor(DEF_VSPLIT_CURSOR));
        rowcol=r;
        mode=MOUSE_ROW_SIZE;
        flags&=~FLAG_UPDATE;
        return 1;
        }
      }

    // Change current item
    setCurrentItem(tablepos.row,tablepos.col,TRUE);

    // Select or deselect
    if(event->state&SHIFTMASK){
      if(0<=anchor.row && 0<=anchor.col){
        if(isItemEnabled(anchor.row,anchor.col)) selectItem(anchor.row,anchor.col);
        extendSelection(current.row,current.col);
        }
      else{
        if(isItemEnabled(current.row,current.col)) selectItem(current.row,current.col);
        setAnchorItem(current.row,current.col);
        }
      }
    else if(event->state&CONTROLMASK){
      if(isItemEnabled(current.row,current.col)) toggleItem(current.row,current.col);
      setAnchorItem(current.row,current.col);
      }
    else{
      if(isItemEnabled(current.row,current.col)){ killSelection(); selectItem(current.row,current.col); }
      setAnchorItem(current.row,current.col);
      }
    mode=MOUSE_SELECT;
    flags&=~FLAG_UPDATE;
    flags|=FLAG_PRESSED;
    return 1;
    }
  return 0;
  }


// Released button
long FXTable::onLeftBtnRelease(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  if(isEnabled()){
    ungrab();
    flags&=~FLAG_PRESSED;
    flags|=FLAG_UPDATE;
    stopAutoScroll();
    if(target && target->handle(this,MKUINT(message,SEL_LEFTBUTTONRELEASE),ptr)) return 1;
    setDragCursor(getApp()->getDefaultCursor(DEF_ARROW_CURSOR));
    mode=MOUSE_NONE;

    // Scroll to make item visibke
    makePositionVisible(current.row,current.col);

    // Update anchor
    setAnchorItem(current.row,current.col);

    // Generate clicked callbacks
    if(event->click_count==1){
      handle(this,MKUINT(0,SEL_CLICKED),(void*)&current);
      }
    else if(event->click_count==2){
      handle(this,MKUINT(0,SEL_DOUBLECLICKED),(void*)&current);
      }
    else if(event->click_count==3){
      handle(this,MKUINT(0,SEL_TRIPLECLICKED),(void*)&current);
      }

    // Command callback only when clicked on item
    if(0<=current.row && 0<=current.col && isItemEnabled(current.row,current.col)){
      handle(this,MKUINT(0,SEL_COMMAND),(void*)&current);
      }
    return 1;
    }
  return 0;
  }


// Pressed right button
long FXTable::onRightBtnPress(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  flags&=~FLAG_TIP;
  handle(this,MKUINT(0,SEL_FOCUS_SELF),ptr);
  if(isEnabled()){
    grab();
    if(target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONPRESS),ptr)) return 1;
    flags&=~FLAG_UPDATE;
    flags|=FLAG_PRESSED;
    grabx=event->win_x-pos_x;
    graby=event->win_y-pos_y;
    mode=MOUSE_SCROLL;
    return 1;
    }
  return 0;
  }


// Released right button
long FXTable::onRightBtnRelease(FXObject*,FXSelector,void* ptr){
  if(isEnabled()){
    ungrab();
    flags&=~FLAG_PRESSED;
    flags|=FLAG_UPDATE;
    mode=MOUSE_NONE;
    if(target && target->handle(this,MKUINT(message,SEL_RIGHTBUTTONRELEASE),ptr)) return 1;
    return 1;
    }
  return 0;
  }


// The widget lost the grab for some reason
long FXTable::onUngrabbed(FXObject* sender,FXSelector sel,void* ptr){
  FXScrollArea::onUngrabbed(sender,sel,ptr);
  flags&=~(FLAG_DODRAG|FLAG_TRYDRAG|FLAG_PRESSED|FLAG_CHANGED|FLAG_SCROLLING);
  flags|=FLAG_UPDATE;
  mode=MOUSE_NONE;
  stopAutoScroll();
  return 1;
  }

/*
// Button or Key activate
long FXTable::onActivate(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXTableItem *item;
  if(0<=current.row && 0<=current.col){
    item=cells[current.row*ncols+current.col];
    if(item && item->isEnabled()){
      if(event->state&SHIFTMASK){
        if(0<=anchor.row && 0<=anchor.col){
          selectItem(anchor.row,anchor.col);
          extendSelection(current.row,current.col);
          }
        else{
          selectItem(current.row,current.col);
          setAnchorItem(current.row,current.col);
          }
        }
      else if(event->state&CONTROLMASK){
        toggleItem(current.row,current.col);
        setAnchorItem(current.row,current.col);
        }
      else{
        killSelection();
        selectItem(current.row,current.col);
        setAnchorItem(current.row,current.col);
        }
      }
    }
  else{
    if(!(event->state&(SHIFTMASK|CONTROLMASK))) killSelection();
    }
  return 1;
  }


// Button or Key deactivate
long FXTable::onDeactivate(FXObject*,FXSelector,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  FXTableItem *item;
  setAnchorItem(current.row,current.col);
  if(event->click_count==1){
    handle(this,MKUINT(0,SEL_CLICKED),(void*)&current);
    }
  else if(event->click_count==2){
    handle(this,MKUINT(0,SEL_DOUBLECLICKED),(void*)&current);
    }
  else if(event->click_count==3){
    handle(this,MKUINT(0,SEL_TRIPLECLICKED),(void*)&current);
    }
  if(0<=current.row && 0<=current.col){
    item=cells[current.row*ncols+current.col];
    if(item && item->isEnabled()){
      handle(this,MKUINT(0,SEL_COMMAND),(void*)&current);
      }
    }
  return 1;
  }
*/


// Command message
long FXTable::onCommand(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_COMMAND),ptr);
  }


// Clicked in list
long FXTable::onClicked(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_CLICKED),ptr);
  }


// Double clicked in list; ptr may or may not point to an item
long FXTable::onDoubleClicked(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_DOUBLECLICKED),ptr);
  }


// Triple clicked in list; ptr may or may not point to an item
long FXTable::onTripleClicked(FXObject*,FXSelector,void* ptr){
  return target && target->handle(this,MKUINT(message,SEL_TRIPLECLICKED),ptr);
  }


// Toggle horizontal grid lines
long FXTable::onCmdHorzGrid(FXObject*,FXSelector,void*){
  showHorzGrid(!hgrid);
  return 1;
  }


long FXTable::onUpdHorzGrid(FXObject* sender,FXSelector,void*){
  FXuint msg=hgrid ? ID_CHECK : ID_UNCHECK;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),NULL);
  return 1;
  }


// Toggle vertical grid lines
long FXTable::onCmdVertGrid(FXObject*,FXSelector,void*){
  showVertGrid(!vgrid);
  return 1;
  }


long FXTable::onUpdVertGrid(FXObject* sender,FXSelector,void*){
  FXuint msg=vgrid ? ID_CHECK : ID_UNCHECK;
  sender->handle(this,MKUINT(msg,SEL_COMMAND),NULL);
  return 1;
  }


// Delete current column
long FXTable::onCmdDeleteColumn(FXObject*,FXSelector,void*){
  if(current.col<0) return 1;
  removeColumns(current.col,1,TRUE);
  setCurrentItem(current.row,current.col,TRUE);
  makePositionVisible(current.row,current.col);
  return 1;
  }


// Update delete current column
long FXTable::onUpdDeleteColumn(FXObject* sender,FXSelector,void*){
  if(0<=current.col && current.col<ncols && 0<ncols)
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Delete current row
long FXTable::onCmdDeleteRow(FXObject*,FXSelector,void*){
  if(current.row<0) return 1;
  removeRows(current.row,1,TRUE);
  setCurrentItem(current.row,current.col,TRUE);
  makePositionVisible(current.row,current.col);
  return 1;
  }


// Update delete current row
long FXTable::onUpdDeleteRow(FXObject* sender,FXSelector,void*){
  if(0<=current.row && current.row<nrows && 0<nrows)
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Insert new column at current
long FXTable::onCmdInsertColumn(FXObject*,FXSelector,void*){
  insertColumns(current.col<0?ncols:current.col,1,TRUE);
  setCurrentItem(current.row,current.col,TRUE);
  makePositionVisible(current.row,current.col);
  return 1;
  }


// Insert new row at current
long FXTable::onCmdInsertRow(FXObject*,FXSelector,void*){
  insertRows(current.row<0?nrows:current.row,1,TRUE);
  setCurrentItem(current.row,current.col,TRUE);
  makePositionVisible(current.row,current.col);
  return 1;
  }


// Move to previous column
long FXTable::onCmdMoveLeft(FXObject*,FXSelector,void*){
  if(current.col<1) return 1;
  setCurrentItem(current.row,current.col-1,TRUE);
  makePositionVisible(current.row,current.col);
  return 1;
  }


// Move to next column
long FXTable::onCmdMoveRight(FXObject*,FXSelector,void*){
  if(current.col>ncols-2) return 1;
  setCurrentItem(current.row,current.col+1,TRUE);
  makePositionVisible(current.row,current.col);
  return 1;
  }


// Move to previous row
long FXTable::onCmdMoveUp(FXObject*,FXSelector,void*){
  if(current.row<1) return 1;
  setCurrentItem(current.row-1,current.col,TRUE);
  makePositionVisible(current.row,current.col);
  return 1;
  }


// Move to next row
long FXTable::onCmdMoveDown(FXObject*,FXSelector,void*){
  if(current.row>nrows-2) return 1;
  setCurrentItem(current.row+1,current.col,TRUE);
  makePositionVisible(current.row,current.col);
  return 1;
  }


// Move begin of row
long FXTable::onCmdMoveHome(FXObject*,FXSelector,void*){
  setCurrentItem(current.row,0,TRUE);
  makePositionVisible(current.row,current.col);
  return 1;
  }


// Move to end of row
long FXTable::onCmdMoveEnd(FXObject*,FXSelector,void*){
  setCurrentItem(current.row,ncols-1,TRUE);
  makePositionVisible(current.row,current.col);
  return 1;
  }


// Move to top
long FXTable::onCmdMoveTop(FXObject*,FXSelector,void*){
  setCurrentItem(0,current.col,TRUE);
  makePositionVisible(current.row,current.col);
  return 1;
  }


// Move to bottom
long FXTable::onCmdMoveBottom(FXObject*,FXSelector,void*){
  setCurrentItem(nrows-1,current.col,TRUE);
  makePositionVisible(current.row,current.col);
  return 1;
  }


// Move to next page
long FXTable::onCmdMovePageDown(FXObject*,FXSelector,void*){
  FXint nr=10;
  setCurrentItem(FXMIN(current.row+nr,nrows-1),current.col,TRUE);
  makePositionVisible(current.row,current.col);
  return 1;
  }


// Move to previous page
long FXTable::onCmdMovePageUp(FXObject*,FXSelector,void*){
  FXint nr=10;
  setCurrentItem(FXMAX(current.row-nr,0),current.col,TRUE);
  makePositionVisible(current.row,current.col);
  return 1;
  }


// Select cell
long FXTable::onCmdSelectCell(FXObject*,FXSelector,void*){
  setAnchorItem(current.row,current.col);
  extendSelection(current.row,current.col,TRUE);
  return 1;
  }


// Select row
long FXTable::onCmdSelectRow(FXObject*,FXSelector,void*){
  setAnchorItem(current.row,0);
  extendSelection(current.row,ncols-1,TRUE);
  return 1;
  }


// Select column
long FXTable::onCmdSelectColumn(FXObject*,FXSelector,void*){
  setAnchorItem(0,current.col);
  extendSelection(nrows-1,current.col,TRUE);
  return 1;
  }


// Select all cells
long FXTable::onCmdSelectAll(FXObject*,FXSelector,void*){
  setAnchorItem(0,0);
  extendSelection(nrows-1,ncols-1,TRUE);
  return 1;
  }


// Deselect all cells
long FXTable::onCmdDeselectAll(FXObject*,FXSelector,void*){
  killSelection(TRUE);
  return 1;
  }


// Mark
long FXTable::onCmdMark(FXObject*,FXSelector,void*){
  setAnchorItem(current.row,current.col);
  return 1;
  }


// Extend
long FXTable::onCmdExtend(FXObject*,FXSelector,void*){
  extendSelection(current.row,current.col,TRUE);
  return 1;
  }


// Change the font
void FXTable::setFont(FXFont* fnt){
  if(!fnt){ fxerror("%s::setFont: NULL font specified.\n",getClassName()); }
  if(font!=fnt){
    font=fnt;
    recalc();
    update();
    }
  }


// Change top margin
void FXTable::setMarginTop(FXint mt){
  if(margintop!=mt){
    margintop=mt;
    recalc();
    update();
    }
  }


// Change bottom margin
void FXTable::setMarginBottom(FXint mb){
  if(marginbottom!=mb){
    marginbottom=mb;
    recalc();
    update();
    }
  }


// Change left margin
void FXTable::setMarginLeft(FXint ml){
  if(marginleft!=ml){
    marginleft=ml;
    recalc();
    update();
    }
  }


// Change right margin
void FXTable::setMarginRight(FXint mr){
  if(marginright!=mr){
    marginright=mr;
    recalc();
    update();
    }
  }


// Set text color
void FXTable::setTextColor(FXColor clr){
  if(clr!=textColor){
    textColor=clr;
    update();
    }
  }


// Set base color
void FXTable::setBaseColor(FXColor clr){
  if(clr!=baseColor){
    baseColor=clr;
    update();
    }
  }


// Set highlight color
void FXTable::setHiliteColor(FXColor clr){
  if(clr!=hiliteColor){
    hiliteColor=clr;
    update();
    }
  }


// Set shadow color
void FXTable::setShadowColor(FXColor clr){
  if(clr!=shadowColor){
    shadowColor=clr;
    update();
    }
  }


// Set border color
void FXTable::setBorderColor(FXColor clr){
  if(clr!=borderColor){
    borderColor=clr;
    update();
    }
  }


// Set select background color
void FXTable::setSelBackColor(FXColor clr){
  if(clr!=selbackColor){
    selbackColor=clr;
    update();
    }
  }


// Set selected text color
void FXTable::setSelTextColor(FXColor clr){
  if(clr!=seltextColor){
    seltextColor=clr;
    update();
    }
  }


// Change grid color
void FXTable::setGridColor(FXColor clr){
  if(clr!=gridColor){
    gridColor=clr;
    update();
    }
  }


// Change stipple color
void FXTable::setStippleColor(FXColor clr){
  if(clr!=stippleColor){
    stippleColor=clr;
    update();
    }
  }

// Change cell border color
void FXTable::setCellBorderColor(FXColor clr){
  if(clr!=cellBorderColor){
    cellBorderColor=clr;
    update();
    }
  }

// Set cell color
void FXTable::setCellColor(FXint r,FXint c,FXColor clr){
  if(clr!=cellBackColor[r&1][c&1]){
    cellBackColor[r&1][c&1]=clr;
    update();
    }
  }


// Get cell color
FXColor FXTable::getCellColor(FXint r,FXint c) const {
  return cellBackColor[r&1][c&1];
  }


// Change list style
void FXTable::setTableStyle(FXuint style){
  options=(options&~TABLE_MASK) | (style&TABLE_MASK);
  }


// Get list style
FXuint FXTable::getTableStyle() const {
  return (options&TABLE_MASK);
  }


// Change cell border width
void FXTable::setCellBorderWidth(FXint borderwidth){
  if(borderwidth!=cellBorderWidth){
    cellBorderWidth=borderwidth;
    update();
    }
  }


// Change table size to nr x nc
void FXTable::setTableSize(FXint nr,FXint nc,FXbool notify){      // FIXME
  register FXint r,c,p,q;

  // Any change?
  if(nrows!=nr || ncols!=nc){

    // Grow it now
    if((nr*nc)>(nrows*ncols)){
      if(!FXRESIZE(&cells,FXTableItem*,nr*nc)){
        fxerror("%s::setTableSize: out of memory\n",getClassName());
        }
      }

    FXTRACE((10,"nr=%d nrows=%d\n",nr,nrows));
    FXTRACE((10,"nc=%d ncols=%d\n",nc,ncols));

    // Shrinking number of columns
    if(nc<ncols){

      p=0;
      q=0;

      // Shrinking number of rows
      if(nr<nrows){
        for(r=0; r<nr; r++){
          for(c=0; c<nc; c++){
            cells[p++]=cells[q++];
            }
          for(c=nc; c<ncols; c++){
            delete cells[q++];
            }
          }
        for(r=nr; r<nrows; r++){
          for(c=0; c<ncols; c++){
            delete cells[q++];
            }
          }
        FXASSERT(p==nr*nc);
        FXASSERT(q==nrows*ncols);
        }

      // Expanding number of rows
      else{
        for(r=0; r<nrows; r++){
          for(c=0; c<nc; c++){
            cells[p++]=cells[q++];
            }
          for(c=nc; c<ncols; c++){
            delete cells[q++];
            }
          }
        for(r=nrows; r<nr; r++){
          for(c=0; c<nc; c++){
            cells[p++]=createItem("",NULL,NULL);
            }
          }
        FXASSERT(p==nr*nc);
        FXASSERT(q==nrows*ncols);
        }
      }

    // Expanding number of columns
    else{

      p=nr*nc;
      q=nrows*ncols;

      // Shrinking number of rows
      if(nr<nrows){
        for(r=nr; r<nrows; r++){
          for(c=0; c<ncols; c++){
            delete cells[--q];
            }
          }
        for(r=0; r<nr; r++){
          for(c=ncols; c<nc; c++){
            cells[--p]=createItem("",NULL,NULL);
            }
          for(c=0; c<ncols; c++){
            cells[--p]=cells[--q];
            }
          }
        FXASSERT(p==0);
        FXASSERT(q==0);
        }

      // Expanding number of rows
      else{
        for(r=nrows; r<nr; r++){
          for(c=0; c<nc; c++){
            cells[--p]=createItem("",NULL,NULL);
            }
          }
        for(r=0; r<nrows; r++){
          for(c=ncols; c<nc; c++){
            cells[--p]=createItem("",NULL,NULL);
            }
          for(c=0; c<ncols; c++){
            cells[--p]=cells[--q];
            }
          }
        FXASSERT(p==0);
        FXASSERT(q==0);
        }
      }

    // Shrink table
    if((nr*nc)<(nrows*ncols)){
      if(!FXRESIZE(&cells,FXTableItem*,nr*nc+1)){
        fxerror("%s::setTableSize: out of memory\n",getClassName());
        }
      }

    // Resize row heights
    if(!FXRESIZE(&row_y,FXint,nr+1)){
      fxerror("%s::setTableSize: out of memory\n",getClassName());
      }

    // Resize column widths
    if(!FXRESIZE(&col_x,FXint,nc+1)){
      fxerror("%s::setTableSize: out of memory\n",getClassName());
      }

    // Got new rows
    for(r=nrows; r<nr; r++){
      row_y[r+1]=row_y[r]+defRowHeight;
      }

    // Got new columns
    for(c=ncols; c<nc; c++){
      col_x[c+1]=col_x[c]+defColumnWidth;
      }

    // Try preserve leading/trailing rows
    if(nr<leading_rows){ leading_rows=nr; trailing_rows=0; }
    else if(nr<leading_rows+trailing_rows){ trailing_rows=nr-leading_rows; }

    // Try preserve leading/trailing columns
    if(nc<leading_cols){ leading_cols=nc; trailing_cols=0; }
    else if(nc<leading_cols+trailing_cols){ trailing_cols=nc-leading_cols; }

    FXASSERT(leading_rows+trailing_rows<=nr);
    FXASSERT(leading_cols+trailing_cols<=nc);

    // Fix up anchor and current
    if(anchor.col>=nc) anchor.col=-1;
    if(anchor.row>=nr) anchor.row=-1;
    if(current.col>=nc) current.col=-1;
    if(current.row>=nr) current.row=-1;

    // Update new size
    nrows=nr;
    ncols=nc;
    scrolling_rows=nrows-leading_rows-trailing_rows;
    scrolling_cols=ncols-leading_cols-trailing_cols;
    recalc();
    }
  }


// Insert a row
void FXTable::insertRows(FXint row,FXint nr,FXbool notify){
  FXTableItem **oldcells=cells;
  FXint oldrow=current.row;
  FXint r,c,s;

  // Nothing to do
  if(nr<1) return;

  // Must be in range
  if(row<0 || row>nrows){ fxerror("%s::insertRows: row out of range\n",getClassName()); }

  // Resize row heights
  if(!FXRESIZE(&row_y,FXint,nrows+nr+1)){
    fxerror("%s::insertRows: out of memory\n",getClassName());
    }

  // Space for nr new rows
  s=nr*defRowHeight;

  // Initial size of added rows is the default row height
  for(r=nrows; r>row; r--){ row_y[r+nr]=row_y[r]+s; }
  for(r=row; r<row+nr; r++){ row_y[r+1]=row_y[r]+defRowHeight; }

  // Allocate new table
  if(!FXMALLOC(&cells,FXTableItem*,(nrows+nr)*ncols)){
    fxerror("%s::insertRows: out of memory\n",getClassName());
    }

  // Copy first part
  for(r=0; r<row; r++){
    for(c=0; c<ncols; c++){
      cells[r*ncols+c]=oldcells[r*ncols+c];
      }
    }
/*
  // Initialize middle part; cells spanning over current row are not split
  for(c=0; c<ncols; c++){
    if(0<row && row<nrows && oldcells[(row-1)*ncols+c]==oldcells[row*ncols+c]){
      for(r=row; r<row+nr; r++){
        cells[r*ncols+c]=oldcells[row*ncols+c];
        }
      if(c==ncols-1 || cells[row+ncols+c]!=cells[row+ncols+c]){
        cells[row*ncols+c]->changeRowSpan(nr);      // Now spanning over nr extra rows
        }
      }
    else{
      for(r=row; r<row+nr; r++){
        cells[r*ncols+c]=createItem("",NULL,NULL);
        }
      }
    }
*/
  for(r=row; r<row+nr; r++){
    for(c=0; c<ncols; c++){
      cells[r*ncols+c]=createItem("",NULL,NULL);
      }
    }

  // Copy last part
  for(r=row; r<nrows; r++){
    for(c=0; c<ncols; c++){
      cells[(r+nr)*ncols+c]=oldcells[r*ncols+c];
      }
    }

  // Free old table
  FXFREE(&oldcells);

  nrows+=nr;
  scrolling_rows=nrows-leading_rows-trailing_rows;

  // Fix up anchor, extent, and current
  if(anchor.row>=row) anchor.row+=nr;
  if(extent.row>=row) extent.row+=nr;
  if(current.row>=row) current.row+=nr;
  if(current.row<0 && nrows==nr) current.row=0;

  // Notify items have been inserted
  if(notify && target){
    FXTableRange tablerange;
    tablerange.fm.row=row;
    tablerange.fm.col=0;
    tablerange.to.row=row+nr-1;
    tablerange.to.col=ncols-1;
    target->handle(this,MKUINT(message,SEL_INSERTED),(void*)&tablerange);
    }

  // Current item may have changed
  if(oldrow!=current.row){
    if(notify && target){ target->handle(this,MKUINT(message,SEL_CHANGED),(void*)&current); }
    }

  // Redo layout
  recalc();
  }


// Insert a column
void FXTable::insertColumns(FXint col,FXint nc,FXbool notify){
  FXTableItem **oldcells=cells;
  FXint oldcol=current.col;
  FXint r,c,s,n;

  // Nothing to do
  if(nc<1) return;

  // Must be in range
  if(col<0 || col>ncols){ fxerror("%s::insertColumns: column out of range\n",getClassName()); }

  // Resize column widths
  if(!FXRESIZE(&col_x,FXint,ncols+nc+1)){
    fxerror("%s::insertColumns: out of memory\n",getClassName());
    }

  // Space for nr new rows
  s=nc*defColumnWidth;
  n=ncols+nc;

  // Initial size of added rows is the default row height
  for(c=ncols; c>col; c--){ col_x[c+nc]=col_x[c]+s; }
  for(c=col; c<col+nc; c++){ col_x[c+1]=col_x[c]+defColumnWidth; }

  // Allocate new table
  if(!FXMALLOC(&cells,FXTableItem*,nrows*n)){
    fxerror("%s::insertColumns: out of memory\n",getClassName());
    }

  // Copy first part
  for(r=0; r<nrows; r++){
    for(c=0; c<col; c++){
      cells[r*n+c]=oldcells[r*ncols+c];
      }
    }

/*
  // Initialize middle part; cells spanning over current column are not split
  for(r=0; r<nrows; r++){
    if(0<col && col<ncols && oldcells[r*ncols+col-1]==oldcells[r*ncols+col]){
      for(c=col; c<col+nc; c++){
        cells[r*n+c]=oldcells[r*ncols+col];
        }
      cells[r*ncols+col]->changeColSpan(nc);      // Now spanning over nc extra columns
      }
    else{
      for(c=col; c<col+nc; c++){
        cells[r*n+c]=createItem("",NULL,NULL);
        }
      }
    }
*/
  for(r=0; r<nrows; r++){
    for(c=col; c<col+nc; c++){
      cells[r*n+c]=NULL;
      }
    }

  // Copy last part
  for(r=0; r<nrows; r++){
    for(c=col; c<ncols; c++){
      cells[r*n+nc+c]=oldcells[r*ncols+c];
      }
    }

  // Free old table
  FXFREE(&oldcells);

  ncols=n;
  scrolling_cols=ncols-leading_cols-trailing_cols;

  // Fix up anchor, extent, and current
  if(anchor.col>=col) anchor.col+=nc;
  if(extent.col>=col) extent.col+=nc;
  if(current.col>=col) current.col+=nc;
  if(current.col<0 && ncols==nc) current.col=0;

  // Notify items have been inserted
  if(notify && target){
    FXTableRange tablerange;
    tablerange.fm.row=0;
    tablerange.fm.col=col;
    tablerange.to.row=nrows-1;
    tablerange.to.col=col+nc-1;
    target->handle(this,MKUINT(message,SEL_INSERTED),(void*)&tablerange);
    }

  // Current item may have changed
  if(oldcol!=current.col){
    if(notify && target){ target->handle(this,MKUINT(message,SEL_CHANGED),(void*)&current); }
    }

  // Redo layout
  recalc();
  }


// Remove rows of cells
void FXTable::removeRows(FXint row,FXint nr,FXbool notify){
  FXTableItem **oldcells=cells;
  FXTableItem *item;
  FXint oldrow=current.row;
  register FXint r,c,s,n;

  // Nothing to do
  if(nr<1) return;

  // Must be in range
  if(row<0 || row+nr>nrows){ fxerror("%s::removeRows: row out of range\n",getClassName()); }

  // Notify items will be deleted
  if(notify && target){
    FXTableRange tablerange;
    tablerange.fm.row=row;
    tablerange.fm.col=0;
    tablerange.to.row=row+nr-1;
    tablerange.to.col=ncols-1;
    target->handle(this,MKUINT(message,SEL_DELETED),(void*)&tablerange);
    }

  // Space removed
  s=row_y[row+nr]-row_y[row];
  n=nrows-nr;

  // Adjust remaining rows
  for(r=row+nr+1; r<=nrows; r++){ row_y[r-nr]=row_y[r]-s; }

  // The array is one longer than nrows
  FXRESIZE(&row_y,FXint,n+1);

  // Allocate new table
  if(!FXMALLOC(&cells,FXTableItem*,n*ncols)){
    fxerror("%s::removeRows: out of memory\n",getClassName());
    }

  // Copy first part
  for(r=0; r<row; r++){
    for(c=0; c<ncols; c++){
      cells[r*ncols+c]=oldcells[r*ncols+c];
      }
    }

  // Delete those items fully contained in the deleted range
  for(r=row; r<row+nr; r++){
    for(c=0; c<ncols; c++){
      item=oldcells[r*ncols+c];
      if(item && (r==0 || oldcells[(r-1)*ncols+c]!=item) && (c==0 || oldcells[r*ncols+c-1]!=item) && (row+nr==nrows || oldcells[(row+nr)*ncols+c]!=item)){
        FXTRACE((150,"delete item %s\n",item->getText().text()));
        delete item;
        }
      }
    }

  // Copy last part
  for(r=row+nr; r<nrows; r++){
    for(c=0; c<ncols; c++){
      cells[(r-nr)*ncols+c]=oldcells[r*ncols+c];
      }
    }

  // Free old table
  FXFREE(&oldcells);

  // Fix up anchor and current
  if(anchor.row>=row+nr)  anchor.row-=nr;  else if(anchor.row>=n)  anchor.row=n-1;
  if(extent.row>=row+nr)  extent.row-=nr;  else if(extent.row>=n)  extent.row=n-1;
  if(current.row>=row+nr) current.row-=nr; else if(current.row>=n) current.row=n-1;

  // Deleted some leading rows
  if(row+nr<=leading_rows) leading_rows-=nr;
  else if(row<leading_rows) leading_rows=row;

  // Deleted some trailing rows
  if(nrows-trailing_rows<=row) trailing_rows-=nr;
  else if(nrows-trailing_rows<=row+nr) trailing_rows=nrows-row-nr;

  nrows=n;
  scrolling_rows=nrows-leading_rows-trailing_rows;

  // Current item may have changed
  if(oldrow!=current.row){
    if(notify && target){ target->handle(this,MKUINT(message,SEL_CHANGED),(void*)&current); }
    }

  // Redo layout
  recalc();
  }



// Remove columns of cells
void FXTable::removeColumns(FXint col,FXint nc,FXbool notify){
  FXTableItem **oldcells=cells;
  FXTableItem *item;
  FXint oldcol=current.col;
  register FXint r,c,s,n;

  // Nothing to do
  if(nc<1) return;

  // Must be in range
  if(col<0 || col+nc>ncols){ fxerror("%s::removeColumns: column out of range\n",getClassName()); }

  // Notify items will be deleted
  if(notify && target){
    FXTableRange tablerange;
    tablerange.fm.row=0;
    tablerange.fm.col=col;
    tablerange.to.row=nrows-1;
    tablerange.to.col=col+nc-1;
    target->handle(this,MKUINT(message,SEL_DELETED),(void*)&tablerange);
    }

  // Space removed
  s=col_x[col+nc]-col_x[col];
  n=ncols-nc;

  // Adjust remaining columns
  for(c=col+nc+1; c<=ncols; c++){ col_x[c-nc]=col_x[c]-s; }

  // The array is one longer than ncols
  FXRESIZE(&col_x,FXint,n+1);

  // Allocate new table
  if(!FXMALLOC(&cells,FXTableItem*,nrows*n)){
    fxerror("%s::removeColumns: out of memory\n",getClassName());
    }

  // Copy first part
  for(r=0; r<nrows; r++){
    for(c=0; c<col; c++){
      cells[r*n+c]=oldcells[r*ncols+c];
      }
    }

  // Delete those items fully contained in the deleted range
  for(r=0; r<nrows; r++){
    for(c=col; c<col+nc; c++){
      item=oldcells[r*ncols+c];
      if(item && (r==0 || oldcells[(r-1)*ncols+c]!=item) && (c==0 || oldcells[r*ncols+c-1]!=item) && (col+nc==ncols || oldcells[r*ncols+col+nc]!=item)){
        FXTRACE((150,"delete item %s\n",item->getText().text()));
        delete item;
        }
      }
    }

  // Copy last part
  for(r=0; r<nrows; r++){
    for(c=col+nc; c<ncols; c++){
      cells[r*n+c-nc]=oldcells[r*ncols+c];
      }
    }

  // Free old table
  FXFREE(&oldcells);

  // Fix up anchor and current
  if(anchor.col>=col+nc)  anchor.col-=nc;  else if(anchor.col>=n)  anchor.col=n-1;
  if(extent.col>=col+nc)  extent.col-=nc;  else if(extent.col>=n)  extent.col=n-1;
  if(current.col>=col+nc) current.col-=nc; else if(current.col>=n) current.col=n-1;

  // Deleted some leading columns
  if(col+nc<=leading_cols) leading_cols-=nc;
  else if(col<leading_cols) leading_cols=col;

  // Deleted some trailing columns
  if(ncols-trailing_cols<=col) trailing_cols-=nc;
  else if(ncols-trailing_cols<=col+nc) trailing_cols=ncols-col-nc;

  ncols=n;
  scrolling_cols=ncols-leading_cols-trailing_cols;

  // Current item may have changed
  if(oldcol!=current.col){
    if(notify && target){ target->handle(this,MKUINT(message,SEL_CHANGED),(void*)&current); }
    }

  // Redo layout
  recalc();
  }


// Remove cell
void FXTable::removeItem(FXint row,FXint col,FXbool notify){
  register FXint sr,er,sc,ec,r,c;

  // Must be in range
  if(row<0 || col<0 || nrows<=row || ncols<=col){ fxerror("%s::removeItem: index out of range.\n",getClassName()); }

  // Extent of cell
  sr=startRow(row,col); er=endRow(row,col);
  sc=startCol(row,col); ec=endCol(row,col);

  // Notify item will be deleted
  if(notify && target){
    FXTableRange tablerange;
    tablerange.fm.row=sr; tablerange.to.row=er-1;
    tablerange.fm.col=sc; tablerange.to.col=ec-1;
    target->handle(this,MKUINT(message,SEL_DELETED),(void*)&tablerange);
    }

  // Delete cell
  delete cells[sr*ncols+sc];

  // Clear entries
  for(r=sr; r<er; r++){
    for(c=sc; c<ec; c++){
      cells[r*ncols+c]=NULL;
      }
    }

  // Repaint these cells
  updateRange(sr,er,sc,ec);
  }



/*

  /// Merge cells into one single multi-column cell
  virtual void mergeCols(FXint row,FXint col,FXint nc,FXbool notify=FALSE);

  /// Merge cells into one single multi-row cell
  virtual void mergeRows(FXint row,FXint col,FXint nr,FXbool notify=FALSE);

  /// Split multi-column cell into single cells
  virtual void splitCols(FXint row,FXint col,FXbool notify=FALSE);

  /// Split multi-row cell into single cells
  virtual void splitRows(FXint row,FXint col,FXbool notify=FALSE);

// Merge cells into one single multi-column cell
void FXTable::mergeCols(FXint row,FXint col,FXint nc,FXbool notify){
  register FXint sr,er,ec,r,c;
  register FXTableItem *item,*it;
  if(row<0 || row>=nrows || col<0 || nc<1 || col+nc>ncols){ fxerror("%s::mergeCols: argument out of range\n",getClassName()); }
  sr=startRow(row,col);
  er=endRow(row,col);
  ec=col+nc-1;
  item=cells[row*ncols+col];
  for(r=sr; r<=er; r++){
    for(c=col+1; c<=ec; c++){
      it=cells[r*ncols+c];
      if(it && it!=item){
        removeCell(r,c,notify);
        }
      cells[r*ncols+c]=item;
      }
    }
  }


// Merge cells into one single multi-row cell
void FXTable::mergeRows(FXint row,FXint col,FXint nr,FXbool notify){
  register FXint er,sc,ec,r,c;
  register FXTableItem *item,*it;
  if(col<0 || col>=ncols || row<0 || nr<1 || row+nr>nrows){ fxerror("%s::mergeRows: argument out of range\n",getClassName()); }
  sc=startCol(row,col);
  ec=endCol(row,col);
  er=row+nr-1;
  item=cells[row*ncols+col];
  for(r=row+1; r<=er; r++){
    for(c=sc; c<=ec; c++){
      it=cells[r*ncols+c];
      if(it && it!=item){
        removeCell(r,c,notify);
        }
      cells[r*ncols+c]=item;
      }
    }
  }


// Split multi-column cell into single cells
void FXTable::splitCols(FXint row,FXint col,FXbool notify){
  if(row<0 || row>=nrows || col<0 || col>=ncols){ fxerror("%s::splitCols: argument out of range\n",getClassName()); }
  }


// Split multi-row cell into single cells
void FXTable::splitRows(FXint row,FXint col,FXbool notify){
  if(col<0 || col>=ncols || row<0 || row>=nrows){ fxerror("%s::splitRows: argument out of range\n",getClassName()); }
  }

*/


// Change width of custom column
void FXTable::setColumnWidth(FXint col,FXint cwidth){
  register FXint i,d;
  if(col<0 || col>=ncols){ fxerror("%s::setColumnWidth: column out of range\n",getClassName()); }
  if(cwidth<0) cwidth=0;
  d=cwidth-col_x[col+1]+col_x[col];
  if(d!=0){
    for(i=col+1; i<=ncols; i++){ col_x[i]+=d; }
    recalc();
    }
  }


// Get width of custom column
FXint FXTable::getColumnWidth(FXint col) const {
  if(col<0 || col>=ncols){ fxerror("%s::getColumnWidth: column out of range\n",getClassName()); }
  return col_x[col+1]-col_x[col];
  }


// Change height of custom row
void FXTable::setRowHeight(FXint row,FXint rheight){
  register FXint i,d;
  if(row<0 || row>=nrows){ fxerror("%s::setRowHeight: row out of range\n",getClassName()); }
  if(rheight<0) rheight=0;
  d=rheight-row_y[row+1]+row_y[row];
  if(d!=0){
    for(i=row+1; i<=nrows; i++){ row_y[i]+=d; }
    recalc();
    }
  }


// Get height of custom row
FXint FXTable::getRowHeight(FXint row) const {
  if(row<0 || row>=nrows){ fxerror("%s::getRowHeight: row out of range\n",getClassName()); }
  return row_y[row+1]-row_y[row];
  }


// Change X coordinate of column c
void FXTable::setColumnX(FXint col,FXint x){
  register FXint i,d;
  if(col<0 || col>ncols){ fxerror("%s::setColumnX: column out of range\n",getClassName()); }
  if(col==0) return;
//   if(ncols-trailing_cols<=col){
//     if(table_right+vgrid==viewport_w && col==ncols) return;
//     x=x-scrollable_right+col_x[ncols-trailing_cols];
//     col_x[col]=x;
//     recalc();
//     return;
//     }
  if(ncols-trailing_cols<=col) x=x-scrollable_right+col_x[ncols-trailing_cols];
  else if(leading_cols<col) x=x-pos_x;
  if(x<col_x[col-1]) x=col_x[col-1];
  d=x-col_x[col];
  if(d!=0){
    for(i=col; i<=ncols; i++){ col_x[i]+=d; }
    recalc();
    }
  }


// X coordinate of column c
FXint FXTable::getColumnX(FXint col) const {
  if(col<0 || col>ncols){ fxerror("%s::getColumnX: column out of range\n",getClassName()); }
  register FXint x=col_x[col];
  if(ncols-trailing_cols<=col) x=x+scrollable_right-col_x[ncols-trailing_cols];
  else if(leading_cols<=col) x=x+pos_x;
  return x;
  }


// Change Y coordinate of row r
void FXTable::setRowY(FXint row,FXint y){
  register FXint i,d;
  if(row<0 || row>nrows){ fxerror("%s::setRowY: row out of range\n",getClassName()); }
  if(row==0) return;
  if(nrows-trailing_rows<=row) y=y-scrollable_bottom+row_y[nrows-trailing_rows];
  else if(leading_rows<row) y=y-pos_y;
  if(y<row_y[row-1]) y=row_y[row-1];
  d=y-row_y[row];
  if(d!=0){
    for(i=row; i<=nrows; i++){ row_y[i]+=d; }
    recalc();
    }
  }


// Y coordinate of row r
FXint FXTable::getRowY(FXint row) const {
  if(row<0 || row>nrows){ fxerror("%s::getRowY: row out of range\n",getClassName()); }
  register FXint y=row_y[row];
  if(nrows-trailing_rows<=row) y=y+scrollable_bottom-row_y[nrows-trailing_rows];
  else if(leading_rows<=row) y=y+pos_y;
  return y;
  }


// Change default column width
void FXTable::setDefColumnWidth(FXint cwidth){
  if(defColumnWidth!=cwidth){
    defColumnWidth=cwidth;
    recalc();
    }
  }


// Change default row height
void FXTable::setDefRowHeight(FXint rheight){
  if(defRowHeight!=rheight){
    defRowHeight=rheight;
    recalc();
    }
  }


// Set leading fixed rows
void FXTable::setLeadingRows(FXint leadrows){
  if(leadrows<0 || leadrows>nrows-trailing_rows){ fxerror("%s::setLeadingRows: number out of range\n",getClassName()); }
  if(leadrows!=leading_rows){
    leading_rows=leadrows;
    scrolling_rows=nrows-leading_rows-trailing_rows;
    recalc();
    }
  }


// Set leading fixed columns
void FXTable::setLeadingCols(FXint leadcols){
  if(leadcols<0 || leadcols>ncols-trailing_cols){ fxerror("%s::setLeadingCols: number out of range\n",getClassName()); }
  if(leadcols!=leading_cols){
    leading_cols=leadcols;
    scrolling_cols=ncols-leading_cols-trailing_cols;
    recalc();
    }
  }


// Set trailing fixed rows
void FXTable::setTrailingRows(FXint trailrows){
  if(trailrows<0 || trailrows>nrows-leading_rows){ fxerror("%s::setTrailingRows: number out of range\n",getClassName()); }
  if(trailrows!=trailing_rows){
    trailing_rows=trailrows;
    scrolling_rows=nrows-leading_rows-trailing_rows;
    recalc();
    }
  }


// Set trailing fixed columns
void FXTable::setTrailingCols(FXint trailcols){
  if(trailcols<0 || trailcols>ncols-leading_cols){ fxerror("%s::setTrailingCols: number out of range\n",getClassName()); }
  if(trailcols!=trailing_cols){
    trailing_cols=trailcols;
    scrolling_cols=ncols-leading_cols-trailing_cols;
    recalc();
    }
  }


// Change visible rows
void FXTable::setVisibleRows(FXint nvrows){
  if(nvrows<0) nvrows=0;
  if(visiblerows!=nvrows){
    visiblerows=nvrows;
    recalc();
    }
  }


// Change visible columns
void FXTable::setVisibleCols(FXint nvcols){
  if(nvcols<0) nvcols=0;
  if(visiblecols!=nvcols){
    visiblecols=nvcols;
    recalc();
    }
  }


// Show or hide horizontal grid
void FXTable::showHorzGrid(FXbool on){
  if(hgrid!=on){
    hgrid=on;
    recalc();
    }
  }


// Show or hide vertical grid
void FXTable::showVertGrid(FXbool on){
  if(vgrid!=on){
    vgrid=on;
    recalc();
    }
  }


// Change help text
void FXTable::setHelpText(const FXString& text){
  help=text;
  }


// Save data
void FXTable::save(FXStream& store) const {
  register FXint i;
  FXScrollArea::save(store);
  store << nrows;
  store << ncols;
  for(i=0; i<nrows*ncols; i++) store << cells[i];
  store.save(col_x,ncols+2);
  store.save(row_y,nrows+2);
  store << visiblerows;
  store << visiblecols;
  store << margintop;
  store << marginbottom;
  store << marginleft;
  store << marginright;
  store << textColor;
  store << baseColor;
  store << hiliteColor;
  store << shadowColor;
  store << borderColor;
  store << selbackColor;
  store << seltextColor;
  store << gridColor;
  store << cellBackColor[0][0];
  store << cellBackColor[0][1];
  store << cellBackColor[1][0];
  store << cellBackColor[1][1];
  store << font;
  store << help;
  }


// Load data
void FXTable::load(FXStream& store){
  register FXint i;
  FXScrollArea::load(store);
  store >> nrows;
  store >> ncols;
  FXMALLOC(&cells,FXTableItem*,nrows*ncols);
  FXMALLOC(&col_x,FXint,ncols+2);
  FXMALLOC(&row_y,FXint,nrows+2);
  for(i=0; i<nrows*ncols; i++) store >> cells[i];
  store.load(col_x,ncols+2);
  store.load(row_y,nrows+2);
  store >> visiblerows;
  store >> visiblecols;
  store >> margintop;
  store >> marginbottom;
  store >> marginleft;
  store >> marginright;
  store >> textColor;
  store >> baseColor;
  store >> hiliteColor;
  store >> shadowColor;
  store >> borderColor;
  store >> selbackColor;
  store >> seltextColor;
  store >> gridColor;
  store >> cellBackColor[0][0];
  store >> cellBackColor[0][1];
  store >> cellBackColor[1][0];
  store >> cellBackColor[1][1];
  store >> font;
  store >> help;
  }


// Clean up
FXTable::~FXTable(){
  if(blinker){ getApp()->removeTimeout(blinker); }
  for(FXint r=0; r<nrows; r++){
    for(FXint c=0; c<ncols; c++){
      FXTableItem* item=cells[r*ncols+c];
      if(item && (r==0 || cells[(r-1)*ncols+c]!=item) && (c==0 || cells[r*ncols+c-1]!=item)){
        delete item;
        }
      }
    }
  FXFREE(&cells);
  FXFREE(&col_x);
  FXFREE(&row_y);
  cells=(FXTableItem**)-1;
  col_x=(FXint*)-1;
  row_y=(FXint*)-1;
  font=(FXFont*)-1;
  blinker=(FXTimer*)-1;
  }


