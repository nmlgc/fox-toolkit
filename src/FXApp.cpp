/********************************************************************************
*                                                                               *
*                     A p p l i c a t i o n   O b j e c t                       *
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
* $Id: FXApp.cpp,v 1.107 1998/10/28 07:58:57 jeroen Exp $                       *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "fxkeys.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXObject.h"
#include "FXAccelTable.h"
#include "FXObjectList.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXCursor.h"
#include "FXFont.h"
#include "FXDrawable.h"
#include "FXImage.h"
#include "FXIcon.h"
#include "FXGIFIcon.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXString.h"
#include "FXLabel.h"
#include "FXButton.h"
#include "FXComposite.h"
#include "FXRootWindow.h"
#include "FXShell.h"
#include "FXTopWindow.h"
#include "FXDialogBox.h"
#include "FXMessageBox.h"


/*
  Notes:
  
  - The alloc color stuff should perhaps go elsewhere.
  - Colors should be obtained in the window, as they are colormap/visual
    dependent, and different windows can have different maps.
  - Fix gray bitmap stuff
  - Do we still need both gray pixmap and bitmap?
  - Move all compiled-in resources to file FXResources.cc
  - We need a ``device context'' concept, which embodies colormaps, palettes, GC, etc.
  - Should not be able to DND drop into some windows while modal window is up
  - Perhaps combine delayed paint, scroll, and layout into one mechanism...
  - refresh() now forces total GUI updating; flush() updates display only.
  - Need way to inquire about DND type name
  - Need some sort of journal facility.
  - Should not always combine rectangles; it might get too big...
  - Sender is the FXApp, but in case of DND ops, we might have a special stand-in object
    as the sender (e.g. so we can send it messages).
  - Need two constructors for FXApp: one for deserialize, one normal...
*/


#ifdef _HPUX_SOURCE
#define SELECT(n,r,w,e,t)  select(n,(int*)(r),(int*)(w),(int*)(e),t)
#else
#if defined(_POWER) || defined(_IBMR2)
#define SELECT(n,r,w,e,t)  select(n,(void*)(r),(void*)(w),(void*)(e),t)
extern "C" int gettimeofday(struct timeval *tv, struct timezone *tz);
#define bzero(ptr,size) memset(ptr,0,size)
#else
#define SELECT(n,r,w,e,t)  select(n,r,w,e,t)
#endif
#endif


/*******************************************************************************/

// A Timer object
struct FXTimer {
  FXTimer       *next;              // Next timeout in list
  FXObject      *target;            // Receiver object
  FXSelector     message;           // Message sent to receiver
  struct timeval due;               // When time is due
  };

  
// An Idle object
struct FXChore {
  FXChore       *next;              // Next chore in list
  FXObject      *target;            // Receiver object
  FXSelector     message;           // Message sent to receiver
  };


// A repaint event record
struct FXRepaint {
  FXRepaint     *next;              // Next repaint in list
  FXID           window;            // Window ID of the dirty window
  FXRectangle    rect;              // Dirty rectangle
  FXbool         synth;             // Synthetic expose event or real one?
  //FXint          hint;
  };

  
   
/*******************************************************************************/


// Horizontal splitter cursor
#define hsplit_width 24
#define hsplit_height 24
#define hsplit_x_hot 11
#define hsplit_y_hot 11
static const FXchar hsplit_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x63, 0x00, 0x00, 0x63, 0x00, 0x00, 0x63, 0x00,
   0x00, 0x63, 0x00, 0x00, 0x63, 0x00, 0x00, 0x63, 0x00, 0x00, 0x63, 0x00,
   0x20, 0x63, 0x02, 0x30, 0x63, 0x06, 0x38, 0x63, 0x0e, 0xfc, 0xe3, 0x1f,
   0x38, 0x63, 0x0e, 0x30, 0x63, 0x06, 0x20, 0x63, 0x02, 0x00, 0x63, 0x00,
   0x00, 0x63, 0x00, 0x00, 0x63, 0x00, 0x00, 0x63, 0x00, 0x00, 0x63, 0x00,
   0x00, 0x63, 0x00, 0x00, 0x63, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define hsplit_mask_width 24
#define hsplit_mask_height 24
#define hsplit_mask_x_hot 11
#define hsplit_mask_y_hot 11
static const FXchar hsplit_mask_bits[] = {
   0x80, 0xf7, 0x00, 0x80, 0xf7, 0x00, 0x80, 0xf7, 0x00, 0x80, 0xf7, 0x00,
   0x80, 0xf7, 0x00, 0x80, 0xf7, 0x00, 0x80, 0xf7, 0x00, 0xe0, 0xf7, 0x03,
   0xf0, 0xf7, 0x07, 0xf8, 0xf7, 0x0f, 0xfc, 0xf7, 0x1f, 0xfe, 0xf7, 0x3f,
   0xfc, 0xf7, 0x1f, 0xf8, 0xf7, 0x0f, 0xf0, 0xf7, 0x07, 0xe0, 0xf7, 0x03,
   0x80, 0xf7, 0x00, 0x80, 0xf7, 0x00, 0x80, 0xf7, 0x00, 0x80, 0xf7, 0x00,
   0x80, 0xf7, 0x00, 0x80, 0xf7, 0x00, 0x80, 0xf7, 0x00, 0x00, 0x00, 0x00};


// Vertical splitter cursor
#define vsplit_width 24
#define vsplit_height 24
#define vsplit_x_hot 11
#define vsplit_y_hot 11
static const FXchar vsplit_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x1c, 0x00,
   0x00, 0x3e, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x00,
   0xfe, 0xff, 0x3f, 0xfe, 0xff, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xfe, 0xff, 0x3f, 0xfe, 0xff, 0x3f, 0x00, 0x08, 0x00,
   0x00, 0x08, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x1c, 0x00,
   0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define vsplit_mask_width 24
#define vsplit_mask_height 24
#define vsplit_mask_x_hot 11
#define vsplit_mask_y_hot 11
static const FXchar vsplit_mask_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x3e, 0x00,
   0x00, 0x7f, 0x00, 0x80, 0xff, 0x00, 0x80, 0xff, 0x00, 0xff, 0xff, 0x7f,
   0xff, 0xff, 0x7f, 0xff, 0xff, 0x7f, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x00,
   0xff, 0xff, 0x7f, 0xff, 0xff, 0x7f, 0xff, 0xff, 0x7f, 0xff, 0xff, 0x7f,
   0x80, 0xff, 0x00, 0x80, 0xff, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x3e, 0x00,
   0x00, 0x1c, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

   
// Corner resize handle cursor
#define resize_width 24
#define resize_height 24
#define resize_x_hot 9
#define resize_y_hot 9
static const FXchar resize_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x3c, 0x00, 0x00,
   0x5c, 0x00, 0x00, 0x8c, 0x00, 0x00, 0x14, 0x01, 0x00, 0x24, 0x02, 0x00,
   0x40, 0x04, 0x00, 0x80, 0x08, 0x00, 0x00, 0x11, 0x00, 0x00, 0x22, 0x01,
   0x00, 0x44, 0x01, 0x00, 0x88, 0x01, 0x00, 0xd0, 0x01, 0x00, 0xe0, 0x01,
   0x00, 0xf8, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define resize_mask_width 24
#define resize_mask_height 24
#define resize_mask_x_hot 9
#define resize_mask_y_hot 9
static const FXchar resize_mask_bits[] = {
   0x00, 0x00, 0x00, 0xfe, 0x01, 0x00, 0xfe, 0x01, 0x00, 0xfe, 0x01, 0x00,
   0xfe, 0x00, 0x00, 0xfe, 0x01, 0x00, 0xfe, 0x03, 0x00, 0xfe, 0x07, 0x00,
   0xee, 0x0f, 0x00, 0xc0, 0x1f, 0x00, 0x80, 0xbf, 0x03, 0x00, 0xff, 0x03,
   0x00, 0xfe, 0x03, 0x00, 0xfc, 0x03, 0x00, 0xf8, 0x03, 0x00, 0xfc, 0x03,
   0x00, 0xfc, 0x03, 0x00, 0xfc, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


// Color swatch drag-and-drop cursor
#define swatch_width 24
#define swatch_height 24
#define swatch_x_hot 5
#define swatch_y_hot 5
static const FXchar swatch_bits[] = {
   0x00, 0x00, 0x00, 0xfe, 0x07, 0x00, 0xfe, 0x07, 0x00, 0xfe, 0x07, 0x00,
   0x8e, 0x07, 0x00, 0x2e, 0x07, 0x00, 0x6e, 0x06, 0x00, 0xee, 0x04, 0x00,
   0xee, 0x01, 0x00, 0xee, 0x03, 0x00, 0xee, 0x07, 0x00, 0xe0, 0x0f, 0x00,
   0xe0, 0x1f, 0x00, 0xe0, 0x03, 0x00, 0x60, 0x03, 0x00, 0x20, 0x06, 0x00,
   0x00, 0x06, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define swatch_mask_width 24
#define swatch_mask_height 24
#define swatch_mask_x_hot 5
#define swatch_mask_y_hot 5
static const FXchar swatch_mask_bits[] = {
   0xff, 0x0f, 0x00, 0xff, 0x0f, 0x00, 0xff, 0x0f, 0x00, 0xff, 0x0f, 0x00,
   0xff, 0x0f, 0x00, 0xff, 0x0f, 0x00, 0xff, 0x0f, 0x00, 0xff, 0x0f, 0x00,
   0xff, 0x0f, 0x00, 0xff, 0x07, 0x00, 0xff, 0x0f, 0x00, 0xff, 0x1f, 0x00,
   0xf0, 0x3f, 0x00, 0xf0, 0x3f, 0x00, 0xf0, 0x07, 0x00, 0x70, 0x0f, 0x00,
   0x30, 0x0f, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x1e, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// NO DROP drag-and-drop cursor
#define dontdrop_width 24
#define dontdrop_height 24
#define dontdrop_x_hot 11
#define dontdrop_y_hot 12
static const FXchar dontdrop_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0xc0, 0xff, 0x01,
   0xf0, 0xff, 0x07, 0xf8, 0xff, 0x0f, 0xf8, 0xff, 0x0f, 0xfc, 0xff, 0x1f,
   0xfc, 0xff, 0x1f, 0xfe, 0xff, 0x3f, 0xfe, 0xff, 0x3f, 0x0e, 0x00, 0x38,
   0x0e, 0x00, 0x38, 0x0e, 0x00, 0x38, 0xfe, 0xff, 0x3f, 0xfe, 0xff, 0x3f,
   0xfc, 0xff, 0x1f, 0xfc, 0xff, 0x1f, 0xf8, 0xff, 0x0f, 0xf8, 0xff, 0x0f,
   0xf0, 0xff, 0x07, 0xc0, 0xff, 0x01, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00};
   
#define dontdrop_mask_width 24
#define dontdrop_mask_height 24
#define dontdrop_mask_x_hot 11
#define dontdrop_mask_y_hot 12
static const FXchar dontdrop_mask_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0xc0, 0xff, 0x01, 0xf0, 0xff, 0x07,
   0xf8, 0xff, 0x0f, 0xfc, 0xff, 0x1f, 0xfc, 0xff, 0x1f, 0xfe, 0xff, 0x3f,
   0xfe, 0xff, 0x3f, 0xff, 0xff, 0x7f, 0xff, 0xff, 0x7f, 0xff, 0xff, 0x7f,
   0xff, 0xff, 0x7f, 0xff, 0xff, 0x7f, 0xff, 0xff, 0x7f, 0xff, 0xff, 0x7f,
   0xfe, 0xff, 0x3f, 0xfe, 0xff, 0x3f, 0xfc, 0xff, 0x1f, 0xfc, 0xff, 0x1f,
   0xf8, 0xff, 0x0f, 0xf0, 0xff, 0x07, 0xc0, 0xff, 0x01, 0x00, 0x7f, 0x00};


// Upper or lower side MDI resize cursor
#define resizetop_width 24
#define resizetop_height 24
#define resizetop_x_hot 12
#define resizetop_y_hot 11
static const FXchar resizetop_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x28, 0x00,
   0x00, 0x44, 0x00, 0x00, 0x82, 0x00, 0x00, 0xc7, 0x01, 0x00, 0x44, 0x00,
   0x00, 0x44, 0x00, 0x00, 0x44, 0x00, 0x00, 0x44, 0x00, 0x00, 0x44, 0x00,
   0x00, 0x44, 0x00, 0x00, 0x44, 0x00, 0x00, 0x44, 0x00, 0x00, 0x44, 0x00,
   0x00, 0xc7, 0x01, 0x00, 0x82, 0x00, 0x00, 0x44, 0x00, 0x00, 0x28, 0x00,
   0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
   
#define resizetop_mask_width 24
#define resizetop_mask_height 24
#define resizetop_mask_x_hot 12
#define resizetop_mask_y_hot 11
static const FXchar resizetop_mask_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x38, 0x00, 0x00, 0x7c, 0x00,
   0x00, 0xfe, 0x00, 0x00, 0xff, 0x01, 0x80, 0xff, 0x03, 0x80, 0xff, 0x03,
   0x00, 0xfe, 0x00, 0x00, 0xfe, 0x00, 0x00, 0xfe, 0x00, 0x00, 0xfe, 0x00,
   0x00, 0xfe, 0x00, 0x00, 0xfe, 0x00, 0x00, 0xfe, 0x00, 0x80, 0xff, 0x03,
   0x80, 0xff, 0x03, 0x00, 0xff, 0x01, 0x00, 0xfe, 0x00, 0x00, 0x7c, 0x00,
   0x00, 0x38, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

   
// Right MDI resize cursor
#define resizetopright_width 24
#define resizetopright_height 24
#define resizetopright_x_hot 12
#define resizetopright_y_hot 12
static const FXchar resizetopright_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x40, 0x04, 0x00, 0x20, 0x04,
   0x00, 0x10, 0x04, 0x00, 0x08, 0x06, 0x00, 0x04, 0x05, 0x00, 0x82, 0x00,
   0x00, 0x41, 0x00, 0xa0, 0x20, 0x00, 0x60, 0x10, 0x00, 0x20, 0x08, 0x00,
   0x20, 0x04, 0x00, 0x20, 0x02, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
   
#define resizetopright_mask_width 24
#define resizetopright_mask_height 24
#define resizetopright_mask_x_hot 11
#define resizetopright_mask_y_hot 11
static const FXchar resizetopright_mask_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0xf0, 0x0f, 0x00, 0xf0, 0x0f, 0x00, 0xf0, 0x0f, 0x00, 0xf0, 0x0f,
   0x00, 0xf8, 0x0f, 0x00, 0xfc, 0x0f, 0x00, 0xfe, 0x0f, 0x00, 0xff, 0x0f,
   0xf0, 0xff, 0x00, 0xf0, 0x7f, 0x00, 0xf0, 0x3f, 0x00, 0xf0, 0x1f, 0x00,
   0xf0, 0x0f, 0x00, 0xf0, 0x0f, 0x00, 0xf0, 0x0f, 0x00, 0xf0, 0x0f, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// Left MDI resize cursor
#define resizetopleft_width 24
#define resizetopleft_height 24
#define resizetopleft_x_hot 11
#define resizetopleft_y_hot 12
static const FXchar resizetopleft_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x20, 0x02, 0x00, 0x20, 0x04, 0x00,
   0x20, 0x08, 0x00, 0x60, 0x10, 0x00, 0xa0, 0x20, 0x00, 0x00, 0x41, 0x00,
   0x00, 0x82, 0x00, 0x00, 0x04, 0x05, 0x00, 0x08, 0x06, 0x00, 0x10, 0x04,
   0x00, 0x20, 0x04, 0x00, 0x40, 0x04, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
   
#define resizetopleft_mask_width 24
#define resizetopleft_mask_height 24
#define resizetopleft_mask_x_hot 11
#define resizetopleft_mask_y_hot 12
static const FXchar resizetopleft_mask_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xf0, 0x0f, 0x00, 0xf0, 0x0f, 0x00, 0xf0, 0x0f, 0x00, 0xf0, 0x0f, 0x00,
   0xf0, 0x1f, 0x00, 0xf0, 0x3f, 0x00, 0xf0, 0x7f, 0x00, 0xf0, 0xff, 0x00,
   0x00, 0xff, 0x0f, 0x00, 0xfe, 0x0f, 0x00, 0xfc, 0x0f, 0x00, 0xf8, 0x0f,
   0x00, 0xf0, 0x0f, 0x00, 0xf0, 0x0f, 0x00, 0xf0, 0x0f, 0x00, 0xf0, 0x0f,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// Left or right side MDI resize cursor
#define resizeleft_width 24
#define resizeleft_height 24
#define resizeleft_x_hot 12
#define resizeleft_y_hot 12
static const FXchar resizeleft_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x80, 0x00, 0x02, 0xc0, 0x00, 0x06, 0xa0, 0xff, 0x0b, 0x10, 0x00, 0x10,
   0x08, 0x00, 0x20, 0x10, 0x00, 0x10, 0xa0, 0xff, 0x0b, 0xc0, 0x00, 0x06,
   0x80, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
   
#define resizeleft_mask_width 24
#define resizeleft_mask_height 24
#define resizeleft_mask_x_hot 12
#define resizeleft_mask_y_hot 12
static const FXchar resizeleft_mask_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x03,
   0xc0, 0x01, 0x07, 0xe0, 0xff, 0x0f, 0xf0, 0xff, 0x1f, 0xf8, 0xff, 0x3f,
   0xfc, 0xff, 0x7f, 0xf8, 0xff, 0x3f, 0xf0, 0xff, 0x1f, 0xe0, 0xff, 0x0f,
   0xc0, 0x01, 0x07, 0x80, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// Move cursor
#define move_width 24
#define move_height 24
#define move_x_hot 11
#define move_y_hot 11
static const FXchar move_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x1c, 0x00,
   0x00, 0x3e, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x1c, 0x00,
   0x20, 0x1c, 0x02, 0x30, 0x1c, 0x06, 0xf8, 0xff, 0x0f, 0xfc, 0xff, 0x1f,
   0xf8, 0xff, 0x0f, 0x30, 0x1c, 0x06, 0x20, 0x1c, 0x02, 0x00, 0x1c, 0x00,
   0x00, 0x1c, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x1c, 0x00,
   0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
   
#define move_mask_width 24
#define move_mask_height 24
#define move_mask_x_hot 11
#define move_mask_y_hot 11
static const FXchar move_mask_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x3e, 0x00,
   0x00, 0x7f, 0x00, 0x80, 0xff, 0x00, 0x80, 0xff, 0x00, 0x60, 0x3e, 0x03,
   0x70, 0x3e, 0x07, 0xf8, 0xff, 0x0f, 0xfc, 0xff, 0x1f, 0xfe, 0xff, 0x3f,
   0xfc, 0xff, 0x1f, 0xf8, 0xff, 0x0f, 0x70, 0x3e, 0x07, 0x60, 0x3e, 0x03,
   0x80, 0xff, 0x00, 0x80, 0xff, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x3e, 0x00,
   0x00, 0x1c, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


// Drag and drop COPY
#define dndcopy_width 24
#define dndcopy_height 24
#define dndcopy_x_hot 3
#define dndcopy_y_hot 2
static const FXchar dndcopy_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0x00, 0x18, 0x40, 0x01,
   0x38, 0x40, 0x02, 0x78, 0x40, 0x04, 0xf8, 0xc0, 0x07, 0xf8, 0x01, 0x04,
   0xf8, 0x03, 0x04, 0xf8, 0x07, 0x04, 0xf8, 0x0f, 0x04, 0xf8, 0x01, 0x04,
   0xb8, 0x01, 0x04, 0x18, 0x03, 0x04, 0x08, 0x03, 0x04, 0x08, 0x06, 0x04,
   0x08, 0x06, 0x04, 0x30, 0x00, 0x04, 0x30, 0x00, 0x04, 0xfc, 0x00, 0x04,
   0xfc, 0x00, 0x04, 0x30, 0xff, 0x07, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00};

#define dndcopy_mask_width 24
#define dndcopy_mask_height 24
#define dndcopy_mask_x_hot 3
#define dndcopy_mask_y_hot 2
static const FXchar dndcopy_mask_bits[] = {
   0x00, 0x00, 0x00, 0xfc, 0xff, 0x00, 0xfc, 0xff, 0x01, 0xfc, 0xff, 0x03,
   0xfc, 0xff, 0x07, 0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f,
   0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f,
   0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f,
   0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f, 0xfe, 0xff, 0x0f, 0xfe, 0xff, 0x0f,
   0xfe, 0xff, 0x0f, 0xfe, 0xff, 0x0f, 0xf8, 0xff, 0x0f, 0x78, 0x00, 0x00};
   
   
// Drag and drop LINK
#define dndlink_width 24
#define dndlink_height 24
#define dndlink_x_hot 3
#define dndlink_y_hot 2
static const FXchar dndlink_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0x00, 0x18, 0x40, 0x01,
   0x38, 0x40, 0x02, 0x78, 0x40, 0x04, 0xf8, 0xc0, 0x07, 0xf8, 0x01, 0x04,
   0xf8, 0x03, 0x04, 0xf8, 0x07, 0x04, 0xf8, 0x0f, 0x04, 0xf8, 0x01, 0x04,
   0xb8, 0x01, 0x04, 0x18, 0x03, 0x04, 0x08, 0x03, 0x04, 0x08, 0x06, 0x04,
   0xfc, 0x07, 0x04, 0x04, 0x01, 0x04, 0x64, 0x01, 0x04, 0x74, 0x01, 0x04,
   0x14, 0xff, 0x07, 0x04, 0x01, 0x00, 0xfc, 0x01, 0x00, 0x00, 0x00, 0x00};

#define dndlink_mask_width 24
#define dndlink_mask_height 24
#define dndlink_mask_x_hot 3
#define dndlink_mask_y_hot 2
static const FXchar dndlink_mask_bits[] = {
   0x00, 0x00, 0x00, 0xfc, 0xff, 0x00, 0xfc, 0xff, 0x01, 0xfc, 0xff, 0x03,
   0xfc, 0xff, 0x07, 0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f,
   0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f,
   0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f, 0xfe, 0xff, 0x0f,
   0xfe, 0xff, 0x0f, 0xfe, 0xff, 0x0f, 0xfe, 0xff, 0x0f, 0xfe, 0xff, 0x0f,
   0xfe, 0xff, 0x0f, 0xfe, 0xff, 0x0f, 0xfe, 0x03, 0x00, 0xfe, 0x03, 0x00};
   
   
// Drag and drop MOVE
#define dndmove_width 24
#define dndmove_height 24
#define dndmove_x_hot 3
#define dndmove_y_hot 2
static const FXchar dndmove_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0x00, 0x18, 0x40, 0x01,
   0x38, 0x40, 0x02, 0x78, 0x40, 0x04, 0xf8, 0xc0, 0x07, 0xf8, 0x01, 0x04,
   0xf8, 0x03, 0x04, 0xf8, 0x07, 0x04, 0xf8, 0x0f, 0x04, 0xf8, 0x01, 0x04,
   0xb8, 0x01, 0x04, 0x18, 0x03, 0x04, 0x08, 0x03, 0x04, 0x08, 0x06, 0x04,
   0x08, 0x06, 0x04, 0x08, 0x00, 0x04, 0x08, 0x00, 0x04, 0x08, 0x00, 0x04,
   0x08, 0x00, 0x04, 0xf8, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
   
#define dndmove_mask_width 24
#define dndmove_mask_height 24
#define dndmove_mask_x_hot 3
#define dndmove_mask_y_hot 2
static const FXchar dndmove_mask_bits[] = {
   0x00, 0x00, 0x00, 0xfc, 0xff, 0x00, 0xfc, 0xff, 0x01, 0xfc, 0xff, 0x03,
   0xfc, 0xff, 0x07, 0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f,
   0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f,
   0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f,
   0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f,
   0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f, 0xfc, 0xff, 0x0f, 0x00, 0x00, 0x00};
   
 
/*******************************************************************************/

   
// Default palette
static const FXuchar default_palette[][3]={
  {0x00,0x00,0x00},
  {0xff,0xff,0xff},
  {0xdd,0xdd,0xdd},
  {0xbb,0xbb,0xbb},
  {0x99,0x99,0x99},
  {0x77,0x77,0x77},
  {0x55,0x55,0x55},
  {0x33,0x33,0x33},
  {0x88,0x00,0x00},
  {0xcc,0x00,0x00},
  {0xff,0x00,0x00},
  {0xff,0x44,0x00},
  {0xff,0x88,0x00},
  {0xff,0xcc,0x00},
  {0xff,0xff,0x00},
  {0xcc,0xcc,0x00},
  {0x88,0x88,0x00},
  {0x44,0x44,0x00},
  {0x00,0x44,0x00},
  {0x00,0x88,0x00},
  {0x00,0xcc,0x00},
  {0x00,0xff,0x00},
  {0x00,0x44,0x44},
  {0x00,0x88,0x88},
  {0x00,0xff,0xff},
  {0x00,0x00,0x44},
  {0x00,0x00,0x88},
  {0x00,0x00,0xff},
  {0x88,0x00,0x88},
  {0xff,0xcc,0x99},
  {0xcc,0xaa,0x77},
  {0xaa,0x88,0x55},
  {0x88,0x66,0x33},
  {0x66,0x44,0x11},
  {0x44,0x22,0x00},
  {0x22,0x00,0x00},
  {0x00,0x44,0x88},
  {0x44,0x88,0xcc},
  {0x88,0xcc,0xff},
  {0x00,0xcc,0x44},
  {0x44,0x88,0x44},
  {0x88,0xcc,0x00},
  {0x22,0x44,0x11},
  {0x33,0x66,0x22},
  {0x44,0x55,0x33},
  {0x66,0x88,0x44},
  {0x33,0x66,0x22},
  {0x22,0x44,0x11},
  };

/*******************************************************************************/

// Map
FXDEFMAP(FXApp) FXAppMap[]={
  FXMAPFUNC(SEL_COMMAND,FXApp::ID_QUIT,FXApp::onQuit),
  };


// Implementation
FXIMPLEMENT(FXApp,FXObject,FXAppMap,ARRAYNUMBER(FXAppMap))


// Application object
FXApp* FXApp::app=NULL;


// Initialize application object
FXApp::FXApp(){
  
  // Test if application object already exists
  if(app){ fxwarning("Warning: Trying to construct multiple application objects.\n"); }
  
  // Initialize private platform independent data
  appname="App";
  focusWindow=NULL;                   // Window which has focus
  cursorWindow=NULL;                  // Window under the cursor
  grabWindow=NULL;                    // Window that is grabbed
  refresher=NULL;                     // Window being refreshed
  root=new FXRootWindow(this);
  timers=NULL;
  chores=NULL;
  repaints=NULL;
  timerrecs=NULL;
  chorerecs=NULL;
  repaintrecs=NULL;
  invocation=NULL;                    // Modal loop invocation
  done=0;                             // True if application is done
  exitcode=0;
  
  // Clear event structure
  event.type=0;
  event.window=0;
  event.time=0;
  event.win_x=0;
  event.win_y=0;
  event.root_x=0;
  event.root_y=0;
  event.state=0;
  event.code=0;
  event.last_x=0;
  event.last_y=0;
  event.click_x=0;
  event.click_y=0;
  event.click_time=0;
  event.click_window=0;
  event.click_button=0;
  event.click_count=0;
  event.moved=0;
  event.rect.x=0;
  event.rect.y=0;
  event.rect.w=0;
  event.rect.h=0;
  event.synthetic=0;
  
  // X Window specific inits
  display=NULL;
  connection=-1;
  wcontext=0;
  visual=NULL;
  depth=0;
  colormap=0;
  wmDeleteWindow=0;
  wmQuitApp=0;
  wmProtocols=0;
  wmMotifHints=0;
  wmTakeFocus=0;
  
  // XDND
  xdndAware=0;                        // XDND awareness atom    
  xdndEnter=0;                        // XDND Message types
  xdndLeave=0;       
  xdndPosition=0;    
  xdndStatus=0;      
  xdndDrop=0;   
  xdndFinished=0;     
  xdndSelection=0;
  xdndActionMove=0;                   // XDND Copy action
  xdndActionCopy=0;                   // XDND Move action
  xdndActionLink=0;                   // XDND Link action
  xdndTypes=0;
  xdndSource=0;                       // XDND drag source window
  xdndTarget=0;                       // XDND drop target window
  xdndAction=0;                       // XDND action being performed
  xdndAccepts=FALSE;                  // XDND true if target accepts
  xdndSendPosition=FALSE;             // XDND send position update when status comes in
  xdndStatusPending=FALSE;            // XDND waiting for status feedback
  xdndFinishPending=FALSE;            // XDND waiting for drop-confirmation
  xdndStatusReceived=FALSE;           // XDND received at least one status
  xdndWantUpdates=TRUE;               // XDND target always wants new positions
  xdndVersion=XDND_PROTOCOL_VERSION;  // Version for this transaction
  xdndTypeList=NULL;                  // XDND types list
  xdndNumTypes=0;
  xdndRect.x=0;                       // XDND motion rectangle
  xdndRect.y=0;
  xdndRect.w=0;
  xdndRect.h=0;

  // DDE
  ddeSelection=0;                     // Data exchange selection
  ddeRequestor=0;                     // Data exchange reqestor
  ddeProperty=0;                      // Data exchange on requestor's window to receive data
  ddeAtom=0;                          // Data exchange atom
  ddeTarget=0;                        // Data exchange requested type
  ddeData=NULL;                       // Data exchange array
  ddeSize=0;                          // Data exchange array size
  
  // Miscellaneous stuff
  graybitmap=0;
  gc=0;
  shmi=TRUE;
  shmp=TRUE;
  synchronize=FALSE;
  palette=NULL;
  ncolors=0;
  
  // Initialize public data
  
  // Make font
  normalFont=new FXFont(this,"-adobe-helvetica-bold-r-*-*-12-*-*-*-*-*-iso8859-1");
  
  // Make some cursors
  arrowCursor=new FXCursor(this);
  rarrowCursor=new FXCursor(this);
  textCursor=new FXCursor(this);
  hsplitCursor=new FXCursor(this);
  vsplitCursor=new FXCursor(this);
  resizeCursor=new FXCursor(this);
  swatchCursor=new FXCursor(this);
  dontdropCursor=new FXCursor(this);
  moveCursor=new FXCursor(this);
  dragHCursor=new FXCursor(this);
  dragVCursor=new FXCursor(this);
  dragTLCursor=new FXCursor(this);
  dragTRCursor=new FXCursor(this);
  dndCopyCursor=new FXCursor(this);
  dndMoveCursor=new FXCursor(this);
  dndLinkCursor=new FXCursor(this);
  
  // Other settings
  clickSpeed=400;
  scrollSpeed=100;
  blinkSpeed=500;
  menuPause=400;
  tooltipPause=800;
  tooltipTime=3000;
  scrollbarWidth=15;
  dragDelta=6;
  
  // Init colors
  backColor=FXRGB(192,192,192);   
  foreColor=FXRGB(0,0,0);   
  hiliteColor=makeHiliteColor(backColor); 
  shadowColor=makeShadowColor(backColor); 
  borderColor=FXRGB(0,0,0);
  selforeColor=FXRGB(255,255,255);
  selbackColor=FXRGB(0,0,128);
  
  // Pointer to FXApp
  app=this;
  }


/*******************************************************************************/

// Create palette
void FXApp::makePalette(){
  XColor table[256],color;
  FXuint i,gottable,alloced;

  // Test just to make sure
  if(visual->map_entries>256){ fxerror("%s::makeColor: bigger colormap than expected.\n",getClassName()); }

//fprintf(stderr,"visual->map_entries: %d\n",visual->map_entries);

  // Make palette
  ncolors=ARRAYNUMBER(default_palette);
  palette=new FXPalEntry[ncolors];
  
  gottable=0;
  
  // Allocate colors
  for(i=0; i<ncolors; i++){
    
    color.red=default_palette[i][0]*257;
    color.green=default_palette[i][1]*257;
    color.blue=default_palette[i][2]*257;
    color.flags=DoRed|DoGreen|DoBlue;

    // First try just using XAllocColor
    alloced=XAllocColor(display,colormap,&color);
    if(alloced==0){
      FXuint dist,mindist,bestmatch;
      FXint j,dr,dg,db;


      // Get colors in the map
      if(!gottable){
        for(j=0; j<visual->map_entries; j++) table[j].pixel=j;
        XQueryColors(display,colormap,table,visual->map_entries);
        gottable=1;
        }

      // Find best (weighted) match; also gives good colors for grey-ramps
      for(mindist=4294967295U,bestmatch=0,j=0; j<visual->map_entries; j++){
        dr=19660*(color.red-table[j].red);
        dg=38666*(color.green-table[j].green);
        db=7209*(color.blue-table[j].blue);
        dist=(FXuint)FXABS(dr) + (FXuint)FXABS(dg) + (FXuint)FXABS(db);
        if(dist<mindist){
          bestmatch=j;
          mindist=dist;
          }
        }

      // Return result
      color.red=table[bestmatch].red;
      color.green=table[bestmatch].green;
      color.blue=table[bestmatch].blue;

      // Try to allocate the closest match color.  This should only
      // fail if the cell is read/write.  Otherwise, we're incrementing
      // the cell's reference count.
      alloced=XAllocColor(display,colormap,&color);
      if(!alloced){
        color.pixel=bestmatch;
        color.red=table[bestmatch].red;
        color.green=table[bestmatch].green;
        color.blue=table[bestmatch].blue;
        color.flags=DoRed|DoGreen|DoBlue;
        }
      }
    
    // Fill in our own data structure
    palette[i].pixel=color.pixel;
    palette[i].red=color.red/257;
    palette[i].green=color.green/257;
    palette[i].blue=color.blue/257;
    palette[i].alloced=alloced;
    }
  }


// Get palette
void FXApp::getPalette(){
  XColor table[256];
  FXuint i;

  // Test just to make sure
  if(visual->map_entries>256){ fxerror("%s::getPalette: bigger colormap than expected.\n",getClassName()); }

//fprintf(stderr,"visual->map_entries: %d\n",visual->map_entries);

  // Make palette
  ncolors=visual->map_entries;
  palette=new FXPalEntry[ncolors];
  
  for(i=0; i<ncolors; i++) table[i].pixel=i;
  XQueryColors(display,colormap,table,visual->map_entries);
  
  // Fill in our own data structure
  for(i=0; i<ncolors; i++){
    palette[i].pixel=table[i].pixel;
    palette[i].red=table[i].red/257;
    palette[i].green=table[i].green/257;
    palette[i].blue=table[i].blue/257;
    palette[i].alloced=0;
    }
  }


// Free palette
void FXApp::freePalette(){
  FXPixel pixels[256];
  FXuint i,n;
  for(i=n=0; i<ncolors; i++){if(palette[i].alloced) pixels[n++]=palette[i].pixel;}
  if(n!=0){XFreeColors(display,colormap,pixels,n,0);}
  delete palette;
  ncolors=0;
  }


// Get a color
FXPixel FXApp::matchColor(FXuint r,FXuint g,FXuint b){
  register FXuint bestmatch,j,dist,mindist;
  register FXint dr,dg,db;
  for(mindist=4294967295U,bestmatch=0,j=0; j<ncolors; j++){
    dr=(r-palette[j].red);
    dg=(g-palette[j].green);
    db=(b-palette[j].blue);
    dist=(FXuint)FXABS(dr) + (FXuint)FXABS(dg) + (FXuint)FXABS(db);
    if(dist<mindist){
      bestmatch=j;
      mindist=dist;
      }
    }
  FXASSERT(bestmatch<ncolors);
  return palette[bestmatch].pixel;
  }


/*******************************************************************************/


// Parhaps should do something else...
static int xerrorhandler(Display* dpy,XErrorEvent* eev){
  char buf[1000];
  XGetErrorText(dpy,eev->error_code,buf,sizeof(buf));
  fxwarning("X Error: code %d major %d minor %d: %s.\n",eev->error_code,eev->request_code,eev->minor_code,buf);
  return 1;
  }


// Fatal error (e.g. lost connection)
static int xfatalerrorhandler(Display*){
  fxerror("X Fatal error.\n");
  return 1;
  }


/*******************************************************************************/


// Open the display
int FXApp::openDisplay(const char* dpyname){
  const unsigned char gray_bits[]={0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa};
  const unsigned char black_bits[]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
  unsigned long gmask;
  const char* dpy;
  XGCValues gval;
 
  
  // Set error handler
  XSetErrorHandler(xerrorhandler);
  
  // Set fatal handler; it should save the app data & exit the program
  XSetIOErrorHandler(xfatalerrorhandler);
  
  // Try locate display
  dpy=dpyname;
  if(!dpy){
    dpy=getenv("DISPLAY");
    if(!dpy) dpy=":0.0";
    }
    
  // Open display
  display=XOpenDisplay(dpy);
  if(!display){ fxerror("%s: unable to open display %s\n",appname,dpy); }

  // For debugging
  if(synchronize) XSynchronize(display,TRUE);

  // Get X connection number
  connection=ConnectionNumber(display);

  // Make hash context for window mapping
  wcontext=XUniqueContext();
  
  // Get default colormap
  colormap=XDefaultColormap(display,XDefaultScreen(display));

  // Get default visual
  visual=XDefaultVisual(display,XDefaultScreen(display));
  
  // If PseudoColor, claim colormap
  if(visual->c_class==PseudoColor || visual->c_class==GrayScale){ 
    makePalette();
    }
  
  // If StaticColor, just grab colormap
  else if(visual->c_class==StaticColor || visual->c_class==StaticGray){
    getPalette();
    }
  
  // See if we wanted/have XSHM
#ifdef HAVE_XSHM
   if(shmi || shmp){
     int maj,min,dum; Bool pm;
     shmi=FALSE;
     shmp=FALSE;
     if(XQueryExtension(display,"MIT-SHM",&dum,&dum,&dum)){
       if(XShmQueryVersion(display,&maj,&min,&pm)){
         shmp=pm && (XShmPixmapFormat(display)==ZPixmap);
         shmi=TRUE;
         }
       }
     }
   
   // Don't have it!
#else
  shmi=FALSE;
  shmp=FALSE;
#endif

  // Window manager communication
  wmDeleteWindow=XInternAtom(display,"WM_DELETE_WINDOW",0);
  wmQuitApp=XInternAtom(display,"_WM_QUIT_APP",0);
  wmProtocols=XInternAtom(display,"WM_PROTOCOLS",0);
  wmMotifHints=XInternAtom(display,"_MOTIF_WM_HINTS",0);
  wmTakeFocus=XInternAtom(display,"WM_TAKE_FOCUS",0);

  // XDND protocol awareness
  xdndAware=XInternAtom(display,"XdndAware",0);
  
  // XDND Messages
  xdndEnter=XInternAtom(display,"XdndEnter",0);
  xdndLeave=XInternAtom(display,"XdndLeave",0);
  xdndPosition=XInternAtom(display,"XdndPosition",0);
  xdndStatus=XInternAtom(display,"XdndStatus",0);
  xdndDrop=XInternAtom(display,"XdndDrop",0);
  xdndFinished=XInternAtom(display,"XdndFinished",0);
  
  // XDND Selection atom
  xdndSelection=XInternAtom(display,"XdndSelection",0);

  // XDND Actions
  xdndActionCopy=XInternAtom(display,"XdndActionCopy",0);
  xdndActionMove=XInternAtom(display,"XdndActionMove",0);
  xdndActionLink=XInternAtom(display,"XdndActionLink",0);
  
  // XDND Types list
  xdndTypes=XInternAtom(display,"XdndTypeList",0);
  
  // DDE property
  ddeAtom=XInternAtom(display,"_FOX_DDE",0);
  
  // Init DDE stuff
  ddeSelection=XA_PRIMARY;
  ddeRequestor=0;
  ddeProperty=0;
  ddeTarget=0;
  ddeData=NULL;
  ddeSize=0;
  
  // Make gray bitmap
  graybitmap=XCreatePixmapFromBitmapData(
               display,XDefaultRootWindow(display),
               (char*)gray_bits,8,8,
               //backColor,whiteColor,
               BlackPixel(display,DefaultScreen(display)),WhitePixel(display,DefaultScreen(display)),
               XDefaultDepth(display,XDefaultScreen(display)));
  
  // Gray stipple
  graystipple=XCreateBitmapFromData(display,XDefaultRootWindow(display),(char*)gray_bits,8,8);
  
  // Black stipple
  blackstipple=XCreateBitmapFromData(display,XDefaultRootWindow(display),(char*)black_bits,8,8);

  // Create cursors
  arrowCursor->createFromFont(XC_top_left_arrow);
  rarrowCursor->createFromFont(XC_arrow);
  textCursor->createFromFont(XC_xterm);
  //hsplitCursor->createFromFont(XC_sb_h_double_arrow);
  //vsplitCursor->createFromFont(XC_sb_v_double_arrow);
  hsplitCursor->createFromBitmapData(hsplit_bits,hsplit_mask_bits,hsplit_width,hsplit_height,hsplit_x_hot,hsplit_y_hot);
  vsplitCursor->createFromBitmapData(vsplit_bits,vsplit_mask_bits,vsplit_width,vsplit_height,vsplit_x_hot,vsplit_y_hot);
  //resizeCursor->createFromFont(XC_bottom_right_corner);
  resizeCursor->createFromBitmapData(resize_bits,resize_mask_bits,resize_width,resize_height,resize_x_hot,resize_y_hot);
  swatchCursor->createFromBitmapData(swatch_bits,swatch_mask_bits,swatch_width,swatch_height,swatch_x_hot,swatch_y_hot);
  dontdropCursor->createFromBitmapData(dontdrop_bits,dontdrop_mask_bits,dontdrop_width,dontdrop_height,dontdrop_x_hot,dontdrop_y_hot);
  
  moveCursor->createFromBitmapData(move_bits,move_mask_bits,move_width,move_height,move_x_hot,move_y_hot);
  
  dragHCursor->createFromBitmapData(resizetop_bits,resizetop_mask_bits,resizetop_width,resizetop_height,resizetop_x_hot,resizetop_y_hot);
  dragVCursor->createFromBitmapData(resizeleft_bits,resizeleft_mask_bits,resizeleft_width,resizeleft_height,resizeleft_x_hot,resizeleft_y_hot);
  dragTRCursor->createFromBitmapData(resizetopright_bits,resizetopright_mask_bits,resizetopright_width,resizetopright_height,resizetopright_x_hot,resizetopright_y_hot);
  dragTLCursor->createFromBitmapData(resizetopleft_bits,resizetopleft_mask_bits,resizetopleft_width,resizetopleft_height,resizetopleft_x_hot,resizetopleft_y_hot);
  
  dndCopyCursor->createFromBitmapData(dndcopy_bits,dndcopy_mask_bits,dndcopy_width,dndcopy_height,dndcopy_x_hot,dndcopy_y_hot);
  dndMoveCursor->createFromBitmapData(dndmove_bits,dndmove_mask_bits,dndmove_width,dndmove_height,dndmove_x_hot,dndmove_y_hot);
  dndLinkCursor->createFromBitmapData(dndlink_bits,dndlink_mask_bits,dndlink_width,dndlink_height,dndlink_x_hot,dndlink_y_hot);

  // Create font
  normalFont->create();
  
  // Create GC
  gmask=GCFillStyle|GCGraphicsExposures;
  gval.fill_style=FillSolid;
  gval.graphics_exposures=False;
  gc=XCreateGC(display,XDefaultRootWindow(display),gmask,&gval);

  return 1;
  }
  
  
// Close display
void FXApp::closeDisplay(){
  if(!display) return;

  XFreeGC(display,gc);

  XCloseDisplay(display);
  }

/*******************************************************************************/

// Compare times
static inline int operator<(const struct timeval& a,const struct timeval& b){
  return (a.tv_sec<b.tv_sec) || (a.tv_sec==b.tv_sec && a.tv_usec<b.tv_usec);
  }

  
// Add timeout, sorted by time
FXTimer* FXApp::addTimeout(FXint ms,FXObject* tgt,FXSelector sel){
  FXTimer *t,*h,**hh;
  if(ms<1){ fxerror("%s::addTimeout: wait time should be greater than 0\n",getClassName()); }
  if(timerrecs){
    t=timerrecs;
    timerrecs=t->next;
    }
  else{
    t=new FXTimer;
    }
  t->target=tgt;
  t->message=sel;
  gettimeofday(&t->due,NULL);
  t->due.tv_sec+=ms/1000;
  t->due.tv_usec+=(ms%1000)*1000;
  if(t->due.tv_usec>=1000000){
    t->due.tv_usec-=1000000;
    t->due.tv_sec+=1;
    }
  hh=&timers;
  h=*hh;
  while(h && (h->due < t->due)){
    hh=&h->next;
    h=*hh;
    }
  t->next=h;
  *hh=t;
//fprintf(stderr,"addTimeout %08x\n",t);
  return t;
  }


// Remove timeout from the list
FXTimer* FXApp::removeTimeout(FXTimer *t){
//fprintf(stderr,"removeTimeout %08x\n",t);
  if(t){
    FXTimer *h,**hh;
    for(h=timers,hh=&timers; h!=t; hh=&h->next,h=h->next){
      if(h==NULL) return NULL;
      }
    FXASSERT(h==t);
    *hh=t->next;
    t->next=timerrecs;
    timerrecs=t;
    }
  return NULL;
  }


// Add chore to list
FXChore* FXApp::addChore(FXObject* tgt,FXSelector sel){
  FXChore *c;
  if(chorerecs){
    c=chorerecs;
    chorerecs=c->next;
    }
  else{
    c=new FXChore;
    }
  c->target=tgt;
  c->message=sel;
  c->next=chores;
  chores=c;
  return c;
  }


// Remove chore from the list
FXChore* FXApp::removeChore(FXChore *c){
  if(c){
    FXChore *h,**hh;
    for(h=chores,hh=&chores; h!=c; hh=&h->next,h=h->next){
      if(h==NULL) return NULL;
      }
    FXASSERT(h==c);
    *hh=c->next;
    c->next=chorerecs;
    chorerecs=c;
    }
  return NULL;
  }


// // Add a repaint or scroll rectangle
// void FXApp::addRepaint(FXID win,FXint x,FXint y,FXint w,FXint h,FXbool synth){
//   register FXint px,py,pw,ph,hint,area;
//   register FXRepaint *r,**pr;
//   hint=w*h;
//   w+=x;
//   h+=y;
//   do{
//     
//     // Find overlap with outstanding rectangles
//     for(r=repaints,pr=&repaints; r; pr=&r->next,r=r->next){
//       if(r->window==win){
// 
//         // Tentatively conglomerate rectangles
//         px=FXMIN(x,r->rect.x);
//         py=FXMIN(y,r->rect.y);
//         pw=FXMAX(w,r->rect.w);
//         ph=FXMAX(h,r->rect.h);
//         area=(pw-px)*(ph-py);
// 
//         // New area much bigger than sum; forget about it
//         if(area>2*(hint+r->hint)) continue;
// 
//         // Take old paintrect out of the list
//         *pr=r->next; 
//         r->next=repaintrecs; 
//         repaintrecs=r;
//         
//         // New rectangle
//         synth|=r->synth;        // Synthethic is preserved!
//         hint=area; 
//         x=px; 
//         y=py; 
//         w=pw; 
//         h=ph; 
//         break;
//         }
//       }
//     }while(r);
//     
//   // Nope, get new rectangle
//   if(repaintrecs){
//     r=repaintrecs;
//     repaintrecs=r->next;
//     }
//   else{
//     r=new FXRepaint;
//     }
//   
//   // And add at the end
//   r->window=win;
//   r->rect.x=x;
//   r->rect.y=y;
//   r->rect.w=w;
//   r->rect.h=h;
//   r->synth=synth;
//   r->hint=hint;
//   r->next=NULL;
//   *pr=r;
//   }
//     
    
// Add a repaint or scroll rectangle
void FXApp::addRepaint(FXID win,FXint x,FXint y,FXint w,FXint h,FXbool synth){
  register FXRepaint *r,**pr;
  w+=x;
  h+=y;
  for(r=repaints,pr=&repaints; r; pr=&r->next,r=r->next){
    if(r->window==win){
      
      // Composite rectangles
      r->rect.x=FXMIN(x,r->rect.x);
      r->rect.y=FXMIN(y,r->rect.y);
      r->rect.w=FXMAX(w,r->rect.w);
      r->rect.h=FXMAX(h,r->rect.h);
      
      // Will become a synthetic expose event!
      r->synth|=synth;

      return;
      }
    }
    
  // Nope, get new rectangle
  if(repaintrecs){
    r=repaintrecs;
    repaintrecs=r->next;
    }
  else{
    r=new FXRepaint;
    }
  
  // And add at the end
  r->window=win;
  r->rect.x=x;
  r->rect.y=y;
  r->rect.w=w;
  r->rect.h=h;
  r->synth=synth;
  r->next=NULL;
  *pr=r;
  }
    


/*******************************************************************************/


// Get an event
void FXApp::getNextEvent(XEvent& ev){
  struct timeval now,delta;
  fd_set readfds;
  fd_set writefds;
  fd_set exceptfds;
  int maxfds;
  int nfds;
  
  while(1){
    while(1){

      // Handle all currently outstanding timers.
      // This can NOT loop forever, as newly scheduled timers
      // will be scheduled LATER than the current time!
      gettimeofday(&now,NULL);
      while(timers){
        FXTimer* t=timers;
        if(now < t->due) break;
        timers=t->next;
        
        // Call the timeout handler
        if(t->target && t->target->handle(this,MKUINT(t->message,SEL_TIMEOUT),&event)) refresh();
        
        // Add record to list of recycled timer records
        t->next=timerrecs;
        timerrecs=t;
        }

      // Any events already received?
//      if(XEventsQueued(display,QueuedAlready)) break;
      if(XEventsQueued(display,QueuedAfterFlush)) break;
#ifdef HUMMINGBIRD
      if(XEventsQueued(display,QueuedAfterReading)) break;
#endif

      // Prepare fd's to watch
#ifndef HUMMINGBIRD
      FD_ZERO(&readfds);
      FD_ZERO(&writefds);
      FD_ZERO(&exceptfds);
      FD_SET(connection,&readfds);

      // Add other inputs...
      maxfds=connection;

      delta.tv_usec=0;
      delta.tv_sec=0;

      // Do a quick poll for any ready events
      nfds=SELECT(maxfds+1,&readfds,&writefds,&exceptfds,&delta);
#else
      nfds=0;
#endif

      // No events, do idle processing
      if(nfds==0){

        // Release the expose events...
        if(repaints){
          FXRepaint *r=repaints;
          ev.xany.type=Expose;
          ev.xexpose.window=r->window;
          ev.xexpose.send_event=r->synth;
          ev.xexpose.x=r->rect.x;
          ev.xexpose.y=r->rect.y;
          ev.xexpose.width=r->rect.w-r->rect.x;
          ev.xexpose.height=r->rect.h-r->rect.y;
          repaints=r->next;
          r->next=repaintrecs;
          repaintrecs=r;
          return;
          }
        
        // GUI updating:- walk the whole widget tree.
        if(refresher){
//fprintf(stderr,"GUI Updating %s (%08x)\n",refresher->getClassName(),refresher);
          
          // Call the refresher handler
          refresher->handle(this,MKUINT(0,SEL_UPDATE),&event);
          
          // Next widget to refresh
          if(refresher->getFirst()){
            refresher=refresher->getFirst();
            continue;
            }
          while(!refresher->getNext() && refresher->getParent()!=root){
            refresher=refresher->getParent();
            }
          refresher=refresher->getNext();
          continue;
          }
        
        // Do our chores :-)
        if(chores){
          FXChore* c=chores;
          chores=c->next;
          
          // Call the chore handler
          if(c->target && c->target->handle(this,MKUINT(c->message,SEL_CHORE),&event)) refresh();
          
          // Add record to list of recycled chore records
          c->next=chorerecs;
          chorerecs=c;
          continue;
          }

        // Now, block till timeout, i/o, or event
#ifndef HUMMINGBIRD
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_ZERO(&exceptfds);
        FD_SET(connection,&readfds);

        // Add other inputs...
        maxfds=connection;

        // We're about to block now; if there are
        // timers, we block only for a little while.
        if(timers){

          // All that testing above may have taken some time...
          gettimeofday(&now,NULL);

          // Compute how long to wait
          delta.tv_usec=timers->due.tv_usec-now.tv_usec;
          delta.tv_sec=timers->due.tv_sec-now.tv_sec;
          while(delta.tv_usec<0){
            delta.tv_usec+=1000000;
            delta.tv_sec-=1;
            }
          FXASSERT(0<=delta.tv_usec && delta.tv_usec<1000000);

          // Some timers are already due; do them right away!
          if(delta.tv_sec<0 || (delta.tv_sec==0 && delta.tv_usec==0)) continue;

//fprintf(stderr,"blocking select waiting for %dsec %dusec...",delta.tv_sec,delta.tv_usec);

          // Block till timer or event or interrupt
          nfds=SELECT(maxfds+1,&readfds,&writefds,&exceptfds,&delta);

          // Timed out, so handle those timeouts
          if(nfds==0) continue;
          }

        // If no timers, we block till event or interrupt
        else{
          nfds=SELECT(maxfds+1,&readfds,&writefds,&exceptfds,NULL);
          }
#endif
        }

      // Interrupt
#ifndef HUMMINGBIRD
      if(nfds<0){
        if(errno==EAGAIN || errno==EINTR){ errno=0; continue; }
        fxerror("Application terminated: interrupt or lost connection errno=%d\n",errno);
        }

      // Got event
      if(FD_ISSET(connection,&readfds)){
        if(XEventsQueued(display,QueuedAfterReading)) break;
        }

      // Other i/o
      else{
        // Shouldn't get here for now...
        fprintf(stderr,"Other active FDs: ");
        for(int fff=0; fff<maxfds; fff++){
          if(FD_ISSET(fff,&readfds)) fprintf(stderr,"R%d ",fff);
          if(FD_ISSET(fff,&writefds)) fprintf(stderr,"W%d ",fff);
          if(FD_ISSET(fff,&exceptfds)) fprintf(stderr,"E%d ",fff);
          }
        fprintf(stderr,"\n");
        refresh();                // I/O also can cause application state changes
        }
#else
      break;
#endif
      }
    
    // Get an event
    XNextEvent(display,&ev);

    // Compress motion events
    if(ev.xany.type==MotionNotify){
      while(XPending(display)){
        XEvent e;
        XPeekEvent(display,&e);
        if((e.xany.type!=MotionNotify) || (ev.xmotion.window!=e.xmotion.window) || (ev.xmotion.subwindow!=e.xmotion.subwindow)) break;
        XNextEvent(display,&ev);
        }
      }

    // Save expose events for later...
#ifndef HUMMINGBIRD
    if(ev.xany.type==Expose || ev.xany.type==GraphicsExpose){
      addRepaint(ev.xexpose.window,ev.xexpose.x,ev.xexpose.y,ev.xexpose.width,ev.xexpose.height,0);
      continue;
      }
#endif
    
    // Regular event
    return;
    }
  }


// Peek for event
FXbool FXApp::peekEvent(){
  struct timeval delta;
  fd_set readfds;
  fd_set writefds;
  fd_set exceptfds;
  int maxfds;
  int nfds;
  
  // Events queued up in client already
  if(XEventsQueued(display,QueuedAfterFlush)) return TRUE;

  // Prepare fd's to watch
  FD_ZERO(&readfds);
  FD_ZERO(&writefds);
  FD_ZERO(&exceptfds);
  FD_SET(connection,&readfds);

  // Add other inputs...
  maxfds=connection;

  delta.tv_usec=0;
  delta.tv_sec=0;

  // Do a quick poll for any ready events
  nfds=SELECT(maxfds+1,&readfds,&writefds,&exceptfds,&delta);
  
  // Interrupt
  if(nfds<0){
    if(errno==EAGAIN || errno==EINTR){ errno=0; return FALSE; }
    fxerror("Application terminated: interrupt or lost connection errno=%d\n",errno);
    }
  
  // I/O activity
  if(nfds>0){
    if(FD_ISSET(connection,&readfds)){
      if(XEventsQueued(display,QueuedAfterReading)) return TRUE;
      }
    }
  
  // No events
  return FALSE;
  }


// Dispatch event to widget
void FXApp::dispatchEvent(XEvent& ev){
  FXWindow *window,*shell;
  FXuint    ks1,ks2;
  Window    tmp;
  XEvent    se;
  if(XFindContext(display,ev.xany.window,wcontext,(XPointer*)&window)==0){
    FXASSERT(window);
    shell=window->getShell();
    FXASSERT(shell);
    
    // Translate it
    switch(ev.xany.type){

      // Composite expose events into rectangle
      case Expose:
      case GraphicsExpose:
        event.type=SEL_PAINT;
        event.window=ev.xexpose.window;
        event.rect.x=ev.xexpose.x;
        event.rect.y=ev.xexpose.y;
        event.rect.w=ev.xexpose.width;
        event.rect.h=ev.xexpose.height;
        event.synthetic=ev.xexpose.send_event;
        //window->setClipRectangle(event.x,event.y,event.w,event.h);
        window->handle(this,MKUINT(0,SEL_PAINT),&event);
        //window->clearClipRectangle();
        break;

      // Keyboard
      case KeyPress:
      case KeyRelease:
        event.type=SEL_KEYPRESS+ev.xkey.type-KeyPress;
        event.window=ev.xkey.window;
        event.time=ev.xkey.time;
        event.win_x=ev.xkey.x;
        event.win_y=ev.xkey.y;
        event.root_x=ev.xkey.x_root;
        event.root_y=ev.xkey.y_root;
        event.state=ev.xkey.state;
        ks1=XLookupKeysym(&ev.xkey,0);
        ks2=XLookupKeysym(&ev.xkey,1);
        if((event.state&NUMLOCKMASK) && (KEY_KP_Space<=ks2) && (ks2<=KEY_KP_Equal)){  // This is still doubtfull!
          if(event.state&SHIFTMASK) event.code=ks1; else event.code=ks2;
          }
        else if((KEY_a<=ks1) && (ks1<=KEY_z)){            // Alpha shifted and/or caplocked
          if(((event.state&CAPSLOCKMASK)!=0) ^ ((event.state&SHIFTMASK)!=0)) event.code=ks2; else event.code=ks1;
          }
        else if((event.state&SHIFTMASK) && (ks2!=0)){     // Simple shifted
          event.code=ks2;
          }
        else{                                             // Unshifted
          event.code=ks1;
          }
// {char buf[20]; KeySym sym; static XComposeStatus cs;
// XLookupString(&ev.xkey,buf,20,&sym,&cs);
// fprintf(stderr,"KEY: keysym=0x%04x state=%04x ks1=%x ks2=%x sym=%x\n",event.code,event.state,ks1,ks2,sym);
// }
        if(event.type==KeyPress){
          if(!(event.state&(LEFTBUTTONMASK|MIDDLEBUTTONMASK|RIGHTBUTTONMASK))) event.moved=0;
          if(!event.moved && (event.time-event.click_time<clickSpeed) && (event.window==event.click_window) && (event.code==event.click_button)){
            event.click_count++;
            }
          else{
            event.click_count=1;
            event.click_x=event.win_x;
            event.click_y=event.win_y;
            event.click_window=event.window;
            event.click_button=event.code;
            }
          event.click_time=event.time;
          }
        if(shell->passModalEvents()){
          if(shell->handle(this,MKUINT(0,event.type),&event)) refresh();
          }
        else if(ev.xany.type==KeyPress){
          beep();
          }
        event.last_x=event.win_x;
        event.last_y=event.win_y;
        break;
        
      // Motion
      case MotionNotify:
        event.type=SEL_MOTION;
        event.window=ev.xmotion.window;
        event.time=ev.xmotion.time;
        event.win_x=ev.xmotion.x;
        event.win_y=ev.xmotion.y;
        event.root_x=ev.xmotion.x_root;
        event.root_y=ev.xmotion.y_root;
        event.code=0;
        event.state=ev.xmotion.state;
        if(!event.moved && (FXABS(ev.xmotion.x-event.click_x)>=dragDelta || FXABS(ev.xmotion.y-event.click_y)>=dragDelta)){
          event.moved=1;
          }
        if(grabWindow){
          if(grabWindow->handle(this,MKUINT(0,event.type),&event)) refresh();
          }
        else if(window->passModalEvents()){
          if(window->handle(this,MKUINT(0,SEL_MOTION),&event)) refresh();
          }
        event.last_x=event.win_x;
        event.last_y=event.win_y;
        break;
        
      // Button
      case ButtonPress:
      case ButtonRelease:
        event.window=ev.xbutton.window;
        event.time=ev.xbutton.time;
        event.win_x=ev.xbutton.x;
        event.win_y=ev.xbutton.y;
        event.root_x=ev.xbutton.x_root;
        event.root_y=ev.xbutton.y_root;
        event.code=ev.xbutton.button;
        event.state=ev.xbutton.state;
        if(ev.xbutton.type==ButtonPress){
          event.type=SEL_LEFTBUTTONPRESS+((ev.xbutton.button-1)<<1);
          if(!(event.state&(LEFTBUTTONMASK|MIDDLEBUTTONMASK|RIGHTBUTTONMASK))) event.moved=0;
          if(!event.moved && (event.time-event.click_time<clickSpeed) && (event.window==event.click_window) && (event.code==event.click_button)){
            event.click_count++;
            }
          else{
            event.click_count=1;
            event.click_x=event.win_x;
            event.click_y=event.win_y;
            event.click_window=event.window;
            event.click_button=event.code;
            }
          event.click_time=event.time;
          }
        else if(ev.xbutton.type==ButtonRelease){
          event.type=SEL_LEFTBUTTONRELEASE+((ev.xbutton.button-1)<<1);
          }
        if(grabWindow){
          if(grabWindow->handle(this,MKUINT(0,event.type),&event)) refresh();
          }
        else if(window->passModalEvents()){
          if(window->handle(this,MKUINT(0,event.type),&event)) refresh();
          }
        else if(ev.xany.type==ButtonPress){
          beep();
          }
        event.last_x=event.win_x;
        event.last_y=event.win_y;
        break;

      // Crossing
      case EnterNotify:
      case LeaveNotify:
        if(ev.xcrossing.mode==NotifyGrab || ev.xcrossing.mode==NotifyUngrab || (ev.xcrossing.mode==NotifyNormal && ev.xcrossing.detail!=NotifyInferior)){
        //if(ev.xcrossing.mode==NotifyNormal && ev.xcrossing.detail!=NotifyInferior){
          event.type=SEL_ENTER+ev.xany.type-EnterNotify;
          event.time=ev.xcrossing.time;
          event.win_x=ev.xcrossing.x;
          event.win_y=ev.xcrossing.y;
          event.root_x=ev.xcrossing.x_root;
          event.root_y=ev.xcrossing.y_root;
          event.window=ev.xcrossing.window;
          event.code=ev.xcrossing.mode;
          if(window->handle(this,MKUINT(0,event.type),&event)) refresh();
          event.last_x=event.win_x;
          event.last_y=event.win_y;
          }
        break;

      // Focus
      case FocusIn:
      case FocusOut:
        event.type=SEL_FOCUSIN+ev.xfocus.type-FocusIn;
        event.window=ev.xfocus.window;
        event.code=ev.xfocus.mode;
        if(shell->handle(this,MKUINT(0,event.type),&event)) refresh();
        break;

      // Map
      case MapNotify:
        event.type=SEL_MAP;
        event.window=ev.xmap.window;
        window->handle(this,MKUINT(0,SEL_MAP),&event);
        break;

      // Unmap
      case UnmapNotify:
        event.type=SEL_UNMAP;
        event.window=ev.xunmap.window;
        window->handle(this,MKUINT(0,SEL_UNMAP),&event);
        break;

      // Create
      case CreateNotify:
        event.type=SEL_CREATE;
        event.window=ev.xcreatewindow.window;
        event.rect.x=ev.xcreatewindow.x;
        event.rect.y=ev.xcreatewindow.y;
        event.rect.w=ev.xcreatewindow.width;
        event.rect.h=ev.xcreatewindow.height;
        if(window->handle(this,MKUINT(0,SEL_CREATE),&event)) refresh();
        break;

      // Destroy
      case DestroyNotify:
        event.type=SEL_DESTROY;
        event.window=ev.xdestroywindow.window;
        if(window->handle(this,MKUINT(0,SEL_DESTROY),&event)) refresh();
        break;

      // Configure
      case ConfigureNotify:
        event.type=SEL_CONFIGURE;
        event.window=ev.xconfigure.window;
        event.rect.x=ev.xconfigure.x;
        event.rect.y=ev.xconfigure.y;
        event.rect.w=ev.xconfigure.width;
        event.rect.h=ev.xconfigure.height;
        if(window->handle(this,MKUINT(0,SEL_CONFIGURE),&event)) refresh();
        break;

      // Selection Clear
      case SelectionClear:
        if(ev.xselectionclear.selection==XA_PRIMARY){
          event.type=SEL_SELECTION_LOST;
          event.window=ev.xselectionclear.window;
          event.time=ev.xselectionclear.time;
          ddeSelection=ev.xselectionclear.selection;
          if(window->handle(this,MKUINT(0,SEL_SELECTION_LOST),&event)) refresh();
          }
        break;

      // Selection Request
      case SelectionRequest:
        event.type=SEL_SELECTION_REQUEST;
        event.window=ev.xselectionrequest.owner;
        event.time=ev.xselectionrequest.time;
        ddeSelection=ev.xselectionrequest.selection;
        ddeRequestor=ev.xselectionrequest.requestor;
        ddeTarget=ev.xselectionrequest.target;
        ddeProperty=ev.xselectionrequest.property;
        if(window->handle(this,MKUINT(0,SEL_SELECTION_REQUEST),&event)) refresh();
        se.xselection.type=SelectionNotify;
        se.xselection.property=ddeProperty; // May be None
        se.xselection.send_event=TRUE;
        se.xselection.display=display;
        se.xselection.requestor=ddeRequestor;
        se.xselection.selection=ddeSelection;
        se.xselection.target=ddeTarget;
        se.xselection.time=event.time;
        XSendEvent(display,ddeRequestor,True,NoEventMask,&se);
        break;

      // Circulate 
      case CirculateNotify:
        event.type=SEL_RAISED+(ev.xcirculate.place&1);
        event.window=ev.xcirculate.window;
        window->handle(this,MKUINT(0,event.type),&event);
        break;

      // Visibility
      case VisibilityNotify:
        event.type=SEL_VISIBILITY;
        event.window=ev.xvisibility.window;
        event.code=ev.xvisibility.state;
        window->handle(this,MKUINT(0,SEL_VISIBILITY),&event);
        break;

      // Keyboard mapping
      case MappingNotify:
        if(ev.xmapping.request!=MappingPointer) XRefreshKeyboardMapping(&ev.xmapping);
        break;

      // Client message
      case ClientMessage:
        if(ev.xclient.format==32){
        
          // WM_PROTOCOLS
          if(ev.xclient.message_type==wmProtocols){
            if((FXID)ev.xclient.data.l[0]==wmDeleteWindow){
              event.type=SEL_CLOSE;
              event.window=ev.xclient.window;
              window->handle(this,MKUINT(0,SEL_CLOSE),&event);
              }
            else if((FXID)ev.xclient.data.l[0]==wmQuitApp){
              event.type=SEL_CLOSE;
              event.window=ev.xclient.window;
              window->handle(this,MKUINT(0,SEL_CLOSE),&event);
              }
            }
          
          // XDND Enter from source
          else if(ev.xclient.message_type==xdndEnter){
//fprintf(stderr,"DNDEnter from remote\n");
            event.type=SEL_DND_ENTER;
            event.window=ev.xclient.window;
            xdndTarget=ev.xclient.window;                             // Target of the message
            xdndSource=ev.xclient.data.l[0];                          // Source window
            xdndVersion=((FXuint)ev.xclient.data.l[1])>>24;           // Protocol version
            if(ev.xclient.data.l[1]&1){                               // More than 3 types
              unsigned long ni,b; int fmt; Atom typ;
              unsigned char* data=NULL;
              if(XGetWindowProperty(display,xdndSource,xdndTypes,0,1024,False,XA_ATOM,&typ,&fmt,&ni,&b,&data)!=Success) return;
              if(!data) return;
              if(typ!=XA_ATOM || fmt!=32 || ni==0){ XFree(data); return; }
              xdndTypeList=(FXID*)data;
              xdndNumTypes=ni;
              }
            else{                                                     // If types if less than 3
              xdndTypeList=(FXID*)malloc(3*sizeof(FXID));
              xdndNumTypes=0;
              if(ev.xclient.data.l[2]){xdndTypeList[0]=ev.xclient.data.l[2];xdndNumTypes++;}
              if(ev.xclient.data.l[3]){xdndTypeList[1]=ev.xclient.data.l[3];xdndNumTypes++;}
              if(ev.xclient.data.l[4]){xdndTypeList[2]=ev.xclient.data.l[4];xdndNumTypes++;}
              }
            ddeSelection=xdndSelection;
            if(window->handle(this,MKUINT(0,SEL_DND_ENTER),&event)) refresh();
            ddeSelection=XA_PRIMARY;
            }
          
          // XDND Leave from source
          else if(ev.xclient.message_type==xdndLeave){
//fprintf(stderr,"DNDLeave from remote\n");
            event.type=SEL_DND_LEAVE;
            event.window=ev.xclient.window;
            xdndTarget=ev.xclient.window;
            xdndSource=ev.xclient.data.l[0];                          // Source window
            ddeSelection=xdndSelection;
            if(window->handle(this,MKUINT(0,SEL_DND_LEAVE),&event)) refresh();
            ddeSelection=XA_PRIMARY;
            if(xdndTypeList) XFree(xdndTypeList);
            xdndTypeList=NULL;
            xdndNumTypes=0;
            xdndVersion=XDND_PROTOCOL_VERSION;                        // Done, restore stuff
            xdndTarget=0;
            xdndSource=0;
            }
          
          // XDND Drop from source
          else if(ev.xclient.message_type==xdndDrop){
//fprintf(stderr,"DNDDrop from remote\n");
            event.type=SEL_DND_DROP;
            event.window=ev.xclient.window;
            xdndTarget=ev.xclient.window;
            xdndSource=ev.xclient.data.l[0];                          // Source window
            event.time=ev.xclient.data.l[2];
            ddeSelection=xdndSelection;
            if(window->handle(this,MKUINT(0,SEL_DND_DROP),&event)) refresh();
            se.xclient.type=ClientMessage;                            // Drop window has finished processing of the drop
            se.xclient.display=display;
            se.xclient.message_type=xdndFinished;
            se.xclient.format=32;
            se.xclient.window=xdndSource;
            se.xclient.data.l[0]=xdndTarget;                          // Target window, i.e. me!
            se.xclient.data.l[1]=0;
            se.xclient.data.l[2]=0;
            se.xclient.data.l[3]=0;
            se.xclient.data.l[4]=0;
            XSendEvent(display,xdndSource,True,NoEventMask,&se);
            ddeSelection=XA_PRIMARY;                                  // Done, clean up this end...
            if(xdndTypeList) XFree(xdndTypeList);
            xdndTypeList=NULL;
            xdndNumTypes=0;
            xdndVersion=XDND_PROTOCOL_VERSION;
            xdndTarget=0;
            xdndSource=0;
            }
          
          // XDND Position from source
          else if(ev.xclient.message_type==xdndPosition){
//fprintf(stderr,"DNDPosition from remote\n");
            event.type=SEL_DND_MOTION;
            event.window=ev.xclient.window;
            xdndTarget=ev.xclient.window;
            xdndSource=ev.xclient.data.l[0];                          // Source window
            event.root_x=((FXuint)ev.xclient.data.l[2])>>16;
            event.root_y=((FXuint)ev.xclient.data.l[2])&0xffff;
            event.time=ev.xclient.data.l[3];
            xdndAction=ev.xclient.data.l[4];                          // Drag and Drop Action
            xdndAccepts=FALSE;                                        // If message is not handled, we assume no drop!
            XTranslateCoordinates(display,XDefaultRootWindow(display),event.window,event.root_x,event.root_y,&event.win_x,&event.win_y,&tmp);
            ddeSelection=xdndSelection;
            if(window->handle(this,MKUINT(0,SEL_DND_MOTION),&event)) refresh();
            event.last_x=event.win_x;
            event.last_y=event.win_y;
            ddeSelection=XA_PRIMARY;
            se.xclient.type=ClientMessage;
            se.xclient.display=display;
            se.xclient.message_type=xdndStatus;
            se.xclient.format=32;
            se.xclient.window=xdndSource;
            se.xclient.data.l[0]=xdndTarget;                          // Target window, i.e. me!
            se.xclient.data.l[1]=0;
            if(xdndAccepts) se.xclient.data.l[1]|=1;                  // Target accepted
            if(xdndWantUpdates) se.xclient.data.l[1]|=2;              // Target wants continuous position updates
            se.xclient.data.l[2]=MKUINT(xdndRect.y,xdndRect.x);
            se.xclient.data.l[3]=MKUINT(xdndRect.h,xdndRect.w);
            se.xclient.data.l[4]=xdndAction;                          // Type of Drag and Drop Action accepted
            XSendEvent(display,xdndSource,True,NoEventMask,&se);
            }
          
          // XDND Status from target
          else if(ev.xclient.message_type==xdndStatus){
//fprintf(stderr,"DNDStatus from %s remote\n",xdndTarget==ev.xclient.data.l[0] ? "active" : "old");

            // Ignore status messages which are (no longer) applicable
            if(xdndTarget==(FXID)ev.xclient.data.l[0]){
              xdndAccepts=ev.xclient.data.l[1]&1;
              xdndAction=ev.xclient.data.l[4];                        // Drag and Drop Action
              xdndWantUpdates=(ev.xclient.data.l[1]&2)!=0;
              xdndRect.x=((FXuint)ev.xclient.data.l[2])>>16;
              xdndRect.y=((FXuint)ev.xclient.data.l[2])&0xffff;
              xdndRect.w=((FXuint)ev.xclient.data.l[3])>>16;
              xdndRect.h=((FXuint)ev.xclient.data.l[3])&0xffff;
              xdndStatusReceived=TRUE;
              xdndStatusPending=FALSE;
              if(xdndSendPosition){                                   // Long overdue position update must now be sent
                se.xclient.type=ClientMessage;
                se.xclient.display=display;
                se.xclient.message_type=xdndPosition;
                se.xclient.format=32;
                se.xclient.window=xdndTarget;
                se.xclient.data.l[0]=xdndSource;                      // Source window
                se.xclient.data.l[1]=0;
                se.xclient.data.l[2]=MKUINT(xdndYPos,xdndXPos);       // Coordinates
                se.xclient.data.l[3]=event.time;                      // Time stamp
                se.xclient.data.l[4]=xdndAction;                      // Drag and Drop action
                XSendEvent(display,xdndTarget,True,NoEventMask,&se);
                xdndSendPosition=FALSE;
                xdndStatusPending=TRUE;                               // We're now waiting for the next status message!
                }
              }
            }
          }
        break;

      // Others
      default:
        break;
      }
    }
  }


/*******************************************************************************/



// Flush pending repaints
void FXApp::flush(){
  FXRepaint *r;
  XEvent ev;
   
  // Process display related events
  while(XCheckMaskEvent(display,ExposureMask|StructureNotifyMask,&ev)){
    
    // Saving expose events for later
    if(ev.xany.type==Expose || ev.xany.type==GraphicsExpose){
      addRepaint(ev.xexpose.window,ev.xexpose.x,ev.xexpose.y,ev.xexpose.width,ev.xexpose.height,0);
      continue;
      }
    
    // Handle other events directly
    dispatchEvent(ev);
    }
  
  // Process pending expose events
  while(repaints){
    r=repaints;
    ev.xany.type=Expose;
    ev.xexpose.window=r->window;
    ev.xexpose.x=r->rect.x;
    ev.xexpose.y=r->rect.y;
    ev.xexpose.width=r->rect.w-r->rect.x;
    ev.xexpose.height=r->rect.h-r->rect.y;
    repaints=r->next;
    r->next=repaintrecs;
    repaintrecs=r;
    dispatchEvent(ev);
    }
  
  // Flush the buffers to the server
  //XSync(display,False);
  XFlush(display);
  }


// Run application
FXint FXApp::run(){
  runUntil(done);
  return exitcode;
  }


// Run till some flag becomes non-zero
void FXApp::runUntil(FXuint& condition){
  XEvent ev;
  do{
    getNextEvent(ev);
    dispatchEvent(ev);
    }
  while(condition==0);
  }


// Run modal for window
// Events of any modal window in existence are only caught in the deepest nested
// modal loop; so we check all modal loop invocations outstanding and see which
// loop matches the given unmap/destroy event, so that once the stack gets unwound 
// back, we can break out; otherwise, we may never get back to the toplevel again!
// Note that only the deepest-nested modal loop's window gets any events!
void FXApp::runModalFor(FXWindow* window){
  FXInvocation inv,*ip;
  XEvent ev;
  if(window==NULL || window->id()==0){ fxerror("%s::runModalFor: illegal modal window specified.\n"); }
  inv.upper=invocation;
  inv.window=window->id();
  inv.done=FALSE;
  invocation=&inv;
  do{
    getNextEvent(ev);
    dispatchEvent(ev);
    if(ev.xany.type==UnmapNotify || ev.xany.type==DestroyNotify){
      for(ip=&inv;ip;ip=ip->upper){if(ip->window==ev.xany.window){ip->done=TRUE;break;}}
      }
    }
  while(!inv.done);
  invocation=inv.upper;
  }


// Force GUI refresh
void FXApp::forceRefresh(){
  refresher=root;
  while(refresher){
    refresher->handle(this,MKUINT(0,SEL_UPDATE),&event);
    if(refresher->getFirst()){
      refresher=refresher->getFirst();
      continue;
      }
    while(!refresher->getNext() && refresher->getParent()!=root){
      refresher=refresher->getParent();
      }
    refresher=refresher->getNext();
    }
  }


// Perform refresh some time
void FXApp::refresh(){
  refresher=root;
  }


// Initialize application
void FXApp::init(int& argc,char** argv){
  char *dpy=NULL;
  int i,j;
  
  // Verify implementation invariants
  FXASSERT(sizeof(FXuchar)==1);
  FXASSERT(sizeof(FXbool)==1);
  FXASSERT(sizeof(FXchar)==1);
  FXASSERT(sizeof(FXushort)==2);
  FXASSERT(sizeof(FXshort)==2);
  FXASSERT(sizeof(FXuint)==4);
  FXASSERT(sizeof(FXint)==4);
  FXASSERT(sizeof(FXfloat)==4);
  FXASSERT(sizeof(FXdouble)==8);
  
  FXASSERT(sizeof(FXString)==sizeof(FXchar*));
  
  // Long is not always available on all implementations
#ifdef FX_LONG
  FXASSERT(sizeof(FXulong)==8);
  FXASSERT(sizeof(FXlong)==8);
#endif

  // No shared memory with Windoze
#ifdef WIN32
  shmi=FALSE;
  shmp=FALSE;
#endif
  
  // Copy application name
  appname=argv[0];
  
  // Parse out FOX args
  i=j=1;
  while(j<argc){
    
    // Start synchronized mode
    if(strcmp(argv[j],"-sync")==0){
      synchronize=TRUE;
      j++;
      continue;
      }
    
    // Start synchronized mode
    if(strcmp(argv[j],"-noshm")==0){
      shmi=FALSE;
      shmp=FALSE;
      j++;
      continue;
      }
    
    // Alternative display
    if(strcmp(argv[j],"-display")==0){
      j++;
      if(j>=argc){ fxerror("%s: missing argument for -display.\n",appname); }
      dpy=argv[j];
      j++;
      continue;
      }
    
    // Copy program arguments
    argv[i++]=argv[j++];
    }
  
  // Adjust argment count
  argv[i]=NULL;
  argc=i;
  
  // Now open display
  openDisplay(dpy);
  }


// Exit application
void FXApp::exit(FXint code){
  ::exit(code);
  exitcode=code;
  done=1;
  }
  

// Create application's windows
void FXApp::create(){ 
  if(display==NULL){ fxerror("%s::create(): Should open display first\n",getClassName()); }
  root->create();
  }


// Destroy application's windows
void FXApp::destroy(){
  root->destroy();
  }


// Handle quit
long FXApp::onQuit(FXObject*,FXSelector,void*){
  exit(0); 
  return 1;
  }


// Register DND type
FXDragType FXApp::registerDragType(const FXString& name) const {
  if(display==NULL){ fxerror("%s::registerDragType: Should open display first\n",getClassName()); }
  return (FXDragType)XInternAtom(display,name,0);
  }


// Get name of registered drag type
FXString FXApp::getDragTypeName(FXDragType type) const {
  if(display==NULL){ fxerror("%s::getDragTypeName: Should open display first\n",getClassName()); }
  FXchar *name=XGetAtomName(display,type);
  FXString dragtypename(name);
  XFree(name);
  return dragtypename;
  }


/*******************************************************************************/


// Show a modal error message
FXuint FXApp::showModalErrorBox(FXuint opts,const char* title,const char* message,...){
  const unsigned char errorIcon[]={
    0x47,0x49,0x46,0x38,0x37,0x61,0x20,0x00,0x20,0x00,0xf1,0x00,0x00,0x80,0x80,0x80,
    0x80,0x00,0x00,0xff,0x00,0x00,0xff,0xff,0xff,0x2c,0x00,0x00,0x00,0x00,0x20,0x00,
    0x20,0x00,0x00,0x02,0x91,0x84,0x8f,0x10,0xcb,0x9b,0x0f,0x0f,0x13,0xb4,0x52,0x16,
    0x73,0xb0,0x9c,0x87,0x9c,0x2c,0xdd,0x58,0x39,0xe0,0x46,0xa6,0x97,0xa6,0xb6,0xc2,
    0x07,0xa1,0x2e,0x09,0x87,0xdd,0xa0,0xe2,0x9e,0xcd,0x0d,0x3e,0xe9,0xd3,0x59,0x6a,
    0x8a,0x5e,0x50,0x58,0x39,0x22,0x5f,0x08,0x59,0xf2,0x68,0x51,0x76,0x6a,0xce,0x28,
    0x54,0x20,0x1d,0xc1,0xaa,0x56,0x65,0x30,0xb5,0xcd,0x79,0x97,0xbb,0xa2,0xf8,0x0a,
    0x36,0x70,0x8d,0x68,0x9a,0xfa,0x9c,0x75,0x9b,0x47,0xe3,0xaf,0x7c,0x8d,0xbd,0xc6,
    0xcb,0xf3,0xa7,0x9d,0xb2,0xb7,0x22,0xc1,0xf6,0x43,0xb8,0x44,0xc4,0xf5,0xc7,0x36,
    0xc5,0x13,0x95,0xc3,0xf8,0x80,0x37,0xc3,0x14,0x33,0x29,0x57,0x69,0x39,0x04,0xa2,
    0x20,0xa9,0x45,0x74,0x32,0xf9,0xb9,0xc9,0x89,0x87,0x31,0x1a,0xd1,0xd0,0x70,0xba,
    0xca,0xda,0xea,0x6a,0x50,0x00,0x00,0x3b
    };
  FXGIFIcon icon(this,errorIcon);
  va_list arguments;
  char buf[1000];
  va_start(arguments,message);
  vsprintf(buf,message,arguments);
  va_end(arguments);
  FXMessageBox box(this,title,buf,&icon,opts|DECOR_TITLE|DECOR_BORDER|DIALOG_MODAL);
  return box.execute();
  }


// Show a modal warning message
FXuint FXApp::showModalWarningBox(FXuint opts,const char* title,const char* message,...){
  const unsigned char warningIcon[]={
    0x47,0x49,0x46,0x38,0x37,0x61,0x20,0x00,0x20,0x00,0xf2,0x00,0x00,0x80,0x80,0x80,
    0x80,0x80,0x00,0xff,0xff,0x00,0xc0,0xc0,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x20,0x00,0x20,0x00,0x00,0x03,
    0x8f,0x08,0xba,0xbc,0xf1,0x2d,0xca,0xa9,0x82,0x10,0x83,0xd0,0x2d,0xed,0xc5,0x1a,
    0x27,0x02,0xde,0x17,0x8e,0x54,0xf9,0x65,0x68,0xfa,0xbd,0x67,0xcb,0xa8,0x2f,0x2b,
    0xcf,0x6f,0x1e,0xcb,0x74,0x6e,0xdf,0xa5,0x0c,0x81,0x30,0x30,0xdd,0x48,0xb0,0xe1,
    0x70,0xb5,0x13,0xa9,0x94,0x4b,0x63,0x8b,0x06,0x25,0xd4,0x9a,0xae,0xa4,0x52,0x87,
    0xea,0x55,0x7d,0xd8,0x48,0x2f,0xa0,0xf4,0x3a,0x73,0x17,0x21,0x11,0xfd,0x9b,0xf4,
    0x2e,0x5b,0xb4,0x20,0x5c,0x91,0x5f,0xc8,0x6f,0x50,0x56,0x9e,0x29,0xda,0xc3,0x79,
    0x26,0x76,0x69,0x58,0x81,0x83,0x5c,0x0d,0x86,0x20,0x6b,0x76,0x6d,0x48,0x87,0x50,
    0x90,0x38,0x87,0x73,0x71,0x8d,0x27,0x8a,0x8b,0x7e,0x83,0x98,0x94,0x9e,0x30,0x0e,
    0x9f,0x9f,0x6d,0x99,0x9e,0x8e,0x24,0x55,0xa9,0xaa,0xa9,0x47,0xad,0xae,0x22,0x09,
    0x00,0x3b
    };
  FXGIFIcon icon(this,warningIcon);
  va_list arguments;
  char buf[1000];
  va_start(arguments,message);
  vsprintf(buf,message,arguments);
  va_end(arguments);
  FXMessageBox box(this,title,buf,&icon,opts|DECOR_TITLE|DECOR_BORDER|DIALOG_MODAL);
  return box.execute();
  }


// Show a modal question dialog
FXuint FXApp::showModalQuestionBox(FXuint opts,const char* title,const char* message,...){
  const unsigned char questionIcon[]={
    0x47,0x49,0x46,0x38,0x37,0x61,0x20,0x00,0x20,0x00,0xf2,0x00,0x00,0x80,0x80,0x80,
    0xc0,0xc0,0xc0,0xff,0xff,0xff,0x00,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x20,0x00,0x20,0x00,0x00,0x03,
    0x97,0x08,0xba,0xdc,0xfe,0xf0,0x05,0x41,0x6b,0x88,0x58,0xcd,0xca,0xf9,0xcd,0xcc,
    0xd6,0x8d,0x16,0x08,0x90,0x02,0x30,0x0c,0x80,0xc8,0x11,0x18,0x19,0xac,0xf4,0xe0,
    0x56,0xf0,0x23,0xdb,0x54,0x7d,0x53,0xb9,0x10,0x69,0xc5,0xa9,0xc9,0x82,0x0a,0x94,
    0x51,0xb0,0x1c,0x21,0x7f,0x3d,0x9a,0x60,0x26,0x3d,0x2e,0x50,0xd3,0xd5,0xa4,0x49,
    0x0a,0x62,0x3d,0x55,0xac,0xf7,0x1b,0x1d,0x90,0x05,0x63,0xb2,0xf6,0x9c,0xfe,0x12,
    0xd9,0x0b,0xe8,0xd9,0x7a,0xfd,0x52,0xe5,0xb8,0x06,0x9e,0x19,0xa6,0x33,0xdc,0x7d,
    0x4e,0x0f,0x04,0x58,0x77,0x62,0x11,0x84,0x73,0x82,0x18,0x04,0x7b,0x23,0x01,0x48,
    0x88,0x8d,0x67,0x90,0x91,0x19,0x04,0x98,0x3f,0x95,0x96,0x26,0x0a,0x98,0x93,0x40,
    0x9d,0x26,0x89,0x68,0xa2,0x20,0xa4,0x9c,0xa6,0x0d,0x89,0xa9,0xaa,0x0c,0x84,0xad,
    0xae,0x0b,0x98,0xb2,0xb5,0xb6,0xaa,0x09,0x00,0x3b
    };
  FXGIFIcon icon(this,questionIcon);
  va_list arguments;
  char buf[1000];
  va_start(arguments,message);
  vsprintf(buf,message,arguments);
  va_end(arguments);
  FXMessageBox box(this,title,buf,&icon,opts|DECOR_TITLE|DECOR_BORDER|DIALOG_MODAL);
  return box.execute();
  }


// Show a modal information dialog
FXuint FXApp::showModalInformationBox(FXuint opts,const char* title,const char* message,...){
  const unsigned char infoIcon[]={
    0x47,0x49,0x46,0x38,0x37,0x61,0x20,0x00,0x20,0x00,0xf2,0x00,0x00,0x80,0x80,0x80,
    0xc0,0xc0,0xc0,0xff,0xff,0xff,0x00,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x20,0x00,0x20,0x00,0x00,0x03,
    0x8c,0x08,0xba,0xdc,0xfe,0xf0,0x05,0x41,0x6b,0x88,0x58,0xcd,0xca,0xf9,0xcd,0xcc,
    0xd6,0x8d,0x16,0x08,0x90,0xc1,0xa0,0x0e,0x22,0x47,0x60,0xa4,0xb0,0xae,0xf1,0xfb,
    0xc4,0xf2,0x3c,0xe0,0x76,0x88,0xa7,0xab,0x56,0x27,0xd0,0x53,0xe0,0x8e,0xa4,0xa2,
    0x10,0x79,0x24,0x2e,0x8e,0x3a,0x15,0xb2,0xc7,0xd4,0x4d,0x9f,0x48,0xeb,0x91,0x9a,
    0x9d,0x5d,0x8d,0x5d,0xda,0x76,0xb1,0xec,0x68,0x63,0x4e,0x2c,0xee,0x9c,0x6c,0x94,
    0x2b,0xec,0x61,0x11,0x3c,0x8a,0x8a,0x3b,0x73,0x05,0x81,0x39,0x86,0xec,0xf9,0x23,
    0x79,0x0d,0x04,0x6f,0x68,0x82,0x0e,0x04,0x84,0x4c,0x44,0x87,0x7e,0x89,0x4b,0x8c,
    0x8d,0x20,0x89,0x8a,0x14,0x92,0x26,0x0c,0x7f,0x02,0x97,0x98,0x7a,0x15,0x9c,0x9d,
    0x00,0x7f,0xa0,0xa1,0x7b,0xa4,0xa5,0xa7,0xa1,0xaa,0xab,0x19,0x09,0x00,0x3b
    };
  FXGIFIcon icon(this,infoIcon);
  va_list arguments;
  char buf[1000];
  va_start(arguments,message);
  vsprintf(buf,message,arguments);
  va_end(arguments);
  FXMessageBox box(this,title,buf,&icon,opts|DECOR_TITLE|DECOR_BORDER|DIALOG_MODAL);
  return box.execute();
  }


// Beep
void FXApp::beep(){
  XBell(display,0);
  }


// Check if we're saving as expected
void FXApp::save(FXStream& store) const {
  FXObject::save(store);
  store << clickSpeed;
  store << scrollSpeed;
  store << blinkSpeed;
  store << menuPause;
  store << tooltipPause;
  store << tooltipTime;
  store << scrollbarWidth;
  store << dragDelta;
  store << backColor;
  store << foreColor;
  store << hiliteColor;
  store << shadowColor;
  store << borderColor;
  store << selforeColor;
  store << selbackColor;
  }


// Check if we're loading as expected
void FXApp::load(FXStream& store){ 
  FXObject::load(store);
  store >> clickSpeed;
  store >> scrollSpeed;
  store >> blinkSpeed;
  store >> menuPause;
  store >> tooltipPause;
  store >> tooltipTime;
  store >> scrollbarWidth;
  store >> dragDelta;
  store >> backColor;
  store >> foreColor;
  store >> hiliteColor;
  store >> shadowColor;
  store >> borderColor;
  store >> selforeColor;
  store >> selbackColor;
  }


// Virtual destructor
FXApp::~FXApp(){
  app=NULL;
  }
