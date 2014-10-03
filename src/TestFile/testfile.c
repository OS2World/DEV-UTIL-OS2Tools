/*****************************************************
 * Testfile Tool.                                    *
 * Just creates a number of files.                   *
 * (c) 1997 Patrick Haller Systemtechnik             *
 *****************************************************/

/* #define DEBUG 1 */

/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#define INCL_DOSFILEMGR
#define INCL_BASE
#define INCL_NOPMAPI
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"

#define MAXPATHLEN 260


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsFileNumber;                                   /* number of files */
  ARGFLAG fsFileSize;                               /* dump starts from tail */
  ARGFLAG fsVerbose;                                  /* produce more output */
  ARGFLAG fsNoErrors;  /* indicates most error messages are to be suppressed */
  ARGFLAG fsOverwrite;              /* overwriting existing files is allowed */
  ARGFLAG fsWrite;               /* actually fill the file with data bytes ? */
  ARGFLAG fsWriteSize;        /* indicates if user specified file write size */
  ARGFLAG fsWriteCache;/* indicates if user wants us to enable write caching */

  ULONG ulFileNumber;                       /* number of files to be created */
  ULONG ulFileSize;                                     /* size of the files */
  ULONG ulWriteSize;                  /* size of chunks to write to the file */
} OPTIONS, *POPTIONS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */

ARGUMENT TabArguments[] =
{ /*Token---------Beschreibung----------------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/SIZE=",      "Size of the test files.",  &Options.ulFileSize,  ARG_ULONG,      &Options.fsFileSize},
  {"/NUMBER=",    "Number of test files.",    &Options.ulFileNumber,ARG_ULONG,      &Options.fsFileNumber},
  {"/VERBOSE",    "Verbose output.",          NULL,                 ARG_NULL,       &Options.fsVerbose},
  {"/NOERROR",    "Suppress some messages.",  NULL,                 ARG_NULL,       &Options.fsNoErrors},
  {"/OVERWRITE",  "Overwrite existing files.",NULL,                 ARG_NULL,       &Options.fsOverwrite},
  {"/WRITE.SIZE=","Bytes per write op..",     &Options.ulWriteSize, ARG_ULONG,      &Options.fsWriteSize},
  {"/WRITE.CACHE","Enable write caching.",    NULL,                 ARG_NULL,       &Options.fsWriteCache},
  {"/WRITE",      "Actually write to files.", NULL,                 ARG_NULL,       &Options.fsWrite},
  {"/?",          "Get help screen.",         NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",          "Get help screen.",         NULL,                 ARG_NULL,       &Options.fsHelp},
  ARG_TERMINATE
};


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void   help                (void);
void   initialize          (void);


/***********************************************************************
 * Name      : void help
 * Funktion  : Anzeigen des Hilfetextes.
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.45.24]
 ***********************************************************************/

void help (void)
{
  TOOLVERSION("Testfile",                               /* application name */
              0x00010000,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


/***************************************************************************
 * Name      : void create_files
 * Funktion  : Creation of the testfiles
 * Parameter :
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.49.57]
 ***************************************************************************/

#define BUFFERSIZE 65535

APIRET CreateFiles(void)
{
  int   rc;
  HFILE hInfoFile;              /* Filehandle fÅr die zu untersuchende Datei */
  ULONG ulAction;
  ULONG ulFlags;                                   /* flags for DosOpen call */
  ULONG ulCacheFlags;                      /* caching flags for DosOpen call */

  ULONG ulCounter;                     /* loop counter for the file creation */
  ULONG ulFilesCreated = 0;                 /* counter of good files created */
  CHAR  szFilename[260];

  PERFSTRUCT TS_Total_Start;         /* this measures operation incl. output */
  PERFSTRUCT TS_Total_End;

  PERFSTRUCT TS_Start;          /* to measure the duration of creating files */
  PERFSTRUCT TS_End;
  float      flTotal = 0.0; /* total number of seconds for the whole process */
  float      flTotal2 = 0.0;/* total number of seconds for the whole process */
  float      flSeconds;                    /* duration of the last operation */
  float      flOverhead;                  /* overhead for measuring the time */

  ULONG      ulBytesTotal;               /* how many bytes to write in total */
  ULONG      ulBufferLength;                             /* length of buffer */
  ULONG      ulBytesWritten;                                /* bytes written */
  PCHAR      pBuffer;                        /* pointer to dummy buffer area */
  float      flBytesWritten = 0.0;          /* total number of written bytes */
  float      flTimeWrite = 0.0;             /* Time to write all those bytes */
  float      flWriting = 0.0;             /* time for last writing operation */

                                                         /* checking options */
  if (!Options.fsFileSize)
    Options.ulFileSize = 0;                         /* default is zero bytes */

  if (!Options.fsFileNumber)
    Options.ulFileNumber = 1;                         /* default is one file */


  if (!Options.fsWriteSize)
    Options.ulWriteSize = BUFFERSIZE;

  if (Options.fsWrite)
  {
    rc = DosAllocMem((PPVOID)&pBuffer,/* allocate buffer for dummy test data */
                     Options.ulWriteSize,
                     PAG_COMMIT |
                     PAG_WRITE  |
                     PAG_READ);
    if (rc != NO_ERROR)                                  /* check for errors */
    {
      ToolsErrorDos(rc);
      return (rc);
    }

    for (ulBufferLength = 0;                    /* commit and fill that area */
         ulBufferLength < Options.ulWriteSize;
         ulBufferLength++)
      * (pBuffer + ulBufferLength) = ulBufferLength & 0xFF;
  }
  else
    pBuffer = NULL;                           /* else no buffer is allocated */


  printf ("\nCreating %u files of %u bytes, %u bytes in total.",
          Options.ulFileNumber,
          Options.ulFileSize,
          Options.ulFileNumber * Options.ulFileSize);    /* check overflow ? */


                                               /* measure the timer overhead */
  for (ulCounter = 0,
       flOverhead = 0;

       ulCounter < 1000;

       ulCounter ++)
  {
    ToolsPerfQuery(&TS_Start);
    ToolsPerfQuery(&TS_End);
    flOverhead += (TS_End.fSeconds - TS_Start.fSeconds);
  }
  printf ("\nTimer overhead per operation is %15.6f milliseconds.",
          flOverhead);

  flOverhead /= 1000;                                /* get the exact factor */


  if (Options.fsOverwrite)
    ulFlags = OPEN_ACTION_REPLACE_IF_EXISTS |
              OPEN_ACTION_CREATE_IF_NEW;
  else
    ulFlags = OPEN_ACTION_FAIL_IF_EXISTS |
              OPEN_ACTION_CREATE_IF_NEW;

  if (Options.fsWriteCache)              /* see if we have to enable caching */
    ulCacheFlags = OPEN_FLAGS_SEQUENTIAL;
  else
    ulCacheFlags = OPEN_FLAGS_WRITE_THROUGH |
                   OPEN_FLAGS_NO_CACHE;


                                                                /* work loop */
  ToolsPerfQuery(&TS_Total_Start);              /* measure complete operation */

  for (ulCounter = 0;
       ulCounter < Options.ulFileNumber;
       ulCounter++)
  {
    sprintf (szFilename,
             "T%07u",
             ulCounter);

    if (Options.fsVerbose)     /* check if we have to produce verbose output */
      printf ("\n%s ",
              szFilename);


    ToolsPerfQuery(&TS_Start);                                  /* stop watch */

    rc = DosOpen (szFilename,
                  &hInfoFile,
                  &ulAction,
                  Options.ulFileSize,                            /* Filesize */
                  0L,                                     /* File attributes */
                  ulFlags,
                  OPEN_SHARE_DENYNONE  |
                  OPEN_ACCESS_WRITEONLY |
                  OPEN_FLAGS_FAIL_ON_ERROR |
                  ulCacheFlags,
                  NULL);

    ToolsPerfQuery(&TS_End);                                    /* stop watch */
    flSeconds = TS_End.fSeconds - TS_Start.fSeconds - flOverhead;
    if (flSeconds < 0)                            /* prevent negative values */
      flSeconds = 0;
    flTotal += flSeconds;                        /* calculate total duration */


    if (rc != NO_ERROR)    /* check for problems during creation of the file */
    {
      if (Options.fsVerbose)   /* check if we have to produce verbose output */
        printf ("SYS%04u - not created",
                rc);
      else
        if (!Options.fsNoErrors)     /* check if we have to produce messages */
          fprintf (stderr,                      /* redirect output to stderr */
                   "\n%s SYS%04u - not created",
                   szFilename,
                   rc);
    }
    else
    {
      ulFilesCreated++;                /* increase counter for created files */

      if (Options.fsVerbose)   /* check if we have to produce verbose output */
        printf ("%14.4fms OK",
                flSeconds * 1000);

                         /* does the user want us to fill the file with data */
      if (Options.fsWrite)
      {
        ulBytesTotal = Options.ulFileSize; /* total number of bytes to write */
        ulBufferLength = Options.ulWriteSize;

        ToolsPerfQuery(&TS_Start);

        for (;
             (rc == NO_ERROR) &&
             (ulBytesTotal != 0)
             ;
            )
        {
          if (ulBytesTotal < ulBufferLength)   /* limit to the max. filesize */
            ulBufferLength = ulBytesTotal;
          else
            ulBufferLength = Options.ulWriteSize;

          rc = DosWrite (hInfoFile,    /* just write that buffer to the file */
                         pBuffer,
                         ulBufferLength,
                         &ulBytesWritten);

          ulBytesTotal -= ulBytesWritten;
        }

        ToolsPerfQuery(&TS_End);

        flBytesWritten += Options.ulFileSize;    /* keep statistics on track */
        flWriting =  (TS_End.fSeconds - TS_Start.fSeconds - flOverhead);
        flTimeWrite += flWriting;

        if (rc != NO_ERROR)                              /* check for errors */
        {
          if (!Options.fsNoErrors)   /* check if we have to produce messages */
            fprintf (stderr,                    /* redirect output to stderr */
                     "\n%s SYS%04u - while writing",
                     szFilename,
                     rc);
        }
      }


      DosClose(hInfoFile);                                /* close that file */

      if (Options.fsVerbose)   /* check if we have to produce verbose output */
        if (Options.fsWrite)              /* check if file writing is active */
        {
          printf (", %14.4fms writing",
                  flWriting * 1000);

          if (flWriting != 0.0)
            printf (", %14.4fkb/s",
                    (float)Options.ulFileSize / flWriting / 1024.0);
        }
    }
  }

  ToolsPerfQuery(&TS_Total_End);                /* measure complete operation */
  flTotal2 = TS_Total_End.fSeconds - TS_Total_Start.fSeconds;

  printf ("\nTotal duration   : %14.4fs (calculated)."
          "\n                   %14.4fs (measured, inclusive output and overhead)."
          "\n                   %14.4fs (to write to the file).",
          flTotal,
          flTotal2,
          flTimeWrite);

  printf ("\nFiles per seconds: %14.4f (calculated)."
          "\n                   %14.4f (measured, inclusive output and overhead).",
          (float)ulFilesCreated / flTotal,
          (float)ulFilesCreated / flTotal2);

  if (flTimeWrite != 0.0)
    printf("\nKilobytes per sec: %14.4fkb/s (while writing to the file).",
          flBytesWritten / flTimeWrite / 1024.0);


  if (pBuffer != NULL)                         /* if we had memory allocated */
    rc = DosFreeMem(pBuffer);                            /* free that memory */
  else
    rc = NO_ERROR;

  return (rc);                                       /* RÅckgabewert liefern */
}


/***********************************************************************
 * Name      : void initialize
 * Funktion  : Initialisierung einiger Variablen
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 ***********************************************************************/

void initialize (void)
{
  memset(&Options,
         0L,
         sizeof(Options));
}


/***********************************************************************
 * Name      : int main
 * Funktion  : Hauptroutine
 * Parameter : int argc, char *argv[]
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 ***********************************************************************/

int main (int argc, char *argv[])
{
  int rc;                                                    /* RÅckgabewert */

  initialize ();                                          /* Initialisierung */

  rc = ArgStandard (argc,                            /* CLI-Parameter parsen */
                    argv,
                    TabArguments,
                    &Options.fsHelp);
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    exit(1);                                                /* abort program */
  }

  if (Options.fsHelp)                    /* check if help is to be displayed */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  rc = CreateFiles();
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  return (rc);
}
