/********************************************************************************
*                                                                               *
*                     C o m p i l e d - i n   R e s o u r c e s                 *
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
* $Id: FXResources.cpp,v 1.1 1998/03/23 01:54:47 jeroen Exp $                          *
********************************************************************************/
#include "fxdefs.h"


/*******************************************************************************/

// Question
static const FXchar *const question_xpm[] = {
  "32 32 6 1",
  "       s none  m white c none",
  ".      s s_darkgray    m black c #808080",
  "X      s s_gray        m white c #c0c0c0",
  "o      s s_white       m white c white",
  "O      s s_blue        m white c blue",
  "+      s s_black       m black c black",
  "           ........             ",
  "        ...XooooooX...          ",
  "      ..XooooooooooooX..        ",
  "     .XooooooooooooooooX.       ",
  "    .oooooooo.OO.Xooooooo+      ",
  "   .oooooooXOOOOOOXooooooo+     ",
  "  .oooooooXOXooOOOOXooooooo+    ",
  " .XoooooooOOooooOOOOoooooooX+   ",
  " .ooooooooOOOOooOOOOoooooooo+.  ",
  ".XooooooooOOOOoXOOOOooooooooX+. ",
  ".oooooooooXOOXoOOOOoooooooooo+. ",
  ".oooooooooooooXOOOooooooooooo+..",
  ".oooooooooooooOOOoooooooooooo+..",
  ".oooooooooooooOOXoooooooooooo+..",
  ".oooooooooooooOOooooooooooooo+..",
  ".XooooooooooooooooooooooooooX+..",
  " .oooooooooooXOOXooooooooooo+...",
  " .XooooooooooOOOOooooooooooX+...",
  "  .ooooooooooOOOOoooooooooo+... ",
  "   +oooooooooXOOXooooooooo+.... ",
  "    +oooooooooooooooooooo+....  ",
  "     +XooooooooooooooooX+....   ",
  "      ++XooooooooooooX++....    ",
  "       .+++XooooooX+++.....     ",
  "        ...+++Xooo+.......      ",
  "          ....+ooo+.....        ",
  "             .+ooo+..           ",
  "               +oo+..           ",
  "                +o+..           ",
  "                 ++..           ",
  "                  ...           ",
  "                   ..           "
  };
  

// Warning
static const FXchar *const warning_xpm[] = {
  "32 32 6 1",
  "       s none  m white c none",
  ".      s s_darkyellow  m black c #808000",
  "X      s s_yellow      m white c yellow",
  "o      s s_gray        m white c #c0c0c0",
  "O      s s_black       m black c black",
  "+      s s_darkgray    m black c #808080",
  "             ...                ",
  "            .XXoO               ",
  "           .XXXXoO+             ",
  "           .XXXXXO++            ",
  "          .XXXXXXoO++           ",
  "          .XXXXXXXO++           ",
  "         .XXXXXXXXoO++          ",
  "         .XXXXXXXXXO++          ",
  "        .XXXXXXXXXXoO++         ",
  "        .XXXoOOOoXXXO++         ",
  "       .XXXXOOOOOXXXoO++        ",
  "       .XXXXOOOOOXXXXO++        ",
  "      .XXXXXOOOOOXXXXoO++       ",
  "      .XXXXXOOOOOXXXXXO++       ",
  "     .XXXXXXOOOOOXXXXXoO++      ",
  "     .XXXXXX.OOO.XXXXXXO++      ",
  "    .XXXXXXXoOOOoXXXXXXoO++     ",
  "    .XXXXXXXXOOOXXXXXXXXO++     ",
  "   .XXXXXXXXX.O.XXXXXXXXoO++    ",
  "   .XXXXXXXXXoOoXXXXXXXXXO++    ",
  "  .XXXXXXXXXXXOXXXXXXXXXXoO++   ",
  "  .XXXXXXXXXXXXXXXXXXXXXXXO++   ",
  " .XXXXXXXXXXXoOOoXXXXXXXXXoO++  ",
  " .XXXXXXXXXXXOOOOXXXXXXXXXXO++  ",
  ".XXXXXXXXXXXXOOOOXXXXXXXXXXoO++ ",
  ".XXXXXXXXXXXXoOOoXXXXXXXXXXXO++ ",
  ".XXXXXXXXXXXXXXXXXXXXXXXXXXXO+++",
  ".XXXXXXXXXXXXXXXXXXXXXXXXXXoO+++",
  " .XXXXXXXXXXXXXXXXXXXXXXXXoO++++",
  "  .OOOOOOOOOOOOOOOOOOOOOOOO+++++",
  "    +++++++++++++++++++++++++++ ",
  "     +++++++++++++++++++++++++  "
  };


// Error
static const FXchar *const error_xpm[] = {
  "32 32 5 1",
  "       s none  m white c none",
  ".      s s_darkred     m black c #800000",
  "X      s s_red m white c red",
  "o      s s_darkgray    m black c #808080",
  "O      s s_white       m white c white",
  "           ........             ",
  "        ...XXXXXXXX...          ",
  "       .XXXXXXXXXXXXXX.         ",
  "     ..XXXXXXXXXXXXXXXX..       ",
  "    .XXXXXXXXXXXXXXXXXXXX.      ",
  "   .XXXXXXXXXXXXXXXXXXXXXX.     ",
  "   .XXXXXXXXXXXXXXXXXXXXXX.o    ",
  "  .XXXXXXOXXXXXXXXXXOXXXXXX.o   ",
  " .XXXXXXOOOXXXXXXXXOOOXXXXXX.   ",
  " .XXXXXOOOOOXXXXXXOOOOOXXXXX.o  ",
  " .XXXXXXOOOOOXXXXOOOOOXXXXXX.oo ",
  ".XXXXXXXXOOOOOXXOOOOOXXXXXXXX.o ",
  ".XXXXXXXXXOOOOOOOOOOXXXXXXXXX.o ",
  ".XXXXXXXXXXOOOOOOOOXXXXXXXXXX.oo",
  ".XXXXXXXXXXXOOOOOOXXXXXXXXXXX.oo",
  ".XXXXXXXXXXXOOOOOOXXXXXXXXXXX.oo",
  ".XXXXXXXXXXOOOOOOOOXXXXXXXXXX.oo",
  ".XXXXXXXXXOOOOOOOOOOXXXXXXXXX.oo",
  ".XXXXXXXXOOOOOXXOOOOOXXXXXXXX.oo",
  " .XXXXXXOOOOOXXXXOOOOOXXXXXX.ooo",
  " .XXXXXOOOOOXXXXXXOOOOOXXXXX.ooo",
  " .XXXXXXOOOXXXXXXXXOOOXXXXXX.oo ",
  "  .XXXXXXOXXXXXXXXXXOXXXXXX.ooo ",
  "   .XXXXXXXXXXXXXXXXXXXXXX.oooo ",
  "   .XXXXXXXXXXXXXXXXXXXXXX.ooo  ",
  "    .XXXXXXXXXXXXXXXXXXXX.ooo   ",
  "     ..XXXXXXXXXXXXXXXX..oooo   ",
  "      o.XXXXXXXXXXXXXX.ooooo    ",
  "       o...XXXXXXXX...ooooo     ",
  "         oo........oooooo       ",
  "          oooooooooooooo        ",
  "             oooooooo           "
  };


// Information
static const FXchar *const info_xpm[] = {
  "32 32 6 1",
  "       s none  m white c none",
  ".      s s_darkgray    m black c #808080",
  "X      s s_gray        m white c #c0c0c0",
  "o      s s_white       m white c white",
  "O      s s_blue        m white c blue",
  "+      s s_black       m black c black",
  "           ........             ",
  "        ...XooooooX...          ",
  "      ..XooooooooooooX..        ",
  "     .XooooooooooooooooX.       ",
  "    .oooooooXOOOOXooooooo+      ",
  "   .ooooooooOOOOOOoooooooo+     ",
  "  .oooooooooOOOOOOooooooooo+    ",
  " .XoooooooooXOOOOXoooooooooX+   ",
  " .oooooooooooooooooooooooooo+.  ",
  ".XooooooooooooooooooooooooooX+. ",
  ".ooooooooooOOOOOOOooooooooooo+. ",
  ".ooooooooooooOOOOOooooooooooo+..",
  ".ooooooooooooOOOOOooooooooooo+..",
  ".ooooooooooooOOOOOooooooooooo+..",
  ".ooooooooooooOOOOOooooooooooo+..",
  ".XoooooooooooOOOOOooooooooooX+..",
  " .oooooooooooOOOOOoooooooooo+...",
  " .XooooooooooOOOOOoooooooooX+...",
  "  .ooooooooOOOOOOOOOooooooo+... ",
  "   +oooooooooooooooooooooo+.... ",
  "    +oooooooooooooooooooo+....  ",
  "     +XooooooooooooooooX+....   ",
  "      ++XooooooooooooX++....    ",
  "       .+++XooooooX+++.....     ",
  "        ...+++Xooo+.......      ",
  "          ....+ooo+.....        ",
  "             .+ooo+..           ",
  "               +oo+..           ",
  "                +o+..           ",
  "                 ++..           ",
  "                  ...           ",
  "                   ..           "
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

   
// Corner resize cursor
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


/*******************************************************************************/

   
// Default palette of colors to allocate
const FXuchar default_palette[][3]={
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

