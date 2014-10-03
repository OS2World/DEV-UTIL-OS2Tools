/***********************************************************************
 * Projekt   : OS2TOOLS General C Library
 * Name      : Modul os2arg
 * Funktion  : Flexible Verarbeitung von Kommandozeilenparametern
 *
 * Autor     : Patrick Haller [Dienstag, 26.09.1995 17.57.39]
 ***********************************************************************/

#ifndef MODULE_OS2TOOLS_ARGUMENTS
#define MODULE_OS2TOOLS_ARGUMENTS

#ifdef __cplusplus
      extern "C" {
#endif


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#include "tooltypes.h"


/*****************************************************************************
 * Definitions                                                               *
 *****************************************************************************/

#ifndef TRUE
#define TRUE -1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* Format der Zielformate */
#define ARG_NULL        0x00
#define ARG_CHAR        0x01
#define ARG_UCHAR       0x02
#define ARG_LONG        0x03
#define ARG_ULONG       0x04
#define ARG_SHORT       0x05
#define ARG_USHORT      0x06
#define ARG_PSZ         0x07

#define ARG_MASK_TYPE   0x0f            /* 4 bits reserved for argument type */

#define ARG_HIDDEN      0x20            /* does not show up in the help list */
#define ARG_MANDATORY   0x40
#define ARG_MUST        ARG_MANDATORY
#define ARG_DEFAULT     0x80
#define ARG_ENDOFLIST   0xFF


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef char    ARGFLAG;
typedef ARGFLAG *PARGFLAG;
        
#pragma pack(1)
        
typedef struct
{
  char         *pszToken;                      /* Token fÅr die Command Line */
  char         *pszDescription;                              /* Beschreibung */
  void         *pTarget;                      /* Zeiger auf die Zielvariable */
  unsigned char ucTargetFormat;                   /* Format der Zielvariable */
  PARGFLAG      pTargetSpecified;  /* Flag, ob dieses Kommando benutzt wurde */
} ARGUMENT, *PARGUMENT;

#pragma pack()

#define ARG_TERMINATE {(PSZ)NULL,(PSZ)NULL,(PVOID)NULL,ARG_ENDOFLIST,NULL}


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

BOOL   TOOLAPI ArgMapToTarget ( PARGUMENT pArg,
                               void     *pData);

APIRET TOOLAPI ArgHelp        ( PARGUMENT   pArguments );

APIRET TOOLAPI ArgParse       ( int         argc,
                               char        *argv[],
                               PARGUMENT   pArguments );

APIRET TOOLAPI ArgGetMissing  ( PARGUMENT   pArguments );

APIRET TOOLAPI ArgIsArgumentMissing ( PARGUMENT pTabArguments );

APIRET TOOLAPI ArgStandard    ( int       argc,
                               char      *argv[],
                               PARGUMENT pTabArguments,
                               char *    pfsHelp );


#ifdef __cplusplus
      }
#endif


#endif /* MODULE_OS2TOOLS_ARGUMENTS */

