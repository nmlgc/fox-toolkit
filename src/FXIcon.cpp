/********************************************************************************
*                                                                               *
*                               I c o n - O b j e c t                           *
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
* $Id: FXIcon.cpp,v 1.47 2002/01/18 22:43:00 jeroen Exp $                       *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXRegistry.h"
#include "FXApp.h"
#include "FXVisual.h"
#include "FXIcon.h"


/*
  Notes:
  - Debug the render function between different hosts.
  - Option to guess alpha color from corners.
  - The shape pixmap [X11] actually contains TWO shapes:- the
    shape mask (transparency) as well as the etch mask (gray out&transparency).
    We do this because most icons are 16x16 or so, and we can fit two of them
    side-by-side in the space of one:- because the bitmaps are typically
    padded to a width of 32 anyway :-)  Either way, the overhead for the
    header will be smaller.
*/

#define DARKCOLOR(r,g,b) (((r)+(g)+(b))<382)


#define DISPLAY(app) ((Display*)((app)->display))

/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXIcon,FXImage,NULL,0)


// Initialize nicely
FXIcon::FXIcon(FXApp* a,const void *pix,FXColor clr,FXuint opts,FXint w,FXint h):
  FXImage(a,pix,opts,w,h){
  FXTRACE((100,"FXIcon::FXIcon %p\n",this));
  shape=0;
  etch=0;
  transp=clr;
  }


// Guess alpha color based on corners; the initial guess is standard GUI color
FXColor FXIcon::guesstransp(){
  register FXint tr,bl,br,best,t;
  register FXColor guess=FXRGB(192,192,192);
  FXColor color[4];
  if(data && 0<width && 0<height){
    best=-1;
    if(options&IMAGE_ALPHA){
      tr=4*(width-1); bl=4*width*(height-1); br=bl+tr;
      }
    else{
      tr=3*(width-1); bl=3*width*(height-1); br=bl+tr;
      }
    color[0]=FXRGB(data[0],data[1],data[2]);
    color[1]=FXRGB(data[tr],data[tr+1],data[tr+2]);
    color[2]=FXRGB(data[bl],data[bl+1],data[bl+2]);
    color[3]=FXRGB(data[br],data[br+1],data[br+2]);
    if((t=((color[0]==color[1])+(color[0]==color[2])+(color[0]==color[3])))>best){ guess=color[0]; best=t; }
    if((t=((color[1]==color[2])+(color[1]==color[3])+(color[1]==color[0])))>best){ guess=color[1]; best=t; }
    if((t=((color[2]==color[3])+(color[2]==color[0])+(color[2]==color[1])))>best){ guess=color[2]; best=t; }
    if((t=((color[3]==color[0])+(color[3]==color[1])+(color[3]==color[2])))>best){ guess=color[3]; }
    }
  return guess;
  }


// Create icon
void FXIcon::create(){
  if(!xid){
    if(getApp()->initialized){
      FXTRACE((100,"%s::create %p\n",getClassName(),this));
#ifndef WIN32

      // Initialize visual
      visual->create();

      // Get depth (should use visual!!)
      int dd=visual->getDepth();

      // Make image pixmap
      xid=XCreatePixmap(DISPLAY(getApp()),XDefaultRootWindow(DISPLAY(getApp())),width,height,dd);
      if(!xid){ fxerror("%s::create: unable to create icon.\n",getClassName()); }

      // Make shape pixmap
      shape=XCreatePixmap(DISPLAY(getApp()),XDefaultRootWindow(DISPLAY(getApp())),width,height,1);
      if(!shape){ fxerror("%s::create: unable to create icon.\n",getClassName()); }

      // Make etch pixmap
      etch=XCreatePixmap(DISPLAY(getApp()),XDefaultRootWindow(DISPLAY(getApp())),width,height,1);
      if(!etch){ fxerror("%s::create: unable to create icon.\n",getClassName()); }

#else

      // Initialize visual
      visual->create();

      // Create a memory DC compatible with current display
      HDC hdc=::GetDC(GetDesktopWindow());
      xid=CreateCompatibleBitmap(hdc,width,height);
      ::ReleaseDC(GetDesktopWindow(),hdc);
      if(!xid){ fxerror("%s::create: unable to create image.\n",getClassName()); }

      // Make shape bitmap
      shape=CreateBitmap(width,height,1,1,NULL);
      if(!shape){ fxerror("%s::create: unable to create icon.\n",getClassName()); }

      // Make etch bitmap
      etch=CreateBitmap(width,height,1,1,NULL);
      if(!etch){ fxerror("%s::create: unable to create icon.\n",getClassName()); }

#endif

      // Render pixels
      render();

      // Zap data
      if(!(options&IMAGE_KEEP) && (options&IMAGE_OWNED)){
        options&=~IMAGE_OWNED;
        FXFREE(&data);
        }
      }
    }
  }


// Detach icon
void FXIcon::detach(){
  visual->detach();
  if(xid){
    FXTRACE((100,"%s::detach %p\n",getClassName(),this));
    shape=0;
    etch=0;
    xid=0;
    }
  }


// Destroy icon
void FXIcon::destroy(){
  if(xid){
    if(getApp()->initialized){
      FXTRACE((100,"%s::destroy %p\n",getClassName(),this));
#ifndef WIN32

      // Delete shape, etch, and image pixmaps
      XFreePixmap(DISPLAY(getApp()),shape);
      XFreePixmap(DISPLAY(getApp()),etch);
      XFreePixmap(DISPLAY(getApp()),xid);
#else

      // Delete shape, etch, and image bitmaps
      DeleteObject(shape);
      DeleteObject(etch);
      DeleteObject(xid);
#endif
      }
    shape=0;
    etch=0;
    xid=0;
    }
  }


#ifndef WIN32


// Render icon X Windows
void FXIcon::render(){
  if(xid){
    register Visual *vis;
    register XImage *xim=NULL;
    register FXbool shmi=FALSE;
    register FXuchar *img;
    register FXint x,y;
    register FXuchar tr,tg,tb;
    XGCValues values;
    GC gc;
#ifdef HAVE_XSHM
    XShmSegmentInfo shminfo;
#endif

    FXTRACE((100,"%s::render shape %p\n",getClassName(),this));

    // Check for legal size
    if(width<1 || height<1){ fxerror("%s::render: illegal icon size %dx%d.\n",getClassName(),width,height); }

    // Render the image pixels
    FXImage::render();

    // Just leave if black if no data
    if(data){

      // Get Visual
      vis=(Visual*)visual->visual;

      // Turn it on iff both supported and desired
#ifdef HAVE_XSHM
      if(options&IMAGE_SHMI) shmi=getApp()->shmi;
#endif

      // First try XShm
#ifdef HAVE_XSHM
      if(shmi){
        xim=XShmCreateImage(DISPLAY(getApp()),vis,1,ZPixmap,NULL,&shminfo,width,height);
        if(!xim){ shmi=0; }
        if(shmi){
          shminfo.shmid=shmget(IPC_PRIVATE,xim->bytes_per_line*xim->height,IPC_CREAT|0777);
          if(shminfo.shmid==-1){ xim->data=NULL; XDestroyImage(xim); xim=NULL; shmi=0; }
          if(shmi){
            shminfo.shmaddr=xim->data=(char*)shmat(shminfo.shmid,0,0);
            shminfo.readOnly=FALSE;
            XShmAttach(DISPLAY(getApp()),&shminfo);
            FXTRACE((150,"Bitmap XSHM attached at memory=%p (%d bytes)\n",xim->data,xim->bytes_per_line*xim->height));
            }
          }
        }
#endif

      // Try the old fashioned way
      if(!shmi){

        // Try create image
        xim=XCreateImage(DISPLAY(getApp()),vis,1,ZPixmap,0,NULL,width,height,32,0);
        if(!xim){ fxerror("%s::render: unable to render icon.\n",getClassName()); }

        // Try create temp pixel store
        if(!FXMALLOC(&xim->data,char,xim->bytes_per_line*height)){ fxerror("%s::render: unable to allocate memory.\n",getClassName()); }
        }

      // Make GC
      values.foreground=0xffffffff;
      values.background=0xffffffff;
      gc=XCreateGC(DISPLAY(getApp()),shape,GCForeground|GCBackground,&values);

      // Should have succeeded
      FXASSERT(xim);

      FXTRACE((150,"bm width = %d\n",xim->width));
      FXTRACE((150,"bm height = %d\n",xim->height));
      FXTRACE((150,"bm format = %s\n",xim->format==XYBitmap?"XYBitmap":xim->format==XYPixmap?"XYPixmap":"ZPixmap"));
      FXTRACE((150,"bm byte_order = %s\n",(xim->byte_order==MSBFirst)?"MSBFirst":"LSBFirst"));
      FXTRACE((150,"bm bitmap_unit = %d\n",xim->bitmap_unit));
      FXTRACE((150,"bm bitmap_bit_order = %s\n",(xim->bitmap_bit_order==MSBFirst)?"MSBFirst":"LSBFirst"));
      FXTRACE((150,"bm bitmap_pad = %d\n",xim->bitmap_pad));
      FXTRACE((150,"bm bitmap_unit = %d\n",xim->bitmap_unit));
      FXTRACE((150,"bm depth = %d\n",xim->depth));
      FXTRACE((150,"bm bytes_per_line = %d\n",xim->bytes_per_line));
      FXTRACE((150,"bm bits_per_pixel = %d\n",xim->bits_per_pixel));

      // Fill shape mask
      img=data;
      if(options&IMAGE_OPAQUE){           // Opaque image
        FXTRACE((150,"Shape rectangle\n"));
        memset(xim->data,0xff,xim->bytes_per_line*height);
        }
      else if(options&IMAGE_ALPHA){       // Transparency channel
        FXTRACE((150,"Shape from alpha-channel\n"));
        for(y=0; y<height; y++){
          for(x=0; x<width; x++){
            XPutPixel(xim,x,y,(img[3]!=0));
            img+=4;
            }
          }
        }
      else{                               // Transparent color
        tr=FXREDVAL(transp);
        tg=FXGREENVAL(transp);
        tb=FXBLUEVAL(transp);
        FXTRACE((150,"Shape from alpha-color\n"));
        for(y=0; y<height; y++){
          for(x=0; x<width; x++){
            XPutPixel(xim,x,y,!(img[0]==tr && img[1]==tg && img[2]==tb));
            img+=3;
            }
          }
        }

      // Transfer image
#ifdef HAVE_XSHM
      if(shmi){
        XShmPutImage(DISPLAY(getApp()),shape,gc,xim,0,0,0,0,width,height,False);
        XSync(DISPLAY(getApp()),False);
        }
#endif
      if(!shmi){
        XPutImage(DISPLAY(getApp()),shape,gc,xim,0,0,0,0,width,height);
        }

      // Fill etch image
      img=data;
      if(options&IMAGE_OPAQUE){           // Opaque image
        for(y=0; y<height; y++){
          for(x=0; x<width; x++){
            XPutPixel(xim,x,y,DARKCOLOR(img[0],img[1],img[2]));
            img+=channels;
            }
          }
        }
      else if(options&IMAGE_ALPHA){       // Transparency channel
        for(y=0; y<height; y++){
          for(x=0; x<width; x++){
            XPutPixel(xim,x,y,(img[3]!=0) && DARKCOLOR(img[0],img[1],img[2]));
            img+=4;
            }
          }
        }
      else{                               // Transparent color
        tr=FXREDVAL(transp);
        tg=FXGREENVAL(transp);
        tb=FXBLUEVAL(transp);
        for(y=0; y<height; y++){
          for(x=0; x<width; x++){
            XPutPixel(xim,x,y,!(img[0]==tr && img[1]==tg && img[2]==tb) && DARKCOLOR(img[0],img[1],img[2]));
            img+=3;
            }
          }
        }

      // Transfer image
#ifdef HAVE_XSHM
      if(shmi){
        XShmPutImage(DISPLAY(getApp()),etch,gc,xim,0,0,0,0,width,height,False);
        XSync(DISPLAY(getApp()),False);
        }
#endif
      if(!shmi){
        XPutImage(DISPLAY(getApp()),etch,gc,xim,0,0,0,0,width,height);
        }

      // Clean up
#ifdef HAVE_XSHM
      if(shmi){
        FXTRACE((150,"Bitmap XSHM detached at memory=%p (%d bytes)\n",xim->data,xim->bytes_per_line*xim->height));
        XShmDetach(DISPLAY(getApp()),&shminfo);
        xim->data=NULL;
        XDestroyImage(xim);
        shmdt(shminfo.shmaddr);
        shmctl(shminfo.shmid,IPC_RMID,0);
        }
#endif
      if(!shmi){
        FXFREE(&xim->data);
        XDestroyImage(xim);
        }
      XFreeGC(DISPLAY(getApp()),gc);
      }
    }
  }


#else


struct BITMAPINFO2 {
  BITMAPINFOHEADER bmiHeader;
  RGBQUAD          bmiColors[2];
  };


// Render Icon MS-Windows
void FXIcon::render(){
  if(xid){
    register FXuchar *maskdata,*etchdata,*msk,*ets,*img;
    register FXint x,y;
    register FXuchar tr,tg,tb;
    register FXuint bytes_per_line;
    BITMAPINFO2 bmi;
    HDC hdcmsk;

    FXTRACE((100,"%s::render %p\n",getClassName(),this));

    // Check for legal size
    if(width<1 || height<1){ fxerror("%s::render: illegal icon size %dx%d.\n",getClassName(),width,height); }

    // Render the image (color) pixels as usual
    FXImage::render();

    // Just leave if black if no data
    if(data){

      // Set up the bitmap info
      bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
      bmi.bmiHeader.biWidth=width;
      bmi.bmiHeader.biHeight=height;
      bmi.bmiHeader.biPlanes=1;
      bmi.bmiHeader.biBitCount=1;
      bmi.bmiHeader.biCompression=BI_RGB;
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

      // Allocate temp bit buffer
      bytes_per_line=((width+31)&~31)>>3;
      FXCALLOC(&maskdata,FXuchar,height*bytes_per_line);
      FXCALLOC(&etchdata,FXuchar,height*bytes_per_line);

      img=data;
      msk=maskdata+(height-1)*bytes_per_line;
      ets=etchdata+(height-1)*bytes_per_line;
      if(options&IMAGE_OPAQUE){
        for(y=0; y<height; y++){
          for(x=0; x<width; x++){
            if(!DARKCOLOR(img[0],img[1],img[2])){ ets[x>>3]|=0x80>>(x&7); }
            img+=channels;
            }
          ets-=bytes_per_line;
          }
        }
      else if(options&IMAGE_ALPHA){
        for(y=0; y<height; y++){
          for(x=0; x<width; x++){
            if(!DARKCOLOR(img[0],img[1],img[2])){ ets[x>>3]|=0x80>>(x&7); }
            if(img[3]==0){ msk[x>>3]|=0x80>>(x&7); ets[x>>3]|=0x80>>(x&7); }
            img+=4;
            }
          msk-=bytes_per_line;
          ets-=bytes_per_line;
          }
        }
      else{
        tr=FXREDVAL(transp);
        tg=FXGREENVAL(transp);
        tb=FXBLUEVAL(transp);
        for(y=0; y<height; y++){
          for(x=0; x<width; x++){
            if(!DARKCOLOR(img[0],img[1],img[2])){ ets[x>>3]|=0x80>>(x&7); }
            if((img[0]==tr) && (img[1]==tg) && (img[2]==tb)){ msk[x>>3]|=0x80>>(x&7); ets[x>>3]|=0x80>>(x&7); }
            img+=3;
            }
          msk-=bytes_per_line;
          ets-=bytes_per_line;
          }
        }

      // The MSDN documentation for SetDIBits() states that "the device context
      // identified by the (first) parameter is used only if the DIB_PAL_COLORS
      // constant is set for the (last) parameter". This may be true, but under
      // Win95 you must pass in a non-NULL hdc for the first parameter; otherwise
      // this call to SetDIBits() will fail (in contrast, it works fine under
      // Windows NT if you pass in a NULL hdc).
      hdcmsk=::CreateCompatibleDC(NULL);

      // Set mask data
      if(!SetDIBits(hdcmsk,(HBITMAP)shape,0,height,maskdata,(BITMAPINFO*)&bmi,DIB_RGB_COLORS)){
        fxerror("%s::render: unable to render pixels\n",getClassName());
        }

      // Set etch data
      if(!SetDIBits(hdcmsk,(HBITMAP)etch,0,height,etchdata,(BITMAPINFO*)&bmi,DIB_RGB_COLORS)){
        fxerror("%s::render: unable to render pixels\n",getClassName());
        }
      FXFREE(&maskdata);
      FXFREE(&etchdata);
      GdiFlush();

      // We AND the image with the mask, then we can do faster and more
      // flicker-free icon painting later using the `black source' method
      SelectObject(hdcmsk,(HBITMAP)shape);
      HDC hdcmem=::CreateCompatibleDC(NULL);
      SelectObject(hdcmem,(HBITMAP)xid);
      SetBkColor(hdcmem,RGB(0,0,0));                // 1 -> black
      SetTextColor(hdcmem,RGB(255,255,255));        // 0 -> white
      BitBlt(hdcmem,0,0,width,height,hdcmsk,0,0,SRCAND);
      ::DeleteDC(hdcmem);
      ::DeleteDC(hdcmsk);
      }
    }
  }


#endif


// Resize pixmap to the specified width and height; the contents become undefined
void FXIcon::resize(FXint w,FXint h){
  if(w<1) w=1;
  if(h<1) h=1;
  if(width!=w || height!=h){
    FXTRACE((100,"%s::resize(%d,%d) %p\n",getClassName(),w,h,this));

    // Resize device dependent pixmap
    if(xid){
#ifndef WIN32

      // Get depth (should use visual!!)
      int dd=visual->getDepth();

      // Free old pixmaps
      XFreePixmap(DISPLAY(getApp()),xid);
      XFreePixmap(DISPLAY(getApp()),etch);
      XFreePixmap(DISPLAY(getApp()),shape);

      // Make new pixmap
      xid=XCreatePixmap(DISPLAY(getApp()),XDefaultRootWindow(DISPLAY(getApp())),w,h,dd);
      if(!xid){ fxerror("%s::resize: unable to resize image.\n",getClassName()); }

      // Make shape pixmap
      shape=XCreatePixmap(DISPLAY(getApp()),XDefaultRootWindow(DISPLAY(getApp())),w,h,1);
      if(!shape){ fxerror("%s::create: unable to create icon.\n",getClassName()); }

      // Make etch pixmap
      etch=XCreatePixmap(DISPLAY(getApp()),XDefaultRootWindow(DISPLAY(getApp())),w,h,1);
      if(!etch){ fxerror("%s::create: unable to create icon.\n",getClassName()); }

#else

      // Delete old bitmaps
      DeleteObject(xid);
      DeleteObject(shape);
      DeleteObject(etch);

      // Create a bitmap compatible with current display
      HDC hdc=::GetDC(GetDesktopWindow());
      xid=CreateCompatibleBitmap(hdc,w,h);
      ::ReleaseDC(GetDesktopWindow(),hdc);
      if(!xid){ fxerror("%s::resize: unable to resize image.\n",getClassName()); }

      // Make shape bitmap
      shape=CreateBitmap(w,h,1,1,NULL);
      if(!shape){ fxerror("%s::create: unable to create icon.\n",getClassName()); }

      // Make etch bitmap
      etch=CreateBitmap(w,h,1,1,NULL);
      if(!etch){ fxerror("%s::create: unable to create icon.\n",getClassName()); }
#endif
      }

    // Resize data array iff total size changed
    if(data && (w*h)!=(width*height)){
      if(options&IMAGE_OWNED){
        FXRESIZE(&data,FXuchar,w*h*channels);
        }
      else{
        FXCALLOC(&data,FXuchar,w*h*channels);
        options|=IMAGE_OWNED;
        }
      }

    // Remember new size
    width=w;
    height=h;
    }
  }



// Clean up
FXIcon::~FXIcon(){
  FXTRACE((100,"FXIcon::~FXIcon %p\n",this));
  destroy();
  }
