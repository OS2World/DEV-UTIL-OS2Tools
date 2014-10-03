/*****************************************************
 * IPScan Tool                                       *
 * Reports information about TCP/IP devices.         *
 * (c) 1997    Patrick Haller Systemtechnik          *
 *****************************************************/

 /* Remark: not all resources are freed explicitly. Some arrays and strings
            are deallocated when the process termiantes. Me's a lazy bone...
  */


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#include <stdlib.h>


#ifdef __OS2__
  #define INCL_DOSFILEMGR
  #define INCL_BASE
  #define INCL_NOPMAPI
  #include <os2.h>

  #define TCPV40HDRS
  #include <netinet/in.h>
  #include <types.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <sys/select.h>
  #include <net/if_arp.h>
  #include <netdb.h>
  #include <netinet/in_systm.h>
  #include <netinet/ip.h>
  #include <netinet/ip_icmp.h>
  #include <netinet/udp.h>
  #define sockErrno sock_errno()
  #define SOCKET_ERROR -1
#endif

#ifdef _WIN32
                         /* @@@PH Yes, Microsoft is most famous and popular  */
                         /* for using easy-to-understand macros ...          */
  #define WIN32_LEAN_AND_MEAN
  #include <winsock2.h>
  #include <windows.h>
  #include "win32/ipscanw32.h"
  typedef void (__cdecl *THREAD)(PVOID pParameters);
  #define MAXHOSTNAMELEN 64
  #ifndef errno
    #define errno WSAGetLastError()
  #endif
  #define EPERM 1
  #define EINTR WSAEINTR
  #define ETIMEDOUT WSAETIMEDOUT
  #define ECONNREFUSED WSAECONNREFUSED
  #define DosSleep Sleep
  #define soclose closesocket
  #define sockErrno WSAGetLastError()
  #define PID int
  #define TID int
  #define AF_NB 17

  /* @@@PH General Windows Annotations:
     To use SOCK_RAW type sockets, you must apparently have administrator
         priviledge. Furthermore this program will only run on Winsock 2 DLLs. */
#endif


#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <process.h>
#include <time.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"

#ifndef MAXPATHLEN
  #define MAXPATHLEN 260
#endif


/*****************************************************************************
 * Defines                                                                   *
 *****************************************************************************/

#define DEBUG 1

#define ERROR_USER                  0xC000
#define ERROR_IP_UNKNOWN_PROTOCOL   ERROR_USER + 0
#define ERROR_IP_UNUSABLE_STACK     ERROR_USER + 1
#define ERROR_IP_SOCKET_ERROR       ERROR_USER + 2
#define ERROR_IP_THREAD_FAILED      ERROR_USER + 3
#define ERROR_IP_UNKNOWN_HOST       ERROR_USER + 4
#define ERROR_IP_OUT_OF_RANGE       ERROR_USER + 5


#define IPPROBESIGNATURE htonl(0xdeadface)
#define IPTIMEOUTMULT    1000


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

#ifdef __OS2__
  typedef int SOCKET;
  typedef SOCKET *PSOCKET;
#endif


#define HOSTMAXIPADDRESSES 3
#define HACT   uStatus.sOK
#define HINACT uStatus.sProblem

#define HOSTSTATUS_UNKNOWN   0x00     /* host status has not been determined */
#define HOSTSTATUS_OK        0x01       /* host should be OK, reachable, ... */
#define HOSTSTATUS_PROBLEM   0x02           /* we received ICMP error report */
#define HOSTSTATUS_GATEWAY   0x04               /* this machine is a gateway */
#define HOSTSTATUS_BROADCAST 0x08            /* multiple replies to one ping */


typedef struct _icmpinfo
{
  USHORT usICMPCode;         /* store some additional information about host */
  USHORT usICMPType;       /* i.e. what ICMP response messages came back     */
  ULONG  ulGatewayAddress;          /* IP address of gateway for redirection */
} ICMPINFO, *PICMPINFO;


typedef struct _ScanHost
{
  ULONG ulIP;                                 /* the IP address of this host */
  ULONG ulIPAlias[HOSTMAXIPADDRESSES];        /* the IP aliases of this host */
  PSZ   pszHostname;                                  /* registered hostname */
  UCHAR ucIPAddresses;                       /* number of IP addresses known */
  UCHAR ucKnown;                                          /* known / unknown */
  UCHAR ucReplies;               /* number of replies received for this host */
  UCHAR ucHostStatus;                                  /* status of the host */
  union
  {
    struct
    {
      BOOL   flResponseMeasured;       /* indicates if this is the 1st value */
      double fResponse;                             /* fastest response time */
      BOOL   flActive;                                  /* active / inactive */
    } sOK;

    ICMPINFO sProblem;       /* case of problem: here's the ICMP information */
  } uStatus;
} SCANHOST, *PSCANHOST;


typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsNetMask;                     /* class of internet network: A,B,C */
  ARGFLAG fsIP;                                              /* base address */
  ARGFLAG fsListFilter;                            /* only list active nodes */
  ARGFLAG fsTimeout;                            /* timeout      was spcified */
  ARGFLAG fsDataLength;                         /* data length was specified */
  ARGFLAG fsTTL;                                             /* time to live */
  ARGFLAG fsWait;                            /* time to wait between packets */
  ARGFLAG fsDontResolve;                           /* dont resolve the nodes */
  ARGFLAG fsDontScanServices;                  /* dont scan for the services */
  ARGFLAG fsQuiet;                                        /* quiet operation */
  ARGFLAG fsPorts;                                      /* scan active ports */
  ARGFLAG fsPortStart;                                  /* start port number */
  ARGFLAG fsPortEnd;                                      /* end port number */
  ARGFLAG fsPortsKnown;                         /* scan only the known ports */
  ARGFLAG fsIPAlias;                            /* scan for all IP addresses */
  ARGFLAG fsHostsFile;               /* we've got to write a /etc/hosts file */

  PSZ     pszIP;                          /* start with this base IP address */
  PSZ     pszNetMask;                                /* this is the netclass */

  ULONG   ulDataLength;                      /* PING: length of data packets */
  ULONG   ulTimeout;                   /* PING: timeout for incoming packets */
  ULONG   ulTTL;                                             /* time to live */
  ULONG   ulWait;                    /* time to wait between sending packets */

  ULONG   ulServiceStart;                /* number of first protocol to scan */
  ULONG   ulServiceEnd;                   /* number of last protocol to scan */

  PSZ     pszHostsFile;              /* we've got to write a /etc/hosts file */
} OPTIONS, *POPTIONS;


typedef struct _Globals
{
  SOCKET sockSend;                                            /* send socket */
  SOCKET sockReceive;                                      /* receive socket */

  ULONG ulHostsScanned;                        /* counts total scanned nodes */
  ULONG ulHostsPinged;                          /* counts total pinged nodes */
  ULONG ulHostsKnown;                                  /* counts known nodes */
  ULONG ulHostsUnknown;                              /* counts unknown nodes */
  ULONG ulHostsActive;                              /* count of active nodes */
  ULONG ulHostsInactive;                          /* count of inactive nodes */

  double fTimeReply;       /* time to wait for the reply of the pinged hosts */
  double fTimeScan;                /* time to scan all nodes gethostbyaddr() */
  double fTimeSend;                /* time to send all probe packets         */
  double fTimeConnectOK;          /* times to establish connections properly */
  ULONG  ulConnectOK;                        /* number of proper connections */
  double fTimeConnectFailed;            /* time spent on failing connections */
  ULONG  ulConnectFailed;                    /* number of failed connections */
  double fTimeTotal;                                /* total program runtime */


  PID   pidIdentifier;                             /* our process identifier */

  PSCANHOST arrScanHost;                    /* pointer to the scanhost array */
  ULONG    ulIPStart;                           /* this is the start address */
  ULONG    ulIPEnd;                                   /* this is the last IP */

  char** arrScanService;                 /* pointer to the scanservice array */
  ULONG    ulServiceStart;                         /* index of the 1st entry */
  ULONG    ulServiceEnd;                          /* index of the last entry */

  CHAR  szLastSent[20];        /* buffer for the IP address of the last host */
  CHAR  szLastRecv[20];        /* buffer for the IP address of the last host */

  ULONG    ulTimeoutCounter;                    /* counts timeouts on recv() */
} GLOBALS, *PGLOBALS;


typedef struct _PacketProbe
{
  struct ip   IP;                                   /* IP part of the packet */
  struct icmp ICMP;                                   /* ICMP part of packet */
  ULONG       ulSignature;         /* a special signature for identification */
  PERFSTRUCT  psSent;        /* exact timer information when packet was sent */
  PERFSTRUCT  psReceived;    /* exact timer information when packet was recv */
  ULONG       ulIPSentTo;       /* IP address of host the packet was sent to */
  ULONG       ulIdentifier;   /* as NT falls flat face with IPROTO_RAW, this */
   /* is our very own icmp_id replacement. Hopefully M$ can't screw this up. */
} PACKETPROBE, *PPACKETPROBE;


typedef struct _ThdFireParameters
{
  APIRET     rc;     /* thread can use this as transport for the return code */
  PERFSTRUCT psStart;                                /* start of the pinging */
  PERFSTRUCT psEnd;                                    /* end of the pinging */
  BOOL       fActive;         /* this flag signals the current thread status */
} THDFIRE, *PTHDFIRE;


typedef enum _ScanHostType        /* enumeration types for host entry update */
{
  SH_IPADDRESS,
  SH_IPALIAS,
  SH_HOSTNAME,
  SH_RESPONSE,
  SH_ACTIVE,
  SH_REPLY_INC,
  SH_KNOWN,
  SH_STATUS_SET,
  SH_STATUS_UNSET,
  SH_STATUS_ICMP,
  SH_STATUS_ICMP_GATEWAY,
  SH_QUERYEXISTS
} SCANHOSTTYPE;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */
GLOBALS Globals;

ARGUMENT TabArguments[] =
{ /*Token---------Beschreibung----------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/?",          "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",          "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/QUIET",      "Quiet operation.",   NULL,                 ARG_NULL,       &Options.fsQuiet},
  {"/Q",          "Quiet operation.",   NULL,                 ARG_NULL |
                                                              ARG_HIDDEN,     &Options.fsQuiet},
  {"/FILTER",     "Don't list inactive, unknown nodes.",
                                        NULL,                 ARG_NULL,       &Options.fsListFilter},
  {"/!RESOLV",    "Don't resolve the IP addresses to "
                  "hostnames.",         NULL,                 ARG_NULL,       &Options.fsDontResolve},
  {"/!SERVICES",  "Don't scan the service names.",
                                        NULL,                 ARG_NULL,       &Options.fsDontScanServices},
  {"/ALIAS",      "Scan for alternate IP addresses.",
                                        NULL,                 ARG_NULL,       &Options.fsIPAlias},
  {"/NETMASK=",   "TCP/IP network "
                  "mask.",              &Options.pszNetMask,  ARG_PSZ,        &Options.fsNetMask},
  {"/TIMEOUT=",   "Time in seconds to wait for "
                  "reply packets.",     &Options.ulTimeout,   ARG_ULONG,      &Options.fsTimeout},
  {"/TTL=",       "Time to live for "
                  "reply packets.",     &Options.ulTTL,       ARG_ULONG,      &Options.fsTTL},
  {"/SIZE=",      "Length of "
                  "the ICMP packets.",  &Options.ulDataLength,ARG_ULONG,      &Options.fsDataLength},
  {"/WAIT=",      "Time to wait between sending "
                  "the ICMP packets.",  &Options.ulWait,      ARG_ULONG,      &Options.fsWait},
  {"/PORTS",      "Scan hosts for active known TCP services.",
                                        NULL,                 ARG_NULL,       &Options.fsPorts},
  {"/PORTS.KNOWN","Scan only the known ports (/etc/services).",
                                        NULL,                 ARG_NULL,       &Options.fsPortsKnown},
  {"/PORTS.START=","Starting port scan with this TCP port"
                  " number.",           &Options.ulServiceStart,ARG_ULONG,    &Options.fsPortStart},
  {"/PORTS.END=", "Ending port scan with this TCP port"
                  " number.",           &Options.ulServiceEnd,ARG_ULONG,      &Options.fsPortEnd},
  {"/HOSTS=",     "Write a /etc/hosts file created as this filename.",
                                        &Options.pszHostsFile,ARG_PSZ,        &Options.fsHostsFile},
  {"1",           "Start with this "
                  "IP address / host.", &Options.pszIP,       ARG_PSZ     |
                                                              ARG_DEFAULT |
                                                              ARG_MUST,       &Options.fsIP},
  ARG_TERMINATE
};



/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void   help                  (void);

void   StrIPToString         (ULONG ulIPAddress,
                              PSZ   pszString);

USHORT NetIPChecksum         (PUSHORT pusAddress,
                              ULONG   ulLength);

PSZ    NetIPQueryAddressType (ULONG ulAddressType);

APIRET NetStatistics         (void);

void   ThdFirePackets        (PVOID pDummy);

APIRET NetHostInfo           (ULONG ulIP);

APIRET NetHostPing           (ULONG ulIP,
                              PVOID pPacket);

APIRET NetScanLoop           (void);

APIRET HostUpdate            (ULONG        ulIP,
                              SCANHOSTTYPE nScanHostType,
                              ULONG        ulData);

APIRET initialize            (void);

int    main                  (int,
                              char **);


/*****************************************************************************
 * Name      : void help
 * Funktion  : Anzeigen des Hilfetextes.
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.45.24]
 *****************************************************************************/

void help (void)
{
  TOOLVERSION("IPScan",                                  /* application name */
              0x00010404,                            /* application revision */
              0x00010100,           /* minimum required revision of PHSTOOLS */
              0x0001FFFF,        /* maximum recommended revision of PHSTOOLS */
              NULL,                                                /* Remark */
              NULL);                                 /* additional copyright */
}


/*****************************************************************************
 * Name      : void StrIPToString
 * Funktion  : Transform an IP address into a string
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung : User is responsible for providing at least 14 bytes.
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

void StrIPToString (ULONG ulIPAddress,
                    PSZ   pszString)
{
  sprintf (pszString,
           "%u.%u.%u.%u",
           ulIPAddress >> 24,
           (ulIPAddress >> 16) & 0x000000FF,
           (ulIPAddress >> 8)  & 0x000000FF,
           ulIPAddress & 0x000000FF);
}


/*****************************************************************************
 * Name      : void DisplayProgress
 * Funktion  : Display the current scanning status
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung : User is responsible for providing at least 14 bytes.
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

void DisplayProgress (void)
{
  if (!Options.fsQuiet)
    fprintf (stderr,
             "\rSent (%7u) %-16s   Recv (%7u) %-16s  Wait (%03u)",
             Globals.ulHostsPinged,
             Globals.szLastSent,
             Globals.ulHostsActive,
             Globals.szLastRecv,
             Globals.ulTimeoutCounter);
}


/*****************************************************************************
 * Name      : PSZ NetIPQueryAddressType
 * Funktion  : Transform an IP address into a string
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung : User is responsible for providing at least 14 bytes.
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/


PSZ NetIPQueryAddressType (ULONG ulAddressType)
{
  static PSZ TableAddressType[] =
  {
    /* AF_UNSPEC    0   */ "unspecified",
    /* AF_UNIX      1   */ "local to host (pipes, portals)",
    /* AF_INET      2   */ "internetwork: UDP, TCP, etc.",
    /* AF_IMPLINK   3   */ "arpanet imp addresses",
    /* AF_PUP       4   */ "pup protocols: e.g. BSP",
    /* AF_CHAOS     5   */ "mit CHAOS protocols",
    /* AF_NS        6   */ "XEROX NS protocols",
    /* AF_NBS       7   */ "nbs protocols",
    /* AF_ECMA      8   */ "european computer manufacturers",
    /* AF_DATAKIT   9   */ "datakit protocols",
    /* AF_CCITT     10  */ "CCITT protocols, X.25 etc",
    /* AF_SNA       11  */ "IBM SNA",
    /* AF_DECnet    12  */ "DECnet",
    /* AF_DLI       13  */ "Direct data link interface",
    /* AF_LAT       14  */ "LAT",
    /* AF_HYLINK    15  */ "NSC Hyperchannel",
    /* AF_APPLETALK 16  */ "Apple Talk",
    /* AF_NB        17  */ "Netbios"
  };

  if (ulAddressType > AF_NB)
    return("<unknown address type>");

  return(TableAddressType[ulAddressType]);
}


/*****************************************************************************
 * Name      : APIRET HostQuery
 * Funktion  : query a pointer to the host with a certain IP address
 * Parameter :
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET HostQuery (ULONG     ulIP,
                  PSCANHOST *ppScanHost)
{
  if (ppScanHost == NULL)                                /* check parameters */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */

  if ( (ulIP < Globals.ulIPStart) ||
      (ulIP > Globals.ulIPEnd) )
  {
    *ppScanHost = (PSCANHOST) NULL;

    return (ERROR_IP_OUT_OF_RANGE);                 /* raise error condition */
  }

  *ppScanHost = &Globals.arrScanHost[ulIP - Globals.ulIPStart];    /* target */

  return (NO_ERROR);
}


/*****************************************************************************
 * Name      : APIRET HostUpdate
 * Funktion  : update the host record in the global array
 * Parameter :
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET HostUpdate (ULONG        ulIP,
                   SCANHOSTTYPE nScanHostType,
                   ULONG        ulData)
{
  PSCANHOST pScanHost;               /* pointer to the target host structure */

  if ( (ulIP < Globals.ulIPStart) ||
       (ulIP > Globals.ulIPEnd) )
    return (ERROR_IP_OUT_OF_RANGE);                 /* raise error condition */


  pScanHost = &Globals.arrScanHost[ulIP - Globals.ulIPStart];      /* target */

  switch (nScanHostType)
  {
    /*************************************************************************
     * Assign new IP address to that record                                  *
     *************************************************************************/
    case SH_IPADDRESS:
      pScanHost->ulIP = ulData;
      break;

    /*************************************************************************
     * Assign new IP alias to that record                                    *
     *************************************************************************/
    case SH_IPALIAS:
      if (ulData == ulIP)     /* don't record the address itself as an alias */
        break;

      if (pScanHost->ucIPAddresses < HOSTMAXIPADDRESSES)     /* registration */
        pScanHost->ulIPAlias[pScanHost->ucIPAddresses] = ulData;

      pScanHost->ucIPAddresses++;     /* increase index counter for IP array */
      break;


    /*************************************************************************
     * Assign new hostname   to that record                                  *
     *************************************************************************/
    case SH_HOSTNAME:
      if (pScanHost->pszHostname != NULL)             /* already allocated ? */
        free (pScanHost->pszHostname);                     /* then free it ! */

      if (ulData == 0L)                           /* check for NULL argument */
        return (ERROR_INVALID_PARAMETER);           /* raise error condition */

      pScanHost->pszHostname = strdup ( (PSZ)ulData ); /* copy the casted str*/
      if (pScanHost->pszHostname == NULL)                /* check allocation */
        return (ERROR_NOT_ENOUGH_MEMORY);           /* raise error condition */

      break;

    /*************************************************************************
     * Assign new response time to that record                               *
     *************************************************************************/
    case SH_RESPONSE:
      if (pScanHost->HACT.flResponseMeasured == FALSE)    /* 1st measurement */
      {
        pScanHost->HACT.fResponse = (double)ulData;    /* reponse time in ms */

        pScanHost->HACT.flResponseMeasured = TRUE; /*now only faster replies */
      }
      else
        if (pScanHost->HACT.fResponse > (double)ulData)  /* fastest response */
          pScanHost->HACT.fResponse = (double)ulData;

      break;

    /*************************************************************************
     * Set this record to in/active                                          *
     *************************************************************************/
    case SH_ACTIVE:
      pScanHost->HACT.flActive = (BOOL)ulData;
      break;


    /*************************************************************************
     * Set this record to in/active                                          *
     *************************************************************************/
    case SH_REPLY_INC:
      if (pScanHost->ucReplies < 255)             /* count the "activations" */
        pScanHost->ucReplies++;
      break;

    /*************************************************************************
     * Set this record to known / unknown                                    *
     *************************************************************************/
    case SH_KNOWN:
      pScanHost->ucKnown = (BOOL)ulData;
      break;

    /*************************************************************************
     * Query if this ulIP address exists                                     *
     *************************************************************************/
    case SH_QUERYEXISTS:
                      /* nop, in every other case ERROR_xxx will be returned */
      break;

    /*************************************************************************
     * Set currently determined host status                                  *
     *************************************************************************/
    case SH_STATUS_SET:
      pScanHost->ucHostStatus |= (UCHAR)ulData;
      break;

    /*************************************************************************
     * Set currently determined host status                                  *
     *************************************************************************/
    case SH_STATUS_UNSET:
      pScanHost->ucHostStatus &= (UCHAR)~ulData;
      break;


    /*************************************************************************
     * Set currently determined host status, ICMP part                       *
     *************************************************************************/
    case SH_STATUS_ICMP:
      pScanHost->HINACT.usICMPType = (USHORT)(ulData >> 16);
      pScanHost->HINACT.usICMPCode = (USHORT)(ulData & 0x0000FFFF);
      break;


    /*************************************************************************
     * Set currently determined host status, ICMP part                       *
     *************************************************************************/
    case SH_STATUS_ICMP_GATEWAY:
      pScanHost->HINACT.ulGatewayAddress = ulData;
      break;


    /*************************************************************************
     * Or else ...                                                           *
     *************************************************************************/

    default:
      return (ERROR_INVALID_PARAMETER);             /* raise error condition */
  }

  return (NO_ERROR);                                                   /* OK */
}


/***********************************************************************
 * Name      : APIRET NetHostScanPorts
 * Funktion  : Scans a host for active TCP protocols
 * Parameter :
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 ***********************************************************************/

APIRET NetHostScanPorts(ULONG ulIP)
{
  ULONG  ulService;                               /* current protocol number */
  CHAR   szServiceName[80];                /* local buffer for protocol name */
  PSZ    pszService;                          /* points to the protocol name */
  SOCKET sockTest;                             /* this is out testing socket */
  struct sockaddr_in sinAddress;                      /* our testing address */
  int    irc;                                         /* integer return code */
  PERFSTRUCT psConnectStart;             /* benchmarking of the TCP/IP stack */
  PERFSTRUCT psConnectEnd;


  /***************************************************************************
   * The testing loop                                                        *
   ***************************************************************************/

  for (ulService =  Globals.ulServiceStart;
       ulService <= Globals.ulServiceEnd;
       ulService++)
  {
    if (Globals.arrScanService != NULL)          /* check for known services */
      pszService = Globals.arrScanService[ulService - Globals.ulServiceStart];
    else
      pszService = NULL;

    if (Options.fsPortsKnown)                   /* scan only the known ports */
      if (pszService == NULL)                /* and if this one is NOT known */
        continue;                     /* then skip the scanning of this port */

    if (pszService == NULL)                              /* check for errors */
    {
      sprintf (szServiceName,
               "<TCP #%u>",
               ulService);
      pszService = szServiceName;                    /* point to this buffer */
    }


  /***************************************************************************
   * Opening the Ping socket                                                 *
   ***************************************************************************/
#ifdef __OS2__
  sockTest = socket(AF_INET,                              /* create a socket */
                    SOCK_STREAM,
                    IPPROTO_TCP);
  if ( sockTest < 0)
    return (sockErrno);                             /* raise error condition */
#endif

#ifdef _WIN32
  sockTest = WSASocket (AF_INET,
                        SOCK_STREAM,
                        IPPROTO_TCP,
                        NULL,
                        0,
                        WSA_FLAG_OVERLAPPED);
  if ( sockTest == INVALID_SOCKET)
    return (sockErrno);                             /* raise error condition */

#endif

    sinAddress.sin_family      = AF_INET;
    sinAddress.sin_addr.s_addr = 0;
    sinAddress.sin_port        = 0;
    irc = bind(sockTest,
               (struct sockaddr *)&sinAddress,
               sizeof(sinAddress));
    /*printf ("\nDEBUG: bind=%i",irc);*/

                                           /* OK, try to connect to the port */
    sinAddress.sin_family      = AF_INET;
    sinAddress.sin_addr.s_addr = htonl(ulIP);
    sinAddress.sin_port        = htons((USHORT)ulService);

    ToolsPerfQuery(&psConnectStart);
    irc=connect(sockTest,
                (struct sockaddr *)&sinAddress,
                sizeof(sinAddress));
    ToolsPerfQuery(&psConnectEnd);

    if (irc != SOCKET_ERROR)
    {
      double fDiff = psConnectEnd.fSeconds - psConnectStart.fSeconds;

      Globals.ulConnectOK++;      /* increase counter for proper connections */
      Globals.fTimeConnectOK += fDiff;

      printf ("\n  %-21s (%5u)"           /* print the service on the screen */
              "                            %10.3fms",
              pszService,
              ulService,
              fDiff * 1000);
    }
    else
    {
      Globals.ulConnectFailed++;  /* increase counter for failed connections */
      Globals.fTimeConnectFailed += (psConnectEnd.fSeconds -
                                     psConnectStart.fSeconds);

      /* check for certain errors maybe we've got to abort probing this host */
      switch (sockErrno)
      {
        case 0:                                                  /* no error */
          break;

        case ETIMEDOUT:
          soclose(sockTest);                      /* close the socket anyway */
          fprintf(stderr,                             /* yield error message */
                  "\n  timeout, aborting.");
          return (NO_ERROR);         /* leave procedure in controlled manner */

        case ECONNREFUSED:          /* ok, host is not active on this port ! */
          break;

        default:
          perror("Scanning services");         /* yield the OS error message */
          break;                            /* default is no specific action */
      }
    }

    soclose(sockTest);
  }

  return (NO_ERROR);
}


/*****************************************************************************
 * Name      : APIRET NetHostInfo
 * Funktion  : gethostbyaddr
 * Parameter :
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET NetHostInfo(ULONG ulIP)
{
  struct in_addr     inHost;                    /* IP address of target host */
  struct hostent     *pheHost;                           /* host information */
  CHAR               szHostScanned[18];
  struct sockaddr_in sinHost;                  /* Internet address structure */
  CHAR               szHostName[128];             /* buffer for the hostname */

  memset (&inHost,                                               /* zero out */
          0,
          sizeof (inHost) );

  StrIPToString (ulIP,                       /* map the scanned host address */
                 szHostScanned);

  inHost.s_addr = htonl(ulIP);                                 /* IP address */

  Globals.ulHostsScanned++;
  pheHost = gethostbyaddr((char *)&inHost,              /* query information */
                          sizeof (struct in_addr),
                          AF_INET);
  if (pheHost != NULL)                                   /* check for errors */
  {
    Globals.ulHostsKnown++;                             /* adjust statistics */

                                      /* update the record in the host array */
    HostUpdate(ulIP,
               SH_IPADDRESS,
               ulIP);

    HostUpdate(ulIP,
               SH_KNOWN,
               TRUE);

    HostUpdate(ulIP,
               SH_HOSTNAME,
               (ULONG)pheHost->h_name);


    if (Options.fsIPAlias)              /* scan for alternate IP addresses ? */
    {
      strcpy (szHostName,                               /* save the hostname */
              pheHost->h_name);

      pheHost = gethostbyname(pheHost->h_name);     /* 2nd query reports the */
                                          /* alias IP addresses for the host */
      if (pheHost == NULL)          /* oops, 2nd query failed unexpectedly ! */
      {
        fprintf (stderr,
                 "\nWarning: record for %s (%s) missing in DNS !"
                 "\n         Please check your DNS configuration.",
                 szHostName,
                 szHostScanned);
      }
      else
      {
        sinHost.sin_family = pheHost->h_addrtype;   /* copy the address type */

        if (pheHost->h_length > sizeof(sinHost.sin_addr))    /* limit length */
          pheHost->h_length = sizeof(sinHost.sin_addr);

        {
          int i;

          for (i=0;
               pheHost->h_addr_list[i] != NULL;
               i++)
          {
            memcpy(&sinHost.sin_addr,
                   pheHost->h_addr_list[i],
                   pheHost->h_length);

                                                 /* check whether real alias */
            if (htonl(ulIP) != sinHost.sin_addr.s_addr)
              HostUpdate(ulIP,                 /* then register the IP alias */
                         SH_IPALIAS,
                         htonl(sinHost.sin_addr.s_addr));
          }
        }
      } /* gethostbyname */
    } /* Options.fsIPAlias */
  }
  else
    Globals.ulHostsUnknown++;                           /* adjust statistics */

  return (NO_ERROR);
}



/*****************************************************************************
 * Name      : void NetICMPDecode
 * Funktion  : prints ICMP information
 * Parameter :
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

void NetICMPDecode(FILE *fOutput,
                   PICMPINFO pICMPInfo)
{
#define ICMPCODE(a,b) case a: fprintf(fOutput,b); break;

  switch(pICMPInfo->usICMPType)
  {
    ICMPCODE(ICMP_ECHOREPLY,"Echo Reply")             /* XXX ID + Seq + Data */

    /*************************************************************************
     * Destination unreachable                                               *
     *************************************************************************/

    case ICMP_UNREACH:
      switch(pICMPInfo->usICMPCode)
      {
        ICMPCODE(ICMP_UNREACH_NET,      "destination net unreachable")
        ICMPCODE(ICMP_UNREACH_HOST,     "destination host unreachable")
        ICMPCODE(ICMP_UNREACH_PROTOCOL, "destination protocol unreachable")
        ICMPCODE(ICMP_UNREACH_PORT,     "destination port unreachable")
        ICMPCODE(ICMP_UNREACH_NEEDFRAG, "frag needed and DF set")
        ICMPCODE(ICMP_UNREACH_SRCFAIL,  "source route failed")
        default: fprintf(fOutput,
                         "destination unreachable, code: %d",
                         pICMPInfo->usICMPCode);
          break;
      }
      break;

    ICMPCODE(ICMP_SOURCEQUENCH,"source quench");

    /*************************************************************************
     * Redirection                                                           *
     *************************************************************************/

    case ICMP_REDIRECT:
      switch(pICMPInfo->usICMPCode)
      {
        ICMPCODE(ICMP_REDIRECT_NET,    "redirect network")
        ICMPCODE(ICMP_REDIRECT_HOST,   "redirect host")
        ICMPCODE(ICMP_REDIRECT_TOSNET, "redirect type of service and network")
        ICMPCODE(ICMP_REDIRECT_TOSHOST,"redirect type of service and host")
        default: fprintf(fOutput,
                         "redirect, code: %d",
                         pICMPInfo->usICMPCode);
          break;
      }
      fprintf(fOutput,
              "(new addr: 0x%08lx)",
               pICMPInfo->ulGatewayAddress);
      break;

    ICMPCODE(ICMP_ECHO,"echo request");

    case ICMP_TIMXCEED:
      switch(pICMPInfo->usICMPCode)
      {
        ICMPCODE(ICMP_TIMXCEED_INTRANS,"time to live exceeded")
        ICMPCODE(ICMP_TIMXCEED_REASS,  "frag reassembly time exceeded")
        default: fprintf(fOutput,
                         "time exceeded, code: %d",
                         pICMPInfo->usICMPCode);
          break;
      }
      break;

    case ICMP_PARAMPROB:
      fprintf(fOutput,
             "parameter problem: pointer = 0x%02x",
              pICMPInfo->ulGatewayAddress);
      break;

    ICMPCODE(ICMP_TSTAMP,     "timestamp")  /* XXX ID + Seq + 3 timestamps */
    ICMPCODE(ICMP_TSTAMPREPLY,"timestamp reply") /* XXX ID + Seq + 3 tstps */
    ICMPCODE(ICMP_IREQ,       "information request")       /* XXX ID + Seq */
    ICMPCODE(ICMP_IREQREPLY,  "information reply")         /* XXX ID + Seq */
    ICMPCODE(ICMP_MASKREQ,    "address mask request")
    ICMPCODE(ICMP_MASKREPLY,  "address mask reply")
    default: fprintf(fOutput,
                     "bad ICMP type: %d",
                     pICMPInfo->usICMPType);
  }
}


/*****************************************************************************
 * Name      : APIRET NetStatistics
 * Funktion  : print out the overall statistics
 * Parameter :
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET NetStatistics(void)
{
  ULONG      ulCounter;              /* iterator counter over the host array */
  ULONG      ulCounterAlias;         /* iterator counter over the host array */
  CHAR       pszIPAddress[18];       /* buffer for the IP address conversion */
  static PSZ pszUnknown = "<unknown>";              /* this MUST be static ! */
  PSCANHOST  pScanHost;                      /* pointer to the selected host */
  ULONG      ulHostRepliesMeasured = 0;            /* number of valid values */
  APIRET     rc;                                           /* API returncode */
  time_t     timeSystem;                              /* current system time */
  CHAR       pszTimeSystem[32];          /* local buffer for the system time */

  timeSystem = time(NULL);                           /* get the current time */

  strcpy(pszTimeSystem,                            /* convert time to string */
         ctime(&timeSystem));
  pszTimeSystem[24] = 0;                /* terminate that string before CRLF */

  printf ("\nHosts:   (%s)",
          pszTimeSystem);

  for (ulCounter = Globals.ulIPStart;
       ulCounter <= Globals.ulIPEnd;
       ulCounter++)
  {
    rc = HostQuery(ulCounter,                 /* query a pointer to the host */
                   &pScanHost);
    if (rc != NO_ERROR)   /* if we fail to do so, continue with the next one */
      continue;

    if (pScanHost->HACT.flActive == TRUE)
                                                 /* calculate response times */
      if (pScanHost->HACT.flResponseMeasured == TRUE)
      {
        Globals.fTimeReply += pScanHost->HACT.fResponse / 1000.0;
        ulHostRepliesMeasured++;                 /* count this host as valid */
      }

    StrIPToString (pScanHost->ulIP,                 /* convert to IP address */
                   pszIPAddress);

    if (pScanHost->ucKnown == FALSE)
      HostUpdate(ulCounter,
                 SH_HOSTNAME,
                 (ULONG)pszUnknown);

    if ( (!Options.fsListFilter)       ||             /* skip selected hosts */
         (pScanHost->ucKnown  == TRUE) ||
         (pScanHost->HACT.flActive == TRUE) )
    {
      printf ("\n%-16s %-40s",
              pszIPAddress,
              pScanHost->pszHostname);

      if (pScanHost->ucHostStatus & HOSTSTATUS_OK)
      {
        if (pScanHost->HACT.flActive == TRUE)
          printf ("* %10.3fms",
                  pScanHost->HACT.fResponse / 1000);
        else
          printf ("      inactive");
      }

      if (pScanHost->ucHostStatus & HOSTSTATUS_GATEWAY)
        printf (" gateway");

      if (pScanHost->ucHostStatus & HOSTSTATUS_BROADCAST)
        printf (" broadcast (%u)",
                  pScanHost->ucReplies);


      if (pScanHost->ucHostStatus & HOSTSTATUS_PROBLEM)
      {
        printf (" ICMP problem: ");
                                                 /* now the ICMP information */
        NetICMPDecode(stdout,
                      &pScanHost->uStatus.sProblem);
      }


      if (Options.fsIPAlias)                       /* print the IP aliases ? */
      {
        if (pScanHost->ucIPAddresses > 0)            /* if aliases available */
        {
          printf ("\n  (%2u):",
                 pScanHost->ucIPAddresses);

          for (ulCounterAlias = 0;                 /* list all known aliases */
               ulCounterAlias < pScanHost->ucIPAddresses;
               ulCounterAlias++)
          {
            StrIPToString(pScanHost->ulIPAlias[ulCounterAlias],
                          pszIPAddress);
            printf("%s ",
                   pszIPAddress);
          }
        }
      }

      if (Options.fsPorts)              /* scan the hosts for active ports ? */
        if (pScanHost->HACT.flActive == TRUE)       /* host must be active ! */
        {
                 /* nicht scannen, wenn ICMP Error flActive getriggert hat ! */
          if (pScanHost->ucHostStatus & HOSTSTATUS_OK)
            NetHostScanPorts(pScanHost->ulIP);         /* OK, scan this host */
          /* @@@PH Error Handling */
        }
    }
  }

  printf("\n\nStatistics:"
         "\n  Nodes scanned:  %8u %10.3f hosts/s %10.3fs"
         "\n  Nodes pinged:   %8u %10.3f hosts/s %10.3fs"
         "\n  Nodes known:    %8u"
         "\n  Nodes unknown:  %8u"
         "\n  Nodes active:   %8u"
         "\n  Nodes inactive: %8u",
         Globals.ulHostsScanned,
         Globals.ulHostsScanned / Globals.fTimeScan,
         Globals.fTimeScan,
         Globals.ulHostsPinged,
         Globals.ulHostsPinged / Globals.fTimeSend,
         Globals.fTimeSend,
         Globals.ulHostsKnown,
         Globals.ulHostsUnknown,
         Globals.ulHostsActive,
         Globals.ulHostsInactive);

  if (ulHostRepliesMeasured != 0)
    printf ("   %10.3f ms average response time",
            Globals.fTimeReply / ulHostRepliesMeasured,
            Globals.fTimeReply);

  if (Options.fsPorts)
  {
    printf("\nServices & Ports:"
           "\n  Connections total:  %8u %10.3f connections/s %10.3fs",
      Globals.ulConnectOK + Globals.ulConnectFailed,
      (Globals.ulConnectOK + Globals.ulConnectFailed) /
        (Globals.fTimeConnectOK + Globals.fTimeConnectFailed),
      Globals.fTimeConnectOK + Globals.fTimeConnectFailed);

    if (Globals.fTimeConnectOK == 0.0)           /* NT's timing weakness ... */
      Globals.fTimeConnectOK = 0.1;

    if ( (Globals.ulConnectOK != 0) &&                 /* proper connections */
         (Globals.fTimeConnectOK != 0.0) )
      printf("\n  Connections OK:     %8u %10.3f connections/s %10.3fs",
             Globals.ulConnectOK,
             Globals.ulConnectOK / Globals.fTimeConnectOK,
             Globals.fTimeConnectOK);

    if (Globals.fTimeConnectFailed == 0.0)       /* NT's timing weakness ... */
      Globals.fTimeConnectFailed = 0.1;

    if ( (Globals.ulConnectFailed != 0) &&             /* failed connections */
         (Globals.fTimeConnectFailed != 0.0) )
      printf("\n  Connections failed: %8u %10.3f connections/s %10.3fs",
             Globals.ulConnectFailed,
             Globals.ulConnectFailed / Globals.fTimeConnectFailed,
             Globals.fTimeConnectFailed);
  }

  return (NO_ERROR);
}


/*****************************************************************************
 * Name      : APIRET NetEtcHosts
 * Funktion  : Write a /etc/hosts file.
 * Parameter :
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung : @@@PH - clean code: still duplicates in HOSTS !
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET NetEtcHosts(void)
{
  ULONG ulCounter;                   /* iterator counter over the host array */
  ULONG ulCounterAlias;              /* iterator counter over the host array */
  CHAR  pszIPAddress[18];            /* buffer for the IP address conversion */
  PSCANHOST  pScanHost;                      /* pointer to the selected host */
  CHAR  szTimeDate[32];                          /* buffer for time and date */
  time_t timeSystem;                                       /* time structure */
  FILE   *fOutput;                   /* the output stream for the hosts file */
  APIRET rc;                                               /* API returncode */
  CHAR   pszShortHostname[80];              /* short version of the hostname */
  PSZ    pszPositionDot;              /* position of . in the short hostname */


  fOutput = fopen(Options.pszHostsFile,               /* open the hosts file */
                  "a+");
  if (fOutput == NULL)                          /* we couldn't open the file */
    return (ERROR_OPEN_FAILED);                     /* raise error condition */


  /********************
   * print the header *
   ********************/

  timeSystem = time(NULL);      /* get a string for current system date/time */
  strcpy (szTimeDate,
          ctime(&timeSystem));
  szTimeDate[24] = 0;                                        /* cut the CRLF */

  fprintf (fOutput,
           "###############################################################\n"
           "#                                                             #\n"
           "# IPScan                     (c) Patrick Haller Systemtechnik #\n"
           "#                                                             #\n"
           "# Creation              %-26s            #\n"
           "# Starting IP address   %-24s              #\n"
           "# Scanned subnet        %-24s              #\n"
           "#                                                             #\n"
           "###############################################################\n"
           "\n",
           szTimeDate,
           Options.pszIP,
           Options.pszNetMask);


  for (ulCounter = Globals.ulIPStart;
       ulCounter <= Globals.ulIPEnd;
       ulCounter++)
  {
    rc = HostQuery(ulCounter,                 /* query a pointer to the host */
                   &pScanHost);
    if (rc != NO_ERROR)   /* if we fail to do so, continue with the next one */
      continue;

    StrIPToString (pScanHost->ulIP,                 /* convert to IP address */
                   pszIPAddress);

    strncpy (pszShortHostname,                        /* first copy the name */
             pScanHost->pszHostname,
             sizeof(pszShortHostname));
    pszPositionDot = strchr(pszShortHostname,          /* find the first dot */
                            '.');
    if (pszPositionDot != NULL)                        /* if that dot exists */
      *pszPositionDot = 0;                     /* terminate the string there */


    if (pScanHost->ucKnown == TRUE)                /* write known hosts only */
    {
      fprintf(fOutput,
              "\n%-16s %-12s %-40s #",
              pszIPAddress,                                /* the IP address */
              pszShortHostname,                                /* host alias */
              pScanHost->pszHostname);           /* fully qualified hostname */

      if (pScanHost->ucHostStatus & HOSTSTATUS_GATEWAY)
        fprintf (fOutput,
                 " gateway");

      if (pScanHost->ucHostStatus & HOSTSTATUS_BROADCAST)
        fprintf (fOutput,
                 " broadcast");

      if (pScanHost->ucHostStatus & HOSTSTATUS_PROBLEM)
                                                 /* now the ICMP information */
        NetICMPDecode(fOutput,
                      &pScanHost->uStatus.sProblem);

                            /* put in aliases too, however check for dupes ! */
      if (pScanHost->ucIPAddresses > 0)              /* if aliases available */
      {
        for (ulCounterAlias = 0;                   /* list all known aliases */
             ulCounterAlias < pScanHost->ucIPAddresses;
             ulCounterAlias++)
        {
                                                                /* query adr */
          rc = HostUpdate(pScanHost->ulIPAlias[ulCounterAlias],
                          SH_QUERYEXISTS,
                          0);

          StrIPToString(pScanHost->ulIPAlias[ulCounterAlias],
                        pszIPAddress);

          if (rc == ERROR_IP_OUT_OF_RANGE)  /* this address is unknown->list */
            fprintf(fOutput,
                    "\n%-16s %-12s %-40s #",
                    pszIPAddress,                          /* the IP address */
                    pszShortHostname,                          /* host alias */
                    pScanHost->pszHostname);     /* fully qualified hostname */
          else
            fprintf(fOutput,                            /* add it as comment */
                    " %s",
                    pszIPAddress);
        }
      }
    }
  }


  fprintf (fOutput,
           "\n\n"
           "\n### END OF FILE ###"
           "\n");

  fclose(fOutput);                               /* OK, close the hosts file */

  return (NO_ERROR);
}


/*****************************************************************************
 * Name      : USHORT NetIPChecksum
 * Funktion  : Checksum routine for Internet Protocol family headers
 * Parameter :
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

USHORT NetIPChecksum (PUSHORT pusAddress,
                      ULONG   ulLength)
{
  int     nleft = ulLength;
  PUSHORT w     = pusAddress;
  USHORT  answer;
  int     sum   = 0;

  /*  Our algorithm is simple, using a 32 bit accumulator (sum),
   *  we add sequential 16 bit words to it, and at the end, fold
   *  back all the carry bits from the top 16 bits into the lower
   *  16 bits.
   */
  while (nleft > 1)

  {
    sum += *w++;
    nleft -= 2;
  }


  if (nleft == 1)                        /* mop up an odd byte, if necessary */
    sum += *(u_char *)w;

                      /* add back carry outs from top 16 bits to low 16 bits */
  sum = (sum >> 16) + (sum & 0xffff);                 /* add hi 16 to low 16 */
  sum += (sum >> 16);                                           /* add carry */
  answer = ~sum;                                      /* truncate to 16 bits */
  return (answer);
}


/*****************************************************************************
 * Name      : APIRET NetHostPingProbeSend
 * Funktion  : send a ICMP probe to target IP address
 * Parameter :
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET NetHostPingProbeSend(SOCKET sockSend,
                            PVOID  pPacketBuffer,
                            ULONG  ulIP,
                            ULONG  ulSequence)
{
  PPACKETPROBE pPacketProbe = (PPACKETPROBE)pPacketBuffer; /* IP ICMP packet */
  int          irc;                                   /* integer return code */
  struct sockaddr_in sinTarget;                     /* target socket address */
  ULONG        ulBytesSend;                       /* number of bytes to send */

  StrIPToString(ulIP,                                   /* convert to string */
                Globals.szLastSent);
  Globals.ulHostsPinged++;                              /* adjust statistics */

  DisplayProgress();                     /* show the current scanning status */

#if __OS2__
  pPacketProbe->IP.ip_hl  = sizeof(struct ip) >> 2;         /* header length */
  pPacketProbe->IP.ip_v   = IPVERSION;               /* IP version of packet */
  pPacketProbe->IP.ip_tos = 0;                            /* type of service */
  pPacketProbe->IP.ip_p   = IPPROTO_ICMP;                        /* protocol */
  pPacketProbe->IP.ip_off = 0;                            /* fragment offset */
  pPacketProbe->IP.ip_len = Options.ulDataLength;          /* length of data */
  pPacketProbe->IP.ip_ttl = Options.ulTTL;            /* time to live / hops */
  pPacketProbe->IP.ip_id  = ulIP;             /* unique identifier of packet */
#endif

  pPacketProbe->ICMP.icmp_type  = ICMP_ECHO;
  pPacketProbe->ICMP.icmp_code  = 0;
  pPacketProbe->ICMP.icmp_cksum = 0;
  pPacketProbe->ICMP.icmp_id    = Globals.pidIdentifier;
  pPacketProbe->ICMP.icmp_seq   = (USHORT)ulSequence;

  pPacketProbe->ulIPSentTo      = ulIP;       /* store the target IP address */
  pPacketProbe->ulIdentifier    = Globals.pidIdentifier;

  sinTarget.sin_addr.s_addr = htonl(ulIP);          /* setting up the socket */
  sinTarget.sin_family      = AF_INET;

  HostUpdate (ulIP,                         /* quickly update the host entry */
              SH_IPADDRESS,
              ulIP);

  pPacketProbe->ulSignature = IPPROBESIGNATURE;          /* unique signature */

  ToolsPerfQuery(&pPacketProbe->psSent);           /* start benchmarking now */

  pPacketProbe->ICMP.icmp_cksum = NetIPChecksum((PUSHORT)&pPacketProbe->ICMP,
                                                Options.ulDataLength -
                                                sizeof (struct ip) );

#ifdef __OS2__
  irc = select(&sockSend,         /* wait for the socket to become writeable */
               0,
               1,
               0,
               IPTIMEOUTMULT);
  if (irc < 0)                                          /* an error occurred */
    return (sockErrno);                             /* raise error condition */
  else
    if (irc == 0)                                      /* this means timeout */
      return (ERROR_SEM_TIMEOUT);                   /* raise error condition */

  ulBytesSend = Options.ulDataLength;     /* OS/2 performs this normally ... */
  irc = sendto(sockSend,                           /* send the raw IP packet */
               (char *)pPacketProbe,
               ulBytesSend,
               0,
               (struct sockaddr *)&sinTarget,
               sizeof(struct sockaddr) );
#endif

   /* Microsoft's Winsock 2.0 RAW Socket Support is totally screwed up. Odd. */
#ifdef _WIN32
  {
    fd_set fds;
    struct timeval tv;

    FD_ZERO(&fds);
    FD_SET(sockSend,&fds);

    tv.tv_sec  = IPTIMEOUTMULT / 1000;
    tv.tv_usec = IPTIMEOUTMULT % 1000;


    irc = select(sockSend+1,         /* wait for the socket to become writeable */
                 NULL,
                 &fds,
                 NULL,
                 &tv);
    if (irc < 0)                                          /* an error occurred */
      return (sockErrno);                             /* raise error condition */
    else
      if (irc == 0)                                      /* this means timeout */
        return (ERROR_SEM_TIMEOUT);                   /* raise error condition */
  }
                                          /* NT adds additional IP header... */
  ulBytesSend = Options.ulDataLength - sizeof (struct ip);

  irc = sendto(sockSend,                           /* send the raw IP packet */
               (char *)pPacketProbe + sizeof (struct ip),
               ulBytesSend,
               0,

               (struct sockaddr *)&sinTarget,
               sizeof(struct sockaddr) );
#endif


#if 0
  printf ("\nDEBUG: &pPacketProbe->ICMP= %08xh, pPacketProbe + 20 = %08xh",
          &pPacketProbe->ICMP,

          (char *)pPacketProbe + sizeof (struct ip) );

  printf("\nDEBUG: sendto: sockSend=%i, pPacketProbe=%08x, "
             "ulDataLength=%u, sinTarget=%s, irc=%i",
                 sockSend,
                 pPacketProbe,
                 Options.ulDataLength,
                 inet_ntoa(sinTarget.sin_addr),
                 irc);
  printf ("\nDEBUG: sendto: ulDataLength=%i, Sent=%i, struct ip=%i",
          Options.ulDataLength,
          Options.ulDataLength - sizeof (struct ip),
          sizeof(struct ip));
  ToolsDumpHex(0,
                   Options.ulDataLength,
                           pPacketBuffer);
  printf("\n");
#endif


  if ( (irc < 0) ||
       (irc != (int)ulBytesSend) )
    return (ERROR_MORE_DATA);
  else
    return (NO_ERROR);                                                 /* OK */
}


/*****************************************************************************
 * Name      : APIRET NetHostPingProbeWait
 * Funktion  : wait for ICMP probe from target IP address
 * Parameter :
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET NetHostPingProbeWait(SOCKET sockReceive,
                            PVOID  pPacketBuffer,
                            ULONG  ulIP)
{
  PPACKETPROBE   pPacketProbe = (PPACKETPROBE)pPacketBuffer;  /* IP ICMP pkt */
  int            irc;                                 /* integer return code */
  struct sockaddr_in sinTarget;                     /* target socket address */
  ULONG              sinTargetLength;   /* length of incoming target address */
  fd_set         fds;
  double         fDummy;         /* double dummy for the conversion to ULONG */
  ULONG          ulIPHost;                           /* IP address of a host */


  FD_ZERO(&fds);
  FD_SET (sockReceive,
          &fds);

  sinTarget.sin_addr.s_addr = htonl(ulIP);         /* or 0.0.0.0 for default */
  sinTargetLength           = sizeof(sinTarget);


select_loop:

#ifdef _WIN32
  {
    fd_set fds;
    struct timeval tv;

    FD_ZERO(&fds);
    FD_SET(sockReceive,&fds);

    tv.tv_sec  = IPTIMEOUTMULT / 1000;
    tv.tv_usec = IPTIMEOUTMULT % 1000;

    irc = select(sockReceive+1,     /* wait for the socket to become writeable */
                 &fds,
                 NULL,
                 NULL,
                 &tv);
  }
#endif

#ifdef __OS2__
  irc = select(&sockReceive,                     /* wait for incoming packet */
               1,
               0,
               0,
               IPTIMEOUTMULT);
#endif

  if (irc == 0)
    return (ERROR_SEM_TIMEOUT);              /* a zero length packet came in */
  else
    if (irc > 0)                 /* positive confirmation of incoming packet */
    {
      irc = recvfrom(sockReceive,                /* receive data from socket */
                     (char *)pPacketBuffer,
                     Options.ulDataLength,
                     0,
                     (struct sockaddr *)&sinTarget,
                     (int *)&sinTargetLength);

      switch (errno)                                       /* error handling */
      {
        case ETIMEDOUT:
          return (ERROR_SEM_TIMEOUT);

        case NO_ERROR:                                                 /* OK */
          break;

        default:                                 /* rest send back to caller */
          return (irc);
      }

      ToolsPerfQuery(&pPacketProbe->psReceived);    /* stop benchmarking now */
          /* problem: if we have an invalid packet and subsequent references */
          /*          to the performance data, this might cause floating     */
          /*          point exceptions as garbage is casted to DOUBLE.       */

                                 /* foreign packet FOR other program came in */
      if ((PID)pPacketProbe->ulIdentifier != Globals.pidIdentifier)
        goto select_loop;

      if ( (pPacketProbe->ICMP.icmp_type != ICMP_ECHOREPLY) ||
           (pPacketProbe->ICMP.icmp_code != 0) )
      {
        ULONG ulIPHost;                /* ip address of the problematic host */
                               /* not to confuse with the responding party ! */

         /* in this case we've got to record the ICMP information about this */
         /* host in order to provide hints about problems, routing, etc.     */

                             /* the responding party is a gateway for sure ! */
        HostUpdate(htonl(sinTarget.sin_addr.s_addr),
                         SH_STATUS_SET,
                         HOSTSTATUS_GATEWAY);

                                                  /* identify concerned host */
        ulIPHost = htonl(*(PULONG)((PSZ)pPacketProbe +
                                   0x2c) /* @@@PH: offset into the IP header */
                        );

                                   /* record that host as HOSTSTATUS_PROBLEM */
#if 0
        fprintf (stderr,
                 "\nDEBUG: ICMP Type : %u"
                 "\n       ICMP Code : %u"
                 "\n       addressee : %08xh"
                 "\n       responder : %08xh"
                 "\n       IP host   : %08xh"
                 "\n",
                 pPacketProbe->ICMP.icmp_type,
                 pPacketProbe->ICMP.icmp_code,
                 pPacketProbe->ulIPSentTo,
                 htonl(sinTarget.sin_addr.s_addr),
                 ulIPHost);
#endif

        HostUpdate(ulIPHost,
                   SH_STATUS_SET,
                   HOSTSTATUS_PROBLEM);

        HostUpdate(ulIPHost,                  /* record the ICMP information */
                   SH_STATUS_ICMP,
                   pPacketProbe->ICMP.icmp_type << 16 |
                   pPacketProbe->ICMP.icmp_code);

        HostUpdate(ulIPHost,                  /* record the ICMP information */
                   SH_STATUS_ICMP_GATEWAY,
                   htonl(pPacketProbe->ICMP.icmp_gwaddr.s_addr));

        goto select_loop;
      }


      if (pPacketProbe->ulSignature != IPPROBESIGNATURE)
      {
        fprintf (stderr,
                 "\nWarning: Probe packet with invalid signature %08xh from %s.\n",
                 pPacketProbe->ulSignature,
                 inet_ntoa(sinTarget.sin_addr));
        goto select_loop;
      }

      ulIPHost = htonl(sinTarget.sin_addr.s_addr);
                                                    /* update the host entry */
      if ( (pPacketProbe->psSent.fSeconds > 0) &&
           (pPacketProbe->psSent.fSeconds <
            pPacketProbe->psReceived.fSeconds) )
      {
        fDummy = (pPacketProbe->psReceived.fSeconds -        /* double dummy */
                  pPacketProbe->psSent.fSeconds) * 1000000.0;

        HostUpdate (ulIPHost, SH_RESPONSE, (ULONG)fDummy);
      }

      HostUpdate(ulIPHost, SH_ACTIVE,     TRUE);
      HostUpdate(ulIPHost, SH_STATUS_SET, HOSTSTATUS_OK);

                 /* did we get a response from the node we actually pinged ? */
                              /* check whether addressee and responder match */
      if (ulIPHost != pPacketProbe->ulIPSentTo)      /* this alone not suff. */
      {
        HostUpdate(pPacketProbe->ulIPSentTo, SH_REPLY_INC,  0);
        HostUpdate(pPacketProbe->ulIPSentTo, SH_STATUS_SET, HOSTSTATUS_BROADCAST);
      }


      strcpy (Globals.szLastRecv,                   /* duplicate this string */
              inet_ntoa(sinTarget.sin_addr));

      DisplayProgress();                 /* show the current scanning status */

      Globals.ulHostsActive++;

      return (NO_ERROR);                                               /* OK */
    }
    else
      return (sockErrno);                           /* return timeout signal */
}


/*****************************************************************************
 * Name      : void ThdFirePackets (PVOID pFirePacketsParameters)
 * Funktion  : This thread fires the packets for all the probes async.
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

void ThdFirePackets (PVOID pDummy)
{
  ULONG    ulIP;                                       /* ip address counter */
  PVOID    pPacket;                           /* buffer for the probe packet */
  APIRET   rc;                                             /* API returncode */
  PTHDFIRE pThdFire = (PTHDFIRE)pDummy;           /* map the parameter block */

  pPacket = malloc(Options.ulDataLength);        /* allocate memory for that */
  if (pPacket == NULL)                                   /* check for errors */
  {
    pThdFire->rc = ERROR_NOT_ENOUGH_MEMORY;         /* raise error condition */
    pThdFire->fActive = FALSE;                          /* set the stop flag */
    return;                                                /* end the thread */
  }

  ToolsPerfQuery (&pThdFire->psStart);             /* exact time measurement */

  for (ulIP  = Globals.ulIPStart;

       ulIP <= Globals.ulIPEnd;
       ulIP++)
  {
    /* do it ! */
    rc = NetHostPingProbeSend(Globals.sockSend,            /* send the probe */
                              pPacket,
                              ulIP,
                              ~ulIP);      /* use ~IP as sequence identifier */
    if (rc != NO_ERROR)                                  /* check for errors */
      ToolsErrorDos(rc);

    if (!Options.fsDontResolve)          /* check out this node for hostinfo */
      NetHostInfo (ulIP);

    if (Options.fsWait)             /* if we have to wait between the probes */
      DosSleep(Options.ulWait);                      /* sleep n milliseconds */
  }

  ToolsPerfQuery (&pThdFire->psEnd);               /* exact time measurement */

  Globals.fTimeSend = pThdFire->psEnd.fSeconds
                      - pThdFire->psStart.fSeconds;

  free (pPacket);               /* free memory used for the packet buffering */

  pThdFire->rc = NO_ERROR;                  /* signal successful termination */
  pThdFire->fActive = FALSE;                            /* set the stop flag */
}


/*****************************************************************************
 * Name      : APIRET NetScanLoop
 * Funktion  : This is the scanning loop
 * Parameter :
 * Variablen :
 * Ergebnis  : API return code
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET NetScanLoop (void)
{
  APIRET rc = NO_ERROR;                                    /* API returncode */
  ULONG  ulIPMask;                                    /* this is the IP mask */
  CHAR   szIPStart[18];                 /* buffers for IP address conversion */
  CHAR   szIPEnd  [18];                 /* buffers for IP address conversion */
  CHAR   szIPMask [18];                 /* buffers for IP address conversion */
  PVOID  pPacketRecv;                         /* buffer for the probe packet */
  PVOID    pPacket;                           /* buffer for the probe packet */
  PERFSTRUCT psStart;                     /* to measure scanning performance */
  PERFSTRUCT psEnd;
  TID        tidPing;                        /* thread ID of the ping thread */
  THDFIRE    ThdFire;                /* parameters for the FirePacket thread */
  int        iAddress;       /* temporary integer variable for tcpip address */


  /***************************************************************************
   * prepare the memory objects                                              *
   ***************************************************************************/

  pPacketRecv = malloc(Options.ulDataLength);    /* allocate memory for that */
  if (pPacketRecv == NULL)                               /* check for errors */
    return(ERROR_NOT_ENOUGH_MEMORY);                /* raise error condition */

  pPacket = malloc(Options.ulDataLength);        /* allocate memory for that */
  if (pPacket == NULL)                                   /* check for errors */
  {
    free (pPacketRecv);                  /* free previously allocated memory */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */
  }


  /***************************************************************************
   * allocate the array for the host information                             *
   ***************************************************************************/

  /**************************************************************
   * resolve stringIPAddress / hostname to numerical IP address *
   **************************************************************/
  iAddress = inet_addr(Options.pszIP);               /* try IP address first */
  if (iAddress != -1)
    Globals.ulIPStart = htonl(iAddress);            /* OK, that's it already */
  else
  {
    struct hostent *pheHost;                            /* hostentry pointer */
    struct sockaddr_in sinIPStart;             /* Internet address structure */

    pheHost = gethostbyname(Options.pszIP);              /* get the hostname */
    if (pheHost != NULL)                      /* if we retrieved information */
    {
      sinIPStart.sin_family = pheHost->h_addrtype;

      if (pheHost->h_length > sizeof(sinIPStart.sin_addr))
        pheHost->h_length = sizeof(sinIPStart.sin_addr);

      memcpy(&sinIPStart.sin_addr,
             pheHost->h_addr,
             pheHost->h_length);

      Globals.ulIPStart = htonl(sinIPStart.sin_addr.s_addr);     /* map it ! */

      if (!Options.fsNetMask)             /* if no netmask supplied by user, */
      {
        ulIPMask = 0xFFFFFFFF;                 /* then assume this host only */
        Options.fsNetMask = TRUE;
      }
    }
    else
    {
      fprintf (stderr,                                /* yield error message */
               "\nError: starting IP hostname is unknown.");

      free(pPacket);                     /* free previously allocated memory */
      free(pPacketRecv);

      return (ERROR_IP_UNKNOWN_HOST);               /* raise error confition */
    }
  }


  if (Options.fsNetMask)              /* check if user specified the netmask */
    ulIPMask  = htonl(inet_addr(Options.pszNetMask));
  else
    ulIPMask  = 0xFFFFFF00;                      /* this is our default mask */

                                                /* build up the last address */
  Globals.ulIPEnd   = ~ulIPMask | Globals.ulIPStart;

  StrIPToString (Globals.ulIPStart, szIPStart);
  StrIPToString (ulIPMask,          szIPMask);
  StrIPToString (Globals.ulIPEnd,   szIPEnd);

  printf ("\nScanning from %s to %s, mask %s, %u nodes.\n",
          szIPStart,
          szIPEnd,
          szIPMask,
          Globals.ulIPEnd - Globals.ulIPStart + 1);

                                 /* allocate the buffer for the node entries */
  Globals.arrScanHost = malloc ( sizeof(SCANHOST) * (Globals.ulIPEnd -
                                                     Globals.ulIPStart + 1) );
  if (Globals.arrScanHost == NULL)            /* check for proper allocation */
  {
    free (pPacket);                      /* free previously allocated memory */
    free (pPacketRecv);                  /* free previously allocated memory */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */
  }
  else
    memset (Globals.arrScanHost,                       /* zero out the array */
            0,
            sizeof(SCANHOST) * (Globals.ulIPEnd -
                                Globals.ulIPStart + 1) );


  /***************************************************************************
   * build the array of known services as getservbyport() is rather slow     *
   ***************************************************************************/

  if (Options.fsPorts)
  {
    struct servent *pseService; /* pointer to the TCP/IP stack service entry */
    ULONG  ulService;                                        /* loop counter */

    Globals.ulServiceStart = Options.ulServiceStart;
    Globals.ulServiceEnd   = Options.ulServiceEnd;

                                 /* allocate the buffer for the node entries */
    Globals.arrScanService = malloc ( sizeof(PSZ *) *
                                      (Globals.ulServiceEnd -
                                       Globals.ulServiceStart + 1) );
    if (Globals.arrScanService == NULL)       /* check for proper allocation */
    {
      free (Globals.arrScanHost);        /* free previously allocated memory */
      free (pPacket);                    /* free previously allocated memory */
      free (pPacketRecv);                /* free previously allocated memory */
      return (ERROR_NOT_ENOUGH_MEMORY);             /* raise error condition */
    }
    else
      memset (Globals.arrScanService,                  /* zero out the array */
              0,
              sizeof(PSZ *) *
               (Globals.ulServiceEnd -
                Globals.ulServiceStart + 1) );

                                   /* query the stack for all known services */
    for (ulService = Globals.ulServiceStart;
         ulService <= Globals.ulServiceEnd;
         ulService++)
    {
      if (!Options.fsDontScanServices)        /* if we have to scan services */
      {
        pseService = getservbyport(htonl(ulService),
                                   NULL);
        if (pseService != NULL)                   /* check if service is known */
        {
          Globals.arrScanService[ulService - Globals.ulServiceStart] =
            strdup(pseService->s_name);

          if (!Options.fsQuiet)
            fprintf (stderr,
                     "\r  %5u %-21s",
                     ulService,
                    pseService->s_name);
        }
        else
          if (!Options.fsQuiet)
            fprintf (stderr,
                     "\r  %5u",
                     ulService);
      }
      else                             /* no scanning, put in port name only */
        Globals.arrScanService[ulService - Globals.ulServiceStart] = NULL;
    }
  }


  /***************************************************************************
   * start the whole mambo jambo                                             *
   ***************************************************************************/

  ToolsPerfQuery (&psStart);                       /* exact time measurement */


  ThdFire.fActive   = TRUE;        /* 1.) use 2nd thread to fire all packets */

#ifdef __BORLANDC__
  tidPing = _beginthread (ThdFirePackets,
                          16384,
                          (PVOID)&ThdFire);
#endif

#ifdef __IBMC__
  tidPing = _beginthread (ThdFirePackets,
                          NULL,
                          16384,
                          (PVOID)&ThdFire);
#endif
  if (tidPing == -1)                        /* check for proper thread start */
  {
    free (pPacket);                      /* free previously allocated memory */
    free (pPacketRecv);                  /* free previously allocated memory */
    return (ERROR_IP_THREAD_FAILED);                /* raise error condition */
  }

                                       /* 2.) collect all response datagrams */
                                /* and wait for the fire thread to terminate */
  while (Globals.ulTimeoutCounter < Options.ulTimeout)
  {
    rc = NetHostPingProbeWait(Globals.sockReceive,         /* wait for probe */
                              pPacketRecv,
                              0);
    if ( (rc == ERROR_SEM_TIMEOUT) &&           /* waiting for stray packets */
        (ThdFire.fActive == FALSE) )             /* and firing has finished */
    {
      Globals.ulTimeoutCounter++;
      DisplayProgress ();
    }
  }

                                         /* collect the data from the thread */
  if (ThdFire.rc != NO_ERROR)                            /* check for errors */
    ToolsErrorDos(ThdFire.rc);                        /* print error message */

  ToolsPerfQuery (&psEnd);                         /* exact time measurement */

  Globals.fTimeScan = psEnd.fSeconds - psStart.fSeconds;


  /***************************************************************************
   * OK, that's (almost) it.                                                 *
   ***************************************************************************/

  rc = NetStatistics();                  /* print out the overall statistics */

  if (Options.fsHostsFile)              /* we've got to write a hosts file ? */
    rc = NetEtcHosts();                                        /* then do it */

  if (Globals.arrScanService != NULL)                  /* check if allocated */
    free (Globals.arrScanService);                  /* free allocated memory */

  free (Globals.arrScanHost);                       /* free allocated memory */
  free (pPacketRecv);                               /* free allocated memory */
  free (pPacket);                        /* free previously allocated memory */
  return (rc);                                       /* default return value */
}


/*****************************************************************************
 * Name      : void initialize
 * Funktion  : Initialisierung einiger Variablen
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET initialize (void)
{
  struct protoent *peProtocol;                             /* protocol entry */
  ULONG           ulTimeout;                /* timeout value for the sockets */
  APIRET          rc;                                      /* API returncode */

  memset(&Options,
         0L,
         sizeof(Options));

  memset(&Globals,
         0L,
         sizeof(Globals));

  Globals.fTimeSend = 0.1;                       /* prevent division by zero */

  /* @@@PH DUMMY */
  Options.ulServiceStart = 1;
  Options.ulServiceEnd   = 1024;

  Globals.fTimeReply         = 0.0;
  Globals.fTimeScan          = 0.0;
  Globals.fTimeScan          = 0.0;
  Globals.fTimeConnectOK     = 0.0;
  Globals.fTimeConnectFailed = 0.0;


#ifdef __OS2__
  sock_init();                                    /* initialize TCP/IP stack */

  setvbuf(stdout,
          NULL,
          _IONBF,
          0);

  setvbuf(stderr,
          NULL,
          _IONBF,
          0);

  sethostent(1);        /* open the tcp/ip database files and keep them open */
  setservent(1);
  setprotoent(1);
  setnetent(1);

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

    if (err != 0)           /* Tell the user that we couldn't find a useable */
                                                         /* winsock.dll.     */
      return(ERROR_IP_UNUSABLE_STACK);

                        /* Confirm that the Windows Sockets DLL supports 1.1.*/
                        /* Note that if the DLL supports versions greater    */
                        /* than 1.1 in addition to 1.1, it will still return */
                        /* 1.1 in wVersion since that is the version we      */
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


  Globals.pidIdentifier = getpid();                           /* get our PID */

  peProtocol = getprotobyname("icmp");                 /* query the protocol */
  if (peProtocol == NULL)                                /* check for errors */
  {
    fprintf (stderr,                                /* display error message */
             "\nError: protocol ICMP failed.");
    return (ERROR_INVALID_FUNCTION);                /* raise error condition */
  }


  /***************************************************************************
   * Opening the Ping socket                                                 *
   ***************************************************************************/
#ifdef __OS2__
  Globals.sockSend = socket(AF_INET,                      /* create a socket */
                            SOCK_RAW,
                            IPPROTO_RAW);
  if ( Globals.sockSend < 0)
    return (sockErrno);                             /* raise error condition */
#endif

#ifdef _WIN32
  /* NT (WinSock 1.1) does not support IPPROTO_RAW ! Support in Winsock 2.x  */
  /* always adds an additional IP header which screws up the key part of the */
                                               /* application. Thanks, Bill. */
  Globals.sockSend = WSASocket (AF_INET,
                                SOCK_RAW,
                                IPPROTO_ICMP,
                                NULL,
                                0,
                                WSA_FLAG_OVERLAPPED);
  if ( Globals.sockSend == INVALID_SOCKET)
    return (sockErrno);                             /* raise error condition */

  ulTimeout = 1000;                                   /* timeout is 1 second */
  rc = setsockopt(Globals.sockSend,                       /* set the timeout */
                      SOL_SOCKET,
                                  SO_SNDTIMEO,
                                  (PSZ)&ulTimeout,
                                  sizeof(ulTimeout));
  if (rc == SOCKET_ERROR)                                /* check for errors */
    return (ERROR_IP_SOCKET_ERROR);                 /* raise error condition */
#endif

  /***************************************************************************
   * Opening the receiving socket                                            *
   ***************************************************************************/
#ifdef __OS2__
  Globals.sockReceive = socket(AF_INET,                   /* create a socket */
                               SOCK_RAW,
                               peProtocol->p_proto);
  if ( Globals.sockReceive < 0)
    return (sockErrno);                             /* raise error condition */
#endif

#ifdef _WIN32
  Globals.sockReceive = WSASocket (AF_INET,
                                SOCK_RAW,
                                peProtocol->p_proto,
                                NULL,
                                0,
                                WSA_FLAG_OVERLAPPED);
  if ( Globals.sockReceive == INVALID_SOCKET)
    return (sockErrno);                             /* raise error condition */

  ulTimeout = 1000;                                   /* timeout is 1 second */
  rc = setsockopt(Globals.sockReceive,                    /* set the timeout */
                  SOL_SOCKET,
                  SO_RCVTIMEO,
                  (PSZ)&ulTimeout,
                  sizeof(ulTimeout));
  if (rc == SOCKET_ERROR)                                /* check for errors */
        return (ERROR_IP_SOCKET_ERROR);             /* raise error condition */

  /* unbound Winsocket socket will fail with 10022 in every case */
  {
    struct sockaddr sa;

        memset (&sa,
                    0,
            sizeof(struct sockaddr));
        sa.sa_family = AF_INET;


    rc = bind(Globals.sockReceive,
                  &sa,
                          sizeof (struct sockaddr) );
  }
#endif


  return (NO_ERROR);                                                   /* OK */
}


/*****************************************************************************
 * Name      : int main
 * Funktion  : Hauptroutine
 * Parameter : int argc, char *argv[]
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

int main (int argc, char *argv[])
{
  int rc;                                                    /* RÅckgabewert */

  rc = initialize ();                                     /* Initialisierung */
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    exit(1);                                                /* abort program */
  }

  rc = ArgStandard (argc,                            /* CLI-Parameter parsen */
                    argv,
                    TabArguments,
                    &Options.fsHelp);
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    soclose(Globals.sockSend);
    soclose(Globals.sockReceive);
    exit(1);                                                /* abort program */
  }

  if ( Options.fsHelp )                                /* user requests help */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

                                           /* perform some parameter mapping */
  if (!Options.fsDataLength)                             /* xx byte per def. */
    Options.ulDataLength = sizeof (PACKETPROBE);
  else
    if (Options.ulDataLength < sizeof(PACKETPROBE) )  /* assure mininum size */
      Options.ulDataLength = sizeof (PACKETPROBE);

  if (!Options.ulTimeout)                    /* no timeout value specified ? */
    Options.ulTimeout = 10;                              /* 1 second timeout */

#if 0
  if (!Options.fsWait)                       /* no wait    value specified ? */
  {
    Options.fsWait = TRUE;                        /* enable it automatically */
    Options.ulWait = 1;                                  /* at least free ts */
  }
#endif

  if ( (Options.fsPortsKnown) || /* any of those tokens triggers the fsPorts */
       (Options.fsPortStart) ||
       (Options.fsPortEnd) )
    Options.fsPorts = TRUE;

#ifdef _WIN32
  setsockopt(Globals.sockReceive,              /* set timeout for the socket */
             SOL_SOCKET,
             SO_RCVTIMEO,
             (char *)&Options.ulTimeout,
             sizeof(Options.ulTimeout));

  setsockopt(Globals.sockSend,                 /* set timeout for the socket */
             SOL_SOCKET,
             SO_SNDTIMEO,
             (char *)&Options.ulTimeout,
             sizeof(Options.ulTimeout));
#endif

  rc = 48 * 1024;                               /* dummy, 48k receive buffer */
  setsockopt(Globals.sockReceive,
             SOL_SOCKET,
             SO_RCVBUF,
             (char *)&rc,
             sizeof(rc));


  if (!Options.fsTTL)                       /* if user forgot to specify TTL */
    Options.ulTTL = 30;                          /* use the 30 standard hops */

  rc = NetScanLoop();                            /* this is our main routine */
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  endhostent();                 /* close the tcp/ip database files kept open */
  endservent();
  endprotoent();
  endnetent();

  soclose(Globals.sockSend);
  soclose(Globals.sockReceive);

  return (rc);
}
