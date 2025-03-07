
#define ADMAS_UTIL "gezXfer"
#define ADMAS_UTIL_MAJOR_VERSION 0.8
#define ADMAS_UTIL_MINOR_VERSION 5
#define ADMAS_UTIL_EXPORT_DATE "Thu Feb 20 08:57:15 EET 1997"

#include <stdio.h>
#include <signal.h>
#include <syslog.h>
#include <stdio.h>
#include "gezxfer.h"

/* #define MYDEFMAJOR  amh */
/* #define MYDEFMAJOR  gez */
#define MYDEFMAJOR  tir 

extern LS LnS[];  /* from libeth */

struct FontInfo AgafariZ, AgafariET, AgafariR, AgafariYB, Alxet, Ethiopia, Ethiopiax, Ethiopiasl, Ethiopiaxsl, Feedel, GezEdit, Washra, Washrax, Washrasl, Washraxsl, GezFree1, GezFree2, YebSe, YebSex, Roman;
// char FontNames[]= {"AgafariZ", "AgafariET", "AgafariR", "AgafariYB", "Alxet", "Ethiopia", "Ethiopiax", "Ethiopiasl", "Ethiopiaxsl", "Feedel", "GezEdit", "Washra", "Washrax", "Washrasl", "Washraxsl", "GezFree1", "GezFree2", "YebSe", "YebSex", "Roman"};

#ifdef __STDC__
  struct FontInfoList* init_vendors( void );
  extern char* extract_header ( FILE* inFile, LEthFlags* lethFlags );
  extern char* get_rtfLine ( FILE* fp );
  static void sig_term ( int signo );
  int flag_reset ( int argc, char** argv, enum Fonts* Fout, FILE** fp, FIDELStats fidelStats[], STATSFlags** statsFlags, LEthFlags** lethFlags );
  extern int (* scan_header ( enum Fonts Fout, struct FontInfoList* fi, struct FontInfo* (*changeList)[], char* header, gezOutPuts gezOutList[], LEthFlags* lethFlags))() ;
#else
  struct FontInfoList* init_vendors();
  extern char*  extract_header ();
  extern int (* scan_header()) ();
  extern char* get_rtfLine ();
  int flag_reset (),
  static void sig_term ();
#endif   /* __STDC__ */


void 
main (argc,argv)
  int argc;
  char **argv;
{

FILE*  inFile = stdin;
FILE* outFile = stdout;
LEthFlags* lethFlags;
STATSFlags* statsFlags;
FIDELStats fidelStats[UNITOTAL];
struct FontInfoList *fi; 
struct FontInfo *changeList[NUMFONTS];
char  *header,  *rtfLine=NULL;
int (*gezOut)();
enum Fonts Fout=Froman; 
gezOutPuts gezOutList[] = FUNCTION_LIST;
int i;



  if (signal(SIGTERM, sig_term) == SIG_ERR)    /* catch kill commands */
    fprintf (stderr, "can't catch SIGTERM");
  if (signal(SIGINT, sig_term) == SIG_ERR)     /* catch Control-C     */
    fprintf (stderr, "can't catch SIGINT");


/*  Initialize structure of font attributes for fonts we know and love...  */

  fi = init_vendors();
  for (i=0; i<NUMFONTS; i++)
    changeList[i] = NULL;

/*  Parse command line options, setup flags */

  flag_reset (argc, argv, &Fout, &inFile, fidelStats, &statsFlags, &lethFlags);


/*  Rip off RTF header, set up conversion routines, and let gezOut(s) do it's thing  */

  header = extract_header ( inFile, lethFlags );
  gezOut = scan_header ( Fout, fi, &changeList, header, gezOutList, lethFlags ) ;

  switch ( lethFlags->docIn )
    {
      case rtf:
        gezOut ( Fout, fi, &changeList, header, inFile, outFile, fidelStats, statsFlags, lethFlags, HEADER );
        while ( !feof(inFile) )
          {
            if ( (rtfLine = get_rtfLine (inFile)) == NULL )
              break;
            gezOut (Fout, fi, &changeList, rtfLine, NULL, outFile, fidelStats, statsFlags, lethFlags, RTFLINE);
            free ( (char *)rtfLine );
          }
        break;

      case plain :
      default    :
        gezOut ( Fout, fi, &changeList, header, inFile, outFile, fidelStats, statsFlags, lethFlags, HEADER );
        gezOut ( Fout, fi, &changeList,   NULL, inFile, outFile, fidelStats, statsFlags, lethFlags, PLAINLINE );
        break;

    }


/*  print stat tables if requested  */

  if ( statsFlags->yes )
    switch ( statsFlags->docOut )
      {
        case plain :
          Print_Text_Stats (fidelStats, statsFlags, lethFlags);
          break;

        case rtf:
          Print_RTF_Stats (Fout, fi, Roman, fidelStats, statsFlags, lethFlags);
          break;

        case ltx:
        case tex:
          Print_TeX_Stats (fidelStats, statsFlags, lethFlags);
          break;

        case html:
        default:
          Print_HTML_Stats (fidelStats, statsFlags, lethFlags);
          break;
      }


  fclose (inFile);
  fclose (outFile);
  exit (0);

}




 /*****************************************************************************/
/*****************************************************************************/
/*  init_vendors ( void )
/*
/*  The job of init_vendors is to initial the values of the global variables
/*  used to identify the fonts known to this conversion utility.  Fonts used
/*  in multiple font character code systems are treated as completely 
/*  independent fonts and code systems.  Previously they were treated as subsets
/*  within their code group which lead to unnecessarily complicated (and slower)
/*  code when only one of, or less than all of, a font group were present.
/*
/*  This approach also simplifies file parsing when mixed fonts of mixed systems
/*  are fond in a file.
/*
/******************************************************************************/
/*****************************************************************************/

struct FontInfoList*
init_vendors( void )
{

register int i;
struct FontInfoList* fi;


  fi = ( struct FontInfoList* ) malloc ( sizeof(struct FontInfoList)  );


  fi->font[0] =& AgafariZ;
  fi->font[1] =& AgafariET;
  fi->font[2] =& AgafariR;
  fi->font[3] =& AgafariYB;
  fi->font[4] =& Alxet;
  fi->font[5] =& Feedel;
  fi->font[6] =& GezEdit;
  fi->font[7] =& Washra;
  fi->font[8] =& Washrax;
  fi->font[9] =& Washrasl;
  fi->font[10] =& Washraxsl;
  fi->font[11] =& Ethiopia;
  fi->font[12] =& Ethiopiax;
  fi->font[13] =& Ethiopiasl;
  fi->font[14] =& Ethiopiaxsl;
  fi->font[15] =& YebSe;
  fi->font[16] =& YebSex;
  fi->font[17] =& GezFree1;
  fi->font[18] =& GezFree2;
  fi->font[NUMFONTS-1] =& Roman;

  fi->top     = NULL;      /* assigned later when we know it */



/****  Init Agafari ****/

  /****  Init Agafari Zemen ****/
  AgafariZ.script    = fidel;
  AgafariZ.charCode  = agafari;
  AgafariZ.setId     = FagafariZ;
  AgafariZ.numSets   = 1;
  AgafariZ.mySetNum  = 1;

  AgafariZ.setName   = (char *) malloc ( 40 * sizeof(char) );
    strcpy (AgafariZ.setName, "AGF - Zemen");
  AgafariZ.rtfId     = (char *) malloc (  5 * sizeof(char) ); /* for \\fxy */
    strcpy (AgafariZ.rtfId, "\\f");
  AgafariZ.rtfPrefix = NULL;

  AgafariZ.myMap   =& AgafariFidel;

  AgafariZ.nextSet = NULL;
  AgafariZ.lastSet = NULL;


  /****  Init Agafari Ejji Tsihuf ****/
  AgafariET.script    = fidel;
  AgafariET.charCode  = agafari;
  AgafariET.setId     = FagafariET;
  AgafariET.numSets   = 1;
  AgafariET.mySetNum  = 1;

  AgafariET.setName   = (char *) malloc ( 40 * sizeof(char) );
    strcpy (AgafariET.setName, "AGF - Ejji Tsihuf");
  AgafariET.rtfId     = (char *) malloc (  5 * sizeof(char) ); /* for \\fxy */
    strcpy (AgafariET.rtfId, "\\f");
  AgafariET.rtfPrefix = NULL;

  AgafariET.myMap   =& AgafariFidel;

  AgafariET.nextSet = NULL;
  AgafariET.lastSet = NULL;


  /****  Init Agafari Rejim ****/
  AgafariR.script    = fidel;
  AgafariR.charCode  = agafari;
  AgafariR.setId     = FagafariR;
  AgafariR.numSets   = 1;
  AgafariR.mySetNum  = 1;

  AgafariR.setName   = (char *) malloc ( 40 * sizeof(char) );
    strcpy (AgafariR.setName, "AGF - Rejim");
  AgafariR.rtfId     = (char *) malloc (  5 * sizeof(char) ); /* for \\fxy */
    strcpy (AgafariR.rtfId, "\\f");
  AgafariR.rtfPrefix = NULL;

  AgafariR.myMap   =& AgafariFidel;

  AgafariR.nextSet = NULL;
  AgafariR.lastSet = NULL;


  /****  Init Agafari Yigezu Bisrat ****/
  AgafariYB.script    = fidel;
  AgafariYB.charCode  = agafari;
  AgafariYB.setId     = FagafariYB;
  AgafariYB.numSets   = 1;
  AgafariYB.mySetNum  = 1;

  AgafariYB.setName   = (char *) malloc ( 40 * sizeof(char) );
    strcpy (AgafariYB.setName, "AGF - Yigezu Bisrat");
  AgafariYB.rtfId     = (char *) malloc (  5 * sizeof(char) ); /* for \\fxy */
    strcpy (AgafariYB.rtfId, "\\f");
  AgafariYB.rtfPrefix = NULL;

  AgafariYB.myMap   =& AgafariFidel;

  AgafariYB.nextSet = NULL;
  AgafariYB.lastSet = NULL;




/****  Init ALXEthiopian ****/

  Alxet.script    = fidel;
  Alxet.charCode  = alxet;
  Alxet.setId     = Falxet;
  Alxet.numSets   = 1;
  Alxet.mySetNum  = 1;

  Alxet.setName   = (char *) malloc ( 40 * sizeof(char) );
    strcpy (Alxet.setName, "ALXethiopian" );
  Alxet.rtfId     = (char *) malloc (  5 * sizeof(char) );  /* for \\fxy */
    strcpy (Alxet.rtfId, "\\f");
  Alxet.rtfPrefix = NULL;

  Alxet.myMap   =& ALXEtFidel;

  Alxet.nextSet = NULL;
  Alxet.lastSet = NULL;



/****  Init Feedel ****/

  Feedel.script    = fidel;
  Feedel.charCode  = feedel;
  Feedel.setId     = Ffeedel;
  Feedel.numSets   = 1;
  Feedel.mySetNum  = 1;

  Feedel.setName  = (char *) malloc ( 40 * sizeof(char) );
    strcpy (Feedel.setName, "Geezigna");
  Feedel.rtfId    = (char *) malloc (  5 * sizeof(char) );/* for \\fxy */
    strcpy (Feedel.rtfId, "\\f");
  Feedel.rtfPrefix = (char *) malloc ( 12 * sizeof(char) );/* for \\fxy */
    strcpy (Feedel.rtfPrefix, "\\fcharset2 ");

  Feedel.myMap   =& FeedelFidel;

  Feedel.nextSet = NULL;
  Feedel.lastSet = NULL;



/****  Init Ge'ezEdit ****/

  GezEdit.script    = fidel;
  GezEdit.charCode  = gezedit;
  GezEdit.setId     = Fgezedit;
  GezEdit.numSets   = 1;
  GezEdit.mySetNum  = 1;

  GezEdit.setName  = (char *) malloc ( 40 * sizeof(char) );
    strcpy (GezEdit.setName, "Ge\xe8zEdit Amharic P");
  GezEdit.rtfId    = (char *) malloc (  5 * sizeof(char) );/* for \\fxy */
    strcpy (GezEdit.rtfId, "\\f");
  GezEdit.rtfPrefix = NULL;

  GezEdit.myMap   =& GeezEditFidel;

  GezEdit.nextSet = NULL;
  GezEdit.lastSet = NULL;



/****  Init EthiO Systems ****/

  /****  Init Washra Primary ****/
  Washra.script    = fidel;
  Washra.charCode  = washra;
  Washra.setId     = Fwashra;
  Washra.numSets   = 2;
  Washra.mySetNum  = 1;

  Washra.setName   = (char *) malloc ( 40 * sizeof(char) );
    strcpy (Washra.setName, "Washra  Primary");
  Washra.rtfId     = (char *) malloc (  5 * sizeof(char) );  /* for \\fxy */
    strcpy (Washra.rtfId, "\\f");
  Washra.rtfPrefix = NULL;

  Washra.myMap   =& WashraFidel;      

  Washra.nextSet =& Washrax;
  Washra.lastSet = NULL;


  /****  Init Washra Secondary ****/
  Washrax.script    = fidel;
  Washrax.charCode  = washrax;
  Washrax.setId     = Fwashrax;
  Washrax.numSets   = 2;
  Washrax.mySetNum  = 2;

  Washrax.setName   = (char *) malloc ( 40 * sizeof(char) );
    strcpy (Washrax.setName, "Washrax Secondary");
  Washrax.rtfId     = (char *) malloc (  5 * sizeof(char) );     /* for \\fxy */
    strcpy (Washrax.rtfId, "\\f");
  Washrax.rtfPrefix = (char *) malloc ( 12 * sizeof(char) ); 
    strcpy (Washrax.rtfPrefix, "\\fcharset2 ");

  Washrax.myMap   =& WashraFidel;

  Washrax.nextSet = NULL;
  Washrax.lastSet =& Washra;


  /****  Init Washra Slanted Primary ****/
  Washrasl.script    = fidel;
  Washrasl.charCode  = washra;
  Washrasl.setId     = Fwashrasl;
  Washrasl.numSets   = 2;
  Washrasl.mySetNum  = 1;

  Washrasl.setName   = (char *) malloc ( 40 * sizeof(char) );
    strcpy (Washrasl.setName, "Washrasl  Primary Slanted");
  Washrasl.rtfId     = (char *) malloc (  5 * sizeof(char) );  /* for \\fxy */
    strcpy (Washra.rtfId, "\\f");
  Washrasl.rtfPrefix = NULL;

  Washrasl.myMap   =& WashraFidel;      

  Washrasl.nextSet =& Washraxsl;
  Washrasl.lastSet = NULL;


  /****  Init Washra Slanted Secondary ****/
  Washraxsl.script    = fidel;
  Washraxsl.charCode  = washrax;
  Washraxsl.setId     = Fwashraxsl;
  Washraxsl.numSets   = 2;
  Washraxsl.mySetNum  = 2;

  Washraxsl.setName   = (char *) malloc ( 40 * sizeof(char) );
    strcpy (Washraxsl.setName, "Washraxsl Secondary Slanted");
  Washraxsl.rtfId     = (char *) malloc (  5 * sizeof(char) );     /* for \\fxy */
    strcpy (Washraxsl.rtfId, "\\f");
  Washraxsl.rtfPrefix = (char *) malloc ( 12 * sizeof(char) ); 
    strcpy (Washraxsl.rtfPrefix, "\\fcharset2 ");

  Washraxsl.myMap   =& WashraFidel;

  Washraxsl.nextSet = NULL;
  Washraxsl.lastSet =& Washrasl;


  /****  Init Ethiopia Primary ****/
  Ethiopia.script    = fidel;
  Ethiopia.charCode  = washra;
  Ethiopia.setId     = Fethiopia;
  Ethiopia.numSets   = 2;
  Ethiopia.mySetNum  = 2;

  Ethiopia.setName   = (char *) malloc ( 40 * sizeof(char) );
    strcpy (Ethiopia.setName, "Ethiopia Primary");
  Ethiopia.rtfId     = (char *) malloc (  5 * sizeof(char) );     /* for \\fxy */
    strcpy (Ethiopia.rtfId, "\\f");
  Ethiopia.rtfPrefix = (char *) malloc ( 12 * sizeof(char) ); 
    strcpy (Ethiopia.rtfPrefix, "\\fcharset2 ");

  Ethiopia.myMap   =& WashraFidel;

  Ethiopia.nextSet =& Ethiopiax;
  Ethiopia.lastSet =  NULL;


  /****  Init Ethiopia Secondary ****/
  Ethiopiax.script    = fidel;
  Ethiopiax.charCode  = washrax;
  Ethiopiax.setId     = Fethiopiax;
  Ethiopiax.numSets   = 2;
  Ethiopiax.mySetNum  = 2;

  Ethiopiax.setName   = (char *) malloc ( 40 * sizeof(char) );
    strcpy (Ethiopiax.setName, "Ethiopia Secondary");
  Ethiopiax.rtfId     = (char *) malloc (  5 * sizeof(char) );     /* for \\fxy */
    strcpy (Ethiopiax.rtfId, "\\f");
  Ethiopiax.rtfPrefix = (char *) malloc ( 12 * sizeof(char) ); 
    strcpy (Ethiopiax.rtfPrefix, "\\fcharset2 ");

  Ethiopiax.myMap   =& WashraFidel;

  Ethiopiax.nextSet =  NULL;
  Ethiopiax.lastSet =& Ethiopia;


  /****  Init Ethiopia Primary Slanted ****/
  Ethiopiasl.script    = fidel;
  Ethiopiasl.charCode  = washra;
  Ethiopiasl.setId     = Fethiopiasl;
  Ethiopiasl.numSets   = 2;
  Ethiopiasl.mySetNum  = 2;

  Ethiopiasl.setName   = (char *) malloc ( 40 * sizeof(char) );
    strcpy (Ethiopiasl.setName, "Ethiopia Primary Slanted");
  Ethiopiasl.rtfId     = (char *) malloc (  5 * sizeof(char) );     /* for \\fxy */
    strcpy (Ethiopiasl.rtfId, "\\f");
  Ethiopiasl.rtfPrefix = (char *) malloc ( 12 * sizeof(char) ); 
    strcpy (Ethiopiasl.rtfPrefix, "\\fcharset2 ");

  Ethiopiasl.myMap   =& WashraFidel;

  Ethiopiasl.nextSet =& Ethiopiaxsl;
  Ethiopiasl.lastSet =  NULL;


  /****  Init Ethiopia Secondary Slanted ****/
  Ethiopiaxsl.script    = fidel;
  Ethiopiaxsl.charCode  = washrax;
  Ethiopiaxsl.setId     = Fethiopiaxsl;
  Ethiopiaxsl.numSets   = 2;
  Ethiopiaxsl.mySetNum  = 2;

  Ethiopiaxsl.setName   = (char *) malloc ( 40 * sizeof(char) );
    strcpy (Ethiopiaxsl.setName, "Ethiopia Secondary Slanted");
  Ethiopiaxsl.rtfId     = (char *) malloc (  5 * sizeof(char) );     /* for \\fxy */
    strcpy (Ethiopiaxsl.rtfId, "\\f");
  Ethiopiaxsl.rtfPrefix = (char *) malloc ( 12 * sizeof(char) ); 
    strcpy (Ethiopiaxsl.rtfPrefix, "\\fcharset2 ");

  Ethiopiaxsl.myMap   =& WashraFidel;

  Ethiopiaxsl.nextSet =  NULL;
  Ethiopiaxsl.lastSet =& Ethiopiasl;


  /****  Init YebSe Primary ****/
  YebSe.script    = fidel;
  YebSe.charCode  = washra;
  YebSe.setId     = Fyebse;
  YebSe.numSets   = 2;
  YebSe.mySetNum  = 2;

  YebSe.setName   = (char *) malloc ( 40 * sizeof(char) );
    strcpy (YebSe.setName, "YebSe Primary");
  YebSe.rtfId     = (char *) malloc (  5 * sizeof(char) );     /* for \\fxy */
    strcpy (YebSe.rtfId, "\\f");
  YebSe.rtfPrefix = (char *) malloc ( 12 * sizeof(char) ); 
    strcpy (YebSe.rtfPrefix, "\\fcharset2 ");

  YebSe.myMap   =& WashraFidel;

  YebSe.nextSet =  NULL;
  YebSe.lastSet =& YebSe;

  /****  Init YebSe Secondary ****/
  YebSex.script    = fidel;
  YebSex.charCode  = washra;
  YebSex.setId     = Fyebsex;
  YebSex.numSets   = 2;
  YebSex.mySetNum  = 2;

  YebSex.setName   = (char *) malloc ( 40 * sizeof(char) );
    strcpy (YebSex.setName, "YebSe Secondary");
  YebSex.rtfId     = (char *) malloc (  5 * sizeof(char) );     /* for \\fxy */
    strcpy (YebSex.rtfId, "\\f");
  YebSex.rtfPrefix = (char *) malloc ( 12 * sizeof(char) ); 
    strcpy (YebSex.rtfPrefix, "\\fcharset2 ");

  YebSex.myMap   =& WashraFidel;

  YebSex.nextSet =& YebSex;
  YebSex.lastSet =  NULL;


/****  Init Ge'ezFree Zemen Primary  ****/


  GezFree1.script    = fidel;
  GezFree1.charCode  = gezfree1;
  GezFree1.setId     = Fgezfree1;
  GezFree1.numSets   = 2;
  GezFree1.mySetNum  = 1;

  GezFree1.setName   = (char *) malloc ( 40 * sizeof(char) );
    strcpy (GezFree1.setName, "Ge'ezFree Zemen Primary");
  GezFree1.rtfId     = (char *) malloc (  5 * sizeof(char) );     /* for \\fxy */
    strcpy (GezFree1.rtfId, "\\f");
  GezFree1.rtfPrefix = NULL;

  GezFree1.myMap   =& GeezFreeFidel;

  GezFree1.nextSet =& GezFree2;
  GezFree1.lastSet =  NULL;



/****  Init Ge'ezFree Zemen Secondary  ****/

  GezFree2.script    = fidel;
  GezFree2.charCode  = gezfree2;
  GezFree2.setId     = Fgezfree2;
  GezFree2.numSets   = 2;
  GezFree2.mySetNum  = 2;

  GezFree2.setName  = (char *) malloc ( 40 * sizeof(char) );
    strcpy (GezFree2.setName, "Ge'ezFree Zemen Secondary");
  GezFree2.rtfId     = (char *) malloc (  5 * sizeof(char) );     /* for \\fxy */
    strcpy (GezFree2.rtfId, "\\f");
  GezFree2.rtfPrefix = NULL;

  GezFree2.myMap   =& GeezFreeFidel;

  GezFree2.nextSet =  NULL;
  GezFree2.lastSet =& GezFree1;



/****  Init Roman  ****/

  Roman.script    = latin;
  Roman.charCode  = NIL;
  Roman.setId     = Froman;
  Roman.numSets   = 1;
  Roman.mySetNum  = 1;

  Roman.setName   = NULL;
  /*
   *   Achtung!!!!
   *   Only Print_RTF_Stats is using the Roman.setName, make sure that
   *   elsewhere in the code that it is not used!!!!!
   */
  Roman.setName  = (char *) malloc ( 40 * sizeof(char) );
    strcpy (Roman.setName, "Times New Roman");
  Roman.rtfId     = (char *) malloc (  5 * sizeof(char) );     /* for \\fxy */
    strcpy (Roman.rtfId, "\\f");
  Roman.rtfPrefix = NULL;

  Roman.myMap   = NULL;

  Roman.nextSet = NULL;
  Roman.lastSet = NULL;

  /*  this is all we know!  */
  return (fi);


}


/*-------------------------------------------------------------------------//
//                                                                         //
//  Simple routine to read input options and reset the default flags.      //
//  The flags for this version are -l if a file starts in Latin and        //
//  -s to use Ethiopic word separators in Ethiopic TeXt zones.             //
//                                                                         //
//-------------------------------------------------------------------------*/


int 
flag_reset (argc, argv, Fout, fp, fidelStats, statsFlags, lethFlags)
  int argc;
  char **argv;
  enum Fonts* Fout;
  FILE** fp;
  FIDELStats fidelStats[];
  STATSFlags** statsFlags;
  LEthFlags** lethFlags;
{

LS* default_lang =& LnS[MYDEFMAJOR];
char* Lnames2[NUMLANGS] = ISO639_2;
char* Lnames3[NUMLANGS] = ISO639_3;
register int i=0,j;


/*---------------- Define and Initialize Flag Values -------------------------*/


  *lethFlags          =  set_defaultLEthFlags (MYDEFMAJOR, DEFMINOR, DEFMAJOR);

  *statsFlags         =  (STATSFlags *) malloc ( sizeof(STATSFlags) );
  (*statsFlags)->yes  =  false;
  (*statsFlags)->docOut  =  plain;
  (*statsFlags)->out  =  utf8;


/* What Do You Want From Me? */

  while ( ++i < argc )
    {
      if ( argv[i][0] == '-' ) 
        {
          switch ( toupper(argv[i][1]) ) 
            {
              case 'C' :  (*lethFlags)->colon = true;
                          break;
              case 'D' :  (*lethFlags)->out = debug;
                          break;

              case 'F' :  
#ifdef SUPPORT_DOS
                          (*lethFlags)->dos =  1;
#else
                            {
                              fprintf (stderr, "\a -fromdos option not available\n");
                              fprintf (stderr, "\a See ``README.dosio'' to enable DOS support\n");
                              exit(0);
                            }
#endif
                          break;

              case 'L' :  j=-1;
                          while( ++j < NUMLANGS && (strcmp(argv[i+1],Lnames2[j]) && strcmp(argv[i+1],Lnames3[j])) );
                          if (j < NUMLANGS ) 
                            (*lethFlags)->minor =& LnS[j];  
                          (*lethFlags)->top     =& LnS[j];  
                          if (j == lat)
                            (*lethFlags)->major = default_lang;
                          i++;
                          if (j == NUMLANGS) 
                            {
                              fprintf (stderr, "Language %s Not Supported\n",argv[i]);
                              exit(1);
                            }
                          break; 

              case 'O' :  i++;
                          if ( strstr ( argv[i], ".html" ) )
                            (*lethFlags)->docOut = html;
                          else if ( strstr ( argv[i], ".rtf" ) )
                            (*lethFlags)->docOut = rtf;
                          else if ( strstr ( argv[i], ".tex" ) )
#ifdef SUPPORT_TEX
                              (*lethFlags)->docOut  =  tex;
#else
                            {
                              fprintf (stderr, "\a TeX Output Not Available By This Package \n");
                              exit(0);
                            }
#endif
                          else if ( strstr ( argv[i], ".ltx" ) )
#ifdef SUPPORT_TEX
                              (*lethFlags)->docOut  =  ltx;
#else
                            {
                              fprintf (stderr, "\a LaTeX Output Not Available By This Package \n");
                              exit(0);
                            }
#endif
                          else  /* the default anyway */
                            (*lethFlags)->docOut = plain;

                          if ( !strncmp (argv[i], "jun", 3) )
                            (*lethFlags)->out  =  jun;
                          else if ( !strncmp (argv[i], "jis", 3) )
                            (*lethFlags)->out  =  jis;
                          else if ( !strncmp (argv[i], "jav", 3) )
                            (*lethFlags)->out  =  java;
                          else if ( !strncmp (argv[i], "Jav", 3) )
                            (*lethFlags)->out  =  Java;
                          else if ( !strncmp (argv[i], "uni", 3) )
                            (*lethFlags)->out  =  uni;
                          else if ( !strncmp (argv[i], "utf8", 4) )
                            (*lethFlags)->out  =  utf8;
                          else if ( !strncmp (argv[i], "agafariZ", 8) )
                            {
                              (*lethFlags)->out = agafari;
                              (*lethFlags)->docOut = rtf;
                              *Fout = FagafariZ;
                            }
                          else if ( !strncmp (argv[i], "agafariET", 9) )
                            {
                              (*lethFlags)->out = agafari;
                              (*lethFlags)->docOut = rtf;
                              *Fout = FagafariET;
                            }
                          else if ( !strncmp (argv[i], "agafariR", 8) )
                            {
                              (*lethFlags)->out = agafari;
                              (*lethFlags)->docOut = rtf;
                              *Fout = FagafariR;
                            }
                          else if ( !strncmp (argv[i], "agafariYB", 9) )
                            {
                              (*lethFlags)->out = agafari;
                              (*lethFlags)->docOut = rtf;
                              *Fout = FagafariYB;
                            }
                          else if ( !strncmp (argv[i], "agafari", 7) )
                            {
                              (*lethFlags)->out = agafari;
                              (*lethFlags)->docOut = rtf;
                              *Fout = FagafariZ;
                            }
                          else if ( !strncmp (argv[i], "feedel", 6) )
                            {
                              (*lethFlags)->out = feedel;
                              (*lethFlags)->docOut = rtf;
                              *Fout = Ffeedel;
                            }
                          else if ( !strncmp (argv[i], "gezedit", 7) )
                            {
                              (*lethFlags)->out = gezedit;
                              (*lethFlags)->docOut = rtf;
                              *Fout = Fgezedit;
                            }
                          else if ( !strncmp (argv[i], "gezfree", 7) )
                            {
                              (*lethFlags)->out = gezfree1;
                              (*lethFlags)->docOut = rtf;
                              *Fout = Fgezfree1;
                            }
                          else if ( !strncmp (argv[i], "alxet", 5) )
                            {
                              (*lethFlags)->out = alxet;
                              (*lethFlags)->docOut = rtf;
                              *Fout = Falxet;
                            }
                          else if ( !strncmp (argv[i], "washrasl", 8) )
                            {
                              (*lethFlags)->out = washra;
                              (*lethFlags)->docOut = rtf;
                              *Fout = Fwashrasl;
                            }
                          else if ( !strncmp (argv[i], "washra", 6) )
                            {
                              (*lethFlags)->out = washra;
                              (*lethFlags)->docOut = rtf; 
                              *Fout = Fwashra;
                            }
                          else if ( !strncmp (argv[i], "yebse", 5) )
                            {
                              (*lethFlags)->out = washra;
                              (*lethFlags)->docOut = rtf; 
                              *Fout = Fyebse;
                            }
                          else if ( !strncmp (argv[i], "ethiopia", 8) )
                            {
                              (*lethFlags)->out = washra;
                              (*lethFlags)->docOut = rtf; 
                              *Fout = Fethiopia;
                            }
                          else if ( !strncmp (argv[i], "ethiopiasl", 10) )
                            {
                              (*lethFlags)->out = washra;
                              (*lethFlags)->docOut = rtf; 
                              *Fout = Fethiopiasl;
                            }
                          else if ( !strncmp (argv[i], "debug", 5) )
                            (*lethFlags)->out = debug;
                          else if ( !strncmp (argv[i], "etmf", 4) )
#ifdef SUPPORT_TEX
                            {
                              (*lethFlags)->out  =  etmf;
                              if ( (*lethFlags)->docOut == plain )
                                (*lethFlags)->docOut  = ltx;
                            }
#else
                            {
                              fprintf (stderr, "\a LaTeX Output Not Available By This Package \n");
                              exit(0);
                            }
#endif
                          else if ( !strncmp (argv[i], "sera", 4) )
                            (*lethFlags)->out  =  sera;
                          else if ( !strncmp (argv[i], "ies", 3) )
#ifdef SUPPORT_IES
                            {
                              (*lethFlags)->out  =  ies;
                              if ( (*lethFlags)->docOut == plain )
                                (*lethFlags)->docOut  = ltx;
                            }
#else
                            {
                              fprintf (stderr, "\a IES Transliteration Output Not Available By This Package \n");
                              exit(0);
                            }
#endif
                          else if ( !strncmp (argv[i], "iso", 3) )
#ifdef SUPPORT_ISO
                            (*lethFlags)->out  =  iso;
#else
                            {
                              fprintf (stderr, "\a ISO Transliteration Output Not Available By This Package \n");
                              exit(0);
                            }
#endif

                          break;

              case 'Q' :  (*lethFlags)->qmark = true;
                          break;
              case 'S' :  if ( toupper(argv[i][2]) == 'T' )
                            {
                              (*statsFlags)->yes = true;            /* give character occurance statistics */
                              for ( j = 0; j < UNITOTAL; j++ )
                                {
                                  fidelStats[j].stat = 0;
                                  fidelStats[j].name = NULL;
                                }
                              if ( argv[i+1] == '\0' || argv[i+1][0] == '-' )
                                break;
                              i++;

                              if ( strstr ( argv[i], ".html" ) )
                                (*statsFlags)->docOut = html;
                              else if ( strstr ( argv[i], ".rtf" ) )
                                (*statsFlags)->docOut = rtf;
                              else if ( strstr ( argv[i], ".tex" ) )
#ifdef SUPPORT_TEX
                                (*statsFlags)->docOut  =  tex;
#else
                                {
                                  fprintf (stderr, "\a TeX Output Not Available By This Package \n");
                                  exit(0);
                                }
#endif
                              else if ( strstr ( argv[i], ".ltx" ) )
#ifdef SUPPORT_TEX
                                  (*statsFlags)->docOut  =  ltx;
#else
                                {
                                  fprintf (stderr, "\a LaTeX Output Not Available By This Package \n");
                                  exit(0);
                                }
#endif
                              else  /* the default anyway */
                                (*statsFlags)->docOut = plain;

                              if ( !strncmp (argv[i], "etmf", 4) )
#ifdef SUPPORT_TEX
                                {
                                  (*statsFlags)->out  = etmf;
                                    if ( (*statsFlags)->docOut == plain )
                                      (*statsFlags)->docOut  = ltx;
                                }
#else
                                {
                                  fprintf (stderr, "\a La/TeX Output Not Available By This Package \n");
                                  exit(0);
                                }
#endif
                              else if ( !strncmp (argv[i], "ies", 3) )
#ifdef SUPPORT_IES
                                {
                                  (*statsFlags)->out  = ies;
                                    if ( (*statsFlags)->docOut == plain )
                                      (*statsFlags)->docOut  = ltx;
                                }
#else
                                {
                                  fprintf (stderr, "\a IES Transliteration Output Not Available By This Package \n");
                                  exit(0);
                                }
#endif
                              else if ( !strncmp (argv[i], "iso", 3) )
#ifdef SUPPORT_ISO
                                (*statsFlags)->out  = iso;
#else
                                {
                                  fprintf (stderr, "\a ISO Transliteration Output Not Available By This Package \n");
                                  exit(0);
                                }
#endif
                              else if ( !strncmp (argv[i], "jun", 3) )
                                (*statsFlags)->out  =  jun;
                              else if ( !strncmp (argv[i], "jis", 3) )
                                (*statsFlags)->out  =  jis;
                              else if ( !strncmp (argv[i], "uni", 3) )
                                (*statsFlags)->out  =  uni;
                              else if ( !strncmp (argv[i], "utf8", 4) )
                                (*statsFlags)->out  =  utf8;
                              else if ( !strncmp (argv[i], "jav", 3) )
                                (*statsFlags)->out  =  java;
                              else if ( !strncmp (argv[i], "Jav", 3) )
                                (*statsFlags)->out  =  Java;
                              else if ( !strncmp (argv[i], "sera", 4) )
                                (*statsFlags)->out  = sera;
                              else if ( !strncmp (argv[i], "agafari", 7) )
                                (*statsFlags)->out  =  agafari;
                              else if ( !strncmp (argv[i], "feedel", 6) )
                                (*statsFlags)->out  =  feedel;
                              else if ( !strncmp (argv[i], "gezedit", 7) )
                                (*statsFlags)->out  =  gezedit;
                              else if ( !strncmp (argv[i], "gezfree", 7) )
                                (*statsFlags)->out  =  gezfree1;
                              else if ( !strncmp (argv[i], "alxet", 5) )
                                (*statsFlags)->out  =  alxet;
                              else if ( !strncmp (argv[i], "washra", 6) )
                                (*statsFlags)->out  =  washra;
                              else
                                i--;  /* no alternative option given, stick with default */
                            }
                          else
                            (*lethFlags)->gspace = true;  /* use Eth Word Sep. for " " */
                          break;

              case 'T' :  
#ifdef SUPPORT_DOS
                          (*lethFlags)->dos =  2;
#else
                            {
                              fprintf (stderr, "\a -todos option not available\n");
                              fprintf (stderr, "\a Recompile %s and libeth with ``make withdos'' \n", ADMAS_UTIL);
                              exit(0);
                            }
#endif
                          break;

              case 'V' :  fprintf (stdout, "This is %s Version %0.1f%i\n", ADMAS_UTIL, ADMAS_UTIL_MAJOR_VERSION, ADMAS_UTIL_MINOR_VERSION);
                          fprintf (stdout, "  Export Date:  %s\n", ADMAS_UTIL_EXPORT_DATE);
                          exit(1);
                          break;
USAGE:
              case '?' :
              default  :
                          fprintf (stderr, "\n\tUseage:  %s option[s] file\n", ADMAS_UTIL);
                          fprintf (stderr, "\tTo substitute Latin spaces with Ge'ez wordspace:\n");
                          fprintf (stderr, "\t   -s\n");
                          fprintf (stderr, "\tTo substitute Latin colon with Ge'ez colon:\n");
                          fprintf (stderr, "\t(instead of wordspace):\n");
                          fprintf (stderr, "\t   -c\n");
                          fprintf (stderr, "\tTo switch the mappings for ?  and `? :\n");
                          fprintf (stderr, "\t   -q\n");
                          fprintf (stderr, "\tTo print tables of statistics in fidel.out and fidel2.out\n");
                          fprintf (stderr, "\t   -stats [encoding]\n");
                          fprintf (stderr, "\t           Encoding is one of: jun, jis, uni, utf8 (default is sera)\n");
                          fprintf (stderr, "\tTo specify output encoding:\n");
                          fprintf (stderr, "\t   -o encoding-name \n");
                          fprintf (stderr, "\t      agafari = Agafari ANSI Mapping \n");
                          fprintf (stderr, "\t      alxet = ALXEthiopian ANSI Mapping \n");
                          fprintf (stderr, "\t      feedel = Feedel ANSI Mapping \n");
                          fprintf (stderr, "\t      java = Java  (lowercase hexidecimal)\n");
                          fprintf (stderr, "\t      Java = Java  (UPPERCASE HEXIDECIMAL)\n");
                          fprintf (stderr, "\t      jis  = JIS \n");
                          fprintf (stderr, "\t      jun  = JUNET \n");
#ifdef SUPPORT_TEX
                          fprintf (stderr, "\t      latex = LaTeX \n");
                          fprintf (stderr, "\t      tex = TeX \n");
#endif
                          fprintf (stderr, "\t      uni = Unicode \n");
                          fprintf (stderr, "\t      utf8 = Unicode-UTF8 \n");
                          fprintf (stderr, "\t      washra = Washra (Ethio ASCII)\n");
                          fprintf (stderr, "\tTo set starting language:\n");
                          fprintf (stderr, "\t   -l iso639-name \n");
                          fprintf (stderr, "\t      am = amh = Amharic \n");
                          fprintf (stderr, "\t      gz = gez = Ge'ez   \n");
                          fprintf (stderr, "\t      la = lat = Latin   \n");
                          fprintf (stderr, "\t      ti = tir = Tigrigna\n");
                          fprintf (stderr, "\t   or append .html to encoding name as in:\n");
                          fprintf (stderr, "\t      -o jis.html\n");
#ifdef SUPPORT_DOS
                          fprintf (stderr, "\tTo strip out DOS ^M end of line characters from input:\n");
                          fprintf (stderr, "\t   -fromdos\n");
                          fprintf (stderr, "\tTo insert DOS ^M end of line characters in output:\n");
                          fprintf (stderr, "\t   -todos\n");
#endif
                          fprintf (stderr, "\tEcho version number and quit:\n");
                          fprintf (stderr, "\t   -v\n");
                          fprintf (stderr, "\n\tTo contact the maintainers:\n");
                          fprintf (stderr, "\t   fisseha@cig.mot.com / yacob@padis.gn.apc.org\n\n");

                          exit(1);
                          break;
             
               }
           }  
         else 
           {
             fclose (*fp);
             if ((*fp = fopen(argv[i], "r")) == NULL)
               {
                 fprintf (stderr, "\n*** File Not Found \"%s\" ***\n",argv[i]);
                 exit(1);
               }
           }
  }

  return(true);

    /* else ignore, and use defaults */
}


/*-------------------------------------------------------------------------//
//                                                                         //
//  sig_term catches the more violent break signals, presumably something  //
//  has gone awry...                                                       //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
sig_term ( int signo )
{

  if ( signo == SIGTERM || signo == SIGINT )
    {
      /* ...personal toying with unix...
       *
	   * openlog ("sera2", LOG_PID, LOG_USER);
	   * syslog (LOG_DEBUG, "failed on %s", filename);
	   * closelog ();
	   */ 
      fprintf (stderr, "\nIndE??  You seem to have found a bug with %s version %0.1f%i\n", ADMAS_UTIL, ADMAS_UTIL_MAJOR_VERSION, ADMAS_UTIL_MINOR_VERSION);
   	  fprintf (stderr, "Please email your input file to the maintainer\n");
	  fprintf (stderr, "Daniel Yacob:-  Daniel_Yacob_at_UNECA@un.org\n");
	  fprintf (stderr, "                                              Thank You!\n\n");
	  exit (0);
    }

}
