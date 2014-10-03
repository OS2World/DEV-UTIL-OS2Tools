/*****************************************************
 * Dump Tool.                                        *
 * (Hex-) dumps a given file.                        *
 * (c) 1997 Patrick Haller Systemtechnik             *
 *****************************************************/

/* #define DEBUG 1 */

/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#ifdef __OS2__
  #define INCL_DOSFILEMGR
  #define INCL_BASE
  #define INCL_NOPMAPI
  #include <os2.h>
#endif

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <process.h>

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
  ARGFLAG fsFile;          /* indicates file was specified from command line */
  ARGFLAG fsHead;                                   /* dump starts from head */
  ARGFLAG fsTail;                                   /* dump starts from tail */
  ARGFLAG fsPosition;                   /* dump starts from special position */
  ARGFLAG fsLength;                      /* length of the dump was specified */
  ARGFLAG fsDumpASCII;                /* dump shall be made as an ascii dump */
  ARGFLAG fsDumpHEX;                  /* dump shall be made as an hex   dump */
  ARGFLAG fsDumpText;                 /* dump shall be made as an text  dump */
  ARGFLAG fsDumpBin;                  /* dump shall be made as an bin.  dump */
  ARGFLAG fsFollow;                   /* dump shall follow the input stream  */
  ARGFLAG fsFollowInterval;     /* time to wait before next read from stream */
  
  PSZ   pszFile;                         /* name of the file to be processed */
  ULONG ulPosition;              /* position from there the dump shall start */
  ULONG ulLength;                                /* length of the total dump */
  ULONG ulFollowInterval;       /* time to wait before next read from stream */
} OPTIONS, *POPTIONS;

/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */

ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung--------------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/HEAD",      "Dump from beginning.",   NULL,                 ARG_NULL,       &Options.fsHead},
  {"/TAIL",      "Dump from end.",         NULL,                 ARG_NULL,       &Options.fsTail},
  {"/POS=",      "Dump from position.",    &Options.ulPosition,  ARG_ULONG,      &Options.fsPosition},
  {"/LENGTH=",   "Length of dump.",        &Options.ulLength,    ARG_ULONG,      &Options.fsLength},
  {"/LEN=",      "Length of dump.",        &Options.ulLength,    ARG_ULONG
                                                                 | ARG_HIDDEN,   &Options.fsLength},
  {"/ASCII",     "ASCII dump.",            NULL,                 ARG_NULL,       &Options.fsDumpASCII},
  {"/HEX",       "HEX   dump.",            NULL,                 ARG_NULL,       &Options.fsDumpHEX},
  {"/TEXT",      "Text  dump.",            NULL,                 ARG_NULL,       &Options.fsDumpText},
  {"/BIN",       "Binary dump (redirect).",NULL,                 ARG_NULL,       &Options.fsDumpBin},
  {"/FOLLOW",    "Follow the input.",      NULL,                 ARG_NULL,       &Options.fsFollow},
  {"/F",         "Follow the input.",      NULL,                 ARG_NULL
                                                                 | ARG_HIDDEN,   &Options.fsFollow},
  {"/INTERVAL=", "Polling interval for follow", &Options.ulFollowInterval, ARG_ULONG, &Options.fsFollowInterval},
  {"/?",         "Get help screen.",       NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",       NULL,                 ARG_NULL,       &Options.fsHelp},
  {"1",          "Filename.",              &Options.pszFile,     ARG_PSZ     |
                                                                 ARG_DEFAULT |
                                                                 ARG_MUST,       &Options.fsFile},
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
  TOOLVERSION("Dump",                                   /* application name */
              0x00010005,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


/***************************************************************************
 * Name      : void dump_file
 * Funktion  : Anzeigen der gefundenen Detailinformation
 * Parameter :
 * Variablen :
 * Ergebnis  : Ausgabe auf dem Bildschirm
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.49.57]
 ***************************************************************************/

int dump_file (PSZ szFilename)
{
  int   rc;
  HFILE hInfoFile;              /* Filehandle fÅr die zu untersuchende Datei */
  ULONG ulAction;
  ULONG ulFlags;                                   /* flags for DosOpen call */
  FILESTATUS3 fs3;         /* Structure to hold the second level information */

  ULONG ulReadPosition;
  ULONG ulReadLength;
  ULONG ulReadLengthTotal;                          /* total number of bytes */
  ULONG ulBytesRead;    /* how many bytes have been read during last read op */
  ULONG ulReadLengthLimit;                /* how many bytes to read in total */
  PVOID pBuffer;                             /* pointer to the memory buffer */
  ULONG ulBytesWritten;                           /* bytes written to stdout */


  if (szFilename == NULL)                            /* ParameterÅberprÅfung */
    return (ERROR_INVALID_PARAMETER);

  if (!Options.fsDumpBin)                       /* don't disturb binary dump */
    printf ("\nDumping [%s]",szFilename);


  ulFlags = OPEN_SHARE_DENYNONE  |
            OPEN_ACCESS_READONLY |
            OPEN_FLAGS_NO_CACHE;

  rc = DosOpen (szFilename,
                &hInfoFile,
                &ulAction,
                0L,                                            /* Filesize */
                0L,                                     /* File attributes */
                OPEN_ACTION_FAIL_IF_NEW |
                OPEN_ACTION_OPEN_IF_EXISTS,
                ulFlags,
                NULL);

  if (rc != NO_ERROR)                                /* was there an error ? */
  {
    ToolsErrorDos(rc);
    return (rc);                               /* abort function immediately */
  }

  /* query filesize */
  rc = DosQueryFileInfo(hInfoFile,                     /* Gather information */
                        FIL_STANDARD,
                        &fs3,
                        sizeof(fs3));
  if (rc != NO_ERROR)                                /* was there an error ? */
  {
    DosClose(hInfoFile);                              /* close the open file */
    ToolsErrorDos(rc);
    return (rc);                               /* abort function immediately */
  }

  ulBytesRead       = 1;                                   /* initialization */
  ulReadLengthLimit = fs3.cbFile;             /* calculate total read length */
  
                /* don't automatically allocate a just too big stream buffer */
  if (ulReadLengthLimit > OS2READSIZE)
    ulReadLengthLimit = OS2READSIZE;
  
  if (Options.fsLength)             /* check if user requests special length */
    if (ulReadLengthLimit > Options.ulLength)
      ulReadLengthLimit = Options.ulLength;

  /* calculate dumping position */
  ulReadPosition    = 0;
  if (Options.fsPosition)
  {
    ulReadPosition = Options.ulPosition;              /* check for file size */
    if (fs3.cbFile - ulReadPosition < ulReadLengthLimit)
      ulReadLengthLimit = fs3.cbFile - ulReadPosition;
  }
  else
    if (Options.fsHead)                      /* start dumping from beginning */
      ulReadPosition = 0;
    else
      if (Options.fsTail)
        ulReadPosition = fs3.cbFile - ulReadLengthLimit;

  /* now seek to the right position */
  rc = DosSetFilePtr(hInfoFile,
                     ulReadPosition,
                     FILE_BEGIN,
                     &ulReadPosition);
  if (rc != NO_ERROR)                                /* was there an error ? */
  {
    DosClose(hInfoFile);                              /* close the open file */
    ToolsErrorDos(rc);
  }


  if (!Options.fsFollow)
    ulReadLength = ulReadLengthLimit;               /* this is a file buffer */
  else
    ulReadLength = OS2READSIZE;            /* as this is a stream buffer ... */
  
  rc = DosAllocMem (&pBuffer,             /* allocate memory for dump buffer */
                    ulReadLength,
                    PAG_COMMIT |
                    PAG_READ   |
                    PAG_WRITE);
  if (rc != NO_ERROR)                                    /* check for errors */
  {
    DosClose (hInfoFile);                             /* close the open file */
    ToolsErrorDos(rc);
    return (rc);                               /* abort function immediately */
  }
  
  
  ulReadLengthTotal = 0;

  while ( (rc == NO_ERROR) &&               /* as long as there was no error */
          (ulBytesRead != 0) )             /* and still bytes were remaining */
  {
    rc = DosRead (hInfoFile,                        /* read buffer from file */
                  pBuffer,
                  ulReadLength,                    /* how many bytes to read */
                  &ulBytesRead);
    if (rc == NO_ERROR)  /* if there was no error, then dump the information */
    {
      /**************
       * ASCII dump *
       **************/

      if (Options.fsDumpASCII & !Options.fsDumpBin)          /* ASCII-Dump ? */
        ToolsDumpAscii (ulReadPosition,                  /* dump information */
                        ulBytesRead,
                        pBuffer);

      /************
       * Hex dump *
       ************/

      if (Options.fsDumpHEX & !Options.fsDumpBin)                /* hex dump */
        ToolsDumpHex (ulReadPosition,                    /* dump information */
                      ulBytesRead,
                      pBuffer);

      /***************
       * Binary dump *
       ***************/

      if (Options.fsDumpBin)
      {
        rc = DosWrite (1,                      /* write buffer 1:1 to stdout */
                       pBuffer,
                       ulReadLength,
                       &ulBytesWritten);
        if (rc != NO_ERROR)                             /* check error codes */
        {
          ToolsErrorDosEx(rc,
                          "writing to stdout.");
          goto error_bail_out;                 /* leave function immediately */
        }

        if (ulBytesWritten != ulReadLength)       /* too few bytes written ? */
        {
          ToolsErrorDosEx(NO_ERROR,
                          "too few bytes written to stdout.");
          goto error_bail_out;                 /* leave function immediately */
        }

      }


      /*************
       * Text dump *
       *************/

      if (Options.fsDumpText & !Options.fsDumpBin)
      {
        ULONG ulTemp;
        PSZ   pszTemp = (PSZ)pBuffer;

        /* quickly replace unprintable characters */
        for (ulTemp = 0;
             ulTemp < ulBytesRead;
             ulTemp++)
          if ( (pszTemp[ulTemp] < ' ') &&
               ! ( (pszTemp[ulTemp] == 0x0a) || /* LF  */
                   (pszTemp[ulTemp] == 0x0d) || /* CR  */
                   (pszTemp[ulTemp] == 0x09)    /* Tab */
                 )
             )
            pszTemp[ulTemp] = '˙';


        DosWrite (1,                           /* standard handle for stdout */
                  pBuffer,
                  ulBytesRead,
                  &ulAction);                                       /* dummy */
      }

      ulReadLengthTotal += ulBytesRead;                   /* update counters */
      ulReadPosition += ulBytesRead;

//      if (ulReadLengthLimit - ulReadLengthTotal < OS2READSIZE)
//        ulReadLength = ulReadLengthLimit - ulReadLengthTotal;
      
      if (!Options.fsFollow)
        if (ulReadLengthLimit == ulReadLengthTotal) /* check if we are done yet */
          break;
    }
    else                              /* rc != NO_ERROR -> an error occurred */
    {
      if (!Options.fsFollow)
        ToolsErrorDos(rc);
    }
    
    // we're in "follow the input stream mode" ...
    if (NO_ERROR == rc)
      if (0 == ulBytesRead)
        if (Options.fsFollow)
        {
          // sleep a little and try again ...
          DosSleep(50);

          // care about the while loop criteria
          ulBytesRead = 1;
          rc = NO_ERROR;
        }
  }


error_bail_out:
  rc = DosFreeMem(pBuffer);                            /* free buffer memory */
  if (rc != NO_ERROR)                                    /* check for errors */
    ToolsErrorDos(rc);

  rc = DosClose(hInfoFile);                                    /* Close file */
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

  if ( (Options.pszFile == NULL) ||          /* check if user specified file */
       (Options.fsHelp) )
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  if (Options.fsHead && Options.fsTail)
  {
    fprintf(stderr,
            "\n/HEAD and /TAIL are mutually exclusive.");
    return (NO_ERROR);
  }


  if (!Options.fsDumpASCII &&                /* check if we have a dump mode */
      !Options.fsDumpHEX &&
      !Options.fsDumpText)
    Options.fsDumpHEX = TRUE;                                /* default mode */

  rc = dump_file (Options.pszFile);
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  return (rc);
}
