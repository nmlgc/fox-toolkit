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
* $Id: FXUndoList.cpp,v 1.19 2002/01/18 22:43:07 jeroen Exp $                   *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXCharset.h"
#include "FXRegistry.h"
#include "FXAccelTable.h"
#include "FXApp.h"
#include "FXWindow.h"
#include "FXUndoList.h"

/*
  Notes:

  - When a command is undone, its moved to the redo list.

  - When a command is redone, its moved back to the undo list.

  - Whenever adding a new command, the redo list is deleted.

  - At any time, you can trim down the undo list down to a given
    maximum size or a given number of undo records.  This should
    keep the memory overhead within sensible bounds.

  - To keep track of when we get back to an "unmodified" state, a mark
    can be set.  The mark is basically a counter which is incremented
    with every undo record added, and decremented when undoing a command.
    When we get back to 0, we are back to the unmodified state.

    If, after setting the mark, we have called undo(), then the mark can be
    reached by calling redo().

    If the marked position is in the redo-list, then adding a new undo
    record will cause the redo-list to be deleted, and the marked position
    will become unreachable.

    The marked state may also become unreachable when the undo list is trimmed.

  - You can call also kill the redo list without adding a new command
    to the undo list, although this may cause the marked position to
    become unreachable.

  - We measure the size of the undo-records in the undo-list; when the
    records are moved to the redo-list, they usually contain different
    information!

*/

#define NOMARK 2147483647       // No mark is set

/*******************************************************************************/

// Map
FXDEFMAP(FXUndoList) FXUndoListMap[]={
  FXMAPFUNC(SEL_COMMAND, FXUndoList::ID_CLEAR,    FXUndoList::onCmdClear),
  FXMAPFUNC(SEL_UPDATE,  FXUndoList::ID_CLEAR,    FXUndoList::onUpdClear),
  FXMAPFUNC(SEL_COMMAND, FXUndoList::ID_REVERT,   FXUndoList::onCmdRevert),
  FXMAPFUNC(SEL_UPDATE,  FXUndoList::ID_REVERT,   FXUndoList::onUpdRevert),
  FXMAPFUNC(SEL_COMMAND, FXUndoList::ID_UNDO,     FXUndoList::onCmdUndo),
  FXMAPFUNC(SEL_UPDATE,  FXUndoList::ID_UNDO,     FXUndoList::onUpdUndo),
  FXMAPFUNC(SEL_COMMAND, FXUndoList::ID_REDO,     FXUndoList::onCmdRedo),
  FXMAPFUNC(SEL_UPDATE,  FXUndoList::ID_REDO,     FXUndoList::onUpdRedo),
  FXMAPFUNC(SEL_COMMAND, FXUndoList::ID_UNDO_ALL, FXUndoList::onCmdUndoAll),
  FXMAPFUNC(SEL_UPDATE,  FXUndoList::ID_UNDO_ALL, FXUndoList::onUpdUndo),
  FXMAPFUNC(SEL_COMMAND, FXUndoList::ID_REDO_ALL, FXUndoList::onCmdRedoAll),
  FXMAPFUNC(SEL_UPDATE,  FXUndoList::ID_REDO_ALL, FXUndoList::onUpdRedo),
  };


// Object implementation
FXIMPLEMENT(FXUndoList,FXObject,FXUndoListMap,ARRAYNUMBER(FXUndoListMap))


// Make new empty undo list
FXUndoList::FXUndoList(){
  redolist=NULL;
  undolist=NULL;
  marker=NOMARK;
  size=0;
  count=0;
  }


// Mark current state
void FXUndoList::mark(){
  marker=0;
  }


// Unmark undo list
void FXUndoList::unmark(){
  marker=NOMARK;
  }


// Check if marked
FXbool FXUndoList::marked() const {
  return marker==0;
  }


// Cut the redo list
void FXUndoList::cut(){
  register FXCommand *p;
  while(redolist){
    p=redolist;
    redolist=redolist->next;
    delete p;
    }
  redolist=NULL;
  if(marker<0) marker=NOMARK;           // Can not return to marked state anymore!
  }


// Add new command, executing if desired
FXbool FXUndoList::add(FXCommand* command,FXbool doit){

  // Cut redo list
  cut();

  // No command given
  if(!command) return TRUE;

  // Add to undo list
  command->next=undolist;
  undolist=command;

  // Execute it right now
  if(doit) command->redo();

  // Update info
  size+=command->size();		// Measure AFTER redo!
  count++;

  if(marker!=NOMARK) marker++;

  FXTRACE((100,"FXUndoList::add: size=%d count=%d marker=%d\n",size,count,marker));

  return TRUE;
  }


// Undo last command
FXbool FXUndoList::undo(){
  register FXCommand *command=undolist;
  if(command){
    size-=command->size();		// Measure BEFORE undo!
    command->undo();
    undolist=undolist->next;
    command->next=redolist;
    redolist=command;
    count--;
    if(marker!=NOMARK) marker--;
    FXTRACE((100,"FXUndoList::undo: size=%d count=%d marker=%d\n",size,count,marker));
    return TRUE;
    }
  return FALSE;
  }


// Redo next command
FXbool FXUndoList::redo(){
  register FXCommand *command=redolist;
  if(command){
    command->redo();
    redolist=redolist->next;
    command->next=undolist;
    undolist=command;
    size+=command->size();		// Measure AFTER redo!
    count++;
    if(marker!=NOMARK) marker++;
    FXTRACE((100,"FXUndoList::redo: size=%d count=%d marker=%d\n",size,count,marker));
    return TRUE;
    }
  return FALSE;
  }


// Undo all commands
FXbool FXUndoList::undoAll(){
  while(canUndo()) undo();
  return TRUE;
  }


// Redo all commands
FXbool FXUndoList::redoAll(){
  while(canRedo()) redo();
  return TRUE;
  }


// Revert to marked
FXbool FXUndoList::revert(){
  if(marker!=NOMARK){
    while(marker>0) undo();
    while(marker<0) redo();
    return TRUE;
    }
  return FALSE;
  }


// Can we undo more commands
FXbool FXUndoList::canUndo() const {
  return undolist!=NULL;
  }


// Can we redo more commands
FXbool FXUndoList::canRedo() const {
  return redolist!=NULL;
  }


// Can revert to marked
FXbool FXUndoList::canRevert() const {
  return marker!=NOMARK && marker!=0;
  }


// Clear list
void FXUndoList::clear(){
  register FXCommand *p;
  FXTRACE((100,"FXUndoList::clear: size=%d count=%d marker=%d\n",size,count,marker));
  while(redolist){
    p=redolist;
    redolist=redolist->next;
    delete p;
    }
  while(undolist){
    p=undolist;
    undolist=undolist->next;
    delete p;
    }
  redolist=NULL;
  undolist=NULL;
  marker=NOMARK;
  count=0;
  size=0;
  }


// Clear undo list
long FXUndoList::onCmdClear(FXObject*,FXSelector,void*){
  clear();
  return 1;
  }


// Update Clear undo list
long FXUndoList::onUpdClear(FXObject* sender,FXSelector,void*){
  if(canUndo() || canRedo())
    sender->handle(this,MKUINT(FXWindow::ID_ENABLE,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(FXWindow::ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Revert to marked
long FXUndoList::onCmdRevert(FXObject*,FXSelector,void*){
  revert();
  return 1;
  }


// Update revert to marked
long FXUndoList::onUpdRevert(FXObject* sender,FXSelector,void*){
  if(canRevert())
    sender->handle(this,MKUINT(FXWindow::ID_ENABLE,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(FXWindow::ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Undo last command
long FXUndoList::onCmdUndo(FXObject*,FXSelector,void*){
  undo();
  return 1;
  }


// Undo all commands
long FXUndoList::onCmdUndoAll(FXObject*,FXSelector,void*){
  undoAll();
  return 1;
  }


// Update undo last command
long FXUndoList::onUpdUndo(FXObject* sender,FXSelector,void*){
  if(canUndo())
    sender->handle(this,MKUINT(FXWindow::ID_ENABLE,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(FXWindow::ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Redo last command
long FXUndoList::onCmdRedo(FXObject*,FXSelector,void*){
  redo();
  return 1;
  }


// Redo all commands
long FXUndoList::onCmdRedoAll(FXObject*,FXSelector,void*){
  redoAll();
  return 1;
  }


// Update redo last command
long FXUndoList::onUpdRedo(FXObject* sender,FXSelector,void*){
  if(canRedo())
    sender->handle(this,MKUINT(FXWindow::ID_ENABLE,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(FXWindow::ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Trim undo list down to at most n records
void FXUndoList::trimCount(FXint nc){
  FXTRACE((100,"FXUndoList::trimCount: was: size=%d count=%d; marker=%d ",size,count,marker));
  if(count>nc){
    register FXCommand **pp=&undolist;
    register FXCommand *p=*pp;
    register FXint i=0;
    while(p && i<nc){
      pp=&p->next;
      p=*pp;
      i++;
      }
    while(*pp){
      p=*pp;
      *pp=p->next;
      size-=p->size();
      count--;
      delete p;
      }
    if(marker>count) marker=NOMARK;
    }
  FXTRACE((100,"now: size=%d count=%d; marker=%d\n",size,count,marker));
  }


// Trim undo list down to at most size sz
void FXUndoList::trimSize(FXuint sz){
  FXTRACE((100,"FXUndoList::trimSize: was: size=%d count=%d; marker=%d ",size,count,marker));
  if(size>sz){
    register FXCommand **pp=&undolist;
    register FXCommand *p=*pp;
    register FXuint s=0;
    while(p && (s=s+p->size())<=sz){
      pp=&p->next;
      p=*pp;
      }
    while(*pp){
      p=*pp;
      *pp=p->next;
      size-=p->size();
      count--;
      delete p;
      }
    if(marker>count) marker=NOMARK;
    }
  FXTRACE((100,"now: size=%d count=%d; marker=%d\n",size,count,marker));
  }


// Clean up
FXUndoList::~FXUndoList(){
  clear();
  }

