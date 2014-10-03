/*****************************************************
 * UDP Benchmark - Server                            *
 * Reports packet transmission times                 *
 * (c) 1999    Patrick Haller                        *
 *****************************************************/

/* #define DEBUG 1 */


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#include "udpbench.h"


/*****************************************************************************
 * Defines                                                                   *
 *****************************************************************************/

#define BUFFER_SIZE 4096


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsPort;                     /* port to listen on                   */
  ARGFLAG fsVerbose;                  /* verbose output                      */

  USHORT  usPort;                     /* port to operate on                  */
} OPTIONS, *POPTIONS;


typedef struct tagGlobals
{
  BOOL fTerminate;                      /* set true to terminate echo thread */
  struct sockaddr_in sinEcho;                /* socketaddress of echo socket */
  struct sockaddr_in sinServer;              /* socketaddress of echo socket */
} GLOBALS, *PGLOBALS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */
GLOBALS Globals;                                 /* program global variables */


ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung-----------------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",          NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",          NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/V",         "Verbose output.",           NULL,                 ARG_NULL,       &Options.fsVerbose},
  {"/Port=",     "Port to operate on.",       &Options.usPort,      ARG_USHORT,     &Options.fsPort},
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
  TOOLVERSION("UDP-Client",                             /* application name */
              0x00010000,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


/***********************************************************************
 * Name      : void runEcho
 * Funktion  : running UDP packet echo thread
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 1999/12/17 23:45]
 ***********************************************************************/

APIRET runEcho(void)
{
  int                sockEcho;                    /* echo socket       */
  PVOID              pPacketBuffer;    /* buffer for packet processing */
  int                rc;                   /* tcp/ip stack return code */
  int                iNameLen;          /* length of the received name */
  int                arrSockets[1];       /* socket array for select() */

  /* -- statistics -- */
  ULONG ulStatRecvPackets = 0;
  ULONG ulStatRecvBytes   = 0;
  ULONG ulStatSendPackets = 0;
  ULONG ulStatSendBytes   = 0;
  ULONG ulStatErrors      = 0;


  pPacketBuffer = malloc(BUFFER_SIZE);              /* allocate buffer */
  if (NULL == pPacketBuffer)                /* check memory allocation */
    return(ERROR_NOT_ENOUGH_MEMORY);          /* raise error condition */

  rc = sock_init();                         /* initialize tcp/ip stack */
  if (0 != rc)                                /* check socket creation */
  {
   psock_errno("Can't initialize tcp/ip stack");
   exit(1);
  }

  sockEcho = socket (PF_INET,                         /* create socket */
                     SOCK_DGRAM,
                     IPPROTO_UDP);
  if (-1 == sockEcho)                         /* check socket creation */
  {
   psock_errno("Can't create socket");
   exit(1);
  }

  rc = bind(sockEcho,                                   /* bind socket */
            (struct sockaddr*)&Globals.sinEcho,
            sizeof(Globals.sinEcho));
  if (-1 == sockEcho)                         /* check socket creation */
  {
   psock_errno("Can't bind socket");
   exit(2);
  }

  arrSockets[0] = sockEcho;
  iNameLen = sizeof(Globals.sinEcho);

  /* loop until ctrl-c */
  printf("Listening for UDP packets on port %d...\n",
         Options.usPort);

  while(!Globals.fTerminate)
  {
    fprintf(stdout,
            "\rRecv: %7d (%8.3fkb), Sent: %7d (%8.3fkb), Errors: %7d",
            ulStatRecvPackets,
            (float)ulStatRecvBytes / 1024.0,
            ulStatSendPackets,
            (float)ulStatSendBytes / 1024.0,
            ulStatErrors);

#if 0
    rc = os2_select(arrSockets,                   /* OS/2 style select */
                    1,
                    0,
                    0,
                    -1);
    if (-1 == rc)
    {
       // If the select was interrupted, just continue
      if (sock_errno() == SOCEINTR)
        continue;

      psock_errno("select()");
      exit(1);
    }
#endif

    iNameLen = sizeof(Globals.sinServer);
    rc = recvfrom(sockEcho,                          /* receive packet */
                  (char*)pPacketBuffer,
                  BUFFER_SIZE,
                  0,
                  (struct sockaddr*)&Globals.sinServer,
                  &iNameLen);

    if (-1 == rc)                             /* check socket creation */
    {
      ulStatErrors++;
      psock_errno("Can't receive from socket");
    }
    else
    {
      ulStatRecvPackets++;                        /* update statistics */
      ulStatRecvBytes+=rc;
    }

    /* echo packet */
    rc = sendto(sockEcho,
                (char*)pPacketBuffer,
                rc,
                0,
                (struct sockaddr*)&Globals.sinServer,
                iNameLen);
    if (-1 == rc)                             /* check socket creation */
    {
      ulStatErrors++;
      psock_errno("Can't send to socket");
    }
    else
    {
      ulStatSendPackets++;                        /* update statistics */
      ulStatSendBytes+=rc;
    }
  }

  free(pPacketBuffer);             /* free previously allocated memory */
  soclose(sockEcho);                              /* destroy resources */
  return (NO_ERROR);
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
  memset (&Options,
          0,
          sizeof(Options));

  Globals.fTerminate = FALSE;
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

                                                /* do some parameter mapping */
  if (FALSE == Options.fsPort)
    Options.usPort = 4321;

//  Globals.sinEcho.sin_len         = sizeof(Globals.sinEcho);
  Globals.sinEcho.sin_family      = AF_INET;
  Globals.sinEcho.sin_addr.s_addr = INADDR_ANY;
  Globals.sinEcho.sin_port        = htons(Options.usPort);

  rc = runEcho(); /* @@@PH operation */
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  return (rc);
}
