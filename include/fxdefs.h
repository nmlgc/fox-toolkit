/********************************************************************************
*                                                                               *
*                     FOX Definitions, Types, and Macros                        *
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
* $Id: fxdefs.h,v 1.95 2002/02/27 02:27:36 fox Exp $                            *
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

/// Euler constant
#define EULER   2.7182818284590452353602874713

/// Multiplier for degrees to radians
#define DTOR    0.0174532925199432957692369077

/// Multipier for radians to degrees
#define RTOD    57.295779513082320876798154814


// Path separator
#ifdef WIN32
#define PATHSEP '\\'
#define PATHSEPSTRING "\\"
#define PATHLISTSEP ';'
#define PATHLISTSEPSTRING ";"
#define ISPATHSEP(c) ((c)=='/' || (c)=='\\')
#else
#define PATHSEP '/'
#define PATHSEPSTRING "/"
#define PATHLISTSEP ':'
#define PATHLISTSEPSTRING ":"
#define ISPATHSEP(c) ((c)=='/')
#endif


// For Windows
#ifdef _DEBUG
#ifndef DEBUG
#define DEBUG
#endif
#endif
#ifdef _NDEBUG
#ifndef NDEBUG
#define NDEBUG
#endif
#endif


// Shared library support
#ifdef WIN32
#ifdef FOXDLL
#ifdef FOXDLL_EXPORTS
#define FXAPI __declspec(dllexport)
#else
#define FXAPI __declspec(dllimport)
#endif
#endif
#endif

#ifndef FXAPI
#define FXAPI
#endif


// Callback
#ifdef WIN32
#ifndef CALLBACK
#define CALLBACK __stdcall
#endif
#endif


// Checking printf and scanf format strings
#if defined(_CC_GNU_) || defined(__GNUG__) || defined(__GNUC__)
#define FX_PRINTF(fmt,arg) __attribute__((format(printf,fmt,arg)))
#define FX_SCANF(fmt,arg)  __attribute__((format(scanf,fmt,arg)))
#else
#define FX_PRINTF(fmt,arg)
#define FX_SCANF(fmt,arg)
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
  SEL_UNGRABBED,                      // Lost the grab (Windows)
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
  SEL_CLOSE,                          // Close window
  SEL_CLOSEALL,                       // Close all windows
  SEL_DELETE,                         // Delete window
  SEL_MINIMIZE,                       // Iconified
  SEL_RESTORE,                        // No longer iconified or maximized
  SEL_MAXIMIZE,                       // Maximized
  SEL_UPDATE,                         // GUI update
  SEL_COMMAND,                        // GUI command
  SEL_CLICKED,                        // Clicked
  SEL_DOUBLECLICKED,                  // Double-clicked
  SEL_TRIPLECLICKED,                  // Triple-clicked
  SEL_MOUSEWHEEL,                     // Mouse wheel
  SEL_CHANGED,                        // GUI has changed
  SEL_VERIFY,                         // Verify change
  SEL_DESELECTED,                     // Deselected
  SEL_SELECTED,                       // Selected
  SEL_INSERTED,                       // Inserted
  SEL_REPLACED,                       // Replaced
  SEL_DELETED,                        // Deleted
  SEL_OPENED,                         // Opened
  SEL_CLOSED,                         // Closed
  SEL_EXPANDED,                       // Expanded
  SEL_COLLAPSED,                      // Collapsed
  SEL_BEGINDRAG,                      // Start a drag
  SEL_ENDDRAG,                        // End a drag
  SEL_DRAGGED,                        // Dragged
  SEL_LASSOED,                        // Lassoed
  SEL_TIMEOUT,                        // Timeout occurred
  SEL_SIGNAL,                         // Signal received
  SEL_CLIPBOARD_LOST,                 // Widget lost clipboard
  SEL_CLIPBOARD_GAINED,               // Widget gained clipboard
  SEL_CLIPBOARD_REQUEST,              // Inquire clipboard data
  SEL_CHORE,                          // Background chore
  SEL_FOCUS_SELF,                     // Focus on widget itself
  SEL_FOCUS_RIGHT,                    // Focus movements
  SEL_FOCUS_LEFT,
  SEL_FOCUS_DOWN,
  SEL_FOCUS_UP,
  SEL_FOCUS_NEXT,
  SEL_FOCUS_PREV,
  SEL_DND_ENTER,                      // Drag action entering potential drop target
  SEL_DND_LEAVE,                      // Drag action leaving potential drop target
  SEL_DND_DROP,                       // Drop on drop target
  SEL_DND_MOTION,                     // Drag position changed over potential drop target
  SEL_DND_REQUEST,                    // Inquire drag and drop data
  SEL_UNCHECK_OTHER,                  // Sent by child to parent to uncheck other children
  SEL_UNCHECK_RADIO,                  // Sent by parent to uncheck radio children
  SEL_IO_READ,                        // Read activity on a pipe
  SEL_IO_WRITE,                       // Write activity on a pipe
  SEL_IO_EXCEPT,                      // Except activity on a pipe
  SEL_PICKED,                         // Picked some location
  SEL_LAST                            // Last message
  };


/// FOX Keyboard and Button states
enum {
  SHIFTMASK        = 0x001,           /// Shift key is down
  CAPSLOCKMASK     = 0x002,           /// Caps Lock key is down
  CONTROLMASK      = 0x004,           /// Ctrl key is down
  ALTMASK          = 0x008,           /// Alt key is down
  NUMLOCKMASK      = 0x010,           /// Num Lock key is down
  SCROLLLOCKMASK   = 0x0E0,           /// Scroll Lock key is down (seems to vary)
  LEFTBUTTONMASK   = 0x100,           /// Left mouse button is down
  MIDDLEBUTTONMASK = 0x200,           /// Middle mouse button is down
  RIGHTBUTTONMASK  = 0x400            /// Right mouse button is down
  };


/// FOX Mouse buttons
enum {
  LEFTBUTTON       = 1,
  MIDDLEBUTTON     = 2,
  RIGHTBUTTON      = 3
  };


/// FOX window crossing modes
enum {
  CROSSINGNORMAL,		     /// Normal crossing event
  CROSSINGGRAB,			     /// Crossing due to mouse grab
  CROSSINGUNGRAB		     /// Crossing due to mouse ungrab
  };


/// FOX window visibility modes
enum {
  VISIBILITYTOTAL,
  VISIBILITYPARTIAL,
  VISIBILITYNONE
  };


/// Options for fxfilematch
enum {
  FILEMATCH_FILE_NAME   = 1,        /// No wildcard can ever match `/'
  FILEMATCH_NOESCAPE    = 2,        /// Backslashes don't quote special chars
  FILEMATCH_PERIOD      = 4,        /// Leading `.' is matched only explicitly
  FILEMATCH_LEADING_DIR = 8,        /// Ignore `/...' after a match
  FILEMATCH_CASEFOLD    = 16        /// Compare without regard to case
  };


/// Drag and drop actions
enum FXDragAction {
  DRAG_REJECT  = 0,                 /// Reject all drop actions
  DRAG_ACCEPT  = 1,                 /// Accept any drop action
  DRAG_COPY    = 2,                 /// Copy
  DRAG_MOVE    = 3,                 /// Move
  DRAG_LINK    = 4,                 /// Link
  DRAG_PRIVATE = 5                  /// Private
  };


/// Origin of data
enum FXDNDOrigin {
  FROM_SELECTION  = 0,              /// Primary selection
  FROM_CLIPBOARD  = 1,              /// Clipboard
  FROM_DRAGNDROP  = 2               /// Drag and drop source
  };


/// Exponent display
enum FXExponent {
  EXP_NEVER=FALSE,                  /// Never use exponential notation
  EXP_ALWAYS=TRUE,                  /// Always use exponential notation
  EXP_AUTO=MAYBE                    /// Use exponential notation if needed
  };


/// Search modes for search/replace dialogs
enum {
  SEARCH_FORWARD      = 0,    /// Search forward (default)
  SEARCH_BACKWARD     = 1,    /// Search backward
  SEARCH_NOWRAP       = 0,    /// Don't wrap (default)
  SEARCH_WRAP         = 2,    /// Wrap around to start
  SEARCH_EXACT        = 0,    /// Exact match (default)
  SEARCH_IGNORECASE   = 4,    /// Ignore case
  SEARCH_REGEX        = 8,    /// Regular expression match
  SEARCH_PREFIX       = 16    /// Prefix of subject string
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
#if defined(_MSC_VER) || (defined(__BCPLUSPLUS__) && __BORLANDC__ > 0x500)
#define FX_LONG
typedef unsigned __int64       FXulong;
typedef __int64                FXlong;
#elif defined(__GNUG__) || defined(__GNUC__) || defined(__SUNPRO_CC)
#define FX_LONG
typedef unsigned long long int FXulong;
typedef long long int          FXlong;
#endif


// Handle to something in server
#ifndef WIN32
typedef unsigned int           FXID;
#else
typedef void*                  FXID;
#endif

// Time since January 1, 1970 (UTC)
typedef long                   FXTime;

// Pixel type (could be color index)
typedef unsigned long          FXPixel;

// RGBA pixel value
typedef FXuint                 FXColor;

// Hot key
typedef FXuint                 FXHotKey;

// Drag type
#ifndef WIN32
typedef FXID                   FXDragType;
#else
typedef FXushort               FXDragType;
#endif

// Raw event type
#ifndef WIN32
union _XEvent;
typedef _XEvent                FXRawEvent;
#else
struct tagMSG;
typedef tagMSG                 FXRawEvent;
#endif

// Input source handle type
#ifndef WIN32
typedef FXint                  FXInputHandle;
#else
typedef void*                  FXInputHandle;
#endif

/**********************************  Macros  ***********************************/


/// Abolute value
#define FXABS(val) (((val)>=0)?(val):-(val))

/// Return the maximum of a or b
#define FXMAX(a,b) (((a)>(b))?(a):(b))

/// Return the minimum of a or b
#define FXMIN(a,b) (((a)>(b))?(b):(a))

/// Return the minimum of x, y and z
#define FXMIN3(x,y,z) ((x)<(y)?FXMIN(x,z):FXMIN(y,z))

/// Return the maximum of x, y and z
#define FXMAX3(x,y,z) ((x)>(y)?FXMAX(x,z):FXMAX(y,z))

/// Return minimum and maximum of a, b
#define FXMINMAX(lo,hi,a,b) ((a)<(b)?((lo)=(a),(hi)=(b)):((lo)=(b),(hi)=(a)))

/// Clamp value x to range [lo..hi]
#define FXCLAMP(lo,x,hi) ((x)<(lo)?(lo):((x)>(hi)?(hi):(x)))

/// Swap a pair of numbers
#define FXSWAP(a,b,t) ((t)=(a),(a)=(b),(b)=(t))

/// Linear interpolation between a and b, where 0<=f<=1
#define FXLERP(a,b,f) ((a)+((b)-(a))*(f))

/// Offset of member in a structure
#define STRUCTOFFSET(str,member) (((char *)(&(((str *)0)->member)))-((char *)0))

/// Number of elements in a static array
#define ARRAYNUMBER(array) (sizeof(array)/sizeof(array[0]))

/// Container class of a member class
#define CONTAINER(ptr,str,mem) ((str*)(((char*)(ptr))-STRUCTOFFSET(str,mem)))

/// Make int out of two shorts
#define MKUINT(l,h) ((((FXuint)(l))&0xffff) | (((FXuint)(h))<<16))

/// Get type from selector
#define SELTYPE(s) ((FXushort)(((s)>>16)&0xffff))

/// Get ID from selector
#define SELID(s) ((FXushort)((s)&0xffff))

/// Reverse bits in byte
#define FXBITREVERSE(b) (((b&0x01)<<7)|((b&0x02)<<5)|((b&0x04)<<3)|((b&0x08)<<1)|((b&0x10)>>1)|((b&0x20)>>3)|((b&0x40)>>5)|((b&0x80)>>7))

// The order in memory is [R G B A] matches that in FXColor

// Definitions for big-endian machines
#if FOX_BIGENDIAN == 1

/// Make RGBA color
#define FXRGBA(r,g,b,a)    (((FXuint)(FXuchar)(r)<<24) | ((FXuint)(FXuchar)(g)<<16) | ((FXuint)(FXuchar)(b)<<8) | ((FXuint)(FXuchar)(a)))

/// Make RGB color
#define FXRGB(r,g,b)       (((FXuint)(FXuchar)(r)<<24) | ((FXuint)(FXuchar)(g)<<16) | ((FXuint)(FXuchar)(b)<<8) | 0x000000ff)

/// Get red value from RGBA color
#define FXREDVAL(rgba)     ((FXuchar)(((rgba)>>24)&0xff))

/// Get green value from RGBA color
#define FXGREENVAL(rgba)   ((FXuchar)(((rgba)>>16)&0xff))

/// Get blue value from RGBA color
#define FXBLUEVAL(rgba)    ((FXuchar)(((rgba)>>8)&0xff))

/// Get alpha value from RGBA color
#define FXALPHAVAL(rgba)   ((FXuchar)((rgba)&0xff))

/// Get component value of RGBA color
#define FXRGBACOMPVAL(rgba,comp) ((FXuchar)(((rgba)>>((3-(comp))<<3))&0xff))

#endif

// Definitions for little-endian machines
#if FOX_BIGENDIAN == 0

/// Make RGBA color
#define FXRGBA(r,g,b,a)    (((FXuint)(FXuchar)(r)) | ((FXuint)(FXuchar)(g)<<8) | ((FXuint)(FXuchar)(b)<<16) | ((FXuint)(FXuchar)(a)<<24))

/// Make RGB color
#define FXRGB(r,g,b)       (((FXuint)(FXuchar)(r)) | ((FXuint)(FXuchar)(g)<<8) | ((FXuint)(FXuchar)(b)<<16) | 0xff000000)

/// Get red value from RGBA color
#define FXREDVAL(rgba)     ((FXuchar)((rgba)&0xff))

/// Get green value from RGBA color
#define FXGREENVAL(rgba)   ((FXuchar)(((rgba)>>8)&0xff))

/// Get blue value from RGBA color
#define FXBLUEVAL(rgba)    ((FXuchar)(((rgba)>>16)&0xff))

/// Get alpha value from RGBA color
#define FXALPHAVAL(rgba)   ((FXuchar)(((rgba)>>24)&0xff))

/// Get component value of RGBA color
#define FXRGBACOMPVAL(rgba,comp) ((FXuchar)(((rgba)>>((comp)<<3))&0xff))

#endif


/**
* FXASSERT() prints out a message when the expression fails,
* and nothing otherwise.  Unlike assert(), FXASSERT() will not
* terminate the execution of the application.
* When compiling your application for release, all assertions
* are compiled out; thus there is no impact on execution speed.
*/
#ifndef NDEBUG
#define FXASSERT(exp) ((exp)?((void)0):(void)fxassert(#exp,__FILE__,__LINE__))
#else
#define FXASSERT(exp) ((void)0)
#endif


/**
* FXTRACE() allows you to trace the execution of your application
* with increasing levels of detail the higher the trace level.
* The trace level is determined by variable fxTraceLevel, which
* may be set from the command line with "-tracelevel <level>".
* When compiling your application for release, all trace statements
* are compiled out, just like FXASSERT.
* A statement like: FXTRACE((10,"The value of x=%d\n",x)) will
* generate output only if fxTraceLevel is set to 11 or greater.
* The default value fxTraceLevel=0 will block all trace outputs.
* Note the double parentheses!
*/
#ifndef NDEBUG
#define FXTRACE(arguments) fxtrace arguments
#else
#define FXTRACE(arguments) ((void)0)
#endif


/// Allocate no elements of type to the specified pointer
#define FXMALLOC(ptr,type,no)     (fxmalloc((void **)(ptr),sizeof(type)*(no)))

/// Allocate cleared memory
#define FXCALLOC(ptr,type,no)     (fxcalloc((void **)(ptr),sizeof(type)*(no)))

/// Resize a previously allocated block of memory
#define FXRESIZE(ptr,type,no)     (fxresize((void **)(ptr),sizeof(type)*(no)))

/// Allocate and initialize memory
#define FXMEMDUP(ptr,type,src,no) (fxmemdup((void **)(ptr),sizeof(type)*(no),(const void*)(src)))

/// Free a block of memory allocated with either FXMALLOC or FXCALLOC
#define FXFREE(ptr)               (fxfree((void **)(ptr)))


/**********************************  Globals  **********************************/


/// Simple, thread-safe, random number generator
extern FXAPI FXuint fxrandom(FXuint& seed);

/// Allocate memory
extern FXAPI FXint fxmalloc(void** ptr,unsigned long size);

/// Allocate cleaned memory
extern FXAPI FXint fxcalloc(void** ptr,unsigned long size);

/// Resize memory
extern FXAPI FXint fxresize(void** ptr,unsigned long size);

/// Duplicate memory
extern FXAPI FXint fxmemdup(void** ptr,unsigned long size,const void* src);

/// Free memory, resets ptr to NULL afterward
extern FXAPI void fxfree(void** ptr);

/// Error routine
extern FXAPI void fxerror(const char* format,...) FX_PRINTF(1,2) ;

/// Warning routine
extern FXAPI void fxwarning(const char* format,...) FX_PRINTF(1,2) ;

/// Log message to [typically] stderr
extern FXAPI void fxmessage(const char* format,...) FX_PRINTF(1,2) ;

/// Assert failed routine:- usually not called directly but called through FXASSERT
extern FXAPI void fxassert(const char* expression,const char* filename,unsigned int lineno);

/// Trace printout routine:- usually not called directly but called through FXTRACE
extern FXAPI void fxtrace(unsigned int level,const char* format,...) FX_PRINTF(2,3) ;

/// Sleep n microseconds
extern FXAPI void fxsleep(unsigned int n);

/// Match a file name with a pattern
extern FXAPI FXint fxfilematch(const char *pattern,const char *string,FXuint flags=(FILEMATCH_NOESCAPE|FILEMATCH_FILE_NAME));

/// Parse for accelerator key codes in a string
extern FXAPI FXHotKey fxparseaccel(const FXchar* s);

/// Parse for hot key codes in a string
extern FXAPI FXHotKey fxparsehotkey(const FXchar* s);

/// Locate hot key underline offset from begin of string
extern FXAPI FXint fxfindhotkeyoffset(const FXchar* s);

/// Get highlight color
extern FXAPI FXColor makeHiliteColor(FXColor clr);

/// Get shadow color
extern FXAPI FXColor makeShadowColor(FXColor clr);

/// Get user name from uid
extern FXAPI FXchar* fxgetusername(FXchar* result,FXuint uid);

/// Get group name from gid
extern FXAPI FXchar* fxgetgroupname(FXchar* result,FXuint gid);

/// Get permissions string from mode.
extern FXAPI FXchar* fxgetpermissions(FXchar* result,FXuint mode);

/// Get process id
extern FXAPI FXint fxgetpid();

/// Duplicate string
extern FXAPI FXchar *fxstrdup(const FXchar* str);

/// Calculate a hash value from a string
extern FXAPI FXint fxstrhash(const FXchar* str);

/// Get RGB value from color name
extern FXAPI FXColor fxcolorfromname(const FXchar* colorname);

/// Get name of (closest) color to RGB
extern FXAPI FXchar* fxnamefromcolor(FXchar *colorname,FXColor color);

/// Convert RGB to HSV
extern FXAPI void fxrgb_to_hsv(FXfloat& h,FXfloat& s,FXfloat& v,FXfloat r,FXfloat g,FXfloat b);

/// Convert HSV to RGB
extern FXAPI void fxhsv_to_rgb(FXfloat& r,FXfloat& g,FXfloat& b,FXfloat h,FXfloat s,FXfloat v);

/// Floating point number classification: 0=OK, +/-1=Inf, +/-2=NaN
extern FXAPI FXint fxieeefloatclass(FXfloat number);
extern FXAPI FXint fxieeedoubleclass(FXdouble number);


/// Version number that the library has been compiled with
extern FXAPI const FXuchar fxversion[3];


/// Controls tracing level
extern FXAPI unsigned int fxTraceLevel;


#endif
