/*****************************************************
 * UDP Benchmark - Server                            *
 * Reports packet transmission times                 *
 * (c) 1999    Patrick Haller Systemtechnik          *
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
  ARGFLAG fsClient;                   /* client address specified ?          */
  ARGFLAG fsSize;                     /* packet size                         */

  USHORT  usPort;                     /* port to operate on                  */
  PSZ     pszClient;                  /* client address                      */
  ULONG   ulSize;                     /* size of the packet to send          */
} OPTIONS, *POPTIONS;


typedef struct tagGlobals
{
  BOOL fTerminate;                      /* set true to terminate echo thread */
  struct sockaddr_in sinClient;            /* socketaddress of client socket */
  struct sockaddr_in sinEcho;                /* socketaddress of echo socket */
  PSZ     pszHostname;
  PVOID   pPacketBuffer;                     /* buffer for packet processing */
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
  {"/Size=",     "Size of packet to use.",    &Options.ulSize,      ARG_ULONG,      &Options.fsSize},
  {"1",          "Client IP Address.",        &Options.pszClient,   ARG_PSZ |
                                                                    ARG_MUST |
                                                                    ARG_DEFAULT,    &Options.fsClient},
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
  TOOLVERSION("UDP-Server",                             /* application name */
              0x00010000,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


/***********************************************************************
 * Name      : u_long parseClientIP
 * Funktion  : Determine client's IP address
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 ***********************************************************************/

void parseClientIP(struct sockaddr_in *to,
                   PSZ pszTarget)
{
  struct hostent *pHostEntry;
  char hnamebuf[128];

  int iAddress = (int)inet_addr(pszTarget);
  if (iAddress != -1)
  {
    to->sin_addr.s_addr = iAddress;
    Globals.pszHostname = pszTarget;
  }
  else
  {
    pHostEntry = gethostbyname(pszTarget);

    if (!pHostEntry)
    {
      fprintf(stderr,
              "UDP-Server: unknown host %s\n",
              pszTarget);
      exit(2);
    }

    to->sin_family = pHostEntry->h_addrtype;

    if (pHostEntry->h_length > sizeof(to->sin_addr))
      pHostEntry->h_length = sizeof(to->sin_addr);

    memcpy(&to->sin_addr,
           pHostEntry->h_addr,
           pHostEntry->h_length);
    strncpy(hnamebuf,
            pHostEntry->h_name,
            sizeof(hnamebuf) - 1);
    Globals.pszHostname = hnamebuf;
  }
}


/***********************************************************************
 * Name      : void PacketFormat
 * Funktion  : Timestamp the packet
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 ***********************************************************************/

void PacketFormat(PVOID pPacketBuffer,
                  int   iLength)
{
  PPACKETBENCH pPB = (PPACKETBENCH)pPacketBuffer;
  static lastID = 0;

  if (iLength < sizeof(PACKETBENCH))    /* is the packet large enough? */
    return;

  pPB->ID=lastID++;
  ToolsPerfQuery(&pPB->psSent);
}


/***********************************************************************
 * Name      : void PacketDiffTime
 * Funktion  : Diff Timestamp the packet
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 ***********************************************************************/

float PacketDiffTime(PVOID pPacketBuffer,
                     int   iLength)
{
  PPACKETBENCH pPB = (PPACKETBENCH)pPacketBuffer;

  if (iLength < sizeof(PACKETBENCH))    /* is the packet large enough? */
    return -1.0;

  ToolsPerfQuery(&pPB->psReceived);

  return (pPB->psReceived.fSeconds -
          pPB->psSent.fSeconds);                   /* return diff time */
}


/***********************************************************************
 * Name      : void runRetriever
 * Funktion  : running UDP packet echo thread
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 1999/12/17 23:45]
 ***********************************************************************/

APIRET runServer(void)
{
  int                sockEcho;                    /* echo socket       */
  int                rc;                   /* tcp/ip stack return code */
  int                iNameLen;          /* length of the received name */

  int                arrSockets[1];       /* socket array for select() */

  /* -- statistics -- */
  ULONG ulStatRecvPackets = 0;
  ULONG ulStatRecvBytes   = 0;
  ULONG ulStatSendPackets = 0;
  ULONG ulStatSendBytes   = 0;
  ULONG ulStatErrors      = 0;

  double dTime            = 0.0;
  double dTimeMin         = 999999.0;
  double dTimeMax         = 0.0;
  double dTimeSum         = 0.0;
  double dTimeNumber      = 0.00001;


  Globals.pPacketBuffer = malloc(BUFFER_SIZE);      /* allocate buffer */
  if (NULL == Globals.pPacketBuffer)        /* check memory allocation */
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
            (struct sockaddr*)&Globals.sinClient,
            sizeof(Globals.sinEcho));
  if (-1 == sockEcho)                         /* check socket creation */
  {
   psock_errno("Can't bind socket");
   exit(2);
  }

  arrSockets[0] = sockEcho;
  iNameLen = sizeof(Globals.sinEcho);

  /* loop until ctrl-c */
  printf("Sending UDP packets to %s on port %d...\n",
         inet_ntoa(Globals.sinClient.sin_addr),
         Options.usPort);

  while(!Globals.fTerminate)
  {
    fprintf(stdout,
            "\rR:%5d,S:%5d,Err:%3d, %8.2f: %8.2f < %8.2f < %8.2f",
            ulStatRecvPackets,
            ulStatSendPackets,
            ulStatErrors,
            dTime,
            dTimeMin,
            dTimeSum / dTimeNumber,
            dTimeMax);


    DosSleep(100);

    PacketFormat(Globals.pPacketBuffer,    /* write timestamped packet */
                 BUFFER_SIZE);

    /* send packet to client */
    rc = sendto(sockEcho,
                (char*)Globals.pPacketBuffer,
                (size_t)Options.ulSize,
                0,
                (struct sockaddr*)&Globals.sinClient,
                sizeof(Globals.sinClient));
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


    /* wait for client to respond */
    rc = select(arrSockets,                       /* OS/2 style select */
                    1,
                    0,
                    0,
                    -1);                      /* wait to prevent flood */
    if (-1 == rc)
    {
       // If the select was interrupted, just continue
      if (sock_errno() == SOCEINTR)
        continue;

      psock_errno("select()");
      exit(1);
    }
    else
      if (rc > 0)
      {
        rc = recvfrom(sockEcho,                          /* receive packet */
                      (char*)Globals.pPacketBuffer,
                      BUFFER_SIZE,
                      0,
                      (struct sockaddr*)&Globals.sinEcho,
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

         dTime = PacketDiffTime(Globals.pPacketBuffer,
                                BUFFER_SIZE) * 1000.0; /* convert to millisecs*/
         if (dTime < dTimeMin) dTimeMin = dTime;
         if (dTime > dTimeMax) dTimeMax = dTime;
         dTimeSum    += dTime;
         dTimeNumber += 1.0;
        }
     }
  }

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

  if (FALSE == Options.fsSize)
    Options.ulSize = sizeof(PACKETBENCH);

//  Globals.sinClient.sin_len         = sizeof(Globals.sinEcho);
  Globals.sinClient.sin_family      = AF_INET;
  parseClientIP(&Globals.sinClient,
                Options.pszClient);
  Globals.sinClient.sin_port        = htons(Options.usPort);


  rc = runServer() ; /* @@@PH operation */
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  free(Globals.pPacketBuffer);           /* free previously allocated memory */

  return (rc);
}
