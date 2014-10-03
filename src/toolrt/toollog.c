/***********************************************************************
 * Projekt   : OS2TOOLS General C Library
 * Name      : Datei LOG.C
 * Funktion  : Enth„lt Funktionen zur Behandlung von LogFiles
 *
 * Autor     : Patrick Haller [Freitag, 12.08.1994 05.19.14]
 * Mod       : 97/09/17 PH rewrite to OS/2 API
 ***********************************************************************/

#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "toollog.h"


/***********************************************************************
 * Name      : Globale Variablen
 * Bemerkung : VORSICHT bei Mehrfachnutzung !
 *
 * Autor     : Patrick Haller [Freitag, 12.08.1994 05.20.48]
 ***********************************************************************/

static struct Globals
{
  UCHAR         ucLoggingLevel;
  HFILE         hLogFile;
  char          pszLogApp[4];
} Globals;                                        /* Module global variables */


/***********************************************************************
 * Name      : int logopen
 * Funktion  : Initialisieren des Logfiles
 * Parameter : unsigned char loglevel, char *logfilename
 *             UCHAR ucMode - 0 logfile write through
 *                            1         write behind (cache)
 * Variablen :
 * Ergebnis  :
 * Bemerkung : via VA_LIST, 0 - OK, 1 - Fehler
 *
 * Autor     : Patrick Haller [Freitag, 12.08.1994 05.18.45]
 * Mod       : 97/09/17 PH rewrite to OS/2 API
 ***********************************************************************/

APIRET TOOLAPI _Export LogOpen (UCHAR ucLogLevel,
                               PSZ   pszLogFilename,
                               UCHAR ucMode)
{
  APIRET rc;                                               /* API returncode */
  ULONG  ulAction;                               /* action code from DosOpen */

  Globals.ucLoggingLevel = ucLogLevel | LOG_SYSTEM;   /* system messages are */
                                                            /* always logged */

  if (pszLogFilename == NULL)                        /* Parameterberprfung */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */

  switch (ucMode)
  {
    case LOGMODE_WRITE_THROUGH:
      rc = DosOpen(pszLogFilename,                             /* File path name */
                   &Globals.hLogFile,                             /* File handle */
                   &ulAction,                                    /* Action taken */
                   0L,                                /* File primary allocation */
                   FILE_NORMAL,                                /* File attribute */
                   OPEN_ACTION_CREATE_IF_NEW |
                   OPEN_ACTION_OPEN_IF_EXISTS,             /* Open function type */
                   OPEN_FLAGS_NOINHERIT |
                   OPEN_FLAGS_WRITE_THROUGH |     /* YES ! Immediately to disk ! */
                   /* OPEN_FLAGS_NO_CACHE | */
                   OPEN_SHARE_DENYNONE  |/* WARNING ! Maybe OPEN_SHARE_DENYWRITE */
                   OPEN_ACCESS_WRITEONLY,               /* Open mode of the file */
                   0L);                                 /* No extended attribute */
      break;

    case LOGMODE_CACHE:
      rc = DosOpen(pszLogFilename,                             /* File path name */
                   &Globals.hLogFile,                             /* File handle */
                   &ulAction,                                    /* Action taken */
                   0L,                                /* File primary allocation */
                   FILE_NORMAL,                                /* File attribute */
                   OPEN_ACTION_CREATE_IF_NEW |
                   OPEN_ACTION_OPEN_IF_EXISTS,             /* Open function type */
                   OPEN_FLAGS_NOINHERIT |
                   OPEN_SHARE_DENYNONE  |/* WARNING ! Maybe OPEN_SHARE_DENYWRITE */
                   OPEN_ACCESS_WRITEONLY,               /* Open mode of the file */
                   0L);                                 /* No extended attribute */
      break;

    default:
      rc = ERROR_INVALID_FUNCTION;    /* we're called with invalid parameter */
  }


  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                                    /* raise error condition */

  rc = DosSetFilePtr(Globals.hLogFile,        /* jump to the end of the file */
                     0,
                     FILE_END,
                     &ulAction);                        /* use this as dummy */
  if (rc != NO_ERROR)                                    /* check for errors */
  {
    DosClose(Globals.hLogFile);                      /* close the file again */
    Globals.hLogFile = NULLHANDLE;                       /* reset the handle */
  }
  else
    LogPrint(LOG_SYSTEM,
             "LOGS",
             "---[Logfile opened]--------------------------");



  return (rc);                                      /* raise error condition */
}


/***********************************************************************
 * Name      : int logclose
 * Funktion  : Schlieáen des Logfiles
 * Parameter :
 * Variablen :
 * Ergebnis  : 0 - OK, 1 - Fehler
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 12.08.1994 05.18.45]
 * Mod       : 97/09/17 PH rewrite to OS/2 API
 ***********************************************************************/

APIRET TOOLAPI _Export LogClose (void)
{
  LogPrint(LOG_SYSTEM,
           "LOGS",
           "---[Logfile closed]--------------------------\n");

  return(DosClose(Globals.hLogFile));                      /* close the file */
}


/***********************************************************************
 * Name      : void LogPrint
 * Funktion  : Aufzeichnen eines Log-Eintrages
 * Parameter : valist
 * Variablen :
 * Ergebnis  :
 * Bemerkung : Logentries may NOT exceed 512 bytes !
 *             This is to be taken in consideration when calling this
 *             function from threads with a small stack !
 *
 * Autor     : Patrick Haller [Freitag, 12.08.1994 05.31.25]
 * Mod       : 97/09/17 PH rewrite to OS/2 API
 ***********************************************************************/

APIRET TOOLAPI _Export LogPrint (UCHAR ucLogLevel,
                                PSZ   pszApp,
                                PSZ   pszFormat,
                                ...)
{
  va_list ap;
  time_t  timeSystem;
  struct  tm *tmTime;                               /* mapped time structure */

  char    szBuf[512];      /* local buffer for the line to write to the file */
  PSZ     pszBufEnd;                           /* points to the buffer's end */
  PSZ     pszLog;          /* local buffer for the line to write to the file */
  ULONG   ulBytesWritten;           /* dummy for the number of bytes written */
  APIRET  rc;                                              /* API returncode */


  if (Globals.hLogFile == 0)                         /* Parameterberprfung */
    return (ERROR_INVALID_HANDLE);

                /* if logging system has not been properly initialized, quit */

  if (pszFormat == NULL)               /* In diesem Falle wird abgebrochen ! */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */


  if (ucLogLevel & Globals.ucLoggingLevel)                /* if levels match */
  {
    switch (ucLogLevel)                                   /* Klassifizierung */
    {
      case LOG_SYSTEM:    pszLog = "SYS"; break;
      case LOG_ERRORS:    pszLog = "ERR"; break;
      case LOG_CRITICALS: pszLog = "CRT"; break;
      case LOG_DEBUG:     pszLog = "DBG"; break;
      case LOG_WARNINGS:  pszLog = "WRN"; break;
      case LOG_INFOS:     pszLog = "INF"; break;
      default:            pszLog = "???";
    }

    timeSystem = time(NULL);                     /* Datum und Zeit schreiben */
    tmTime = localtime(&timeSystem);                         /* map the time */
    if (tmTime == NULL)                                  /* check for errors */
      return (ERROR_SYS_INTERNAL);                       /* raise error code */

    if (pszApp == NULL)                             /* NULL-Pointer abfangen */
      pszApp = "----";
    
    // stupid fix for Y2K-Bug in IBM CRT
    if (tmTime->tm_year >= 100)
      tmTime->tm_year %= 100;

    sprintf(szBuf,
            "\r\n%02u/%02u/%02u %02u:%02u:%02u³%3s %4s³",
            tmTime->tm_year,
            tmTime->tm_mon + 1,
            tmTime->tm_mday,
            tmTime->tm_hour,
            tmTime->tm_min,
            tmTime->tm_sec,
            pszLog,
            pszApp);
    pszBufEnd = szBuf + strlen(szBuf);          /* points to the buffers end */


    va_start(ap,
             pszFormat);                            /* Den Eintrag schreiben */

    vsprintf(pszBufEnd,
             pszFormat,
             ap);
    pszBufEnd = pszBufEnd + strlen(pszBufEnd);  /* points to the buffers end */

    va_end(ap);

    rc = DosWrite (Globals.hLogFile,                    /* write to the file */
                   szBuf,
                   strlen(szBuf),
                   &ulBytesWritten);
#if 0
    if (rc == NO_ERROR)                            /* if the write succeeded */
      DosResetBuffer(Globals.hLogFile);                  /* flush the buffer */
#endif
  }
  else
    rc = NO_ERROR;                                      /* ignore this write */

  return (rc);                                         /* deliver returncode */
}
