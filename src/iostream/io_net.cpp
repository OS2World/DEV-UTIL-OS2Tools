/*****************************************************
 * Unix-like NetCat Tool.                            *
 *                                                   *
 * (c) 2002 Patrick Haller                           *
 *****************************************************/

/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#ifdef __OS2__
  #define INCL_DOSMISC
  #define INCL_DOSFILEMGR
  #define INCL_BASE
  #define INCL_NOPMAPI
  #include <os2.h>
#endif

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <process.h>
#include <ctype.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"


#define TCPV40HDRS

#include <sys/time.h>		/* timeval, time_t */
// #include <setjmp.h>		/* jmp_buf et al */
#include <sys/socket.h>		/* basics, SO_ and AF_ defs, sockaddr, ... */
#include <sys/select.h>	    
#include <netinet/in.h>		/* sockaddr_in, htons, in_addr */
#include <netinet/in_systm.h>	/* misc crud that netinet/ip.h references */
#include <netinet/ip.h>		/* IPOPT_LSRR, header stuff */
#include <netdb.h>		/* hostent, gethostby*, getservby* */
#include <errno.h>
#include <signal.h>
#include <fcntl.h>		/* O_WRONLY et al */
#include <types.h>


#include "iostream.h"


/*****************************************************************************
 * Defines                                                                   *
 *****************************************************************************/

/* TCP/IP header additions (to equalize backlevel headers */
#ifndef SO_REUSEPORT
#define SO_REUSEPORT 0x1000
#endif


#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif
#ifdef MAXHOSTNAMELEN
#undef MAXHOSTNAMELEN		/* might be too small on aix, so fix it */
#endif
#define MAXHOSTNAMELEN 256


/* local prototypes for dynamically loaded functions */
/*
 
 inet_addr
 sock_init
 connect
 gethostbyname
 htons
 soclose
 socket
 listen
 bind
 accept
 sock_errno
 recv
 send
 */

// global module handles
static HMODULE hmodTCP32DLL = NULLHANDLE;
static HMODULE hmodSO32DLL  = NULLHANDLE;


// sys/socket.h
int  (* _System net_accept     )( int, struct sockaddr *, int * );
//void (* _System net_addsockettolist)(int);
int  (* _System net_bind       )( int, struct sockaddr *, int );
int  (* _System net_connect    )( int, struct sockaddr *, int );
//int  (* _System net_gethostid  )(void);
//int  (* _System net_getpeername)( int, struct sockaddr *, int * );
//int  (* _System net_getsockname)( int, struct sockaddr *, int * );
//int  (* _System net_getsockopt )( int, int, int, char *, int * );
//int  (* _System net_ioctl      )(int, int, char *, int);
int  (* _System net_listen     )( int, int);
int  (* _System net_recv       )( int, char *, int, int );
int  (* _System net_send       )( int, char *, int, int );
//int  (* _System net_sendmsg    )( int, struct msghdr * , int);
int  (* _System net_sendto     )( int, char *, int, int, struct sockaddr *, int);
//int  (* _System net_setsockopt )( int, int, int, char *, int );
int  (* _System net_sock_init  )( void );
int  (* _System net_sock_errno )( void );
void (* _System net_psock_errno)( char * );
int  (* _System net_socket     )( int, int, int );
int  (* _System net_soclose    )( int );
//int  (* _System net_so_cancel  )(int);

// netinet/in.h
unsigned long (* _System net_inet_addr)(char *);

// netdb.h
struct hostent * (* _System net_gethostbyname)( char * );

// utils.h
unsigned short (* _System net_bswap)(unsigned short);


/* Definition for dynamic bswap */
#define net_htons(x)   (net_bswap(x))
  
  


APIRET i_startup(void)
{
  static int    flagInitialized = 0;
  static APIRET rcNet = NO_ERROR;
  
  // shortcut
  if (1 == flagInitialized)
    return rcNet;
  
  if (0 == flagInitialized)
  {
    flagInitialized = 1;
    
    // Try to load the above entry points from the
    // tcp/ip dlls if available. This may very well fail if
    // the IOStream utility is run from a set of boot disks, etc.
    CHAR   szModuleFailure[ 256 ];
    
    // load TCP32DLL.DLL
    rcNet = DosLoadModule(szModuleFailure,
                          sizeof( szModuleFailure ),
                          "TCP32DLL",
                          &hmodTCP32DLL);
    if (NO_ERROR == rcNet)
      // load SO32DLL.DLL
      rcNet = DosLoadModule(szModuleFailure,
                            sizeof( szModuleFailure ),
                            "SO32DLL",
                            &hmodSO32DLL);
    
    if (NO_ERROR != rcNet)
    {
      Debug(("Error: unable to load module %s\n",
             szModuleFailure));
    }
    else
    {

      // OK, both modules have been mapped into addres space
      // successfully, not query the dynamic entry points.
#define QPROCADDR(hmod,name,entry)                                   \
      rcNet = DosQueryProcAddr(hmod, 0, name, (PFN*)&entry);            \
      if (NO_ERROR != rcNet)                                            \
      {                                                              \
        strncpy( szModuleFailure, name, sizeof( szModuleFailure ) ); \
        goto _err;                                                   \
      }
      
      QPROCADDR(hmodSO32DLL,  "ACCEPT",        net_accept)
      QPROCADDR(hmodSO32DLL,  "BIND",          net_bind)
      QPROCADDR(hmodSO32DLL,  "CONNECT",       net_connect)
      QPROCADDR(hmodSO32DLL,  "LISTEN",        net_listen)
      QPROCADDR(hmodSO32DLL,  "RECV",          net_recv)
      QPROCADDR(hmodSO32DLL,  "SEND",          net_send)
      QPROCADDR(hmodSO32DLL,  "SOCK_INIT",     net_sock_init)
      QPROCADDR(hmodSO32DLL,  "SOCK_ERRNO",    net_sock_errno)
      QPROCADDR(hmodSO32DLL,  "PSOCK_ERRNO",   net_psock_errno)
      QPROCADDR(hmodSO32DLL,  "SOCKET",        net_socket)
      QPROCADDR(hmodSO32DLL,  "SOCLOSE",       net_soclose)
        
      QPROCADDR(hmodTCP32DLL, "INET_ADDR",     net_inet_addr)
      QPROCADDR(hmodTCP32DLL, "GETHOSTBYNAME", net_gethostbyname)
      QPROCADDR(hmodTCP32DLL, "BSWAP",         net_bswap)
    }
    
    _err:
    if (NO_ERROR != rcNet)
    {
      Debug(("Error while dynamically loading TCP/IP functions:\n#%d - %s\n",
             rcNet,
             szModuleFailure));
      
      // abort further network operations
      return rcNet;
    }
    
    
    int rc = net_sock_init();                  /* initialize tcp/ip stack */
    if (0 != rc)                                /* check socket creation */
    {
      net_psock_errno("Can't initialize tcp/ip stack");
      rcNet = ERROR_NOT_SUPPORTED;
    }
  }
  
  return rcNet;
}



int SockConnect(int iSocket,
                struct sockaddr *name,
                int iNameLength)
{
  return net_connect( iSocket, name, iNameLength );
}


APIRET parseIP(struct sockaddr_in *to,
               PSZ pszTarget)
{
  struct hostent *pHostEntry;
  char hnamebuf[128];

  int iAddress = (int)net_inet_addr(pszTarget);
  if (iAddress != -1)
  {
    memset(to, 0, sizeof( sockaddr_in ) );
    to->sin_addr.s_addr = iAddress;
    to->sin_family      = AF_INET;
  }
  else
  {
    pHostEntry = net_gethostbyname(pszTarget);

    if (!pHostEntry)
    {
      fprintf(stderr,
              "Error: unknown host %s\n",
              pszTarget);
      return ERROR_PATH_NOT_FOUND;
    }

    to->sin_family = pHostEntry->h_addrtype;

    if (pHostEntry->h_length > sizeof(to->sin_addr))
      pHostEntry->h_length = sizeof(to->sin_addr);

    memcpy(&to->sin_addr,
           pHostEntry->h_addr,
           pHostEntry->h_length);
  }

  return NO_ERROR;
}
  
  



 /*
  
  IOStream
  +---IOStreamInput
  !   +--IOStreamInputStdIn   OK
  !   +--IOStreamInputTCP
  !   +--IOStreamInputFile
  !   +--IOStreamInputDASD
  !   +--IOStreamInputOne     OK
  !   +--IOStreamInputZero    OK
  !   +--IOStreamInputRandom  OK
  +---IOStreamOutput
      +--IOStreamOutputStdOut OK
      +--IOStreamOutputTCP
      +--IOStreamOutputFile
      +--IOStreamOutputGZIP
      +--IOStreamOutputUnGZIP
      +--IOStreamOutputBZIP2
      +--IOStreamOutputUnBZIP2
      +--IOStreamOutputDASD
      +--IOStreamOutputNull   OK
      */



IOStreamOutputTCP::IOStreamOutputTCP( PSZ pszStreamToken )
: IOStreamOutput("IOStreamOutputTCP",
                 "Output stream to TCP connection",
                 pszStreamToken)
{
  // start the IP stack
  APIRET rc = i_startup();
  if (NO_ERROR != rc)
    setAvailable( FALSE );

  // cut off leading "TCP:" signature
  if (0 == strnicmp("TCP:", pszStreamToken, 4))
    pszStreamToken += 4;

  pszURL = strdup( pszStreamToken );
}


IOStreamOutputTCP::~IOStreamOutputTCP( )
{
  if (0 != sockOutput)
  {
    Debug(("~IOStreamOutputTCP called with open socket (%d)",
           sockOutput))

    net_soclose( sockOutput );
    sockOutput = 0;
  }

  if (NULL != pszURL)
    free( pszURL );
}


/* setup connection to 'device' */
APIRET IOStreamOutputTCP::connect( IOStreamInput* in )
{
  // 1 - parse the URL into hostname and port
  PSZ pszHostname = pszURL;
  PSZ pszPort = strchr(pszHostname, ':');
  int iPort = 0;

  if (NULL != pszPort)
    iPort = atoi( pszPort+1 );

  if (0 == iPort)
  {
    fprintf(stderr,
            "ERROR: '%s' is not a valid URL (host:port)\n",
            pszURL);

    return ERROR_INVALID_PARAMETER;
  }

  // separate the two parts
  *pszPort = 0;

  // OK, we now can try to establish the connection
  sockaddr_in to;
  APIRET      rc;
  int         irc;

  rc = parseIP(&to,
               pszHostname);
  if (NO_ERROR != rc)
    return rc;

  to.sin_port = net_htons( iPort );

  // try to connect to the remote host
  sockOutput = net_socket(PF_INET,
                              SOCK_STREAM,
                              IPPROTO_TCP);
  if (-1 == sockOutput)
  {
    fprintf(stderr,
            "ERROR: cannot create socket\n");

    return ERROR_OPEN_FAILED;
  }

  irc = SockConnect( sockOutput,
                        (struct sockaddr *)&to, 
                    sizeof( to ) );
  if (-1 == irc)
  {
    fprintf(stderr,
            "ERROR: cannot connect to '%s' on port '%d'\n",
            pszHostname,
            iPort);

    net_soclose( sockOutput );
    sockOutput = 0;
    return ERROR_OPEN_FAILED;
  }

  return NO_ERROR;
}

/* terminate existing connection to 'device' */
APIRET IOStreamOutputTCP::disconnect()
{
  // @@@PH map error codes
  if (-1 == net_soclose( sockOutput ) )
    return net_sock_errno();

  sockOutput = 0;
  return NO_ERROR;
}
  

APIRET IOStreamOutputTCP::write(IOBuffer *ioBuffer)
{
  APIRET rc = NO_ERROR;
  PVOID  pData = ioBuffer->acquire();

  // @@@PH map error codes
  if (-1 == net_send(sockOutput,
                         (char*)pData,
                         ioBuffer->getValid(),
                         0) )
    rc = net_sock_errno();

  ioBuffer->release();

  return rc;
}


IOStreamInputTCP::IOStreamInputTCP( PSZ pszStreamToken )
: IOStreamInput("IOStreamInputTCP",
                "Input stream from TCP connection",
                pszStreamToken)
{
  // start the IP stack
  APIRET rc = i_startup();
  if (NO_ERROR != rc)
    setAvailable( FALSE );

  // cut off leading "TCP:" signature
  if (0 == strnicmp("TCP:", pszStreamToken, 4))
    pszStreamToken += 4;

  pszURL = strdup( pszStreamToken );
}


IOStreamInputTCP::~IOStreamInputTCP( )
{
  if (0 != sockInput)
  {
    Debug(("~IOStreamInputTCP called with open socket (%d)",
           sockInput))

    net_soclose( sockInput );
    sockInput = 0;
  }

  if (NULL != pszURL)
    free( pszURL );
}

  
/* setup connection to 'device' */
APIRET IOStreamInputTCP::connect( )
{
  // 1 - parse the URL into hostname and port
  PSZ pszListen = pszURL;
  PSZ pszPort = strchr(pszListen, ':');
  int iPort = 0;

  if (NULL != pszPort)
    pszListen = pszPort + 1;

  iPort = atoi( pszListen );

  if (0 == iPort)
  {
    fprintf(stderr,
            "ERROR: '%s' is not a valid port number\n",
            pszURL);

    return ERROR_INVALID_PARAMETER;
  }

  // OK, we now can try to establish the connection
  sockaddr_in sin_from;
  sockaddr_in sin_listen;
  int         sin_fromlen;
  APIRET      rc;
  int         irc;

  // try to connect to the remote host
  sockListen = net_socket(PF_INET,
                              SOCK_STREAM,
                              IPPROTO_TCP);
  if (-1 == sockListen)
  {
    fprintf(stderr,
            "ERROR: cannot create socket\n");

    return ERROR_OPEN_FAILED;
  }

  // bind the socket
  sin_listen.sin_family = AF_INET;
  sin_listen.sin_addr.s_addr = INADDR_ANY;
  sin_listen.sin_port = net_htons( iPort );

  irc = net_bind(sockListen,
                     (struct sockaddr *)&sin_listen,
                     sizeof( sin_listen ));
  if (-1 == irc)
  {
    perror("Binding socket for incoming connection");
    net_soclose( sockListen );
    sockInput = 0;
    return ERROR_OPEN_FAILED;
  }

  // listen for incoming connection
  irc = net_listen( sockListen,
               1 );
  if (-1 == irc)
  {
    perror("Listening for incoming connection");
    net_soclose( sockListen );
    sockInput = 0;
    return ERROR_OPEN_FAILED;
  }

  // accept that connection
  sockInput = net_accept(sockListen, 
                             (struct sockaddr *)&sin_from, 
                             &sin_fromlen);
  if (-1 == sockInput)
  {
    perror("Accepting incoming connection");
    net_soclose( sockListen );
    sockListen = 0;
    return ERROR_OPEN_FAILED;
  }

  // OK, connection is established

  return NO_ERROR;
}


/* terminate existing connection to 'device' */
APIRET IOStreamInputTCP::disconnect()
{
  // @@@PH map error codes
  int rc1 = net_soclose( sockInput );
  int rc2 = net_soclose( sockListen );

  if (-1 == rc1 )
    return net_sock_errno();

  if (-1 == rc2 )
    return net_sock_errno();

  sockListen = 0;
      sockInput = 0;
  return NO_ERROR;
}
  

APIRET IOStreamInputTCP::read(IOBuffer *ioBuffer)
{
  APIRET rc = NO_ERROR;
  PVOID  pData = ioBuffer->acquire();

  int irc = net_recv(sockInput,
                         (char*)pData,
                         ioBuffer->getSize(),
                 0);
  if (irc == 0)
  {
    // socket has closed!
  }
  else
    if (irc == -1)
    {
      // @@@PH map error codes
      rc = net_sock_errno();
    }

  ioBuffer->setValid( irc );

  ioBuffer->release();

  return rc;
}
