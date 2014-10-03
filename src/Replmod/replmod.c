/***********************************************************************
 * Name      : Module ReplMod
 * Funktion  : Ersetzen gelockter DLLs
 * Bemerkung : Eine etwas heikle und gef�hrliche Angelegenheit.
 *
 * Autor     : Patrick Haller [Samstag, 28.10.1995 14.54.33]
 ***********************************************************************/

/*
  DOS32REPLACEMODULE 417

      APIRET APIENTRY DosReplaceModule( PSZ pszOldModule,
                                  PSZ pszNewModule,
                                  PSZ pszBackupModule );

When a DLL or EXE file is in use by the system, the file is locked. It
can not therefore be replaced on the harddisk by a newer copy. The API
DosReplaceModule is to allow the replacement on the disk of the new
module whilst the system continues to run the old module. The contents
of the file pszOldModule are cached by the system, and the file is
closed. A backup copy of the file is created as pszBackupModule for
recovery purposes should the install program fail. The new module
pszModule takes the place of the original module on the disk.

Note - the system will continue to use the cached old module until all
references to it are released. The next reference to the module will
cause a reload from the new module on disk.

Note - only protect mode executable files can be replaced by this API.
This API can not be used for DOS/Windows programs, or for data files.

Note - pszNewModule and pszBackupModule may be NULL pointers.

Note - this API uses the swap file to cache the old module. This API
is expensive in terms of disk space usage.
*/


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#define INCL_DOSFILEMGR
#define INCL_DOS
#define INCL_DOSERRORS
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
#include "toolerror.h"

#define MAXPATHLEN 260



/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsModuleOld;
  ARGFLAG fsModuleNew;
  ARGFLAG fsModuleBackup;
  ARGFLAG fsCmdQueryCS;                     /* query module from CS selector */
  ARGFLAG fsCmdQueryEIP;                   /* query module from EIP register */
  ARGFLAG fsRecursive;             /* recursion through the subdirectories ? */
  ARGFLAG fsFind;                  /* for compatibility with old filesystems */
  ARGFLAG fsFileMask;                      /* file mask / wildcards supplied */
  ARGFLAG fsCmdQueryProc;                     /* query procedure from module */

  PSZ pszModuleOld;
  PSZ pszModuleNew;
  PSZ pszModuleBackup;

  ULONG  ulCS;                     /* code selector for DosQueryModuleFromCS */
  ULONG  ulEIP;                    /* EIP register for DosQueryModuleFromEIP */

  ULONG  ulFindNumberMaximum;                         /* maximum find number */
  PSZ    pszFileMask;                      /* file mask / wildcards supplied */
  PSZ    pszQueryProc;                     /* query this procedure from modl */
} OPTIONS, *POPTIONS;


typedef PSZ *PPSZ;

typedef struct
{
  ULONG ulFileSize;                                /* total size of the file */
  PSZ   pszFileBuffer;        /* points to the memory allocated for the file */
  ULONG ulFileFindBufferSize;                 /* size of the filefind buffer */
  ULONG ulFilesScanned;                           /* number of files scanned */
  ULONG ulFileMask;                  /* file attribute bits for the filefind */
} GLOBALS, *PGLOBALS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/
OPTIONS Options;                /* this structure holds command line options */
GLOBALS Globals;                /* this structure holds global variables     */

OPTIONS Options;                /* this structure holds command line options */

ARGUMENT TabArguments[] =
{ /*Token-------Beschreibung-------------pTarget-------------------ucTargetFormat--pTargetSpecified--*/
  {"/?",        "Get help screen.",      NULL,                     ARG_NULL,       &Options.fsHelp},
  {"/H",        "Get help screen.",      NULL,                     ARG_NULL,       &Options.fsHelp},
  {"/REPLACE=", "Replacement filename.", &Options.pszModuleNew,    ARG_PSZ,        &Options.fsModuleNew},
  {"/BACKUP=",  "Replacement filename.", &Options.pszModuleBackup, ARG_PSZ,        &Options.fsModuleBackup},
  {"/CS=",      "Query module from CS selector.",
    &Options.ulCS,            ARG_ULONG,      &Options.fsCmdQueryCS},
  {"/EIP=",     "Query module from EIP register.",
                                         &Options.ulEIP,           ARG_ULONG,      &Options.fsCmdQueryEIP},
  {"/CRAP",     "Compatibility to some Windows 95 requester",
                                         NULL,                     ARG_NULL,       &Options.fsFind},
  {"/S",        "Recurse through subdirectories.",
                                         NULL,                     ARG_NULL,       &Options.fsRecursive},
  {"/NAME=",    "Allows wildcards to replace found modules.",
                                         &Options.pszFileMask,     ARG_PSZ,        &Options.fsFileMask},
  {"/QUERY=",   "Querys this procedure (name / ordinal) from module.",
                                         &Options.pszQueryProc,    ARG_PSZ,        &Options.fsCmdQueryProc},
  {"1",         "Module filename.",      &Options.pszModuleOld,    ARG_PSZ |
                                                                   ARG_DEFAULT,    &Options.fsModuleOld},
  ARG_TERMINATE
};



/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

/*** IMPORTIERTE PROTOTYPEN ***/
APIRET APIENTRY DosReplaceModule( PSZ pszOldModule,
                                  PSZ pszNewModule,
                                  PSZ pszBackupModule );

/*** PROTOTYPEN ***/
void help     ( void );

APIRET ReplaceModules (PSZ pszModuleOld,
                       PSZ pszModuleNew,
                       PSZ pszModuleBackup);

int  main     ( int    argc,
                char  *argv[] );


/***********************************************************************
 * Name      : void help
 * Funktion  : Darstellen der Hilfe
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

void help ()
{
  TOOLVERSION("ReplMod",                                /* application name */
              0x00010009,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              "Warning: Executable is mapped to swap, file is unlocked.",
              NULL);                                /* additional copyright */
} /* void help */


/***********************************************************************
 * Name      : APIRET ReplaceModules
 * Funktion  : Ruft Dos32ReplaceModules auf.
 * Parameter : PSZ pszModuleOld, PSZ pszModuleNew, PSZ pszModuleBackup
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 28.10.1995 15.06.19]
 ***********************************************************************/

APIRET ReplaceModules (PSZ pszModuleOld,
                       PSZ pszModuleNew,
                       PSZ pszModuleBackup)
{
  APIRET rc = NO_ERROR;                                /* R…kgabewert */

  if ( pszModuleOld == NULL)                   /* Parameter｜erpr’ung */
    return (ERROR_INVALID_PARAMETER);
                /* pszModuleNew und pszModuleBackup d〉fen NULL sein ! */

  printf ("\nReplacing %s",
          pszModuleOld);

  if (pszModuleNew != NULL)
    printf (", by %s",
           pszModuleNew);

  if (pszModuleBackup != NULL)
    printf (", backup as %s",
           pszModuleBackup);

  rc = DosReplaceModule  (pszModuleOld,   /* Die API-Funktion aufrufen */
                          pszModuleNew,
                          pszModuleBackup);


  return (rc);                                 /* R…kgabewert liefern */
} /* APIRET ReplaceModules */


/***********************************************************************
 * Name      : APIRET QueryModuleFromCS
 * Funktion  : Determine which module resides at CS:xxxx
 * Parameter :
 * Variablen :
 * Ergebnis  : R…kgabewert ans Betriebssystem
 * Bemerkung : Dos16QueryModuleFromCS needs to be fed with
 *             a SELECTOROF(p) macro value.
 *             Crossing the 0x2000000 border, the same modules
 *             appear again.
 *
 * Autor     : Patrick Haller [Samstag, 28.10.1995 14.55.13]
 ***********************************************************************/

/* is defined in Toolkit 4.51
APIRET APIENTRY DosQueryModFromCS(USHORT usCS,
PQMRESULT pqmr;
*/



APIRET QueryModuleFromCS(void)
{
  APIRET rc;                                              /* API return code */
  QMRESULT qm;                              /* query module result structure */
  SEL selCS = SELECTOROF(Options.ulCS);

  rc = Dos16QueryModFromCS(selCS,
                           &qm);
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                               /* then raise error condition */

  printf ("\nCS 0x%08xh ",
          selCS,
          selCS);
  printf ("CS=%08xh (%u): HMTE=%04xh (%u) %s",
          qm.seg,
          qm.seg,
          qm.hmte,
          qm.hmte,
          qm.name);

  return (NO_ERROR);                                                   /* OK */
}


/***********************************************************************
 * Name      : APIRET QueryModuleProcedure
 * Funktion  : Queries a procedure from a module
 * Parameter :
 * Variablen :
 * Ergebnis  : R…kgabewert ans Betriebssystem
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 28.10.1995 14.55.13]
 ***********************************************************************/

APIRET QueryModuleProcedure(void)
{
  APIRET  rc;                                             /* API return code */
  CHAR    szModule[256];                       /* buffer for the module name */
  HMODULE hModule;                                          /* module handle */
  PFN     pfnProcedure;                      /* pointer to the DLL procedure */
  ULONG   ulProcedureType;                              /* type of procedure */

  if (!Options.fsModuleOld)                              /* check parameters */
  {
    fprintf (stderr,
             "\nError: /QUERY needs a module name.");
    return (ERROR_TOOLS_INVALID_ARGUMENT);
  }

  printf ("\nQuerying %s from module %s.",
          Options.pszQueryProc,
          Options.pszModuleOld);

  rc = DosLoadModule(szModule,
                     sizeof(szModule),
                     Options.pszModuleOld,
                     &hModule);
  if (rc != NO_ERROR)                                    /* check for errors */
  {
    ToolsErrorDosEx(rc,                               /* yield error message */
                    szModule);
    return (rc);                                    /* raise error condition */
  }


  rc = DosQueryModuleName(hModule,                    /* get the module name */
                          sizeof(szModule),
                          szModule);
  if (rc != NO_ERROR)                                    /* check for errors */
  {
    ToolsErrorDosEx(rc,                               /* yield error message */
                    "DosQueryModuleName");
    return (rc);                                    /* raise error condition */
  }


  /*****************
   * Ordinal Query *
   *****************/

  if (atoi(Options.pszQueryProc) != 0)                  /* argument usable ? */
  {
    rc = DosQueryProcAddr(hModule,            /* query procedure from module */
                          atoi(Options.pszQueryProc),
                          NULL,
                          &pfnProcedure);
    if (rc == NO_ERROR)                                  /* check for errors */
      printf ("\n  %s.%s (ord.) -> EIP = 0x%08x",
              szModule,
              Options.pszQueryProc,
              pfnProcedure);
    else
      printf ("\n  %s.%s (ord.) -> failed (%u)",
              szModule,
              Options.pszQueryProc,
              rc);

    rc = DosQueryProcType(hModule,            /* query procedure from module */
                          atoi(Options.pszQueryProc),
                          NULL,
                          &ulProcedureType);
    if (rc == NO_ERROR)                                  /* check for errors */
    {
      switch (ulProcedureType)
      {
        case PT_16BIT: printf (" (16 bit)");       break;
        case PT_32BIT: printf (" (32 bit)");       break;
        default:       printf (" <unknown type>"); break;
      }
    }
  }


  /*****************
   * Name    Query *
   *****************/

  rc = DosQueryProcAddr(hModule,              /* query procedure from module */
                        0,
                        Options.pszQueryProc,
                        &pfnProcedure);
  if (rc == NO_ERROR)                                    /* check for errors */
    printf ("\n  %s.%s (name) -> EIP = 0x%08x",
            szModule,
            Options.pszQueryProc,
            pfnProcedure);
  else
    printf ("\n  %s.%s (name) -> failed (%u)",
            szModule,
            Options.pszQueryProc,
            rc);

  rc = DosQueryProcType(hModule,            /* query procedure from module */
                        0,
                        Options.pszQueryProc,
                        &ulProcedureType);
  if (rc == NO_ERROR)                                  /* check for errors */
  {
    switch (ulProcedureType)
    {
      case PT_16BIT: printf (" (16 bit)");       break;
      case PT_32BIT: printf (" (32 bit)");       break;
      default:       printf (" <unknown type>"); break;
    }
  }


  DosFreeModule(hModule);                        /* release the module again */
   /* some handles cannot be freed. Particularly the handle to DOSCALLS, the */
   /* OS/2 kernel. So we ignore error codes here.                            */
  return(NO_ERROR);                                                    /* OK */
}


/***********************************************************************
 * Name      : APIRET QueryModuleFromEIP
 * Funktion  : Determine which module resides at 0:EIP
 * Parameter :
 * Variablen :
 * Ergebnis  : R…kgabewert ans Betriebssystem
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 28.10.1995 14.55.13]
 ***********************************************************************/

APIRET APIENTRY DosQueryModFromEIP (PULONG pulModule,
                                    PULONG pulObject,
                                    ULONG  ulBufferLength,
                                    PSZ    pszBuffer,
                                    PULONG pulOffset,
                                    ULONG  ulEIP);

APIRET QueryModuleFromEIP(void)
{
  APIRET rc;                                               /* API returncode */
  ULONG  ulModule;                                          /* module number */
  ULONG  ulObject;                        /* object number within the module */
  CHAR   szModule[260];                        /* buffer for the module name */
  ULONG  ulOffset;             /* offset within the object within the module */

  printf ("\nQuerying module at EIP 0x%08x (%u)",
          Options.ulEIP,
          Options.ulEIP);

  rc = DosQueryModFromEIP(&ulModule,
                          &ulObject,
                          sizeof(szModule),
                          szModule,
                          &ulOffset,
                          Options.ulEIP);
  if (rc == NO_ERROR)
  {
    printf ("\nModule %s (#%u), Object #%u, Offset %ub",
            szModule,
            ulModule,
            ulObject,
            ulOffset);
  }

  return (rc);                                                         /* OK */
}


/***********************************************************************
 * Name      : APIRET RMProcessFile
 * Funktion  : Process the found DLL name
 * Parameter :
 * Variablen :
 * Ergebnis  : R…kgabewert ans Betriebssystem
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 28.10.1995 14.55.13]
 ***********************************************************************/

APIRET RMProcessFile(PSZ pszFile)
{
  APIRET rc;                                               /* API returncode */

  /* OK, now build replace all "OLD" modules with this */
  rc = ReplaceModules(pszFile,
                      NULL,
                      NULL);

  if (rc == ERROR_FILE_NOT_FOUND)                       /* ignore this error */
  {
    printf (" (not loaded).");
    rc = NO_ERROR;
  }
  else
    if (rc == ERROR_MODULE_IN_USE)
    {
      printf (" (NE module - 16-bit).");
      rc = NO_ERROR;
    }
    else
      if (rc == NO_ERROR)
        printf (" OK.");

  return (rc);                                                         /* OK */
}


/******************************************************************************
 * Name      : APIRET RMProcessScan
 * Funktion  : Scan directories
 * Parameter : PSZ pszPath      - the current path
 *             PSZ pszWildcard  - the wildcard pattern
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:48:00]
 ******************************************************************************/

APIRET RMProcessScan (PSZ pszPath,
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
           "\nRMProcessScan(%s,%s)",
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
        rc = RMProcessFile(pszPathNext);            /* then proceed as usual */
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
 * Name      : APIRET RMProcessScanDir
 * Funktion  : Scan directories ad max. speed
 * Parameter : PSZ pszPath      - the current path
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:48:00]
 ******************************************************************************/

APIRET RMProcessScanDir (PSZ pszPath,
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
           "\nWCProcessScanDir(%s,%s)",
           pszPath,
           pszWildcard);
#endif

  rc = RMProcessScan(pszPath,                   /* scan the directory first */
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
        rc = RMProcessScanDir(pszPathNext,
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
 * Name      : APIRET RMProcessFiles
 * Funktion  : Separate all the filenames
 * Parameter : PSZ pszFile
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

APIRET RMProcessFiles (PSZ pszFile)
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
           "\nWCProcessFiles(%s)",
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
        rc = RMProcessScanDir(pszTemp,       /* recursively scan directories */
                               pszWildcard);
      else
        rc = RMProcessScan(pszTemp,                                  /* path */
                           pszWildcard);
    }
    else
      rc = RMProcessFile(pszTemp);                         /* this file only */

    if (rc != NO_ERROR)                                  /* check for errors */
      ToolsErrorDosEx(rc,                             /* yield error message */
                      "RMProcessFiles");

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
 * Ergebnis  : R…kgabewert ans Betriebssystem
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 28.10.1995 14.55.13]
 ***********************************************************************/

int main (int argc, char *argv[])
{
  APIRET rc = NO_ERROR;                                /* R…kgabewert */

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


  if (Options.fsCmdQueryCS)
    rc = QueryModuleFromCS();
  else
    if (Options.fsCmdQueryEIP)
      rc = QueryModuleFromEIP();
    else
    if (Options.fsCmdQueryProc)
      rc = QueryModuleProcedure();
    else
    if (Options.fsFileMask)                        /* wildcards supplied ? */
      rc = RMProcessFiles(Options.pszFileMask);                     /* yes */
    else
      if (!Options.fsModuleOld)
      {
        fprintf(stderr,
                "\nError: no module name specified");
        rc = NO_ERROR;                            /* not really an error ... */
      }
      else
        rc = ReplaceModules (Options.pszModuleOld,                   /* no */
                             Options.pszModuleNew,
                             Options.pszModuleBackup);

  if (rc != NO_ERROR)                        /* Ist ein Fehler aufgetreten ? */
      ToolsErrorDos(rc);                           /* Fehlermeldung ausgeben */

  return (rc);                                       /* R…kgabewert liefern */
} /* int main */
