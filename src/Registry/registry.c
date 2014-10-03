/***********************************************************
 * Registry Tool.                                          *
 * Command line interface to Microsoft Windows NT Registry *
 * IBM OS/2 Profiles (INIs) and OS/2 Registry.             *
 * (c) 1997 Patrick Haller Systemtechnik                   *
 ***********************************************************/

/* #define DEBUG 1 */

/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#ifdef __OS2__
  #define INCL_DOSERRORS
  #define INCL_BASE
  #include <os2.h>

  /* entries from the OS/2 Warp registry (Open32) */

  /* Registry Definitions
   */
  #define REGH_SYSINFO              0xFFFFFFFDL
  #define REGH_WINOS2INI            0xFFFFFFFEL
  #define REGH_INIMAPPING           0xFFFFFFFFL

  #define HKEY_LOCAL_MACHINE      0xFFFFFFEFL
  #define HKEY_CURRENT_USER       0xFFFFFFEEL
  #define HKEY_USERS              0xFFFFFFEDL
  #define HKEY_CLASSES_ROOT       0xFFFFFFECL
  #define HKEY_PERFORMANCE_DATA   0x00000000L

  #define KEY_QUERY_VALUE         0x0001
  #define KEY_SET_VALUE           0x0002
  #define KEY_CREATE_SUB_KEY      0x0004
  #define KEY_ENUMERATE_SUB_KEYS  0x0008
  #define KEY_NOTIFY              0x0010
  #define KEY_CREATE_LINK         0x0020
  #define REG_OPTION_NON_VOLATILE 0x00000000L
  #define REG_OPTION_VOLATILE     0x00000001L
  #define REG_CREATED_NEW_KEY     0x00000001L
  #define REG_OPENED_EXISTING_KEY 0x00000002L

  #define KEY_READ         READ_CONTROL | KEY_QUERY_VALUE |\
                             KEY_ENUMERATE_SUB_KEYS | KEY_NOTIFY

  #define KEY_WRITE        READ_CONTROL | KEY_SET_VALUE | KEY_CREATE_SUB_KEY

  #define KEY_EXECUTE      KEY_READ

  #define KEY_ALL_ACCESS   STANDARD_RIGHTS_ALL | KEY_QUERY_VALUE |\
                             KEY_SET_VALUE | KEY_CREATE_SUB_KEY |\
                             KEY_ENUMERATE_SUB_KEYS | KEY_NOTIFY |\
                             KEY_CREATE_LINK

  #define REG_NONE                  0
  #define REG_SZ                    1
  #define REG_EXPAND_SZ             2
  #define REG_BINARY                3
  #define REG_DWORD                 4
  #define REG_DWORD_LITTLE_ENDIAN   4
  #define REG_DWORD_BIG_ENDIAN      5
  #define REG_LINK                  6
  #define REG_MULTI_SZ              7
  #define REG_RESOURCE_LIST         8

  #define WINAPI      _System
  #define ERROR_SUCCESS NO_ERROR

  typedef ULONG  DWORD;
  typedef DWORD  *PDWORD;
  typedef DWORD  *LPDWORD;
  typedef ULONG  HKEY;
  typedef HKEY   *PHKEY;
  typedef PSZ    LPCTSTR;
  typedef PSZ    LPSTR;
  typedef PSZ    LPCSTR;
  typedef BYTE   *LPBYTE;
  typedef UCHAR  TCHAR;
  typedef TCHAR  *PTCHAR;
  typedef USHORT WORD;

  typedef struct _tagFILETIME
  {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
  } FILETIME, *PFILETIME, *LPFILETIME;

  /* Systemtime Structure
   */
  typedef struct _tagSYSTEMTIME
  {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
  } SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;


  #define REGSAM                ULONG
  #define LPSECURITY_ATTRIBUTES PVOID
  #define PFILETIME             PVOID
  #define STANDARD_RIGHTS_ALL   0


  LONG    WINAPI   RegCloseKey( HKEY );

  LONG    WINAPI    RegCreateKey( HKEY, LPCTSTR, PHKEY );

  LONG    WINAPI    RegCreateKeyEx( HKEY, LPCTSTR, DWORD, LPSTR, DWORD, REGSAM, LPSECURITY_ATTRIBUTES, PHKEY, PDWORD );

  LONG    WINAPI    RegDeleteKey( HKEY, LPCTSTR );

  LONG    WINAPI    RegDeleteValue( HKEY, LPSTR );

  LONG    WINAPI   RegEnumKey( HKEY, DWORD, LPSTR, DWORD );

  LONG    WINAPI   RegEnumKeyEx( HKEY, DWORD, LPSTR, PDWORD, PDWORD, LPSTR, PDWORD, PFILETIME );

  LONG    WINAPI   RegEnumValue( HKEY, DWORD, LPSTR, PDWORD, PDWORD, PDWORD, LPBYTE, PDWORD );

  LONG    WINAPI   RegOpenKey( HKEY, LPCTSTR, PHKEY );

  LONG    WINAPI   RegOpenKeyEx( HKEY, LPCTSTR, DWORD, REGSAM, PHKEY );

  LONG    WINAPI   RegQueryInfoKey( HKEY, LPSTR, PDWORD, PDWORD, PDWORD, PDWORD,
                                        PDWORD, PDWORD, PDWORD, PDWORD, PDWORD, PFILETIME );

  LONG    WINAPI   RegQueryValue( HKEY, LPCTSTR, LPSTR, PLONG );

  LONG    WINAPI   RegQueryValueEx( HKEY, LPCTSTR, PDWORD, PDWORD, LPBYTE, PDWORD );

  LONG    WINAPI   RegSetValue( HKEY, LPCTSTR, DWORD, LPCTSTR, DWORD );

  LONG    WINAPI   RegSetValueEx( HKEY, LPCTSTR, DWORD, DWORD, const BYTE *, DWORD );

  BOOL    WINAPI   OemToChar( LPCSTR, LPSTR );

  BOOL    WINAPI   CharToOem( LPCSTR, LPSTR );

  BOOL    WINAPI   FileTimeToSystemTime( const FILETIME *, PSYSTEMTIME );
#endif


#ifdef _WIN32
  #include <windows.h>
#endif


#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <process.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"

#define MAXPATHLEN 260


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsKey;                                   /* key name was specified */
  ARGFLAG fsValueName;                                /* value was specified */
  ARGFLAG fsValueType;                    /* type of the value was specified */
  ARGFLAG fsValue;                        /* data of the value was specified */
  ARGFLAG fsFile;                                  /* filename was specified */
  ARGFLAG fsCommand;                 /* indicates if a command was specified */
  ARGFLAG fsServer;                        /* for remote registry operations */

  PSZ pszKey;                                      /* points to the key name */
  PSZ pszValueName;                                   /* points to the value */
  PSZ pszValueType;                               /* type of value specified */
  PSZ pszValue;                                   /* data of value specified */
  PSZ pszFile;                   /* filename of the file to import or export */
  PSZ pszCommand;                /* command given for the registry operation */
  PSZ pszServer;                           /* for remote registry operations */
} OPTIONS, *POPTIONS;


typedef enum
{
  CMD_NULL,
  CMD_QUERY,
  CMD_FIND,
  CMD_SETVALUE,
  CMD_CREATEKEY,
  CMD_DELETEVALUE,
  CMD_DELETEKEY,
#ifdef _WIN32
  CMD_BACKUP,
  CMD_RESTORE,
#endif
  CMD_STATISTICS,
} COMMAND_ID;


typedef APIRET (*CMDFN)(void);


typedef struct
{
  COMMAND_ID id;                                        /* id of the command */
  PSZ        pszCommand;                                    /* command token */
  CMDFN      pfnCommand;                                   /* function table */
} COMMAND, *PCOMMAND;


typedef struct
{
  DWORD      dwType;                          /* numerical type of the value */
  PSZ        pszType;                    /* alphanumerical type of the value */
} REGTYPE, *PREGTYPE;


/*****************************************************************************
 * Prototypes #1                                                             *
 *****************************************************************************/

APIRET CmdQuery       (void);
APIRET CmdFind        (void);
APIRET CmdSetValue    (void);
APIRET CmdCreateKey   (void);
APIRET CmdDeleteValue (void);
APIRET CmdDeleteKey   (void);
APIRET CmdBackup      (void);
APIRET CmdRestore     (void);
APIRET CmdStatistics  (void);


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */

COMMAND  TabCommands[]=
{
  /* ID ---------- Token ---------- */
  {CMD_QUERY,      "QUERY",      CmdQuery},
  {CMD_FIND,       "FIND",       CmdFind},
  {CMD_SETVALUE,   "SET",        CmdSetValue},
  {CMD_CREATEKEY,  "CREATE",     CmdCreateKey},
  {CMD_DELETEVALUE,"DELVALUE",   CmdDeleteValue},
  {CMD_DELETEKEY,  "DELKEY",     CmdDeleteKey},
#ifdef _WIN32
  {CMD_BACKUP,     "BACKUP",     CmdBackup},
  {CMD_RESTORE,    "RESTORE",    CmdRestore},
#endif
  {CMD_STATISTICS, "STATISTICS", CmdStatistics},
  {CMD_STATISTICS, "STAT",       CmdStatistics},     /* alias for STATISTICS */
  {CMD_NULL,       NULL,         NULL}
};

REGTYPE TabRegTypes[]=
{
  /* dwType ---------------- pszType ------------------- */
  {REG_BINARY,               "REG_BINARY"},
  {REG_BINARY,               "BINARY"},
  {REG_BINARY,               "BIN"},

  {REG_DWORD,                "REG_DWORD"},
  {REG_DWORD,                "DWORD"},
  {REG_DWORD,                "ULONG"},

  {REG_DWORD_LITTLE_ENDIAN,  "REG_DWORD_LITTLE_ENDIAN"},

  {REG_DWORD_BIG_ENDIAN,     "REG_BIG_LITTLE_ENDIAN"},

  {REG_EXPAND_SZ,            "REG_EXPAND_SZ"},
  {REG_EXPAND_SZ,            "EXPAND_SZ"},

  {REG_LINK,                 "REG_LINK"},
  {REG_LINK,                 "LINK"},

  {REG_MULTI_SZ,             "REG_MULTI_SZ"},
  {REG_MULTI_SZ,             "MULTI_SZ"},

  {REG_SZ,                   "REG_SZ"},
  {REG_SZ,                   "SZ"},
  {REG_SZ,                   "STRING"},

  {REG_RESOURCE_LIST,        "REG_RESOURCE_LIST"},
  {REG_RESOURCE_LIST,        "RESOURCE_LIST"},
  {REG_RESOURCE_LIST,        "RESOURCE"},

  {REG_NONE,                 "REG_NONE"},
  {REG_NONE,                 "NONE"},

  {0,                        NULL}
};



ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung----------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/NAME=",     "Registry value name.",&Options.pszValueName,ARG_PSZ,       &Options.fsValueName},
  {"/TYPE=",     "Registry value type or key class.",
                                        &Options.pszValueType,ARG_PSZ,       &Options.fsValueType},
  {"/DATA=",     "Registry value data.",&Options.pszValue,   ARG_PSZ,        &Options.fsValue},
  {"/FILE=",     "Filename of the file to import or export.",
                                       &Options.pszFile,     ARG_PSZ,        &Options.fsFile},
  {"/SERVER=",   "For remote retistry operations.",
                                       &Options.pszServer,   ARG_PSZ,        &Options.fsServer},
  {"1",          "QUERY/FIND/SET/CREATE/DELKEY/DELVALUE/BACKUP/RESTORE/STAT",
                                       &Options.pszCommand,  ARG_PSZ  |
                                                             ARG_MUST |
                                                             ARG_DEFAULT,    &Options.fsCommand},
  {"2",          "Registry key name, e.g. HKEY_USERS\\subkey1\\subkey2.",
                                       &Options.pszKey,      ARG_PSZ  |
                                                             ARG_MUST |
                                                             ARG_DEFAULT,    &Options.fsKey},

  ARG_TERMINATE
};


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void   help                (void);

APIRET RegistryKeyOpen     (PSZ   pszKey,
                            PHKEY phKey);

APIRET RegistryKeyClose    (HKEY hKey);

APIRET CmdDispatcher       (PSZ pszCommand);


void   initialize          (void);

int    main                (int,
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
  TOOLVERSION("Registry",                               /* application name */
              0x00009901,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


/*****************************************************************************
 * Name      : APIRET CmdDispatcher
 * Funktion  : Command dispatcher
 * Parameter : PSZ pszCommand
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET CmdDispatcher (PSZ pszCommand)
{
  PCOMMAND pCommand;                       /* command pointer into the table */
  APIRET   rc;                                             /* API returncode */

  if (pszCommand == NULL)                                /* check parameters */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */

  for (pCommand = TabCommands;             /* iterate over the command table */
       pCommand->id != CMD_NULL;
           pCommand++)
  {
    if (stricmp(pCommand->pszCommand,                    /* find our command */
                        pszCommand) == 0)
          break;
  }

  if (pCommand->id == CMD_NULL)             /* check for the abort condition */
        return (ERROR_INVALID_FUNCTION);                /* raise error condition */

                   /* now call the function directly from the function table */
  rc = pCommand->pfnCommand();

  return (rc);                                     /* deliver API returncode */
}


/*****************************************************************************
 * Name      : APIRET CmdTypeQuery
 * Funktion  : Determines type of value to add
 * Parameter : PSZ pszType
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET CmdTypeQuery (PSZ pszType,
                     DWORD *pdwType)
{
  PREGTYPE pRegType;                       /* RegType pointer into the table */

  if ( (pszType == NULL) ||                              /* check parameters */
       (pdwType == NULL) )
    return (ERROR_INVALID_PARAMETER);              /* return this as default */

  for (pRegType = TabRegTypes;             /* iterate over the command table */
       pRegType->pszType != NULL;           /* REG_NONE must be last entry ! */
             pRegType++)
  {
    if (stricmp(pRegType->pszType,                          /* find our type */
                            pszType) == 0)
          break;
  }

  if (pRegType->pszType == NULL)                 /* have we found anything ? */
    return (ERROR_INVALID_DATA);                    /* raise error condition */

  *pdwType = pRegType->dwType;                          /* return the result */

  return (NO_ERROR);                                         /* deliver type */
}


/*****************************************************************************
 * Name      : APIRET RegistryKeyParse
 * Funktion  : Parse a complete key name and split up
 * Parameter : PSZ   pszKey
 *             PHKEY phKey
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET RegistryKeyParse (PSZ   pszKey,
                         PHKEY phKeyRoot,
                         PSZ  *ppszSubKey)
{
  if ( (pszKey     == NULL) ||                           /* check parameters */
       (phKeyRoot  == NULL) ||
       (ppszSubKey == NULL) )
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */

  *phKeyRoot  = 0;
  if (strnicmp("HKEY_CLASSES_ROOT", pszKey, 17) == 0) *phKeyRoot = HKEY_CLASSES_ROOT;
  else
    if (strnicmp("HKEY_CURRENT_USER", pszKey, 17) == 0) *phKeyRoot = HKEY_CURRENT_USER;
    else
      if (strnicmp("HKEY_LOCAL_MACHINE", pszKey, 18) == 0) *phKeyRoot = HKEY_LOCAL_MACHINE;
      else
        if (strnicmp("HKEY_USERS", pszKey, 10) == 0) *phKeyRoot = HKEY_USERS;
#ifdef _WIN32
        else
          if (strnicmp("HKEY_PERFORMANCE_DATA", pszKey, 17) == 0) *phKeyRoot = HKEY_PERFORMANCE_DATA;
#endif

  *ppszSubKey = strchr(pszKey,
                       '\\');
  if (*ppszSubKey != NULL)                       /* check whether key exists */
    (*ppszSubKey)++;                                   /* skip the backslash */
  else
    if (*phKeyRoot == 0)                                  /* still invalid ? */
    {
      *ppszSubKey = pszKey;                      /* then take the key itself */
      *phKeyRoot = HKEY_CURRENT_USER;            /* then use this as default */
    }
    else
      *ppszSubKey = NULL;             /* then user specified a root key only */

#ifdef DEBUG
    fprintf (stderr,
             "\nDEBUG: pszKey={%s} hKeyRoot=%08xh pszSubKey={%s}",
             pszKey,
             *phKeyRoot,
             *ppszSubKey);
#endif

  return (NO_ERROR);                                                   /* OK */
}


/*****************************************************************************
 * Name      : APIRET RegistryKeyOpen
 * Funktion  : Open a registry key from key name
 * Parameter : PSZ pszKey, PHKEY phKey
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET RegistryKeyOpen (PSZ   pszKey,
                        PHKEY phKey)
{
  LONG lResult;                    /* result code from the Registry API call */
  HKEY hKeyRoot;                   /* the root key for the Registry API call */
  PSZ  pszSubKey;                  /* points to the subkey without root part */
  APIRET rc;                                               /* API returncode */

  rc =RegistryKeyParse(pszKey,                              /* parse the key */
                       &hKeyRoot,
                       &pszSubKey);
  if (rc != NO_ERROR)                                        /* check errors */
    return (rc);                                    /* return API returncode */


  lResult = RegOpenKeyEx(hKeyRoot,                        /* root key handle */
                         pszSubKey,                          /* sub key name */
                         0,                                       /* options */
                         KEY_ALL_ACCESS,         /* SAM security information */
                         phKey);                        /* result key handle */
  if (lResult == ERROR_SUCCESS)                     /* check the result code */
    return (NO_ERROR);                                                 /* OK */
  else
    return (lResult);        /* Win32 docs don't say anything about this ... */
}


/*****************************************************************************
 * Name      : APIRET RegistryKeyCreate
 * Funktion  : Create a registry key from key name
 * Parameter : PSZ pszKey, PHKEY phKey
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET RegistryKeyCreate (PSZ     pszKey,
                          PSZ     pszClass,
                          PHKEY   phKey,
                          LPDWORD pdwDisposition)
{
  LONG lResult;                    /* result code from the Registry API call */
  HKEY hKeyRoot;                   /* the root key for the Registry API call */
  PSZ  pszSubKey;                  /* points to the subkey without root part */
  TCHAR  lptstrClass[256];                  /* reserve buffer for class name */
  PTCHAR pstrClass;
  APIRET rc;                                               /* API returncode */

  rc =RegistryKeyParse(pszKey,                              /* parse the key */
                       &hKeyRoot,
                       &pszSubKey);
  if (rc != NO_ERROR)                                        /* check errors */
    return (rc);                                    /* return API returncode */

  if (pszClass != NULL)                      /* if class specified, then ... */
  {
#ifdef _WIN32
    OemToChar(pszClass,               /* convert value name to UniCODE Class */
              lptstrClass);
#endif

#ifdef __OS2__
    strcpy(lptstrClass,
           pszClass);
#endif

    pstrClass = lptstrClass;                    /* and set the pointer to it */
  }
  else
    pstrClass = NULL;                              /* else there is no class */

#ifdef DEBUG
fprintf (stderr,
         "\nDEBUG: subkey=%s hKeyRoot=%08xh",
         pszSubKey,
         hKeyRoot);
#endif

  lResult = RegCreateKeyEx(hKeyRoot,                      /* root key handle */
                           pszSubKey,                        /* sub key name */
                           0,                                     /* options */
                           pstrClass,
                           REG_OPTION_NON_VOLATILE, /* Win95 is targetted ...*/
                           KEY_ALL_ACCESS,       /* SAM security information */
                           NULL,                              /* no security */
                           phKey,                       /* result key handle */
                           pdwDisposition);                   /* action code */
  if (lResult == ERROR_SUCCESS)                     /* check the result code */
    return (NO_ERROR);                                                 /* OK */
  else
    return (lResult);        /* Win32 docs don't say anything about this ... */
}


/*****************************************************************************
 * Name      : APIRET RegistryKeyClose
 * Funktion  : Close an open registry key
 * Parameter : HKEY hKey
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET RegistryKeyClose (HKEY hKey)
{
  return (RegCloseKey(hKey));                       /* simply forwarder call */
}

/*****************************************************************************
 * Name      : APIRET CmdQuery
 * Funktion  : Process the QUERY command
 * Parameter : void
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET CmdQuery (void)
{
  HKEY       hKey;                                    /* handle to "our" key */
  APIRET     rc;                                          /* API return code */
  LONG       lResult;              /* return code from the Registry API call */
  TCHAR      lptstrClass[256];              /* reserve buffer for class name */
  CHAR       szClass[256];                            /* dto as ASCII string */
  TCHAR      lptstrValueName[256];          /* reserve buffer for value name */
  ULONG      ulcbClass;
  ULONG      ulSubKeys;
  ULONG      ulMaxSubKeyLen;
  ULONG      ulMaxClassLen;
  ULONG      ulValues;
  ULONG      ulMaxValueNameLen;
  ULONG      ulMaxValueLen;
  FILETIME   ftLastWriteTime;        /* date & time last written to this key */
  SYSTEMTIME syLastWriteTime;
  CHAR       szLastWriteTime[48];    /* buffer for date to string conversion */

  ULONG      ulValueType;                  /* values used to dump the values */
  ULONG      ulValueBufferSize;          /* size of the buffer for the value */
  PVOID      pBuffer;                            /* the value buffer pointer */

  if (!Options.fsKey)            /* check whether "key" was specified or not */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */

  rc = RegistryKeyOpen (Options.pszKey,
                        &hKey);
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                                    /* raise error condition */

  fprintf (stderr,
           "\nQuerying %s",
           Options.pszKey);

  /* @@@ query the info */
  ulcbClass = sizeof(szClass);

  lResult = RegQueryInfoKey(hKey,
                            lptstrClass,
                            &ulcbClass,
                            NULL,
                            &ulSubKeys,
                            &ulMaxSubKeyLen,
                            &ulMaxClassLen,
                            &ulValues,
                            &ulMaxValueNameLen,
                            &ulMaxValueLen,
                            NULL,                     /* security descriptor */
                            &ftLastWriteTime);
  if (lResult == ERROR_SUCCESS)
  {
#ifdef _WIN32
    if (CharToOem(lptstrClass,              /* convert from UniCode to ASCII */
                  szClass) == FALSE)
      strcpy (szClass,
              "<unknown>");

    if (FileTimeToSystemTime(&ftLastWriteTime,    /* convert the date struct */
                             &syLastWriteTime) == TRUE)
      sprintf(szLastWriteTime,
              "%04u/%02u/%02u %02u:%02u:%02u",
              syLastWriteTime.wYear,
              syLastWriteTime.wMonth,
              syLastWriteTime.wDay,
              syLastWriteTime.wHour,
              syLastWriteTime.wMinute,
              syLastWriteTime.wSecond);
    else
      strcpy (szLastWriteTime,                    /* unknown last write time */
              "<unknown>");
#endif

#ifdef __OS2__
  /* @@@PH */
      sprintf(szLastWriteTime,
              "%04u/%02u/%02u %02u:%02u:%02u",
              syLastWriteTime.wYear,
              syLastWriteTime.wMonth,
              syLastWriteTime.wDay,
              syLastWriteTime.wHour,
              syLastWriteTime.wMinute,
              syLastWriteTime.wSecond);
#endif

    printf ("\n[%s] of class [%s]"
      "\n  Structure Size:    %5u     Subkeys:                %5u"
      "\n  Max. class length: %5u     Max. subkey length:     %5u"
      "\n  Values:            %5u     Max. value name length: %5u"
      "\n  Max. value length: %5u     Last write date:        %s",
      Options.pszKey,
      szClass,
      ulcbClass,
      ulSubKeys,
      ulMaxClassLen,
      ulMaxSubKeyLen,
      ulValues,
      ulMaxValueNameLen,
      ulMaxValueLen,
      szLastWriteTime);
  }
  else
  {
    RegistryKeyClose(hKey);                    /* close the key and bail out */
    return((APIRET)lResult);                       /* save error information */
  }



  if (Options.fsValueName)          /* check if user wants value information */
  {
#ifdef _WIN32
    OemToChar(Options.pszValueName,                /* convert the value name */
              lptstrValueName);
#endif

#ifdef __OS2__
    strcpy(Options.pszValueName,
           lptstrValueName);
#endif

    lResult = RegQueryValueEx(hKey,              /* now query the key values */
                              lptstrValueName,
                              NULL,
                              NULL,
                              NULL,
                              &ulValueBufferSize);
       /* now ulValueBufferSize holds the required buffer size for the value */
    pBuffer = malloc(ulValueBufferSize);
    if (pBuffer == NULL)                      /* check for proper allocation */
    {
      RegistryKeyClose(hKey);                               /* close the key */
      return (ERROR_NOT_ENOUGH_MEMORY);             /* raise error condition */
    }


    lResult = RegQueryValueEx(hKey,
                              lptstrValueName,
                              NULL,
                              &ulValueType,
                              pBuffer,
                              &ulValueBufferSize);

    printf("\nValue [%s] ",
           Options.pszValueName);

    switch (ulValueType)
    {
      /************************************************************************
       * Dump REG_BINARY type value                                           *
       ************************************************************************/

      case REG_BINARY:
        printf ("[REG_BINARY] (%u bytes)",
                ulValueBufferSize);
        ToolsDumpHex(0,
                     ulValueBufferSize,
                     pBuffer);
        break;

      /************************************************************************
       * Dump REG_DWORD_LITTLE_ENDIAN type value                              *
       ************************************************************************/

      case REG_DWORD_LITTLE_ENDIAN:
        printf ("[REG_DWORD_LITTLE_ENDIAN] : %u (%08xh)",
                *(PULONG)pBuffer,
                *(PULONG)pBuffer);
        break;

      /************************************************************************
       * Dump REG_DWORD_BIG_ENDIAN type value                                 *
       ************************************************************************/

      case REG_DWORD_BIG_ENDIAN:
        printf ("[REG_DWORD_BIG_ENDIAN] : %u (%08xh)",
                *(PULONG)pBuffer,
                *(PULONG)pBuffer);
        break;

      /************************************************************************
       * Dump REG_EXPAND_SZ type value                                        *
       ************************************************************************/

      case REG_EXPAND_SZ:
        printf ("[REG_EXPAND_SZ] : [%s]",
                (PSZ)pBuffer);
        break;

      /************************************************************************
       * Dump REG_EXPAND_SZ type value                                        *
       ************************************************************************/

      case REG_LINK:
        printf ("[REG_LINK]");
        ToolsDumpHex(0,
                     ulValueBufferSize,
                     pBuffer);
        break;

      /************************************************************************
       * Dump REG_EXPAND_SZ type value                                        *
       ************************************************************************/

      case REG_MULTI_SZ:
      {
        ULONG ulCounter = 0;             /* loop counter for the string array */
        PSZ   pszTemp = (PSZ)pBuffer;         /* pointer into the data buffer */

        printf ("[REG_MULTI_SZ]");

        for (ulCounter = 0;
             (ulCounter < ulValueBufferSize) &&
               (*pszTemp != 0);
             ulCounter++,
             pszTemp++)
        {
          printf ("\n[%2u] %s",
                  pszTemp);

          pszTemp += strlen(pszTemp) + 1;          /* skip to the next string */
        }
      }
      break;

      /************************************************************************
       * Dump REG_SZ type value                                               *
       ************************************************************************/

      case REG_SZ:
        printf ("[REG_SZ] : [%s]",
                (PSZ)pBuffer);
        break;

      /************************************************************************
       * Dump REG_NONE type value                                             *
       ************************************************************************/

      case REG_NONE:
        printf ("[REG_NONE] (%u bytes)",
                ulValueBufferSize);
        ToolsDumpHex(0,
                     ulValueBufferSize,
                     pBuffer);
        break;

      /************************************************************************
       * Dump REG_RESOURCE type value                                         *
       ************************************************************************/

      case REG_RESOURCE_LIST:
        printf ("[REG_RESOURCE_LIST] (%u bytes)",
                ulValueBufferSize);
        ToolsDumpHex(0,
                     ulValueBufferSize,
                     pBuffer);
        break;

      /************************************************************************
       * Dump default type value                                              *
       ************************************************************************/

      default:
        printf ("[<unknown>] (%u bytes)",
                ulValueBufferSize);
        ToolsDumpHex(0,
                     ulValueBufferSize,
                     pBuffer);
        break;
    }

    free(pBuffer);                /* free the buffer used to store the value */
  }

  rc = RegistryKeyClose(hKey);                       /* close this key again */
  return (rc);                                                         /* OK */
}



/*****************************************************************************
 * Name      : APIRET CmdSetValue
 * Funktion  : Process the SET command
 * Parameter : void
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET CmdSetValue (void)
{
  HKEY       hKey;                                    /* handle to "our" key */
  APIRET     rc;                                          /* API return code */
  LONG       lResult;              /* return code from the Registry API call */
  TCHAR      lptstrValueName[256];          /* reserve buffer for value name */
  DWORD      dwType;                                    /* type of the entry */
  PVOID      pData;                              /* typeless pointer to data */
  DWORD      dwDummy;                                /* 32-bit integer dummy */
  ULONG      ulDataLength;                      /* length of the data buffer */

  if ( (!Options.fsKey)       || /* check whether "key" was specified or not */
       (!Options.fsValueName) ||
       (!Options.fsValueType) ||
       (!Options.fsValue) )
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */


  rc = CmdTypeQuery (Options.pszValueType,                   /* get REG_type */
                     &dwType);
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                               /* then raise error condition */


  switch (dwType)                                      /* map to data buffer */
  {
    case REG_BINARY:
    case REG_RESOURCE_LIST:
    case REG_NONE:
    case REG_MULTI_SZ:
      fprintf(stderr,
              "\nError: Value type currently not supported.");
      return (ERROR_INVALID_FUNCTION);

    case REG_DWORD:
    /*case REG_DWORD_LITTLE_ENDIAN:*/
    case REG_DWORD_BIG_ENDIAN:
      dwDummy = atoi(Options.pszValue);         /* convert to 32-bit integer */
      pData = &dwDummy;                                       /* point to it */
      ulDataLength = sizeof(dwDummy);            /* and calculate the length */
      break;

    case REG_SZ:
    case REG_EXPAND_SZ:
    case REG_LINK:
      pData = Options.pszValue;                               /* point to it */
      ulDataLength = strlen(Options.pszValue)+1; /* and calculate the length */
      break;

    default:
      fprintf(stderr,
              "\nError: unknown internal type (%u).",
              dwType);
      return (ERROR_INVALID_FUNCTION);
  }


  rc = RegistryKeyOpen (Options.pszKey,
                        &hKey);
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                                    /* raise error condition */

#ifdef _WIN32
  OemToChar(Options.pszValueName,                  /* convert the value name */
            lptstrValueName);
#endif

#ifdef __OS2__
  strcpy(lptstrValueName,
         Options.pszValueName);
#endif


  lResult = RegSetValueEx(hKey,                                /* key handle */
                          lptstrValueName,                   /* UniCODE Name */
                          0,                                     /* reserved */
                          dwType,                           /* type of entry */
                          pData,                          /* pointer to data */
                          ulDataLength);                   /* length of data */

  if (lResult == ERROR_SUCCESS)
    printf ("\n%s.%s[%s]=%s OK",                                       /* OK */
            Options.pszKey,
            Options.pszValueName,
            Options.pszValueType,
            Options.pszValue);
  else
  {
    fprintf (stderr,                                  /* yield error message */
             "\n%s.%s[%s]=%s failed",
             Options.pszKey,
             Options.pszValueName,
             Options.pszValueType,
             Options.pszValue);

    RegistryKeyClose(hKey);                    /* close the key and bail out */
    return((APIRET)lResult);                       /* save error information */
  }

  rc = RegistryKeyClose(hKey);                       /* close this key again */
  return (rc);                                                         /* OK */
}



/*****************************************************************************
 * Name      : APIRET CmdFind
 * Funktion  : Process the FIND command
 * Parameter : void
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET CmdFind (void)
{
  fprintf (stderr,
               "\nFIND not yet implemented.");

  return (NO_ERROR);                                                   /* OK */
}


/*****************************************************************************
 * Name      : APIRET CmdDeleteKey
 * Funktion  : Process the Delete command
 * Parameter : void
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET CmdDeleteKey (void)
{
  HKEY       hKeyRoot;                                /* handle to "our" key */
  HKEY       hKey;                                    /* handle to "our" key */
  APIRET     rc;                                          /* API return code */
  LONG       lResult;              /* return code from the Registry API call */
  TCHAR      lptstrKeyName[256];            /* reserve buffer for value name */
  PSZ        pszSubKey;       /* the user specified part of the registry key */

  rc =RegistryKeyParse(Options.pszKey,                      /* parse the key */
                       &hKeyRoot,
                       &pszSubKey);
  if (rc != NO_ERROR)                                        /* check errors */
    return (rc);                                    /* return API returncode */

  lResult = RegOpenKeyEx(hKeyRoot,                        /* root key handle */
                         NULL,                               /* sub key name */
                         0,                                       /* options */
                         KEY_ALL_ACCESS,         /* SAM security information */
                         &hKey);                        /* result key handle */
  if (lResult != ERROR_SUCCESS)                     /* check the result code */
    return (lResult);        /* Win32 docs don't say anything about this ... */

#ifdef _WIN32
  OemToChar(pszSubKey,                             /* convert the value name */
            lptstrKeyName);
#endif

#ifdef __OS2__
  strcpy (lptstrKeyName,
          pszSubKey);
#endif

  lResult = RegDeleteKey(hKey,                                 /* key handle */
                         lptstrKeyName);                     /* UniCODE Name */
  if (lResult == ERROR_SUCCESS)
    printf ("\nKey %s deleted.",                                       /* OK */
            Options.pszKey);
  else
  {
    fprintf (stderr,                                  /* yield error message */
             "\nDeleting of key %s failed.",
             Options.pszKey);

    RegistryKeyClose(hKey);                    /* close the key and bail out */
    return((APIRET)lResult);                       /* save error information */
  }

  rc = RegistryKeyClose(hKey);                       /* close this key again */
  return (rc);                                                         /* OK */
}


/*****************************************************************************
 * Name      : APIRET CmdDeleteValue
 * Funktion  : Process the Delete command
 * Parameter : void
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET CmdDeleteValue (void)
{
  HKEY       hKey;                                    /* handle to "our" key */
  APIRET     rc;                                          /* API return code */
  LONG       lResult;              /* return code from the Registry API call */
  TCHAR      lptstrValueName[256];          /* reserve buffer for value name */

  if (!Options.fsKey)            /* check whether "key" was specified or not */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */


  rc = RegistryKeyOpen (Options.pszKey,
                        &hKey);
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                                    /* raise error condition */

#ifdef _WIN32
  OemToChar(Options.pszValueName,                  /* convert the value name */
            lptstrValueName);
#endif

#ifdef __OS2__
  strcpy(lptstrValueName,
         Options.pszValueName);
#endif

  lResult = RegDeleteValue(hKey,                               /* key handle */
                           lptstrValueName);                 /* UniCODE Name */
  if (lResult == ERROR_SUCCESS)
    printf ("\nValue %s.%s deleted.",                                  /* OK */
            Options.pszKey,
            Options.pszValueName);
  else
  {
    fprintf (stderr,                                  /* yield error message */
             "\nDeleting of value %s.%s failed",
             Options.pszKey,
             Options.pszValueName);

    RegistryKeyClose(hKey);                    /* close the key and bail out */
    return((APIRET)lResult);                       /* save error information */
  }

  rc = RegistryKeyClose(hKey);                       /* close this key again */
  return (rc);                                                         /* OK */
}


/*****************************************************************************
 * Name      : APIRET CmdCreateKey
 * Funktion  : Process the CREATEKEY command
 * Parameter : void
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET CmdCreateKey (void)
{
  HKEY       hKey;                                    /* handle to "our" key */
  APIRET     rc;                                          /* API return code */
  ULONG      ulAction;                               /* registry action code */

  if (!Options.fsKey)            /* check whether "key" was specified or not */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */


  rc = RegistryKeyCreate (Options.pszKey,
                          Options.pszValueType,          /* re-used as class */
                          &hKey,
                          &ulAction);                 /* and the action code */
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                                    /* raise error condition */

  switch (ulAction)
  {
    case REG_CREATED_NEW_KEY:
      printf ("\nCreated new key %s [%s].",
              Options.pszKey,
              Options.pszValueType ? Options.pszValueType : "<no class>");
      break;

    case REG_OPENED_EXISTING_KEY:
      printf ("\nKey %s already exists.",
              Options.pszKey);
      break;
  }

  rc = RegistryKeyClose(hKey);                       /* close this key again */
  return (rc);                                                         /* OK */
}


#ifdef _WIN32
/*****************************************************************************
 * Name      : APIRET CmdBackup
 * Funktion  : Process the BACKUP command
 * Parameter : void
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET CmdBackup (void)
{
  HKEY       hKey;                                    /* handle to "our" key */
  APIRET     rc;                                          /* API return code */
  LONG       lResult;              /* return code from the Registry API call */
  TCHAR      lptstrFile[256];               /* reserve buffer for file  name */

  if ( (!Options.fsKey) ||       /* check whether "key" was specified or not */
       (!Options.fsFile) )
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */

  rc = RegistryKeyOpen (Options.pszKey,
                        &hKey);
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                                    /* raise error condition */

  OemToChar(Options.pszFile,                       /* convert the value name */
            lptstrFile);

  printf ("\nBacking up to %s.",
          Options.pszFile);

  lResult = RegSaveKey(hKey,                                   /* key handle */
                       lptstrFile,                           /* UniCODE Name */
                       NULL);                  /* default security behaviour */
  if (lResult == ERROR_SUCCESS)
    printf ("\nBackup of %s to %s is OK.",                             /* OK */
            Options.pszKey,
            Options.pszFile);
  else
  {
    fprintf (stderr,                                  /* yield error message */
             "\nBackup of %s to %s failed.",
             Options.pszKey,
             Options.pszFile);

    RegistryKeyClose(hKey);                    /* close the key and bail out */
    return((APIRET)lResult);                       /* save error information */
  }

  rc = RegistryKeyClose(hKey);                       /* close this key again */
  return (rc);                                                         /* OK */
}


/*****************************************************************************
 * Name      : APIRET CmdRestore
 * Funktion  : Process the RESTORE command
 * Parameter : void
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET CmdRestore (void)
{
  HKEY       hKeyRoot;                                /* handle to "our" key */
  HKEY       hKey;                                    /* handle to "our" key */
  APIRET     rc;                                          /* API return code */
  LONG       lResult;              /* return code from the Registry API call */
  TCHAR      lptstrKeyName[256];            /* reserve buffer for value name */
  TCHAR      lptstrFile[256];               /* reserve buffer for file  name */
  PSZ        pszSubKey;       /* the user specified part of the registry key */

  if ( (!Options.fsKey) ||       /* check whether "key" was specified or not */
       (!Options.fsFile) )
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */

  rc =RegistryKeyParse(Options.pszKey,                      /* parse the key */
                       &hKeyRoot,
                       &pszSubKey);
  if (rc != NO_ERROR)                                        /* check errors */
    return (rc);                                    /* return API returncode */

  lResult = RegOpenKeyEx(hKeyRoot,                        /* root key handle */
                         NULL,                               /* sub key name */
                         0,                                       /* options */
                         KEY_ALL_ACCESS,         /* SAM security information */
                         &hKey);                        /* result key handle */
  if (lResult != ERROR_SUCCESS)                     /* check the result code */
    return (lResult);        /* Win32 docs don't say anything about this ... */

  OemToChar(pszSubKey,                             /* convert the value name */
            lptstrKeyName);

  OemToChar(Options.pszFile,                       /* convert the value name */
            lptstrFile);

  lResult = RegLoadKey(hKey,                                   /* key handle */
                       lptstrKeyName,                        /* UniCODE Name */
                       lptstrFile);                          /* UniCODE Name */
  if (lResult == ERROR_SUCCESS)
    printf ("\nRestore of %s from %s is OK.",                          /* OK */
            Options.pszKey,
            Options.pszFile);
  else
  {
    fprintf (stderr,                                  /* yield error message */
             "\nRestore of %s from %s failed.",
             Options.pszKey,
             Options.pszFile);

    RegistryKeyClose(hKey);                    /* close the key and bail out */
    return((APIRET)lResult);                       /* save error information */
  }

  rc = RegistryKeyClose(hKey);                       /* close this key again */
  return (rc);                                                         /* OK */
}
#endif


/*****************************************************************************
 * Name      : APIRET CmdStatistics
 * Funktion  : Process the STATISTICS command
 * Parameter : void
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET CmdStatistics (void)
{
  fprintf (stderr,
               "\nSTATISTICS not yet implemented.");

  return (NO_ERROR);                                                   /* OK */
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

void initialize (void)
{
  memset(&Options,
         0L,
         sizeof(Options));
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

  initialize ();                                          /* Initialisierung */

  rc = ArgStandard (argc,                            /* CLI-Parameter parsen */
                    argv,
                    TabArguments,
                    &Options.fsHelp);
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    exit(1);                                                /* abort program */
  }

  if (Options.fsHelp)                                  /* user want's help ! */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  rc = CmdDispatcher(Options.pszCommand);    /* call the command interpreter */
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  return (rc);
}
