/***********************************************************************
 * Name      : Which
 * Funktion  : Scanning a given path for a given filename
 *
 * Autor     : Patrick Haller [2001-06-19]
 ***********************************************************************/


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#ifdef __OS2__
  #define INCL_DOSFILEMGR
  #define INCL_DOSMISC
  #define INCL_BASE
  #define INCL_NOPMAPI
  #include <os2.h>
#endif

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
  ARGFLAG fsFile;                      /* user specified the input file name */
  ARGFLAG fsPath;                          /* user specified the search path */
  ARGFLAG fsVerbose;                                       /* verbose output */
  ARGFLAG fsShort;                                         /* shorted output */
  ARGFLAG fsEnvVar;                   /* scan specified environment variable */
  ARGFLAG fsDontScanCurrent;                /* do not scan current directory */

  PSZ   pszFile;                               /* this is the input filename */
  PSZ   pszPath;                           /* user specified the search path */
  PSZ   pszEnvVar;                               /* this is the env var name */
} OPTIONS, *POPTIONS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/
OPTIONS Options;                /* this structure holds command line options */


ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung---------------pTarget----------------ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",        NULL,                  ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",        NULL,                  ARG_NULL,       &Options.fsHelp},
  {"/V",         "Verbose output.",         NULL,                  ARG_NULL |
                                                                   ARG_HIDDEN,     &Options.fsVerbose},
  {"/VERBOSE",   "Verbose output.",         NULL,                  ARG_NULL,       &Options.fsVerbose},
  {"/S",         "Shortened output.",       NULL,                  ARG_NULL ,      &Options.fsShort},
  {"/!C",        "Don't scan current directory first.",
                                            NULL,                  ARG_NULL,       &Options.fsDontScanCurrent},
  {"/ENV=",      "Scan the specified environment variable.",
                                            &Options.pszEnvVar,    ARG_PSZ,        &Options.fsEnvVar},
  {"/PATH=",     "Scan the specified path.",
                                            &Options.pszPath,      ARG_PSZ,        &Options.fsPath},
  {"1",          "Input file(s).",          &Options.pszFile,      ARG_PSZ  |
                                                                   ARG_MUST |
                                                                   ARG_DEFAULT,    &Options.fsFile},
  ARG_TERMINATE
};


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/
void   help              (void);

int    main              (int,
                          char **);


/***********************************************************************
 * Name      : APIRET querySystemLIBPATH
 * Funktion  : query the global system loader libpath
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung : this approach is non-documented, there is no
 *             published method on how to get the LIBPATH
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:43:33]
 ***********************************************************************/


APIRET APIENTRY DosQueryHeaderInfo(HMODULE hMod,
                                   ULONG   ulIndex,
                                   PUCHAR  pBuffer,
                                   ULONG   ulBufferLength,
                                   ULONG   ulSubFunction);

#define QHI_EXEINFO             1
#define QHI_QUERYRESOURCETABLE  2
#define QHI_READRESOURCETABLE   3
#define QHI_LIBPATHLEN          4
#define QHI_LIBPATH             5
#define QHI_FIXENTRY            6
#define QHI_STE                 7
#define QHI_MAPSELECTOR         8

// Note: QHI_LIBPATHLEN requires lBufferLength to be equal 4
// Note: QHI_LIBPATH and QHI_LIBPATHLEN don't use hMod


APIRET querySystemLIBPATH(PSZ   pszBuffer,
                          ULONG ulBufferLength)
{
    APIRET rc;                                      /* operation return code */
    ULONG ulLibpathLength;

    rc = DosQueryHeaderInfo(NULLHANDLE,
                            0,
                            (PUCHAR)&ulLibpathLength,
                            sizeof(ulLibpathLength),
                            QHI_LIBPATHLEN);
    if (NO_ERROR != rc)
        return rc;

    if (ulBufferLength < ulLibpathLength)
        return ERROR_INSUFFICIENT_BUFFER;

    rc = DosQueryHeaderInfo(NULLHANDLE,
                            0,
                            pszBuffer,
                            ulBufferLength,
                            QHI_LIBPATH);
    return rc;
}


/***********************************************************************
 * Name      : APIRET displayPathInfo
 * Funktion  : display more information about the found file
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:43:33]
 ***********************************************************************/

APIRET displayPathInfo(PSZ pszPath,
                       BOOL flagShort)
{
    char   szFCreation  [40];   /* Buffers for the conversion from time/date */
    char   szFLastWrite [40];
    char   szFLastAccess[40];
    char   szBuffer     [128];
    APIRET rc;
    FILESTATUS4 fs4;       /* Structure to hold the second level information */

    rc = DosQueryPathInfo(pszPath,                     /* Gather information */
                          FIL_QUERYEASIZE,
                          &fs4,
                          sizeof(fs4));
    if (NO_ERROR != rc)
        return rc;


    StrFDateTimeToString(fs4.fdateCreation,
                         fs4.ftimeCreation,
                         szFCreation);

    StrFDateTimeToString(fs4.fdateLastWrite,
                         fs4.ftimeLastWrite,
                         szFLastWrite);

    StrFDateTimeToString(fs4.fdateLastAccess,
                         fs4.ftimeLastAccess,
                         szFLastAccess);


    if (flagShort)
    {
        printf("%s %10u %5u %s\n",
               szFLastWrite,
               fs4.cbFile,
               fs4.cbList >> 1,
               pszPath);

        return NO_ERROR;
    }


    printf ("\n컴횳eneral Information컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�"
            "\n   Full name              : %s"
            "\n   Actual file size       : [%12u]"
            "\n   Allocated size         : [%12u]"
            "   --> slack space : [%12i]",
            pszPath,
            fs4.cbFile,
            fs4.cbFileAlloc,
            fs4.cbFileAlloc - fs4.cbFile);

    if ( (fs4.cbFileAlloc < fs4.cbFile) &&
         (fs4.cbFileAlloc != 0) )
      printf ("\n   Compression            : 1:%2.2f",
              (float)fs4.cbFile / (float)fs4.cbFileAlloc);

    if (fs4.cbList > 4)
      printf ("\n   Extended Attribute size: [%12u] (buffer)",
              fs4.cbList >> 1);

    printf ("\n컴횫ccess컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴"
            "\n   Creation               : %s"
            "\n   Last write             : %s"
            "\n   Last access            : %s",
            szFCreation,
            szFLastWrite,
            szFLastAccess
           );

    StrFAttrToString (fs4.attrFile,
                      szBuffer);

    printf ("\n   Attributes             : %s",
            szBuffer);

    return NO_ERROR;
}


/***********************************************************************
 * Name      : APIRET scanPath
 * Funktion  : scan a certain path for a specified file name
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:43:33]
 ***********************************************************************/

#define SCAN_UNKNOWN        0
#define SCAN_PATH           1
#define SCAN_PATH_GUESS_EXT 2
#define SCAN_DPATH          3
#define SCAN_LIBPATH        4
#define SCAN_CLASSPATH      5

static PSZ arrpszExecutables[] = {".exe",
                                  ".com",
                                  ".cmd",
                                  ".bat",
                                  NULL};

APIRET scanPath(void)
{
    PSZ       pszPathValue;        /* will contain the search path to follow */
    PSZ       pszSearchName;        /* name of the kind of search we've done */
    ULONG     ulSearchMode = 0;       /* search mode flags for DosSearchPath */
    CHAR      searchResult[256];
    APIRET    rc;
    int       iScanMode = SCAN_DPATH;

    // determine which path to scan


    // implied scan of current directory ?
    ulSearchMode = SEARCH_IGNORENETERRS;
    if (Options.fsDontScanCurrent)
        ;
    else
        ulSearchMode |= SEARCH_CUR_DIRECTORY;

    // specified environment variable name treated as path
    if (Options.fsEnvVar)
    {
        rc = DosScanEnv(Options.pszEnvVar, &pszPathValue);
        if (NO_ERROR != rc)
            return rc;

        pszSearchName = Options.pszEnvVar;
    }
    else

    // if the user specified a complete path string ...
    if (Options.fsPath)
    {
        pszPathValue = Options.pszPath;
        pszSearchName = "(specified)";
    }
    else
    {
        PSZ pszExt;

        // for executables (.exe .cmd .com .bat), we're supposed to scan PATH
        // .dll -> libpath
        // .class .zip .jar -> classpath
        // other -> DPATH

        // find last dot and point to the extension string
        pszExt = strrchr(Options.pszFile, '.');
        if (pszExt != NULL)
        {
            // executables
            if ( (stricmp(".exe", pszExt) == 0) ||
                 (stricmp(".com", pszExt) == 0) ||
                 (stricmp(".cmd", pszExt) == 0) ||
                 (stricmp(".bat", pszExt) == 0) )
            {
                iScanMode = SCAN_PATH;
            }
            else
                // dlls
                if (stricmp(".dll", pszExt) == 0)
                {
                    iScanMode = SCAN_LIBPATH;
                }
                else
                    // java support
                    if ( (stricmp(".class", pszExt) == 0) ||
                         (stricmp(".jar", pszExt) == 0) ||
                         (stricmp(".zip", pszExt) == 0) )
                    {
                        iScanMode = SCAN_CLASSPATH;
                    }
                    else
                        // other
                        iScanMode = SCAN_DPATH;
        }
        else
        {
            // as no extension is present, we've
            // got to search for various combinations of
            // extensions as is done by CMD.EXE:
            // .exe .com .cmd .bat
            iScanMode = SCAN_PATH_GUESS_EXT;
        }


        // OK, extract the path to be scanned
        switch (iScanMode)
        {
          case SCAN_PATH:
          case SCAN_PATH_GUESS_EXT:
              rc = DosScanEnv("PATH", &pszPathValue);
              pszSearchName = "PATH";
              break;
              
          case SCAN_DPATH:
              rc = DosScanEnv("DPATH", &pszPathValue);
              pszSearchName = "DPATH";
              break;
  
          case SCAN_CLASSPATH:
              rc = DosScanEnv("CLASSPATH", &pszPathValue);
              pszSearchName = "CLASSPATH";
              break;
  
          case SCAN_LIBPATH:
              pszSearchName = "LIBPATH";
              rc = NO_ERROR;
              break;
  
          default:
              pszSearchName = "(unknown)";
              return NO_ERROR;
        }
    }

    // check for previous errors
    if (NO_ERROR != rc)
        return rc;

    // verbose output ?
    if (Options.fsVerbose)
        printf("Scanning %s for %s:\n%s\n",
               pszSearchName,
               Options.pszFile,
               pszPathValue);

    // scan the determined path
    if (iScanMode == SCAN_PATH_GUESS_EXT)
    {
        CHAR szBuffer[256];
        int  iIndex = 0;

        // try sequence of executable extensions
        do
        {
            strcpy(szBuffer, Options.pszFile);
            strcat(szBuffer, arrpszExecutables[iIndex]);

            // search for the file
            rc = DosSearchPath(ulSearchMode,
                               pszPathValue,
                               szBuffer,
                               (PBYTE)&searchResult,
                               sizeof(searchResult));

            // break if file was found
            if (rc == NO_ERROR)
                break;

            iIndex++; // if not found, try next extension
        }
        while (arrpszExecutables[iIndex] != NULL);

    }
    else
    if (iScanMode == SCAN_LIBPATH)
    {
        // WARNING: kernel seems to have flawed buffer validation scheme,
        // traps are expected on overflow.

        // first, we get BEGINLIBPATH, ENDLIBPATH, and LIBPATH
        CHAR szLPBegin[4096];
        CHAR szLPEnd[4096];
        CHAR szLPSystem[4096];


        rc = DosQueryExtLIBPATH(szLPBegin, BEGIN_LIBPATH);
        if (rc != NO_ERROR)
            szLPBegin[0] = 0;

        rc = DosQueryExtLIBPATH(szLPEnd,   END_LIBPATH);
        if (rc != NO_ERROR)
            szLPEnd[0] = 0;

        rc = querySystemLIBPATH(szLPSystem, sizeof(szLPSystem));
        if (rc != NO_ERROR)
            szLPSystem[0] = 0;

        // check for verbose output
        if (Options.fsVerbose)
            printf ("LIBPATH settings:\n"
                    "--- begin  %s\n"
                    "--- system %s\n"
                    "--- end    %s\n",
                    szLPBegin,
                    szLPSystem,
                    szLPEnd);

        // now search through the three paths in specified order
        rc = DosSearchPath(ulSearchMode,
                           szLPBegin,
                           Options.pszFile,
                           (PBYTE)&searchResult,
                           sizeof(searchResult));
        if (rc != NO_ERROR)
            rc = DosSearchPath(ulSearchMode,
                               szLPSystem,
                               Options.pszFile,
                               (PBYTE)&searchResult,
                               sizeof(searchResult));
        if (rc != NO_ERROR)
            rc = DosSearchPath(ulSearchMode,
                               szLPEnd,
                               Options.pszFile,
                               (PBYTE)&searchResult,
                               sizeof(searchResult));

    }
    else
    {
        // scan with single given extension
        rc = DosSearchPath(ulSearchMode,
                           pszPathValue,
                           Options.pszFile,
                           (PBYTE)&searchResult,
                           sizeof(searchResult));
    }
    if (NO_ERROR != rc)
        return rc;

    // print information obtained by DosQueryPathInfo()
    if (Options.fsVerbose)
        displayPathInfo(searchResult, FALSE);
    else
        if (Options.fsShort)
            printf("\n%s",
                   searchResult);
        else
            displayPathInfo(searchResult, TRUE);

    return NO_ERROR;
}


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
  TOOLVERSION("Which",                                   /* application name */
              0x00010000,                            /* application revision */
              0x00010100,           /* minimum required revision of PHSTOOLS */
              0x0001FFFF,        /* maximum recommended revision of PHSTOOLS */
              NULL,                                                /* Remark */
              NULL);                                 /* additional copyright */
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
  int rc;                                                    /* R갷kgabewert */

  memset (&Options,                      /* initialize the global structures */
          0,
          sizeof(Options));

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

  /* do it */
  rc = scanPath();
  if (rc != NO_ERROR)
      ToolsErrorDos(rc);

  return (rc);
}
