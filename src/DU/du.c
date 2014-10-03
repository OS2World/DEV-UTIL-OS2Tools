/******************************************************************************
 * Name      : File DU.C
 * Funktion  : Disk Usage
 * Bemerkung : (c) Copyright 1994,97 Patrick Haller
 *
 * Autor     : Patrick Haller [Mittwoch, 09.06.1994 02.03.43]
 ******************************************************************************/


 /*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSMISC                                         /* DosGetMessage */
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <malloc.h>
#include <string.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"


/******************************************************************************
 * Structures                                                                 *
 ******************************************************************************/

/* workaround for long long support with VAC 3.08 */
#define BIGNUMBER float


typedef struct     /* structure for results of scanning one single directory */
{
  BIGNUMBER llBytesAllocated;
  BIGNUMBER llBytesSize;
  BIGNUMBER llFiles;
} ANALYSE, *PANALYSE;


typedef struct
{
  ULONG ulRecursionDepth;

  #define MAXPATHLEN              260
  PSZ    pszPathNext;
  ULONG  ulFileMask;

  /* Variables for attribute scanning */
  ULONG attrNormal;
  ULONG attrArchived;
  ULONG attrReadonly;
  ULONG attrDirectory;
  ULONG attrDirpseudo;
  ULONG attrSystem;
  ULONG attrHidden;
  ULONG attrCompressed;
  ULONG attrUndefined;

  ULONG ulFilesScanned;

  ULONG ulFileFindBufferSize;    /* size of the DosFindFirst buffer in bytes */

  char   DUPathStr[MAXPATHLEN];
  char   szBuf_1[20];
  char   szBuf_2[20];

  CHAR       szFileDate[32];                     /* buffer for the file date */
  CHAR       szFileAttr[6];                /* buffer for the file attributes */

                                                       /* statistical values */
  float  fTotalAgeCurrent;                               /* our current date */
  float  fTotalAgeCreation;                        /* sum of all file's ages */
  float  fTotalAgeLastWrite;                       /* sum of all file's ages */
  float  fTotalAgeLastAccess;                      /* sum of all file's ages */

  float  fTotalCompression;         /* virtual sum of all compression ratios */

  BOOL   bMaximumCompression;  /* indicates if maximum compression specified */
  float  fMaximumCompression;                  /* record maximum compression */
  char   pszMaximumCompression[MAXPATHLEN];

  BOOL   bMaximumFileSize;     /* indicates if maximum file size   specified */
  ULONG  ulMaximumFileSize;                                  /* largest file */
  char   pszMaximumFileSize[MAXPATHLEN];

  BOOL   bMaximumSlackSpace;   /* indicates if maximum slack space specified */
  int    iMaximumSlackSpace;
  char   pszMaximumSlackSpace[MAXPATHLEN];

  BOOL   bMaximumAllocationSize; /* indic.  if maximum allocation size       */
  ULONG  ulMaximumAllocationSize;                 /* largest allocation size */
  char   pszMaximumAllocationSize[MAXPATHLEN];

  BOOL   bMaximumCreation;     /* indicates if maximum creation date specif. */
  FDATE  fdateMaximumCreation;                  /* most recent file creation */
  FTIME  ftimeMaximumCreation;
  char   pszMaximumCreation[MAXPATHLEN];

  BOOL   bMaximumLastAccess;                /* indicates maximum last access */
  FDATE  fdateMaximumLastAccess;                /* most recent accessed file */
  FTIME  ftimeMaximumLastAccess;
  char   pszMaximumLastAccess[MAXPATHLEN];

  BOOL   bMaximumLastWrite;                 /* indicates maximum last write  */
  FDATE  fdateMaximumLastWrite;                    /* most recent file write */
  FTIME  ftimeMaximumLastWrite;
  char   pszMaximumLastWrite[MAXPATHLEN];

  BOOL   bMinimumCompression;               /* indicates minimum compression */
  float  fMinimumCompression;                  /* record minimum compression */
  char   pszMinimumCompression[MAXPATHLEN];

  BOOL   bMinimumFileSize;                  /* indicates minimum file size   */
  ULONG  ulMinimumFileSize;                                 /* smallest file */
  char   pszMinimumFileSize[MAXPATHLEN];

  BOOL   bMinimumAllocationSize;            /* indicates minimum alloc size  */
  ULONG  ulMinimumAllocationSize;
  char   pszMinimumAllocationSize[MAXPATHLEN];

  BOOL   bMinimumSlackSpace;   /* indicates if minimum slack space specified */
  int    iMinimumSlackSpace;
  char   pszMinimumSlackSpace[MAXPATHLEN];

  BOOL   bMinimumCreation;                  /* indicates minimum creation    */
  FDATE  fdateMinimumCreation;                  /* most recent file creation */
  FTIME  ftimeMinimumCreation;
  char   pszMinimumCreation[MAXPATHLEN];

  BOOL   bMinimumLastAccess;                /* indicates minimum last access */
  FDATE  fdateMinimumLastAccess;                /* most recent accessed file */
  FTIME  ftimeMinimumLastAccess;
  char   pszMinimumLastAccess[MAXPATHLEN];

  BOOL   bMinimumLastWrite;                 /* indicates minimum last write  */
  FDATE  fdateMinimumLastWrite;                    /* most recent file write */
  FTIME  ftimeMinimumLastWrite;
  char   pszMinimumLastWrite[MAXPATHLEN];
} GLOBALS, *PGLOBALS;


typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsPath;                                    /* path supplied or not */
  PSZ     pszPath;                               /* pointer to the path data */

  ARGFLAG fsSpeed;                            /* measure scanning speed only */
  ARGFLAG fsSpeedNr;                          /* how often to benchmark      */
  ULONG   ulSpeedNr;                  /* how often to benchmark the scanning */
  ARGFLAG fsFiles;                                         /* show files     */
  ARGFLAG fsFiles2;                                        /* show files     */
  ARGFLAG fsIterate;            /* show statistics on subsequent directories */
  ARGFLAG fsAttributes;                           /* show attribute analysis */
  ULONG   ulRecursionDepthMaximum;                /* maximum recursion depth */
  ARGFLAG fsRecursion;                  /* maximum recursion depth specified */
  ULONG   ulFindNumberMaximum;                        /* maximum find number */
  ARGFLAG fsFind;                           /* maximum find number specified */
  ARGFLAG fsExcludeDirectories;         /* exclude directories from scanning */

  ARGFLAG fsFileNameMask;    /* indicates if user specified a file name mask */
  PSZ     pszFileNameMask;                             /* the file name mask */
} OPTIONS, *POPTIONS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/
OPTIONS Options;                /* this structure holds command line options */
GLOBALS Globals;                     /* this structure holds global varibles */

ARGUMENT TabArguments[] =
{ /*Token-------Beschreibung-----------------------pTarget--------------------------ucTargetFmt-pTargetSpecified--*/
  {"/?",        "Get help screen.",                NULL,                            ARG_NULL,   &Options.fsHelp},
  {"/H",        "Get help screen.",                NULL,                            ARG_NULL,   &Options.fsHelp},
  {"/A",        "Attribute statistics.",           NULL,                            ARG_NULL |
                                                                                    ARG_HIDDEN, &Options.fsAttributes},
  {"/STAT",     "Attribute statistics.",           NULL,                            ARG_NULL,   &Options.fsAttributes},
  {"/I",        "Include info on subdirectories.", NULL,                            ARG_NULL,   &Options.fsIterate},
  {"/FILES2",   "Show more file information.",     NULL,                            ARG_NULL,   &Options.fsFiles2},
  {"/F2",       "Show more file information.",     NULL,                            ARG_NULL,   &Options.fsFiles2},
  {"/FILES",    "Show file information.",          NULL,                            ARG_NULL,   &Options.fsFiles},
  {"/F",        "Show file information.",          NULL,                            ARG_NULL,   &Options.fsFiles},
  {"/ED",       "Exclude directories from statistics",
                                                   NULL,                            ARG_NULL,   &Options.fsExcludeDirectories},
  {"/SPEED.NR=","How often to benchmark.",         &Options.ulSpeedNr,              ARG_ULONG,  &Options.fsSpeedNr},
  {"/SPEED",    "Measure scanning speed only.",    NULL,                            ARG_NULL,   &Options.fsSpeed},
  {"/R",        "Maximum recursion depth e.g. /R3",&Options.ulRecursionDepthMaximum,ARG_ULONG,  &Options.fsRecursion},
  {"/CRAP",     "Compatibility to some Windows 95 requester",
                                                   NULL,                            ARG_NULL,   &Options.fsFind},
  {"/NAME=",    "File name mask.",                 &Options.pszFileNameMask,        ARG_PSZ,    &Options.fsFileNameMask},
  {"1",         "Directory, pathname.",            &Options.pszPath,                ARG_PSZ |
                                                                                    ARG_DEFAULT,&Options.fsPath},
  ARG_TERMINATE
};


/******************************************************************************
 * Prototypes                                                                 *
 ******************************************************************************/

void   help            (void);

void   scan_attributes (PSZ          pszPath,
                        FILEFINDBUF3 *pFileInfo);

APIRET ProcessRootPath (PSZ pszPath);

APIRET ProcessScan     (PSZ,
                        ANALYSE *);


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
  TOOLVERSION("DiskUsage",                              /* application name */
              0x00010005,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


/*****************************************************************************
 * Name      : void scan_attributes
 * Funktion  : Ascertening of the files attributes and creating statistics
 * Parameter : PSZ           pszPath,
 *             PFILEFINDBUF3 pFileData
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:48:00]
 *****************************************************************************/

void scan_attributes (PSZ          pszPath,
                      FILEFINDBUF3 *pFileInfo)
{
  char   szFileNameBuffer[MAXPATHLEN];             /* buffer for DosEditName */
  CHAR   szTokenizerBuffer[MAXPATHLEN];                   /* i hate strtok ! */
  APIRET rc;                                               /* API returncode */

  if (Options.fsExcludeDirectories)              /* shall dirs be included ? */
    if (pFileInfo->attrFile & FILE_DIRECTORY)
      return;

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
        return;                                   /* raise error condition */

      if (stricmp(pFileInfo->achName,     /* check if filename has changed */
                  szFileNameBuffer) == 0)
        fMatch = TRUE;                       /* the same, abort processing */

      pszToken = strtok(NULL,                    /* skip to the next token */
                        ",");
    }

    if (fMatch == FALSE)                           /* no match was found ! */
      return;                             /* then go not beyond this point */
  }


  sprintf (szFileNameBuffer,
           "%s\\%s",
           pszPath,
           pFileInfo->achName);


  /***************************************************************************
   * display file information                                                *
   ***************************************************************************/

  if (Options.fsFiles2)                               /* very verbose output */
  {
    StrFAttrToStringShort (pFileInfo->attrFile,        /* map the attributes */
                           Globals.szFileAttr);

    printf ("\n  %s %s",                              /* attributes and name */
            Globals.szFileAttr,
            szFileNameBuffer);

    StrFDateTimeToString
                  (pFileInfo->fdateCreation,           /* map the date       */
                   pFileInfo->ftimeCreation,
                   Globals.szFileDate);

    printf ("\n        Creation    %s, Actual    Size %9u",
            Globals.szFileDate,
            pFileInfo->cbFile);

    StrFDateTimeToString
                  (pFileInfo->fdateLastAccess,         /* map the date       */
                   pFileInfo->ftimeLastAccess,
                   Globals.szFileDate);

    printf ("\n        Last Access %s, Allocated Size %9u",
            Globals.szFileDate,
            pFileInfo->cbFileAlloc);

    StrFDateTimeToString
                  (pFileInfo->fdateLastWrite,          /* map the date       */
                   pFileInfo->ftimeLastWrite,
                   Globals.szFileDate);

    printf ("\n        Last Write  %s, Slack space    %9i",
            Globals.szFileDate,
            pFileInfo->cbFileAlloc - pFileInfo->cbFile);

    if ( (pFileInfo->cbFileAlloc != 0) &&
         (pFileInfo->cbFileAlloc < pFileInfo->cbFile) )
      printf ("\n                                      Compression       1:%2.2f",
              (float)pFileInfo->cbFile / (float)pFileInfo->cbFileAlloc);
  }
  else
    if (Options.fsFiles)           /* if we have to display file information */
    {
      StrFAttrToStringShort (pFileInfo->attrFile,      /* map the attributes */
                             Globals.szFileAttr);

      StrFDateTimeToString
                    (pFileInfo->fdateLastWrite,        /* map the date       */
                     pFileInfo->ftimeLastWrite,
                     Globals.szFileDate);

      printf ("\n  %9u %s %s %s",
              pFileInfo->cbFile,
              Globals.szFileAttr,
              Globals.szFileDate,
              szFileNameBuffer);
    }


  /***************************************************************************
   * attribute flags                                                         *
   ***************************************************************************/

  if (pFileInfo->cbFileAlloc < pFileInfo->cbFile)
    Globals.attrCompressed++;

  if (!pFileInfo->attrFile)
    Globals.attrNormal++;

  if (pFileInfo->attrFile & FILE_HIDDEN)
    Globals.attrHidden++;

  if (pFileInfo->attrFile & FILE_READONLY)
    Globals.attrReadonly++;

  if (pFileInfo->attrFile & FILE_DIRECTORY)
    Globals.attrDirectory++;

  if ((pFileInfo->achName[0] == '.') &&
      (( !pFileInfo->achName[1] ||
      ((pFileInfo->achName[1] == '.') && !pFileInfo->achName[2])) ))
      Globals.attrDirpseudo++;

  if (pFileInfo->attrFile & FILE_ARCHIVED)
    Globals.attrArchived++;

  if (pFileInfo->attrFile & FILE_SYSTEM)
    Globals.attrSystem++;

                                                /* Find undefined attributes */
  if (pFileInfo->attrFile & 0xffffffc0)
    Globals.attrUndefined++;


  /***************************************************************************
   * minimum and maximum compression                                         *
   ***************************************************************************/
  {
    float fCompressionRate;

    if ( (pFileInfo->cbFileAlloc != 0) &&                   /* prevent div 0 */
         (pFileInfo->cbFileAlloc < pFileInfo->cbFile) )
    {
      fCompressionRate = (float)pFileInfo->cbFile /
                         (float)pFileInfo->cbFileAlloc;

      Globals.fTotalCompression += fCompressionRate;     /* build up the sum */

      if ( (fCompressionRate > Globals.fMaximumCompression) ||    /* maximum */
           (Globals.bMaximumCompression == FALSE) )
      {
        Globals.fMaximumCompression = fCompressionRate;
        Globals.bMaximumCompression = TRUE;

        strcpy (Globals.pszMaximumCompression,
                szFileNameBuffer);
      }

      if ( (fCompressionRate < Globals.fMinimumCompression) ||    /* minimum */
           (Globals.bMinimumCompression == FALSE) )
      {
        Globals.fMinimumCompression = fCompressionRate;
        Globals.bMinimumCompression = TRUE;

        strcpy (Globals.pszMinimumCompression,
                szFileNameBuffer);
      }
    }
  }


  /***************************************************************************
   * minimum and maximum file and allocation size                            *
   ***************************************************************************/
  {
    int iSlackSpace = pFileInfo->cbFileAlloc - pFileInfo->cbFile;

    if ( (pFileInfo->cbFile > Globals.ulMaximumFileSize) ||
         (Globals.bMaximumFileSize == FALSE) )
    {
      Globals.ulMaximumFileSize = pFileInfo->cbFile;
      Globals.bMaximumFileSize = TRUE;

      strcpy (Globals.pszMaximumFileSize,
              szFileNameBuffer);
    }

    if ( (pFileInfo->cbFileAlloc > Globals.ulMaximumAllocationSize) ||
         (Globals.bMaximumAllocationSize == FALSE) )
    {
      Globals.ulMaximumAllocationSize = pFileInfo->cbFileAlloc;
      Globals.bMaximumAllocationSize = TRUE;

      strcpy (Globals.pszMaximumAllocationSize,
              szFileNameBuffer);
    }

    if ( (iSlackSpace > Globals.iMaximumSlackSpace) ||
         (Globals.bMaximumSlackSpace == FALSE) )
    {
      Globals.iMaximumSlackSpace = iSlackSpace;
      Globals.bMaximumSlackSpace = TRUE;

      strcpy (Globals.pszMaximumSlackSpace,
              szFileNameBuffer);
    }

    if ( (pFileInfo->cbFile < Globals.ulMinimumFileSize) ||
         (Globals.bMinimumFileSize == FALSE) )
    {
      Globals.ulMinimumFileSize = pFileInfo->cbFile;
      Globals.bMinimumFileSize = TRUE;

      strcpy (Globals.pszMinimumFileSize,
              szFileNameBuffer);
    }

    if ( (pFileInfo->cbFileAlloc < Globals.ulMinimumAllocationSize) ||
         (Globals.bMinimumAllocationSize == FALSE) )
    {
      Globals.ulMinimumAllocationSize = pFileInfo->cbFileAlloc;
      Globals.bMinimumAllocationSize = TRUE;

      strcpy (Globals.pszMinimumAllocationSize,
              szFileNameBuffer);
    }

    if ( (iSlackSpace < Globals.iMinimumSlackSpace) ||
         (Globals.bMinimumSlackSpace == FALSE) )
    {
      Globals.iMinimumSlackSpace = iSlackSpace;
      Globals.bMinimumSlackSpace = TRUE;

      strcpy (Globals.pszMinimumSlackSpace,
              szFileNameBuffer);
    }
  }


  /***************************************************************************
   * maximum file dates (most recent files)                                  *
   ***************************************************************************/
  {
    Globals.fTotalAgeCreation   += (float)ToolsDateToAge(pFileInfo->fdateCreation.day,
                                                         pFileInfo->fdateCreation.month,
                                                         pFileInfo->fdateCreation.year + 1980)
                                   - Globals.fTotalAgeCurrent;
    Globals.fTotalAgeLastWrite  += (float)ToolsDateToAge(pFileInfo->fdateLastWrite.day,
                                                         pFileInfo->fdateLastWrite.month,
                                                         pFileInfo->fdateLastWrite.year + 1980)
                                   - Globals.fTotalAgeCurrent;
    Globals.fTotalAgeLastAccess += (float)ToolsDateToAge(pFileInfo->fdateLastAccess.day,
                                                         pFileInfo->fdateLastAccess.month,
                                                         pFileInfo->fdateLastAccess.year + 1980)
                                   - Globals.fTotalAgeCurrent;

    if ( (ToolsDateCompare(pFileInfo->fdateCreation,
                           pFileInfo->ftimeCreation,
                           Globals.fdateMaximumCreation,
                           Globals.ftimeMaximumCreation) > 0 ) ||
         (Globals.bMaximumCreation == FALSE) )
    {
      Globals.fdateMaximumCreation = pFileInfo->fdateCreation;
      Globals.ftimeMaximumCreation = pFileInfo->ftimeCreation;
      Globals.bMaximumCreation = TRUE;

      strcpy (Globals.pszMaximumCreation,
              szFileNameBuffer);
    }

    if ( (ToolsDateCompare(pFileInfo->fdateLastAccess,
                           pFileInfo->ftimeLastAccess,
                           Globals.fdateMaximumLastAccess,
                           Globals.ftimeMaximumLastAccess) > 0 ) ||
         (Globals.bMaximumLastAccess == FALSE) )

    {
      Globals.fdateMaximumLastAccess = pFileInfo->fdateLastAccess;
      Globals.ftimeMaximumLastAccess = pFileInfo->ftimeLastAccess;
      Globals.bMaximumLastAccess = TRUE;

      strcpy (Globals.pszMaximumLastAccess,
              szFileNameBuffer);
    }

    if ( (ToolsDateCompare(pFileInfo->fdateLastWrite,
                           pFileInfo->ftimeLastWrite,
                           Globals.fdateMaximumLastWrite,
                           Globals.ftimeMaximumLastWrite) > 0 ) ||
         (Globals.bMaximumLastWrite == FALSE) )
    {
      Globals.fdateMaximumLastWrite = pFileInfo->fdateLastWrite;
      Globals.ftimeMaximumLastWrite = pFileInfo->ftimeLastWrite;
      Globals.bMaximumLastWrite = TRUE;

      strcpy (Globals.pszMaximumLastWrite,
              szFileNameBuffer);
    }
  }


  /***************************************************************************
   * minimum file dates (oldest files)                                       *
   ***************************************************************************/
  {
    if ( (ToolsDateCompare(pFileInfo->fdateCreation,
                           pFileInfo->ftimeCreation,
                           Globals.fdateMinimumCreation,
                           Globals.ftimeMinimumCreation) < 0 ) ||
         (Globals.bMinimumCreation == FALSE) )
    {
      Globals.fdateMinimumCreation = pFileInfo->fdateCreation;
      Globals.ftimeMinimumCreation = pFileInfo->ftimeCreation;
      Globals.bMinimumCreation = TRUE;

      strcpy (Globals.pszMinimumCreation,
              szFileNameBuffer);
    }

    if ( (ToolsDateCompare(pFileInfo->fdateLastAccess,
                           pFileInfo->ftimeLastAccess,
                           Globals.fdateMinimumLastAccess,
                           Globals.ftimeMinimumLastAccess) < 0 ) ||
         (Globals.bMinimumLastAccess == FALSE) )
    {
      Globals.fdateMinimumLastAccess = pFileInfo->fdateLastAccess;
      Globals.ftimeMinimumLastAccess = pFileInfo->ftimeLastAccess;
      Globals.bMinimumLastAccess = TRUE;

      strcpy (Globals.pszMinimumLastAccess,
              szFileNameBuffer);
    }

    if ( (ToolsDateCompare(pFileInfo->fdateLastWrite,
                           pFileInfo->ftimeLastWrite,
                           Globals.fdateMinimumLastWrite,
                           Globals.ftimeMinimumLastWrite) < 0 ) ||
         (Globals.bMinimumLastWrite == FALSE) )
    {
      Globals.fdateMinimumLastWrite = pFileInfo->fdateLastWrite;
      Globals.ftimeMinimumLastWrite = pFileInfo->ftimeLastWrite;
      Globals.bMinimumLastWrite = TRUE;

      strcpy (Globals.pszMinimumLastWrite,
              szFileNameBuffer);
    }
  }
}


/******************************************************************************
 * Name      : APIRET ProcessScan
 * Funktion  : Scan one directory and perform recursion if necessary
 * Parameter : PSZ pszPath      - the current path
 *             PANALYSE rtab - pointer to last recursions analysis data
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:48:00]
 ******************************************************************************/

APIRET ProcessScan (PSZ     pszPath,
                    ANALYSE *rtab)
{
  /* complete rewrite necessary - this routine is CRAP ! */

  APIRET          rc;
  HDIR            hDirectory = HDIR_CREATE;
  FILEFINDBUF3    *pFileInfo;
  PSZ             pFileFindBuffer;
  PSZ             pszPathNext;
  ULONG           ulFindCount;

  ANALYSE         tab;
  ANALYSE         dummytab;
  ANALYSE         subtab;

  ULONG           ulPathLength;

  pFileFindBuffer = (PSZ)malloc(Globals.ulFileFindBufferSize);         /* 4 x scanfiles */
  if (pFileFindBuffer == NULL)                         /* check if allocation succeeded */
    return(ERROR_NOT_ENOUGH_MEMORY);

  ulPathLength = strlen(pszPath);                     /* OK, get the origin path */
  if (ulPathLength > MAXPATHLEN)
    return(ERROR_FILENAME_EXCED_RANGE);       /* Bail out, subdir is too long */

  pszPathNext = (PSZ)malloc(MAXPATHLEN);          /* for copying the pathname */
  if (pszPathNext == NULL)                                 /* out of memory ? */
  {
    free (pFileFindBuffer);                         /* free previously allocated memory */
    return (ERROR_NOT_ENOUGH_MEMORY);                /* raise error condition */
  }

  memmove ((PSZ)pszPathNext,                          /* copy the path string */
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
  ulFindCount    = Options.ulFindNumberMaximum;
  memset (&tab,
          0,
          sizeof(ANALYSE));

  memmove (&dummytab,
           &tab,
           sizeof(ANALYSE));

  pFileInfo = (FILEFINDBUF3 *)pFileFindBuffer;

  rc = DosFindFirst(pszPathNext,
                    &hDirectory,
                    Globals.ulFileMask,
                    pFileInfo,
                    Globals.ulFileFindBufferSize,
                    &ulFindCount,
                    FIL_STANDARD);
  if ( (rc != NO_ERROR) &&                      /* check for error condition */
       (rc != ERROR_NO_MORE_FILES) )
    return (rc);                                        /* return error code */

  Globals.ulRecursionDepth++;                    /* increase recursion depth */

  do
  {
    Globals.ulFilesScanned += ulFindCount;

    while (ulFindCount)
    {
      if (Options.fsFiles ||
          Options.fsFiles2 ||
          Options.fsAttributes)
        scan_attributes (pszPath,                /* scan the file attributes */
                         pFileInfo);

     tab.llBytesSize      +=pFileInfo->cbFile;
     tab.llBytesAllocated +=pFileInfo->cbFileAlloc;
     tab.llFiles++;

     strcpy (pszPathNext,
             pszPath);

     ulPathLength = strlen(pszPathNext);         /* query the strings length */
                                                /* ignore trailing backslash */
     * ( (PUSHORT)(pszPathNext + ulPathLength) ) = 0x005c;              /* \ */

     strcat (pszPathNext,                                 /* append new name */
             pFileInfo->achName);


     if ((pFileInfo->achName[0] == '.') &&                /* ignore . and .. */
         (( !pFileInfo->achName[1] ||
         ((pFileInfo->achName[1] == '.') && !pFileInfo->achName[2])) ));
     else
     {
       if (pFileInfo->attrFile & FILE_DIRECTORY)
       {
         rc = ProcessScan(pszPathNext,
                          &dummytab);
         if (rc != NO_ERROR)
           ToolsErrorDosEx(rc,     /* and the operating system error message */
                           pszPathNext);
       }
     }
     ulFindCount--;
     pFileInfo = (FILEFINDBUF3 *) ((BYTE*)pFileInfo + pFileInfo->oNextEntryOffset);
    }

    ulFindCount = Options.ulFindNumberMaximum;
    pFileInfo = (FILEFINDBUF3 *)pFileFindBuffer;
    rc = DosFindNext (hDirectory,
                      pFileInfo,
                      Globals.ulFileFindBufferSize,
                      &ulFindCount);
  }
  while (rc == NO_ERROR);

  free((void *)pFileFindBuffer);
  free((void *)pszPathNext);

  Globals.ulRecursionDepth--;               /* here decrease recursion depth */

  /* Ergebnis an vorige Rekursion zurckliefern */
  /*memmove(rtab,&tab,sizeof(ANALYSE));*/
  rtab->llBytesSize     += tab.llBytesSize      + dummytab.llBytesSize;
  rtab->llBytesAllocated+= tab.llBytesAllocated + dummytab.llBytesAllocated;
  rtab->llFiles         += tab.llFiles          + dummytab.llFiles;

  if (Globals.ulRecursionDepth <= Options.ulRecursionDepthMaximum)
  {
    if (!Options.fsFiles)        /* not when file information is displayed ! */
      if (Options.fsIterate)
      {
        StrValueToSizeFloat (Globals.szBuf_1,
                             tab.llBytesSize + dummytab.llBytesSize);

        StrValueToSizeFloat (Globals.szBuf_2,
                             tab.llBytesAllocated + dummytab.llBytesAllocated);

        printf ("\n%7.0f %s %s %s",
                tab.llFiles + dummytab.llFiles,
                Globals.szBuf_1,
                Globals.szBuf_2,
                pszPath);
      }
      else
      {
        StrValueToSizeFloat (Globals.szBuf_1,
                             tab.llBytesSize);

        StrValueToSizeFloat (Globals.szBuf_2,
                             tab.llBytesAllocated);

        printf ("\n%7.0f %s %s %s",
                tab.llFiles,
                Globals.szBuf_1,
                Globals.szBuf_2,
                pszPath);
      }
  }


  return(DosFindClose(hDirectory));
}


/******************************************************************************
 * Name      : APIRET ProcessScanSpeed
 * Funktion  : Scan directories ad max. speed
 * Parameter : PSZ pszPath      - the current path
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:48:00]
 ******************************************************************************/

APIRET ProcessScanSpeed (PSZ pszPath)
{
  APIRET          rc;
  HDIR            hDirectory = HDIR_CREATE;
  FILEFINDBUF3    *pFileInfo;
  PSZ             pFileFindBuffer;
  PSZ             pszPathNext;
  ULONG           ulFindCount;
  ULONG           ulPathLength;

  pFileFindBuffer = (PSZ)malloc(Globals.ulFileFindBufferSize);         /* 4 x scanfiles */
  if (pFileFindBuffer == NULL)                         /* check if allocation succeeded */
    return(ERROR_NOT_ENOUGH_MEMORY);

  ulPathLength = strlen(pszPath);                     /* OK, get the origin path */

  pszPathNext = (PSZ)malloc(MAXPATHLEN);          /* for copying the pathname */
  if (pszPathNext == NULL)                                 /* out of memory ? */
  {
    free (pFileFindBuffer);                         /* free previously allocated memory */
    return (ERROR_NOT_ENOUGH_MEMORY);                /* raise error condition */
  }

  memmove ((PSZ)pszPathNext,                          /* copy the path string */
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
  ulFindCount    = Options.ulFindNumberMaximum;

  pFileInfo = (FILEFINDBUF3 *)pFileFindBuffer;

  rc = DosFindFirst(pszPathNext,
                    &hDirectory,
                    Globals.ulFileMask,
                    pFileInfo,
                    Globals.ulFileFindBufferSize,
                    &ulFindCount,
                    FIL_STANDARD);
  if ( (rc != NO_ERROR) &&                      /* check for error condition */
       (rc != ERROR_NO_MORE_FILES) )
    return (rc);                                        /* return error code */

  do
  {
    Globals.ulFilesScanned += ulFindCount;

    while (ulFindCount)
    {
      if (pFileInfo->attrFile & FILE_DIRECTORY)   /* examine directories only */
      {
       if ((pFileInfo->achName[0] == '.') &&               /* ignore . and .. */
           (( !pFileInfo->achName[1] ||
           ((pFileInfo->achName[1] == '.') && !pFileInfo->achName[2])) ));
       else
       {
         strcpy (pszPathNext,
                 pszPath);

         ulPathLength = strlen(pszPathNext);     /* query the strings length */
                                                /* ignore trailing backslash */
         * ( (PUSHORT)(pszPathNext + ulPathLength) ) = 0x005c;          /* \ */

         strcat (pszPathNext,                             /* append new name */
                 pFileInfo->achName);

         ProcessScanSpeed(pszPathNext);                         /* recursion */
                           /* PH: no-one is interested in the returncode ... */
        }
      }

      ulFindCount--;
      pFileInfo = (FILEFINDBUF3 *) ((BYTE*)pFileInfo + pFileInfo->oNextEntryOffset);
    }

    ulFindCount = Options.ulFindNumberMaximum;
    pFileInfo = (FILEFINDBUF3 *)pFileFindBuffer;
    rc = DosFindNext (hDirectory,
                      pFileInfo,
                      Globals.ulFileFindBufferSize,
                      &ulFindCount);
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



/*****************************************************************************
 * Name      : int main
 * Funktion  : main routine
 * Parameter : int argc, PSZargv[]
 * Variablen :
 * Ergebnis  : APIRET rc
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Mittwoch, 11.05.1994 14.04.33]
 *****************************************************************************/

int main (int argc,
          PSZ argv[])
{
  PERFSTRUCT TS_Start;                       /* time information about start */
  PERFSTRUCT TS_End;                        /* time information about finish */
  float      seconds;                                     /* elapsed seconds */
  ANALYSE    total;
  APIRET     rc;                                           /* API-Returncode */
  ULONG      ulCounter;                                      /* loop counter */
  DATETIME   dtDateTime;          /* structure to hold current date and time */


  /****************************************************************************
   * Initialize Globals                                                       *
   ****************************************************************************/

  memset (&total,                                /* reset variable structure */
          0,
          sizeof(ANALYSE));

  memset (&Globals,                              /* reset variable structure */
          0,
          sizeof(GLOBALS));

  Globals.ulFileMask = FILE_NORMAL    |
                       FILE_DIRECTORY |
                       FILE_SYSTEM    |
                       FILE_READONLY  |
                       FILE_HIDDEN;

  Globals.fTotalAgeCreation   = 0.0;
  Globals.fTotalAgeLastWrite  = 0.0;
  Globals.fTotalAgeLastAccess = 0.0;
  Globals.fTotalAgeCurrent    =

  DosGetDateTime(&dtDateTime);                          /* query system time */

  Globals.fTotalAgeCurrent = ToolsDateToAge(dtDateTime.day,  /* transform it */
                                            dtDateTime.month,
                                            dtDateTime.year);

  Globals.fTotalCompression   = 0.0;

  Options.ulRecursionDepthMaximum = -1;                           /* maximum */




  /***************************************************************************
   * Parse arguments                                                         *
   ***************************************************************************/

  rc = ArgStandard (argc,                            /* CLI-Parameter parsen */
                    argv,
                    TabArguments,
                    &Options.fsHelp);
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    exit(1);                                                /* abort program */
  }

  if (Options.fsHelp)                        /* check if user specified help */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }


  /***************************************************************************
   * Map arguments                                                           *
   ***************************************************************************/

  if (!Options.fsPath)               /* user specified no path, take current */
    strcpy (Globals.DUPathStr,".");                      /* current, default */
  else
    strcpy (Globals.DUPathStr,Options.pszPath);                 /* user path */


  if (!Options.fsFind)           /* user did not specify maximum find number */
    Options.ulFindNumberMaximum = 65535 / sizeof(FILEFINDBUF3);
  else
    Options.ulFindNumberMaximum = 1;        /* for some Windows 95 requester */
 /* OS/2 2.x, 3.x provide by far best performance when avoiding 64k switches */

  Globals.ulFileFindBufferSize = sizeof(FILEFINDBUF3) * /* our target struct */
                                 Options.ulFindNumberMaximum;

  if (!Options.fsSpeedNr)                 /* if user only wants to scan once */
    Options.ulSpeedNr = 1;                            /* this is the default */

  if (Options.fsFiles2)        /* whenever user wants more file information, */
    Options.fsFiles = TRUE;                      /* also map the basic level */


  /****************************************************************************
   * Processing ...                                                           *
   ****************************************************************************/

  ProcessRootPath(Globals.DUPathStr);       /* get fully qualified path name */

  printf ("\nScanning [%s]",
          Globals.DUPathStr);

  if (Options.fsSpeed)                             /* measure scanning speed */
  {
    printf (" (measuring scanning speed %ux)",
           Options.ulSpeedNr);


    ToolsPerfQuery (&TS_Start);                    /* exact time measurement */

    for (ulCounter = 1;
         ulCounter <= Options.ulSpeedNr;
         ulCounter++)
      ProcessScanSpeed (Globals.DUPathStr);

    ToolsPerfQuery (&TS_End);                      /* exact time measurement */
    seconds = TS_End.fSeconds - TS_Start.fSeconds;     /* calculate duration */

  }
  else
  {
    if (Options.fsIterate)                               /* iteration mode ? */
      printf (" (iterative)");


    ToolsPerfQuery (&TS_Start);                    /* exact time measurement */

    /* ... attributes ... */
    rc = ProcessScan (Globals.DUPathStr,
                      &total);
    if (rc != NO_ERROR)                                  /* check for errors */
      ToolsErrorDosEx(rc,          /* and the operating system error message */
                      Globals.DUPathStr);

    ToolsPerfQuery (&TS_End);                      /* exact time measurement */
    seconds = TS_End.fSeconds - TS_Start.fSeconds;     /* calculate duration */


    StrValueToSizeFloat (Globals.szBuf_1,
                    total.llBytesSize);
    StrValueToSizeFloat (Globals.szBuf_2,
                    total.llBytesAllocated);

    printf ("\nTotal       : [%7.0f files, %s filesize, %s allocated]",
            total.llFiles,
            Globals.szBuf_1,
            Globals.szBuf_2);


    if (Options.fsAttributes)                        /* Attribute anzeigen ? */
    {
      printf ("\nStatistics"
              "\nþ Attributes");

      if (Globals.attrNormal)
        printf ("\n  ù Files without attributes: [%8u]",
                Globals.attrNormal);

      if (Globals.attrArchived)
        printf ("\n  ù Files archived:           [%8u]",
                Globals.attrArchived);

      if (Globals.attrReadonly)
        printf ("\n  ù Files read-only:          [%8u]",
                Globals.attrReadonly);

      if (Globals.attrHidden)
        printf ("\n  ù Files hidden:             [%8u]",
                Globals.attrHidden);

      if (Globals.attrSystem)
        printf ("\n  ù Files system:             [%8u]",
                Globals.attrSystem);

      if (Globals.attrCompressed)
        printf ("\n  ù Files compressed:         [%8u]",
                Globals.attrCompressed);

      if (Globals.attrDirectory)
      {
        printf ("\n  ù Directories:              [%8u]"
                "\n    ú '.' and '..' included:  [%8u]"
                "\n    ú remaining rest:         [%8u]",
                Globals.attrDirectory,
                Globals.attrDirpseudo,
                Globals.attrDirectory - Globals.attrDirpseudo);
      }

      if (Globals.attrUndefined)
        printf ("\n  ù WARNING: files with undefined attributes: [%8u]",
                Globals.attrUndefined);

      /***********************************************************************
       * Averages                                                            *
       ***********************************************************************/

      if (total.llFiles != 0)
      {
        printf ("\nþ Averages"
                "\n  ù File Ages and Dates"
                "\n    ú Average file creation          %6.1f days ago"
                "\n    ú Average last file access       %6.1f days ago"
                "\n    ú Average last file modification %6.1f days ago",
                -Globals.fTotalAgeCreation     / (float)total.llFiles,
                -Globals.fTotalAgeLastAccess   / (float)total.llFiles,
                -Globals.fTotalAgeLastWrite    / (float)total.llFiles);

        printf ("\n  ù File Size and Allocation"
                "\n    ú Average file size       %10.0fb"
                "\n    ú Average allocation size %10.0fb"
                "\n    ú Average slack space     %10.0fb",
                (float)total.llBytesSize      / (float)total.llFiles,
                (float)total.llBytesAllocated / (float)total.llFiles,
                ( (float)total.llBytesAllocated - (float)total.llBytesSize)
                  / (float)total.llFiles);

        if (Globals.attrCompressed != 0)
          printf ("\n  ù Compression"
                  "\n    ú Average file compression (compressed only) 1:%2.2f"
                  "\n    ú Average file compression (total)           1:%2.2f",
                  Globals.fTotalCompression     / (float)Globals.attrCompressed,
                  Globals.fTotalCompression     / (float)total.llFiles);
      }


      /***********************************************************************
       * Superlatives, Records, Minima and Maxima                            *
       ***********************************************************************/

      printf ("\nþ Superlatives");

      /***************
       * Compression *
       ***************/

      if (Globals.bMaximumCompression || Globals.bMinimumCompression)
        printf ("\n  ù Compression");

      if (Globals.bMaximumCompression)
        printf ("\n    ú Maximum Ratio  1:%2.2f %s",
                Globals.fMaximumCompression,
                Globals.pszMaximumCompression);

      if (Globals.bMinimumCompression)
        printf ("\n    ú Minimum Ratio   1:%2.2f %s",
                Globals.fMinimumCompression,
                Globals.pszMinimumCompression);


      /*****************************
       * Files size and allocation *
       *****************************/

      printf ("\n  ù File Size and Allocation");

      if (Globals.bMaximumFileSize)
        printf ("\n    ú Maximum File Size        %9ub %s",
                Globals.ulMaximumFileSize,
                Globals.pszMaximumFileSize);

      if (Globals.bMaximumAllocationSize)
        printf ("\n    ú Maximum Allocation Size  %9ub %s",
                Globals.ulMaximumAllocationSize,
                Globals.pszMaximumAllocationSize);

      if (Globals.bMaximumSlackSpace)
        printf ("\n    ú Maximum Slack Space      %9ib %s",
                Globals.iMaximumSlackSpace,
                Globals.pszMaximumSlackSpace);

      if (Globals.bMinimumFileSize)
        printf ("\n    ú Minimum File Size        %9ub %s",
                Globals.ulMinimumFileSize,
                Globals.pszMinimumFileSize);

      if (Globals.bMinimumAllocationSize)
        printf ("\n    ú Minimum Allocation Size  %9ub %s",
                Globals.ulMinimumAllocationSize,
                Globals.pszMinimumAllocationSize);

      if (Globals.bMaximumSlackSpace)
        printf ("\n    ú Minimum Slack Space      %9ib %s",
                Globals.iMinimumSlackSpace,
                Globals.pszMinimumSlackSpace);


      /************************
       * Files ages and dates *
       ************************/

      printf ("\n  ù File Ages and Dates");

      if (Globals.bMaximumCreation)
      {
        StrFDateTimeToString(Globals.fdateMaximumCreation,   /* map the date */
                             Globals.ftimeMaximumCreation,
                             Globals.szFileDate);

        printf ("\n    ú Most recently created   %s %s",
                Globals.szFileDate,
                Globals.pszMaximumCreation);
      }

      if (Globals.bMaximumLastAccess)
      {
        StrFDateTimeToString(Globals.fdateMaximumLastAccess, /* map the date */
                             Globals.ftimeMaximumLastAccess,
                             Globals.szFileDate);

        printf ("\n    ú Most recently accessed  %s %s",
                Globals.szFileDate,
                Globals.pszMaximumLastAccess);
      }

      if (Globals.bMaximumLastWrite)
      {
        StrFDateTimeToString(Globals.fdateMaximumLastWrite,  /* map the date */
                             Globals.ftimeMaximumLastWrite,
                             Globals.szFileDate);

        printf ("\n    ú Most recently modified  %s %s",
                Globals.szFileDate,
                Globals.pszMaximumLastWrite);
      }

      if (Globals.bMinimumCreation)
      {
        StrFDateTimeToString(Globals.fdateMinimumCreation,   /* map the date */
                             Globals.ftimeMinimumCreation,
                             Globals.szFileDate);

        printf ("\n    ú Least recently created  %s %s",
                Globals.szFileDate,
                Globals.pszMinimumCreation);
      }

      if (Globals.bMinimumLastAccess)
      {
        StrFDateTimeToString(Globals.fdateMinimumLastAccess, /* map the date */
                             Globals.ftimeMinimumLastAccess,
                             Globals.szFileDate);

        printf ("\n    ú Least recently accessed %s %s",
                Globals.szFileDate,
                Globals.pszMinimumLastAccess);
      }

      if (Globals.bMinimumLastWrite)
      {
        StrFDateTimeToString(Globals.fdateMinimumLastWrite,  /* map the date */
                             Globals.ftimeMinimumLastWrite,
                             Globals.szFileDate);

        printf ("\n    ú Least recently modified %s %s",
                Globals.szFileDate,
                Globals.pszMinimumLastWrite);
      }

    }
  }

  if (seconds)
    printf ("\n[%8u] files, [%9.4f] sec, [%9.3f] files/sec\n",
            Globals.ulFilesScanned,
            seconds,
            ((float)Globals.ulFilesScanned / seconds));

  return NO_ERROR;
}
