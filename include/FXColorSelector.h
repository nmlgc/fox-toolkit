/********************************************************************************
*                                                                               *
*                          C o l o r   S e l e c t o r                          *
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
* $Id: FXColorSelector.h,v 1.7 1998/07/21 06:04:16 jeroen Exp $                 *
********************************************************************************/
#ifndef FXCOLORSELECTOR_H
#define FXCOLORSELECTOR_H


// File selection widget
class FXColorSelector : public FXPacker {
  FXDECLARE(FXColorSelector)
protected:
  FXColorWell *well;
  FXList      *list;
  FXButton    *accept;
  FXButton    *cancel;
  FXSlider    *rgbaslider[4];
  FXTextField *rgbatext[4];
  FXSlider    *hsvaslider[4];
  FXTextField *hsvatext[4];
  FXfloat      rgba[4];
  FXfloat      hsva[4];
protected:
  FXColorSelector(){}
  FXColorSelector(const FXColorSelector&){}
  void updateRGBText();
  void updateHSVText();
  void updateRGBSliders();
  void updateHSVSliders();
  void updateWell();
public:
  long onCmdWell(FXObject*,FXSelector,void*);
  long onCmdRGBSlider(FXObject*,FXSelector,void*);
  long onCmdRGBText(FXObject*,FXSelector,void*);
  long onCmdHSVSlider(FXObject*,FXSelector,void*);
  long onCmdHSVText(FXObject*,FXSelector,void*);
  long onCmdList(FXObject*,FXSelector,void*);
public:
  enum{
    ID_RGB_RED_SLIDER=FXPacker::ID_LAST,
    ID_RGB_GREEN_SLIDER,
    ID_RGB_BLUE_SLIDER,
    ID_RGB_ALPHA_SLIDER,
    ID_RGB_RED_TEXT,
    ID_RGB_GREEN_TEXT,
    ID_RGB_BLUE_TEXT,
    ID_RGB_ALPHA_TEXT,
    ID_HSV_HUE_SLIDER,
    ID_HSV_SATURATION_SLIDER,
    ID_HSV_VALUE_SLIDER,
    ID_HSV_ALPHA_SLIDER,
    ID_HSV_HUE_TEXT,
    ID_HSV_SATURATION_TEXT,
    ID_HSV_VALUE_TEXT,
    ID_HSV_ALPHA_TEXT,
    ID_COLOR_LIST,
    ID_WELL_CHANGED,
    ID_LAST
    };
public:
  FXColorSelector(FXComposite *p,FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0,FXint x=0,FXint y=0,FXint w=0,FXint h=0);
  virtual void create();
  FXButton *acceptButton() const { return accept; }
  FXButton *cancelButton() const { return cancel; }
  void setRGBA(FXColor clr);
  FXColor getRGBA() const;
  virtual ~FXColorSelector();
  };


#endif
