/********************************************************************************
*                                                                               *
*                            V i s u a l   C l a s s                            *
*                                                                               *
*********************************************************************************
* Copyright (C) 1999,2002 by Jeroen van der Zijp.   All Rights Reserved.        *
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
* $Id: FXVisual.cpp,v 1.41.4.1 2003/04/30 03:38:37 fox Exp $                     *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxkeys.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXRegistry.h"
#include "FXAccelTable.h"
#include "FXObjectList.h"
#include "FXApp.h"
#include "FXVisual.h"


/*
  Notes:

  - Purpose is to provide the illusion of having True Color on all systems.
  - The visual is what determines how an RGB tuple maps to a color on the device.
  - Thus, the visual also knows the colormap, and perhaps other lookup mechanisms.
  - When a drawable is created, it is created with a certain visual.
  - When painting, graphic contexts are specific for a certain visual; hence,
    the visual should probably keep track of the gc's.
  - FIRST set up pseudo color ramp, then read it back and set up the tables.
    This way, table setup can be reused for read-only colormaps [StaticColor]
    also...
  - We try to match the deepest visual not deeper than the specified depth.
  - For some info on visual setup, see:

	http://www.wolfram.com/~cwikla/articles/txa/visual.html
	http://www.wolfram.com/~cwikla/articles/txa/xcc.1.html
	http://www.wolfram.com/~cwikla/articles/txa/xcc.2.html

  - Freshly constructed FXVisual sets maxcolors to 1000000 in anticipation
    of private colormap. [FXApp however sets the default FXVisual's maximum
    to a lower value as the colormap is shared between lots of apps].
  - Always check for Standard Colormap first [even for default colormap], as
    that (a) makes initialization simpler, and (b) may give us the preferred
    colors to grab on that system [*** Too bad this does not work! ***].
  - Find closest depth to the given depth hint
  - RGB Ordering:

       RGB 111      > | R  G  B
       BGR 000      --+--------
       RBG 110      R | x  4  2
       GBR 001      G |    x  1
       BRG 100      B |       x
       GRB 011

  - Just because I always forget:

      StaticGray   0
      GrayScale    1
      StaticColor  2
      PseudoColor  3
      TrueColor    4
      DirectColor  5

  - SGI Infinite Reality may have up to 12 bits for red, green, blue each!
*/


#define DISPLAY(app) ((Display*)((app)->display))


// Maximum size of the colormap; for high-end graphics systems
// you may want to define HIGHENDGRAPHICS to allow large colormaps
#ifdef HIGHENDGRAPHICS
#define MAX_MAPSIZE 4096
#else
#define MAX_MAPSIZE 256
#endif


/*******************************************************************************/

#ifndef WIN32

// Standard dither kernel
static const FXint dither[16]={
   0*16,  8*16,  2*16, 10*16,
  12*16,  4*16, 14*16,  6*16,
   3*16, 11*16,  1*16,  9*16,
  15*16,  7*16, 13*16,  5*16,
  };

#endif

/*******************************************************************************/


// Object implementation
FXIMPLEMENT(FXVisual,FXId,NULL,0)



// Deserialization
FXVisual::FXVisual(){
  flags=0;
  hint=1;
  depth=0;
  info=NULL;
  numcolors=0;
  numred=0;
  numgreen=0;
  numblue=0;
  maxcolors=1000000;
  type=VISUALTYPE_UNKNOWN;
#ifndef WIN32
  visual=NULL;
  colormap=0;
  freemap=FALSE;
  gc=0;
  scrollgc=0;
#else
  hPalette=NULL;
#endif
  }


// Construct
FXVisual::FXVisual(FXApp* a,FXuint flgs,FXuint d):FXId(a){
  FXTRACE((100,"FXVisual::FXVisual %p\n",this));
  flags=flgs;
  hint=FXMAX(1,d);
  depth=0;
  info=NULL;
  numcolors=0;
  numred=0;
  numgreen=0;
  numblue=0;
  maxcolors=1000000;
  type=VISUALTYPE_UNKNOWN;
#ifndef WIN32
  visual=NULL;
  colormap=0;
  freemap=FALSE;
  gc=0;
  scrollgc=0;
#else
  hPalette=NULL;
  info=NULL;
  pixelformat=0;
#endif
  }


#ifndef WIN32

/*******************************************************************************/

// X11 Internal helper functions


// Make GC for this visual
void* FXVisual::makegc(FXbool gex){
  XGCValues gval;
  FXID drawable;
  GC gg;

  gval.fill_style=FillSolid;
  gval.graphics_exposures=gex;

  // Monochrome gc; create a temporary pixmap of depth 1
  if(flags&VISUAL_MONOCHROME){
    FXTRACE((150,"%s::create: gc for monochrome pixmap\n",getClassName()));
    drawable=XCreatePixmap(DISPLAY(getApp()),XDefaultRootWindow(DISPLAY(getApp())),1,1,1);
    gg=XCreateGC(DISPLAY(getApp()),drawable,GCFillStyle|GCGraphicsExposures,&gval);
    XFreePixmap(DISPLAY(getApp()),drawable);
    }

  // For default visual; this is easy as we already have a matching window
  else if(visual==DefaultVisual(DISPLAY(getApp()),DefaultScreen(DISPLAY(getApp())))){
    FXTRACE((150,"%s::create: gc for default visual\n",getClassName()));
    gg=XCreateGC(DISPLAY(getApp()),XDefaultRootWindow(DISPLAY(getApp())),GCFillStyle|GCGraphicsExposures,&gval);
    }

  // For arbitrary visual; create a temporary pixmap of the same depth as the visual
  else{
    FXTRACE((150,"%s::create: gc for non-default visual\n",getClassName()));
    drawable=XCreatePixmap(DISPLAY(getApp()),XDefaultRootWindow(DISPLAY(getApp())),1,1,depth);
    gg=XCreateGC(DISPLAY(getApp()),drawable,GCFillStyle|GCGraphicsExposures,&gval);
    XFreePixmap(DISPLAY(getApp()),drawable);
    }

  return gg;
  }


// Find shift amount
static inline FXuint findshift(FXPixel mask){
  register FXuint sh=0;
  while(!(mask&(1<<sh))) sh++;
  return sh;
  }


// Get number of bits in n
static inline FXuint findnbits(FXPixel n){
  register FXuint nb=0;
  while(n){nb+=(n&1);n>>=1;}
  return nb;
  }


// Apply gamma correction to an intensity value in [0..max].
static FXuint gamma_adjust(FXdouble gamma,FXuint value,FXuint max){
  register FXdouble x=(FXdouble)value / (FXdouble)max;
  return (FXuint) (((FXdouble)max * pow(x,1.0/gamma))+0.5);
  }


// Apparently, fabs() gives trouble on HP-UX aCC compiler
static inline double fxabs(double a){ return a<0 ? -a : a; }


/*******************************************************************************/


// Setup for true color
void FXVisual::setuptruecolor(){
  register FXuint  redshift,greenshift,blueshift;
  register FXPixel redmask,greenmask,bluemask;
  register FXPixel redmax,greenmax,bluemax;
  register FXuint i,c,mapsize,d,r,g,b;
  register FXdouble gamma;

  // Get gamma
  gamma=getApp()->reg().readRealEntry("SETTINGS","displaygamma",1.0);

  // Get map size
  mapsize=((Visual*)visual)->map_entries;

  // Arrangement of pixels
  redmask=((Visual*)visual)->red_mask;
  greenmask=((Visual*)visual)->green_mask;
  bluemask=((Visual*)visual)->blue_mask;
  redshift=findshift(redmask);
  greenshift=findshift(greenmask);
  blueshift=findshift(bluemask);
  redmax=redmask>>redshift;
  greenmax=greenmask>>greenshift;
  bluemax=bluemask>>blueshift;
  numred=redmax+1;
  numgreen=greenmax+1;
  numblue=bluemax+1;
  numcolors=numred*numgreen*numblue;

  // Make dither tables
  for(d=0; d<16; d++){
    for(i=0; i<256; i++){
      c=gamma_adjust(gamma,i,255);
      r=(redmax*c+dither[d])/255;
      g=(greenmax*c+dither[d])/255;
      b=(bluemax*c+dither[d])/255;
      rpix[d][i]=r << redshift;
      gpix[d][i]=g << greenshift;
      bpix[d][i]=b << blueshift;
      }
    }

  // What did we get
  FXTRACE((150,"True color:\n"));
  FXTRACE((150,"  visual id    = 0x%02x\n",((Visual*)visual)->visualid));
  FXTRACE((150,"  depth        = %d\n",depth));
  FXTRACE((150,"  gamma        = %6f\n",gamma));
  FXTRACE((150,"  map_entries  = %d\n",mapsize));
  FXTRACE((150,"  numcolors    = %d\n",numcolors));
  FXTRACE((150,"  BitOrder     = %s\n",(BitmapBitOrder(DISPLAY(getApp()))==MSBFirst)?"MSBFirst":"LSBFirst"));
  FXTRACE((150,"  ByteOrder    = %s\n",(ImageByteOrder(DISPLAY(getApp()))==MSBFirst)?"MSBFirst":"LSBFirst"));
  FXTRACE((150,"  Padding      = %d\n",BitmapPad(DISPLAY(getApp()))));
  FXTRACE((150,"  redmax       = %3d; redmask   =%08x; redshift   = %-2d\n",redmax,redmask,redshift));
  FXTRACE((150,"  greenmax     = %3d; greenmask =%08x; greenshift = %-2d\n",greenmax,greenmask,greenshift));
  FXTRACE((150,"  bluemax      = %3d; bluemask  =%08x; blueshift  = %-2d\n",bluemax,bluemask,blueshift));

  // Set type
  type=VISUALTYPE_TRUE;
  }


/*******************************************************************************/


// Setup direct color
void FXVisual::setupdirectcolor(){
  register FXuint  redshift,greenshift,blueshift;
  register FXPixel redmask,greenmask,bluemask;
  register FXPixel redmax,greenmax,bluemax;
  register FXuint  mapsize,maxcols,i,j,r,g,b,emax,rr,gg,bb,rm,gm,bm,em,d;
  register FXuint  bestmatchr,bestmatchg,bestmatchb;
  register FXdouble mindist,dist,gamma;
  register FXbool gottable,allocedcolor;
  XColor *table,color;
  FXPixel *alloced;

  // Get gamma
  gamma=getApp()->reg().readRealEntry("SETTINGS","displaygamma",1.0);

  // Get map size
  mapsize=((Visual*)visual)->map_entries;

  // Arrangement of pixels
  redmask=((Visual*)visual)->red_mask;
  greenmask=((Visual*)visual)->green_mask;
  bluemask=((Visual*)visual)->blue_mask;
  redshift=findshift(redmask);
  greenshift=findshift(greenmask);
  blueshift=findshift(bluemask);
  redmax=redmask>>redshift;
  greenmax=greenmask>>greenshift;
  bluemax=bluemask>>blueshift;

  rm=redmax;
  gm=greenmax;
  bm=bluemax;
  em=FXMAX3(rm,gm,bm);

  // Maximum number of colors to allocate
  maxcols=FXMIN(maxcolors,mapsize);

  // No more allocations than allowed
  if(redmax>=maxcols) redmax=maxcols-1;
  if(greenmax>=maxcols) greenmax=maxcols-1;
  if(bluemax>=maxcols) bluemax=maxcols-1;

  numred=redmax+1;
  numgreen=greenmax+1;
  numblue=bluemax+1;
  numcolors=numred*numgreen*numblue;
  emax=FXMAX3(redmax,greenmax,bluemax);

  gottable=0;

  // Allocate color table
  FXMALLOC(&table,XColor,mapsize);
  FXMALLOC(&alloced,FXPixel,mapsize);

  // Allocate ramp
  for(i=r=g=b=0; i<=emax; i++){

    // We try to get gamma-corrected colors
    color.red=gamma_adjust(gamma,(r*65535)/redmax,65535);
    color.green=gamma_adjust(gamma,(g*65535)/greenmax,65535);
    color.blue=gamma_adjust(gamma,(b*65535)/bluemax,65535);
    color.flags=DoRed|DoGreen|DoBlue;

    // First try just using XAllocColor
    allocedcolor=XAllocColor(DISPLAY(getApp()),colormap,&color);
    if(!allocedcolor){

      // Get colors in the map
      if(!gottable){
        rr=0;
        gg=0;
        bb=0;
        for(j=0; j<mapsize; j++){
          table[j].pixel=(rr<<redshift) | (gg<<greenshift) | (bb<<blueshift);
          table[j].flags=DoRed|DoGreen|DoBlue;
          if(rr<redmax) rr++;
          if(gg<greenmax) gg++;
          if(bb<bluemax) bb++;
          }
        XQueryColors(DISPLAY(getApp()),colormap,table,mapsize);
        gottable=1;
        }

      // Find best match for red
      for(mindist=1.0E10,bestmatchr=0,j=0; j<mapsize; j++){
        dist=fxabs(color.red-table[j].red);
        if(dist<mindist){ bestmatchr=j; mindist=dist; if(mindist==0.0) break; }
        }

      // Find best match for green
      for(mindist=1.0E10,bestmatchg=0,j=0; j<mapsize; j++){
        dist=fxabs(color.green-table[j].green);
        if(dist<mindist){ bestmatchg=j; mindist=dist; if(mindist==0.0) break; }
        }

      // Find best match for blue
      for(mindist=1.0E10,bestmatchb=0,j=0; j<mapsize; j++){
        dist=fxabs(color.blue-table[j].blue);
        if(dist<mindist){ bestmatchb=j; mindist=dist; if(mindist==0.0) break; }
        }

      // Now try to allocate this color
      color.red=table[bestmatchr].red;
      color.green=table[bestmatchg].green;
      color.blue=table[bestmatchb].blue;

      // Try to allocate the closest match color.  This should only
      // fail if the cell is read/write.  Otherwise, we're incrementing
      // the cell's reference count.
      allocedcolor=XAllocColor(DISPLAY(getApp()),colormap,&color);
      if(!allocedcolor){
        color.red=table[bestmatchr].red;
        color.green=table[bestmatchg].green;
        color.blue=table[bestmatchb].blue;
        color.pixel=(table[bestmatchr].pixel&redmask) | (table[bestmatchg].pixel&greenmask) | (table[bestmatchb].pixel&bluemask);
        }
      }

    FXTRACE((200,"Alloc %3d %3d %3d (%6d %6d %6d) pixel=%08x\n",r,g,b,color.red,color.green,color.blue,color.pixel));

    alloced[i]=color.pixel;

    if(r<redmax) r++;
    if(g<greenmax) g++;
    if(b<bluemax) b++;
    }

  // Fill dither tables
  for(d=0; d<16; d++){
    for(i=0; i<256; i++){
      rpix[d][i]=alloced[((redmax*i+dither[d])/255)]&redmask;
      gpix[d][i]=alloced[((greenmax*i+dither[d])/255)]&greenmask;
      bpix[d][i]=alloced[((bluemax*i+dither[d])/255)]&bluemask;
      }
    }

  // Free table
  FXFREE(&table);
  FXFREE(&alloced);

  // What did we get
  FXTRACE((150,"Direct color:\n"));
  FXTRACE((150,"  visual id    = 0x%02x\n",((Visual*)visual)->visualid));
  FXTRACE((150,"  depth        = %d\n",depth));
  FXTRACE((150,"  gamma        = %6f\n",gamma));
  FXTRACE((150,"  map_entries  = %d\n",mapsize));
  FXTRACE((150,"  numcolors    = %d\n",numcolors));
  FXTRACE((150,"  redmax       = %3d; redmask   =%08x; redshift   = %-2d\n",redmax,redmask,redshift));
  FXTRACE((150,"  greenmax     = %3d; greenmask =%08x; greenshift = %-2d\n",greenmax,greenmask,greenshift));
  FXTRACE((150,"  bluemax      = %3d; bluemask  =%08x; blueshift  = %-2d\n",bluemax,bluemask,blueshift));

  // Set type
  type=VISUALTYPE_TRUE;
  }



/*******************************************************************************/


// Setup for pseudo color
void FXVisual::setuppseudocolor(){
  register FXuint r,g,b,mapsize,bestmatch,maxcols,i,d;
  register FXdouble mindist,dist,gamma,dr,dg,db;
  register FXPixel redmax,greenmax,bluemax;
  register FXbool gottable,allocedcolor;
  XColor table[256],color;

  // Get gamma
  gamma=getApp()->reg().readRealEntry("SETTINGS","displaygamma",1.0);

  // Get map size
  mapsize=((Visual*)visual)->map_entries;
  if(mapsize>256) mapsize=256;

  // How many colors to allocate
  maxcols=FXMIN(maxcolors,mapsize);

  // Find a product of r*g*b which will fit the available map.
  // We prefer b+1>=g and g>=r>=b; start with 6x7x6 or 252 colors.
  numred=6;
  numgreen=7;
  numblue=6;
  while(numred*numgreen*numblue>maxcols){
    if(numblue==numred && numblue==numgreen) numblue--;
    else if(numred==numgreen) numred--;
    else numgreen--;
    }

  // We want at most maxcols colors
  numcolors=numred*numgreen*numblue;
  redmax=numred-1;
  greenmax=numgreen-1;
  bluemax=numblue-1;
  gottable=0;

  // Allocate color ramp
  for(r=0; r<numred; r++){
    for(g=0; g<numgreen; g++){
      for(b=0; b<numblue; b++){

        // We try to get gamma-corrected colors
        color.red=gamma_adjust(gamma,(r*65535)/redmax,65535);
        color.green=gamma_adjust(gamma,(g*65535)/greenmax,65535);
        color.blue=gamma_adjust(gamma,(b*65535)/bluemax,65535);
        color.flags=DoRed|DoGreen|DoBlue;

        // First try just using XAllocColor
        allocedcolor=XAllocColor(DISPLAY(getApp()),colormap,&color);
        if(!allocedcolor){

          // Get colors in the map
          if(!gottable){
            for(i=0; i<mapsize; i++){
              table[i].pixel=i;
              table[i].flags=DoRed|DoGreen|DoBlue;
              }
            XQueryColors(DISPLAY(getApp()),colormap,table,mapsize);
            gottable=1;
            }

          // Find best match
          for(mindist=1.0E10,bestmatch=0,i=0; i<mapsize; i++){
            dr=color.red-table[i].red;
            dg=color.green-table[i].green;
            db=color.blue-table[i].blue;
            dist=dr*dr+dg*dg+db*db;
            if(dist<mindist){
              bestmatch=i;
              mindist=dist;
              if(mindist==0.0) break;
              }
            }

          // Return result
          color.red=table[bestmatch].red;
          color.green=table[bestmatch].green;
          color.blue=table[bestmatch].blue;

          // Try to allocate the closest match color.  This should only
          // fail if the cell is read/write.  Otherwise, we're incrementing
          // the cell's reference count.
          allocedcolor=XAllocColor(DISPLAY(getApp()),colormap,&color);

          // Cell was read/write; we can't use read/write cells as some
          // other app might change our colors and mess up the display.
          // However, rumor has it that some X terminals and the Solaris
          // X server have XAllocColor fail even if we're asking for a
          // color which is known to be in the table; so we'll use this
          // color anyway and hope nobody changes it..
          if(!allocedcolor){
            color.pixel=bestmatch;
            color.red=table[bestmatch].red;
            color.green=table[bestmatch].green;
            color.blue=table[bestmatch].blue;
            }
          }

        // Remember this color
        lut[(r*numgreen+g)*numblue+b]=color.pixel;
        }
      }
    }

  // Set up dither table
  for(d=0; d<16; d++){
    for(i=0; i<256; i++){
      r=(redmax*i+dither[d])/255;
      g=(greenmax*i+dither[d])/255;
      b=(bluemax*i+dither[d])/255;
      rpix[d][i]=r*numgreen*numblue;
      gpix[d][i]=g*numblue;
      bpix[d][i]=b;
      }
    }

  // What did we get
  FXTRACE((150,"Pseudo color display:\n"));
  FXTRACE((150,"  visual id    = 0x%02x\n",((Visual*)visual)->visualid));
  FXTRACE((150,"  depth        = %d\n",depth));
  FXTRACE((150,"  gamma        = %6f\n",gamma));
  FXTRACE((150,"  map_entries  = %d\n",mapsize));
  FXTRACE((150,"  numcolors    = %d\n",numcolors));
  FXTRACE((150,"  redmax       = %d\n",redmax));
  FXTRACE((150,"  greenmax     = %d\n",greenmax));
  FXTRACE((150,"  bluemax      = %d\n",bluemax));

  // Set type
  type=VISUALTYPE_INDEX;
  }


/*******************************************************************************/


// Setup for static color
void FXVisual::setupstaticcolor(){
  register FXuint mapsize,bestmatch,i,nr,ng,nb,r,g,b,j,d;
  register FXdouble mindist,dist,gamma,dr,dg,db;
  register FXPixel redmax,greenmax,bluemax;
  FXbool rc[256],gc[256],bc[256];
  XColor table[256],color;

  // Get gamma
  gamma=getApp()->reg().readRealEntry("SETTINGS","displaygamma",1.0);
  mapsize=((Visual*)visual)->map_entries;
  if(mapsize>256) mapsize=256;

  // Read back table
  for(i=0; i<mapsize; i++) table[i].pixel=i;
  XQueryColors(DISPLAY(getApp()),colormap,table,mapsize);

  // How many shades of r,g,b do we have?
  for(i=0; i<256; i++){ rc[i]=gc[i]=bc[i]=0; }
  for(i=0; i<mapsize; i++){
    rc[table[i].red/257]=1;
    gc[table[i].green/257]=1;
    bc[table[i].blue/257]=1;
    }
  nr=ng=nb=0;
  for(i=0; i<256; i++){
    if(rc[i]) nr++;
    if(gc[i]) ng++;
    if(bc[i]) nb++;
    }
  FXTRACE((200,"nr=%3d ng=%3d nb=%3d\n",nr,ng,nb));

  // Limit to a reasonable table size
  if(nr*ng*nb>4096){
    numred=16;
    numgreen=16;
    numblue=16;
    }
  else{
    numred=nr;
    numgreen=ng;
    numblue=nb;
    }

  numcolors=numred*numgreen*numblue;
  redmax=numred-1;
  greenmax=numgreen-1;
  bluemax=numblue-1;

  // Allocate color ramp
  for(r=0; r<numred; r++){
    for(g=0; g<numgreen; g++){
      for(b=0; b<numblue; b++){

        // Color to get
        color.red=gamma_adjust(gamma,(r*65535)/redmax,65535);
        color.green=gamma_adjust(gamma,(g*65535)/greenmax,65535);
        color.blue=gamma_adjust(gamma,(b*65535)/bluemax,65535);

        // Find best match
        for(mindist=1.0E10,bestmatch=0,j=0; j<mapsize; j++){
          dr=(color.red-table[j].red);
          dg=(color.green-table[j].green);
          db=(color.blue-table[j].blue);
          dist=dr*dr+dg*dg+db*db;
          if(dist<mindist){
            bestmatch=j;
            mindist=dist;
            if(mindist==0.0) break;
            }
          }

        // Add color into table
        lut[(r*numgreen+g)*numblue+b]=table[bestmatch].pixel;
        }
      }
    }

  // Set up dither table
  for(d=0; d<16; d++){
    for(i=0; i<256; i++){
      r=(redmax*i+dither[d])/255;
      g=(greenmax*i+dither[d])/255;
      b=(bluemax*i+dither[d])/255;
      rpix[d][i]=r*numgreen*numblue;
      gpix[d][i]=g*numblue;
      bpix[d][i]=b;
      }
    }

  // What did we get
  FXTRACE((150,"Static color:\n"));
  FXTRACE((150,"  visual id    = 0x%02x\n",((Visual*)visual)->visualid));
  FXTRACE((150,"  depth        = %d\n",depth));
  FXTRACE((150,"  gamma        = %6f\n",gamma));
  FXTRACE((150,"  map_entries  = %d\n",mapsize));
  FXTRACE((150,"  numcolors    = %d\n",numcolors));
  FXTRACE((150,"  redmax       = %d\n",redmax));
  FXTRACE((150,"  greenmax     = %d\n",greenmax));
  FXTRACE((150,"  bluemax      = %d\n",bluemax));

  // Set type
  type=VISUALTYPE_INDEX;
  }


/*******************************************************************************/


// Setup for gray scale
void FXVisual::setupgrayscale(){
  register FXuint g,bestmatch,mapsize,maxcols,graymax,i,d;
  register FXdouble mindist,dist,gamma,dr,dg,db;
  register FXbool gottable,allocedcolor;
  XColor table[256],color;
  FXPixel alloced[256];

  // Get gamma
  gamma=getApp()->reg().readRealEntry("SETTINGS","displaygamma",1.0);

  // Get map size
  mapsize=((Visual*)visual)->map_entries;
  if(mapsize>256) mapsize=256;

  // How many to allocate
  maxcols=FXMIN(mapsize,maxcolors);

  // Colors
  numcolors=maxcols;
  graymax=numcolors-1;
  gottable=0;

  // Allocate gray ramp
  for(g=0; g<numcolors; g++){

    // We try to allocate gamma-corrected colors!
    color.red=color.green=color.blue=gamma_adjust(gamma,(g*65535)/graymax,65535);
    color.flags=DoRed|DoGreen|DoBlue;

    // First try just using XAllocColor
    allocedcolor=XAllocColor(DISPLAY(getApp()),colormap,&color);
    if(!allocedcolor){

      // Get colors in the map
      if(!gottable){
        for(i=0; i<mapsize; i++){
          table[i].pixel=i;
          table[i].flags=DoRed|DoGreen|DoBlue;
          }
        XQueryColors(DISPLAY(getApp()),colormap,table,mapsize);
        gottable=1;
        }

      // Find best match
      for(mindist=1.0E10,bestmatch=0,i=0; i<mapsize; i++){
        dr=color.red-table[i].red;
        dg=color.green-table[i].green;
        db=color.blue-table[i].blue;
        dist=dr*dr+dg*dg+db*db;
        if(dist<mindist){
          bestmatch=i;
          mindist=dist;
          if(mindist==0.0) break;
          }
        }

      // Return result
      color.red=table[bestmatch].red;
      color.green=table[bestmatch].green;
      color.blue=table[bestmatch].blue;

      // Try to allocate the closest match color.  This should only
      // fail if the cell is read/write.  Otherwise, we're incrementing
      // the cell's reference count.
      allocedcolor=XAllocColor(DISPLAY(getApp()),colormap,&color);

      // Cell was read/write; we can't use read/write cells as some
      // other app might change our colors and mess up the display.
      // However, rumor has it that some X terminals and the Solaris
      // X server have XAllocColor fail even if we're asking for a
      // color which is known to be in the table; so we'll use this
      // color anyway and hope nobody changes it..
      if(!allocedcolor){
        color.pixel=bestmatch;
        color.red=table[bestmatch].red;
        color.green=table[bestmatch].green;
        color.blue=table[bestmatch].blue;
        }
      }

    // Keep track
    alloced[g]=color.pixel;
    }

  // Set up color ramps
  for(d=0; d<16; d++){
    for(i=0; i<256; i++){
      rpix[d][i]=gpix[d][i]=bpix[d][i]=alloced[(graymax*i+dither[d])/255];
      }
    }

  // What did we get
  FXTRACE((150,"Gray Scale:\n"));
  FXTRACE((150,"  visual id    = 0x%02x\n",((Visual*)visual)->visualid));
  FXTRACE((150,"  depth        = %d\n",depth));
  FXTRACE((150,"  gamma        = %6f\n",gamma));
  FXTRACE((150,"  map_entries  = %d\n",mapsize));
  FXTRACE((150,"  numcolors    = %d\n",numcolors));
  FXTRACE((150,"  graymax      = %d\n",graymax));

  // Set type
  type=VISUALTYPE_GRAY;
  }


/*******************************************************************************/


// Setup for static gray
void FXVisual::setupstaticgray(){
  register FXuint i,d,c,graymax;
  register FXdouble gamma;

  // Get gamma
  gamma=getApp()->reg().readRealEntry("SETTINGS","displaygamma",1.0);

  // Number of colors
  numcolors=((Visual*)visual)->map_entries;
  graymax=(numcolors-1);

  // Set up color ramps
  for(d=0; d<16; d++){
    for(i=0; i<256; i++){
      c=gamma_adjust(gamma,i,255);
      rpix[d][i]=gpix[d][i]=bpix[d][i]=(graymax*c+dither[d])/255;
      }
    }

  // What did we get
  FXTRACE((150,"Static Gray:\n"));
  FXTRACE((150,"  visual id    = 0x%02x\n",((Visual*)visual)->visualid));
  FXTRACE((150,"  depth        = %d\n",depth));
  FXTRACE((150,"  gamma        = %6f\n",gamma));
  FXTRACE((150,"  map_entries  = %d\n",((Visual*)visual)->map_entries));
  FXTRACE((150,"  numcolors    = %d\n",numcolors));
  FXTRACE((150,"  graymax      = %d\n",graymax));

  type=VISUALTYPE_GRAY;
  }


/*******************************************************************************/


// Setup for pixmap monochrome; this one has no colormap!
void FXVisual::setuppixmapmono(){
  register FXuint d,i,c;
  register FXdouble gamma;

  // Get gamma
  gamma=getApp()->reg().readRealEntry("SETTINGS","displaygamma",1.0);

  // Number of colors
  numcolors=2;

  // Set up color ramps
  for(d=0; d<16; d++){
    for(i=0; i<256; i++){
      c=gamma_adjust(gamma,i,255);
      rpix[d][i]=gpix[d][i]=bpix[d][i]=(c+dither[d])/255;
      }
    }

  // What did we get
  FXTRACE((150,"Pixmap monochrome:\n"));
  FXTRACE((150,"  depth        = %d\n",depth));
  FXTRACE((150,"  gamma        = %6f\n",gamma));
  FXTRACE((150,"  map_entries  = %d\n",2));
  FXTRACE((150,"  numcolors    = %d\n",2));
  FXTRACE((150,"  black        = 0\n"));
  FXTRACE((150,"  white        = 1\n"));

  // Set type
  type=VISUALTYPE_MONO;
  }



/*******************************************************************************/

// Try determine standard colormap
static FXbool getstdcolormap(Display *dpy,VisualID visualid,XStandardColormap& map){
  XStandardColormap *stdmaps=NULL;
  int status,count,i;
  status=XGetRGBColormaps(dpy,RootWindow(dpy,DefaultScreen(dpy)),&stdmaps,&count,XA_RGB_DEFAULT_MAP);
  if(status){
    status=FALSE;
    for(i=0; i<count; i++){
      FXTRACE((150,"Standarn XA_RGB_DEFAULT_MAP map #%d:\n",i));
      FXTRACE((150,"  colormap   = %d\n",stdmaps[i].colormap));
      FXTRACE((150,"  red_max    = %d  red_mult   = %d\n",stdmaps[i].red_max,stdmaps[i].red_mult));
      FXTRACE((150,"  green_max  = %d  green_mult = %d\n",stdmaps[i].green_max,stdmaps[i].green_mult));
      FXTRACE((150,"  blue_max   = %d  blue_mult  = %d\n",stdmaps[i].blue_max,stdmaps[i].blue_mult));
      FXTRACE((150,"  base pixel = %d\n",stdmaps[i].base_pixel));
      FXTRACE((150,"  visualid   = 0x%02x\n",stdmaps[i].visualid));
      FXTRACE((150,"  killid     = %d\n",stdmaps[i].killid));
      if(stdmaps[i].visualid==visualid){
        FXTRACE((150,"  Matched\n"));
        map=stdmaps[i];
        status=1;
        break;
        }
      }
    }
  if(stdmaps) XFree(stdmaps);
  return status;
  }


// Determine colormap, then initialize it
void FXVisual::setupcolormap(){
  //XStandardColormap stdmap;
  if(flags&VISUAL_MONOCHROME){
    colormap=None;
    FXTRACE((150,"%s::create: need no colormap\n",getClassName()));
    setuppixmapmono();
    }
  else{
    if((flags&VISUAL_OWNCOLORMAP) || (visual!=DefaultVisual(DISPLAY(getApp()),DefaultScreen(DISPLAY(getApp()))))){
      colormap=XCreateColormap(DISPLAY(getApp()),RootWindow(DISPLAY(getApp()),DefaultScreen(DISPLAY(getApp()))),((Visual*)visual),AllocNone);
      FXTRACE((150,"%s::create: allocate colormap\n",getClassName()));
      freemap=TRUE;
      }
    else{
      //getstdcolormap(DISPLAY(getApp()),visual->visualid,stdmap);
      colormap=DefaultColormap(DISPLAY(getApp()),DefaultScreen(DISPLAY(getApp())));
      FXTRACE((150,"%s::create: use default colormap\n",getClassName()));
      }
    switch(((Visual*)visual)->c_class){
      case TrueColor:   setuptruecolor(); break;
      case DirectColor: setupdirectcolor(); break;
      case PseudoColor: setuppseudocolor(); break;
      case StaticColor: setupstaticcolor(); break;
      case GrayScale:   setupgrayscale(); break;
      case StaticGray:  setupstaticgray(); break;
      }
    }
  }

#else

/*******************************************************************************/

/*

// Creates an "all-purpose" palette
// From "Programming Windows", 5th ed. by Charles Petzold
HPALETTE FXVisual::createAllPurposePalette(){
  LOGPALETTE *plp;
  HPALETTE hPalette=NULL;
  if(FXMALLOC(&plp,BYTE,sizeof(LOGPALETTE)+246*sizeof(PALETTEENTRY))){
    plp->palVersion=0x0300;
    plp->palNumEntries=247;

    // Calculate 31 gray shades, 3 of which match the standard 20 colors
    int i,G,incr;
    for(i=0, G=0, incr=8; G<=0xff; i++, G+=incr){
      plp->palPalEntry[i].peRed=(BYTE)G;
      plp->palPalEntry[i].peGreen=(BYTE)G;
      plp->palPalEntry[i].peBlue=(BYTE)G;
      plp->palPalEntry[i].peFlags=0;
      incr=(incr==9 ? 8 : 9);
      }

    // Calculate remaining 216 colors, 8 of which match the standard
    // 20 colors and 4 of which match the gray shades above
    numred=6;
    numgreen=6;
    numblue=6;
    for(int R=0; R<0xff; R+=0x33){
      for(G=0; G<=0xff; G+=0x33){
        for(int B=0; B<=0xff; B+=0x33){
          plp->palPalEntry[i].peRed=(BYTE)R;
          plp->palPalEntry[i].peGreen=(BYTE)G;
          plp->palPalEntry[i].peBlue=(BYTE)B;
          plp->palPalEntry[i].peFlags=0;
          i++;
          }
        }
      }

    // Create palette and we're done
    hPalette=CreatePalette(plp);
    FXFREE(&plp);
    }
  return hPalette;
  }
*/

static FXuchar defSysClr[20][3] = {                         // System colors to match against
    { 0,   0,   0 },
    { 0x80,0,   0 },
    { 0,   0x80,0 },
    { 0x80,0x80,0 },
    { 0,   0,   0x80 },
    { 0x80,0,   0x80 },
    { 0,   0x80,0x80 },
    { 0xC0,0xC0,0xC0 },

    { 192, 220, 192 },
    { 166, 202, 240 },
    { 255, 251, 240 },
    { 160, 160, 164 },

    { 0x80,0x80,0x80 },
    { 0xFF,0,   0 },
    { 0,   0xFF,0 },
    { 0xFF,0xFF,0 },
    { 0,   0,   0xFF },
    { 0xFF,0,   0xFF },
    { 0,   0xFF,0xFF },
    { 0xFF,0xFF,0xFF }
    };


static int defaultOverride[13] = {
  0, 3, 24, 27, 64, 67, 88, 173, 181, 236, 247, 164, 91
  };



// Make palette
FXID FXVisual::createAllPurposePalette(){
  int rr,gg,bb;
  int i,rmax,gmax,bmax;
  LOGPALETTE *pal;
  HPALETTE hPalette=NULL;

  FXMALLOC(&pal,char,sizeof(LOGPALETTE)+sizeof(PALETTEENTRY)*256);
  pal->palVersion = 0x300;
  pal->palNumEntries = 256;

  rmax=7;
  gmax=7;
  bmax=3;

  // Build palette
  for(rr=0; rr<=rmax; rr++){
    for(gg=0; gg<=gmax; gg++){
      for(bb=0; bb<=bmax; bb++){
        i = rr | (gg<<3) | (bb<<6);
        pal->palPalEntry[i].peRed = (255*rr)/rmax;
        pal->palPalEntry[i].peGreen = (255*gg)/gmax;
        pal->palPalEntry[i].peBlue = (255*bb)/bmax;
        pal->palPalEntry[i].peFlags = PC_NOCOLLAPSE;
        }
      }
    }
//for(int i=0; i<256; i++){
//FXTRACE((100,"%3d %3d %3d %3d\n",i,pal->palPalEntry[i].peRed,pal->palPalEntry[i].peGreen,pal->palPalEntry[i].peBlue));
//}
/*
  for(int j=1; j<=12; j++){
    pal->palPalEntry[defaultOverride[j]].peRed = defSysClr[j][0];
    pal->palPalEntry[defaultOverride[j]].peGreen = defSysClr[j][1];
    pal->palPalEntry[defaultOverride[j]].peBlue = defSysClr[j][1];
    pal->palPalEntry[defaultOverride[j]].peFlags = 0;
    }
*/

  hPalette=CreatePalette(pal);
  FXFREE(&pal);
  return hPalette;
  }
#endif


/*******************************************************************************/


// Initialize
void FXVisual::create(){
  if(!xid){
    if(getApp()->initialized){
      FXTRACE((100,"%s::create %p\n",getClassName(),this));
#ifndef WIN32
      XVisualInfo vitemplate;
      XVisualInfo *vi;
      FXint nvi,i,d,dbest;

      // Assume the default
      visual=DefaultVisual(DISPLAY(getApp()),DefaultScreen(DISPLAY(getApp())));
      depth=DefaultDepth(DISPLAY(getApp()),DefaultScreen(DISPLAY(getApp())));

      // True color
      if(flags&VISUAL_TRUECOLOR){
        vitemplate.screen=DefaultScreen(DISPLAY(getApp()));
        vi=XGetVisualInfo(DISPLAY(getApp()),VisualScreenMask,&vitemplate,&nvi);
        if(vi){
          for(i=0,dbest=1000000; i<nvi; i++){
            if((vi[i].c_class==DirectColor) || (vi[i].c_class==TrueColor)){
              d=vi[i].depth-hint;
              if(d<0) d*=-100;         // Strongly prefer >= hint
              if(d<dbest){
                dbest=d;
                visual=vi[i].visual;
                depth=vi[i].depth;
                }
              }
            }
          XFree((char*)vi);
          }
        }

      // Index color
      else if(flags&VISUAL_INDEXCOLOR){
        vitemplate.screen=DefaultScreen(DISPLAY(getApp()));
        vi=XGetVisualInfo(DISPLAY(getApp()),VisualScreenMask,&vitemplate,&nvi);
        if(vi){
          for(i=0,dbest=1000000; i<nvi; i++){
            if((vi[i].c_class==StaticColor) || (vi[i].c_class==PseudoColor)){
              d=vi[i].depth-hint;
              if(d<0) d*=-100;
              if(d<dbest){
                dbest=d;
                visual=vi[i].visual;
                depth=vi[i].depth;
                }
              }
            }
          XFree((char*)vi);
          }
        }

      // Gray scale color
      else if(flags&VISUAL_GRAYSCALE){
        vitemplate.screen=DefaultScreen(DISPLAY(getApp()));
        vi=XGetVisualInfo(DISPLAY(getApp()),VisualScreenMask,&vitemplate,&nvi);
        if(vi){
          for(i=0,dbest=1000000; i<nvi; i++){
            if((vi[i].c_class==GrayScale) || (vi[i].c_class==StaticGray)){
              d=vi[i].depth-hint;
              if(d<0) d*=-100;
              if(d<dbest){
                dbest=d;
                visual=vi[i].visual;
                depth=vi[i].depth;
                }
              }
            }
          XFree((char*)vi);
          }
        }

      // Get the best (deepest) visual
      else if(flags&VISUAL_BEST){
        vitemplate.screen=DefaultScreen(DISPLAY(getApp()));
        vi=XGetVisualInfo(DISPLAY(getApp()),VisualScreenMask,&vitemplate,&nvi);
        if(vi){
          for(i=0,dbest=1000000; i<nvi; i++){
            d=vi[i].depth-hint;
            if(d<0) d*=-100;
            if(d<dbest){
              dbest=d;
              visual=vi[i].visual;
              depth=vi[i].depth;
              }
            }
          XFree((char*)vi);
          }
        }

      // Monochrome visual (for masks and stipples, not for windows)
      else if(flags&VISUAL_MONOCHROME){
        numcolors=2;
        depth=1;
        }

      FXASSERT(visual);

      // Initialize colormap
      setupcolormap();

      // Make GC's for this visual
      gc=makegc(FALSE);
      scrollgc=makegc(TRUE);

      xid=1;
#else

      // Check for palette support
      HDC hdc=GetDC(GetDesktopWindow());
      if((GetDeviceCaps(hdc,RASTERCAPS)&RC_PALETTE)!=0){
        depth=GetDeviceCaps(hdc,COLORRES);
        if(depth<=8){
          hPalette=createAllPurposePalette();   // Palette
          type=VISUALTYPE_INDEX;
          }
        else if(depth==16){
          numred=32;
          numgreen=64;
          numblue=32;
          type=VISUALTYPE_TRUE;
          }
        else if(depth==15){
          numred=32;
          numgreen=32;
          numblue=32;
          type=VISUALTYPE_TRUE;
          }
        else if(depth>=24){
          numred=256;
          numgreen=256;
          numblue=256;
          type=VISUALTYPE_TRUE;
          }
        }
      else{
        int nPlanes=GetDeviceCaps(hdc,PLANES);
        int nBitsPixel=GetDeviceCaps(hdc,BITSPIXEL);
        depth=1<<(nPlanes*nBitsPixel);
        type=VISUALTYPE_UNKNOWN;
        }
      numcolors=numred*numgreen*numblue;
      ReleaseDC(GetDesktopWindow(),hdc);

      // This is just a placeholder
      xid=(void*)1;
#endif
      }
    }
  }


// Detach visual
void FXVisual::detach(){
  if(xid){
    FXTRACE((100,"%s::detach %08x\n",getClassName(),this));
    xid=0;
    }
  }


// Destroy visual
void FXVisual::destroy(){
  if(xid){
    if(getApp()->initialized){
      FXTRACE((100,"%s::destroy %08x\n",getClassName(),this));
#ifndef WIN32
      if(freemap){ XFreeColormap(DISPLAY(getApp()),colormap); }
      XFreeGC(DISPLAY(getApp()),(GC)gc);
      XFreeGC(DISPLAY(getApp()),(GC)scrollgc);
      freemap=FALSE;
#else

      // Delete palette
      if(hPalette){ DeleteObject(hPalette); }
      hPalette=NULL;
#endif
      }
    xid=0;
    }
  }


// Get pixel value for color
FXPixel FXVisual::getPixel(FXColor clr){
#ifndef WIN32
  switch(type){
    case VISUALTYPE_TRUE:    return rpix[1][FXREDVAL(clr)] | gpix[1][FXGREENVAL(clr)] | bpix[1][FXBLUEVAL(clr)];
    case VISUALTYPE_INDEX:   return lut[rpix[1][FXREDVAL(clr)]+gpix[1][FXGREENVAL(clr)]+bpix[1][FXBLUEVAL(clr)]];
    case VISUALTYPE_GRAY:    return gpix[1][(77*FXREDVAL(clr)+151*FXGREENVAL(clr)+29*FXBLUEVAL(clr))>>8];
    case VISUALTYPE_MONO:    return gpix[1][(77*FXREDVAL(clr)+151*FXGREENVAL(clr)+29*FXBLUEVAL(clr))>>8];
    case VISUALTYPE_UNKNOWN: return 0;
    }
  return 0;
#else
  return PALETTERGB(FXREDVAL(clr),FXGREENVAL(clr),FXBLUEVAL(clr));
#endif
  }


// Get color value for pixel
FXColor FXVisual::getColor(FXPixel pix){
#ifndef WIN32
  XColor color;
  color.pixel=pix;
  XQueryColor(DISPLAY(getApp()),colormap,&color);
  return FXRGB((color.red>>8),(color.green>>8),(color.blue>>8));
#else
  return PALETTEINDEX(pix);
#endif
  }


// Set maximum number of colors to allocate
void FXVisual::setMaxColors(FXuint maxcols){
  if(xid){ fxerror("%s::setMaxColors: visual already initialized.\n",getClassName()); }
  if(maxcols<2) maxcols=2;
  maxcolors=maxcols;
  }


// Save to stream
void FXVisual::save(FXStream& store) const {
  FXId::save(store);
  store << flags;
  store << depth;
  }


// Load from stream
void FXVisual::load(FXStream& store){
  FXId::load(store);
  store >> flags;
  store >> depth;
  }


// Destroy
FXVisual::~FXVisual(){
  FXTRACE((100,"FXVisual::~FXVisual %08x\n",this));
  destroy();
  }
