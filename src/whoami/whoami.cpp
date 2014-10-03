/*****************************************************
 * Unix-like WHOAMI tool                             *
 *                                                   *
 * (c) 2002 Patrick Haller                           *
 *****************************************************/


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#define  INCL_DOS
#define  INCL_DOSERRORS
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"                                              /* phstools */


#define PURE_32
#define INCL_32
#include <upm.h>


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG  fsHelp;                       /* help is requested from command line */
  ARGFLAG  fsVerbose;                    /* verbose mode                        */
  ARGFLAG  fsRemoteNode;                 /* remote server name specified        */
  ARGFLAG  fsRemoteDomain;               /* remote domain name specified        */
  ARGFLAG  fsLocalLogon;                 /* local logons only                   */
  ARGFLAG  fsLocalLogonHPFS386;          /* local logons to HPFS386 only        */
  PSZ      pszRemoteName;                /* remote server name                  */

} OPTIONS, *POPTIONS;


typedef struct
{
  USHORT usRemoteType;
} GLOBALS, *PGLOBALS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */
GLOBALS Globals;


ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung----------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",
    NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",
    NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/V",         "Verbose mode.",
    NULL,                 ARG_NULL,       &Options.fsVerbose},
  {"/N:",        "Remote node name",      &Options.pszRemoteName,
                          ARG_PSZ,        &Options.fsRemoteNode},
  {"/D:",        "Remote domain name",    &Options.pszRemoteName,
                          ARG_PSZ,        &Options.fsRemoteDomain},
  {"/L",         "Local logon only",      NULL,
                          ARG_NULL,       &Options.fsLocalLogon},
  {"/HPFS386",   "Local HPFS386 logon only", NULL,
                          ARG_NULL,       &Options.fsLocalLogonHPFS386},
  ARG_TERMINATE
};


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
  TOOLVERSION("Whoami",                                 /* application name */
              0x00010000,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


// Note:
// UPM32's error codes are preceeded by 0xffff0000, yet the documented
// error codes all have only significant lower bits.
#define MAP_UPM_ERROR( a ) a &= 0x0000ffff


/***********************************************************************
 * Name      : void UpmDisplayError
 * Funktion  : display the UPM error message
 * Parameter : LSINT lsrc
 *             PSZ   pszLocation
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-04-23]
 ***********************************************************************/

void UpmDisplayError(LSINT lsrc,
                     PSZ   pszLocation)
{
  USHORT idError = (lsrc & 0x0000ffff);
  PSZ    pszMsg;
  
  switch ( idError )
  {
    default:
      pszMsg = "(unknown)";
      break;
    
    case UPM_OK:                 
      pszMsg = "OK"; 
      break;
      
    case UPM_LOG_INPROC:         
      pszMsg = "Another logon is in process"; 
      break;
      
    case UPM_BAD_TYPE:           
      pszMsg = "Bad remotetype"; 
      break;
      
    case UPM_NOMEM:              
      pszMsg = "Cannot allocate required memory"; 
      break;
      
    case UPM_LOG_FILE_NOT_FOUND: 
      pszMsg = "An execute file could not be found"; 
      break;
      
    case UPM_FAIL_SECURITY:      
      pszMsg = "User not logged, failed security clearance"; 
      break;
      
    case UPM_BAD_PARAMETER:      
      pszMsg = "A parameter passed was invalid"; 
      break;
      
    case UPM_BAD_AUTHCHECK:      
      pszMsg = "Authcheck was not valid"; 
      break;
      
    case UPM_LOG_CANCEL:         
      pszMsg = "User has canceled from the logon panel"; 
      break;
      
    case UPM_NOT_LOGGED:         
      pszMsg = "A logon has not occured for this userid"; 
      break;
      
    case UPM_LOGGED:             
      pszMsg = "A domain logon is currently active"; 
      break;
      
    case UPM_SYS_ERROR:          
      pszMsg = "An unexpected system error"; 
      break;
      
    case UPM_OPEN_SESSIONS:      
      pszMsg = "The domain logoff failed, The domain logon has active sessions"; 
      break;
      
    case UPM_ULP_LOADED:         
      pszMsg = "The local logon failed, a local logon with a user logon profile is active"; 
      break;
      
    case UPM_LOGGED_ELSEWHERE:   
      pszMsg = "The domain logon failed, the userid is already logged on the domain"; 
      break;
      
    case UPM_PASSWORD_EXP:       
      pszMsg = "The users password is expired"; 
      break;
      
    case UPM_UNAVAIL:            
      pszMsg = "The logon failed, The remote node or server could not be contacted to process the logon request"; 
      break;
      
    case UPM_ACTIVE:             
      pszMsg = "The domain logon or logoff failed, a domain logon, logoff or net command is in process"; 
      break;
      
    case UPM_SS_PWDEXPWARNING:   
      pszMsg = "The local logon succeeded. The users password is expired"; 
      break;
      
    case UPM_SS_BUSY:            
      pszMsg = "The local logon failed. The Local security was busy"; 
      break;
      
    case UPM_SS_DEAD:            
      pszMsg = "The local logon failed. Local security has terminated unexpectedly"; 
      break;
      
    case UPM_ERROR_MORE_DATA:    
      pszMsg = "More data is available, the buffer provided is not large enough"; 
      break;
      
    case UPM_MAX_ENT_EXCEEDED:   
      pszMsg = "Update failed, the input buffer contains more than 16 entries"; 
      break;
      
    case UPM_DUP_ULP_ENTRY:      
      pszMsg = "Two or more entries with the same remote name and user ID were detected"; 
      break;
      
    case UPM_MAX_ULP_EXCEEDED:   
      pszMsg = "Database contains maximum number entries"; 
      break;
      
    case UPM_NODISK:             
      pszMsg = "Insufficient disk space was available to process this request"; 
      break;
      
    case UPM_PROF_NOT_FOUND:     
      pszMsg = "Did not find user logon profile for user ID"; 
      break;
      
    case UPM_ERROR_NONVAL_LOGON: 
      pszMsg = "Non validated Lan Server logons are not allowed with server service started"; 
      break;
  }
  
  printf("\nERROR %xh: %s, %s",
         lsrc,
         pszMsg,
         pszLocation);
}



/***********************************************************************
 * Name      : void UpmDisplayError
 * Funktion  : display the UPM error message
 * Parameter : LSINT lsrc
 *             PSZ   pszLocation
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-04-23]
 ***********************************************************************/

PSZ UpmGetRemoteTypeDescription( ULONG remotetype )
{
  switch ( remotetype )
  {
    case UPM_LOCAL: return "local";
    case UPM_DNODE: return "node";
    case UPM_DOMAIN: return "domain";
    case UPM_LOCAL_HPFS: return "HPFS386";
    case UPM_ALL: return "(all)";
    default: return "(unknown)";
  }
}


/***********************************************************************
 * Name      : APIRET whoami
 * Funktion  : process the whoami command
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-04-23]
 ***********************************************************************/

APIRET whoami(void)
{
  APIRET rc;
  USHORT lsrc;
  ULONG  ulLocalAuthority;        /* HPFS386 local security identifier */
  
#if 0
  // obtain local user identifier for HPFS386
  CHAR szUserID[ 260 ];
  
  lsrc = U32ELOCU((unsigned char*)szUserID,
                  &ulLocalAuthority);
  MAP_UPM_ERROR( lsrc );
  if (UPM_NOT_LOGGED == lsrc)
  {
    printf("(not locally logged on)");
    return TRUE;
  }
  
  // error ?
  if (UPM_OK != lsrc)
  {
    UpmDisplayError(lsrc,
                    "querying local logon information");
    return FALSE;
  }
  
  // display the gathered information
  printf(szUserID);
#endif
  
  CHAR  Buffer[ 16384 ];
  PVOID pBuffer            = Buffer;
  ULONG ulBufferLength     = sizeof( Buffer );
  ULONG ulEntriesReturned  = 0;
  ULONG ulEntriesAvailable = 0;
  
  lsrc = U32EUSRL((unsigned char*)Options.pszRemoteName,
                  Globals.usRemoteType,
                  (unsigned char*)pBuffer,
                  ulBufferLength,
                  &ulEntriesReturned,
                  &ulEntriesAvailable);
  MAP_UPM_ERROR( lsrc );
  if (UPM_OK != lsrc)
  {
    UpmDisplayError(lsrc,
                    "querying local logon information");
    return FALSE;
  }
  
  if (Options.fsVerbose)
    printf("%d logon entries returned, %d available\n",
           ulEntriesReturned,
           ulEntriesAvailable);
  
  // display all entries
  struct UPM_USER_LOGON* pUser = (struct UPM_USER_LOGON*)pBuffer;
  for (int i = 0;
       i < ulEntriesReturned;
       i++)
  {
    if (Options.fsVerbose)
      printf("%-20s %-12s (%-20s)   session %d\n",
             pUser->userid,
             UpmGetRemoteTypeDescription( pUser->remotetype ),
             pUser->remotename,
             pUser->sessionid);
    else
    {
      // check if a valid remotename is available
      if ( (NULL != pUser->remotename) &&
           (pUser->remotename[0] != 0) )
      {
        printf("%s/%s ",
               pUser->remotename,
               pUser->userid);
      }
      else
      {
        // in this case we just print the username
        printf(pUser->userid);
      }
    }
      
           
    // advance to next user
    pUser++;
  }
  
  return NO_ERROR;
}


/***********************************************************************
 * Name      : int main
 * Funktion  : main routine
 * Parameter : int argc, char **argv, char **envp
 * Variablen :
 * Ergebnis  : returncode to the operating system
 * Bemerkung :
 *
 * Autor     : Patrick Haller
 ***********************************************************************/

int main(int argc,
         char *argv[],
         char *envp[])
{
  APIRET       rc;
  
  // reset global variables
  memset(&Globals,
         0,
         sizeof( Globals ) );
  
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
  
  
  // check remotetype
  if (Options.fsRemoteDomain)
    Globals.usRemoteType = UPM_DOMAIN;
  else
    if (Options.fsRemoteNode)
      Globals.usRemoteType = UPM_DNODE;
    else
      if (Options.fsLocalLogon)
        Globals.usRemoteType = UPM_LOCAL;
      else
        if (Options.fsLocalLogonHPFS386)
          Globals.usRemoteType = UPM_LOCAL_HPFS;
        else
          Globals.usRemoteType = UPM_ALL;
  

  // determine currently logged on user
  // (MUGLIB API, LAN API)
  whoami();
  
  return NO_ERROR;
}
