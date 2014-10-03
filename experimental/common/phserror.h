/*****************************************************************************
 * Name      : PHSErrorTable
 * Purpose   : Class / Functions for universal error mapping table 
 * Parameter : 
 * Variables :
 * Result    : 
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

#ifndef MODULE_PHS_ERROR
#define MODULE_PHS_ERROR


/*****************************************************************************
 * Includes                                                                  *         
 *****************************************************************************/


/*****************************************************************************
 * Definitions                                                               *         
 *****************************************************************************/

typedef unsigned long APIRET;
typedef char *        PSZ;  
typedef PSZ  *        PPSZ;

#ifndef NULL
#define NULL 0L
#endif


/* OS/2 error codes */
#ifndef NO_ERROR
  #define NO_ERROR                        0
  #define ERROR_INVALID_PARAMETER         87
#endif

#define ERROR_PHS                           0xc0000000
#define ERROR_MESSAGE_NO_TABLE              ERROR_PHS + 0
#define ERROR_MESSAGE_NOT_FOUND             ERROR_PHS + 1

#define ERROR_SOCKET_ALREADY_CONNECTED		ERROR_PHS + 1000
#define ERROR_SOCKET_CREATION         		ERROR_PHS + 1001

#define ERROR_SOCKET_EINTR         		    ERROR_PHS + 1002
#define ERROR_SOCKET_EBADF                  ERROR_PHS + 1003
#define ERROR_SOCKET_EACCES                 ERROR_PHS + 1004 
#define ERROR_SOCKET_EDISCON                ERROR_PHS + 1005         
#define ERROR_SOCKET_EFAULT                 ERROR_PHS + 1006
#define ERROR_SOCKET_EINVAL                 ERROR_PHS + 1007
#define ERROR_SOCKET_EMFILE                 ERROR_PHS + 1008
#define ERROR_SOCKET_EWOULDBLOCK            ERROR_PHS + 1009
#define ERROR_SOCKET_EINPROGRESS            ERROR_PHS + 1010
#define ERROR_SOCKET_EALREADY               ERROR_PHS + 1011
#define ERROR_SOCKET_ENOTSOCK               ERROR_PHS + 1012
#define ERROR_SOCKET_EDESTADDRREQ           ERROR_PHS + 1013
#define ERROR_SOCKET_EMSGSIZE               ERROR_PHS + 1014
#define ERROR_SOCKET_EPROTOTYPE             ERROR_PHS + 1015
#define ERROR_SOCKET_ENOPROTOOPT            ERROR_PHS + 1016
#define ERROR_SOCKET_EPROTONOSUPPORT        ERROR_PHS + 1017
#define ERROR_SOCKET_ESOCKTNOSUPPORT        ERROR_PHS + 1018
#define ERROR_SOCKET_EOPNOTSUPP             ERROR_PHS + 1019
#define ERROR_SOCKET_EPFNOSUPPORT           ERROR_PHS + 1020
#define ERROR_SOCKET_EAFNOSUPPORT           ERROR_PHS + 1021
#define ERROR_SOCKET_EADDRINUSE             ERROR_PHS + 1022
#define ERROR_SOCKET_EADDRNOTAVAIL          ERROR_PHS + 1023
#define ERROR_SOCKET_ENETDOWN               ERROR_PHS + 1024
#define ERROR_SOCKET_ENETUNREACH            ERROR_PHS + 1025
#define ERROR_SOCKET_ENETRESET              ERROR_PHS + 1026
#define ERROR_SOCKET_ECONNABORTED           ERROR_PHS + 1027
#define ERROR_SOCKET_ECONNRESET             ERROR_PHS + 1028
#define ERROR_SOCKET_ENOBUFS                ERROR_PHS + 1029
#define ERROR_SOCKET_EISCONN                ERROR_PHS + 1030
#define ERROR_SOCKET_ENOTCONN               ERROR_PHS + 1031
#define ERROR_SOCKET_ESHUTDOWN              ERROR_PHS + 1032
#define ERROR_SOCKET_ETOOMANYREFS           ERROR_PHS + 1033
#define ERROR_SOCKET_ETIMEDOUT              ERROR_PHS + 1034
#define ERROR_SOCKET_ECONNREFUSED           ERROR_PHS + 1035
#define ERROR_SOCKET_ELOOP                  ERROR_PHS + 1036
#define ERROR_SOCKET_ENAMETOOLONG           ERROR_PHS + 1037
#define ERROR_SOCKET_EHOSTDOWN              ERROR_PHS + 1038
#define ERROR_SOCKET_EHOSTUNREACH           ERROR_PHS + 1039
#define ERROR_SOCKET_SYSNOTREADY            ERROR_PHS + 1040
#define ERROR_SOCKET_VERNOTSUPPORTED        ERROR_PHS + 1041
#define ERROR_SOCKET_NOTINITIALISED         ERROR_PHS + 1042
#define ERROR_SOCKET_HOST_NOT_FOUND         ERROR_PHS + 1043
#define ERROR_SOCKET_TRY_AGAIN              ERROR_PHS + 1044
#define ERROR_SOCKET_NO_RECOVERY            ERROR_PHS + 1045
#define ERROR_SOCKET_NO_DATA                ERROR_PHS + 1046


/*****************************************************************************
 * Structures                                                                *         
 *****************************************************************************/

typedef struct _ErrorTable
{
  int    iOSError;                                   /* OS native error code */
  APIRET rcError;                                       /* mapped error code */
  PSZ    pszMessage;                                              /* message */
} ERRORTABLE, *PERRORTABLE;

#define ERRORTABLEEND {0,0,NULL}


class PHSErrorTable
{
  protected:
    PERRORTABLE pErrorTable;          /* pointer to the internal error table */

  public:
	PHSErrorTable(PERRORTABLE pErrorTable);                   /* constructor */
	~PHSErrorTable(void);                                      /* destructor */

	APIRET      ErrorTableSet  (PERRORTABLE pErrorTable);   /* set new table */
	PERRORTABLE ErrorTableQuery(void);                /* query table pointer */

	APIRET QueryFromOSError(int  iOSError,  /* query from OS dependent error */
		                    PPSZ ppszMessage);  

    APIRET Query           (APIRET rc,            /* query from socket error */
		                    PPSZ ppszMessage);  
};


#endif /* MODULE_PHS_ERROR */
