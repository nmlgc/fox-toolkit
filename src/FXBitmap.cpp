/********************************************************************************
*                                                                               *
*                             B i t m a p    O b j e c t                        *
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
* $Id: FXBitmap.cpp,v 1.35 2002/01/18 22:42:58 jeroen Exp $                     *
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
#include "FXVisual.h"
#include "FXBitmap.h"
#include "FXVisual.h"
#include "FXDCWindow.h"

/*
  Note:
  - Try eliminate temp copy:- slap pixels into XImage directly, if possible...
  - Perhaps enforce system-native padding necessary for the above.
  - Our bitmap data is 01234567, i.e. LS-BIT first; byte order ditto.
  - Issue: should FXBitmap return the DC for drawing onto the X-Server resident
    pixmap, or the client-side image bits?

    My idea is it should be the latter:

      - Allows even richer set of drawing primitives, as everything is
        drawn in software.
      - Very useful to generate off-screen renderings, e.g. during printing.
      - Allows for building and running true-color drawing programs on
        low-end graphics hardware.
      - The only drawback I can see is it will be a fairly large implementation
        effort...

  - Need to implement restore, resize, and such.

*/

#define DISPLAY(app) ((Display*)((app)->display))


/*******************************************************************************/


// Object implementation
FXIMPLEMENT(FXBitmap,FXDrawable,NULL,0)


// For deserialization
FXBitmap::FXBitmap(){
  data=NULL;
  options=0;
  }


// Initialize
FXBitmap::FXBitmap(FXApp* a,const void *pix,FXuint opts,FXint w,FXint h):FXDrawable(a,w,h){
  FXTRACE((100,"FXBitmap::FXBitmap %p\n",this));
  visual=getApp()->getMonoVisual();
  if(pix) opts&=~BITMAP_OWNED;
  data=(FXuchar*)pix;
  options=opts;
  }


// Create bitmap
void FXBitmap::create(){
  if(!xid){
    if(getApp()->initialized){
      FXTRACE((100,"%s::create %p\n",getClassName(),this));
#ifndef WIN32

      // Initialize visual
      visual->create();

      // Make pixmap
      xid=XCreatePixmap(DISPLAY(getApp()),XDefaultRootWindow(DISPLAY(getApp())),width,height,1);
      if(!xid){ fxerror("%s::create: unable to create bitmap.\n",getClassName()); }
#else

      // Initialize visual
      visual->create();

      // Create uninitialized shape bitmap
      xid=CreateBitmap(width,height,1,1,NULL);
      if(!xid){ fxerror("%s::create: unable to create bitmap.\n",getClassName()); }
#endif
      // Render pixels
      render();

      // Zap data
      if(!(options&BITMAP_KEEP) && (options&BITMAP_OWNED)){
        options&=~BITMAP_OWNED;
        FXFREE(&data);
        }
      }
    }
  }


// Detach bitmap
void FXBitmap::detach(){
  visual->detach();
  if(xid){
    FXTRACE((100,"%s::detach %p\n",getClassName(),this));
    xid=0;
    }
  }


// Destroy bitmap
void FXBitmap::destroy(){
  if(xid){
    if(getApp()->initialized){
      FXTRACE((100,"%s::destroy %p\n",getClassName(),this));
#ifndef WIN32

      // Delete pixmap
      XFreePixmap(DISPLAY(getApp()),xid);
#else

      // Delete bitmap
      DeleteObject(xid);
#endif
      }
    xid=0;
    }
  }


// Render into pixmap
void FXBitmap::render(){
  if(xid){

    FXTRACE((100,"%s::render bitmap %p\n",getClassName(),this));

    // Do it for X-Windows
#ifndef WIN32

    // XImage xim;
    register XImage *xim=NULL;
    register Visual *vis;
    register int size;
    register FXuchar *pix;
    register int i;
    XGCValues values;
    GC gc;

    // Check for legal size
    if(width<1 || height<1){ fxerror("%s::render: illegal bitmap size.\n",getClassName()); }

    // Make GC
    values.foreground=0xffffffff;
    values.background=0;
    gc=XCreateGC(DISPLAY(getApp()),xid,GCForeground|GCBackground,&values);

    // Just leave if black if no data
    if(data){

      // Get Visual
      vis=(Visual*)visual->visual;

      xim=XCreateImage(DISPLAY(getApp()),vis,1,XYBitmap,0,NULL,width,height,8,(width+7)>>3);
      if(!xim){ fxerror("%s::render: unable to render image.\n",getClassName()); }

      // Try create temp pixel store
      if(!FXMALLOC(&xim->data,char,xim->bytes_per_line*height)){ fxerror("%s::render: unable to allocate memory.\n",getClassName()); }

      // Render bits into server-formatted bitmap
      size=xim->bytes_per_line*height;
      pix=(FXuchar*)xim->data;

      // Most significant bit first
      if(xim->bitmap_bit_order==MSBFirst){
        for(i=0; i<size; i++) pix[i]=FXBITREVERSE(data[i]);
        }

      // Least significant bit first
      else{
        memcpy(pix,data,size);
        }

      // Blast the image
      XPutImage(DISPLAY(getApp()),xid,gc,xim,0,0,0,0,width,height);
      FXFREE(&xim->data);
      XDestroyImage(xim);
      }

    // No data, fill with black
    else{
      XFillRectangle(DISPLAY(getApp()),xid,gc,0,0,width,height);
      }

    // We're done
    XFreeGC(DISPLAY(getApp()),gc);

#else

    register FXuchar *p,*q,bits;
    register FXint nb,i,j,bytes_per_line;
    FXuchar *widedata;

    // Check for legal size
    if(width<1 || height<1){ fxerror("%s::render: illegal bitmap size.\n",getClassName()); }

    HDC hdcmem=::CreateCompatibleDC(NULL);

    // Fill with data
    if(data){

      // Now make the device dependent bitmap
      struct BITMAPINFO2 {
        BITMAPINFOHEADER bmiHeader;
        RGBQUAD          bmiColors[2];
        };

      // Set up the bitmap info
      BITMAPINFO2 bmi;
      bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
      bmi.bmiHeader.biWidth=width;
      bmi.bmiHeader.biHeight=height;
      bmi.bmiHeader.biPlanes=1;
      bmi.bmiHeader.biBitCount=1;
      bmi.bmiHeader.biCompression=0;
      bmi.bmiHeader.biSizeImage=0;
      bmi.bmiHeader.biXPelsPerMeter=0;
      bmi.bmiHeader.biYPelsPerMeter=0;
      bmi.bmiHeader.biClrUsed=0;
      bmi.bmiHeader.biClrImportant=0;
      bmi.bmiColors[0].rgbBlue=0;
      bmi.bmiColors[0].rgbGreen=0;
      bmi.bmiColors[0].rgbRed=0;
      bmi.bmiColors[0].rgbReserved=0;
      bmi.bmiColors[1].rgbBlue=255;
      bmi.bmiColors[1].rgbGreen=255;
      bmi.bmiColors[1].rgbRed=255;
      bmi.bmiColors[1].rgbReserved=0;

      nb=(width+7)>>3;
      bytes_per_line=((width+31)&~31)>>3;
      FXCALLOC(&widedata,FXuchar,height*bytes_per_line);
      p=widedata+(height-1)*bytes_per_line;
      q=data;
      for(i=0; i<height; i++){
        for(j=0; j<nb; j++){
          bits=~q[j];
          p[j]=FXBITREVERSE(bits);
          }
        q+=nb;
        p-=bytes_per_line;
        }
      if(!SetDIBits(hdcmem,(HBITMAP)xid,0,height,widedata,(BITMAPINFO*)&bmi,DIB_RGB_COLORS)){
        fxerror("%s::render: unable to render pixels\n",getClassName());
        }
      FXFREE(&widedata);
      GdiFlush();
      }

    // No data, fill with black
    else{
      SelectObject(hdcmem,(HBITMAP)xid);
      BitBlt(hdcmem,0,0,width,height,hdcmem,0,0,BLACKNESS);
      }
    ::DeleteDC(hdcmem);
#endif
    }
  }


// Get pixel at x,y
FXbool FXBitmap::getPixel(FXint x,FXint y) const {
  register FXint nb=(width+7)>>3;
  return (data[y*nb+(x>>3)] & (1<<(x&7)))!=0;
  }


// Change pixel at x,y
void FXBitmap::setPixel(FXint x,FXint y,FXbool color){
  register FXint nb=(width+7)>>3;
  if(color) data[y*nb+(x>>3)]|=(1<<(x&7)); else data[y*nb+(x>>3)]&=~(1<<(x&7));
  }


// Resize bitmap to the specified width and height; the contents become undefined
void FXBitmap::resize(FXint w,FXint h){
  if(w<1) w=1;
  if(h<1) h=1;
  width=w;
  height=h;
  FXTRACE((100,"%s::resize %p\n",getClassName(),this));
  if(xid){

#ifndef WIN32

    // Free old pixmap
    XFreePixmap(DISPLAY(getApp()),xid);

    // Make new pixmap
    xid=XCreatePixmap(DISPLAY(getApp()),XDefaultRootWindow(DISPLAY(getApp())),width,height,1);
    if(!xid){ fxerror("%s::resize: unable to resize bitmap.\n",getClassName()); }

#else

    // Delete old bitmap
    DeleteObject(xid);

    // Create a bitmap compatible with current display
    xid=CreateBitmap(width,height,1,1,NULL);
    if(!xid){ fxerror("%s::resize: unable to resize bitmap.\n",getClassName()); }

#endif
    }
  }


#ifdef WIN32

// Get the image's device context
FXID FXBitmap::GetDC() const {
  HDC hdc=::CreateCompatibleDC(NULL);
  SelectObject(hdc,(HBITMAP)xid);
  return hdc;
  }


// Release it (no-op)
int FXBitmap::ReleaseDC(FXID hdc) const {
  return ::DeleteDC((HDC)hdc);
  }

#endif


// Save pixel data only
void FXBitmap::savePixels(FXStream& store) const {
  FXuint size=height*((width+7)>>3);
  store.save(data,size);
  }


// Load pixel data only
void FXBitmap::loadPixels(FXStream& store){
  FXuint size=height*((width+7)>>3);
  if(options&BITMAP_OWNED) FXFREE(&data);
  FXMALLOC(&data,FXuchar,size);
  store.load(data,size);
  options|=BITMAP_OWNED;
  }


// Save data
void FXBitmap::save(FXStream& store) const {
  FXuchar haspixels=(data!=NULL);
  FXDrawable::save(store);
  store << options;
  store << haspixels;
  if(haspixels) savePixels(store);
  }


// Load data
void FXBitmap::load(FXStream& store){
  FXuchar haspixels;
  FXDrawable::load(store);
  store >> options;
  store >> haspixels;
  if(haspixels) loadPixels(store);
  }


// Clean up
FXBitmap::~FXBitmap(){
  FXTRACE((100,"FXBitmap::~FXBitmap %p\n",this));
  destroy();
  if(options&BITMAP_OWNED){FXFREE(&data);}
  data=(FXuchar*)-1;
  }

