/*****************************************************
 * UserDel Tool                                      *
 * Adds a user to the domain (LAN).                  *
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

  #define PURE_32

#ifdef __BORLANDC__
  #define _System __syscall
#endif

  #include <netcons.h>                               /* Lan Server Constants */
  #include <access.h>                                   /* Lan Server Access */
  #include <neterr.h>                              /* Lan Server Error Codes */
  #include <process.h>

  #define NETRET API_RET_TYPE
#endif

#ifdef _WIN32
  #define _UNICODE 1
  #define UNICODE 1
  #include <windows.h>
  #include <TCHAR.H>
  #include <lm.h>
  #include <lmaccess.h>
  #include <winnt.h>
  #define NETRET NET_API_STATUS
#endif

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "tooltypes.h"
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
  ARGFLAG fsNoPrompt;                 /* delete the account without prompt   */

  ARGFLAG fsServer;                   /* the server for the account          */
  ARGFLAG fsUser;                     /* username was specified              */

  PSZ     pszServer;          /* name of the server machine for this account */
  PSZ     pszUser;                                           /* the username */
  } OPTIONS, *POPTIONS;


typedef struct _Globals
{
#ifdef _WIN32
  TCHAR   lptstrServer[512];  /* name of the server machine for this account */
  TCHAR   lptstrUser[512];                                   /* the username */
#endif

#ifdef __OS2__
  UCHAR ucDummy; /* prevents a zero structure */
#endif
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
  {"/Y",            "Delete the account without confirmation.",
                                        NULL,                      ARG_NULL,       &Options.fsNoPrompt},
  {"/SERVER=",      "The server for this account.",
                                        &Options.pszServer,        ARG_PSZ,        &Options.fsServer},

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
  TOOLVERSION("UserDel",                                /* application name */
              0x00010000,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


/***********************************************************************
 * Name      : PSZ UserParameterError
 * Funktion  : Returns string to name of erroneous parameter
 * Parameter : ULONG ulError
 * Variablen :
 * Ergebnis  :
 * Bemerkung : Descriptive for NetUserDel errors
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 ***********************************************************************/

PSZ UserParameterError (ULONG ulError)
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

  if (ulError > 28)
    return ("<out of range>");
  else
    return (pszParameterTable[ulError]);
}


/***********************************************************************
 * Name      : void UserDelError
 * Funktion  : Provides information about error in NetUserDel
 * Parameter : NETRET rc, ULONG ulError
 * Variablen :
 * Ergebnis  :
 * Bemerkung : Descriptive for NetUserDel errors
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 ***********************************************************************/

void UserDelError (NETRET rc,
		   ULONG  ulError)
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

  if (ulError != 0)
	  printf ("\nParameter: %s",
	          UserParameterError (ulError));
}



/***********************************************************************
 * Name      : APIRET UserDel
 * Funktion  : Add the user
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 ***********************************************************************/

APIRET UserDel (void)
{
  NETRET rc;                                           /* NET_API returncode */

#define ASC2UNI(a,b) OemToChar(Options.a,Globals.b)

  /**************************************************************************
   * convert ASCII to UNICODE strings                                       *
   **************************************************************************/

#ifdef _WIN32
  if (Options.fsUser)          ASC2UNI(pszUser,         lptstrUser);
  if (Options.fsServer)        ASC2UNI(pszServer,       lptstrServer);
#endif

  printf ("\nDeleting account %s from %s.",
	      Options.pszUser,
		  Options.fsServer ? Options.pszServer : "<local>");

  /* @@@confirmation !! */
  if (!Options.fsNoPrompt)
  {
	int iAnswer;                                         /* answer from user */

	iAnswer = ToolsConfirmationQuery();               /* ask the user yes/no */
	if (iAnswer != 1)                                   /* answer is not YES */
	  return (0);                      /* then abort the process immediately */
  }

#ifdef _WIN32
  rc = NetUserDel(Globals.lptstrServer,
                  Globals.lptstrUser);
#endif

#ifdef __OS2__
  rc = Net32UserDel(Options.pszServer,
                    Options.pszUser,
                    NULL);
#endif
  if (rc != NERR_Success)
  {
	UserDelError (rc,
		          0);
  }

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

  rc = UserDel();                                /* this is our main routine */
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  return (rc);
}
