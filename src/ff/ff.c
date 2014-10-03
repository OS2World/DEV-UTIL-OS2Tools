/*****************************************************************************
 * Name      : File FILEFIND.CPP
 * Funktion  : Find files distributed accross the filesystem
 * Bemerkung : 
 *
 * Autor     : Patrick Haller [2001-04-27]
 *****************************************************************************/


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

#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "tools.h"
#include "tooltypes.h"
#include "toolarg.h"


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
  ARGFLAG fsStatistics;                         /* user wants statistics     */
  ARGFLAG fsDisplayNamesOnly;                     /* only display file names */
  ARGFLAG fsFileAge;                            /* deletion selective on age */
  ARGFLAG fsFileAgeWrite;                /* deletion selective on last-write */
  ARGFLAG fsFileAgeAccess;              /* deletion selective on last-access */
  ARGFLAG fsFileAgeCreate;                 /* deletion selective on creation */
  ARGFLAG fsFileAgeNewer;                           /* inverse the selection */
  ARGFLAG fsFileNameMask;       /* user wants only special files to be found */

  ARGFLAG fsSizeLarger;                    /* file must be larger  than this */
  ULONG   ulSizeLarger;                    /* file must be larger  than this */
  ARGFLAG fsSizeSmaller;                   /* file must be smaller than this */
  ULONG   ulSizeSmaller;                   /* file must be smaller than this */


  ULONG ulFileAge;                                  /* files older than this */
  PSZ  pszPath;                                            /* directory path */
  PSZ  pszFileNameMask;        /* special file name mask including wildcards */
} OPTIONS, *POPTIONS;


typedef struct
{
  ULONG ulFindNumberMaximum;         /* number of files to scan per API call */
  ULONG ulFileFindBufferSize;              /* buffer size of the scan buffer */
  ULONG ulFileMask;                   /* mask of file attributes to scan for */
  ULONG ulFileDate;                                 /* files older than this */

  CHAR  szRootPath[MAXPATHLEN];           /* buffer for the root path string */

  ULONG ulFilesScanned;                           /* number of files scanned */
  ULONG ulDirectoriesScanned;               /* number of directories scanned */

  ULONG ulFoundBytes;            /* number of bytes contained in files found */
  ULONG ulFoundAlloc;                         /* actually freed up diskspace */
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
  {"/AGE=",     "Find files older than this.",  &Options.ulFileAge,   ARG_ULONG,      &Options.fsFileAge},
  {"/AGE.WRITE","Last-write-date is relevant"
                 " (HPFS)."                     ,NULL,                 ARG_NULL,       &Options.fsFileAgeWrite},
  {"/AGE.ACCESS","Last-access-date is relevant"
                 " (HPFS)."                     ,NULL,                 ARG_NULL,       &Options.fsFileAgeAccess},
  {"/AGE.CREATE","Creation-date is relevant"
                 " (HPFS)."                     ,NULL,                 ARG_NULL,       &Options.fsFileAgeCreate},
  {"/AGE.NEWER","Find files younger than <age>"
                 " instead of older."           ,NULL,                 ARG_NULL,       &Options.fsFileAgeNewer},
  {"/SIZE.LARGER=", "Find files larger than this.",  &Options.ulSizeLarger, ARG_ULONG, &Options.fsSizeLarger},
  {"/SIZE.SMALLER=","Find files smaller than this.", &Options.ulSizeSmaller, ARG_ULONG, &Options.fsSizeSmaller},
  {"/NAMES",     "Print file(s) names only.",    NULL,                 ARG_NULL,       &Options.fsDisplayNamesOnly},
  {"/STAT",      "Print statistics.",            NULL,                 ARG_NULL,       &Options.fsStatistics},
  {"/STATISTICS","Print statistics.",            NULL,                 ARG_NULL |
                                                                       ARG_HIDDEN,     &Options.fsStatistics},
  {"/V",         "Verbose output.",              NULL,                 ARG_NULL,       &Options.fsVerbose},
  {"/?",         "Get help screen.",             NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",             NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/NAME=",     "File name mask.",              &Options.pszFileNameMask, ARG_PSZ,    &Options.fsFileNameMask},
  {"1",          "Directory, pathname.",         &Options.pszPath,     ARG_PSZ |
                                                                       ARG_MUST|
                                                                       ARG_DEFAULT,    &Options.fsPath},
  ARG_TERMINATE
};


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void   help                (void);
void   initialize          (void);
APIRET FindDir             (PSZ          pszPath);
APIRET FindFile            (PSZ          pszPath,
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
  TOOLVERSION("FileFind",                               /* application name */
              0x00010002,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


/*****************************************************************************
 * Name      : APIRET FindDir
 * Funktion  : Find in one directory
 * Parameter : PSZ path      - the current path
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:48:00]
 *****************************************************************************/

APIRET FindDir (PSZ pszPath)
{
  APIRET rc = NO_ERROR;                                    /* API-Returncode */

  Globals.ulDirectoriesScanned++;                   /* update the statistics */

  return (rc);                                                         /* ok */
}


/*****************************************************************************
 * Name      : APIRET FindFile
 * Funktion  : Find one file
 * Parameter : PSZ path      - the current path
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:48:00]
 *****************************************************************************/

APIRET FindFile (PSZ          pszPath,
                 FILEFINDBUF3 *pFileInfo)
{
  APIRET     rc = NO_ERROR;                                /* API-Returncode */
  ULONG      ulAgeCurrent;             /* canonical file are of current file */
  FDATE      fdAgeCurrent;                             /* filedate structure */
  FILESTATUS fStat;                   /* filestatus structure for attributes */
  int        iAnswer;             /* user answer on the confirmation request */
  CHAR       szFileNameBuffer[MAXPATHLEN];         /* buffer for DosEditName */
  FILESTATUS3 fs3;   /* filestatus level 3 information from DosQueryPathInfo */
  CHAR       szFileDate[32];                     /* buffer for the file date */
  CHAR       szFileAttr[6];                /* buffer for the file attributes */
  CHAR       szTokenizerBuffer[MAXPATHLEN];               /* i hate strtok ! */
  FDATE      tsDate;
  FTIME      tsTime;

  if (Options.fsFileAge)                 /* deletion selective on file age */
  {
    if (Options.fsFileAgeWrite)
    {
      fdAgeCurrent = pFileInfo->fdateLastWrite;
      ulAgeCurrent = ToolsDateToAge(fdAgeCurrent.day,
                                    fdAgeCurrent.month,
                                    fdAgeCurrent.year + 1980);
    }
    else
      if (Options.fsFileAgeAccess)
      {
        fdAgeCurrent = pFileInfo->fdateLastAccess;
        ulAgeCurrent = ToolsDateToAge(fdAgeCurrent.day,
                                 fdAgeCurrent.month,
                                 fdAgeCurrent.year + 1980);
      }
      else
        if (Options.fsFileAgeCreate)
        {
          fdAgeCurrent = pFileInfo->fdateCreation;
          ulAgeCurrent = ToolsDateToAge(fdAgeCurrent.day,
                               fdAgeCurrent.month,
                               fdAgeCurrent.year + 1980);
        }
        else
        {
                                     /* none specified, take last-write date */
          fdAgeCurrent = pFileInfo->fdateLastWrite;
          ulAgeCurrent = ToolsDateToAge(fdAgeCurrent.day,
                                   fdAgeCurrent.month,
                                   fdAgeCurrent.year + 1980);
        }

                                           /* now check if it is to be found */
    if (Options.fsFileAgeNewer)
    {
      if (ulAgeCurrent < Globals.ulFileDate)
        return (NO_ERROR);                 /* abort processing for this file */
    }
    else
    {
      if (ulAgeCurrent > Globals.ulFileDate)
        return (NO_ERROR);                 /* abort processing for this file */
    }
  }


  /* check fize sizes */
  if (Options.fsSizeLarger)
      if (Options.ulSizeLarger > pFileInfo->cbFile)
          return (NO_ERROR);

  if (Options.fsSizeSmaller)
      if (Options.ulSizeSmaller < pFileInfo->cbFile)
          return (NO_ERROR);


                                            /* check with the file name mask */
  if (Options.fsFileNameMask)
  {
    PSZ  pszToken;  /* string pointer points to file token within filemask */
    BOOL fMatch;                         /* match - true, no match - false */

    strcpy (szTokenizerBuffer,                /* strtok is a real PITA !!! */
            Options.pszFileNameMask);

    pszToken = strtok(szTokenizerBuffer,                  /* tokenize this */
                      ",");
    fMatch   = FALSE;                               /* this is the default */

    while ( (pszToken != NULL) && /* as long as there is a name to process */
            (fMatch   == FALSE) )                /* and no match was found */
    {
      rc = DosEditName(1,                /* use OS/2 1.2 editing semantics */
                       pFileInfo->achName,                /* source string */
                       pszToken,                         /* editing string */
                       szFileNameBuffer,              /* local name buffer */
                       sizeof (szFileNameBuffer));        /* buffer length */
      if (rc != NO_ERROR)                              /* check for errors */
        return (rc);                              /* raise error condition */

      if (stricmp(pFileInfo->achName,     /* check if filename has changed */
                  szFileNameBuffer) == 0)
        fMatch = TRUE;                       /* the same, abort processing */

      pszToken = strtok(NULL,                    /* skip to the next token */
                        ",");
    }

    if (fMatch == FALSE)                           /* no match was found ! */
      return (NO_ERROR);                  /* then go not beyond this point */
  }


  StrFAttrToStringShort (pFileInfo->attrFile,        /* map the attributes */
                         szFileAttr);

  /* check which timestamp to display */
  if (Options.fsFileAgeWrite)
  {
      tsDate = pFileInfo->fdateLastWrite;
      tsTime = pFileInfo->ftimeLastWrite;
  }
  else
      if (Options.fsFileAgeAccess)
      {
          tsDate = pFileInfo->fdateLastAccess;
          tsTime = pFileInfo->ftimeLastAccess;
      }
      else
      {
          tsDate = pFileInfo->fdateCreation;
          tsTime = pFileInfo->ftimeCreation;
      }

  StrFDateTimeToString (tsDate,                      /* map the date       */
                        tsTime,
                        szFileDate);

  if (Options.fsDisplayNamesOnly)
      puts(pszPath);
  else
      printf ("\n  %9u %s %s %s",
              pFileInfo->cbFile,
              szFileAttr,
              szFileDate,
              pszPath);

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

  /* @@@PH
  printf ("\n%s",
  pszPath);
  */

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
          rc = ProcessScan(pszPathNext);                          /* Recurse */
          if (rc != NO_ERROR)
            ToolsErrorDos(rc);

          rc = FindDir(pszPathNext);                   /* find the directory */
          if (rc != NO_ERROR)
            ToolsErrorDos(rc);
        }
        else
        {
          rc = FindFile(pszPathNext,
                          pFileInfo);                       /* find the file */
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


  if (Options.fsFileAge)            /* check for extended find functionality */
  {
    DATETIME dtDateTime;          /* structure to hold current date and time */

    DosGetDateTime(&dtDateTime);                        /* query system time */

    Globals.ulFileDate = ToolsDateToAge(dtDateTime.day,           /* transform it */
                                        dtDateTime.month,
                                        dtDateTime.year)
        - Options.ulFileAge;
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

  if (!Options.fsDisplayNamesOnly)
  {
      if (!Options.fsFileNameMask)
          printf ("\nScanning [%s]",
                  Globals.szRootPath);
      else
          printf ("\nScanning [%s\\%s]",
                  Globals.szRootPath,
                  Options.pszFileNameMask);
  }

  ToolsPerfQuery(&psStart);                 /* start the performance counter */

  rc=ProcessScan (Globals.szRootPath);                          /* do it ... */
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  ToolsPerfQuery(&psEnd);                   /* stop  the performance counter */


                                                 /* now print the statistics */
  fSeconds = psEnd.fSeconds - psStart.fSeconds;        /* calculate duration */
  if (Options.fsStatistics)                /* if we have to print statistics */
    if (fSeconds != 0.0)                           /* avoid division by zero */
    {
      CHAR szValueBytes[20];                         /* local string buffers */
      CHAR szValueAlloc[20];

      StrValueToSize(szValueBytes,
                     Globals.ulFoundBytes);
      StrValueToSize(szValueAlloc,
                     Globals.ulFoundAlloc);

      printf ("\nTime needed        :             %10.3f seconds"
              "\nScanned bytes      : %-10s"
              "\nScanned bytes      : %-10s (allocation)"
              "\nFiles scanned      : %10u (%10.3f files per sec)"
              "\nDirectories scanned: %10u (%10.3f files per sec)",
              fSeconds,
              szValueBytes,
              szValueAlloc,
              Globals.ulFilesScanned,
              Globals.ulFilesScanned / fSeconds,
              Globals.ulDirectoriesScanned,
              Globals.ulDirectoriesScanned / fSeconds);
    }

  return (rc);
}
