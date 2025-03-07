
#include "libeth.h"
#include "gezxfer.h"
#include<stdio.h>

extern char* parse_line ( struct FontInfo* (*changeList)[], char** rtfText, enum Fonts* inGez, char** appendRTF );

#define MYDEFMAJOR  tir 

void 
main (argc,argv)
  int argc;
  char **argv;
{
char hexNum[3];
unsigned char escape[20];

char *Begin=NULL, *End=NULL, *lineSegment=NULL, *appendRTF=NULL, *section=NULL,
     *header=NULL, *markBegin=NULL;
unsigned char* rtfOut=NULL;
FCHAR* uniString=NULL;
enum Fonts newGez; 
char *outString = (char *) malloc (1 * sizeof (char));
char *lastString = outString;
char *tempString=NULL;
SERAFlags* seraFlags;

char rtfText[] = "u    u&    \\'be@    \\'be    \\'be\\'85    U    \\'d2\\";


  seraFlags  =  set_defaultSERAFlags (MYDEFMAJOR, DEFMINOR, DEFMAJOR);

  lastString[0] = '\0';

  newGez = Froman;
  Begin = (char *) malloc ( (strlen(rtfText) + 1) * sizeof(char) );
  markBegin = Begin;
  strcpy ( Begin, rtfText );
  End   = Begin + strlen(Begin) - 1;

  lineSegment = parse_line ( NULL, &Begin, &newGez, &appendRTF );

  fprintf (stderr, "  lineSegment = [%s]\n", lineSegment );

  rtfOut = (char *) malloc (( 2 ) * sizeof(char));
  rtfOut[0] = '>';
  rtfOut[0] = '\0';
  appendRTF = rtfOut;


  fprintf (stderr, "  Before sget_fstring\n" );
  sget_fstring ( &uniString, lineSegment, seraFlags ); 
  fprintf (stderr, "  After sget_fstring\n" );
  fprintf (stderr, "  Before fidel_sputs\n" );
  fidel_sputs ( uniString, &rtfOut, seraFlags );
  fprintf (stderr, "  After fidel_sputs\n" );


  fprintf (stderr, "  Before Malloc\n" );
  outString = (char *) malloc ( 
               ( strlen(lastString)   /* last size           */
               + strlen(rtfOut)       /* ethiopic increase   */
               + strlen(appendRTF)    /* unknown RTF escapes */
               + 1 )                  /* to store a \0       */
               * sizeof(char) );
  fprintf (stderr, "  After Malloc\n" );


  fprintf (stderr, "  Before strcpy\n" );
  strcpy ( outString, lastString );   /* copy old   */
  fprintf (stderr, "    After strcpy 1\n" );
  strcat ( outString, rtfOut     );   /* cat new   */
  fprintf (stderr, "    After strcat 2\n" );
  strcat ( outString, appendRTF );    /* cat extra */
  fprintf (stderr, "  After strcpy\n" );


  free ( (FCHAR *)uniString );
  free ( (char *)rtfOut     );
  free ( (char *)appendRTF  );


  fprintf (stderr, "  Before Free\n" );
  free ( (char *)lastString );
  free ( (char *)lineSegment );
  fprintf (stderr, "  After Free\n" );

  fprintf (stdout, "\n\n%s\n", outString );
  free (outString);
  exit (0);

}


