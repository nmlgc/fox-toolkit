/********************************************************************************
*                                                                               *
*                             I m a g e    O b j e c t                          *
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
* $Id: FXImage.cpp,v 1.67.4.1 2002/05/13 21:10:32 fox Exp $                      *
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
#include "FXImage.h"
#include "FXDCWindow.h"


/*
  Notes:
  - FXImage::create() renders rgb[a] data into X/GDI resident, device
    dependent pixmap.
  - Need to be able to re-render subpart of image.
  - We should implement shared pixmaps.
  - If IMAGE_KEEP, repeated rendering is usually desired; should we
    hang on to XImage, and the shared memory segment in that case?
    How about shared pixmaps...
  - Slight change in interpretation of IMAGE_OWNED flag:- if passed, the
    FXImage will own client-side pixel buffer, otherwise it will not; if
    no pixel-buffer has been passed and IMAGE_OWNED *is* passed, a pixel
    buffer will be allocated [and cleared to zero].
    No pixel buffer will be allocated if neither IMAGE_OWNED nor pixels
    are passed.
  - When using shared image/pixmaps, if IMAGE_KEEP is set, hang on to pixel buffer.
  - Need resize, rotate member functions.
  - We need to speed up 16/15 bpp true color.
  - We need dither tables with 3bit and 2bit rounding for 5,6,5/5,5,5 modes
  - We need dither tables with 5bit, 6bit rounding for 3,3,2 mode.
  - We need to split true color from direct color, because direct color
    has random mapping, true has not.
  - Just because I always forget:

      StaticGray   0
      GrayScale    1
      StaticColor  2
      PseudoColor  3
      TrueColor    4
      DirectColor  5

  - An unowned data array should be considered read-only; so an image
    operation will cause a copy of the array, even if the size does not
    change.

  - The smooth scaling algorithm is based on the idea of keeping track which
    pixels of the source are contributing to each pixel in the destination.

    The smallest fraction of a pixel of interest is 1/(w*w'), where w is
    the old width, and w' is the new one.

    Consider scaling from 3->7 pixels, with 1/(w*w') being 1/21:

      Source Pixel 2 --------------+        Source Pixel is 7/21 units wide
      Source Pixel 1 --------+     |
      Source Pixel 0 -+      |     |
                      |      |     |
                      v      v     v
      Original:    000000011111112222222
      Scaled:      000111222333444555666
                    ^  ^  ^  ^  ^  ^  ^
                    |  |  |  |  |  |  |
      Dest Pixel 0 -+  |  |  |  |  |  |
      Dest Pixel 1 ----+  |  |  |  |  |
      Dest Pixel 2 -------+  |  |  |  |
      Dest Pixel 3 ----------+  |  |  |
      Dest Pixel 4 -------------+  |  |
      Dest Pixel 5 ----------------+  |
      Dest Pixel 6 -------------------+     Dest Pixel is 3/21 units wide

    As can be seen from the picture above, destination pixel 2 is comprised
    of 1/3 times source pixel 0, and 2/3 times source pixel 1.  Splitting
    into the individual fragments yields 9 fragments total:

      Fragment     0   1 2  3   4  5 6   7   8
      ========================================
      Original:  000 000 0 11 111 11 2 222 222
      Scaled:    000 111 2 22 333 44 4 555 666

    Note no fragment can be smaller than one unit, or 1/(w*w').

    The algorithm iterates over fragments; and for each fragment, it adds
    the source pixel multiplied by the length of the fragment.  It repeats
    until enough fragments have been collected to make one output pixel:

      fin  = w';                        The fractions fin and fout have
      fout = w;                         been multiplied by w*w'
      in   = 0;
      out  = 0;
      acc  = 0;
      while out < w' do
        if fin < fout then
          acc = acc + fin*source[in];   Add fin units of source pixel
          fout = fout - fin;
          fin = outw;
          in = in + 1;
        else
          acc = acc + fout*source[in];  Add fout (the remaining unfilled fraction) of source pixel
          dest[out] = acc / w;          Output one pixel
          acc = 0;
          fin = fin - fout;
          fout = inw;
          out = out + 1;
        end if
      end while

    Remember, you saw it here first!!!!

  - When compositing, out-of-image data behaves as if clear (0,0,0,0)
  - Absence of data behaves as if clear
  - Operations work on subrectangle of an image
  - Images and regions are at least 1x1 pixels
  - Operations of the form a = a op b, or a = op a.
  - Fast blend (0..255) colors
            blue  = (ALPHA * (sb - db) >> 8) + db;
            green = (ALPHA * (sg - dg) >> 8) + dg;
            red   = (ALPHA * (sr - dr) >> 8) + dr;
  - Need to change FXImage/FXIcon implementation so it ALWAYS has alpha channel.
*/


// RGB Ordering code
enum {
  RGB = 7,   // RGB 111      > | R  G  B
  BGR = 0,   // BGR 000      --+--------
  RBG = 6,   // RBG 110      R | x  4  2
  GBR = 1,   // GBR 001      G |    x  1
  BRG = 4,   // BRG 100      B |       x
  GRB = 3    // GRB 011
  };

#define DISPLAY(app) ((Display*)((app)->display))


// Maximum size of the colormap; for high-end graphics systems
// you may want to define HIGHENDGRAPHICS to allow for large colormaps
#ifdef HIGHENDGRAPHICS
#define MAX_MAPSIZE 4096
#else
#define MAX_MAPSIZE 256
#endif


/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXImage,FXDrawable,NULL,0)


// For deserialization
FXImage::FXImage(){
  data=NULL;
  options=0;
  channels=4;
  }


// Initialize
FXImage::FXImage(FXApp* a,const void *pix,FXuint opts,FXint w,FXint h):FXDrawable(a,w,h){
  FXTRACE((100,"FXImage::FXImage %p\n",this));
  visual=getApp()->getDefaultVisual();
  if(opts&IMAGE_ALPHA) channels=4; else channels=3;
  if(!pix && (opts&IMAGE_OWNED)){
    FXCALLOC(&pix,FXuchar,width*height*channels);
    }
  data=(FXuchar*)pix;
  options=opts;
  }


// Create image
void FXImage::create(){
  if(!xid){
    if(getApp()->initialized){
      FXTRACE((100,"%s::create %p\n",getClassName(),this));
#ifndef WIN32

      // Initialize visual
      visual->create();

      // Get depth (should use visual!!)
      int dd=visual->getDepth();

      // Make pixmap
      xid=XCreatePixmap(DISPLAY(getApp()),XDefaultRootWindow(DISPLAY(getApp())),width,height,dd);
      if(!xid){ fxerror("%s::create: unable to create image.\n",getClassName()); }
#else

      // Initialize visual
      visual->create();

      // Create a bitmap compatible with current display
      HDC hdc=::GetDC(GetDesktopWindow());
      xid=CreateCompatibleBitmap(hdc,width,height);
      ::ReleaseDC(GetDesktopWindow(),hdc);
      if(!xid){ fxerror("%s::create: unable to create image.\n",getClassName()); }
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


// Detach image
void FXImage::detach(){
  visual->detach();
  if(xid){
    FXTRACE((100,"%s::detach %p\n",getClassName(),this));
    xid=0;
    }
  }


// Destroy image
void FXImage::destroy(){
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


#ifndef WIN32

// Find shift amount
static inline FXuint findshift(unsigned long mask){
  register FXuint sh=0;
  while(!(mask&(1<<sh))) sh++;
  return sh;
  }


// Find low bit in mask
static inline FXPixel lowbit(FXPixel mask){
  return (~mask+1)&mask;
  }


// Restore client-side pixel buffer from image
void FXImage::restore(){
  if(xid){
    register FXPixel red,green,blue;
    register FXPixel red1,green1,blue1;
    register FXPixel pixel;
    register FXuint  redshift,greenshift,blueshift;
    register FXPixel redmask,greenmask,bluemask;
    register int size,dd,i;
    register FXbool shmi=FALSE;
    register XImage *xim=NULL;
    register Visual *vis;
    register FXint x,y;
    register FXuchar *img;
    register FXuint r,g,b;
    FXuchar rtab[MAX_MAPSIZE];
    FXuchar gtab[MAX_MAPSIZE];
    FXuchar btab[MAX_MAPSIZE];
#ifdef HAVE_XSHM
    XShmSegmentInfo shminfo;
#endif

    FXTRACE((100,"%s::restore image %p\n",getClassName(),this));

    // Check for legal size
    if(width<1 || height<1){ fxerror("%s::restore: illegal image size %dx%d.\n",getClassName(),width,height); }

    // Get Visual
    vis=(Visual*)visual->visual;
    dd=visual->getDepth();

    // Just in case you're on a high-end system
    FXASSERT(vis->map_entries<=MAX_MAPSIZE);

    // Make array for data if needed
    if(!data || !(options&IMAGE_OWNED)){
      size=width*height*channels;
      FXMALLOC(&data,FXuchar,size);
      options|=IMAGE_OWNED;
      }

    // Got local buffer to receive into
    if(data){

      // Turn it on iff both supported and desired
#ifdef HAVE_XSHM
      if(options&IMAGE_SHMI) shmi=getApp()->shmi;
#endif

      // First try XShm
#ifdef HAVE_XSHM
      if(shmi){
        xim=XShmCreateImage(DISPLAY(getApp()),vis,dd,(dd==1)?XYPixmap:ZPixmap,NULL,&shminfo,width,height);
        if(!xim){ shmi=0; }
        if(shmi){
          shminfo.shmid=shmget(IPC_PRIVATE,xim->bytes_per_line*xim->height,IPC_CREAT|0777);
          if(shminfo.shmid==-1){ xim->data=NULL; XDestroyImage(xim); xim=NULL; shmi=0; }
          if(shmi){
            shminfo.shmaddr=xim->data=(char*)shmat(shminfo.shmid,0,0);
            shminfo.readOnly=FALSE;
            XShmAttach(DISPLAY(getApp()),&shminfo);
            FXTRACE((150,"RGBPixmap XSHM attached at memory=%p (%d bytes)\n",xim->data,xim->bytes_per_line*xim->height));
            XShmGetImage(DISPLAY(getApp()),xid,xim,0,0,AllPlanes);
            XSync(DISPLAY(getApp()),False);
            }
          }
        }
#endif

      // Try the old fashioned way
      if(!shmi){
        xim=XGetImage(DISPLAY(getApp()),xid,0,0,width,height,AllPlanes,ZPixmap);
        if(!xim){ fxerror("%s::restore: unable to restore image.\n",getClassName()); }
        }

      // Should have succeeded
      FXASSERT(xim);

      FXTRACE((150,"im width = %d\n",xim->width));
      FXTRACE((150,"im height = %d\n",xim->height));
      FXTRACE((150,"im format = %s\n",xim->format==XYBitmap?"XYBitmap":xim->format==XYPixmap?"XYPixmap":"ZPixmap"));
      FXTRACE((150,"im byte_order = %s\n",(xim->byte_order==MSBFirst)?"MSBFirst":"LSBFirst"));
      FXTRACE((150,"im bitmap_unit = %d\n",xim->bitmap_unit));
      FXTRACE((150,"im bitmap_bit_order = %s\n",(xim->bitmap_bit_order==MSBFirst)?"MSBFirst":"LSBFirst"));
      FXTRACE((150,"im bitmap_pad = %d\n",xim->bitmap_pad));
      FXTRACE((150,"im bitmap_unit = %d\n",xim->bitmap_unit));
      FXTRACE((150,"im depth = %d\n",xim->depth));
      FXTRACE((150,"im bytes_per_line = %d\n",xim->bytes_per_line));
      FXTRACE((150,"im bits_per_pixel = %d\n",xim->bits_per_pixel));


      {
      XColor colors[MAX_MAPSIZE];
      
      // Get masks
      redmask=vis->red_mask;
      greenmask=vis->green_mask;
      bluemask=vis->blue_mask;

      // Read back the colormap and convert to more usable form
      if(vis->c_class!=TrueColor && vis->c_class!=DirectColor){
        for(i=0 ; i<vis->map_entries; i++){
          colors[i].pixel=i;
          colors[i].flags=DoRed|DoGreen|DoBlue;
          }
        }
      else{
        red=green=blue=0;
        red1=lowbit(redmask);
        green1=lowbit(greenmask);
        blue1=lowbit(bluemask);
        for(i=0; i<vis->map_entries; i++){
          colors[i].pixel=red|green|blue;
          colors[i].flags=DoRed|DoGreen|DoBlue;
          if(red<redmask) red+=red1;
          if(green<greenmask) green+=green1;
          if(blue<bluemask) blue+=blue1;
          }
        }
      XQueryColors(DISPLAY(getApp()),visual->colormap,colors,vis->map_entries);
      for(i=0; i<vis->map_entries; i++){
        rtab[i]=colors[i].red >> 8;
        gtab[i]=colors[i].green >> 8;
        btab[i]=colors[i].blue >> 8;
        }
      }

      // Now we convert the pixels back to color
      img=data;
      switch(xim->bits_per_pixel){
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
          for(y=0; y<height; y++){
            for(x=0; x<width; x++){
              pixel=XGetPixel(xim,x,y);
              img[0]=rtab[pixel];
              img[1]=gtab[pixel];
              img[2]=btab[pixel];
              img+=channels;
              }
            }
          break;
        case 15:
        case 16:
        case 24:
        case 32:
        default:
          FXASSERT(vis->c_class==TrueColor || vis->c_class==DirectColor);
          redshift=findshift(redmask);
          greenshift=findshift(greenmask);
          blueshift=findshift(bluemask);
          for(y=0; y<height; y++){
            for(x=0; x<width; x++){
              pixel=XGetPixel(xim,x,y);
              r=(pixel&redmask)>>redshift;
              g=(pixel&greenmask)>>greenshift;
              b=(pixel&bluemask)>>blueshift;
              img[0]=rtab[r];
              img[1]=gtab[g];
              img[2]=btab[b];
              img+=channels;
              }
            }
          break;
        }

      // Destroy image
#ifdef HAVE_XSHM
      if(shmi){
        FXTRACE((150,"RGBPixmap XSHM detached at memory=%p (%d bytes)\n",xim->data,xim->bytes_per_line*xim->height));
        XShmDetach(DISPLAY(getApp()),&shminfo);
        XDestroyImage(xim);
        shmdt(shminfo.shmaddr);
        shmctl(shminfo.shmid,IPC_RMID,0);
        }
#endif

      // Destroy image
      if(!shmi){
        XDestroyImage(xim);
        }
      }
    }
  }


#else


// Restore client-side pixel buffer from image
void FXImage::restore(){
  if(xid){
    register FXint size,bytes_per_line,skip;
    register FXint x,y;
    register FXuchar *pixels,*pix,*img;
    BITMAPINFO bmi;
    HDC hdcmem;

    FXTRACE((100,"%s::restore image %p\n",getClassName(),this));

    // Check for legal size
    if(width<1 || height<1){ fxerror("%s::restore: illegal image size %dx%d.\n",getClassName(),width,height); }

    // Make array for data if needed
    if(!data || !(options&IMAGE_OWNED)){
      size=width*height*channels;
      FXMALLOC(&data,FXuchar,size);
      options|=IMAGE_OWNED;
      }

    // Got local buffer to receive into
    if(data){

      // Set up the bitmap info
      bytes_per_line=(width*3+3)/4*4;
      skip=bytes_per_line-width*3;

      bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
      bmi.bmiHeader.biWidth=width;
      bmi.bmiHeader.biHeight=-height; // Negative heights means upside down!
      bmi.bmiHeader.biPlanes=1;
      bmi.bmiHeader.biBitCount=24;
      bmi.bmiHeader.biCompression=BI_RGB;
      bmi.bmiHeader.biSizeImage=0;
      bmi.bmiHeader.biXPelsPerMeter=0;
      bmi.bmiHeader.biYPelsPerMeter=0;
      bmi.bmiHeader.biClrUsed=0;
      bmi.bmiHeader.biClrImportant=0;

      // DIB format pads to multiples of 4 bytes...
      FXMALLOC(&pixels,FXuchar,bytes_per_line*height);

      // Make device context
      hdcmem=::CreateCompatibleDC(NULL);
      if(!GetDIBits(hdcmem,(HBITMAP)xid,0,height,pixels,&bmi,DIB_RGB_COLORS)){
        fxerror("%s::render: unable to restore pixels\n",getClassName());
        }

      // Stuff it into our own data structure
      img=data;
      pix=pixels;
      for(y=0; y<height; y++){
        for(x=0; x<width; x++){
          img[0]=pix[2];
          img[1]=pix[1];
          img[2]=pix[0];
          img+=channels;
          pix+=3;
          }
        pix+=skip;
        }
      FXFREE(&pixels);
      ::DeleteDC(hdcmem);
      }
    }
  }


#endif

#if 0
/*************************************************************************
 *
 * BitmapToDIB()
 *
 * Parameters:
 *
 * HBITMAP hBitmap  - specifies the bitmap to convert
 *
 * HPALETTE hPal    - specifies the palette to use with the bitmap
 *
 * Return Value:
 *
 * HDIB             - identifies the device-dependent bitmap
 *
 * Description:
 *
 * This function creates a DIB from a bitmap using the specified palette.
 *
 ************************************************************************/

HDIB BitmapToDIB(HBITMAP hBitmap, HPALETTE hPal)
{
    BITMAP              bm;         // bitmap structure
    BITMAPINFOHEADER    bi;         // bitmap header
    LPBITMAPINFOHEADER  lpbi;       // pointer to BITMAPINFOHEADER
    DWORD               dwLen;      // size of memory block
    HANDLE              hDIB, h;    // handle to DIB, temp handle
    HDC                 hDC;        // handle to DC
    WORD                biBits;     // bits per pixel

    // check if bitmap handle is valid

    if (!hBitmap)
        return NULL;

    // fill in BITMAP structure, return NULL if it didn't work

    if (!GetObject(hBitmap, sizeof(bm), (LPSTR)&bm))
        return NULL;

    // if no palette is specified, use default palette

    if (hPal == NULL)
        hPal = GetStockObject(DEFAULT_PALETTE);

    // calculate bits per pixel

    biBits = bm.bmPlanes * bm.bmBitsPixel;

    // make sure bits per pixel is valid

    if (biBits <= 1)
        biBits = 1;
    else if (biBits <= 4)
        biBits = 4;
    else if (biBits <= 8)
        biBits = 8;
    else // if greater than 8-bit, force to 24-bit
        biBits = 24;

    // initialize BITMAPINFOHEADER

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bm.bmWidth;
    bi.biHeight = bm.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = biBits;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    // calculate size of memory block required to store BITMAPINFO

    dwLen = bi.biSize + PaletteSize((LPSTR)&bi);

    // get a DC

    hDC = GetDC(NULL);

    // select and realize our palette

    hPal = SelectPalette(hDC, hPal, FALSE);
    RealizePalette(hDC);

    // alloc memory block to store our bitmap

    hDIB = GlobalAlloc(GHND, dwLen);

    // if we couldn't get memory block

    if (!hDIB)
    {
      // clean up and return NULL

      SelectPalette(hDC, hPal, TRUE);
      RealizePalette(hDC);
      ReleaseDC(NULL, hDC);
      return NULL;
    }

    // lock memory and get pointer to it

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB);

    /// use our bitmap info. to fill BITMAPINFOHEADER

    *lpbi = bi;

    // call GetDIBits with a NULL lpBits param, so it will calculate the
    // biSizeImage field for us

    GetDIBits(hDC, hBitmap, 0, (UINT)bi.biHeight, NULL, (LPBITMAPINFO)lpbi,
        DIB_RGB_COLORS);

    // get the info. returned by GetDIBits and unlock memory block

    bi = *lpbi;
    GlobalUnlock(hDIB);

    // if the driver did not fill in the biSizeImage field, make one up
    if (bi.biSizeImage == 0)
        bi.biSizeImage = WIDTHBYTES((DWORD)bm.bmWidth * biBits) * bm.bmHeight;

    // realloc the buffer big enough to hold all the bits

    dwLen = bi.biSize + PaletteSize((LPSTR)&bi) + bi.biSizeImage;

    if (h = GlobalReAlloc(hDIB, dwLen, 0))
        hDIB = h;
    else
    {
        // clean up and return NULL

        GlobalFree(hDIB);
        hDIB = NULL;
        SelectPalette(hDC, hPal, TRUE);
        RealizePalette(hDC);
        ReleaseDC(NULL, hDC);
        return NULL;
    }

    // lock memory block and get pointer to it */

    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB);

    // call GetDIBits with a NON-NULL lpBits param, and actualy get the
    // bits this time

    if (GetDIBits(hDC, hBitmap, 0, (UINT)bi.biHeight, (LPSTR)lpbi +
            (WORD)lpbi->biSize + PaletteSize((LPSTR)lpbi), (LPBITMAPINFO)lpbi,
            DIB_RGB_COLORS) == 0)
    {
        // clean up and return NULL

        GlobalUnlock(hDIB);
        hDIB = NULL;
        SelectPalette(hDC, hPal, TRUE);
        RealizePalette(hDC);
        ReleaseDC(NULL, hDC);
        return NULL;
    }

    bi = *lpbi;

    // clean up
    GlobalUnlock(hDIB);
    SelectPalette(hDC, hPal, TRUE);
    RealizePalette(hDC);
    ReleaseDC(NULL, hDC);

    // return handle to the DIB
    return hDIB;
}
#endif




#ifndef WIN32



// True generic mode
void FXImage::render_true_N_fast(void *xim,FXuchar *img){
  register FXint x,y;
  FXTRACE((150,"True MSB/LSB N bpp render nearest\n"));
  y=0;
  do{
    x=0;
    do{
      XPutPixel(((XImage*)xim),x,y,visual->rpix[1][img[0]] | visual->gpix[1][img[1]] | visual->bpix[1][img[2]]);
      img+=channels;
      }
    while(++x<width);
    }
  while(++y<height);
  }



// True generic mode
void FXImage::render_true_N_dither(void *xim,FXuchar *img){
  register FXint x,y,d;
  FXTRACE((150,"True MSB/LSB N bpp render dither\n"));
  y=0;
  do{
    x=0;
    do{
      d=((y&3)<<2)|(x&3);
      XPutPixel(((XImage*)xim),x,y,visual->rpix[d][img[0]] | visual->gpix[d][img[1]] | visual->bpix[d][img[2]]);
      img+=channels;
      }
    while(++x<width);
    }
  while(++y<height);
  }



// True 24 bit color
void FXImage::render_true_24(void *xim,FXuchar *img){
  register FXuint jmp=((XImage*)xim)->bytes_per_line-(width*3);
  register FXuchar *pix=(FXuchar*)((XImage*)xim)->data;
  register FXPixel val;
  register FXint w,h;
  if(((XImage*)xim)->byte_order==MSBFirst){    // MSB
    FXTRACE((150,"True MSB 24bpp render\n"));
    h=height-1;
    do{
      w=width-1;
      do{
        val=visual->rpix[1][img[0]] | visual->gpix[1][img[1]] | visual->bpix[1][img[2]];
        pix[0]=(FXuchar)(val>>16);
        pix[1]=(FXuchar)(val>>8);
        pix[2]=(FXuchar)val;
        img+=channels;
        pix+=3;
        }
      while(--w>=0);
      pix+=jmp;
      }
    while(--h>=0);
    }
  else{                             // LSB
    FXTRACE((150,"True LSB 24bpp render\n"));
    h=height-1;
    do{
      w=width-1;
      do{
        val=visual->rpix[1][img[0]] | visual->gpix[1][img[1]] | visual->bpix[1][img[2]];
        pix[0]=(FXuchar)val;
        pix[1]=(FXuchar)(val>>8);
        pix[2]=(FXuchar)(val>>16);
        img+=channels;
        pix+=3;
        }
      while(--w>=0);
      pix+=jmp;
      }
    while(--h>=0);
    }
  }



// True 32 bit color
void FXImage::render_true_32(void *xim,FXuchar *img){
  register FXuchar *pix=(FXuchar*)((XImage*)xim)->data;
  register FXuint jmp=((XImage*)xim)->bytes_per_line-(width<<2);
  register FXPixel val;
  register FXint w,h;

  // Byte order matches
  if(((XImage*)xim)->byte_order == FOX_BIGENDIAN){
    FXTRACE((150,"True MSB/LSB 32bpp render\n"));
    h=height-1;
    do{
      w=width-1;
      do{
        *((FXuint*)pix)=visual->rpix[1][img[0]] | visual->gpix[1][img[1]] | visual->bpix[1][img[2]];
        img+=channels;
        pix+=4;
        }
      while(--w>=0);
      pix+=jmp;
      }
    while(--h>=0);
    }

  // MSB Byte order
  else if(((XImage*)xim)->byte_order==MSBFirst){
    FXTRACE((150,"True MSB 32bpp render\n"));
    h=height-1;
    do{
      w=width-1;
      do{
        val=visual->rpix[1][img[0]] | visual->gpix[1][img[1]] | visual->bpix[1][img[2]];
        pix[0]=(FXuchar)(val>>24);
        pix[1]=(FXuchar)(val>>16);
        pix[2]=(FXuchar)(val>>8);
        pix[3]=(FXuchar)val;
        img+=channels;
        pix+=4;
        }
      while(--w>=0);
      pix+=jmp;
      }
    while(--h>=0);
    }

  // LSB Byte order
  else{
    FXTRACE((150,"True LSB 32bpp render\n"));
    h=height-1;
    do{
      w=width-1;
      do{
        val=visual->rpix[1][img[0]] | visual->gpix[1][img[1]] | visual->bpix[1][img[2]];
        pix[0]=(FXuchar)val;
        pix[1]=(FXuchar)(val>>8);
        pix[2]=(FXuchar)(val>>16);
        pix[3]=(FXuchar)(val>>24);
        img+=channels;
        pix+=4;
        }
      while(--w>=0);
      pix+=jmp;
      }
    while(--h>=0);
    }
  }



// True 16 bit color
void FXImage::render_true_16_fast(void *xim,FXuchar *img){
  register FXuint jmp=((XImage*)xim)->bytes_per_line-(width<<1);
  register FXuchar *pix=(FXuchar*)((XImage*)xim)->data;
  register FXPixel val;
  register FXint w,h;

  // Byte order matches
  if(((XImage*)xim)->byte_order == FOX_BIGENDIAN){
    FXTRACE((150,"True MSB/LSB 16bpp 5,6,5/5,5,5 render nearest\n"));
    h=height-1;
    do{
      w=width-1;
      do{
        *((FXushort*)pix)=visual->rpix[1][img[0]] | visual->gpix[1][img[1]] | visual->bpix[1][img[2]];
        img+=channels;
        pix+=2;
        }
      while(--w>=0);
      pix+=jmp;
      }
    while(--h>=0);
    }

  // MSB Byte order
  else if(((XImage*)xim)->byte_order==MSBFirst){
    FXTRACE((150,"True MSB 16bpp 5,6,5/5,5,5 render nearest\n"));
    h=height-1;
    do{
      w=width-1;
      do{
        val=visual->rpix[1][img[0]] | visual->gpix[1][img[1]] | visual->bpix[1][img[2]];
        pix[0]=(FXuchar)(val>>8);
        pix[1]=(FXuchar)val;
        img+=channels;
        pix+=2;
        }
      while(--w>=0);
      pix+=jmp;
      }
    while(--h>=0);
    }

  // LSB Byte order
  else{
    FXTRACE((150,"True LSB 16bpp 5,6,5/5,5,5 render nearest\n"));
    h=height-1;
    do{
      w=width-1;
      do{
        val=visual->rpix[1][img[0]] | visual->gpix[1][img[1]] | visual->bpix[1][img[2]];
        pix[0]=(FXuchar)val;
        pix[1]=(FXuchar)(val>>8);
        img+=channels;
        pix+=2;
        }
      while(--w>=0);
      pix+=jmp;
      }
    while(--h>=0);
    }
  }


// True 16 bit color, dithered
void FXImage::render_true_16_dither(void *xim,FXuchar *img){
  register FXuint jmp=((XImage*)xim)->bytes_per_line-(width<<1);
  register FXuchar *pix=(FXuchar*)((XImage*)xim)->data;
  register FXPixel val;
  register FXint w,h,d;

  // Byte order matches
  if(((XImage*)xim)->byte_order == FOX_BIGENDIAN){
    FXTRACE((150,"True MSB/LSB 16bpp 5,6,5/5,5,5 render dither\n"));
    h=height-1;
    do{
      w=width-1;
      do{
        d=((h&3)<<2)|(w&3);
        *((FXushort*)pix)=visual->rpix[d][img[0]] | visual->gpix[d][img[1]] | visual->bpix[d][img[2]];
        img+=channels;
        pix+=2;
        }
      while(--w>=0);
      pix+=jmp;
      }
    while(--h>=0);
    }

  // MSB Byte order
  else if(((XImage*)xim)->byte_order==MSBFirst){
    FXTRACE((150,"True MSB 16bpp 5,6,5/5,5,5 render dither\n"));
    h=height-1;
    do{
      w=width-1;
      do{
        d=((h&3)<<2)|(w&3);
        val=visual->rpix[d][img[0]] | visual->gpix[d][img[1]] | visual->bpix[d][img[2]];
        pix[0]=(FXuchar)(val>>8);
        pix[1]=(FXuchar)val;
        img+=channels;
        pix+=2;
        }
      while(--w>=0);
      pix+=jmp;
      }
    while(--h>=0);
    }

  // LSB Byte order
  else{
    FXTRACE((150,"True LSB 16bpp 5,6,5/5,5,5 render dither\n"));
    h=height-1;
    do{
      w=width-1;
      do{
        d=((h&3)<<2)|(w&3);
        val=visual->rpix[d][img[0]] | visual->gpix[d][img[1]] | visual->bpix[d][img[2]];
        pix[0]=(FXuchar)val;
        pix[1]=(FXuchar)(val>>8);
        img+=channels;
        pix+=2;
        }
      while(--w>=0);
      pix+=jmp;
      }
    while(--h>=0);
    }
  }



// True 8 bit color
void FXImage::render_true_8_fast(void *xim,FXuchar *img){
  register FXuint jmp=((XImage*)xim)->bytes_per_line-width;
  register FXuchar *pix=(FXuchar*)((XImage*)xim)->data;
  register FXint w,h;
  FXTRACE((150,"True MSB/LSB 8bpp render nearest\n"));
  h=height-1;
  do{
    w=width-1;
    do{
      *pix=visual->rpix[1][img[0]] | visual->gpix[1][img[1]] | visual->bpix[1][img[2]];
      img+=channels;
      pix++;
      }
    while(--w>=0);
    pix+=jmp;
    }
  while(--h>=0);
  }



// True 8 bit color, dithered
void FXImage::render_true_8_dither(void *xim,FXuchar *img){
  register FXuint jmp=((XImage*)xim)->bytes_per_line-width;
  register FXuchar *pix=(FXuchar*)((XImage*)xim)->data;
  register FXint w,h,d;
  FXTRACE((150,"True MSB/LSB 8bpp render dither\n"));
  h=height-1;
  do{
    w=width-1;
    do{
      d=((h&3)<<2)|(w&3);
      *pix=visual->rpix[d][img[0]] | visual->gpix[d][img[1]] | visual->bpix[d][img[2]];
      img+=channels;
      pix++;
      }
    while(--w>=0);
    pix+=jmp;
    }
  while(--h>=0);
  }



// Render 4 bit index color mode
void FXImage::render_index_4_fast(void *xim,FXuchar *img){
  register FXuchar *pix=(FXuchar*)((XImage*)xim)->data;
  register FXuint jmp=((XImage*)xim)->bytes_per_line-width;
  register FXuint val,half;
  register FXint w,h;
  if(((XImage*)xim)->byte_order==MSBFirst){    // MSB
    FXTRACE((150,"Index MSB 4bpp render nearest\n"));
    h=height-1;
    do{
      w=width-1;
      half=0;
      do{
        val=visual->lut[visual->rpix[1][img[0]]+visual->gpix[1][img[1]]+visual->bpix[1][img[2]]];
        if(half) *pix++|=val;
        else *pix=val<<4;
        half^=1;
        img+=channels;
        }
      while(--w>=0);
      pix+=jmp;
      }
    while(--h>=0);
    }
  else{                               // LSB
    FXTRACE((150,"Index LSB 4bpp render nearest\n"));
    h=height-1;
    do{
      w=width-1;
      half=0;
      do{
        val=visual->lut[visual->rpix[1][img[0]]+visual->gpix[1][img[1]]+visual->bpix[1][img[2]]];
        if(half) *pix++|=val<<4;
        else *pix=val;
        half^=1;
        img+=channels;
        }
      while(--w>=0);
      pix+=jmp;
      }
    while(--h>=0);
    }
  }



// Render 4 bit index color mode
void FXImage::render_index_4_dither(void *xim,FXuchar *img){
  register FXuchar *pix=(FXuchar*)((XImage*)xim)->data;
  register FXuint jmp=((XImage*)xim)->bytes_per_line-width;
  register FXuint val,half,d;
  register FXint w,h;
  if(((XImage*)xim)->byte_order==MSBFirst){    // MSB
    FXTRACE((150,"Index MSB 4bpp render dither\n"));
    h=height-1;
    do{
      w=width-1;
      half=0;
      do{
        d=((h&3)<<2)|(w&3);
        val=visual->lut[visual->rpix[d][img[0]]+visual->gpix[d][img[1]]+visual->bpix[d][img[2]]];
        if(half) *pix++|=val;
        else *pix=val<<4;
        half^=1;
        img+=channels;
        }
      while(--w>=0);
      pix+=jmp;
      }
    while(--h>=0);
    }
  else{                               // LSB
    FXTRACE((150,"Index LSB 4bpp render dither\n"));
    h=height-1;
    do{
      w=width-1;
      half=0;
      do{
        d=((h&3)<<2)|(w&3);
        val=visual->lut[visual->rpix[d][img[0]]+visual->gpix[d][img[1]]+visual->bpix[d][img[2]]];
        if(half) *pix++|=val<<4;
        else *pix=val;
        half^=1;
        img+=channels;
        }
      while(--w>=0);
      pix+=jmp;
      }
    while(--h>=0);
    }
  }



// Render 8 bit index color mode
void FXImage::render_index_8_fast(void *xim,FXuchar *img){
  register FXuint jmp=((XImage*)xim)->bytes_per_line-width;
  register FXuchar *pix=(FXuchar*)((XImage*)xim)->data;
  register FXint w,h;
  FXTRACE((150,"Index MSB/LSB 8bpp render nearest\n"));
  h=height-1;
  do{
    w=width-1;
    do{
      *pix=visual->lut[visual->rpix[1][img[0]]+visual->gpix[1][img[1]]+visual->bpix[1][img[2]]];
      img+=channels;
      pix++;
      }
    while(--w>=0);
    pix+=jmp;
    }
  while(--h>=0);
  }



// Render 8 bit index color mode
void FXImage::render_index_8_dither(void *xim,FXuchar *img){
  register FXuint jmp=((XImage*)xim)->bytes_per_line-width;
  register FXuchar *pix=(FXuchar*)((XImage*)xim)->data;
  register FXint w,h,d;
  FXTRACE((150,"Index MSB/LSB 8bpp render dither\n"));
  h=height-1;
  do{
    w=width-1;
    do{
      d=((h&3)<<2)|(w&3);
      *pix=visual->lut[visual->rpix[d][img[0]]+visual->gpix[d][img[1]]+visual->bpix[d][img[2]]];
      img+=channels;
      pix++;
      }
    while(--w>=0);
    pix+=jmp;
    }
  while(--h>=0);
  }



// Render generic N bit index color mode
void FXImage::render_index_N_fast(void *xim,FXuchar *img){
  register FXint x,y;
  FXTRACE((150,"Index MSB/LSB N bpp render nearest\n"));
  y=0;
  do{
    x=0;
    do{
      XPutPixel(((XImage*)xim),x,y,visual->lut[visual->rpix[1][img[0]]+visual->gpix[1][img[1]]+visual->bpix[1][img[2]]]);
      img+=channels;
      }
    while(++x<width);
    }
  while(++y<height);
  }



// Render generic N bit index color mode
void FXImage::render_index_N_dither(void *xim,FXuchar *img){
  register FXint x,y,d;
  FXTRACE((150,"Index MSB/LSB N bpp render dither\n"));
  y=0;
  do{
    x=0;
    do{
      d=((y&3)<<2)|(x&3);
      XPutPixel(((XImage*)xim),x,y,visual->lut[visual->rpix[d][img[0]]+visual->gpix[d][img[1]]+visual->bpix[d][img[2]]]);
      img+=channels;
      }
    while(++x<width);
    }
  while(++y<height);
  }



// Render 8 bit gray mode
void FXImage::render_gray_8_fast(void *xim,FXuchar *img){
  register FXuchar *pix=(FXuchar*)((XImage*)xim)->data;
  register FXuint jmp=((XImage*)xim)->bytes_per_line-width;
  register FXint w,h;
  FXTRACE((150,"Gray MSB/LSB 8bpp render nearest\n"));
  h=height-1;
  do{
    w=width-1;
    do{
      *pix=visual->gpix[1][(77*img[0]+151*img[1]+29*img[2])>>8];
      img+=channels;
      pix++;
      }
    while(--w>=0);
    pix+=jmp;
    }
  while(--h>=0);
  }



// Render 8 bit gray mode
void FXImage::render_gray_8_dither(void *xim,FXuchar *img){
  register FXuchar *pix=(FXuchar*)((XImage*)xim)->data;
  register FXuint jmp=((XImage*)xim)->bytes_per_line-width;
  register FXint w,h;
  FXTRACE((150,"Gray MSB/LSB 8bpp render dither\n"));
  h=height-1;
  do{
    w=width-1;
    do{
      *pix=visual->gpix[((h&3)<<2)|(w&3)][(77*img[0]+151*img[1]+29*img[2])>>8];
      img+=channels;
      pix++;
      }
    while(--w>=0);
    pix+=jmp;
    }
  while(--h>=0);
  }



// Render generic N bit gray mode
void FXImage::render_gray_N_fast(void *xim,FXuchar *img){
  register FXint x,y;
  FXTRACE((150,"Gray MSB/LSB N bpp render nearest\n"));
  y=0;
  do{
    x=0;
    do{
      XPutPixel(((XImage*)xim),x,y,visual->gpix[1][(77*img[0]+151*img[1]+29*img[2])>>8]);
      img+=channels;
      }
    while(++x<width);
    }
  while(++y<height);
  }



// Render generic N bit gray mode
void FXImage::render_gray_N_dither(void *xim,FXuchar *img){
  register FXint x,y;
  FXTRACE((150,"Gray MSB/LSB N bpp render dither\n"));
  y=0;
  do{
    x=0;
    do{
      XPutPixel(((XImage*)xim),x,y,visual->gpix[((y&3)<<2)|(x&3)][(77*img[0]+151*img[1]+29*img[2])>>8]);
      img+=channels;
      }
    while(++x<width);
    }
  while(++y<height);
  }




// Render monochrome mode
void FXImage::render_mono_1_fast(void *xim,FXuchar *img){
  register FXint x,y;
  FXTRACE((150,"Monochrome MSB/LSB 1bpp render nearest\n"));
  y=0;
  do{
    x=0;
    do{
      XPutPixel(((XImage*)xim),x,y,visual->gpix[1][(77*img[0]+151*img[1]+29*img[2])>>8]);
      img+=channels;
      }
    while(++x<width);
    }
  while(++y<height);
  }



// Render monochrome mode
void FXImage::render_mono_1_dither(void *xim,FXuchar *img){
  register FXint x,y;
  FXTRACE((150,"Monochrome MSB/LSB 1bpp render dither\n"));
  y=0;
  do{
    x=0;
    do{
      XPutPixel(((XImage*)xim),x,y,visual->gpix[((y&3)<<2)|(x&3)][(77*img[0]+151*img[1]+29*img[2])>>8]);
      img+=channels;
      }
    while(++x<width);
    }
  while(++y<height);
  }



#endif



#ifndef WIN32


// Render into pixmap
void FXImage::render(){
  if(xid){
    register FXbool shmi=FALSE;
    register XImage *xim=NULL;
    register Visual *vis;
    register int dd;
    XGCValues values;
    GC gc;
#ifdef HAVE_XSHM
    XShmSegmentInfo shminfo;
#endif

    FXTRACE((100,"%s::render image %p\n",getClassName(),this));

    // Check for legal size
    if(width<1 || height<1){ fxerror("%s::render: illegal image size %dx%d.\n",getClassName(),width,height); }

    // Just leave if black if no data
    if(data){

      // Make GC
      values.foreground=BlackPixel(DISPLAY(getApp()),DefaultScreen(DISPLAY(getApp())));
      values.background=WhitePixel(DISPLAY(getApp()),DefaultScreen(DISPLAY(getApp())));
      gc=XCreateGC(DISPLAY(getApp()),xid,GCForeground|GCBackground,&values);

      // Get Visual
      vis=(Visual*)visual->visual;

      dd=visual->getDepth();

      // Turn it on iff both supported and desired
#ifdef HAVE_XSHM
      if(options&IMAGE_SHMI) shmi=getApp()->shmi;
#endif

      // First try XShm
#ifdef HAVE_XSHM
      if(shmi){
        xim=XShmCreateImage(DISPLAY(getApp()),vis,dd,(dd==1)?XYPixmap:ZPixmap,NULL,&shminfo,width,height);
        if(!xim){ shmi=0; }
        if(shmi){
          shminfo.shmid=shmget(IPC_PRIVATE,xim->bytes_per_line*xim->height,IPC_CREAT|0777);
          if(shminfo.shmid==-1){ xim->data=NULL; XDestroyImage(xim); xim=NULL; shmi=0; }
          if(shmi){
            shminfo.shmaddr=xim->data=(char*)shmat(shminfo.shmid,0,0);
            shminfo.readOnly=FALSE;
            XShmAttach(DISPLAY(getApp()),&shminfo);
            FXTRACE((150,"RGBPixmap XSHM attached at memory=%p (%d bytes)\n",xim->data,xim->bytes_per_line*xim->height));
            }
          }
        }
#endif

      // Try the old fashioned way
      if(!shmi){
        xim=XCreateImage(DISPLAY(getApp()),vis,dd,(dd==1)?XYPixmap:ZPixmap,0,NULL,width,height,32,0);
        if(!xim){ fxerror("%s::render: unable to render image.\n",getClassName()); }

        // Try create temp pixel store
        if(!FXMALLOC(&xim->data,char,xim->bytes_per_line*height)){ fxerror("%s::render: unable to allocate memory.\n",getClassName()); }
        }

      // Should have succeeded
      FXASSERT(xim);

      FXTRACE((150,"im width = %d\n",xim->width));
      FXTRACE((150,"im height = %d\n",xim->height));
      FXTRACE((150,"im format = %s\n",xim->format==XYBitmap?"XYBitmap":xim->format==XYPixmap?"XYPixmap":"ZPixmap"));
      FXTRACE((150,"im byte_order = %s\n",(xim->byte_order==MSBFirst)?"MSBFirst":"LSBFirst"));
      FXTRACE((150,"im bitmap_unit = %d\n",xim->bitmap_unit));
      FXTRACE((150,"im bitmap_bit_order = %s\n",(xim->bitmap_bit_order==MSBFirst)?"MSBFirst":"LSBFirst"));
      FXTRACE((150,"im bitmap_pad = %d\n",xim->bitmap_pad));
      FXTRACE((150,"im bitmap_unit = %d\n",xim->bitmap_unit));
      FXTRACE((150,"im depth = %d\n",xim->depth));
      FXTRACE((150,"im bytes_per_line = %d\n",xim->bytes_per_line));
      FXTRACE((150,"im bits_per_pixel = %d\n",xim->bits_per_pixel));

      // Determine what to do
      switch(visual->getType()){
        case VISUALTYPE_TRUE:
          switch(xim->bits_per_pixel){
            case 32:
              render_true_32(xim,data);
              break;
            case 24:
              render_true_24(xim,data);
              break;
            case 15:
            case 16:
              if(options&IMAGE_NEAREST)
                render_true_16_fast(xim,data);
              else
                render_true_16_dither(xim,data);
              break;
            case 8:
              if(options&IMAGE_NEAREST)
                render_true_8_fast(xim,data);
              else
                render_true_8_dither(xim,data);
              break;
            default:
              if(options&IMAGE_NEAREST)
                render_true_N_fast(xim,data);
              else
                render_true_N_dither(xim,data);
              break;
            }
          break;
        case VISUALTYPE_GRAY:
          switch(xim->bits_per_pixel){
            case 1:
              if(options&IMAGE_NEAREST)
                render_mono_1_fast(xim,data);
              else
                render_mono_1_dither(xim,data);
              break;
            case 8:
              if(options&IMAGE_NEAREST)
                render_gray_8_fast(xim,data);
              else
                render_gray_8_dither(xim,data);
              break;
            default:
              if(options&IMAGE_NEAREST)
                render_gray_N_fast(xim,data);
              else
                render_gray_N_dither(xim,data);
              break;
            }
          break;
        case VISUALTYPE_INDEX:
          switch(xim->bits_per_pixel){
            case 4:
              if(options&IMAGE_NEAREST)
                render_index_4_fast(xim,data);
              else
                render_index_4_dither(xim,data);
              break;
            case 8:
              if(options&IMAGE_NEAREST)
                render_index_8_fast(xim,data);
              else
                render_index_8_dither(xim,data);
              break;
            default:
              if(options&IMAGE_NEAREST)
                render_index_N_fast(xim,data);
              else
                render_index_N_dither(xim,data);
              break;
            }
          break;
        case VISUALTYPE_MONO:
          if(options&IMAGE_NEAREST)
            render_mono_1_fast(xim,data);
          else
            render_mono_1_dither(xim,data);
        case VISUALTYPE_UNKNOWN:
          break;
        }

      // Transfer image with shared memory
#ifdef HAVE_XSHM
      if(shmi){
        XShmPutImage(DISPLAY(getApp()),xid,gc,xim,0,0,0,0,width,height,False);
        XSync(DISPLAY(getApp()),False);
        FXTRACE((150,"RGBPixmap XSHM detached at memory=%p (%d bytes)\n",xim->data,xim->bytes_per_line*xim->height));
        XShmDetach(DISPLAY(getApp()),&shminfo);
        xim->data=NULL;
        XDestroyImage(xim);
        shmdt(shminfo.shmaddr);
        shmctl(shminfo.shmid,IPC_RMID,0);
        }
#endif

      // Transfer the image old way
      if(!shmi){
        XPutImage(DISPLAY(getApp()),xid,gc,xim,0,0,0,0,width,height);
        FXFREE(&xim->data);
        XDestroyImage(xim);
        }
      XFreeGC(DISPLAY(getApp()),gc);
      }
    }
  }


#else


void FXImage::render(){
  if(xid){
    register FXuint bytes_per_line,skip;
    register FXuchar *src,*dst;
    register FXint h,w;
    BITMAPINFO bmi;
    FXuchar *pixels;
    HDC hdcmem;

    FXTRACE((100,"%s::render %p\n",getClassName(),this));

    // Check for legal size
    if(width<1 || height<1){ fxerror("%s::render: illegal image size %dx%d.\n",getClassName(),width,height); }


    // Just leave if black if no data
    if(data){

      // Set up the bitmap info
      bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
      bmi.bmiHeader.biWidth=width;
      bmi.bmiHeader.biHeight=height;
      bmi.bmiHeader.biPlanes=1;
      bmi.bmiHeader.biBitCount=24;
      bmi.bmiHeader.biCompression=BI_RGB;
      bmi.bmiHeader.biSizeImage=0;
      bmi.bmiHeader.biXPelsPerMeter=0;
      bmi.bmiHeader.biYPelsPerMeter=0;
      bmi.bmiHeader.biClrUsed=0;
      bmi.bmiHeader.biClrImportant=0;

      // DIB format pads to multiples of 4 bytes...
      bytes_per_line=(width*3+3)&~3;
      skip=bytes_per_line+width*3;
      FXMALLOC(&pixels,FXuchar,bytes_per_line*height);
      h=height-1;
      src=data;
      dst=pixels+h*bytes_per_line;
      do{
        w=width-1;
        do{
          dst[0]=src[2];
          dst[1]=src[1];
          dst[2]=src[0];
          src+=channels;
          dst+=3;
          }
        while(--w>=0);
        dst-=skip;
        }
      while(--h>=0);

      // The MSDN documentation for SetDIBits() states that "the device context
      // identified by the (first) parameter is used only if the DIB_PAL_COLORS
      // constant is set for the (last) parameter". This may be true, but under
      // Win95 you must pass in a non-NULL hdc for the first parameter; otherwise
      // this call to SetDIBits() will fail (in contrast, it works fine under
      // Windows NT if you pass in a NULL hdc).
      hdcmem=::CreateCompatibleDC(NULL);
      if(!SetDIBits(hdcmem,(HBITMAP)xid,0,height,pixels,&bmi,DIB_RGB_COLORS)){
        fxerror("%s::render: unable to render pixels\n",getClassName());
        }
      GdiFlush();
      FXFREE(&pixels);
      ::DeleteDC(hdcmem);
      }
    }
  }

#endif



// Get pixel at x,y
FXColor FXImage::getPixel(FXint x,FXint y) const {
  return (options&IMAGE_ALPHA) ? ((FXColor*)data)[y*width+x] : FXRGB(data[(y*width+x)*3],data[(y*width+x)*3+1],data[(y*width+x)*3+2]);
  }


// Change pixel at x,y
void FXImage::setPixel(FXint x,FXint y,FXColor color){
  if(options&IMAGE_ALPHA){
    ((FXColor*)data)[y*width+x]=color;
    }
  else{
    data[(y*width+x)*3+0]=FXREDVAL(color);
    data[(y*width+x)*3+1]=FXGREENVAL(color);
    data[(y*width+x)*3+2]=FXBLUEVAL(color);
    }
  }


// Resize pixmap to the specified width and height; the data
// array is resized also, but its contents will be undefined.
void FXImage::resize(FXint w,FXint h){
  FXTRACE((100,"%s::resize(%d,%d) %p\n",getClassName(),w,h,this));
  if(w<1) w=1;
  if(h<1) h=1;
  if(width!=w || height!=h){
    if(xid){
#ifndef WIN32
      int dd=visual->getDepth();
      XFreePixmap(DISPLAY(getApp()),xid);
      xid=XCreatePixmap(DISPLAY(getApp()),XDefaultRootWindow(DISPLAY(getApp())),w,h,dd);
      if(!xid){ fxerror("%s::resize: unable to resize image.\n",getClassName()); }
#else
      DeleteObject(xid);
      HDC hdc=::GetDC(GetDesktopWindow());
      xid=CreateCompatibleBitmap(hdc,w,h);
      ::ReleaseDC(GetDesktopWindow(),hdc);
      if(!xid){ fxerror("%s::resize: unable to resize image.\n",getClassName()); }
#endif
      }
    if(data){
      if(!(options&IMAGE_OWNED)){       // Need to own array
        FXMALLOC(&data,FXuchar,w*h*channels);
        options|=IMAGE_OWNED;
        }
      else if(w*h!=width*height){
        FXRESIZE(&data,FXuchar,w*h*channels);
        }
      }
    width=w;
    height=h;
    }
  }


static void hscalergba(FXuchar *dst,const FXuchar* src,FXint dw,FXint dh,FXint sw,FXint ){
  register FXint fin,fout,ar,ag,ab,aa;
  register FXint ss=4*sw;
  register FXint ds=4*dw;
  register FXuchar *end=dst+ds*dh;
  register FXuchar *d;
  register const FXuchar *s;
  do{
    s=src; src+=ss;
    d=dst; dst+=ds;
    fin=dw;
    fout=sw;
    ar=ag=ab=aa=0;
    while(1){
      if(fin<fout){
        ar+=fin*s[0];
        ag+=fin*s[1];
        ab+=fin*s[2];
        aa+=fin*s[3];
        fout-=fin;
        fin=dw;
        s+=4;
        }
      else{
        ar+=fout*s[0]; d[0]=ar/sw; ar=0;
        ag+=fout*s[1]; d[1]=ag/sw; ag=0;
        ab+=fout*s[2]; d[2]=ab/sw; ab=0;
        aa+=fout*s[3]; d[3]=aa/sw; aa=0;
        fin-=fout;
        fout=sw;
        d+=4;
        if(d>=dst) break;
        }
      }
    }
  while(dst<end);
  }


static void hscalergb(FXuchar *dst,const FXuchar* src,FXint dw,FXint dh,FXint sw,FXint){
  register FXint fin,fout,ar,ag,ab;
  register FXint ss=3*sw;
  register FXint ds=3*dw;
  register FXuchar *end=dst+ds*dh;
  register FXuchar *d;
  register const FXuchar *s;
  do{
    s=src; src+=ss;
    d=dst; dst+=ds;
    fin=dw;
    fout=sw;
    ar=ag=ab=0;
    while(1){
      if(fin<fout){
        ar+=fin*s[0];
        ag+=fin*s[1];
        ab+=fin*s[2];
        fout-=fin;
        fin=dw;
        s+=3;
        }
      else{
        ar+=fout*s[0]; d[0]=ar/sw; ar=0;
        ag+=fout*s[1]; d[1]=ag/sw; ag=0;
        ab+=fout*s[2]; d[2]=ab/sw; ab=0;
        fin-=fout;
        fout=sw;
        d+=3;
        if(d>=dst) break;
        }
      }
    }
  while(dst<end);
  }


static void vscalergba(FXuchar *dst,const FXuchar* src,FXint dw,FXint dh,FXint sw,FXint sh){
  register FXint fin,fout,ar,ag,ab,aa;
  register FXint ss=4*sw;
  register FXint ds=4*dw;
  register FXint dss=ds*dh;
  register FXuchar *end=dst+ds;
  register FXuchar *d,*dd;
  register const FXuchar *s;
  do{
    s=src; src+=4;
    d=dst; dst+=4;
    dd=d+dss;
    fin=dh;
    fout=sh;
    ar=ag=ab=aa=0;
    while(1){
      if(fin<fout){
        ar+=fin*s[0];
        ag+=fin*s[1];
        ab+=fin*s[2];
        aa+=fin*s[3];
        fout-=fin;
        fin=dh;
        s+=ss;
        }
      else{
        ar+=fout*s[0]; d[0]=ar/sh; ar=0;
        ag+=fout*s[1]; d[1]=ag/sh; ag=0;
        ab+=fout*s[2]; d[2]=ab/sh; ab=0;
        aa+=fout*s[3]; d[3]=aa/sh; aa=0;
        fin-=fout;
        fout=sh;
        d+=ds;
        if(d>=dd) break;
        }
      }
    }
  while(dst<end);
  }


static void vscalergb(FXuchar *dst,const FXuchar* src,FXint dw,FXint dh,FXint sw,FXint sh){
  register FXint fin,fout,ar,ag,ab;
  register FXint ss=3*sw;
  register FXint ds=3*dw;
  register FXint dss=ds*dh;
  register FXuchar *end=dst+ds;
  register FXuchar *d,*dd;
  register const FXuchar *s;
  do{
    s=src; src+=3;
    d=dst; dst+=3;
    dd=d+dss;
    fin=dh;
    fout=sh;
    ar=ag=ab=0;
    while(1){
      if(fin<fout){
        ar+=fin*s[0];
        ag+=fin*s[1];
        ab+=fin*s[2];
        fout-=fin;
        fin=dh;
        s+=ss;
        }
      else{
        ar+=fout*s[0]; d[0]=ar/sh; ar=0;
        ag+=fout*s[1]; d[1]=ag/sh; ag=0;
        ab+=fout*s[2]; d[2]=ab/sh; ab=0;
        fin-=fout;
        fout=sh;
        d+=ds;
        if(d>=dd) break;
        }
      }
    }
  while(dst<end);
  }


// Resize drawable to the specified width and height
void FXImage::scale(FXint w,FXint h){
  if(w<1) w=1;
  if(h<1) h=1;
  FXTRACE((100,"%s::scale(%d,%d) %p\n",getClassName(),w,h,this));
  if(w!=width || h!=height){
    if(data){
      register FXint ow=width;
      register FXint oh=height;
      FXuchar *interim;

      // Allocate interim buffer
      FXMALLOC(&interim,FXuchar,w*oh*channels);

      // Scale horizontally first, placing result into interim buffer
      if(w==ow){
        memcpy(interim,data,w*oh*channels);
        }
      else if(channels==4){
        hscalergba(interim,data,w,oh,ow,oh);
        }
      else{
        hscalergb(interim,data,w,oh,ow,oh);
        }

      // Resize the pixmap and target buffer
      resize(w,h);

      // Scale vertically from the interim buffer into target buffer
      if(h==oh){
        memcpy(data,interim,w*h*channels);
        }
      else if(channels==4){
        vscalergba(data,interim,w,h,w,oh);
        }
      else{
        vscalergb(data,interim,w,h,w,oh);
        }

      // Free interim buffer
      FXFREE(&interim);
      render();
      }
    else{
      resize(w,h);
      }
    }
  }


// Mirror image horizontally and/or vertically
void FXImage::mirror(FXbool horizontal,FXbool vertical){
  FXTRACE((100,"%s::mirror(%d,%d) %p\n",getClassName(),horizontal,vertical,this));
  if(horizontal || vertical){
    if(data){
      register FXuchar *end,*paa,*pa,*pbb,*pb;
      register FXint nbytes=channels*width;
      register FXuint size=channels*width*height;
      FXuchar *olddata=data;
      FXMALLOC(&data,FXuchar,size);
      if(vertical && height>1){     // Mirror vertically
        end=data+nbytes*height;
        paa=data;
        pbb=olddata+nbytes*(height-1);
        if(channels==4){
          do{
            pa=paa; paa+=nbytes;
            pb=pbb; pbb-=nbytes;
            do{
              *((FXColor*)pa)=*((FXColor*)pb);
              pa+=4;
              pb+=4;
              }
            while(pa<paa);
            }
          while(paa<end);
          }
        else{
          do{
            pa=paa; paa+=nbytes;
            pb=pbb; pbb-=nbytes;
            do{
              pa[0]=pb[0]; pa[1]=pb[1]; pa[2]=pb[2];
              pa+=3;
              pb+=3;
              }
            while(pa<paa);
            }
          while(paa<end);
          }
        }
      if(horizontal && width>1){    // Mirror horizontally
        end=data+nbytes*height;
        paa=data;
        pbb=olddata;
        if(channels==4){
          do{
            pa=paa; paa+=nbytes;
            pbb+=nbytes; pb=pbb;
            do{
              pb-=4;
              *((FXColor*)pa)=*((FXColor*)pb);
              pa+=4;
              }
            while(pa<paa);
            }
          while(paa<end);
          }
        else{
          do{
            pa=paa; paa+=nbytes;
            pbb+=nbytes; pb=pbb;
            do{
              pb-=3;
              pa[0]=pb[0]; pa[1]=pb[1]; pa[2]=pb[2];
              pa+=3;
              }
            while(pa<paa);
            }
          while(paa<end);
          }
        }
      if(options&IMAGE_OWNED){FXFREE(&olddata);}
      options|=IMAGE_OWNED;
      render();
      }
    }
  }



// Rotate image by degrees ccw
void FXImage::rotate(FXint degrees){
  FXTRACE((100,"%s::rotate(%d) %p\n",getClassName(),degrees,this));
  degrees=(degrees+360)%360;
  if(degrees!=0 && width>1 && height>1){
    if(data){
      register FXuchar *paa,*pbb,*end,*pa,*pb;
      register FXint size=channels*width*height;
      register FXint nbytesa;
      register FXint nbytesb;
      FXuchar *olddata;
      FXMALLOC(&olddata,FXuchar,size);
      memcpy(olddata,data,size);
      switch(degrees){
        case 90:
          resize(height,width);
          nbytesa=channels*width;
          nbytesb=channels*height;
          paa=data;
          pbb=olddata+channels*(height-1);
          end=data+size;
          if(channels==4){
            do{
              pa=paa; paa+=nbytesa;
              pb=pbb; pbb-=4;
              do{
                *((FXColor*)pa)=*((FXColor*)pb);
                pa+=4;
                pb+=nbytesb;
                }
              while(pa<paa);
              }
            while(paa<end);
            }
          else{
            do{
              pa=paa; paa+=nbytesa;
              pb=pbb; pbb-=3;
              do{
                pa[0]=pb[0]; pa[1]=pb[1]; pa[2]=pb[2];
                pa+=3;
                pb+=nbytesb;
                }
              while(pa<paa);
              }
            while(paa<end);
            }
          break;
        case 180:
          resize(width,height);
          nbytesa=channels*width;
          nbytesb=channels*width;
          paa=data;
          pbb=olddata+size;
          end=data+size;
          if(channels==4){
            do{
              pa=paa; paa+=nbytesa;
              pb=pbb; pbb-=nbytesb;
              do{
                pb-=4;
                *((FXColor*)pa)=*((FXColor*)pb);
                pa+=4;
                }
              while(pa<paa);
              }
            while(paa<end);
            }
          else{
            do{
              pa=paa; paa+=nbytesa;
              pb=pbb; pbb-=nbytesb;
              do{
                pb-=3;
                pa[0]=pb[0]; pa[1]=pb[1]; pa[2]=pb[2];
                pa+=3;
                }
              while(pa<paa);
              }
            while(paa<end);
            }
          break;
        case 270:
          resize(height,width);
          nbytesa=channels*width;
          nbytesb=channels*height;
          paa=data;
          pbb=olddata+nbytesb*(width-1);
          end=data+size;
          if(channels==4){
            do{
              pa=paa; paa+=nbytesa;
              pb=pbb; pbb+=4;
              do{
                *((FXColor*)pa)=*((FXColor*)pb);
                pa+=4;
                pb-=nbytesb;
                }
              while(pa<paa);
              }
            while(paa<end);
            }
          else{
            do{
              pa=paa; paa+=nbytesa;
              pb=pbb; pbb+=3;
              do{
                pa[0]=pb[0]; pa[1]=pb[1]; pa[2]=pb[2];
                pa+=3;
                pb-=nbytesb;
                }
              while(pa<paa);
              }
            while(paa<end);
            }
          break;
        default:
          fxwarning("%s::rotate: rotation by %d degrees not implemented.\n",getClassName(),degrees);
          break;
        }
      FXFREE(&olddata);
      render();
      }
    else{
      switch(degrees){
        case 90:
          resize(height,width);
          break;
        case 180:
          resize(width,height);
          break;
        case 270:
          resize(height,width);
          break;
        default:
          fxwarning("%s::rotate: rotation by %d degrees not implemented.\n",getClassName(),degrees);
          break;
        }
      }
    }
  }

// FXColor ____blend(FXColor fg,FXColor bg){
//   register FXuint r,g,b,s,t,tmp;
//   s=FXALPHAVAL(fg);
//   t=~s;
//   tmp=FXREDVAL(fg)*s+FXREDVAL(bg)*t+127;     r=(tmp+(tmp>>8))>>8;
//   tmp=FXGREENVAL(fg)*s+FXGREENVAL(bg)*t+127; g=(tmp+(tmp>>8))>>8;
//   tmp=FXBLUEVAL(fg)*s+FXBLUEVAL(bg)*t+127;   b=(tmp+(tmp>>8))>>8;
//   return FXRGB(r,g,b);
//   }

// Crop image to given rectangle
void FXImage::crop(FXint x,FXint y,FXint w,FXint h){
  if(w<1) w=1;
  if(h<1) h=1;
  if(x<0 || y<0 || x+w>width || y+h>height){ fxerror("%s::crop: rectangle outside of image.\n",getClassName()); }
  if(data){
    register FXuchar *paa,*pbb,*end,*pa,*pb;
    register FXint size=channels*width*height;
    register FXint nbytesa;
    register FXint nbytesb;
    FXuchar *olddata;
    FXMALLOC(&olddata,FXuchar,size);
    memcpy(olddata,data,size);
    nbytesa=channels*w;
    nbytesb=channels*width;
    pbb=olddata+nbytesb*y+channels*x;
    resize(w,h);
    paa=data;
    end=data+channels*w*h;
    if(channels==4){
      do{
        pa=paa; paa+=nbytesa;
        pb=pbb; pbb+=nbytesb;
        do{
          *((FXColor*)pa)=*((FXColor*)pb);
          pa+=4;
          pb+=4;
          }
        while(pa<paa);
        }
      while(paa<end);
      }
    else{
      do{
        pa=paa; paa+=nbytesa;
        pb=pbb; pbb+=nbytesb;
        do{
          pa[0]=pb[0]; pa[1]=pb[1]; pa[2]=pb[2];
          pa+=3;
          pb+=3;
          }
        while(pa<paa);
        }
      while(paa<end);
      }
    FXFREE(&olddata);
    render();
    }
  else{
    resize(w,h);
    }
  }


#ifdef WIN32

// Return the device context; the image already selected into it
FXID FXImage::GetDC() const {
  HDC hdc=::CreateCompatibleDC(NULL);
  SelectObject(hdc,(HBITMAP)xid);
  return hdc;
  }


// Release it (no-op)
int FXImage::ReleaseDC(FXID hdc) const {
  return ::DeleteDC((HDC)hdc);
  }

#endif


// Change options
void FXImage::setOptions(FXuint opts){
  opts&=~IMAGE_OWNED;
  if(opts!=options){
    register FXuchar *pa,*pb,*end;
    FXuchar *olddata;

    // Had no alpha, but now we do
    if((opts&IMAGE_ALPHA)&&!(options&IMAGE_ALPHA)){
      olddata=data;
      FXMALLOC(&data,FXuchar,width*height*4);
      pa=data;
      pb=olddata;
      end=data+width*height*4;
      do{
        pa[0]=pb[0]; pa[1]=pb[1]; pa[2]=pb[2]; pa[3]=255;
        pa+=4;
        pb+=3;
        }
      while(pa<end);
      if(options&IMAGE_OWNED){FXFREE(&olddata);}
      opts|=IMAGE_OWNED;
      channels=4;
      }

    // Had alpha, but now we don't
    else if(!(opts&IMAGE_ALPHA)&&(options&IMAGE_ALPHA)){
      olddata=data;
      FXMALLOC(&data,FXuchar,width*height*3);
      pa=data;
      pb=olddata;
      end=data+width*height*3;
      do{
        pa[0]=pb[0]; pa[1]=pb[1]; pa[2]=pb[2];
        pa+=3;
        pb+=4;
        }
      while(pa<end);
      if(options&IMAGE_OWNED){FXFREE(&olddata);}
      opts|=IMAGE_OWNED;
      channels=3;
      }

    // Set options
    options=opts;
    }
  }


// Save pixel data only
void FXImage::savePixels(FXStream& store) const {
  FXuint size=width*height*channels;
  store.save(data,size);
  }


// Load pixel data only
void FXImage::loadPixels(FXStream& store){
  FXuint size=width*height*channels;
  if(options&IMAGE_OWNED){FXFREE(&data);}
  FXMALLOC(&data,FXuchar,size);
  store.load(data,size);
  options|=IMAGE_OWNED;
  }


// Save data
void FXImage::save(FXStream& store) const {
  FXuchar haspixels=(data!=NULL);
  FXDrawable::save(store);
  store << options;
  store << channels;
  store << haspixels;
  if(haspixels) savePixels(store);
  }


// Load data
void FXImage::load(FXStream& store){
  FXuchar haspixels;
  FXDrawable::load(store);
  store >> options;
  store >> channels;
  store >> haspixels;
  if(haspixels) loadPixels(store);
  }


// Clean up
FXImage::~FXImage(){
  FXTRACE((100,"FXImage::~FXImage %p\n",this));
  destroy();
  if(options&IMAGE_OWNED){FXFREE(&data);}
  data=(FXuchar*)-1;
  }
