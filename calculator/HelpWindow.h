/********************************************************************************
*                                                                               *
*                            H e l p   W i n d o w                              *
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
* $Id: HelpWindow.h,v 1.1.4.1 2002/04/30 13:13:07 fox Exp $                      *
********************************************************************************/
#ifndef HELPWINDOW_H
#define HELPWINDOW_H


class FXText;


/// Online help dialog box
class HelpWindow : public FXDialogBox {
  FXDECLARE(HelpWindow)
protected:
  FXText *helptext;         // Help display
private:
  HelpWindow(){}
  HelpWindow(const HelpWindow&);
public:
  HelpWindow(FXWindow *owner,const FXString& title);
  void setHelp(const FXString& help);
  FXString getHelp() const;
  virtual ~HelpWindow();
  };

#endif
