/********************************************************************************
*                                                                               *
*                     U n d o a b l e   C o m m a n d s                         *
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
* $Id: Commands.cpp,v 1.7 2002/02/27 05:12:45 fox Exp $                         *
********************************************************************************/
#include "fx.h"
#include "Commands.h"


/*
  Notes:

  - Need to keep better track of modified state of text buffer.
*/


/*******************************************************************************/

extern FXbool restyle;/// TEST

// Insert command
FXTextInsert::FXTextInsert(FXText* txt,FXint p,FXint ni):
  FXTextCommand(txt,p,ni,0){
  }


// Undo an insert removes the inserted text
void FXTextInsert::undo(){
  FXCALLOC(&buffer,FXchar,ins);
  text->extractText(buffer,pos,ins);
  text->removeText(pos,ins);
  text->setCursorPos(pos);
  text->makePositionVisible(pos);
restyle=TRUE;
  }


// Redo an insert inserts the same old text again
void FXTextInsert::redo(){
  text->insertText(pos,buffer,ins);
  text->setCursorPos(pos+ins);
  text->makePositionVisible(pos+ins);
  FXFREE(&buffer);
restyle=TRUE;
  }



// Delete command
FXTextDelete::FXTextDelete(FXText* txt,FXint p,FXint nd):
  FXTextCommand(txt,p,0,nd){
  FXCALLOC(&buffer,FXchar,del);
  text->extractText(buffer,pos,del);
  }


// Undo a delete reinserts the old text
void FXTextDelete::undo(){
  text->insertText(pos,buffer,del);
  text->setCursorPos(pos+del);
  text->makePositionVisible(pos+del);
  FXFREE(&buffer);
restyle=TRUE;
  }


// Redo a delete removes it again
void FXTextDelete::redo(){
  FXCALLOC(&buffer,FXchar,del);
  text->extractText(buffer,pos,del);
  text->removeText(pos,del);
  text->setCursorPos(pos);
  text->makePositionVisible(pos);
restyle=TRUE;
  }




