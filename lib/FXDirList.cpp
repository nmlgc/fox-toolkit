/********************************************************************************
*                                                                               *
*                     D i r e c t o r y   L i s t   O b j e c t                 *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998,2012 by Jeroen van der Zijp.   All Rights Reserved.        *
*********************************************************************************
* This library is free software; you can redistribute it and/or modify          *
* it under the terms of the GNU Lesser General Public License as published by   *
* the Free Software Foundation; either version 3 of the License, or             *
* (at your option) any later version.                                           *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
* GNU Lesser General Public License for more details.                           *
*                                                                               *
* You should have received a copy of the GNU Lesser General Public License      *
* along with this program.  If not, see <http://www.gnu.org/licenses/>          *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxascii.h"
#include "fxkeys.h"
#include "FXArray.h"
#include "FXHash.h"
#include "FXMutex.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXPath.h"
#include "FXStat.h"
#include "FXFile.h"
#include "FXDir.h"
#include "FXURL.h"
#include "FXSettings.h"
#include "FXRegistry.h"
#include "FXFont.h"
#include "FXEvent.h"
#include "FXWindow.h"
#include "FXApp.h"
#include "FXIcon.h"
#include "FXBMPIcon.h"
#include "FXGIFIcon.h"
#include "FXScrollBar.h"
#include "FXDirList.h"
#include "FXMenuPane.h"
#include "FXMenuCaption.h"
#include "FXMenuCommand.h"
#include "FXMenuCascade.h"
#include "FXMenuRadio.h"
#include "FXMenuCheck.h"
#include "FXMenuSeparator.h"
#include "FXFileDict.h"
#include "FXMessageBox.h"
#ifdef WIN32
#include <shellapi.h>
#endif
#include "icons.h"

/*
  Notes:
  - One can never create items in constructor:- createItem() may be overloaded!
  - Instead of FXTreeItems, callbacks should pass pointer to directory?
  - Clipboard of a filenames.
  - Should do drag and drop and such.
  - Clipboard, DND, etc. support.
  - We should NOT assume the root's name is just '/'.  It could be C:\ etc.
  - Try read icons from <path>/.dir.gif and <path>/.opendir.gif!
  - Special icon for root.
  - We should generate SEL_INSERTED, SEL_DELETED, SEL_CHANGED
    messages as the FXDirList updates itself from the file system.
  - Under MS-Windows, always pass MatchNoEscape setting match mode!
  - Every once in a while, even if checking doesn't reveal a change,
    refresh the entire list anyway!
*/


#define REFRESHINTERVAL     1000000000  // Interval between refreshes
#define REFRESHFREQUENCY    30          // File systems not supporting mod-time, refresh every nth time

using namespace FX;

/*******************************************************************************/

namespace FX {


// Object implementation
FXIMPLEMENT(FXDirItem,FXTreeItem,NULL,0)


// Map
FXDEFMAP(FXDirList) FXDirListMap[]={
  FXMAPFUNC(SEL_DND_ENTER,0,FXDirList::onDNDEnter),
  FXMAPFUNC(SEL_DND_LEAVE,0,FXDirList::onDNDLeave),
  FXMAPFUNC(SEL_DND_DROP,0,FXDirList::onDNDDrop),
  FXMAPFUNC(SEL_DND_MOTION,0,FXDirList::onDNDMotion),
  FXMAPFUNC(SEL_DND_REQUEST,0,FXDirList::onDNDRequest),
  FXMAPFUNC(SEL_BEGINDRAG,0,FXDirList::onBeginDrag),
  FXMAPFUNC(SEL_DRAGGED,0,FXDirList::onDragged),
  FXMAPFUNC(SEL_ENDDRAG,0,FXDirList::onEndDrag),
  FXMAPFUNC(SEL_TIMEOUT,FXDirList::ID_REFRESHTIMER,FXDirList::onRefreshTimer),
  FXMAPFUNC(SEL_UPDATE,FXDirList::ID_SHOW_HIDDEN,FXDirList::onUpdShowHidden),
  FXMAPFUNC(SEL_UPDATE,FXDirList::ID_HIDE_HIDDEN,FXDirList::onUpdHideHidden),
  FXMAPFUNC(SEL_UPDATE,FXDirList::ID_TOGGLE_HIDDEN,FXDirList::onUpdToggleHidden),
  FXMAPFUNC(SEL_UPDATE,FXDirList::ID_SHOW_FILES,FXDirList::onUpdShowFiles),
  FXMAPFUNC(SEL_UPDATE,FXDirList::ID_HIDE_FILES,FXDirList::onUpdHideFiles),
  FXMAPFUNC(SEL_UPDATE,FXDirList::ID_TOGGLE_FILES,FXDirList::onUpdToggleFiles),
  FXMAPFUNC(SEL_UPDATE,FXDirList::ID_SET_PATTERN,FXDirList::onUpdSetPattern),
  FXMAPFUNC(SEL_UPDATE,FXDirList::ID_SORT_REVERSE,FXDirList::onUpdSortReverse),
  FXMAPFUNC(SEL_UPDATE,FXDirList::ID_SORT_CASE,FXDirList::onUpdSortCase),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETVALUE,FXDirList::onCmdSetValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_SETSTRINGVALUE,FXDirList::onCmdSetStringValue),
  FXMAPFUNC(SEL_COMMAND,FXWindow::ID_GETSTRINGVALUE,FXDirList::onCmdGetStringValue),
  FXMAPFUNC(SEL_COMMAND,FXDirList::ID_SHOW_HIDDEN,FXDirList::onCmdShowHidden),
  FXMAPFUNC(SEL_COMMAND,FXDirList::ID_HIDE_HIDDEN,FXDirList::onCmdHideHidden),
  FXMAPFUNC(SEL_COMMAND,FXDirList::ID_TOGGLE_HIDDEN,FXDirList::onCmdToggleHidden),
  FXMAPFUNC(SEL_COMMAND,FXDirList::ID_SHOW_FILES,FXDirList::onCmdShowFiles),
  FXMAPFUNC(SEL_COMMAND,FXDirList::ID_HIDE_FILES,FXDirList::onCmdHideFiles),
  FXMAPFUNC(SEL_COMMAND,FXDirList::ID_TOGGLE_FILES,FXDirList::onCmdToggleFiles),
  FXMAPFUNC(SEL_COMMAND,FXDirList::ID_SET_PATTERN,FXDirList::onCmdSetPattern),
  FXMAPFUNC(SEL_COMMAND,FXDirList::ID_SORT_REVERSE,FXDirList::onCmdSortReverse),
  FXMAPFUNC(SEL_COMMAND,FXDirList::ID_SORT_CASE,FXDirList::onCmdSortCase),
  FXMAPFUNC(SEL_COMMAND,FXDirList::ID_REFRESH,FXDirList::onCmdRefresh),
  FXMAPFUNC(SEL_CHORE,FXDirList::ID_DROPASK,FXDirList::onCmdDropAsk),
  FXMAPFUNC(SEL_CHORE,FXDirList::ID_DROPCOPY,FXDirList::onCmdDropCopy),
  FXMAPFUNC(SEL_COMMAND,FXDirList::ID_DROPCOPY,FXDirList::onCmdDropCopy),
  FXMAPFUNC(SEL_CHORE,FXDirList::ID_DROPMOVE,FXDirList::onCmdDropMove),
  FXMAPFUNC(SEL_COMMAND,FXDirList::ID_DROPMOVE,FXDirList::onCmdDropMove),
  FXMAPFUNC(SEL_CHORE,FXDirList::ID_DROPLINK,FXDirList::onCmdDropLink),
  FXMAPFUNC(SEL_COMMAND,FXDirList::ID_DROPLINK,FXDirList::onCmdDropLink),
  };


// Object implementation
FXIMPLEMENT(FXDirList,FXTreeList,FXDirListMap,ARRAYNUMBER(FXDirListMap))


// For serialization
FXDirList::FXDirList(){
  dropEnable();
  associations=NULL;
  list=NULL;
  opendiricon=NULL;
  closeddiricon=NULL;
  documenticon=NULL;
  applicationicon=NULL;
  cdromicon=NULL;
  harddiskicon=NULL;
  networkicon=NULL;
  floppyicon=NULL;
  zipdiskicon=NULL;
#ifdef WIN32
  matchmode=FXPath::PathName|FXPath::NoEscape|FXPath::CaseFold;
#else
  matchmode=FXPath::PathName|FXPath::NoEscape;
#endif
  setSortFunc(ascendingCase);
  dropaction=DRAG_MOVE;
  draggable=true;
  counter=0;
  }


// Directory List Widget
FXDirList::FXDirList(FXComposite *p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):FXTreeList(p,tgt,sel,opts,x,y,w,h),pattern("*"){
  dropEnable();
  associations=NULL;
  list=NULL;
  if(!(options&DIRLIST_NO_OWN_ASSOC)) associations=new FXFileDict(getApp());
  opendiricon=new FXGIFIcon(getApp(),minifolderopen);
  closeddiricon=new FXGIFIcon(getApp(),minifolder);
  documenticon=new FXGIFIcon(getApp(),minidoc);
  applicationicon=new FXGIFIcon(getApp(),miniapp);
  cdromicon=new FXGIFIcon(getApp(),minicdrom);
  harddiskicon=new FXGIFIcon(getApp(),miniharddisk);
  networkicon=new FXGIFIcon(getApp(),mininetdrive);
  floppyicon=new FXGIFIcon(getApp(),minifloppy);
  zipdiskicon=new FXGIFIcon(getApp(),minizipdrive);
#ifdef WIN32
  matchmode=FXPath::PathName|FXPath::NoEscape|FXPath::CaseFold;
#else
  matchmode=FXPath::PathName|FXPath::NoEscape;
#endif
  setSortFunc(ascendingCase);
  dropaction=DRAG_MOVE;
  draggable=true;
  counter=0;
  }


// Create X window
void FXDirList::create(){
  FXTreeList::create();
  if(!deleteType){deleteType=getApp()->registerDragType(deleteTypeName);}
  if(!urilistType){urilistType=getApp()->registerDragType(urilistTypeName);}
  if(!actionType){actionType=getApp()->registerDragType(actionTypeName);}
  getApp()->addTimeout(this,ID_REFRESHTIMER,REFRESHINTERVAL);
  opendiricon->create();
  closeddiricon->create();
  documenticon->create();
  applicationicon->create();
  cdromicon->create();
  harddiskicon->create();
  networkicon->create();
  floppyicon->create();
  zipdiskicon->create();
  scan(false);
  }


// Detach disconnects the icons
void FXDirList::detach(){
  FXTreeList::detach();
  getApp()->removeTimeout(this,ID_REFRESHTIMER);
  opendiricon->detach();
  closeddiricon->detach();
  documenticon->detach();
  applicationicon->detach();
  cdromicon->detach();
  harddiskicon->detach();
  networkicon->detach();
  floppyicon->detach();
  zipdiskicon->detach();
  deleteType=0;
  urilistType=0;
  actionType=0;
  }


// Destroy zaps the icons
void FXDirList::destroy(){
  FXTreeList::destroy();
  getApp()->removeTimeout(this,ID_REFRESHTIMER);
  opendiricon->destroy();
  closeddiricon->destroy();
  documenticon->destroy();
  applicationicon->destroy();
  cdromicon->destroy();
  harddiskicon->destroy();
  networkicon->destroy();
  floppyicon->destroy();
  zipdiskicon->destroy();
  }


// Create item
FXTreeItem* FXDirList::createItem(const FXString& text,FXIcon* oi,FXIcon* ci,void* ptr){
  return new FXDirItem(text,oi,ci,ptr);
  }

/*******************************************************************************/

// Sort ascending order, keeping directories first
FXint FXDirList::ascending(const FXTreeItem* pa,const FXTreeItem* pb){
  register FXint diff=static_cast<const FXDirItem*>(pb)->isDirectory() - static_cast<const FXDirItem*>(pa)->isDirectory();
  return diff ? diff : compare(pa->label,pb->label);
  }


// Sort descending order, keeping directories first
FXint FXDirList::descending(const FXTreeItem* pa,const FXTreeItem* pb){
  register FXint diff=static_cast<const FXDirItem*>(pb)->isDirectory() - static_cast<const FXDirItem*>(pa)->isDirectory();
  return diff ? diff : compare(pb->label,pa->label);
  }


// Sort ascending order, case insensitive, keeping directories first
FXint FXDirList::ascendingCase(const FXTreeItem* pa,const FXTreeItem* pb){
  register FXint diff=static_cast<const FXDirItem*>(pb)->isDirectory() - static_cast<const FXDirItem*>(pa)->isDirectory();
  return diff ? diff : comparecase(pa->label,pb->label);
  }


// Sort descending order, case insensitive, keeping directories first
FXint FXDirList::descendingCase(const FXTreeItem* pa,const FXTreeItem* pb){
  register FXint diff=static_cast<const FXDirItem*>(pb)->isDirectory() - static_cast<const FXDirItem*>(pa)->isDirectory();
  return diff ? diff : comparecase(pb->label,pa->label);
  }

/*******************************************************************************/

// Copy
long FXDirList::onCmdDropCopy(FXObject*,FXSelector,void*){
  FXString filedst,filesrc;
  FXint beg,end;
  for(beg=0; beg<dropfiles.length(); beg=end+2){
    if((end=dropfiles.find_first_of("\r\n",beg))<0) end=dropfiles.length();
    filesrc=FXURL::fileFromURL(dropfiles.mid(beg,end-beg));
    filedst=FXPath::absolute(dropdirectory,FXPath::name(filesrc));
    if(!FXFile::copyFiles(filesrc,filedst,false)){
      if(FXMessageBox::question(this,MBOX_YES_NO,tr("Overwrite File"),tr("Overwrite existing file or directory: %s?"),filedst.text())==MBOX_CLICKED_NO) break;
      FXFile::copyFiles(filesrc,filedst,true);
      }
    }
  dropdirectory=FXString::null;
  dropfiles=FXString::null;
  dropaction=DRAG_REJECT;
  return 1;
  }


// Copy
long FXDirList::onCmdDropMove(FXObject*,FXSelector,void*){
  FXString filedst,filesrc;
  FXint beg,end;
  for(beg=0; beg<dropfiles.length(); beg=end+2){
    if((end=dropfiles.find_first_of("\r\n",beg))<0) end=dropfiles.length();
    filesrc=FXURL::fileFromURL(dropfiles.mid(beg,end-beg));
    filedst=FXPath::absolute(dropdirectory,FXPath::name(filesrc));
    if(!FXFile::moveFiles(filesrc,filedst,false)){
      if(FXMessageBox::question(this,MBOX_YES_NO,tr("Overwrite File"),tr("Overwrite existing file or directory: %s?"),filedst.text())==MBOX_CLICKED_NO) break;
      FXFile::moveFiles(filesrc,filedst,true);
      }
    }
  dropdirectory=FXString::null;
  dropfiles=FXString::null;
  dropaction=DRAG_REJECT;
  return 1;
  }


// Copy
long FXDirList::onCmdDropLink(FXObject*,FXSelector,void*){
  FXString filedst,filesrc;
  FXint beg,end;
  for(beg=0; beg<dropfiles.length(); beg=end+2){
    if((end=dropfiles.find_first_of("\r\n",beg))<0) end=dropfiles.length();
    filesrc=FXURL::fileFromURL(dropfiles.mid(beg,end-beg));
    filedst=FXPath::absolute(dropdirectory,FXPath::name(filesrc));
    if(!FXFile::symlink(filesrc,filedst)){
      if(FXMessageBox::question(this,MBOX_YES_NO,tr("Overwrite File"),tr("Overwrite existing file or directory: %s?"),filedst.text())==MBOX_CLICKED_NO) break;
      }
    }
  dropdirectory=FXString::null;
  dropfiles=FXString::null;
  dropaction=DRAG_REJECT;
  return 1;
  }


// Deal with the drop that has just occurred
long FXDirList::onCmdDropAsk(FXObject*,FXSelector,void* ptr){
  FXMenuPane dropmenu(this);
  FXGIFIcon filemoveicon(getApp(),filemove);
  FXGIFIcon filecopyicon(getApp(),filecopy);
  FXGIFIcon filelinkicon(getApp(),filelink);
  FXGIFIcon filecancelicon(getApp(),filecancel);
  new FXMenuCommand(&dropmenu,tr("Move Here"),&filemoveicon,this,ID_DROPMOVE);
  new FXMenuCommand(&dropmenu,tr("Copy Here"),&filecopyicon,this,ID_DROPCOPY);
  new FXMenuCommand(&dropmenu,tr("Link Here"),&filelinkicon,this,ID_DROPLINK);
  new FXMenuSeparator(&dropmenu);
  new FXMenuCommand(&dropmenu,tr("Cancel"),&filecancelicon);
  dropmenu.create();
  dropmenu.popup(NULL,((FXEvent*)ptr)->root_x,((FXEvent*)ptr)->root_y);
  getApp()->runModalWhileShown(&dropmenu);
  dropdirectory=FXString::null;
  dropfiles=FXString::null;
  dropaction=DRAG_REJECT;
  return 1;
  }


// Handle drag-and-drop enter
long FXDirList::onDNDEnter(FXObject* sender,FXSelector sel,void* ptr){
  FXTreeList::onDNDEnter(sender,sel,ptr);
  return 1;
  }


// Handle drag-and-drop leave
long FXDirList::onDNDLeave(FXObject* sender,FXSelector sel,void* ptr){
  FXTreeList::onDNDLeave(sender,sel,ptr);
  stopAutoScroll();
  dropdirectory.clear();
  dropaction=DRAG_REJECT;
  return 1;
  }


// Handle drag-and-drop motion
long FXDirList::onDNDMotion(FXObject* sender,FXSelector sel,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  FXTreeItem *item;

  // Start autoscrolling
  if(startAutoScroll(event,false)) return 1;

  // Give base class a shot
  if(FXTreeList::onDNDMotion(sender,sel,ptr)) return 1;

  // Dropping list of filenames
  if(offeredDNDType(FROM_DRAGNDROP,urilistType)){

    // Locate drop place
    item=getItemAt(event->win_x,event->win_y);

    // We can drop in a directory
    if(item && isItemDirectory(item)){

      // Get drop directory
      dropdirectory=getItemPathname(item);

      // What is being done (move,copy,link)
      dropaction=inquireDNDAction();

      // See if this is writable
      if(FXStat::isWritable(dropdirectory)){
        acceptDrop(DRAG_ACCEPT);
        }
      }
    return 1;
    }
  return 0;
  }


// Handle drag-and-drop drop
long FXDirList::onDNDDrop(FXObject* sender,FXSelector sel,void* ptr){

  // Stop scrolling
  stopAutoScroll();

  // Perhaps target wants to deal with it
  if(FXTreeList::onDNDDrop(sender,sel,ptr)) return 1;

  // Get uri-list of files being dropped
  if(getDNDData(FROM_DRAGNDROP,urilistType,dropfiles)){

    // Now handle it
    switch(dropaction){
      case DRAG_COPY:
        getApp()->addChore(this,ID_DROPCOPY,ptr);
        break;
      case DRAG_MOVE:
        getApp()->addChore(this,ID_DROPMOVE,ptr);
        break;
      case DRAG_LINK:
        getApp()->addChore(this,ID_DROPLINK,ptr);
        break;
      default:
        getApp()->addChore(this,ID_DROPASK,ptr);
        break;
      }
    return 1;
    }

  return 0;
  }


// Somebody wants our dragged data
long FXDirList::onDNDRequest(FXObject* sender,FXSelector sel,void* ptr){

  // Perhaps the target wants to supply its own data
  if(FXTreeList::onDNDRequest(sender,sel,ptr)) return 1;

  // Return list of filenames as a uri-list
  if(((FXEvent*)ptr)->target==urilistType){
    setDNDData(FROM_DRAGNDROP,urilistType,dragfiles);
    return 1;
    }

  // Delete selected files
  if(((FXEvent*)ptr)->target==deleteType){
    FXTRACE((100,"Delete files not yet implemented\n"));
    return 1;
    }

  return 0;
  }


// Start a drag operation
long FXDirList::onBeginDrag(FXObject* sender,FXSelector sel,void* ptr){
  if(!FXTreeList::onBeginDrag(sender,sel,ptr)){
    beginDrag(&urilistType,1);
    dragfiles=getSelectedFiles();
    }
  return 1;
  }


// Dragged stuff around
long FXDirList::onDragged(FXObject* sender,FXSelector sel,void* ptr){
  FXEvent* event=(FXEvent*)ptr;
  if(!FXTreeList::onDragged(sender,sel,ptr)){
    FXDragAction action=DRAG_ASK;
    if(event->state&CONTROLMASK) action=DRAG_COPY;
    if(event->state&SHIFTMASK) action=DRAG_MOVE;
    if(event->state&ALTMASK) action=DRAG_LINK;
    handleDrag(event->root_x,event->root_y,action);
    action=didAccept();
    switch(action){
      case DRAG_COPY:
        setDragCursor(getApp()->getDefaultCursor(DEF_DNDCOPY_CURSOR));
        break;
      case DRAG_MOVE:
        setDragCursor(getApp()->getDefaultCursor(DEF_DNDMOVE_CURSOR));
        break;
      case DRAG_LINK:
        setDragCursor(getApp()->getDefaultCursor(DEF_DNDLINK_CURSOR));
        break;
      case DRAG_ASK:
        setDragCursor(getApp()->getDefaultCursor(DEF_DNDASK_CURSOR));
        break;
      default:
        setDragCursor(getApp()->getDefaultCursor(DEF_DNDSTOP_CURSOR));
        break;
      }
    }
  return 1;
  }


// End drag operation
long FXDirList::onEndDrag(FXObject* sender,FXSelector sel,void* ptr){
  if(!FXTreeList::onEndDrag(sender,sel,ptr)){
    endDrag((didAccept()!=DRAG_REJECT));
    setDragCursor(getDefaultCursor());
    dragfiles=FXString::null;
    }
  return 1;
  }

/*******************************************************************************/

// Open up the path down to the given string
long FXDirList::onCmdSetValue(FXObject*,FXSelector,void* ptr){
  setCurrentFile((const FXchar*)ptr);
  return 1;
  }


// Open up the path down to the given string
long FXDirList::onCmdSetStringValue(FXObject*,FXSelector,void* ptr){
  setCurrentFile(*((FXString*)ptr));
  return 1;
  }


// Obtain value of the current item
long FXDirList::onCmdGetStringValue(FXObject*,FXSelector,void* ptr){
  *((FXString*)ptr)=getCurrentFile();
  return 1;
  }


// Toggle hidden files
long FXDirList::onCmdToggleHidden(FXObject*,FXSelector,void*){
  showHiddenFiles(!showHiddenFiles());
  return 1;
  }


// Update toggle hidden files widget
long FXDirList::onUpdToggleHidden(FXObject* sender,FXSelector,void*){
  sender->handle(this,showHiddenFiles()?FXSEL(SEL_COMMAND,ID_CHECK):FXSEL(SEL_COMMAND,ID_UNCHECK),NULL);
  return 1;
  }


// Show hidden files
long FXDirList::onCmdShowHidden(FXObject*,FXSelector,void*){
  showHiddenFiles(true);
  return 1;
  }


// Update show hidden files widget
long FXDirList::onUpdShowHidden(FXObject* sender,FXSelector,void*){
  sender->handle(this,showHiddenFiles()?FXSEL(SEL_COMMAND,ID_CHECK):FXSEL(SEL_COMMAND,ID_UNCHECK),NULL);
  return 1;
  }


// Hide hidden files
long FXDirList::onCmdHideHidden(FXObject*,FXSelector,void*){
  showHiddenFiles(false);
  return 1;
  }


// Update hide hidden files widget
long FXDirList::onUpdHideHidden(FXObject* sender,FXSelector,void*){
  sender->handle(this,showHiddenFiles()?FXSEL(SEL_COMMAND,ID_UNCHECK):FXSEL(SEL_COMMAND,ID_CHECK),NULL);
  return 1;
  }


// Toggle files display
long FXDirList::onCmdToggleFiles(FXObject*,FXSelector,void*){
  showFiles(!showFiles());
  return 1;
  }


// Update toggle files widget
long FXDirList::onUpdToggleFiles(FXObject* sender,FXSelector,void*){
  sender->handle(this,showFiles()?FXSEL(SEL_COMMAND,ID_CHECK):FXSEL(SEL_COMMAND,ID_UNCHECK),NULL);
  return 1;
  }


// Show files
long FXDirList::onCmdShowFiles(FXObject*,FXSelector,void*){
  showFiles(true);
  return 1;
  }


// Update show files widget
long FXDirList::onUpdShowFiles(FXObject* sender,FXSelector,void*){
  sender->handle(this,showFiles()?FXSEL(SEL_COMMAND,ID_CHECK):FXSEL(SEL_COMMAND,ID_UNCHECK),NULL);
  return 1;
  }


// Hide files
long FXDirList::onCmdHideFiles(FXObject*,FXSelector,void*){
  showFiles(false);
  return 1;
  }


// Update hide files widget
long FXDirList::onUpdHideFiles(FXObject* sender,FXSelector,void*){
  sender->handle(this,showFiles()?FXSEL(SEL_COMMAND,ID_UNCHECK):FXSEL(SEL_COMMAND,ID_CHECK),NULL);
  return 1;
  }


// Change pattern
long FXDirList::onCmdSetPattern(FXObject*,FXSelector,void* ptr){
  setPattern((const char*)ptr);
  return 1;
  }


// Update pattern
long FXDirList::onUpdSetPattern(FXObject* sender,FXSelector,void*){
  sender->handle(this,FXSEL(SEL_COMMAND,FXWindow::ID_SETVALUE),(void*)pattern.text());
  return 1;
  }


// Reverse sort order
long FXDirList::onCmdSortReverse(FXObject*,FXSelector,void*){
  if(getSortFunc()==ascending) setSortFunc(descending);
  else if(getSortFunc()==descending) setSortFunc(ascending);
  else if(getSortFunc()==ascendingCase) setSortFunc(descendingCase);
  else if(getSortFunc()==descendingCase) setSortFunc(ascendingCase);
  sortItems();
  return 1;
  }


// Update sender
long FXDirList::onUpdSortReverse(FXObject* sender,FXSelector,void* ptr){
  sender->handle(this,(getSortFunc()==descending || getSortFunc()==descendingCase) ? FXSEL(SEL_COMMAND,ID_CHECK) : FXSEL(SEL_COMMAND,ID_UNCHECK),ptr);
  return 1;
  }


// Toggle case sensitivity
long FXDirList::onCmdSortCase(FXObject*,FXSelector,void*){
  if(getSortFunc()==ascending) setSortFunc(ascendingCase);
  else if(getSortFunc()==descending) setSortFunc(descendingCase);
  else if(getSortFunc()==ascendingCase) setSortFunc(ascending);
  else if(getSortFunc()==descendingCase) setSortFunc(descending);
  sortItems();
  return 1;
  }


// Check if case sensitive
long FXDirList::onUpdSortCase(FXObject* sender,FXSelector,void* ptr){
  sender->handle(this,(getSortFunc()==ascendingCase || getSortFunc()==descendingCase) ? FXSEL(SEL_COMMAND,ID_CHECK) : FXSEL(SEL_COMMAND,ID_UNCHECK),ptr);
  return 1;
  }


/*******************************************************************************/

// Refresh; don't update if user is interacting with the list
long FXDirList::onRefreshTimer(FXObject*,FXSelector,void*){
  if(flags&FLAG_UPDATE){
    scan(false);
    counter=(counter+1)%REFRESHFREQUENCY;
    }
  getApp()->addTimeout(this,ID_REFRESHTIMER,REFRESHINTERVAL);
  return 0;
  }


// Force an immediate update of the list
long FXDirList::onCmdRefresh(FXObject*,FXSelector,void*){
  scan(true);
  return 1;
  }


// Scan items to see if listing is necessary
void FXDirList::scan(FXbool force){
  FXString   pathname;
  FXDirItem *item;
  FXStat     info;

  // Do root first time
  if(!firstitem || force){
    listRootItems();
    sortRootItems();
    }

  // Check all items
  item=(FXDirItem*)firstitem;
  while(item){

    // Is expanded directory?
    if(item->isDirectory() && item->isExpanded()){

      // Get the full path of the item
      pathname=getItemPathname(item);

      // Stat this directory; should not fail as parent has been scanned already
      FXStat::statFile(pathname,info);

      // Get the mod date of the item
      FXTime newdate=info.modified();

      // Forced, date was changed, or failed to get proper date and counter expired
      if(force || (item->date!=newdate) || (counter==0)){

        // And do the refresh
        listChildItems(item);
        sortChildItems(item);

        // Remember when we did this
        item->date=newdate;
        }

      // Go deeper
      if(item->first){
        item=(FXDirItem*)item->first;
        continue;
        }
      }

    // Go up
    while(!item->next && item->parent){
      item=(FXDirItem*)item->parent;
      }

    // Go to next
    item=(FXDirItem*)item->next;
    }
  }


#ifdef WIN32            // Windows flavor

// List root directories
void FXDirList::listRootItems(){
  FXDirItem  *oldlist=list;
  FXDirItem  *newlist=NULL;
  FXDirItem **po=&oldlist;
  FXDirItem **pn=&newlist;
  FXDirItem  *item;
  FXDirItem  *link;
  FXString    name;
  FXuint      mask;

  // Loop over drive letters
  for(mask=GetLogicalDrives(),name="A:\\"; mask; mask>>=1,name[0]++){

    // Skip unavailable drives
    if(!(mask&1)) continue;

    // Find it, and take it out from the old list if found
    for(FXDirItem** pp=po; (item=*pp)!=NULL; pp=&item->link){
      if(comparecase(item->label,name)==0){
        *pp=item->link; item->link=NULL;
        goto fnd;
        }
      }

    // Not found; prepend before list
    item=(FXDirItem*)appendItem(NULL,name,opendiricon,closeddiricon,NULL,true);

    // Next gets hung after this one
fnd:*pn=item; pn=&item->link;

    // Update item information
    item->setHasItems(true);
    item->assoc=NULL;
    item->size=0L;
    item->date=0;
    item->mode=FXIO::Directory;

    // Assume no associations
    switch(GetDriveTypeA(name.text())){
      case DRIVE_REMOVABLE:
        if(name[0]=='A' || name[0]=='B'){
          item->setOpenIcon(floppyicon);
          item->setClosedIcon(floppyicon);
          }
        else{
          item->setOpenIcon(zipdiskicon);
          item->setClosedIcon(zipdiskicon);
          }
        break;
      case DRIVE_REMOTE:
        item->setOpenIcon(networkicon);
        item->setClosedIcon(networkicon);
        break;
      case DRIVE_CDROM:
        item->setOpenIcon(cdromicon);
        item->setClosedIcon(cdromicon);
        break;
      case DRIVE_RAMDISK:
        item->setOpenIcon(opendiricon);
        item->setClosedIcon(closeddiricon);
        break;
      case DRIVE_FIXED:
        item->setOpenIcon(harddiskicon);
        item->setClosedIcon(harddiskicon);
        break;
      case DRIVE_UNKNOWN:
      case DRIVE_NO_ROOT_DIR:
      default:
        item->setOpenIcon(opendiricon);
        item->setClosedIcon(closeddiricon);
        break;
      }

    // If association is found, use it
    if(associations) item->assoc=associations->findDirBinding(name.text());
    if(item->assoc){
      if(item->assoc->miniicon) item->setClosedIcon(item->assoc->miniicon);
      if(item->assoc->miniiconopen) item->setOpenIcon(item->assoc->miniiconopen);
      }

    // Create item
    if(id()) item->create();
    }

  // Wipe items remaining in list:- they have disappeared!!
  for(item=oldlist; item; item=link){
    link=item->link;
    removeItem(item,true);
    }

  // Remember new list
  list=newlist;
  }

#else                   // UNIX flavor

// List root directories
void FXDirList::listRootItems(){
  FXDirItem *item=(FXDirItem*)firstitem;

  // First time, make root node
  if(!item) item=list=(FXDirItem*)appendItem(NULL,PATHSEPSTRING,harddiskicon,harddiskicon,NULL,true);

  // Update item information
  item->setHasItems(true);
  item->setOpenIcon(harddiskicon);
  item->setClosedIcon(harddiskicon);
  item->assoc=NULL;
  item->size=0L;
  item->date=0;
  item->mode=FXIO::Directory;

  // If association is found, use it
  if(associations) item->assoc=associations->findDirBinding(PATHSEPSTRING);
  if(item->assoc){
    if(item->assoc->miniicon) item->setClosedIcon(item->assoc->miniicon);
    if(item->assoc->miniiconopen) item->setOpenIcon(item->assoc->miniiconopen);
    }

  // Create item
  if(id()) item->create();

  // Need to layout
  recalc();
  }



#endif


/*******************************************************************************/

// FIXME make like FXFileList in terms of logic

// List child items
void FXDirList::listChildItems(FXDirItem *par){
  FXDirItem   *oldlist=par->list;
  FXDirItem   *newlist=NULL;
  FXDirItem  **po=&oldlist;
  FXDirItem  **pn=&newlist;
  FXDirItem   *item;
  FXDirItem   *link;
  FXString     pathname;
  FXString     directory;
  FXString     name;
  FXStat       info;
  FXDir        dir;
  FXuint       mode;

  // Path to parent node
  directory=getItemPathname(par);

  // Managed to open directory
  if(dir.open(directory)){

    // Process directory entries
    while(dir.next(name)){

      // A dot special file?
      if(name[0]=='.' && (name[1]==0 || (name[1]=='.' && name[2]==0))) continue;

      // Hidden file or directory normally not shown
      if(name[0]=='.' && !(options&DIRLIST_SHOWHIDDEN)) continue;

      // Build full pathname of entry
      pathname=directory;
      if(!ISPATHSEP(pathname.tail())) pathname+=PATHSEPSTRING;
      pathname+=name;

#ifdef WIN32

      // Get file/link info
      if(!FXStat::statFile(pathname,info)) continue;

      mode=info.mode();

      // Hidden file or directory normally not shown
      if((mode&FXIO::Hidden) && !(options&DIRLIST_SHOWHIDDEN)) continue;

#else

      // Get file/link info
      if(!FXStat::statLink(pathname,info)) continue;

      mode=info.mode();

      // Check if link is a link to a directory
      if((mode&FXIO::SymLink) && FXStat::isDirectory(pathname)) mode|=FXIO::Directory;

#endif

      // If it is not a directory, and not showing files and matching pattern skip it
      if(!(mode&FXIO::Directory) && !((options&DIRLIST_SHOWFILES) && FXPath::match(name,pattern,matchmode))) continue;

      // Find it, and take it out from the old list if found
      for(FXDirItem** pp=po; (item=*pp)!=NULL; pp=&item->link){
        if(compare(item->label,name)==0){
          *pp=item->link; item->link=NULL;
          goto fnd;
          }
        }

      // Not found; prepend before list
      item=(FXDirItem*)appendItem(par,name,opendiricon,closeddiricon,NULL,true);

      // Next gets hung after this one
fnd:  *pn=item; pn=&item->link;

      // Update item information
      item->setDraggable(draggable);
      item->setAssoc(NULL);
      item->setSize(info.size());
      item->setDate(info.modified());
      item->setMode(mode);

      // Determine icons and type
      if(item->isDirectory()){
        item->setHasItems(true);
        item->setOpenIcon(opendiricon);
        item->setClosedIcon(closeddiricon);
        if(associations) item->setAssoc(associations->findDirBinding(pathname.text()));
        }
      else if(item->isExecutable()){
        item->setHasItems(false);
        item->setOpenIcon(applicationicon);
        item->setClosedIcon(applicationicon);
        if(associations) item->setAssoc(associations->findExecBinding(pathname.text()));
        }
      else{
        item->setHasItems(false);
        item->setOpenIcon(documenticon);
        item->setClosedIcon(documenticon);
        if(associations) item->setAssoc(associations->findFileBinding(pathname.text()));
        }

      // If association is found, use it
      if(item->getAssoc()){
        if(item->getAssoc()->miniicon) item->setClosedIcon(item->getAssoc()->miniicon);
        if(item->getAssoc()->miniiconopen) item->setOpenIcon(item->getAssoc()->miniiconopen);
        }

      // Create item
      if(id()) item->create();
      }

    // Close it
    dir.close();
    }

  // Wipe items remaining in list:- they have disappeared!!
  for(item=oldlist; item; item=link){
    link=item->link;
    removeItem(item,true);
    }

  // Now we know for sure whether we really have subitems or not
  if(par->first)
    par->state|=FXDirItem::HASITEMS;
  else
    par->state&=~FXDirItem::HASITEMS;

  // Remember new list
  par->list=newlist;

  // Need to layout
  recalc();
  }

/*******************************************************************************/

// Set current (dir/file) name path
void FXDirList::setCurrentFile(const FXString& pathname,FXbool notify){
  FXTRACE((100,"%s::setCurrentFile(%s)\n",getClassName(),pathname.text()));
  if(!pathname.empty()){
    FXTreeItem* item;
    FXbool expand=false;
    FXString path=FXPath::absolute(pathname);
    while(!FXPath::isTopDirectory(path) && !FXStat::exists(path)){
      path=FXPath::upLevel(path);
      expand=true;
      }
    if((item=getPathnameItem(path))!=NULL){
      if(expand) expandTree(item,notify);
      setAnchorItem(item);
      setCurrentItem(item,notify);
      makeItemVisible(item);
      }
    }
  }


// Get current (dir/file) name path
FXString FXDirList::getCurrentFile() const {
  return getItemPathname(currentitem);
  }


// Return uri-list of selected files
FXString FXDirList::getSelectedFiles() const {
  register FXTreeItem *item=getFirstItem();
  FXString result;
  while(item){
    if(isItemSelected(item)){
      result.append(FXURL::fileToURL(getItemPathname(item)));
      result.append("\r\n");
      }
    if(item->getFirst()){
      item=item->getFirst();
      }
    else{
      while(!item->getNext() && item->getParent()) item=item->getParent();
      item=item->getNext();
      }
    }
  return result;
  }


// Open all intermediate directories down toward given one
void FXDirList::setDirectory(const FXString& pathname,FXbool notify){
  FXTRACE((100,"%s::setDirectory(%s)\n",getClassName(),pathname.text()));
  if(!pathname.empty()){
    FXTreeItem* item;
    FXString path=FXPath::absolute(pathname);
    while(!FXPath::isTopDirectory(path) && !FXStat::isDirectory(path)){
      path=FXPath::upLevel(path);
      }
    if((item=getPathnameItem(path))!=NULL){
      expandTree(item,notify);
      setAnchorItem(item);
      setCurrentItem(item,notify);
      makeItemVisible(item);
      }
    }
  }


// Return directory part of path to current item
FXString FXDirList::getDirectory() const {
  const FXTreeItem* item=currentitem;
  while(item){
    if(((FXDirItem*)item)->isDirectory()) return getItemPathname(item);
    item=item->parent;
    }
  return FXString::null;
  }


// Set the pattern to filter
void FXDirList::setPattern(const FXString& ptrn){
  if(ptrn.empty()) return;
  if(pattern!=ptrn){
    pattern=ptrn;
    if(getFirst()) scan(true);
    }
  }

/*******************************************************************************/

// Return absolute pathname of item
FXString FXDirList::getItemPathname(const FXTreeItem* item) const {
  FXString pathname;
  if(item){
    while(1){
      pathname.prepend(item->getText());
      item=item->parent;
      if(!item) break;
      if(item->parent) pathname.prepend(PATHSEP);
      }
    }
  return pathname;
  }


// Return the item from the absolute pathname
FXTreeItem* FXDirList::getPathnameItem(const FXString& path){
  register FXTreeItem *item,*it;
  register FXint beg=0,end=0;
  FXString name;
  if(!path.empty()){
#ifdef WIN32
    if(ISPATHSEP(path[0])){
      end++;
      if(ISPATHSEP(path[1])) end++;
      }
    else if(Ascii::isLetter((FXuchar)path[0]) && path[1]==':'){
      end+=2;
      if(ISPATHSEP(path[2])) end++;
      }
#else
    if(ISPATHSEP(path[0])) end++;
#endif
    if(beg<end){
      name=path.mid(beg,end-beg);
      for(it=firstitem; it; it=it->next){
#ifdef WIN32
        if(comparecase(name,it->getText())==0) goto x;
#else
        if(compare(name,it->getText())==0) goto x;
#endif
        }
      listRootItems();
      sortRootItems();
      for(it=firstitem; it; it=it->next){
#ifdef WIN32
        if(comparecase(name,it->getText())==0) goto x;
#else
        if(compare(name,it->getText())==0) goto x;
#endif
        }
      return NULL;
x:    item=it;
      FXASSERT(item);
      while(end<path.length()){
        beg=end;
        while(end<path.length() && !ISPATHSEP(path[end])) end++;
        name=path.mid(beg,end-beg);
        for(it=item->first; it; it=it->next){
#ifdef WIN32
          if(comparecase(name,it->getText())==0) goto y;
#else
          if(compare(name,it->getText())==0) goto y;
#endif
          }
        listChildItems((FXDirItem*)item);
        sortChildItems(item);
        for(it=item->first; it; it=it->next){
#ifdef WIN32
          if(comparecase(name,it->getText())==0) goto y;
#else
          if(compare(name,it->getText())==0) goto y;
#endif
          }
        return item;
y:      item=it;
        FXASSERT(item);
        if(end<path.length() && ISPATHSEP(path[end])) end++;
        }
      FXASSERT(item);
      return item;
      }
    }
  return NULL;
  }


// Is file
FXbool FXDirList::isItemFile(const FXTreeItem* item) const {
  return item && ((FXDirItem*)item)->isFile();
  }


// Is directory
FXbool FXDirList::isItemDirectory(const FXTreeItem* item) const {
  return item && ((FXDirItem*)item)->isDirectory();
  }


// Is executable
FXbool FXDirList::isItemExecutable(const FXTreeItem* item) const {
  return item && ((FXDirItem*)item)->isExecutable();
  }


// Return true if this is a symbolic link item
FXbool FXDirList::isItemSymlink(const FXTreeItem* item) const {
  return item && ((FXDirItem*)item)->isSymlink();
  }


// Return file association of item
FXFileAssoc* FXDirList::getItemAssoc(const FXTreeItem* item) const {
  return item ? ((FXDirItem*)item)->getAssoc() : NULL;
  }


// Return the file size for this item
FXlong FXDirList::getItemSize(const FXTreeItem* item) const {
  return item ? ((FXDirItem*)item)->getSize() : 0;
  }


// Return the date for this item, in nanoseconds
FXTime FXDirList::getItemDate(const FXTreeItem* item) const {
  return item ? ((FXDirItem*)item)->getDate() : 0;
  }


// Return the mode bits for this item
FXuint FXDirList::getItemMode(const FXTreeItem* item) const {
  return item ? ((FXDirItem*)item)->getMode() : 0;
  }


/*******************************************************************************/

// Expand tree
FXbool FXDirList::expandTree(FXTreeItem* tree,FXbool notify){
  if(FXTreeList::expandTree(tree,notify)){
    if(isItemDirectory(tree)){
      listChildItems((FXDirItem*)tree);
      sortChildItems(tree);
      }
    return true;
    }
  return false;
  }


// Collapse tree
FXbool FXDirList::collapseTree(FXTreeItem* tree,FXbool notify){
  if(FXTreeList::collapseTree(tree,notify)){
    if(isItemDirectory(tree)){
      removeItems(tree->getFirst(),tree->getLast(),notify);
      ((FXDirItem*)tree)->list=NULL;
      }
    return true;
    }
  return false;
  }


// Change file match mode
void FXDirList::setMatchMode(FXuint mode){
  if(matchmode!=mode){
    matchmode=mode;
    if(getFirst()) scan(true);
    }
  }


// Get list style
FXbool FXDirList::showFiles() const {
  return (options&DIRLIST_SHOWFILES)!=0;
  }


// Change list style
void FXDirList::showFiles(FXbool flag){
  FXuint opts=(((0-flag)^options)&DIRLIST_SHOWFILES)^options;
  if(options!=opts){
    options=opts;
    if(getFirst()) scan(true);
    }
  }


// Return true if showing hidden files
FXbool FXDirList::showHiddenFiles() const {
  return (options&DIRLIST_SHOWHIDDEN)!=0;
  }


// Change show hidden files mode
void FXDirList::showHiddenFiles(FXbool flag){
  FXuint opts=(((0-flag)^options)&DIRLIST_SHOWHIDDEN)^options;
  if(opts!=options){
    options=opts;
    if(getFirst()) scan(true);
    }
  }


// Change file associations; delete the old one unless it was shared
void FXDirList::setAssociations(FXFileDict* assocs,FXbool owned){
  FXuint opts=options;
  options^=((owned-1)^options)&DIRLIST_NO_OWN_ASSOC;
  if(associations!=assocs){
    if(!(opts&DIRLIST_NO_OWN_ASSOC)) delete associations;
    associations=assocs;
    if(getFirst()) scan(true);
    }
  }


// Set draggable files
void FXDirList::setDraggableFiles(FXbool flag){
  if(draggable!=flag){
    draggable=flag;
    if(getFirst()) scan(true);
    }
  }


// Save data
void FXDirList::save(FXStream& store) const {
  FXTreeList::save(store);
  store << associations;
  store << opendiricon;
  store << closeddiricon;
  store << documenticon;
  store << applicationicon;
  store << cdromicon;
  store << harddiskicon;
  store << networkicon;
  store << floppyicon;
  store << zipdiskicon;
  store << pattern;
  store << matchmode;
  store << draggable;
  }


// Load data
void FXDirList::load(FXStream& store){
  FXTreeList::load(store);
  store >> associations;
  store >> opendiricon;
  store >> closeddiricon;
  store >> documenticon;
  store >> applicationicon;
  store >> cdromicon;
  store >> harddiskicon;
  store >> networkicon;
  store >> floppyicon;
  store >> zipdiskicon;
  store >> pattern;
  store >> matchmode;
  store >> draggable;
  }


// Cleanup
FXDirList::~FXDirList(){
  clearItems();
  getApp()->removeChore(this);
  getApp()->removeTimeout(this,ID_REFRESHTIMER);
  if(!(options&DIRLIST_NO_OWN_ASSOC)) delete associations;
  delete opendiricon;
  delete closeddiricon;
  delete documenticon;
  delete applicationicon;
  delete cdromicon;
  delete harddiskicon;
  delete networkicon;
  delete floppyicon;
  delete zipdiskicon;
  associations=(FXFileDict*)-1L;
  opendiricon=(FXGIFIcon*)-1L;
  closeddiricon=(FXGIFIcon*)-1L;
  documenticon=(FXGIFIcon*)-1L;
  applicationicon=(FXGIFIcon*)-1L;
  cdromicon=(FXIcon*)-1L;
  harddiskicon=(FXIcon*)-1L;
  networkicon=(FXIcon*)-1L;
  floppyicon=(FXIcon*)-1L;
  zipdiskicon=(FXIcon*)-1L;
  list=(FXDirItem*)-1L;
  }

}