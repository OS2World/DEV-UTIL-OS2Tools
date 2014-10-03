/*****************************************************
 * Reflect Tool                                      *
 * Allows redirection of IP packets                  *
 * (c) 1997    Patrick Haller Systemtechnik          *
 *****************************************************/

#ifndef MODULE_REFLECT
#define MODULE_REFLECT



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

#include "phsarg.h"
#include "rconfig.h"


/*****************************************************************************
 * Defines                                                                   *
 *****************************************************************************/

/* #define DEBUG 1 */

#define ERROR_USER                  0xC000
#define ERROR_IP_UNKNOWN_PROTOCOL   ERROR_USER + 0
#define ERROR_IP_UNUSABLE_STACK     ERROR_USER + 1
#define ERROR_IP_SOCKET_ERROR       ERROR_USER + 2


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

#ifdef __OS2__
typedef int SOCKET;
typedef SOCKET *PSOCKET;
#endif


typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsConfig;        /* check whether configuration file was specified */
  ARGFLAG fsDontResolve;        /* dont resolve the hostnames / IP addresses */

  PSZ     pszConfig;                    /* holds the configuration file name */
} OPTIONS, *POPTIONS;


typedef struct _reflectstatstruct
{
  ULONG ulPacketsFrom;                      /* number of packets from client */
  ULONG ulPacketsTo;                        /* number of packets to   client */
  ULONG ulBytesFrom;                        /* number of bytes   from client */
  ULONG ulBytesTo;                          /* number of bytes   to   client */
  ULONG ulErrorsFrom;                       /* number of errors  from client */
  ULONG ulErrorsTo;                         /* number of errors  to   client */
  float fTimeWait;                                /* time waiting on packets */
} REFLECTSTATSTRUCT, *PREFLECTSTATSTRUCT;


typedef struct _reflectstat
{
  REFLECTSTATSTRUCT rssClient;           /* statistics for client connection */
  REFLECTSTATSTRUCT rssServer;           /* statistics for server connection */
  ULONG             ulTimeoutsServer;/* timeouts waiting for server response */
} REFLECTSTAT, *PREFLECTSTAT;


typedef struct _ReflectionIP
{
  struct _ReflectionIP *pNext;                                /* linked list */
  struct _ReflectionIP *pPrev;                                /* linked list */
  struct _ReflectionIP *pParent;        /* NOT part of the list, this is for */
                                        /* passing thru the statistics only. */
  
  USHORT usProtocol;                                   /* type of reflection */
  UCHAR  ucState;                                /* status of the reflection */

  PSZ    pszName;                                 /* name of this reflection */

  TID    tidMainThread;         /* thread identifier for the listener thread */
  TID    tidIPThreadCS;   /* thread identifier for the Client->Server thread */
  TID    tidIPThreadSC;   /* thread identifier for the Server->Client thread */
  BOOL   fTermination;             /* set TRUE to request thread termination */

  SOCKET sockListener;                 /* the listening socket for that port */
  SOCKET sockClient;         /* socket for the connection client<->reflector */
  SOCKET sockServer;         /* socket for the connection reflector<->server */
  USHORT usClientPortSource;             /* client sent request to this port */
  USHORT usClientPortDestination;      /* client expects answer on this port */
  USHORT usServerPortSource;          /* server expects request on this port */
  USHORT usServerPortDestination; /* server answers our request on this port */

  ULONG  ulSocketOptions;               /* bitmasked options for the sockets */

  ULONG  ulIPServer;                             /* IP address of the server */
  ULONG  ulLocalIPAddress;                          /* our interface address */
  
  ULONG  ulTimeoutFromClient;               /* timeout for receiving packets */
  ULONG  ulTimeoutToClient;                   /* timeout for sending packets */
  ULONG  ulTimeoutFromServer;               /* timeout for receiving packets */
  ULONG  ulTimeoutToServer;                   /* timeout for sending packets */

  ULONG  ulTCPConnectionsActive;         /* number of active TCP connections */
  ULONG  ulTCPConnectionsTotal;           /* number of total TCP connections */
  
  REFLECTSTAT refstatIP;                            /* reflection statistics */
} REFLECTIONIP, *PREFLECTIONIP;

#define REFSTATE_UNKNOWN      0
#define REFSTATE_UDP          1
#define REFSTATE_TCP_LISTENER 2
#define REFSTATE_TCP_SPAWN    3


typedef struct _TransportThread
{
  struct _TransportThread *pNext; /* pointer to the next thread in the chain */
  TID tidThread;                                    /* the thread identifier */
  PSZ pszName;                                         /* name of the thread */
} TRANSPORTTHREAD, *PTRANSPORTTHREAD;


typedef struct _Globals
{
  REFLECTSTAT refstatIP;                                   /* UDP statistics */
  REFLECTSTAT refstatTCP;                                  /* TCP statistics */
  
  ULONG ulThreads;                        /* total number of threads running */
  
  HMTX  hMutexAddressToString;                     /* serializes inet_ntoa() */
  HMTX  hMutexRefManDelete;                       /* serializes RefManDelete */
  
  PREFLECTIONIP pRootReflection;              /* the root of the linked list */
  ULONG         ulReflections;           /* number of registered reflections */

  PVARLIST      pConfiguration;                        /* configuration data */
} GLOBALS, *PGLOBALS;


/*****************************************************************************
 * Externalize                                                               *
 *****************************************************************************/
extern GLOBALS Globals;
extern OPTIONS Options;

#endif /* MODULE_REFLECT */

