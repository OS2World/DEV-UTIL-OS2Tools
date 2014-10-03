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
#include <conio.h>
#include <string.h>

#ifdef _WIN32
  #include <windows.h>
  #include <winsock.h>
#endif

#ifdef __OS2__
  #define INCL_DOS
  #define INCL_BASE
  #include <os2.h>
#endif

#include "phsarg.h"
#include "phstypes.h"
#include "rsocket.h"


/*****************************************************************************
 * Definitions                                                               *
 *****************************************************************************/

#ifdef _WIN32
typedef char * PSZ;
#endif

#ifdef __OS2__
typedef ULONG HANDLE;
typedef ULONG DWORD;
#endif


/*****************************************************************************
 * Structures                                                                *         
 *****************************************************************************/


typedef struct _Reflection
{
  PSZ   pszName;                                  /* name of this reflection */  

  PRSOCKET             prsockInput;      /* recv from client, send to server */
  PRSOCKET             prsockOutput;     /* recv from server, send to client */
} REFLECTION, *PREFLECTION;


typedef struct _ReflectorParams            /* parameter for reflector thread */
{
  PSZ   pszName;
  ULONG ulIPClient;
  ULONG ulPortInput;
  ULONG ulPortOutput;
  ULONG ulTypeInput;
  ULONG ulTypeOutput;
  ULONG ulProtocolInput;
  ULONG ulProtocolOutput;
} REFLECTORPARAMS, *PREFLECTORPARAMS;


class Reflector
{
  protected:
	PSZ pszName;                                   /* name of this reflector */

  public:
	  Reflector(PSZ pszNewName);                             /* constructor */
	  ~Reflector(void);                                       /* destructor */

	  APIRET ReflectConnection      (ULONG ulIPClient,        /* reflector ! */
									 ULONG ulPortInput,
									 ULONG ulPortOutput,
									 ULONG ulTypeInput,
									 ULONG ulTypeOutput,
									 ULONG ulProtocolInput,
									 ULONG ulProtocolOutput);
};


/*****************************************************************************
 * Name      : 
 * Purpose   : 
 * Parameter : 
 * Variables :
 * Result    : 
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

Reflector::Reflector(PSZ pszNewName)
{
                                                        /* 1st copy the name */
  if (pszNewName == NULL)                  /* check whether specified or not */
    pszNewName = "<unnamed>";                       /* provide default value */
  pszName = strdup(pszNewName);                             /* copy the name */    
}


/*****************************************************************************
 * Name      : 
 * Purpose   : 
 * Parameter : 
 * Variables :
 * Result    : 
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

Reflector::~Reflector(void)
{
  if (pszName != NULL)                        /* check if name was allocated */
	  free (pszName);                                       /* free the name */
}


/*****************************************************************************
 * Name      : Reflector::ReflectTransport
 * Purpose   : this is the main reflector :)
 * Parameter : 
 * Variables :
 * Result    : 
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

APIRET ReflectTransport(PSZ      pszName,
						PRSOCKET prsockSource,
	   				    PRSOCKET prsockDest)
{
  APIRET rc;                                               /* API-Returncode */
  char   szBuffer[8192];
  ULONG  ulBytesRead;
  ULONG  ulBytesWritten;

  do
  {  	  
    rc = prsockSource->recv(szBuffer,
		                    sizeof(szBuffer),
					        0, 
				            &ulBytesRead);
	if (rc) 
	  break;

	if (ulBytesRead)
	{
      rc = prsockDest->send(szBuffer,
	                        ulBytesRead,
			  	            0,
			 	            &ulBytesWritten);
      if (rc) 
  	    break;
	}
	else
	  break;
  }
  while (!rc);  

  printf ("\n%-20s: Transport terminated with #%u", 
	      pszName,
	      rc);

  prsockSource->close(); /* close ports to unblock partner thread */
  prsockDest->close();

  return (rc);
}


/*****************************************************************************
 * Name      : ReflectHelperCS
 * Purpose   : helper: transports from input to output socket
 * Parameter :
 * Variables :
 * Result    : 
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

unsigned long __stdcall ReflectHelperCS(PVOID pData)
{
  PREFLECTION pReflection = (PREFLECTION)pData;
  APIRET      rc;

  rc = ReflectTransport(pReflection->pszName,
	                    pReflection->prsockInput,
	                    pReflection->prsockOutput);
  return (rc);
}


/*****************************************************************************
 * Name      : ReflectHelperSC
 * Purpose   : helper: transports from output to input socket
 * Parameter : 
 * Variables :
 * Result    : 
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

unsigned long __stdcall ReflectHelperSC(PVOID pData)
{
  PREFLECTION pReflection = (PREFLECTION)pData;
  APIRET      rc;

  rc = ReflectTransport(pReflection->pszName,
	                    pReflection->prsockOutput,
	                    pReflection->prsockInput);

  delete pReflection;    /* @@@PH this thread is responsible for cleaning up */

  return (rc);
}


/*****************************************************************************
 * Name      : ReflectHelperReflection
 * Purpose   : helper: create listener thread
 * Parameter : 
 * Variables :
 * Result    : 
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

unsigned long __stdcall ReflectHelperReflection(PVOID pData)
{
  APIRET           rc = NO_ERROR;                          /* API returncode */
  PREFLECTORPARAMS pReflectorParams = (PREFLECTORPARAMS)pData;

  Reflector        rReflector (pReflectorParams->pszName);

  rReflector.ReflectConnection(pReflectorParams->ulIPClient,
	                           pReflectorParams->ulPortInput,
							   pReflectorParams->ulPortOutput,
							   pReflectorParams->ulTypeInput,
							   pReflectorParams->ulTypeOutput,
							   pReflectorParams->ulProtocolInput,
							   pReflectorParams->ulProtocolOutput);
  free (pData);                                 /* free reflector parameters */

  return (rc);
}


/*****************************************************************************
 * Name      : ReflectionCreate
 * Purpose   : helper: creates reflection structures and threads
 * Parameter : 
 * Variables :
 * Result    : 
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

HANDLE ReflectionCreate(PSZ   pszName,
                        ULONG ulIPClient,
                        ULONG ulPortInput,
                        ULONG ulPortOutput,
                        ULONG ulTypeInput,
                        ULONG ulTypeOutput,
                        ULONG ulProtocolInput,
                        ULONG ulProtocolOutput)
{
  PREFLECTORPARAMS pReflectorParams;
  DWORD            tidThreadReflector;


  printf ("\n%-20s:ip=%08xh port=%u,%u type=%u,%u protocol=%u,%u",
	      pszName,
		  ulIPClient,
		  ulPortInput,
		  ulPortOutput,
		  ulTypeInput,
		  ulTypeOutput,
		  ulProtocolInput,
		  ulProtocolOutput);

  pReflectorParams = (PREFLECTORPARAMS)
	                 malloc( sizeof(REFLECTORPARAMS) );      /* alloc memory */
  if (pReflectorParams == NULL)                          /* check allocation */
	return ( (HANDLE) 0L );                                  /* signal error */

  pReflectorParams->pszName          = pszName;            /* map parameters */
  pReflectorParams->ulIPClient       = ulIPClient;
  pReflectorParams->ulPortInput      = ulPortInput;
  pReflectorParams->ulPortOutput     = ulPortOutput;
  pReflectorParams->ulTypeInput      = ulTypeInput;
  pReflectorParams->ulTypeOutput     = ulTypeOutput;
  pReflectorParams->ulProtocolInput  = ulProtocolInput;
  pReflectorParams->ulProtocolOutput = ulProtocolOutput;

  return (           CreateThread (NULL,          /* create transport thread */
                                   0,
                                   ReflectHelperReflection,
                                   pReflectorParams,
                                   0,
                                   &tidThreadReflector)
  		 );
}



/*****************************************************************************
 * Name      : Reflector::ReflectConnection
 * Purpose   : this is the main reflector :)
 * Parameter : 
 * Variables :
 * Result    : 
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

APIRET Reflector::ReflectConnection(ULONG ulIPClient,
				    ULONG ulPortInput,
				    ULONG ulPortOutput,
				    ULONG ulTypeInput,
				    ULONG ulTypeOutput,
				    ULONG ulProtocolInput,
				    ULONG ulProtocolOutput)
{
  APIRET rc;
  HANDLE hThreadCS;
  HANDLE hThreadSC;
  DWORD  tidThreadCS;
  DWORD  tidThreadSC;

  RSOCKET rsockListener = RSOCKET(ulPortOutput,
	                              ulTypeOutput,
								  ulProtocolOutput);
  PREFLECTION pReflection;

  rc = rsockListener.init();
  if (rc) return (rc);

  rc = rsockListener.bind();
  if (rc) return (rc);


  for(;;)
  {
    printf ("\n%-20s: Waiting for connection on port %u.",
		    pszName,
	        ulPortOutput);

    pReflection = new REFLECTION;                   /* create new reflection */

    pReflection->pszName     = pszName;

    pReflection->prsockInput = new RSOCKET (ulPortInput,
		                                    ulTypeInput,
											ulProtocolInput);
	rc = pReflection->prsockInput->init();
    if (rc) return (rc);

    pReflection->prsockOutput = new RSOCKET;

    rc = rsockListener.listen();
    if (rc) return (rc);

    rc = rsockListener.accept(*pReflection->prsockOutput);
    if (rc)
	  return (rc);

    rc = pReflection->prsockInput->connect(ulIPClient); /* connect to SMTP server T22113 */
    if (rc) 
	  return (rc);

    printf ("\n%-20s: Connection on port %u to port %u is active.",
		    pszName,
	        ulPortOutput,
			ulPortInput);


    hThreadCS = CreateThread (NULL,               /* create transport thread */
                              16384,
                              ReflectHelperCS,
                              pReflection,
                              0,
                              &tidThreadCS);

    hThreadSC = CreateThread (NULL,               /* create transport thread */
                              16384,
                              ReflectHelperSC,
                              pReflection,
                              0,
                              &tidThreadSC);
	CloseHandle(hThreadCS);                           /* close threadhandles */
	CloseHandle(hThreadSC);
  }

  return 0;
}


/*****************************************************************************
 * Name      : 
 * Purpose   : 
 * Parameter : 
 * Variables :
 * Result    : 
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

int main (int argc, char **argv)
{
  int iAttachedToStack;           /* indiates if we are attached to IP stack */
  int iErrorState;                         /* current object error condition */
  WSADATA wsaData;
	
  iErrorState = WSAStartup (MAKEWORD(1,0),    /* initialize the TCP/IP stack */
	                        &wsaData);
  if (iErrorState != 0)                                  /* check for errors */
  {
	iAttachedToStack = 0;                         /* means no Winsock in use */
	fprintf (stderr,
	         "\nError: can't initialize WinSock interface.");
  }
  else
  {
	iAttachedToStack = 1;                         /* means no Winsock in use */

    printf ("\nAttached to Winsock:"
		    "\n  Version      %u"
			"\n  High Version %u"
			"\n  Description  %s"
			"\n  Status       %s"
			"\n  Max Sockets  %u"
			"\n  Max UDPs     %u",            
			wsaData.wVersion,
			wsaData.wHighVersion,
			wsaData.szDescription,
			wsaData.szSystemStatus,
			wsaData.iMaxSockets,
			wsaData.iMaxUdpDg);
  }

  printf ("\nInitialization OK");
	                     

  /* create reflectors */
  HANDLE hThreadTest;

  hThreadTest = ReflectionCreate("SENDMAIL.TCP",
                                 0x823d1603,
                                 25,
                                 25,
                                 SOCK_STREAM,
                                 SOCK_STREAM,
                                 IPPROTO_TCP,
                                 IPPROTO_TCP);

  hThreadTest = ReflectionCreate("PRINTER.TCP",
                                 0x823d1603,
                                 515,
                                 515,
                                 SOCK_STREAM,
                                 SOCK_STREAM,
                                 IPPROTO_TCP,
                                 IPPROTO_TCP);

  hThreadTest = ReflectionCreate("TELNET.TCP",
                                 0x823d1603,
                                 23,
                                 23,
                                 SOCK_STREAM,
                                 SOCK_STREAM,
                                 IPPROTO_TCP,
                                 IPPROTO_TCP);

  hThreadTest = ReflectionCreate("TELNET.Redirect",
                                 0x823d1603,
                                 23,
                                 1023,
                                 SOCK_STREAM,
                                 SOCK_STREAM,
                                 IPPROTO_TCP,
                                 IPPROTO_TCP);

  getch();


  if (iAttachedToStack == 1)                 /* are we attached to WinSock ? */
    iErrorState = WSACleanup();                        /* clean up the stack */


  return (0);                            /* return code for operating system */
}



