/********************************************************************************
*                                                                               *
*                        P r e f e r e n c e s   D i a l o g                    *
*                                                                               *
*********************************************************************************
* Copyright (C) 2001 by Jeroen van der Zijp.   All Rights Reserved.             *
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
* $Id: Preferences.h,v 1.2 2001/03/15 05:11:05 jeroen Exp $                     *
********************************************************************************/
#ifndef PREFERENCES_H
#define PREFERENCES_H

class Calculator;

class FXAPI Preferences : public FXDialogBox {
  FXDECLARE(Preferences)
protected:
  FXIcon *palette;
  FXIcon *calculator;
  FXIcon *info;
private:
  Preferences(){}
  Preferences(const Preferences&);
public:
  Preferences(Calculator *owner);
  virtual ~Preferences();
  };

#endif
