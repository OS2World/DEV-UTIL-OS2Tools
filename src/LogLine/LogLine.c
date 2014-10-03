/***********************************************************************
 * Name      : Module LogLine
 * Funktion  : Echoing parameters and doing nothing.
 * Bemerkung : 
 *
 * Autor     : Patrick Haller [Samstag, 28.10.1995 14.54.33]
 ***********************************************************************/


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

int  main ( int    argc,
            char  *argv[] );


/***********************************************************************
 * Name      : int main
 * Funktion  : Hauptroutine
 * Parameter : int argc, char *argv[]
 * Variablen :
 * Ergebnis  : RÅckgabewert ans Betriebssystem
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 28.10.1995 14.55.13]
 ***********************************************************************/

int main (int argc, char *argv[])
{
  char   *pszLogfile;                          /* points to the logfile name */
  char   szTimestamp[26];           /* local buffer for the timestamp string */
  int    iArgument;                                         /* loop iterator */
  FILE   *fileLogfile;                                     /* logfile stream */
  time_t timeCurrent;                             /* the current system time */
  struct tm *ptmCurrent;             /* the current system time as structure */

  /* log one line with timestamp to a given logfile */
  if (argc < 2)
  {
    fprintf (stderr,
             "\nLogLine 1.00.00 (c) Patrick Haller Systemtechnik 1997"
             "\nUsage: LogLine <Logfile> <string>");
    return (1);                                             /* abort process */
  }

  pszLogfile = argv[1];                                  /* get the filename */

  fileLogfile = fopen(pszLogfile,"a+");                  /* open the logfile */
  if (fileLogfile == NULL)                               /* check for errors */
    fileLogfile = stdout;                      /* so we dump to standard-out */


  timeCurrent = time(NULL);                   /* get the current system time */
  ptmCurrent = localtime(&timeCurrent);                  /* convert the time */
  
  sprintf (szTimestamp,
           "%04u/%02u/%02u %02u:%02u.%02u ",
           ptmCurrent->tm_year + 1900,
           ptmCurrent->tm_mon + 1,
           ptmCurrent->tm_mday,
           ptmCurrent->tm_hour,
           ptmCurrent->tm_min,
           ptmCurrent->tm_sec);

  fputs (szTimestamp,                  /* print the timestamp to the logfile */
         fileLogfile);

  for (iArgument = 2;              /* now list all arguments in output line */
       iArgument < argc;
       iArgument++)
  {
    fputc(' ',             fileLogfile);             
    fputs(argv[iArgument], fileLogfile);
  }
 
  fputs ("\n",             fileLogfile);        /* add a CRLF to the logfile */

  if (fileLogfile != stdout)       /* we're not supposed to close stdout ... */
    fclose(fileLogfile);                                /* close the logfile */

  return 0;                           /* return code to the operating system */
} /* int main */
