/********************************************************************************
*                                                                               *
*                      P r o g r e s s B a r   W i d g e t                      *
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
* $Id: FXProgressBar.h,v 1.6 1998/08/28 22:58:47 jvz Exp $                   *
********************************************************************************/
#ifndef FXPROGRESSBAR_H
#define FXPROGRESSBAR_H


// Progress bar styles
enum FXProgressBarStyle {
  PROGRESSBAR_PERCENTAGE = 0x00008000,    // Show percentage done
  PROGRESSBAR_VERTICAL   = 0x00010000     // Vertical display
  };


// Progress bar widget
class FXProgressBar : public FXCell {
  FXDECLARE(FXProgressBar)
protected:
  FXuint   progress;                      // Integer percentage number
  FXuint   total;                         // Amount for completion
  FXint    barsize;                       // Bar size
  FXFont*  font;
  FXPixel  barBGColor;
  FXPixel  barColor;
  FXPixel  textNumColor;
  FXPixel  textAltColor;
protected:
  FXProgressBar(){}
  FXProgressBar(const FXProgressBar&){}
public:
  long onPaint(FXObject*,FXSelector,void*);
  long onCmdSetValue(FXObject*,FXSelector,void*);
public:
  FXProgressBar(FXComposite* p,FXObject* target=NULL,FXSelector sel=0,FXuint opts=(FRAME_SUNKEN|FRAME_THICK),FXint x=0,FXint y=0,FXint w=0,FXint h=0,FXint pl=DEFAULT_PAD,FXint pr=DEFAULT_PAD,FXint pt=DEFAULT_PAD,FXint pb=DEFAULT_PAD);
  virtual FXint getDefaultWidth();
  virtual FXint getDefaultHeight();
  void create();
  void setProgress(FXuint value);
  FXuint getProgress() const { return progress; }
  void setTotal(FXuint value);
  FXuint getTotal() const { return total; }
  void increment(FXuint value);
  void hideNumber();
  void showNumber();
  void setBarSize(FXint size);
  FXint getBarSize() const { return barsize; }
  void setBarBGColor(FXPixel clr);
  FXPixel getBarBGColor() const { return barBGColor; }
  void setBarColor(FXPixel clr);
  FXPixel getBarColor() const { return barColor; }
  void setTextColor(FXPixel clr);
  FXPixel getTextColor() const { return textNumColor; }
  void setTextAltColor(FXPixel clr);
  FXPixel getTextAltColor() const { return textAltColor; }
  void setFont(FXFont *fnt);
  FXFont* getFont() const { return font; }
  virtual ~FXProgressBar();
  };
    

#endif
