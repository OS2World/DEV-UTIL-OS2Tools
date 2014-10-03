/*****************************************************
 * Shutdown tool                                     *
 * (c) 1992-95 Patrick Haller Systemtechnik          *
 *****************************************************/

/* To Do

2001-02-23 PH Make a hard reboot via direct 16-bit IOPL code
              after a specifyable time to prevent hangs during
              OS/2 shutdown operation.
*/

/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#ifdef __OS2__
  #define INCL_DOSFILEMGR
  #define INCL_BASE
  #define INCL_NOPMAPI
  #include <os2.h>
#endif

#ifdef _WIN32
  #include <windows.h>
#endif


#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

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
  ARGFLAG fsReboot;                         /* automatic reboot is requested */

  ARGFLAG fsMessage;         /* an additional shutdown message was specified */
  PSZ     pszMessage;        /* an additional shutdown message was specified */

  ARGFLAG fsTimeout;                           /* a timeout for the shutdown */
  ULONG   ulTimeout;                           /* a timeout for the shutdown */

  ARGFLAG fsAbortShutdown;          /* we've got to abort a pending shutdown */

#ifdef __OS2__
  ARGFLAG fsFlush;                    /* only flushing the filesystems cache */
#endif

#ifdef _WIN32
  ARGFLAG fsVerbose;                            /* verbose output is desired */

  ARGFLAG fsMachine;                       /* a remote machine was specified */
  PSZ     pszMachine;                      /* a remote machine was specified */

  ARGFLAG fsForceAppsClose;               /* force applications to be closed */
#endif

} OPTIONS, *POPTIONS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/
OPTIONS Options;                /* this structure holds command line options */

ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung-------------------pTarget-ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",            NULL,   ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",            NULL,   ARG_NULL,       &Options.fsHelp},
  {"/R",         "Reboot after shutdown.",      NULL,   ARG_NULL,       &Options.fsReboot},
  {"/TIME=",     "A timeout for the shutdown.", &Options.ulTimeout,  ARG_ULONG,   &Options.fsTimeout},
  {"/MSG=",      "Shutdown message to display.",&Options.pszMessage, ARG_PSZ,     &Options.fsMessage},
  {"/ABORT",     "Abort a pending shutdown.",   NULL,   ARG_NULL,       &Options.fsAbortShutdown},
#ifdef __OS2__
  {"/F",         "Only flush filesystem cache.",NULL,   ARG_NULL,       &Options.fsFlush},
#endif

#ifdef _WIN32
  {"/VERBOSE",   "Provide verbose output.",     NULL,   ARG_NULL,       &Options.fsVerbose},
  {"/APPS",      "Force applications to be closed.",NULL,ARG_NULL,      &Options.fsForceAppsClose},
  {"1",          "Remote machine name.",        &Options.pszMachine, ARG_PSZ |
                                                                     ARG_DEFAULT, &Options.fsMachine},
#endif

  ARG_TERMINATE
};



/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void   help                (void);

APIRET DosReboot           (void);

void   ErrorProc           (APIRET errc);

APIRET ShutdownProcess     (void);

int    main                (int,
                            char **);


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
#ifdef __OS2__
  TOOLVERSION("Shutdown",                               /* application name */
              0x00010007,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              "Warning: Workplace Shell will not be shut down.",  /* Remark */
              NULL);                                /* additional copyright */
#endif

#ifdef _WIN32
  TOOLVERSION("Shutdown",                               /* application name */
              0x00010007,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
#endif
}


#ifdef __OS2__
/***********************************************************************
 * Name      : void title
 * Funktion  : Darstellen des Titels
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

APIRET DosReboot (void)
{
  HFILE  hf;                                  /* handle to the device driver */
  ULONG  dummy;
  APIRET rc;                                               /* API returncode */

  rc = DosOpen("\\DEV\\DOS$",                      /* open the device driver */
               &hf,
               &dummy,
               0L,
               FILE_NORMAL,
               FILE_OPEN,
               OPEN_ACCESS_WRITEONLY |
               OPEN_SHARE_DENYNONE |
               OPEN_FLAGS_FAIL_ON_ERROR,
               NULL);
  if(rc == NO_ERROR)                                     /* check for errors */
  {
    printf ("\nShutdown...");

    rc = DosShutdown(0L);                /* sync on filesystems and shutdown */
    ErrorProc(rc);

     /* 32-Bit reboot */
    rc=DosDevIOCtl(hf,
                   0xd5,
                   0xab,
                   NULL,
                   0,
                   NULL,
                   NULL,
                   0,
                   NULL );
    if (rc)
      ToolsErrorDos(rc);

     /* old 16-bit reboot
     DosDevIOCtl(NULL, NULL, 0xab, 0xd5, hf);
     */

    DosClose(hf);
  }
  else
  {
    fprintf (stderr,
             "\nDOS.SYS not installed. Can't Reboot.");
    ToolsErrorDos(rc);
  }

  return (rc);
}


/***********************************************************************
 * Name      : void ErrorProc
 * Funktion  : process the returncode
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

void ErrorProc (APIRET errc)
{
  switch (errc)
  {
    case NO_ERROR:
         printf ("\nShutdown is complete. You may power off "
                 "or press Ctrl-Alt-Del now.\n");
         break;

    case ERROR_INVALID_PARAMETER:
         printf ("Shutdown error: invalid internal parameter occured...\n");
         break;

    case ERROR_ALREADY_SHUTDOWN:
         printf ("Shutdown error: the system was already shut down ?!?\n");
         break;

    default:
         printf ("Shutdown error: strange things are going on...\n"
                 "                Unidentified Error %li\n",errc);
         ToolsErrorDos(errc);
  }
}


/***********************************************************************
 * Name      : APIRET ShutdownProcess
 * Funktion  : this routine performs the actual shutdown
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

APIRET ShutdownProcess (void)
{
  APIRET rc;                                               /* API returncode */
  int    iSeconds;                             /* this is the second counter */
  PSZ    pszShutdown = "Shutdown";                       /* type of shutdown */
  time_t timeSystem;                              /* the current system time */
  time_t timeShutdown;                    /* time scheduled for the shutdown */
  CHAR   szTime[32];                           /* buffer for the time string */

  HEV    hevSemShutdown;                           /* the shutdown semaphore */
  PSZ    pszSemShutdown = "\\SEM32\\SYSTEM.SHUTDOWN";      /* semaphore name */

  if (Options.fsReboot)        pszShutdown = "Shutdown and reboot";
  if (Options.fsFlush)         pszShutdown = "Flushing filesystem caches";
  if (Options.fsAbortShutdown) pszShutdown = "Aborting shutdown";


  /***************************************************************************
   * print the shutdown message                                              *
   ***************************************************************************/

  if (Options.fsMessage)
    printf ("\n[%s]"
            "\n",
            Options.pszMessage);


  /***************************************************************************
   * handle the timeout for a scheduled shutdown                             *
   ***************************************************************************/

  rc = DosCreateEventSem(pszSemShutdown,
                         &hevSemShutdown,
                         0,               /* all named semaphores are shared */
                         0);                      /* initial semaphore state */
  switch (rc)
  {
    case NO_ERROR:
      break;

    case ERROR_DUPLICATE_NAME:                /* as semaphore already exists */
      hevSemShutdown = 0;               /* parameter to be initialized first */
      rc = DosOpenEventSem(pszSemShutdown,
                           &hevSemShutdown);
      if (rc != NO_ERROR)                                /* check for errors */
        return (rc);
      else
        break;

    default:
      return(rc);                                /* abort further processing */
  }                           /* now we should have a valid semaphore handle */


  if (Options.fsTimeout)
  {
    timeSystem = time(NULL);                    /* query current system time */
    timeShutdown = timeSystem + Options.ulTimeout;         /* scheduled time */
    strcpy(szTime,
           ctime(&timeShutdown));
    szTime[24] = 0;                                          /* cut the CRLF */


    for (iSeconds = Options.ulTimeout;
         iSeconds > 0;
         iSeconds--)
    {
      printf ("\r%s in %03uh %02um %02us. (%s) ",
              pszShutdown,
              iSeconds / 3600,
              iSeconds / 60 % 60,
              iSeconds % 60,
              szTime);

      rc = DosWaitEventSem(hevSemShutdown,         /* wait for the semaphore */
                           1000);
      switch (rc)
      {
        case NO_ERROR:       /* this means some process aborted the shutdown */
          printf ("\nShutdown aborted.");
          return (NO_ERROR);                                 /* OK, bail out */

        case ERROR_TIMEOUT:                             /* that's normal, OK */
                  break;

        default:
          return (rc);                                /* some other problems */
      }
    }
  }


  /***************************************************************************
   * abort pending shutdown on same machine                                  *
   ***************************************************************************/

  printf ("\n%s ...",
          pszShutdown);

  if (Options.fsAbortShutdown)
  {
    rc = DosPostEventSem(hevSemShutdown);              /* abort the shutdown */
    if (rc != NO_ERROR)
      ToolsErrorDos(rc);
    else
      printf ("\nShutdown aborted.");

    return (rc);                             /* OK, abort further processing */
  }

  DosCloseEventSem(hevSemShutdown);            /* close the semaphore handle */


  /***************************************************************************
   * reboot / shutdown / flushing filesystems                                *
   ***************************************************************************/

  if (Options.fsReboot)          /* user wants shutdown and automatic reboot */
    rc = DosReboot();
  else
    if (Options.fsFlush)              /* user wants only to flush the caches */
    {
      rc = DosShutdown(1);                /* only flushing filesystem caches */
      if (rc != NO_ERROR)                                /* check for errors */
        ToolsErrorDos(rc);
    }
    else
    {
      rc = DosShutdown(0);                   /* shutting down IFS and kernel */
      ErrorProc (rc);
    }

  return (rc);
}
#endif


#ifdef _WIN32
/***********************************************************************
 * Name      : APIRET ShutdownProcess
 * Funktion  : this routine performs the actual shutdown
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

APIRET ShutdownProcess (void)
{
  HANDLE           hToken;                        /* handle to process token */
  TOKEN_PRIVILEGES tkp;                           /* ptr. to token structure */

  if (Options.fsVerbose)
  {
    if (Options.fsAbortShutdown)
      printf ("\nAborting shutdown on %s.",
              Options.fsMachine ? Options.pszMachine : "<local machine>");
    else
    {
      printf("\nShutting down %s in %u seconds.",
             Options.fsMachine ? Options.pszMachine : "<local machine>",
             Options.ulTimeout);

      if (Options.fsMessage)
        printf ("\nMessage is [%s].",
                Options.pszMessage);

      if (Options.fsForceAppsClose)
        printf ("\nClosing applications automatically.");
      else
        printf ("\nAsking user to close applications.");

      printf ("\nAutomatic reboot %s.",
              Options.fsReboot ? "enabled" : "disabled");
    }
  }

   /* Get the current process token handle so we can get shutdown privilege. */
  if (!OpenProcessToken(GetCurrentProcess(),
                        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                        &hToken))
    return (GetLastError());                        /* raise error condition */

  if (LookupPrivilegeValue(NULL,     /* Get the LUID for shutdown privilege. */
                           SE_SHUTDOWN_NAME,
                           &tkp.Privileges[0].Luid) == FALSE)
    if (LookupPrivilegeValue(NULL,         /* for remote shutdown privilege. */
                             SE_REMOTE_SHUTDOWN_NAME,
                             &tkp.Privileges[0].Luid) == FALSE)
    {
      if (Options.fsMachine)
      {
        if (LookupPrivilegeValue(Options.pszMachine,
                                 SE_SHUTDOWN_NAME,
                                 &tkp.Privileges[0].Luid) == FALSE)
          if (LookupPrivilegeValue(Options.pszMachine,
                                   SE_REMOTE_SHUTDOWN_NAME,
                                   &tkp.Privileges[0].Luid) == FALSE)
             return(GetLastError());                /* raise error condition */
      }
      else
        return (GetLastError());                    /* raise error condition */
    }


  tkp.PrivilegeCount           = 1;               /* one privilege to set    */
  tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;


  if (AdjustTokenPrivileges(hToken,   /* Get shutdown privilege for process. */
                            FALSE,
                            &tkp,
                            0,
                          (PTOKEN_PRIVILEGES) NULL,
                           0 ) == 0)
    return (GetLastError());                        /* raise error condition */



/*****************************************
 * OK, now the shutdown / shutdown abort *
 *****************************************/

  if (Options.fsAbortShutdown)          /* we've got to abort the shutdown ? */
  {
    if (AbortSystemShutdown(Options.pszMachine) == FALSE)
      return (GetLastError());
  }
  else
  {
    if (InitiateSystemShutdown(Options.pszMachine,   /* shutdown the machine */
                               Options.pszMessage,
                               Options.ulTimeout,
                               Options.fsForceAppsClose,
                               Options.fsReboot) == 0)
       return(GetLastError());
  }


  tkp.Privileges[0].Attributes = 0;           /* Disable shutdown privilege. */
  if (AdjustTokenPrivileges(hToken,
                            FALSE,
                            &tkp,
                            0,
                            (PTOKEN_PRIVILEGES) NULL,
                            0) == 0)
    return (GetLastError());
  else
    return (NO_ERROR);                                                 /* OK */


}
#endif


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

  memset (&Options,                                        /* initialization */
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

  if ( Options.fsHelp )                                /* user requests help */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  rc = ShutdownProcess();                            /* perform the shutdown */
  if (rc != NO_ERROR)                                    /* check for errors */
    ToolsErrorDos(rc);                                /* yield error message */

  return (rc);
}
