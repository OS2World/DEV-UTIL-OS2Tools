/*****************************************************************************
 * Name      : File DELDIR.CPP
 * Funktion  : Delete Directory
 * Bemerkung : (c) Copyright 1994,97 Patrick Haller Systemtechnik
 *
 * Autor     : Patrick Haller [Mittwoch, 11.05.1994 14.03.43]
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
  ARGFLAG fsSimulation;                        /* user wants simulation only */
  ARGFLAG fsForceDelete;                       /* user wants to force delete */
  ARGFLAG fsShowFiles;                /* user wants us to show the files     */
  ARGFLAG fsDontShowDirs;             /* do not show the directory names     */
  ARGFLAG fsDontDeleteDirs;                  /* do not remove subdirectories */
  ARGFLAG fsDontDeleteRoot;                  /* do not remove root directory */
  ARGFLAG fsDontDeleteFiles;                          /* do not remove files */
  ARGFLAG fsFileAge;                            /* deletion selective on age */
  ARGFLAG fsFileAgeWrite;                /* deletion selective on last-write */
  ARGFLAG fsFileAgeAccess;              /* deletion selective on last-access */
  ARGFLAG fsFileAgeCreate;                 /* deletion selective on creation */
  ARGFLAG fsFileAgeNewer;                           /* inverse the selection */
  ARGFLAG fsConfirmationSkip;             /* user skips initial confirmation */
  ARGFLAG fsConfirmationPrompt;  /* user wants to be prompted for every file */
  ARGFLAG fsRemoveAttributes;                 /* try all to delete the files */
  ARGFLAG fsFileNameMask;     /* user wants only special files to be deleted */

  ULONG ulFileAge;                        /* files older than this to delete */
  PSZ  pszPath;                                            /* directory path */
  PSZ  pszFileNameMask;        /* special file name mask including wildcards */
} OPTIONS, *POPTIONS;


typedef struct
{
  ULONG ulFindNumberMaximum;         /* number of files to scan per API call */
  ULONG ulFileFindBufferSize;              /* buffer size of the scan buffer */
  ULONG ulFileMask;                   /* mask of file attributes to scan for */
  ULONG ulFileDate;                       /* files older than this to delete */

  CHAR  szRootPath[MAXPATHLEN];           /* buffer for the root path string */

  ULONG ulFilesScanned;                           /* number of files scanned */
  ULONG ulDirectoriesScanned;               /* number of directories scanned */
  ULONG ulFilesDeleted;                           /* number of files deleted */
  ULONG ulDirectoriesDeleted;               /* number of removed directories */

  ULONG ulDeletedBytes;        /* number of bytes contained in files deleted */
  ULONG ulDeletedAlloc;                       /* actually freed up diskspace */
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
  {"/SIM",       "Simulation only, no deletion.",NULL,                 ARG_NULL,       &Options.fsSimulation},
  {"/FORCE",     "Deleted files are non-recover"
                 "able.",                        NULL,                 ARG_NULL,       &Options.fsForceDelete},
  {"/A",         "Reset attributes of the files"
                 " if non-deletable otherwise.", NULL,                 ARG_NULL,       &Options.fsRemoveAttributes},
  {"/SF",        "Show the files.",              NULL,                 ARG_NULL,       &Options.fsShowFiles},
  {"/!SD",       "Don't show the directories.",  NULL,                 ARG_NULL,       &Options.fsDontShowDirs},
  {"/!DD",       "Don't remove directories.",    NULL,                 ARG_NULL,       &Options.fsDontDeleteDirs},
  {"/!DR",       "Don't remove root directory.", NULL,                 ARG_NULL,       &Options.fsDontDeleteRoot},
  {"/!DF",       "Don't remove files.",          NULL,                 ARG_NULL,       &Options.fsDontDeleteFiles},
  {"/DAGE=",     "Remove files older than this.",&Options.ulFileAge,   ARG_ULONG,      &Options.fsFileAge},
  {"/DAGE.WRITE","Last-write-date is relevant"
                 " (HPFS)."                     ,NULL,                 ARG_NULL,       &Options.fsFileAgeWrite},
  {"/DAGE.ACCESS","Last-access-date is relevant"
                 " (HPFS)."                     ,NULL,                 ARG_NULL,       &Options.fsFileAgeAccess},
  {"/DAGE.CREATE","Creation-date is relevant"
                 " (HPFS)."                     ,NULL,                 ARG_NULL,       &Options.fsFileAgeCreate},
  {"/DAGE.NEWER","Remove files younger than <age>"
                 " instead of older."           ,NULL,                 ARG_NULL,       &Options.fsFileAgeNewer},
  {"/Y",         "Skip initial confirmation for "
                 "the deletion."                ,NULL,                 ARG_NULL,       &Options.fsConfirmationSkip},
  {"/PROMPT",    "Prompts for deletion of every"
                 " file and directory."         ,NULL,                 ARG_NULL,       &Options.fsConfirmationPrompt},
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
APIRET DeleteDir           (PSZ          pszPath);
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
  TOOLVERSION("Deldir",                                 /* application name */
              0x00010009,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


/*****************************************************************************
 * Name      : APIRET DeleteDir
 * Funktion  : Delete one directory
 * Parameter : PSZ path      - the current path
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:48:00]
 *****************************************************************************/

APIRET DeleteDir (PSZ pszPath)
{
  APIRET rc = NO_ERROR;                                    /* API-Returncode */
  int    iAnswer;                /* answer from the user confirmation prompt */
  FILESTATUS fStat;                   /* filestatus structure for attributes */

  Globals.ulDirectoriesScanned++;                   /* update the statistics */

  if (!Options.fsDontDeleteDirs)             /* if we have to remove subdirs */
  {
                                                     /* remove the directory */
    if (Options.fsSimulation)                    /* simulate deletion only ? */
      printf ("\n  rd %s",
              pszPath);
    else
    {
      if (Options.fsVerbose ||                           /* verbose output ? */
          Options.fsConfirmationPrompt)
        printf ("\nRemoving %s ",
                pszPath);

      if (Options.fsConfirmationPrompt)       /* prompt for every deletion ? */
      {
        iAnswer = ToolsConfirmationQuery();                  /* ask the user */
        switch (iAnswer)
        {
          case 0:                                                      /* no */
            return (NO_ERROR);                           /* abort processing */

          case 1:                                                     /* yes */
            break;                                           /* continue ... */

          case 2:                                                  /* escape */
            exit (1);                     /* PH: urgs, terminate the process */
        }
      }

      rc = DosDeleteDir(pszPath);                 /* OK, remove that thing ! */
      if ( (rc == ERROR_ACCESS_DENIED) &&             /* check for READ-ONLY */
          Options.fsRemoveAttributes)
      {
        rc = DosQueryPathInfo (pszPath,            /* query file information */
                               FIL_STANDARD,
                               &fStat,
                               sizeof(fStat));
        if (rc == NO_ERROR)                              /* check for errors */
        {
          fStat.attrFile = FILE_NORMAL;              /* reset the attributes */
          rc = DosSetPathInfo (pszPath,               /* set the information */
                               FIL_STANDARD,
                               &fStat,
                               sizeof(fStat),
                               0L);
          if (rc == NO_ERROR)                            /* check for errors */                                                            /* now try again */
            rc = DosDeleteDir(pszPath);           /* OK, remove that thing ! */
        }
      }

      if (rc == NO_ERROR)                                /* check for errors */
        Globals.ulDirectoriesDeleted++;             /* update the statistics */
    }
  }

  return (rc);                                                         /* ok */
}


/*****************************************************************************
 * Name      : APIRET DeleteFile
 * Funktion  : Delete one file
 * Parameter : PSZ path      - the current path
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:48:00]
 *****************************************************************************/

APIRET DeleteFile (PSZ          pszPath,
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

  if (!Options.fsDontDeleteFiles)              /* if we have to remove files */
  {
                                                            /* check for age */
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

                                         /* now check if it is to be deleted */
      if (Options.fsFileAgeNewer)
      {
        if (ulAgeCurrent < Globals.ulFileDate)
          return (NO_ERROR);               /* abort processing for this file */
      }
      else
      {
        if (ulAgeCurrent > Globals.ulFileDate)
          return (NO_ERROR);               /* abort processing for this file */
      }
    }


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

    StrFDateTimeToString (pFileInfo->fdateLastWrite,   /* map the date       */
                          pFileInfo->ftimeLastWrite,
                          szFileDate);

    if (Options.fsSimulation)                    /* simulate deletion only ? */
      printf ("\n  %9u %s %s %s",
              pFileInfo->cbFile,
              szFileAttr,
              szFileDate,
              pFileInfo->achName);
    else
    {
      if (Options.fsVerbose ||                           /* verbose output ? */
          Options.fsShowFiles ||
          Options.fsConfirmationPrompt)
        printf ("\n  %9u %s %s %s",
                pFileInfo->cbFile,
                szFileAttr,
                szFileDate,
                pFileInfo->achName);

      if (Options.fsConfirmationPrompt)       /* prompt for every deletion ? */
      {
        iAnswer = ToolsConfirmationQuery();                  /* ask the user */
        switch (iAnswer)
        {
          case 0:                                                      /* no */
            return (NO_ERROR);                           /* abort processing */

          case 1:                                                     /* yes */
            break;                                           /* continue ... */

          case 2:                                                  /* escape */
            exit (1);                     /* PH: urgs, terminate the process */
        }
      }


      if (Options.fsForceDelete)
        rc = DosForceDelete(pszPath);        /* file will not be recoverable */
      else
        rc = DosDelete(pszPath);                  /* OK, remove that thing ! */

      if ( (rc == ERROR_ACCESS_DENIED) &&             /* check for READ-ONLY */
          Options.fsRemoveAttributes)
      {
        rc = DosQueryPathInfo (pszPath,            /* query file information */
                               FIL_STANDARD,
                               &fStat,
                               sizeof(fStat));
        if (rc != NO_ERROR)                              /* check for errors */
          return (rc);                              /* raise error condition */

        fStat.attrFile = FILE_NORMAL;                /* reset the attributes */

        rc = DosSetPathInfo (pszPath,                 /* set the information */
                             FIL_STANDARD,
                             &fStat,
                             sizeof(fStat),
                             0L);
        if (rc != NO_ERROR)                              /* check for errors */
          return (rc);                              /* raise error condition */

                                                            /* now try again */
        if (Options.fsForceDelete)
          rc = DosForceDelete(pszPath);      /* file will not be recoverable */
        else
          rc = DosDelete(pszPath);                /* OK, remove that thing ! */
      }
    }

    if (rc == NO_ERROR)             /* if the file has actually been deleted */
    {
      Globals.ulDeletedBytes += pFileInfo->cbFile;
      Globals.ulDeletedAlloc += pFileInfo->cbFileAlloc;
      Globals.ulFilesDeleted++;                     /* update the statistics */
    }
  }

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

  if (!Options.fsDontShowDirs)         /* shall we display directory names ? */
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
          rc = ProcessScan(pszPathNext);                          /* Recurse */
          if (rc != NO_ERROR)
            ToolsErrorDos(rc);

          rc = DeleteDir(pszPathNext);               /* remove the directory */
          if (rc != NO_ERROR)
            ToolsErrorDos(rc);
        }
        else
        {
          rc = DeleteFile(pszPathNext,
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


  if (Options.fsFileAge)          /* check for extended delete functionality */
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

  if (!Options.fsFileNameMask)
    printf ("\nDeleting [%s]",
            Globals.szRootPath);
  else
    printf ("\nDeleting [%s\\%s]",
            Globals.szRootPath,
            Options.pszFileNameMask);


  if (!Options.fsConfirmationSkip)            /* skip initial confirmation ? */
  {
    iAnswer = ToolsConfirmationQuery();                      /* ask the user */
    switch (iAnswer)
    {
      case 0:                                                          /* no */
        return (NO_ERROR);                               /* abort processing */

      case 1:                                                         /* yes */
        break;                                               /* continue ... */

      case 2:                                                      /* escape */
        exit (1);                         /* PH: urgs, terminate the process */
    }
  }

                               /* now perform a little bit parameter mapping */
  if (Options.fsFileNameMask) /* if the user supplied special file name mask */
  {
    Options.fsDontDeleteDirs = TRUE;                       /* makes no sense */
    Options.fsDontDeleteRoot = TRUE;                       /* makes no sense */
    Options.fsShowFiles      = TRUE;                /* display the filenames */
  }


  ToolsPerfQuery(&psStart);                 /* start the performance counter */

  rc=ProcessScan (Globals.szRootPath);                          /* do it ... */
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);
  else
  {
    if (!Options.fsDontDeleteRoot &&    /* if we have to delete the root dir */
        !Options.fsDontDeleteDirs)
    {
      rc=DeleteDir(Globals.szRootPath);                /* now remove root path */
      if (rc != NO_ERROR)
        ToolsErrorDos(rc);
    }
  }

  ToolsPerfQuery(&psEnd);                   /* stop  the performance counter */


                                                 /* now print the statistics */
  fSeconds = psEnd.fSeconds - psStart.fSeconds;        /* calculate duration */
  if (Options.fsStatistics)                /* if we have to print statistics */
    if (fSeconds != 0.0)                           /* avoid division by zero */
    {
      CHAR szValueBytes[20];                         /* local string buffers */
      CHAR szValueAlloc[20];

      StrValueToSize(szValueBytes,
                     Globals.ulDeletedBytes);
      StrValueToSize(szValueAlloc,
                     Globals.ulDeletedAlloc);

      printf ("\nTime needed        :             %10.3f seconds"
              "\nDeleted bytes      : %-10s"
              "\nFreed   bytes      : %-10s (allocation)"
              "\nFiles scanned      : %10u (%10.3f files per sec)"
              "\nFiles deleted      : %10u (%10.3f files per sec)"
              "\nDirectories scanned: %10u (%10.3f files per sec)"
              "\nDirectories removed: %10u (%10.3f files per sec)",
              fSeconds,
              szValueBytes,
              szValueAlloc,
              Globals.ulFilesScanned,
              Globals.ulFilesScanned / fSeconds,
              Globals.ulFilesDeleted,
              Globals.ulFilesDeleted / fSeconds,
              Globals.ulDirectoriesScanned,
              Globals.ulDirectoriesScanned / fSeconds,
              Globals.ulDirectoriesDeleted,
              Globals.ulDirectoriesDeleted / fSeconds);
    }

  return (rc);
}
