/***********************************************************************
 * Name      : Modul ERROR
 * Funktion  : Fehlerbehandlungsmodul
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 05.09.1995 22.50.30]
 ***********************************************************************/

#ifndef MODULE_ERROR
#define MODULE_ERROR

#define INCL_DOSEXCEPTIONS   /* Exception values */
#define INCL_DOSPROCESS
#define INCL_DOSMISC
#define INCL_WINDIALOGS
#include <os2.h>
#include <stdarg.h>
#include <stdio.h>
#include <malloc.h>

#include "log.h"


/*** GLOBALE VARIABLEN ***/
extern PSZ pszErrorStrProc; /* Dieser Pointer soll auf den Namen der
			       aktuellen Routine zeigen, damit die
			       Fehlerursache festgestellt werden kann.*/
extern PSZ   pszErrorStrFile;                   /* Aktives Sourcefile */
extern ULONG ulErrorStrLine;                    /* Aktive Sourcezeile */
/* Vorsicht, kann irrefÅhrend sein, da keine Rekursionen aufgelîst werden
   kînnen ! D.h. auch wenn eine Unterroutine korrekt zurÅckgekehrt ist,
   kann sie als Fehlerursache angegeben sein. */


VOID APIENTRY ErrorTerminate (ULONG code);
APIRET        ErrorInfo      (ULONG ulCode);
APIRET        ErrorInst      (void);
VOID          ErrorMsg       (PSZ,... );

/*** DEBUGGING MACROS ***/
#ifdef DEBUG
  /*** LOGGING ***/
  #define LOG(a,b) logprint(LOG_ERRORS,SZLOGAPP,a,b); \
		   logprint(LOG_ERRORS,SZLOGAPP,"Occured in %s, %05u", \
				       __FILE__, __LINE__); \
		   ErrorMsg (a,b);


  /*** ParameterÅberprÅfung ***/
  #define CHECKPARAM(a) if (a) { \
			  logprint(LOG_ERRORS,SZLOGAPP,"Check param: %s, #%05u", \
			  __FILE__,__LINE__); \
			  return (ERROR_INVALID_PARAMETER); }

  /*** MODULDEKLARATION ***/
  /* Ohne Log Aufzeichnung */
  #define INFONL(a) pszErrorStrProc=(a); \
		  pszErrorStrFile= __FILE__; \
		  ulErrorStrLine = __LINE__;
  /* Mit Log Aufzeichnung */
  #define INFO(a) INFONL(a) \
		  logprint(LOG_INFOS,SZLOGAPP,"%s, #%05u: %s", \
			   pszErrorStrFile,ulErrorStrLine,pszErrorStrProc);

#else /*** oder sonst ... ***/
  #define LOG(a,b) ;
  #define CHECKPARAM(a) ;
  #define INFONL(a) ;
  #define INFO(a) ;
#endif


/*** ERRINFO - erweiterte Fehlerinformation ***/
#define ERRINFO(a) if ( (a) != NO_ERROR) { \
		   logprint(LOG_WARNINGS,SZLOGAPP,"Runtime error: %u",(a)); \
		   logprint(LOG_ERRORS,SZLOGAPP,"Occured in %s, line %u", \
				       __FILE__, __LINE__); \
		   ErrorInfo(a); }

#define HWNDERR( hwnd )         \
	    (ERRORIDERROR( WinGetLastError( WinQueryAnchorBlock( hwnd ) ) ))

#endif MODULE_ERROR
