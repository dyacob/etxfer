#include "gezxfer.h"
#include "rtfansi.h"

extern char* RTFMAP[];
extern struct FontInfo Roman;

char* extract_header ( FILE* inFile, LEthFlags* lethFlags );
void (* scan_header ( enum Fonts Fout, struct FontInfoList* fi, struct FontInfo* (*changeList)[], char* header, gezOutPuts gezOutList[], LEthFlags* lethFlags))() ;
char* rename_header ( enum Fonts Fout, struct FontInfoList* fi, struct FontInfo* (*changeList)[], char* header );

char* get_rtfLine ( FILE* fp );
char* parse_line ( struct FontInfo* (*changeList)[], char** rtfText, struct FontInfo** inGez, char** appendRTF );
void replace_rtfFlags ( unsigned char** rtfText, char* prefix, char* suffix, enum Fonts Fout, struct FontInfo* inGez, struct FontInfoList* fi );

void strip_rtf ( char** rtfText );
boolean rtf2ansi ( char* escape );
char* get_fontNumber ( char* headerBottom );
struct FontInfo* reset_fontId ( struct FontInfo* changeList[], char** rtfLine, struct FontInfo* isGez );
void get_rtfIds ( struct FontInfoList* fi, char* header );

#ifdef DEBUG
# define  _debugf( String )  printf String 
else
# define  _debugf( String ) 
#endif /* DEBUG */


 /*****************************************************************************/
/*****************************************************************************/
/*  extract_header ( inFile )
/*
/*  The job of extract_header is to locate and copy the header of an RTF file
/*  and return it in a string for processing.  The RTF header is considered to
/*  be the segment of a file beginning from {\rtf to {\fonttbl up to the closing
/*  } of \fonttbl .
/*
/*  inFile  - is an RTF file handle
/*
/******************************************************************************/
/*****************************************************************************/

char* 
extract_header ( inFile, lethFlags )
  FILE* inFile;
  LEthFlags* lethFlags;
{

char *header=NULL, *lastHeader=NULL, *mark=NULL;
char scan[120], tempString[120];
boolean firstCycle = true;
int open = 0, i = 0, k = 0;
char masterString[120+120], ws;
char* returnMe;



  fscanf ( inFile, "%s%c", scan, &ws );
  k = strlen ( scan );
  scan[k++] = ws;
  scan[k] = '\0';
  mark = (char *) strstr ( scan, "{\\fonttbl{" );
  if ( mark == NULL )
    {
      if ( (mark = (char *) strstr ( scan, "<html>" )) != NULL
           || (mark = (char *) strstr ( scan, "<HTML>" )) != NULL )
        lethFlags->docIn = html;
      else
        lethFlags->docIn = plain;

      rewind (inFile);
      return ( NULL );
    }
  else
    lethFlags->docIn = rtf;

  mark += 9;
  strncpy ( tempString, scan, (mark-scan) );
  tempString[mark-scan] = '\0';
  masterString[0] = 0;
  strcat ( masterString, tempString ); 
  strcpy ( scan, mark );

  do         /*  read rtf header into buffer */
    {

      mark = scan;
      while ( (mark = (char *) strchr (mark, '{')) )
        {
          open++;
          mark++;
        }
	  mark = scan;
      while ( (open || firstCycle) && (mark = (char *) strchr (mark, '}')) )
        {
          open--;
          mark++;
        }

      i = (header == NULL) ? 0 : strlen(header);
      lastHeader = header;

      if ( !open )  /* last time through */
        {
           mark = (char *) strchr (mark, '}'); 
           strncpy ( tempString, scan, (mark-scan) );
           tempString[mark-scan] = '\0';
           header = (char *) malloc ( sizeof(char) * (i + mark-scan + 1) );
           sprintf (header, "%s%s", lastHeader, tempString);
           strcpy (tempString, mark);
        }
      else         /* normal cycle */
        {
          header = (char *) malloc ( sizeof(char) * (i + strlen(scan) + 1) );
          if (firstCycle)
            {
              strcat (header, scan);
              firstCycle = false;
            }
          else
            sprintf (header, "%s%s", lastHeader, scan);

          free (lastHeader);

          fscanf ( inFile, "%s", scan  );
		  k = strlen ( scan );
		  scan[k] = fgetc ( inFile );
		  while ( isspace( scan[k] ) )
		    scan[++k] = fgetc ( inFile );
          ungetc ( scan[k], inFile );
		  scan[k] = '\0';
        }

    }
  while (open);

  strcat (masterString, header) ; 

  free ((char *)header);

  strcat (masterString, tempString) ; 
  returnMe = (char *) malloc ((strlen(masterString) + 1) * sizeof(char));
  strcpy (returnMe, masterString);
  return ( returnMe );

}


 /*****************************************************************************/
/*****************************************************************************/
/*  scan_header ( out, fi, changeList, header )
/*
/*  The job of Rename is to find and replace the input fonts' name with an
/*  output font's name.  Also extra Ethiopic fontsets are added or removed
/*  if required.
/*
/*  out         - the output font type
/*  fi          - font attributes array
/*  changeList  - list of input fonts that need to be replaced
/*  header      - the RTF header extracted by the "extract_header" routine
/*
/******************************************************************************/
/*****************************************************************************/

void
(* scan_header ( Fout, fi, changeList, header, gezOutList, lethFlags ))()
  enum Fonts Fout;
  struct FontInfoList* fi;
  struct FontInfo* (*changeList)[];
  char* header;
  gezOutPuts gezOutList[]; 
  LEthFlags* lethFlags;
{
char fontName[80];
char *Top=NULL, *End=NULL, *test;
enum Fonts top;
int i = 0, j = 0, k = 0, l = 0, numFonts = 0, gOut = NIL ;


  if ( header == NULL )
    {
      /*  Search for a conversion function to verify that we handle user's request  */
        l = 0;
        while ( gezOutList[l].func != NULL 
          && (gezOutList[l].in != lethFlags->in || gezOutList[l].out != lethFlags->out) )
          l++;

        if ( gezOutList[l].func == NULL )
          {
            fprintf ( stderr, "%d -> %d will NOT be converted!\n No Conversion Routine Available.\n", lethFlags->in, lethFlags->out );
            exit (0);
          }
        else
          return ( gezOutList[l].func );
    }

  Top = header;
  End = header + strlen(header);

  while ( Top < End && ( Top = (char *) strstr(Top, "\\f") ) != NULL ) 
    if( isdigit(Top[2]) )
    /*  this had _better_ be a font!!  */
    {
      while ( Top[0] != ' ' )
        Top++;

      Top++;  /* don't read next ' ' into fontName */
      i = l = 0;
      while ( Top[0] != ';' )
        {
          fontName[i] = Top[0];
          i++;
          Top++;
        }
      fontName[i] = '\0';

      fprintf (  stderr, "The font %s ", fontName );

      for ( i = 0 ; i < NUMFONTS && l != NIL ; i++ )
         if ( !strcmp(fi->font[i]->setName, fontName)  &&  Fout != fi->font[i]->setId )
          {
            top = i;

            for ( k = 0; k < numFonts; k++ )
              if ( (*changeList)[k] == fi->font[top] )    /*  found, skip it  */
                break;

            lethFlags->in = fi->font[top]->charCode;

          /*  Search for a conversion function to verify that we handle user's request  */
            l = 0;
            while ( gezOutList[l].func != NULL 
              && (gezOutList[l].in != lethFlags->in || gezOutList[l].out != lethFlags->out) )
              l++;

            if ( gezOutList[l].func == NULL )
              {
                fprintf ( stderr, "%d -> %d will NOT be converted!\n No Conversion Routine Available.\n", lethFlags->in, lethFlags->out );
              }
            else
              {
                fprintf ( stderr, "will be converted!\n" );
                if (gOut == NIL)
                  gOut = l;
                if ( k == numFonts )                     /* not found, add it  */
                  (*changeList)[numFonts++] = fi->font[top] ;
              }

             l = NIL;  /* just an easy way to say we were here */

          }  /* end if */

      if ( l != NIL )
        fprintf ( stderr, "will be treated as Latin.\n" );

    }  /* end if*/
  else 
    Top += 3;

  return ( gezOutList[gOut].func );

}


 /*****************************************************************************/
/*****************************************************************************/
/*  rename_header ( Fout, fi, changeList, header )
/*
/*  The job of Rename is to find and replace the input fonts' name with an
/*  output font's name.  Also extra Ethiopic fontsets are added or removed
/*  if required.
/*
/*  Fout        - the output font type
/*  fi          - font attributes array
/*  changeList  - list of input fonts that need to be replaced
/*  header      - the RTF header extracted by the "extract_header" routine
/*
/******************************************************************************/
/*****************************************************************************/

char*
rename_header ( Fout, fi, changeList, header )
  enum Fonts Fout;
  struct FontInfoList* fi;
  struct FontInfo* (*changeList)[];
  char* header;
{

char *headerBottom=NULL, *headerTop=NULL, *lastHeader=NULL,
     *mark=NULL,  *nameFound=NULL, *fontNum;
int i = 0, j = 0, k = 0, lastFont, increase, insert, numFonts, setCount = 0, prefixLength;
enum Fonts top, firstFout=Fout;

  fprintf ( stderr, "In Rename !!\n" );

  /* replace existing Ethiopic fonts with target font */

  top = NIL;
  k = numFonts = 0;

  for ( i = 0 ; i < NUMFONTS ; i++ )
      if ( (mark = (char *) strstr(header, fi->font[i]->setName) ) != NULL 
	        && ( fi->font[i]->setId < firstFout
	           || fi->font[firstFout+fi->font[firstFout]->numSets-1]->setId < fi->font[i]->setId ) )
        {
          top = i;
          nameFound = fi->font[top]->setName;
          headerTop = mark + strlen( nameFound );  /* skip over old name */

          if ( fi->font[top]->rtfPrefix != NULL )
            mark -= ( strlen( fi->font[top]->rtfPrefix ) - 1 );

          if ( fi->font[Fout]->rtfPrefix != NULL )
            mark--;

          headerBottom = (char *) malloc ( sizeof(char) * (mark - header + 1) );
          strncpy ( headerBottom, header, (mark-header) );
          headerBottom[mark-header] = '\0';

          fontNum = get_fontNumber(headerBottom);
          lastFont = atoi(fontNum);
          strcat ( fi->font[top]->rtfId, fontNum );
          strcat ( fi->font[Fout]->rtfId, fontNum );
		  free ( (char *)fontNum );

          lastHeader = header;

          prefixLength = (fi->font[Fout]->rtfPrefix == NULL) ? 0 : strlen(fi->font[Fout]->rtfPrefix) ;

          header = (char *) malloc ( ( 
                   strlen(headerBottom) 
                 + prefixLength
                 + strlen(fi->font[Fout]->setName) 
                 + strlen(headerTop) 
                 + 1 ) * sizeof(char) );

          if (fi->font[Fout]->rtfPrefix == NULL) 
            sprintf ( header, "%s%s%s", headerBottom, fi->font[Fout]->setName, headerTop );
          else
            sprintf ( header, "%s%s%s%s", headerBottom, fi->font[Fout]->rtfPrefix, fi->font[Fout]->setName, headerTop );

          if ( fi->font[Fout]->numSets > setCount )
            {
              setCount++;
              if ( fi->font[Fout]->nextSet != NULL )
                Fout = fi->font[Fout]->nextSet->setId;
              // this is a little cleaner vs using [Fout+setCount] everywhere
            }

          free (lastHeader);
          free (headerBottom);

        }  /* end if */


  /* tag extra font sets (if applicable) onto end of header */

  if ( setCount && setCount < fi->font[Fout]->numSets && top != NIL )
    {
      Fout = firstFout;
      increase = strlen(header);

      for ( i = setCount; i < fi->font[Fout]->numSets; i++ )
        {
          prefixLength = (fi->font[i]->rtfPrefix == NULL) ? 0 : strlen(fi->font[i]->rtfPrefix) ;
          increase += strlen( fi->font[Fout+i]->setName ) + prefixLength + 10 /* property length */ ;
          sprintf ( &fi->font[Fout+i]->rtfId[2], "%d", ++lastFont);
        }

      lastHeader = header;
      header = (char *) malloc ( sizeof(char) * ( increase + 1 ) );
      strcpy (header, lastHeader);
      free (lastHeader);

      for ( i = setCount; i < fi->font[Fout]->numSets; i++ )
        {
          insert = strlen(header);

          while ( header[insert] != '}' )
            insert--;
          if (fi->font[Fout+i]->rtfPrefix == NULL) 
            sprintf ( &header[insert], "{%s\\fnil\\frq2 %s;}}\r\n",
                      fi->font[Fout+i]->rtfId, fi->font[Fout+i]->setName );
          else
            sprintf ( &header[insert], "{%s\\fnil\\frq2%s%s;}}\r\n",
                      fi->font[Fout+i]->rtfId, fi->font[Fout+i]->rtfPrefix, fi->font[Fout+i]->setName );
        }
    }


  fprintf ( stderr, "Leave Rename !!\n" );

  return ( header );


}



 /*****************************************************************************/
/*****************************************************************************/
/*  get_rtfLine ( inFile )
/*
/*  The job of get_rtfLine is to read in exactly one line of text from an RTF
/*  file.  One line means up to and including the first \n.  We *ASSUME* no RTF
/*  software will split rtf tokens with newlines.
/*
/*  inFile  - is an RTF file handle
/*
/******************************************************************************/
/*****************************************************************************/

char*
get_rtfLine ( inFile )
  FILE* inFile;
{

char scan[120];
char *returnString = (char *) malloc (1 * sizeof (char));
char *tempString = returnString;

  fprintf ( stderr, "In get_rtfLine!!\n" );
  tempString[0] = '\0';

  do 
    {
       if ( fgets ( scan, 120, inFile ) == NULL ) 
         return (NULL);   /* fgets will read up to the first \n if
                           * encountered before 120 chars....      */

      returnString = (char *) malloc ( (strlen(returnString) + strlen(scan) + 1)
	                                    * sizeof(char) );

      strcpy (returnString, tempString); 
      strcat (returnString, scan); 

	  free ( (char *)tempString );
	  tempString = returnString;

    } while ( returnString[strlen(returnString)-1] != '\n' );


  fprintf ( stderr, "Leave get_rtfLine !!\n %s\n\n", returnString );
  return (returnString);


}



 /*****************************************************************************/
/*****************************************************************************/
/*  parse_line ( changeList, rtfText, isGez, appendRTF )
/*
/*  The job of parse_line is to read in exactly one line of text from an RTF
/*  file.  One line means up to and including the first \n.  We *ASSUME* no RTF
/*  software will split rtf tokens with newlines.
/*
/*  changeList  - list of input fonts that need to be replaced
/*  inFile  - is an RTF file handle
/*
/******************************************************************************/
/*****************************************************************************/

char*
parse_line ( changeList, rtfText, inGez, appendRTF )
  struct FontInfo* (*changeList)[];
  char** rtfText;
  struct FontInfo** inGez;
  char** appendRTF;
{

int i;
char *outString, *markString; 
char holdEscape[40];
struct FontInfo* isGez;
char *Begin=NULL, *End=NULL, *Top=NULL, *markFont=NULL;
char *lBrace=NULL, *rBrace=NULL, *lCheck=NULL, *rCheck=NULL, EndBrace='\0';
boolean done=false;




  holdEscape[0] = '\0';
  isGez = *inGez;
  fprintf ( stderr, "In parse_line [%i]!!\n", isGez->setId );

  Begin = *rtfText;
  Top = Begin;
  End  =  Begin + strlen (Begin);
  fprintf ( stderr, "  Begin = %s\n", Begin );

  if ( isGez->setId == Froman )
    {
  fprintf ( stderr, "  isGez == Froman\n" );
      /*  Read text until an Ethiopic font change occurs.
       *  Then return text and include the updated font ID.
       *  Deal with Ethiopic on next call to parse_line.
       */

  fprintf ( stderr, "  Start while" );
      while ( Top < End )
        {
  fprintf ( stderr, "." );
          if ( (Top = (char *) strstr(Top, "\\f")) != NULL && isdigit(Top[2]) )
            {
  fprintf ( stderr, "  found \\f \n" );
  fprintf ( stderr, "           Top = %s\n", Top );
              if ( (isGez = reset_fontId ( *changeList, &Top, isGez ))->setId != Froman)
                {
              /*  Detectected change to Ethiopic, return text upto the
               *  Ethiopic and treat Ethiopic on next call to parse_line.
               */
  fprintf ( stderr, "  Before Malloc \n" );
                  outString = (char *) malloc ( (Top - Begin + 1) * sizeof(char) );
  fprintf ( stderr, "  After Malloc \n" );
                  strncpy (outString, Begin, Top-Begin);
                  outString[Top-Begin] = '\0';
                  Begin = Top;
                  Top   = End;
                }
              /*  else text is still Latin, Top was advanced past end of \f# 
               *  by reset_fontId and we continue through the rtfText
               */
            }
          else if (Top == NULL)    /* no \f found, copy text and go home */
            {
  fprintf ( stderr, "\n  Top == NULL\n" );
              Top = End;
  fprintf ( stderr, "\n  Before Malloc\n" );
              outString = (char *) malloc ( (Top - Begin + 1) * sizeof(char) );
  fprintf ( stderr, "  After Malloc\n" );
              strcpy ( outString, Begin );
              Begin = Top;
            }
          else       /* !isdigit(Top[2]) */
            Top++;


        }  /* end while */

    }
  else  /*  isGez != Froman  */
    {
  fprintf ( stderr, "  isGez != Froman\n" );
      /*  Read text until first unkown escape.  The first unkown escape we
       *  treat as Roman text.  Escape we do know are only RTF definitions
       *  for ANSI text.  Our job here is only to extract Ethiopic text
       *  and return it as ANSI that LibEth and the calling gezOut routine
       *  can deal with.  The unkown escape is returned in the "appendRTF".
       */

      /*  Lets do this once instead of updating size each cycle  */
	  fprintf ( stderr, "  Before Malloc\n" );
      outString = (char *) malloc ( (End - Begin + 1) * sizeof(char) );
	  outString[0] = '\0';
	  fprintf ( stderr, "  After Malloc\n" );

/*
 *   This next block of code is required to detect the secondary meta characters
 *   { and } and make sure they are treated as roman.
 */
	  fprintf ( stderr, "  Before Brace Check\n" );

      while ( !done && Top < End )
        {
          lBrace = (char *) strstr (Top, "{");   
          rBrace = (char *) strstr (Top, "}");

          if ( !( (lBrace && !(lCheck = (char *) strstr (Top, "\\{")))
               || (rBrace && !(rCheck = (char *) strstr (Top, "\\}")))
                )
             )
            { 
              done = true; 
	          fprintf ( stderr, "  No Braces Found!\n" );
            } 
          else if ( (lBrace != NULL && (lBrace < lCheck))
                    || (rBrace != NULL && (rBrace < rCheck))
                    || (lCheck == NULL && lBrace != NULL) 
                    || (rCheck == NULL && rBrace != NULL) 
                  )
            {
            /*  We found a valid brace!!!
             */
	          fprintf ( stderr, "  Found A Brace!!\n[  %s\n|  %s\n|  %s\n|  %s\n]\n", lCheck, lBrace, rCheck, rBrace );
              if ( lBrace != NULL && lBrace < rBrace )
                End = lBrace;
              else
                End = rBrace;

              EndBrace = End[0];
              End[0] = '\0';
              done = true;
            }
          else  /* keep checking to end of string */
            {
	          fprintf ( stderr, "  Resetting Top!\n" );
              if (Top < rBrace)
                Top = rBrace+1;
              if (Top < rCheck)
                Top = rCheck+2;
              if (Top < lBrace)
                Top = lBrace+1;
              if (Top < lCheck)
                Top = lCheck+2;

            }
          fprintf ( stderr, "  End While \n[  %s\n|  %s\n|  %s\n|  %s\n|  %s]\n", lCheck, lBrace, rCheck, rBrace, Top );
        }

	  fprintf ( stderr, "  After Brace Check:- Got = %c\n", EndBrace );
      Top = Begin;
      while ( Top < End )
        {
	      fprintf ( stderr, "  Entering Next While!! [%s]\n", Top );
          if ( (Top = (char *) strstr(Top, "\\")) != NULL ) 
            {
              if ( Top[1] == 'f' && isdigit(Top[2]) )
                {
              /*  Detectected font change, return text upto the end of Ethiopic */
                  markFont = Top;
                  isGez = reset_fontId ( *changeList, &Top, isGez );

                  *appendRTF = (char *) malloc ( (Top - markFont + 1) * sizeof(char) );

                  if (markFont != Begin)
                    strncpy (outString, Begin, markFont-Begin-1);  /* Don't copy \ at markFont[0] */
                  strncpy (*appendRTF, Begin, Top-markFont);

                  outString[markFont-Begin-1] = '\0';
                  (*appendRTF)[Top-markFont]  = '\0';

                  Begin = Top;
                  Top   = End;
                }
              else
                {
              /*  Look up escape in our RTF->ANSI library  */

                  i = strlen (outString) + Top - Begin;
                  strcat ( outString, Begin );
				  outString[i] = '\0';

                  i = 0;
                  holdEscape[i] = Top[i++];  /*   Top[i] = \   */
                  if ( Top[i] != '\'' && ispunct(Top[i]) )
                    holdEscape[i] = Top[i++];
                  else
                    do 
                      {
                        holdEscape[i] = Top[i++];
                      }
                    while ( isalnum(Top[i]) );  /*  Read to end of escape  */

                  /*
                   *  Minor hack to handle DOS/UNIX/RTF endline chars
                   */
                  if ( holdEscape[1] == '\r' && Top[i] == '\n' )
                    {
                      holdEscape[1] = Top[i];
                      holdEscape[i++] = '\0';
                    }
                  else
                    holdEscape[i] = '\0';

                  Top += i;
                  Begin = Top;
                  if ( rtf2ansi (holdEscape) )
                    {  
                  /*  text was converted to ANSI, continue onward  */
              fprintf ( stderr, "  rtf2ansi True!  Escape convertered!\n" );
                      strcat (outString, holdEscape);
                    }  
                  else
                    {  
              fprintf ( stderr, "  rtf2ansi False!  Doing Append Line\n" );
                      *appendRTF = (char *) malloc ( i * sizeof(char) );
                      strcpy (*appendRTF, holdEscape);
                      Top = End;
                    }  

                  holdEscape[0] = '\0';
                }
            }
          else if (Top == NULL)    /* no new \ found, copy text and go home */
            {
              fprintf ( stderr, "  Doing Rest Of Line Top == NULL\n" );
              Top = End;
              strcat ( outString, Begin );
              Begin = End;
            }

        }  /* end while */

/*   The next event is true only if we found a brace and did NOT hit an
 *   unknown (and process terminating) escape.  If true, we copy the
 *   brace into the appendRTF buffer, and set the next cycle to start after
 *   the brace.
 */

        if ( EndBrace && Begin == End )
          {
            fprintf ( stderr, "  Append End!!\n" );
            if ( *appendRTF[0] == '\0' )            /* this is setNull */
              {
                *appendRTF = (char *) malloc ( (2) * sizeof(char) );
                sprintf (*appendRTF, "%c", EndBrace );
              }
            else
              {
                markString = *appendRTF;
                *appendRTF = (char *) malloc ( (strlen(*appendRTF)+2) * sizeof(char) );
                sprintf (*appendRTF, "%s%c", markString, EndBrace );
                free ( (char *)markString );
              }
            Begin = End + 1;

 /*
  *     This Begin = End+1 could be dangerous if "End" is really the End of the
  *     string and not the middle where a { or } was found (a brace could really
  *     be at the end of a string).  So, another variable (MidEnd?  RealEnd?)
  *     should probably be used as a test before the "Begin = End + 1" assignment 
  *     is made.
  */

          }
        else if ( EndBrace && Begin != End )  /* repair */
          {
            End[0] = EndBrace;
          }



    } /* end if ( isGez == Froman ) */


  *rtfText = Begin;       /* reset incoming string pointer */
  *inGez = isGez;
  fprintf ( stderr, "Leave parse_line [%i]!!\n", (*inGez)->setId );
  return ( outString );

}


 /*****************************************************************************/
/*****************************************************************************/
/*  replace_rtfFlags ( rtfText, prefix, suffix, Fout, fi )
/*
/*  The job of 
/*  
/*  
/*
/*  rtfText - 
/*  prefix  - 
/*  suffix  - 
/*  Fout    - 
/*  fi      - 
/*
/******************************************************************************/
/*****************************************************************************/

void 
replace_rtfFlags ( rtfText, prefix, suffix, Fout, inGez, fi )
  unsigned char** rtfText; 
  char* prefix; 
  char* suffix;
  enum Fonts Fout; 
  struct FontInfo* inGez;
  struct FontInfoList* fi;
{
unsigned char *Begin, *Top;
char *outString = NULL, *tempString = NULL;
int top, beginLength, outLength;
enum Fonts nextFout;
boolean same;
/*
 *  this needs a major rewrite to reset isGez to a "top" value to keep
 *  track of who is current, maybe using fi->top.
 */

  outString = (char *)malloc ( (strlen(*rtfText) + 1) * sizeof (char) );

  /* this is simple at the moment because I *know* we are dealing only with
   * Ethiopian ASCII as a multi font system
   */
  nextFout = fi->font[Fout]->nextSet->setId; 
  if (inGez->rtfId == NULL)
    same = false;
  else
    same = ( !strcmp(fi->font[nextFout]->rtfId, inGez->rtfId) ) ? true : false;

  Begin = Top = *rtfText;
  top = 0;
  while ( ( Top = (unsigned char *)strstr(Begin, "{\\fetas2 ") ) != NULL )
    {
      strncpy ( &outString[top], Begin, Top-Begin );
      top += Top-Begin;
      if ( !same )
        {
          sprintf ( &outString[top], "%s ", fi->font[nextFout]->rtfId );
          top += strlen (fi->font[nextFout]->rtfId) + 1;
        }
      Top += 9;
      while (Top[0] != '}')
        {
          outString[top++] = Top[0];
          Top++;
        }
      Top++;
      Begin = Top;
    }

  outString[top] = '\0';
  tempString = outString;
  beginLength = (Begin != NULL) ? strlen(Begin) : 0 ;
  outLength   = (outString != NULL) ? strlen(outString) : 0 ;
  outString = (char *)malloc ( ( beginLength + outLength + 1) * sizeof (char) );
  strcpy (outString, tempString);
  strcat (outString, Begin);

  free ( (char *)tempString );
  free ( (unsigned char *)*rtfText );

  *rtfText = outString;
  return;

}


 /*****************************************************************************/
/*****************************************************************************/
/*  reset_fontId ( changeList, rtfLine, isGez )
/*
/*  The job of reset_fontId is to compare an externally detected font change 
/*  against the list of Ethiopic fonts we are updating.  In cases where we
/*  convert between multiple and single font systems the font number may need
/*  to be updated for the outputted system.
/*  
/*  changeList  - list of input fonts that need to be replaced
/*  escape      - an RTF escape that will be, or not be, converted to ANSI text
/*  isGez       - a boolean identifying the script type
/*
/******************************************************************************/
/*****************************************************************************/

struct FontInfo*
reset_fontId ( changeList, rtfLine, isGez )
  struct FontInfo* changeList[];
  char** rtfLine;
  struct FontInfo* isGez;
{

register int i, j;
enum Fonts inFont;
char* endOfEscape, *test;


  fprintf ( stderr, "In reset_fontId [%i]!!\n", isGez->setId );

  i = 0;
  while ( !isspace( (*rtfLine)[++i] ) );  /*  && (*rtfLine)[i] != '\\' ); */

  fprintf ( stderr, "  After while %i\n", i );
  endOfEscape = &(*rtfLine)[i];

  fprintf ( stderr, "  Before for\nRTFLINE = %s\nEOE =    %s\n", *rtfLine, endOfEscape );
  fprintf ( stderr, "  %i => %s\n", changeList[0]->numSets, changeList[0]->rtfId );

  inFont = isGez->setId;
  for ( i = 0 ; changeList[i] != NULL ; i++ )
    if ( (test = (char *) strstr( *rtfLine, changeList[i]->rtfId) ) != NULL
            && test < endOfEscape )
      { isGez = changeList[i]; break; }

  fprintf ( stderr, "  After for\n" );
  /*  below added on 23/2/97, could wreck havoc.
   *  if we find no known font, assume a roman font was found
   */
/*if ( changeList[i] == NULL )
    isGez =& Roman;
*/
  *rtfLine = endOfEscape;

  /*  For now only simple systems are supported so the incoming rtfLine 
   *  does not need adjustment.  This will change shortly...
   */
   fprintf ( stderr, "Leave reset_fontId [%i] !!\n\n", isGez->setId );

   return (isGez);

}



 /*****************************************************************************/
/*****************************************************************************/
/*  rtf2ansi ( escape )
/*
/*  The job of rtf2ansi is to "lookup" RTF escapes and if found return them
/*  as ANSI text that LibEth will use for Ethiopic RTF conversions.
/*
/*  escape  - an RTF escape that will be, or not be, converted to ANSI text
/*
/******************************************************************************/
/*****************************************************************************/

boolean
rtf2ansi ( escape )
  char* escape;
{
char hexNum[3];
unsigned register char i;


  fprintf ( stderr, "  In rtf2ansi !!\n" );

  if (escape[1] == '\'')      /* two place hex number */
    {
      hexNum[0] = escape[2]; 
      hexNum[1] = escape[3]; 
      hexNum[2] = '\0';
      sscanf (hexNum, "%x", (unsigned int*) escape);
	  return (true);
    }
  else   /* check library */
    {
      if (escape[1] == '\\'    || escape[1] == '{'  || escape[1] == '}' 
          || escape[1] == '~'  || escape[1] == '\n' || escape[1] == '\r')
        {
          escape[0] = escape[1];
          escape[1] = '\0';
          return (true);
        }
      else /* get string, do string compares */
        {

          fprintf ( stderr, "  Looking Up [%s : ", &escape[1] );
          i = 0;
          while ( strcmp (rtfansi[i], &escape[1]) && ++i < RTFANSINAMES );
          fprintf ( stderr, "%i : ", i );

          if ( i < RTFANSINAMES )
            {
              i += ' ';
              if ( i > 'A' )    /* adjust [A-Z] range */
                i += 26;
              if ( i > 'a' )    /* adjust [a-z] range */
                i += 26;
          fprintf ( stderr, "%i : ", i );
              escape[0] = i;
              escape[1] = '\0';
          fprintf ( stderr, "%c ]\n", escape[0] );
              return (true);
            }

          return (false);   /* don't know it... */
        }
    }

  fprintf ( stderr, "  Leave rtf2ansi !!\n\n" );

    return (false);

}


 /*****************************************************************************/
/*****************************************************************************/
/*  rtf2ansi ( escape )
/*
/*  The job of rtf2ansi is to "lookup" RTF escapes and if found return them
/*  as ANSI text that LibEth will use for Ethiopic RTF conversions.
/*
/*  escape  - an RTF escape that will be, or not be, converted to ANSI text
/*
/******************************************************************************/
/*****************************************************************************/

void
strip_rtf ( rtfText )
  char** rtfText;
{

register int i;
int outTop=0;
char *outString, *Top, *Begin, *End;
char *lBrace=NULL, *rBrace=NULL, *lCheck=NULL, *rCheck=NULL, EndBrace='\0';
boolean done=false;


  Begin = Top = *rtfText;
  End = Begin + strlen(*rtfText);
  outString = (char *) malloc ( (strlen(*rtfText) + 1) * sizeof(char) );

  /*  this next block hopefully removes RTF { and } and none others!
   *  this is the same block of code from parse_line, the two should
   *  be updated simultaneously.
   */ 
  while ( !done && Top < End )
    {
      lBrace = (char *) strstr (Top, "{");   
      rBrace = (char *) strstr (Top, "}");

      if ( !( (lBrace && !(lCheck = (char *) strstr (Top, "\\{")))
           || (rBrace && !(rCheck = (char *) strstr (Top, "\\}")))
            )
         )
        { 
          done = true; 
          fprintf ( stderr, "  No Braces Found!\n" );
        } 
      else if ( (lBrace != NULL && (lBrace < lCheck))
                || (rBrace != NULL && (rBrace < rCheck))
                || (lCheck == NULL && lBrace != NULL) 
                || (rCheck == NULL && rBrace != NULL) 
              )
        {
        /*  We found a valid brace!!!,  so we delete it...
         */

          if ( lBrace != NULL && lBrace < rBrace )
            strcpy ( lBrace, lBrace+1 ) ;
          else
            strcpy ( rBrace, rBrace+1 ) ;

          done = true;
        }
      else  /* keep checking to end of string */
        {
          fprintf ( stderr, "  Resetting Top!\n" );
          if (Top < rBrace)
            Top = rBrace+1;
          if (Top < rCheck)
            Top = rCheck+2;
          if (Top < lBrace)
            Top = lBrace+1;
          if (Top < lCheck)
            Top = lCheck+2;
        }
    }

  Top = Begin;
  while ( Begin < End )
    {
      if ( (Top = (char *) strstr(Begin, "\\")) != NULL ) /*  &&  Top[1] != '\\' ) */
        {
          strncpy ( &outString[outTop], Begin, Top-Begin );
          outTop += Top-Begin;
          i = 0;
          while ( !isspace(Top[i]) )   /*  Read to end of escapes  */
            i++;
          Top += i;
        }
/*    else if ( Top[1] == '\\' ) */
      else
        {
          strncpy ( &outString[outTop], Begin, End-Begin );
          Top = End;
          outTop += Top-Begin;
        }

      Begin = Top;

    }

  outString[outTop] = '\0';
  free ((char *) *rtfText);
  *rtfText = (char *) malloc ( (strlen(outString) +1) * sizeof(char) );
  strcpy (*rtfText, outString); 
  free ((char *) outString);

}

 /*****************************************************************************/
/*****************************************************************************/
/*  get_fontNumber ( headerBottom )
/*
/*  The job of get_fontNumber is to find and return the font number at the
/*  end of a passed RTF string.
/*
/*  At the moment get_fontNumber is only called by "Rename" which operates
/*  on RTF headers.  The variable names and logic used may not be appropriate
/*  for other routines (read "maybe we revise later...").
/*
/*  headerBottom  - a portion of an RTF header.
/*
/******************************************************************************/
/*****************************************************************************/

char*
get_fontNumber (headerBottom)
  char* headerBottom;
{

char* charNumber, *mark=NULL, *lastMark=NULL;
int i;



  charNumber = (char *) malloc ( 4 * sizeof(char) );
  mark = headerBottom;
  while ( (mark = (char *) strstr (mark, "{\\f")) )
    {
      lastMark = mark;
      mark++;
    }
  mark = lastMark + 3;

  i = 0;
  while ( isdigit(mark[i]) )
    charNumber[i] = mark[i++];
  charNumber[i] = '\0';

  return (charNumber);

}

void
get_rtfIds ( fi, header )
  struct FontInfoList* fi;
  char* header;
{

char *mark=NULL,  *markTop=NULL, *fontNum, *nameSection;
int i = 0, j = 0, k = 0, lastFont, increase, insert, numFonts, setCount = 0, prefixLength;
enum Fonts top=NIL, firstFout=NUMFONTS;



  for ( i = 0 ; i < NUMFONTS ; i++ )
      if ( (mark = (char *) strstr(header, fi->font[i]->setName) ) != NULL 
	     )//  && ( fi->font[i]->setId < firstFout
	      //     || fi->font[firstFout+fi->font[firstFout]->numSets-1]->setId < fi->font[i]->setId ) )
        {
          top = i;
          markTop = mark;
          while (mark[0] != '{')
            mark--;
          nameSection = (char *) malloc ( (markTop-mark+1) * sizeof(char) );
          strncpy ( nameSection, mark, markTop-mark );
          nameSection[markTop-mark] = '\0';
          fontNum = get_fontNumber(nameSection);
          strcat ( fi->font[top]->rtfId, fontNum );
		  free ( (char *)fontNum );
		  free ( (char *)nameSection );

        }  /* end if */

}
