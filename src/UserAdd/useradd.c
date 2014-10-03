/*****************************************************
 * UserAdd Tool                                      *
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
  ARGFLAG fsVerbose;                  /* more verbosely output is desired    */
  ARGFLAG fsOverwrite;                /* overwrite existing user account     */
  ARGFLAG fsNever;                    /* never overwrite existing account    */

  ARGFLAG fsServer;                   /* the server for the account          */
  ARGFLAG fsUser;                     /* username was specified              */
  ARGFLAG fsPassword;                 /* password was specified              */
  ARGFLAG fsHomeDirectory;            /* user's homedirectory was specified  */
  ARGFLAG fsScriptPath;               /* logon script path                   */
  ARGFLAG fsComment;                  /* an account comment was specified    */

  ARGFLAG fsUserPrivGuest;                                /* user privileges */
  ARGFLAG fsUserPrivUser;
  ARGFLAG fsUserPrivAdmin;
  ARGFLAG fsTypeNormal;                                      /* account type */
  ARGFLAG fsTypeDupe;
  ARGFLAG fsTypeWksta;
  ARGFLAG fsTypeServer;
  ARGFLAG fsTypeDomain;
  ARGFLAG fsAccDisabled;                                   /* account status */
  ARGFLAG fsAccNoPasswd;
  ARGFLAG fsAccNoChange;
  ARGFLAG fsAccLockout;
  ARGFLAG fsAccNoExpire;

                                                            /* LEVEL 2 flags */
  ARGFLAG fsUserFullName;                 /* the full username was specified */
  ARGFLAG fsUserComment;                          /* additional user comment */
  ARGFLAG fsWorkstations;            /* logon limited fro these workstations */
  ARGFLAG fsMaxStorage;                          /* maximum storage for user */
  ARGFLAG fsProfilePath;                                     /* profile path */
  ARGFLAG fsHomeDrive;                     /* drive to map home directory to */
  ARGFLAG fsPasswdChange;         /* password change required for next logon */

  PSZ     pszServer;          /* name of the server machine for this account */
  PSZ     pszUser;                                           /* the username */
  PSZ     pszPassword;                                       /* the password */
  PSZ     pszHomeDirectory;                            /* the home directory */
  PSZ     pszScriptPath;                                /* logon script path */
  PSZ     pszComment;                                     /* account comment */

                                                             /* LEVEL 2 vars */
  PSZ     pszUserFullName;                                 /* full user name */
  PSZ     pszUserComment;                         /* additional user comment */
  PSZ     pszWorkstations;          /* logon limited from these workstations */

                                                             /* LEVEL 3 vars */
  PSZ     pszProfilePath;                                /* the profile path */
  PSZ     pszHomeDrive;                /* drive to map the home directory to */
  ULONG   ulMaxStorage;                      /* maximum storage for the user */
} OPTIONS, *POPTIONS;


typedef struct _Globals
{
  UCHAR ucStructureLevel;                      /* 1 - USER_INFO_1 sufficient */
                                               /* 2 - USER_INFO_2 sufficient */
                                               /* 3 - USER_INFO_3 sufficient */
  USER_INFO_3 uiUserInfo;    /* buffer is the complete USER_INFO_3 structure */

  TCHAR   lptstrServer[512];  /* name of the server machine for this account */
  TCHAR   lptstrUser[512];                                   /* the username */
  TCHAR   lptstrPassword[512];                               /* the password */
  TCHAR   lptstrHomeDirectory[512];                    /* the home directory */
  TCHAR   lptstrScriptPath[512];                        /* logon script path */
  TCHAR   lptstrComment[512];                             /* account comment */

                                                             /* LEVEL 2 vars */
  TCHAR   lptstrUserFullName[512];                         /* full user name */
  TCHAR   lptstrUserComment[512];                 /* additional user comment */
  TCHAR   lptstrWorkstations[512];  /* logon limited from these workstations */

                                                             /* LEVEL 3 vars */
  TCHAR   lptstrProfilePath[512];                        /* the profile path */
  TCHAR   lptstrHomeDrive[512];        /* drive to map the home directory to */
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
  {"/V",            "Verbose output.",  NULL,                      ARG_NULL,       &Options.fsVerbose},
  {"/ALWAYS",       "Always update account with new information automatically.",
                                        NULL,                      ARG_NULL,       &Options.fsOverwrite},
  {"/NEVER",        "Never update account with new information.",
                                        NULL,                      ARG_NULL,       &Options.fsNever},

  {"/SERVER=",      "The server for this account.",
                                        &Options.pszServer,        ARG_PSZ,        &Options.fsServer},
  {"/PASSWORD=",    "Account password.",&Options.pszPassword,      ARG_PSZ,        &Options.fsPassword},
  {"/HOME=",        "Home directory.",  &Options.pszHomeDirectory, ARG_PSZ,        &Options.fsHomeDirectory},
  {"/SCRIPT=",      "Logon script path",&Options.pszScriptPath,    ARG_PSZ,        &Options.fsScriptPath},
  {"/COMMENT=",     "Comment.",         &Options.pszComment,       ARG_PSZ,        &Options.fsComment},
  {"/PRIV.GUEST",   "User has GUEST privilege (default).",
                                        NULL,                      ARG_NULL,       &Options.fsUserPrivGuest},
  {"/PRIV.USER",    "User has USER privilege.",
                                        NULL,                      ARG_NULL,       &Options.fsUserPrivUser},
  {"/PRIV.ADMIN",   "User has ADMINISTRATOR privilege.",
                                        NULL,                      ARG_NULL,       &Options.fsUserPrivAdmin},
  {"/TYPE.NORMAL",  "This account is NORMAL type (default).",
                                        NULL,                      ARG_NULL,       &Options.fsTypeNormal},
  {"/TYPE.TEMP",    "This account is TEMPORARY DUPLICATE type.",
                                        NULL,                      ARG_NULL,       &Options.fsTypeDupe},
  {"/TYPE.WORKSTATION",  "This account is WORKSTATION TRUST type.",
                                        NULL,                      ARG_NULL,       &Options.fsTypeWksta},
  {"/TYPE.SERVER",  "This account is SERVER TRUST type.",
                                        NULL,                      ARG_NULL,       &Options.fsTypeServer},
  {"/TYPE.DOMAIN",  "This account is DOMAIN TRUST type.",
                                        NULL,                      ARG_NULL,       &Options.fsTypeDomain},
  {"/ACC.DISABLED", "This account is DISABLED.",
                                        NULL,                      ARG_NULL,       &Options.fsAccDisabled},
  {"/ACC.NOPASSWD", "This account does NOT REQUIRE PASSWORD.",
                                        NULL,                      ARG_NULL,       &Options.fsAccNoPasswd},
  {"/ACC.NOCHANGE", "This account does NOT ALLOW PASSWORD CHANGES.",
                                        NULL,                      ARG_NULL,       &Options.fsAccNoChange},
  {"/ACC.LOCKOUT",  "This account is currently LOCKED OUT.",
                                        NULL,                      ARG_NULL,       &Options.fsAccLockout},
  {"/ACC.NOEXPIRE", "This account is NEVER EXPIRES.",
                                        NULL,                      ARG_NULL,       &Options.fsAccNoExpire},

  {"/USERFULL=",    "Full username.",   &Options.pszUserFullName,  ARG_PSZ,        &Options.fsUserFullName},
  {"/USERCOMMENT=", "Additional user comment.",
                                        &Options.pszUserComment,   ARG_PSZ,        &Options.fsUserComment},
  {"/WORKSTATIONS=","Logon limited to these workstations.",
                                        &Options.pszWorkstations,  ARG_PSZ,        &Options.fsWorkstations},
  {"/STORAGE=",     "Maximum storage allowed for this user.",
                                        &Options.ulMaxStorage,     ARG_ULONG,      &Options.fsMaxStorage},

  {"/PROFILE=",     "Path for a (roaming) user profile.",
                                        &Options.pszProfilePath,   ARG_PSZ,        &Options.fsProfilePath},
  {"/HOMEDRIVE=",   "Drive to map the home directory to.",
                                        &Options.pszHomeDrive,     ARG_PSZ,        &Options.fsHomeDrive},
  {"/PASSWD.CHANGE","User is forced to change password at next logon.",
                                        NULL,                      ARG_NULL,       &Options.fsPasswdChange},

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
  TOOLVERSION("UserAdd",                                /* application name */
              0x00010002,                           /* application revision */
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
  printf ("\nAccount Information:"
                  "\n  user [%s] with password [%s]"
                  "\n  account server is [%s]"
                  "\n  home directory is [%s]"
                  "\n  script path is    [%s]"
                  "\n  comment           [%s]",
                  Options.pszUser,
                  Options.fsPassword      ? Options.pszPassword      : "<none>",
                  Options.fsServer        ? Options.pszServer        : "<local>",
                  Options.fsHomeDirectory ? Options.pszHomeDirectory : "<none>",
                  Options.fsScriptPath    ? Options.pszScriptPath    : "<none>",
                  Options.fsComment       ? Options.pszComment       : "<none>");

  printf ("\n  privilege level is [");
  switch (Globals.uiUserInfo.usri3_priv)
  {
    case USER_PRIV_USER:  printf ("USER]");          break;
    case USER_PRIV_GUEST: printf ("GUEST]");         break;
    case USER_PRIV_ADMIN: printf ("ADMINISTRATOR]"); break;
    default:              printf ("<unknown>]");     break;
  }

  printf ("\n  account type is    [");
  switch (Globals.uiUserInfo.usri3_flags & UF_ACCOUNT_TYPE_MASK)
  {
    case UF_NORMAL_ACCOUNT:            printf ("NORMAL]");              break;
    case UF_TEMP_DUPLICATE_ACCOUNT:    printf ("TEMPORARY DUPLICATE]"); break;
    case UF_WORKSTATION_TRUST_ACCOUNT: printf ("WORKSTATION]");         break;
    case UF_SERVER_TRUST_ACCOUNT:      printf ("SERVER]");              break;
    case UF_INTERDOMAIN_TRUST_ACCOUNT: printf ("DOMAIN TRUST]");        break;
    default:                           printf ("<unknown>]");           break;
  }

  printf ("\n  account status is  ");
  if (Options.fsAccDisabled) printf ("\n    DISABLED");
  if (Options.fsAccNoPasswd) printf ("\n    PASSWORD NOT REQUIRED");
  if (Options.fsAccNoChange) printf ("\n    PASSWORD CHANGE NOT ALLOWED");
  if (Options.fsAccLockout)  printf ("\n    LOCKED OUT");
  if (Options.fsAccNoExpire) printf ("\n    PASSWORD DOES NOT EXPIRE");


  if (ucLevel > 1)                              /* add level 2 information ? */
        printf ("\n  full username is       [%s]"
                "\n  user comment is        [%s]"
                    "\n  logon workstations are [%s]\n",
                        Options.fsUserFullName ? Options.pszUserFullName : "<none>",
                        Options.fsUserComment  ? Options.pszUserComment  : "<none>",
                        Options.fsWorkstations ? Options.pszWorkstations : "<unrestricted>");

  if (ucLevel > 2)                              /* add level 3 information ? */
  {
    printf ("\n  profile path is        [%s]"
                "\n  home directory drive   [%s]"
                        "\n  storage limits at      [%u]",
                        Options.fsProfilePath ? Options.pszProfilePath : "<none>",
                        Options.fsHomeDrive   ? Options.pszHomeDrive   : "<none>",
                        Options.fsMaxStorage  ? Options.ulMaxStorage   : 0);

    if (Options.fsPasswdChange) printf ("\n  password CHANGE REQUIRED");
    else                        printf ("\n  password CHANGE NOT REQUIRED");
  }
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

APIRET UserAdd (void)
{
  NET_API_STATUS rc;                                   /* NET_API returncode */
  DWORD          dwError;                   /* erroneous parameter indicator */

#define GUI Globals.uiUserInfo
#define ASC2UNI(a,b) OemToChar(Options.a,Globals.b)

     /* 1st perform parameter mapping and decide which structure is required */
  memset (&Globals.uiUserInfo,                     /* zero out the structure */
              0,
                  sizeof(Globals.uiUserInfo));

  /**************************************************************************
   * convert ASCII to UNICODE strings                                       *
   **************************************************************************/

  if (Options.fsUser)          ASC2UNI(pszUser,         lptstrUser);
  if (Options.fsPassword)      ASC2UNI(pszPassword,     lptstrPassword);
  if (Options.fsServer)        ASC2UNI(pszServer,       lptstrServer);
  if (Options.fsHomeDirectory) ASC2UNI(pszHomeDirectory,lptstrHomeDirectory);
  if (Options.fsScriptPath)    ASC2UNI(pszScriptPath,   lptstrScriptPath);
  if (Options.fsComment)       ASC2UNI(pszComment,      lptstrComment);
  if (Options.fsUserFullName)  ASC2UNI(pszUserFullName, lptstrUserFullName);
  if (Options.fsUserComment)   ASC2UNI(pszUserComment,  lptstrUserComment);
  if (Options.fsWorkstations)  ASC2UNI(pszWorkstations, lptstrWorkstations);
  if (Options.fsProfilePath)   ASC2UNI(pszProfilePath,  lptstrProfilePath);
  if (Options.fsHomeDrive)     ASC2UNI(pszHomeDrive,    lptstrHomeDrive);


  /**************************************************************************
   * USER_INFO_1 level                                                      *
   **************************************************************************/

  Globals.ucStructureLevel = 1;      /* 1st we start with the smallest level */

                                                               /* usri3_name */
  if (Options.fsUser)
    GUI.usri3_name         = Globals.lptstrUser;        /* this is mandatory */

  if (Options.fsPassword)
    GUI.usri3_password     = Globals.lptstrPassword;       /* usri3_password */
  GUI.usri3_password_age = 0;                  /* no need when creating user */

  /* default */                GUI.usri3_priv = USER_PRIV_USER;
  if (Options.fsUserPrivGuest) GUI.usri3_priv = USER_PRIV_GUEST;
  if (Options.fsUserPrivAdmin) GUI.usri3_priv = USER_PRIV_ADMIN;

  if (Options.fsHomeDirectory)
    GUI.usri3_home_dir    = Globals.lptstrHomeDirectory;   /* home directory */

  if (Options.fsComment)
    GUI.usri3_comment     = Globals.lptstrComment;         /* user's comment */

  GUI.usri3_flags       = UF_NORMAL_ACCOUNT;
  if (Options.fsTypeDupe)   GUI.usri3_flags = UF_TEMP_DUPLICATE_ACCOUNT;
  if (Options.fsTypeWksta)  GUI.usri3_flags = UF_WORKSTATION_TRUST_ACCOUNT;
  if (Options.fsTypeServer) GUI.usri3_flags = UF_SERVER_TRUST_ACCOUNT;
  if (Options.fsTypeDomain) GUI.usri3_flags = UF_INTERDOMAIN_TRUST_ACCOUNT;

                             GUI.usri3_flags |= UF_SCRIPT;     /* always set */
  if (Options.fsAccDisabled) GUI.usri3_flags |= UF_ACCOUNTDISABLE;
  if (Options.fsAccNoPasswd) GUI.usri3_flags |= UF_PASSWD_NOTREQD;
  if (Options.fsAccNoChange) GUI.usri3_flags |= UF_PASSWD_CANT_CHANGE;
  if (Options.fsAccLockout)  GUI.usri3_flags |= UF_LOCKOUT;
  if (Options.fsAccNoExpire) GUI.usri3_flags |= UF_DONT_EXPIRE_PASSWD;

  if (Options.fsScriptPath)
    GUI.usri3_script_path = Globals.lptstrScriptPath;        /* logon script */


  /**************************************************************************
   * USER_INFO_2 level                                                      *
   **************************************************************************/
  if (Options.fsUserFullName ||
          Options.fsUserComment  ||
          Options.fsWorkstations ||
          Options.fsMaxStorage)
  {
          Globals.ucStructureLevel = 2; /* now at least structure 2 is required */

          if (Options.fsUserFullName) GUI.usri3_full_name    = Globals.lptstrUserFullName;
      if (Options.fsUserComment)  GUI.usri3_usr_comment  = Globals.lptstrUserComment;
      if (Options.fsWorkstations) GUI.usri3_workstations = Globals.lptstrWorkstations;

          if (Options.fsMaxStorage)   GUI.usri3_max_storage  = Options.ulMaxStorage;
          else                        GUI.usri3_max_storage  = USER_MAXSTORAGE_UNLIMITED;
  }


  /**************************************************************************
   * USER_INFO_3 level                                                      *
   **************************************************************************/
  if (Options.fsProfilePath  ||
          Options.fsHomeDrive    ||
          Options.fsPasswdChange)
  {
          Globals.ucStructureLevel = 3; /* now at least structure 3 is required */

      if (Options.fsProfilePath)  GUI.usri3_profile      = Globals.lptstrProfilePath;
      if (Options.fsHomeDrive)    GUI.usri3_home_dir_drive=Globals.lptstrHomeDrive;

          if (Options.fsPasswdChange) GUI.usri3_password_expired = 0xFFFFFFFF; /* change forced */
          else                        GUI.usri3_password_expired = 0;

          GUI.usri3_primary_group_id = DOMAIN_GROUP_RID_USERS;              /* default */
  }

  /* verbose output ? */
  if (Options.fsVerbose)
        UserPrintInformation(Globals.ucStructureLevel);


  /* finally add the user */
  Globals.uiUserInfo.usri3_password_age = 1;
  Globals.uiUserInfo.usri3_acct_expires = TIMEQ_FOREVER; /* account never expires */


  rc = NetUserAdd(Globals.lptstrServer,
                      (DWORD)Globals.ucStructureLevel,
                      (LPBYTE)&Globals.uiUserInfo,
                                  &dwError);

  if (!Options.fsNever) /* if we can take action on overwriting this account */
  {
    if (rc == NERR_UserExists)                /* this account already exists */
    {
        int iAnswer;                                         /* answer from user */

          if (!Options.fsOverwrite)              /* if no overwriting is allowed */
          {
        fprintf (stderr,
                    "\nWarning: User account already exists. Overwrite ? ");
            iAnswer = ToolsConfirmationQuery();

            if ( iAnswer != 1)                                          /* yes ? */
          goto _useradderror_exit;                            /* then cancel */
          }

          rc = NetUserSetInfo(Globals.lptstrServer,
                              Globals.lptstrUser,
                                                  (DWORD)Globals.ucStructureLevel,
                                                  (LPBYTE)&Globals.uiUserInfo,
                                                  &dwError);
    }
  }

_useradderror_exit:
  if (rc != NERR_Success)
        UserAddError (rc,
                          dwError);

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

  rc = UserAdd();                                /* this is our main routine */
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  return (rc);
}
