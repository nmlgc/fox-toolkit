/********************************************************************************
*                                                                               *
*                 F O X   Definitions, Types, and  Macros                       *
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
* $Id: fxdefs.h,v 1.69 1998/10/29 07:42:21 jeroen Exp $                         *
********************************************************************************/
#ifndef FXDEFS_H
#define FXDEFS_H


/********************************  Definitions  ********************************/

// Truth values
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef MAYBE
#define MAYBE 2
#endif
#ifndef NULL
#define NULL 0
#endif

#ifndef PI 
#define PI      3.1415926535897932384626433833
#endif

// Euler constant
#define EULER   2.7182818284590452353602874713

// Multiplier for degrees to radians
#define DTOR    0.0174532925199432957692369077

// Multipier for radians to degrees
#define RTOD    57.295779513082320876798154814

// Path separator
#ifdef WIN32
#define PATHSEP '\\'
#define PATHLISTSEP ';'
#else
#define PATHSEP '/'
#define PATHLISTSEP ':'
#endif


// FOX System Defined Selector Types 
enum FXSelType {
  SEL_NONE,
  SEL_KEYPRESS,                       // Key
  SEL_KEYRELEASE,            
  SEL_LEFTBUTTONPRESS,                // Buttons
  SEL_LEFTBUTTONRELEASE,     
  SEL_MIDDLEBUTTONPRESS,     
  SEL_MIDDLEBUTTONRELEASE,   
  SEL_RIGHTBUTTONPRESS,      
  SEL_RIGHTBUTTONRELEASE,    
  SEL_MOTION,                         // Mouse motion
  SEL_ENTER,                 
  SEL_LEAVE,                 
  SEL_FOCUSIN,               
  SEL_FOCUSOUT,              
  SEL_KEYMAP,                
  SEL_PAINT,                          // Must repaint window
  SEL_CREATE,                
  SEL_DESTROY,               
  SEL_UNMAP,                 
  SEL_MAP,                   
  SEL_CONFIGURE,                      // Resize
  SEL_SELECTION_LOST,                 // Widget lost selection    
  SEL_SELECTION_GAINED,               // Widget gained selection
  SEL_SELECTION_REQUEST,              // Inquire selection data     
  SEL_RAISED,                
  SEL_LOWERED,               
  SEL_VISIBILITY,                     // Visibility changed
  SEL_CLOSE,                          // Close window
  SEL_UPDATE,                         // GUI update
  SEL_COMMAND,                        // GUI command
  SEL_CLICKED,                        // Clicked
  SEL_DOUBLECLICKED,                  // Double-clicked
  SEL_TRIPLECLICKED,                  // Triple-clicked
  SEL_CHANGED,                        // GUI changed
  SEL_DESELECTED,                     // Deselected
  SEL_SELECTED,                       // Selected
  SEL_INSERTED,                       // Inserted
  SEL_DELETED,                        // Deleted
  SEL_OPENED,                         // Opened
  SEL_CLOSED,                         // Closed
  SEL_EXPANDED,                       // Expanded
  SEL_COLLAPSED,                      // Collapsed
  SEL_DRAGGED,                        // Dragged
  SEL_TIMEOUT,                        // Timeout occurred
  SEL_CHORE,                          // Background chore
  SEL_FOCUS_RIGHT,                    // Focus movements
  SEL_FOCUS_LEFT,            
  SEL_FOCUS_DOWN,            
  SEL_FOCUS_UP,  
  SEL_FOCUS_HOME,
  SEL_FOCUS_END,            
  SEL_FOCUS_NEXT,            
  SEL_FOCUS_PREV,
  SEL_DND_ENTER,                      // Drag action entering potential drop target
  SEL_DND_LEAVE,                      // Drag action leaving potential drop target
  SEL_DND_DROP,                       // Drop on drop target
  SEL_DND_MOTION,                     // Drag position changed over potential drop target
  SEL_ACTIVATE,                       // Activate through mouse or keyboard
  SEL_DEACTIVATE,                     // Deactivate through mouse or keyboard
  SEL_UNCHECK_OTHER,                  // Sent by child to parent to uncheck other children
  SEL_UNCHECK_RADIO,                  // Sent by parent to uncheck radio children
  SEL_LAST                            // Last message
  };


// FOX Keyboard/Button states
enum FXModifierMasks {
  SHIFTMASK        = 0x001,
  CAPSLOCKMASK     = 0x002,
  CONTROLMASK      = 0x004,
  ALTMASK          = 0x008,
  NUMLOCKMASK      = 0x010,
  SCROLLLOCKMASK   = 0x0E0,           // This seems to vary
  LEFTBUTTONMASK   = 0x100,
  MIDDLEBUTTONMASK = 0x200,
  RIGHTBUTTONMASK  = 0x400
  };


// FOX Mouse Buttons
enum FXMouseButton {
  LEFTBUTTON       = 1,
  MIDDLEBUTTON     = 2,
  RIGHTBUTTON      = 3
  };


// FOX Crossing Modes
enum FXCrossingMode {
  CROSSINGNORMAL,
  CROSSINGGRAB,
  CROSSINGUNGRAB
  };


// FOX Visibility Modes
enum FXVisibility {
  VISIBILITYTOTAL,
  VISIBILITYPARTIAL,
  VISIBILITYNONE
  };


// Drawing (BITBLT) functions
enum FXFunction {
  BLT_CLR,
  BLT_SRC_AND_DST,
  BLT_SRC_AND_NOT_DST,
  BLT_SRC,
  BLT_NOT_SRC_AND_DST,
  BLT_DST,
  BLT_SRC_XOR_DST,
  BLT_SRC_OR_DST,
  BLT_NOT_SRC_AND_NOT_DST,
  BLT_NOT_SRC_XOR_DST,
  BLT_NOT_DST,
  BLT_SRC_OR_NOT_DST,
  BLT_NOT_SRC,
  BLT_NOT_SRC_OR_DST,
  BLT_NOT_SRC_OR_NOT_DST,
  BLT_SET
  };


// Line Styles
enum FXLineStyle {
  LINE_SOLID,
  LINE_ONOFF_DASH,
  LINE_DOUBLE_DASH
  };


// Line Cap Styles
enum FXCapStyle {
  CAP_NOT_LAST,
  CAP_BUTT,
  CAP_ROUND,
  CAP_PROJECTING
  };


// Line Join Styles
enum FXJoinStyle {
  JOIN_MITER,
  JOIN_ROUND,
  JOIN_BEVEL
  };


// Fill Styles
enum FXFillStyle {
  FILL_SOLID,
  FILL_TILED,
  FILL_STIPPLED,
  FILL_OPAQUESTIPPLED
  };


// Fill Rules
enum FXFillRule {
  RULE_EVEN_ODD,
  RULE_WINDING
  };
  
  
// Options for fxfilematch
enum FXFileMatchOptions {
  FILEMATCH_FILE_NAME   = 1,        // No wildcard can ever match `/'
  FILEMATCH_NOESCAPE    = 2,        // Backslashes don't quote special chars
  FILEMATCH_PERIOD      = 4,        // Leading `.' is matched only explicitly
  FILEMATCH_LEADING_DIR = 8,        // Ignore `/...' after a match
  FILEMATCH_CASEFOLD    = 16        // Compare without regard to case
  };


// Drag and drop actions
enum FXDragAction {
  DRAG_REJECT  = 0,                 // Reject all drop actions
  DRAG_ACCEPT  = 1,                 // Accept any drop action
  DRAG_COPY    = 2,                 // Copy
  DRAG_MOVE    = 3,                 // Move
  DRAG_LINK    = 4                  // Link
  };
  
  
/*********************************  Typedefs  **********************************/


class FXObject;
class FXStream;

// Streamable types; these are fixed size!
typedef unsigned char          FXuchar;
typedef char                   FXchar;
typedef FXuchar                FXbool;
typedef unsigned short         FXushort;
typedef short                  FXshort;
typedef unsigned int           FXuint;
typedef int                    FXint;
typedef float                  FXfloat;
typedef double                 FXdouble;
typedef FXObject              *FXObjectPtr;
#if defined(__MSC_VER) || defined(__BCPLUSPLUS__) || defined(__TCPLUSPLUS__)
#define FX_LONG 
typedef unsigned __int64       FXulong;
typedef __int64                FXlong;
#elif defined(__GNUG__) || defined(__GNUC__)
#define FX_LONG 
typedef unsigned long long int FXulong;
typedef long long int          FXlong;
#endif


// Handle to something in server
typedef unsigned int FXID;


// Opaque X-Window types
#ifndef X_PROTOCOL
struct Display;
struct Visual;
struct XFontStruct;
struct XVisualInfo;
struct XImage;
struct XColor;
struct XEvent;
typedef struct _XGC *GC;
#endif


// Pixel type (could be color index)
typedef unsigned long FXPixel;


// RGBA pixel value
typedef FXuint FXColor;


// Drag type
typedef FXID FXDragType;


// Hot key
typedef FXuint FXHotKey;


// Palette entry
struct FXPalEntry {
  FXPixel pixel;
  FXuchar red;
  FXuchar green;
  FXuchar blue;
  FXuchar alloced;
  };


// Named color
struct FXNamedColor {
  FXchar* name;
  FXuchar r,g,b;
  };
  

// Server time
typedef FXuint FXTime;


// Generic Item for lists of stuff
struct FXItem {
  FXItem *prev;
  FXItem *next;
  };


// Modal loop invocation
struct FXInvocation {
  FXInvocation *upper;              // Upper invocation
  FXID          window;             // Window to match
  FXbool        done;               // True if higher invocation is done
  };
  

// Item collate function
typedef FXbool (*FXItemSortFunc)(const FXItem*,const FXItem*);


// Context
typedef int FXContext;

// Rectangle
struct FXRectangle { FXshort x,y,w,h; };

// Point
struct FXPoint { FXshort x,y; };

// Line segment
struct FXSegment { FXshort x1,y1,x2,y2; };

// Arc
struct FXArc { FXshort x,y,w,h,a,b; };


// FOX Event 
struct FXEvent {
  FXuint      type;           // Event type
  FXID        window;         // Window
  FXTime      time;           // Time of last event
  FXint       win_x;          // Window-relative x-coord
  FXint       win_y;          // Window-relative y-coord
  FXint       root_x;         // Root x-coord
  FXint       root_y;         // Root y-coord
  FXuint      state;          // Keyboard/Modifier state
  FXuint      code;           // Button, Keysym, or mode
  FXint       last_x;         // Window-relative x-coord of previous mouse location
  FXint       last_y;         // Window-relative y-coord of previous mouse location
  FXint       click_x;        // Window-relative x-coord of mouse button press
  FXint       click_y;        // Window-relative y-coord of mouse press
  FXTime      click_time;     // Time of mouse button press
  FXuint      click_button;   // Mouse button pressed
  FXID        click_window;   // Window of mouse button press
  FXint       click_count;    // Click-count
  FXbool      moved;          // Moved cursor since press
  FXRectangle rect;           // Rectangle
  FXbool      synthetic;      // True if synthetic expose event
  };
  

/**********************************  Macros  ***********************************/

  
// Abolute value
#define FXABS(val) (((val)>=0)?(val):-(val))

// Min and Max
#define FXMAX(a,b) (((a)>(b))?(a):(b))
#define FXMIN(a,b) (((a)>(b))?(b):(a))

// Min of three
#define FXMIN3(x,y,z) ((x)<(y)?FXMIN(x,z):FXMIN(y,z))

// Max of three
#define FXMAX3(x,y,z) ((x)>(y)?FXMAX(x,z):FXMAX(y,z))


// Return minimum and maximum of a, b
#define FXMINMAX(lo,hi,a,b) ((a)<(b)?((lo)=(a),(hi)=(b)):((lo)=(b),(hi)=(a)))

// Offset of member in a structure 
#define STRUCTOFFSET(str,member) (((char *)(&(((str *)0)->member)))-((char *)0))


// Number of elements in a static array 
#define ARRAYNUMBER(array) (sizeof(array)/sizeof(array[0]))


// Container class of a member class 
#define CONTAINER(ptr,str,mem) ((str*)(((char*)(ptr))-STRUCTOFFSET(str,mem)))


// Make int out of two shorts 
#define MKUINT(l,h) ((unsigned int)(((unsigned short)(l))|((unsigned int)((unsigned short)(h)))<<16))


// Get type from selector
#define SELTYPE(s) (((unsigned int)(s))>>16)


// Get ID from selector
#define SELID(s) ((unsigned int)((unsigned short)(s)))


// Make RGBA color
#define FXRGBA(r,g,b,a) (((FXuint)(FXuchar)(r)) | ((FXuint)(FXuchar)(g)<<8) | ((FXuint)(FXuchar)(b)<<16) | ((FXuint)(FXuchar)(a)<<24))

// Make RGB color
#define FXRGB(r,g,b) (((FXuint)(FXuchar)(r)) | ((FXuint)(FXuchar)(g)<<8) | ((FXuint)(FXuchar)(b)<<16) | 0xff000000)

// Get alpha value from RGBA color
#define FXALPHAVAL(rgba)   ((FXuchar)((FXuint)(rgba)>>24))

// Get blue value from RGBA color
#define FXBLUEVAL(rgba) ((FXuchar)((FXuint)(rgba)>>16))

// Get green value from RGBA color
#define FXGREENVAL(rgba)  ((FXuchar)((FXuint)(rgba)>>8))

// Get red value from RGBA color
#define FXREDVAL(rgba) ((FXuchar)(rgba))

// Get component value of RGBA color
#define FXRGBACOMPVAL(rgba,comp) ((FXuchar)((rgba)>>((comp)<<3)))


// Assertion
#ifndef NDEBUG
#define FXASSERT(exp) ((exp)?((void)0):(void)fxassert(#exp,__FILE__,__LINE__))
#else
#define FXASSERT(exp) ((void)0)
#endif



// Memory allocation
#define FXMALLOC(ptr,type,no)     (fxmalloc((void **)(ptr),sizeof(type)*(no)))
#define FXCALLOC(ptr,type,no)     (fxcalloc((void **)(ptr),sizeof(type)*(no)))
#define FXRESIZE(ptr,type,no)     (fxresize((void **)(ptr),sizeof(type)*(no)))
#define FXFREE(ptr)               (fxfree((void **)(ptr)))


/**********************************  Globals  **********************************/

// Allocate memory
extern FXint fxmalloc(void** ptr,unsigned long size);

// Allocate cleaned memory
extern FXint fxcalloc(void** ptr,unsigned long size);

// Resize memory
extern FXint fxresize(void** ptr,unsigned long size);

// Free memory, resets ptr to NULL afterward
extern void fxfree(void** ptr);

// Assert failed routine
extern void fxassert(const char* expression,const char* filename,unsigned int lineno);

// Error routine
extern void fxerror(const char* format,...);

// Warning routine
extern void fxwarning(const char* format,...);

// Sleep n microseconds
extern void fxsleep(unsigned int n);

// Key character value of keysym + modifier state
extern FXchar fxkeyval(FXuint code,FXuint state=0);

// Sort a doubly linked list of stuff
extern void fxsort(FXItem*& f1,FXItem*& t1,FXItem*& f2,FXItem*&t2,FXItemSortFunc greater,FXint n);

// Match a file name with a pattern
extern FXint fxfilematch(const char *pattern,const char *string,FXuint flags=(FILEMATCH_NOESCAPE|FILEMATCH_FILE_NAME));

// Parse for accelerator key codes in a string
extern FXHotKey fxparseaccel(const FXchar* s);

// Parse for hot key codes in a string
extern FXHotKey fxparsehotkey(const FXchar* s);

// Locate hot key underline offset from begin of string
extern FXint fxfindhotkeyoffset(const FXchar* s);

// Get highlight color
extern FXColor makeHiliteColor(FXColor clr);

// Get shadow color
extern FXColor makeShadowColor(FXColor clr);

// Get user name from uid
extern FXchar* fxgetusername(FXchar* result,FXuint uid);

// Get group name from gid
extern FXchar* fxgetgroupname(FXchar* result,FXuint gid);

// Get permissions string from mode
extern FXchar* fxgetpermissions(FXchar* result,FXuint mode);

// Return TRUE iff s is a prefix of t
extern FXbool fxprefix(const FXchar* s,const FXchar* t);

// Return TRUE iff s is a suffix of t
extern FXbool fxsuffix(const FXchar* s, const FXchar* t);

// Expand ~ in filenames; using HOME environment variable
extern FXchar* fxexpand(FXchar* result,const FXchar* name);

// Construct a full pathname from the given directory and file name
extern FXchar* fxpathname(FXchar* result,const FXchar* dirname,const FXchar* filename);

// Return the directory part of pathname
extern FXchar* fxdirpart(FXchar* result,const FXchar* pathname);

// Return the filename part of pathname
extern FXchar* fxfilepart(FXchar* result,const FXchar* pathname);

// Return the file extension part of pathname
extern FXchar* fxfileext(FXchar* result,const FXchar* pathname);

// Check whether a file exists
extern FXbool fxexists(const FXchar *name);

// Create a unique numbered backup file name for the given pathname 
extern FXchar* fxbakname(FXchar* result,const FXchar* pathname);

// Check whether two files are identical (refer to the same inode)
extern FXbool fxidentical(const FXchar *name1,const FXchar *name2);

// Split a string into substrings delimited by a given character
extern FXchar *fxsplit(FXchar*& s,FXchar c);

// Return the shortest path equivalent to pathname (remove . and ..)
extern FXchar *fxshortestpath(FXchar *result,const FXchar *pathname);

// Return directory one level above given one
extern FXchar* fxupdir(FXchar* result,const FXchar *dirname);

// Translate filename to an absolute pathname; ~ in filename is expanded,
// and if the resulting pathname is still relative, basename is prepended
extern FXchar *fxabspath(FXchar *result,const FXchar *basename,const FXchar *filename);

// Test if path is the toplevel directory
extern FXbool fxistopdir(const FXchar* path);

// Get current working directory
extern FXchar* fxgetcurrentdir(FXchar *result);

// Get home directory
extern FXchar* fxgethomedir(FXchar *result);

// Search path for file name. A ~ in path is expanded. If name is absolute
// it is returned unchanged. Otherwise the absolute name is returned in
// result. If name is not found on path, NULL is returned. 
extern FXchar *fxsearchpath(FXchar *result,const FXchar *path,const FXchar *name);

// Get RGB value from color name
extern FXColor fxcolorfromname(const FXchar* colorname);

// Get name of (closest) color to RGB
extern const FXchar* fxnamefromcolor(FXColor rgb);

// Load a gif file from a stream
extern FXbool fxloadGIF(FXStream& store,FXuchar*& data,FXColor& transp,FXint& width,FXint& height);

// Save a gif file to a stream
extern FXbool fxsaveGIF(FXStream& store,const FXuchar *const& data,const FXColor& transp,const FXint& width,const FXint& height);

// Load a bmp file from a stream
extern FXbool fxloadBMP(FXStream& store,FXuchar*& data,FXColor& transp,FXint& width,FXint& height);

// Save a bmp file to a stream
extern FXbool fxsaveBMP(FXStream& store,const FXuchar *const& data,const FXColor& transp,const FXint& width,const FXint& height);

// Convert RGB to HSV
extern void fxrgb_to_hsv(FXfloat& h,FXfloat& s,FXfloat& v,FXfloat r,FXfloat g,FXfloat b);

// Convert HSV to RGB
extern void fxhsv_to_rgb(FXfloat& r,FXfloat& g,FXfloat& b,FXfloat h,FXfloat s,FXfloat v);

// Calculate a hash value from a string
extern FXuint fxstrhash(const FXchar* str);

// List of color names
extern const FXNamedColor fxcolornames[];

// Number of color names
extern const FXuint fxnumcolornames;

#endif
