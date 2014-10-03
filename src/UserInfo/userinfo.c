/*****************************************************
 * UserInfo Tool                                     *
 * Dump all information about an user account.       *
 * (c) 1997    Patrick Haller Systemtechnik          *
 *****************************************************/

/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#ifdef __OS2__
  #define INCL_DOSFILEMGR
  #define INCL_BASE
  #define INCL_NOPMAPI
  #include <os2.h>
#endif

#ifdef _WIN32
  #define _UNICODE 1
  #define UNICODE 1
  #include <windows.h> 
  #include <TCHAR.H>
  #include <lm.h> 
  #include <lmaccess.h>
  #include <winnt.h>
#endif

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include "tools.h"
#include "toolarg.h"

#ifndef MAXPATHLEN
  #define MAXPATHLEN 260
#endif


/*****************************************************************************
 * Defines                                                                   *
 *****************************************************************************/

/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsVerbose;                  /* more verbosely output is desired    */
  ARGFLAG fsServer;    /* the server from which the information is requested */
  ARGFLAG fsUser;                         /* the account name that is dumped */
  ARGFLAG fsLevel;                                   /* level of information */
  
  PSZ     pszServer;                                      /* the server name */
  PSZ     pszUser;                                  /* the user/account name */
  UCHAR   ucLevel;                          /* the information request level */
} OPTIONS, *POPTIONS;


typedef struct _Globals
{
  USER_INFO_3 *puiUserInfo;  /* buffer is the complete USER_INFO_3 structure */

  TCHAR   lptstrServer[512];  /* name of the server machine for this account */
  TCHAR   lptstrUser[512];                                   /* the username */

  CHAR    szUser[256];     /* buffers for converted information from LAN MAN */
  CHAR    szHomeDirectory[256];
  CHAR    szScriptPath[256];
  CHAR    szComment[256];

  CHAR    szUserFullName[256];
  CHAR    szUserComment[256];
  CHAR    szParameters[256];
  CHAR    szWorkstations[256];
  CHAR    szLogonServer[256];

  CHAR    szProfile[256];
  CHAR    szHomeDrive[256];
} GLOBALS, *PGLOBALS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */
GLOBALS Globals;

ARGUMENT TabArguments[] =
{ /*Token-----------Beschreibung--------pTarget--------------------ucTargetFormat--pTargetSpecified--*/
  {"/?",            "Get help screen.", NULL,                      ARG_NULL,       &Options.fsHelp},
  {"/H",            "Get help screen.", NULL,                      ARG_NULL,       &Options.fsHelp},
  {"/SERVER=",      "The server for this account.",
                                        &Options.pszServer,        ARG_PSZ,        &Options.fsServer},
  {"/LEVEL=",       "Level of information that is requested (1..3)",
                                        &Options.ucLevel,          ARG_UCHAR,      &Options.fsLevel},
  {"1",           "Name of user.",      &Options.pszUser,     ARG_PSZ     |
                                                              ARG_DEFAULT |
                                                              ARG_MUST,            &Options.fsUser},
  ARG_TERMINATE
};



/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void   help                  (void);

APIRET initialize            (void);

int    main                  (int,
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
  TOOLVERSION("UserInfo",                               /* application name */
              0x00010001,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


/***********************************************************************
 * Name      : PSZ UserParameterError
 * Funktion  : Returns string to name of erroneous parameter
 * Parameter : DWORD dwError
 * Variablen :
 * Ergebnis  :
 * Bemerkung : Descriptive for NetUserAdd errors
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 ***********************************************************************/

PSZ UserParameterError (DWORD dwError)
{
  static PSZ pszParameterTable[] =
  {/* 00 */ "user name",
   /* 01 */ "password",
   /* 02 */ "password age",
   /* 03 */ "privilege",
   /* 04 */ "home directory",
   /* 05 */ "comment",
   /* 06 */ "flags (type and status)",
   /* 07 */ "script path",

   /* 08 */ "authentication flags",
   /* 09 */ "full user name",
   /* 10 */ "user comment",
   /* 11 */ "parameters",
   /* 12 */ "logon workstations",
   /* 13 */ "last logon",
   /* 14 */ "last logoff",
   /* 15 */ "account expiry date",
   /* 16 */ "maximum storage",
   /* 17 */ "units per week",
   /* 18 */ "logon hours",
   /* 19 */ "bad password counter",
   /* 20 */ "number of logons",
   /* 21 */ "logon server",
   /* 22 */ "country code",
   /* 23 */ "code page",

   /* 24 */ "user id",
   /* 25 */ "primary group id",
   /* 26 */ "profile",
   /* 27 */ "home drive",
   /* 28 */ "password expiry state"};

  if (dwError > 28)
    return ("<out of range>");
  else
	return (pszParameterTable[dwError]);
}


/***********************************************************************
 * Name      : void UserAddError
 * Funktion  : Provides information about error in NetUserAdd
 * Parameter : NET_API_STATUS rc, DWORD dwError
 * Variablen :
 * Ergebnis  :
 * Bemerkung : Descriptive for NetUserAdd errors
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 ***********************************************************************/

void UserAddError (NET_API_STATUS rc,
				   DWORD          dwError)
{
  switch (rc)
  {
    case NERR_Success:        
		printf ("\nOK."); 
		break;

    case ERROR_ACCESS_DENIED: 
		printf ("\nError: Access Denied. You should be admin.");
		break;

	case NERR_InvalidComputer:
		printf ("\nError: Invalid Computer Name");
		break;

	case NERR_NotPrimary:
		printf ("\nError: The operation is allowed only on the "
			    "\n       primary domain controller of the domain.");
		break;

	case NERR_GroupExists:
		printf ("\nError: The group already exists.");
		break;

	case NERR_UserExists:
		printf ("\nError: The user account already exists.");
		break;

	case NERR_PasswordTooShort:
		printf ("\nError: The password is shorter than required.");
		break;

	default:
		printf ("\nError: <unknown>");
		break;
  }

  if (dwError != 0)
	  printf ("\nParameter: %s",
	          UserParameterError (dwError));
}


/***********************************************************************
 * Name      : APIRET UserPrintInformation
 * Funktion  : Print information about the current user
 * Parameter : UCHAR ucLevel
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 ***********************************************************************/

void UserPrintInformation (UCHAR ucLevel)
{
  time_t timeLastLogon;                  /* for the UTC to string conversion */
  time_t timeLastLogoff;
  time_t timeExpiry;
  char   szLastLogon[64];             /* for the time_t to string conversion */
  char   szLastLogoff[64];            /* for the time_t to string conversion */
  char   szExpiry[64];                /* for the time_t to string conversion */


  /***************************************************************************
   * Level 1 information                                                     *
   ***************************************************************************/

  printf ("\nAccount Information:"
 		  "\n  user is                  [%s]"		  
		  "\n  account server is        [%s]"
		  "\n  home directory is        [%s]"
		  "\n  script path is           [%s]"
		  "\n  comment                  [%s]",		  
		  Globals.szUser,
		  Options.pszServer,
		  Globals.szHomeDirectory,
		  Globals.szScriptPath,
		  Globals.szComment);

  printf ("\n  password is              %u days, %u hours, %u minutes old.",
	      Globals.puiUserInfo->usri3_password_age / 86400,
		  Globals.puiUserInfo->usri3_password_age / 3600 % 24,
		  Globals.puiUserInfo->usri3_password_age / 60 % 60);
  
  printf ("\n  privilege is             [");
  switch (Globals.puiUserInfo->usri3_priv)
  {
    case USER_PRIV_GUEST: printf ("GUEST]");         break;
    case USER_PRIV_USER:  printf ("USER]");          break;
    case USER_PRIV_ADMIN: printf ("ADMINISTRATOR]"); break;
    default:              printf ("<unknown>]");     break;
  }

  printf ("\n\n  account status flags are ");
  if (Globals.puiUserInfo->usri3_flags & UF_SCRIPT) 
	printf ("\n    The logon script executed. (Required for NT and LANMAN 2.0)");

  if (Globals.puiUserInfo->usri3_flags & UF_ACCOUNTDISABLE) 
	printf ("\n    The account is disabled.");
  else
	printf ("\n    The account is NOT disabled.");

  if (Globals.puiUserInfo->usri3_flags & UF_HOMEDIR_REQUIRED) 
	printf ("\n    The home directory is required.");
  
  if (Globals.puiUserInfo->usri3_flags & UF_PASSWD_NOTREQD) 
	printf ("\n    A password is NOT required.");
  else
	printf ("\n    A password is required.");

  if (Globals.puiUserInfo->usri3_flags & UF_PASSWD_CANT_CHANGE) 
	printf ("\n    The password cannot be changed by the user.");
  else
	printf ("\n    The password can be changed by the user.");

  if (Globals.puiUserInfo->usri3_flags & UF_LOCKOUT) 
	printf ("\n    The account is currently locked out.");
  else
	printf ("\n    The account is not locked out.");
  
  if (Globals.puiUserInfo->usri3_flags & UF_DONT_EXPIRE_PASSWD) 
	printf ("\n    The password will not expire.");
  else
	printf ("\n    The password will expire.");


  printf ("\n  account type is [");
  switch (Globals.puiUserInfo->usri3_flags & UF_ACCOUNT_TYPE_MASK)
  {
    case UF_NORMAL_ACCOUNT:            printf ("NORMAL]");              break;
    case UF_TEMP_DUPLICATE_ACCOUNT:    printf ("TEMPORARY DUPLICATE]"); break;
	case UF_WORKSTATION_TRUST_ACCOUNT: printf ("WORKSTATION]");         break;
    case UF_SERVER_TRUST_ACCOUNT:      printf ("SERVER]");              break;
    case UF_INTERDOMAIN_TRUST_ACCOUNT: printf ("DOMAIN TRUST]");        break;
	default:                           printf ("<unknown>]");           break;
  }

  printf ("\n");


  /***************************************************************************
   * Level 2 information                                                     *
   ***************************************************************************/

  if (ucLevel > 1)
  {
    if (Globals.puiUserInfo->usri3_auth_flags)
	{
      printf ("\n  authentication flags are");
	  if (Globals.puiUserInfo->usri3_auth_flags & AF_OP_PRINT)
  	    printf ("\n    The print operator privilege is enabled.");
  
      if (Globals.puiUserInfo->usri3_auth_flags & AF_OP_COMM)
  	    printf ("\n    The communications operator privilege is enabled. (IBM OS/2 LAN Server)");
  
      if (Globals.puiUserInfo->usri3_auth_flags & AF_OP_SERVER)
  	    printf ("\n    The server operator privilege is enabled.");
  
      if (Globals.puiUserInfo->usri3_auth_flags & AF_OP_ACCOUNTS)
	    printf ("\n    The accounts operator privilege is enabled.");	  

	  printf ("\n");
	}

    printf ("\n  full user name is        [%s]"
		    "\n  user comment is          [%s]"
			"\n  parameters are           [%s]"
	        "\n  logon workstations       [%s]"
			"\n  logon server             [%s]",
			Globals.szUserFullName,
			Globals.szUserComment,
			Globals.szParameters,
			Globals.szWorkstations,
			Globals.szLogonServer);

	_tzset();                                        /* set current timezone */
	timeLastLogon  = Globals.puiUserInfo->usri3_last_logon;
	timeLastLogoff = Globals.puiUserInfo->usri3_last_logoff;
	timeExpiry     = Globals.puiUserInfo->usri3_acct_expires;

    if (timeLastLogon != 0)
    {
      localtime(&timeLastLogon);
      strcpy (szLastLogon,  ctime(&timeLastLogon));
    }

    if (timeLastLogoff != 0)
    {
      localtime(&timeLastLogoff);      
      strcpy (szLastLogoff, ctime(&timeLastLogoff));
    }

    if (timeExpiry != TIMEQ_FOREVER)
    {
      localtime(&timeExpiry);      
      strcpy (szExpiry,     ctime(&timeExpiry));
    }

    printf ("\n  Effective timezone is    %s %i %s %i",
            _tzname[0],
            _timezone / 3600,
            _tzname[1],
            _daylight);
			  
    printf ("\n  Last Logon at            %s"
		      "  Last Logoff at           %s"
              "  Account expiry at        %s",
              timeLastLogon  ? szLastLogon  : "<unknown>\n",
              timeLastLogoff ? szLastLogoff : "<unknown>\n",
			  timeExpiry != TIMEQ_FOREVER ? szExpiry     : "<never>\n");
		
	if (Globals.puiUserInfo->usri3_max_storage == USER_MAXSTORAGE_UNLIMITED)
      printf ("\n  Maximum storage limit is [unlimited]");
	else
	  printf ("\n  Maximum storage limit is [%9u]",
        Globals.puiUserInfo->usri3_max_storage);
			
	printf ("\n  Bad password counter is  [%9u]"
			"\n  Number of logons is      [%9u]",
		    Globals.puiUserInfo->usri3_bad_pw_count,
			Globals.puiUserInfo->usri3_num_logons);

    printf ("\n  User's country code is   [%9u]"
		    "\n  User's language (chcp)   [%9u]",
		    Globals.puiUserInfo->usri3_country_code,
			Globals.puiUserInfo->usri3_code_page);
		
	printf ("\n");
  }


  /***************************************************************************
   * Level 3 information                                                     *
   ***************************************************************************/

  if (ucLevel > 2) 
	  printf ("\n  User ID is               [%9u]"
		      "\n  Primary Group ID is      [%9u]"
			  "\n  Profile is               [%s]"
			  "\n  Home drive is            [%s]"
			  "\n  Password expired         %s",
			  Globals.puiUserInfo->usri3_user_id,
			  Globals.puiUserInfo->usri3_primary_group_id,
			  Globals.szProfile,
			  Globals.szHomeDrive,
			  Globals.puiUserInfo->usri3_password_expired ? "YES" : "NO");
}


/***********************************************************************
 * Name      : APIRET UserAdd
 * Funktion  : Add the user
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 ***********************************************************************/

APIRET UserInfo (void)
{
  NET_API_STATUS rc;                                   /* NET_API returncode */


#define ASC2UNI(a,b) OemToChar(Options.a,Globals.b)
#define UNI2ASC(a,b) if (Globals.puiUserInfo->a != NULL)           \
                       CharToOem(Globals.puiUserInfo->a,           \
                                 Globals.b);                       \
					 else                                          \
                       Globals.b[0] = 0;


  /**************************************************************************
   * convert ASCII to UNICODE strings                                       *
   **************************************************************************/

  if (Options.fsUser)     ASC2UNI(pszUser,     lptstrUser);
  if (Options.fsServer)   ASC2UNI(pszServer,   lptstrServer);

  
  /**************************************************************************
   * call the Lan Manager APIs                                              *
   **************************************************************************/

  rc = NetUserGetInfo(Globals.lptstrServer,
	                  Globals.lptstrUser,
	                  (DWORD)Options.ucLevel,
	                  (PUCHAR*)&Globals.puiUserInfo);				      
  if (rc != NERR_Success)  
	UserAddError (rc,0);
	  
  else
  {
    /* change the UniCODE Strings back to the OEM characterset */
    UNI2ASC(usri3_name,       szUser);
	UNI2ASC(usri3_home_dir,   szHomeDirectory);
	UNI2ASC(usri3_comment,    szComment);
    UNI2ASC(usri3_script_path,szScriptPath);

	if (Options.ucLevel > 1)
	{
	  UNI2ASC(usri3_full_name,   szUserFullName);
	  UNI2ASC(usri3_usr_comment, szUserComment);
	  UNI2ASC(usri3_parms,       szParameters);
	  UNI2ASC(usri3_workstations,szWorkstations);
	  UNI2ASC(usri3_logon_server,szLogonServer);
	}

	if (Options.ucLevel > 2)
	{
	  UNI2ASC(usri3_profile,       szProfile);
      UNI2ASC(usri3_home_dir_drive,szHomeDrive);
	}  

	/* print the information */
	UserPrintInformation(Options.ucLevel);
  }

  NetApiBufferFree(Globals.puiUserInfo);                  /* free the buffer */
  
  return (rc); 
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

APIRET initialize (void)
{
  memset(&Options,
         0L,
         sizeof(Options));
  
  memset(&Globals,
         0L,
         sizeof(Globals));
  
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
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 ***********************************************************************/

int main (int argc, char *argv[])
{
  int rc;                                                    /* RÅckgabewert */

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

  if (!Options.fsLevel)                   /* no level explicitly specified ? */
    Options.ucLevel = 3;                                      /* the default */
  else
	if ( (Options.ucLevel < 1) ||                       /* are we in range ? */
	     (Options.ucLevel > 3) )
    {
	  fprintf (stderr,
		       "\nError: /LEVEL=<nr> must be in within 1 to 3.");
      exit(ERROR_INVALID_PARAMETER);                     /* exit immediately */
    }

  rc = UserInfo();                               /* this is our main routine */
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);
  
  return (rc);
}
