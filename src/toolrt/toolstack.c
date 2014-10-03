/***********************************************************************
 * Projekt   : OS2TOOLS General C Library
 * Name      : Module Stack
 * Funktion  : Implementation eines abstrakten Stackobjektes.
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [(null), 19.05.1996 22.36.34]
 ***********************************************************************/


/****************** Include-Dateien *************************************/
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "toolstack.h"


/************************************************************************/

/************************************************************************
 * Funktion:         APIRET StackPush(PSTACK pStack,
 *                                    PVOID  pData,
 *                                    ULONG  ulDataLength)
 *
 * Kurzbeschreibung: Legt einen Datenblock auf dem Stack ab
 *
 * Ergebnis:            NO_ERROR:
 *                         String wurde auf Stack abgelegt. Stack
 *                         ist noch nicht voll, Aufruf wurde korrekt
 *                         abgearbeitet.
 *                      ERROR_INVALID_PARAMETER:
 *                         Funktion wurde mit ungueltigen Werten
 *                         aufgerufen.
 *                      ERROR_NOT_ENOUGH_MEMORY:
 *                         Es konnte kein Speicher allokiert werden.
 *
 *
 * Import-Parameter:    PSTACK pStack       - Der benutzte Stack
 *                      PVOID  pData        - der Datenblock
 *                      ULONG  ulDataLength - Laenge des Datenblockes
 *
 * Export-Parameter:    PSTACK pStack       - Der benutzte Stack
 *
 * Modul-globale Variablen:
 *
 ************************************************************************/
APIRET TOOLAPI _Export StackPush(PSTACK pStack,
                        PVOID  pData,
                        ULONG  ulDataLength)
{
  PSTACKELEMENT pElement;                /* Zeiger auf ein Stackelement */

  if ( (pData        == NULL) ||               /* Parameterueberprfung */
       (pStack       == NULL) ||
       (ulDataLength == 0) )
     return (ERROR_INVALID_PARAMETER);
                                   /* Fehlerhafter Parameter -> Abbruch */

                        /* Speicher fuer die Elementstruktur allokieren */
  pElement = (PSTACKELEMENT)malloc (sizeof(STACKELEMENT));
  if (pElement == NULL)       /* Konnte der Speicher allokiert werden ? */
    return (ERROR_NOT_ENOUGH_MEMORY);                          /* NEIN! */

                    /* Jetzt den allokierten Block mit Daten auffuellen */
  pElement->pData = malloc(ulDataLength);
  if (pElement->pData == NULL) /* Konnte der Speicher allokiert werden? */
  {
    free (pElement);             /* Zuvor angelegtes Element aufraeumen */
    return (ERROR_NOT_ENOUGH_MEMORY);                          /* NEIN! */
  }

              /* Ab hier sind beide Speicherbereiche korrekt allokiert. */
  memcpy (pElement->pData, pData, ulDataLength);      /* Daten kopieren */
                                         /* Zeiger auf letztes Element  */
  pElement->pPreviousElement = pStack->pStackElement;
  pElement->ulDataLength = ulDataLength; /* Groesse ebenfalls eintragen */
  pStack->pStackElement = pElement;      /* Stack-Pointer aktualisieren */
  pStack->ulElements++;                     /* Elementezaehler erhoehen */

  return NO_ERROR;                              /* Rckgabewert liefern */
}


/************************************************************************
 * Funktion:         APIRET StackPop (PSTACK pStack,
 *                                    PVOID  pData,
 *                                    ULONG  ulDataLength)
 *
 * Kurzbeschreibung: Holt einen Datenblock vom Stack ab.
 *
 * Ergebnis:            NO_ERROR:
 *                        String wurde auf Stack abgelegt. Stack
 *                        ist noch nicht voll, Aufruf wurde korrekt
 *                        abgearbeitet.
 *                      ERROR_INVALID_PARAMETER:
 *                         Funktion wurde mit ungueltigen Werten
 *                         aufgerufen.
 *                      ERROR_STACK_UNDERFLOW:
 *                         Ein Pop wurde auf einem leeren Stack ausgefuehrt.
 *                      ERROR_BUFFER_OVERFLOW:
 *                         Wenn das eingetragene Datenelement groesser
 *                         als der Zielpuffer ist.
 *
 *
 * Import-Parameter:    PSTACK pStack        - Der benutzte Stack
 *                      PULONG pulDataLength - Groesse des Datenpuffers
 *
 * Export-Parameter:    PSTACK pStack        - Der benutzte Stack
 *                      PVOID  pData         - Zeiger auf den Rueckgabepuffer
 *                      PULONG pulDataLength - Groesse des Rueckgabepuffers
 *
 * Modul-globale Variablen:
 *
 ************************************************************************/
APIRET TOOLAPI _Export StackPop (PSTACK pStack,
                        PVOID  pData,
                        PULONG pulDataLength)
{
  PSTACKELEMENT pStackDummy;    /* Zwischenspeichern des Elementzeigers */

  if ( (pData         == NULL) ||              /* Parameterueberprfung */
       (pulDataLength == NULL) )
     return (ERROR_INVALID_PARAMETER);
                                   /* Fehlerhafter Parameter -> Abbruch */

  pStackDummy = pStack->pStackElement;           /* Stackelement merken */

                                           /* Stack-Underflow abpruefen */
  if (pStackDummy == NULL)                      /* Ist der Stack leer ? */
    return (ERROR_NO_MORE_FILES);            /* Underflow signalisieren */

  if (*pulDataLength >= pStackDummy->ulDataLength)   /* Groesse pruefen */
  {
     memcpy(pData,                                       /* Zieladresse */
       pStackDummy->pData,                              /* Quelladresse */
       pStackDummy->ulDataLength);      /* Daten zum Ziel transferieren */

     *pulDataLength = pStackDummy->ulDataLength;/*Kopierte Bytes melden */

                 /* Jetzt die Pointer und Speicherobjekte aktualisieren */
     pStack->pStackElement = pStackDummy->pPreviousElement;
     pStack->ulElements--;               /* Elementezaehler erniedrigen */
                                           /* Speicherobjekte freigeben */
     free (pStackDummy->pData);           /* pData kann nicht NULL sein */
     free (pStackDummy);               /* Stackelement selbst freigeben */

     return (NO_ERROR);                      /* Bearbeitung erfolgreich */
  }
  else      /* Datenpuffer der aufrufenden Funktion ist zu klein, daher */
               /* wird ein Fehlercode geliefert und gemeldet, wie gross */
                          /* der Puffer mindestens haette sein muessen. */
  {
    *pulDataLength = pStackDummy->ulDataLength;   /* min. Puffergroesse */
    return (ERROR_BUFFER_OVERFLOW);    /* Pufferueberlauf signalisieren */
  }
}


/************************************************************************
 * Funktion:            APIRET StackInit (PSTACK pStack)
 *
 * Kurzbeschreibung:    Initialisiert die internen Stackstrukturen.
 *
 * Ergebnis:            NO_ERROR: alles i.O.
 *                      ERROR_INVALID_PARAMETER: interner Fehler.
 *
 * Import-Parameter:    PSTACK   pStack - Der benutzte Stack
 *
 * Export-Parameter:    -
 *
 * Modul-globale Variablen: -
 *
 ************************************************************************/
APIRET TOOLAPI _Export StackInit (PSTACK pStack)
{
  if (pStack == NULL)                        /* Parametereueberpruefung */
    return (ERROR_INVALID_PARAMETER);           /* Fehler signalisieren */

    /* Das ganze Hexenwerk besteht darin, das erste Element auf NULL zu */
    /* initialisieren. StackInit() dient der Kapselung der Stack-API.   */
  pStack->pStackElement = (PSTACKELEMENT)NULL;
  pStack->ulElements    = 0;          /* Elementezaehler initialisieren */

  return (NO_ERROR);                           /* Rueckgabewert liefern */
}


/************************************************************************
 * Funktion:         APIRET StackFlush (PSTACK pStack,
 *                                      PVOID  pData,
 *                                      ULONG  ulDataLength)
 *
 * Kurzbeschreibung: Entleert einen Stack vollstaendig.
 *
 * Ergebnis:            NO_ERROR:
 *                        String wurde auf Stack abgelegt. Stack
 *                        ist noch nicht voll, Aufruf wurde korrekt
 *                        abgearbeitet.
 *                      ERROR_INVALID_PARAMETER:
 *                         Funktion wurde mit ungueltigen Werten
 *                         aufgerufen.
 *
 * Import-Parameter:    PSTACK pStack        - Der benutzte Stack
 *
 * Export-Parameter:    PSTACK pStack        - Der benutzte Stack
 *
 * Modul-globale Variablen:
 *
 ************************************************************************/
APIRET TOOLAPI _Export StackFlush (PSTACK pStack)
{
  PSTACKELEMENT pStackDummy;    /* Zwischenspeichern des Elementzeigers */

  if ( pStack == NULL)                         /* Parameterueberprfung */
     return (ERROR_INVALID_PARAMETER);
                                   /* Fehlerhafter Parameter -> Abbruch */

  pStackDummy = pStack->pStackElement;           /* Stackelement merken */

  while (pStackDummy != NULL)                   /* Ist der Stack leer ? */
  {
                 /* Jetzt die Pointer und Speicherobjekte aktualisieren */
     pStack->pStackElement = pStackDummy->pPreviousElement;
                                           /* Speicherobjekte freigeben */
     free (pStackDummy->pData);           /* pData kann nicht NULL sein */
     free (pStackDummy);               /* Stackelement selbst freigeben */

     pStackDummy = pStack->pStackElement;        /* Stackelement merken */
  }

  pStack->ulElements = 0;          /* Elementezaehler re-initialisieren */
  return (NO_ERROR);                                    /* return value */
}


/************************************************************************
 * Funktion:         APIRET StackIsEmpty (PSTACK pStack)
 *
 * Kurzbeschreibung: Checks if the stack is empty.
 *
 * Ergebnis:            TRUE or FALSE
 *
 * Import-Parameter:    PSTACK pStack        - Der benutzte Stack
 *
 * Export-Parameter:    PSTACK pStack        - Der benutzte Stack
 *
 * Modul-globale Variablen:
 *
 ************************************************************************/
BOOL TOOLAPI _Export StackIsEmpty (PSTACK pStack)
{
  if ( pStack == NULL )                        /* Parameterueberprfung */
     return (TRUE);  /* CAUTION ! This is not a proper error handling ! */

  return (pStack->pStackElement == NULL);/* Return current stack status */
}


/************************************************************************
 * Funktion:            void StackDump (PSTACK pStack,
 *                                      FILE   *fFile)
 *
 * Kurzbeschreibung:    Gibt den Inhalt eines Stacks auf ein FILE aus.
 *
 * Ergebnis:            -
 *
 * Import-Parameter:    PSTACK pStack - Der benutzte Stack
 *                      FILE   *fFile - Ausgabedatei
 *
 * Export-Parameter:    -
 *
 * Modul-globale Variablen: -
 *
 * Bemerkung:           Dient vorrangig zum Debugging des Stackmodules.
 *
 ************************************************************************/
void TOOLAPI _Export StackDump (PSTACK pStack, 
                       FILE   *fFile)
{
  PSTACKELEMENT pTemp;              /* Iterator ueber die Stackelemente */

  fprintf (fFile,"\nStackdump: (%u Elements)"
                 "\nAddress--Length---Data------------------------------",
                 pStack->ulElements );
                                                 /* Titelzeile ausgeben */

  for (pTemp = pStack->pStackElement;    /* Start beim obersten Element */
       pTemp != NULL;           /* Solange noch Elemente vorhanden sind */
       pTemp = pTemp->pPreviousElement)          /* Tiefer in den Stack */

    fprintf (fFile,"\n%08x %8u (%s)",
                   pTemp,                      /* Adresse des Elementes */
                   pTemp->ulDataLength,         /* Laenge des Eintrages */
                   pTemp->pData);                 /* Der Eintrag selbst */
}
