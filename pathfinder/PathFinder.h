/********************************************************************************
*                                                                               *
*              T h e   P a t h F i n d e r   F i l e   B r o w s e r            *
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
* $Id: PathFinder.h,v 1.5 1998/09/22 02:54:18 jeroen Exp $                      *
********************************************************************************/
#ifndef PATHFINDER_H
#define PATHFINDER_H


// Mini application object
class PathFinderApp : public FXApp {
  FXDECLARE(PathFinderApp)
public:
  long onQuit(FXObject*,FXSelector,void*);
  long onCmdDirectory(FXObject*,FXSelector,void*);
  long onCmdItemClicked(FXObject*,FXSelector,void*);
  long onCmdItemDblClicked(FXObject*,FXSelector,void*);
protected:
  FXMainWindow*      mainwindow;
  FXMenuBar*         menubar;
  FXHorizontalFrame* toolbar;
  FXSplitter*        splitter;
  FXVerticalFrame*   group1;
  FXVerticalFrame*   subgroup1;
  FXVerticalFrame*   group2;
  FXVerticalFrame*   subgroup2;
  FXDirList*         dirlist;
  FXFileList*        filelist;
  FXTextField*       pattern;
public:
  enum{
    ID_ABOUT=FXApp::ID_LAST,
    ID_DIRECTORY,
    ID_FILELIST,
    ID_LAST
    };
public:
  long onCmdAbout(FXObject*,FXSelector,void*);
public:
  PathFinderApp();
  void create();
  void start();
  };


#endif
