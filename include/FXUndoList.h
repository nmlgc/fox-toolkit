/********************************************************************************
*                                                                               *
*                  U n d o / R e d o - a b l e   C o m m a n d                  *
*                                                                               *
*********************************************************************************
* Copyright (C) 2000,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXUndoList.h,v 1.16 2002/01/18 22:42:55 jeroen Exp $                     *
********************************************************************************/
#ifndef FXUNDOLIST_H
#define FXUNDOLIST_H

#ifndef FXOBJECT_H
#include "FXObject.h"
#endif



class FXUndoList;


/// Base class for undoable commands
class FXAPI FXCommand {
  friend class FXUndoList;
private:
  FXCommand *next;
private:
  FXCommand(const FXCommand&);
  FXCommand &operator=(const FXCommand&);
protected:
  FXCommand():next(NULL){}
public:

  /**
  * Undo this command; this should save the
  * information for a subsequent redo.
  */
  virtual void undo() = 0;

  /**
  * Redo this command; this should save the
  * information for a subsequent undo.
  */
  virtual void redo() = 0;

  /**
  * Return the size of the information in the undo record.
  * The undo list may be trimmed to limit memory usage to
  * a certain limit.
  */
  virtual FXuint size() const = 0;

  /**
  * Name of the undo command to be shown on a button;
  * for example, "Undo Delete".
  */
  virtual FXString undoName() const = 0;

  /**
  * Name of the redo command to be shown on a button;
  * for example, "Redo Delete".
  */
  virtual FXString redoName() const = 0;

  /// Delete undo command
  virtual ~FXCommand(){}
  };


/**
* The Undo List class manages a list of undoable commands.
*/
class FXAPI FXUndoList : public FXObject {
  FXDECLARE(FXUndoList)
private:
  FXCommand *redolist;      // List yet to be done from this point
  FXCommand *undolist;      // List which has been done from this point
  FXint      marker;        // Marker value
  FXint      count;         // Number of undo records
  FXuint     size;          // Size of undo records
public:
  long onCmdUndo(FXObject*,FXSelector,void*);
  long onUpdUndo(FXObject*,FXSelector,void*);
  long onCmdRedo(FXObject*,FXSelector,void*);
  long onUpdRedo(FXObject*,FXSelector,void*);
  long onCmdClear(FXObject*,FXSelector,void*);
  long onUpdClear(FXObject*,FXSelector,void*);
  long onCmdRevert(FXObject*,FXSelector,void*);
  long onUpdRevert(FXObject*,FXSelector,void*);
  long onCmdUndoAll(FXObject*,FXSelector,void*);
  long onCmdRedoAll(FXObject*,FXSelector,void*);
public:
  enum{
    ID_CLEAR=FXWindow::ID_LAST,
    ID_REVERT,
    ID_UNDO,
    ID_REDO,
    ID_UNDO_ALL,
    ID_REDO_ALL,
    ID_LAST
    };
public:

  /// Make new empty undo list, initially unmarked.
  FXUndoList();

  /// Cut the redo list
  void cut();

  /// Add new command, executing if desired
  FXbool add(FXCommand* command,FXbool doit=FALSE);

  /// Undo last command
  FXbool undo();

  /// Redo next command
  FXbool redo();

  /// Undo all commands
  FXbool undoAll();

  /// Redo all commands
  FXbool redoAll();

  /// Revert to marked
  FXbool revert();

  /// Can we undo more commands
  FXbool canUndo() const;

  /// Can we redo more commands
  FXbool canRedo() const;

  /// Can revert to marked
  FXbool canRevert() const;

  /// Current undo command
  FXCommand* current() const { return undolist; }

  /// Number of undo records
  FXint undoCount() const { return count; }

  /// Size of undo information
  FXuint undoSize() const { return size; }

  /// Clear list, and unmark all states.
  void clear();

  /// Trim undo list down to at most nc commands
  void trimCount(FXint nc);

  /// Trim undo list down to at most size sz
  void trimSize(FXuint sz);

  /**
  * Mark the current state of the undo list, which is initially unmarked.
  * There can be only one active mark at any time.
  */
  void mark();

  /**
  * Unmark all states in the undo list.
  */
  void unmark();

  /**
  * Check if the current state was marked, if the application has returned
  * to the previously marked state.
  */
  FXbool marked() const;

  /// Clean up
  ~FXUndoList();
  };


#endif
