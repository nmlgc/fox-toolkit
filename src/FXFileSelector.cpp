/********************************************************************************
*                                                                               *
*                  F i l e   S e l e c t i o n   W i d g e t                    *
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
* $Id: FXFileSelector.cpp,v 1.101 2002/02/27 03:45:55 fox Exp $                  *
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
#include "FXFile.h"
#include "FXSettings.h"
#include "FXRegistry.h"
#include "FXAccelTable.h"
#include "FXApp.h"
#include "FXFont.h"
#include "FXGIFIcon.h"
#include "FXBMPIcon.h"
#include "FXRecentFiles.h"
#include "FXFrame.h"
#include "FXLabel.h"
#include "FXMenuButton.h"
#include "FXTextField.h"
#include "FXButton.h"
#include "FXToggleButton.h"
#include "FXCheckButton.h"
#include "FXMenuButton.h"
#include "FXPacker.h"
#include "FXHorizontalFrame.h"
#include "FXVerticalFrame.h"
#include "FXMatrix.h"
#include "FXCanvas.h"
#include "FXShell.h"
#include "FXPopup.h"
#include "FXMenuPane.h"
#include "FXScrollbar.h"
#include "FXScrollArea.h"
#include "FXList.h"
#include "FXTreeList.h"
#include "FXComboBox.h"
#include "FXTreeListBox.h"
#include "FXDirBox.h"
#include "FXHeader.h"
#include "FXIconList.h"
#include "FXFileList.h"
#include "FXFileSelector.h"
#include "FXMenuCaption.h"
#include "FXMenuCascade.h"
#include "FXMenuCommand.h"
#include "FXMenuSeparator.h"
#include "FXTopWindow.h"
#include "FXDialogBox.h"
#include "FXInputDialog.h"
#include "FXSeparator.h"
#include "FXMessageBox.h"


/*
  Notes:
  - Getting a file name according to what we want:

    - Any filename for saving (but with existing dir part)
    - An existing file for loading
    - An existing directory
    - Multiple filenames.

  - Get network drives to work.

  - Change filter specification; below sets two filters:

      "Source Files (*.cpp,*.cc,*.C)\nHeader files (*.h,*.H)"

    Instead of ',' you should also be able to use '|' in the above.

  - Multi-file mode needs to allow for manual entry in the text field.

  - Got nifty handling when entering path in text field:

      1) If its a directory you typed, switch to the directory.

      2) If the directory part of the file name exists:
         if SELECTFILE_ANY mode, then we're done.
         if SELECTFILE_EXISTING mode AND file exists, we're done.
         if SELECTFILE_MULTIPLE mode AND all files exist, we're done.

      3) Else use the fragment of the directory which still exists, and
         switch to that directory; leave the incorrect tail-end in the
         text field to be edited further

  - In directory mode, only way to return is by accept.

  - Switching directories zaps text field value, but not in SELECTFILE_ANY
    mode, because when saving a file you may want to give the same name
    even if directory changes.

  - When changing filter, maybe update the extension (if not more than
    one extension given).
    
  - Perhaps ".." should be excluded from SELECTFILE_MULTIPLE_ALL selections.
*/


#define FILELISTMASK (ICONLIST_EXTENDEDSELECT|ICONLIST_SINGLESELECT|ICONLIST_BROWSESELECT|ICONLIST_MULTIPLESELECT)

/*******************************************************************************/

const unsigned char tbuplevel[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x10,0x00,0x10,0x00,0xf1,0x00,0x00,0xb2,0xc0,0xdc,
  0x00,0x00,0x00,0xff,0xff,0x00,0xff,0xff,0xff,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,
  0x10,0x00,0x00,0x02,0x32,0x84,0x8f,0x79,0xc1,0xac,0x18,0xc4,0x90,0x22,0x3c,0xd0,
  0xb2,0x76,0x78,0x7a,0xfa,0x0d,0x56,0xe4,0x05,0x21,0x35,0x96,0xcc,0x29,0x62,0x92,
  0x76,0xa6,0x28,0x08,0x8e,0x35,0x5b,0x75,0x28,0xfc,0xba,0xf8,0x27,0xfb,0xf5,0x36,
  0x44,0xce,0xe5,0x88,0x44,0x14,0x00,0x00,0x3b
  };

const unsigned char tbnewfolder[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x10,0x00,0x10,0x00,0xf1,0x00,0x00,0xb2,0xc0,0xdc,
  0x00,0x00,0x00,0xff,0xff,0x00,0xff,0xff,0xff,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,
  0x10,0x00,0x00,0x02,0x30,0x84,0x8f,0xa9,0x19,0xeb,0xbf,0x1a,0x04,0xf2,0x54,0x1a,
  0xb2,0x69,0x1c,0x05,0x31,0x80,0x52,0x67,0x65,0xe6,0xb9,0x51,0x54,0xc8,0x82,0xe2,
  0xf5,0xb5,0x2d,0xb9,0xba,0xf2,0xb8,0xd9,0xfa,0x55,0xcb,0x22,0xa3,0x9b,0x31,0x4e,
  0x44,0xde,0x24,0x51,0x00,0x00,0x3b
  };

const unsigned char tbbigicons[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x10,0x00,0x10,0x00,0xf1,0x00,0x00,0xb2,0xc0,0xdc,
  0x00,0x00,0x80,0xff,0xff,0xff,0x00,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,
  0x10,0x00,0x00,0x02,0x22,0x84,0x8f,0xa9,0xab,0xe1,0x9c,0x82,0x78,0xd0,0xc8,0x59,
  0xad,0xc0,0xd9,0xd1,0x0c,0x1a,0xc3,0x48,0x1e,0x5e,0x28,0x7d,0xd0,0x15,0x80,0xac,
  0x7b,0x86,0x21,0x59,0xca,0x46,0x01,0x00,0x3b
  };

const unsigned char tbdetails[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x10,0x00,0x10,0x00,0xf1,0x00,0x00,0xb2,0xc0,0xdc,
  0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,
  0x10,0x00,0x00,0x02,0x27,0x84,0x8f,0xa9,0xbb,0xe1,0x01,0x5e,0x74,0xac,0x8a,0x8b,
  0xb3,0x16,0x75,0xf1,0x49,0x49,0x52,0xa7,0x7c,0x0f,0x24,0x52,0x64,0x62,0xa6,0xa8,
  0xba,0x1e,0x6d,0x48,0x43,0xb1,0x6c,0x9c,0xe0,0x7e,0x1b,0x05,0x00,0x3b
  };

const unsigned char tblist[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x10,0x00,0x10,0x00,0xf1,0x00,0x00,0xb2,0xc0,0xdc,
  0x00,0x00,0x80,0xff,0xff,0xff,0x00,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,
  0x10,0x00,0x00,0x02,0x2a,0x84,0x8f,0xa9,0x8b,0x11,0xea,0xa0,0x78,0xa3,0x82,0x30,
  0x41,0x1d,0x37,0x36,0xcf,0x84,0x22,0x03,0x1e,0xa5,0x81,0x51,0x56,0xaa,0xad,0xa7,
  0xf3,0x8c,0xf2,0x7c,0x76,0x92,0xca,0xb1,0x5b,0x17,0x9b,0xf5,0x6c,0x28,0x00,0x00,
  0x3b
  };

const unsigned char home[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x10,0x00,0x10,0x00,0xc2,0x00,0x00,0xb2,0xc0,0xdc,
  0x00,0x00,0x00,0x84,0x00,0x00,0xff,0xff,0x00,0xff,0xff,0xff,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,0x10,0x00,0x00,0x03,
  0x3b,0x08,0xba,0xdc,0xae,0x21,0x28,0x21,0x1f,0x08,0x63,0x48,0x6a,0x71,0xd4,0x9c,
  0xe3,0x11,0x84,0x57,0x31,0x23,0x59,0x6a,0x4d,0xaa,0xae,0xa7,0xfb,0xc2,0x4b,0x30,
  0xcf,0xe7,0xa5,0x46,0xd1,0x8e,0xee,0x19,0x1b,0x29,0x27,0x5c,0x69,0x7c,0x35,0xe0,
  0x71,0xf8,0x1b,0x06,0x91,0x10,0x9e,0x54,0x6a,0xa9,0x02,0x12,0x00,0x3b
  };

const unsigned char fileshown[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x10,0x00,0x10,0x00,0xf2,0x00,0x00,0xb2,0xc0,0xdc,
  0x00,0x00,0x00,0xc0,0xc0,0xff,0xff,0xff,0xff,0xc0,0xc0,0xc0,0xff,0xff,0xc0,0xff,
  0x80,0x00,0x00,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,0x10,0x00,0x00,0x03,
  0x43,0x08,0xaa,0xd1,0xfb,0x30,0x08,0x19,0xe0,0x6a,0x4d,0x68,0x5c,0xef,0xf8,0x03,
  0x46,0x80,0x1d,0x10,0x80,0x28,0x5a,0x9e,0x5a,0xeb,0x86,0xa6,0x99,0xce,0x4d,0xd1,
  0x0c,0x6e,0x1e,0x16,0xc6,0x3d,0x93,0xbc,0x0a,0x2b,0xb7,0x09,0x32,0x7e,0xbb,0x9e,
  0x87,0x18,0x08,0xae,0x68,0x84,0x1e,0xe1,0x99,0xc3,0x08,0x60,0x47,0xe4,0xa7,0x64,
  0xe2,0x78,0xb9,0x09,0x00,0x3b
  };

const unsigned char filehidden[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x10,0x00,0x10,0x00,0xf1,0x00,0x00,0xb2,0xc0,0xdc,
  0xa0,0xa0,0xa4,0xc0,0xc0,0xff,0xff,0xff,0xff,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,
  0x10,0x00,0x00,0x02,0x2c,0x84,0x1d,0x79,0xcb,0x21,0x1e,0xd8,0x49,0x49,0x58,0x1a,
  0xe7,0xd8,0x83,0xf2,0xae,0x7d,0x22,0x88,0x8c,0x62,0x06,0x04,0xe6,0x87,0xaa,0xeb,
  0xd6,0xbe,0x70,0xf8,0xc6,0xb2,0x5d,0xd3,0x2b,0xbe,0xeb,0x26,0x9a,0xc2,0x08,0x81,
  0x05,0x00,0x3b
  };

const unsigned char mark[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x10,0x00,0x10,0x00,0xf2,0x00,0x00,0xb2,0xc0,0xdc,
  0x00,0x00,0x00,0xff,0x00,0x00,0x00,0x00,0x80,0xa0,0x8c,0x68,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,0x10,0x00,0x00,0x03,
  0x2f,0x08,0xba,0xac,0xd1,0x70,0xbd,0x08,0xc3,0xa4,0x52,0x88,0x4b,0x83,0xde,0x18,
  0xe0,0x69,0x03,0xd7,0x58,0xe4,0x50,0x9a,0xa2,0xa3,0xaa,0x1a,0x77,0x05,0xef,0x37,
  0x33,0xf4,0x60,0x4b,0xd5,0x78,0x47,0x04,0x0b,0x2f,0x44,0x2c,0x1a,0x8f,0x8a,0x04,
  0x00,0x3b
  };

const unsigned char clear[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x10,0x00,0x10,0x00,0xf2,0x00,0x00,0xb2,0xc0,0xdc,
  0x00,0x00,0xff,0xad,0xd8,0xe6,0x00,0x00,0x00,0xff,0x00,0x00,0x00,0x00,0x80,0xa0,
  0x8c,0x68,0x00,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,0x10,0x00,0x00,0x03,
  0x43,0x08,0xba,0x10,0x21,0x2c,0x2e,0x07,0xa5,0x0b,0x8a,0xca,0x25,0xee,0x63,0x83,
  0xd4,0x69,0x4a,0x28,0x7a,0xa5,0x19,0x69,0xce,0x40,0x10,0x6a,0xf6,0xc9,0x6f,0xdc,
  0xcc,0x6e,0x40,0x14,0x36,0xee,0xee,0x05,0x9e,0x4a,0x00,0x31,0x0d,0x82,0xc1,0x9a,
  0xa2,0xb8,0x38,0x26,0x95,0x25,0xd0,0x13,0xd6,0x94,0x0c,0x7e,0x2a,0xdb,0xc2,0x70,
  0xdd,0x78,0x23,0x09,0x00,0x3b
  };

const unsigned char work[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x10,0x00,0x10,0x00,0xf1,0x00,0x00,0xb2,0xc0,0xdc,
  0x00,0x00,0x00,0x80,0x80,0x00,0x80,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,
  0x10,0x00,0x00,0x02,0x28,0x84,0x8f,0xa9,0x10,0x1d,0x8b,0x9a,0x10,0x0e,0x1e,0x39,
  0x9f,0x65,0x93,0x6e,0x16,0x64,0xda,0x12,0x0c,0x4e,0x46,0x9a,0x46,0x38,0x46,0xea,
  0x95,0xb6,0x50,0x29,0xc7,0x1f,0xf3,0x7e,0xcd,0xcd,0xf7,0x4a,0x01,0x00,0x3b
  };


const unsigned char dlgnewfolder[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x20,0x00,0x20,0x00,0xf2,0x00,0x00,0xb2,0xc0,0xdc,
  0x80,0x80,0x80,0xff,0xff,0xff,0xff,0xff,0x00,0xc0,0xc0,0xc0,0x00,0x00,0x00,0x80,
  0x80,0x00,0x00,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x20,0x00,0x20,0x00,0x00,0x03,
  0x83,0x08,0xba,0xdc,0xfe,0x30,0xca,0x49,0x6b,0x0c,0x38,0x67,0x0b,0x83,0xf8,0x20,
  0x18,0x70,0x8d,0x37,0x10,0x67,0x8a,0x12,0x23,0x09,0x98,0xab,0xaa,0xb6,0x56,0x40,
  0xdc,0x78,0xae,0x6b,0x3c,0x5f,0xbc,0xa1,0xa0,0x70,0x38,0x2c,0x14,0x60,0xb2,0x98,
  0x32,0x99,0x34,0x1c,0x05,0xcb,0x28,0x53,0xea,0x44,0x4a,0xaf,0xd3,0x2a,0x74,0xca,
  0xc5,0x6a,0xbb,0xe0,0xa8,0x16,0x4b,0x66,0x7e,0xcb,0xe8,0xd3,0x38,0xcc,0x46,0x9d,
  0xdb,0xe1,0x75,0xba,0xfc,0x9e,0x77,0xe5,0x70,0xef,0x33,0x1f,0x7f,0xda,0xe9,0x7b,
  0x7f,0x77,0x7e,0x7c,0x7a,0x56,0x85,0x4d,0x84,0x82,0x54,0x81,0x88,0x62,0x47,0x06,
  0x91,0x92,0x93,0x94,0x95,0x96,0x91,0x3f,0x46,0x9a,0x9b,0x9c,0x9d,0x9e,0x9a,0x2e,
  0xa1,0xa2,0x13,0x09,0x00,0x3b
  };

const unsigned char deletefile[]={
  0x42,0x4d,0x7e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3e,0x00,0x00,0x00,0x28,0x00,
  0x00,0x00,0x10,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x01,0x00,0x01,0x00,0x00,0x00,
  0x00,0x00,0x40,0x00,0x00,0x00,0x6d,0x0b,0x00,0x00,0x6d,0x0b,0x00,0x00,0x02,0x00,
  0x00,0x00,0x02,0x00,0x00,0x00,0xdc,0xc0,0xb2,0x00,0x00,0x00,0x84,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x04,0x00,0x00,0x70,0x00,0x00,0x00,0x78,0x08,
  0x00,0x00,0x38,0x10,0x00,0x00,0x1c,0x30,0x00,0x00,0x0e,0x60,0x00,0x00,0x07,0xc0,
  0x00,0x00,0x03,0x80,0x00,0x00,0x07,0xc0,0x00,0x00,0x0e,0x60,0x00,0x00,0x3c,0x30,
  0x00,0x00,0x78,0x18,0x00,0x00,0x70,0x04,0x00,0x00,0x00,0x00,0x00,0x00
  };


const unsigned char copyfile[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x10,0x00,0x10,0x00,0xa1,0x00,0x00,0xb2,0xc0,0xdc,
  0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xcb,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,
  0x10,0x00,0x00,0x02,0x3c,0x84,0x8f,0x09,0xc1,0xad,0x18,0x84,0x14,0xec,0x99,0x38,
  0x63,0xb0,0xba,0x57,0x85,0x4d,0xd2,0x06,0x52,0x8c,0xd9,0x90,0x97,0x48,0x0d,0xee,
  0xf0,0x2d,0x68,0xfb,0x06,0xf0,0x2a,0xda,0x0c,0xec,0xa4,0xcd,0x0b,0x54,0x41,0x78,
  0x3f,0xe1,0xc1,0x06,0x74,0x19,0x2f,0xc4,0xdd,0x72,0x91,0x54,0x3e,0x7c,0x29,0x40,
  0x01,0x00,0x3b
  };


const unsigned char movefile[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x10,0x00,0x10,0x00,0xc2,0x00,0x00,0xb2,0xc0,0xdc,
  0x7f,0x7f,0x65,0xb3,0xb3,0x8e,0x00,0x00,0x00,0xff,0xff,0xcb,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,0x10,0x00,0x00,0x03,
  0x3e,0x08,0xba,0xdc,0x10,0x30,0x3a,0x16,0x84,0x15,0x70,0xaa,0x7a,0x6b,0xd0,0x5e,
  0x98,0x39,0xdc,0x65,0x7d,0x24,0x06,0x09,0x43,0xeb,0x52,0x26,0x4b,0xcc,0x44,0xbb,
  0x88,0x32,0x3d,0xd4,0x5b,0xbc,0xb7,0xb5,0x57,0x24,0xb2,0xa3,0xe9,0x26,0x3f,0xd7,
  0x0f,0x69,0x3c,0x3a,0x92,0x2e,0xde,0xb3,0x39,0x1b,0x20,0x95,0x4a,0x40,0x02,0x00,
  0x3b
  };

const unsigned char linkfile[]={
  0x47,0x49,0x46,0x38,0x37,0x61,0x10,0x00,0x10,0x00,0xa1,0x00,0x00,0xb2,0xc0,0xdc,
  0x00,0x00,0x00,0x00,0xff,0xff,0x00,0x00,0x00,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,
  0x10,0x00,0x00,0x02,0x20,0x84,0x8f,0xa9,0xcb,0xed,0x0f,0x4f,0x98,0x33,0xd1,0x00,
  0x82,0xd8,0xfb,0x6a,0x3e,0x09,0x1e,0x48,0x89,0x1f,0x59,0x82,0xd9,0x65,0x51,0xd1,
  0x0b,0xc7,0xf2,0x8c,0x14,0x00,0x3b
  };

/*******************************************************************************/


// Map
FXDEFMAP(FXFileSelector) FXFileSelectorMap[]={
  FXMAPFUNC(SEL_COMMAND,FXFileSelector::ID_ACCEPT,FXFileSelector::onCmdAccept),
  FXMAPFUNC(SEL_COMMAND,FXFileSelector::ID_FILEFILTER,FXFileSelector::onCmdFilter),
  FXMAPFUNC(SEL_DOUBLECLICKED,FXFileSelector::ID_FILELIST,FXFileSelector::onCmdItemDblClicked),
  FXMAPFUNC(SEL_SELECTED,FXFileSelector::ID_FILELIST,FXFileSelector::onCmdItemSelected),
  FXMAPFUNC(SEL_DESELECTED,FXFileSelector::ID_FILELIST,FXFileSelector::onCmdItemDeselected),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,FXFileSelector::ID_FILELIST,FXFileSelector::onPopupMenu),
  FXMAPFUNC(SEL_COMMAND,FXFileSelector::ID_DIRECTORY_UP,FXFileSelector::onCmdDirectoryUp),
  FXMAPFUNC(SEL_UPDATE,FXFileSelector::ID_DIRECTORY_UP,FXFileSelector::onUpdDirectoryUp),
  FXMAPFUNC(SEL_COMMAND,FXFileSelector::ID_DIRTREE,FXFileSelector::onCmdDirTree),
  FXMAPFUNC(SEL_COMMAND,FXFileSelector::ID_HOME,FXFileSelector::onCmdHome),
  FXMAPFUNC(SEL_COMMAND,FXFileSelector::ID_WORK,FXFileSelector::onCmdWork),
  FXMAPFUNC(SEL_COMMAND,FXFileSelector::ID_VISIT,FXFileSelector::onCmdVisit),
  FXMAPFUNC(SEL_COMMAND,FXFileSelector::ID_BOOKMARK,FXFileSelector::onCmdBookmark),
  FXMAPFUNC(SEL_COMMAND,FXFileSelector::ID_NEW,FXFileSelector::onCmdNew),
  FXMAPFUNC(SEL_UPDATE,FXFileSelector::ID_NEW,FXFileSelector::onUpdNew),
  FXMAPFUNC(SEL_COMMAND,FXFileSelector::ID_DELETE,FXFileSelector::onCmdDelete),
  FXMAPFUNC(SEL_COMMAND,FXFileSelector::ID_MOVE,FXFileSelector::onCmdMove),
  FXMAPFUNC(SEL_COMMAND,FXFileSelector::ID_COPY,FXFileSelector::onCmdCopy),
  FXMAPFUNC(SEL_COMMAND,FXFileSelector::ID_LINK,FXFileSelector::onCmdLink),
  FXMAPFUNC(SEL_UPDATE,FXFileSelector::ID_COPY,FXFileSelector::onUpdSelected),
  FXMAPFUNC(SEL_UPDATE,FXFileSelector::ID_MOVE,FXFileSelector::onUpdSelected),
  FXMAPFUNC(SEL_UPDATE,FXFileSelector::ID_LINK,FXFileSelector::onUpdSelected),
  FXMAPFUNC(SEL_UPDATE,FXFileSelector::ID_DELETE,FXFileSelector::onUpdSelected),
  };


// Implementation
FXIMPLEMENT(FXFileSelector,FXPacker,FXFileSelectorMap,ARRAYNUMBER(FXFileSelectorMap))


// Default pattern
static const FXchar allfiles[]="All Files (*)";


/*******************************************************************************/

// Separator item
FXFileSelector::FXFileSelector(FXComposite *p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXPacker(p,opts,x,y,w,h),
  mrufiles("Visited Directories"){
  FXAccelTable *table;
  target=tgt;
  message=sel;
  FXHorizontalFrame *buttons=new FXHorizontalFrame(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X,0,0,0,0, DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING,DEFAULT_SPACING, 0,0);
  FXMatrix *fields=new FXMatrix(this,3,MATRIX_BY_COLUMNS|LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X);
  new FXLabel(fields,"&File Name:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y);
  filename=new FXTextField(fields,25,this,ID_ACCEPT,TEXTFIELD_ENTER_ONLY|LAYOUT_FILL_COLUMN|LAYOUT_FILL_X|FRAME_SUNKEN|FRAME_THICK);
  new FXButton(fields,"&OK",NULL,this,ID_ACCEPT,BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_X,0,0,0,0,20,20);
  accept=new FXButton(buttons,NULL,NULL,NULL,0,LAYOUT_FIX_X|LAYOUT_FIX_Y|LAYOUT_FIX_WIDTH|LAYOUT_FIX_HEIGHT,0,0,0,0, 0,0,0,0);
  new FXLabel(fields,"File F&ilter:",NULL,JUSTIFY_LEFT|LAYOUT_CENTER_Y);
  FXHorizontalFrame *filterframe=new FXHorizontalFrame(fields,LAYOUT_FILL_COLUMN|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);
  filefilter=new FXComboBox(filterframe,10,4,this,ID_FILEFILTER,COMBOBOX_STATIC|LAYOUT_FILL_X|FRAME_SUNKEN|FRAME_THICK);
  readonly=new FXCheckButton(filterframe,"Read Only",NULL,0,ICON_BEFORE_TEXT|JUSTIFY_LEFT|LAYOUT_CENTER_Y);
  cancel=new FXButton(fields,"&Cancel",NULL,NULL,0,BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_FILL_X,0,0,0,0,20,20);
  FXHorizontalFrame *frame=new FXHorizontalFrame(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0,0,0,0,0);
  filebox=new FXFileList(frame,this,ID_FILELIST,ICONLIST_MINI_ICONS|ICONLIST_BROWSESELECT|ICONLIST_AUTOSIZE|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  new FXLabel(buttons,"Directory:",NULL,LAYOUT_CENTER_Y);
  updiricon=new FXGIFIcon(getApp(),tbuplevel);
  newdiricon=new FXGIFIcon(getApp(),tbnewfolder);
  listicon=new FXGIFIcon(getApp(),tblist);
  iconsicon=new FXGIFIcon(getApp(),tbbigicons);
  detailicon=new FXGIFIcon(getApp(),tbdetails);
  homeicon=new FXGIFIcon(getApp(),home);
  workicon=new FXGIFIcon(getApp(),work);
  shownicon=new FXGIFIcon(getApp(),fileshown);
  hiddenicon=new FXGIFIcon(getApp(),filehidden);
  markicon=new FXGIFIcon(getApp(),mark);
  clearicon=new FXGIFIcon(getApp(),clear);
  deleteicon=new FXBMPIcon(getApp(),deletefile,0,IMAGE_ALPHAGUESS);
  moveicon=new FXGIFIcon(getApp(),movefile);
  copyicon=new FXGIFIcon(getApp(),copyfile);
  linkicon=new FXGIFIcon(getApp(),linkfile);
  dirbox=new FXDirBox(buttons,5,this,ID_DIRTREE,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FIX_WIDTH|LAYOUT_CENTER_Y,0,0,160,0);
  bookmarks=new FXMenuPane(this,POPUP_SHRINKWRAP);
  new FXMenuCommand(bookmarks,"&Set bookmark\t\tBookmark current directory.",markicon,this,ID_BOOKMARK);
  new FXMenuCommand(bookmarks,"&Clear bookmarks\t\tClear bookmarks.",clearicon,&mrufiles,FXRecentFiles::ID_CLEAR);
  FXMenuSeparator* sep1=new FXMenuSeparator(bookmarks);
  sep1->setTarget(&mrufiles);
  sep1->setSelector(FXRecentFiles::ID_ANYFILES);
  new FXMenuCommand(bookmarks,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_1);
  new FXMenuCommand(bookmarks,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_2);
  new FXMenuCommand(bookmarks,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_3);
  new FXMenuCommand(bookmarks,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_4);
  new FXMenuCommand(bookmarks,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_5);
  new FXMenuCommand(bookmarks,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_6);
  new FXMenuCommand(bookmarks,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_7);
  new FXMenuCommand(bookmarks,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_8);
  new FXMenuCommand(bookmarks,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_9);
  new FXMenuCommand(bookmarks,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_10);
  new FXFrame(buttons,LAYOUT_FIX_WIDTH,0,0,4,1);
  new FXButton(buttons,"\tGo up one directory\tMove up to higher directory.",updiricon,this,ID_DIRECTORY_UP,BUTTON_TOOLBAR|FRAME_RAISED,0,0,0,0, 3,3,3,3);
  new FXButton(buttons,"\tGo to home directory\tBack to home directory.",homeicon,this,ID_HOME,BUTTON_TOOLBAR|FRAME_RAISED,0,0,0,0, 3,3,3,3);
  new FXButton(buttons,"\tGo to work directory\tBack to working directory.",workicon,this,ID_WORK,BUTTON_TOOLBAR|FRAME_RAISED,0,0,0,0, 3,3,3,3);
  new FXMenuButton(buttons,"\tBookmarks\tVisit bookmarked directories.",markicon,bookmarks,MENUBUTTON_NOARROWS|MENUBUTTON_ATTACH_LEFT|MENUBUTTON_TOOLBAR|FRAME_RAISED,0,0,0,0, 3,3,3,3);
  new FXButton(buttons,"\tCreate new directory\tCreate new directory.",newdiricon,this,ID_NEW,BUTTON_TOOLBAR|FRAME_RAISED,0,0,0,0, 3,3,3,3);
  new FXButton(buttons,"\tShow list\tDisplay directory with small icons.",listicon,filebox,FXFileList::ID_SHOW_MINI_ICONS,BUTTON_TOOLBAR|FRAME_RAISED,0,0,0,0, 3,3,3,3);
  new FXButton(buttons,"\tShow icons\tDisplay directory with big icons.",iconsicon,filebox,FXFileList::ID_SHOW_BIG_ICONS,BUTTON_TOOLBAR|FRAME_RAISED,0,0,0,0, 3,3,3,3);
  new FXButton(buttons,"\tShow details\tDisplay detailed directory listing.",detailicon,filebox,FXFileList::ID_SHOW_DETAILS,BUTTON_TOOLBAR|FRAME_RAISED,0,0,0,0, 3,3,3,3);
  new FXToggleButton(buttons,"\tShow hidden files\tShow hidden files and directories.","\tHide Hidden Files\tHide hidden files and directories.",hiddenicon,shownicon,filebox,FXFileList::ID_TOGGLE_HIDDEN,TOGGLEBUTTON_TOOLBAR|FRAME_RAISED,0,0,0,0, 3,3,3,3);
  mrufiles.setTarget(this);
  mrufiles.setSelector(ID_VISIT);
  table=getShell()->getAccelTable();
  readonly->hide();
  if(table){
    table->addAccel(MKUINT(KEY_BackSpace,0),this,MKUINT(ID_DIRECTORY_UP,SEL_COMMAND));
    table->addAccel(MKUINT(KEY_h,CONTROLMASK),this,MKUINT(ID_HOME,SEL_COMMAND));
    table->addAccel(MKUINT(KEY_w,CONTROLMASK),this,MKUINT(ID_WORK,SEL_COMMAND));
    table->addAccel(MKUINT(KEY_n,CONTROLMASK),this,MKUINT(ID_NEW,SEL_COMMAND));
    table->addAccel(MKUINT(KEY_b,CONTROLMASK),filebox,MKUINT(FXFileList::ID_SHOW_BIG_ICONS,SEL_COMMAND));
    table->addAccel(MKUINT(KEY_s,CONTROLMASK),filebox,MKUINT(FXFileList::ID_SHOW_MINI_ICONS,SEL_COMMAND));
    table->addAccel(MKUINT(KEY_l,CONTROLMASK),filebox,MKUINT(FXFileList::ID_SHOW_DETAILS,SEL_COMMAND));
    }
  setSelectMode(SELECTFILE_ANY);    // For backward compatibility, this HAS to be the default!
  setPatternList(allfiles);
  setDirectory(FXFile::getCurrentDirectory());
  filebox->setFocus();
  }


// Double-clicked item in file list
long FXFileSelector::onCmdItemDblClicked(FXObject*,FXSelector,void* ptr){
  FXint index=(FXint)(long)ptr;

  FXTRACE((1,"FXFileSelector::onCmdItemDblClicked\n"));

  if(index<0) return 1;

  // If directory, open the directory
  if(filebox->isItemDirectory(index)){
    setDirectory(filebox->getItemPathname(index));
    return 1;
    }

  // Only return if we wanted a file
  if(selectmode!=SELECTFILE_DIRECTORY){
    if(filebox->isItemFile(index)){
      FXObject *tgt=accept->getTarget();
      FXSelector sel=accept->getSelector();
      if(tgt) tgt->handle(accept,MKUINT(sel,SEL_COMMAND),(void*)1);
      }
    }
  return 1;
  }


// Change in items which are selected
long FXFileSelector::onCmdItemSelected(FXObject*,FXSelector,void* ptr){
  FXint index=(FXint)(long)ptr;
  FXString text,file;
  if(selectmode==SELECTFILE_MULTIPLE){
    for(FXint i=0; i<filebox->getNumItems(); i++){
      if(filebox->isItemFile(i) && filebox->isItemSelected(i)){
        if(!text.empty()) text+=' ';
        text+="\""+filebox->getItemFilename(i)+"\"";
        }
      }
    filename->setText(text);
    }
  else if(selectmode==SELECTFILE_MULTIPLE_ALL){
    for(FXint i=0; i<filebox->getNumItems(); i++){
      if(filebox->isItemSelected(i) && filebox->getItemFilename(i)!=".."){
        if(!text.empty()) text+=' ';
        text+="\""+filebox->getItemFilename(i)+"\"";
        }
      }
    filename->setText(text);
    }
  else if(selectmode==SELECTFILE_DIRECTORY){
    if(filebox->isItemDirectory(index)){
      text=filebox->getItemFilename(index);
      filename->setText(text);
      }
    }
  else{
    if(filebox->isItemFile(index)){
      text=filebox->getItemFilename(index);
      filename->setText(text);
      }
    }
  return 1;
  }


// Change in items which are selected
long FXFileSelector::onCmdItemDeselected(FXObject*,FXSelector,void*){
  FXString text,file;
  if(selectmode==SELECTFILE_MULTIPLE){
    for(FXint i=0; i<filebox->getNumItems(); i++){
      if(filebox->isItemFile(i) && filebox->isItemSelected(i)){
        if(!text.empty()) text+=' ';
        text+="\""+filebox->getItemFilename(i)+"\"";
        }
      }
    filename->setText(text);
    }
  else if(selectmode==SELECTFILE_MULTIPLE_ALL){
    for(FXint i=0; i<filebox->getNumItems(); i++){
      if(filebox->isItemSelected(i) && filebox->getItemFilename(i)!=".."){
        if(!text.empty()) text+=' ';
        text+="\""+filebox->getItemFilename(i)+"\"";
        }
      }
    filename->setText(text);
    }
  return 1;
  }


// Hit the accept button or enter in text field
long FXFileSelector::onCmdAccept(FXObject*,FXSelector,void*){
  FXSelector sel=accept->getSelector();
  FXObject *tgt=accept->getTarget();

  // Get (first) filename
  FXString path=getFilename();

//FXString *list;
//FXint     count,i;
//count=FXFile::listFiles(list,path,"*",LIST_ALL_FILES|LIST_ALL_DIRS);
//for(i=0; i<count; i++){ FXTRACE((1,"list[%d]=%s\n",i,list[i].text())); }
//delete [] list;
//return 1;

  // Is directory?
  if(FXFile::isDirectory(path)){

    // In directory mode:- we got our answer!
    if(selectmode==SELECTFILE_DIRECTORY){
      if(tgt) tgt->handle(accept,MKUINT(sel,SEL_COMMAND),(void*)1);
      return 1;
      }

    // Hop over to that directory
    dirbox->setDirectory(path);
    filebox->setDirectory(path);
    filename->setText(FXString::null);
    return 1;
    }

  // Get directory part of path
  FXString dir=FXFile::directory(path);

  // In file mode, directory part of path should exist
  if(FXFile::isDirectory(dir)){

    // In any mode, existing directory part is good enough
    if(selectmode==SELECTFILE_ANY){
      if(tgt) tgt->handle(accept,MKUINT(sel,SEL_COMMAND),(void*)1);
      return 1;
      }

    // In existing mode, the whole filename must exist and be a file
    else if(selectmode==SELECTFILE_EXISTING){
      if(FXFile::isFile(path)){
        if(tgt) tgt->handle(accept,MKUINT(sel,SEL_COMMAND),(void*)1);
        return 1;
        }
      }

    // In multiple mode, return if all selected files exist
    else if(selectmode==SELECTFILE_MULTIPLE){
      for(FXint i=0; i<filebox->getNumItems(); i++){
        if(filebox->isItemSelected(i) && filebox->isItemFile(i)){
          if(tgt) tgt->handle(accept,MKUINT(sel,SEL_COMMAND),(void*)1);
          return 1;
          }
        }
      }
      
    // Multiple files and/or directories
    else{
      for(FXint i=0; i<filebox->getNumItems(); i++){
        if(filebox->isItemSelected(i) && filebox->getItemFilename(i)!=".."){
          if(tgt) tgt->handle(accept,MKUINT(sel,SEL_COMMAND),(void*)1);
          return 1;
          }
        }
      }
    }

  // Go up to the lowest directory which still exists
  while(!FXFile::isTopDirectory(dir) && !FXFile::isDirectory(dir)){
    dir=FXFile::upLevel(dir);
    }

  // Switch as far as we could go
  dirbox->setDirectory(dir);
  filebox->setDirectory(dir);

  // Put the tail end back for further editing
  FXASSERT(dir.length()<=path.length());
  if(ISPATHSEP(path[dir.length()]))
    path.remove(0,dir.length()+1);
  else
    path.remove(0,dir.length());

  // Replace text box with new stuff
  filename->setText(path);
  filename->selectAll();

  // Beep
  getApp()->beep();
  return 1;
  }


// User clicked up directory button
long FXFileSelector::onCmdDirectoryUp(FXObject*,FXSelector,void*){
  setDirectory(FXFile::upLevel(filebox->getDirectory()));
  return 1;
  }


// Can we still go up
long FXFileSelector::onUpdDirectoryUp(FXObject* sender,FXSelector,void*){
  if(FXFile::isTopDirectory(filebox->getDirectory())){
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
    }
  else{
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
    }
  return 1;
  }


// Back to home directory
long FXFileSelector::onCmdHome(FXObject*,FXSelector,void*){
  setDirectory(FXFile::getHomeDirectory());
  return 1;
  }


// Back to current working directory
long FXFileSelector::onCmdWork(FXObject*,FXSelector,void*){
  setDirectory(FXFile::getCurrentDirectory());
  return 1;
  }


// Move to recent directory
long FXFileSelector::onCmdVisit(FXObject*,FXSelector,void* ptr){
  setDirectory((FXchar*)ptr);
  return 1;
  }


// Bookmark this directory
long FXFileSelector::onCmdBookmark(FXObject*,FXSelector,void*){
  mrufiles.appendFile(filebox->getDirectory());
  return 1;
  }


// Switched directories using directory tree
long FXFileSelector::onCmdDirTree(FXObject*,FXSelector,void* ptr){
  filebox->setDirectory((FXchar*)ptr);
  if(selectmode==SELECTFILE_DIRECTORY){
    filename->setText(FXString::null);
    }
  return 1;
  }


// Create new directory
long FXFileSelector::onCmdNew(FXObject*,FXSelector,void*){
  FXString dir=filebox->getDirectory();
  FXString name="DirectoryName";
  FXGIFIcon newdiricon(getApp(),dlgnewfolder);
  if(FXInputDialog::getString(name,this,"Create New Directory","Create new directory in: "+dir,&newdiricon)){
    FXString dirname=FXFile::absolute(dir,name);
    if(FXFile::exists(dirname)){
      FXMessageBox::error(this,MBOX_OK,"Already Exists","File or directory %s already exists.\n",dirname.text());
      }
    else if(!FXFile::createDirectory(dirname,0777)){
      FXMessageBox::error(this,MBOX_OK,"Cannot Create","Cannot create directory %s.\n",dirname.text());
      }
    }
  return 1;
  }


// Update create new directory
long FXFileSelector::onUpdNew(FXObject* sender,FXSelector,void*){
  FXString dir=filebox->getDirectory();
  if(FXFile::isWritable(dir))
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Copy file or directory
long FXFileSelector::onCmdCopy(FXObject*,FXSelector,void*){
  FXString dir=filebox->getDirectory();
  FXString newname,oldname,name;
  for(FXint i=0; i<filebox->getNumItems(); i++){
    if(filebox->isItemSelected(i)){
      name=filebox->getItemFilename(i);
      if(name=="..") continue;
      oldname=FXFile::absolute(dir,name);
      newname=FXFile::absolute(dir,"CopyOf"+name);
      //FXGIFIcon copybigicon(getApp(),copyfilebig);
      FXInputDialog inputdialog(this,"Copy File","Copy file from location:\n\n"+oldname+"\n\nto location:",NULL,INPUTDIALOG_STRING,0,0,0,0);
      inputdialog.setText(newname);
      inputdialog.setNumColumns(60);
      if(inputdialog.execute()){
        newname=inputdialog.getText();
        if(!FXFile::copy(oldname,newname,FALSE)){
          if(MBOX_CLICKED_NO==FXMessageBox::error(this,MBOX_YES_NO,"Error Copying File","Unable to copy file:\n\n%s  to:  %s\n\nContinue with operation?",oldname.text(),newname.text())){
            break;
            }
          }
        }
      }
    }
  return 1;
  }


// Move file or directory
long FXFileSelector::onCmdMove(FXObject*,FXSelector,void*){
  FXString dir=filebox->getDirectory();
  FXString newname,oldname,name;
  for(FXint i=0; i<filebox->getNumItems(); i++){
    if(filebox->isItemSelected(i)){
      name=filebox->getItemFilename(i);
      if(name=="..") continue;
      oldname=FXFile::absolute(dir,name);
      newname=FXFile::absolute(dir,name);
      //FXGIFIcon movebigicon(getApp(),movefilebig);
      FXInputDialog inputdialog(this,"Move File","Move file from location:\n\n"+oldname+"\n\nto location:",NULL,INPUTDIALOG_STRING,0,0,0,0);
      inputdialog.setText(newname);
      inputdialog.setNumColumns(60);
      if(inputdialog.execute()){
        newname=inputdialog.getText();
        if(!FXFile::move(oldname,newname,FALSE)){
          if(MBOX_CLICKED_NO==FXMessageBox::error(this,MBOX_YES_NO,"Error Moving File","Unable to move file:\n\n%s  to:  %s\n\nContinue with operation?",oldname.text(),newname.text())){
            break;
            }
          }
        }
      }
    }
  return 1;
  }


// Link file or directory
long FXFileSelector::onCmdLink(FXObject*,FXSelector,void*){
  FXString dir=filebox->getDirectory();
  FXString newname,oldname,name;
  for(FXint i=0; i<filebox->getNumItems(); i++){
    if(filebox->isItemSelected(i)){
      name=filebox->getItemFilename(i);
      if(name=="..") continue;
      oldname=FXFile::absolute(dir,name);
      newname=FXFile::absolute(dir,"LinkTo"+name);
      //FXGIFIcon copybigicon(getApp(),linkfilebig);
      FXInputDialog inputdialog(this,"Link File","Link file from location:\n\n"+oldname+"\n\nto location:",NULL,INPUTDIALOG_STRING,0,0,0,0);
      inputdialog.setText(newname);
      inputdialog.setNumColumns(60);
      if(inputdialog.execute()){
        newname=inputdialog.getText();
        if(!FXFile::link(oldname,newname,FALSE)){
          if(MBOX_CLICKED_NO==FXMessageBox::error(this,MBOX_YES_NO,"Error Linking File","Unable to link file:\n\n%s  to:  %s\n\nContinue with operation?",oldname.text(),newname.text())){
            break;
            }
          }
        }
      }
    }
  return 1;
  }


// Delete file or directory
long FXFileSelector::onCmdDelete(FXObject*,FXSelector,void*){
  FXString dir=filebox->getDirectory();
  FXString fullname,name;
  FXuint answer;
  for(FXint i=0; i<filebox->getNumItems(); i++){
    if(filebox->isItemSelected(i)){
      name=filebox->getItemFilename(i);
      if(name=="..") continue;
      fullname=FXFile::absolute(dir,name);
      answer=FXMessageBox::warning(this,MBOX_YES_NO_CANCEL,"Deleting files","Are you sure you want to delete the file:\n\n%s",fullname.text());
      if(answer==MBOX_CLICKED_CANCEL) break;
      if(answer==MBOX_CLICKED_NO) continue;
      if(!FXFile::remove(fullname)){
        if(MBOX_CLICKED_NO==FXMessageBox::error(this,MBOX_YES_NO,"Error Deleting File","Unable to delete file:\n\n%s\n\nContinue with operation?",fullname.text())){
          break;
          }
        }
      }
    }
  return 1;
  }


// Sensitize when files are selected
long FXFileSelector::onUpdSelected(FXObject* sender,FXSelector,void*){
  for(FXint i=0; i<filebox->getNumItems(); i++){
    if(filebox->isItemSelected(i)){
      sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
      return 1;
      }
    }
  sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Popup menu for item in file list
long FXFileSelector::onPopupMenu(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  if(event->moved) return 1;

  FXMenuPane filemenu(this);
  new FXMenuCommand(&filemenu,"Up one level",updiricon,this,ID_DIRECTORY_UP);
  new FXMenuCommand(&filemenu,"Home directory",homeicon,this,ID_HOME);
  new FXMenuCommand(&filemenu,"Work directory",workicon,this,ID_WORK);
  new FXMenuSeparator(&filemenu);

  FXMenuPane sortmenu(this);
  new FXMenuCascade(&filemenu,"Sort by",NULL,&sortmenu);
  new FXMenuCommand(&sortmenu,"Name",NULL,filebox,FXFileList::ID_SORT_BY_NAME);
  new FXMenuCommand(&sortmenu,"Type",NULL,filebox,FXFileList::ID_SORT_BY_TYPE);
  new FXMenuCommand(&sortmenu,"Size",NULL,filebox,FXFileList::ID_SORT_BY_SIZE);
  new FXMenuCommand(&sortmenu,"Time",NULL,filebox,FXFileList::ID_SORT_BY_TIME);
  new FXMenuCommand(&sortmenu,"User",NULL,filebox,FXFileList::ID_SORT_BY_USER);
  new FXMenuCommand(&sortmenu,"Group",NULL,filebox,FXFileList::ID_SORT_BY_GROUP);
  new FXMenuCommand(&sortmenu,"Reverse",NULL,filebox,FXFileList::ID_SORT_REVERSE);

  FXMenuPane viewmenu(this);
  new FXMenuCascade(&filemenu,"View",NULL,&viewmenu);
  new FXMenuCommand(&viewmenu,"Small icons",NULL,filebox,FXFileList::ID_SHOW_MINI_ICONS);
  new FXMenuCommand(&viewmenu,"Big icons",NULL,filebox,FXFileList::ID_SHOW_BIG_ICONS);
  new FXMenuCommand(&viewmenu,"Details",NULL,filebox,FXFileList::ID_SHOW_DETAILS);
  new FXMenuCommand(&viewmenu,"Rows",NULL,filebox,FXFileList::ID_ARRANGE_BY_ROWS);
  new FXMenuCommand(&viewmenu,"Columns",NULL,filebox,FXFileList::ID_ARRANGE_BY_COLUMNS);
  new FXMenuCommand(&viewmenu,"Hidden files",NULL,filebox,FXFileList::ID_TOGGLE_HIDDEN);

  FXMenuPane bookmarks(this);
  new FXMenuCascade(&filemenu,"Bookmarks",NULL,&bookmarks);
  new FXMenuCommand(&bookmarks,"Set bookmark",markicon,this,ID_BOOKMARK);
  new FXMenuCommand(&bookmarks,"Clear bookmarks",clearicon,&mrufiles,FXRecentFiles::ID_CLEAR);
  new FXMenuSeparator(&bookmarks);
  new FXMenuCommand(&bookmarks,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_1);
  new FXMenuCommand(&bookmarks,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_2);
  new FXMenuCommand(&bookmarks,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_3);
  new FXMenuCommand(&bookmarks,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_4);
  new FXMenuCommand(&bookmarks,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_5);
  new FXMenuCommand(&bookmarks,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_6);
  new FXMenuCommand(&bookmarks,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_7);
  new FXMenuCommand(&bookmarks,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_8);
  new FXMenuCommand(&bookmarks,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_9);
  new FXMenuCommand(&bookmarks,NULL,NULL,&mrufiles,FXRecentFiles::ID_FILE_10);

  new FXMenuSeparator(&filemenu);
  new FXMenuCommand(&filemenu,"New directory...",newdiricon,this,ID_NEW);
  new FXMenuCommand(&filemenu,"Copy...",copyicon,this,ID_COPY);
  new FXMenuCommand(&filemenu,"Move...",moveicon,this,ID_MOVE);
  new FXMenuCommand(&filemenu,"Link...",linkicon,this,ID_LINK);
  new FXMenuCommand(&filemenu,"Delete...",deleteicon,this,ID_DELETE);

  filemenu.create();
  filemenu.popup(NULL,event->root_x,event->root_y);
  getApp()->runModalWhileShown(&filemenu);
  return 1;
  }


// Strip pattern from text if present
FXString FXFileSelector::patternFromText(const FXString& pattern){
  FXint beg,end;
  end=pattern.findb(')');         // Search from the end so we can allow ( ) in the pattern name itself
  beg=pattern.findb('(',end-1);
  if(0<=beg && beg<end) return pattern.mid(beg+1,end-beg-1);
  return pattern;
  }


// Return the first extension "ext1" found in the pattern if the
// pattern is of the form "*.ext1,*.ext2,..." or the empty string
// if the pattern contains other wildcard combinations.
FXString FXFileSelector::extensionFromPattern(const FXString& pattern){
  FXint beg,end,c;
  beg=0;
  if(pattern[beg]=='*'){
    beg++;
    if(pattern[beg]=='.'){
      beg++;
      end=beg;
      while((c=pattern[end])!='\0' && c!=',' && c!='|'){
        if(c=='*' || c=='?' || c=='[' || c==']' || c=='^' || c=='!') return FXString::null;
        end++;
        }
      return pattern.mid(beg,end-beg);
      }
    }
  return FXString::null;
  }


// Change the pattern; change the filename to the suggested extension
long FXFileSelector::onCmdFilter(FXObject*,FXSelector,void* ptr){
  FXString pat=patternFromText((FXchar*)ptr);
  filebox->setPattern(pat);
  if(selectmode==SELECTFILE_ANY){
    FXString ext=extensionFromPattern(pat);
    if(!ext.empty()){
      FXString name=FXFile::stripExtension(filename->getText());
      if(!name.empty()) filename->setText(name+"."+ext);
      }
    }
  return 1;
  }


// Set directory
void FXFileSelector::setDirectory(const FXString& path){
  FXString abspath=FXFile::absolute(path);
  filebox->setDirectory(abspath);
  dirbox->setDirectory(abspath);
  if(selectmode!=SELECTFILE_ANY){
    filename->setText(FXString::null);
    }
  }


// Get directory
FXString FXFileSelector::getDirectory() const {
  return filebox->getDirectory();
  }


// Set file name
void FXFileSelector::setFilename(const FXString& path){
  FXString abspath=FXFile::absolute(path);
  if(selectmode==SELECTFILE_DIRECTORY){
    filebox->setDirectory(abspath);
    dirbox->setDirectory(abspath);
    filename->setText(FXString::null);
    }
  else{
    filebox->setCurrentFile(abspath);
    dirbox->setDirectory(FXFile::directory(abspath));
    filename->setText(FXFile::name(abspath));
    }
  }


// Get complete path + filename
FXString FXFileSelector::getFilename() const {
  return FXFile::absolute(filebox->getDirectory(),filename->getText());
  }


// Return empty-string terminated list of selected file names, or NULL
FXString* FXFileSelector::getFilenames() const {
  register FXString *files=NULL;
  register FXint i,n;
  if(filebox->getNumItems()){
    if(selectmode==SELECTFILE_MULTIPLE_ALL){
      for(i=n=0; i<filebox->getNumItems(); i++){
        if(filebox->isItemSelected(i) && filebox->getItemFilename(i)!=".."){
          n++;
          }
        }
      if(n){
        files=new FXString [n+1];
        for(i=n=0; i<filebox->getNumItems(); i++){
          if(filebox->isItemSelected(i) && filebox->getItemFilename(i)!=".."){
            files[n++]=filebox->getItemPathname(i);
            }
          }
        files[n]=FXString::null;
        }
      }
    else{
      for(i=n=0; i<filebox->getNumItems(); i++){
        if(filebox->isItemSelected(i) && filebox->isItemFile(i)){
          n++;
          }
        }
      if(n){
        files=new FXString [n+1];
        for(i=n=0; i<filebox->getNumItems(); i++){
          if(filebox->isItemSelected(i) && filebox->isItemFile(i)){
            files[n++]=filebox->getItemPathname(i);
            }
          }
        files[n]=FXString::null;
        }
      }
    }
  return files;
  }


// Set bunch of patterns
void FXFileSelector::setPatternList(const FXchar **ptrns){
  filefilter->clearItems();
  if(ptrns){
    while(ptrns[0] && ptrns[1]){
      filefilter->appendItem(FXStringFormat("%s (%s)",ptrns[0],ptrns[1]));
      ptrns+=2;
      }
    }
  if(!filefilter->getNumItems()) filefilter->appendItem(allfiles);
  setCurrentPattern(0);
  }


// Change patterns, each pattern separated by newline
void FXFileSelector::setPatternList(const FXString& patterns){
  FXString pat; FXint i;
  filefilter->clearItems();
  for(i=0; !(pat=patterns.extract(i,'\n')).empty(); i++){
    filefilter->appendItem(pat);
    }
  if(!filefilter->getNumItems()) filefilter->appendItem(allfiles);
  setCurrentPattern(0);
  }


// Return list of patterns
FXString FXFileSelector::getPatternList() const {
  FXString pat; FXint i;
  for(i=0; i<filefilter->getNumItems(); i++){
    if(!pat.empty()) pat+='\n';
    pat+=filefilter->getItemText(i);
    }
  return pat;
  }


// Set current filter pattern
void FXFileSelector::setPattern(const FXString& ptrn){
  filefilter->setText(ptrn);
  filebox->setPattern(ptrn);
  }


// Get current filter pattern
FXString FXFileSelector::getPattern() const {
  return filebox->getPattern();
  }


// Set current file pattern from the list
void FXFileSelector::setCurrentPattern(FXint patno){
  if(patno<0 || patno>=filefilter->getNumItems()){ fxerror("%s::setCurrentPattern: index out of range.\n",getClassName()); }
  filefilter->setCurrentItem(patno);
  filebox->setPattern(patternFromText(filefilter->getItemText(patno)));
  }


// Return current pattern
FXint FXFileSelector::getCurrentPattern() const {
  return filefilter->getCurrentItem();
  }


// Change pattern for pattern number patno
void FXFileSelector::setPatternText(FXint patno,const FXString& text){
  if(patno<0 || patno>=filefilter->getNumItems()){ fxerror("%s::setPatternText: index out of range.\n",getClassName()); }
  filefilter->setItemText(patno,text);
  if(patno==filefilter->getCurrentItem()){
    setPattern(patternFromText(text));
    }
  }


// Return pattern text of pattern patno
FXString FXFileSelector::getPatternText(FXint patno) const {
  if(patno<0 || patno>=filefilter->getNumItems()){ fxerror("%s::getPatternText: index out of range.\n",getClassName()); }
  return filefilter->getItemText(patno);
  }


// Change space for item
void FXFileSelector::setItemSpace(FXint s){
  filebox->setItemSpace(s);
  }


// Get space for item
FXint FXFileSelector::getItemSpace() const {
  return filebox->getItemSpace();
  }


// Change File List style
void FXFileSelector::setFileBoxStyle(FXuint style){
  filebox->setListStyle(style);
  }


// Return File List style
FXuint FXFileSelector::getFileBoxStyle() const {
  return filebox->getListStyle();
  }


// Change file selection mode
void FXFileSelector::setSelectMode(FXuint mode){
  switch(mode){
    case SELECTFILE_EXISTING:
      filebox->showOnlyDirectories(FALSE);
      filebox->setListStyle((filebox->getListStyle()&~FILELISTMASK)|ICONLIST_BROWSESELECT);
      break;
    case SELECTFILE_MULTIPLE:
    case SELECTFILE_MULTIPLE_ALL:
      filebox->showOnlyDirectories(FALSE);
      filebox->setListStyle((filebox->getListStyle()&~FILELISTMASK)|ICONLIST_EXTENDEDSELECT);
      break;
    case SELECTFILE_DIRECTORY:
      filebox->showOnlyDirectories(TRUE);
      filebox->setListStyle((filebox->getListStyle()&~FILELISTMASK)|ICONLIST_BROWSESELECT);
      break;
    default:
      filebox->showOnlyDirectories(FALSE);
      filebox->setListStyle((filebox->getListStyle()&~FILELISTMASK)|ICONLIST_BROWSESELECT);
      break;
    }
  selectmode=mode;
  }


// Show readonly button
void FXFileSelector::showReadOnly(FXbool show){
  show ? readonly->show() : readonly->hide();
  }


// Return TRUE if readonly is shown
FXbool FXFileSelector::shownReadOnly() const {
  return readonly->shown();
  }



// Set initial state of readonly button
void FXFileSelector::setReadOnly(FXbool state){
  readonly->setCheck(state);
  }


// Get readonly state
FXbool FXFileSelector::getReadOnly() const {
  return readonly->getCheck();
  }


// Save data
void FXFileSelector::save(FXStream& store) const {
  FXPacker::save(store);
  store << filebox;
  store << filename;
  store << filefilter;
  store << bookmarks;
  store << readonly;
  store << dirbox;
  store << accept;
  store << cancel;
  store << updiricon;
  store << newdiricon;
  store << listicon;
  store << detailicon;
  store << iconsicon;
  store << homeicon;
  store << workicon;
  store << shownicon;
  store << hiddenicon;
  store << markicon;
  store << clearicon;
  store << moveicon;
  store << copyicon;
  store << selectmode;
  }


// Load data
void FXFileSelector::load(FXStream& store){
  FXPacker::load(store);
  store >> filebox;
  store >> filename;
  store >> filefilter;
  store >> bookmarks;
  store >> readonly;
  store >> dirbox;
  store >> accept;
  store >> cancel;
  store >> updiricon;
  store >> newdiricon;
  store >> listicon;
  store >> detailicon;
  store >> iconsicon;
  store >> homeicon;
  store >> workicon;
  store >> shownicon;
  store >> hiddenicon;
  store >> markicon;
  store >> clearicon;
  store >> moveicon;
  store >> copyicon;
  store >> selectmode;
  }


// Cleanup; icons must be explicitly deleted
FXFileSelector::~FXFileSelector(){
  FXAccelTable *table=getShell()->getAccelTable();
  if(table){
    table->removeAccel(MKUINT(KEY_BackSpace,0));
    table->removeAccel(MKUINT(KEY_h,CONTROLMASK));
    table->removeAccel(MKUINT(KEY_w,CONTROLMASK));
    table->removeAccel(MKUINT(KEY_n,CONTROLMASK));
    table->removeAccel(MKUINT(KEY_b,CONTROLMASK));
    table->removeAccel(MKUINT(KEY_s,CONTROLMASK));
    table->removeAccel(MKUINT(KEY_l,CONTROLMASK));
    }
  delete bookmarks;
  delete updiricon;
  delete newdiricon;
  delete listicon;
  delete detailicon;
  delete iconsicon;
  delete homeicon;
  delete workicon;
  delete shownicon;
  delete hiddenicon;
  delete markicon;
  delete clearicon;
  delete deleteicon;
  delete moveicon;
  delete copyicon;
  delete linkicon;
  filebox=(FXFileList*)-1;
  filename=(FXTextField*)-1;
  filefilter=(FXComboBox*)-1;
  bookmarks=(FXMenuPane*)-1;
  readonly=(FXCheckButton*)-1;
  dirbox=(FXDirBox*)-1;
  accept=(FXButton*)-1;
  cancel=(FXButton*)-1;
  updiricon=(FXIcon*)-1;
  newdiricon=(FXIcon*)-1;
  listicon=(FXIcon*)-1;
  detailicon=(FXIcon*)-1;
  iconsicon=(FXIcon*)-1;
  homeicon=(FXIcon*)-1;
  workicon=(FXIcon*)-1;
  shownicon=(FXIcon*)-1;
  hiddenicon=(FXIcon*)-1;
  markicon=(FXIcon*)-1;
  clearicon=(FXIcon*)-1;
  deleteicon=(FXIcon*)-1;
  moveicon=(FXIcon*)-1;
  copyicon=(FXIcon*)-1;
  linkicon=(FXIcon*)-1;
  }


