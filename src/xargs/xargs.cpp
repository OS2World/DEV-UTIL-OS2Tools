/***********************************************************************
 * Name      : XArgs
 * Funktion  : Call client processes with specified arguments and
 *             additional parameter read in from stdin / file
 *
 * Autor     : Patrick Haller [2002-08-17]
 ***********************************************************************/

/* To Do
   - placeholder variables for special parameters:
   * date, time, timestamp
   * sequential number
   * unique number
   * unique temporary file
   * ... ?
   - parameter delimiter character (CRLF default, i.e. quote-parser optional)
 */


#ifdef DEBUG
#define DEBUGPRINT(a) printf("DEBUG: "); printf(a);
#else
#define DEBUGPRINT(a)
#endif


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
#include <process.h>

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
  ARGFLAG fsFileInput;                 /* user specified the input file name */
  ARGFLAG fsAsync;                  /* start client processes asynchornously */
  ARGFLAG fsVerbose;                                       /* verbose output */
  ARGFLAG fsCommand;                 /* the command to execute was specified */

  PSZ   pszFileInput;                          /* this is the input filename */
  PSZ   pszCommand;                                /* the command to execute */

  // @@@PH Limit for max number of active child processes
  // @@@PH wait until last child has terminated
} OPTIONS, *POPTIONS;



typedef struct
{
  ULONG ulClientsActive;        /* current number of active client processes */
  ULONG ulClientsTotal;          /* total number of started client processes */

  PERFSTRUCT   psStart;                       /* for performance measurement */
  PERFSTRUCT   psEnd;
  double       dTimeTotal;                   /* total time needed in seconds */
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
  {"/V",         "Verbose output.",         NULL,                  ARG_NULL |
                                                                   ARG_HIDDEN,     &Options.fsVerbose},
  {"/VERBOSE",   "Verbose output.",         NULL,                  ARG_NULL,       &Options.fsVerbose},
  {"/A",         "Start client processes asynchronously.", NULL,   ARG_NULL,       &Options.fsAsync},
  {"/IN=",       "Input file to read arguments from",    &Options.pszFileInput, ARG_NULL, &Options.fsFileInput},
  {"@",          "Input file to read arguments from",    &Options.pszFileInput, ARG_NULL, &Options.fsFileInput},
  {"1",          "Command to execute (in quotes)", &Options.pszCommand, ARG_PSZ  |
                                                                   ARG_MUST |
                                                                   ARG_DEFAULT,    &Options.fsCommand},
  ARG_TERMINATE
};


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/
void   help              (void);

int    main              (int,
                          char **);


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
  TOOLVERSION("XArgs",                                   /* application name */
              0x00010000,                            /* application revision */
              0x00010100,           /* minimum required revision of PHSTOOLS */
              0x0001FFFF,        /* maximum recommended revision of PHSTOOLS */
              NULL,                                                /* Remark */
              NULL);                                 /* additional copyright */
}


/***********************************************************************
 * Name      : APIRET xargsStartClient
 * Funktion  : start client processes
 * Parameter : PSZ pszLine
 * Variablen :
 * Ergebnis  : APIRET rc
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-08-17]
 ***********************************************************************/

APIRET xargsStartClient( PSZ pszCommand )
{
    APIRET      rc = NO_ERROR;
    RESULTCODES rcCodes;
    CHAR        szObjBuffer[ 260 ];
    ULONG       ulFlags;

    DEBUGPRINT(("xargsStartClient(%s)\n",
               pszCommand));


    // start client process
    ulFlags = 0;

    if (Options.fsAsync)
        ulFlags |= EXEC_ASYNCRESULT;
    else
        ulFlags |= EXEC_SYNC;

    rc = DosExecPgm(szObjBuffer,
                    sizeof( szObjBuffer ),
                    ulFlags,
                    pszCommand,
                    NULL, // inherit environment
                    &rcCodes,
                    pszProgram);

    if (Options.fsVerbose)
        if (NO_ERROR == rc)
            printf("Started (PID %d) %s\n",
                   rcCodes.codeTerminate,
                   pszCommand);

    if (NO_ERROR != rc)
    {
        printf("Failed to start %s\n",
               pszCommand);
        ToolsErrorDosEx(rc,
                        szObjBuffer);
    }

    // eventually wait for the child
    // DosWaitChild() ...

    return rc;
}


/***********************************************************************
 * Name      : APIRET xargsLine
 * Funktion  : extract parameters from the argument line and start
 *             client processes
 * Parameter : PSZ pszLine
 * Variablen :
 * Ergebnis  : APIRET rc
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-08-17]
 ***********************************************************************/

#define BUF_SIZE 8192

APIRET xargsLine( PSZ pszLine )
{
    APIRET rc = NO_ERROR;

    DEBUGPRINT(("xargsLine(%s)\n",
               pszLine));


    // @@@PH
    // do some advanced line parsing (quotes, backslashes, etc)

    // split the argument strings
    PSZ pszBuf = (PSZ) malloc( BUF_SIZE );
    if (NULL == pszBuf)
        return ERROR_NOT_ENOUGH_MEMORY;

    // build buffer string
    strncpy(pszBuf,
            Options.pszCommand,
            BUF_SIZE);
    strcat(pszBuf,
           " ");
    strcat(pszBuf,
           pszLine);

    // add additional trailing zero to terminate
    // the argument string
    pszBuf[ strlen( pszBuf ) + 1 ] = 0;

    // separate the argument strings



    // start client process
    rc = xargsStartClient( pszBuf );

    return rc;
}


/***********************************************************************
 * Name      : APIRET xargs
 * Funktion  : get a parameter and start a client process
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-08-17]
 ***********************************************************************/

APIRET xargs(void)
{
    APIRET rc = NO_ERROR;
    PSZ    pszLineBuffer = (PSZ)malloc( BUF_SIZE );
    PSZ    pszLine;

    // check buffer allocation
    if (NULL == pszLineBuffer)
        return ERROR_NOT_ENOUGH_MEMORY;

    // open the input file to read arguments from
    FILE* fileInput = stdin;

    if (Options.fsFileInput)
    {
        fileInput = fopen(Options.pszFileInput, "r");
        if (NULL == fileInput)
        {
            ToolsErrorDosEx(ERROR_OPEN_FAILED,
                            Options.pszFileInput);
            return ERROR_OPEN_FAILED;
        }
    }


    // parameter parser loop
    while (!feof( fileInput ) )
    {
        // read in the next parameter
        pszLine = fgets(pszLineBuffer, BUF_SIZE, fileInput);
        if (NULL != pszLine)
        {
            // extract parameter
            // start client proces
            rc = xargsLine( pszLine );
            if (NO_ERROR != rc)
                break;
        }
        else
            break;
    }

    // close the input file
    if (fileInput != stdin)
        fclose(stdin);

    // free line buffer memory
    free( pszLineBuffer );

    return rc;
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



  ToolsPerfQuery(&Globals.psStart);                          /* benchmarking */

  rc = xargs();

  ToolsPerfQuery(&Globals.psEnd);                            /* benchmarking */

  if (Options.fsVerbose)
  {
      Globals.dTimeTotal = Globals.psEnd.fSeconds - Globals.psStart.fSeconds;

      printf ("\ntotal %10.3f sec",
              Globals.dTimeTotal);
  }

  if (rc != NO_ERROR)
    ToolsErrorDos(rc);                                /* yield error message */

  return (rc);
}
