/********************************************************************************
*                                                                               *
*                        P r e f e r e n c e s   D i a l o g                    *
*                                                                               *
*********************************************************************************
* Copyright (C) 2001,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: Preferences.h,v 1.21.4.1 2002/04/30 13:13:07 fox Exp $                       *
********************************************************************************/
#ifndef PREFERENCES_H
#define PREFERENCES_H


//////////////////////////////  UNDER DEVELOPMENT  //////////////////////////////


class TextWindow;


class Preferences : public FXDialogBox {
  FXDECLARE(Preferences)
protected:
  FXText          *filepattext;
  FXList          *langlist;
  FXTextField     *langname;
  FXTextField     *langext;
  FXListBox       *langbox;
  FXList          *stylelist;
  FXTextField     *stylename;
  FXList          *patternlist;
  FXTextField     *patternname;
  FXComboBox      *parentcombo;
  FXComboBox      *stylecombo;
  FXText          *patterntext;
  FXTextField     *contexttext;
  FXSpinner       *priospinner;
  FXIcon          *pal;
  FXIcon          *ind;
  FXIcon          *pat;
  FXIcon          *sty;
  FXIcon          *syn;
  FXIcon          *lng;
private:
  Preferences(){}
  Preferences(const Preferences&);
  Preferences& operator=(const Preferences&);
public:
  long onCmdStyleName(FXObject*,FXSelector,void*);
  long onUpdStyleName(FXObject*,FXSelector,void*);
  long onCmdStyleFlags(FXObject*,FXSelector,void*);
  long onUpdStyleFlags(FXObject*,FXSelector,void*);
  long onCmdStyleNew(FXObject*,FXSelector,void*);
  long onUpdStyleNew(FXObject*,FXSelector,void*);
  long onCmdStyleDelete(FXObject*,FXSelector,void*);
  long onUpdStyleDelete(FXObject*,FXSelector,void*);
  long onCmdStyleColor(FXObject*,FXSelector,void*);
  long onUpdStyleColor(FXObject*,FXSelector,void*);

  long onCmdLangIndex(FXObject*,FXSelector,void*);
  long onCmdLangNew(FXObject*,FXSelector,void*);
  long onCmdLangDelete(FXObject*,FXSelector,void*);
  long onCmdLangChanged(FXObject*,FXSelector,void*);
  long onUpdLangSelected(FXObject*,FXSelector,void*);

  long onCmdSyntaxLang(FXObject*,FXSelector,void*);
  long onCmdSyntaxIndex(FXObject*,FXSelector,void*);
  long onCmdSyntaxNew(FXObject*,FXSelector,void*);
  long onUpdSyntaxNew(FXObject*,FXSelector,void*);
  long onCmdSyntaxDelete(FXObject*,FXSelector,void*);
  long onCmdSyntaxChanged(FXObject*,FXSelector,void*);
  long onUpdSyntaxChanged(FXObject*,FXSelector,void*);
public:
  enum{
    ID_STYLE_NAME=FXDialogBox::ID_LAST,
    ID_STYLE_DELETE,
    ID_STYLE_NEW,
    ID_STYLE_NORMAL_FG,
    ID_STYLE_NORMAL_BG,
    ID_STYLE_SELECT_FG,
    ID_STYLE_SELECT_BG,
    ID_STYLE_HILITE_FG,
    ID_STYLE_HILITE_BG,
    ID_STYLE_ACTIVE_BG,
    ID_STYLE_UNDERLINE,
    ID_STYLE_STRIKEOUT,

    ID_LANG_INDEX,
    ID_LANG_NEW,
    ID_LANG_DELETE,
    ID_LANG_CHANGED,

    ID_SYNTAX_LANG,
    ID_SYNTAX_INDEX,
    ID_SYNTAX_NEW,
    ID_SYNTAX_DELETE,
    ID_SYNTAX_CHANGED,
    ID_SYNTAX_PATTERN,
    ID_LAST
    };
public:

  // Create preferences dialog
  Preferences(TextWindow *owner);

  // Owner is text window
  TextWindow* getOwner() const { return (TextWindow*)FXDialogBox::getOwner(); }

  // Set filename patterns
  void setPatterns(const FXString& patterns);

  // Get filename patterns
  FXString getPatterns() const;

  // Clean up
  virtual ~Preferences();
  };

#endif
