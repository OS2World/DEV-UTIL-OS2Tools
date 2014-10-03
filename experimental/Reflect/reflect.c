/*****************************************************
 * Reflect Tool                                      *
 * Allows redirection of IP packets                  *
 * (c) 1997    Patrick Haller Systemtechnik          *
 *****************************************************/

/*
- liste/verwaltung: laufende threads / connections
- statistiken
- logging
- dumping (moeglich: ? komplette 802.5/802.3 frames zusammenbasteln
           -> IPFORMAT ?)
- access restrictions
- balancing / redirecting
*/


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#ifdef __OS2__
  #define INCL_DOSFILEMGR
  #define INCL_BASE
  #define INCL_NOPMAPI
  #include <os2.h>

  #include <types.h>
  #include <netinet/in_systm.h>
  #include <netinet/in.h>
  #include <netinet/ip.h>
  #include <netinet/ip_icmp.h>
  #include <sys/socket.h>
  #include <sys/select.h>
  #include <netdb.h>
  #include <nerrno.h>

  typedef void (__stdcall *THREAD)(PVOID pParameters);
  #define sockErrno errno
  #define SOCKET_ERROR -1
#endif

#ifdef _WIN32
  /* @@@PH Yes, Microsoft is most famous and popular for using easy-to-understand macros ... */
  #define WIN32_LEAN_AND_MEAN
  #include <winsock2.h>
  #include <windows.h>
  #include <process.h>
  #include "win32\pingw32.h"
  typedef void (__cdecl *THREAD)(PVOID pParameters);
  #define MAXHOSTNAMELEN 64
  #define EPERM 1
  #define EINTR WSAEINTR
  #define DosSleep Sleep
  #define soclose closesocket
  #define sockErrno WSAGetLastError()

  /* @@@PH General Windows Annotations:
     To use SOCK_RAW type sockets, you must apparently have administrator
    priviledge. Furthermore this program will only run on Winsock 2 DLLs. */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <process.h>
#include <conio.h>
#include <ctype.h>

#include "phstypes.h"
#include "phstools.h"
#include "phsarg.h"

#include "rconfig.h"
#include "reflect.h"


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */
GLOBALS Globals;

ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung----------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/!RESOLV",   "Don't resolve hostnames / IP addresses.",
                                       NULL,                 ARG_NULL,       &Options.fsDontResolve},
  {"1",          "Configuration file.",&Options.pszConfig,   ARG_PSZ     |
                                                             ARG_DEFAULT,    &Options.fsConfig},
  ARG_TERMINATE
};


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void   help                  (void);

void __stdcall ReflectionUDP (PVOID pParameters);

void __stdcall ReflectionTCP (PVOID pParameters);

APIRET ReflectionCreate      (PSZ   pszName,
                              PSZ   pszIPServer,
                              ULONG ulLocalIPAddress,
                              ULONG ulPortSource,
                              ULONG ulPortDest,
                              PSZ   pszProtocol);

APIRET initialize            (void);

int    main                  (int,
                              char **);


/***********************************************************************
 * Name      : APIRET IPAddressToString
 * Funktion  : calls inet_ntoa in a synchronized, serialized manner
 * Parameter : PSZ   pszTargetBuffer,
 *             ULONG ulTargetBufferSize
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung : Unix-style sux
 *
 * Autor     : Patrick Haller [Donnerstag, 1997/07/28 00.45.24]
 ***********************************************************************/

APIRET IPAddressToString(struct in_addr sin,
                         PSZ            pszTarget,
                         ULONG          ulTargetLength)
{
  APIRET rc;                                               /* API returncode */

  if ( (pszTarget      == NULL) ||                       /* check parameters */
       (ulTargetLength == 0   ) )
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */


  rc = DosRequestMutexSem(Globals.hMutexAddressToString,
                          5000);                           /* 5000ms timeout */
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                                    /* raise error condition */

  strncpy (pszTarget,           /* perform the operation and copy the buffer */
           inet_ntoa(sin),
           ulTargetLength);

  rc = DosReleaseMutexSem(Globals.hMutexAddressToString);
  return (rc);                                                         /* OK */
}


/***********************************************************************
 * Name      : void help
 * Funktion  : Anzeigen des Hilfetextes.
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 1997/07/28 00.45.24]
 ***********************************************************************/

void help (void)
{
  TOOLVERSION("Reflect",                                 /* application name */
              0x00000501,                            /* application revision */
              0x00010100,           /* minimum required revision of PHSTOOLS */
              0x0001FFFF,        /* maximum recommended revision of PHSTOOLS */
              NULL,                                                /* Remark */
              NULL);                                 /* additional copyright */
}


/***********************************************************************
 * Name      : APIRET RefManCreateFromTemplate
 * Funktion  : Create a new reflection structure in the linked list
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung : dynamically allocated string within the pRefTemplate
 *             are copied !
 *
 * Autor     : Patrick Haller [Donnerstag, 1997/07/28 00.45.24]
 ***********************************************************************/

APIRET RefManCreateFromTemplate(PREFLECTIONIP *ppReflection,
                                PREFLECTIONIP pRefTemplate)
{
  PREFLECTIONIP pRefNew;                     /* the new reflection structure */

  if (pRefTemplate == NULL)                              /* check parameters */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */

  pRefNew = (PREFLECTIONIP)malloc(sizeof(REFLECTIONIP));  /* allocate struct */
  if (pRefNew == NULL)                            /* check proper allocation */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */

  memcpy (pRefNew,                                     /* fill the structure */
          pRefTemplate,
          sizeof(REFLECTIONIP));

  memset (&pRefNew->refstatIP,            /* zero out the performance values */
          0,
          sizeof(REFLECTSTAT));

  pRefNew->pszName = strdup(pRefTemplate->pszName);      /* copy the strings */
  if (pRefNew->pszName == NULL)                   /* check proper allocation */
  {
    free(pRefNew);                       /* free previously allocated memory */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */
  }

  pRefNew->pNext   = NULL;                     /* reset the linkage pointers */
  pRefNew->pPrev   = NULL;
  pRefNew->pParent = NULL;

                             /* now put the new structure in the linked list */
  pRefNew->pNext          = Globals.pRootReflection;  /* insert at beginning */
  if (pRefNew->pNext != NULL)                       /* if a successor exists */
    pRefNew->pNext->pPrev   = pRefNew;                  /* and the back-link */

                 /* this linked list treadment should be quite thread-safe ! */
  Globals.pRootReflection = pRefNew;    /* OK, that's it for the 1st element */

  Globals.ulReflections++;                              /* update statistics */

  if (ppReflection != NULL)                    /* return parameter desired ? */
    *ppReflection = pRefNew;                        /* pass back the pointer */

  return (NO_ERROR);
}


/***********************************************************************
 * Name      : APIRET RefManCreate
 * Funktion  : Create a new reflection structure in the linked list
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung : dynamically allocated string within the pRefTemplate
 *             are copied !
 *
 * Autor     : Patrick Haller [Donnerstag, 1997/07/28 00.45.24]
 ***********************************************************************/

APIRET RefManCreateFromParam(PREFLECTIONIP *ppReflection,
                             USHORT        usProtocol,
                             PSZ           pszName,
                             USHORT        usClientPortSource,
                             USHORT        usClientPortDestination,
                             USHORT        usServerPortSource,
                             USHORT        usServerPortDestination,
                             ULONG         ulSocketOptions,
                             ULONG         ulIPServer,
                             ULONG         ulLocalIPAddress,
                             ULONG         ulTimeoutFromClient,
                             ULONG         ulTimeoutToClient,
                             ULONG         ulTimeoutFromServer,
                             ULONG         ulTimeoutToServer)
{
  REFLECTIONIP refTemplate;                             /* a static template */

  refTemplate.usProtocol              = usProtocol;
  refTemplate.pszName                 = pszName;
  refTemplate.tidMainThread           = -1;
  refTemplate.tidIPThreadCS           = -1;
  refTemplate.tidIPThreadSC           = -1;
  refTemplate.fTermination            = FALSE;
  refTemplate.sockListener            = -1;
  refTemplate.sockClient              = -1;
  refTemplate.sockServer              = -1;
  refTemplate.usClientPortSource      = usClientPortSource;
  refTemplate.usClientPortDestination = usClientPortDestination;
  refTemplate.usServerPortSource      = usServerPortSource;
  refTemplate.usServerPortDestination = usServerPortDestination;
  refTemplate.ulSocketOptions         = ulSocketOptions;
  refTemplate.ulIPServer              = ulIPServer;
  refTemplate.ulLocalIPAddress        = ulLocalIPAddress;
  refTemplate.ulTimeoutFromClient     = ulTimeoutFromClient;
  refTemplate.ulTimeoutToClient       = ulTimeoutToClient;
  refTemplate.ulTimeoutFromServer     = ulTimeoutFromServer;
  refTemplate.ulTimeoutToServer       = ulTimeoutToServer;
  refTemplate.ulTCPConnectionsActive  = 0;
  refTemplate.ulTCPConnectionsTotal   = 0;

  return (RefManCreateFromTemplate(ppReflection,                /* create it */
                                   &refTemplate));
}


/***********************************************************************
 * Name      : APIRET RefManDelete
 * Funktion  : Removes a reflection structure from the linked list
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 1997/07/28 00.45.24]
 ***********************************************************************/

APIRET RefManDelete(PREFLECTIONIP pReflection)
{
  if (pReflection == NULL)                               /* check parameters */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */

                        /* this linked-list handling should be thread-safe ! */
  if (pReflection->pPrev != NULL)                      /* predecessor exists */
    pReflection->pPrev->pNext = pReflection->pNext;        /* create linkage */

  if (pReflection->pNext != NULL)                        /* successor exists */
    pReflection->pNext->pPrev = pReflection->pPrev;        /* create linkage */

  if (Globals.pRootReflection == pReflection)          /* is this the root ? */
    Globals.pRootReflection = pReflection->pNext;     /* then reset the root */

  free (pReflection->pszName);       /* free dynamically allocated variables */
  free (pReflection);                       /* and free the structure itself */

  if (Globals.ulReflections > 0)                         /* prevent underrun */
    Globals.ulReflections--;                            /* update statistics */

  return (NO_ERROR);
}


/***********************************************************************
 * Name      : APIRET RefManCloseAll
 * Funktion  : Closes and removes all reflections
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 1997/07/28 00.45.24]
 ***********************************************************************/

APIRET RefManCloseAll(void)
{
  PREFLECTIONIP pReflection;                           /* reflection pointer */
  PREFLECTIONIP pReflectionNext;                       /* reflection pointer */
  APIRET      rc;                                         /* API return code */

  for (pReflection = Globals.pRootReflection;     /* flag all reflections to */
       pReflection != NULL;                       /* terminate gracefully    */
       pReflection = pReflection->pNext)
    pReflection->fTermination = TRUE;                /* I will kill you ! :) */

  DosSleep(500);      /* give the threads a break, perhaps they will already */
                                                     /* have terminated then */

                                /* now wait for the last thread to terminate */
  for (pReflection = Globals.pRootReflection,     /* flag all reflections to */
       pReflectionNext = NULL;

       pReflection != NULL;                       /* terminate gracefully    */

       pReflection = pReflectionNext)
  {
    printf ("\n%-20s: waiting for termination.",
            pReflection->pszName);

    switch (pReflection->usProtocol)
    {
     /**************************************
      * Wait for the UDP main thread only. *
      **************************************/
      case IPPROTO_UDP:
        soclose(pReflection->sockServer);               /* close the sockets */
        soclose(pReflection->sockClient);

        if (pReflection->tidMainThread != -1)
          DosWaitThread(&pReflection->tidMainThread,
                        DCWW_WAIT);
        break;

     /***********************************************************
      * Wait for the TCP transport threads and the main thread. *
      ***********************************************************/

      case IPPROTO_TCP:
        soclose(pReflection->sockListener);         /* close all the sockets */
        soclose(pReflection->sockServer);
        soclose(pReflection->sockClient);

        if (pReflection->tidIPThreadCS != -1)
          DosWaitThread(&pReflection->tidIPThreadCS,
                        DCWW_WAIT);

        if (pReflection->tidIPThreadSC != -1)
          DosWaitThread(&pReflection->tidIPThreadSC,
                        DCWW_WAIT);

        if (pReflection->tidMainThread != -1)
          DosWaitThread(&pReflection->tidMainThread,
                        DCWW_WAIT);
        break;

      /***********
       * default *
       ***********/
      default:
        fprintf (stderr,                              /* yield error message */
                 "\nError: %s"
                 "\n       has unknown protocol %u.",
                 pReflection->pszName);
        break;
    }

    pReflectionNext = pReflection->pNext;              /* save the pointer ! */
    rc = RefManDelete(pReflection);   /* OK, remove the reflection from list */
    if (rc != NO_ERROR)                                  /* check for errors */
      ToolsErrorDosEx(rc,
                      "RefManCloseAll::RefManDelete");
  }

  return(NO_ERROR);                                                    /* OK */
}


/***********************************************************************
 * Name      : APIRET RefManListReflection
 * Funktion  : Dump information about a certain reflection
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 1997/07/28 00.45.24]
 ***********************************************************************/

APIRET RefManListReflection(PREFLECTIONIP pReflection)
{
  CHAR szIPServer[32];                           /* IP address of the server */
  CHAR szIPLocal [32];                           /* IP address of the local  */


  if (pReflection == NULL)                               /* check parameters */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */

#if 0
  printf ("\n  pNext = %08xh pPrev=%08xh pParent=%08xh"
          "\n  tidMainThread = %i"
          "  tidIPThreadCS = %i"
          "\n  tidIPThreadSC = %i"
          "  fTermination  = %s"
          "\n  sockListener  = %i"
          "  sockClient    = %i"
          "\n  sockServer    = %i",
          pReflection->pNext,
          pReflection->pPrev,
          pReflection->pParent,
          pReflection->tidMainThread,
          pReflection->tidIPThreadCS,
          pReflection->tidIPThreadSC,
          pReflection->fTermination ? "TRUE" : "FALSE",
          pReflection->sockListener,
          pReflection->sockClient,
          pReflection->sockServer);
#endif

  printf ("\n---%s, ",
          pReflection->pszName);

  switch (pReflection->ucState)
  {
    case REFSTATE_UNKNOWN:      printf ("UNKNOWN"); break;
    case REFSTATE_UDP:          printf ("UDP"); break;
    case REFSTATE_TCP_LISTENER: printf ("TCP Listener"); break;
    case REFSTATE_TCP_SPAWN:    printf ("TCP Spawned"); break;
    default:                    printf ("<invalid>"); break;
  }

#if 0
  printf ("\n  Protocol                = %i"
          "\n  usClientPortSource      = %i"
          "  usServerPortSource      = %i"
          "\n  usClientPortDestination = %i"
          "  usServerPortDestination = %i"
          "\n  ulSocketOptions         = %08xh",
          pReflection->usProtocol,
          pReflection->usClientPortSource,
          pReflection->usServerPortSource,
          pReflection->usClientPortDestination,
          pReflection->usServerPortDestination,
          pReflection->ulSocketOptions);
#endif

  sprintf (szIPServer,
           "%u.%u.%u.%u",
           pReflection->ulIPServer >> 24,
           pReflection->ulIPServer >> 16 & 0xFF,
           pReflection->ulIPServer >> 8 & 0xFF,
           pReflection->ulIPServer & 0xFF);

  sprintf (szIPLocal,
           "%u.%u.%u.%u",
           pReflection->ulLocalIPAddress >> 24,
           pReflection->ulLocalIPAddress >> 16 & 0xFF,
           pReflection->ulLocalIPAddress >> 8 & 0xFF,
           pReflection->ulLocalIPAddress & 0xFF);

  printf ("\n  IP Server          : %s"
          "  IP local interface : %s",
          szIPServer,
          szIPLocal);

#if 0
  printf ("\n  Timeouts"
          "\n    from client : %u"
          "    to   client : %u"
          "\n    from server : %u"
          "    to   server : %u",
          pReflection->ulTimeoutFromClient,
          pReflection->ulTimeoutToClient,
          pReflection->ulTimeoutFromServer,
          pReflection->ulTimeoutToServer);
#endif

  if (pReflection->usProtocol == IPPROTO_TCP)
    printf ("\n  Connections"
            "  active : %u"
            "  total  : %u",
            pReflection->ulTCPConnectionsActive,
            pReflection->ulTCPConnectionsTotal);

  printf ("\n  Transfer Packets from client : %u"
          "  from server : %u"
          "\n                   to   client : %u"
          "  to   server : %u",
          pReflection->refstatIP.rssClient.ulPacketsFrom,
          pReflection->refstatIP.rssServer.ulPacketsFrom,
          pReflection->refstatIP.rssClient.ulPacketsTo,
          pReflection->refstatIP.rssServer.ulPacketsTo);

  printf ("\n           Bytes   from client : %u"
          "  from server : %u"
          "\n                   to   client : %u"
          "  to   server : %u",
          pReflection->refstatIP.rssClient.ulBytesFrom,
          pReflection->refstatIP.rssServer.ulBytesFrom,
          pReflection->refstatIP.rssClient.ulBytesTo,
          pReflection->refstatIP.rssServer.ulBytesTo);

  printf ("\n           Errors  from client : %u"
          "  from server : %u"
          "\n                   to   client : %u"
          "  to   server : %u",
          pReflection->refstatIP.rssClient.ulErrorsFrom,
          pReflection->refstatIP.rssServer.ulErrorsFrom,
          pReflection->refstatIP.rssClient.ulErrorsTo,
          pReflection->refstatIP.rssServer.ulErrorsTo);

  printf ("\n  Waiting on client: %10.3fs  %10.3fpkt/s  %10.3fb/s"
          "\n             server: %10.3fs  %10.3fpkt/s  %10.3fb/s",
          pReflection->refstatIP.rssClient.fTimeWait,

          pReflection->refstatIP.rssClient.ulPacketsFrom /
          pReflection->refstatIP.rssClient.fTimeWait,

          pReflection->refstatIP.rssClient.ulBytesFrom /
          pReflection->refstatIP.rssClient.fTimeWait,

          pReflection->refstatIP.rssServer.fTimeWait,

          pReflection->refstatIP.rssServer.ulPacketsFrom /
          pReflection->refstatIP.rssServer.fTimeWait,

          pReflection->refstatIP.rssServer.ulBytesFrom /
          pReflection->refstatIP.rssServer.fTimeWait);



  if (pReflection->refstatIP.ulTimeoutsServer)
    printf ("\n  Timeouts to the server: %u:",
            pReflection->refstatIP.ulTimeoutsServer);

  return (NO_ERROR);                                                   /* OK */
}


/***********************************************************************
 * Name      : APIRET RefManListAll
 * Funktion  : Dump information about all registered reflections
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 1997/07/28 00.45.24]
 ***********************************************************************/

APIRET RefManListAll(void)
{
  PREFLECTIONIP pReflection;                           /* reflection pointer */

  for (pReflection = Globals.pRootReflection;
       pReflection != NULL;
       pReflection = pReflection->pNext)
    RefManListReflection(pReflection);

  return (NO_ERROR);                                                   /* OK */
}


/***********************************************************************
 * Name      : THREAD ReflectionUDP
 * Funktion  : Create a new reflection thread
 * Parameter : -> THREAD prototype
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 1997/07/28 00.45.24]
 ***********************************************************************/

void __stdcall ReflectionUDP (PVOID pParameters)
{
  PREFLECTIONIP  pReflectionUDP = (PREFLECTIONIP)pParameters;
  int            rc;                        /* return code from TCP/IP stack */
  struct sockaddr_in sinClient;       /* address of the client that connects */
  struct sockaddr_in sinAnyClient;    /* address of the client that connects */
  struct sockaddr_in sinServer;                      /* the server's address */
  int            iClientNameLength;             /* length of the client name */
  PVOID          pBuffer;            /* buffer for the packet transportation */
  int            iBytesRead;                         /* number of bytes read */
  int            iBytesWritten;                      /* number of bytes sent */

  PERFSTRUCT     psMark1;        /* for performance benchmarking / measuring */
  PERFSTRUCT     psMark2;
  BOOL           flTimeValid;                        /* for the benchmarking */

  CHAR           szClient[32];    /* buffer for client IP address conversion */


  Globals.ulThreads++;       /* we just succeeded in creating another thread */

  flTimeValid = FALSE;                      /* the initial time is not valid */

#define UDPBUFFERSIZE 32768

  pBuffer = malloc(UDPBUFFERSIZE);                     /* reserve 32k for it */
  if (pBuffer == NULL)                        /* check for proper allocation */
  {
    fprintf(stderr,
            "\nError: out of memory in ReflectionUDP()");
    goto _bail_out_UDP2;                        /* terminate this reflection */
  }


  pReflectionUDP->sockClient = socket(PF_INET,            /* create a socket */
                                      SOCK_DGRAM,
                                      IPPROTO_UDP);
  if (pReflectionUDP->sockClient == -1)             /* check socket creation */
  {
    rc = sock_errno();                                 /* get the error code */
    fprintf (stderr,
             "\nError: Connection %s"
             "\n       Error      %i"
             "\n       Location   %s",
             pReflectionUDP->pszName,
             rc,
             "sockClient");
    psock_errno("\n\rUDP sockClient::socket");
    pReflectionUDP->sockServer = -1;
    goto _bail_out_UDP;                          /* terminate the reflection */
  }


  pReflectionUDP->sockServer = socket(PF_INET,            /* create a socket */
                                      SOCK_DGRAM,
                                      IPPROTO_UDP);
  if (pReflectionUDP->sockServer == -1)             /* check socket creation */
  {
    rc = sock_errno();                                 /* get the error code */
    fprintf (stderr,
             "\nError: Connection %s"
             "\n       Error      %i"
             "\n       Location   %s",
             pReflectionUDP->pszName,
             rc,
             "sockServer");
    psock_errno("\n\rUDP sockServer::socket");
    goto _bail_out_UDP;                          /* terminate the reflection */
  }

  sinServer.sin_family            = AF_INET;
  sinServer.sin_port              = htons(pReflectionUDP->usServerPortSource);
  sinServer.sin_addr.s_addr       = pReflectionUDP->ulIPServer;

  sinClient.sin_family            = AF_INET;
  sinClient.sin_port              = htons(pReflectionUDP->usClientPortSource);
  sinClient.sin_addr.s_addr       = INADDR_ANY;

  sinAnyClient.sin_family         = AF_INET;
  sinAnyClient.sin_port           = htons(pReflectionUDP->usClientPortSource);
  sinAnyClient.sin_addr.s_addr    = pReflectionUDP->ulLocalIPAddress;


  if (bind(pReflectionUDP->sockClient,          /* bind the socket to a name */
           (struct sockaddr *)&sinAnyClient,
           sizeof(sinAnyClient)) == SOCKET_ERROR)
  {
    rc = sock_errno();                                 /* get the error code */
    fprintf (stderr,
             "\nError: Connection %s"
             "\n       Error      %i"
             "\n       Location   %s",
             pReflectionUDP->pszName,
             rc,
             "bind");
    psock_errno("\n\rUDP sockServer::bind");
    goto _bail_out_UDP;                          /* terminate the reflection */
  }


  /* OK, this is the select() loop */
  while(pReflectionUDP->fTermination == FALSE)
  {
    if (flTimeValid == FALSE)   /* do we have to collect performance marks ? */
    {
      flTimeValid = TRUE;                     /* next the time will be valid */
      ToolsPerfQuery(&psMark1);                  /* start waiting for client */
    }

    /********************************
     * receive data from any client *
     ********************************/
    rc = select(&pReflectionUDP->sockClient,
                1,
                0,
                0,
                pReflectionUDP->ulTimeoutFromClient);    /* 2 second timeout */
    if (rc > 0)                                      /* if data is available */
    {
      ToolsPerfQuery(&psMark2);                   /* OK, packet is coming in */
      flTimeValid = FALSE;                /* ok, next time get new timestamp */

      pReflectionUDP->refstatIP.rssClient.fTimeWait +=  /* calculate seconds */
        psMark2.fSeconds - psMark1.fSeconds;

      iClientNameLength = sizeof(sinClient);
      iBytesRead = recvfrom(pReflectionUDP->sockClient,
                            pBuffer,
                            UDPBUFFERSIZE,
                            0,
                            (struct sockaddr *)&sinClient,
                            &iClientNameLength);
      if (iBytesRead == -1)                              /* check for errors */
      {
        rc = sock_errno();                             /* get the error code */
        fprintf (stderr,
                 "\nError: Connection %s"
                 "\n       Error      %i"
                 "\n       Location   %s",
                 pReflectionUDP->pszName,
                 rc,
                 "recvfrom(client)");
        psock_errno("\n\rUDP sockClient::recvfrom");
        pReflectionUDP->refstatIP.rssClient.ulErrorsFrom++;
        goto _bail_out_UDP;                      /* terminate the reflection */
      }

      pReflectionUDP->refstatIP.rssClient.ulPacketsFrom++;        /* statistics */
      pReflectionUDP->refstatIP.rssClient.ulBytesFrom+=iBytesRead;

      IPAddressToString(sinClient.sin_addr,
                        szClient,
                        sizeof(szClient));

      printf ("\n%-20s: %s (%ib)",
              pReflectionUDP->pszName,
              szClient,
              iBytesRead);


      /********************************
       * forward packet to the server *
       ********************************/

      iBytesWritten = sendto(pReflectionUDP->sockServer,
                             pBuffer,
                             iBytesRead,
                             0,
                             (struct sockaddr *)&sinServer,
                             sizeof(sinServer));
      if (iBytesWritten == -1)                           /* check for errors */
      {
        rc = sock_errno();                             /* get the error code */
        fprintf (stderr,
                 "\nError: Connection %s"
                 "\n       Error      %i"
                 "\n       Location   %s",
                 pReflectionUDP->pszName,
                 rc,
                 "sendto(server)");
        psock_errno("\n\rUDP sockServer::sendto");
        pReflectionUDP->refstatIP.rssServer.ulErrorsTo++;
        goto _bail_out_UDP;                      /* terminate the reflection */
      }

      pReflectionUDP->refstatIP.rssServer.ulPacketsTo++;       /* statistics */
      pReflectionUDP->refstatIP.rssServer.ulBytesTo+=iBytesWritten;


      /********************************
       * receive data from our server *
       ********************************/

      ToolsPerfQuery(&psMark1);                 /* get performance timestamp */

      rc = select(&pReflectionUDP->sockServer,
                  1,
                  0,
                  0,
                  pReflectionUDP->ulTimeoutFromServer);  /* 1 minute timeout */
      if (rc > 0)                                    /* if data is available */
      {
        iClientNameLength = sizeof(sinServer);
        iBytesRead = recvfrom(pReflectionUDP->sockServer,
                              pBuffer,
                              UDPBUFFERSIZE,
                              0,
                              (struct sockaddr *)&sinServer,
                              &iClientNameLength);
        ToolsPerfQuery(&psMark2);           /* don't count transmission time */

        if (iBytesRead == -1)                            /* check for errors */
        {
          rc = sock_errno();                           /* get the error code */
          fprintf (stderr,
                   "\nError: Connection %s"
                   "\n       Error      %i"
                   "\n       Location   %s",
                   pReflectionUDP->pszName,
                   rc,
                   "select(server)");
          psock_errno("\n\rUDP sockServer::recvfrom");
          pReflectionUDP->refstatIP.rssServer.ulErrorsFrom++;
          goto _bail_out_UDP;                    /* terminate the reflection */
        }

                                        /* else we can update our statistics */
        pReflectionUDP->refstatIP.rssServer.fTimeWait +=
          psMark2.fSeconds - psMark1.fSeconds;

        pReflectionUDP->refstatIP.rssServer.ulPacketsFrom++;      /* statistics */
        pReflectionUDP->refstatIP.rssServer.ulBytesFrom+=iBytesRead;


        /***************************
         * send data to our client *
         ***************************/
        iBytesWritten = sendto(pReflectionUDP->sockClient,
                               pBuffer,
                               iBytesRead,
                               0,
                               (struct sockaddr *)&sinClient,
                               sizeof(sinClient));
        if (iBytesWritten == -1)                         /* check for errors */
        {
          rc = sock_errno();                           /* get the error code */
          fprintf (stderr,
                   "\nError: Connection %s"
                   "\n       Error      %i"
                   "\n       Location   %s",
                   pReflectionUDP->pszName,
                   rc,
                   "sendto(client)");

          psock_errno("\n\rUDP sockClient::sendto");
          pReflectionUDP->refstatIP.rssClient.ulErrorsTo++;
          goto _bail_out_UDP;                    /* terminate the reflection */
        }

        pReflectionUDP->refstatIP.rssClient.ulPacketsTo++;     /* statistics */
        pReflectionUDP->refstatIP.rssClient.ulBytesTo+=iBytesWritten;
      } /* select */
      else
        if (rc == 0)                                      /* 0 means timeout */
          pReflectionUDP->refstatIP.ulTimeoutsServer++;
        else                                            /* < 0 means problem */
          pReflectionUDP->refstatIP.rssServer.ulErrorsTo++;
    } /* select client */
    else
      if (rc < 0)                         /* means a problem with the socket */
      {
        rc = sock_errno();                             /* get the error code */

        switch (rc)                  /* take action according to return code */
        {
          case 0:                                                /* no error */
          case SOCENOTSOCK:     /* this error is returned, when other thread */
                                /* closes the handles of the sockets which   */
                                /* means a normal shutdown of the reflection.*/
            rc = NO_ERROR;            /* Therefore it's OK and we ignore it. */
          break;

          default:
            fprintf (stderr,
                     "\nError: Connection %s"
                     "\n       Error      %i"
                     "\n       Location   %s",
                     pReflectionUDP->pszName,
                     rc,
                     "select");
            psock_errno("\n\rUDP select::sockClient");
            break;
        }

        goto _bail_out_UDP;
      }
  }

_bail_out_UDP:
  soclose(pReflectionUDP->sockClient);            /* close the client socket */
  soclose(pReflectionUDP->sockServer);            /* close the server socket */
  free(pBuffer);                         /* free previously allocated memory */

_bail_out_UDP2:
  pReflectionUDP->fTermination = TRUE;         /* request thread termination */

  if (Globals.ulThreads > 0)                             /* prevent underrun */
    Globals.ulThreads--;        /* we're dying, so adjust the thread counter */
}


/*****************************************************************************
 * Name      : ReflectTransportTCP
 * Purpose   : this is the main reflector :)
 * Parameter :
 * Variables :
 * Result    :
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

APIRET ReflectTransportTCP(PREFLECTIONIP      pReflectionTCP,
                           PSZ                pszThreadName,
                           SOCKET             sockSource,
                           SOCKET             sockDest,
                           PREFLECTSTATSTRUCT prssChannelSource,
                           PREFLECTSTATSTRUCT prssParentSource,
                           PREFLECTSTATSTRUCT prssChannelDest,
                           PREFLECTSTATSTRUCT prssParentDest)
{
  int    rc = NO_ERROR;                                    /* API-Returncode */
  PVOID  pBuffer;                    /* buffer for the packet transportation */
  int    iBytesRead;                                 /* number of bytes read */
  int    iBytesWritten;                              /* number of bytes sent */

  PERFSTRUCT     psMark1;        /* for performance benchmarking / measuring */
  PERFSTRUCT     psMark2;
  BOOL           flTimeValid;                        /* for the benchmarking */

#define SOCKETBUFFERSIZE 8192

  if ( (pReflectionTCP    == NULL) ||                    /* check parameters */
       (prssChannelSource == NULL) ||
       (prssParentDest    == NULL) ||
       (prssParentSource  == NULL) ||
       (prssChannelDest   == NULL) )
     return (ERROR_INVALID_PARAMETER);              /* raise error condition */

  pBuffer = malloc(SOCKETBUFFERSIZE);                   /* reserve 8k for it */
  if (pBuffer == NULL)                        /* check for proper allocation */
    return (ERROR_NOT_ENOUGH_MEMORY);           /* and raise error condition */


  flTimeValid = FALSE;          /* the initial timestamp is declared invalid */


  while(pReflectionTCP->fTermination == FALSE)
  {
    if (flTimeValid == FALSE)         /* if our current timestamp is invalid */
    {
      ToolsPerfQuery(&psMark1);                             /* get a new one */
      flTimeValid = TRUE;                            /* no we're valid again */
    }

    rc = select (&sockSource,                             /* check for input */
                 1,
                 0,
                 0,
                 pReflectionTCP->ulTimeoutFromClient);      /* timeout value */
    if (rc > 0)                                     /* if input is available */
    {
      ToolsPerfQuery(&psMark2);                       /* OK, incoming packet */
      flTimeValid = FALSE;                      /* need to get new timestamp */

      prssChannelSource->fTimeWait +=                   /* calculate seconds */
        psMark2.fSeconds - psMark1.fSeconds;

      prssParentSource->fTimeWait +=                    /* calculate seconds */
        psMark2.fSeconds - psMark1.fSeconds;


      /****************************
       * receiving request packet *
       ****************************/

      iBytesRead = recv(sockSource,            /* read bytes from the client */
                        pBuffer,
                        SOCKETBUFFERSIZE,
                        0);
      if (iBytesRead < 0)                  /* check if an error has occurred */
      {
        rc = sock_errno();                             /* get the error code */
        fprintf (stderr,
                 "\nError: Connection %s"
                 "\n       Thread     %s"
                 "\n       Error      %i"
                 "\n       Location   %s",
                 pReflectionTCP->pszName,
                 pszThreadName,
                 rc,
                 "recv");
        psock_errno("\n\rTCP recv::sockSource");
        prssChannelSource->ulErrorsFrom++;                     /* statistics */
        prssParentSource->ulErrorsFrom++;                      /* statistics */
        goto _bail_out;
      }
      else
        if (iBytesRead == 0)                        /* connection has closed */
        {
          rc = NO_ERROR;                    /* allright, we're shutting down */
          goto _bail_out;
        }
        else
        {
          prssChannelSource->ulPacketsFrom++;                  /* statistics */
          prssChannelSource->ulBytesFrom+=iBytesRead;
          prssParentSource->ulPacketsFrom++;                   /* statistics */
          prssParentSource->ulBytesFrom+=iBytesRead;
        }


      /*******************************
       * forwarding packet to server *
       *******************************/

      ToolsPerfQuery(&psMark1);

      iBytesWritten = send(sockDest,     /* forward the packet to the server */
                           pBuffer,
                           iBytesRead,
                           0);
      if (iBytesWritten == -1)
      {
        rc = sock_errno();                             /* get the error code */
        fprintf (stderr,
                 "\nError: Connection %s"
                 "\n       Thread     %s"
                 "\n       Error      %i"
                 "\n       Location   %s",
                 pReflectionTCP->pszName,
                 pszThreadName,
                 rc,
                 "send");
        psock_errno("\n\rTCP send::sockDest");
        prssChannelDest->ulErrorsTo++;                         /* statistics */
        prssParentDest->ulErrorsTo++;                          /* statistics */
        goto _bail_out;
      }
      else
      {
        prssChannelDest->ulPacketsTo++;                        /* statistics */
        prssChannelDest->ulBytesTo  += iBytesWritten;
        prssParentDest->ulPacketsTo++;                         /* statistics */
        prssParentDest->ulBytesTo   += iBytesWritten;
      }
    } /* select */
    else
      if (rc < 0)                         /* means a problem with the socket */
      {
        rc = sock_errno();                             /* get the error code */

        switch (rc)                  /* take action according to return code */
        {
          case 0:                                                /* no error */
          case SOCENOTSOCK:     /* this error is returned, when other thread */
                                /* closes the handles of the sockets which   */
                                /* means a normal shutdown of the reflection.*/
            rc = NO_ERROR;            /* Therefore it's OK and we ignore it. */
          break;

          default:
            fprintf (stderr,
                     "\nError: Connection %s"
                     "\n       Thread     %s"
                     "\n       Error      %i"
                     "\n       Location   %s",
                     pReflectionTCP->pszName,
                     pszThreadName,
                     rc,
                     "select");
            psock_errno("\n\rTCP select::sockSource");
            prssChannelSource->ulErrorsFrom++;                 /* statistics */
            prssParentSource->ulErrorsFrom++;                  /* statistics */
            break;
        }

        goto _bail_out;                       /* close this reflector thread */
      }
  }

_bail_out:
  pReflectionTCP->fTermination = TRUE;              /* request termination ! */

  soclose(sockSource);                                   /* close the socket */
  soclose(sockDest);                                     /* close the socket */
  free (pBuffer);                        /* free previously allocated memory */

  return (rc);
}


/***********************************************************************
 * Name      : THREAD ReflectionTCP
 * Funktion  : Create a new reflection thread
 * Parameter : -> THREAD prototype
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 1997/07/28 00.45.24]
 ***********************************************************************/

void __stdcall ReflectHelperCS_TCP(PVOID pParameters)
{
  PREFLECTIONIP pReflectionTCP = (PREFLECTIONIP)pParameters;   /* thread prm */
  APIRET rc;                                               /* API-Returncode */

  Globals.ulThreads++;       /* we just succeeded in creating another thread */

  rc = ReflectTransportTCP(pReflectionTCP,
                           "Client2Server",
                           pReflectionTCP->sockClient,
                           pReflectionTCP->sockServer,      /* the transport */
                           &pReflectionTCP->refstatIP.rssClient,
                           &pReflectionTCP->pParent->refstatIP.rssClient,
                           &pReflectionTCP->refstatIP.rssServer,
                           &pReflectionTCP->pParent->refstatIP.rssServer);
  pReflectionTCP->fTermination = TRUE;/* be sure other thread terminates too */

  printf ("\n%-20s: terminated (Client->Server).",
          pReflectionTCP->pszName);

  if (rc != NO_ERROR)                                    /* check for errors */
    ToolsErrorDosEx(rc,                               /* yield error message */
                    pReflectionTCP->pszName);

  pReflectionTCP->pParent->ulTCPConnectionsActive--;    /* update statistics */

  if (Globals.ulThreads > 0)                             /* prevent underrun */
    Globals.ulThreads--;        /* we're dying, so adjust the thread counter */

  DosWaitThread(&pReflectionTCP->tidIPThreadSC,     /* wait for other thread */
                DCWW_WAIT);

  RefManDelete(pReflectionTCP);              /* remove ourself from the list */
}


/***********************************************************************
 * Name      : THREAD ReflectionTCP
 * Funktion  : Create a new reflection thread
 * Parameter : -> THREAD prototype
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 1997/07/28 00.45.24]
 ***********************************************************************/

void __stdcall ReflectHelperSC_TCP(PVOID pParameters)
{
  PREFLECTIONIP pReflectionTCP = (PREFLECTIONIP)pParameters;   /* thread prm */
  APIRET rc;                                               /* API-Returncode */

  Globals.ulThreads++;       /* we just succeeded in creating another thread */

  rc = ReflectTransportTCP(pReflectionTCP,
                           "Server->Client",
                           pReflectionTCP->sockServer,
                           pReflectionTCP->sockClient,      /* the transport */
                           &pReflectionTCP->refstatIP.rssServer,
                           &pReflectionTCP->pParent->refstatIP.rssServer,
                           &pReflectionTCP->refstatIP.rssClient,
                           &pReflectionTCP->pParent->refstatIP.rssClient);
  pReflectionTCP->fTermination = TRUE;   /* sure other thread terminates too */

  printf ("\n%-20s: terminated (Server->Client).",
          pReflectionTCP->pszName);

  if (rc != NO_ERROR)                                     /* check for errors */
    ToolsErrorDosEx(rc,                                /* yield error message */
                    pReflectionTCP->pszName);

  if (Globals.ulThreads > 0)                             /* prevent underrun */
    Globals.ulThreads--;        /* we're dying, so adjust the thread counter */
}


/***********************************************************************
 * Name      : THREAD ReflectionTCP
 * Funktion  : Create a new reflection thread
 * Parameter : -> THREAD prototype
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 1997/07/28 00.45.24]
 ***********************************************************************/

void __stdcall ReflectionTCP (PVOID pParameters)
{
  PREFLECTIONIP pReflectionTCP = (PREFLECTIONIP)pParameters; /* thread prm */
  PREFLECTIONIP pReflectionCopy; /* copy of the pReflectionTCP structure    */
  int            rc;                        /* return code from TCP/IP stack */
  struct sockaddr_in sinAddress;         /* our interface for the connection */
  struct sockaddr_in sinClient;       /* address of the client that connects */
  struct sockaddr_in sinServer;                      /* the server's address */
  int            iClientNameLength;             /* length of the client name */
  CHAR           szHostName[32];                     /* buffer for inet_ntoa */

  Globals.ulThreads++;       /* we just succeeded in creating another thread */


  pReflectionTCP->sockListener = socket(PF_INET,          /* create a socket */
                                        SOCK_STREAM,
                                        IPPROTO_TCP);
  if (pReflectionTCP->sockListener == -1)           /* check socket creation */
  {
    rc = sock_errno();                                 /* get the error code */
    fprintf (stderr,
             "\nError: Connection %s"
             "\n       Error      %i"
             "\n       Location   %s",
             pReflectionTCP->pszName,
             rc,
             "socket(listener)");
    psock_errno("\n\rTCP sockListener::socket");
    goto _bail_out_TCP_2;
  }


  sinAddress.sin_family           = AF_INET;
  sinAddress.sin_port             = htons(pReflectionTCP->usClientPortSource);
  sinAddress.sin_addr.s_addr      = pReflectionTCP->ulLocalIPAddress;

  sinServer.sin_family            = AF_INET;
  sinServer.sin_port              = htons(pReflectionTCP->usServerPortSource);
  sinServer.sin_addr.s_addr       = pReflectionTCP->ulIPServer;

  sinClient.sin_family            = AF_INET;
  sinClient.sin_port              = 0;
  sinClient.sin_addr.s_addr       = INADDR_ANY;


  if (bind(pReflectionTCP->sockListener,        /* bind the socket to a name */
           (struct sockaddr *)&sinAddress,
           sizeof(sinAddress)) == SOCKET_ERROR)
  {
    rc = sock_errno();                                 /* get the error code */
    fprintf (stderr,
             "\nError: Connection %s"
             "\n       Error      %i"
             "\n       Location   %s",
             pReflectionTCP->pszName,
             rc,
             "bind(listener)");
    psock_errno("\n\rTCP sockListener::bind");
    goto _bail_out_TCP_1;
  }


  /* OK, this is the accept() loop */
  while(pReflectionTCP->fTermination == FALSE)
  {
    printf ("\n%-20s: Waiting for connection.",
            pReflectionTCP->pszName);

    /************************
     * listen to the socket *
     ************************/
    if (listen(pReflectionTCP->sockListener,             /* listen to socket */
               5) == SOCKET_ERROR)                  /* 5 is the queue length */
    {
      rc = sock_errno();                               /* get the error code */
      fprintf (stderr,
               "\nError: Connection %s"
               "\n       Error      %i"
               "\n       Location   %s",
               pReflectionTCP->pszName,
               rc,
               "listen(listener)");
      psock_errno("\n\rTCP sockListener::listen");
      goto _bail_out_TCP_1;
    }


    /*****************************************
     * accept the connection from the client *
     *****************************************/

    iClientNameLength = sizeof(sinClient);
    pReflectionTCP->sockClient = accept(pReflectionTCP->sockListener,
                                        (struct sockaddr *)&sinClient,
                                        &iClientNameLength);
    if (pReflectionTCP->sockClient == -1)
    {
      rc = sock_errno();                               /* get the error code */

      switch (rc)
      {
        case SOCENOTSOCK:                          /* socket has been closed */
          goto _bail_out_TCP_1;

        default:
          fprintf (stderr,
                   "\nError: Connection %s"
                   "\n       Error      %i"
                   "\n       Location   %s",
                   pReflectionTCP->pszName,
                   rc,
                   "accept(listener)");
          psock_errno("\n\rTCP sockClient::accept");
        break;
      }
    }
    else
    {
      /**************************************
       * establish connection to the server *
       **************************************/

      pReflectionTCP->sockServer = socket(PF_INET,        /* create a socket */
                                          SOCK_STREAM,
                                          IPPROTO_TCP);
      if (pReflectionTCP->sockServer == -1)         /* check socket creation */
      {
        rc = sock_errno();                             /* get the error code */
        fprintf (stderr,
                 "\nError: Connection %s"
                 "\n       Error      %i"
                 "\n       Location   %s",
                 pReflectionTCP->pszName,
                 rc,
                 "socket(server)");
        psock_errno("\n\rTCP sockServer::socket");
        soclose(pReflectionTCP->sockClient);      /* close client connection */
      }
      else
      {
        if (connect(pReflectionTCP->sockServer,     /* connect to the server */
               (struct sockaddr *)&sinServer,
               sizeof(sinServer)) == SOCKET_ERROR)
        {
          rc = sock_errno();                           /* get the error code */
          fprintf (stderr,
                   "\nError: Connection %s"
                   "\n       Error      %i"
                   "\n       Location   %s",
                   pReflectionTCP->pszName,
                   rc,
                   "connect(server)");
          psock_errno("\n\rTCP sockServer::connect");
                                             /* however continue to listen ! */
          soclose(pReflectionTCP->sockClient);    /* close client connection */
          soclose(pReflectionTCP->sockServer);    /* close server socket     */
        }
        else
        {
          IPAddressToString(sinClient.sin_addr,          /* convert the name */
                            szHostName,
                            sizeof(szHostName));

          printf ("\n%-20s: %s connected.",
                  pReflectionTCP->pszName,
                  szHostName);

          rc = RefManCreateFromTemplate(&pReflectionCopy,
                                        pReflectionTCP);
          if (rc != NO_ERROR)                 /* check for proper allocation */
          {
            ToolsErrorDosEx(rc,
                            "ReflectionTCP::RefManCreateFromTemplate");

            soclose(pReflectionTCP->sockClient);       /* close both sockets */
            soclose(pReflectionTCP->sockServer);
          }
          else
          {
            pReflectionCopy->pParent = pReflectionTCP;    /* set parent link */
            pReflectionTCP->ulTCPConnectionsActive++;   /* update statistics */
            pReflectionTCP->ulTCPConnectionsTotal++;
            pReflectionCopy->ucState = REFSTATE_TCP_SPAWN;

            pReflectionCopy->tidIPThreadSC = _beginthread(ReflectHelperSC_TCP,
                                                          16384,
                                                          (PVOID)pReflectionCopy);

            pReflectionCopy->tidIPThreadCS = _beginthread(ReflectHelperCS_TCP,
                                                          16384,
                                                          (PVOID)pReflectionCopy);
          } /* pReflectionCopy */
        } /* connect */
      } /* socket */
    } /* listen */
  }

                          /* OK, this thread received a request to terminate */
_bail_out_TCP_1:
  soclose(pReflectionTCP->sockListener);

_bail_out_TCP_2:
  pReflectionTCP->fTermination = TRUE;         /* request thread termination */

  if (Globals.ulThreads > 0)                             /* prevent underrun */
    Globals.ulThreads--;        /* we're dying, so adjust the thread counter */
}


/***********************************************************************
 * Name      : APIRET ReflectionCreate
 * Funktion  : Create a new reflection thread
 * Parameter : PSZ   pszName      - name of the reflection
 *             ULONG ulIPServer   - IP address of the forwarder server
 *             ULONG ulPortSource - the source port (client)
 *             ULONG ulPortDest   - the forwarder port (server)
 *             ULONG ulProtocol   - IPPROTO_UDP / IPPROTO_TCP
 * Variablen :
 * Ergebnis  : API return code
 * Bemerkung : no other threads may call inet_ntoa, get***, etc.
 *
 * Autor     : Patrick Haller [Donnerstag, 1997/07/28 00.45.24]
 ***********************************************************************/

APIRET ReflectionCreate(PSZ   pszName,
                        PSZ   pszIPServer,
                        ULONG ulLocalIPAddress,
                        ULONG ulPortSource,
                        ULONG ulPortDest,
                        PSZ   pszProtocol)
{
  CHAR            szName[128];                 /* buffer for building a name */
  CHAR            szServer[128];                      /* name of the server  */
  CHAR            szService1[32];                     /* name of the service */
  CHAR            szService2[32];                     /* name of the service */
  struct protoent *ppeProtocol;                            /* protocol entry */
  struct servent  *pseService;                             /* service entry  */
  struct hostent  *pheHost;                                /* host entry     */
  struct in_addr  inHost;                           /* host internet address */
  PSZ             pszServerName = "UNKNOWN";       /* hostname of the server */
  USHORT          usProtocol;                         /* the protocol number */
  PREFLECTIONIP   pReflectionIP;                /* the reflection parameters */
  APIRET          rc;                                      /* API returncode */
  int             iIPServer;                     /* address of the IP server */
  ULONG           ulIPServer = 0;              /* server IP address as ULONG */

  if ( (pszProtocol == NULL) ||                          /* check parameters */
       (pszIPServer == NULL) )
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */


  ppeProtocol = getprotobyname(pszProtocol);           /* query the protocol */
  if (ppeProtocol == NULL)                               /* check for errors */
  {
    fprintf (stderr,                                /* display error message */
            "\nError: protocol %s is unknown.",
            pszProtocol);
    return (ERROR_INVALID_FUNCTION);                /* raise error condition */
  }
  else
    usProtocol = ppeProtocol->p_proto;           /* save the protocol number */


  pseService = getservbyport(htons(ulPortSource),       /* query the service */
                             NULL);
  if (pseService != NULL)                                /* check for errors */
    strcpy(szService1,
           pseService->s_name);
  else
    sprintf(szService1,
            "#%u",
            ulPortSource);


  pseService = getservbyport(htons(ulPortDest),         /* query the service */
                             NULL);
  if (pseService != NULL)                                /* check for errors */
    strcpy(szService2,
           pseService->s_name);
  else
    sprintf(szService2,
            "#%u",
            ulPortSource);


  /**************************************************************
   * resolve stringIPAddress / hostname to numerical IP address *
   **************************************************************/
  iIPServer = inet_addr(pszIPServer);                /* try IP address first */
  if (iIPServer != -1)
  {
    inHost.s_addr = iIPServer;                        /* convert the address */
    ulIPServer    = iIPServer;             /* save the address in every case */

    if (Options.fsDontResolve)                    /* resolve the hostnames ? */
      pheHost = NULL;
    else
      pheHost = gethostbyaddr((char *)&inHost,          /* query information */
                              sizeof (struct in_addr),
                              AF_INET);
  }
  else
  {
    if (Options.fsDontResolve)                    /* resolve the hostnames ? */
      pheHost = NULL;
    else
    {
      pheHost = gethostbyname(pszIPServer);              /* get the hostname */
      if (pheHost != NULL)                    /* if we retrieved information */
      {
        struct sockaddr_in sinIPServer;        /* Internet address structure */

        sinIPServer.sin_family = pheHost->h_addrtype;

        if (pheHost->h_length > sizeof(sinIPServer.sin_addr))
          pheHost->h_length = sizeof(sinIPServer.sin_addr);

        memcpy(&sinIPServer.sin_addr,
               pheHost->h_addr,
               pheHost->h_length);

        ulIPServer = (ULONG)sinIPServer.sin_addr.s_addr;         /* map it ! */
      }
    }
  }



  if (pheHost == NULL)                        /* if information was returned */
  {
    if (ulIPServer == 0)
    {
      fprintf (stderr,                                /* yield error message */
               "\nError: Host %s is unknown.",
               pszIPServer);
      return (ERROR_BAD_NETPATH);                    /* return error message */
    }
    else
      fprintf (stderr,                                /* yield error message */
               "\nError: Host %s is unknown, using IP dddress.",
               pszIPServer);
  }


  if (pszName == NULL)                                   /* check parameters */
  {
    pszName = szName;                                /* build a default name */

    sprintf(szName,
            "%s-%s/%s-%s",
            pszServerName,
            pszProtocol,
            szService1,
            szService2);
  }


  /***********************************************************
   * fill out the parameter structures and start the threads *
   ***********************************************************/

  rc = RefManCreateFromParam(&pReflectionIP,
                             usProtocol,
                             pszName,
                             ulPortSource,
                             0,
                             ulPortDest,          /* usServerPortSource      */
                             0,                   /* usServerPortDestination */
                             0,                   /* ulSocketOptions         */
                             htonl(ulIPServer),
                             htonl(ulLocalIPAddress),
                             2000,                /* ulTimeoutFromClient     */
                             2000,                /* ulTimeoutToClient       */
                             30000,               /* ulTimeoutFromServer     */
                             30000);              /* ulTimeoutToServer       */
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                                    /* raise error condition */

  switch (usProtocol)
  {
    /*************************************
     * launch the TCP reflection threads *
     *************************************/
    case IPPROTO_TCP:
    {
                                                        /* launch the thread */
      pReflectionIP->ucState = REFSTATE_TCP_LISTENER;
      pReflectionIP->tidMainThread    = _beginthread(ReflectionTCP,
                                                     16384,
                                                     (PVOID)pReflectionIP);
      if (pReflectionIP->tidMainThread == -1)            /* check for launch */
      {
        fprintf (stderr,
                 "\nError: failed to launch thread.");
        return (ERROR_MAX_THRDS_REACHED);           /* raise error condition */
      }
    }
    break;

    /*************************************
     * launch the UDP reflection threads *
     *************************************/
    case IPPROTO_UDP:
    {
                                                        /* launch the thread */
      pReflectionIP->ucState          = REFSTATE_UDP;
      pReflectionIP->tidMainThread    = _beginthread(ReflectionUDP,
                                                     16384,
                                                     (PVOID)pReflectionIP);
      if (pReflectionIP->tidMainThread == -1)            /* check for launch */
      {
        fprintf (stderr,
                 "\nError: failed to launch thread.");
        return (ERROR_MAX_THRDS_REACHED);           /* raise error condition */
      }
    }
    break;

    /***********
     * default *
     ***********/
    default:
       fprintf(stderr,
               "\nError: protocol %s is not supported.",
               pszProtocol);
       return (ERROR_INVALID_FUNCTION);             /* raise error condition */
  }

  return (NO_ERROR);                                                   /* OK */
}


/***********************************************************************
 * Name      : void initialize
 * Funktion  : Initialisierung einiger Variablen
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 1997/07/28 00.52.41]
 ***********************************************************************/

APIRET initialize (void)
{
  APIRET rc;                                               /* API returncode */

  memset(&Options,
         0L,
         sizeof(Options));

  memset(&Globals,
         0L,
         sizeof(Globals));


  Globals.ulThreads       = 1;                    /* this is the main thread */
  Globals.pRootReflection = NULL;         /* no reflections active initially */
  Globals.ulReflections   = 0;            /* no reflections active initially */

  rc = DosCreateMutexSem(NULL,                          /* unnamed semaphore */
                         &Globals.hMutexAddressToString,
                         0,
                         FALSE);                  /* create in unowned state */
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                                    /* raise error condition */



#ifdef __OS2__
  sock_init();                                    /* initialize TCP/IP stack */
#endif

#ifdef _WIN32
  {
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = 0x0002;                      /* at least Winsock 2.0 */
    err = WSAStartup(wVersionRequested,           /* initialize TCP/IP stack */
                   &wsaData);

      printf ("\nMicrosoft Winsock %2u.%02u (%u)"
            " [%s %s]",
            LOBYTE(wsaData.wVersion),
            HIBYTE(wsaData.wVersion),
            wsaData.wHighVersion,
            wsaData.szDescription,
            wsaData.szSystemStatus);

    if (err != 0)
      /* Tell the user that we couldn't find a useable */
      /* winsock.dll.     */
      return(ERROR_IP_UNUSABLE_STACK);

    /* Confirm that the Windows Sockets DLL supports 1.1.*/
    /* Note that if the DLL supports versions greater */
    /* than 1.1 in addition to 1.1, it will still return */
    /* 1.1 in wVersion since that is the version we */
    /* requested. */

    if ( LOBYTE( wsaData.wVersion ) < 2 )
   {
        /* Tell the user that we couldn't find a useable */
        /* winsock.dll. */
        WSACleanup();
        return(ERROR_IP_UNUSABLE_STACK);
    }

    /* The Windows Sockets DLL is acceptable. Proceed. */
  }
#endif

  return (NO_ERROR);                                                   /* OK */
}


/***********************************************************************
 * Name      : int main
 * Funktion  : Hauptroutine
 * Parameter : int argc, char *argv[]
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 1997/07/28 00.53.13]
 ***********************************************************************/

int main (int argc, char *argv[])
{
  int rc;                                                    /* Rckgabewert */

  rc = initialize ();                                     /* Initialisierung */
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    exit(1);                                                /* abort program */
  }

  rc = ArgStandard (argc,                          /* CLI-Parameter parsen */
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

                                           /* perform some parameter mapping */
  if (!Options.fsConfig)                /* no configuration file specified ? */
    Options.pszConfig = "REFLECT.CFG";              /* then use this default */

  /* process configuration file */

  /* DNS */
  rc = ReflectionCreate("T22113-DNS-UDP",
                        "SGPS2",
                        INADDR_ANY,
                        53,
                        53,
                        "UDP");
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);


  rc = ReflectionCreate("T22113-DNS-TCP",
                        "SGPS2",
                        INADDR_ANY,
                        53,
                        53,
                        "TCP");
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  /* FTP */
  rc = ReflectionCreate("T22113-FTPDATA-UDP",
                        "T22113",
                        INADDR_ANY,
                        20,
                        20,
                        "UDP");
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);


  rc = ReflectionCreate("T22113-FTPDATA-TCP",
                        "T22113",
                        INADDR_ANY,
                        20,
                        20,
                        "TCP");
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);


  rc = ReflectionCreate("T22113-FTP-UDP",
                        "T22113",
                        INADDR_ANY,
                        21,
                        21,
                        "UDP");
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);


  rc = ReflectionCreate("T22113-FTP-TCP",
                        "T22113",
                        INADDR_ANY,
                        21,
                        21,
                        "TCP");
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  /* telnet */
  rc = ReflectionCreate("T22113-TELNET-UDP",
                        "T22113",
                        INADDR_ANY,
                        23,
                        23,
                        "UDP");
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);


  rc = ReflectionCreate("T22113-TELNET-TCP",
                        "T22113",
                        INADDR_ANY,
                        23,
                        23,
                        "TCP");
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  /* smtp */
  rc = ReflectionCreate("T22113-SMTP-UDP",
                        "T22113",
                        INADDR_ANY,
                        25,
                        25,
                        "UDP");
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);


  rc = ReflectionCreate("T22113-SMTP-TCP",
                        "T22113",
                        INADDR_ANY,
                        25,
                        25,
                        "TCP");
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  /* www */
  rc = ReflectionCreate("T22113-WWW-UDP",
                        "130.60.2.218",
                        INADDR_ANY,
                        80,
                        80,
                        "UDP");
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);


  rc = ReflectionCreate("T22113-WWW-TCP",
                        "T22113",
                        INADDR_ANY,
                        80,
                        80,
                        "TCP");
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  /* this is to be removed subsequently */
  rc = 0;
  for(;rc==0;)
  {
    char ch;

    ch = getch(); /* wait for the threads */

    switch (ch)
    {
      case 27:
      case 'X':
      case 'x':
        rc = RefManCloseAll();
        rc = 1; /* finished ! */
        break; /* quit the app */


      case 's':
      case 'S':
        printf ("\nStatistics"
                "\n  threads     : %u"
                "\n  reflections : %u",
                Globals.ulThreads,
                Globals.ulReflections);
        RefManListAll();
        break;

      case '?':
      case 'h':
      case 'H':
        printf ("\nX,x,ESC - quit the application"
                "\nS,s     - statistics"
                "\nH,h,?   - help");
        break;

    }
  }

  DosCloseMutexSem(Globals.hMutexAddressToString);    /* close the semaphore */

  return (rc);
}
