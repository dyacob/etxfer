#include "gezxfer.h"


extern char* rename_header ( enum Fonts Fout, struct FontInfoList* fi, struct FontInfo* (*changeList)[], char* header );
extern char* parse_line ( struct FontInfo* (*changeList)[], char** rtfText, struct FontInfo** inGez, char** appendRTF );
extern void replace_rtfFlags ( unsigned char** rtfText, char* prefix, char* suffix, enum Fonts Fout, struct FontInfo* inGez, struct FontInfoList* fi );
extern void strip_rtf ( char** rtfText );
extern void get_rtfIds ( struct FontInfoList* fi, char* header );

#define  _debugf( String )  printf String 

extern struct FontInfo Roman;

void
rtf2rtf ( Fout, fi, changeList, rtfText, inFile, outFile, isGez, fidelStats, statsFlags, lethFlags, sectionID )
  enum Fonts Fout, lastFont;
  struct FontInfoList* fi;
  struct FontInfo* (*changeList)[];
  char* rtfText;
  FILE* inFile;
  FILE* outFile;
  struct FontInfo** isGez;
  FIDELStats  fidelStats[];
  STATSFlags* statsFlags;
  LEthFlags*  lethFlags;
  int sectionID;
{
struct FontInfo *newGez;
FCHAR* uniString=NULL;
unsigned char* rtfOut=NULL;
char *outString=NULL, *lastString=NULL, *setNull=NULL, *nullTest=NULL;
char *Begin=NULL, *End=NULL, *lineSegment=NULL, *appendRTF=NULL, *header=NULL, *markBegin=NULL;
register int i;

  lastFont = fi->top = fi->font[Fout];

 fprintf (stderr, "In rtfOut [%i]! \n", sectionID);

  if (sectionID == HEADER)
    {
      outString = rename_header ( Fout, fi, changeList, rtfText );
    }
  else if (sectionID == RTFLINE)
    {
      newGez = *isGez;
      setNull = (char *) malloc (1 * sizeof (char));
      setNull[0] = '\0';
      appendRTF = lastString = outString = setNull;

      Begin = (char *) malloc ( (strlen(rtfText) + 1) * sizeof(char) );
      fprintf (stderr, "  After Begin Malloc\n" );
	  markBegin = Begin;
      strcpy ( Begin, rtfText );
      End   = Begin + strlen(Begin);  /* fix this later...*/

      while ( Begin < End )
        {
          fprintf (stderr, "  New Cycle Before [%s][%s][%s]\n", lastString, rtfOut, appendRTF );
          lineSegment = parse_line ( changeList, &Begin, &newGez, &appendRTF );
          fprintf (stderr, "  New Cycle After [%s][%s][%s]\n", lastString, rtfOut, appendRTF );
          if ( (*isGez)->setId == Froman )                 /* translate for gez font */
            {
              fprintf (stderr, "  isGez == Froman [%i]\n", (*isGez)->setId );
              fprintf (stderr, "  outString = [%i : %s]\n", strlen(outString), outString);
              fprintf (stderr, "  lineSegment = [%i : %s]\n", strlen(lineSegment), lineSegment);

              fprintf (stderr, "  Before Malloc\n" );
              outString = (char *) malloc (
                           ( strlen(lastString)   /* last size           */
                           + strlen(lineSegment)  /* size increment      */
                           + 1 )                  /* to store a \0       */
                           * sizeof(char) );
              fprintf (stderr, "  After Malloc\n" );

              strcpy ( outString, lastString );   /* copy old            */
              strcat ( outString, lineSegment );  /* cat new             */

            }
          else    /* isGez != Froman */
            {   
              /*  The problem here is that isGez has changed to Ethiopic but
               *  the lineSegment text is still Roman.  This occurs each time
               *  the script changes.  Better make a "lastGez" variable to
               *  the isGez flag prior to change.
               */
              fprintf (stderr, "  isGez != Froman [%i]\n", (*isGez)->setId );
              fprintf (stderr, "  lineSegment = [%s]\n", lineSegment );

/*            fprintf (stderr, "  Before rtfOut Malloc\n" );
              rtfOut = (unsigned char *) malloc( (strlen(lineSegment)+1) * sizeof(char) );
              fprintf (stderr, "  After rtfOut Malloc\n" );
              outString = (char *) malloc( (strlen(lineSegment)+4) * sizeof(char) );
              fprintf (stderr, "  After outString Malloc\n" );
              strcpy ( rtfOut, lineSegment );
*/
              fprintf (stderr, "  Before sget_fstring\n" );
              lethFlags->in = (*isGez)->charCode;
              sget_fstring ( &uniString, lineSegment, lethFlags ); 
              fprintf (stderr, "  After sget_fstring\n" );
              fprintf (stderr, "  Before fidel_sputs\n" );
//            no, a lookup is needed to go from Fchar to encoding...
              fidel_sputs ( uniString, &rtfOut, lethFlags );
              fprintf (stderr, "  After fidel_sputs\n" );

              if ( ( nullTest = (char *)strstr ( rtfOut, "{\\fetas2" ) ) != NULL )
                /* replace_rtfFlags (&rtfOut, "\\plain", "\\fs20");
                 *
                 *  Skip the prefix, suffix for now.  A test with WordPad shows
                 *  the extra specifiers are NOT required (though it uses them)
                 *  when fonts change.  It would be more work to keep track of
                 *  the current specifier...
                 */
                replace_rtfFlags (&rtfOut, "\0", "\0", Fout, *isGez, fi);

              fprintf (stderr, "  Before Malloc\n" );
              outString = (char *) malloc ( 
                           ( strlen(lastString)   /* last size           */
                           + strlen(rtfOut)       /* ethiopic increase   */
                           + strlen(appendRTF)    /* unknown RTF escapes */
                           + 1 )                  /* to store a \0       */
                           * sizeof(char) );
              fprintf (stderr, "  After Malloc\n" );

              fprintf (stderr, "  Catting [%s][%s][%s]\n", lastString, rtfOut, appendRTF );
              strcpy ( outString, lastString );   /* copy old  */
              strcat ( outString, rtfOut     );   /* cat new   */
              strcat ( outString, appendRTF );    /* cat extra */

              fprintf (stderr, "  Before Append Free\n" );
              if ( appendRTF != setNull )
                {
                  free ( (char *)appendRTF  );
                  appendRTF = setNull;
                }
              fprintf (stderr, "  After Append Free\n" );
              if (statsFlags->yes)
                {
                  i = 0;
                  while ( uniString[i] )
                    {
                      if ( isethio (uniString[i]) )
                        if ( isprivate(uniString[i]) )
                          fidelStats[(UNITOTAL-1) + (uniString[i] - PRIVATE_USE_END)].stat ++;
                        else
                          fidelStats[uniString[i]-UNIFIDEL].stat ++;
                      i++;
                    }
                } 
              free ( (FCHAR *)uniString );
              free ( (unsigned char *)rtfOut );

            }    /* isGez != Froman */

          fprintf (stderr, "  Freeing Last Line & Line Segment\n" );
          if ( lastString != setNull )            /* first cycle check */
            free ( (char *)lastString );
          free ( (char *)lineSegment );
          lastString = outString;                 /* old points to new */
          *isGez = newGez;

        }  /*  end while ( Begin < End )  */

      fprintf (stderr, "  Freeing Mark Begin\n" );
      free ( (char *)markBegin );
      free ( (char *)setNull );
    }
  else  /*  I don't know what we're dealing with...  */
    {
      fprintf (stderr, "Can Not Process RTF Body Section : %s", rtfText); 
      exit (1);
    }


  fprintf (stderr, "\noutstring = [%s]\n", outString);
  //fputs (outString, outFile);
  fprintf (outFile, "%s", outString);
  free ( (char *)outString );
  fprintf (stderr, "Leave rtfOut!\n\n");

  return;
   
}



void
rtf2plain ( Fout, fi, changeList, rtfText, inFile, outFile, isGez, fidelStats, statsFlags, lethFlags, sectionID )
  enum Fonts Fout;
  struct FontInfoList* fi;
  struct FontInfo* (*changeList)[];
  char* rtfText;
  FILE* inFile;
  FILE* outFile;
  struct FontInfo** isGez;
  FIDELStats  fidelStats[];
  STATSFlags* statsFlags;
  LEthFlags*  lethFlags;
  int sectionID;
{
struct FontInfo* newGez; 
FCHAR* uniString=NULL;
unsigned char* plainOut=NULL;
char *outString=NULL, *lastString=NULL, *setNull=NULL;
char *Begin=NULL, *End=NULL, *lineSegment=NULL, *appendRTF=NULL, *header=NULL, *markBegin=NULL;
register int i;



 fprintf (stderr, "In plainOut [%i]! \n", sectionID);

  if (sectionID == HEADER)       /* don't use it... */
    {
      get_rtfIds ( fi, rtfText );
      return;
    }
  else if (sectionID == RTFLINE)
    {
      lethFlags->verbatim = true;
      // lethFlags->docOut = plain;  already set?
      newGez = *isGez;
      setNull = (char *) malloc (1 * sizeof (char));
      setNull[0] = '\0';
      appendRTF = lastString = outString = setNull;

      Begin = (char *) malloc ( (strlen(rtfText) + 1) * sizeof(char) );
      fprintf (stderr, "  After Begin Malloc\n" );
	  markBegin = Begin;
      strcpy ( Begin, rtfText );
      End   = Begin + strlen(Begin);  /* fix this later...*/

      while ( Begin < End )
        {
          fprintf (stderr, "  New Cycle Before [%s][%s]\n", lastString, plainOut );
          lineSegment = parse_line ( changeList, &Begin, &newGez, &appendRTF );
          fprintf (stderr, "  New Cycle After [%s][%s]\n", lastString, plainOut );
          if ( (*isGez)->setId == Froman )                 /* translate for gez font */
            {

              fprintf (stderr, "  isGez == Froman [%i]\n", (*isGez)->setId );
              fprintf (stderr, "  outString = [%i : %s]\n", strlen(outString), outString);
              fprintf (stderr, "  lineSegment = [%i : %s]\n", strlen(lineSegment), lineSegment);

              fprintf (stderr, "  Before Malloc\n" );
              strip_rtf ( &lineSegment );
              outString = (char *) malloc (
                           ( strlen(lastString)   /* last size           */
                           + strlen(lineSegment)  /* size increment      */
                           + 1 )                  /* to store a \0       */
                           * sizeof(char) );
              fprintf (stderr, "  After Malloc\n" );

              strcpy ( outString, lastString );   /* copy old            */
              strcat ( outString, lineSegment );  /* cat new             */

            }
          else    /* isGez != Froman */
            {   
              fprintf (stderr, "  isGez != Froman [%i]\n", (*isGez)->setId );
              fprintf (stderr, "  lineSegment = [%s]\n", lineSegment );

              fprintf (stderr, "  Before sget_fstring\n" );
              sget_fstring ( &uniString, lineSegment, lethFlags ); 
              fprintf (stderr, "  After sget_fstring\n" );
              fprintf (stderr, "  Before fidel_sputs\n" );
              fidel_sputs ( uniString, &plainOut, lethFlags );
              fprintf (stderr, "  After fidel_sputs\n" );

              fprintf (stderr, "  Before Malloc\n" );
              outString = (char *) malloc ( 
                           ( strlen(lastString)   /* last size           */
                           + strlen(plainOut)       /* ethiopic increase   */
                           + 1 )                  /* to store a \0       */
                           * sizeof(char) );
              fprintf (stderr, "  After Malloc\n" );

              fprintf (stderr, "  Catting [%s][%s]\n", lastString, plainOut );
              strcpy ( outString, lastString );   /* copy old  */
              strcat ( outString, plainOut     );   /* cat new   */

              fprintf (stderr, "  Before Append Free\n" );
              if ( appendRTF != setNull )
                {
                  free ( (char *)appendRTF  );
                  appendRTF = setNull;
                }
              fprintf (stderr, "  After Append Free\n" );

              if (statsFlags->yes)
                {
                  i = 0;
                  while ( uniString[i] )
                    {
                      if ( isethio (uniString[i]) )
                        if ( isprivate(uniString[i]) )
                          fidelStats[(UNITOTAL-1) + (uniString[i] - PRIVATE_USE_END)].stat ++;
                        else
                          fidelStats[uniString[i]-UNIFIDEL].stat ++;
                      i++;
                    }
                } 
              free ( (FCHAR *)uniString );
              free ( (unsigned char *)plainOut );

            }    /* isGez != Froman */

          fprintf (stderr, "  Freeing Last Line & Line Segment\n" );
          if ( lastString != setNull )            /* first cycle check */
            free ( (char *)lastString );
          free ( (char *)lineSegment );
          lastString = outString;                 /* old points to new */
          *isGez = newGez;

        }  /*  end while ( Begin < End )  */

     /*   need to filter "unknown" RTF escapes 
      *   i.e. ones we can not convert back into chars 
      */ 

      fprintf (stderr, "  Freeing Mark Begin\n" );
      free ( (char *)markBegin );
      free ( (char *)setNull );
    }
  else  /*  I don't know what we're dealing with...  */
    {
      fprintf (stderr, "Can Not Process RTF Body Section : %s", rtfText); 
      exit (1);
    }


  fprintf (stderr, "\noutstring = [%s]\n", outString);
  fprintf (outFile, "%s", outString);
  free ( (char *)outString );
  fprintf (stderr, "Leave plainOut!\n\n");

  return;
   
}


void
plain2plain ( Fout, fi, changeList, rtfText, inFile, outFile, isGez, fidelStats, statsFlags, lethFlags, sectionID )
  enum Fonts Fout;
  struct FontInfoList* fi;
  struct FontInfo* (*changeList)[];
  char* rtfText;
  FILE* inFile;
  FILE* outFile;
  struct FontInfo** isGez;
  FIDELStats  fidelStats[];
  STATSFlags* statsFlags;
  LEthFlags*  lethFlags;
  int sectionID;
{
struct FontInfo* newGez; 
FCHAR* uniString=NULL;
unsigned char* plainOut=NULL;
char *plainString=NULL, *lastString=NULL, *setNull=NULL;
char *Begin=NULL, *End=NULL, *lineSegment=NULL, *appendRTF=NULL, *header=NULL, *markBegin=NULL;
int test=true;
register int i;


  if (sectionID == HEADER)       /* don't use it... */
    {
      return;
    }
  else if (sectionID == PLAINLINE)
    {
      while (!feof(inFile) && test)
       {
         test = fidel_fgets (&uniString, WSIZE, inFile, lethFlags);
         fidel_fputs (uniString, outFile, lethFlags);

         if (statsFlags->yes)
           {
             i = 0;
             while ( uniString[i] )
               {
                 if ( isethio (uniString[i]) )
                   if ( isprivate(uniString[i]) )
                     fidelStats[(UNITOTAL-1) + (uniString[i] - PRIVATE_USE_END)].stat ++;
                   else
                     fidelStats[uniString[i]-UNIFIDEL].stat ++;
                 i++;
               }
           } 
         free ((FCHAR *)uniString); 
       }

    }

  return;

}



/*
 *  NOTE!!!!!!!  This has NO check for changes to LATIN!!!!!!!!!!!!!!!
 */
void
plain2rtf ( Fout, fi, changeList, rtfText, inFile, outFile, isGez, fidelStats, statsFlags, lethFlags, sectionID )
  enum Fonts Fout;
  struct FontInfoList* fi;
  struct FontInfo* (*changeList)[];
  char* rtfText;
  FILE* inFile;
  FILE* outFile;
  struct FontInfo** isGez;
  FIDELStats  fidelStats[];
  STATSFlags* statsFlags;
  LEthFlags*  lethFlags;
  int sectionID;
{
struct FontInfo* newGez; 
FCHAR* uniString=NULL;
unsigned char* rtfOut=NULL;
char *outString=NULL, *lastString=NULL, *setNull=NULL, *nullTest=NULL, fontCh='4';
char *Begin=NULL, *End=NULL, *lineSegment=NULL, *appendRTF=NULL, *header=NULL, *markBegin=NULL;
int test=true, fontNum=4;
register int i;


  lastFont = fi->top = fi->font[Fout];

  if (sectionID == HEADER)       /* make header */
    {
      fprintf ( outFile, "{\\rtf1\\ansi\\deff0\\deftab720{\\fonttbl{\\f0\\fnil MS Sans Serif;}{\\f1\\fnil\\fcharset2 Symbol;}{\\f2\\fswiss\\fprq2 System;}{\\f3\\fnil %s;}", Roman.setName );
      for ( i=0; i < fi->font[Fout]->numSets; i++ )
        {
          strcat ( fi->font[Fout+i]->rtfId, &fontCh );
          fontCh++;
          if (fi->font[Fout+i]->rtfPrefix == NULL) 
            fprintf ( outFile, "{%s\\fnil\\frq2 %s;}}\r\n",
                      fi->font[Fout+i]->rtfId, fi->font[Fout+i]->setName );
          else
            fprintf ( outFile, "{%s\\fnil\\frq2%s%s;}}\r\n",
                      fi->font[Fout+i]->rtfId, fi->font[Fout+i]->rtfPrefix, fi->font[Fout+i]->setName );
        }
      fprintf ( outFile, "{\\colortbl\\red0\\green0\\blue0;}\r\n" );
      fprintf ( outFile, "\\deflang1033\\pard\\plain\\f4\\fs20 " );
      return;
    }
  else if (sectionID == PLAINLINE)
    {
      while (!feof(inFile) && test)
       {
         test = fidel_fgets ( &uniString, WSIZE, inFile, lethFlags );

         fidel_sputs ( uniString, &rtfOut, lethFlags );

         /* if this is ethiopic */  *isGez = fi->font[Fout];
         // else   *isGez = Roman;
         if ( ( nullTest = (char *)strstr ( rtfOut, "{\\fetas2" ) ) != NULL )
           replace_rtfFlags (fi, &rtfOut, lastFont) ;

         fprintf ( outFile, "%s", rtfOut );

         if (statsFlags->yes)
           {
             i = 0;
             while ( uniString[i] )
               {
                 if ( isethio (uniString[i]) )
                   if ( isprivate(uniString[i]) )
                     fidelStats[(UNITOTAL-1) + (uniString[i] - PRIVATE_USE_END)].stat ++;
                   else
                     fidelStats[uniString[i]-UNIFIDEL].stat ++;
                 i++;
               }
           } 
         free ((FCHAR *)uniString); 
         free ((unsigned char *)rtfOut); 
         lastFont = fi->top;
       }

    }


  fprintf ( stdout, "\\par }\n" );  /* close this file and we're done! */

  return;

}



#ifdef  SUPPORT_TEX


void
plain2tex ( Fout, fi, changeList, rtfText, inFile, outFile, isGez, fidelStats, statsFlags, lethFlags, sectionID )
  enum Fonts Fout;
  struct FontInfoList* fi;
  struct FontInfo* (*changeList)[];
  char* rtfText;
  FILE* inFile;
  FILE* outFile;
  struct FontInfo** isGez;
  FIDELStats  fidelStats[];
  STATSFlags* statsFlags;
  LEthFlags*  lethFlags;
  int sectionID;
{
struct FontInfo* newGez; 
FCHAR* uniString=NULL;
char* texOut=NULL;
char *outString=NULL, *lastString=NULL, *setNull=NULL, *nullTest=NULL, fontCh=4;
char *Begin=NULL, *End=NULL, *lineSegment=NULL, *appendRTF=NULL, *header=NULL, *markBegin=NULL;
int test=true, fontNum=4;
register int i;



  if (sectionID == HEADER)       /* make header */
    {
      if (lethFlags->docOut == ltx) 
        {
          fprintf ( outFile, "\\documentstyle{report} \n\\begin{document}\n\n" );
          fprintf ( outFile, "\\newfont{\\fidel}{%s%sio%d.%dpk}\n", TEXFONTDIR, FONTPREFIX, TEXFONTSIZE, FONTDPI );
          fprintf ( outFile, "\\newfont{\\fidelx}{%s%sxx%d.%dpk}\n", TEXFONTDIR, FONTPREFIX, TEXFONTSIZE, FONTDPI );
          fprintf ( outFile, "\\include{fidelmap}\n" );
          fprintf ( outFile, "\\def\\noi{\\noindent}\n" );
        }
      else
        {
          fprintf ( outFile, "\\font\\fidel=%s%sio%d.%dpk\n", TEXFONTDIR, FONTPREFIX, TEXFONTSIZE, FONTDPI );
          fprintf ( outFile, "\\font\\fidelx=%s%sxx%d.%dpk\n", TEXFONTDIR, FONTPREFIX, TEXFONTSIZE, FONTDPI );
          fprintf ( outFile, "\\def\\noi{\\noindent}\n" );
          fprintf ( outFile, "\\def\\nl{\\noi\\par}\n" );
          fprintf ( outFile, "\\input fidelmap.tex\n" );
          fprintf ( outFile, "\\noi\n" );
        }

      return;
    }
  else if (sectionID == PLAINLINE)
    {
      while (!feof(inFile) && test)
       {
         test = fidel_fgets ( &uniString, WSIZE, inFile, lethFlags );
         fidel_tex_sputs ( uniString, &texOut, lethFlags );

         fprintf ( outFile, "%s", texOut );

         if (statsFlags->yes)
           {
             i = 0;
             while ( uniString[i] )
               {
                 if ( isethio (uniString[i]) )
                   if ( isprivate(uniString[i]) )
                     fidelStats[(UNITOTAL-1) + (uniString[i] - PRIVATE_USE_END)].stat ++;
                   else
                     fidelStats[uniString[i]-UNIFIDEL].stat ++;
                 i++;
               }
           } 
         free ((FCHAR *)uniString); 
         free ((unsigned char *)texOut); 
       }

    }


/* close this file and we're done! */
  if ( lethFlags->docOut == ltx )          
    fprintf ( stdout, "\n\\end{document}\n" );
  else                                          /* it's tex */
    fprintf ( stdout, "\n\\bye\n" );


  return;

}



void
rtf2tex ( Fout, fi, changeList, rtfText, inFile, outFile, isGez, fidelStats, statsFlags, lethFlags, sectionID )
  enum Fonts Fout;
  struct FontInfoList* fi;
  struct FontInfo* (*changeList)[];
  char* rtfText;
  FILE* inFile;
  FILE* outFile;
  struct FontInfo** isGez;
  FIDELStats  fidelStats[];
  STATSFlags* statsFlags;
  LEthFlags*  lethFlags;
  int sectionID;
{
struct FontInfo* newGez; 
FCHAR* uniString=NULL;
char* texOut=NULL;
char *outString=NULL, *lastString=NULL, *setNull=NULL;
char *Begin=NULL, *End=NULL, *lineSegment=NULL, *appendRTF=NULL, *header=NULL, *markBegin=NULL;



 fprintf (stderr, "In plainOut [%i]! \n", sectionID);

  if (sectionID == HEADER)       /* don't use it... */
    {
      if (lethFlags->docOut == ltx) 
        {
          fprintf ( outFile, "\\documentstyle{report} \n\\begin{document}\n\n" );
          fprintf ( outFile, "\\newfont{\\fidel}{%s%sio%d.%dpk}\n", TEXFONTDIR, FONTPREFIX, TEXFONTSIZE, FONTDPI );
          fprintf ( outFile, "\\newfont{\\fidelx}{%s%sxx%d.%dpk}\n", TEXFONTDIR, FONTPREFIX, TEXFONTSIZE, FONTDPI );
          fprintf ( outFile, "\\include{fidelmap}\n" );
          fprintf ( outFile, "\\def\\noi{\\noindent}\n" );
        }
      else
        {
          fprintf ( outFile, "\\font\\fidel=%s%sio%d.%dpk\n", TEXFONTDIR, FONTPREFIX, TEXFONTSIZE, FONTDPI );
          fprintf ( outFile, "\\font\\fidelx=%s%sxx%d.%dpk\n", TEXFONTDIR, FONTPREFIX, TEXFONTSIZE, FONTDPI );
          fprintf ( outFile, "\\def\\noi{\\noindent}\n" );
          fprintf ( outFile, "\\def\\nl{\\noi\\par}\n" );
          fprintf ( outFile, "\\input fidelmap.tex\n" );
          fprintf ( outFile, "\\noi\n" );
        }

      return;
    }
  else if (sectionID == RTFLINE)
    {
      lethFlags->verbatim = true;
      newGez = *isGez;
      setNull = (char *) malloc (1 * sizeof (char));
      setNull[0] = '\0';
      appendRTF = lastString = outString = setNull;

      Begin = (char *) malloc ( (strlen(rtfText) + 1) * sizeof(char) );
	  markBegin = Begin;
      strcpy ( Begin, rtfText );
      End = Begin + strlen(Begin);  /* fix this later...*/

      while ( Begin < End )
        {
          lineSegment = parse_line ( changeList, &Begin, &newGez, &appendRTF );
          if ( (*isGez)->setId == Froman )                 /* translate for gez font */
            {

              strip_rtf ( &lineSegment );
              outString = (char *) malloc (
                           ( strlen(lastString)   /* last size           */
                           + strlen(lineSegment)  /* size increment      */
                           + 1 )                  /* to store a \0       */
                           * sizeof(char) );

              strcpy ( outString, lastString );   /* copy old            */
              strcat ( outString, lineSegment );  /* cat new             */

            }
          else    /* isGez != Froman */
            {   

              sget_fstring ( &uniString, lineSegment, lethFlags ); 
              fidel_tex_sputs ( uniString, &texOut, lethFlags );

              outString = (char *) malloc ( 
                           ( strlen(lastString)   /* last size           */
                           + strlen(texOut)       /* ethiopic increase   */
                           + 1 )                  /* to store a \0       */
                           * sizeof(char) );

              strcpy ( outString, lastString );   /* copy old  */
              strcat ( outString, texOut     );   /* cat new   */

              if ( appendRTF != setNull )
                {
                  free ( (char *)appendRTF  );
                  appendRTF = setNull;
                }

              free ( (FCHAR *)uniString );
              free ( (unsigned char *)texOut );

            }    /* isGez != Froman */

          if ( lastString != setNull )            /* first cycle check */
            free ( (char *)lastString );
          free ( (char *)lineSegment );
          lastString = outString;                 /* old points to new */
          *isGez = newGez;

        }  /*  end while ( Begin < End )  */

      free ( (char *)markBegin );
      free ( (char *)setNull );
    }
  else  /*  I don't know what we're dealing with...  */
    {
      fprintf (stderr, "Can Not Process RTF Body Section : %s", rtfText); 
      exit (1);
    }


  fprintf (outFile, "%s", outString);
  free ( (char *)outString );

  return;
   
}




#ifdef SUPPORT_IES 

int
sout_ies (fstring, texOut, lethFlags)
  FCHAR* fstring;
  char** texOut;
  LEthFlags* lethFlags;
{
register int i=-1;
int outTop=0;
FCHAR fch;
char* outString;
phonset *iesOut;


  if ( fstring == NULL || lethFlags == NULL )
    return ( _LET_FAIL );
  
  outString = (char *) malloc ( (fidel_strlen(fstring) * 10) * sizeof(char) );

  switch ( lethFlags->top->l )
    {
      case amh:
        iesOut = IESAmharic;
        break;
      case gez:
        iesOut = IESGeez;
        break;
      case tir:
      default :
        iesOut = IESTigrigna;
        break;
    }

  while ( fch = fstring[++i] )
    if ( fch < ANSI )
      outString[outTop++] = fstring[i];
    else if ( UNIFIDEL <= fch && fch < UNIFIDEL+UNITOTAL )
      {
        fch -= UNIFIDEL;
        if ( fstring[i+1] == GEMINATION )  /* double the consonant */
          {
            sprintf ( &outString[outTop], "%s%s%s", iesOut[fch].c, iesOut[fch].c, iesOut[fch].v );
            i++;
          }
          else
            sprintf ( &outString[outTop], "%s%s", iesOut[fch].c, iesOut[fch].v );

        outTop += strlen ( &outString[outTop] );
      }
    else if ( PRIVATE_USE_BEGIN <= fch && fch <= PRIVATE_USE_END )
      {
        fch  = (UNITOTAL-1)  + (fch - PRIVATE_USE_END);
        sprintf ( &outString[outTop], "%s", iesOut[fch] );
        sprintf ( &outString[outTop], "%s%s", iesOut[fch].c, iesOut[fch].v );
      }


  outString[outTop] = '\0';
  *texOut = (char *) malloc ( (strlen(outString) +1) * sizeof(char) );
  strcpy (*texOut, outString);
  free ((char *) outString);

  return ( outTop );

}
#endif  /* SUPPORT_IES */

#endif  /* SUPPORT_TEX */
 
