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

#include "error.h"

/*** GLOBALE VARIABLEN ***/
PSZ   pszErrorStrProc = NULL; /* Dieser Pointer soll auf den Namen der
                                 aktuellen Routine zeigen, damit die
                               Fehlerursache festgestellt werden kann.*/
PSZ   pszErrorStrFile = NULL;                   /* Aktives Sourcefile */
ULONG ulErrorStrLine = 0;                       /* Aktive Sourcezeile */

static const PSZ SZLOGAPP = "ERRH";                  /* FÅr's Logfile */


/***********************************************************************
 * Name      : VOID APIENTRY ErrorTerminate
 * Funktion  : Handling Routine fÅr DosExitList-Processing
 * Parameter : ULONG code - der Terminierungsgrund
 * Variablen :
 * Ergebnis  :
 * Bemerkung : Wird benutzt, um Exceptions/Traps aufzuzeichnen.
 *             Daher MUSS die Routine selbst i.O. sein :)
 *
 * Autor     : Patrick Haller [Mittwoch, 06.09.1995 07.32.20]
 ***********************************************************************/

VOID APIENTRY ErrorTerminate (ULONG code)
{
  PSZ pszExitReason;

  switch (code)              /* Welcher Terminierungsgrund liegt vor ? */
  {
    case TC_EXIT:
      pszExitReason = "Normal Exit";
      break;

    case TC_HARDERROR:
      pszExitReason = "Hard-error halt";
      break;

    case TC_TRAP:
      pszExitReason = "Trap operation for a 16-bit child process";
      break;

    case TC_KILLPROCESS:
      pszExitReason = "Unintercepted DosKillProcess";
      break;

    case TC_EXCEPTION:
      pszExitReason = "Exception operation for a 32-bit child process";
      break;

    default:
      pszExitReason = "Undetermined !";
  }

  /* Noch ein bischen Quietschen ! */
  if (code != TC_EXIT)                    /* Aha, kein normaler Abbruch */
  {
    DosBeep (220,50);
    DosBeep (440,50);
    DosBeep (880,50);
    DosBeep (440,50);
    DosBeep (220,50);
  }

    /* Die letzten Aufzeichnungen fÅr's Logfile - beinahe "post mortem" */
  if (pszErrorStrProc != NULL)
    logprint (LOG_ERRORS,SZLOGAPP,"Active routine: [%s] in %s, line %u.",
          pszErrorStrProc,
          pszErrorStrFile,
          ulErrorStrLine);
  logprint (LOG_ERRORS,SZLOGAPP,"Termination: #%u - %s",code,pszExitReason);
  logclose();

                /* Und Bearbeitung terminieren */
  DosExitList (EXLST_EXIT, (PFNEXITLIST) ErrorTerminate);
}


/***********************************************************************
 * Name      : VOID APIENTRY ErrorInfo
 * Funktion  : Ermittelt erweiterte Fehlerinformationen
 * Parameter : ULONG code - der Fehlercode
 * Variablen :
 * Ergebnis  :
 * Bemerkung : Wird benutzt, um Exceptions/Traps aufzuzeichnen.
 *
 * Autor     : Patrick Haller [Mittwoch, 06.09.1995 07.32.20]
 ***********************************************************************/

APIRET ErrorInfo (ULONG ulCode)
{
  APIRET rc;                                          /* RÅckgabewert */
  ULONG  ulClass;                                     /* Fehlerklasse */
  ULONG  ulAction;       /* Welche Aktion kînnte unternommen werden ? */
  ULONG  ulLocation;               /* Wo ist der Fehler aufgetreten ? */

  INFO("ErrorInfo");                               /* Moduldeklaration */


  rc = DosErrClass (ulCode, &ulClass, &ulAction, &ulLocation);
  ERRINFO(rc);                        /* Ist ein Fehler aufgetreten ? */
  logprint (LOG_WARNINGS,SZLOGAPP,
       "ErrorInfo (%u): Class=%u, Action=%u, Location=%u.",
       ulCode,
       ulClass,
       ulAction,
       ulLocation);

  return (rc);
}


/***********************************************************************
 * Name      : APIRET ErrorInst
 * Funktion  : Handling Routine fÅr DosExitList-Processing installieren
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung : Wichtig fÅr's Post Mortem Debugging
 *
 * Autor     : Patrick Haller [Mittwoch, 06.09.1995 07.32.20]
 ***********************************************************************/

APIRET ErrorInst (void)
{
   APIRET        rc;                                    /* Return code */

   INFO ("ErrorInst");                             /* Moduldeklaration */

   rc = DosError (2);                    /* Harderr Popups ausschalten */
   ERRINFO (rc);                                   /* Ist ein Fehler aufgetreten ? */

   rc = DosExitList (EXLST_ADD | 0x00009a00, (PFNEXITLIST) ErrorTerminate);
   ERRINFO (rc);                                   /* Ist ein Fehler aufgetreten ? */

   return (rc);                                /* RÅckgabewert liefern */
}



/**********************************************************************/
/*------------------------------- Msg --------------------------------*/
/*                                                                    */
/*  DISPLAY A MESSAGE TO THE USER.                                    */
/*                                                                    */
/*  INPUT: a message in printf format with its parms                  */
/*                                                                    */
/*  1. Format the message using vsprintf.                             */
/*  2. Sound a warning sound.                                         */
/*  3. Display the message in a message box.                          */
/*                                                                    */
/*  OUTPUT: nothing                                                   */
/*                                                                    */
/*--------------------------------------------------------------------*/
/**********************************************************************/

#define MESSAGE_SIZE 1024

VOID ErrorMsg( PSZ szFormat,... )
{
    PSZ     szMsg;
    va_list argptr;

    if( (szMsg = (PSZ) malloc( MESSAGE_SIZE )) == NULL )
    {
   DosBeep( 1000, 1000 );
   return;
    }

    va_start( argptr, szFormat );
    vsprintf( szMsg, szFormat, argptr );
    va_end( argptr );

    szMsg[ MESSAGE_SIZE - 1 ] = 0;
    (void) WinAlarm( HWND_DESKTOP, WA_WARNING );
    (void) WinMessageBox(  HWND_DESKTOP, HWND_DESKTOP, szMsg,
            SZLOGAPP, 1, MB_OK | MB_MOVEABLE );
    free( szMsg );
    return;
}

