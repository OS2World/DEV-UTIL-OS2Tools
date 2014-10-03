/***********************************************************************
 * Projekt   : OS2TOOLS General C Library
 * Name      : Modul os2log
 * Funktion  : Enth„lt Funktionen zur Behandlung von LogFiles
 *
 * Autor     : Patrick Haller [Freitag, 12.08.1994 05.19.14]
 ***********************************************************************/

#ifndef MODULE_OS2TOOLS_LOG
#define MODULE_OS2TOOLS_LOG

#ifdef __cplusplus
      extern "C" {
#endif
        
#pragma pack(1)
        
/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#include "tooltypes.h"
        
        
/*****************************************************************************
 * Types and structures                                                      *
 *****************************************************************************/

typedef char LOGAPPNAME[5];
        
        
/*****************************************************************************
 * Defines and constants                                                     *
 *****************************************************************************/
        
/* logging modes */
#define LOGMODE_WRITE_THROUGH 0
#define LOGMODE_CACHE         1
        
        
/* logging levels */
#define LOG_SYSTEM      128
#define LOG_ERRORS      16
#define LOG_CRITICALS   8
#define LOG_DEBUG       4
#define LOG_WARNINGS    2
#define LOG_INFOS       1

#define LOG_ALL   LOG_SYSTEM    | \
                  LOG_ERRORS    | \
                  LOG_CRITICALS | \
                  LOG_DEBUG     | \
                  LOG_WARNINGS  | \
                  LOG_INFOS
        
        
/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/
        
APIRET TOOLAPI LogOpen  (UCHAR ucLogLevel,
                         PSZ   pszLogFilename,
                         UCHAR ucMode);

APIRET TOOLAPI LogClose (void);

APIRET TOOLAPI LogPrint (UCHAR ucLogLevel,
                         PSZ   pszApp,
                         PSZ   pszFormat,
                         ...);


#ifdef __cplusplus
      }
#endif


#endif /* MODULE_OS2TOOLS_LOG */
