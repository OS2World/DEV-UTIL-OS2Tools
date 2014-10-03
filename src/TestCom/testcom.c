/*****************************************************
 * TestCOM  Tool.                                    *
 * Can be used to debug serial communications.       *
 * (c) 1999    Patrick Haller Systemtechnik          *
 *****************************************************/

/* #define DEBUG 1 */


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#ifdef __OS2__
  #define INCL_DOSMISC
  #define INCL_DOSFILEMGR
  #define INCL_DOSDEVIOCTL
  #define INCL_DEV
  #define INCL_BASE
  #define INCL_NOPMAPI
  #include <os2.h>
#endif

#include <stdio.h>
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

#define MODE_DIRECT 0
#define MODE_HEX    1
#define MODE_ASCII  2

#define HFSTDOUT    1
#define HFSTDERR    2


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsPort;                     /* which port to use                   */
  ARGFLAG fsLineControl;              /* line control settings               */
  ARGFLAG fsFile;                     /* file to send to the port            */
  ARGFLAG fsVerbose;                  /* set verbosity level                 */
  ARGFLAG fsWait;                     /* wait for incoming data              */
  ARGFLAG fsModeDirect;               /* output modes                        */
  ARGFLAG fsModeHex;
  ARGFLAG fsModeAscii;

  PSZ   pszPort;
  PSZ   pszLineControl;
  PSZ   pszFile;
  ULONG ulWait;
} OPTIONS, *POPTIONS;


typedef struct
{
  HFILE hCOM;                                      /* handle to the com port */
  PBYTE pSendBuffer;           /* buffer of the file to send to the COM port */
  ULONG ulSendBufferLength;
  TID   tidReader;                             /* thread id of reader thread */

  /* cached options */
  UCHAR fsMode;
  ULONG ulOptionBaud;
  ULONG ulOptionDatabits;
  UCHAR ucOptionParity;
  ULONG ulOptionStopbits;
} GLOBALS, *PGLOBALS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */
GLOBALS Globals;                /* this structure holds global variables     */

ARGUMENT TabArguments[] =
{ /*Token--Beschreibung----------------------pTarget------------------ucTargetFormat--pTargetSpecified--*/
  {"/?",   "Get help screen.",               NULL,                    ARG_NULL,       &Options.fsHelp},
  {"/H",   "Get help screen.",               NULL,                    ARG_NULL,       &Options.fsHelp},
  {"/V",   "Scan device drivers.",           NULL,                    ARG_NULL,       &Options.fsVerbose},
  {"/IN=", "Input file. Default ist STDIN.", &Options.pszFile,        ARG_PSZ,        &Options.fsFile},
  {"/L=",  "Line control settings.",         &Options.pszLineControl, ARG_PSZ,        &Options.fsLineControl},
  {"/WAIT=","Time to wait for incoming data.",&Options.ulWait,        ARG_ULONG,      &Options.fsWait},
  {"/DIRECT", "Output: direct",              NULL,                    ARG_NULL,       &Options.fsModeDirect},
  {"/HEX",    "Output: hex dump",            NULL,                    ARG_NULL,       &Options.fsModeHex},
  {"/ASCII",  "Output: ascii dump",          NULL,                    ARG_NULL,       &Options.fsModeAscii},
  {"1",    "Port to use. e.g. \\DEV\\COM1.", &Options.pszPort,        ARG_PSZ |
                                                                      ARG_DEFAULT,    &Options.fsPort},
  ARG_TERMINATE
};


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void   help                (void);

void   ThdReaderThread     (PVOID pDummy);

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
  TOOLVERSION("TestCOM",                                /* application name */
              0x00010000,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


/***********************************************************************
 * Name      :
 * Funktion  :
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 21.09.1999 00.46.09]
 ***********************************************************************/

void ThdReaderThread (PVOID pDummy)
{
  APIRET rc;                                        /* API return code */
  char   szBuffer[4096];                                /* read buffer */
  ULONG  ulRead;                               /* number of bytes read */
  ULONG  ulWritten;                         /* number of bytes written */

  ULONG  ulTotalRead = 0;                /* total number of bytes read */
  PERFSTRUCT psStart;                         /* start of transmission */
  PERFSTRUCT psEnd;                             /* end of transmission */

  ToolsPerfQuery(&psStart);                      /* start benchmarking */
  do
  {
    rc = DosRead(Globals.hCOM,                     /* read from device */
                 &szBuffer,
                 sizeof(szBuffer),
                 &ulRead);
    if (rc == ERROR_INVALID_HANDLE)     /* this happens if main thread */
      break;                            /* closes the port handle      */

    if (rc != NO_ERROR)                             /* check for error */
    {
      ToolsErrorDosEx(rc,                  /* only yield error message */
                      "Reading data");
    }
    else
    {
      /* output */
      switch (Globals.fsMode)
      {
        case MODE_DIRECT:
          DosWrite(HFSTDOUT,               /* forward buffer to device */
                   szBuffer,
                   ulRead,
                   &ulWritten);
          break;

        case MODE_HEX:   ToolsDumpHex  (ulTotalRead, ulRead, szBuffer); break;
        case MODE_ASCII: ToolsDumpAscii(ulTotalRead, ulRead, szBuffer); break;
      }

      ulTotalRead += ulRead;                        /* keep statistics */
    }
  }
  while (rc == NO_ERROR);                        /* terminate on error */
  ToolsPerfQuery(&psEnd);                          /* end benchmarking */

  if (Options.fsVerbose)
    fprintf(stderr,
            "\n%u bytes read in %10.3f seconds (%10.3fb/s)",
            ulTotalRead,
            psEnd.fSeconds - psStart.fSeconds,
            ulTotalRead / (psEnd.fSeconds - psStart.fSeconds));
}


/***********************************************************************
 * Name      : COM_SetBaudrate
 * Funktion  :
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 21.09.1999 00.46.09]
 ***********************************************************************/

APIRET COMSetBaudRate(void)
{
  APIRET rc;                                        /* API return code */
  USHORT usBaud1;
  ULONG  ulParmLen = sizeof(usBaud1);
  int    iParity;
  int    iStopbits;
  char   parms[3];                    /* communication parameter block */

  sscanf(Options.pszLineControl,
         "%u,%u,%c,%u",
         &Globals.ulOptionBaud,
         &Globals.ulOptionDatabits,
         &Globals.ucOptionParity,
         &Globals.ulOptionStopbits);
  usBaud1 = Globals.ulOptionBaud;

  //@@@PH extended baudrate !
  rc = DosDevIOCtl (Globals.hCOM,     /* Device handle                 */
                    IOCTL_ASYNC,      /* Serial-device control         */
                    ASYNC_SETBAUDRATE,/* Sets bit rate                 */
                    (PULONG)&usBaud1,     /* Points at bit rate        */
                    sizeof(usBaud1),  /* Maximum size of parameter list*/
                    &ulParmLen,       /* Size of parameter packet      */
                    NULL,             /* No data packet                */
                    0,                /* Maximum size of data packet   */
                    NULL);            /* Size of data packet           */
  if (rc != NO_ERROR)
    return (rc);

  parms[0] = Globals.ulOptionDatabits;

  switch (Globals.ucOptionParity)
  {
    case 'n': // NO parity
    case 'N': iParity = 0; break;
    case 'o': // ODD parity
    case 'O': iParity = 1; break;
    case 'e': // EVEN parity
    case 'E': iParity = 2; break;
    case 'm': // MARK parity (parity bit always 1)
    case 'M': iParity = 3; break;
    case 's': // SPACE parity (parity bit always 0)
    case 'S': iParity = 4; break;
    default:  iParity = Globals.ulOptionDatabits;
  }
  parms[1]=iParity;    // see above

  switch (Globals.ulOptionStopbits)
  {
    case 1: iStopbits = 0; break;
    case 2: iStopbits = 2; break;
    // Note: SendBidi does not support 1.5 stop bits
  }
  parms[2]=iStopbits;  // "0->1", "1->1.5", "2->2"
  ulParmLen = 3;

  rc = DosDevIOCtl (Globals.hCOM,         /* Device handle             */
                    IOCTL_ASYNC,          /* Serial-device control     */
                    ASYNC_SETLINECTRL,    /* Sets line charcterristics */
                    (PULONG) &parms[0],   /* Points at bit rate        */
                    3,                    /* Maximum size of list      */
                    &ulParmLen,           /* Size of parameter packet  */
                    NULL,                 /* No data packet            */
                    0,                    /* Maximum size of data pkt  */
                    NULL);                /* Size of data packet       */
  return (rc);
}


/***********************************************************************
 * Name      : COM_SendFile
 * Funktion  :
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 21.09.1999 00.46.09]
 ***********************************************************************/

APIRET COMSendFile(void)
{
  ULONG  ulTotalSent = 0;
  ULONG  ulRemaining = Globals.ulSendBufferLength;
  ULONG  ulWritten;                         /* number of bytes written */
  PBYTE  pBuffer;                 /* points to current buffer position */
  APIRET rc;                                        /* API return code */

  if (Options.fsVerbose)
     fprintf(stdout,
             "Writing to port ...\n");

  pBuffer = Globals.pSendBuffer;
  do
  {
    rc = DosWrite(Globals.hCOM,
                  pBuffer,
                  ulRemaining,
                  &ulWritten);

    if (Options.fsVerbose)
      fprintf(stdout,
             "Wrote %u bytes, position (%u)\n",
             ulWritten,
             ulTotalSent);

    if (rc != NO_ERROR)                               /* check for error */
    {
      DosClose(Globals.hCOM);                         /* close open port */
      ToolsErrorDos(rc);
      return (rc);                              /* raise error condition */
    }
    else
    {
      ulTotalSent += ulWritten;
      ulRemaining -= ulWritten;
      pBuffer     += ulWritten;
    }
  }
  while (ulTotalSent < Globals.ulSendBufferLength);
}


/***********************************************************************
 * Name      :
 * Funktion  :
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 21.09.1999 00.46.09]
 ***********************************************************************/

APIRET COMSetup(void)
{
  APIRET rc;                                        /* API return code */
  USHORT usBaud1;                                  /* 16-bit baud rate */
  ULONG  ulAction;                             /* dos open action code */

  /* initialize default parameters */
  if (!Options.fsLineControl) Options.pszLineControl = "9600,8,N,1";
  if (!Options.fsWait)        Options.ulWait         = 5000;  /* 5 sec */

  if (Options.fsModeHex)    Globals.fsMode = MODE_HEX;
  if (Options.fsModeAscii)  Globals.fsMode = MODE_ASCII;
  if (Options.fsModeDirect) Globals.fsMode = MODE_DIRECT;

  /* open com port       */
  if (Options.fsVerbose)
     fprintf(stdout,
             "Opening port %s\n",
             Options.pszPort);

  rc = DosOpen(Options.pszPort,                         /* open device */
               &Globals.hCOM,
               &ulAction,
               0,
               FILE_NORMAL,
               FILE_OPEN,
               OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE,
               (PEAOP2) NULL);
  if (rc != NO_ERROR)                               /* check for error */
  {
    ToolsErrorDosEx(rc,
                    "Opening port");
    return (rc);                              /* raise error condition */
  }

  /* initialize port     */
  if (Options.fsVerbose)
     fprintf(stdout,
             "Initializing port to %s\n",
             Options.pszLineControl);
  rc = COMSetBaudRate();
  if (rc != NO_ERROR)                               /* check for error */
  {
    DosClose(Globals.hCOM);                         /* close open port */
    ToolsErrorDos(rc);
    return (rc);                              /* raise error condition */
  }


  /* read file           */
  if (Options.fsVerbose)
     fprintf(stdout,
             "Reading file %s\n",
             Options.pszFile);
  rc = ToolsReadFileToBuffer (Options.pszFile,   /* read a file to mem */
                              &Globals.pSendBuffer,
                              &Globals.ulSendBufferLength);
  if (rc != NO_ERROR)                               /* check for error */
  {
    DosClose(Globals.hCOM);                         /* close open port */
    ToolsErrorDos(rc);
    return (rc);                              /* raise error condition */
  }

  /* start reader thread */
  #ifdef __BORLANDC__
    Globals.tidReader  = _beginthread(ThdReaderThread,
                                      16384,
                                      (PVOID)NULL);
  #endif

  #ifdef __IBMC__
    Globals.tidReader  = _beginthread(ThdReaderThread,
                                      NULL,
                                      16384,
                                      (PVOID)NULL);
  #endif

  /* send file           */
  DosSleep(10);
  rc = COMSendFile();

  /* wait for timeout    */
  DosSleep(Options.ulWait);

  /* close port          */
  rc = DosClose(Globals.hCOM);

  /* stop thread         */
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
  int rc;                                                    /* Rckgabewert */

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

  rc = COMSetup();                                     /* map the parameters */
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  return (rc);
}
