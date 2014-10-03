/***********************************************************************
 * Name      : Datei LOG.H
 * Funktion  : EnthÑlt Funktionen zur Behandlung von LogFiles
 *
 * Autor     : Patrick Haller [Freitag, 12.08.1994 05.19.14]
 ***********************************************************************/

#ifndef LOG_CLASS
#define LOG_CLASS

typedef char LOGAPPNAME[5];

/* Konstanten fÅr die LoggingLevels */
#define LOG_ERRORS	8
#define LOG_CRITICALS   4
#define LOG_WARNINGS    2
#define LOG_INFOS       1

#define LOG_ALL   LOG_ERRORS | LOG_CRITICALS | LOG_WARNINGS | LOG_INFOS

/* Fehlercodes */
#define NO_ERROR	0
#define LOG_ERROR	1

int  logopen  (unsigned char loglevel, char *logfilename);
int  logclose (void);
void logprint (unsigned char LogLevel, char *app, char *format, ...);

#endif
