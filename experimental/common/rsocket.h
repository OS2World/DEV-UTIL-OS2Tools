/*****************************************************************************
 * Name      : Reflect TCP/IP 
 * Purpose   : TCP/IP reflector for configurable services. Reflects
 *             requests on one machine to another. 
 * Parameter : 
 * Variables :
 * Result    : 
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

#ifndef MODULE_RSOCKET
#define MODULE_RSOCKET

#ifdef __cplusplus
extern "C" {
#endif
  

/*****************************************************************************
 * Includes                                                                  *         
 *****************************************************************************/

#ifdef _WIN32
                         /* @@@PH Yes, Microsoft is most famous and popular  */
                         /* for using easy-to-understand macros ...          */
  #define WIN32_LEAN_AND_MEAN
  #include <process.h>
  #include <winsock2.h>
  #include <windows.h>  
  #include "win32/ipscanw32.h"
  typedef void (__cdecl *THREAD)(PVOID pParameters);
  #define MAXHOSTNAMELEN 64
  #define errno WSAGetLastError()
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

#ifdef __OS2__
  #define INCL_DOSFILEMGR
  #define INCL_BASE
  #define INCL_NOPMAPI
  #include <os2.h>

  #include <netinet/in.h>
  #include <types.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #define BSD_SELECT
  #include <sys/select.h>
  #include <net/if_arp.h>
  #include <netdb.h>
  #include <netinet/in_systm.h>
  #include <netinet/ip.h>
  #include <netinet/ip_icmp.h>
  #include <netinet/udp.h>
  #include <process.h>
  #define sockErrno sock_errno()
  #define SOCKET_ERROR -1

  #define INVALID_SOCKET -1
#endif


#ifdef __cplusplus
}
#endif



/*****************************************************************************
 * Definitions                                                               *
 *****************************************************************************/


/*****************************************************************************
 * Structures                                                                *         
 *****************************************************************************/

typedef struct _SocketPerformance
{
  ULONG   ulTotalTransactions;              /* counter of total transactions */
  ULONG   ulTotalBytes;                            /* counter of total bytes */
  ULONG   ulTotalErrors;                          /* counter of total errors */
  ULONG   ulPerfTime;            /* timestamp of last perfcounter collection */
  ULONG   ulPerfTransactions;     /* transactions since last perf collection */
  ULONG   ulPerfBytes;                   /* bytes since last perf collection */
  ULONG   ulPerfErrors;                 /* errors since last perf collection */
} SOCKETPERFORMANCE, *PSOCKETPERFORMANCE;

#define RSOCKET_PERF_TOTAL_TRANSACTIONS              1
#define RSOCKET_PERF_TOTAL_BYTES                     2
#define RSOCKET_PERF_TOTAL_ERRORS                    3
#define RSOCKET_PERF_TIME                            4
#define RSOCKET_PERF_TRANSACTIONS                    5
#define RSOCKET_PERF_BYTES                           6
#define RSOCKET_PERF_ERRORS                          7	  


#ifdef __OS2__
typedef int SOCKET;
#endif


class RSOCKET
{
  protected:
    ULONG   ulPort;                                   /* the port to be used */
    ULONG   ulType;
    ULONG   ulProtocol;
    SOCKET  sockSocket;                                 /* our socket handle */
    SOCKETPERFORMANCE sockPerf;               /* socket performance counters */

    BOOL    bConnected;           /* FALSE - not connected, TRUE - connected */
    BOOL    bSocket;              /* FALSE - no socket, TRUE - socket avail. */

  public: 
    RSOCKET(void);                                      /* basic constructor */
    RSOCKET(ULONG ulNewPort,                                  /* constructor */
      	    ULONG ulNewType,
            ULONG ulNewProtocol);
    RSOCKET(SOCKET sockClone);                          /* clone constructor */
    ~RSOCKET(void);                                            /* destructor */

    APIRET SocketSet(SOCKET sockClone);  /* attach to existing socket handle */
    SOCKET SocketQuery(void);                /* query existing socket handle */

    ULONG  collectPerfCounter (ULONG ulIndex); /* query performance counters */

    APIRET init(void);                              /* initialize the socket */

    APIRET connect(ULONG ulIPAddress);                            /* connect */

    APIRET send   (PVOID  pData,                                     /* send */
                   ULONG  ulDataLength,
		   ULONG  ulFlags,
		   PULONG pulBytesSent);

    APIRET recv   (PVOID  pData,                                     /* recv */
                   ULONG  ulDataLength,
		   ULONG  ulFlags,
		   PULONG pulBytesRecv);

    APIRET bind   (ULONG ulIPAddress = 0);            /* bind unbound socket */

    APIRET listen (int iQueueLength=5);                  /* listen to a port */

    APIRET accept (SOCKET      *psockSocket,                       /* accept */
                   sockaddr_in *psinAddress = NULL,
		   PULONG      pulAddressLength = NULL);

    APIRET accept (RSOCKET& rsockConnection);                 /* alternative */

    APIRET close  (void);                                /* close the socket */

    APIRET SocketError (int iErrorCode);                  /* map error codes */

};

typedef RSOCKET * PRSOCKET;


#endif /* MODULE_RSOCKET */
