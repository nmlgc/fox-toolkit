/********************************************************************************
*                                                                               *
*                               I c o n - O b j e c t                           *
*                                                                               *
*********************************************************************************
* Copyright (C) 1997 by Jeroen van der Zijp.   All Rights Reserved.             *
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
* $Id: FXIcon.cpp,v 1.16 1998/10/28 15:09:53 jvz Exp $                        *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXObject.h"
#include "FXObjectList.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXDrawable.h"
#include "FXImage.h"
#include "FXIcon.h"


/*
  To do:
  - Debug the render function between different hosts.
  - Bug: icons with no transparency
  - Need option to guess alpha color from corners and/or sides
*/

/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXIcon,FXImage,NULL,0)


// Initialize nicely
FXIcon::FXIcon(FXApp* a,const void *pix,FXColor clr,FXuint opts,FXint w,FXint h):
  FXImage(a,pix,opts,w,h){
  shape=0;
  transp=clr;
  }
  

// Create icon
void FXIcon::create(){
  if(!xid){
    
    // App should exist
    if(!getApp()->display){ fxerror("%s::create: trying to create image before opening display.\n",getClassName()); }
  
    // Get depth
    depth=DefaultDepth(getDisplay(),DefaultScreen(getDisplay()));
    
    // Make image pixmap
    xid=XCreatePixmap(getDisplay(),XDefaultRootWindow(getDisplay()),width,height,depth);
    if(!xid){ fxerror("%s::create: unable to create icon.\n",getClassName()); }

    // Make shape pixmap
    shape=XCreatePixmap(getDisplay(),XDefaultRootWindow(getDisplay()),width,height,1);
    if(!shape){ fxerror("%s::create: unable to create icon.\n",getClassName()); }
  
    // Render pixels 
    render();
    
    // Zap data 
    if(!(options&IMAGE_KEEP) && (options&IMAGE_OWNED)){
      options&=~IMAGE_OWNED;
      FXFREE(&data);
      }
    }
  }


// Destroy drawable
void FXIcon::destroy(){
  FXImage::destroy(); 
  if(shape){
    XFreePixmap(getDisplay(),shape);
    shape=0;
    }
  }


// // Restore image from drawables
// int FXIcon::restore(){
//   if(FXImage::restore()){
//     if(shape){
//       if(mask) XDestroyImage(mask);
//       mask=XGetImage(getDisplay(),shape,0,0,width,height,AllPlanes,ZPixmap);
//       }
//     }
//   return mask!=0 && image!=0;
//   }


// Render pixels into bitmap
void FXIcon::render_bitmap(XImage *xim,FXuchar *img){
  register FXuchar *pix,tr,tg,tb;
  register FXuint jmp;
  register FXint x,y;
  pix=(FXuchar*)xim->data;
  jmp=xim->bytes_per_line;
  
  // Transparent image
  if(!(options&IMAGE_OPAQUE)){
    
    // Transparency through alpha channel
    if(options&IMAGE_ALPHA){
//fprintf(stderr,"have alpha channel\n");
      if(xim->bitmap_bit_order==MSBFirst){
        for(y=0; y<height; y++){
          for(x=0; x<width; x++){
            if(img[3]==0)
              XPutPixel(xim,x,y,0);
//             pix[x>>3]&=(0x80>>(x&7));
            else
              XPutPixel(xim,x,y,1);
//             pix[x>>3]|=(0x80>>(x&7));
            img+=4;
            }
          pix+=jmp;
          }
        }
      else{
        for(y=0; y<height; y++){
          for(x=0; x<width; x++){
            if(img[3]==0)
              XPutPixel(xim,x,y,0);
//             pix[x>>3]&=(0x01<<(x&7));
            else
              XPutPixel(xim,x,y,1);
//             pix[x>>3]|=(0x01<<(x&7));
            img+=4;
            }
          pix+=jmp;
          }
        }
      }

    // Transparency indicated by special pixel value
    else{
//fprintf(stderr,"have alpha color\n");
      tr=FXREDVAL(transp);
      tg=FXGREENVAL(transp);
      tb=FXBLUEVAL(transp);
      if(xim->bitmap_bit_order==MSBFirst){
        for(y=0; y<height; y++){
          for(x=0; x<width; x++){
            if(img[0]==tr && img[1]==tg && img[2]==tb)
              XPutPixel(xim,x,y,0);
//             pix[x>>3]&=(0x80>>(x&7));
            else
              XPutPixel(xim,x,y,1);
//             pix[x>>3]|=(0x80>>(x&7));
            img+=3;
            }
          pix+=jmp;
          }
        }
      else{
        for(y=0; y<height; y++){
          for(x=0; x<width; x++){
            if(img[0]==tr && img[1]==tg && img[2]==tb)
              XPutPixel(xim,x,y,0);
//             pix[x>>3]&=(0x01<<(x&7));
            else
              XPutPixel(xim,x,y,1);
//             pix[x>>3]|=(0x01<<(x&7));
            img+=3;
            }
          pix+=jmp;
          }
        }
      }
    }
  
  // No transparency at all:- just set it all to 1's
  else{
//fprintf(stderr,"opaque\n");
    for(y=0; y<height; y++){
      for(x=0; x<width; x++){
        XPutPixel(xim,x,y,1);
        }
      }
//     memset(pix,0xff,xim->bytes_per_line*height);
    }
  }


// Render icon
void FXIcon::render(){
#ifdef HAVE_XSHM
  XShmSegmentInfo shminfo;
#endif
  register Visual *visual;
  register XImage *xim=NULL;
  register FXbool shmi=FALSE;
  XGCValues values;
  GC gc;
  
  // Can not render before creation
  if(!xid || !shape){ fxerror("%s::render: trying to render icon before it has been created.\n",getClassName()); }

  // Check for legal size
  if(width<2 || height<2){ fxerror("%s::render: illegal icon size.\n",getClassName()); }

  // Just leave if black if no data
  if(data){
   
    // Render the image pixels
    FXImage::render();
    
    // Get Visual
    visual=DefaultVisual(getDisplay(),DefaultScreen(getDisplay()));
  
    // Make GC
    values.foreground=1;
    values.background=0;
    gc=XCreateGC(getDisplay(),shape,GCForeground|GCBackground,&values);
    
    // Turn it on iff both supported and desired
#ifdef HAVE_XSHM
    if(options&IMAGE_SHMI) shmi=getApp()->shmi;
#endif
    
    // First try XShm
#ifdef HAVE_XSHM
    if(shmi){
      xim=XShmCreateImage(getDisplay(),visual,1,ZPixmap,NULL,&shminfo,width,height);
      if(!xim){ shmi=0; }
      if(shmi){
        shminfo.shmid=shmget(IPC_PRIVATE,xim->bytes_per_line*xim->height,IPC_CREAT|0777);
        if(shminfo.shmid==-1){ XDestroyImage(xim); xim=NULL; shmi=0; }
        if(shmi){
          shminfo.shmaddr=xim->data=(char*)shmat(shminfo.shmid,0,0);
          shminfo.readOnly=FALSE;
          XShmAttach(getDisplay(),&shminfo);
//fprintf(stderr,"Bitmap XSHM attached at memory=0x%08x (%d bytes)\n",xim->data,xim->bytes_per_line*xim->height);
          }
        }
      }
#endif
    
    // Try the old fashioned way
    if(!shmi){
      // Try create image
      xim=XCreateImage(getDisplay(),visual,1,ZPixmap,0,NULL,width,height,32,0);
      if(!xim){ fxerror("%s::render: unable to render icon.\n",getClassName()); }
  
      // Try create temp pixel store
      xim->data=(char*)malloc(xim->bytes_per_line*height);
      if(!xim->data){ fxerror("%s::render: unable to allocate memory.\n",getClassName()); }
      }

    // Should have succeeded
    FXASSERT(xim);
    
// fprintf(stderr,"bm format = %d\n",xim->format);
// fprintf(stderr,"bm byte_order = %d\n",xim->byte_order);
// fprintf(stderr,"bm bitmap_unit = %d\n",xim->bitmap_unit);
// fprintf(stderr,"bm bitmap_bit_order = %d\n",xim->bitmap_bit_order);
// fprintf(stderr,"bm bitmap_pad = %d\n",xim->bitmap_pad);
// fprintf(stderr,"bm bitmap_unit = %d\n",xim->bitmap_unit);
// fprintf(stderr,"bm depth = %d\n",xim->depth);
// fprintf(stderr,"bm bytes_per_line = %d\n",xim->bytes_per_line);
// fprintf(stderr,"bm bits_per_pixel = %d\n",xim->bits_per_pixel);

    // Render the shape
    render_bitmap(xim,data);
    
    // Transfer image with shared memory
#ifdef HAVE_XSHM
    if(shmi){
      XShmPutImage(getDisplay(),shape,gc,xim,0,0,0,0,width,height,False);
      XSync(getDisplay(),False);
//fprintf(stderr,"Bitmap XSHM detached at memory=0x%08x (%d bytes)\n",xim->data,xim->bytes_per_line*xim->height);
      XShmDetach(getDisplay(),&shminfo);
      XDestroyImage(xim);
      shmdt(shminfo.shmaddr);
      shmctl(shminfo.shmid,IPC_RMID,0);
      }
#endif
   
    // Transfer the image old way
    if(!shmi){
      XPutImage(getDisplay(),shape,gc,xim,0,0,0,0,width,height);
#ifndef WIN32
      //// Need to use something other than malloc for WIN32....
      XDestroyImage(xim);
#endif
      }
    
    // We're done
    XFreeGC(getDisplay(),gc);
    }
  }


// Clean up
FXIcon::~FXIcon(){
  if(shape){XFreePixmap(getDisplay(),shape);}
  }
  

