/********************************************************************************
*                                                                               *
*              T h e   P a t h F i n d e r   F i l e   B r o w s e r            *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998,2000 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: PathFinder.cpp,v 1.57 2002/01/14 05:52:05 jeroen Exp $                   *
********************************************************************************/
#include "xincs.h"
#include "fx.h"
#include "fxkeys.h"
#include "PathFinder.h"
#include "PropertyDialog.h"
#include "icons.h"
#include <stdio.h>
#include <stdlib.h>


/*
  Notes:

  - Copy / Paste /Cut doesn't seem to work
  - If you drag some files to a certain directory in the dir-list, hilight the directory so
    we're sure we are dropping it in the right directory...
  - Settings dialog like the one in TextEdit....
  - Edit menu layout should change:

      Undo
      Redo
      separator
      Cut
      Copy
      Paste
      Delete
      separator
      Select
      DeSelect
      Invert Selection


  - Mount/Unmount functionality....
  - A special bookmark button like home/work for /mnt/cdrom or/and just the /mnt directory
  - Selecting multiple files and clicking Open With only displays the first file ...
    not the whole range you've selected.... mayby we should show the first and the last
    file of the selection: (file1.htm ... file99.htm)
  - Change 'Delete Files' dynamically  depending on the amount of files you've selected:
    so 1 file shows only Delete File....  same thing for the dialog that shows up....
  - Time in statusbar as in TextEdit

*/

/*******************************************************************************/

// Map
FXDEFMAP(PathFinderMain) PathFinderMainMap[]={
  FXMAPFUNC(SEL_COMMAND,PathFinderMain::ID_QUIT,PathFinderMain::onCmdQuit),
  FXMAPFUNC(SEL_SIGNAL,PathFinderMain::ID_QUIT,PathFinderMain::onCmdQuit),
  FXMAPFUNC(SEL_CLOSE,PathFinderMain::ID_MAINWINDOW,PathFinderMain::onCmdQuit),
  FXMAPFUNC(SEL_UPDATE,PathFinderMain::ID_MAINWINDOW,PathFinderMain::onUpdTitle),
  FXMAPFUNC(SEL_COMMAND,PathFinderMain::ID_DIRECTORYLIST,PathFinderMain::onCmdDirectory),
  FXMAPFUNC(SEL_CLICKED,PathFinderMain::ID_FILELIST,PathFinderMain::onCmdFileClicked),
  FXMAPFUNC(SEL_DOUBLECLICKED,PathFinderMain::ID_FILELIST,PathFinderMain::onCmdFileDblClicked),
  FXMAPFUNC(SEL_RIGHTBUTTONRELEASE,PathFinderMain::ID_FILELIST,PathFinderMain::onFileListPopup),
  FXMAPFUNC(SEL_COMMAND,PathFinderMain::ID_ABOUT,PathFinderMain::onCmdAbout),
  FXMAPFUNC(SEL_COMMAND,PathFinderMain::ID_DIRBOX,PathFinderMain::onCmdDirTree),
  FXMAPFUNC(SEL_COMMAND,PathFinderMain::ID_SEARCHPATH,PathFinderMain::onCmdSetSearchPath),
  FXMAPFUNC(SEL_COMMAND,PathFinderMain::ID_UPDIRECTORY,PathFinderMain::onCmdUpDirectory),
  FXMAPFUNC(SEL_UPDATE,PathFinderMain::ID_UPDATE_FILES,PathFinderMain::onUpdFiles),
  FXMAPFUNC(SEL_UPDATE,PathFinderMain::ID_STATUSLINE,PathFinderMain::onUpdStatusline),
  FXMAPFUNC(SEL_COMMAND,PathFinderMain::ID_GO_WORK,PathFinderMain::onCmdWorkDirectory),
  FXMAPFUNC(SEL_COMMAND,PathFinderMain::ID_GO_HOME,PathFinderMain::onCmdHomeDirectory),
  FXMAPFUNC(SEL_COMMAND,PathFinderMain::ID_GO_RECENT,PathFinderMain::onCmdRecentDirectory),
  FXMAPFUNC(SEL_COMMAND,PathFinderMain::ID_GO_BACK,PathFinderMain::onCmdBackDirectory),
  FXMAPFUNC(SEL_UPDATE,PathFinderMain::ID_GO_BACK,PathFinderMain::onUpdBackDirectory),
  FXMAPFUNC(SEL_COMMAND,PathFinderMain::ID_GO_FORWARD,PathFinderMain::onCmdForwardDirectory),
  FXMAPFUNC(SEL_UPDATE,PathFinderMain::ID_GO_FORWARD,PathFinderMain::onUpdForwardDirectory),
  FXMAPFUNC(SEL_COMMAND,PathFinderMain::ID_SAVE_SETTINGS,PathFinderMain::onCmdSaveSettings),
  FXMAPFUNC(SEL_COMMAND,PathFinderMain::ID_NEW_PATHFINDER,PathFinderMain::onCmdNewPathFinder),
  FXMAPFUNC(SEL_COMMAND,PathFinderMain::ID_PROPERTIES,PathFinderMain::onCmdProperties),
  FXMAPFUNC(SEL_UPDATE,PathFinderMain::ID_PROPERTIES,PathFinderMain::onUpdProperties),
  FXMAPFUNC(SEL_COMMAND,PathFinderMain::ID_BOOKMARK,PathFinderMain::onCmdBookmark),
  FXMAPFUNC(SEL_COMMAND,PathFinderMain::ID_DELETE,PathFinderMain::onCmdDelete),
  FXMAPFUNC(SEL_UPDATE,PathFinderMain::ID_DELETE,PathFinderMain::onUpdDelete),
  FXMAPFUNC(SEL_COMMAND,PathFinderMain::ID_NEW,PathFinderMain::onCmdNew),
  FXMAPFUNC(SEL_UPDATE,PathFinderMain::ID_NEW,PathFinderMain::onUpdNew),
  FXMAPFUNC(SEL_COMMAND,PathFinderMain::ID_FILEPATTERNS,PathFinderMain::onCmdFilePatterns),
  FXMAPFUNC(SEL_COMMAND,PathFinderMain::ID_FILEFILTER,PathFinderMain::onCmdFilter),
  FXMAPFUNC(SEL_COMMAND,PathFinderMain::ID_GOTO_DIR,PathFinderMain::onCmdGotoDir),
  FXMAPFUNCS(SEL_COMMAND,PathFinderMain::ID_RUSR,PathFinderMain::ID_SVTX,PathFinderMain::onCmdChmod),
  FXMAPFUNCS(SEL_UPDATE,PathFinderMain::ID_RUSR,PathFinderMain::ID_SVTX,PathFinderMain::onUpdChmod),
  FXMAPFUNC(SEL_UPDATE,PathFinderMain::ID_OWNER,PathFinderMain::onUpdOwner),
  FXMAPFUNC(SEL_UPDATE,PathFinderMain::ID_GROUP,PathFinderMain::onUpdGroup),
  FXMAPFUNC(SEL_UPDATE,PathFinderMain::ID_FILE_CREATED,PathFinderMain::onUpdCreateTime),
  FXMAPFUNC(SEL_UPDATE,PathFinderMain::ID_FILE_MODIFIED,PathFinderMain::onUpdModifyTime),
  FXMAPFUNC(SEL_UPDATE,PathFinderMain::ID_FILE_ACCESSED,PathFinderMain::onUpdAccessTime),
  FXMAPFUNC(SEL_UPDATE,PathFinderMain::ID_FILE_TYPE,PathFinderMain::onUpdFileType),
  FXMAPFUNC(SEL_UPDATE,PathFinderMain::ID_FILE_LOCATION,PathFinderMain::onUpdFileLocation),
  FXMAPFUNC(SEL_UPDATE,PathFinderMain::ID_FILE_SIZE,PathFinderMain::onUpdFileSize),
  FXMAPFUNC(SEL_UPDATE,PathFinderMain::ID_FILE_DESC,PathFinderMain::onUpdFileDesc),
  FXMAPFUNC(SEL_UPDATE,PathFinderMain::ID_OPEN,PathFinderMain::onUpdOpen),
  FXMAPFUNC(SEL_COMMAND,PathFinderMain::ID_OPEN,PathFinderMain::onCmdOpen),
  FXMAPFUNC(SEL_UPDATE,PathFinderMain::ID_OPEN_WITH,PathFinderMain::onUpdOpenWith),
  FXMAPFUNC(SEL_COMMAND,PathFinderMain::ID_OPEN_WITH,PathFinderMain::onCmdOpenWith),
  FXMAPFUNC(SEL_COMMAND,PathFinderMain::ID_RUN,PathFinderMain::onCmdRun),
  FXMAPFUNC(SEL_COMMAND,PathFinderMain::ID_TERMINAL,PathFinderMain::onCmdTerminal),
  FXMAPFUNC(SEL_SIGNAL,PathFinderMain::ID_HARVEST,PathFinderMain::onSigHarvest),
  };


// Object implementation
FXIMPLEMENT(PathFinderMain,FXMainWindow,PathFinderMainMap,ARRAYNUMBER(PathFinderMainMap))

/*******************************************************************************/


// Executable for another PathFinder
FXchar* PathFinderMain::pathfindercommand;

/*******************************************************************************/

// Make some windows
PathFinderMain::PathFinderMain(FXApp* a):
  FXMainWindow(a,"PathFinder",NULL,NULL,DECOR_ALL,0,0,800,600,0,0),
  bookmarkeddirs("Bookmarked Directories"){

  // Make some icons
  bigp=new FXBMPIcon(getApp(),bigpenguin,0,IMAGE_ALPHAGUESS);
  minip=new FXBMPIcon(getApp(),minipenguin,0,IMAGE_ALPHAGUESS);
  cuticon=new FXBMPIcon(getApp(),cut,0,IMAGE_ALPHAGUESS);
  copyicon=new FXGIFIcon(getApp(),copyit,FXRGB(178,192,220),IMAGE_ALPHACOLOR);
  moveicon=new FXGIFIcon(getApp(),moveit,FXRGB(178,192,220),IMAGE_ALPHACOLOR);
  pasteicon=new FXBMPIcon(getApp(),paste,0,IMAGE_ALPHAGUESS);
  upicon=new FXBMPIcon(getApp(),dirup,0,IMAGE_ALPHAGUESS);
  homeicon=new FXGIFIcon(getApp(),home);
  backicon=new FXBMPIcon(getApp(),goback,0,IMAGE_ALPHAGUESS);
  forwicon=new FXBMPIcon(getApp(),goforw,0,IMAGE_ALPHAGUESS);
  bigiconsicon=new FXBMPIcon(getApp(),bigicons,0,IMAGE_ALPHAGUESS);
  miniiconsicon=new FXBMPIcon(getApp(),smallicons,0,IMAGE_ALPHAGUESS);
  detailsicon=new FXBMPIcon(getApp(),details,0,IMAGE_ALPHAGUESS);
  mapicon=new FXBMPIcon(getApp(),maphost,0,IMAGE_ALPHAGUESS);
  unmapicon=new FXBMPIcon(getApp(),unmaphost,0,IMAGE_ALPHAGUESS);
  propicon=new FXBMPIcon(getApp(),properties,0,IMAGE_ALPHAGUESS);
  deleteicon=new FXBMPIcon(getApp(),deleteit,0,IMAGE_ALPHAGUESS);
  setbookicon=new FXGIFIcon(getApp(),setbook);
  clrbookicon=new FXGIFIcon(getApp(),clrbook);
  workicon=new FXGIFIcon(getApp(),work);

  // Set application icons for Window Manager
  setIcon(bigp);
  setMiniIcon(minip);

  // Make main window; set myself as the target
  setTarget(this);
  setSelector(ID_MAINWINDOW);

  // Make menu bar
  FXMenubar *menubar=new FXMenubar(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X,0,0,0,0, 3,3,3,3, 0,0);

  // Separator
  new FXHorizontalSeparator(this,LAYOUT_SIDE_TOP|SEPARATOR_GROOVE|LAYOUT_FILL_X);

  // Make Tool bar
  FXToolbar *toolbar=new FXToolbar(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X,0,0,0,0, 3,3,3,3, 0,0);

  // Properties panel
  property=new PropertyDialog(this);

  // File menu pane
  filemenu=new FXMenuPane(this);
  new FXMenuCommand(filemenu,"Open\t\tOpen selected file.",NULL,this,ID_OPEN);
  new FXMenuCommand(filemenu,"Open with...\t\tOpen selected file using program.",NULL,this,ID_OPEN_WITH);
  new FXMenuSeparator(filemenu);
  new FXMenuCommand(filemenu,"&Run...\tCtl-R\tRun a command.",NULL,this,ID_RUN);
  new FXMenuCommand(filemenu,"&Terminal...\tCtl-T\tOpen Terminal.",NULL,this,ID_TERMINAL);
  new FXMenuSeparator(filemenu);
  new FXMenuCommand(filemenu,"&New Directory...",NULL,this,ID_NEW);
  new FXMenuCommand(filemenu,"Delete\tDel\tDelete the selection.",deleteicon,this,ID_DELETE);
  new FXMenuSeparator(filemenu);
  new FXMenuCommand(filemenu,"New &PathFinder...\tCtl-P\tStart another PathFinder.",minip,this,ID_NEW_PATHFINDER);
  new FXMenuCommand(filemenu,"&Quit\tCtl-Q\tQuit PathFinder",NULL,this,ID_QUIT);

  // Command Menu Pane
  new FXMenuTitle(menubar,"&File",NULL,filemenu);

  // Status bar
  FXStatusbar *status=new FXStatusbar(this,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|STATUSBAR_WITH_DRAGCORNER);
  FXStatusline *statusline=status->getStatusline();
  statusline->setTarget(this);
  statusline->setSelector(ID_STATUSLINE);

  // Subtle plug for LINUX
  new FXButton(status,"\tHello, I'm Tux...\nThe symbol for the Linux Operating System.\nAnd all it stands for.\tLinux:- The Only Operating System.",minip,this,ID_ABOUT,LAYOUT_TOP|LAYOUT_RIGHT);

  // Make file associations object; shared between FXFileList and FXDirList
  associations=new FXFileDict(getApp());

  // Main window interior
  FXSplitter* splitter=new FXSplitter(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y|SPLITTER_TRACKING);
  group1=new FXVerticalFrame(splitter,LAYOUT_FILL_Y|LAYOUT_FILL_Y, 0,0,180,0, 0,0,0,0);
  group2=new FXVerticalFrame(splitter,LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);


  // Directories
  FXLabel* folders=new FXLabel(group1,"Folders",NULL,JUSTIFY_LEFT|LAYOUT_TOP|LAYOUT_FILL_X|FRAME_GROOVE);
  subgroup1=new FXVerticalFrame(group1,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);

  // Files
  FXLabel* files=new FXLabel(group2,"Files",NULL,JUSTIFY_LEFT|LAYOUT_TOP|LAYOUT_FILL_X|FRAME_GROOVE);
  files->setTarget(this);
  files->setSelector(ID_UPDATE_FILES);
  subgroup2=new FXVerticalFrame(group2,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0);

  // Directory List on the left
  dirlist=new FXDirList(subgroup1,0,this,ID_DIRECTORYLIST,LAYOUT_FILL_X|LAYOUT_FILL_Y|LAYOUT_TOP|LAYOUT_RIGHT|TREELIST_SHOWS_LINES|TREELIST_SHOWS_BOXES|TREELIST_BROWSESELECT|DIRLIST_NO_OWN_ASSOC);
  dirlist->setAssociations(associations);

  // Icon list on the right
  filelist=new FXFileList(subgroup2,this,ID_FILELIST,LAYOUT_FILL_X|LAYOUT_FILL_Y|ICONLIST_BIG_ICONS|ICONLIST_AUTOSIZE|FILELIST_NO_OWN_ASSOC);
  filelist->setAssociations(associations);

  // Make some icons

  // Edit Menu Pane
  editmenu=new FXMenuPane(this);
  new FXMenuCommand(editmenu,"Cu&t\tCtl-X\tCut to clipboard.",cuticon,NULL,0);
  new FXMenuCommand(editmenu,"&Copy\tCtl-C\tCopy to clipboard.",copyicon,NULL,0);
  new FXMenuCommand(editmenu,"&Paste\tCtl-V\tPaste from clipboard.",pasteicon,NULL,0);
  new FXMenuSeparator(editmenu);
  new FXMenuCommand(editmenu,"&Select All\tCtl-A\tSelect all icons",NULL,filelist,FXFileList::ID_SELECT_ALL);
  new FXMenuCommand(editmenu,"&Deselect All\t\tDeselect all icons",NULL,filelist,FXFileList::ID_DESELECT_ALL);
  new FXMenuCommand(editmenu,"&Invert Selection\t\tInvert selection",NULL,filelist,FXFileList::ID_SELECT_INVERSE);
  new FXMenuTitle(menubar,"&Edit",NULL,editmenu);


  // Pattern
  pattern=new FXComboBox(status,25,4,this,ID_FILEFILTER,COMBOBOX_STATIC|LAYOUT_RIGHT|LAYOUT_FIX_WIDTH|FRAME_SUNKEN|FRAME_THICK, 0,0,200,0, 0,0,1,1);

  // Caption before pattern
  new FXLabel(status,"Pattern:",NULL,LAYOUT_RIGHT|LAYOUT_CENTER_Y);

  // Spacer
  new FXFrame(toolbar,LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FIX_WIDTH,0,0,8,0);

  // Directory box
  dirbox=new FXDirBox(toolbar,5,this,ID_DIRBOX,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FIX_WIDTH|LAYOUT_CENTER_Y,0,0,180,0, 0,0, 1,1);

  // Spacer
  new FXFrame(toolbar,LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FIX_WIDTH,0,0,8,0);

  bookmarkmenu=new FXMenuPane(this,POPUP_SHRINKWRAP);
  new FXMenuCommand(bookmarkmenu,"&Bookmark...\t\tBookmark current directory.",setbookicon,this,ID_BOOKMARK);
  new FXMenuCommand(bookmarkmenu,"&Clear...\t\tClear bookmarks.",clrbookicon,&bookmarkeddirs,FXRecentFiles::ID_CLEAR);
  FXMenuSeparator* sep2=new FXMenuSeparator(bookmarkmenu);
  sep2->setTarget(&bookmarkeddirs);
  sep2->setSelector(FXRecentFiles::ID_ANYFILES);
  new FXMenuCommand(bookmarkmenu,NULL,NULL,&bookmarkeddirs,FXRecentFiles::ID_FILE_1);
  new FXMenuCommand(bookmarkmenu,NULL,NULL,&bookmarkeddirs,FXRecentFiles::ID_FILE_2);
  new FXMenuCommand(bookmarkmenu,NULL,NULL,&bookmarkeddirs,FXRecentFiles::ID_FILE_3);
  new FXMenuCommand(bookmarkmenu,NULL,NULL,&bookmarkeddirs,FXRecentFiles::ID_FILE_4);
  new FXMenuCommand(bookmarkmenu,NULL,NULL,&bookmarkeddirs,FXRecentFiles::ID_FILE_5);
  new FXMenuCommand(bookmarkmenu,NULL,NULL,&bookmarkeddirs,FXRecentFiles::ID_FILE_6);
  new FXMenuCommand(bookmarkmenu,NULL,NULL,&bookmarkeddirs,FXRecentFiles::ID_FILE_7);
  new FXMenuCommand(bookmarkmenu,NULL,NULL,&bookmarkeddirs,FXRecentFiles::ID_FILE_8);
  new FXMenuCommand(bookmarkmenu,NULL,NULL,&bookmarkeddirs,FXRecentFiles::ID_FILE_9);
  new FXMenuCommand(bookmarkmenu,NULL,NULL,&bookmarkeddirs,FXRecentFiles::ID_FILE_10);


  // Add some toolbar buttons
  new FXButton(toolbar,"\tUp\tChange up one level.",upicon,this,ID_UPDIRECTORY,BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  new FXButton(toolbar,"\tHome\tChange to home directory.",homeicon,this,ID_GO_HOME,BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  new FXButton(toolbar,"\tWork\tChange to current working directory.",workicon,this,ID_GO_WORK,BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  new FXMenuButton(toolbar,"\tBookmarks\tVisit bookmarked directories.",setbookicon,bookmarkmenu,BUTTON_TOOLBAR|MENUBUTTON_NOARROWS|MENUBUTTON_ATTACH_LEFT|FRAME_RAISED);
  new FXButton(toolbar,"\tBack\tChange to previous directory.",backicon,this,ID_GO_BACK,BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  new FXButton(toolbar,"\tForward\tChange to next directory.",forwicon,this,ID_GO_FORWARD,BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);

  // Spacer
  new FXFrame(toolbar,LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FIX_WIDTH,0,0,4,0);
  new FXFrame(toolbar,FRAME_SUNKEN|LAYOUT_CENTER_Y|LAYOUT_FIX_HEIGHT|LAYOUT_FIX_WIDTH,0,0,2,22);
  new FXFrame(toolbar,LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FIX_WIDTH,0,0,4,0);


  new FXButton(toolbar,"\tMount\tMount device.",mapicon,NULL,0,BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  new FXButton(toolbar,"\tUnmount\tUnmount device.",unmapicon,NULL,0,BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);

   // Spacer
  new FXFrame(toolbar,LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FIX_WIDTH,0,0,4,0);
  new FXFrame(toolbar,FRAME_SUNKEN|LAYOUT_CENTER_Y|LAYOUT_FIX_HEIGHT|LAYOUT_FIX_WIDTH,0,0,2,22);
  new FXFrame(toolbar,LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FIX_WIDTH,0,0,4,0);

  new FXButton(toolbar,"\tCut\tCut to clipboard.",cuticon,NULL,0,BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  new FXButton(toolbar,"\tCopy\tCopy to clipboard.",copyicon,NULL,0,BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  new FXButton(toolbar,"\tMove\tMove file.",moveicon,NULL,0,BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  new FXButton(toolbar,"\tPaste\tPaste from clipboard.",pasteicon,NULL,0,BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);

  // Spacer
  new FXFrame(toolbar,LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FIX_WIDTH,0,0,4,0);
  new FXFrame(toolbar,FRAME_SUNKEN|LAYOUT_CENTER_Y|LAYOUT_FIX_HEIGHT|LAYOUT_FIX_WIDTH,0,0,2,22);
  new FXFrame(toolbar,LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FIX_WIDTH,0,0,4,0);

  new FXButton(toolbar,"\tProperties\tDisplay file properties.",propicon,this,ID_PROPERTIES,BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);

 // Spacer
  new FXFrame(toolbar,LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FIX_WIDTH,0,0,4,0);
  new FXFrame(toolbar,FRAME_SUNKEN|LAYOUT_CENTER_Y|LAYOUT_FIX_HEIGHT|LAYOUT_FIX_WIDTH,0,0,2,22);
  new FXFrame(toolbar,LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FIX_WIDTH,0,0,4,0);

  // Switch display modes
  new FXButton(toolbar,"\tBig Icons\tShow big icons.",bigiconsicon,filelist,FXFileList::ID_SHOW_BIG_ICONS,BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  new FXButton(toolbar,"\tSmall Icons\tShow small icons.",miniiconsicon,filelist,FXFileList::ID_SHOW_MINI_ICONS,BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);
  new FXButton(toolbar,"\tDetails\tShow detail view.",detailsicon,filelist,FXFileList::ID_SHOW_DETAILS,BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);

 // Spacer
  new FXFrame(toolbar,LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FIX_WIDTH,0,0,4,0);
  new FXFrame(toolbar,FRAME_SUNKEN|LAYOUT_CENTER_Y|LAYOUT_FIX_HEIGHT|LAYOUT_FIX_WIDTH,0,0,2,22);
  new FXFrame(toolbar,LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FIX_WIDTH,0,0,4,0);

  // Delete button far away
  new FXButton(toolbar,"\tDelete\tDelete file.",deleteicon,this,ID_DELETE,BUTTON_TOOLBAR|FRAME_RAISED|LAYOUT_TOP|LAYOUT_LEFT);

  // Go Menu Pane
  gomenu=new FXMenuPane(this);
  new FXMenuCommand(gomenu,"&Up\t\tChange up one level.",upicon,this,ID_UPDIRECTORY);
  new FXMenuCommand(gomenu,"&Back\t\tChange to previous directory.",backicon,this,ID_GO_BACK);
  new FXMenuCommand(gomenu,"&Forward\t\tChange to next directory.",forwicon,this,ID_GO_FORWARD);
  new FXMenuCommand(gomenu,"&Home\t\tChange to home directory.",homeicon,this,ID_GO_HOME);
  new FXMenuCommand(gomenu,"&Work\t\tChange to current working directory.",workicon,this,ID_GO_WORK);
  new FXMenuCommand(gomenu,"&Goto...\tCtl-G\tGoto directory.",NULL,this,ID_GOTO_DIR);
  new FXMenuCommand(gomenu,"Bookmark...\t\tBookmark current directory.",setbookicon,this,ID_BOOKMARK);
  new FXMenuCommand(gomenu,"&Clear...\t\tClear bookmarks.",clrbookicon,&bookmarkeddirs,FXRecentFiles::ID_CLEAR);
  FXMenuSeparator* sep1=new FXMenuSeparator(gomenu);
  sep1->setTarget(&bookmarkeddirs);
  sep1->setSelector(FXRecentFiles::ID_ANYFILES);
  new FXMenuCommand(gomenu,NULL,NULL,&bookmarkeddirs,FXRecentFiles::ID_FILE_1);
  new FXMenuCommand(gomenu,NULL,NULL,&bookmarkeddirs,FXRecentFiles::ID_FILE_2);
  new FXMenuCommand(gomenu,NULL,NULL,&bookmarkeddirs,FXRecentFiles::ID_FILE_3);
  new FXMenuCommand(gomenu,NULL,NULL,&bookmarkeddirs,FXRecentFiles::ID_FILE_4);
  new FXMenuCommand(gomenu,NULL,NULL,&bookmarkeddirs,FXRecentFiles::ID_FILE_5);
  new FXMenuCommand(gomenu,NULL,NULL,&bookmarkeddirs,FXRecentFiles::ID_FILE_6);
  new FXMenuCommand(gomenu,NULL,NULL,&bookmarkeddirs,FXRecentFiles::ID_FILE_7);
  new FXMenuCommand(gomenu,NULL,NULL,&bookmarkeddirs,FXRecentFiles::ID_FILE_8);
  new FXMenuCommand(gomenu,NULL,NULL,&bookmarkeddirs,FXRecentFiles::ID_FILE_9);
  new FXMenuCommand(gomenu,NULL,NULL,&bookmarkeddirs,FXRecentFiles::ID_FILE_10);
  new FXMenuTitle(menubar,"&Go",NULL,gomenu);

  // Arrange menu
  arrangemenu=new FXMenuPane(this);
  new FXMenuCommand(arrangemenu,"&Details\t\tShow detail view.",NULL,filelist,FXFileList::ID_SHOW_DETAILS);
  new FXMenuCommand(arrangemenu,"&Small Icons\t\tShow small icons.",NULL,filelist,FXFileList::ID_SHOW_MINI_ICONS);
  new FXMenuCommand(arrangemenu,"&Big Icons\t\tShow big icons.",NULL,filelist,FXFileList::ID_SHOW_BIG_ICONS);
  new FXMenuSeparator(arrangemenu);
  new FXMenuCommand(arrangemenu,"&Rows\t\tView row-wise.",NULL,filelist,FXFileList::ID_ARRANGE_BY_ROWS);
  new FXMenuCommand(arrangemenu,"&Columns\t\tView column-wise.",NULL,filelist,FXFileList::ID_ARRANGE_BY_COLUMNS);
  new FXMenuTitle(menubar,"&Arrange",NULL,arrangemenu);

  // Sort menu
  sortmenu=new FXMenuPane(this);
  new FXMenuCommand(sortmenu,"&Name\t\tSort by file name.",NULL,filelist,FXFileList::ID_SORT_BY_NAME);
  new FXMenuCommand(sortmenu,"&Type\t\tSort by file type.",NULL,filelist,FXFileList::ID_SORT_BY_TYPE);
  new FXMenuCommand(sortmenu,"&Size\t\tSort by file size.",NULL,filelist,FXFileList::ID_SORT_BY_SIZE);
  new FXMenuCommand(sortmenu,"T&ime\t\tSort by modification time.",NULL,filelist,FXFileList::ID_SORT_BY_TIME);
  new FXMenuCommand(sortmenu,"&User\t\tSort by user name.",NULL,filelist,FXFileList::ID_SORT_BY_USER);
  new FXMenuCommand(sortmenu,"&Group\t\tSort by group name.",NULL,filelist,FXFileList::ID_SORT_BY_GROUP);
  new FXMenuCommand(sortmenu,"&Reverse\t\tReverse sort direction.",NULL,filelist,FXFileList::ID_SORT_REVERSE);
  new FXMenuTitle(menubar,"&Sort",NULL,sortmenu);

  // Preferences menu
  prefmenu=new FXMenuPane(this);
  new FXMenuCommand(prefmenu,"&Icon path...\t\tIcon search path.",NULL,this,ID_SEARCHPATH);
  new FXMenuCommand(prefmenu,"File patterns...\t\tFile patterns shown when opening files.",NULL,this,ID_FILEPATTERNS);
  new FXMenuCommand(prefmenu,"&Save Settings...\t\tSave current settings.",NULL,this,ID_SAVE_SETTINGS);
  new FXMenuTitle(menubar,"&Preferences",NULL,prefmenu);

  // View Menu Pane
  viewmenu=new FXMenuPane(this);
  new FXMenuCommand(viewmenu,"&Toolbar\t\tShow or hide tool bar",NULL,toolbar,FXWindow::ID_TOGGLESHOWN);
  new FXMenuCommand(viewmenu,"&Statusbar\t\tShow or hide status bar",NULL,status,FXWindow::ID_TOGGLESHOWN);
  new FXMenuCommand(viewmenu,"&Hidden Directories\t\tShow hidden directories.",NULL,dirlist,FXDirList::ID_TOGGLE_HIDDEN);
  new FXMenuCommand(viewmenu,"&Hidden Files\t\tShow hidden files and directories.",NULL,filelist,FXFileList::ID_TOGGLE_HIDDEN);
  new FXMenuTitle(menubar,"&View",NULL,viewmenu);

  // Help menu
  helpmenu=new FXMenuPane(this);
  new FXMenuTitle(menubar,"&Help",NULL,helpmenu,LAYOUT_RIGHT);
  new FXMenuCommand(helpmenu,"&About PathFinder...\t\tDisplay PathFinder About Panel.",NULL,this,ID_ABOUT,0);

  // Install some accelerators
  getAccelTable()->addAccel(MKUINT(KEY_Delete,0),this,MKUINT(ID_DELETE,SEL_COMMAND));
  getAccelTable()->addAccel(MKUINT(KEY_F1,0),this,MKUINT(ID_ABOUT,SEL_COMMAND));
  getAccelTable()->addAccel(MKUINT(KEY_Return,ALTMASK),this,MKUINT(ID_PROPERTIES,SEL_COMMAND));

  // Make a tool tip
  new FXTooltip(getApp(),0);

  // Recent directories
  visiting=0;

  // Bookmarked directories
  bookmarkeddirs.setTarget(this);
  bookmarkeddirs.setSelector(ID_GO_RECENT);

  // Set patterns
  setPatterns("All Files (*)");
  setCurrentPattern(0);
  }


// Clean up
PathFinderMain::~PathFinderMain(){
  delete property;
  delete associations;
  delete filemenu;
  delete editmenu;
  delete commandmenu;
  delete viewmenu;
  delete gomenu;
  delete arrangemenu;
  delete sortmenu;
  delete prefmenu;
  delete helpmenu;
  delete bookmarkmenu;
  delete bigp;
  delete minip;
  delete cuticon;
  delete copyicon;
  delete moveicon;
  delete pasteicon;
  delete upicon;
  delete homeicon;
  delete backicon;
  delete forwicon;
  delete bigiconsicon;
  delete miniiconsicon;
  delete detailsicon;
  delete mapicon;
  delete unmapicon;
  delete propicon;
  delete deleteicon;
  delete setbookicon;
  delete clrbookicon;
  delete workicon;
  }


/*******************************************************************************/


// Open
long PathFinderMain::onCmdOpen(FXObject*,FXSelector,void*){
  FXint index=filelist->getCurrentItem();
  if(0<=index){

    // If directory, open the directory
    if(filelist->isItemDirectory(index)){
      FXString directory=filelist->getItemPathname(index);
      filelist->setDirectory(directory);
      dirlist->setDirectory(directory);
      dirbox->setDirectory(directory);
      visitDirectory(directory);
      }

    // If executable, execute it!
    else if(filelist->isItemExecutable(index)){
      //FXString executable=filelist->getItemPathname(index) + " &";
      FXString executable=FXFile::enquote(filelist->getItemPathname(index)) + " &";
      FXTRACE((100,"system(%s)\n",executable.text()));
      system(executable.text());
      }

    // If regular file return as the selected file
    else if(filelist->isItemFile(index)){
      FXFileAssoc *association=filelist->getItemAssoc(index);
      if(association){
        if(association->command.text()){
          FXString command=FXStringFormat(association->command.text(),FXFile::enquote(filelist->getItemPathname(index)).text());
          FXTRACE((100,"system(%s)\n",command.text()));
          system(command.text());
          }
        else{
          FXMessageBox::information(this,MBOX_OK,"Unknown Command","No command defined for file: %s",filelist->getItemFilename(index).text());
          }
        }
      else{
        FXMessageBox::information(this,MBOX_OK,"Unknown File Type","No association has been set for file: %s",filelist->getItemFilename(index).text());
        }
      }
    }
  return 1;
  }


// Update open
long PathFinderMain::onUpdOpen(FXObject* sender,FXSelector,void*){
  if(filelist->getCurrentItem()>=0)
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }



// File Item was double-clicked
long PathFinderMain::onCmdFileDblClicked(FXObject*,FXSelector,void* ptr){
  FXint index=(FXint)(long)ptr;
  if(0<=index){

    // If directory, open the directory
    if(filelist->isItemDirectory(index)){
      FXString directory=filelist->getItemPathname(index);
      filelist->setDirectory(directory);
      dirlist->setDirectory(directory);
      dirbox->setDirectory(directory);
      visitDirectory(directory);
      }

    // If executable, execute it!
    else if(filelist->isItemExecutable(index)){
      //FXString executable=filelist->getItemPathname(index) + " &";
      FXString executable=FXFile::enquote(filelist->getItemPathname(index)) + " &";
      FXTRACE((100,"system(%s)\n",executable.text()));
      system(executable.text());
      }

    // If regular file return as the selected file
    else if(filelist->isItemFile(index)){
      FXFileAssoc *association=filelist->getItemAssoc(index);
      if(association){
        if(association->command.text()){
          FXString command=FXStringFormat(association->command.text(),FXFile::enquote(filelist->getItemPathname(index)).text());
          FXTRACE((100,"system(%s)\n",command.text()));
          system(command.text());
          }
        else{
          FXMessageBox::information(this,MBOX_OK,"Unknown Command","No command defined for file: %s",filelist->getItemFilename(index).text());
          }
        }
      else{
        FXMessageBox::information(this,MBOX_OK,"Unknown File Type","No association has been set for file: %s",filelist->getItemFilename(index).text());
        }
      }
    }
  return 1;
  }


// File Item was clicked
long PathFinderMain::onCmdFileClicked(FXObject*,FXSelector,void*){
//   FXchar path[MAXPATHLEN+1],name[MAXPATHLEN+1],dir[MAXPATHLEN+1],*p;
//   FXOldIconItem *item=(FXOldIconItem*)ptr;
//   name[0]=0;
//   strcpy(path,filelist->getDirectory());
//   if(item){
//     strcpy(name,filelist->getItemText(item));
//     if((p=strchr(name,'\t'))) *p=0;
//     }
//   fxpathname(dir,path,name);
//   filename->setText(dir);
  return 1;
  }


// Popup menu for item in file list
long PathFinderMain::onFileListPopup(FXObject*,FXSelector,void* ptr){
  FXEvent *event=(FXEvent*)ptr;
  if(event->moved) return 1;
  FXint index=filelist->getItemAt(event->win_x,event->win_y);
  FXMenuPane pane(this);

  // We clicked in the background
  if(index<0){
    new FXMenuCommand(&pane,"&Up\t\tChange up one level.",upicon,this,ID_UPDIRECTORY);
    new FXMenuCommand(&pane,"&Back\t\tChange to previous directory.",backicon,this,ID_GO_BACK);
    new FXMenuCommand(&pane,"&Forward\t\tChange to next directory.",forwicon,this,ID_GO_FORWARD);
    new FXMenuCommand(&pane,"&Home\t\tChange to home directory.",homeicon,this,ID_GO_HOME);
    new FXMenuCommand(&pane,"&Work\t\tChange to current working directory.",workicon,this,ID_GO_WORK);
    new FXMenuCommand(&pane,"Bookmark...\t\tBookmark this directory.",setbookicon,this,ID_BOOKMARK);
    new FXMenuSeparator(&pane);
    new FXMenuCascade(&pane,"Arrange",NULL,arrangemenu);
    new FXMenuSeparator(&pane);
    new FXMenuCascade(&pane,"Sort by",NULL,sortmenu);
    }

  // We clicked on an item
  else{
    new FXMenuCommand(&pane,"Open...",NULL,this,ID_OPEN);
    new FXMenuCommand(&pane,"Open with...",NULL,this,ID_OPEN_WITH);
    new FXMenuSeparator(&pane);
    new FXMenuCommand(&pane,"Copy",copyicon,NULL,0);
    new FXMenuCommand(&pane,"Rename",NULL,NULL,0);
    new FXMenuCommand(&pane,"Delete",deleteicon,this,ID_DELETE);
    new FXMenuSeparator(&pane);
    new FXMenuCommand(&pane,"Properties...",propicon,this,ID_PROPERTIES);
    }
  pane.create();
  pane.popup(NULL,event->root_x,event->root_y);
  getApp()->runModalWhileShown(&pane);
  return 1;
  }


// About
long PathFinderMain::onCmdAbout(FXObject*,FXSelector,void*){
  FXMessageBox about(this,"About PathFinder","PathFinder File Browser V0.4\n\nUsing the FOX C++ GUI Library (http://www.fox-tookit.org)\n\nCopyright (C) 1998,2001 Jeroen van der Zijp (jeroen@fox-toolkit.org)",bigp,MBOX_OK|DECOR_TITLE|DECOR_BORDER);
  about.execute();
  return 1;
  }


// Set search path
long PathFinderMain::onCmdSetSearchPath(FXObject*,FXSelector,void*){
  FXDialogBox dialog(getApp(),"Change icon search path",DECOR_TITLE|DECOR_BORDER);
  const FXchar *path;
  FXVerticalFrame* content=new FXVerticalFrame(&dialog,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,10,10,10,10,10,10);
  new FXLabel(content,"&Specify a `" PATHLISTSEPSTRING "' separated list of directories where icons are to be found:",NULL,LAYOUT_FILL_X|JUSTIFY_LEFT);
  FXTextField *text=new FXTextField(content,30,&dialog,FXDialogBox::ID_ACCEPT,TEXTFIELD_ENTER_ONLY|FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X);
  new FXHorizontalSeparator(content,SEPARATOR_GROOVE|LAYOUT_FILL_X);
  FXHorizontalFrame* buttons=new FXHorizontalFrame(content,LAYOUT_FILL_X|PACK_UNIFORM_WIDTH,0,0,0,0,0,0,0,0);
  new FXButton(buttons,"&OK",NULL,&dialog,FXDialogBox::ID_ACCEPT,BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT);
  new FXButton(buttons,"&Cancel",NULL,&dialog,FXDialogBox::ID_CANCEL,BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT,0,0,0,0,20,20);
  path=getApp()->reg().readStringEntry("SETTINGS","iconpath","~/.foxicons");
  text->setText(path);
  dialog.create();
  if(dialog.execute()){
    getApp()->reg().writeStringEntry("SETTINGS","iconpath",text->getText().text());
    }
  return 1;
  }


// Change the directory from the FXDirList
long PathFinderMain::onCmdDirectory(FXObject*,FXSelector,void* ptr){
  FXTreeItem *item=(FXTreeItem*)ptr;
  FXString path=dirlist->getItemPathname(item);
  filelist->setDirectory(path);
  dirbox->setDirectory(path);
  visitDirectory(path);
  return 1;
  }


// Change the directory from the FXDirBox
long PathFinderMain::onCmdDirTree(FXObject*,FXSelector,void*){
  FXString path=dirbox->getDirectory();
  dirlist->setDirectory(path);
  filelist->setDirectory(path);
  visitDirectory(path);
  return 1;
  }


// Move up one directory
long PathFinderMain::onCmdUpDirectory(FXObject*,FXSelector,void*){
  FXString path=FXFile::upLevel(filelist->getDirectory());
  setDirectory(path);
  visitDirectory(path);
  return 1;
  }


// Move to home directory
long PathFinderMain::onCmdHomeDirectory(FXObject*,FXSelector,void*){
  FXString path=FXFile::getHomeDirectory();
  setDirectory(path);
  visitDirectory(path);
  return 1;
  }


// Move to work directory
long PathFinderMain::onCmdWorkDirectory(FXObject*,FXSelector,void*){
  FXString path=FXFile::getCurrentDirectory();
  setDirectory(path);
  visitDirectory(path);
  return 1;
  }


// Bookmark this directory
long PathFinderMain::onCmdBookmark(FXObject*,FXSelector,void*){
  bookmarkeddirs.appendFile(filelist->getDirectory());
  return 1;
  }


// Move to recent directory
long PathFinderMain::onCmdRecentDirectory(FXObject*,FXSelector,void* ptr){
  FXString path((FXchar*)ptr);
  setDirectory(path);
  visitDirectory(path);
  return 1;
  }


// Move to previous directory
long PathFinderMain::onCmdBackDirectory(FXObject*,FXSelector,void*){
  FXASSERT(visiting<9);
  setDirectory(visiteddir[++visiting]);
  return 1;
  }


// Update move to previous directory
long PathFinderMain::onUpdBackDirectory(FXObject* sender,FXSelector,void*){
  if(visiting>=9 || visiteddir[visiting].empty())
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Move to next directory
long PathFinderMain::onCmdForwardDirectory(FXObject*,FXSelector,void*){
  FXASSERT(visiting>0);
  setDirectory(visiteddir[--visiting]);
  return 1;
  }


// Update move to next directory
long PathFinderMain::onUpdForwardDirectory(FXObject* sender,FXSelector,void*){
  if(visiting==0)
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Update title
long PathFinderMain::onUpdTitle(FXObject* sender,FXSelector,void*){
  FXString title="PathFinder:- " + filelist->getDirectory();
  sender->handle(this,MKUINT(FXWindow::ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&title);
  return 1;
  }


// Update files heading
long PathFinderMain::onUpdFiles(FXObject* sender,FXSelector,void*){
  FXString string="Files in: " + filelist->getDirectory();
  sender->handle(this,MKUINT(FXWindow::ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&string);
  return 1;
  }


// Update status line to show some info about the icon the cursor is over
long PathFinderMain::onUpdStatusline(FXObject* sender,FXSelector,void*){
  FXint index;
  index=filelist->getCursorItem();
  if(0<=index){
    FXString info;
    FXFileItem *item=(FXFileItem*)filelist->retrieveItem(index);
    FXFileAssoc *assoc=item->getAssoc();

    // What is this thing?
    if(item->isDirectory())       info="Directory: ";
    else if(item->isSymlink())    info="Symlink: ";
    else if(item->isSocket())     info="Socket: ";
    else if(item->isFifo())       info="Fifo: ";
    else if(item->isBlockdev())   info="BlockDev: ";
    else if(item->isChardev())    info="CharDev: ";
    else if(item->isExecutable()) info="Application: ";
    else                          info="File: ";

    // Add the name
    info+=filelist->getItemFilename(index);

    // Add size if its a file
    if(item->isFile()) info+=" ("+FXStringVal((FXuint)item->getSize())+" bytes) ";

    // Add the extension
    if(assoc) info+=assoc->extension;

    // Set the status line
    sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&info);
    return 1;
    }
  return 0;
  }


// Save settings to disk
long PathFinderMain::onCmdSaveSettings(FXObject*,FXSelector,void*){
  saveSettings();
  getApp()->reg().write();
  return 1;
  }


// Spawn new PathFinder
long PathFinderMain::onCmdNewPathFinder(FXObject*,FXSelector,void*){
  FXString path=filelist->getDirectory();
  saveSettings();
  getApp()->reg().write();
  FXString command=FXStringFormat("%s %s &",pathfindercommand,path.text());
  system(command.text());
  return 1;
  }


// Pop up properties panel
long PathFinderMain::onCmdProperties(FXObject*,FXSelector,void*){
  property->show(PLACEMENT_OWNER);
  return 1;
  }


// Update Pop up properties panel buttons
long PathFinderMain::onUpdProperties(FXObject* sender,FXSelector,void*){
  if(!property->shown())
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Delete file or directory
long PathFinderMain::onCmdDelete(FXObject*,FXSelector,void*){
  FXuint answer=FXMessageBox::warning(this,MBOX_YES_NO,"Deleting files","Are you sure you want to delete these files?");
  if(answer==MBOX_CLICKED_YES){
    FXString filetoremove,file;
    for(FXint i=0; i<filelist->getNumItems(); i++){
      if(filelist->isItemSelected(i)){
        file=filelist->getItemFilename(i);
        if(file=="..") continue;
        filetoremove=FXFile::absolute(filelist->getDirectory(),file);
        FXTRACE((100,"filetoremove=%s\n",filetoremove.text()));
        if(!FXFile::remove(filetoremove)){
          if(MBOX_CLICKED_NO==FXMessageBox::error(this,MBOX_YES_NO,"Error Deleting File","Unable to delete file: %s\nContinue with operation?",filetoremove.text())){
            break;
            }
          }
        }

      }
    }
  return 1;
  }


// Update delete button
long PathFinderMain::onUpdDelete(FXObject* sender,FXSelector,void*){
  FXSelector message=MKUINT(ID_DISABLE,SEL_COMMAND);
  for(FXint i=0; i<filelist->getNumItems(); i++){
    if(filelist->isItemSelected(i)){message=MKUINT(ID_ENABLE,SEL_COMMAND);break;}
    }
  sender->handle(this,message,NULL);
  return 1;
  }


// Create new directory
long PathFinderMain::onCmdNew(FXObject*,FXSelector,void*){
  FXDialogBox dialog(this,"Create New Directory",DECOR_TITLE|DECOR_BORDER);
  const FXchar suggestedname[]="DirectoryName";
  FXVerticalFrame* content=new FXVerticalFrame(&dialog,LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0,10,10,10,10,10,10);
  new FXLabel(content,"Create new directory in: "+filelist->getDirectory(),NULL,LAYOUT_FILL_X|JUSTIFY_LEFT);
  FXTextField *text=new FXTextField(content,40,&dialog,FXDialogBox::ID_ACCEPT,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X);
  new FXHorizontalSeparator(content,SEPARATOR_GROOVE|LAYOUT_FILL_X);
  FXHorizontalFrame* buttons=new FXHorizontalFrame(content,LAYOUT_FILL_X|PACK_UNIFORM_WIDTH,0,0,0,0,0,0,0,0);
  new FXButton(buttons,"&OK",NULL,&dialog,FXDialogBox::ID_ACCEPT,BUTTON_INITIAL|BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT);
  new FXButton(buttons,"&Cancel",NULL,&dialog,FXDialogBox::ID_CANCEL,BUTTON_DEFAULT|FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT,0,0,0,0,20,20);
  dialog.create();
  text->setText(suggestedname);
  text->setFocus();
  text->setSelection(0,sizeof(suggestedname));
  if(dialog.execute()){
    FXString dirname=FXFile::absolute(filelist->getDirectory(),text->getText());
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
long PathFinderMain::onUpdNew(FXObject* sender,FXSelector,void*){
  FXString path=filelist->getDirectory();
  if(FXFile::isWritable(path))
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Sort functions
static const FXIconListSortFunc sortfuncs[]={
  FXFileList::cmpFName,
  FXFileList::cmpRName,
  FXFileList::cmpFType,
  FXFileList::cmpRType,
  FXFileList::cmpFSize,
  FXFileList::cmpRSize,
  FXFileList::cmpFTime,
  FXFileList::cmpRTime,
  FXFileList::cmpFUser,
  FXFileList::cmpRUser,
  FXFileList::cmpFGroup,
  FXFileList::cmpRGroup
  };


// Save settings
void PathFinderMain::saveSettings(){
  FXString path;
  FXString filter;
  FXuint   iconview;
  FXuint   hiddenfiles;
  FXuint   hiddendirs;
  FXuint   index;
  FXIconListSortFunc sortfunc;

  // Save pathfinder directory
  path=filelist->getDirectory();
  getApp()->reg().writeStringEntry("PathFinder Settings","directory",path.text());

  // Save file list mode
  iconview=filelist->getListStyle();
  getApp()->reg().writeUnsignedEntry("PathFinder Settings","iconview",iconview);

  // Showing hidden files...
  hiddenfiles=filelist->showHiddenFiles();
  getApp()->reg().writeUnsignedEntry("PathFinder Settings","hiddenfiles",hiddenfiles);

  // Showing hidden directories...
  hiddendirs=dirlist->showHiddenFiles();
  getApp()->reg().writeUnsignedEntry("PathFinder Settings","hiddendirs",hiddendirs);

  // Write new window size back to registry
  getApp()->reg().writeIntEntry("PathFinder Settings","x",getX());
  getApp()->reg().writeIntEntry("PathFinder Settings","y",getY());
  getApp()->reg().writeIntEntry("PathFinder Settings","width",getWidth());
  getApp()->reg().writeIntEntry("PathFinder Settings","height",getHeight());

  // Width of tree
  getApp()->reg().writeIntEntry("PathFinder Settings","dirwidth",group1->getWidth());

  // Filter
  filter=filelist->getPattern();
  getApp()->reg().writeStringEntry("PathFinder Settings","filter",filter.text());

  // Header sizes
  getApp()->reg().writeIntEntry("PathFinder Settings","nameheader",filelist->getHeaderSize(0));
  getApp()->reg().writeIntEntry("PathFinder Settings","typeheader",filelist->getHeaderSize(1));
  getApp()->reg().writeIntEntry("PathFinder Settings","sizeheader",filelist->getHeaderSize(2));
  getApp()->reg().writeIntEntry("PathFinder Settings","dateheader",filelist->getHeaderSize(3));
  getApp()->reg().writeIntEntry("PathFinder Settings","userheader",filelist->getHeaderSize(4));
  getApp()->reg().writeIntEntry("PathFinder Settings","attrheader",filelist->getHeaderSize(5));

  // Visited directories
  getApp()->reg().writeStringEntry("Visited Directories","0",visiteddir[0].text());
  getApp()->reg().writeStringEntry("Visited Directories","1",visiteddir[1].text());
  getApp()->reg().writeStringEntry("Visited Directories","2",visiteddir[2].text());
  getApp()->reg().writeStringEntry("Visited Directories","3",visiteddir[3].text());
  getApp()->reg().writeStringEntry("Visited Directories","4",visiteddir[4].text());
  getApp()->reg().writeStringEntry("Visited Directories","5",visiteddir[5].text());
  getApp()->reg().writeStringEntry("Visited Directories","6",visiteddir[6].text());
  getApp()->reg().writeStringEntry("Visited Directories","7",visiteddir[7].text());
  getApp()->reg().writeStringEntry("Visited Directories","8",visiteddir[8].text());
  getApp()->reg().writeStringEntry("Visited Directories","9",visiteddir[9].text());

  // Visiting
  getApp()->reg().writeIntEntry("Visited Directories","visiting",visiting);

  // File patterns
  getApp()->reg().writeIntEntry("PathFinder Settings","filepatternno",getCurrentPattern());
  getApp()->reg().writeStringEntry("PathFinder Settings","filepatterns",getPatterns().text());

  // Sort function
  sortfunc=filelist->getSortFunc();
  for(index=ARRAYNUMBER(sortfuncs)-1; index; index--){ if(sortfuncs[index]==sortfunc) break; }
  getApp()->reg().writeIntEntry("PathFinder Settings","sorting",index);
  }


// Load settings
void PathFinderMain::loadSettings(){
  FXString path;
  FXString filter;
  FXuint   iconview;
  FXuint   hiddenfiles;
  FXuint   hiddendirs;
  FXuint   sortfunc;
  FXint    ww,hh,xx,yy,treewidth;

  // Read last path setting
  path=getApp()->reg().readStringEntry("PathFinder Settings","directory","~");
  setDirectory(path);

  // Read icon view mode
  iconview=getApp()->reg().readUnsignedEntry("PathFinder Settings","iconview",ICONLIST_BIG_ICONS|ICONLIST_AUTOSIZE);
  filelist->setListStyle(iconview);

  // Showing hidden files...
  hiddenfiles=getApp()->reg().readUnsignedEntry("PathFinder Settings","hiddenfiles",FALSE);
  filelist->showHiddenFiles(hiddenfiles);

  // Showing hidden directories...
  hiddendirs=getApp()->reg().readUnsignedEntry("PathFinder Settings","hiddendirs",FALSE);
  dirlist->showHiddenFiles(hiddendirs);

  // Get size
  xx=getApp()->reg().readIntEntry("PathFinder Settings","x",100);
  yy=getApp()->reg().readIntEntry("PathFinder Settings","y",100);
  ww=getApp()->reg().readIntEntry("PathFinder Settings","width",800);
  hh=getApp()->reg().readIntEntry("PathFinder Settings","height",600);

  setX(xx);
  setY(yy);
  setWidth(ww);
  setHeight(hh);

  // Width of tree
  treewidth=getApp()->reg().readIntEntry("PathFinder Settings","dirwidth",100);

  // Set tree width
  group1->setWidth(treewidth);

  // Filter
  filter=getApp()->reg().readStringEntry("PathFinder Settings","filter","*");
  filelist->setPattern(filter);

  // Header sizes
  filelist->setHeaderSize(0,getApp()->reg().readIntEntry("PathFinder Settings","nameheader",200));
  filelist->setHeaderSize(1,getApp()->reg().readIntEntry("PathFinder Settings","typeheader",100));
  filelist->setHeaderSize(2,getApp()->reg().readIntEntry("PathFinder Settings","sizeheader",60));
  filelist->setHeaderSize(3,getApp()->reg().readIntEntry("PathFinder Settings","dateheader",150));
  filelist->setHeaderSize(4,getApp()->reg().readIntEntry("PathFinder Settings","userheader",50));
  filelist->setHeaderSize(5,getApp()->reg().readIntEntry("PathFinder Settings","attrheader",60));

  // Visited directories
  visiteddir[0]=getApp()->reg().readStringEntry("Visited Directories","0",FXString::null);
  visiteddir[1]=getApp()->reg().readStringEntry("Visited Directories","1",FXString::null);
  visiteddir[2]=getApp()->reg().readStringEntry("Visited Directories","2",FXString::null);
  visiteddir[3]=getApp()->reg().readStringEntry("Visited Directories","3",FXString::null);
  visiteddir[4]=getApp()->reg().readStringEntry("Visited Directories","4",FXString::null);
  visiteddir[5]=getApp()->reg().readStringEntry("Visited Directories","5",FXString::null);
  visiteddir[6]=getApp()->reg().readStringEntry("Visited Directories","6",FXString::null);
  visiteddir[7]=getApp()->reg().readStringEntry("Visited Directories","7",FXString::null);
  visiteddir[8]=getApp()->reg().readStringEntry("Visited Directories","8",FXString::null);
  visiteddir[9]=getApp()->reg().readStringEntry("Visited Directories","9",FXString::null);

  // Visiting
  visiting=getApp()->reg().readIntEntry("Visited Directories","visiting",0);

  // File patterns
  setPatterns(getApp()->reg().readStringEntry("PathFinder Settings","filepatterns","All Files (*)"));
  setCurrentPattern(getApp()->reg().readIntEntry("PathFinder Settings","filepatternno",0));

  // Sort function
  sortfunc=getApp()->reg().readIntEntry("PathFinder Settings","sorting",0);
  if(sortfunc>=ARRAYNUMBER(sortfuncs)) sortfunc=0;
  filelist->setSortFunc(sortfuncs[sortfunc]);
  }


// Handle quitting
long PathFinderMain::onCmdQuit(FXObject*,FXSelector,void*){
  saveSettings();
  getApp()->exit();
  return 1;
  }


// Switch to given directory
void PathFinderMain::setDirectory(const FXString& dir){
  filelist->setDirectory(dir);
  dirbox->setDirectory(dir);
  dirlist->setDirectory(dir);
  }


// Get current directory
FXString PathFinderMain::getDirectory() const {
  return filelist->getDirectory();
  }


// Visit directory
void PathFinderMain::visitDirectory(const FXString& dir){
  FXint i;
  if(visiting==0){
    for(i=9; i; i--) visiteddir[i]=visiteddir[i-1];
    }
  else{
    for(i=1; i+visiting-1<=9; i++) visiteddir[i]=visiteddir[i+visiting-1];
    for( ; i<10; i++) visiteddir[i]=FXString::null;
    }
  visiteddir[0]=dir;
  visiting=0;
  }


// Change patterns, each pattern separated by newline
void PathFinderMain::setPatterns(const FXString& patterns){
  FXString pat; FXint i;
  pattern->clearItems();
  for(i=0; !(pat=patterns.extract(i,'\n')).empty(); i++){
    pattern->appendItem(pat);
    }
  if(!pattern->getNumItems()) pattern->appendItem("All Files (*)");
  setCurrentPattern(0);
  }


// Return list of patterns
FXString PathFinderMain::getPatterns() const {
  FXString pat; FXint i;
  for(i=0; i<pattern->getNumItems(); i++){
    if(!pat.empty()) pat+='\n';
    pat+=pattern->getItemText(i);
    }
  return pat;
  }


// Strip pattern from text if present
static FXString patternFromText(const FXString& pattern){
  FXint beg,end;
  end=pattern.findb(')');
  beg=pattern.findb('(',end-1);
  if(0<=beg && beg<end) return pattern.mid(beg+1,end-beg-1);
  return pattern;
  }


// Set current pattern
void PathFinderMain::setCurrentPattern(FXint n){
  n=FXCLAMP(0,n,pattern->getNumItems()-1);
  pattern->setCurrentItem(n);
  dirlist->setPattern(patternFromText(pattern->getItemText(n)));
  }


// Return current pattern
FXint PathFinderMain::getCurrentPattern() const {
  return pattern->getCurrentItem();
  }


// Make application
void PathFinderMain::create(){
  loadSettings();
  FXMainWindow::create();
  show();
  }


// Change file patterns
long PathFinderMain::onCmdFilePatterns(FXObject*,FXSelector,void*){
  FXString pats=getPatterns();
  pats.substitute('\n',';');
  if(FXInputDialog::getString(pats,this,"Change File Patterns","Specify a list of file patterns separated by a ';' between each pattern.\nFor example:\n\n  Source Files (*.c,*.h);Text Files (*.txt)\n\nThis list will be used to select files.")){
    pats.substitute(';','\n');
    setPatterns(pats);
    setCurrentPattern(0);
    }
  return 1;
  }


// Change the pattern
long PathFinderMain::onCmdFilter(FXObject*,FXSelector,void* ptr){
  filelist->setPattern(patternFromText((FXchar*)ptr));
  return 1;
  }


// Goto directory
long PathFinderMain::onCmdGotoDir(FXObject*,FXSelector,void*){
  FXBMPIcon icon(getApp(),gotodir,0,IMAGE_ALPHAGUESS);
  FXString dir=getDirectory();
  if(FXInputDialog::getString(dir,this,"Goto Directory","&Goto directory:",&icon)){
    setDirectory(dir);
    }
  return 1;
  }


// Open with program
long PathFinderMain::onCmdOpenWith(FXObject*,FXSelector,void*){
  FXString cmd=getApp()->reg().readStringEntry("SETTINGS","command","textedit");
  FXString filename=filelist->getCurrentFile();
  if(FXInputDialog::getString(cmd,this,"Open File With","Open " + FXFile::name(filename) + " with:")){
    getApp()->reg().writeStringEntry("SETTINGS","command",cmd.text());
    FXString command=cmd+" "+filename+" &";
    system(command.text());
    /*
    // Spawn child
    if(fork()==0){
      // Close on exec of file descriptor
      //fcntl(fd,F_SETFD,TRUE);
      // Start command and pass it the filename
      execlp(cmd.text(),cmd.text(),filename.text(),NULL);

      // Just in case we failed to execute the command
      ::exit(0);
      }
    */
    }
  return 1;
  }


// Update open with
long PathFinderMain::onUpdOpenWith(FXObject* sender,FXSelector,void*){
  if(!filelist->getCurrentFile().empty())
    sender->handle(this,MKUINT(ID_ENABLE,SEL_COMMAND),NULL);
  else
    sender->handle(this,MKUINT(ID_DISABLE,SEL_COMMAND),NULL);
  return 1;
  }


// Run program
long PathFinderMain::onCmdRun(FXObject*,FXSelector,void*){
  FXString prg=getApp()->reg().readStringEntry("SETTINGS","program","textedit");
  if(FXInputDialog::getString(prg,this,"Run Program","Run Program:")){
    getApp()->reg().writeStringEntry("SETTINGS","program",prg.text());
    prg+=" &";
    system(prg.text());
    }
  return 1;
  }


// Run terminal
long PathFinderMain::onCmdTerminal(FXObject*,FXSelector,void*){
  FXString prg=getApp()->reg().readStringEntry("SETTINGS","terminal","xterm");
  if(!prg.empty()){
    prg+=" &";
    system(prg.text());
    }
  else{
    FXMessageBox::information(this,MBOX_OK,"Cannot run terminal!","Terminal Application is not specified.\n");
    }
  return 1;
  }


// Change mode
long PathFinderMain::onCmdChmod(FXObject*,FXSelector sel,void*){
  FXString filename=filelist->getCurrentFile();
  FXuint mode=FXFile::mode(filename);
#ifndef WIN32
  switch(SELID(sel)) {
    case ID_RUSR: mode^=S_IRUSR; break;
    case ID_WUSR: mode^=S_IWUSR; break;
    case ID_XUSR: mode^=S_IXUSR; break;
    case ID_RGRP: mode^=S_IRGRP; break;
    case ID_WGRP: mode^=S_IWGRP; break;
    case ID_XGRP: mode^=S_IXGRP; break;
    case ID_ROTH: mode^=S_IROTH; break;
    case ID_WOTH: mode^=S_IWOTH; break;
    case ID_XOTH: mode^=S_IXOTH; break;
    case ID_SUID: mode^=S_ISUID; break;
    case ID_SGID: mode^=S_ISGID; break;
    case ID_SVTX: mode^=S_ISVTX; break;
    }
#endif
  if(!FXFile::mode(filename,mode)){
    FXMessageBox::error(this,MBOX_OK,"Error Changing Permissions","Unable to change permissions on file: %s",filename.text());
    }
  return 1;
  }


// Update change mode
long PathFinderMain::onUpdChmod(FXObject* sender,FXSelector sel,void*){
  FXString filename=filelist->getCurrentFile();
  FXuint test=0;
  switch(SELID(sel)) {
    case ID_RUSR: test=FXFile::isOwnerReadable(filename); break;
    case ID_WUSR: test=FXFile::isOwnerWritable(filename); break;
    case ID_XUSR: test=FXFile::isOwnerExecutable(filename); break;
    case ID_RGRP: test=FXFile::isGroupReadable(filename); break;
    case ID_WGRP: test=FXFile::isGroupWritable(filename); break;
    case ID_XGRP: test=FXFile::isGroupExecutable(filename); break;
    case ID_ROTH: test=FXFile::isOtherReadable(filename); break;
    case ID_WOTH: test=FXFile::isOtherWritable(filename); break;
    case ID_XOTH: test=FXFile::isOtherExecutable(filename); break;
    case ID_SUID: test=FXFile::isSetUid(filename); break;
    case ID_SGID: test=FXFile::isSetGid(filename); break;
    case ID_SVTX: test=FXFile::isSetSticky(filename); break;
    }
  sender->handle(this,test ? MKUINT(ID_CHECK,SEL_COMMAND) : MKUINT(ID_UNCHECK,SEL_COMMAND),NULL);
  return 1;
  }


// Update owner
long PathFinderMain::onUpdOwner(FXObject* sender,FXSelector,void*){
  FXString owner=FXFile::owner(filelist->getCurrentFile());
  sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&owner);
  return 1;
  }


// Update group
long PathFinderMain::onUpdGroup(FXObject* sender,FXSelector,void*){
  FXString group=FXFile::group(filelist->getCurrentFile());
  sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&group);
  return 1;
  }


// Update create time
long PathFinderMain::onUpdCreateTime(FXObject* sender,FXSelector,void*){
  FXString time=FXFile::time(FXFile::created(filelist->getCurrentFile()));
  sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&time);
  return 1;
  }


// Update modified time
long PathFinderMain::onUpdModifyTime(FXObject* sender,FXSelector,void*){
  FXString time=FXFile::time(FXFile::modified(filelist->getCurrentFile()));
  sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&time);
  return 1;
  }


// Update access time
long PathFinderMain::onUpdAccessTime(FXObject* sender,FXSelector,void*){
  FXString time=FXFile::time(FXFile::accessed(filelist->getCurrentFile()));
  sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&time);
  return 1;
  }


// Update file location
long PathFinderMain::onUpdFileLocation(FXObject* sender,FXSelector,void*){
  FXString location=filelist->getCurrentFile();
  sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&location);
  return 1;
  }

// Update file type
long PathFinderMain::onUpdFileSize(FXObject* sender,FXSelector,void*){
  FXString size=FXStringVal((FXuint)FXFile::size(filelist->getCurrentFile()));
  sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&size);
  return 1;
  }



// Update file type
long PathFinderMain::onUpdFileType(FXObject* sender,FXSelector,void*){
  FXString filename=filelist->getCurrentFile();
  FXFileAssoc *fileassoc=NULL;
  FXString type;
  if(FXFile::isDirectory(filename)){
    fileassoc=associations->findDirBinding(filename.text());
    type="Folder";
    }
  else if(FXFile::isExecutable(filename)){
    fileassoc=associations->findExecBinding(filename.text());
    type="Application";
    }
  else{
    fileassoc=associations->findFileBinding(filename.text());
    type="Document";
    }
  if(fileassoc) type=fileassoc->extension;
  sender->handle(this,MKUINT(ID_SETSTRINGVALUE,SEL_COMMAND),(void*)&type);
  return 1;
  }


// Update file type
long PathFinderMain::onUpdFileDesc(FXObject* sender,FXSelector,void*){
  FXString filename=filelist->getCurrentFile();
  FXLabel *label=(FXLabel*)sender;
  FXFileAssoc *fileassoc=NULL;
  if(FXFile::isDirectory(filename)){
    fileassoc=associations->findDirBinding(filename.text());
    }
  else if(FXFile::isExecutable(filename)){
    fileassoc=associations->findExecBinding(filename.text());
    }
  else{
    fileassoc=associations->findFileBinding(filename.text());
    }
  label->setText(FXFile::name(filename));
  if(fileassoc){
    if(fileassoc->bigicon) fileassoc->bigicon->create();
    label->setIcon(fileassoc->bigicon);
    }
  else{
    label->setIcon(NULL); // FIXME need a default suggestion here
    }
  return 1;
  }


// Harvest the zombies :-)
long PathFinderMain::onSigHarvest(FXObject*,FXSelector,void*){
#ifndef WIN32
  while(waitpid(-1,NULL,WNOHANG)>0);
#endif
  return 1;
  }

/*******************************************************************************/


// Start the whole thing
int main(int argc,char *argv[]){

  // Create application
  FXApp application("PathFinder","FoxTest");

  // Keep original launch name
  PathFinderMain::pathfindercommand=argv[0];

  // Initialize application
  application.init(argc,argv);

  // Build GUI
  PathFinderMain* window=new PathFinderMain(&application);

  // On unix, we need to catch SIGCHLD to harvest zombie child processes.
#ifndef WIN32
  application.addSignal(SIGCHLD,window,PathFinderMain::ID_HARVEST,TRUE);
#endif

  // Also catch interrupt so we can gracefully terminate
  application.addSignal(SIGINT,window,PathFinderMain::ID_QUIT);

  // Create window
  application.create();

  // If given, start in indicated directory
  if(argc==2) window->setDirectory(argv[1]);

  // Run the app now...
  return application.run();
  }


