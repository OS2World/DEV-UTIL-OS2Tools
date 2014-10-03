/*****************************************************
 * Map Filename Extensions                           *
 * Renames / moves files according to their content. *
 * (c) 2000    Patrick Haller Systemtechnik          *
 *****************************************************/

/* #define DEBUG 1 */


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#ifdef __OS2__
  #define INCL_DOSMISC
  #define INCL_DOSFILEMGR
  #define INCL_BASE
  #define INCL_NOPMAPI
  #include <os2.h>
#endif

#include <stdio.h>
#include <direct.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <process.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"


/*****************************************************************************
 * Defines                                                                   *
 *****************************************************************************/

//#define DEBUG 1

#ifndef MAX_PATH_LEN
#define MAX_PATH_LEN 260
#endif


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsVerbose;                               /* provide verbose output */
  ARGFLAG fsRenameLong;                      /* rename files to long version */
  ARGFLAG fsPath;                      /* root path to start processing from */

  PSZ     pszPath;
} OPTIONS, *POPTIONS;


typedef struct
{
  ULONG ulRenamed;                                /* number of renamed files */
} GLOBALS, *PGLOBALS;


/* format check table definitions and macros */
typedef int (*PFNFORMATCHECK)(ULONG flags, PVOID pData, ULONG ulLength, PSZ pszRetFilename);

typedef struct tagCheckTableEntry
{
  PFNFORMATCHECK pfnCheck;                  /* pointer to the check function */
  PSZ pszName;                          /* name / description of this format */
  PSZ pszRenameShort;                               /* mask for short rename */
  PSZ pszRenameLong;                                 /* mask for long rename */
  PSZ pszSubdirectory;                    /* name for subdirectory to create */
} CHECKTABLEENTRY, *PCHECKTABLEENTRY;


#define FORMATCHECK(a) int a(ULONG flags, PVOID pData, ULONG ulLength, PSZ pszRetFilename)
FORMATCHECK(fnARCH);
FORMATCHECK(fnARJ);
FORMATCHECK(fnBITMAP);
FORMATCHECK(fnBZ2);
FORMATCHECK(fnCLASS);
FORMATCHECK(fnCCPP);
FORMATCHECK(fnDLL);
FORMATCHECK(fnEXE);
FORMATCHECK(fnGIF);
FORMATCHECK(fnICON);
FORMATCHECK(fnJAR);
FORMATCHECK(fnJPEG);
FORMATCHECK(fnLZH);
FORMATCHECK(fnMID);
FORMATCHECK(fnMP3);
FORMATCHECK(fnRAR);
FORMATCHECK(fnWAV);
FORMATCHECK(fnZIP);

static CHECKTABLEENTRY cteDefault =
  {NULL,     "Default",       "*",     "*",       "Default"};

static CHECKTABLEENTRY tabCheckTable[] =
{
  {fnARCH,   "Elf-Lib,Arch ?","*.ACH", "*.ARCH",  "ARCH"},
  {fnARJ,    "ARJ",           "*.ARJ", "*.ARJ",   "ARJ"},
  {fnBITMAP, "Bitmap",        "*.BMP", "*.BMP",   "Bitmap"},
  {fnBZ2,    "BZip",          "*.BZ2", "*.BZ2",   "BZ2"},
  {fnCLASS,  "Java Class",    "*.cla", "*.class", "Class"},
  {fnCCPP,   "C/C++/H/Rexx",  "*.C",   "*.C",     "C"},
  {fnDLL,    "DLL",           "*.DLL", "*.DLL",   "DLL"},
  {fnEXE,    "EXE",           "*.EXE", "*.EXE",   "EXE"},
  {fnGIF,    "GIF87/89",      "*.GIF", "*.GIF",   "GIF"},
  {fnICON,   "Icon",          "*.ICO", "*.ICO",   "Icon"},
  {fnJAR,    "JAR",           "*.JAR", "*.JAR",   "JAR"},
  {fnJPEG,   "JPEG/JFIF",     "*.JPG", "*.JPEG",  "JPEG"},
  {fnLZH,    "LHarc",         "*.LZH", "*.Lharc", "LZH"},
  {fnMID,    "MIDI",          "*.MID", "*.MIDI",  "MIDI"},
  {fnMP3,    "MPEG2 Layer 3", "*.MP3", "*.MPEG3", "MPEG3"},
  {fnRAR,    "RAR",           "*.RAR", "*.RAR",   "RAR"},
  {fnWAV,    "Wave/Riff",     "*.WAV", "*.WAVE",  "Wave"},
  {fnZIP,    "ZIP",           "*.ZIP", "*.ZIP",   "ZIP"},

    /*
     - extract real filename from file (ID3, etc.) ?
     */

  {0} // table termination
};


/* format detection constants */
#define FD_ERROR_NOT_ENOUGH_BUFFER -2
#define FD_ERROR_GENERAL           -1
#define FD_NOT_DETECTED             0
#define FD_NO_OPERATION             0
#define FD_UNSURE                   1
#define FD_DETECTED                 2


/* format detection control flags */
#define FDF_DETECT                 0x01
#define FDF_QUERY_FILENAME         0x02


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */
GLOBALS Globals;                                         /* global variables */

ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung-----------------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",          NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",          NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/V",         "Verbose output.",           NULL,                 ARG_NULL,       &Options.fsVerbose},
  {"/L",         "Rename to long extension.", NULL,                 ARG_NULL,       &Options.fsRenameLong},
  {"1",          "Path to process.",          &Options.pszPath,     ARG_PSZ |
                                                                    ARG_DEFAULT,    &Options.fsPath},
  ARG_TERMINATE
};


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void   help                (void);

VOID   Initialize          (VOID);

int    main                (int,
                            char **);


/***********************************************************************
 * Name      : all format check functions
 *             int fcXXX (ULONG flags, PVOID pData, ULONG ulLength, PSZ pszRetFilename)
 * Funktion  : Detect the format of the given file buffer
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Tuesday, 2000/09/05]
 ***********************************************************************/

FORMATCHECK(fnARCH)
{
  static BYTE sigARCH[] = {0x21, 0x3C, 0x61, 0x72,   // !<arch>
                           0x63, 0x68, 0x3E, 0x0A};

  if (flags & FDF_DETECT)
  {
    // ensure data length is sufficient
    if (ulLength < sizeof(sigARCH))
      return FD_ERROR_NOT_ENOUGH_BUFFER;

    // compare signatures
    if (memcmp(sigARCH,
               pData,
               sizeof(sigARCH)) == 0)
      return FD_DETECTED;
    else
      return FD_NOT_DETECTED;
  }
  //else
  //  if (flags & FDF_QUERY_FILENAME)
  //    return FD_NO_OPERATION

  return FD_NOT_DETECTED;
}


FORMATCHECK(fnARJ)   { return FD_NOT_DETECTED; }


FORMATCHECK(fnBITMAP)
{
  static BYTE sigBMP[] = {'B','M'};

  if (flags & FDF_DETECT)
  {
    // ensure data length is sufficient
    if (ulLength < sizeof(sigBMP))
      return FD_ERROR_NOT_ENOUGH_BUFFER;

    // compare signatures
    if (memcmp(sigBMP,
               pData,
               sizeof(sigBMP)) == 0)
      return FD_DETECTED;
    else
      return FD_NOT_DETECTED;
  }
  //else
  //  if (flags & FDF_QUERY_FILENAME)
  //    return FD_NO_OPERATION

  return FD_NOT_DETECTED;
}


FORMATCHECK(fnBZ2)
{
  static BYTE sigBZ2[] = {'B','Z'};

  if (flags & FDF_DETECT)
  {
    // ensure data length is sufficient
    if (ulLength < sizeof(sigBZ2))
      return FD_ERROR_NOT_ENOUGH_BUFFER;

    // compare signatures
    if (memcmp(sigBZ2,
               pData,
               sizeof(sigBZ2)) == 0)
      return FD_DETECTED;
    else
      return FD_NOT_DETECTED;
  }
  //else
  //  if (flags & FDF_QUERY_FILENAME)
  //    return FD_NO_OPERATION

  return FD_NOT_DETECTED;
}



FORMATCHECK(fnCLASS)
{
  static BYTE sigCLASS[] = {0xCA,0xFE,0xBA,0xBE};

  if (flags & FDF_DETECT)
  {
    // ensure data length is sufficient
    if (ulLength < sizeof(sigCLASS))
      return FD_ERROR_NOT_ENOUGH_BUFFER;

    // compare signatures
    if (memcmp(sigCLASS,
               pData,
               sizeof(sigCLASS)) == 0)
      return FD_DETECTED;
    else
      return FD_NOT_DETECTED;
  }
  //else
  //  if (flags & FDF_QUERY_FILENAME)
  //    return FD_NO_OPERATION

  return FD_NOT_DETECTED;
}

FORMATCHECK(fnCCPP)
{
  static BYTE sigC[] = {'/','*'};

  if (flags & FDF_DETECT)
  {
    // ensure data length is sufficient
    if (ulLength < sizeof(sigC))
      return FD_ERROR_NOT_ENOUGH_BUFFER;

    // compare signatures
    if (memcmp(sigC,
               pData,
               sizeof(sigC)) == 0)
      return FD_DETECTED;
    else
      return FD_NOT_DETECTED;
  }
  //else
  //  if (flags & FDF_QUERY_FILENAME)
  //    return FD_NO_OPERATION

  return FD_NOT_DETECTED;
}


FORMATCHECK(fnDLL)   { return FD_NOT_DETECTED; }
FORMATCHECK(fnEXE)
{
  static BYTE sigEXE[] = {'M','Z'};

  if (flags & FDF_DETECT)
  {
    // ensure data length is sufficient
    if (ulLength < sizeof(sigEXE))
      return FD_ERROR_NOT_ENOUGH_BUFFER;

    // compare signatures
    if (memcmp(sigEXE,
               pData,
               sizeof(sigEXE)) == 0)
      return FD_DETECTED;
    else
      return FD_NOT_DETECTED;
  }
  //else
  //  if (flags & FDF_QUERY_FILENAME)
  //    return FD_NO_OPERATION

  return FD_NOT_DETECTED;
}

FORMATCHECK(fnGIF)
{
  // GIF87a, GIF89a
  static BYTE sigGIF[] = {'G','I','F','8'};

  if (flags & FDF_DETECT)
  {
    // ensure data length is sufficient
    if (ulLength < sizeof(sigGIF))
      return FD_ERROR_NOT_ENOUGH_BUFFER;

    // compare signatures
    if (memcmp(sigGIF,
               pData,
               sizeof(sigGIF)) == 0)
      return FD_DETECTED;
    else
      return FD_NOT_DETECTED;
  }
  //else
  //  if (flags & FDF_QUERY_FILENAME)
  //    return FD_NO_OPERATION

  return FD_NOT_DETECTED;
}


FORMATCHECK(fnICON)
{
  static BYTE sigICON[] = {'B','A'};

  if (flags & FDF_DETECT)
  {
    // ensure data length is sufficient
    if (ulLength < sizeof(sigICON))
      return FD_ERROR_NOT_ENOUGH_BUFFER;

    // compare signatures
    if (memcmp(sigICON,
               pData,
               sizeof(sigICON)) == 0)
      return FD_DETECTED;
    else
      return FD_NOT_DETECTED;
  }
  //else
  //  if (flags & FDF_QUERY_FILENAME)
  //    return FD_NO_OPERATION

  return FD_NOT_DETECTED;
}


FORMATCHECK(fnJAR)   { return FD_NOT_DETECTED; }

FORMATCHECK(fnJPEG)
{
  static BYTE sigJFIF[] = {0xFF,0xD8,0xFF,0xE0,0x00,0x10,'J','F','I','F'};

  if (flags & FDF_DETECT)
  {
    // ensure data length is sufficient
    if (ulLength < sizeof(sigJFIF))
      return FD_ERROR_NOT_ENOUGH_BUFFER;

    // compare signatures
    if (memcmp(sigJFIF,
               pData,
               sizeof(sigJFIF)) == 0)
      return FD_DETECTED;
    else
      return FD_NOT_DETECTED;
  }
  //else
  //  if (flags & FDF_QUERY_FILENAME)
  //    return FD_NO_OPERATION

  return FD_NOT_DETECTED;
}

FORMATCHECK(fnLZH)   { return FD_NOT_DETECTED; }
FORMATCHECK(fnMID)   { return FD_NOT_DETECTED; }
FORMATCHECK(fnMP3)   { return FD_NOT_DETECTED; }

FORMATCHECK(fnRAR)
{
  static BYTE sigRAR[] = {'R','a','r','!'};

  if (flags & FDF_DETECT)
  {
    // ensure data length is sufficient
    if (ulLength < sizeof(sigRAR))
      return FD_ERROR_NOT_ENOUGH_BUFFER;

    // compare signatures
    if (memcmp(sigRAR,
               pData,
               sizeof(sigRAR)) == 0)
      return FD_DETECTED;
    else
      return FD_NOT_DETECTED;
  }
  //else
  //  if (flags & FDF_QUERY_FILENAME)
  //    return FD_NO_OPERATION

  return FD_NOT_DETECTED;
}


FORMATCHECK(fnWAV)   { return FD_NOT_DETECTED; }

FORMATCHECK(fnZIP)
{
  // PK $03 $04
  static BYTE sigZIP[] = {'P','K',0x03,0x04};

  if (flags & FDF_DETECT)
  {
    // ensure data length is sufficient
    if (ulLength < sizeof(sigZIP))
      return FD_ERROR_NOT_ENOUGH_BUFFER;

    // compare signatures
    if (memcmp(sigZIP,
               pData,
               sizeof(sigZIP)) == 0)
      return FD_DETECTED;
    else
      return FD_NOT_DETECTED;
  }
  //else
  //  if (flags & FDF_QUERY_FILENAME)
  //    return FD_NO_OPERATION

  return FD_NOT_DETECTED;
}


/***********************************************************************
 * Name      : detectionPass
 * Funktion  : do a complete detection pass
 * Parameter : PVOID pData, ULONG ulDataLength, PSZ pszRetFilename
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Tuesday, 2000/09/05]
 ***********************************************************************/

PCHECKTABLEENTRY detectionPass(HFILE hFile,
                               PVOID pData,
                               ULONG ulDataLength,
                               PSZ   pszRetFilename)
{
  int  rc; // return code from subsequent function calls
  BOOL flagRescanUnsure   = FALSE; // do we have to do a complete rescan?
  BOOL flagRescanComplete = FALSE; // do we have to do a complete rescan?
  BOOL flagFound          = FALSE;

  // start table scan
  PCHECKTABLEENTRY pcteFormat;

  // iterate over all known functions
  for (pcteFormat = tabCheckTable;
       pcteFormat->pfnCheck != NULL;
       pcteFormat++)
  {
    // call detection function
    rc = pcteFormat->pfnCheck(FDF_DETECT,
                             pData,
                             ulDataLength,
                             NULL);

    // examine return code
    switch (rc)
    {
      case FD_ERROR_NOT_ENOUGH_BUFFER:
        // Hmm, insufficient data for the scan
        flagRescanComplete = TRUE;
        break;

      case FD_ERROR_GENERAL:
        // @@@PH: some undetermined problem
        break;

      case FD_NOT_DETECTED:
        // format not detected, try next function
        break;

      case FD_UNSURE:
        // Hmm, unsure detection. We can wait for better results from
        // other functions or do a rescan with more buffer data.
        flagRescanUnsure = TRUE;
        break;

      case FD_DETECTED:
        // OK, we've found it! No further passes are necessary.
        flagFound = TRUE;
        break;
    }

    if (flagFound == TRUE) // abort loop immediately
      break;
  }

  // do a 2nd scan as we only had an unsure result before
  if ( (flagFound == FALSE) &&
       (flagRescanUnsure == TRUE) )
  {
    // iterate over all known functions
    for (pcteFormat = tabCheckTable;
         pcteFormat->pfnCheck != NULL;
         pcteFormat++)
    {
      // call detection function
      rc = pcteFormat->pfnCheck(FDF_DETECT,
                               pData,
                               ulDataLength,
                               NULL);

      // examine return code
      switch (rc)
      {
        case FD_UNSURE:
        case FD_DETECTED:
          // As this is the 2nd pass, we take this as 2nd best result ...
          // OK, we've found it! No further passes are necessary.
          flagFound = TRUE;
          break;
      }

      if (flagFound == TRUE) // abort loop immediately
        break;
    }
  }

  // if we still have no result, we could do a more thorough scan
  // with the full file buffer.
  // @@@PH To Do

  // do final processing
  if (flagFound)
  {
    pcteFormat->pfnCheck(FDF_QUERY_FILENAME,
                         pData,
                         ulDataLength,
                         pszRetFilename);

    return pcteFormat; // OK, done
  }
  else
  {
    *pszRetFilename = '\0';
    return NULL; // no format found!
  }
}


/***********************************************************************
 * Name      : process file
 * Funktion  : process a single file
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Tuesday, 2000/09/05]
 ***********************************************************************/

APIRET ProcessFile(PSZ pszFilename)
{
  HFILE  hFile;                            /* handle to the data file  */
  APIRET rc;                               /* API return code          */
  ULONG  ulAction;                         /* action code from DosOpen */
  PCHECKTABLEENTRY pcteFormat;
  CHAR   szRetFilename[MAX_PATH_LEN];
  BYTE   arrBuffer[512];               /* static buffer for first read */
  ULONG  ulBytesRead;               /* number of bytes read by DosRead */
  PSZ    pszTemp1;
  CHAR   szFileNameBuffer[MAX_PATH_LEN];
  PSZ    pszEdit; // the editing string
  PSZ    pszFileNameOnly;

  // 1. build complete filename and pathname
  // 2. extract filename part only
  // 3. open file
  rc = DosOpen(pszFilename,
               &hFile,
               &ulAction,
               0L,                                               /* Filesize */
               0L,                                        /* File attributes */
               OPEN_ACTION_FAIL_IF_NEW |
               OPEN_ACTION_OPEN_IF_EXISTS,
               OPEN_SHARE_DENYNONE |
               OPEN_ACCESS_READONLY |
               OPEN_FLAGS_SEQUENTIAL,
               NULL);
  if (rc)                                                /* check for errors */
  {
    ToolsErrorDos(rc);                                /* print error message */
    return (rc);                                           /* abort function */
  }


  // 4. read first buffer (512b)
  rc = DosRead(hFile,                                           /* read file */
               arrBuffer,
               sizeof(arrBuffer),
               &ulBytesRead);
  if (rc)                                                /* check for errors */
  {
    DosClose(hFile);
    ToolsErrorDos(rc);                                /* print error message */
    return rc; /* abort function */
  }


  /* 5. do scan pass #1 (short buffer */
  szRetFilename[0] = 0; /* reset the buffer */
  pcteFormat = detectionPass(hFile,                   /* do detection pass */
                             arrBuffer,
                             ulBytesRead,
                             szRetFilename);

  // 5a. read long buffer / complete
  // 5b. do scan pass #2 (long buffer

  // 6. close file
  rc = DosClose(hFile);

  // OK, we've for a check table entry now, hopefully
  if (pcteFormat == NULL)
    pcteFormat = &cteDefault; // in this case, use the default!

  // --------------------------------------------------------------------
  // now do the real work!

  // just extract the filename part
  pszTemp1 = strrchr(pszFilename, '\\');
  if (pszTemp1 == NULL) pszTemp1 = strrchr(pszFilename, '/');
  if (pszTemp1 == NULL)
    return ERROR_INVALID_PATH; // Hmm, we shouldn't get here !

  // any suggested filename ?
  if (szRetFilename[0] == 0)
    pszFileNameOnly = pszTemp1+1;
  else
    // use the suggested filename from the format detectors
    pszFileNameOnly = szRetFilename;

  if (Options.fsRenameLong)
    pszEdit = pcteFormat->pszRenameLong;
  else
    pszEdit = pcteFormat->pszRenameShort;

#ifdef DEBUG
  fprintf(stderr, "A: %s, %s, %s\n",
          pszFilename,
          pszFileNameOnly,
          pszEdit);
#endif

  rc = DosEditName(1,                /* use OS/2 1.2 editing semantics */
                   pszFileNameOnly,                   /* source string */
                   pszEdit,                          /* editing string */
                   szFileNameBuffer,              /* local name buffer */
                   sizeof (szFileNameBuffer));        /* buffer length */
  if (rc != NO_ERROR)                              /* check for errors */
    return rc;                                /* raise error condition */

  if (Options.fsVerbose)
    printf("%s: %s -> %s\n",
           pszFilename,
           pcteFormat->pszName,
           szFileNameBuffer);

  if (stricmp(pszFileNameOnly,        /* check if filename has changed */
              szFileNameBuffer) != 0)
  {
    // build full new name for the move operation
    // reuse that buffer
    strcpy(szRetFilename,
           pszFilename);
    strcpy(szRetFilename + (ULONG)pszTemp1 - (ULONG)pszFilename + 1,
           szFileNameBuffer);

#ifdef DEBUG
  fprintf(stderr, "B: %s - %s\n",
          pszFilename,
          szRetFilename);
#endif


    // OK, we got to rename that file
    rc = DosMove(pszFilename, szRetFilename);
    if (rc)
      return rc;

    Globals.ulRenamed++; // adjust statistics
  }


  // - rename short / rename long
  // - move (with automatic creation of subdirectory)


  return NO_ERROR; // OK, done
}


/******************************************************************************
 * Name      : APIRET ProcessDirectory
 * Funktion  : Flat scan of given directory
 * Parameter : PSZ pszPath      - the current path
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:48:00]
 ******************************************************************************/

#define FINDBUFFERSIZE 16384
#define FILEFINDCOUNT  1024

APIRET ProcessDirectory (PSZ pszPath,
                         PSZ pszWildcard)
{
  APIRET          rc;
  HDIR            hDirectory = HDIR_CREATE;
  FILEFINDBUF3    *pFileInfo;
  PSZ             pFileFindBuffer;
  PSZ             pszPathNext;
  PSZ             pszTemp;                       /* temporary string pointer */
  ULONG           ulFindCount;
  ULONG           ulPathLength;

#ifdef DEBUG
  fprintf (stderr,
           "\nProcessScan(%s,%s)",
           pszPath,
           pszWildcard);
#endif

  pFileFindBuffer = (PSZ)malloc(FINDBUFFERSIZE);
  if (pFileFindBuffer == NULL)              /* check if allocation succeeded */
    return(ERROR_NOT_ENOUGH_MEMORY);

  ulPathLength = strlen(pszPath);                 /* OK, get the origin path */
  if (ulPathLength > MAX_PATH_LEN)
  {
    free (pFileFindBuffer);              /* free previously allocated memory */
    return(ERROR_FILENAME_EXCED_RANGE);      /* Bail out, subdir is too long */
  }

  pszPathNext = (PSZ)malloc(MAX_PATH_LEN);         /* for copying the pathname */
  if (pszPathNext == NULL)                                /* out of memory ? */
  {
    free (pFileFindBuffer);              /* free previously allocated memory */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */
  }

  memmove ((PSZ)pszPathNext,                         /* copy the path string */
           (PSZ)pszPath,
           ulPathLength);

  pszTemp = pszPathNext + ulPathLength;           /* calculate this position */

  if (( pszPathNext[ulPathLength-1] == '\\' )|| /* ignore trailing backslash */
      ( pszPathNext[ulPathLength-1] == '/' ))   /* ignore trailing slash     */
  {
    ulPathLength--;
    pszPath[ulPathLength] = 0;                         /* cut trailing slash */
  }

                                                       /* "\*" -> 0x00002a5b */
  *( (PSHORT)pszTemp ) = 0x005c;                               /* append "\" */
  strcat (pszTemp,                    /* and concatenate the wildcard string */
          pszWildcard);

                                                           /* OK, los geht's */
  ulFindCount    = FILEFINDCOUNT;

  pFileInfo = (FILEFINDBUF3 *)pFileFindBuffer;

  rc = DosFindFirst(pszPathNext,
                    &hDirectory,
                    FILE_NORMAL    |
                    FILE_READONLY,
                    pFileInfo,
                    FINDBUFFERSIZE,
                    &ulFindCount,
                    FIL_STANDARD);
  if ( rc != NO_ERROR)                          /* check for error condition */
    return (rc);                                        /* return error code */

  do
  {
    while (ulFindCount)
    {
      strcpy (pszPathNext,                            /* build the full path */
              pszPath);

      ulPathLength = strlen(pszPathNext);        /* query the strings length */
                                                /* ignore trailing backslash */
      * ( (PUSHORT)(pszPathNext + ulPathLength) ) = 0x005c;             /* \ */

      strcat (pszPathNext,                                /* append new name */
              pFileInfo->achName);


      if ( !(pFileInfo->attrFile & FILE_DIRECTORY))           /* ignore dirs */
      {
        rc = ProcessFile(pszPathNext);              /* then proceed as usual */
        if (rc != NO_ERROR)
          ToolsErrorDos(rc);
      }

      ulFindCount--;
      pFileInfo = (FILEFINDBUF3 *) ((BYTE*)pFileInfo + pFileInfo->oNextEntryOffset);
    }

    ulFindCount = FILEFINDCOUNT;
    pFileInfo = (FILEFINDBUF3 *)pFileFindBuffer;
    rc = DosFindNext (hDirectory,
                      pFileInfo,
                      FINDBUFFERSIZE,
                      &ulFindCount);
  }
  while (rc == NO_ERROR);

  free((void *)pFileFindBuffer);
  free((void *)pszPathNext);

  return(DosFindClose(hDirectory));
}


/*****************************************************************************
 * Name      : APIRET ProcessRoot
 * Funktion  : Separate all the filenames
 * Parameter : PSZ pszFile
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

APIRET ProcessRoot(PSZ pszFile)
{
  APIRET      rc;                                         /* API return code */
  CHAR        szDirectoryBuffer[MAX_PATH_LEN];       /* static string buffer */

  /* 1. argument specified ? */
  if (pszFile == NULL)
  {
    pszFile = _getcwd(szDirectoryBuffer,
                      sizeof(szDirectoryBuffer));
    if (pszFile == NULL)
      return ERROR_INVALID_DATA;
  }

  /* 2. determine real name */
  rc = DosQueryPathInfo(pszFile,                          /* query full name */
                        FIL_QUERYFULLNAME,
                        szDirectoryBuffer,
                        sizeof(szDirectoryBuffer));
  if (rc)
  {
    ToolsErrorDos(rc);                                /* print error message */
    return (rc);                                           /* abort function */
  }


#ifdef DEBUG
  fprintf (stderr,
           "\nDosQueryPathInfo(%s) = #%u",
           szDirectoryBuffer,
           rc);
#endif

  rc = ProcessDirectory(szDirectoryBuffer, "*");
  return (rc);                                                         /* OK */
}


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
  PCHECKTABLEENTRY pcte;

  TOOLVERSION("MapExt",                                 /* application name */
              0x00010000,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */

  // display registered file types
  printf("\nKnown filetypes:\n");
  for (pcte = tabCheckTable;
       pcte->pfnCheck != NULL;
       pcte++)
  {
    printf("%-30s %10s %10s\n",
           pcte->pszName,
           pcte->pszRenameShort,
           pcte->pszRenameLong);
  }
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

VOID Initialize ( VOID )
{
  /* No Harderr popups */
  DosError (FERR_DISABLEHARDERR | FERR_ENABLEEXCEPTION);

  // PH: is pre-initialized
  //memset (&Options, 0, sizeof(Options));
  //memset (&Globals, 0, sizeof(Globals));
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
  PERFSTRUCT TS_Start;                       /* time information about start */
  PERFSTRUCT TS_End;                        /* time information about finish */
  float      seconds;                                     /* elapsed seconds */

  Initialize ();                                          /* Initialisierung */

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

  /* OK, do the work ! */
  ToolsPerfQuery (&TS_Start);                    /* exact time measurement */

  rc = ProcessRoot(Options.pszPath);
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  ToolsPerfQuery (&TS_End);                      /* exact time measurement */
  seconds = TS_End.fSeconds - TS_Start.fSeconds;     /* calculate duration */

  // some closing statistics
  printf("%u files renamed in %9.3fs (%10.3f files/sec).\n",
         Globals.ulRenamed,
         seconds,
         ((float)Globals.ulRenamed / seconds));

  return (rc);
}
