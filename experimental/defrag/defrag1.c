/*****************************************************************************
 * Name      : File DeFrag.CPP
 * Funktion  : Defragment Files
 * Bemerkung : (c) Copyright 1994,97 Patrick Haller Systemtechnik
 *
 * Autor     : Patrick Haller [Mittwoch, 11.05.1994 14.03.43]
 *****************************************************************************/

#define DEBUG

/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#ifdef __OS2__
  #define INCL_DOS
  #define INCL_DOSERRORS
  #define INCL_DOSFILEMGR
  #define INCL_BASE
  #define INCL_NOPMAPI
  #include <os2.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "phsarg.h"


#define MAXPATHLEN 260


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsPath;                     /* user specified directory path       */
  ARGFLAG fsScanSingle; /* user needs compatibility to some Win 95 requester */
  ARGFLAG fsVerbose;                            /* user wants verbose output */
  ARGFLAG fsRecursive;                     /* recurse through subdirectories */
  ARGFLAG fsThreshold;                       /* user specified the threshold */
  
#ifdef DEBUG
  ARGFLAG fsDebug;                                 /* turn debug messages on */
#endif
  
  PSZ  pszPath;                                            /* directory path */
  ULONG ulThreshold;              /* threshold for spave to reserve for file */
} OPTIONS, *POPTIONS;


typedef struct
{
  ULONG ulFindNumberMaximum;         /* number of files to scan per API call */
  ULONG ulFileFindBufferSize;              /* buffer size of the scan buffer */
  ULONG ulFileMask;                   /* mask of file attributes to scan for */
  
  CHAR  szRootPath[MAXPATHLEN];           /* buffer for the root path string */

  ULONG ulFilesScanned;                           /* number of files scanned */
  double dBytesIO;                                              /* I/O bytes */
} GLOBALS, *PGLOBALS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */
GLOBALS Globals;                /* this structure holds global variables     */


ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung--------------------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/CRAP",      "Compatibility with some "
                 "Windows 95 requester.",        NULL,                 ARG_NULL,       &Options.fsScanSingle},
  {"/THRESHOLD=","Reserve more space if file needs to grow. (Bytes)",
                                                 &Options.ulThreshold, ARG_ULONG,      &Options.fsThreshold},
  {"/S",         "Recurse through directories.", NULL,                 ARG_NULL,       &Options.fsRecursive},
  {"/V",         "Verbose output.",              NULL,                 ARG_NULL,       &Options.fsVerbose},
#ifdef DEBUG
  {"/DEBUG",     "Show debug messages.",         NULL,                 ARG_NULL,       &Options.fsDebug},
#endif
  {"/?",         "Get help screen.",             NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",             NULL,                 ARG_NULL,       &Options.fsHelp},
  {"1",          "Path to defragment.",          &Options.pszPath,     ARG_PSZ |
                                                                       ARG_MUST|
                                                                       ARG_DEFAULT,    &Options.fsPath},
  ARG_TERMINATE
};


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void   help                (void);
void   initialize          (void);
APIRET DeleteFile          (PSZ          pszPath,
                            FILEFINDBUF3 *pFileInfo);
APIRET ProcessRootPath     (PSZ          pszPath);
APIRET ProcessScan         (PSZ          pszPath);


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
  TOOLVERSION("Defrag",                                 /* application name */
              0x00010000,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


/*****************************************************************************
 * Name      : APIRET DefragFile
 * Funktion  : Defrag one file
 * Parameter : PSZ path      - the current path
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:48:00]
 *****************************************************************************/

APIRET DefragFile (PSZ          pszPath,
                   FILEFINDBUF3 *pFileInfo)
{
  APIRET     rc = NO_ERROR;                                /* API-Returncode */
  FILESTATUS fStat;                   /* filestatus structure for attributes */
  CHAR       szFileNameBuffer[MAXPATHLEN];         /* buffer for DosEditName */
  CHAR       szFileDate[32];                     /* buffer for the file date */
  CHAR       szFileAttr[6];                /* buffer for the file attributes */
  PSZ        pszFileTemp;                   /* temporary pointer to filename */
  USHORT     usFileNumber;                     /* for the temporary filename */
  HFILE      hfFileInput    = 0L;       /* Handle for file being manipulated */
  HFILE      hfFileOutput   = 0L;       /* Handle for file being manipulated */
  ULONG      ulAction       = 0;                  /* Action taken by DosOpen */

  StrFAttrToStringShort (pFileInfo->attrFile,          /* map the attributes */
                         szFileAttr);

  StrFDateTimeToString (pFileInfo->fdateLastWrite,     /* map the date       */
                        pFileInfo->ftimeLastWrite,
                        szFileDate);

  if (Options.fsVerbose)
    printf ("\n  %9u %s %s %s",
            pFileInfo->cbFile,
            szFileAttr,
            szFileDate,
            pFileInfo->achName);
  
#if 0
                                                     /* open the source file */
  rc = DosOpen(pszPath,                                    /* File path name */
               &hfFileSource,                                 /* File handle */
               &ulAction,                                    /* Action taken */
               0,                                      /* primary allocation */
               FILE_NORMAL,                                /* File attribute */
               OPEN_ACTION_FAIL_IF_NEW |
               OPEN_ACTION_OPEN_IF_EXISTS,             /* Open function type */
               OPEN_FLAGS_FAIL_ON_ERROR |
               OPEN_FLAGS_NOINHERIT  |
               OPEN_SHARE_DENYWRITE  |
               OPEN_FLAGS_SEQUENTIAL |
               OPEN_ACCESS_READONLY,                /* Open mode of the file */
               0L);                                 /* No extended attribute */
  if (rc != NO_ERROR)
    return (rc);                                    /* raise error condition */

printf("\nDEBUG: DosOpenSource(%s)=%u.",pszPath,rc);
#endif
  

  strcpy (szFileNameBuffer,                      /* first, copy the filename */
          pszPath);
  pszFileTemp = strrchr(szFileNameBuffer,   /* search for the last backslash */
                        '\\');
  if (pszFileTemp == NULL)                              /* if none was found */
     pszFileTemp = szFileNameBuffer;              /* the use this as default */

  /* @@@PH EAs ! */
  usFileNumber = 0;                                          /* start with 0 */
  do
  {
    sprintf (pszFileTemp,                       /* create temporary filename */
             "\\~DFR%04x.TMP",
             usFileNumber);

    rc = DosOpen(szFileNameBuffer,                         /* File path name */
                 &hfFileOutput,                               /* File handle */
                 &ulAction,                                  /* Action taken */
                                                       /* primary allocation */
                 pFileInfo->cbFile + Options.ulThreshold,
                 FILE_NORMAL,                              /* File attribute */
                 OPEN_ACTION_CREATE_IF_NEW |
                 OPEN_ACTION_FAIL_IF_EXISTS,           /* Open function type */
                 OPEN_FLAGS_FAIL_ON_ERROR |
                 OPEN_FLAGS_NOINHERIT  |
                 OPEN_SHARE_DENYWRITE  |
                 OPEN_FLAGS_SEQUENTIAL |
                 OPEN_ACCESS_WRITEONLY,             /* Open mode of the file */
                 0L);                               /* No extended attribute */
    
#ifdef DEBUG
    if (Options.fsDebug)                                            /* DEBUG */
      printf("\nDEBUG: %s created, rc = %u",
             szFileNameBuffer,
             rc);
#endif

    usFileNumber++;         /* for the next temporary filename to be created */
  }
  while ( (ulAction == FILE_EXISTED) ||              /* check response codes */
          (rc       == ERROR_OPEN_FAILED) );
  
  DosClose(hfFileOutput);                                  /* close the file */

  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                               /* then raise error condition */

  rc = DosCopy(pszPath,           /* copy the data + EAs + attributes + info */
               szFileNameBuffer,
               DCPY_EXISTING |
               DCPY_FAILEAS);
  
#ifdef DEBUG
  if (Options.fsDebug)
    printf("\nDEBUG: %s copied to %s, rc = %u",
           pszPath,
           szFileNameBuffer,
           rc);
#endif
  
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                               /* then raise error condition */
  
  Globals.dBytesIO += pFileInfo->cbFile;                /* update statistics */
  
  rc = DosSetPathInfo (szFileNameBuffer,              /* set the information */
                       FIL_STANDARD,
                       &pFileInfo->fdateCreation,/* FILEFINDBUF3 FILESTATUS3 */
                       sizeof(FILESTATUS3),
                       0L);                        /* no write thru required */
  
#ifdef DEBUG
  if (Options.fsDebug)
    printf("\nDEBUG: setinfo on %s, rc = %u",
           szFileNameBuffer,
           rc);
#endif
  
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                                    /* raise error condition */
  
  
  rc = DosDelete(pszPath);                       /* delete the original file */
  
#ifdef DEBUG
  if (Options.fsDebug)
    printf("\nDEBUG: %s deleted, rc = %u",
           pszPath,
           rc);
#endif
  
  if (rc != NO_ERROR)                                    /* check for errors */
  {
    ULONG ulFileAttributes;                  /* save the original attributes */
    
                                           /* reset attributes and try again */
    ulFileAttributes = pFileInfo->attrFile;               /* save them first */
    pFileInfo->attrFile = FILE_NORMAL;              /* reset read-only, etc. */
    rc = DosSetPathInfo (pszPath,                     /* set the information */
                         FIL_STANDARD,
                         &pFileInfo->fdateCreation,/* FILEFINDBUF3FILESTATUS3*/
                         sizeof(FILESTATUS3),
                         0L);                      /* no write thru required */
    
#ifdef DEBUG
    if (Options.fsDebug)
      printf("\nDEBUG: reset RHS on %s, rc = %u",
             pszPath,
             rc);
#endif
    
    if (rc == NO_ERROR)                           /* this must work at first */
    {
      rc = DosDelete(pszPath);                 /* then delete the file again */
      
#ifdef DEBUG
      if (Options.fsDebug)
        printf("\nDEBUG: deleted %s, rc = %u",
               pszPath,
               rc);
#endif
      
      if (rc != NO_ERROR)     /* if delete failed, then reset the attributes */
      {
        pFileInfo->attrFile = ulFileAttributes; /* put in the old attributes */
        rc = DosSetPathInfo (pszPath,                 /* set the information */
                             FIL_STANDARD,
                             &pFileInfo->fdateCreation,
                             sizeof(FILESTATUS3),
                             0L);                  /* no write thru required */
        
#ifdef DEBUG
        if (Options.fsDebug)
          printf("\nDEBUG: reset attribs on %s, rc = %u",
                 pszPath,
                 rc);
#endif
        
      }
    }
      
    if (rc != NO_ERROR)                                  /* check for errors */
    {
      fprintf (stderr,
               "\nERROR: cannot delete original file, "
               "therefore cannot defrag. (#%u)",
               rc);
      DosDelete(szFileNameBuffer);        /* delete the temporary file again */
      return (rc);                             /* then raise error condition */
    }
  }
  
  rc = DosMove(szFileNameBuffer,                          /* rename the file */
               pszPath);
  
#ifdef DEBUG
  if (Options.fsDebug)
    printf("\nDEBUG: %s renamed to %s, rc = %u",
           szFileNameBuffer,
           pszPath,
           rc);
#endif
  
  if (rc != NO_ERROR)                                    /* check for errors */
  {
    fprintf (stderr,
             "\nERROR: cannot rename temporary file to original file."
             "\n       %s -> %s",
             szFileNameBuffer,
             pszPath);
    return (rc);                               /* then raise error condition */
  }

  /* copy the EAs */
  /* copy (open,r/w,close) */
  /* rename */
  /* set info */

  return (rc);                                                         /* ok */
}


/*****************************************************************************
 * Name      : APIRET ProcessScan
 * Funktion  : Scan one directory and perform recursion if necessary
 * Parameter : PSZ path      - the current path
 *             PANALYSE rtab - pointer to last recursions analysis data
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:48:00]
 *****************************************************************************/

APIRET ProcessScan (PSZ pszPath)
{
  APIRET          rc;                                      /* API-Returncode */
  HDIR            hDirectory = HDIR_CREATE;         /* directory scan handle */
  FILEFINDBUF3    *pFileInfo;                        /* filefind information */
  PSZ             pFileFindBuffer;                  /* buffer for the result */
  PSZ             pszPathNext;                    /* next path to be scanned */
  ULONG           ulFindCounter;           /* number of files found per call */
  ULONG           ulPathLength;                /* length of the current pszPath */


  pFileFindBuffer = (PSZ)malloc(Globals.ulFileFindBufferSize);         /* 4 x scanfiles */
  if (pFileFindBuffer == NULL)                         /* check if allocation succeeded */
    return(ERROR_NOT_ENOUGH_MEMORY);

  ulPathLength = strlen(pszPath);                     /* OK, get the origin pszPath */
  if (ulPathLength > MAXPATHLEN)
    return(ERROR_FILENAME_EXCED_RANGE);       /* Bail out, subdir is too long */

  pszPathNext = (PSZ)malloc(MAXPATHLEN);          /* for copying the pszPathname */
  if (pszPathNext == NULL)                                 /* out of memory ? */
  {
    free (pFileFindBuffer);                         /* free previously allocated memory */
    return (ERROR_NOT_ENOUGH_MEMORY);                /* raise error condition */
  }

  printf ("\n%s",
          pszPath);

  memmove ((PSZ)pszPathNext,                          /* copy the pszPath string */
           (PSZ)pszPath,
           ulPathLength);

  if (( pszPathNext[ulPathLength-1] == '\\' )|| /* ignore trailing backslash */
      ( pszPathNext[ulPathLength-1] == '/' ))   /* ignore trailing slash     */
  {
    ulPathLength--;
    pszPath[ulPathLength] = 0;                         /* cut trailing slash */
  }
                                                       /* "\*" -> 0x00002a5b */
  *( (PULONG)(pszPathNext + ulPathLength) ) = 0x00002a5c;     /* append "\*" */


  /* OK, los geht's */
  ulFindCounter    = Globals.ulFindNumberMaximum;

  pFileInfo = (FILEFINDBUF3 *)pFileFindBuffer;

  rc = DosFindFirst(pszPathNext,                          /* first scan call */
                    &hDirectory,
                    Globals.ulFileMask,
                    pFileInfo,
                    Globals.ulFileFindBufferSize,
                    &ulFindCounter,
                    FIL_STANDARD);
  if ( (rc != NO_ERROR) &&                      /* check for error condition */
       (rc != ERROR_NO_MORE_FILES) )
    return (rc);                                        /* return error code */

  do
  {
    Globals.ulFilesScanned += ulFindCounter;                   /* statistics */

    while (ulFindCounter)
    {
                                                          /* ignore . and .. */
      if ((pFileInfo->achName[0] == '.') &&
          (( !pFileInfo->achName[1] ||
            ((pFileInfo->achName[1] == '.') && !pFileInfo->achName[2])) ));
      else
      {
        strcpy (pszPathNext,                  /* build a correct path string */
                pszPath);

        ulPathLength = strlen(pszPathNext);      /* query the strings length */
                                                /* ignore trailing backslash */
        * ( (PUSHORT)(pszPathNext + ulPathLength) ) = 0x005c;           /* \ */

        strcat (pszPathNext,                              /* append new name */
                pFileInfo->achName);


        if (pFileInfo->attrFile & FILE_DIRECTORY)/* examine directories only */
        {
          if (Options.fsRecursive)
          {
            rc = ProcessScan(pszPathNext);                        /* Recurse */
            if (rc != NO_ERROR)
              ToolsErrorDos(rc);
          }
        }
        else
        {
          rc = DefragFile(pszPathNext,
                          pFileInfo);                     /* delete the file */
          if (rc != NO_ERROR)
            ToolsErrorDos(rc);
        }
      }

      ulFindCounter--;

      pFileInfo = (FILEFINDBUF3 *) ((BYTE*)pFileInfo +
                                    pFileInfo->oNextEntryOffset);
    }

    ulFindCounter = Globals.ulFindNumberMaximum;

    pFileInfo = (FILEFINDBUF3 *)pFileFindBuffer;
    rc = DosFindNext (hDirectory,
                      pFileInfo,
                      Globals.ulFileFindBufferSize,
                      &ulFindCounter);
  }
  while (rc == NO_ERROR);

  free((void *)pFileFindBuffer);
  free((void *)pszPathNext);

  return(DosFindClose(hDirectory));
}


/*****************************************************************************
 * Name      : APIRET ProcessRootPath
 * Funktion  : Gets fully qualified root path name
 * Parameter : PSZ pszPath
 * Variablen :
 * Ergebnis  : APIRET rc
 * Bemerkung : pszPath must hold enough space for result string
 *
 * Autor     : Patrick Haller [Mittwoch, 11.05.1994 14.04.33]
 *****************************************************************************/

APIRET ProcessRootPath (PSZ pszPath)
{
  APIRET rc;                                               /* API-Returncode */
  CHAR   szBuffer[MAXPATHLEN];                        /* static local buffer */

  rc = DosQueryPathInfo (pszPath,
                         FIL_QUERYFULLNAME,
                         szBuffer,
                         sizeof(szBuffer));
  if (rc != NO_ERROR)                                    /* check for errors */
    ToolsErrorDos(rc);

  strcpy (pszPath,               /* return the result in the input parameter */
          szBuffer);

  return (NO_ERROR);                                       /* signal success */
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

  memset(&Globals,
         0L,
         sizeof(Globals));
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
  int iAnswer;   /* answer from the user on the security confirmation prompt */

  PERFSTRUCT psStart;              /* structure for the performance counters */
  PERFSTRUCT psEnd;
  float      fSeconds;                      /* total duration of the process */

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

  if ( Options.fsHelp )                      /* check if user specified file */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }


                                                     /* build up a root path */
  if (Options.fsPath)
    strcpy (Globals.szRootPath,               /* copy the user supplied path */
            Options.pszPath);
  else
  {
    fprintf (stderr,
             "\nError: you must supply a directory path.");
    return (ERROR_PATH_NOT_FOUND);                          /* abort program */
  }

  if (!Options.fsScanSingle)                   /* determine find scan number */
  {
    Globals.ulFindNumberMaximum = 65535 / sizeof(FILEFINDBUF3);
    Globals.ulFileFindBufferSize = 65535;           /* 64k, best performance */
  }
  else
  {
    Globals.ulFindNumberMaximum = 1;            /* for windows 95 ... uarg ! */
    Globals.ulFileFindBufferSize = 1024;                   /* 1k, sufficient */
  }

  Globals.ulFileMask = FILE_NORMAL   |
                       FILE_SYSTEM   |
                       FILE_READONLY |
                       FILE_HIDDEN   |
                       FILE_DIRECTORY;

  rc = ProcessRootPath(Globals.szRootPath);       /* get qualified root path */
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);
              /* abort processing here, don't accidentually go to wrong path */
    return (rc);
  }

  printf ("\nDefragmenting [%s]",
          Options.pszPath);


  ToolsPerfQuery(&psStart);                 /* start the performance counter */

  rc=ProcessScan (Globals.szRootPath);                          /* do it ... */
  ToolsPerfQuery(&psEnd);                   /* stop  the performance counter */

  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

                                                 /* now print the statistics */
  fSeconds = psEnd.fSeconds - psStart.fSeconds;        /* calculate duration */
  
  if (Options.fsVerbose)
    printf ("\n%u files copied in %10.3f seconds, %10.3f files/sec"
            "\n%12.0f bytes copied, %10.3f kb/s average",
            Globals.ulFilesScanned,
            fSeconds,
            (double)Globals.ulFilesScanned / fSeconds,
            Globals.dBytesIO,
            Globals.dBytesIO / fSeconds / 1024.0);
  
  return (rc);
}
