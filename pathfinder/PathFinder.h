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
* $Id: PathFinder.h,v 1.20 2001/12/10 23:37:59 jeroen Exp $                     *
********************************************************************************/
#ifndef PATHFINDER_H
#define PATHFINDER_H


class PropertyDialog;


// PathFinder Main Window
class PathFinderMain : public FXMainWindow {
  FXDECLARE(PathFinderMain)
protected:
  PropertyDialog    *property;
  FXMenuPane        *filemenu;
  FXMenuPane        *editmenu;
  FXMenuPane        *commandmenu;
  FXMenuPane        *viewmenu;
  FXMenuPane        *gomenu;
  FXMenuPane        *arrangemenu;
  FXMenuPane        *sortmenu;
  FXMenuPane        *prefmenu;
  FXMenuPane        *helpmenu;
  FXMenuPane        *bookmarkmenu;
  FXDirBox          *dirbox;
  FXVerticalFrame   *group1;
  FXVerticalFrame   *subgroup1;
  FXVerticalFrame   *group2;
  FXVerticalFrame   *subgroup2;
  FXDirList         *dirlist;
  FXFileList        *filelist;
  FXComboBox        *pattern;
  FXRecentFiles      bookmarkeddirs;
  FXFileDict        *associations;
  FXIcon            *bigp;
  FXIcon            *minip;
  FXIcon            *cuticon;
  FXIcon            *copyicon;
  FXIcon            *moveicon;
  FXIcon            *pasteicon;
  FXIcon            *upicon;
  FXIcon            *homeicon;
  FXIcon            *backicon;
  FXIcon            *forwicon;
  FXIcon            *bigiconsicon;
  FXIcon            *miniiconsicon;
  FXIcon            *detailsicon;
  FXIcon            *mapicon;
  FXIcon            *unmapicon;
  FXIcon            *propicon;
  FXIcon            *deleteicon;
  FXIcon            *setbookicon;
  FXIcon            *clrbookicon;
  FXIcon            *workicon;
  FXString           visiteddir[10];
  FXint              visiting;
protected:
  PathFinderMain(){}
  void visitDirectory(const FXString& dir);
  void setPatterns(const FXString& patterns);
  FXString getPatterns() const;
  void setCurrentPattern(FXint n);
  FXint getCurrentPattern() const;
public:
  enum{
    ID_ABOUT=FXMainWindow::ID_LAST,
    ID_FILEPATTERNS,
    ID_FILEFILTER,
    ID_DIRECTORYLIST,
    ID_FILELIST,
    ID_DIRBOX,
    ID_SEARCHPATH,
    ID_UPDIRECTORY,
    ID_QUIT,
    ID_MAINWINDOW,
    ID_UPDATE_FILES,
    ID_GO_WORK,
    ID_GO_HOME,
    ID_GO_BACK,
    ID_GO_FORWARD,
    ID_GO_RECENT,
    ID_SAVE_SETTINGS,
    ID_NEW_PATHFINDER,
    ID_STATUSLINE,
    ID_PROPERTIES,
    ID_BOOKMARK,
    ID_DELETE,
    ID_NEW,
    ID_GOTO_DIR,
    ID_OPEN,
    ID_OPEN_WITH,
    ID_RUN,
    ID_TERMINAL,
    ID_RUSR,          // File modes
    ID_WUSR,
    ID_XUSR,
    ID_RGRP,
    ID_WGRP,
    ID_XGRP,
    ID_ROTH,
    ID_WOTH,
    ID_XOTH,
    ID_SUID,
    ID_SGID,
    ID_SVTX,
    ID_OWNER,         // File ownership
    ID_GROUP,
    ID_FILE_CREATED,
    ID_FILE_ACCESSED,
    ID_FILE_MODIFIED,
    ID_FILE_TYPE,
    ID_FILE_LOCATION,
    ID_FILE_SIZE,
    ID_FILE_DESC,
    ID_HARVEST,
    ID_LAST
    };
public:
  long onCmdAbout(FXObject*,FXSelector,void*);
  long onCmdQuit(FXObject*,FXSelector,void*);
  long onCmdDirTree(FXObject*,FXSelector,void*);
  long onCmdDirectory(FXObject*,FXSelector,void*);
  long onCmdFileClicked(FXObject*,FXSelector,void*);
  long onCmdFileDblClicked(FXObject*,FXSelector,void*);
  long onCmdSetSearchPath(FXObject*,FXSelector,void*);
  long onCmdUpDirectory(FXObject*,FXSelector,void*);
  long onUpdTitle(FXObject*,FXSelector,void*);
  long onUpdFiles(FXObject*,FXSelector,void*);
  long onCmdHomeDirectory(FXObject*,FXSelector,void*);
  long onCmdWorkDirectory(FXObject*,FXSelector,void*);
  long onCmdRecentDirectory(FXObject*,FXSelector,void*);
  long onCmdSaveSettings(FXObject*,FXSelector,void*);
  long onCmdNewPathFinder(FXObject*,FXSelector,void*);
  long onFileListPopup(FXObject*,FXSelector,void*);
  long onUpdStatusline(FXObject*,FXSelector,void*);
  long onCmdProperties(FXObject*,FXSelector,void*);
  long onUpdProperties(FXObject*,FXSelector,void*);
  long onCmdBookmark(FXObject*,FXSelector,void*);
  long onCmdBackDirectory(FXObject*,FXSelector,void*);
  long onUpdBackDirectory(FXObject*,FXSelector,void*);
  long onCmdForwardDirectory(FXObject*,FXSelector,void*);
  long onUpdForwardDirectory(FXObject*,FXSelector,void*);
  long onCmdDelete(FXObject*,FXSelector,void*);
  long onUpdDelete(FXObject*,FXSelector,void*);
  long onCmdNew(FXObject*,FXSelector,void*);
  long onUpdNew(FXObject*,FXSelector,void*);
  long onCmdFilePatterns(FXObject*,FXSelector,void*);
  long onCmdFilter(FXObject*,FXSelector,void*);
  long onCmdGotoDir(FXObject*,FXSelector,void*);
  long onCmdChmod(FXObject*,FXSelector,void*);
  long onUpdChmod(FXObject*,FXSelector,void*);
  long onUpdOwner(FXObject*,FXSelector,void*);
  long onUpdGroup(FXObject*,FXSelector,void*);
  long onUpdCreateTime(FXObject*,FXSelector,void*);
  long onUpdModifyTime(FXObject*,FXSelector,void*);
  long onUpdAccessTime(FXObject*,FXSelector,void*);
  long onUpdFileType(FXObject*,FXSelector,void*);
  long onUpdFileLocation(FXObject*,FXSelector,void*);
  long onUpdFileSize(FXObject*,FXSelector,void*);
  long onUpdFileDesc(FXObject*,FXSelector,void*);
  long onUpdOpen(FXObject*,FXSelector,void*);
  long onCmdOpen(FXObject*,FXSelector,void*);
  long onUpdOpenWith(FXObject*,FXSelector,void*);
  long onCmdOpenWith(FXObject*,FXSelector,void*);
  long onCmdRun(FXObject*,FXSelector,void*);
  long onCmdTerminal(FXObject*,FXSelector,void*);
  long onSigHarvest(FXObject*,FXSelector,void*);
public:
  static FXchar* pathfindercommand;
public:
  PathFinderMain(FXApp* a);
  void setDirectory(const FXString& dir);
  FXString getDirectory() const;
  virtual void create();
  void saveSettings();
  void loadSettings();
  virtual ~PathFinderMain();
  };


#endif
