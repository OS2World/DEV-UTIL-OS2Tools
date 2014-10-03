/***********************************************************************
 * Name      : Datei LOG.C
 * Funktion  : EnthÑlt Funktionen zur Behandlung von LogFiles
 *
 * Autor     : Patrick Haller [Freitag, 12.08.1994 05.19.14]
 ***********************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "log.h"

/***********************************************************************
 * Name      : Globale Variablen
 * Bemerkung : VORSICHT bei Mehrfachnutzung !
 *
 * Autor     : Patrick Haller [Freitag, 12.08.1994 05.20.48]
 ***********************************************************************/

unsigned char LoggingLevel;
FILE         *LogFile;
char          LogApp[4] = "LOG\0";


/***********************************************************************
 * Name      : int logopen
 * Funktion  : Initialisieren des Logfiles
 * Parameter : unsigned char loglevel, char *logfilename
 * Variablen :
 * Ergebnis  :
 * Bemerkung : via VA_LIST, 0 - OK, 1 - Fehler
 *
 * Autor     : Patrick Haller [Freitag, 12.08.1994 05.18.45]
 ***********************************************************************/

int logopen (unsigned char loglevel, char *logfilename)
{
  LoggingLevel = loglevel;

  if (logfilename == NULL)                     /* ParameterÅberprÅfung */
    return (LOG_ERROR);

  LogFile = fopen (logfilename,"a+");            /* append the logfile */
  if (!LogFile)
    return LOG_ERROR;
  else
  {
    fprintf (LogFile,"\n");                   /* erst mal ein Linefeed */

#ifdef DEBUG
    logprint (LOG_INFOS,LogApp,"LogOpen (%u,%s) OK.",((unsigned int)loglevel),logfilename);
#endif

    return NO_ERROR;
  }
}


/***********************************************************************
 * Name      : int logclose
 * Funktion  : Schlie·en des Logfiles
 * Parameter :
 * Variablen :
 * Ergebnis  : 0 - OK, 1 - Fehler
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 12.08.1994 05.18.45]
 ***********************************************************************/

int logclose (void)
{

#ifdef DEBUG
  logprint (LOG_INFOS,LogApp,"LogClose.");
#endif

  if (LogFile == NULL)                         /* ParameterÅberprÅfung */
      return LOG_ERROR;


  if(!fclose (LogFile))
  {
    logprint (LOG_INFOS,LogApp,"LogClose failed.");
    return LOG_ERROR;
  }
  return NO_ERROR;
}


/***********************************************************************
 * Name      : void logprint
 * Funktion  : Aufzeichnen eines Log-Eintrages
 * Parameter : valist
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 12.08.1994 05.31.25]
 ***********************************************************************/

void logprint (unsigned char LogLevel,char *app, char *format, ...)
{
  va_list ap;
  time_t  t;
  char    *szBuf;
  char    szTime[30];

  if (LogFile == NULL)                         /* ParameterÅberprÅfung */
      return;


  if (LogLevel & LoggingLevel)
  {
    switch (LogLevel)                              /* Klassifizierung */
    {
      case LOG_ERRORS:    szBuf = "ERR"; break;
      case LOG_CRITICALS: szBuf = "CRT"; break;
      case LOG_WARNINGS:  szBuf = "WRN"; break;
      case LOG_INFOS:  	  szBuf = "INF"; break;
      default:	          szBuf = "BAD";
    }

    t = time(NULL);                       /* Datum und Zeit schreiben */
    strcpy (szTime,ctime(&t));
    szTime[strlen(szTime)-1] = 0;                /* Strip trailing \0 */

    if (app == NULL)                         /* NULL-Pointer abfangen */
      app = "----";

    fprintf(LogFile, "\n%s %3s %4s ",szTime, szBuf, app);

    if (format == NULL)         /* In diesem Falle wird abgebrochen ! */
      return;


    va_start(ap, format);                    /* Den Eintrag schreiben */
    vfprintf(LogFile, format, ap);
    va_end(ap);
  }
}
