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

/*****************************************************************************
 * Includes                                                                  *         
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#ifdef _WIN32
  #include <windows.h>
  #include <winsock.h>
#endif

#ifdef __OS2__
  #define INCL_DOSERRORS
  #define INCL_DOS
  #include <os2.h>
#endif

#include "rserror.h"
#include "rsocket.h"


/*****************************************************************************
 * Definitions                                                               *         
 *****************************************************************************/

#define DEBUG printf
//#define DEBUG //


/*****************************************************************************
 * Structures                                                                *         
 *****************************************************************************/

/*****************************************************************************
 * Name      : RSOCKET::RSOCKET
 * Purpose   : Socket constructor
 * Parameter : 
 * Variables :
 * Result    : 
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

RSOCKET::RSOCKET(void)
{  
  DEBUG("\nDEBUG: RSOCKET::RSOCKET(void)");	

  bConnected = FALSE;                       /* new socket is never connected */  
  bSocket    = FALSE;                       /* new socket has no open socket */  

  memset ((PVOID)&sockPerf,               /* zero out the sockPerf structure */
	      0L,
		  sizeof (sockPerf) );
}

/*****************************************************************************
 * Name      : RSOCKET::RSOCKET
 * Purpose   : Socket constructor
 * Parameter : 
 * Variables :
 * Result    : 
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

RSOCKET::RSOCKET(ULONG ulNewPort,     
		         ULONG ulNewType,
			     ULONG ulNewProtocol)  				 
{
  DEBUG("\nDEBUG: RSOCKET::RSOCKET(Port=%i,Type=%i,Protocol=%i)",ulNewPort,ulNewType,ulNewProtocol);	

  ulPort     = ulNewPort;                 /* copy data to internal variables */
  ulType     = ulNewType;
  ulProtocol = ulNewProtocol;

  bConnected = FALSE;                       /* new socket is never connected */  
  bSocket    = FALSE;                       /* new socket has no open socket */  

  memset ((PVOID)&sockPerf,               /* zero out the sockPerf structure */
	      0L,
		  sizeof (sockPerf) );
}


/*****************************************************************************
 * Name      : RSOCKET::RSOCKET
 * Purpose   : Socket constructor
 * Parameter : 
 * Variables :
 * Result    : 
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

RSOCKET::RSOCKET(SOCKET sockClone)
{  
  DEBUG("\nDEBUG: RSOCKET::RSOCKET(SOCKET=%u)",sockClone);	

  memset ((PVOID)&sockPerf,               /* zero out the sockPerf structure */
	      0L,
		  sizeof (sockPerf) );

  SocketSet(sockClone);                              /* call member function */
}


/*****************************************************************************
 * Name      : RSOCKET::SocketSet
 * Purpose   : Connect RSOCKET to an existing socket handle
 * Parameter : SOCKET sockClone
 * Variables :
 * Result    : API return code 
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

APIRET RSOCKET::SocketSet(SOCKET sockClone)
{  
  DEBUG("\nDEBUG: RSOCKET::SocketSet(SOCKET=%u)",sockClone);	

  if (bSocket == TRUE)             /* check whether we are already connected */
    return (ERROR_SOCKET_EALREADY);                 /* raise error condition */

  ulPort     = 0;                         /* copy data to internal variables */
  ulType     = 0;              /* @@@PH Dummies ! query the stack for info ! */
  ulProtocol = 0;

  bConnected = TRUE;                        /* new socket is never connected */  
  bSocket    = TRUE;                        /* new socket has no open socket */  
  sockSocket = sockClone;                                 /* copy the socket */

  return (NO_ERROR);                                                   /* OK */
}


/*****************************************************************************
 * Name      : RSOCKET::SocketQuery
 * Purpose   : Query existing socket handle
 * Parameter : SOCKET sockClone
 * Variables :
 * Result    : API return code 
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

SOCKET RSOCKET::SocketQuery(void)
{  
  return (sockSocket);                               /* return socket handle */
}


/*****************************************************************************
 * Name      : RSOCKET::~RSOCKET
 * Purpose   : Socket destructor
 * Parameter : 
 * Variables :
 * Result    : 
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

RSOCKET::~RSOCKET(void)
{
  DEBUG("\nDEBUG: RSOCKET::~RSOCKET(connected=%s,socket=%s)",bConnected ? "yes" : "no",bSocket ? "yes" : "no");	

  if (bConnected) ; /* disconnect */
  
  if (bSocket == TRUE)                              /* close the open socket */
    soclose(sockSocket);
}



/*****************************************************************************
 * Name      : APIRET RSOCKET::init
 * Purpose   : Socket initialization
 * Parameter : 
 * Variables :
 * Result    : API Returncode
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

APIRET RSOCKET::init(void)
{  
  DEBUG("\nDEBUG: RSOCKET::init"); 

  if (bSocket == TRUE)             /* check whether socket is already opened */
	return (NO_ERROR);                                  /* abort immediately */

  sockSocket = socket(PF_INET,
	                  ulType,
					  ulProtocol);

  if (sockSocket == INVALID_SOCKET)       /* check for proper initialization */
  {
    sockPerf.ulTotalErrors++;                                  /* statistics */
	sockPerf.ulPerfErrors++;

    return (ERROR_SOCKET_CREATION);                 /* raise error condition */  
  }
  else
  {
    bSocket = TRUE;                          /* yeah, now we have a socket ! */
	return (NO_ERROR);                                                 /* OK */
  }
}


/*****************************************************************************
 * Name      : RSOCKET::~RSOCKET
 * Purpose   : Socket destructor
 * Parameter : sockaddr
 * Variables :
 * Result    : 
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

APIRET RSOCKET::connect(ULONG ulIPAddress)
{ 
  DEBUG("\nDEBUG: RSOCKET::connect(%08x)",ulIPAddress);	

  sockaddr_in sinAddress;

  sinAddress.sin_family           = AF_INET;
  sinAddress.sin_port             = htons((unsigned short)ulPort);
#ifdef _WIN32
  sinAddress.sin_addr.S_un.S_addr = htonl(ulIPAddress);
#endif

#ifdef __OS2__
  sinAddress.sin_addr.s_addr = htonl(ulIPAddress);
#endif

  if (bConnected == TRUE)          /* check whether we are already connected */
	return (ERROR_SOCKET_ALREADY_CONNECTED);       /* signal error condition */

  if (::connect (sockSocket,
	             (struct sockaddr *)&sinAddress,
		         sizeof(sinAddress)) == SOCKET_ERROR)
  {   
    sockPerf.ulTotalErrors++;                                  /* statistics */
	sockPerf.ulPerfErrors++;

	return (SocketError(sockErrno));      /* return proper API error */
  }
  else
  {
	bConnected = TRUE;                          /* yes, now we are connected */
	return (NO_ERROR);                                                 /* OK */
  }		   
}


/*****************************************************************************
 * Name      : RSOCKET::~RSOCKET
 * Purpose   : Socket destructor
 * Parameter : int iErrorCode
 * Variables :
 * Result    : 
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

APIRET RSOCKET::SocketError (int iErrorCode)
{
  APIRET rc;                                              /* API return code */
  PSZ    pszMessage;                               /* string message pointer */

  static ERRORTABLE errortableSocket[] = 
  { /* Windows Sockets code	Berkeley equivalent	Error	Interpretation */
	  {SOCEINTR,           ERROR_SOCKET_EINTR,           "Socket: Interrupted"},
	  {SOCEBADF,           ERROR_SOCKET_EBADF,           "Socket: "},
	  {SOCEACCES,          ERROR_SOCKET_EACCES,          "Socket: Access denied"},
#ifdef _WIN32
	  {SOCEDISCON,         ERROR_SOCKET_EDISCON,         "Socket: The message terminated "
	                                                     "gracefully. Used only for "
                                                             "message-oriented protocols."},
#endif
	  {SOCEFAULT,          ERROR_SOCKET_EFAULT,          "Socket: fault"},
	  {SOCEINVAL,          ERROR_SOCKET_EINVAL,          "Socket: invalid value"},
	  {SOCEMFILE,          ERROR_SOCKET_EMFILE,          "Socket: "},
	  {SOCEWOULDBLOCK,     ERROR_SOCKET_EWOULDBLOCK,     "Socket: call would block"},
	  {SOCEINPROGRESS,     ERROR_SOCKET_EINPROGRESS,     "Socket: This error is returned if "
	                                                     "any Windows Sockets function is "
                                                             "called while a blocking function "
                                                             "is in progress."},
	  {SOCEALREADY,        ERROR_SOCKET_EALREADY,        "Socket: already existing"},
	  {SOCENOTSOCK,        ERROR_SOCKET_ENOTSOCK,        "Socket: not a socket"},
	  {SOCEDESTADDRREQ,    ERROR_SOCKET_EDESTADDRREQ,    "Socket: destination address request"},
	  {SOCEMSGSIZE,        ERROR_SOCKET_EMSGSIZE,        "Socket: message size invalid"},
	  {SOCEPROTOTYPE,      ERROR_SOCKET_EPROTOTYPE,      "Socket: prototype"},
	  {SOCENOPROTOOPT,     ERROR_SOCKET_ENOPROTOOPT,     "Socket: no protocol option"},
	  {SOCEPROTONOSUPPORT, ERROR_SOCKET_EPROTONOSUPPORT, "Socket: protocol not supported"},
	  {SOCESOCKTNOSUPPORT, ERROR_SOCKET_ESOCKTNOSUPPORT, "Socket: socket type not supported"},
	  {SOCEOPNOTSUPP,      ERROR_SOCKET_EOPNOTSUPP,      "Socket: operation not supported"},
	  {SOCEPFNOSUPPORT,    ERROR_SOCKET_EPFNOSUPPORT,    "Socket: protocol family not supported"},
	  {SOCEAFNOSUPPORT,    ERROR_SOCKET_EAFNOSUPPORT,    "Socket: address family not supported"},
	  {SOCEADDRINUSE,      ERROR_SOCKET_EADDRINUSE,      "Socket: address is in use"},
	  {SOCEADDRNOTAVAIL,   ERROR_SOCKET_EADDRNOTAVAIL,   "Socket: address is not available"},
	  {SOCENETDOWN,        ERROR_SOCKET_ENETDOWN,        "Socket: This error may be reported at any time "
	                                                     "if the Windows Sockets implementation detects an "
                                                             "underlying failure."},
	  {SOCENETUNREACH,     ERROR_SOCKET_ENETUNREACH,     "Socket: network is unreachable"},
	  {SOCENETRESET,       ERROR_SOCKET_ENETRESET,       "Socket: network is reset"},
	  {SOCECONNABORTED,    ERROR_SOCKET_ECONNABORTED,    "Socket: connection aborted"},
	  {SOCECONNRESET,      ERROR_SOCKET_ECONNRESET,      "Socket: connection reset"},
	  {SOCENOBUFS,         ERROR_SOCKET_ENOBUFS,         "Socket: no buffers"},
	  {SOCEISCONN,         ERROR_SOCKET_EISCONN,         "Socket: is connected"},
	  {SOCENOTCONN,        ERROR_SOCKET_ENOTCONN,        "Socket: not connected"},
	  {SOCESHUTDOWN,       ERROR_SOCKET_ESHUTDOWN,       "Socket: shutdown"},
	  {SOCETOOMANYREFS,    ERROR_SOCKET_ETOOMANYREFS,    "Socket: too many references"},
	  {SOCETIMEDOUT,       ERROR_SOCKET_ETIMEDOUT,       "Socket: timed out"},
	  {SOCECONNREFUSED,    ERROR_SOCKET_ECONNREFUSED,    "Socket: connection refused"},
	  {SOCELOOP,           ERROR_SOCKET_ELOOP,           "Socket: loop"},
	  {SOCENAMETOOLONG,    ERROR_SOCKET_ENAMETOOLONG,    "Socket: name is too long"},
	  {SOCEHOSTDOWN,       ERROR_SOCKET_EHOSTDOWN,       "Socket: host is down"},
	  {SOCEHOSTUNREACH,    ERROR_SOCKET_EHOSTUNREACH,    "Socket: host is unreachable"},
#ifdef _WIN32
	  {WSASYSNOTREADY,     ERROR_SOCKET_SYSNOTREADY,     "Socket: Returned by WSAStartup, "
	                                                     "indicating that the network subsystem "
                                                             "is unusable."},
	  {WSAVERNOTSUPPORTED, ERROR_SOCKET_VERNOTSUPPORTED, "Socket: Returned by WSAStartup, "
	                                                     "indicating that the Windows "
                                                             "Sockets DLL cannot support this app."},
	  {WSANOTINITIALISED,  ERROR_SOCKET_NOTINITIALISED,  "Socket: Returned by any function "
	                                                     "except WSAStartup, indicating that "
                                                             "a successful WSAStartup, has not yet "
                                                             "been performed."},
	  {WSAHOST_NOT_FOUND,  ERROR_SOCKET_HOST_NOT_FOUND,  "Socket: host not found"},
	  {WSATRY_AGAIN,       ERROR_SOCKET_TRY_AGAIN,       "Socket: try again"},
	  {WSANO_RECOVERY,     ERROR_SOCKET_NO_RECOVERY,     "Socket: no recovery"},
	  {WSANO_DATA,         ERROR_SOCKET_NO_DATA,         "Socket: no data"},
#endif
	ERRORTABLEEND
  };

  static PHSErrorTable etSocket(&errortableSocket[0]);

  rc = etSocket.QueryFromOSError(iErrorCode,
	                             &pszMessage);

  /* @@@PH DEBUG */
  fprintf (stderr,
		   "\nSocketError(%i) = #%ud(%08xh) %s",
		   iErrorCode,
		   rc,
		   rc,
		   pszMessage);

  return (rc);                                        /* deliver return code */
}


/*****************************************************************************
 * Name      : RSOCKET::send
 * Purpose   : send data over a socket
 * Parameter : PVOID  pData        - pointer to the data buffer
 *             ULONG  ulDataLength - length of the data to send
 *             ULONG  ulFlags      - flags of the packet 
 *             PULONG pulBytesSend - pointer to variable for sent bytes counter
 * Variables :
 * Result    : API returncode
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

APIRET RSOCKET::send   (PVOID  pData,                                 /* send */
		                ULONG  ulDataLength,
				        ULONG  ulFlags,
						PULONG pulBytesSent)
{
  DEBUG("\nDEBUG: RSOCKET::send(%08xh, %u bytes, flags%08x )", pData, ulDataLength, ulFlags);	

  int rc;                                          /* return code from send() */

  if ( (pData        == NULL) ||                          /* check parameters */
	   (ulDataLength == 0   ) )
	return(ERROR_INVALID_PARAMETER);                /* signal error condition */

  rc = ::send (sockSocket,                   /* send the data over the socket */
	           (char *)pData,
		  	   ulDataLength,
			   ulFlags);
  if (rc == SOCKET_ERROR)                                 /* check for errors */
  {
    if (pulBytesSent != NULL)        /* check if caller is interested in this */
      *pulBytesSent = 0;                                  /* return the value */

    sockPerf.ulTotalErrors++;                                   /* statistics */
	sockPerf.ulPerfErrors++;

    return (SocketError(sockErrno));       /* return proper API error */
  }

  sockPerf.ulTotalTransactions++;                               /* statistics */
  sockPerf.ulTotalBytes += rc;
  sockPerf.ulPerfTransactions++;
  sockPerf.ulPerfBytes  += rc;

  if (pulBytesSent != NULL)          /* check if caller is interested in this */
    *pulBytesSent = (ULONG)rc;                            /* return the value */

  return (NO_ERROR);                                                    /* OK */
}


/*****************************************************************************
 * Name      : RSOCKET::recv
 * Purpose   : receive data from a socket
 * Parameter : PVOID  pData        - pointer to the data buffer
 *             ULONG  ulDataLength - length of the data to send
 *             ULONG  ulFlags      - flags of the packet 
 *             PULONG pulBytesRecv - pointer to variable for read bytes counter
 * Variables :
 * Result    : API returncode
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

APIRET RSOCKET::recv   (PVOID  pData,                                 /* send */
		                ULONG  ulDataLength,
				        ULONG  ulFlags,
						PULONG pulBytesRecv)
{
  DEBUG("\nDEBUG: RSOCKET::recv(%08xh, %u bytes, flags%08x )", pData, ulDataLength, ulFlags);	

  int rc;                                          /* return code from send() */

  if ( (pData        == NULL) ||                          /* check parameters */
	   (ulDataLength == 0   ) )
	return(ERROR_INVALID_PARAMETER);                /* signal error condition */

  rc = ::recv (sockSocket,                   /* send the data over the socket */
	           (char *)pData,
		  	   ulDataLength,
			   ulFlags);
  if (rc == SOCKET_ERROR)                                 /* check for errors */
  {
    if (pulBytesRecv != NULL)        /* check if caller is interested in this */
      *pulBytesRecv = 0;                                  /* return the value */

    sockPerf.ulTotalErrors++;
	sockPerf.ulPerfErrors++;

    return (SocketError(sockErrno));       /* return proper API error */
  }

  sockPerf.ulTotalTransactions++;
  sockPerf.ulTotalBytes += rc;
  sockPerf.ulPerfTransactions++;
  sockPerf.ulPerfBytes  += rc;

  if (pulBytesRecv != NULL)          /* check if caller is interested in this */
    *pulBytesRecv = (ULONG)rc;                            /* return the value */

  return (NO_ERROR);                                                    /* OK */
}


/*****************************************************************************
 * Name      : APIRET RSOCKET::bind
 * Purpose   : Bind a socket for service
 * Parameter : 
 * Variables :
 * Result    : API Returncode
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

APIRET RSOCKET::bind(ULONG ulIPAddress)
{  
  DEBUG("\nDEBUG: RSOCKET::bind(%08x)",ulIPAddress);	

  sockaddr_in sinAddress;

  sinAddress.sin_family           = AF_INET;
  sinAddress.sin_port             = htons((unsigned short)ulPort);
#ifdef _WIN32
  sinAddress.sin_addr.S_un.S_addr = htonl(ulIPAddress);
#endif

#ifdef __OS2__
  sinAddress.sin_addr.s_addr = htonl(ulIPAddress);
#endif

  if (::bind (sockSocket,
	          (struct sockaddr *)&sinAddress,
		      sizeof(sinAddress)) == SOCKET_ERROR)
  {
    sockPerf.ulTotalErrors++;                                  /* statistics */
	sockPerf.ulPerfErrors++;

    return (SocketError(sockErrno));      /* return proper API error */
  }
  else
	return (NO_ERROR);                                                 /* OK */
}


/*****************************************************************************
 * Name      : APIRET RSOCKET::listen
 * Purpose   : Listen a socket for service
 * Parameter : 
 * Variables :
 * Result    : API Returncode
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

APIRET RSOCKET::listen(int iQueueLength)
{  
  DEBUG("\nDEBUG: RSOCKET::listen(%i)",iQueueLength);	

  if (::listen (sockSocket,
                iQueueLength) == SOCKET_ERROR)
  {
    sockPerf.ulTotalErrors++;                                  /* statistics */
    sockPerf.ulPerfErrors++;

    return (SocketError(sockErrno));      /* return proper API error */
  }
  else
    return (NO_ERROR);                                                 /* OK */
}


/*****************************************************************************
 * Name      : APIRET RSOCKET::accept
 * Purpose   : Accept a socket for service
 * Parameter : 
 * Variables :
 * Result    : API Returncode
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

APIRET RSOCKET::accept(SOCKET      *psockSocket,
                       sockaddr_in *psinAddress,
                       PULONG      pulAddressLength)
{ 
  DEBUG("\nDEBUG: RSOCKET::accept(%08x,%08x, %08x)",psockSocket,psinAddress,pulAddressLength);	
	
  SOCKET      sockAccept;                   /* descriptor of accepted socket */

  if (psockSocket == NULL)                            /* check the parameter */	   
	return (ERROR_INVALID_PARAMETER);               /* raise error condition */
  
  sockAccept = ::accept (sockSocket,
                         (struct sockaddr *)psinAddress,
		         (int *)pulAddressLength);
  if (sockAccept == SOCKET_ERROR)
  {
    sockPerf.ulTotalErrors++;                                  /* statistics */
    sockPerf.ulPerfErrors++;

    *psockSocket = (SOCKET)0;                         /* no socket available */
    return (SocketError(sockErrno));      /* return proper API error */
  }
  else
  {
    *psockSocket = sockAccept;               /* return the socket descriptor */
    return (NO_ERROR);                                                 /* OK */
  }
}


/*****************************************************************************
 * Name      : APIRET RSOCKET::accept
 * Purpose   : Accept a socket for service
 * Parameter : 
 * Variables :
 * Result    : API Returncode
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

APIRET RSOCKET::accept(RSOCKET& rsockConnection)
{  
  DEBUG("\nDEBUG: RSOCKET::accept(RSOCKET)");

  SOCKET      sockAccept;                   /* descriptor of accepted socket */
  sockaddr_in sinAddress;
  ULONG       ulAddressLength = sizeof(sinAddress);
    
  sockAccept = ::accept (sockSocket,
                         (struct sockaddr *)&sinAddress,
     	                 (int *)&ulAddressLength);
  if (sockAccept == SOCKET_ERROR)
  {
    sockPerf.ulTotalErrors++;                                  /* statistics */
    sockPerf.ulPerfErrors++;

    return (SocketError(sockErrno));              /* return proper API error */
  }
  else
  {
    rsockConnection.SocketSet(sockAccept);              /* create new socket */
	/* @@@PH copy some more values */
    return (NO_ERROR);                                                 /* OK */
  }
}


/*****************************************************************************
 * Name      : APIRET RSOCKET::accept
 * Purpose   : Accept a socket for service
 * Parameter : 
 * Variables :
 * Result    : API Returncode
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

APIRET RSOCKET::close (void)                             /* close the socket */
{  
  DEBUG("\nDEBUG: RSOCKET::close(connected=%s,socket=%s)",bConnected ? "yes" : "no",bSocket ? "yes" : "no");	

  if (bConnected) ; /* disconnect */
  
  if (bSocket == TRUE)                              /* close the open socket */
  {
    if (soclose(sockSocket) == SOCKET_ERROR)         /* check for errors */
	{
      sockPerf.ulTotalErrors++;                                /* statistics */
	  sockPerf.ulPerfErrors++;

      return (SocketError(sockErrno));    /* return proper API error */
	}
  }
  
  return (NO_ERROR);                                                   /* OK */
}


/*****************************************************************************
 * Name      : APIRET RSOCKET::accept
 * Purpose   : Accept a socket for service
 * Parameter : 
 * Variables :
 * Result    : API Returncode
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

ULONG RSOCKET::collectPerfCounter (ULONG ulIndex)
{
  time_t tTime, deltaTime;                                /* UTC system time */
  float  fValue;                                              /* dummy value */

#define DYNAMIC_PERF(a,factor)                 \
      tTime = time(NULL);                      \
	  deltaTime = sockPerf.ulPerfTime - tTime; \
	  if (deltaTime == 0) return ( 0 );        \
	  fValue = (float)a * (float)factor /      \
               (float)deltaTime;               \
	  sockPerf.ulPerfTime = tTime;             \
	  a = 0;                                   \
	  return ( (ULONG) fValue );               


  switch (ulIndex)
  {
    /* static requests */
    case RSOCKET_PERF_TOTAL_TRANSACTIONS: return (sockPerf.ulTotalTransactions);
    case RSOCKET_PERF_TOTAL_BYTES:        return (sockPerf.ulTotalBytes);
    case RSOCKET_PERF_TOTAL_ERRORS:       return (sockPerf.ulTotalErrors);
    case RSOCKET_PERF_TIME:               return (sockPerf.ulPerfTime);

    /* dynamic requests */
    case RSOCKET_PERF_TRANSACTIONS:       DYNAMIC_PERF(sockPerf.ulPerfTransactions,1000)
    case RSOCKET_PERF_BYTES:              DYNAMIC_PERF(sockPerf.ulPerfBytes,1000)
    case RSOCKET_PERF_ERRORS:             DYNAMIC_PERF(sockPerf.ulPerfErrors,60000)
    default:                              return ( 0 );           /* default */
  }  
}