/********************************************************************************
*                                                                               *
*                                 Test Table Widget                             *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997 by Jeroen van der Zijp.   All Rights Reserved.             *
*********************************************************************************
* $Id: table.cpp,v 1.36 2001/09/19 06:09:26 jeroen Exp $                        *
********************************************************************************/
#include "fx.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>




/*******************************************************************************/


// Table Window
class TableWindow : public FXMainWindow {
  FXDECLARE(TableWindow)
protected:

  // Member data
  FXTooltip*         tooltip;
  FXMenubar*         menubar;
  FXMenuPane*        filemenu;
  FXMenuPane*        tablemenu;
  FXMenuPane*        manipmenu;
  FXMenuPane*        selectmenu;
  FXVerticalFrame*   contents;
  FXVerticalFrame*   frame;
  FXTable*           table;
  FXBMPIcon*         penguinicon;

protected:
  TableWindow(){}

public:
  long onCmdTest(FXObject*,FXSelector,void*);
  long onCmdResizeTable(FXObject*,FXSelector,void*);
  long onTableSelected(FXObject*,FXSelector,void*);
  long onTableDeselected(FXObject*,FXSelector,void*);
  long onTableInserted(FXObject*,FXSelector,void*);
  long onTableDeleted(FXObject*,FXSelector,void*);
  long onTableChanged(FXObject*,FXSelector,void*);

public:
  enum{
    ID_TEST=FXMainWindow::ID_LAST,
    ID_RESIZETABLE,
    ID_TABLE,
    ID_LAST
    };

public:
  TableWindow(FXApp* a);
  virtual void create();
  virtual ~TableWindow();
  };


/*******************************************************************************/

// Map
FXDEFMAP(TableWindow) TableWindowMap[]={
  FXMAPFUNC(SEL_COMMAND,TableWindow::ID_TEST,TableWindow::onCmdTest),
  FXMAPFUNC(SEL_COMMAND,TableWindow::ID_RESIZETABLE,TableWindow::onCmdResizeTable),
  FXMAPFUNC(SEL_SELECTED,TableWindow::ID_TABLE,TableWindow::onTableSelected),
  FXMAPFUNC(SEL_DESELECTED,TableWindow::ID_TABLE,TableWindow::onTableDeselected),
  FXMAPFUNC(SEL_INSERTED,TableWindow::ID_TABLE,TableWindow::onTableInserted),
  FXMAPFUNC(SEL_DELETED,TableWindow::ID_TABLE,TableWindow::onTableDeleted),
  FXMAPFUNC(SEL_CHANGED,TableWindow::ID_TABLE,TableWindow::onTableChanged),
  };


// Object implementation
FXIMPLEMENT(TableWindow,FXMainWindow,TableWindowMap,ARRAYNUMBER(TableWindowMap))


/*******************************************************************************/

extern const unsigned char penguin[]={
  0x42,0x4d,0xe2,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x52,0x00,0x00,0x00,0x28,0x00,
  0x00,0x00,0x10,0x00,0x00,0x00,0x12,0x00,0x00,0x00,0x01,0x00,0x04,0x00,0x00,0x00,
  0x00,0x00,0x90,0x00,0x00,0x00,0x6d,0x0b,0x00,0x00,0x6d,0x0b,0x00,0x00,0x07,0x00,
  0x00,0x00,0x07,0x00,0x00,0x00,0xdc,0xc0,0xb2,0x00,0x80,0x80,0x80,0x00,0x00,0x00,
  0x00,0x00,0xc0,0xc0,0xc0,0x00,0x10,0x10,0x10,0x00,0xff,0xff,0xff,0x00,0x08,0xa0,
  0xe0,0x00,0x00,0x66,0x61,0x22,0x22,0x21,0x66,0x00,0x06,0x66,0x66,0x55,0x22,0x26,
  0x66,0x60,0x66,0x66,0x65,0x55,0x55,0x26,0x66,0x66,0x66,0x66,0x65,0x55,0x55,0x36,
  0x66,0x66,0x06,0x66,0x55,0x55,0x55,0x56,0x22,0x60,0x00,0x63,0x55,0x35,0x55,0x55,
  0x22,0x20,0x00,0x23,0x55,0x35,0x55,0x55,0x22,0x20,0x00,0x02,0x55,0x33,0x55,0x55,
  0x22,0x10,0x00,0x02,0x55,0x53,0x55,0x55,0x22,0x00,0x00,0x01,0x25,0x55,0x55,0x51,
  0x21,0x00,0x00,0x00,0x21,0x55,0x55,0x52,0x10,0x00,0x00,0x00,0x02,0x66,0x55,0x22,
  0x00,0x00,0x00,0x00,0x02,0x66,0x63,0x21,0x00,0x00,0x00,0x00,0x02,0x66,0x63,0x20,
  0x00,0x00,0x00,0x00,0x02,0x42,0x25,0x20,0x00,0x00,0x00,0x00,0x02,0x32,0x32,0x20,
  0x00,0x00,0x00,0x00,0x01,0x22,0x22,0x10,0x00,0x00,0x00,0x00,0x00,0x12,0x21,0x00,
  0x00,0x00
  };


// Make some windows
TableWindow::TableWindow(FXApp* a):FXMainWindow(a,"Table Widget Test",NULL,NULL,DECOR_ALL,0,0,0,0){
  static const FXchar *months[]={"January","February","March","April","May","June","July","August","September","October","November","December"};
  FXint r,c;

  // Tooltip
  tooltip=new FXTooltip(getApp());

  penguinicon=new FXBMPIcon(getApp(),penguin,0,IMAGE_ALPHAGUESS);
  //penguinicon=new FXBMPIcon(getApp(),penguin,0,IMAGE_OPAQUE);

  // Menubar
  menubar=new FXMenubar(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X);

  // Separator
  new FXHorizontalSeparator(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|SEPARATOR_GROOVE);

  // Contents
  contents=new FXVerticalFrame(this,LAYOUT_SIDE_TOP|FRAME_NONE|LAYOUT_FILL_X|LAYOUT_FILL_Y);

  frame=new FXVerticalFrame(contents,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);

  // Table
  //table=new FXTable(frame,50,13,NULL,0,TABLE_HOR_GRIDLINES|TABLE_VER_GRIDLINES|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 2,2,2,2);
  table=new FXTable(frame,20,8,this,ID_TABLE,TABLE_COL_SIZABLE|TABLE_ROW_SIZABLE|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 2,2,2,2);
  table->setTableSize(50,14);
//   table->setLeadingRows(3);
//   table->setLeadingCols(3);
//   table->setTrailingRows(3);
//   table->setTrailingCols(3);
   table->setLeadingRows(1);
   table->setLeadingCols(1);
   table->setTrailingRows(1);
   table->setTrailingCols(1);

  // Initialize first/last fixed rows
  for(c=1; c<=12; c++){
    table->setItemText(0,c,months[c-1]);
    table->setItemText(49,c,months[c-1]);
    table->getItem(0,c)->setButton(TRUE);
    }
  table->getItem(0,0)->setButton(TRUE);
  table->getItem(0,13)->setButton(TRUE);

  // Initialize first/last fixed columns
  for(r=1; r<=48; r++){
    table->setItemText(r,0,FXStringVal(r));
    table->setItemText(r,13,FXStringVal(r));
    table->getItem(r,0)->setButton(TRUE);
    }
  table->getItem(49,0)->setButton(TRUE);
  table->getItem(4,0)->setPressed(TRUE);

  // Initialize scrollable part of table
  for(r=1; r<=48; r++){
    for(c=1; c<=12; c++){
      table->setItemText(r,c,"r:"+FXStringVal(r)+" c:"+FXStringVal(c));
      }
    }

  table->setItemText(10,10,"This is multi-\nline text");
  table->setRowHeight(10,35);
  table->getItem(10,10)->setJustify(0);

  table->setItem(3,3,NULL);
  table->setItem(5,6,table->getItem(5,5));
  table->setItem(5,7,table->getItem(5,5));
  table->setItemText(5,5,"Spanning Item");
  table->getItem(5,5)->setJustify(0);

  table->getItem(9,9)->setBorders(FXTableItem::TBORDER|FXTableItem::LBORDER|FXTableItem::BBORDER);
  table->getItem(9,10)->setBorders(FXTableItem::TBORDER|FXTableItem::RBORDER|FXTableItem::BBORDER);

  table->getItem(40,13)->setBorders(FXTableItem::LBORDER|FXTableItem::TBORDER|FXTableItem::RBORDER|FXTableItem::BBORDER);
  table->getItem(49,13)->setBorders(FXTableItem::LBORDER|FXTableItem::TBORDER|FXTableItem::RBORDER|FXTableItem::BBORDER);
  table->getItem(5,0)->setBorders(FXTableItem::LBORDER|FXTableItem::TBORDER|FXTableItem::RBORDER|FXTableItem::BBORDER);

  table->getItem(6,6)->setIcon(penguinicon);
  table->getItem(6,6)->setIconPosition(FXTableItem::ABOVE);
  table->getItem(6,6)->setJustify(0);

  table->getItem(3,4)->setStipple(STIPPLE_CROSSDIAG);

  // File Menu
  filemenu=new FXMenuPane(this);
  new FXMenuCommand(filemenu,"&Quit\tCtl-Q",NULL,getApp(),FXApp::ID_QUIT);
  new FXMenuTitle(menubar,"&File",NULL,filemenu);

  // Tab side
  tablemenu=new FXMenuPane(this);
  new FXMenuCommand(tablemenu,"Horizontal grid",NULL,table,FXTable::ID_HORZ_GRID);
  new FXMenuCommand(tablemenu,"Vertical grid",NULL,table,FXTable::ID_VERT_GRID);
  new FXMenuTitle(menubar,"&Options",NULL,tablemenu);

  manipmenu=new FXMenuPane(this);
  new FXMenuCommand(manipmenu,"Delete Column\tCtl-C",NULL,table,FXTable::ID_DELETE_COLUMN);
  new FXMenuCommand(manipmenu,"Delete Row\tCtl-R",NULL,table,FXTable::ID_DELETE_ROW);
  new FXMenuCommand(manipmenu,"Insert Column\tCtl-Shift-C",NULL,table,FXTable::ID_INSERT_COLUMN);
  new FXMenuCommand(manipmenu,"Insert Row\tCtl-Shift-R",NULL,table,FXTable::ID_INSERT_ROW);
  new FXMenuCommand(manipmenu,"Resize table...",NULL,this,TableWindow::ID_RESIZETABLE);
  new FXMenuTitle(menubar,"&Manipulations",NULL,manipmenu);

  selectmenu=new FXMenuPane(this);
  new FXMenuCommand(selectmenu,"Select All",NULL,table,FXTable::ID_SELECT_ALL);
  new FXMenuCommand(selectmenu,"Select Cell",NULL,table,FXTable::ID_SELECT_CELL);
  new FXMenuCommand(selectmenu,"Select Row",NULL,table,FXTable::ID_SELECT_ROW);
  new FXMenuCommand(selectmenu,"Select Column",NULL,table,FXTable::ID_SELECT_COLUMN);
  new FXMenuCommand(selectmenu,"Deselect All",NULL,table,FXTable::ID_DESELECT_ALL);
  new FXMenuTitle(menubar,"&Selection",NULL,selectmenu);
  }


TableWindow::~TableWindow(){
  delete filemenu;
  delete tablemenu;
  delete manipmenu;
  delete selectmenu;
  delete penguinicon;
  }


// Test
long TableWindow::onCmdTest(FXObject*,FXSelector,void*){
  return 1;
  }


// Resize table
long TableWindow::onCmdResizeTable(FXObject*,FXSelector,void*){
  FXDialogBox dlg(this,"Resize Table");
  FXHorizontalFrame *frame=new FXHorizontalFrame(&dlg,LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXLabel(frame,"Rows:",NULL,LAYOUT_SIDE_LEFT|LAYOUT_CENTER_Y);
  FXTextField* rows=new FXTextField(frame,5,NULL,0,JUSTIFY_RIGHT|FRAME_SUNKEN|FRAME_THICK|LAYOUT_SIDE_LEFT|LAYOUT_CENTER_Y);
  new FXLabel(frame,"Columns:",NULL,LAYOUT_SIDE_LEFT|LAYOUT_CENTER_Y);
  FXTextField* cols=new FXTextField(frame,5,NULL,0,JUSTIFY_RIGHT|FRAME_SUNKEN|FRAME_THICK|LAYOUT_SIDE_LEFT|LAYOUT_CENTER_Y);
  new FXButton(frame,"Cancel",NULL,&dlg,FXDialogBox::ID_CANCEL,FRAME_RAISED|FRAME_THICK|LAYOUT_SIDE_LEFT|LAYOUT_CENTER_Y);
  new FXButton(frame,"  OK  ",NULL,&dlg,FXDialogBox::ID_ACCEPT,FRAME_RAISED|FRAME_THICK|LAYOUT_SIDE_LEFT|LAYOUT_CENTER_Y);
  FXint oldnr,oldnc;
  oldnr=table->getNumRows();
  oldnc=table->getNumCols();
  rows->setText(FXStringVal(oldnr));
  cols->setText(FXStringVal(oldnc));
  if(dlg.execute()){
    FXint nr,nc,r,c;
    nr=FXIntVal(rows->getText());
    nc=FXIntVal(cols->getText());
    if(nr<0) nr=0;
    if(nc<0) nc=0;
    table->setTableSize(nr,nc);
    for(r=0; r<nr; r++){
      for(c=0; c<nc; c++){
        if(r>oldnr || c>oldnc){
          table->setItemText(r,c,"r:"+FXStringVal(r+1)+" c:"+FXStringVal(c+1));
          }
        }
      }
    }
  return 1;
  }

// Selected
long TableWindow::onTableSelected(FXObject*,FXSelector,void* ptr){
  FXTableRange *tr=(FXTableRange*)ptr;
  FXTRACE((10,"SEL_SELECTED fm.row=%d, fm.col=%d to.row=%d, to.col=%d\n",tr->fm.row,tr->fm.col,tr->to.row,tr->to.col));
  return 1;
  }

// Deselected
long TableWindow::onTableDeselected(FXObject*,FXSelector,void* ptr){
  FXTableRange *tr=(FXTableRange*)ptr;
  FXTRACE((10,"SEL_DESELECTED fm.row=%d, fm.col=%d to.row=%d, to.col=%d\n",tr->fm.row,tr->fm.col,tr->to.row,tr->to.col));
  return 1;
  }

// Inserted
long TableWindow::onTableInserted(FXObject*,FXSelector,void* ptr){
  FXTableRange *tr=(FXTableRange*)ptr;
  FXTRACE((10,"SEL_INSERTED fm.row=%d, fm.col=%d to.row=%d, to.col=%d\n",tr->fm.row,tr->fm.col,tr->to.row,tr->to.col));
  return 1;
  }

// Deleted
long TableWindow::onTableDeleted(FXObject*,FXSelector,void* ptr){
  FXTableRange *tr=(FXTableRange*)ptr;
  FXTRACE((10,"SEL_DELETED fm.row=%d, fm.col=%d to.row=%d, to.col=%d\n",tr->fm.row,tr->fm.col,tr->to.row,tr->to.col));
  return 1;
  }

// Changed
long TableWindow::onTableChanged(FXObject*,FXSelector,void* ptr){
  FXTablePos *tp=(FXTablePos*)ptr;
  FXTRACE((10,"SEL_CHANGED row=%d, col=%d\n",tp->row,tp->col));
  return 1;
  }


// Start
void TableWindow::create(){
  FXMainWindow::create();
  show(PLACEMENT_SCREEN);
  }


/*******************************************************************************/


// Start the whole thing
int main(int argc,char *argv[]){

  // Make application
  FXApp application("TableApp","FoxTest");

  // Open display
  application.init(argc,argv);

  // Make window
  new TableWindow(&application);

  // Create app
  application.create();

  // Run
  return application.run();
  }


