/********************************************************************************
*                                                                               *
*                            H e l p   W i n d o w                              *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998,2001 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: HelpWindow.cpp,v 1.3 2001/11/21 06:21:31 jeroen Exp $                    *
********************************************************************************/
#include "fx.h"
#include "HelpWindow.h"
#include "icons.h"
#include "help.h"
#include <stdio.h>
#include <stdlib.h>


/*******************************************************************************/

  
FXIMPLEMENT(HelpWindow,FXDialogBox,NULL,0)


// Construct help dialog box
HelpWindow::HelpWindow(FXWindow *owner):
  FXDialogBox(owner,"Help on Adie",DECOR_TITLE|DECOR_BORDER|DECOR_RESIZE,0,0,0,0, 6,6,6,6, 4,4){
  
  // Bottom part
  FXHorizontalFrame *closebox=new FXHorizontalFrame(this,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH);
  FXButton *button=new FXButton(closebox,"&Close",NULL,this,FXDialogBox::ID_ACCEPT,BUTTON_INITIAL|BUTTON_DEFAULT|LAYOUT_RIGHT|FRAME_RAISED|FRAME_THICK,0,0,0,0, 20,20,5,5);

  // Editor part
  FXHorizontalFrame *editbox=new FXHorizontalFrame(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
  helptext=new FXText(editbox,NULL,0,TEXT_READONLY|TEXT_WORDWRAP|LAYOUT_FILL_X|LAYOUT_FILL_Y);
  helptext->setVisRows(50);
  helptext->setVisCols(90);
  
  // Fill with help
  helptext->setText(help,strlen(help));
  helptext->setTabColumns(35);
  button->setFocus();
  }


// Clean up
HelpWindow::~HelpWindow(){
  helptext=(FXText*)-1;
  }
