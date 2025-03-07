
#include <libeth/fidel.h>
#include <libeth/libeth.h>

#define HEADER     1
#define RTFLINE    2
#define PLAINLINE  3

/* from fidel.map in libeth */
extern ECHAR AgafariFidel[], ALXEtFidel[], FeedelFidel[], GeezEditFidel[], WashraFidel[], GeezFreeFidel[];


#define NUMFONTS 20
#define NUMSYSTEMS 6
enum Fonts {FagafariZ, FagafariET, FagafariR, FagafariYB, Falxet, Ffeedel, Fgezedit, Fwashra, Fwashrax, Fwashrasl, Fwashraxsl, Fethiopia, Fethiopiax, Fethiopiasl, Fethiopiaxsl, Fyebse, Fyebsex, Fgezfree1, Fgezfree2, Froman};


/**
 *  This structure should contain all useful info that a 
 *  font should know about itself.
 */

struct FontInfo {
  int numSets;                 /* how many in my group                     */
  int mySetNum;                /* which one am I?                          */
  unsigned char* setName;      /* my TT name, found in font menus and rtf files */ 

  char* rtfId;                  /* my \f# in an rtf file                   */
  char* rtfPrefix;              /* do I also have a \fcharset2 ?           */

  ECHAR (*myMap)[];             /* my map onto Unicode                     */

  enum Fonts setId;             /* an enumerated version of setName        */
  enum CharacterSet charCode;   /* my character coding system              */
  enum Scripts script;          /* the script I am used for                */

  struct FontInfo* nextSet;     /* the next font in my group               */
  struct FontInfo* lastSet;     /* the previous font in my group           */
};


struct FontInfoList {
  struct FontInfo* topIn;                 /* the current font of the current script (writing  in) */
  struct FontInfo* topOut;                /* the current font of the current script (writing out) */
  enum Fonts Fout;                        /* User requested out bound font Id       */
  struct FontInfo* font[NUMFONTS];        /* pointers to all fonts we know about    */
  struct FontInfo* changeList[NUMFONTS];  /* points only to those we are updating,  */
                                          /*   end of list is marked by NULL        */
};


extern void rtf2rtf  ();
extern void rtf2plain  ();
extern void plain2rtf  (); 
extern void plain2plain  ();
#ifdef  SUPPORT_TEX
  extern void rtf2tex  ();
  extern void plain2tex  ();
#endif /* SUPPORT_TEX */

typedef struct {
  void (*func)();
  enum CharacterSet  in;
  enum CharacterSet out;
} gezOutPuts;  

                 /* ---------------------------------------- */
                 /* function name,  System In,  System Out   */
                 /* ---------------------------------------- */
#define FUNCTION_LIST { \
                      { rtf2plain,    agafari,       sera }, \
                      { rtf2plain,      alxet,       sera }, \
                      { rtf2plain,     feedel,       sera }, \
                      { rtf2plain,    gezedit,       sera }, \
                      { rtf2plain,   gezfree1,       sera }, \
                      { rtf2plain,   gezfree2,       sera }, \
                      { rtf2plain,     washra,       sera }, \
                      { rtf2plain,    washrax,       sera }, \
\
                      { rtf2plain,    agafari,       utf8 }, \
                      { rtf2plain,      alxet,       utf8 }, \
                      { rtf2plain,     feedel,       utf8 }, \
                      { rtf2plain,    gezedit,       utf8 }, \
                      { rtf2plain,   gezfree1,       utf8 }, \
                      { rtf2plain,   gezfree2,       utf8 }, \
                      { plain2plain,     sera,       utf8 }, \
                      { rtf2plain,     washra,       utf8 }, \
                      { rtf2plain,    washrax,       utf8 }, \
\
                      { rtf2rtf,      agafari,      alxet }, \
                      { rtf2rtf,       feedel,      alxet }, \
                      { rtf2rtf,      agafari,      alxet }, \
                      { rtf2rtf,       feedel,      alxet }, \
                      { rtf2rtf,      agafari,      alxet }, \
                      { rtf2rtf,       feedel,      alxet }, \
                      { rtf2rtf,      gezedit,      alxet }, \
                      { rtf2rtf,     gezfree1,      alxet }, \
                      { rtf2rtf,     gezfree2,      alxet }, \
                      { plain2rtf,       sera,      alxet }, \
                      { rtf2rtf,       washra,      alxet }, \
                      { rtf2rtf,      washrax,      alxet }, \
\
                      { rtf2rtf,        alxet,    agafari }, \
                      { rtf2rtf,       feedel,    agafari }, \
                      { rtf2rtf,      gezedit,    agafari }, \
                      { rtf2rtf,     gezfree1,    agafari }, \
                      { rtf2rtf,     gezfree2,    agafari }, \
                      { plain2rtf,       sera,    agafari }, \
                      { rtf2rtf,       washra,    agafari }, \
                      { rtf2rtf,      washrax,    agafari }, \
\
                      { rtf2rtf,        alxet,     feedel }, \
                      { rtf2rtf,      agafari,     feedel }, \
                      { rtf2rtf,     gezfree1,     feedel }, \
                      { rtf2rtf,     gezfree2,     feedel }, \
                      { plain2rtf,       sera,     feedel }, \
                      { rtf2rtf,       washra,     feedel }, \
                      { rtf2rtf,      washrax,     feedel }, \
\
                      { rtf2rtf,      agafari,    gezedit }, \
                      { rtf2rtf,        alxet,    gezedit }, \
                      { rtf2rtf,       feedel,    gezedit }, \
                      { rtf2rtf,      gezedit,    gezedit }, \
                      { plain2rtf,       sera,    gezedit }, \
                      { rtf2rtf,       washra,    gezedit }, \
\
                      { rtf2rtf,      agafari,   gezfree1 }, \
                      { rtf2rtf,        alxet,   gezfree1 }, \
                      { rtf2rtf,       feedel,   gezfree1 }, \
                      { rtf2rtf,      gezedit,   gezfree1 }, \
                      { plain2rtf,       sera,   gezfree1 }, \
                      { rtf2rtf,       washra,   gezfree1 }, \
\
                      { rtf2rtf,      agafari,   gezfree2 }, \
                      { rtf2rtf,        alxet,   gezfree2 }, \
                      { rtf2rtf,       feedel,   gezfree2 }, \
                      { rtf2rtf,      gezedit,   gezfree2 }, \
                      { plain2rtf,       sera,   gezfree2 }, \
                      { rtf2rtf,      washrax,   gezfree2 }, \
\
                      { rtf2rtf,      agafari,     washra }, \
                      { rtf2rtf,        alxet,     washra }, \
                      { rtf2rtf,       feedel,     washra }, \
                      { rtf2rtf,      gezedit,     washra }, \
                      { plain2rtf,       sera,     washra }, \
                      { rtf2rtf,       washra,     washra }, \
\
                      { rtf2rtf,      agafari,    washrax }, \
                      { rtf2rtf,        alxet,    washrax }, \
                      { rtf2rtf,       feedel,    washrax }, \
                      { rtf2rtf,      gezedit,    washrax }, \
                      { plain2rtf,       sera,    washrax }, \
                      { rtf2rtf,      washrax,    washrax }, \
\
                      { rtf2plain,    agafari,        iso }, \
                      { rtf2plain,      alxet,        iso }, \
                      { rtf2plain,     feedel,        iso }, \
                      { rtf2plain,    gezedit,        iso }, \
                      { rtf2plain,   gezfree1,        iso }, \
                      { rtf2plain,   gezfree2,        iso }, \
                      { plain2plain,     sera,        iso }, \
                      { rtf2plain,     washra,        iso }, \
                      { rtf2plain,    washrax,        iso }, \
\
                      { rtf2tex,      agafari,       etmf }, \
                      { rtf2tex,        alxet,       etmf }, \
                      { rtf2tex,       feedel,       etmf }, \
                      { rtf2tex,      gezedit,       etmf }, \
                      { rtf2tex,     gezfree1,       etmf }, \
                      { rtf2tex,     gezfree2,       etmf }, \
                      { plain2tex,       sera,       etmf }, \
                      { rtf2tex,       washra,       etmf }, \
                      { rtf2tex,      washrax,       etmf }, \
\
                      { rtf2tex,      agafari,        ies }, \
                      { rtf2tex,        alxet,        ies }, \
                      { rtf2tex,       feedel,        ies }, \
                      { rtf2tex,      gezedit,        ies }, \
                      { rtf2tex,     gezfree1,        ies }, \
                      { rtf2tex,     gezfree2,        ies }, \
                      { plain2tex,       sera,        ies }, \
                      { rtf2tex,       washra,        ies }, \
                      { rtf2tex,      washrax,        ies }, \
\
                      { rtf2plain,    agafari,      debug }, \
                      { rtf2plain,      alxet,      debug }, \
                      { rtf2plain,     feedel,      debug }, \
                      { rtf2plain,    gezedit,      debug }, \
                      { rtf2plain,   gezfree1,      debug }, \
                      { rtf2plain,   gezfree1,      debug }, \
                      { plain2plain,     sera,      debug }, \
                      { rtf2plain,     washra,      debug }, \
                      { rtf2plain,    washrax,      debug }, \
\
                      { rtf2plain,    agafari,        jis }, \
                      { rtf2plain,      alxet,        jis }, \
                      { rtf2plain,     feedel,        jis }, \
                      { rtf2plain,    gezedit,        jis }, \
                      { rtf2plain,   gezfree1,        jis }, \
                      { rtf2plain,   gezfree1,        jis }, \
                      { plain2plain,     sera,        jis }, \
                      { rtf2plain,     washra,        jis }, \
                      { rtf2plain,    washrax,        jis }, \
\
                      { rtf2plain,    agafari,       java }, \
                      { rtf2plain,      alxet,       java }, \
                      { rtf2plain,     feedel,       java }, \
                      { rtf2plain,    gezedit,       java }, \
                      { rtf2plain,   gezfree1,       java }, \
                      { rtf2plain,   gezfree1,       java }, \
                      { plain2plain,     sera,       java }, \
                      { rtf2plain,     washra,       java }, \
                      { rtf2plain,    washrax,       java }, \
\
                      { rtf2plain,    agafari,       Java }, \
                      { rtf2plain,      alxet,       Java }, \
                      { rtf2plain,     feedel,       Java }, \
                      { rtf2plain,    gezedit,       Java }, \
                      { rtf2plain,   gezfree1,       Java }, \
                      { rtf2plain,   gezfree1,       Java }, \
                      { plain2plain,     sera,       Java }, \
                      { rtf2plain,     washra,       Java }, \
                      { rtf2plain,    washrax,       Java }, \
\
                      {     NULL,        NIL,        NIL  }  \
               }



#ifdef  SUPPORT_TEX
#  define TEXFONTDIR     "/usr/lib/texmf/fonts/ethiopian/"
#  define FONTPREFIX     "eth"
#  define FONTDPI        300
#  define TEXFONTSIZE    10  /* make this a flag later */
   extern char *TeXFidel[];     /* from fidel.map         */
#  ifdef  SUPPORT_IES
     extern phonset IESAmharic[];     /* from ies.map */
     extern phonset IESTigrigna[];    /* from ies.map */
     extern phonset IESGeez[];        /* from ies.map */
#  endif  /* SUPPORT_IES */
#endif


/*  the following structures are for the support of statistics  */

typedef struct {
  boolean yes;
  enum CharacterSet out;
  enum ContentType  docOut;
} STATSFlags;

typedef struct {
  int  stat;
  unsigned char*  name;
} FIDELStats;


