/********************************************************************************
*                                                                               *
*                               F o n t   O b j e c t                           *
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
* $Id: FXFont.cpp,v 1.52 2002/02/12 00:58:29 fox Exp $                          *
********************************************************************************/
#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "fxpriv.h"
#include "FXStream.h"
#include "FXString.h"
#include "FXSize.h"
#include "FXPoint.h"
#include "FXRectangle.h"
#include "FXSettings.h"
#include "FXRegistry.h"
#include "FXApp.h"
#include "FXId.h"
#include "FXFont.h"


/*
  Notes:

  - Interpretation of the hints:

      FONTPITCH_DEFAULT     No preference for pitch
      FONTPITCH_FIXED       If specified, match for fixed pitch fonts are strongly preferred
      FONTPITCH_VARIABLE    If specified, match for variable pitch font strongly preferred

      FONTHINT_DONTCARE     No hints given
      FONTHINT_DECORATIVE   Ye Olde Fonte
      FONTHINT_MODERN       Monospace fonts such as courier and so on
      FONTHINT_ROMAN        Serif font such as times
      FONTHINT_SCRIPT       Cursive font/script
      FONTHINT_SWISS        Sans serif font such as swiss, helvetica, arial
      FONTHINT_SYSTEM       Raster based fonts, typically monospaced

      FONTHINT_X11          Set internally to force X11-style font specification for font name

      FONTHINT_SCALABLE     Strong emphasis on scalable fonts; under Windows, this means
                            TrueType fonts are desired

      FONTHINT_POLYMORPHIC  Strong emphasis on polymorphic fonts; under Windows, this means
                            TrueType fonts desired also

  - For font size, the largest font not larger than the indicated size is taken;
    rationale is that larger fonts may make applications too big for the screen, but
    smaller ones are OK.

  - FONTENCODING_DEFAULT means we don't care about the encoding; if another value is
    specified, we'll insist/prefer the encoding must match.

  - FONTSLANT_ITALIC is a cursive typeface for some fonts; FONTSLANT_OBLIQUE is the same
    basic font written at an angle; for many fonts, FONTSLANT_ITALIC and FONTSLANT_OBLIQUE
    means pretty much the same thing.

  - FONTWEIGHT_DONTCARE means we don't care about the typeface weight; if non-null, we'll
    try to get as close as we can.

  - Alternative face names:

      helvetica   arial, swiss
      times       roman
      courier     modern

    [Perhaps we can come up with a few additional equivalents to try before failing].

  - We have hidden TEXTMETRICS or XFontStruct knowlegde from the public interface

  - XFontStruct.ascent+XFontStruct.descent is the height of the font, as far as line
    spacing goes.  XFontStruct.max_bounds.ascent+XFontStruct.max_bounds.descent is
    larger, as some characters can apparently extend beyond ascent or descent!!

  - We should assume success first, i.e. only fall back on complicated matching if
    stuff fails; in many cases, fonts are picked from FXFontSelector and already known
    to exist...

  - Registry section FONTSUBSTITUTIONS can be used to map typeface names to platform
    specific typeface names.

    e.g.
    [FONTSUBSTITUTIONS]
    arial = helvetica
    swiss = helvetica

    To make Windows programs work elsewhere w/o mods to source code...

  - Text txfm matrix [a b c d] premultiplies.

  - Should we perhaps build our own tables of font metrics? This might make
    things simpler for the advanced stuff, and be conceivably a lot faster
    under MS-Windows [no need to SelectObject() all the time just to get some
    info; also, this could be useful in case the drawing surface is not a
    window].

  - FOR THE MOMENT we're creating a dummy DC to keep the font locked into the GDI
    for MUCH quicker access to text metrics.  Soon however we want to just build
    our own font metrics tables and determine the metrics entirely with client-side
    code.  This will [predictably] be the fastest possible method as it will not
    involve context switches...

  - Somehow, create() of fonts assumes its for the screen; how about other
    surfaces?

  - Need some API to make OpenGL fonts out of the 2D fonts...

  - Matching algorithm should favor bitmapped fonts over scalable ones [as the
    latter may not be optimized easthetically; also, the matching algorithm should
    not weight resolution as much.

  - Use PANOSE numbers for matching fonts...

  - UNICODE means registry and encoding are set to iso10646-1

  - More human-readable font strings (e.g. registry):

    family,size,weight,slant,setwidth,encoding,hints

    Example:

    helvetica,12,bold,i,normal,iso8859-1,0


    weight

    "medium",            FONTWEIGHT_MEDIUM
    "bold",              FONTWEIGHT_BOLD
    "demibold",          FONTWEIGHT_DEMIBOLD
    "demi",              FONTWEIGHT_DEMIBOLD
    "black",             FONTWEIGHT_BLACK
    "light",             FONTWEIGHT_LIGHT
    "normal",            FONTWEIGHT_NORMAL
    "regular",           FONTWEIGHT_NORMAL

    setwidth

    "ultracondensed",    FONTSETWIDTH_ULTRACONDENSED
    "extracondensed",    FONTSETWIDTH_EXTRACONDENSED
    "condensed",         FONTSETWIDTH_CONDENSED
    "narrow",            FONTSETWIDTH_CONDENSED
    "compressed",        FONTSETWIDTH_CONDENSED
    "semicondensed",     FONTSETWIDTH_SEMICONDENSED
    "medium",            FONTSETWIDTH_MEDIUM
    "normal",            FONTSETWIDTH_MEDIUM
    "regular",           FONTSETWIDTH_MEDIUM
    "semiexpanded",      FONTSETWIDTH_SEMIEXPANDED
    "demiexpanded",      FONTSETWIDTH_SEMIEXPANDED
    "expanded",          FONTSETWIDTH_EXPANDED
    "wide",              FONTSETWIDTH_EXTRAEXPANDED
    "extraexpanded",     FONTSETWIDTH_EXTRAEXPANDED
    "ultraexpanded",     FONTSETWIDTH_ULTRAEXPANDED

    slant

    "i",                 FONTSLANT_ITALIC
    "o",                 FONTSLANT_OBLIQUE
    "ri",                FONTSLANT_REVERSE_ITALIC
    "ro",                FONTSLANT_REVERSE_OBLIQUE
    "r",                 FONTSLANT_REGULAR

    encoding

    iso-8851-1          FONTENCODING_ISO_8859_1
    usascii             FONTENCODING_USASCII


    o If no exact match, leave as number, e.g. weight=550.
    o leave hints as number.
    o need only specify family, or more if you want.
*/


// X11
#ifndef WIN32


// XLFD Fields
#define XLFD_FOUNDRY      0
#define XLFD_FAMILY       1
#define XLFD_WEIGHT       2
#define XLFD_SLANT        3
#define XLFD_SETWIDTH     4
#define XLFD_ADDSTYLE     5
#define XLFD_PIXELSIZE    6
#define XLFD_POINTSIZE    7
#define XLFD_RESOLUTION_X 8
#define XLFD_RESOLUTION_Y 9
#define XLFD_SPACING      10
#define XLFD_AVERAGE      11
#define XLFD_REGISTRY     12
#define XLFD_ENCODING     13

// Match factors
#define ENCODING_FACTOR   256
#define PITCH_FACTOR      128
#define RESOLUTION_FACTOR 64
#define SCALABLE_FACTOR   32
#define POLY_FACTOR       16
#define SIZE_FACTOR       8
#define WEIGHT_FACTOR     4
#define SLANT_FACTOR      2
#define SETWIDTH_FACTOR   1

// Hint mask
#define FONTHINT_MASK (FONTHINT_DECORATIVE|FONTHINT_MODERN|FONTHINT_ROMAN|FONTHINT_SCRIPT|FONTHINT_SWISS|FONTHINT_SYSTEM)

// Convenience macros
#define DISPLAY(app)      ((Display*)((app)->display))
#define EQUAL1(str,c)     (str[0]==c && str[1]=='\0')
#define EQUAL2(str,c1,c2) (str[0]==c1 && str[1]==c2 && str[2]=='\0')

// A glyph exists for the given character
#define HASCHAR(font,ch)  ((((XFontStruct*)font)->min_char_or_byte2 <= (FXuint)ch) && ((FXuint)ch <= ((XFontStruct*)font)->max_char_or_byte2))

#define FIRSTCHAR(font)   (((XFontStruct*)font)->min_char_or_byte2)
#define LASTCHAR(font)    (((XFontStruct*)font)->max_char_or_byte2)


// MS-Windows
#else

// A glyph exists for the given character
#define HASCHAR(font,ch)  ((((TEXTMETRIC*)font)->tmFirstChar <= (FXuint)ch) && ((FXuint)ch <= ((TEXTMETRIC*)font)->tmLastChar))

#define FIRSTCHAR(font)   (((TEXTMETRIC*)font)->tmFirstChar)
#define LASTCHAR(font)    (((TEXTMETRIC*)font)->tmLastChar)

#endif


/*******************************************************************************/


// Convert text to font weight
static FXuint weightfromtext(const FXchar* text){
  register FXchar c1=tolower((FXuchar)text[0]);
  register FXchar c2=tolower((FXuchar)text[1]);
  if(c1=='l' && c2=='i') return FONTWEIGHT_LIGHT;
  if(c1=='n' && c2=='o') return FONTWEIGHT_NORMAL;
  if(c1=='r' && c2=='e') return FONTWEIGHT_REGULAR;
  if(c1=='m' && c2=='e') return FONTWEIGHT_MEDIUM;
  if(c1=='d' && c2=='e') return FONTWEIGHT_DEMIBOLD;
  if(c1=='b' && c2=='o') return FONTWEIGHT_BOLD;
  if(c1=='b' && c2=='l') return FONTWEIGHT_BLACK;
  return FONTWEIGHT_DONTCARE;
  }


// Convert text to slant
static FXuint slantfromtext(const FXchar* text){
  register FXchar c1=tolower((FXuchar)text[0]);
  register FXchar c2=tolower((FXuchar)text[1]);
  if(c1=='i') return FONTSLANT_ITALIC;
  if(c1=='o') return FONTSLANT_OBLIQUE;
  if(c1=='r' && c2=='i') return FONTSLANT_REVERSE_ITALIC;
  if(c1=='r' && c2=='o') return FONTSLANT_REVERSE_OBLIQUE;
  if(c1=='r') return FONTSLANT_REGULAR;
  return FONTSLANT_DONTCARE;
  }


// Convert text to setwidth
static FXuint setwidthfromtext(const FXchar* text){
  if(text[0]=='m') return FONTSETWIDTH_MEDIUM;
  if(text[0]=='w') return FONTSETWIDTH_EXTRAEXPANDED;
  if(text[0]=='r') return FONTSETWIDTH_MEDIUM;
  if(text[0]=='c') return FONTSETWIDTH_CONDENSED;
  if(text[0]=='n'){
    if(text[1]=='a') return FONTSETWIDTH_CONDENSED;
    if(text[1]=='o') return FONTSETWIDTH_MEDIUM;
    return FONTSETWIDTH_DONTCARE;
    }
  if(text[0]=='e' && text[1]=='x' && text[2]=='p') return FONTSETWIDTH_EXPANDED;
  if(text[0]=='e' && text[1]=='x' && text[2]=='t' && text[3]=='r' && text[4]=='a'){
    if(text[5]=='c') return FONTSETWIDTH_EXTRACONDENSED;
    if(text[5]=='e') return FONTSETWIDTH_EXTRAEXPANDED;
    return FONTSETWIDTH_DONTCARE;
    }
  if(text[0]=='u' && text[1]=='l' && text[2]=='t' && text[3]=='r' && text[4]=='a'){
    if(text[5]=='c') return FONTSETWIDTH_ULTRACONDENSED;
    if(text[5]=='e') return FONTSETWIDTH_ULTRAEXPANDED;
    return FONTSETWIDTH_DONTCARE;
    }
  if((text[0]=='s' || text[0]=='d') && text[1]=='e' && text[2]=='m' && text[3]=='i'){
    if(text[5]=='c') return FONTSETWIDTH_SEMICONDENSED;
    if(text[5]=='e') return FONTSETWIDTH_SEMIEXPANDED;
    return FONTSETWIDTH_DONTCARE;
    }
  return FONTSETWIDTH_DONTCARE;
  }


// Convert pitch to flags
static FXuint pitchfromtext(const FXchar* text){
  register FXchar c=tolower((FXuchar)text[0]);
  if(c=='p') return FONTPITCH_VARIABLE;
  if(c=='m' || c=='c') return FONTPITCH_FIXED;
  return FONTPITCH_DEFAULT;
  }


// Test if font is ISO8859
static FXbool isISO8859(const FXchar* text){
  return tolower((FXuchar)text[0])=='i' && tolower((FXuchar)text[1])=='s' && tolower((FXuchar)text[2])=='o' && text[3]=='8' && text[4]=='8' && text[5]=='5' && text[6]=='9';
  }


// Test if font is KOI8
static FXbool isKOI8(const FXchar* text){
  return tolower((FXuchar)text[0])=='k' && tolower((FXuchar)text[1])=='o' && tolower((FXuchar)text[2])=='i' && text[3]=='8';
  }


// Test if font is multi-byte
static FXbool ismultibyte(const FXchar* text){

  // Unicode font; not yet ...
  if(tolower((FXuchar)text[0])=='i' && tolower((FXuchar)text[1])=='s' && tolower((FXuchar)text[2])=='o' && text[3]=='6' && text[4]=='4' && text[5]=='6') return TRUE;

  // Japanese font
  if(tolower((FXuchar)text[0])=='j' && tolower((FXuchar)text[1])=='i' && tolower((FXuchar)text[2])=='s' && text[3]=='x') return TRUE;

  // Chinese font
  if(tolower((FXuchar)text[0])=='g' && tolower((FXuchar)text[1])=='b') return TRUE;

  // Another type of chinese font
  if(tolower((FXuchar)text[0])=='b' && tolower((FXuchar)text[1])=='i' && tolower((FXuchar)text[2])=='g' && text[3]=='5') return TRUE;

  // Korean
  if(tolower((FXuchar)text[0])=='k' && tolower((FXuchar)text[1])=='s' && tolower((FXuchar)text[2])=='c') return TRUE;

  return FALSE;
  }


/*******************************************************************************/

// Helper functions X11

#ifndef WIN32


// Get list of font names matching pattern
static char **listfontnames(Display *dpy,const char* pattern,int& numfnames){
  int maxfnames=1024;
  char **fnames;
  for(;;){
    fnames=XListFonts(dpy,pattern,maxfnames,&numfnames);
    if((fnames==NULL) || (numfnames<maxfnames)) break;
    XFreeFontNames(fnames);
    maxfnames<<=1;
    }
  return fnames;
  }


// Return number of fonts matching name
static int matchingfonts(Display *dpy,const char *pattern){
  char **fnames;
  int numfnames;
  fnames=listfontnames(dpy,pattern,numfnames);
  XFreeFontNames(fnames);
  if(numfnames>0) FXTRACE((100,"matched: %s\n",pattern));
  return numfnames;
  }


// Parse font name into parts
static int parsefontname(char *fields[],char* fname){
  int f=0;
  if(fname && *fname++ == '-'){
    while(*fname){
      fields[f++]=fname;
      if(f>XLFD_ENCODING) return 1;
      fname=strchr(fname,'-');
      if(!fname) break;
      *fname++='\0';
      }
    }
  return 0;
  }


// Try find matching font
char* FXFont::findmatch(char *fontname,const char* family) const {
  FXchar pattern[300],fname[300],*field[14],**fnames;
  FXint bestf,bestdweight,bestdsize,bestvalue,bestscalable,bestxres,bestyres;
  FXint screenres,xres,yres;
  FXint dweight,w,scalable,polymorphic,dsize;
  FXint numfnames,f,value;
  FXuint sw,sz,sl,pitch,en;

  // Get fonts matching the pattern
  sprintf(pattern,"-*-%s-*-*-*-*-*-*-*-*-*-*-*-*",family);
  fnames=listfontnames(DISPLAY(getApp()),pattern,numfnames);
  if(!fnames) return NULL;

  // Init match values
  bestf=-1;
  bestvalue=0;
  bestdsize=10000000;
  bestdweight=10000000;
  bestscalable=0;
  bestxres=75;
  bestyres=75;
  fname[299]=0;


  // Perhaps override screen resolution via registry
  screenres=getApp()->reg().readUnsignedEntry("SETTINGS","screenres",100);

  // Validate
  if(screenres<50) screenres=50;
  if(screenres>200) screenres=200;

  FXTRACE((150,"Matching Fonts for screenres=%d :\n",screenres));

  // Loop over all fonts to find the best match
  for(f=0; f<numfnames; f++){
    strncpy(fname,fnames[f],299);
    if(parsefontname(field,fname)){

      // This font's match value
      value=0;
      scalable=0;
      polymorphic=0;
      dsize=1000000;
      dweight=1000;

      // Match encoding
      if(encoding==FONTENCODING_DEFAULT){
        if(!ismultibyte(field[XLFD_REGISTRY])) value+=ENCODING_FACTOR;
        }
      else{
        if(isISO8859(field[XLFD_REGISTRY])){
          en=FONTENCODING_ISO_8859_1+atoi(field[XLFD_ENCODING])-1;
          }
        else if(isKOI8(field[XLFD_REGISTRY])){
          if(field[XLFD_ENCODING][0]=='u' || field[XLFD_ENCODING][0]=='U'){
            en=FONTENCODING_KOI8_U;
            }
          else if(field[XLFD_ENCODING][0]=='r' || field[XLFD_ENCODING][0]=='R'){
            en=FONTENCODING_KOI8_R;
            }
          else{
            en=FONTENCODING_KOI8;
            }
          }
        else{
          en=FONTENCODING_DEFAULT;
          }
        if(en==encoding) value+=ENCODING_FACTOR;
        }

      // Check pitch
      pitch=pitchfromtext(field[XLFD_SPACING]);
      if(hints&FONTPITCH_FIXED){
        if(pitch&FONTPITCH_FIXED) value+=PITCH_FACTOR;
        }
      else if(hints&FONTPITCH_VARIABLE){
        if(pitch&FONTPITCH_VARIABLE) value+=PITCH_FACTOR;
        }
      else{
        value+=PITCH_FACTOR;
        }

      // Scalable
      if(EQUAL1(field[XLFD_PIXELSIZE],'0') && EQUAL1(field[XLFD_POINTSIZE],'0') && EQUAL1(field[XLFD_AVERAGE],'0')){
        value+=SCALABLE_FACTOR;
        scalable=1;
        }
      else{
        if(!(hints&FONTHINT_SCALABLE)) value+=SCALABLE_FACTOR;
        }

      // Polymorphic
      if(EQUAL1(field[XLFD_WEIGHT],'0') || EQUAL1(field[XLFD_SETWIDTH],'0') || EQUAL1(field[XLFD_SLANT],'0') || EQUAL1(field[XLFD_ADDSTYLE],'0')){
        value+=POLY_FACTOR;
        polymorphic=1;
        }
      else{
        if(!(hints&FONTHINT_POLYMORPHIC)) value+=POLY_FACTOR;
        }

      // Check weight
      if(weight==FONTWEIGHT_DONTCARE){
        dweight=0;
        }
      else{
        w=weightfromtext(field[XLFD_WEIGHT]);
        dweight=w-weight;
        dweight=FXABS(dweight);
        }

      // Check slant
      if(slant==FONTSLANT_DONTCARE){
        value+=SLANT_FACTOR;
        }
      else{
        sl=slantfromtext(field[XLFD_SLANT]);
        if(sl==slant) value+=SLANT_FACTOR;
        }

      // Check SetWidth
      if(setwidth==FONTSETWIDTH_DONTCARE){
        value+=SETWIDTH_FACTOR;
        }
      else{
        sw=setwidthfromtext(field[XLFD_SETWIDTH]);
        if(setwidth==sw) value+=SETWIDTH_FACTOR;
        }

      // If the font can be rendered at any resolution, we'll render at our actual device resolution
      if(EQUAL1(field[XLFD_RESOLUTION_X],'0') && EQUAL1(field[XLFD_RESOLUTION_Y],'0')){
        xres=screenres;
        yres=screenres;
        }

      // Else get the resolution for which the font is designed
      else{
        xres=atoi(field[XLFD_RESOLUTION_X]);
        yres=atoi(field[XLFD_RESOLUTION_Y]);
        }

      // If scalable, we can of course get the exact size we want
      // We do not set dsize to 0, as we prefer a bitmapped font that gets within
      // 10% over a scalable one that's exact, as the bitmapped fonts look much better
      // at small sizes than scalable ones...
      if(scalable){
        value+=SIZE_FACTOR;
        dsize=size/10;
        }

      // Otherwise, we try to get something close
      else{

        // We correct for the actual screen resolution; if the font is rendered at a
        // 100 dpi, and we have a screen with 90dpi, the actual point size of the font
        // should be multiplied by (100/90).
        sz=(yres*atoi(field[XLFD_POINTSIZE]))/screenres;

        // We strongly prefer the largest pointsize not larger than the desired pointsize
        if(sz<=size){
          value+=SIZE_FACTOR;
          dsize=size-sz;
          }

        // But if we can't get that, we'll take anything thats close...
        else{
          dsize=sz-size;
          }
        }

      FXTRACE((160,"%4d: match=%-3x dw=%-3d ds=%3d sc=%d py=%d xres=%-3d yres=%-3d xlfd=%s\n",f,value,dweight,dsize,scalable,polymorphic,xres,yres,fnames[f]));

      // How close is the match?
      if((value>bestvalue) || ((value==bestvalue) && (dsize<bestdsize)) || ((value==bestvalue) && (dsize==bestdsize) && (dweight<bestdweight))){
        bestvalue=value;
        bestdsize=dsize;
        bestdweight=dweight;
        bestscalable=scalable;
        bestxres=xres;
        bestyres=yres;
        bestf=f;
        }
      }
    }
  if(0<=bestf){
    if(!bestscalable){
      strncpy(fontname,fnames[bestf],299);
      }
    else{
      strncpy(fname,fnames[bestf],299);
      parsefontname(field,fname);

      // Build XLFD, correcting for possible difference between font resolution and screen resolution
      sprintf(fontname,"-%s-%s-%s-%s-%s-%s-*-%d-%d-%d-%s-*-%s-%s",field[XLFD_FOUNDRY],field[XLFD_FAMILY],field[XLFD_WEIGHT],field[XLFD_SLANT],field[XLFD_SETWIDTH],field[XLFD_ADDSTYLE],(bestyres*size)/screenres,bestxres,bestyres,field[XLFD_SPACING],field[XLFD_REGISTRY],field[XLFD_ENCODING]);

      // This works!! draw text sideways:- but need to get some more experience with this
      //sprintf(fontname,"-%s-%s-%s-%s-%s-%s-*-[0 64 ~64 0]-%d-%d-%s-*-%s-%s",field[XLFD_FOUNDRY],field[XLFD_FAMILY],field[XLFD_WEIGHT],field[XLFD_SLANT],field[XLFD_SETWIDTH],field[XLFD_ADDSTYLE],screenxres,screenyres,field[XLFD_SPACING],field[XLFD_REGISTRY],field[XLFD_ENCODING]);
      }
    FXTRACE((150,"Best Font:\n"));
    FXTRACE((150,"%4d: match=%3x dw=%-3d ds=%-3d sc=%d py=%d xres=%-3d yres=%-3d xlfd=%s\n",bestf,bestvalue,bestdweight,bestdsize,bestscalable,polymorphic,bestxres,bestyres,fontname));
    XFreeFontNames(fnames);
    return fontname;
    }
  XFreeFontNames(fnames);
  return NULL;
  }


// Try load the best matching font
char* FXFont::findbestfont(char *fontname) const {
  char *match=NULL;

  // Try specified font family first
  if(!name.empty()){
    match=findmatch(fontname,getApp()->reg().readStringEntry("FONTSUBSTITUTIONS",name.text(),name.text()));
    }

  // Try swiss if we didn't have a match yet
  if(!match && ((hints&(FONTHINT_SWISS|FONTHINT_SYSTEM)) || !(hints&FONTHINT_MASK))){
    match=findmatch(fontname,getApp()->reg().readStringEntry("FONTSUBSTITUTIONS","helvetica","helvetica"));
    }

  // Try roman if we didn't have a match yet
  if(!match && ((hints&FONTHINT_ROMAN) || !(hints&FONTHINT_MASK))){
    match=findmatch(fontname,getApp()->reg().readStringEntry("FONTSUBSTITUTIONS","times","times"));
    }

  // Try modern if we didn't have a match yet
  if(!match && ((hints&FONTHINT_MODERN) || !(hints&FONTHINT_MASK))){
    match=findmatch(fontname,getApp()->reg().readStringEntry("FONTSUBSTITUTIONS","courier","courier"));
    }

  // Try decorative if we didn't have a match yet
  if(!match && ((hints&FONTHINT_DECORATIVE) || !(hints&FONTHINT_MASK))){
    match=findmatch(fontname,getApp()->reg().readStringEntry("FONTSUBSTITUTIONS","gothic","gothic"));
    }

  return fontname;
  }


// Try these fallbacks for swiss hint
const char* swissfallback[]={
  "-*-helvetica-bold-r-*-*-*-120-*-*-*-*-*-*",
  "-*-lucida-bold-r-*-*-*-120-*-*-*-*-*-*",
  "-*-helvetica-medium-r-*-*-*-120-*-*-*-*-*-*",
  "-*-lucida-medium-r-*-*-*-120-*-*-*-*-*-*",
  "-*-helvetica-*-*-*-*-*-120-*-*-*-*-*-*",
  "-*-lucida-*-*-*-*-*-120-*-*-*-*-*-*",
  NULL
  };


// Try these fallbacks for times hint
const char* romanfallback[]={
  "-*-times-bold-r-*-*-*-120-*-*-*-*-*-*",
  "-*-charter-bold-r-*-*-*-120-*-*-*-*-*-*",
  "-*-times-medium-r-*-*-*-120-*-*-*-*-*-*",
  "-*-charter-medium-r-*-*-*-120-*-*-*-*-*-*",
  "-*-times-*-*-*-*-*-120-*-*-*-*-*-*",
  "-*-charter-*-*-*-*-*-120-*-*-*-*-*-*",
  NULL
  };


// Try these fallbacks for modern hint
const char* modernfallback[]={
  "-*-courier-bold-r-*-*-*-120-*-*-*-*-*-*",
  "-*-lucidatypewriter-bold-r-*-*-*-120-*-*-*-*-*-*",
  "-*-courier-medium-r-*-*-*-120-*-*-*-*-*-*",
  "-*-lucidatypewriter-medium-r-*-*-*-120-*-*-*-*-*-*",
  "-*-courier-*-*-*-*-*-120-*-*-*-*-*-*",
  "-*-lucidatypewriter-*-*-*-*-*-120-*-*-*-*-*-*",
  NULL
  };


// Try these final fallbacks
const char* finalfallback[]={
  "7x13",
  "8x13",
  "7x14",
  "8x16",
  "9x15",
  NULL
  };


// See which fallback font exists
const char* FXFont::fallbackfont() const {
  const char *fname=NULL;
  int i=0;

  // Try swiss if we wanted swiss, or if we don't care
  if((hints&FONTHINT_SWISS) || !(hints&FONTHINT_MASK)){
    while((fname=swissfallback[i])!=NULL){
      if(matchingfonts(DISPLAY(getApp()),fname)>0) break;
      i++;
      }
    }

  // Try roman if we wanted roman, or if we don't care
  if(!fname && ((hints&FONTHINT_ROMAN) || !(hints&FONTHINT_MASK))){
    while((fname=romanfallback[i])!=NULL){
      if(matchingfonts(DISPLAY(getApp()),fname)>0) break;
      i++;
      }
    }

  // Try modern if we wanted modern, or if we don't care
  if(!fname && ((hints&FONTHINT_MODERN) || !(hints&FONTHINT_MASK))){
    while((fname=modernfallback[i])!=NULL){
      if(matchingfonts(DISPLAY(getApp()),fname)>0) break;
      i++;
      }
    }

  // Try final fallback fonts
  if(!fname){
    while((fname=finalfallback[i])!=NULL){
      if(matchingfonts(DISPLAY(getApp()),fname)>0) break;
      i++;
      }
    if(!fname) fname="fixed";
    }

  return fname;
  }



/*******************************************************************************/

// Helper functions WIN32

#else


// Character set encoding
static BYTE FXFontEncoding2CharSet(FXuint encoding){
  switch(encoding){
    case FONTENCODING_DEFAULT: return DEFAULT_CHARSET;
    case FONTENCODING_TURKISH: return TURKISH_CHARSET;
    case FONTENCODING_BALTIC: return BALTIC_CHARSET;
    case FONTENCODING_CYRILLIC: return RUSSIAN_CHARSET;
    case FONTENCODING_ARABIC: return ARABIC_CHARSET;
    case FONTENCODING_GREEK: return GREEK_CHARSET;
    case FONTENCODING_HEBREW: return HEBREW_CHARSET;
    case FONTENCODING_THAI: return THAI_CHARSET;
    case FONTENCODING_EASTEUROPE: return EASTEUROPE_CHARSET;
    case FONTENCODING_USASCII: return ANSI_CHARSET;
    }
  return DEFAULT_CHARSET;
  }


// Character set encoding
static FXuint CharSet2FXFontEncoding(BYTE lfCharSet){
  switch(lfCharSet){
    case DEFAULT_CHARSET: return FONTENCODING_DEFAULT;
    case OEM_CHARSET: return FONTENCODING_DEFAULT;
    case SYMBOL_CHARSET: return FONTENCODING_DEFAULT;
    case MAC_CHARSET: return FONTENCODING_DEFAULT;
    case ANSI_CHARSET: return FONTENCODING_USASCII;
    case TURKISH_CHARSET: return FONTENCODING_TURKISH;
    case BALTIC_CHARSET: return FONTENCODING_BALTIC;
    case RUSSIAN_CHARSET: return FONTENCODING_CYRILLIC;
    case ARABIC_CHARSET: return FONTENCODING_ARABIC;
    case GREEK_CHARSET: return FONTENCODING_GREEK;
    case HEBREW_CHARSET: return FONTENCODING_HEBREW;
    case THAI_CHARSET: return FONTENCODING_THAI;
    case EASTEUROPE_CHARSET: return FONTENCODING_EASTEUROPE;
    }
  return FONTENCODING_DEFAULT;
  }


// Yuk. Need to get some data into the callback function.
struct FXFontStore {
  HDC         hdc;
  FXFontDesc *fonts;
  FXuint      numfonts;
  FXuint      size;
  FXFontDesc  desc;
  };


// Callback function for EnumFontFamiliesEx()
static int CALLBACK EnumFontFamExProc(const LOGFONT *lf,const TEXTMETRIC *lptm,DWORD FontType,LPARAM lParam){
  FXASSERT(lf!=0);
  FXASSERT(lptm!=0);
  FXFontStore *pFontStore=(FXFontStore*)lParam;
  FXASSERT(pFontStore!=0);

  // Get pitch
  FXuint flags=FONTPITCH_DEFAULT;
  if(lf->lfPitchAndFamily&FIXED_PITCH) flags|=FONTPITCH_FIXED;
  if(lf->lfPitchAndFamily&VARIABLE_PITCH) flags|=FONTPITCH_VARIABLE;

  // Skip if no match
  FXuint h=pFontStore->desc.flags;
  if((h&FONTPITCH_FIXED) && !(flags&FONTPITCH_FIXED)) return 1;
  if((h&FONTPITCH_VARIABLE) && !(flags&FONTPITCH_VARIABLE)) return 1;

  // Get weight (also guess from the name)
  FXuint weight=lf->lfWeight;
  if(strstr(lf->lfFaceName," Bold")!=NULL) weight=FONTWEIGHT_BOLD;
  if(strstr(lf->lfFaceName," Demi")!=NULL) weight=FONTWEIGHT_DEMIBOLD;
  if(strstr(lf->lfFaceName," Light")!=NULL) weight=FONTWEIGHT_LIGHT;
  if(strstr(lf->lfFaceName," Medium")!=NULL) weight=FONTWEIGHT_MEDIUM;

  // Skip if weight doesn't match
  FXuint wt=pFontStore->desc.weight;
  if((wt!=FONTWEIGHT_DONTCARE) && (wt!=weight)) return 1;

  // Get slant
  FXuint slant=FONTSLANT_REGULAR;
  if(lf->lfItalic==TRUE) slant=FONTSLANT_ITALIC;
  if(strstr(lf->lfFaceName," Italic")!=NULL) slant=FONTSLANT_ITALIC;
  if(strstr(lf->lfFaceName," Roman")!=NULL) slant=FONTSLANT_REGULAR;

  // Skip if no match
  FXuint sl=pFontStore->desc.slant;
  if((sl!=FONTSLANT_DONTCARE) && (sl!=slant)) return 1;

  // Get set width (also guess from the name)
  FXuint setwidth=FONTSETWIDTH_DONTCARE;
  if(strstr(lf->lfFaceName," Cond")!=NULL) setwidth=FONTSETWIDTH_CONDENSED;
  if(strstr(lf->lfFaceName," Ext Cond")!=NULL) setwidth=FONTSETWIDTH_EXTRACONDENSED;

  // Skip if no match
  FXuint sw=pFontStore->desc.setwidth;
  if((sw!=FONTSETWIDTH_DONTCARE) && (sw!=setwidth)) return 1;

  // Get encoding
  FXuint encoding=CharSet2FXFontEncoding(lf->lfCharSet);

  // Skip if no match
  FXuint en=pFontStore->desc.encoding;
  if((en!=FONTENCODING_DEFAULT) && (en!=encoding)) return 1;

  // Is it scalable?
  if(FontType==TRUETYPE_FONTTYPE){
    flags|=FONTHINT_SCALABLE;
    }

  // Is it polymorphic?
  if(FontType==TRUETYPE_FONTTYPE){
    flags|=FONTHINT_POLYMORPHIC;
    }

  // Initial allocation of storage?
  if(pFontStore->numfonts==0){
    FXMALLOC(&pFontStore->fonts,FXFontDesc,50);
    if(pFontStore->fonts==0) return 0;
    pFontStore->size=50;
    }

  // Grow the array if needed
  if(pFontStore->numfonts>=pFontStore->size){
    FXRESIZE(&pFontStore->fonts,FXFontDesc,pFontStore->size+50);
    if(pFontStore->fonts==0) return 0;
    pFontStore->size+=50;
    }

  FXFontDesc *fonts=pFontStore->fonts;
  FXuint numfonts=pFontStore->numfonts;

  strncpy(fonts[numfonts].face,lf->lfFaceName,sizeof(fonts[0].face));
  fonts[numfonts].size=fxheight_to_pointsize(pFontStore->hdc,lf->lfHeight); /// This may be incorrect
  fonts[numfonts].weight=weight;
  fonts[numfonts].slant=slant;
  fonts[numfonts].encoding=encoding;
  fonts[numfonts].setwidth=setwidth;
  fonts[numfonts].flags=flags;

  pFontStore->fonts=fonts;
  pFontStore->numfonts++;

  // Must return 1 to continue enumerating fonts
  return 1;
  }



#endif


/*******************************************************************************/

// Object implementation
FXIMPLEMENT(FXFont,FXId,NULL,0)


// Deserialization
FXFont::FXFont(){
  font=(void*)-1;
#ifdef WIN32
  dc=NULL;
#endif
  }


// Construct font from X11 font spec
FXFont::FXFont(FXApp* a,const FXString& nm):FXId(a),name(nm){
  FXTRACE((100,"FXFont::FXFont %p\n",this));
  size=0;
  weight=0;
  slant=0;
  encoding=FONTENCODING_DEFAULT;
  setwidth=FONTSETWIDTH_DONTCARE;
#ifndef WIN32
  hints=FONTHINT_X11;               // X11 font string method
#else
  hints=0;
  dc=NULL;
#endif
  font=NULL;
  }


// Construct a font with given face name, size in points(pixels), weight, slant, character set encoding, setwidth, and hints
FXFont::FXFont(FXApp* a,const FXString& face,FXuint sz,FXuint wt,FXuint sl,FXuint enc,FXuint setw,FXuint h):FXId(a),name(face){
  FXTRACE((100,"FXFont::FXFont %p\n",this));
  size=10*sz;
  weight=wt;
  slant=sl;
  encoding=enc;
  setwidth=setw;
  hints=(h&~FONTHINT_X11);          // System-independent method
  font=NULL;
#ifdef WIN32
  dc=NULL;
#endif
  }


// Construct font from font description
FXFont::FXFont(FXApp* a,const FXFontDesc& fontdesc):FXId(a){
  FXTRACE((100,"FXFont::FXFont %p\n",this));
  name=fontdesc.face;
  size=fontdesc.size;
  weight=fontdesc.weight;
  slant=fontdesc.slant;
  encoding=fontdesc.encoding;
  setwidth=fontdesc.setwidth;
  hints=fontdesc.flags;             // We may get system dependent fonts
  font=NULL;
#ifdef WIN32
  dc=NULL;
#endif
  }


// Get font description
void FXFont::getFontDesc(FXFontDesc& fontdesc) const {
  strncpy(fontdesc.face,name.text(),sizeof(fontdesc.face));
  fontdesc.size=size;
  fontdesc.weight=weight;
  fontdesc.slant=slant;
  fontdesc.encoding=encoding;
  fontdesc.setwidth=setwidth;
  fontdesc.flags=hints;
  }


// Change font description
void FXFont::setFontDesc(const FXFontDesc& fontdesc){
  if(xid){ fxerror("%s::setFontDesc: trying to change font after creation.\n",getClassName()); }
  name=fontdesc.face;
  size=fontdesc.size;
  weight=fontdesc.weight;
  slant=fontdesc.slant;
  encoding=fontdesc.encoding;
  setwidth=fontdesc.setwidth;
  hints=fontdesc.flags;
  }

/*******************************************************************************/


// Create font
void FXFont::create(){
  if(!xid){
    if(getApp()->initialized){
      FXTRACE((100,"%s::create %p\n",getClassName(),this));
#ifndef WIN32

      // X11 font specification
      if(hints&FONTHINT_X11){

        // Should have non-NULL font name
        if(!name.text()){ fxerror("%s::create: font name should not be NULL.\n",getClassName()); }

        FXTRACE((150,"%s::create: X11 font: %s\n",getClassName(),name.text()));

        // Try load the font
        font=XLoadQueryFont(DISPLAY(getApp()),name.text());
        }

      // Platform independent specification
      else{
        FXTRACE((150,"%s::create: face: %s size: %d weight: %d slant: %d encoding: %d hints: %04x\n",getClassName(),name.text()?name.text():"none",size,weight,slant,encoding,hints));

        // Try load best font
        char fontname[400];
        font=XLoadQueryFont(DISPLAY(getApp()),findbestfont(fontname));
        }

      // If we still don't have a font yet, try fallback fonts
      if(!font){
        font=XLoadQueryFont(DISPLAY(getApp()),fallbackfont());
        }

      // Remember font id
      if(font){ xid=((XFontStruct*)font)->fid; }

      // Uh-oh, we failed
      if(!xid){ fxerror("%s::create: unable to create font.\n",getClassName()); }

      // Dump some useful stuff
      FXTRACE((150,"min_char_or_byte2   = %d\n",((XFontStruct*)font)->min_char_or_byte2));
      FXTRACE((150,"max_char_or_byte2   = %d\n",((XFontStruct*)font)->max_char_or_byte2));
      FXTRACE((150,"default_char        = %c\n",((XFontStruct*)font)->default_char));
      FXTRACE((150,"min_bounds.lbearing = %d\n",((XFontStruct*)font)->min_bounds.lbearing));
      FXTRACE((150,"min_bounds.rbearing = %d\n",((XFontStruct*)font)->min_bounds.rbearing));
      FXTRACE((150,"min_bounds.width    = %d\n",((XFontStruct*)font)->min_bounds.width));
      FXTRACE((150,"min_bounds.ascent   = %d\n",((XFontStruct*)font)->min_bounds.ascent));
      FXTRACE((150,"min_bounds.descent  = %d\n",((XFontStruct*)font)->min_bounds.descent));
      FXTRACE((150,"max_bounds.lbearing = %d\n",((XFontStruct*)font)->max_bounds.lbearing));
      FXTRACE((150,"max_bounds.rbearing = %d\n",((XFontStruct*)font)->max_bounds.rbearing));
      FXTRACE((150,"max_bounds.width    = %d\n",((XFontStruct*)font)->max_bounds.width));
      FXTRACE((150,"max_bounds.ascent   = %d\n",((XFontStruct*)font)->max_bounds.ascent));
      FXTRACE((150,"max_bounds.descent  = %d\n",((XFontStruct*)font)->max_bounds.descent));

//     // At this point, we should examine the font properties
//     if(font){
//       XFontStruct *fs=(XFontStruct*) font;
//       int i;
//       char *val;
//       if(fs->properties){
//         for(i=0; i<fs->n_properties; i++){
//           fprintf(stderr,"ATOM = %d (%s)",fs->properties[i].name,XGetAtomName(DISPLAY(getApp()),fs->properties[i].name));
//             fprintf(stderr," = %d\n",fs->properties[i].card32);
//           }
//         }
//       }
#else

      // Windows will not support the X11 font string specification method
      if(hints&FONTHINT_X11){ fxerror("%s::create: this method of font specification not supported under Windows.\n",getClassName()); }

      // Hang on to this for text metrics functions
      dc=CreateCompatibleDC(NULL);

      // Now fill in the fields
      LOGFONT lf;
      //lf.lfHeight=fxpointsize_to_height(dc,size);
      lf.lfHeight = -MulDiv(size, GetDeviceCaps((HDC)dc, LOGPIXELSY), 720);  // This looks correct!!
      lf.lfWidth=0;
      lf.lfEscapement=0;
      lf.lfOrientation=0;
      lf.lfWeight=weight;
      if((slant==FONTSLANT_ITALIC) || (slant==FONTSLANT_OBLIQUE))
        lf.lfItalic=TRUE;
      else
        lf.lfItalic=FALSE;
      lf.lfUnderline=FALSE;
      lf.lfStrikeOut=FALSE;

      // Character set encoding
      lf.lfCharSet=FXFontEncoding2CharSet(encoding);

      // Other hints
      lf.lfOutPrecision=OUT_DEFAULT_PRECIS;
      if(hints&FONTHINT_SYSTEM) lf.lfOutPrecision=OUT_RASTER_PRECIS;
      if(hints&FONTHINT_SCALABLE) lf.lfOutPrecision=OUT_TT_PRECIS;
      if(hints&FONTHINT_POLYMORPHIC) lf.lfOutPrecision=OUT_TT_PRECIS;

      // Clip precision
      lf.lfClipPrecision=CLIP_DEFAULT_PRECIS;

      // Quality
      lf.lfQuality=DEFAULT_QUALITY;

      // Pitch and Family
      lf.lfPitchAndFamily=0;

      // Pitch
      if(hints&FONTPITCH_FIXED) lf.lfPitchAndFamily|=FIXED_PITCH;
      else if(hints&FONTPITCH_VARIABLE) lf.lfPitchAndFamily|=VARIABLE_PITCH;
      else lf.lfPitchAndFamily|=DEFAULT_PITCH;

      // Family
      if(hints&FONTHINT_DECORATIVE) lf.lfPitchAndFamily|=FF_DECORATIVE;
      else if(hints&FONTHINT_MODERN) lf.lfPitchAndFamily|=FF_MODERN;
      else if(hints&FONTHINT_ROMAN) lf.lfPitchAndFamily|=FF_ROMAN;
      else if(hints&FONTHINT_SCRIPT) lf.lfPitchAndFamily|=FF_SCRIPT;
      else if(hints&FONTHINT_SWISS) lf.lfPitchAndFamily|=FF_SWISS;
      else lf.lfPitchAndFamily|=FF_DONTCARE;

      // Font substitution
      if(!name.empty()){
        strncpy(lf.lfFaceName,getApp()->reg().readStringEntry("FONTSUBSTITUTIONS",name.text(),name.text()),sizeof(lf.lfFaceName));
        lf.lfFaceName[sizeof(lf.lfFaceName)-1]='\0';
        }
      else{
        lf.lfFaceName[0]='\0';
        }

      // Here we go!
      xid=CreateFontIndirect(&lf);

      // Uh-oh, we failed
      if(!xid){ fxerror("%s::create: unable to create font.\n",getClassName()); }

      // Obtain text metrics
      FXCALLOC(&font,TEXTMETRIC,1);
      SelectObject((HDC)dc,xid);
      GetTextMetrics((HDC)dc,(TEXTMETRIC*)font);
#endif
      }
    }
  }


// Detach font
void FXFont::detach(){
  if(xid){
    FXTRACE((100,"%s::detach %p\n",getClassName(),this));
#ifndef WIN32

    // Free font struct w/o doing anything else...
    XFreeFont(DISPLAY(getApp()),(XFontStruct*)font);
#else

    // Free font metrics
    FXFREE(&font);
#endif
    font=NULL;
    xid=0;
    }
  }


// Destroy font
void FXFont::destroy(){
  if(xid){
    if(getApp()->initialized){
      FXTRACE((100,"%s::destroy %p\n",getClassName(),this));
#ifndef WIN32

      // Delete font & metrics
      XFreeFont(DISPLAY(getApp()),(XFontStruct*)font);
#else

      // Necessary to prevent resource leak
      SelectObject((HDC)dc,GetStockObject(SYSTEM_FONT));

      // Delete font
      DeleteObject((HFONT)xid);

      // Delete dummy DC
      DeleteDC((HDC)dc);

      // Free font metrics
      FXFREE(&font);
#endif
      }
    font=NULL;
    xid=0;
    }
  }


// Does font have given character glyph?
FXbool FXFont::hasChar(FXint ch) const {
  if(font) return HASCHAR(font,ch);
  return FALSE;
  }


// Get first character glyph in font
FXint FXFont::getMinChar() const {
  if(font) return FIRSTCHAR(font);
  return 0;
  }


// Get last character glyph in font
FXint FXFont::getMaxChar() const {
  if(font) return LASTCHAR(font);
  return 0;
  }


// Get font leading [that is lead-ing as in Pb!]
FXint FXFont::getFontLeading() const {
  if(font){
#ifndef WIN32
    return ((XFontStruct*)font)->ascent+((XFontStruct*)font)->descent-((XFontStruct*)font)->max_bounds.ascent-((XFontStruct*)font)->max_bounds.descent;
#else
    return ((TEXTMETRIC*)font)->tmExternalLeading;
#endif
    }
  return 0;
  }


// Get font line spacing [height+leading]
FXint FXFont::getFontSpacing() const {
  if(font){
#ifndef WIN32
    return ((XFontStruct*)font)->ascent+((XFontStruct*)font)->descent;
#else
    return ((TEXTMETRIC*)font)->tmHeight; // tmHeight includes font point size plus internal leading
#endif
    }
  return 1;
  }


// Left bearing
FXint FXFont::leftBearing(FXchar ch) const {
  if(font){
#ifndef WIN32
    if(((XFontStruct*)font)->per_char){
      if(!HASCHAR(font,ch)){ ch=((XFontStruct*)font)->default_char; }
      return ((XFontStruct*)font)->per_char[(FXuint)ch-((XFontStruct*)font)->min_char_or_byte2].lbearing;
      }
    return ((XFontStruct*)font)->max_bounds.lbearing;
#else
    return 0; // FIXME
#endif
    }
  return 0;
  }


// Right bearing
FXint FXFont::rightBearing(FXchar ch) const {
  if(font){
#ifndef WIN32
    if(((XFontStruct*)font)->per_char){
      if(!HASCHAR(font,ch)){ ch=((XFontStruct*)font)->default_char; }
      return ((XFontStruct*)font)->per_char[(FXuint)ch-((XFontStruct*)font)->min_char_or_byte2].rbearing;
      }
    return ((XFontStruct*)font)->max_bounds.rbearing;
#else
    return 0; // FIXME
#endif
    }
  return 0;
  }


// Is it a mono space font
FXbool FXFont::isFontMono() const {
  if(font){
#ifndef WIN32
    return ((XFontStruct*)font)->min_bounds.width == ((XFontStruct*)font)->max_bounds.width;
#else
    return !(((TEXTMETRIC*)font)->tmPitchAndFamily&TMPF_FIXED_PITCH);
#endif
    }
  return TRUE;
  }


// Get font width
FXint FXFont::getFontWidth() const {
  if(font){
#ifndef WIN32
    return ((XFontStruct*)font)->max_bounds.width;
#else
    return ((TEXTMETRIC*)font)->tmMaxCharWidth;
#endif
    }
  return 1;
  }


// Get font height
FXint FXFont::getFontHeight() const {
  if(font){
#ifndef WIN32
    return ((XFontStruct*)font)->ascent+((XFontStruct*)font)->descent;  // This is wrong!
    //return ((XFontStruct*)font)->max_bounds.ascent+((XFontStruct*)font)->max_bounds.descent;  // This is right!
#else
    return ((TEXTMETRIC*)font)->tmHeight;
#endif
    }
  return 1;
  }


// Get font ascent
FXint FXFont::getFontAscent() const {
  if(font){
#ifndef WIN32
    return ((XFontStruct*)font)->ascent;
#else
    return ((TEXTMETRIC*)font)->tmAscent;
#endif
    }
  return 1;
  }


// Get font descent
FXint FXFont::getFontDescent() const {
  if(font){
#ifndef WIN32
    return ((XFontStruct*)font)->descent;
#else
    return ((TEXTMETRIC*)font)->tmDescent;
#endif
    }
  return 0;
  }


// Text width
FXint FXFont::getTextWidth(const FXchar *text,FXuint n) const {
  if(!text && n){ fxerror("%s::getTextWidth: NULL string argument\n",getClassName()); }
  if(font){
#ifndef WIN32
    return XTextWidth((XFontStruct*)font,text,n);
#else
    SIZE size;
    FXASSERT(dc!=NULL);
    GetTextExtentPoint32((HDC)dc,text,n,&size);
    return size.cx;
#endif
    }
  return n;
  }


// Text height
FXint FXFont::getTextHeight(const FXchar *text,FXuint n) const {
  if(!text && n){ fxerror("%s::getTextHeight: NULL string argument\n",getClassName()); }
  if(font){
#ifndef WIN32
    XCharStruct chst; int dir,asc,desc;
    XTextExtents((XFontStruct*)font,text,n,&dir,&asc,&desc,&chst);
    return asc+desc;
#else
    SIZE size;
    FXASSERT(dc!=NULL);
    GetTextExtentPoint32((HDC)dc,text,n,&size);
    return size.cy;
#endif
    }
  return 1;
  }


/*******************************************************************************/


// Function to sort by name, weight, slant, and size
#ifdef __IBMCPP__
static int _Optlink comparefont(const void * a,const void *b){
#else
static int comparefont(const void * a,const void *b){
#endif
  register FXFontDesc *fa=(FXFontDesc*)a;
  register FXFontDesc *fb=(FXFontDesc*)b;
  register int cmp;
  return (cmp=strcmp(fa->face,fb->face)) ? cmp : (fa->weight!=fb->weight) ? fa->weight-fb->weight : (fa->slant!=fb->slant) ? fa->slant-fb->slant : fa->size-fb->size;
  }


#ifndef WIN32

// List all fonts matching hints
FXbool FXFont::listFonts(FXFontDesc*& fonts,FXuint& numfonts,const FXString& face,FXuint wt,FXuint sl,FXuint sw,FXuint en,FXuint h){
  FXuint size,weight,slant,encoding,setwidth,flags;
  FXint screenres,xres,yres;
  FXchar pattern[300],fname[300],*field[14];
  FXchar **fnames;
  const FXchar *scal;
  const FXchar *facename;
  FXint numfnames,f,j,addit;

  fonts=NULL;
  numfonts=0;

  // Gotta have display open!
  if(!FXApp::instance()){ fxerror("FXFont::listFonts: no application object.\n"); }
  if(!DISPLAY(FXApp::instance())){ fxerror("FXFont::listFonts: trying to list fonts before opening display.\n"); }

  // Screen resolution may be overidden by registry
  screenres=FXApp::app->reg().readUnsignedEntry("SETTINGS","screenres",100);

  // Validate
  if(screenres<50) screenres=50;
  if(screenres>200) screenres=200;

  FXTRACE((150,"Listing fonts for screenres=%d:\n",screenres));

  if(en>FONTENCODING_KOI8_UNIFIED) en=FONTENCODING_DEFAULT;

  // Define pattern to match against
  if(h&FONTHINT_X11){
    facename="*";
    if(!face.empty()) facename=face.text();
    strcpy(pattern,facename);
    }

  // Match XLFD fonts; try to limit the number by using
  // some of the info we already have acquired.
  else{
    scal="*";
    if(h&FONTHINT_SCALABLE) scal="0";
    facename="*";
    if(!face.empty()) facename=face.text();
    sprintf(pattern,"-*-%s-*-*-*-*-%s-%s-*-*-*-%s-*-*",facename,scal,scal,scal);
    }

  // Get list of all font names
  fnames=listfontnames(DISPLAY(FXApp::instance()),pattern,numfnames);
  if(!fnames) return FALSE;

  // Make room to receive face names
  FXMALLOC(&fonts,FXFontDesc,numfnames);
  if(!fonts){ XFreeFontNames(fnames); return FALSE; }

  // Add all matching fonts to the list
  for(f=0; f<numfnames; f++){
    strncpy(fname,fnames[f],299);

    // XLFD font name; parse out unique face names
    if(parsefontname(field,fname)){

      flags=0;

      // Get encoding
      if(isISO8859(field[XLFD_REGISTRY]))
        encoding=FONTENCODING_ISO_8859_1+atoi(field[XLFD_ENCODING])-1;
      else if(isKOI8(field[XLFD_REGISTRY])){
        if(field[XLFD_ENCODING][0]=='u' || field[XLFD_ENCODING][0]=='U')
          encoding=FONTENCODING_KOI8_U;
        else if(field[XLFD_ENCODING][0]=='r' || field[XLFD_ENCODING][0]=='R')
          encoding=FONTENCODING_KOI8_R;
        else
          encoding=FONTENCODING_KOI8;
        }
      else
        encoding=FONTENCODING_DEFAULT;

      // Skip if no match
      if((en!=FONTENCODING_DEFAULT) && (en!=encoding)) continue;

      // Get pitch
      flags|=pitchfromtext(field[XLFD_SPACING]);

      // Skip this font if pitch does not match
      if((h&FONTPITCH_FIXED) && !(flags&FONTPITCH_FIXED)) continue;
      if((h&FONTPITCH_VARIABLE) && !(flags&FONTPITCH_VARIABLE)) continue;

      // Skip if weight does not match
      weight=weightfromtext(field[XLFD_WEIGHT]);
      if((wt!=FONTWEIGHT_DONTCARE) && (wt!=weight)) continue;

      // Skip if slant does not match
      slant=slantfromtext(field[XLFD_SLANT]);
      if((sl!=FONTSLANT_DONTCARE) && (sl!=slant)) continue;

      // Skip if setwidth does not match
      setwidth=setwidthfromtext(field[XLFD_SETWIDTH]);
      if((sw!=FONTSETWIDTH_DONTCARE) && (sw!=setwidth)) continue;

      // Scalable
      if(EQUAL1(field[XLFD_PIXELSIZE],'0') && EQUAL1(field[XLFD_POINTSIZE],'0') && EQUAL1(field[XLFD_AVERAGE],'0')){
        flags|=FONTHINT_SCALABLE;
        }

      // Polymorphic
      if(EQUAL1(field[XLFD_WEIGHT],'0') || EQUAL1(field[XLFD_SETWIDTH],'0') || EQUAL1(field[XLFD_SLANT],'0') || EQUAL1(field[XLFD_ADDSTYLE],'0')){
        flags|=FONTHINT_POLYMORPHIC;
        }

      // Get Font resolution
      if(EQUAL1(field[XLFD_RESOLUTION_X],'0') && EQUAL1(field[XLFD_RESOLUTION_Y],'0')){
        xres=screenres;
        yres=screenres;
        }
      else{
        xres=atoi(field[XLFD_RESOLUTION_X]);
        yres=atoi(field[XLFD_RESOLUTION_Y]);
        }

      // Get size, corrected for screen resolution
      if(!(flags&FONTHINT_SCALABLE)){
        size=(yres*atoi(field[XLFD_POINTSIZE]))/screenres;
        }
      else{
        size=0;
        }

      // Dump what we have found out
      FXTRACE((160,"Font=%s weight=%d slant=%d size=%3d setwidth=%d encoding=%d\n",field[XLFD_FAMILY],weight,slant,size,setwidth,encoding));

      addit=1;

      // If NULL face name, just list one of each face
      if(face.empty()){
        for(j=numfonts-1; j>=0; j--){
          if(strcmp(field[XLFD_FAMILY],fonts[j].face)==0){
            addit=0;
            break;
            }
          }
        }

      if(addit){
        strncpy(fonts[numfonts].face,field[XLFD_FAMILY],sizeof(fonts[0].face));
        fonts[numfonts].size=size;
        fonts[numfonts].weight=weight;
        fonts[numfonts].slant=slant;
        fonts[numfonts].encoding=encoding;
        fonts[numfonts].setwidth=setwidth;
        fonts[numfonts].flags=flags;
        numfonts++;
        }
      }

    // X11 font, add it to the list
    else{
      strncpy(fonts[numfonts].face,fnames[f],sizeof(fonts[0].face));
      fonts[numfonts].size=0;
      fonts[numfonts].weight=FONTWEIGHT_DONTCARE;
      fonts[numfonts].slant=FONTSLANT_DONTCARE;
      fonts[numfonts].encoding=FONTENCODING_DEFAULT;
      fonts[numfonts].setwidth=FONTSETWIDTH_DONTCARE;
      fonts[numfonts].flags=FONTHINT_X11;
      numfonts++;
      }
    }

  // Any fonts found?
  if(numfonts==0){
    FXFREE(&fonts);
    XFreeFontNames(fnames);
    return FALSE;
    }

  // Realloc to shrink the block
  FXRESIZE(&fonts,FXFontDesc,numfonts);

  // Sort them by name, weight, slant, and size respectively
  qsort(fonts,numfonts,sizeof(FXFontDesc),comparefont);

//   FXTRACE((150,"%d fonts:\n",numfonts));
//   for(f=0; f<numfonts; f++){
//     FXTRACE((150,"Font=%s weight=%d slant=%d size=%3d setwidth=%d encoding=%d\n",fonts[f].face,fonts[f].weight,fonts[f].slant,fonts[f].size,fonts[f].setwidth,fonts[f].encoding));
//     }
//   FXTRACE((150,"\n\n"));

  // Free the font names
  XFreeFontNames(fnames);
  return TRUE;
  }


#else


// List all fonts matching hints
FXbool FXFont::listFonts(FXFontDesc*& fonts,FXuint& numfonts,const FXString& face,FXuint wt,FXuint sl,FXuint sw,FXuint en,FXuint h){
  register FXuint i,j;

  // Initialize return values
  fonts=NULL;
  numfonts=0;

  // This data gets passed into the callback function
  FXFontStore fontStore;
  HDC hdc=GetDC(GetDesktopWindow());
  SaveDC(hdc);
  fontStore.hdc=hdc;
  fontStore.fonts=fonts;
  fontStore.numfonts=numfonts;
  fontStore.desc.weight=wt;
  fontStore.desc.slant=sl;
  fontStore.desc.setwidth=sw;
  fontStore.desc.encoding=en;
  fontStore.desc.flags=h;

  // Fill in the appropriate fields of the LOGFONT structure. Note that
  // EnumFontFamiliesEx() only examines the lfCharSet, lfFaceName and
  // lpPitchAndFamily fields of this struct.
  LOGFONT lf;
  lf.lfCharSet=FXFontEncoding2CharSet(en);
  FXASSERT(face.length()<LF_FACESIZE);
  strncpy(lf.lfFaceName,face.text(),LF_FACESIZE);
  lf.lfPitchAndFamily=0;                          // Should be MONO_FONT for Hebrew and Arabic?

  // Start enumerating!
  EnumFontFamiliesEx(hdc,&lf,EnumFontFamExProc,(LPARAM)&fontStore,0);
  RestoreDC(hdc,-1);
  ReleaseDC(GetDesktopWindow(),hdc);

  // Copy stuff back from the store
  fonts=fontStore.fonts;
  numfonts=fontStore.numfonts;

  // Any fonts found?
  if(numfonts==0){
    FXFREE(&fonts);
    return FALSE;
    }

  // Sort them by name, weight, slant, and size respectively
  qsort(fonts,numfonts,sizeof(FXFontDesc),comparefont);

  // Weed out duplicates if we were just listing the face names
  if(lf.lfCharSet==DEFAULT_CHARSET && lf.lfFaceName[0]==0){
    i=j=1;
    while(j<numfonts){
      if(strcmp(fonts[i-1].face,fonts[j].face)!=0){
        fonts[i]=fonts[j];
        i++;
        }
      j++;
      }
    numfonts=i;
    }

  // Realloc to shrink the block
  FXRESIZE(&fonts,FXFontDesc,numfonts);

//   FXTRACE((150,"%d fonts:\n",numfonts));
//   for(FXuint f=0; f<numfonts; f++){
//     FXTRACE((150,"Font=%s weight=%d slant=%d size=%3d setwidth=%d encoding=%d\n",fonts[f].face,fonts[f].weight,fonts[f].slant,fonts[f].size,fonts[f].setwidth,fonts[f].encoding));
//     }
//   FXTRACE((150,"\n\n"));

  return TRUE;
  }

#endif


/*******************************************************************************/


// Save font to stream
void FXFont::save(FXStream& store) const {
  FXId::save(store);
  store << name;
  store << size;
  store << weight;
  store << slant;
  store << encoding;
  store << setwidth;
  store << hints;
  }


// Load font from stream; create() should be called later
void FXFont::load(FXStream& store){
  FXId::load(store);
  store >> name;
  store >> size;
  store >> weight;
  store >> slant;
  store >> encoding;
  store >> setwidth;
  store >> hints;
  }




// // Font text angles
// static const FXchar* slanttable[]={
//   "",
//   "regular",
//   "italic",
//   "oblique",
//   "reverse italic",
//   "reverse oblique"
//   };
//
//
// // Character set encodings
// static const FXchar* encodingtable[]={
//   "",
//   "iso8859-1",
//   "iso8859-2",
//   "iso8859-3",
//   "iso8859-4",
//   "iso8859-5",
//   "iso8859-6",
//   "iso8859-7",
//   "iso8859-8",
//   "iso8859-9",
//   "iso8859-10",
//   "iso8859-11",
//   "koi8"
//   };
//
//
// // Set width table
// static const FXchar* setwidthtable[]={
//   "",
//   "ultracondensed",
//   "extracondensed",
//   "narrow",
//   "semicondensed",
//   "normal",
//   "semiexpanded",
//   "expanded",
//   "wide",
//   "ultraexpanded"
//   };
//
//
// // Weight table
// static const FXchar* weighttable[]={
//   "",
//   "light",
//   "light",
//   "light",
//   "normal",
//   "medium",
//   "demibold",
//   "bold",
//   "bold",
//   "black"
//   };
//     //helvetica,12,bold,i,normal,iso8859-1,0
//
// static inline FXuint findintable(const FXchar** table,const FXString& what,FXint n){
//   register FXint i;
//   if(!what.empty()){ for(i=0; i<n; i++){ if(what==table[i]) return i; } }
//   return 0;
//   }
//
//
// // Parse font description from a string
// FXbool FXFont::parseFontDesc(FXFontDesc& fontdesc,const FXString& string){
//   strncpy(fontdesc.face,string.extract(0,',').text(),sizeof(fontdesc.face));
//   fontdesc.size=(FXint)(10.0*FXDoubleVal(string.extract(1,',')));
//   fontdesc.weight=100*findintable(weighttable,string.extract(2,','),ARRAYNUMBER(weighttable));
//   fontdesc.slant=findintable(slanttable,string.extract(3,','),ARRAYNUMBER(slanttable));
//   fontdesc.setwidth=10*findintable(setwidthtable,string.extract(4,','),ARRAYNUMBER(setwidthtable));
//   fontdesc.encoding=findintable(encodingtable,string.extract(5,','),ARRAYNUMBER(encodingtable));
//   fontdesc.flags=FXUIntVal(string.extract(6,','));
//   return TRUE;
//   }
//
//
// // Unparse font description into a string
// FXbool FXFont::unparseFontDesc(FXString& string,const FXFontDesc& fontdesc){
//   if(fontdesc.flags&FONTHINT_X11){
//     string.format("%s",fontdesc.face);
//     }
//   else{
//     if(fontdesc.weight>FONTWEIGHT_BLACK) return FALSE;
//     if(fontdesc.setwidth>FONTSETWIDTH_ULTRAEXPANDED) return FALSE;
//     if(fontdesc.slant>FONTSLANT_REVERSE_OBLIQUE) return FALSE;
//     if(fontdesc.encoding>FONTENCODING_KOI8) return FALSE;
//     string.format("%s,%g,%s,%s,%s,%s,%d",fontdesc.face,0.1*fontdesc.size,weighttable[fontdesc.weight/100],slanttable[fontdesc.slant],setwidthtable[fontdesc.setwidth/10],encodingtable[fontdesc.encoding],fontdesc.flags);
//     }
//   return TRUE;
//   }


//   FXString string;
//   fprintf(stderr,"face=%s,size=%d,weight=%d,slant=%d,encoding=%d,setwidth=%d,flags=%d\n",selected.face,selected.size,selected.weight,selected.slant,selected.encoding,selected.setwidth,selected.flags);
//   string=FXFont::unparseFontDesc(selected);
//   fprintf(stderr,"unparse = %s\n",string.text());
//   {
//     FXFontDesc selected;
//   FXFont::parseFontDesc(selected,string);
//   fprintf(stderr,"face=%s,size=%d,weight=%d,slant=%d,encoding=%d,setwidth=%d,flags=%d\n",selected.face,selected.size,selected.weight,selected.slant,selected.encoding,selected.setwidth,selected.flags);
//   FXFont::parseFontDesc(selected,"helvetica,12,bold");
//   fprintf(stderr,"face=%s,size=%d,weight=%d,slant=%d,encoding=%d,setwidth=%d,flags=%d\n",selected.face,selected.size,selected.weight,selected.slant,selected.encoding,selected.setwidth,selected.flags);
//   }


// Thanks to Yakubenko Maxim <max@tiki.sio.rssi.ru> the patch to the
// functions below; the problem was that spaces may occur in the font
// name (e.g. Courier New).  The scanset method will simply parse the
// string, including any spaces, until the matching bracket.

// Parse font description DEPRECATED
FXbool fxparsefontdesc(FXFontDesc& fontdesc,const FXchar* string){
  return string && (sscanf(string,"[%[^]]] %u %u %u %u %u %u",fontdesc.face,&fontdesc.size,&fontdesc.weight,&fontdesc.slant,&fontdesc.encoding,&fontdesc.setwidth,&fontdesc.flags)==7);
  }


// Unparse font description DEPRECATED
FXbool fxunparsefontdesc(FXchar *string,const FXFontDesc& fontdesc){
  sprintf(string,"[%s] %u %u %u %u %u %u",fontdesc.face,fontdesc.size,fontdesc.weight,fontdesc.slant,fontdesc.encoding,fontdesc.setwidth,fontdesc.flags);
  return TRUE;
  }


// Clean up
FXFont::~FXFont(){
  FXTRACE((100,"FXFont::~FXFont %p\n",this));
  destroy();
  }

