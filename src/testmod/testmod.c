/*****************************************************
 * Testmod Tool.                                    *
 * Just creates a number of files.                   *
 * (c) 2000 Patrick Haller Systemtechnik             *
 *****************************************************/

/* #define DEBUG 1 */

/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#define INCL_DOSFILEMGR
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

#define MAXPATHLEN 260


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsInteractive;                  /* start the process interactively */
  ARGFLAG fsModuleList;               /* Filename containing the module list */

  PSZ pszModuleList;                  /* Filename containing the module list */
} OPTIONS, *POPTIONS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */

ARGUMENT TabArguments[] =
{ /*Token---------Beschreibung----------------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/LIST=",      "Module list filename",     &Options.pszModuleList, ARG_PSZ,      &Options.fsModuleList},
  {"/I",          "Enable interactive console",NULL,                ARG_NULL,       &Options.fsInteractive},
  {"/?",          "Get help screen.",         NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",          "Get help screen.",         NULL,                 ARG_NULL,       &Options.fsHelp},
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
  TOOLVERSION("Testmod",                               /* application name */
              0x00010000,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


/***********************************************************************
 * Name      : APIRET LoadModule
 * Funktion  : Load the specified module
 * Parameter : PSZ pszModuleName
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 2000/12/22]
 ***********************************************************************/

APIRET LoadModule(PSZ pszModuleName,
                  PHMODULE phModule)
{
  APIRET rc;                                        /* operation return code */
  char pszBuffer[512];              /* module name buffer in case of failure */
  HMODULE hModule;                   /* load result buffer for module handle */

  if (NULL == phModule)                 /* check if module handle is desired */
    phModule = &hModule;       /* point to local buffer, forget after return */

  pszBuffer[0] = 0;              /* prevent previous results from disturbing */
  rc = DosLoadModule(pszBuffer,
                     sizeof(pszBuffer),
                     pszModuleName,
                     phModule);
  if (rc != NO_ERROR)
  {
    if (pszBuffer[0] != 0)
      ToolsErrorDosEx(rc,                           /* display error message */
                      pszBuffer);
    else
      ToolsErrorDos(rc);
  }

  return rc;                                                           /* OK */
}


/***********************************************************************
 * Name      : APIRET LoadModulesFromListFile
 * Funktion  : This procedure loads several modules specified in a list file
 * Parameter : PSZ pszModuleList
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 2000/12/22]
 ***********************************************************************/

APIRET LoadModulesFromListFile(PSZ pszModuleList)
{
  PSZ  pszLine;                                   /* returned line from file */
  int  rc;                                          /* operation return code */
  char pszLineBuffer[512];                             /* static line buffer */
  FILE *fileList = fopen(pszModuleList, "r");

  // check if file could be opened
  if (NULL == fileList)
    return ERROR_FILE_NOT_FOUND;

  // read lines until end of file
  for(;;)
  {
    // read line from file
    pszLine = fgets(pszLineBuffer,
                    sizeof(pszLineBuffer),
                    fileList);
    // done ?
    if (NULL == pszLine)
      break;
    else
    {
      // remove trailing CRLF
      StrTrim(pszLine);

      // load the specified module
      rc = LoadModule(pszLine, NULL);
      if (rc == NO_ERROR)                                  /* check for errors */
        printf("%s : loaded\n",
               pszLine);
      else
        printf("%s : failed with error %u.\n",
               pszLine,
               rc);
    }
  }

  return NO_ERROR; /* OK */
}


/***********************************************************************
 * Name      : ULONG QueryFreeMemory
 * Funktion  : Determine currently free memory total
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 2000/12/22]
 ***********************************************************************/

ULONG QueryFreeMemory(void)
{
  APIRET rc;                                        /* operation return code */
  ULONG  ulFreeMemory = 0;

  rc = DosQuerySysInfo(QSV_TOTAVAILMEM,
                       QSV_TOTAVAILMEM,
                       &ulFreeMemory,
                       sizeof(ulFreeMemory) );
  if (rc != NO_ERROR)
    ToolsErrorDosEx(rc,
                    "QueryDosSystemInfo(QSV_TOTAVAILMEM)");

  return ulFreeMemory;
}


/***********************************************************************
 * Name      : APIRET LoadModulePerf
 * Funktion  : Load the specified module with some profiling
 * Parameter : PSZ pszModuleName
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 2000/12/22]
 ***********************************************************************/

APIRET LoadModulePerf(PSZ pszModuleName,
                      PHMODULE phModule)
{
  PERFSTRUCT TS_Start;
  PERFSTRUCT TS_End;
  ULONG      ulFreeStart;
  ULONG      ulFreeEnd;
  double     seconds;
  APIRET     rc;                                  /* operation return code */

  printf ("Loading %s ...\n",
          pszModuleName);

  ulFreeStart = QueryFreeMemory();
  ToolsPerfQuery (&TS_Start);                    /* exact time measurement */

  rc = LoadModule(pszModuleName,
                  phModule);

  ToolsPerfQuery (&TS_End);                      /* exact time measurement */
  seconds = TS_End.fSeconds - TS_Start.fSeconds;     /* calculate duration */
  ulFreeEnd = QueryFreeMemory();

  printf("%u kb used, %11.6f ms\n",                  /* print report */
         (ulFreeStart - ulFreeEnd) / 1024,
         seconds * 1000.0);

  return rc;                                                         /* OK */
}


/***********************************************************************
 * Name      : APIRET InteractiveConsole
 * Funktion  : The console procedure
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 2000/12/22]
 ***********************************************************************/

APIRET InteractiveConsole(void)
{
  BOOL flagTerminate = FALSE;
  PSZ  pszLine;                                   /* returned line from file */
  int  rc;                                          /* operation return code */
  char pszLineBuffer[2048];                            /* static line buffer */


  while (flagTerminate == FALSE)
  {
    printf(">");                                       /* this is our prompt */

    pszLine = gets(pszLineBuffer);               /* now get the command line */
    StrTrim(pszLine);                            /* remove excess characters */

    /* our simple command dispatcher */
    if (strnicmp("LOAD", pszLine, 4) == 0)
    {
      PSZ pszModule = pszLine + 5;
      StrTrim(pszModule);

      /* load the module with some profiling */
      rc = LoadModulePerf(pszModule, NULL);
    }
    else if (strnicmp("QUIT", pszLine, 4) == 0)
    {
      flagTerminate = TRUE;
    }
    else if (strnicmp("EXIT", pszLine, 4) == 0)
    {
      flagTerminate = TRUE;
    }
    else if (strnicmp("HELP", pszLine, 4) == 0)
    {
      printf("\nKnown commands:\n"
             "  LOAD [module name]\n"
             "  HELP\n"
             "  QUIT, EXIT\n");
    }

    // ...

    else
      printf("Unknown command.\n");

  }

  return NO_ERROR;
}


/***********************************************************************
 * Name      : void Run
 * Funktion  : Main procedure
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 2000/12/22]
 ***********************************************************************/

int Run(void)
{
  APIRET rc;                                        /* operation return code */

  // check if we've got to load modules from a specified list file
  // which contains fully-qualified file names.
  if (Options.fsModuleList)
  {
    printf("Loading modules from list %s\n",
           Options.pszModuleList);
    rc = LoadModulesFromListFile(Options.pszModuleList);
    if (rc != NO_ERROR)                                  /* check for errors */
      return rc;
  }

  // if requested, we've got to open a console processor
  if (Options.fsInteractive)
  {
    printf("Console active.\n");

    rc = InteractiveConsole();
  }
  else
    if (Options.fsModuleList)
    {
      // enter daemon mode.
      printf("Daemon mode. Press Ctrl-C to terminate.\n");

      DosSleep(0xFFFFFFFF);
    }
    else
      // if nothing is loaded, we can terminate.
      printf("Nothing to do. Terminating.\n");

  return NO_ERROR;
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

  if (Options.fsHelp)                    /* check if help is to be displayed */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  rc = Run();
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  return (rc);
}
