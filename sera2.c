/*
**  sera2any v0.26
**
**  Generalized code to convert SERA (The System for Ethiopic Representation
**  in ASCII) to an arbitrary second system.  The generalized transcription
**  is achieved here by requiring a mapping of the output system into the
**  "fidel" address table (see fidel.map).
** 
**  This is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2, or (at your option)
**  any later version.  See the included "COPYING" file.
** 
**  --Daniel Yaqob, 1995-1997
** 
*/

#define ADMAS_UTIL "sera2any"
#define ADMAS_UTIL_MAJOR_VERSION 0.2
#define ADMAS_UTIL_MINOR_VERSION 6
#define ADMAS_UTIL_EXPORT_DATE "Wed Dec 11 21:39:02 EET 1996"

#include <signal.h>
#include <syslog.h>
#include <stdio.h>
#include <libeth/fidel.h>
#include <libeth/libeth.h>
#include <libeth/letchar.h>
// #include "rtf.map"

/* #define MYLANGMAJOR  amh */
/* #define MYLANGMAJOR  gez */
#define MYLANGMAJOR  tir 

extern char *TeXFidel[], *RTFMAP[];
extern int UniToTraditional[];
extern LS LnS[];

typedef struct {
  boolean yes;
  enum CharacterSet out;
  enum ContentType  docOut;
} STATSFlags;

static struct mystats {
  int  stat;
  unsigned char*  name;
} Stats[UNITOTAL];

#ifdef __STDC__
  int flag_reset ( int argc, char** argv, FILE** fp, STATSFlags** statsFlags, LEthFlags** lethFlags );
  int gt ( struct mystats** A, struct mystats** B );
  int get_traditional ( int trad, FCHAR* uni );
  void print_stats ( STATSFlags* statsFlags, LEthFlags* lethFlags ); 
  int fout_java ( FCHAR* fstring, FILE* fp, LEthFlags* lethFlags );
  int fout_rtf ( FILE* fp, STATSFlags* statsFlags, LEthFlags* lethFlags );
  static void sig_term ( int signo );
# ifdef  SUPPORT_TEX
    int fout_tex  ( FCHAR* fstring, FILE* fp, LEthFlags* lethFlags );
# endif  /* SUPPORT_TEX */
#else
  int flag_reset (),
  int gt ();
  int get_traditional ();
  void print_stats ();
  int fout_java ();
  int fout_rtf ();
  static void sig_term ();
# ifdef  SUPPORT_TEX
    int fout_tex  ();
# endif  /* SUPPORT_TEX */
#endif   /* __STDC_ */

/*-------------------------------------------------------------------------//
//                                                                         //
//  Main reads input switches, does file read-in, and sends output where   //
//  we want it.                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

void 
main ( argc, argv )
  int argc;
  char **argv;
{

FCHAR* uniString;
FILE *inptr = stdin;
FILE* outptr = stdout;
LEthFlags* lethFlags;
STATSFlags* statsFlags;
int i, test = true;


  if (signal(SIGTERM, sig_term) == SIG_ERR)    /* catch kill commands */
    fprintf (stderr, "can't catch SIGTERM");
  if (signal(SIGINT, sig_term) == SIG_ERR)     /* catch Control-C     */
    fprintf (stderr, "can't catch SIGINT");

  flag_reset (argc, argv, &inptr, &statsFlags, &lethFlags);

  if ( lethFlags->docOut == rtf )
    fout_rtf ( inptr, statsFlags, lethFlags );

  while (!feof(inptr) && test)
   {
     if ( lethFlags->in != sera )
       test = fidel_fgets (&uniString, WSIZE, inptr, lethFlags);
     else
       test = check_lang (&uniString, WSIZE, inptr, lethFlags);

     switch ( lethFlags->out )
	   {

       /*  TeX and Java are not character codes schemes so we
	    *  handle I/O at this level and not in the library.
		*/

         case java:  
         case Java:  
		   fout_java (uniString, outptr, lethFlags);
		   break;
#ifdef  SUPPORT_TEX
         case etmf:  
		   fout_tex (uniString, outptr, lethFlags);
		   break;
#endif  /* SUPPORT_TEX */

         default:
		   fidel_fputs (uniString, outptr, lethFlags);
	   }
     if (statsFlags->yes)
       {
         i = 0;
         while ( uniString[i] )
           {
             if ( isethio (uniString[i]) )
               if ( isprivate(uniString[i]) )
                 Stats[(UNITOTAL-1) + (uniString[i] - PRIVATE_USE_END)].stat ++;
               else
                 Stats[uniString[i]-UNIFIDEL].stat ++;
             i++;
           }
       }
     free ((FCHAR *)uniString); 
   }

  if (statsFlags->yes) 
    print_stats ( statsFlags, lethFlags );

/*-------Lets Be A Good Citizen-----------------*/

  fclose (inptr);
  free ((LEthFlags *)lethFlags);
  free ((STATSFlags *)statsFlags);
  exit (0);

}



/*-------------------------------------------------------------------------//
//                                                                         //
//  Print Table of Character Occurances                                    //
//                                                                         //
//-------------------------------------------------------------------------*/


void 
print_stats (statsFlags, lethFlags)
  STATSFlags* statsFlags;
  LEthFlags* lethFlags;
{

int trad, uniMap, charBase, i, mod, space=0, stop;
FCHAR uni=0, *uniP;
FILE* fp;
struct mystats* PStats[UNITOTAL];


  uniP =& uni;
  lethFlags->out = statsFlags->out;


  if ( statsFlags->docOut == html ) 
    {
      fp = fopen ( "fidel-stats.html", "w" );
      fprintf (fp, "<html>\n<head>\n");
      fprintf (fp, "  <title>Unsorted Fidel Character Occurances</title>\n</head>\n<body>\n\n");
      fprintf (fp, "<a name=\"top\"><h3><hr>Unsorted Fidel Character Occurances<hr></h3>\n\n<p>\n\n");
      fprintf (fp, "<table border>\n");
    }
  else
    {
      fp = fopen ( "fidel.stats", "w" );
      fprintf (fp, "-------------------------------------------------------------------------------------\n\n");
    }

  for (trad = 0; trad < UNITOTAL; trad += ROW) 
    {
      if ( statsFlags->docOut == html ) 
        fprintf (fp, "  <tr>") ;

      charBase = get_traditional ( trad, uniP );

      mod = get_fmodulo ( uni ); 

      if ( uni == SPACE || uni == MYA || uni == ONE )
        if ( !statsFlags->docOut == html ) 
          fprintf (fp, "-------------------------------------------------------------------------------------\n\n");

      if ( uni == SPACE ) 
        space = 1;

      for (i=0; i<mod; i++) 
        {
          if ( (mod == 12 && i > 6) || (mod == 15) )
            uniMap = get_traditional (trad+i, uniP);
          else
            uniMap = charBase + i;
          fidel_sputc (uni, &Stats[uniMap].name, lethFlags);
          if ( statsFlags->docOut == html ) 
            fprintf (fp, "<td>%s<br>%d</td>", Stats[uniMap].name, Stats[uniMap].stat);
          else
            fprintf (fp, "%*s    ", 3+space, Stats[uniMap].name);
          uni++;
        }

      if ( statsFlags->docOut == html ) 
        fprintf (fp, "</tr>\n  <tr>");
      else
       {
         fprintf (fp, "\n");

         if ( mod == 12 )                           /* start over, uniMap was trashed */
           uniMap = get_traditional ( trad, uniP );
         for (i=0; i<mod; i++) 
           {
             if ( mod == 12 && i > 6 )
               uniMap = get_traditional (trad+i, uniP);
             else
               uniMap = charBase + i;

             fprintf (fp, "%*d    ", 3+space, Stats[uniMap].stat);
           }

           fprintf (fp, "\n\n");
       }

      if (mod > 8)
        {
          if (mod == 12)
            trad += ROW;
          else if ( (uni-mod) == ONE )
            trad += 2;
          else if ( (uni-mod) == (TEN+1) )   /* (TEN+1) = TWENTY */
            trad += 2;
          else if ( isfpunct(uni) )
            trad = UNITOTAL;
        }

    }   /* end for */


  if ( statsFlags->docOut == html ) 
    {
      fprintf (fp, "</table>\n\n");
      fprintf (fp, "<a href=\"#top\">Back To Top</a>\n<p>");
      fprintf (fp, "<strong><em><a href=\"fidel2-stats.html\">To Sorted Stats</a></em></strong>\n");
      fprintf (fp, "\n\n<hr>\n\n</body>\n</html>\n");
    }
  else
    fprintf (fp, "-------------------------------------------------------------------------------------");
  fclose(fp);

 
  for ( i = 0; i < UNITOTAL; i++ )
    PStats[i] =& Stats[i]; 

  if ( statsFlags->docOut == html ) 
    {
      fp = fopen ( "fidel2-stats.html", "w" );
      fprintf (fp, "<html>\n<head>\n");
      fprintf (fp, "  <title>Sorted Fidel Character Occurances</title>\n</head>\n<body>\n\n");
      fprintf (fp, "<a name=\"top\"><h3><hr>Sorted Fidel Character Occurances<hr></h3>\n\n<p>\n\n");
      fprintf (fp, "<table border>\n");
    }
  else
    {
      fp = fopen("fidel2.stats", "w");
      fprintf (fp, "-------------------------------------------------------------------------------------\n\n");
    }

  qsort ( PStats, UNITOTAL, sizeof(struct mystruct *), gt );
  space = stop = 0;



  for (trad = 0; trad < UNITOTAL; trad += ROW) 
    {
      if ( statsFlags->docOut == html ) 
        fprintf (fp, "  <tr>") ;
      else
        {
          fprintf (fp, "\n");

          for (i=0; i < 8; i++) 
            fprintf (fp, "%0*d    ", 3+space, trad+i);
          fprintf (fp, "\n");
          stop = 0;
        }


      for (i=0; stop != 8 && (trad+i) < UNITOTAL; i++) 
        if ( PStats[trad+i]->name != '\0' )
          {
            if ( statsFlags->docOut == html ) 
              fprintf (fp, "<td>%0*d<br>%s<br>%d</td>", 3+space, trad+i, PStats[trad+i]->name, PStats[trad+i]->stat);
            else
              fprintf (fp, "%*s    ", 3+space, PStats[trad+i]->name);

            stop++;
          }

      fprintf (fp, "\n");
      stop = 0;


      if ( statsFlags->docOut == html ) 
        fprintf (fp, "</tr>\n  <tr>");
      else
       {
         for (i=0; stop != 8 && (trad+i) < UNITOTAL; i++) 
           if ( PStats[trad+i]->name != '\0' )
             {
               fprintf (fp, "%*d    ", 3+space, PStats[trad+i]->stat);
               stop++;
             }
         fprintf (fp, "\n\n");
       }


      if (i > ROW)
        trad += (i-ROW);


    }  /* end for */

  if ( statsFlags->docOut == html ) 
    {
      fprintf (fp, "</table>\n\n");
      fprintf (fp, "<a href=\"#top\">Back To Top</a>\n<p>");
      fprintf (fp, "<strong><em><a href=\"fidel-stats.html\">To UnSorted Stats</a></em></strong>");
      fprintf (fp, "\n\n<hr>\n\n</body>\n</html>\n");
    }
  else
    fprintf (fp, "-------------------------------------------------------------------------------------");


  fclose(fp);
  return;

}



 /*****************************************************************************/
/*****************************************************************************/
/* get_traditional
/*
/* is here becuase fidel.map does not have a TraditionalToUni
/* ...maybe it should ?
/*
/******************************************************************************/
/*****************************************************************************/

int 
get_traditional ( trad, uni )
  int trad; 
  FCHAR* uni;
{

int uniMap;


  uniMap = *uni = 0;
  while ( UniToTraditional[uniMap] != trad && uniMap < UNITOTAL ) 
    (uniMap)++;

  if ( uniMap == UNITOTAL ) /* nothing found */
    return ( NIL );

  *uni = uniMap + UNIFIDEL;

  if ( *uni >= UNILAST+NUM_EXTEND )
    *uni = (*uni - (UNILAST + NUM_EXTEND) ) + PRIVATE_USE_END - NUM_SPECIAL;

  return ( uniMap ) ;

}


int 
gt ( A, B )
  struct mystats **A, **B;
{
  return ( (*B)->stat - (*A)->stat );
}


/*-------------------------------------------------------------------------//
//                                                                         //
//  Simple routine to read input options and reset the default flags.      //
//  The flags for this version are -l if a file starts in Latin and        //
//  -s to use Ethiopic word separators in Ethiopic text zones.             //
//                                                                         //
//-------------------------------------------------------------------------*/


int 
flag_reset (argc, argv, fp, statsFlags, lethFlags)
  int argc;
  char **argv;
  FILE** fp;
  STATSFlags** statsFlags;
  LEthFlags** lethFlags;
{

char* Lnames2[NUMLANGS] = ISO639_2;
char* Lnames3[NUMLANGS] = ISO639_3;
register int i,j;


/*---------------- Define and Initialize Flag Values -------------------------*/


  *lethFlags          =  set_defaultLEthFlags (NIL, LETLANGMINOR, NIL);

  *statsFlags         =  (STATSFlags *) malloc ( sizeof(STATSFlags) );
  (*statsFlags)->yes  =  false;
  (*statsFlags)->docOut  =  plain;
  (*statsFlags)->out  =  sera;


  i = 0;

/* Who Am I? */

  if ( strncmp( &argv[0][strlen(argv[0])-9], "sera2any", 8) )
    if ( strstr(argv[0], "sera2jis") ) 
      (*lethFlags)->out  =  jis;
    else if ( strstr(argv[0], "sera2jun") || strstr(argv[0], "sera2ps") ) 
      (*lethFlags)->out  =  jun;
    else if ( strstr(argv[0], "sera2java") ) 
      (*lethFlags)->out  =  java;
    else if ( strstr(argv[0], "sera2Java") ) 
      (*lethFlags)->out  =  Java;
    else if ( strstr(argv[0], "sera2uni") ) 
      (*lethFlags)->out  =  uni;
    else if ( strstr(argv[0], "sera2utf8") ) 
      (*lethFlags)->out  =  utf8;
    else if ( strstr(argv[0], "sera2nci") ) 
      (*lethFlags)->out  =  nci;
    else if ( strstr(argv[0], "sera2ncic") ) 
      (*lethFlags)->out  =  ncic;
    else if ( strstr(argv[0], "sera2brana") ) 
      (*lethFlags)->out  =  branai;
    else if ( strstr(argv[0], "sera2geezigna") ) 
      (*lethFlags)->out  =  gezigna;
    else if ( strstr(argv[0], "sera2geez") ) 
      (*lethFlags)->out  =  gezii;
    else if ( strstr(argv[0], "sera2gf") ) 
      (*lethFlags)->out  =  gezfree1;
    else if ( strstr(argv[0], "sera2acu") ) 
      (*lethFlags)->out  =  acuwork;
    else if ( strstr(argv[0], "sera2ethio") ) 
      (*lethFlags)->out  =  ethiosys;
    else if ( strstr(argv[0], "sera2tex") ) 
#ifdef SUPPORT_TEX
      {
        (*lethFlags)->out  =  etmf;
        (*lethFlags)->docOut  =  tex;
      }
#else
      {
        fprintf (stderr, "\a TeX Output Not Available By This Package \n");
        exit(0);
      }
#endif
    


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

              case 'H' :  if ( !strcmp ( &argv[i][1], "html") )
                            (*lethFlags)->docOut = html;
                          else
                            goto USAGE;
                          break;

              case 'I' :  i++;
                          if ( !strncmp (argv[i], "geezigna", 8) )
                            (*lethFlags)->in  =  gezigna;
                          else if ( !strncmp (argv[i], "geez", 4) )
                            (*lethFlags)->in  =  gezii;
                          else if ( !strncmp (argv[i], "acuwork", 7) )
                            (*lethFlags)->in  =  acuwork;
                          else if ( !strncmp (argv[i], "nci", 3) )
                            (*lethFlags)->in  =  nci;
                          else if ( !strncmp (argv[i], "ncic", 4) )
                            (*lethFlags)->in  =  ncic;
                          else if ( !strncmp (argv[i], "ed", 2) )
                            (*lethFlags)->in  =  ed;
                          break;
                         
              case 'L' :  j=-1;
                          while( ++j < NUMLANGS && (strcmp(argv[i+1],Lnames2[j]) && strcmp(argv[i+1],Lnames3[j])) );
                          if (j < NUMLANGS ) 
                            (*lethFlags)->minor =& LnS[j];  
                          (*lethFlags)->top     =& LnS[j];  
                          if (j == lat)                            /* why? */
                            (*lethFlags)->major =& LnS[MYLANGMAJOR];
                          i++;
                          if (j == NUMLANGS) 
                            {
                              fprintf (stderr, "Language %s Not Supported\n",argv[i]);
                              exit(1);
                            }
                          break; 

              case 'O' :  i++;
                          if ( strstr ( argv[i], "-html" ) )
                            (*lethFlags)->docOut = html;
                          if ( strstr ( argv[i], "-rtf" ) )
                            (*lethFlags)->docOut = rtf;
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
                          else if ( !strncmp (argv[i], "ncic", 4) )
                            (*lethFlags)->out  =  ncic;
                          else if ( !strncmp (argv[i], "nci", 3) )
                            (*lethFlags)->out  =  nci;
                          else if ( !strncmp (argv[i], "brana", 5) )
                            (*lethFlags)->out  =  branai;
                          else if ( !strncmp (argv[i], "geezigna", 8) )
                            (*lethFlags)->out  =  gezigna;
                          else if ( !strncmp (argv[i], "geez", 4) )
                            (*lethFlags)->out  =  gezii;
                          else if ( !strncmp (argv[i], "gezfree", 7) )
                            (*lethFlags)->out  =  gezfree1;
                          else if ( !strncmp (argv[i], "acuwork", 7) )
                            (*lethFlags)->out  =  acuwork;
                          else if ( !strncmp (argv[i], "ethiosys", 8) )
                            (*lethFlags)->out  =  ethiosys;
                          else if ( !strcmp (argv[i], "ed") )
                            (*lethFlags)->out  =  ed;
                          else if ( !strcmp (argv[i], "debug") )
                            (*lethFlags)->out = debug;
                          else if ( !strcmp (argv[i], "tex") )
#ifdef SUPPORT_TEX
                            {
                              (*lethFlags)->out  =  etmf;
                              (*lethFlags)->docOut  =  tex;
                            }
#else
                            {
                              fprintf (stderr, "\a TeX Output Not Available By This Package \n");
                              exit(0);
                            }
#endif

                          else if ( !strcmp (argv[i], "latex") )
#ifdef SUPPORT_TEX
                            {
                              (*lethFlags)->out  =  etmf;
                              (*lethFlags)->docOut  =  ltx;
                            }
#else
                            {
                              fprintf (stderr, "\a LaTeX Output Not Available By This Package \n");
                              exit(0);
                            }
#endif
                          else if ( !strcmp (argv[i], "sera") )
                            (*lethFlags)->out  =  sera;

                          break;

              case 'Q' :  (*lethFlags)->qmark = true;
                          break;
              case 'R' :  (*lethFlags)->docOut = rtf;
                          break;
              case 'S' :  if ( toupper(argv[i][2]) == 'T' )
                            {
                              (*statsFlags)->yes = true;            /* give character occurance statistics */
                              for ( j = 0; j < UNITOTAL; j++ )
                                Stats[j].stat = 0;
                              if ( argv[i+1] == '\0' || argv[i+1][0] == '-' )
                                break;
                              i++;

                              /*  hmmm... we need to work on making tex a
                               *  stats output option...
                               */
                              if ( strstr ( argv[i], "-html" ) )
                                (*statsFlags)->docOut  = html;
                              if ( !strncmp (argv[i], "jun", 3) )
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
                              else if ( !strncmp (argv[i], "ncic", 4) )
                                (*statsFlags)->out  =  ncic;
                              else if ( !strncmp (argv[i], "brana", 5) )
                                (*statsFlags)->out  =  branai;
                              else if ( !strncmp (argv[i], "geezigna", 8) )
                                (*statsFlags)->out  =  gezigna;
                              else if ( !strncmp (argv[i], "geez", 4) )
                                (*statsFlags)->out  =  gezii;
                              else if ( !strncmp (argv[i], "gezfree", 7) )
                                (*lethFlags)->out  =  gezfree1;
                              else if ( !strncmp (argv[i], "acuwork", 7) )
                                (*statsFlags)->out  =  acuwork;
                              else if ( !strncmp (argv[i], "ethiosys", 8) )
                                (*statsFlags)->out  =  ethiosys;
                              else if ( !strncmp (argv[i], "ed", 2) )
                                (*statsFlags)->out  =  ed;
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
                              fprintf (stderr, "\a Recompile sera2any and libeth with ``make withdos'' \n");
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
                          fprintf (stderr, "\n\tUseage:  sera2any option[s] file\n");
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
                          fprintf (stderr, "\t      gezfree = Ge'ezFree (Ethiopian ASCII)\n");
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
                          fprintf (stderr, "\t      washra = Washra (Ethiopian ASCII)\n");
                          fprintf (stderr, "\tTo set starting language:\n");
                          fprintf (stderr, "\t   -l iso639-name \n");
                          fprintf (stderr, "\t      am = amh = Amharic \n");
                          fprintf (stderr, "\t      gz = gez = Ge'ez   \n");
                          fprintf (stderr, "\t      la = lat = Latin   \n");
                          fprintf (stderr, "\t      ti = tir = Tigrigna\n");
                          fprintf (stderr, "\tTo force HTML conversion:\n");
                          fprintf (stderr, "\t   -html\n");
                          fprintf (stderr, "\t   or append -html to encoding name as in:\n");
                          fprintf (stderr, "\t      -o jis-html\n");
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


/*
 *  Simple routine to prefix Java escape sequences to post-ANSI chars
 *  and then write them to the passed output stream.
 *    This may or may not be useful...
 */
int
fout_java (fstring, fp, lethFlags)
  FCHAR* fstring;
  FILE* fp;
  LEthFlags* lethFlags;
{

  int i=-1, count=0;

  if ( fp == NULL || feof(fp) || fstring == NULL || lethFlags == NULL )
    return ( _LET_FAIL );
  
  while ( fstring[++i] )
    if ( fstring[i] < ANSI )
      fprintf (fp, "%c", fstring[i]);
    else
      {
        count++;
		if (lethFlags->out == java)            /* lowercase */
          fprintf (fp, "\\u%4x", fstring[i]);
        else                                   /* UPPERCASE */
          fprintf (fp, "\\u%4X", fstring[i]);
      }

  return ( count );

}

/* extern LETCHAR NCICFidel[], ALXEtFidel[], GeezignaFidel[], EthioSysFidel[], GeezFreeFidel[];
*/
extern LETCHAR GeezignaFidel[], EthioSysFidel[], GeezFreeFidel[], NCIFidel[], NCICFidel[], ALXEtFidel[];

/*
 *  Simple routine to prefix Java escape sequences to post-ANSI chars
 *  and then write them to the passed output stream.
 */
int
fout_rtf (fp, statsFlags, lethFlags)
  FILE* fp;
  STATSFlags* statsFlags;
  LEthFlags* lethFlags;
{
#define ROMAN "Times New Roman"
#define FONTSIZE  "\\fs20 "        /* hackers can change this, 20 = 10, RTF seems to use a "divide by 2" system.. */
                                   /* DO NOT DELETE SPACE, " ", AT THE END!! */

int i, j, test = true;
FCHAR *uniString, mapAddr;
LETCHAR* myCharmap;
unsigned char *coco, rtfAddress;
char* FidelFonts[] = {            /* at the moment I don't want to relate these to the enum-ed values */
        " AGF - Zemen",
        " ALXethiopian",
        "\\fcharset2 Geezigna",
        "\\fcharset2 Washra  Primary",
        "\\fcharset2 Washrax Secondary",
        " Ge'ezFree Zemen Primary",
        " Ge'ezFree Zemen Secondary",
        " ET-NEBAR Normal"
};
char defRoman[80], defFidel[80], defFidel2[80], printString[80];
enum Fonts {Fagafari, Falxet, Ffeedel, Fgezfree1, Fgezfree2, Fnci, Froman, Fwashra, Fwashrax} fidelFont, lastFont;



  /*
   *  Write Obligatory RTF Header      maybe we need \f instead of \r ?
   *  This is based on saving sample text with the Win '95 Word Pad.
   *  Any of this changes later. The fidel font is now fixed as "f4".
   */
  fprintf (stdout, "{\\rtf1\\ansi\\deff0\\deftab720{\\fonttbl{\\f0\\fnil MS Sans Serif;}{\\f1\\fnil\\fcharset2 Symbol;}{\\f2\\fswiss\\fprq2 System;}{\\f3\\fnil %s;}", ROMAN );

  switch (lethFlags->out)  /* do some font/coding specific tasks here */
    {
      case nci :
        myCharmap = NCIFidel;
		fidelFont = Fnci;
        fprintf ( stdout, "{\\f4\\fnil\\fprq2%s;}}\r\n", FidelFonts[0] );
        break;

      case ncic :
        myCharmap = NCICFidel;
		fidelFont = Fagafari;
        fprintf ( stdout, "{\\f4\\fnil\\fprq2%s;}}\r\n", FidelFonts[0] );
        break;

      case acuwork :
        myCharmap = ALXEtFidel;
		fidelFont = Falxet;
        fprintf ( stdout, "{\\f4\\fnil\\fprq2%s;}}\r\n", FidelFonts[1] );
        break;

      case gezigna :
        myCharmap = GeezignaFidel;
		fidelFont = Ffeedel;
        fprintf ( stdout, "{\\f4\\fnil\\fprq2%s;}}\r\n", FidelFonts[2] );
        break;

      case ethiosys :
/*      myCharmap = EthioSysFidel; */
		fidelFont = Fwashra;
        fprintf ( stdout, "{\\f4\\fnil\\fprq2%s;}", FidelFonts[3] );
        fprintf ( stdout, "{\\f5\\fnil\\fprq2%s;}}\r\n", FidelFonts[4] );
        break;

      case gezfree1 :
		fidelFont = Fgezfree1;
        fprintf ( stdout, "{\\f4\\fnil\\fprq2%s;}", FidelFonts[5] );
        fprintf ( stdout, "{\\f5\\fnil\\fprq2%s;}}\r\n", FidelFonts[6] );
        break;

      default:
        break;
    }

  fprintf ( stdout, "{\\colortbl\\red0\\green0\\blue0;}\r\n" );
  fprintf ( stdout, "\\deflang1033\\pard" );


  /*
   *  Set up these recurring RTF strings, essentially they now work like
   *  sera language switches.
   */

  strcpy ( defRoman, "\\plain\\f3" );
  strcat ( defRoman, FONTSIZE );
  strcpy ( defFidel, "\\plain\\f4" );
  strcat ( defFidel, FONTSIZE );
  if ( lethFlags->out == ethiosys || lethFlags->out == gezfree1 )
    {
      strcpy ( defFidel2, "\\plain\\f5" );  
      strcat ( defFidel2, FONTSIZE );  
    }

  /*
   *  Assume text starts in Ge'ez...
   */
  lastFont = fidelFont;
  fprintf ( stdout, defFidel );


  /* 
   *   Let's Read And Write!! 
   */

  while (!feof(fp) && test)
   {
     test = fidel_fgets ( &uniString, WSIZE, fp, lethFlags );
     i = 0;
     while ( uniString[i] )
	   /*
	    *  We do this one char at a time to handle multiple ethiopic fonts simply...
		*/
       {
	     printString[0] = '\0';
         fidel_sputc ( uniString[i], &coco, lethFlags );

         j = 0;
         while ( coco[j] )
           {
             rtfAddress = coco[j];
             if ( isethio (uniString[i]) )
               {
			     mapAddr =  uniString[i];
			     mapAddr =  (isprivate(mapAddr))  
                         ?  (UNITOTAL-1)  + (mapAddr - PRIVATE_USE_END)
                         :  (mapAddr - UNIFIDEL)
                         ;
                 if (lethFlags->out == ethiosys)
                   {
                     if ( EthioSysFidel[mapAddr].set == SET1 )
                       fidelFont = Fwashrax;
                     else
                       fidelFont = Fwashra;
                   }
                 if (lethFlags->out == gezfree1)
                   {
                     if ( GeezFreeFidel[mapAddr].set == SET1 )
                       fidelFont = Fgezfree2;
                     else
                       fidelFont = Fgezfree1;
                   }
                 if (lastFont != fidelFont)
                   {
                     if ( fidelFont == Fwashrax || fidelFont == Fgezfree2 )
                       strcat ( printString, defFidel2 );
                     else
                       strcat ( printString, defFidel );
                   }
                 strcat ( printString, RTFMAP[rtfAddress] );
                 lastFont = fidelFont;
               }
             else if ( coco[j] > ' ' )
               {
                 if (lastFont != Froman)
                   strcat ( printString, defRoman );
                 strcat ( printString, RTFMAP[rtfAddress] );
                 lastFont = Froman;
               }
             else  /* ASCII control char <= ' ' */
               {
/*               if ( coco[j] == '\n' )             /* this does DOS->UNIX */
/*                 sprintf ( printString, "%s\r\n\\par", defRoman );  
                   strcat ( printString, "\\\n" );  // returns must be in roman -? //
                 else
                   strcat ( printString, RTFMAP[rtfAddress] ); */
/*               lastFont = Froman; */
               }

             j++;
		   }  /* end while (coco[j]) */
         free ((unsigned char *)coco);
         fprintf ( stdout, "%s", printString );
         i++;
       }  /* end while (uniString[i]) */


     if (statsFlags->yes)
       {
         i = 0;
         while ( uniString[i] )
           {
             if ( isethio (uniString[i]) )
               if ( isprivate(uniString[i]) )
                 Stats[(UNITOTAL-1) + (uniString[i] - PRIVATE_USE_END)].stat ++;
               else
                 Stats[uniString[i]-UNIFIDEL].stat ++;
             i++;
           }
       }
     free ((FCHAR *)uniString);
   }  /* end while (!feof(inptr) && test) */

  fprintf ( stdout, "\\par }\n" );  /* close this file and we're done! */

  if (statsFlags->yes) 
    print_stats ( statsFlags, lethFlags );

/*-------Lets Be A Good Citizen-----------------*/

  fclose (fp);
  free ((LEthFlags *)lethFlags);
  free ((STATSFlags *)statsFlags);
  exit (0);
}


#ifdef  SUPPORT_TEX

/*
 *  Simple routine to write files with tex macros for Ethiopic.
 *  This escapes correspond to those in the "fidelmap.tex" file.
 *  La/TeX support is limited at this time, some formatting will
 *  suffer or need revision afterwards.
 */
int
fout_tex (fstring, fp, lethFlags)
  FCHAR* fstring;
  FILE* fp;
  LEthFlags* lethFlags;
{

  int i=-1, count=0;
  FCHAR fch;

  if ( fp == NULL || feof(fp) || fstring == NULL || lethFlags == NULL )
    return ( _LET_FAIL );
  
  while ( fch = fstring[++i] )
    if ( fch < ANSI )
      fprintf (fp, "%c", fstring[i]);
    else if ( fch >= UNIFIDEL && fch < UNIFIDEL+UNITOTAL )
      {
        count++;
        fch -= UNIFIDEL;
        if ( fstring[i+1] == GEMINATION )
		  {
            fprintf ( fp, "\\geminateG{\\%s}", TeXFidel[fch] );
			i++;
		  }
        else
          fprintf ( fp, "{\\%s}", TeXFidel[fch] );
      }
    else if ( fch >= PRIVATE_USE_BEGIN && fch <= PRIVATE_USE_END )
      {
        count++;
        fch  = (UNITOTAL-1)  + (fch - PRIVATE_USE_END);
        fprintf ( fp, "{\\%s}", TeXFidel[fch] );
      }

  return ( count );

}

#endif  /* SUPPORT_TEX */
 
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


 /*****************************************************************************/
/*****************************************************************************/
/*  check_lang (target, n, fp, mylflags)
/*
/*  This routine is the simple analog of fgets for the FCHAR data type.
/*  This routine scans a single string of length ``n'' from the file pointed 
/*  to by ``fp''.  ``n'' is also the return value.
/*
/*  target   -is the address of an FCHAR pointer whom we will direct to an FCHAR
/*            output string allocated from memory by this routine.
/*  n        -is the number of fchars to copy.
/*  fp       -is a file pointer to SERA text
/*  mylflags -is a SERA Flag structure which MUST contain a valid input and
/*            output type.
/*
/******************************************************************************/
/*****************************************************************************/


int
check_lang (target, n, fp, mylflags)
  FCHAR** target;
  int n;
  FILE* fp;
  LEthFlags* mylflags;
{

FCHAR start = 0;
register int i;
FCHAR fch = 0, *holdText;
char* = fileText;


  if ( ferror(fp) || feof(fp) || fp == NULL )
    return ( _LET_FAIL );
  if ( mylflags == NULL )                  /* create or use internal   */
    {
	  if ( libeflags == NULL )
        libeflags =  set_defaultLEthFlags (LETLANGMAJOR, LETLANGMINOR, LETLANGMAJOR);
	  mylflags = libeflags;
    }

  if ( n < 1 )
    return ( _LET_FAIL );

  holdText = ( FCHAR * ) malloc ( (n+1) * sizeof (FCHAR) );

  start = check_fidel_ungetc ( fp );              /* reget ungot char, if any */

  if ( start )                                    /* if token returned, add to start of list */
    {
      holdText[0] = start;
      start = 1;
    }


  startLang = fileText = (char *) malloc ( (start+n) * sizeof (char) );

  fgets (fileText, n, fp);

  while ( fileText < fileTextEnd )
    {

  if ( (startLang = strstr(fileText, "<lang")) )
    {
      /* we got it, verify and find closing ">" */
    }
  else
    {
      /*   check for substring such as "<l" if found near end read another
       *   "n" chars, verify and find closing ">" 
       */
    }

  while ( fileText < startLang )
    {
      for (i = start; i < n && (fch = get_fchar(NULL, fp, mylflags)) != 0; i++)
        holdText[i] = fch; 
    }

  while ( startLang < endLang )  // sorta...  from ">" + 1
    {
      /* copy direct */
    }

  fileText = endLang + 7;

    }


  *target = ( FCHAR * ) malloc ( (i+1) * sizeof (FCHAR) );

  fidel_strncpy (*target, holdText, i);
  (*target)[i] = '\0';

  free ((FCHAR *)holdText);

  // if (fch == EOF || fch == 0)
  if ( feof(fp) || fch == 0)
    return (0);


  return ( i );

}

