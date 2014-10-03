/***********************************************************************
 * Name      : CHKPATH.C
 * Funktion  : PrÅfung einiger Environmentvariablen.
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 12.01.1996 09.54.09]
 ***********************************************************************/

/*===Includes================================================================*/

#ifdef __OS2__
  #define INCL_NOPMAPI
  #define INCL_DOS
  #define INCL_DOSERRORS                              /* DOS error values    */
  #define INCL_DOSFILEMGR                             /* File Manager values */

  #include <os2.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <stdarg.h>


  #include "tooltypes.h"
  #include "tools.h"
  #include "toollog.h"
  #include "toolarg.h"
  #include "chkpath.h"
#endif

/*===End Includes============================================================*/


//#define DEBUG 0

#define DisplayText ChkPrintf
#define MAXPATHLEN  260

/*===Prototypen==============================================================*/
APIRET    querySystemLIBPATH(PSZ   pszBuffer,
                             ULONG ulBufferLength);
void      ChkPrintf (PSZ   pszFormat,
                     ...);
PSZ       EnvGetToken         (PSTATEMENT pStatement);
PPATHLIST ListElementAlloc    (PSZ        pszName,
                PSZ        pszFullName,
                ULONG      cbFile,
                FDATE      fdateLastWrite,
                FTIME      ftimeLastWrite);
PPATHLIST ListAdd             (PPATHLIST  *ppPathList,
                PSZ        pszName,
                PSZ        pszFullName,
                ULONG      cbFile,
                FDATE      fdateLastWrite,
                FTIME      ftimeLastWrite);
APIRET    ReadDirectoryToList (PSTATEMENT pStatement,
                PSZ        pszPath,
                PPATHLIST *ppPathList);
VOID      CleanDirectoryName  (PSZ        pszDirectory);
BOOL      CheckFileExtensions (PSZ        pszFilename,
                PSZ        pszExtensions);
APIRET    TokenProcess        (PSTATEMENT pStatement);
APIRET    TokenCheckFile      (PSTATEMENT pStatement,
                PSZ        pszValue);
APIRET    TokenCheckDir       (PSTATEMENT pStatement,
                PSZ        pszValue);
APIRET    TokenCheckFiles     (PSTATEMENT pStatement,
                PSZ        pszValue);
APIRET    TokenCheckDirs      (PSTATEMENT pStatement,
                PSZ        pszValue);
VOID      DisplayListDupes    (PSTATEMENT pStatement,
                PPATHLIST  pPathList);
VOID      DisplayListForeign  (PSTATEMENT pStatement,
                PPATHLIST  pPathList);
VOID      DisplayListOK       (PSTATEMENT pStatement,
                PPATHLIST  pPathList);
APIRET    ProcessEnvironment  (PSTATEMENT pStatementTable);
int       main                (int        argc,
                char      *argv[]);
/*===End Prototypen==========================================================*/


/*===Globale Strukturen======================================================*/
STATEMENT pTableStatements[] =
{ /*-Flag---Must--Value---Token---Specific-Extents---Remark---*/
  {SF_DIRS, TRUE, NULL,   "PATH",        ".EXE,.COM,.BAT,.CMD","OS/2 looks for your executables here."},
  {SF_DIRS, TRUE, NULL,   "DPATH",       ""                   ,"OS/2 looks for your data files here."},
  {SF_DIRS, FALSE,NULL,   "LIBPATH",     ".DLL"               ,"OS/2 looks for your DLLs here.", SFFN_LIBPATH},
  {SF_DIRS, FALSE,NULL,   "HELP",        ".HLP"               ,"Help Manager looks for your help files here."},
  {SF_DIRS, FALSE,NULL,   "BOOKSHELF",   ".INF"               ,"Help Manager looks for your INF-files here."},
  {SF_FILES,TRUE, NULL,   "SOMIR",       ".IR"                ,"SOM looks for your SOM Repositories here."},
  {SF_DIR,  FALSE,NULL,   "TMPDIR",      ""                   ,"Some programs store temporary files here."},
  {SF_DIR,  FALSE,NULL,   "TMP",         ""                   ,"Some programs store temporary files here."},
  {SF_DIR,  FALSE,NULL,   "TEMP",        ""                   ,"Some programs store temporary files here."},
  {SF_DIR,  TRUE, NULL,   "SOMDDIR",     ""                   ,"SOM (SOMDD) stores vital files here."},
  {SF_DIR,  FALSE,NULL,   "MMBASE",      ""                   ,"Multimedia Manager OS/2 uses this directory."},
  {SF_DIR,  FALSE,NULL,   "DSPPATH",     ""                   ,"Multimedia Manager OS/2 uses this directory."},
  {SF_DIR,  FALSE,NULL,   "ETC",         ""                   ,"TCP/IP Stacks use this path to their configs."},
  {SF_FILE, TRUE, NULL,   "USER_INI",    ".INI"               ,"OS/2 uses this for OS2.INI (User INI)."},
  {SF_FILE, TRUE, NULL,   "SYSTEM_INI",  ".INI"               ,"OS/2 uses this for OS2SYS.INI (System INI)."},
  {SF_FILE, TRUE, NULL,   "RUNWORKPLACE",".EXE"               ,"This is used for the Workplace Process."},
  {SF_FILE, TRUE, NULL,   "COMSPEC",     ".EXE"               ,"This is the command processor."},
  {SF_DIR,  FALSE,NULL,   "EPMPATH",""                        ,"EPM: Path"},
  {SF_DIR,  FALSE,NULL,   "GLOSSARY",    ""                   ,"Glossary"},
  {SF_DIR,  FALSE,NULL,   "IBMI18N",     ""                   ,"IBM Internationalization Package (I18N)"},
  {SF_DIRS, FALSE,NULL,   "LOCPATH",     ""                   ,"IBM Internationalization Locales"},
  {SF_DIRS, FALSE,NULL,   "ULSPATH",     ""                   ,"IBM Internationalization Language"},
  {SF_FILE, TRUE, NULL,   "OS2_SHELL",   ".EXE"               ,"OS/2 Shell"},
  {SF_DIRS, FALSE,NULL,   "UMPATH",      ""                   ,"Ultimedia Data Path"},
  {SF_DIRS, FALSE,NULL,   "VIEWER",      ""                   ,"Multimedia Viewers"},
  {SF_DIR,  FALSE,NULL,   "WORKPLAC",    ""                   ,"Multimedia WPS Path"},

  {SF_DIR,  FALSE,NULL,   "CAT_HOST_BIN_PATH",""              ,"ICAT Kernel Debugger Programs"},
  {SF_DIR,  FALSE,NULL,   "CAT_HOST_SOURCE_PATH",""           ,"ICAT Kernel Debugger Sources"},
  {SF_DIR,  FALSE,NULL,   "CAT_LOCAL_PATH",""                 ,"ICAT Kernel Debugger Local Path"},
  {SF_DIR,  FALSE,NULL,   "CAT_PATH",""                       ,"ICAT Kernel Debugger"},

  {SF_FILES,FALSE,NULL,   "CLASSPATH",".ZIP"                  ,"Java: Classes"},
  {SF_DIR,  FALSE,NULL,   "JAVA_HOME",   ""                   ,"Java: Home directory"},
  {SF_DIR,  FALSE,NULL,   "JAVA_USER",   ""                   ,"Java: User's directory"},
  {SF_DIR,  FALSE,NULL,   "JAVA_WEBLOGS",""                   ,"Java: Web logfiles directory"},

  {SF_DIRS, FALSE,NULL,   "CODELPATH",""                      ,"IBM VisualAge C++: Macros (Code)"},
  {SF_DIR,  FALSE,NULL,   "LPATH",""                          ,"IBM VisualAge C++: Macros"},
  {SF_DIR,  FALSE,NULL,   "CPPHELP_INI",""                    ,"IBM VisualAge C++: Path to INIs"},
  {SF_DIR,  FALSE,NULL,   "CPPLOCAL", ""                      ,"IBM VisualAge C++: Local"},
  {SF_DIR,  FALSE,NULL,   "CPPMAIN",""                        ,"IBM VisualAge C++: Main"},
  {SF_DIR,  FALSE,NULL,   "CPPWORK",""                        ,"IBM VisualAge C++: Working Directory"},
  {SF_DIR,  FALSE,NULL,   "IWFOPT",       ""                  ,"IBM VisualAge C++: Workframe"},
  {SF_DIRS, FALSE,NULL,   "VBPATH",       ""                  ,"IBM VIsualAge C++: Visual Builder"},

  {SF_DIRS, FALSE,NULL,   "SMINCLUDE",    ".H,.HPP,.HH,.C,.INC,.INL,.AVL,.PDL,.XH,.CPP","SOM Include Path"},
  {SF_DIR,  FALSE,NULL,   "SMTMP",        ""                  ,"SOM Temporary Path"},
  {SF_DIR,  FALSE,NULL,   "SOMBASE",      ""                  ,"SOM Base Path"},
  {SF_DIR,  FALSE,NULL,   "SOMDDIR",      ""                  ,"SOM DSOM Repository"},
  {SF_FILES,FALSE,NULL,   "SOMIR",        ".IR"               ,"SOM Implementation Repository"},
  {SF_DIR,  FALSE,NULL,   "SOMRUNTIME",   ""                  ,"SOM Runtime"},

  {SF_FILE, FALSE,NULL,   "DLSINI",""                         ,"LAN Services: NetGUI Ini"},
  {SF_DIR,  FALSE,NULL,   "LANINSTEP",    ""                  ,"LAN Services: Installation Source Path"},
  {SF_DIR,  FALSE,NULL,   "NWDBPATH",     ""                  ,"LAN Services: Network Database Path (DCDB)"},

  {SF_DIR,  FALSE,NULL,   "IBMWORKS_INI", ""                  ,"IBM Works Directory"},

  {SF_DIRS, FALSE,NULL,   "INCLUDE",      ".H,.HPP,.HH,.C,.INC,.INL,.AVL,.PDL,.XH,.CPP","C / Java Include Path"},
  {SF_DIRS, FALSE,NULL,   "LIB",          ".LIB,.DLL"                                  ,"C / Java Library Path"},
  {SF_DIR,  FALSE,NULL,   "IPFC",         ".APS,.RC,.DIC,.NLS","Information Presentation Facility Path"},

  {SF_NONE, FALSE,NULL,   NULL,       NULL                 ,NULL}
};

ULONG ulCheckError = 0;                                  /* PrÅfungsstatus */
ULONG ulCheckWarnings = 0;                               /* PrÅfungsstatus */

struct
{
  ARGFLAG fVerbose;                      /* AusfÅhrliche Meldungen ausgeben ? */
  ARGFLAG fConfig;                   /* Wurde die Boot-CONFIG.SYS angegeben ? */
  ARGFLAG fConfigAlternate;        /* Wurde eine extra CONFIG.SYS angegeben ? */
  ARGFLAG fHelp;                   /* Muss Hilfestellung dargestellt werden ? */
  ARGFLAG fDisplayOK;                              /* Display Correct Modules */
  ARGFLAG fDisplayForeign;                         /* Display Foreign Modules */
  ARGFLAG fDisplayDupes;                         /* Display duplicate Modules */
  ARGFLAG fDisplayLogo;                                /* Do not display logo */
  ARGFLAG fInformationExtended;        /* Erweiterte Informationen anzeigen ? */
  ARGFLAG fLogfile;                         /* indicates user wants a logfile */
  PSZ     pszConfig;                       /* Wert der angegebenen CONFIG.SYS */
  PSZ     pszLogfile;                                          /* logfilename */
} Options;


ARGUMENT TabArguments[] =
{ /*Token--Beschreibung--pTarget--ucTargetFormat--pTargetSpecified--*/
  {"/C=",   "Use specified CONFIG.SYS.",              &Options.pszConfig,ARG_PSZ,   &Options.fConfigAlternate},
  {"/C",    "Use booted CONFIG.SYS.",                 NULL,              ARG_NULL,  &Options.fConfig},
  {"/DO",   "Display correct modules.",               NULL,              ARG_NULL,  &Options.fDisplayOK},
  {"/DF",   "Display foreign modules.",               NULL,              ARG_NULL,  &Options.fDisplayForeign},
  {"/DD",   "Display duplicate modules.",             NULL,              ARG_NULL,  &Options.fDisplayDupes},
  {"/DI",   "Display extended information.",          NULL,              ARG_NULL,  &Options.fInformationExtended},
  {"/NL",   "Do not display program logo.",           NULL,              ARG_NULL,  &Options.fDisplayLogo},
  {"/V",    "Verbose mode, much output.",             NULL,              ARG_NULL,  &Options.fVerbose},
  {"/?",    "Get help screen.",                       NULL,              ARG_NULL,  &Options.fHelp},
  {"/H",    "Get help screen.",                       NULL,              ARG_NULL,  &Options.fHelp},
  {"/LOG=", "Write logfile also.",                    &Options.pszLogfile,ARG_PSZ,  &Options.fLogfile},
  ARG_TERMINATE
};
/*===End Globale Strukturen==================================================*/


/***********************************************************************
 * Name      : VOID DisplayHelp
 * Funktion  : Darstellen der Programmhilfe
 * Parameter : VOID
 * Variablen :
 * Ergebnis  : -
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 12.01.1996 10.36.34]
 ***********************************************************************/

VOID help (VOID)
{
  TOOLVERSION("CheckPath",                              /* application name */
              0x00010001,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,
              NULL);                                /* additional copyright */
  {
  PSTATEMENT pStatement;                               /* Tabelleniterator */

  DisplayText ("\nUsage: CHKPATH [Options] [/c=<CONFIG.SYS>]"
     "\n       /c=<CONFIG.SYS> You can even specify an alternate CONFIG.SYS which you"
     "\n                       want to have scanned."
     "\n"
     "\nThe following environment settings will be checked:"
     "\nToken        Files               Description"
     "\n------------------------------------------------------------------------------");

  for (pStatement = pTableStatements;  /* Alle Tokens der Tabelle ausgeben */
       pStatement->pszDescription != NULL;
       pStatement++)
    DisplayText ("\n%-12s %-19s %s",
       pStatement->pszToken,
       pStatement->pszExtensions,
            pStatement->pszDescription);
  }
}


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
 * Name      : void ChkPrintf
 * Funktion  : Print string and eventually redirect to logfile
 * Parameter : PSZ pszPrint
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 1997/10/28]
 ***********************************************************************/

void ChkPrintf (PSZ   pszFormat,
                ...)
{

  va_list ap;
  char    szBuf[512];      /* local buffer for the line to write to the file */
  PSZ     pszBufEnd;                           /* points to the buffer's end */

  pszBufEnd = szBuf;           /* point to the current end of the textbuffer */

  va_start(ap,
           pszFormat);                              /* Den Eintrag schreiben */

  vsprintf(pszBufEnd,
           pszFormat,
           ap);
  pszBufEnd = pszBufEnd + strlen(pszBufEnd);    /* points to the buffers end */

  va_end(ap);

  printf (szBuf);                                         /* write to screen */

  pszBufEnd = szBuf;                                /* remove \n from string */
  if (pszBufEnd[0] == 0x13)                                            /* CR */
     pszBufEnd++;
  if (pszBufEnd[0] == 0x10)                                            /* LF */
     pszBufEnd++;

  if (Options.fLogfile)                                         /* logfile ? */
    LogPrint(LOG_INFOS,
             "CHKP",
             pszBufEnd);
}


/***********************************************************************
 * Name      : PSZ EnvGetToken
 * Funktion  : Wert einer Variable aus dem Environment lesen.
 * Parameter : PSTATEMENT pStatement
 * Variablen :
 * Ergebnis  : Wert der Environmentvariable oder NULL.
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 12.01.1996 10.36.34]
 ***********************************************************************/

PSZ EnvGetToken (PSTATEMENT pStatement)
{
  APIRET rc = NO_ERROR;                                   /* Rueckgabewert */
  PSZ    pszTemp;         /* Lokale Variable fuer Rueckgabe des Env-Wertes */

  if ( pStatement == NULL )                     /* Parametereueberpruefung */
    return (NULL);                                 /* Fehler signalisieren */

  // check special functions
  switch(pStatement->sffn)
  {
    case SFFN_LIBPATH:
      {
        // obtain the libpath from the system
        #define LIBPATH_LENGTH 4096
        PSZ pszBuffer = (PSZ)malloc(LIBPATH_LENGTH);
        if (NULL == pszBuffer)
            return NULL;
  
        rc = querySystemLIBPATH(pszBuffer,
                                LIBPATH_LENGTH);
        if (NO_ERROR != rc)
            return NULL;
          
        // OK, note this is an intentional memory leak:
        // we cannot avoid to allocate the libpath buffer dynamically,
        // however there is no convenient place to free it again.
        pszTemp = pszBuffer;
        break;
      }

    case SFFN_NONE:
    default:
      // we've got to scan the environment
      rc = DosScanEnv (pStatement->pszToken,    /* OS/2 scans it's environment */
                       &pszTemp);
      if (rc == ERROR_ENVVAR_NOT_FOUND)           /* Variable has not been set */
          pszTemp = NULL;                              /* Fehler signalisieren */
  }

  return (pszTemp);                               /* Rueckgabewert liefern */
}


/***************************************************************************
 * Name      : PPATHLIST ListElementAlloc
 * Funktion  : Erzeugt ein Datenelement fuer die Liste.
 * Parameter : PSZ pszName, ULONG cbFile, FDATE fdateLastWrite, FTIME ftimeLastWrite
 * Variablen :
 * Ergebnis  : Zeiger auf die Struktur oder NULL im Fehlerfall.
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 13.01.1996 20.08.25]
 ***************************************************************************/

PPATHLIST ListElementAlloc (PSZ   pszName,
             PSZ   pszFullName,
             ULONG cbFile,
             FDATE fdateLastWrite,
             FTIME ftimeLastWrite)
{
  PPATHLIST pPathData;              /* Zeiger fuer einen Knoten im Datenbaum */

  if (pszName == NULL)                             /* Parameterueberpruefung */
    return ( (PPATHLIST) NULL );                     /* Fehler signalisieren */

  pPathData = (PPATHLIST)malloc (sizeof(PATHLIST));            /* Allokieren */
  if (pPathData != NULL)          /* Konnte der Speicher reserviert werden ? */
  {
    pPathData->pszName = strdup(pszName);             /* Speicher allokieren */
    if (pPathData->pszName != NULL)                  /* Fehler aufgetreten ? */
    {
      pPathData->pszFullName = strdup (pszFullName);
      if (pPathData->pszFullName != NULL)            /* Fehler aufgetreten ? */
      {  pPathData->pLeft          = NULL;         /* Strukturdaten auffÅllen */
   pPathData->pRight         = NULL;
   pPathData->pSame          = 0;
   pPathData->fdateLastWrite = fdateLastWrite;
   pPathData->ftimeLastWrite = ftimeLastWrite;
   pPathData->cbFile         = cbFile;
      }
      else
      {
   free (pPathData->pszName);                          /* Daten freigeben */
   free (pPathData);              /* Zuvor allokierten Speicher freigeben */
   return ( (PPATHLIST) NULL);                    /* Fehler signalisieren */
      }

    }
    else
    {
      free (pPathData);              /* Zuvor allokierten Speicher freigeben */
      return ( (PPATHLIST) NULL);                    /* Fehler signalisieren */
    }
  }

  return (pPathData);                               /* Zeiger zurueckliefern */
}


/***************************************************************************
 * Name      : PPATHLIST ListAdd
 * Funktion  : HÑngt eine Datenstruktur an den Baum an.
 * Parameter : PPATHLIST ,PSZ pszName, ULONG cbFile, FDATE fdateLastWrite, FTIME ftimeLastWrite
 * Variablen :
 * Ergebnis  : Zeiger auf die Struktur oder NULL im Fehlerfall.
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 13.01.1996 20.10.27]
 ***************************************************************************/

PPATHLIST ListAdd (PPATHLIST *ppPathList,
         PSZ       pszName,
         PSZ       pszFullName,
         ULONG     cbFile,
         FDATE     fdateLastWrite,
         FTIME     ftimeLastWrite){
  PPATHLIST *ppEntry;
  PPATHLIST pBufEntry;
  int       iResult;                 /* Ergebnis des Stringvergleiches */

  ppEntry = ppPathList;                      /* Startknoten des Baumes */

  while (*ppEntry != NULL)               /* Einen freien Knoten suchen */
  {
    iResult = stricmp ( (*ppEntry)->pszName, /* Dateinamen vergleichen */
         pszName);
    if (iResult > 0)
      ppEntry = (PPATHLIST *)&((*ppEntry)->pRight);     /* Rechter Ast */
    else
      if (iResult < 0)
   ppEntry = (PPATHLIST *)&((*ppEntry)->pLeft);     /* Linker Ast */
      else                        /* Datei gleichen Namens gefunden... */
      {
   pBufEntry = ListElementAlloc (pszName,   /* Neues Datenelement */
                  pszFullName,
                  cbFile,
                  fdateLastWrite,
                  ftimeLastWrite);
   if (pBufEntry != NULL)                          /* Fehler aufgetreten ? */
   {
     pBufEntry->pSame = (*ppEntry)->pSame;              /* Nein ! */
     (*ppEntry)->pSame = (PVOID)pBufEntry;
   }
   return (pBufEntry);                        /* Ergebnis liefern */
      }
  }

  *ppEntry = ListElementAlloc (pszName,          /* Neues Datenelement */
                pszFullName,
                cbFile,
                fdateLastWrite,
                ftimeLastWrite);
  return (*ppEntry);                               /* Ergebnis liefern */
}


/***************************************************************************
 * Name      : VOID ListFree
 * Funktion  : Gibt einen allokierten Baum frei.
 * Parameter : PPATHLIST pPathList
 * Variablen :
 * Ergebnis  :
 * Bemerkung : Rekursionstiefe koennte problematisch werden !
 *
 * Autor     : Patrick Haller [Samstag, 13.01.1996 20.10.27]
 ***************************************************************************/

VOID ListFree (PPATHLIST pPathList)
{
  if (pPathList == NULL)                        /* validate this one first */
     return;                                          /* abort immediately */

   if (pPathList->pLeft != NULL)                             /* Linker Ast */
     ListFree (pPathList->pLeft);

   if (pPathList->pRight != NULL)                           /* Rechter Ast */
     ListFree (pPathList->pRight);

   if (pPathList->pSame != NULL)                           /* Gleicher Ast */
     ListFree (pPathList->pSame);

   free (pPathList->pszName);       /* Nun den aktuellen Eintrag freigeben */
   free (pPathList);
}


/***************************************************************************
 * Name      : APIRET ReadDirectoryToList
 * Funktion  : Liest den Inhalt eines Verzeichnisses in eine verkettete Liste ein.
 * Parameter : PSZ pszPath, PPATHLIST pPathList
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 13.01.1996 19.00.37]
 ***************************************************************************/

APIRET ReadDirectoryToList (PSTATEMENT pStatement,
             PSZ        pszPath,
             PPATHLIST *ppPathList)
{
#define NFILES 2560
  APIRET       rc            = NO_ERROR;      /* Return code                 */
  HDIR         hDir          = HDIR_CREATE;         /* Directory-Scan handle */
  char         pszPathTemp[MAXPATHLEN];                   /* TemporÑrer Pfad */
  FILEFINDBUF3 *FFBuf3;                        /* Datenpuffer fÅr DosFindXXX */
  PVOID        pFindBuffer;                                /* Ergebnispuffer */
  ULONG        ulCount       = NFILES;        /* Scan for 256 files per call */


  if ( (pszPath    == NULL)  ||                      /* ParameterÅberprÅfung */
       (pStatement == NULL)  ||
       (ppPathList == NULL ) )
    return (ERROR_INVALID_PARAMETER);

  if (*pszPath == 0)                                 /* ParameterÅberprÅfung */
    return (ERROR_INVALID_PARAMETER);

                     /* Nun das Directory abscannen */
  strcpy (pszPathTemp,pszPath);
  if ( (pszPathTemp[strlen(pszPathTemp)-1] != '\\') &&  /* Slash anhaengen ? */
       (pszPathTemp[strlen(pszPathTemp)-1] != '/') )
    strcat (pszPathTemp,"\\*");
  else
    strcat (pszPathTemp,"*");

  pFindBuffer = malloc( sizeof(PVOID) * ulCount);       /* Puffer allokieren */
  if (pFindBuffer == NULL)                           /* Fehler aufgetreten ? */
    return (ERROR_NOT_ENOUGH_MEMORY);                /* Fehler signalisieren */

  FFBuf3 = (FILEFINDBUF3 *)pFindBuffer;                           /* Casting */
  rc = DosFindFirst(pszPathTemp,                                   /* Suchen */
          &hDir,
          FILE_NORMAL,
          FFBuf3,
          sizeof (PVOID) * ulCount,
          &ulCount,
          FIL_STANDARD);
#ifdef DEBUG
  printf ("\nReadDirectoryToList:DosFindFirst(%s)=%u",pszPath,rc);
#endif

  if (ulCount == 0)             /* Wurden eventuell keine Dateien gefunden ? */
    return (ERROR_NO_MORE_FILES);                    /* Fehler signalisieren */

  if (rc != NO_ERROR)                                /* Fehler aufgetreten ? */
    return (rc);                                     /* Fehler signalisieren */

  while (rc == NO_ERROR)                     /* Solange kein Fehler auftritt */
  {
    while (ulCount != 0)                          /* Alle Dateien abarbeiten */
    {
                      /* Gefundene Datei bearbeiten */
      strcpy (pszPathTemp, pszPath);      /* Kompletten Dateinamen ermitteln */
      strcat (pszPathTemp,"\\");
      strcat (pszPathTemp,FFBuf3->achName);
      ListAdd (ppPathList,         /* Gefundene Datei in die Liste eintragen */
          FFBuf3->achName,
          pszPathTemp,
          FFBuf3->cbFile,
          FFBuf3->fdateLastWrite,
          FFBuf3->ftimeLastWrite);
      //@@@TokenCheckFile (pStatement,pszPathTemp);               /* Datei prÅfen */

      ulCount--;
      FFBuf3 = (FILEFINDBUF3 *) ((BYTE*)FFBuf3 + FFBuf3->oNextEntryOffset);
    }
    ulCount   = NFILES;
    FFBuf3 = (FILEFINDBUF3 *)pFindBuffer;                         /* Casting */
    rc = DosFindNext (hDir,                                  /* Weitersuchen */
            FFBuf3,
            sizeof (PVOID) * ulCount,
            &ulCount);
#ifdef DEBUG
  printf ("\nReadDirectoryToList:DosFindNext(%s)=%u",pszPath,rc);
#endif
  }

  DosFindClose(hDir);                     /* Directory-Scan-Handle schlie·en */
  free(pFindBuffer);                                /* Find-Puffer freigeben */

  if (rc == ERROR_NO_MORE_FILES)                 /* Kein _wirklicher_ Fehler */
    rc = NO_ERROR;
  return (rc);                                       /* RÅckgabewert liefern */
#undef NFILES
} /* APIRET ReadDirectoryToList */


/***************************************************************************
 * Name      : VOID CleanDirectoryName
 * Funktion  : SÑubert den Namen eines Directorys, wenn ungÅltige Zeichen
 *             darin vorhanden sind.
 * Parameter : PSZ pszDirectory
 * Variablen :
 * Ergebnis  : -
 * Bemerkung : siehe SET MMBASE=D:\MMOS2;
 *
 * Autor     : Patrick Haller [Samstag, 13.01.1996 19.17.55]
 ***************************************************************************/

VOID CleanDirectoryName (PSZ pszDirectory)
{
  ULONG ulPos;

  if (pszDirectory == NULL)                          /* ParameterÅberprÅfung */
    return;

  ulPos = strlen(pszDirectory);                         /* LÑnge des Strings */
  if (pszDirectory[ulPos-1] == ';')                   /* Semikolon am Ende ? */
    pszDirectory[ulPos-1] = '\0';                        /* Dann terminieren */

} /* VOID CleanDirectoryName */


/***************************************************************************
 * Name      : VOID DisplayListDupes
 * Funktion  : Anzeigen der Liste mit doppelten Modulen
 * Parameter : PSTATEMENT pStatement, PPATHLIST pPathList
 * Variablen :
 * Ergebnis  : -
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 13.01.1996 22.22.46]
 ***************************************************************************/

VOID DisplayListDupesIterator (PSTATEMENT pStatement,
                PPATHLIST  pPathList,
                PULONG     pulCounter)
{
  PPATHLIST pIterator;                             /* Iterator ueber pSame */

  if (pPathList == NULL)                          /* validate this pointer */
     return;                                          /* abort immediately */

   if (pPathList->pLeft != NULL)                             /* Linker Ast */
     DisplayListDupesIterator (pStatement,pPathList->pLeft,pulCounter);

   if (pPathList->pRight != NULL)                           /* Rechter Ast */
     DisplayListDupesIterator (pStatement,pPathList->pRight,pulCounter);

   if (pPathList->pSame != NULL) /* Wenn es sich um mehrere Dateien handelt */
   {
     BOOL bOK;                                      /* GÅltigkeitsindikator */

     bOK = CheckFileExtensions (pPathList->pszName, pStatement->pszExtensions);

     for (pIterator = pPathList;              /* Anfang der Ausgabeschleife */
     pIterator != NULL;                            /* Abbruchkriterium */
     pIterator = pIterator->pSame)                  /* Naechstes Modul */
     {
       (*pulCounter)++;                                     /* Dupes zÑhlen */
       ulCheckWarnings++;                      /* Zaehlt als CONFIG-Warnung */

       if (Options.fDisplayDupes == TRUE) /* Sollen Dupes angezeigt werden ? */
       {
    DisplayText("\n      - %-40s %10u %02d/%02d/%d %02d:%02d",
       pIterator->pszFullName,
       pIterator->cbFile,
       pIterator->fdateLastWrite.month,           /* Month            */
       pIterator->fdateLastWrite.day,             /* Day              */
       (pIterator->fdateLastWrite.year+1980L),    /* Years since 1980 */
       pIterator->ftimeLastWrite.hours,           /* Hours            */
       pIterator->ftimeLastWrite.minutes);        /* Minutes          */
    if (bOK == FALSE)          /* Liegt keine gÅltige Dateiendung vor */
      DisplayText(" FF");
    else
      DisplayText(" OK");
   }
      }
     if (Options.fDisplayDupes == TRUE) /* Sollen Dupes angezeigt werden ? */
       DisplayText ("\n");                               /* Zeilenvorschub */
   }
}


/***************************************************************************
 * Name      : VOID DisplayListDupes
 * Funktion  : Anzeigen der Liste mit doppelten Modulen
 * Parameter : PSTATEMENT pStatement, PPATHLIST pPathList
 * Variablen :
 * Ergebnis  : -
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 13.01.1996 22.22.46]
 ***************************************************************************/

VOID DisplayListDupes (PSTATEMENT pStatement, PPATHLIST pPathList)
{
  ULONG ulDupeModules = 0;

  DisplayText ("\n    ˘ Dupe Modules in [%s]",pStatement->pszToken);
  DisplayListDupesIterator (pStatement, pPathList, &ulDupeModules);
  DisplayText ("\n    ˘ Dupe Check has found %u duplicate modules.",
          ulDupeModules);
} /* VOID DisplayListDupes */


/***************************************************************************
 * Name      : VOID DisplayListDupes
 * Funktion  : Anzeigen der Liste mit doppelten Modulen
 * Parameter : PSTATEMENT pStatement, PPATHLIST pPathList
 * Variablen :
 * Ergebnis  : -
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 13.01.1996 22.22.46]
 ***************************************************************************/

VOID DisplayListForeignIterator (PSTATEMENT pStatement,
             PPATHLIST  pPathList,
             PULONG     pulCounter)
{
  BOOL      bOK = FALSE;                           /* GÅltigkeitsindikator */

  if (pPathList == NULL)                          /* validate this pointer */
    return;                                           /* abort immediately */

  if (pPathList->pLeft != NULL)                             /* Linker Ast */
     DisplayListForeignIterator (pStatement,pPathList->pLeft,pulCounter);

   if (pPathList->pRight != NULL)                           /* Rechter Ast */
     DisplayListForeignIterator (pStatement,pPathList->pRight,pulCounter);

   if (pPathList->pSame != NULL) /* Wenn es sich um mehrere Dateien handelt */
     DisplayListForeignIterator (pStatement,pPathList->pSame,pulCounter);

                   /* GÅltigkeit der Datei prÅfen */
     bOK = CheckFileExtensions (pPathList->pszName,
            pStatement->pszExtensions);
     if (bOK == FALSE)          /* Wenn es sich um eine fremde Datei handelt */
     {
                /* Sollen fremde Module gezeigt werden */
       if (Options.fDisplayForeign == TRUE)
    DisplayText("\n      - %-40s %10u %02d/%02d/%d %02d:%02d FF",
       pPathList->pszFullName,
       pPathList->cbFile,
       pPathList->fdateLastWrite.month,           /* Month            */
       pPathList->fdateLastWrite.day,             /* Day              */
       (pPathList->fdateLastWrite.year+1980L),    /* Years since 1980 */
       pPathList->ftimeLastWrite.hours,           /* Hours            */
       pPathList->ftimeLastWrite.minutes);        /* Minutes          */
       (*pulCounter)++;                          /* Fremde Dateien zaehlen */
     }
}


/***************************************************************************
 * Name      : VOID DisplayListForeign
 * Funktion  : Anzeigen der Liste mit fremden Modulen
 * Parameter : PSTATEMENT pStatement, PPATHLIST pPathList
 * Variablen :
 * Ergebnis  : -
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 13.01.1996 22.22.46]
 ***************************************************************************/

VOID DisplayListForeign (PSTATEMENT pStatement, PPATHLIST pPathList)
{
  ULONG ulForeignModules = 0;

  DisplayText ("\n    ˘ Foreign Modules in [%s]",pStatement->pszToken);
  DisplayListForeignIterator (pStatement, pPathList, &ulForeignModules);
  DisplayText ("\n    ˘ Foreign Check has found %u foreign modules.",
          ulForeignModules);
} /* VOID DisplayListForeign */


/***************************************************************************
 * Name      : VOID DisplayListDupes
 * Funktion  : Anzeigen der Liste mit doppelten Modulen
 * Parameter : PSTATEMENT pStatement, PPATHLIST pPathList
 * Variablen :
 * Ergebnis  : -
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 13.01.1996 22.22.46]
 ***************************************************************************/

VOID DisplayListOKIterator (PSTATEMENT pStatement,
             PPATHLIST  pPathList,
             PULONG     pulCounter)
{
  BOOL      bOK = FALSE;                           /* GÅltigkeitsindikator */

  if (pPathList == NULL)                          /* validate this pointer */
     return;                                          /* abort immediately */

   if (pPathList->pLeft != NULL)                             /* Linker Ast */
     DisplayListOKIterator (pStatement,pPathList->pLeft,pulCounter);

   if (pPathList->pRight != NULL)                           /* Rechter Ast */
     DisplayListOKIterator (pStatement,pPathList->pRight,pulCounter);

   if (pPathList->pSame != NULL) /* Wenn es sich um mehrere Dateien handelt */
     DisplayListOKIterator (pStatement,pPathList->pSame,pulCounter);

                   /* GÅltigkeit der Datei prÅfen */
   bOK = CheckFileExtensions (pPathList->pszName,
               pStatement->pszExtensions);
   if (bOK == TRUE)          /* Wenn es sich um keine fremde Datei handelt */
   {
              /* Sollen korrekte Module gezeigt werden */
       if (Options.fDisplayOK == TRUE)
    DisplayText("\n      - %-40s %10u %02d/%02d/%d %02d:%02d OK",
       pPathList->pszFullName,
       pPathList->cbFile,
       pPathList->fdateLastWrite.month,           /* Month            */
       pPathList->fdateLastWrite.day,             /* Day              */
       (pPathList->fdateLastWrite.year+1980L),    /* Years since 1980 */
       pPathList->ftimeLastWrite.hours,           /* Hours            */
       pPathList->ftimeLastWrite.minutes);        /* Minutes          */
       (*pulCounter)++;                          /* Fremde Dateien zaehlen */
    }
}

VOID DisplayListOK (PSTATEMENT pStatement, PPATHLIST pPathList)
{
  ULONG ulOKModules = 0;

  DisplayText ("\n    ˘ Correct Modules in [%s]",pStatement->pszToken);
  DisplayListOKIterator (pStatement, pPathList, &ulOKModules);
  DisplayText ("\n    ˘ Correct Check has found %u correct modules.",
          ulOKModules);
} /* VOID DisplayListOK */

/***************************************************************************
 * Name      : APIRET TokenCheckFile
 * Funktion  : öberprÅfen des Statements Typ "eine Datei"
 * Parameter : PSZ pszValue
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 13.01.1996 15.39.00]
 ***************************************************************************/

APIRET TokenCheckFile (PSTATEMENT pStatement, PSZ pszValue)
{
  FILESTATUS3  fs3Info       = {{0}};         /* Buffer for file information */
  ULONG        ulBufSize     = sizeof(FILESTATUS3);  /* Size of above buffer */
  APIRET       rc            = NO_ERROR;      /* Return code                 */

  if (pStatement == NULL)                            /* ParameterÅberprÅfung */
    return (ERROR_INVALID_PARAMETER);
  if (pStatement->pszValue == NULL)                  /* ParameterÅberprÅfung */
    return (ERROR_INVALID_PARAMETER);

  if (*pStatement->pszValue == 0)                    /* ParameterÅberprÅfung */
    return (ERROR_INVALID_PARAMETER);

  if (pszValue == NULL)                             /* Parametermodifikation */
    pszValue = pStatement->pszValue;

  rc = DosQueryPathInfo(pszValue,                /* Handle of file           */
         FIL_STANDARD,     /* Request standard (Level 1) info */
         &fs3Info,            /* Buffer for file information  */
         ulBufSize);       /* Size of buffer                  */
#ifdef DEBUG
  printf ("\nTokenCheckFile:DosQueryPathInfo(%s)=%u",pszValue,rc);
#endif
  switch (rc)                                /* Ist ein Fehler aufgetreten ? */
  {
    case NO_ERROR:
      break;

    case ERROR_PATH_NOT_FOUND:
    case ERROR_FILE_NOT_FOUND:
      DisplayText("\n      - %s does not exist.", pszValue);
      return (rc);                                   /* Fehler signalisieren */

    default:
      DisplayText("DosQueryPathInfo error: return code = %u\n", rc);
      return (rc);
  }

                                         /* Soweit alles i.O. */
  if (Options.fInformationExtended == TRUE) /* Erweiterte Informationen ? */
  {
    DisplayText("\n      - %-40s %10u %02d/%02d/%d %02d:%02d",
       pszValue,
       fs3Info.cbFile,
       fs3Info.fdateLastWrite.month,                /* Month            */
       fs3Info.fdateLastWrite.day,                  /* Day              */
       (fs3Info.fdateLastWrite.year+1980L),         /* Years since 1980 */
       fs3Info.ftimeLastWrite.hours,                /* Hours            */
       fs3Info.ftimeLastWrite.minutes);             /* Minutes          */

   if (CheckFileExtensions (pszValue, pStatement->pszExtensions)
             == FALSE) /* Liegt keine gÅltige Dateiendung vor */
     DisplayText(" FF");
   else
     DisplayText(" OK");
  }

  return (rc);                                       /* RÅckgabewert liefern */
} /* APIRET TokenCheckFile */


/***************************************************************************
 * Name      : BOOL CheckFileExtensions
 * Funktion  : PrÅft die GÅltigkeit der Dateinamenserweiterung
 * Parameter : PSZ pszFilename, PSZ pszExtensionlist
 * Variablen :
 * Ergebnis  : TRUE / FALSE
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 13.01.1996 22.31.00]
 ***************************************************************************/

BOOL CheckFileExtensions (PSZ pszFilename, PSZ pszExtensions)
{
  PSZ          pszTemp;                               /* TemporÑre Variablen */
  PSZ          pszTemp2;                              /* TemporÑre Variablen */
  ULONG        ulTemp;                                /* TemporÑre Variablen */
  PSZ          pszTempExtension;
  BOOL         bExtensionFound = FALSE;            /* PrÅfung des Dateitypes */

  if (pszExtensions != NULL)                  /* Gehîrt die Datei dort hin ? */
    if (*pszExtensions != 0)
    {                                        /* OK, PrÅfung kann stattfinden */
      pszTempExtension = strdup(pszExtensions);                  /* kopieren */
      if (pszTempExtension != NULL)                           /* Soweit OK ? */
      {
   char pszBuffer[1024];                  /* Lokaler, temporÑrer Puffer */

   pszTemp = pszTempExtension;                       /* String zerlegen */
        /* Anmerkung: (Borlands) STRTOK-Implementation ist SCHEISSE !!! */
   ulTemp = strcspn(pszTemp,",;");      /* Nach dem ersten Komma suchen */
   if (ulTemp == 0)                          /* Kein Delimiter gefunden */
     ulTemp = strlen(pszTemp)+1;     /* Dann kompletten String benutzen */
   memcpy (pszBuffer,pszTemp,ulTemp);                 /* Kopie erzeugen */
   pszBuffer[ulTemp] = '\0';                    /* KÅnstlich teminieren */
   if (ulTemp != 0)                                 /* Token gefunden ? */
     pszTemp += ulTemp + 1;               /* Auf naechstes Token setzen */
   while ( (ulTemp != 0) &&                   /* Alle Tokens abarbeiten */
      (bExtensionFound == FALSE) )
   {

     pszTemp2 = pszFilename                              /* Vergleichen */
           - strlen(pszBuffer)
           + strlen(pszFilename);
     if (stricmp (pszBuffer, pszTemp2) == 0)             /* Endung OK ? */
       bExtensionFound = TRUE;          /* Jupp. Diese Datei ist gÅltig */

     ulTemp = strcspn(pszTemp,",;");    /* Nach dem ersten Komma suchen */
     memcpy (pszBuffer,pszTemp,ulTemp);               /* Kopie erzeugen */
     pszBuffer[ulTemp] = '\0';                  /* KÅnstlich teminieren */

     if (ulTemp != 0)                               /* Token gefunden ? */
       pszTemp += ulTemp + 1;             /* Auf naechstes Token setzen */
   }
   free (pszTempExtension);                   /* Kopie wieder freigeben */
      }
   }
   return (bExtensionFound);                         /* RÅckgabewert liefern */
} /* BOOL CheckFileExtensions */

/***************************************************************************
 * Name      : APIRET TokenCheckFiles
 * Funktion  : öberprÅfen des Statements Typ "mehrere Dateien"
 * Parameter : PSZ pszValue
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 13.01.1996 15.39.00]
 ***************************************************************************/

APIRET TokenCheckFiles (PSTATEMENT pStatement, PSZ pszValue)
{
  APIRET rc  = NO_ERROR;                               /* RÅckgabewert */
  APIRET rc2 = NO_ERROR;                               /* RÅckgabewert */
  PSZ    pszTemp;                                   /* String-Iterator */
  PSZ    pszVal;         /* TemporÑrer String fÅr pStatement->pszValue */

  if (pStatement == NULL)                      /* ParameterÅberprÅfung */
    return (ERROR_INVALID_PARAMETER);          /* Fehler signalisieren */

  if (pszValue == NULL)                       /* Parametermodifikation */
    pszValue = pStatement->pszValue;

  pszVal = strdup (pszValue);                        /* Kopie erzeugen */
  if (pszVal == NULL)                          /* Fehler aufgetreten ? */
    return (ERROR_NOT_ENOUGH_MEMORY);          /* Fehler signalisieren */

  pszTemp = strtok (pszVal,";,");                   /* String zerlegen */
  while (pszTemp != NULL)              /* Solange noch Angaben da sind */
  {                                         /* EinzelprÅfung vornehmen */
    rc2 = TokenCheckFile (pStatement,pszTemp);
    if (rc2 != NO_ERROR)                       /* Fehler aufgetreten ? */
      rc = rc2;                               /* Letzten Fehler merken */

    pszTemp = strtok (NULL,";,");                   /* String zerlegen */
  }

  free (pszVal);                      /* String-Kopie wieder freigeben */
  return (rc);                                 /* RÅckgabewert liefern */
} /* APIRET TokenCheckFiles */


/***************************************************************************
 * Name      : APIRET TokenCheckDir
 * Funktion  : öberprÅfen des Statements Typ "ein Verzeichnis"
 * Parameter : PSZ pszValue
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 13.01.1996 15.39.00]
 ***************************************************************************/

APIRET TokenCheckDir (PSTATEMENT pStatement, PSZ pszValue)
{
  APIRET       rc            = NO_ERROR;      /* Return code                 */
  FILESTATUS3  fs3Info       = {{0}};         /* Buffer for file information */
  ULONG        ulBufSize     = sizeof(FILESTATUS3);  /* Size of above buffer */
  PSZ          pszTemp;              /* TemporÑrer Puffer fÅr pszValue-kopie */


  if (pStatement == NULL)                      /* ParameterÅberprÅfung */
    return (ERROR_INVALID_PARAMETER);          /* Fehler signalisieren */

  if (pszValue == NULL)                       /* Parametermodifikation */
    pszValue = pStatement->pszValue;
  if (*pszValue == 0)                                /* ParameterÅberprÅfung */
    return (ERROR_INVALID_PARAMETER);

  pszTemp = strdup(pszValue);              /* pszValue kopieren, damit durch */
      /* Cleandirectoryname das Environment nicht veraendert wird. */
  CleanDirectoryName (pszTemp);               /* UngÅltige Zeichen entfernen */

  rc = DosQueryPathInfo(pszTemp,          /* Handle of file                  */
         FIL_STANDARD,     /* Request standard (Level 1) info */
         &fs3Info,            /* Buffer for file information  */
         ulBufSize);       /* Size of buffer                  */
#ifdef DEBUG
  printf ("\nTokenCheckDir:DosQueryPathInfo(%s)=%u",pszValue,rc);
#endif
  switch (rc)                                /* Ist ein Fehler aufgetreten ? */
  {
    case NO_ERROR:
      break;

    case ERROR_PATH_NOT_FOUND:
    case ERROR_FILE_NOT_FOUND:
      DisplayText("\n      - %s does not exist.", pszTemp);
      return (rc);                                   /* Fehler signalisieren */

    default:
      DisplayText("DosQueryPathInfo error: return code = %u\n", rc);
      return (rc);
  }

                                         /* Soweit alles i.O. */
  DisplayText("\n      - %-40s: (%02d/%02d/%d %02d:%02d)",
       pszTemp,
       fs3Info.fdateLastWrite.month,                /* Month            */
       fs3Info.fdateLastWrite.day,                  /* Day              */
       (fs3Info.fdateLastWrite.year+1980L),         /* Years since 1980 */
       fs3Info.ftimeLastWrite.hours,                /* Hours            */
       fs3Info.ftimeLastWrite.minutes);             /* Minutes          */

                     /* Nun das Directory abscannen */

  return (rc);                                       /* RÅckgabewert liefern */
} /* APIRET TokenCheckDir */


/***************************************************************************
 * Name      : APIRET TokenCheckDirs
 * Funktion  : öberprÅfen des Statements Typ "mehrere Dateien"
 * Parameter : PSZ pszValue
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 13.01.1996 15.39.00]
 ***************************************************************************/

APIRET TokenCheckDirs (PSTATEMENT pStatement, PSZ pszValue)
{
  APIRET    rc  = NO_ERROR;                            /* RÅckgabewert */
  APIRET    rc2 = NO_ERROR;                            /* RÅckgabewert */
  PSZ       pszTemp;                                /* String-Iterator */
  PSZ       pszVal;      /* TemporÑrer String fÅr pStatement->pszValue */
  PPATHLIST pPathList = NULL;          /* Liste der gefundenen Dateien */

  if (pStatement == NULL)                      /* ParameterÅberprÅfung */
    return (ERROR_INVALID_PARAMETER);          /* Fehler signalisieren */

  if (pszValue == NULL)                       /* Parametermodifikation */
    pszValue = pStatement->pszValue;

  pszVal = strdup (pszValue);                        /* Kopie erzeugen */
  if (pszVal == NULL)                          /* Fehler aufgetreten ? */
    return (ERROR_NOT_ENOUGH_MEMORY);          /* Fehler signalisieren */

  pszTemp = strtok (pszVal,";,");                   /* String zerlegen */
  while (pszTemp != NULL)              /* Solange noch Angaben da sind */
  {

    rc2 = TokenCheckDir (pStatement,pszTemp); /* EinzelprÅfung vornehmen */
    if (rc2 != NO_ERROR)                       /* Fehler aufgetreten ? */
      rc = rc2;                               /* Letzten Fehler merken */
    else
    {
      rc2 = ReadDirectoryToList(pStatement,
            pszTemp,
            &pPathList);      /* Directory scannen */
      if (rc2 == ERROR_NO_MORE_FILES)
      {
   DisplayText ("\n        ! %s is completely empty !",pszTemp);
   ulCheckWarnings++;              /* Konfiguration ist sub-optimal */
   rc2 = NO_ERROR;                  /* Ist kein wirklicher Fehler */
      }
                 /* kein ELSE hier. Ist Absicht :) */
      if (rc2 != NO_ERROR)                     /* Fehler aufgetreten ? */
   rc = rc2;                             /* Letzten Fehler merken */    }

    pszTemp = strtok (NULL,";,");                   /* String zerlegen */
  }

   /* Alle Directories sind nun gescannt. Wichtige Bearbeitung jetzt ! */
   /* Statistik getrennt nach folgenden Kriterien:
      + "hineingehîrende" Dateien
      + fremde Dateien
   */

  DisplayListDupes (pStatement,pPathList);         /* Dupes ausgeben */

  if (*(pStatement->pszExtensions) != 0)  /* Darf hier geprÅft werde ? */
  {
    DisplayListForeign (pStatement,pPathList);/* Fremde Dateien ausgeben */
    DisplayListOK (pStatement,pPathList);   /* Korrekte Dateien ausgeben */
  }

  ListFree (pPathList);                  /* Allokierte Liste freigeben */
  free (pszVal);                      /* String-Kopie wieder freigeben */
  return (rc);                                 /* RÅckgabewert liefern */
} /* APIRET TokenCheckDirs */


/***********************************************************************
 * Name      : APIRET TokenProcess
 * Funktion  : Bearbeitet ein Åbergebenes Token mit entspr. Datenwerten.
 * Parameter : PSTATEMENT pStatement
 * Variablen :
 * Ergebnis  : API-Returncode.
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 13.01.1996 14.08.52]
 ***********************************************************************/

APIRET TokenProcess (PSTATEMENT pStatement)
{
  APIRET rc = NO_ERROR;                                /* RÅckgabewert */

  if ( pStatement == NULL )                 /* Parametereueberpruefung */
    return (ERROR_INVALID_PARAMETER);          /* Fehler signalisieren */

  DisplayText ("\n  ˘ Processing Token [%s]:",pStatement->pszToken);

  if (pStatement->pszValue == NULL)          /* Hat Token einen Wert ? */
  {
    if (pStatement->bMustBeSet == FALSE)
      DisplayText ("\n    ˙ Token not specified or accessible, but is OK.");
    else
    {
      DisplayText ("\n    ˙ Token not specified, this is FATAL !");
      ulCheckError++;                            /* Fehler signalisieren */
    }
  }
  else                                                /* ansonsten ... */
  switch (pStatement->sFlag)      /* Tokens gemÑ· den Typen bearbeiten */
  {
    case SF_FILE:
      DisplayText ("\n    ˙ Token specifies ONE file.");
      rc = TokenCheckFile (pStatement,NULL);
      break;

    case SF_DIR:
      DisplayText ("\n    ˙ Token specifies ONE directory.");
      rc = TokenCheckDir (pStatement,NULL);
      break;
    case SF_FILES:
      DisplayText ("\n    ˙ Token specifies MULTIPLE files.");
      rc = TokenCheckFiles (pStatement,NULL);
      break;
    case SF_DIRS:
      DisplayText ("\n    ˙ Token specifies MULTIPLE directories.");
      rc = TokenCheckDirs (pStatement,NULL);
      break;
    case SF_NONE:
    default:
      DisplayText ("\n    ˙ no defined style.");
  }

  return (rc);                                 /* RÅckgabewert liefern */
} /* APIRET TokenProcess */


/***********************************************************************
 * Name      : APIRET ProcessEnvironment
 * Funktion  : Alle Tokens in der Tabelle/Environment abarbeiten
 * Parameter : PSTATEMENT pStatement
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung : Scannt die Variablen aus dem aktiven Environment,
 *             fÅr die CONFIG.SYS ist eine andere Funktion verantwortlich.
 *
 * Autor     : Patrick Haller [Freitag, 12.01.1996 10.36.34]
 ***************************************************************************/

APIRET ProcessEnvironment (PSTATEMENT pStatementTable)
{
  APIRET     rc = NO_ERROR;                               /* Rueckgabewert */
  PSTATEMENT pStatement;                               /* Tabelleniterator */

  if (pStatementTable == NULL)                   /* Parameterueberpruefung */
    return (ERROR_INVALID_PARAMETER);              /* Fehler signalisieren */

  DisplayText ("\n˛ Scanning environment:");

  for (pStatement = pStatementTable;      /* Tokens der Tabelle bearbeiten */
       pStatement->pszDescription != NULL;
       pStatement++)
  {
    pStatement->pszValue = EnvGetToken(pStatement);     /* Get Token Value */
    rc = TokenProcess (pStatement);                       /* Process Token */

    if (rc != NO_ERROR)   /* Ist ein Fehler / PrÅfungsfehler aufgetreten ? */
    {
      ulCheckError++;                              /* Globalen Status Ñndern */
      DisplayText ("\n  * Action reported error code #%u.",rc);
    }

   /*
     EnvGetToken (Token-Wert ermitteln)
     TokenSplit  (Mehrfach Tokens zerlegen)
            TokenCheck (GÅltigkeit prÅfen)
              * Pfade existieren
              * Dateien existieren
            In die Tabelle des entspr. Tokens eintragen wg Dupecheck
          TokenListDupecheck (Doppelte Namen,Versionen prÅfen)
        */
  }

  return (rc);                                    /* Rueckgabewert liefern */
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

int main(int argc, char *argv[])
{
  int rc;                                                    /* RÅckgabewert */

  memset(&Options,                                        /* Initialisierung */
         0,
         sizeof(Options));

  rc = ArgStandard (argc,                            /* CLI-Parameter parsen */
                    argv,
                    TabArguments,
                    &Options.fHelp);
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    exit(1);                                                /* abort program */
  }

  if ( Options.fHelp)                        /* check if user specified file */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  /* eventually open the logfile */
  if (Options.fLogfile)
  {
    rc = LogOpen(LOG_ALL,                                   /* open the file */
                 Options.pszLogfile,
                 LOGMODE_CACHE);
    if (rc != NO_ERROR)
       ToolsErrorDosEx(rc,
                       "Opening logfile");
  }


   ProcessEnvironment(pTableStatements);

   /* Abschlie·end noch eine Wertung ausgeben. */
   DisplayText ("\n˛ Check Result:");
   if (ulCheckWarnings == 0)
      DisplayText ("\n  ˘ No warnings have been detected.");
    else
      DisplayText ("\n  ˘ %u warnings have been detected.",ulCheckWarnings);

    if (ulCheckError == 0)
      DisplayText ("\n  ˘ No errors have been detected.");
    else
      DisplayText ("\n  ˘ %u errors have been detected.",ulCheckError);

  /* eventually close the logfile */
  if (Options.fLogfile)
    LogClose();

  return (rc);
}
