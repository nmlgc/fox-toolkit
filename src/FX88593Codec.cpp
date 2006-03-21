#include "xincs.h"
#include "fxver.h"
#include "fxdefs.h"
#include "FXHash.h"
#include "FXStream.h"
#include "FXTextCodec.h"
#include "FX88593Codec.h"

namespace FX {

FXIMPLEMENT(FX88593Codec,FXTextCodec,NULL,0)


//// Created by codec tool on 03/25/2005 from: 8859-3.TXT ////
const unsigned short forward_data[256]={
   0,    1,    2,    3,    4,    5,    6,    7,    8,    9,    10,   11,   12,   13,   14,   15,
   16,   17,   18,   19,   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,   30,   31,
   32,   33,   34,   35,   36,   37,   38,   39,   40,   41,   42,   43,   44,   45,   46,   47,
   48,   49,   50,   51,   52,   53,   54,   55,   56,   57,   58,   59,   60,   61,   62,   63,
   64,   65,   66,   67,   68,   69,   70,   71,   72,   73,   74,   75,   76,   77,   78,   79,
   80,   81,   82,   83,   84,   85,   86,   87,   88,   89,   90,   91,   92,   93,   94,   95,
   96,   97,   98,   99,   100,  101,  102,  103,  104,  105,  106,  107,  108,  109,  110,  111,
   112,  113,  114,  115,  116,  117,  118,  119,  120,  121,  122,  123,  124,  125,  126,  127,
   128,  129,  130,  131,  132,  133,  134,  135,  136,  137,  138,  139,  140,  141,  142,  143,
   144,  145,  146,  147,  148,  149,  150,  151,  152,  153,  154,  155,  156,  157,  158,  159,
   160,  294,  728,  163,  164,  65533, 292,  167,  168,  304,  350,  286,  308,  173,  65533, 379,
   176,  295,  178,  179,  180,  181,  293,  183,  184,  305,  351,  287,  309,  189,  65533, 380,
   192,  193,  194,  65533, 196,  266,  264,  199,  200,  201,  202,  203,  204,  205,  206,  207,
   65533, 209,  210,  211,  212,  288,  214,  215,  284,  217,  218,  219,  220,  364,  348,  223,
   224,  225,  226,  65533, 228,  267,  265,  231,  232,  233,  234,  235,  236,  237,  238,  239,
   65533, 241,  242,  243,  244,  289,  246,  247,  285,  249,  250,  251,  252,  365,  349,  729,
  };


const unsigned char reverse_plane[17]={
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  };

const unsigned char reverse_pages[65]={
  0,  46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46,
  46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46,
  46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46,
  46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46,
  46,
  };

const unsigned short reverse_block[110]={
  0,   16,  32,  48,  64,  80,  96,  112, 128, 144, 160, 176, 192, 208, 224, 240,
  253, 265, 281, 297, 303, 307, 323, 337, 303, 303, 303, 303, 303, 303, 303, 303,
  303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 350, 303, 303,
  303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 303,
  303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 303,
  303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 303,
  303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 303, 303,
  };

const unsigned char reverse_data[366]={
   0,    1,    2,    3,    4,    5,    6,    7,    8,    9,    10,   11,   12,   13,   14,   15,
   16,   17,   18,   19,   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,   30,   31,
   32,   33,   34,   35,   36,   37,   38,   39,   40,   41,   42,   43,   44,   45,   46,   47,
   48,   49,   50,   51,   52,   53,   54,   55,   56,   57,   58,   59,   60,   61,   62,   63,
   64,   65,   66,   67,   68,   69,   70,   71,   72,   73,   74,   75,   76,   77,   78,   79,
   80,   81,   82,   83,   84,   85,   86,   87,   88,   89,   90,   91,   92,   93,   94,   95,
   96,   97,   98,   99,   100,  101,  102,  103,  104,  105,  106,  107,  108,  109,  110,  111,
   112,  113,  114,  115,  116,  117,  118,  119,  120,  121,  122,  123,  124,  125,  126,  127,
   128,  129,  130,  131,  132,  133,  134,  135,  136,  137,  138,  139,  140,  141,  142,  143,
   144,  145,  146,  147,  148,  149,  150,  151,  152,  153,  154,  155,  156,  157,  158,  159,
   160,  26,   26,   163,  164,  26,   26,   167,  168,  26,   26,   26,   26,   173,  26,   26,
   176,  26,   178,  179,  180,  181,  26,   183,  184,  26,   26,   26,   26,   189,  26,   26,
   192,  193,  194,  26,   196,  26,   26,   199,  200,  201,  202,  203,  204,  205,  206,  207,
   26,   209,  210,  211,  212,  26,   214,  215,  26,   217,  218,  219,  220,  26,   26,   223,
   224,  225,  226,  26,   228,  26,   26,   231,  232,  233,  234,  235,  236,  237,  238,  239,
   26,   241,  242,  243,  244,  26,   246,  247,  26,   249,  250,  251,  252,  26,   26,   26,
   26,   26,   26,   26,   26,   198,  230,  197,  229,  26,   26,   26,   26,   26,   26,   26,
   26,   26,   26,   26,   26,   216,  248,  171,  187,  213,  245,  26,   26,   166,  182,  161,
   177,  26,   26,   26,   26,   26,   26,   26,   26,   169,  185,  26,   26,   172,  188,  26,
   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   222,
   254,  170,  186,  26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   221,
   253,  26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   26,   175,  191,  26,   26,
   26,   26,   26,   26,   26,   26,   162,  255,  26,   26,   26,   26,   26,   26,
  };


FXint FX88593Codec::mb2wc(FXwchar& wc,const FXchar* src,FXint nsrc) const {
  if(nsrc<1) return -1;
  wc=forward_data[(FXuchar)src[0]];
  return 1;
  }


FXint FX88593Codec::wc2mb(FXchar* dst,FXint ndst,FXwchar wc) const {
  if(ndst<1) return -1;
  dst[0]=reverse_data[reverse_block[reverse_pages[reverse_plane[wc>>16]+((wc>>10)&63)]+((wc>>4)&63)]+(wc&15)];
  return 1;
  }

FXint FX88593Codec::mibEnum() const {
  return 6;
  }


const FXchar* FX88593Codec::name() const {
  return "ISO-8859-3";
  }


const FXchar* FX88593Codec::mimeName() const {
  return "ISO-8859-3";
  }


const FXchar* const* FX88593Codec::aliases() const {
  static const FXchar *const list[]={"iso8859-3","ISO-8859-3","ISO_8859-3","latin3","iso-ir-109","l3","csISOLatin3",NULL};
  return list;
  }

}
