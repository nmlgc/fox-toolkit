/********************************************************************************
*                                                                               *
*                     T h e   A d i e   T e x t   E d i t o r                   *
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
* $Id: Adie.h,v 1.26.4.1 2002/04/30 13:13:07 fox Exp $                              *
********************************************************************************/
#ifndef ADIE_H
#define ADIE_H


/*******************************************************************************/

class HelpWindow;
class Preferences;


// Number of highlight styles
#define MAXSTYLES   64



// Mini application object
class TextWindow : public FXMainWindow {
  FXDECLARE(TextWindow)
protected:
  struct Bookmark {
    FXint top;                              // Top position
    FXint pos;                              // Character position
    };
protected:
  HelpWindow          *helpwindow;          // Help window
  FXMenuPane          *filemenu;            // File menu
  FXMenuPane          *editmenu;            // Edit menu
  FXMenuPane          *gotomenu;            // Goto menu
  FXMenuPane          *searchmenu;          // Search menu
  FXMenuPane          *optionmenu;          // Option menu
  FXMenuPane          *viewmenu;            // View menu
  FXMenuPane          *helpmenu;            // Help menu
  FXMenuPane          *popupmenu;           // Popup menu
  FXVerticalFrame     *treebox;             // Tree box containing directories/files
  FXText              *editor;              // Multiline text widget
  FXDirList           *dirlist;             // Directory view
  FXComboBox          *filter;              // Combobox for pattern list
  FXTextField         *clock;               // Time
  FXMenubar           *menubar;             // Menu bar
  FXToolbar           *toolbar;             // Tool bar
  FXStatusbar         *statusbar;           // Status bar
  FXTimer             *clocktimer;          // Periodically update clock
  FXTimer             *filetimer;           // Periodically check if file changed outside text editor
  FXFont              *font;		    // Text window font
  FXIcon              *bigicon;             // Big application icon
  FXIcon              *smallicon;           // Small application icon
  FXIcon              *newicon;
  FXIcon              *reloadicon;
  FXIcon              *openicon;
  FXIcon              *saveicon;
  FXIcon              *saveasicon;
  FXIcon              *printicon;
  FXIcon              *cuticon;
  FXIcon              *copyicon;
  FXIcon              *pasteicon;
  FXIcon              *deleteicon;
  FXIcon              *undoicon;
  FXIcon              *redoicon;
  FXIcon              *fontsicon;
  FXIcon              *helpicon;
  FXIcon              *quiticon;
  FXIcon              *searchicon;
  FXIcon              *searchnexticon;
  FXIcon              *searchprevicon;
  FXIcon              *bookseticon;
  FXIcon              *booknexticon;
  FXIcon              *bookprevicon;
  FXIcon              *bookdelicon;
  FXIcon              *shiftlefticon;
  FXIcon              *shiftrighticon;
  Bookmark             bookmark[10];          // Book marks
  FXString             language;              // Current language
  Hilite               hilite;                // Hilite engine
  FXHiliteStyle        stylecolor[MAXSTYLES]; // Text style colors
  FXString             stylename[MAXSTYLES];  // Text style names
  FXint                nstyles;               // Number of styles
  FXUndoList           undolist;              // Undo list
  FXRecentFiles        mrufiles;              // Recent files
  FXString             filename;              // File being edited
  long                 filetime;              // Original modtime of file
  FXbool               filenameset;           // Filename is set
  FXString             searchpath;            // To search for files
  FXbool               stripcr;               // Strip carriage returns
  FXbool               stripsp;               // Strip trailing spaces
  FXbool               saveviews;             // Save views of files
  FXbool               savemarks;             // Save bookmarks of files
  FXbool               warnchanged;           // Warn if changed by other program
protected:
  void readRegistry();
  void writeRegistry();
  FXbool saveChanges();
protected:
  enum{
    MAXUNDOSIZE    = 1000000,               // Don't let the undo buffer get out of hand
    KEEPUNDOSIZE   = 500000                 // When MAXUNDOSIZE was exceeded, trim down to this size
    };
private:
  TextWindow(){}
  TextWindow(const TextWindow&);
  TextWindow& operator=(const TextWindow&);
public:
  long onCmdAbout(FXObject*,FXSelector,void*);
  long onCmdOpen(FXObject*,FXSelector,void*);
  long onCmdOpenSelected(FXObject*,FXSelector,void*);
  long onCmdReopen(FXObject*,FXSelector,void*);
  long onUpdReopen(FXObject*,FXSelector,void*);
  long onCmdSave(FXObject*,FXSelector,void*);
  long onUpdSave(FXObject*,FXSelector,void*);
  long onCmdSaveAs(FXObject*,FXSelector,void*);
  long onCmdNew(FXObject*,FXSelector,void*);
  long onCmdFont(FXObject*,FXSelector,void*);
  long onCmdQuit(FXObject*,FXSelector,void*);
  long onCmdPrint(FXObject*,FXSelector,void*);
  long onUpdTitle(FXObject*,FXSelector,void*);
  long onCmdTreeList(FXObject*,FXSelector,void*);
  long onCmdTextBackColor(FXObject*,FXSelector,void*);
  long onUpdTextBackColor(FXObject*,FXSelector,void*);
  long onCmdTextForeColor(FXObject*,FXSelector,void*);
  long onUpdTextForeColor(FXObject*,FXSelector,void*);
  long onCmdTextSelBackColor(FXObject*,FXSelector,void*);
  long onUpdTextSelBackColor(FXObject*,FXSelector,void*);
  long onCmdTextSelForeColor(FXObject*,FXSelector,void*);
  long onUpdTextSelForeColor(FXObject*,FXSelector,void*);
  long onCmdTextHiliteBackColor(FXObject*,FXSelector,void*);
  long onUpdTextHiliteBackColor(FXObject*,FXSelector,void*);
  long onCmdTextHiliteForeColor(FXObject*,FXSelector,void*);
  long onUpdTextHiliteForeColor(FXObject*,FXSelector,void*);
  long onCmdTextCursorColor(FXObject*,FXSelector,void*);
  long onUpdTextCursorColor(FXObject*,FXSelector,void*);
  long onCmdTextActBackColor(FXObject*,FXSelector,void*);
  long onUpdTextActBackColor(FXObject*,FXSelector,void*);
  long onCmdTextBarColor(FXObject*,FXSelector,void*);
  long onUpdTextBarColor(FXObject*,FXSelector,void*);
  long onCmdTextNumberColor(FXObject*,FXSelector,void*);
  long onUpdTextNumberColor(FXObject*,FXSelector,void*);
  long onCmdDirBackColor(FXObject*,FXSelector,void*);
  long onUpdDirBackColor(FXObject*,FXSelector,void*);
  long onCmdDirForeColor(FXObject*,FXSelector,void*);
  long onUpdDirForeColor(FXObject*,FXSelector,void*);
  long onCmdDirSelBackColor(FXObject*,FXSelector,void*);
  long onUpdDirSelBackColor(FXObject*,FXSelector,void*);
  long onCmdDirSelForeColor(FXObject*,FXSelector,void*);
  long onUpdDirSelForeColor(FXObject*,FXSelector,void*);
  long onCmdDirLineColor(FXObject*,FXSelector,void*);
  long onUpdDirLineColor(FXObject*,FXSelector,void*);
  long onCmdRecentFile(FXObject*,FXSelector,void*);
  long onCmdSaveSettings(FXObject*,FXSelector,void*);
  long onTextInserted(FXObject*,FXSelector,void*);
  long onTextDeleted(FXObject*,FXSelector,void*);
  long onTextRightMouse(FXObject*,FXSelector,void*);
  long onTextChanged(FXObject*,FXSelector,void*);
  long onCmdWrap(FXObject*,FXSelector,void*);
  long onUpdWrap(FXObject*,FXSelector,void*);
  long onCmdWrapFixed(FXObject*,FXSelector,void*);
  long onUpdWrapFixed(FXObject*,FXSelector,void*);
  long onEditDNDMotion(FXObject*,FXSelector,void*);
  long onEditDNDDrop(FXObject*,FXSelector,void*);
  long onCmdStripReturns(FXObject*,FXSelector,void*);
  long onUpdStripReturns(FXObject*,FXSelector,void*);
  long onCmdStripSpaces(FXObject*,FXSelector,void*);
  long onUpdStripSpaces(FXObject*,FXSelector,void*);
  long onCmdIncludePaths(FXObject*,FXSelector,void*);
  long onCmdShowHelp(FXObject*,FXSelector,void*);
  long onCmdFilter(FXObject*,FXSelector,void*);
  long onUpdOverstrike(FXObject*,FXSelector,void*);
  long onUpdReadOnly(FXObject*,FXSelector,void*);
  long onUpdNumChars(FXObject*,FXSelector,void*);
  long onCheckFile(FXObject*,FXSelector,void*);
  long onClock(FXObject*,FXSelector,void*);
  long onCmdPreferences(FXObject*,FXSelector,void*);
  long onCmdTabColumns(FXObject*,FXSelector,void*);
  long onUpdTabColumns(FXObject*,FXSelector,void*);
  long onCmdDelimiters(FXObject*,FXSelector,void*);
  long onUpdDelimiters(FXObject*,FXSelector,void*);
  long onCmdWrapColumns(FXObject*,FXSelector,void*);
  long onUpdWrapColumns(FXObject*,FXSelector,void*);
  long onCmdInsertTabs(FXObject*,FXSelector,void*);
  long onUpdInsertTabs(FXObject*,FXSelector,void*);
  long onCmdAutoIndent(FXObject*,FXSelector,void*);
  long onUpdAutoIndent(FXObject*,FXSelector,void*);
  long onCmdBraceMatch(FXObject*,FXSelector,void*);
  long onUpdBraceMatch(FXObject*,FXSelector,void*);
  long onCmdInsertFile(FXObject*,FXSelector,void*);
  long onUpdInsertFile(FXObject*,FXSelector,void*);
  long onCmdExtractFile(FXObject*,FXSelector,void*);
  long onUpdExtractFile(FXObject*,FXSelector,void*);
  long onCmdWheelAdjust(FXObject*,FXSelector,void*);
  long onUpdWheelAdjust(FXObject*,FXSelector,void*);
  long onCmdNextMark(FXObject*,FXSelector,void*);
  long onUpdNextMark(FXObject*,FXSelector,void*);
  long onCmdPrevMark(FXObject*,FXSelector,void*);
  long onUpdPrevMark(FXObject*,FXSelector,void*);
  long onCmdSetMark(FXObject*,FXSelector,void*);
  long onUpdSetMark(FXObject*,FXSelector,void*);
  long onCmdClearMarks(FXObject*,FXSelector,void*);
  long onCmdSaveMarks(FXObject*,FXSelector,void*);
  long onUpdSaveMarks(FXObject*,FXSelector,void*);
  long onCmdSaveViews(FXObject*,FXSelector,void*);
  long onUpdSaveViews(FXObject*,FXSelector,void*);
  long onCmdShowActive(FXObject*,FXSelector,void*);
  long onUpdShowActive(FXObject*,FXSelector,void*);
  long onCmdLineNumbers(FXObject*,FXSelector,void*);
  long onUpdLineNumbers(FXObject*,FXSelector,void*);
  long onCmdWarnChanged(FXObject*,FXSelector,void*);
  long onUpdWarnChanged(FXObject*,FXSelector,void*);
  long onCmdSyntax(FXObject*,FXSelector,void*);
  long onUpdSyntax(FXObject*,FXSelector,void*);
  long onCmdRestyle(FXObject*,FXSelector,void*);
  long onUpdRestyle(FXObject*,FXSelector,void*);

public:
  enum{
    ID_ABOUT=FXMainWindow::ID_LAST,
    ID_FILEFILTER,
    ID_OPEN,
    ID_OPEN_SELECTED,
    ID_REOPEN,
    ID_SAVE,
    ID_SAVEAS,
    ID_NEW,
    ID_TITLE,
    ID_FONT,
    ID_QUIT,
    ID_PRINT,
    ID_TREELIST,
    ID_TEXT_BACK,
    ID_TEXT_FORE,
    ID_TEXT_SELBACK,
    ID_TEXT_SELFORE,
    ID_TEXT_HILITEBACK,
    ID_TEXT_HILITEFORE,
    ID_TEXT_ACTIVEBACK,
    ID_TEXT_CURSOR,
    ID_TEXT_NUMBACK,
    ID_TEXT_NUMFORE,
    ID_TEXT_LINENUMS,
    ID_DIR_BACK,
    ID_DIR_FORE,
    ID_DIR_SELBACK,
    ID_DIR_SELFORE,
    ID_DIR_LINES,
    ID_RECENTFILE,
    ID_TOGGLE_WRAP,
    ID_FIXED_WRAP,
    ID_SAVE_SETTINGS,
    ID_TEXT,
    ID_STRIP_CR,
    ID_STRIP_SP,
    ID_INCLUDE_PATH,
    ID_SHOW_HELP,
    ID_OVERSTRIKE,
    ID_READONLY,
    ID_FILETIME,
    ID_CLOCKTIME,
    ID_PREFERENCES,
    ID_TABCOLUMNS,
    ID_WRAPCOLUMNS,
    ID_DELIMITERS,
    ID_INSERTTABS,
    ID_AUTOINDENT,
    ID_BRACEMATCH,
    ID_NUMCHARS,
    ID_INSERT_FILE,
    ID_EXTRACT_FILE,
    ID_WHEELADJUST,
    ID_SET_MARK,
    ID_NEXT_MARK,
    ID_PREV_MARK,
    ID_CLEAR_MARKS,
    ID_SAVEMARKS,
    ID_SAVEVIEWS,
    ID_SHOWACTIVE,
    ID_WARNCHANGED,
    ID_SYNTAX,
    ID_RESTYLE,
    ID_LAST
    };
public:

  // Create new text window
  TextWindow(FXApp* a);

  // Create window
  virtual void create();

  // Load text from file
  FXbool loadFile(const FXString& file);

  // Save text to file
  FXbool saveFile(const FXString& file);

  // Insert file at cursor
  FXbool insertFile(const FXString& file);

  // Extract selection to file
  FXbool extractFile(const FXString& file);

  // Change pattern list
  void setPatterns(const FXString& patterns);

  // Get pattern list
  FXString getPatterns() const;

  // Change current file pattern
  void setCurrentPattern(FXint n);

  // Return current file pattern
  FXint getCurrentPattern() const;

  // Add bookmark at current cursor position
  void setBookmark(FXint pos,FXint top);

  // Goto bookmark
  void gotoBookmark(FXint b);

  // Clear bookmarks
  void clearBookmarks();

  // Read/write bookmarks
  void readBookmarks(const FXString& file);
  void writeBookmarks(const FXString& file);

  // Read/write view
  void readView(const FXString& file);
  void writeView(const FXString& file);

  // Style management
  void readStyles();
  void writeStyles();

  // Determine language from filename
  FXString getLanguage(const FXString& filename);

  // Read syntax for language
  void readSyntax(const FXString& language);

  // Return number of styles
  FXint getNumStyles() const { return nstyles; }

  // Delete style
  void deleteStyle(FXint index);

  // Append style
  void appendStyle(const FXString& name,const FXHiliteStyle& style);

  // Get style name
  const FXString& getStyleName(FXint index) const { return stylename[index]; }

  // Change style name
  void setStyleName(FXint index,const FXString& name){ stylename[index]=name; }

  // Get style colors
  const FXHiliteStyle& getStyleColors(FXint index) const { return stylecolor[index]; }

  // Change style colors
  void setStyleColors(FXint index,const FXHiliteStyle& style);

  // Delete text window
  virtual ~TextWindow();
  };

#endif

