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
* $Id: Commands.h,v 1.5.4.1 2002/04/30 13:13:07 fox Exp $                           *
********************************************************************************/
#ifndef COMMANDS_H
#define COMMANDS_H


/*******************************************************************************/

class FXText;


// Undo record for text fragment
class FXTextCommand : public FXCommand {
protected:
  FXText *text;     // Text widget
  FXchar *buffer;   // Character buffer
  FXint   pos;      // Character position
  FXint   del;      // Deleted characters
  FXint   ins;      // Inserted characters
public:
  FXTextCommand(FXText* txt,FXint p,FXint ni,FXint nd):text(txt),buffer(NULL),pos(p),del(nd),ins(ni){}
  virtual FXuint size() const { return del; }
  virtual ~FXTextCommand(){FXFREE(&buffer);}
  };


// Insert command
class FXTextInsert : public FXTextCommand {
public:
  FXTextInsert(FXText* txt,FXint p,FXint ni);
  virtual FXString undoName() const { return "Undo insert"; }
  virtual FXString redoName() const { return "Redo insert"; }
  virtual void undo();
  virtual void redo();
  };


// Delete command
class FXTextDelete : public FXTextCommand {
public:
  FXTextDelete(FXText* txt,FXint p,FXint nd);
  virtual FXString undoName() const { return "Undo delete"; }
  virtual FXString redoName() const { return "Redo delete"; }
  virtual void undo();
  virtual void redo();
  };


#endif

