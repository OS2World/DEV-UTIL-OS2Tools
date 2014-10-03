/*****************************************************************************
 * Name      : Module Config
 * Funktion  : Behandlung von Konfigurationsscripts.
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [(null), 19.05.1996 22.28.36]
 *****************************************************************************/

#ifndef MODULE_CONFIG
#define MODULE_CONFIG


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#include "rstack.h"


/*****************************************************************************
 * Defines                                                                   *
 *****************************************************************************/

#ifndef MAXPATHLEN
#define MAXPATHLEN      260                     /* Maximale Dateinamenlaenge */
#endif

#define RC_EXIT         65535         /* Rueckgabewert signalisiert Beendung */
                                                           /* des Programmes */

/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef unsigned long (*PFNI)(PSTACK pStackParameters);

typedef struct
{
  PSZ   pszCommand;                                    /* Name des Kommandos */
  ULONG ulParameters;                               /* Anzahl von Parametern */
  PFNI  pfFunction;                   /* Zeiger auf die zugehoerige Funktion */
  PSZ   pszHelp;                                      /* Beschreibender Text */
} COMMAND, *PCOMMAND;


typedef struct
{
  PSZ   pszName;                                    /* Name of this variable */
  PSZ   pszValue;                                  /* Value of this variable */
  PVOID pNext;            /* pointer to the next variable in the linked list */
} VARLIST, *PVARLIST, **PPVARLIST;


/****************************************************************************
 * Prototypen                                                               *
 ****************************************************************************/

APIRET ConfigProcess          (PSZ       pszConfigFile);

void   ParserRun              (void);

APIRET ParseCommand           (PSZ       pszCommand,
                               PSZ       pszParameter);

APIRET CommandSearch          (PSZ       pszCommand,
                               PULONG    pulIndex);

APIRET CommandParameterGet    (PSTACK    pStack,
                               PSZ       pszParameter,
                               PULONG    pulReturnParameters);

APIRET CommandRun             (PSTACK    pStack,
                               ULONG     ulCommand);

APIRET VarListAdd             (PPVARLIST pVarList,
                               PSZ       pszName,
                               PSZ       pszValue);

PSZ    VarListQuery           (PVARLIST  pVarList,
                               PSZ       pszName);

ULONG  ColorTableQuery        (ULONG     ulID);

BOOL   ConfigTagToBool        (PSZ       pszTag);


/*****************************************************************************
 * Eingetragene Kommandos fuer die Scriptsprache                             *
 *****************************************************************************/

APIRET CmdInclude             (PSTACK pStackParameters);
APIRET CmdHelp                (PSTACK pStackParameters);
APIRET CmdSet                 (PSTACK pStackParameters);
APIRET CmdBitmap              (PSTACK pStackParameters);
APIRET CmdDebugDump           (PSTACK pStackParameters);
APIRET CmdBitmapCache         (PSTACK pStackParameters);
APIRET CmdScreen              (PSTACK pStackParameters);
APIRET CmdScreenText          (PSTACK pStackParameters);
APIRET CmdScreenBitmap        (PSTACK pStackParameters);
APIRET CmdScreenButton        (PSTACK pStackParameters);
APIRET CmdColor               (PSTACK pStackParameters);
APIRET CmdHelpPage            (PSTACK pStackParameters);
APIRET CmdHelpEntry           (PSTACK pStackParameters);
APIRET CmdAnimation           (PSTACK pStackParameters);
APIRET CmdAnimationFrame      (PSTACK pStackParameters);


#endif /* MODULE_CONFIG */
