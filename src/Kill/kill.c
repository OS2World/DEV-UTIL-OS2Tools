/***********************************************************************
 * Name      : Module Kill
 * Funktion  : Stoppen oder abschiessen eines Prozesses
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 28.10.1995 14.54.33]
 ***********************************************************************/

/* Remark: for the temporarily removed parts, we need the undocumented
 DosQueryProcStatus or DosQuerySysState calls. Both are not reachable
 with Borlame C 1.0 :( */

/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#ifdef __OS2__
  #define INCL_DOSFILEMGR
  #define INCL_DOS
  #define INCL_DOSERRORS
  #define INCL_DOSPROCESS
  #define INCL_BASE
  #define INCL_NOPMAPI
  #include <os2.h>
  #include <os2.h>
#endif

#ifdef _WIN32
  #include <windows.h>
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
 * Defines for OS/2 RAS                                                      *
 *****************************************************************************/

#define DDP_DISABLEPROCDUMP 0x00000000L
#define DDP_ENABLEPROCDUMP  0x00000001L
#define DDP_PERFORMPROCDUMP 0x00000002L


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsKill;                                        /* kill the process */

#ifdef __OS2__
  ARGFLAG fsSignal1;                         /* send a ctrl-c to the process */
  ARGFLAG fsSignal4;                     /* send a ctrl-break to the process */
  ARGFLAG fsMegadeath;   /* \DEV\XF86SUP$ ioctl() to SES API to kill process */
  ARGFLAG fsForce;       /* force in any case the termination of the process */
  ARGFLAG fsDump;                          /* dump the process to a corefile */
  ARGFLAG fsDumpEnable;                    /* enable process dumping         */
  ARGFLAG fsDumpDisable;                   /* disable process dumping        */
  ARGFLAG fsDumpDrive;                 /* which drive to dump the process to */
  PSZ   pszDumpDrive;           /* which drive number to dump the process to */
  ULONG ulDumpDrive;            /* which drive number to dump the process to */
#endif

  ARGFLAG fsID;               /* process identifier or process name supplied */

#ifdef _WIN32
  ARGFLAG fsWait;                /* wait for the termination of that process */
#endif

  ARGFLAG fsInfo;                  /* display information about this process */

  PSZ  pszID;                 /* process identifier or process name supplied */
} OPTIONS, *POPTIONS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */

ARGUMENT TabArguments[] =
{ /*Token-------Beschreibung---------------------pTarget--ucTargetFormat--pTargetSpecified--*/
  {"/?",        "Get help screen.",              NULL,    ARG_NULL,       &Options.fsHelp},
  {"/H",        "Get help screen.",              NULL,    ARG_NULL,       &Options.fsHelp},

#if 0
  {"/INFO",     "Display information about the"
                " process.",                     NULL,    ARG_NULL,       &Options.fsInfo},
#endif

#ifdef _WIN32
  {"/WAIT",     "Wait for the termination of"
                " that process.",                NULL,    ARG_NULL,       &Options.fsWait},
#endif

#ifdef __OS2__
  {"/SIGNAL1",  "Send Ctrl-C to the process.",   NULL,    ARG_NULL,       &Options.fsSignal1},
  {"/SIGNAL4",  "Send Ctrl-Break to the "
                "process.",                      NULL,    ARG_NULL,       &Options.fsSignal4},
  {"/FORCE",    "Try everything to kill the"
                " process.",                     NULL,    ARG_NULL,       &Options.fsForce},
  {"/DUMP",     "Dump the process to corefile.", NULL,    ARG_NULL,       &Options.fsDump},
  {"/DUMP.ENABLE","Enable automatic process dumps by ring 3 trap.",
                                                 NULL,    ARG_NULL,       &Options.fsDumpEnable},
  {"/DUMP.DISABLE","Disable automatic process dumps.",
                                                 NULL,    ARG_NULL,       &Options.fsDumpDisable},
  {"/DUMPDRIVE=","To which drive to dump to (default is boot drive).",
                                                 &Options.pszDumpDrive,
                                                          ARG_PSZ,        &Options.fsDumpDrive},
  {"/DEATH",    "Kill the process (megadeath).", NULL,    ARG_NULL,       &Options.fsMegadeath},
#endif

  {"/KILL",     "Kill the process (standard).",  NULL,    ARG_NULL,       &Options.fsKill},
  {"1",         "Process ID (NOT YET: or name).",&Options.pszID,
                                                          ARG_PSZ |
                                                          ARG_DEFAULT,    &Options.fsID},
  ARG_TERMINATE
};



/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

/*** PROTOTYPEN ***/
void help     ( void );

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

void help (void)
{
#ifdef __OS2__
  TOOLVERSION("Kill",                                   /* application name */
              0x00010004,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              "For /DEATH /DEV/XF86SUP$ must be installed.",
              NULL);                                /* additional copyright */
#else
  TOOLVERSION("Kill",                                   /* application name */
              0x00010001,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,
              NULL);                                /* additional copyright */
#endif
} /* void help */


#ifdef __OS2__
/***********************************************************************
 * Name      : APIRET ProcessDump
 * Funktion  : perform the dump of a process
 * Parameter :
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Syntax
 *        APIRET APIENTRY DosDumpProcess(ULONG Flag, ULONG Drive, PID pid);
 *                       32-bit code Example using CSet/2
 * Parameters
 * Flag
 *     Doubleword field that may take one of the following values:
 *        (DDP_DISABLEPROCDUMP 0x00000000L)
 *        (DDP_ENABLEPROCDUMP 0x00000001L)
 *        (DDP_PERFORMPROCDUMP 0x00000002L)
 *
 * Drive
 *     Doubleword containing the ASCII value of the drive letter to which the
 *     PDUMP.nnn dump files will be written when DDP_ENABLEPROCDUMP is
 *     specified.  For DDP_DISABLEPROCDUMP this parameter is ignored.
 *
 * pid Doubleword containing the process Id of the process to be dumped.
 *
 *     This option is valid only with DDP_PERFORMPROCDUMP. If zero is
 *     specified for PiD then the current process is dumped.
 * Returns.
 *
 * Return Code
 *   DosDumpProcess returns the following values:
 *     0 NO_ERROR
 *    87 ERROR_INVALID_PARAMETER
 *   303 ERROR_INVALID_PROCID
 *
 * When process dump is enabled a dump file is written whenever a ring 3
 * process traps.  The file takes the name PDUMP.nnn where nnn is incremented
 * sequentially (staring from 000) for each successive dump.
 *
 * The directory to which PDUMP.nnn will be written is always the root
 * directory of Drive.
 *
 * C Language prototype definitions for the DosDumpProcess may be found in
 * "RAS API Prototypes" in topic 2.9.
 *
 * The content of a process dump comprise register information at time of
 * trap, system control blocks (TCB, TSD, PTDA, MTE, SMTE, OTE, VMAR, VMOB
 * and LTD) that describe the state of the process at the time of error, ring
 * 0 and ring 3 stack data for the trapping process.
 *
 * See the process Dump Formatter section of the Dump Formatter User Guide
 * for information on formatting Process Dumps.
 *
 * Note:  DDP_PERFORMPROCDUMP is not available in some early releases of OS/2
 *        V2.11.
 *
 * Autor     : Patrick Haller [Samstag, 28.10.1995 14.55.13]
 ***********************************************************************/

APIRET ProcessDump(PID   pid,
                   ULONG ulOption)
{
  APIRET  rc;                                 /* API returncode              */
  HMODULE hKernel;                            /* module handle to the kernel */

                                                     /* the function pointer */
#ifndef __IBMC__
  APIRET APIENTRY (*pfnDosDumpProcess)(ULONG Flag,
                                       ULONG Drive,
                                       PID   pid);
#else
  APIRET (* APIENTRY pfnDosDumpProcess)(ULONG Flag,
                                        ULONG Drive,
                                        PID   pid);
#endif

  rc = DosQueryModuleHandle("DOSCALLS",                   /* the OS/2 kernel */
                            &hKernel);
  if (rc!=NO_ERROR)                                      /* check for errors */
     return (rc);                              /* then raise error condition */

  rc = DosQueryProcAddr(hKernel,                     /* kernel module handle */
                        113,    /* ordinal export number of Dos32DumpProcess */
                        NULL,                           /* no export by name */
                        (PFN*)&pfnDosDumpProcess);/* and the target function */
  if (rc!=NO_ERROR)                                      /* check for errors */
     return (rc);                              /* then raise error condition */


                                       /* now determine the drive to dump to */
  if (!Options.fsDumpDrive)
  {
     rc = DosQuerySysInfo(QSV_BOOT_DRIVE,     /* query the system boot drive */
                          QSV_BOOT_DRIVE,
                          &Options.ulDumpDrive,
                          sizeof(Options.ulDumpDrive));
    if (rc!=NO_ERROR)                                    /* check for errors */
       return (rc);                            /* then raise error condition */

    Options.ulDumpDrive += 'A'-1;                                  /* adjust */
  }
  else
     Options.ulDumpDrive = Options.pszDumpDrive[0];                /* adjust */


  switch (ulOption)
  {
    case DDP_DISABLEPROCDUMP:
       printf ("\nDisabling process dumps.");
       break;

    case DDP_ENABLEPROCDUMP:
       printf ("\nEnabling process dumps to drive %c:.",
               Options.ulDumpDrive);
       break;

    case DDP_PERFORMPROCDUMP:
      printf ("\nDumping process with pid %u (0x%x) to %c: ... "
              "\nNote: I've been unable to fix the exception that occurs."
              "\n      Might be originated somewhere in the RAS Kernel API.\n",
              pid,
              pid,
              (Options.ulDumpDrive & 0xFF) );
      break;
  }

  rc = pfnDosDumpProcess(ulOption,
                         Options.ulDumpDrive,
                         pid);


  return (rc);
}
#endif


/***********************************************************************
 * Name      : APIRET SES32Kill
 * Funktion  : perform the kill via the SES subsystem
 * Parameter :
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung : from xf86sup / holger veit
 *
 * Autor     : Patrick Haller [Samstag, 28.10.1995 14.55.13]
 ***********************************************************************/

APIRET SES32Kill(PID pid)
{
  APIRET rc;
  HFILE  hfd;
  ULONG  action,
         plen;
  USHORT param;

  if ((rc=DosOpen((PSZ)"/dev/fastio$",
                  (PHFILE)&hfd,
                  (PULONG)&action,
                  (ULONG)0,
                  FILE_SYSTEM,
                  FILE_OPEN,
                  OPEN_SHARE_DENYNONE|
                  OPEN_FLAGS_NOINHERIT|
                  OPEN_ACCESS_READONLY,
                  (ULONG)0)))
  {
    ToolsErrorDosEx(rc,
                    "Opening XF86SUP.SYS/FASTIO$ driver.");
    return rc;
  }

  param = pid;

  rc = DosDevIOCtl(hfd,
                   (ULONG)0x76,
                   (ULONG)0x65,
                   (PULONG*)&param,
                   sizeof(USHORT),
                   &plen,
                   NULL,
                   0,
                   NULL);

  DosClose(hfd);
  return rc;
}


/***********************************************************************
 * Name      : APIRET ProcessKill
 * Funktion  : perform the kill
 * Parameter :
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 28.10.1995 14.55.13]
 ***********************************************************************/

APIRET ProcessKill(void)
{
/********
 * OS/2 *
 ********/

#ifdef __OS2__
  APIRET rc;                                               /* API-Returncode */
  PTIB   pTib;                        /* pointer to thread information block */
  PPIB   pPib;                       /* pointer to process information block */
  ULONG  ulPID;       /* first we assume pszID is a PID. If it is not a PID, */
  /* check the module table to find out the PID from process name or process */
  /* path. Apply WILDCARDS -> DosEditname here ... @@@PH                     */

  ulPID = StrToNumber(Options.pszID,
                      FALSE);

                                   /* perform a little bit parameter mapping */
  if (!Options.fsSignal1 &&
      !Options.fsSignal4 &&
      !Options.fsKill    &&
      !Options.fsDump    &&
      !Options.fsForce)
    Options.fsKill = TRUE;                     /* this is out default action */

#if 0
  if (Options.fsInfo)                                   /* information first */
  {
    printf ("\nInformation about process %s.",
            Options.pszID);


  }
#endif


#ifdef __OS2__
  /***************************************************************************
   * Dump the process / RAS                                                  *
   ***************************************************************************/

  if (Options.fsDump)
  {
    rc = ProcessDump(ulPID,                              /* dump the process */
                     DDP_ENABLEPROCDUMP);
    if (rc != NO_ERROR)                                  /* check for errors */
      return (rc);                        /* return code to the main routine */

    rc = ProcessDump(ulPID,                              /* dump the process */
                     DDP_PERFORMPROCDUMP);
    return (rc);                          /* return code to the main routine */
  }

  if (Options.fsDumpEnable)
  {
    rc = ProcessDump(0,                                  /* dump the process */
                     DDP_ENABLEPROCDUMP);
    return (rc);                          /* return code to the main routine */
  }

  if (Options.fsDumpDisable)
  {
    rc = ProcessDump(0,                                  /* dump the process */
                     DDP_DISABLEPROCDUMP);
    return (rc);                          /* return code to the main routine */
  }
#endif


  /***************************************************************************
   * Send Signal 1                                                           *
   ***************************************************************************/

  /* @@@PH: undoc. exception: XCPT_APTERM */

                                 /* first go for the least harmful operation */
  if (Options.fsSignal1 || Options.fsForce)
  {
    printf ("\nSignalling (ctrl-c) process %s.",
            Options.pszID);
    rc = DosSendSignalException (ulPID,
                                 XCPT_SIGNAL_INTR);
    if (rc != NO_ERROR)                                  /* check for errors */
      ToolsErrorDos(rc);         /* just display error message, but continue */
  }


  /***************************************************************************
   * Send Signal 4                                                           *
   ***************************************************************************/

                                 /* first go for the least harmful operation */
  if (Options.fsSignal4 || Options.fsForce)
  {
    printf ("\nSignalling (ctrl-break) process %s.",
            Options.pszID);
    rc = DosSendSignalException (ulPID,
                                 XCPT_SIGNAL_BREAK);
    if (rc != NO_ERROR)                                  /* check for errors */
      ToolsErrorDos(rc);         /* just display error message, but continue */
  }


  /***************************************************************************
   * Send the KILL                                                           *
   ***************************************************************************/

                                     /* then the next more harmful operation */
  if (Options.fsKill || Options.fsForce)
  {
    printf ("\nKilling process %s.",
            Options.pszID);

    rc = DosKillProcess (DKP_PROCESS,   /* other values seem not to be valid */
                         ulPID);
    if (rc != NO_ERROR)                                  /* check for errors */
      ToolsErrorDos(rc);         /* just display error message, but continue */
  }

#if 0
                                         /* now show the Jack Hammer ... ]:> */
  if (Options.fsMegadeath || Options.fsForce)
  {
    printf ("\nNuking process %s.",
            Options.pszID);

    rc = SES32Kill(ulPID);                                   /* call XF86SUP */
  }

                                    /* do we have to wait for that process ? */
  if (Options.fsWait)
  {
                                       /* we have to do busy wait on the PID */
    do
    {
      /* "DosFindProcess()" */
      if (rc == NO_ERROR)                             /* free timeslices !!! */
        DosSleep(200);      /* we sleep for 200ms, should be friendly enough */
    }
    while (rc == NO_ERROR);                 /* loop until process terminates */
  }
#endif
#endif


/*********
 * Win32 *
 *********/

#ifdef _WIN32
  ULONG  ulPID;       /* first we assume pszID is a PID. If it is not a PID, */
  HANDLE hProcess;                     /* handle to the process to be killed */
  ULONG  ulAccess = PROCESS_TERMINATE;      /* desired access to the process */

  ulPID = StrToNumber(Options.pszID,
                      FALSE);

                                   /* perform a little bit parameter mapping */
  Options.fsKill = TRUE;                       /* this is out default action */

  if (Options.fsWait)
    ulAccess |= SYNCHRONIZE;                          /* add this access bit */

  if (Options.fsKill)
  {
    printf ("\nKilling process %s. ",
            Options.pszID);

    hProcess = OpenProcess(ulAccess,                     /* the access flags */
                           FALSE,          /* no handle inheritance required */
                           ulPID);                         /* the process ID */
    if (hProcess == NULL)                                /* check for errors */
      return (GetLastError());                   /* return error information */

    if (TerminateProcess(hProcess,                    /* terminate and check */
                         0xFFFFFFFF) == FALSE)  /* return code is 0xFFFFFFFF */
    {
      CloseHandle(hProcess);                     /* close the process handle */
      return (GetLastError());                   /* return error information */
    }

    if (CloseHandle(hProcess) == FALSE)                   /* close and check */
      return (GetLastError());                   /* return error information */
  }
#endif

  return (NO_ERROR);                                                   /* OK */
}


/***********************************************************************
 * Name      : int main
 * Funktion  : Hauptroutine
 * Parameter : int argc, char *argv[]
 * Variablen :
 * Ergebnis  : RÅckgabewert ans Betriebssystem
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 28.10.1995 14.55.13]
 ***********************************************************************/

int main (int argc, char *argv[])
{
  APIRET rc = NO_ERROR;                                      /* RÅckgabewert */

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

                                                     /* check the parameters */
  if (!Options.fsID)                           /* means no PID was specified */
  {
#ifdef __OS2__
    if (!Options.fsDumpEnable &&      /* only available in OS/2, NT lacks it */
        !Options.fsDumpDisable)
#endif
    {
      fprintf (stderr,                                /* yield error message */
               "\nError: no process identifier (PID) was specified.");
      return (ERROR_INVALID_PARAMETER);                    /* OS return code */
    }
  }

  if (rc == NO_ERROR)                          /* Wenn soweit alles i.O. ist */
  {
    rc = ProcessKill();                                  /* OK , do the work */
    if (rc != NO_ERROR)                      /* Ist ein Fehler aufgetreten ? */
      ToolsErrorDos(rc);                           /* Fehlermeldung ausgeben */
  }

  return (rc);                                       /* RÅckgabewert liefern */
} /* int main */
