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
* $Id: FXColorSelector.cpp,v 1.24 1998/09/25 23:23:42 jvz Exp $               *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "fxkeys.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXObject.h"
#include "FXAccelTable.h"
#include "FXObjectList.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXFont.h"
#include "FXDrawable.h"
#include "FXImage.h"
#include "FXIcon.h"
#include "FXGIFIcon.h"
#include "FXWindow.h"
#include "FXFrame.h"
#include "FXLabel.h"
#include "FXColorWell.h"
#include "FXTextField.h"
#include "FXButton.h"
#include "FXComposite.h"
#include "FXPacker.h"
#include "FXTab.h"
#include "FXHorizontalFrame.h"
#include "FXVerticalFrame.h"
#include "FXMatrix.h"
#include "FXCanvas.h"
#include "FXShell.h"
#include "FXScrollbar.h"
#include "FXSlider.h"
#include "FXScrollWindow.h"
#include "FXList.h"
#include "FXColorSelector.h"


/*
  To do:
  
  HSV Model:
  
    green          yellow
          o-----o
         /       \
        /  white  \
  cyan o     o     o red
        \         /
         \       /
          o-----o
    blue           magenta
*/

/*******************************************************************************/

  
// Map
FXDEFMAP(FXColorSelector) FXColorSelectorMap[]={
  FXMAPFUNC(SEL_CHANGED,FXColorSelector::ID_WELL_CHANGED,FXColorSelector::onCmdWell),
  FXMAPFUNCS(SEL_CHANGED,FXColorSelector::ID_RGB_RED_SLIDER,FXColorSelector::ID_RGB_ALPHA_SLIDER,FXColorSelector::onCmdRGBSlider),
  FXMAPFUNCS(SEL_COMMAND,FXColorSelector::ID_RGB_RED_TEXT,FXColorSelector::ID_RGB_ALPHA_TEXT,FXColorSelector::onCmdRGBText),
  FXMAPFUNCS(SEL_CHANGED,FXColorSelector::ID_HSV_HUE_SLIDER,FXColorSelector::ID_HSV_ALPHA_SLIDER,FXColorSelector::onCmdHSVSlider),
  FXMAPFUNCS(SEL_COMMAND,FXColorSelector::ID_HSV_HUE_TEXT,FXColorSelector::ID_HSV_ALPHA_TEXT,FXColorSelector::onCmdHSVText),
  FXMAPFUNC(SEL_COMMAND,FXColorSelector::ID_COLOR_LIST,FXColorSelector::onCmdList),
  };


// Object implementation
FXIMPLEMENT(FXColorSelector,FXPacker,FXColorSelectorMap,ARRAYNUMBER(FXColorSelectorMap))


// Separator item
FXColorSelector::FXColorSelector(FXComposite *p,FXObject* tgt,FXSelector sel,FXuint opts,FXint x,FXint y,FXint w,FXint h):
  FXPacker(p,opts,x,y,w,h){
  target=tgt;
  message=sel;
  
  // Buttons
  FXHorizontalFrame *buttons=new FXHorizontalFrame(this,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X|PACK_UNIFORM_WIDTH);
  accept=new FXButton(buttons,"&Accept",NULL,NULL,0,FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT,0,0,0,0,20,20);
  cancel=new FXButton(buttons,"&Cancel",NULL,NULL,0,FRAME_RAISED|FRAME_THICK|LAYOUT_RIGHT,0,0,0,0,20,20);
  
  // Separator
  new FXHorizontalSeparator(this,SEPARATOR_RIDGE|LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X);
  
  // Color wells
  FXHorizontalFrame *colors=new FXHorizontalFrame(this,LAYOUT_SIDE_BOTTOM|LAYOUT_FILL_X, 0,0,0,0, 0,0,0,0, 0,0);
  new FXColorWell(colors,FXRGBA(255,255,255,255),NULL,0,LAYOUT_CENTER_Y|LAYOUT_CENTER_X, 0,0,0,0 ,0,0,0,0);
  new FXColorWell(colors,FXRGBA(  0,  0,  0,255),NULL,0,LAYOUT_CENTER_Y|LAYOUT_CENTER_X, 0,0,0,0 ,0,0,0,0);
  new FXColorWell(colors,FXRGBA(255,  0,  0,255),NULL,0,LAYOUT_CENTER_Y|LAYOUT_CENTER_X, 0,0,0,0 ,0,0,0,0);
  new FXColorWell(colors,FXRGBA(  0,255,  0,255),NULL,0,LAYOUT_CENTER_Y|LAYOUT_CENTER_X, 0,0,0,0 ,0,0,0,0);
  
  new FXColorWell(colors,FXRGBA(  0,  0,255,255),NULL,0,LAYOUT_CENTER_Y|LAYOUT_CENTER_X, 0,0,0,0 ,0,0,0,0);
  new FXColorWell(colors,FXRGBA(  0,  0,255,255),NULL,0,LAYOUT_CENTER_Y|LAYOUT_CENTER_X, 0,0,0,0 ,0,0,0,0);
  new FXColorWell(colors,FXRGBA(255,255,  0,255),NULL,0,LAYOUT_CENTER_Y|LAYOUT_CENTER_X, 0,0,0,0 ,0,0,0,0);
  new FXColorWell(colors,FXRGBA(  0,255,255,255),NULL,0,LAYOUT_CENTER_Y|LAYOUT_CENTER_X, 0,0,0,0 ,0,0,0,0);
  
  new FXColorWell(colors,FXRGBA(255,  0,255,255),NULL,0,LAYOUT_CENTER_Y|LAYOUT_CENTER_X, 0,0,0,0 ,0,0,0,0);
  new FXColorWell(colors,FXRGBA(128,  0,  0,255),NULL,0,LAYOUT_CENTER_Y|LAYOUT_CENTER_X, 0,0,0,0 ,0,0,0,0);
  new FXColorWell(colors,FXRGBA(  0,128,  0,255),NULL,0,LAYOUT_CENTER_Y|LAYOUT_CENTER_X, 0,0,0,0 ,0,0,0,0);
  new FXColorWell(colors,FXRGBA(  0,  0,128,255),NULL,0,LAYOUT_CENTER_Y|LAYOUT_CENTER_X, 0,0,0,0 ,0,0,0,0);
  
  new FXColorWell(colors,FXRGBA(128,128,  0,255),NULL,0,LAYOUT_CENTER_Y|LAYOUT_CENTER_X, 0,0,0,0 ,0,0,0,0);
  new FXColorWell(colors,FXRGBA(128,  0,128,255),NULL,0,LAYOUT_CENTER_Y|LAYOUT_CENTER_X, 0,0,0,0 ,0,0,0,0);
  new FXColorWell(colors,FXRGBA(  0,128,128,255),NULL,0,LAYOUT_CENTER_Y|LAYOUT_CENTER_X, 0,0,0,0 ,0,0,0,0);
  new FXColorWell(colors,FXRGBA(  0,128,128,255),NULL,0,LAYOUT_CENTER_Y|LAYOUT_CENTER_X, 0,0,0,0 ,0,0,0,0);
  
  new FXColorWell(colors,FXRGBA(255,  0,255,255),NULL,0,LAYOUT_CENTER_Y|LAYOUT_CENTER_X, 0,0,0,0 ,0,0,0,0);
  new FXColorWell(colors,FXRGBA(128,  0,  0,255),NULL,0,LAYOUT_CENTER_Y|LAYOUT_CENTER_X, 0,0,0,0 ,0,0,0,0);
  new FXColorWell(colors,FXRGBA(  0,128,  0,255),NULL,0,LAYOUT_CENTER_Y|LAYOUT_CENTER_X, 0,0,0,0 ,0,0,0,0);
  new FXColorWell(colors,FXRGBA(  0,  0,128,255),NULL,0,LAYOUT_CENTER_Y|LAYOUT_CENTER_X, 0,0,0,0 ,0,0,0,0);
  
  new FXColorWell(colors,FXRGBA(128,128,  0,255),NULL,0,LAYOUT_CENTER_Y|LAYOUT_CENTER_X, 0,0,0,0 ,0,0,0,0);
  new FXColorWell(colors,FXRGBA(128,  0,128,255),NULL,0,LAYOUT_CENTER_Y|LAYOUT_CENTER_X, 0,0,0,0 ,0,0,0,0);
  new FXColorWell(colors,FXRGBA(  0,128,128,255),NULL,0,LAYOUT_CENTER_Y|LAYOUT_CENTER_X, 0,0,0,0 ,0,0,0,0);
  new FXColorWell(colors,FXRGBA(  0,128,128,255),NULL,0,LAYOUT_CENTER_Y|LAYOUT_CENTER_X, 0,0,0,0 ,0,0,0,0);
    
  // Main part
  FXHorizontalFrame *main=new FXHorizontalFrame(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y);

  // Main color well
  well=new FXColorWell(main,FXRGBA(0,0,128,255),this,ID_WELL_CHANGED,LAYOUT_TOP|LAYOUT_LEFT|LAYOUT_FILL_Y|LAYOUT_FIX_WIDTH, 0,0,64,0 ,0,0,0,0);
  
  // Tab book with switchable panels
  FXTabBook* panels=new FXTabBook(main,NULL,0,LAYOUT_FILL_Y|LAYOUT_FILL_X);

  // RGB Mode
  new FXTabItem(panels,"RGB",NULL,0);

    // RGB Sliders
    FXMatrix *rgbblock=new FXMatrix(panels->getContents(),3,LAYOUT_FILL_Y|LAYOUT_FILL_X|LAYOUT_TOP|LAYOUT_LEFT|MATRIX_BY_COLUMNS,0,0,0,0,10,10,10,10, 5,8);

    // Red
    new FXLabel(rgbblock,"&Red:",NULL,LAYOUT_FILL_ROW|LAYOUT_CENTER_Y|LAYOUT_RIGHT);
    rgbatext[0]=new FXTextField(rgbblock,3,this,FXColorSelector::ID_RGB_RED_TEXT,LAYOUT_FILL_ROW|LAYOUT_CENTER_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, DEFAULT_PAD,DEFAULT_PAD,0,0);
    rgbaslider[0]=new FXSlider(rgbblock,this,FXColorSelector::ID_RGB_RED_SLIDER,LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT|SLIDER_HORIZONTAL|SLIDER_INSIDE_BAR,0,0,0,15);

    // Green slider
    new FXLabel(rgbblock,"&Green:",NULL,LAYOUT_FILL_ROW|LAYOUT_CENTER_Y|LAYOUT_RIGHT);
    rgbatext[1]=new FXTextField(rgbblock,3,this,FXColorSelector::ID_RGB_GREEN_TEXT,LAYOUT_FILL_ROW|LAYOUT_CENTER_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, DEFAULT_PAD,DEFAULT_PAD,0,0);
    rgbaslider[1]=new FXSlider(rgbblock,this,FXColorSelector::ID_RGB_GREEN_SLIDER,LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT|SLIDER_HORIZONTAL|SLIDER_INSIDE_BAR,0,0,0,15);

    // Blue slider  
    new FXLabel(rgbblock,"&Blue:",NULL,LAYOUT_FILL_ROW|LAYOUT_CENTER_Y|LAYOUT_RIGHT);
    rgbatext[2]=new FXTextField(rgbblock,3,this,FXColorSelector::ID_RGB_BLUE_TEXT,LAYOUT_FILL_ROW|LAYOUT_CENTER_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, DEFAULT_PAD,DEFAULT_PAD,0,0);
    rgbaslider[2]=new FXSlider(rgbblock,this,FXColorSelector::ID_RGB_BLUE_SLIDER,LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT|SLIDER_HORIZONTAL|SLIDER_INSIDE_BAR,0,0,0,15);

    // Alpha slider
    new FXLabel(rgbblock,"&Alpha:",NULL,LAYOUT_FILL_ROW|LAYOUT_CENTER_Y|LAYOUT_RIGHT);
    rgbatext[3]=new FXTextField(rgbblock,3,this,FXColorSelector::ID_RGB_ALPHA_TEXT,LAYOUT_FILL_ROW|LAYOUT_CENTER_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, DEFAULT_PAD,DEFAULT_PAD,0,0);
    rgbaslider[3]=new FXSlider(rgbblock,this,FXColorSelector::ID_RGB_ALPHA_SLIDER,LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT|SLIDER_HORIZONTAL|SLIDER_INSIDE_BAR,0,0,0,15);

    // Set ranges
    rgbaslider[0]->setRange(0,255);
    rgbaslider[1]->setRange(0,255);
    rgbaslider[2]->setRange(0,255);
    rgbaslider[3]->setRange(0,255);
    
  // HSV Mode
  new FXTabItem(panels,"HSV",NULL,0);

    // RGB Sliders
    FXMatrix *hsvblock=new FXMatrix(panels->getContents(),3,LAYOUT_FILL_Y|LAYOUT_FILL_X|LAYOUT_TOP|LAYOUT_LEFT|MATRIX_BY_COLUMNS,0,0,0,0,10,10,10,10, 5,8);

    // Hue Slider
    new FXLabel(hsvblock,"Hue:",NULL,LAYOUT_FILL_ROW|LAYOUT_CENTER_Y|LAYOUT_RIGHT);
    hsvatext[0]=new FXTextField(hsvblock,3,this,FXColorSelector::ID_HSV_HUE_TEXT,LAYOUT_FILL_ROW|LAYOUT_CENTER_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, DEFAULT_PAD,DEFAULT_PAD,0,0);
    hsvaslider[0]=new FXSlider(hsvblock,this,FXColorSelector::ID_HSV_HUE_SLIDER,LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT|SLIDER_HORIZONTAL|SLIDER_INSIDE_BAR,0,0,0,15);

    // Saturation slider
    new FXLabel(hsvblock,"Saturation:",NULL,LAYOUT_FILL_ROW|LAYOUT_CENTER_Y|LAYOUT_RIGHT);
    hsvatext[1]=new FXTextField(hsvblock,3,this,FXColorSelector::ID_HSV_SATURATION_TEXT,LAYOUT_FILL_ROW|LAYOUT_CENTER_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, DEFAULT_PAD,DEFAULT_PAD,0,0);
    hsvaslider[1]=new FXSlider(hsvblock,this,FXColorSelector::ID_HSV_SATURATION_SLIDER,LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT|SLIDER_HORIZONTAL|SLIDER_INSIDE_BAR,0,0,0,15);

    // Value slider  
    new FXLabel(hsvblock,"Value:",NULL,LAYOUT_FILL_ROW|LAYOUT_CENTER_Y|LAYOUT_RIGHT);
    hsvatext[2]=new FXTextField(hsvblock,3,this,FXColorSelector::ID_HSV_VALUE_TEXT,LAYOUT_FILL_ROW|LAYOUT_CENTER_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, DEFAULT_PAD,DEFAULT_PAD,0,0);
    hsvaslider[2]=new FXSlider(hsvblock,this,FXColorSelector::ID_HSV_VALUE_SLIDER,LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT|SLIDER_HORIZONTAL|SLIDER_INSIDE_BAR,0,0,0,15);

    // Alpha slider
    new FXLabel(hsvblock,"Alpha:",NULL,LAYOUT_FILL_ROW|LAYOUT_CENTER_Y|LAYOUT_RIGHT);
    hsvatext[3]=new FXTextField(hsvblock,3,this,FXColorSelector::ID_HSV_ALPHA_TEXT,LAYOUT_FILL_ROW|LAYOUT_CENTER_Y|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, DEFAULT_PAD,DEFAULT_PAD,0,0);
    hsvaslider[3]=new FXSlider(hsvblock,this,FXColorSelector::ID_HSV_ALPHA_SLIDER,LAYOUT_FILL_ROW|LAYOUT_FILL_COLUMN|LAYOUT_CENTER_Y|LAYOUT_FILL_X|LAYOUT_FIX_HEIGHT|SLIDER_HORIZONTAL|SLIDER_INSIDE_BAR,0,0,0,15);

    // Set ranges
    hsvaslider[0]->setRange(0,360);
    hsvaslider[1]->setRange(0,100);
    hsvaslider[2]->setRange(0,100);
    hsvaslider[3]->setRange(0,255);
    
  // Named Color Mode
  new FXTabItem(panels,"Name",NULL,0);
  
    // Name list
    FXHorizontalFrame *outer=new FXHorizontalFrame(panels->getContents(),LAYOUT_FILL_Y|LAYOUT_FILL_X);
    FXHorizontalFrame *frame=new FXHorizontalFrame(outer,LAYOUT_FILL_Y|LAYOUT_FILL_X|FRAME_SUNKEN|FRAME_THICK,0,0,0,0, 0,0,0,0);
    list=new FXList(frame,this,ID_COLOR_LIST,LAYOUT_FILL_Y|LAYOUT_FILL_X|LIST_BROWSESELECT);
    
    // Add color names
    for(FXint i=0; i<fxnumcolornames; i++){ 
      list->addItemLast(fxcolornames[i].name,(void*)FXRGB(fxcolornames[i].r,fxcolornames[i].g,fxcolornames[i].b));
      }
    
  // Init RGBA
  rgba[0]=0.0;
  rgba[1]=0.0;
  rgba[2]=0.0;
  rgba[3]=1.0;
  
  // Init HSVA
  hsva[0]=360.0;
  hsva[1]=0.0;
  hsva[2]=0.0;
  hsva[3]=1.0;
  }


// Init the panel
void FXColorSelector::create(){
  FXPacker::create();
  updateRGBText();
  updateHSVText();
  updateRGBSliders();
  updateHSVSliders();
  updateWell();
  }



// RGB

// Update well from rgb slider
long FXColorSelector::onCmdRGBSlider(FXObject* sender,FXSelector sel,void* ptr){
  FXint which=SELID(sel)-ID_RGB_RED_SLIDER;
  rgba[which]=0.0039215686*((FXint)ptr);
  fxrgb_to_hsv(hsva[0],hsva[1],hsva[2],rgba[0],rgba[1],rgba[2]); hsva[3]=rgba[3];
  updateRGBText();
  updateHSVText();
  updateHSVSliders();
  updateWell();
  return 1;
  }

// Update well from rgb text fields
long FXColorSelector::onCmdRGBText(FXObject* sender,FXSelector sel,void* ptr){
  FXint which=SELID(sel)-ID_RGB_RED_TEXT;
  rgba[which]=0.01*atof(((FXTextField*)sender)->getText());
  fxrgb_to_hsv(hsva[0],hsva[1],hsva[2],rgba[0],rgba[1],rgba[2]); hsva[3]=rgba[3];
  updateRGBSliders();
  updateHSVText();
  updateHSVSliders();
  updateWell();
  return 1;
  }


// HSV

// Update well from hsv sliders
long FXColorSelector::onCmdHSVSlider(FXObject* sender,FXSelector sel,void* ptr){
  const FXdouble factor[4]={1.0,0.01,0.01,0.0039215686};
  FXint which=SELID(sel)-ID_HSV_HUE_SLIDER;
  hsva[which]=factor[which]*((FXint)ptr);
  fxhsv_to_rgb(rgba[0],rgba[1],rgba[2],hsva[0],hsva[1],hsva[2]); rgba[3]=hsva[3];
  updateRGBText();
  updateHSVText();
  updateRGBSliders();
  updateWell();
  return 1;
  }

// Update well from hsv text fields
long FXColorSelector::onCmdHSVText(FXObject* sender,FXSelector sel,void* ptr){
  const FXfloat factor[4]={1.0,0.01,0.01,0.01};
  FXint which=SELID(sel)-ID_HSV_HUE_TEXT;
  hsva[which]=factor[which]*atof(((FXTextField*)sender)->getText());
  fxhsv_to_rgb(rgba[0],rgba[1],rgba[2],hsva[0],hsva[1],hsva[2]); rgba[3]=hsva[3];
  updateRGBText();
  updateHSVSliders();
  updateRGBSliders();
  updateWell();
  return 1;
  }


// Dropped color in well
long FXColorSelector::onCmdWell(FXObject*,FXSelector sel,void* ptr){
  rgba[0]=0.0039215686*FXREDVAL(well->getRGBA());
  rgba[1]=0.0039215686*FXGREENVAL(well->getRGBA());
  rgba[2]=0.0039215686*FXBLUEVAL(well->getRGBA());
  rgba[3]=0.0039215686*FXALPHAVAL(well->getRGBA());
  fxrgb_to_hsv(hsva[0],hsva[1],hsva[2],rgba[0],rgba[1],rgba[2]); hsva[3]=rgba[3];
  updateRGBText();
  updateHSVText();
  updateRGBSliders();
  updateHSVSliders();
  return 1;
  }


// Clicked on color in list
long FXColorSelector::onCmdList(FXObject*,FXSelector,void* ptr){
  if(ptr){
    FXColor clr=(FXColor)list->getItemData((FXListItem*)ptr);
    rgba[0]=0.0039215686*FXREDVAL(clr);
    rgba[1]=0.0039215686*FXGREENVAL(clr);
    rgba[2]=0.0039215686*FXBLUEVAL(clr);
    rgba[3]=1.0;
    fxrgb_to_hsv(hsva[0],hsva[1],hsva[2],rgba[0],rgba[1],rgba[2]); hsva[3]=rgba[3];
    updateRGBText();
    updateHSVText();
    updateRGBSliders();
    updateHSVSliders();
    updateWell();
    }
  return 1;
  }


void FXColorSelector::updateRGBText(){
  FXchar buf[30];
  sprintf(buf,"%.1f",100.0*rgba[0]); rgbatext[0]->setText(buf);
  sprintf(buf,"%.1f",100.0*rgba[1]); rgbatext[1]->setText(buf);
  sprintf(buf,"%.1f",100.0*rgba[2]); rgbatext[2]->setText(buf);
  sprintf(buf,"%.1f",100.0*rgba[3]); rgbatext[3]->setText(buf);
  }


void FXColorSelector::updateHSVText(){
  FXchar buf[30];
  sprintf(buf,"%.1f",hsva[0]); hsvatext[0]->setText(buf);
  sprintf(buf,"%.1f",100.0*hsva[1]); hsvatext[1]->setText(buf);
  sprintf(buf,"%.1f",100.0*hsva[2]); hsvatext[2]->setText(buf);
  sprintf(buf,"%.1f",100.0*hsva[3]); hsvatext[3]->setText(buf);
  }


void FXColorSelector::updateRGBSliders(){
  rgbaslider[0]->setPosition((int)(255.0*rgba[0]));
  rgbaslider[1]->setPosition((int)(255.0*rgba[1]));
  rgbaslider[2]->setPosition((int)(255.0*rgba[2]));
  rgbaslider[3]->setPosition((int)(255.0*rgba[3]));
  }


void FXColorSelector::updateHSVSliders(){
  hsvaslider[0]->setPosition((int)hsva[0]);
  hsvaslider[1]->setPosition((int)(100.0*hsva[1]));
  hsvaslider[2]->setPosition((int)(100.0*hsva[2]));
  hsvaslider[3]->setPosition((int)(255.0*hsva[3]));
  }


// Update well
void FXColorSelector::updateWell(){
  well->setRGBA(FXRGBA((int)(rgba[0]*255.0),(int)(rgba[1]*255.0),(int)(rgba[2]*255.0),(int)(rgba[3]*255.0)));
  }


// Change RGBA color 
void FXColorSelector::setRGBA(FXColor clr){
  well->setRGBA(clr);
  }


// Retrieve RGBA color
FXColor FXColorSelector::getRGBA() const {
  return well->getRGBA();
  }


// Cleanup; icons must be explicitly deleted
FXColorSelector::~FXColorSelector(){
  well=(FXColorWell*)-1;
  accept=(FXButton*)-1;
  cancel=(FXButton*)-1;
  }


