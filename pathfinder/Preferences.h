/********************************************************************************
*                                                                               *
*                        P r e f e r e n c e s   D i a l o g                    *
*                                                                               *
*********************************************************************************
* Copyright (C) 2003 by Jeroen van der Zijp.   All Rights Reserved.             *
*********************************************************************************
* This program is free software; you can redistribute it and/or modify          *
* it under the terms of the GNU General Public License as published by          *
* the Free Software Foundation; either version 2 of the License, or             *
* (at your option) any later version.                                           *
*                                                                               *
* This program is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
* GNU General Public License for more details.                                  *
*                                                                               *
* You should have received a copy of the GNU General Public License             *
* along with this program; if not, write to the Free Software                   *
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.    *
*********************************************************************************
* $Id: Preferences.h,v 1.4 2003/11/27 06:59:24 fox Exp $                        *
********************************************************************************/
#ifndef PREFERENCES_H
#define PREFERENCES_H



class PathFinderMain;


// Preferences for PathFinder
class Preferences : public FXDialogBox {
  FXDECLARE(Preferences)
protected:
  FXText          *pattern;
  FXTextField     *editor;
  FXTextField     *terminal;
  FXCheckButton   *preview;
  FXCheckButton   *blending;
  FXText          *icondirs;
  FXIcon          *brw;
  FXIcon          *pat;
  FXIcon          *icp;
private:
  Preferences(){}
  Preferences(const Preferences&);
  Preferences& operator=(const Preferences&);
public:
  long onCmdBrowseEditor(FXObject*,FXSelector,void*);
  long onCmdBrowseTerminal(FXObject*,FXSelector,void*);
public:
  enum{
    ID_EDITOR=FXDialogBox::ID_LAST,
    ID_TERMINAL
    };
public:

  // Create preferences dialog
  Preferences(PathFinderMain *owner);

  // Get/set filename patterns
  void setPatterns(const FXString& pat){ pattern->setText(pat); }
  FXString getPatterns() const { return pattern->getText(); }

  // Get/set text editor
  void setEditor(const FXString& edit){ editor->setText(edit); }
  FXString getEditor() const { return editor->getText(); }

  // Get/set terminal
  void setTerminal(const FXString& term){ terminal->setText(term); }
  FXString getTerminal() const { return terminal->getText(); }

  // Set image preview
  void setPreview(FXbool prev){ preview->setCheck(prev); }
  FXbool getPreview() const { return preview->getCheck(); }

  // Set image preview
  void setBlend(FXbool blend){ blending->setCheck(blend); }
  FXbool getBlend() const { return blending->getCheck(); }

  // Get/set icon path
  void setIconPath(const FXString& text){ icondirs->setText(text); }
  FXString getIconPath() const { return icondirs->getText(); }

  // Clean up
  virtual ~Preferences();
  };

#endif
