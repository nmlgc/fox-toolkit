/********************************************************************************
*                                                                               *
*                         C u r s o r - O b j e c t                             *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXCursor.cpp,v 1.22.4.1 2003/06/20 19:02:07 fox Exp $                     *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXSettings.h"
#include "FXRegistry.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXVisual.h"
#include "FXCursor.h"

/*
  To do:
  - Add subclass to read in .CUR file (similar to FXGIFIcon reading
    e.g. a GIF icon) from an FXStream.
  - Cursor size should be less than or equal to 32x32; this is necessary for Windows!
  - Need standard glyph for "invisible" cursor.
*/

#define DISPLAY(app) ((Display*)((app)->display))

/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXCursor,FXId,NULL,0)


// Deserialization
FXCursor::FXCursor(){
  source=NULL;
  mask=NULL;
  width=0;
  height=0;
  hotx=-1;
  hoty=-1;
  glyph=0;
  owned=FALSE;
  }


// Make stock cursor
FXCursor::FXCursor(FXApp* a,FXStockCursor curid):FXId(a){
  FXTRACE((100,"FXCursor::FXCursor %p\n",this));
  source=NULL;
  mask=NULL;
  width=0;
  height=0;
  hotx=-1;
  hoty=-1;
  glyph=(FXuchar)curid;   // Got to do this, we can't serialize enums (don't know how big they are!)
  owned=FALSE;
  }


// Make cursor from source and mask
FXCursor::FXCursor(FXApp* a,const void* src,const void* msk,FXint w,FXint h,FXint hx,FXint hy):FXId(a){
  FXTRACE((100,"FXCursor::FXCursor %p\n",this));
  source=(FXuchar*)src;
  mask=(FXuchar*)msk;
  width=w;
  height=h;
  hotx=hx;
  hoty=hy;
  glyph=0;
  owned=FALSE;
  }


// Create cursor
void FXCursor::create(){
  if(!xid){
    if(getApp()->initialized){
      FXTRACE((100,"%s::create %p\n",getClassName(),this));

#ifndef WIN32

      // Stock cursor glyphs
      const FXuint stock[]={XC_top_left_arrow,XC_arrow,XC_xterm,XC_watch,XC_crosshair,XC_sb_h_double_arrow,XC_sb_v_double_arrow,XC_fleur};
      Pixmap srcpix,mskpix;
      XColor color[2];

      // Building custom cursor
      if(glyph==0){

        // Should have both source and mask
        if(!source || !mask){ fxerror("%s::create: cursor needs both source and mask.\n",getClassName()); }

        color[0].pixel=BlackPixel(DISPLAY(getApp()),DefaultScreen(DISPLAY(getApp())));
        color[1].pixel=WhitePixel(DISPLAY(getApp()),DefaultScreen(DISPLAY(getApp())));
        color[0].flags=DoRed|DoGreen|DoBlue;
        color[1].flags=DoRed|DoGreen|DoBlue;
        XQueryColors(DISPLAY(getApp()),DefaultColormap(DISPLAY(getApp()),DefaultScreen(DISPLAY(getApp()))),color,2);

        // Create pixmaps for source and mask
        srcpix=XCreateBitmapFromData(DISPLAY(getApp()),XDefaultRootWindow(DISPLAY(getApp())),(char*)source,width,height);
        mskpix=XCreateBitmapFromData(DISPLAY(getApp()),XDefaultRootWindow(DISPLAY(getApp())),(char*)mask,width,height);

        // Create cursor
        xid=XCreatePixmapCursor(DISPLAY(getApp()),srcpix,mskpix,&color[0],&color[1],hotx,hoty);

        // No longer needed
        XFreePixmap(DISPLAY(getApp()),srcpix);
        XFreePixmap(DISPLAY(getApp()),mskpix);
        }

      // Building stock cursor
      else{
        FXASSERT(glyph<=ARRAYNUMBER(stock));
        xid=XCreateFontCursor(DISPLAY(getApp()),stock[glyph-1]);
        }

      // Were we successful?
      if(!xid){ fxerror("%s::create: unable to create cursor.\n",getClassName()); }

#else

      // Stock cursor glyphs
      const LPCTSTR stock[]={IDC_ARROW,IDC_ARROW,IDC_IBEAM,IDC_WAIT,IDC_CROSS,IDC_SIZENS,IDC_SIZEWE,IDC_SIZEALL};

      // Building custom cursor
      if(glyph==0){

        // Should have both source and mask
        if(!source || !mask){ fxerror("%s::create: cursor needs both source and mask.\n",getClassName()); }

        // Let's hope it's the correct size!
        if(width>32 || height>32){ fxerror("%s::create: cursor exceeds maximum size of 32x32 pixels\n",getClassName()); }

        FXASSERT(GetSystemMetrics(SM_CXCURSOR)==32);
        FXASSERT(GetSystemMetrics(SM_CYCURSOR)==32);

        // Windows cursor bitmaps are reversed left-to-right
        FXint i,j,srcbytes,srcoffset,dstoffset;
        FXuchar tmp;
        BYTE tmpxor[128],tmpand[128];
        srcbytes=(width+7)/8;
        srcoffset=dstoffset=0;
        memset(tmpand,0xff,sizeof(tmpand));
        memset(tmpxor,0,sizeof(tmpxor));
        for(i=0; i<height; i++){
          for(j=0; j<srcbytes; j++){
            tmp=~source[srcoffset+j] & mask[srcoffset+j];
            tmpxor[dstoffset+j]=FXBITREVERSE(tmp);
            tmp=~mask[srcoffset+j];
            tmpand[dstoffset+j]=FXBITREVERSE(tmp);
            }
          srcoffset+=srcbytes;
          dstoffset+=4;
          }

        // Always 32x32
        xid=CreateCursor((HINSTANCE)FXApp::hInstance,hotx,hoty,32,32,tmpand,tmpxor);
        }

      // Building stock cursor
      else{
        FXASSERT(glyph-1<ARRAYNUMBER(stock));
        xid=LoadCursor(NULL,stock[glyph-1]);
        }

      // Were we successful?
      if(!xid){ fxerror("%s::create: unable to create cursor.\n",getClassName()); }

#endif
      }
    }
  }


// Detach cursor
void FXCursor::detach(){
  if(xid){
    FXTRACE((100,"%s::detach %p\n",getClassName(),this));
    xid=0;
    }
  }


// Destroy cursor
void FXCursor::destroy(){
  if(xid){
    if(getApp()->initialized){
      FXTRACE((100,"%s::destroy %p\n",getClassName(),this));
#ifndef WIN32

      // Delete cursor
      XFreeCursor(DISPLAY(getApp()),xid);
#else

      // Delete cursor
      DestroyCursor((HCURSOR)xid);
#endif
      }
    xid=0;
    }
  }


// Save pixel data only
void FXCursor::savePixels(FXStream& store) const {
  store << width << height;
  store << hotx << hoty;
  store << glyph;
  if(glyph==0){
    FXuint size=height*((width+7)>>3);
    store.save(source,size);
    store.save(mask,size);
    }
  }


// Load pixel data only
void FXCursor::loadPixels(FXStream& store){
  store >> width >> height;
  store >> hotx >> hoty;
  store >> glyph;
  if(glyph==0){
    if(owned){FXFREE(&source);FXFREE(&mask);}
    FXuint size=height*((width+7)>>3);
    FXMALLOC(&source,FXuchar,size);
    FXMALLOC(&mask,FXuchar,size);
    store.load(source,size);
    store.load(mask,size);
    owned=TRUE;
    }
  }


// Save object to stream
void FXCursor::save(FXStream& store) const {
  FXId::save(store);
  savePixels(store);
  }


// Load object from stream
void FXCursor::load(FXStream& store){
  FXId::load(store);
  loadPixels(store);
  }


// Clean up
FXCursor::~FXCursor(){
  FXTRACE((100,"FXCursor::~FXCursor %p\n",this));
  destroy();
  if(owned){FXFREE(&source);FXFREE(&mask);}
  source=(FXuchar*)-1;
  mask=(FXuchar*)-1;
  }


