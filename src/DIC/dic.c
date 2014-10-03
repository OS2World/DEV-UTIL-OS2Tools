/***********************************************************************
 * Name      : DIC
 * Funktion  : Building a dictionary over several text files
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:25:17]
 ***********************************************************************/

/* #define DEBUG */
/*#error Not completed !*/

/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#ifdef __OS2__
  #define INCL_DOSFILEMGR
  #define INCL_BASE
  #define INCL_NOPMAPI
  #include <os2.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <io.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"


#define MAXPATHLEN 260


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct _Report
{
  ULONG ulChars;                         /* number of characters in the file */
  ULONG ulWords;                              /* number of words in the file */
  ULONG ulLines;                              /* number of lines in the file */
} REPORT, *PREPORT;


typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsFileInput;                 /* user specified the input file name */
  ARGFLAG fsFileOutput;               /* user specified the output file name */
  ARGFLAG fsVerbose;                                       /* verbose output */
  ARGFLAG fsRecursive;             /* recursion through the subdirectories ? */
  ARGFLAG fsFind;                  /* for compatibility with old filesystems */

  PSZ   pszFileInput;                          /* this is the input filename */

  ULONG   ulFindNumberMaximum;                        /* maximum find number */
} OPTIONS, *POPTIONS;

typedef PSZ *PPSZ;

typedef struct
{
  ULONG ulFileSize;                                /* total size of the file */
  PSZ   pszFileBuffer;        /* points to the memory allocated for the file */
  ULONG ulFileFindBufferSize;                 /* size of the filefind buffer */
  ULONG ulFilesScanned;                           /* number of files scanned */
  ULONG ulFileMask;                  /* file attribute bits for the filefind */

  PPSZ  parrStrings;              /* pointer to dynamic string pointer array */
  ULONG ulStrings;                             /* number of string in buffer */
  ULONG ulStringsCurrent;                 /* index of current string pointer */

  REPORT Report;                                        /* the global report */
} GLOBALS, *PGLOBALS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/
OPTIONS Options;                /* this structure holds command line options */
GLOBALS Globals;                /* this structure holds global variables     */


ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung---------------pTarget----------------ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",        NULL,                  ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",        NULL,                  ARG_NULL,       &Options.fsHelp},
  {"/CRAP",      "Compatibility to some Windows 95 requester",
                                            NULL,                  ARG_NULL,       &Options.fsFind},
  {"/S",         "Recurse through subdirectories.",
                                            NULL,                  ARG_NULL,       &Options.fsRecursive},
  {"/VERBOSE",   "Verbose output.",         NULL,                  ARG_NULL,       &Options.fsVerbose},
  {"1",          "Input file(s).",          &Options.pszFileInput, ARG_PSZ  |
                                                                   ARG_DEFAULT,    &Options.fsFileInput},
  ARG_TERMINATE
};


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/
void  help               (void);
int   main               (int, char **);


/***********************************************************************
 * Name      : void help
 * Funktion  : Anzeigen des Hilfetextes
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:43:33]
 ***********************************************************************/

void help (void)
{
  TOOLVERSION("WordCount",                               /* application name */
              0x00010001,                            /* application revision */
              0x00010100,           /* minimum required revision of PHSTOOLS */
              0x0001FFFF,        /* maximum recommended revision of PHSTOOLS */
              NULL,                                                /* Remark */
              NULL);                                 /* additional copyright */
}


/*****************************************************************************
 * Name      : APIRET DICProcess
 * Funktion  : build pointer list
 * Parameter :
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

APIRET DICProcess (PREPORT pReport,
                  ULONG   ulSize)
{
  ULONG ulCount;                        /* loop counter over the file buffer */
  PSZ   pszTemp;                                 /* temporary string pointer */
  BOOL  fInWord = FALSE;                          /* we start outside a word */

  memset (pReport,                          /* zero out the report structure */
          0L,
          sizeof(REPORT));

  pReport->ulChars = ulSize;       /* the number of characters in the buffer */

  for (ulCount = 0,
       pszTemp = Globals.pszFileBuffer;

       ulCount < ulSize;

       ulCount++,
       pszTemp++)
  {
    if (*pszTemp == 0x0a)                                        /* LINEFEED */
      pReport->ulLines++;

    if ( ( (*pszTemp < '0') ||                                  /* word scan */
           (*pszTemp > '9') ) &&
         ( (*pszTemp < 'A') ||
           (*pszTemp > 'Z') ) &&
         ( (*pszTemp < 'a') ||
           (*pszTemp > 'z') ) &&
           (*pszTemp != '_')
       )
    {
      if (fInWord == TRUE)                 /* if we have been outside a word */
        fInWord = FALSE;                             /* still outside a word */
    }
    else
    {
      if (fInWord == FALSE)
      {
        pReport->ulWords++;                               /* count this word */
        fInWord = TRUE;                       /* then we start a new one now */
      }
    }
  }

  return (NO_ERROR);                                                   /* OK */
}


/*****************************************************************************
 * Name      : APIRET DICProcessSTDIN
 * Funktion  : DIC data read from stdin
 * Parameter :
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

APIRET DICProcessSTDIN (void)
{
  APIRET      rc = NO_ERROR;                              /* API return code */
  PSZ         pStdinBuffer;                       /* pointer to stdin buffer */
  ULONG       ulStdinBufferSize = 65536;                /* stdin buffer size */
  ULONG       ulReadSize = 65535;                           /* standard size */
  ULONG       ulStdinPosition;                      /* current read position */
  ULONG       ulReadBytes;           /* bytes read during last I/O operation */

  if (Options.fsVerbose)                                 /* verbose output ? */
    printf ("\nDICing ..."
            "\n- reading from \\DEV\\STDIN\n");

  rc = DosAllocMem (&pStdinBuffer,              /* initial memory allocation */
                    ulStdinBufferSize,              /* but not yet committed */
                    PAG_READ | PAG_WRITE | PAG_COMMIT);
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                                    /* raise error condition */

  ulStdinPosition = 0;                                     /* initialization */

  do                                               /* read to dynamic buffer */
  {
#if 0
    rc = DosRead (0 /* HFILE_STDIN */,              /* read from standard in */
                  &pStdinBuffer[ulStdinPosition],
                  ulReadSize,
                  &ulReadBytes);
    if (rc == NO_ERROR)                            /* check if error occured */
#endif

    ulReadBytes = read (0,
                        &pStdinBuffer[ulStdinPosition],
                        ulReadSize);

    if (errno == 0)
    {
      ulStdinPosition += ulReadBytes;             /* calculate next position */

                                  /* running out of committed memory pages ? */
      if (ulStdinPosition + ulReadSize > ulStdinBufferSize)
      {
        PVOID pNewBuffer;               /* pointer to newly allocated buffer */

        rc = DosAllocMem (&pNewBuffer,         /* re-allocate memory block ! */
                          2 * ulStdinBufferSize,
                          PAG_READ | PAG_WRITE | PAG_COMMIT);
        if (rc != NO_ERROR)                              /* check for errors */
        {
          DosFreeMem(pStdinBuffer);      /* free previously allocated memory */
          return (rc);
        }

        memcpy (pNewBuffer,                           /* copy the data bytes */
                pStdinBuffer,
                ulStdinPosition);

        DosFreeMem(pStdinBuffer);                     /* free the old buffer */

        pStdinBuffer = pNewBuffer;                    /* adjust the pointers */
        ulStdinBufferSize *= 2;                 /* double the reserved range */
      }
    }
  }
  while ( (rc == NO_ERROR) & (ulReadBytes != 0) );

  Globals.pszFileBuffer = (PSZ)pStdinBuffer;  /* adjust the global variables */
  Globals.ulFileSize    = ulStdinPosition;

  DICProcess(&Globals.Report,
            Globals.ulFileSize);                      /* call cwlc functions */

  return (rc);                                             /* signal success */
}


/*****************************************************************************
 * Name      : APIRET DICProcessFile
 * Funktion  : Map file into memory
 * Parameter : PSZ pszFile
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

APIRET DICProcessFile (PSZ   pszFile,
                      ULONG ulSize)
{
  APIRET      rc;                                         /* API return code */
  REPORT      Report;                   /* the char/word/line counter report */

#ifdef DEBUG
  fprintf (stderr,
           "\nDICProcessFile(%s)",
           pszFile);
#endif

  if (pszFile == NULL)                                   /* check parameters */
    return (ERROR_INVALID_PARAMETER);                   /* return error code */

  if (ulSize != 0)                                       /* zero-sized files */
  {
    rc = ToolsReadFileToBuffer(pszFile,                     /* read the file */
                               &Globals.pszFileBuffer,
                               &Globals.ulFileSize);
    if (rc != NO_ERROR)
    {
      ToolsErrorDos(rc);                                 /* check for errors */
      return (rc);                             /* abort function immediately */
    }

    DICProcess(&Report,
              Globals.ulFileSize);                    /* call cwlc functions */

    rc = DosFreeMem(Globals.pszFileBuffer);        /* free the memory buffer */
  }
  else
  {
    Report.ulChars = 0;                        /* set all statistics to zero */
    Report.ulWords = 0;
    Report.ulLines = 0;
    rc             = NO_ERROR;
  }

  printf ("\n%8u %8u %8u %s",                            /* print the report */
          Report.ulChars,
          Report.ulWords,
          Report.ulLines,
          pszFile);

  Globals.Report.ulChars += Report.ulChars;      /* update global statistics */
  Globals.Report.ulWords += Report.ulWords;
  Globals.Report.ulLines += Report.ulLines;

  return (rc);                                             /* signal success */
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

APIRET DICProcessScan (PSZ pszPath,
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
           "\nDICProcessScan(%s,%s)",
           pszPath,
           pszWildcard);
#endif

  pFileFindBuffer = (PSZ)malloc(Globals.ulFileFindBufferSize);
  if (pFileFindBuffer == NULL)              /* check if allocation succeeded */
    return(ERROR_NOT_ENOUGH_MEMORY);

  ulPathLength = strlen(pszPath);                 /* OK, get the origin path */
  if (ulPathLength > MAXPATHLEN)
    return(ERROR_FILENAME_EXCED_RANGE);      /* Bail out, subdir is too long */

  pszPathNext = (PSZ)malloc(MAXPATHLEN);         /* for copying the pathname */
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
  ulFindCount    = Options.ulFindNumberMaximum;

  pFileInfo = (FILEFINDBUF3 *)pFileFindBuffer;

  rc = DosFindFirst(pszPathNext,
                    &hDirectory,
                    Globals.ulFileMask,
                    pFileInfo,
                    Globals.ulFileFindBufferSize,
                    &ulFindCount,
                    FIL_STANDARD);
  if ( rc != NO_ERROR)                          /* check for error condition */
    return (rc);                                        /* return error code */

  do
  {
    Globals.ulFilesScanned += ulFindCount;

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
        rc = DICProcessFile(pszPathNext,             /* then proceed as usual */
                           pFileInfo->cbFile);
        if (rc != NO_ERROR)
          ToolsErrorDos(rc);
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


/******************************************************************************
 * Name      : APIRET DICProcessScanDir
 * Funktion  : Scan directories ad max. speed
 * Parameter : PSZ pszPath      - the current path
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:48:00]
 ******************************************************************************/

APIRET DICProcessScanDir (PSZ pszPath,
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
           "\nDICProcessScanDir(%s,%s)",
           pszPath,
           pszWildcard);
#endif

  rc = DICProcessScan(pszPath,                   /* scan the directory first */
                      pszWildcard);

  pFileFindBuffer = (PSZ)malloc(Globals.ulFileFindBufferSize);
  if (pFileFindBuffer == NULL)              /* check if allocation succeeded */
    return(ERROR_NOT_ENOUGH_MEMORY);

  ulPathLength = strlen(pszPath);                 /* OK, get the origin path */
  if (ulPathLength > MAXPATHLEN)
    return(ERROR_FILENAME_EXCED_RANGE);      /* Bail out, subdir is too long */

  pszPathNext = (PSZ)malloc(MAXPATHLEN);         /* for copying the pathname */
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
  *( (PULONG)pszTemp ) = 0x00002a5c;                          /* append "\*" */

                                                           /* OK, los geht's */
  ulFindCount    = Options.ulFindNumberMaximum;

  pFileInfo = (FILEFINDBUF3 *)pFileFindBuffer;

  rc = DosFindFirst(pszPathNext,
                    &hDirectory,
                    MUST_HAVE_DIRECTORY,
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
      strcpy (pszPathNext,                            /* build the full path */
              pszPath);

      ulPathLength = strlen(pszPathNext);        /* query the strings length */
                                                /* ignore trailing backslash */
      * ( (PUSHORT)(pszPathNext + ulPathLength) ) = 0x005c;             /* \ */

      strcat (pszPathNext,                                /* append new name */
              pFileInfo->achName);

      if ((pFileInfo->achName[0] == '.') &&               /* ignore . and .. */
          (( !pFileInfo->achName[1] ||
          ((pFileInfo->achName[1] == '.') && !pFileInfo->achName[2])) ));
      else
      {
        rc = DICProcessScanDir(pszPathNext,
                               pszWildcard);                    /* recursion */
        if (rc != NO_ERROR)
          ToolsErrorDos(rc);
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
 * Name      : APIRET DICProcessFiles
 * Funktion  : Separate all the filenames
 * Parameter : PSZ pszFile
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

APIRET DICProcessFiles (PSZ pszFile)
{
  APIRET      rc;                                         /* API return code */
  PSZ         pszTemp;                           /* temporary string pointer */
  PSZ         pszTemp2;                          /* temporary string pointer */
  FILESTATUS3 fs3;          /* filestatus 3 structure for the given filename */
  PSZ         pszWildcard = "*";   /* points to wildcard section of filename */
  CHAR        szRoot[MAXPATHLEN];     /* buffer for qualified root path name */
  ULONG       ulTemp;                      /* temporary calculation variable */


  if (pszFile == NULL)                                   /* check parameters */
    return (ERROR_INVALID_PARAMETER);                   /* return error code */

#ifdef DEBUG
  fprintf (stderr,
           "\nDICProcessFiles(%s)",
           pszFile);
#endif

  /* 1. loop -> tokenizer */
  pszTemp = strtok(pszFile,                 /* tokenize the string by commas */
                   ",");
  while (pszTemp)
  {
    /* 2. loop -> scan wildcards */
    rc = DosQueryPathInfo(pszFile,             /* if file is a normal "FILE" */
                          FIL_STANDARD,/* then don't try to expand wildcards */
                          &fs3,
                          sizeof(fs3));

#ifdef DEBUG
  fprintf (stderr,
           "\nDosQueryPathInfo(%s) = #%u",
           pszFile,
           rc);
#endif

    if (rc != NO_ERROR)
    {
                                            /* query root path and file part */
                     /* scan the last part of the filename for the file only */
      pszWildcard = strrchr(pszTemp,'\\');    /* point to last part of fname */
      pszTemp2 = strrchr(pszTemp,'/');       /* scan for alternate delimiter */

      if (pszTemp2 > pszWildcard)                 /* take tha last character */
        pszWildcard = pszTemp2;

      if (pszWildcard == NULL)                 /* still no delimiter found ? */
        pszWildcard = strrchr(pszTemp,':');       /* then the drive letter ? */

      if (pszWildcard == NULL)   /* if still no path part found, assume file */
      {
        pszWildcard = pszTemp;      /* then the remaining part is wildcard ! */
        pszTemp = ".";
      }
      else
      {
        ulTemp = pszWildcard - pszTemp + 1;       /* calculate string length */

        strncpy (szRoot,                               /* copy the path part */
                 pszTemp,
                 ulTemp);
        szRoot[ulTemp] = 0;                          /* terminate the string */
        pszTemp = szRoot;                  /* and set pointer to root buffer */

        pszWildcard++;                             /* skip to next character */
        if (*pszWildcard == 0)        /* check if string termination reached */
          pszWildcard = "*";                 /* then assume default wildcard */
      }

                                       /* cut forbidden trailing backslashes */
      ulTemp = strlen(pszTemp) - 1;        /* note: strlen(pszTemp) != 0 !!! */
      if ( ( (pszTemp[ulTemp] == '\\') ||
             (pszTemp[ulTemp] == '/' ) ) &&
           (ulTemp > 0) )
        pszTemp[ulTemp] = 0;                           /* cut this backslash */

      /* 2. loop -> scan wildcards */
      rc = DosQueryPathInfo(pszTemp,           /* if file is a normal "FILE" */
                            FIL_STANDARD,   /* then don't try expand wildcrds*/
                            &fs3,
                            sizeof(fs3));

#ifdef DEBUG
    fprintf (stderr,
           "\nDosQueryPathInfo2(%s) = #%u, pszWildcard=%s",
           pszTemp,
           rc,
           pszWildcard);
#endif

      if (rc != NO_ERROR)                                /* check for errors */
      {
        ToolsErrorDosEx(rc,                           /* yield error message */
                        pszTemp);                               /* and abort */
        return (rc);
      }
    }

    if (fs3.attrFile & FILE_DIRECTORY)                  /* is it a directory */
    {
      if (Options.fsRecursive)
        rc = DICProcessScanDir(pszTemp,       /* recursively scan directories */
                               pszWildcard);
      else
        rc = DICProcessScan(pszTemp,                                  /* path */
                           pszWildcard);
    }
    else
      rc = DICProcessFile(pszTemp,
                         fs3.cbFile);                      /* this file only */

    if (rc != NO_ERROR)                                  /* check for errors */
      ToolsErrorDosEx(rc,                             /* yield error message */
                      "DICProcessFiles");

    pszTemp = strtok(NULL,                  /* tokenize the string by commas */
                     ",");
  }

  return (NO_ERROR);                                                   /* OK */
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

  memset (&Options,                      /* initialize the global structures */
          0,
          sizeof(Options));

  memset (&Globals,
          0,
          sizeof(Globals));
  

  rc = ArgStandard (argc,                            /* CLI-Parameter parsen */
                    argv,
                    TabArguments,
                    &Options.fsHelp);
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    exit(1);                                                /* abort program */
  }

  if (Options.fsHelp)                                  /* user requests help */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  /***************************************************************************
   * Map arguments                                                           *
   ***************************************************************************/

  if (!Options.fsFind)           /* user did not specify maximum find number */
    Options.ulFindNumberMaximum = 65535 / sizeof(FILEFINDBUF3);
  else
    Options.ulFindNumberMaximum = 1;        /* for some Windows 95 requester */
 /* OS/2 2.x, 3.x provide by far best performance when avoiding 64k switches */

  Globals.ulFileFindBufferSize = sizeof(FILEFINDBUF3) * /* our target struct */
                                 Options.ulFindNumberMaximum;

  Globals.ulFileMask = FILE_NORMAL    |
                       FILE_READONLY;

  if (Options.fsFileInput)  /* check whether cwlcing from stdin of from file */
    rc = DICProcessFiles(Options.pszFileInput);
  else
    rc = DICProcessSTDIN();                                /* cwlc from stdin */

  printf ("\n%8u %8u %8u (all)",
          Globals.Report.ulChars,
          Globals.Report.ulWords,
          Globals.Report.ulLines);

  if (rc != NO_ERROR)                                 /* print error message */
    ToolsErrorDos(rc);

  return (rc);
}
