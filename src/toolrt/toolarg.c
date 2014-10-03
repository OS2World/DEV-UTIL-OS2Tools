/*****************************************************************************
 * Projekt   : OS2TOOLS General C Library
 * Name      : Arguments
 * Funktion  : Flexible Behandlung von Kommandozeilenparametern
 *
 * Autor     : Patrick Haller [Dienstag, 26.09.1995 17.57.39]
 *****************************************************************************/

#define SZLOGAPP "ARGS"
#define DEBUG_ENABLE

/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "tooltypes.h"
#include "toolerror.h"
#include "toolarg.h"
#include "tools.h"


/* Beispieltabelle der allgemeinen Parameter
ARGUMENT TabArguments[] =
{ Token--Beschreibung--pTarget--ucTargetFormat--pTargetSpecified--
  {"/COM:", "<COM-Port-Number>,    e.g. /COM:2",      &ucCOMPort,    ARG_UCHAR, &fsCOMPort},
  {"/ASC:" ,"<data string>,        e.g. /ASC:atz0",   &pszData,      ARG_PSZ,   &fsData},
  {"/DEC:" ,"<decimal data string>,e.g. /DEC:010 013",&pszDataDec,   ARG_PSZ,   &fsDataDec},
  {"/Q",    "Quiet Operation, no output.",            NULL,          ARG_NULL,  &fsQuiet},
  {"/?",    "Get help screen.",                       NULL,          ARG_NULL,  &fsHelp},
  {"/H",    "Get help screen.",                       NULL,          ARG_NULL,  &fsHelp},
  ARG_TERMINATE
};
*/


/*****************************************************************************
 * Name      : APIRET ArgHelp
 * Funktion  : Gibt Hilfetext aus.
 * Parameter : PARGCOMMAND pCommands
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 07.10.1995 03.22.44]
 *****************************************************************************/

APIRET TOOLAPI _Export ArgHelp (PARGUMENT pArguments)
{
  PARGUMENT   pArg;                                          /* Laufvariable */

  if (pArguments == NULL)                            /* Parameter갶erpr갽ung */
    return(ERROR_TOOLS_INVALID_PARAMETER);          /* raise error condition */

  printf ("\nCommand            Description"
          "\n컴컴컴컴컴컴컴컴컴 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴");

  for (pArg = pArguments;                            /* Bearbeitungsschleife */
       pArg->pszToken != NULL;
       pArg++)
  {
    if (!(pArg->ucTargetFormat & ARG_HIDDEN))           /* argument hidden ? */
      if (pArg->ucTargetFormat & ARG_DEFAULT)  /* check if default attribute */
        printf ("\n(nr. #%-9s)   %s",
                pArg->pszToken,
                pArg->pszDescription);
      else
        printf ("\n%-18s %s",                                     /* Ausgabe */
                pArg->pszToken,
                pArg->pszDescription);
  }

  return (NO_ERROR);                                                   /* OK */
} /* ArgHelp */


/*****************************************************************************
 * Name      : BOOL ArgMapToTarget
 * Funktion  : Eintragen eines ARGUMENTes in die Zielvariable.
 * Parameter : PARGUMENT pArg, PVOID pData
 * Variablen :
 * Ergebnis  : Erfolgscode TRUE / FALSE
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 07.10.1995 14.11.19]
 * Mod       : 1998/05/14 PH ARG_HIDDEN wurde falsch behandelt
 *****************************************************************************/

BOOL TOOLAPI _Export ArgMapToTarget (PARGUMENT pArg,
                                    void *pData)
{
  if (pArg == NULL)                                  /* Parameter갶erpr갽ung */
    return (FALSE);

                                              /* M걌sen Daten vorhanden sein */
  if ( (pArg->ucTargetFormat & ARG_MASK_TYPE) != ARG_NULL)
  {
    if (pData == NULL)                                      /* dieses pr갽en */
      return (FALSE);

    if (*(char*)pData == 0)                             /* und dieses pr갽en */
      return (FALSE);
  }

                                                     /* Zielformat ermitteln */
  switch (pArg->ucTargetFormat & ARG_MASK_TYPE)             /* mask the bits */
  {
    case ARG_NULL:                         /* Keine Zieleintragung vornehmen */
      break;

    case ARG_CHAR:                                       /* Zielformat: CHAR */
      {
        char *pChar = (pArg->pTarget);                   /* Zwischenvariable */
        *pChar = (char)StrToNumber(pData,
                                   TRUE);                       /* Eintragen */
      }
      break;

    case ARG_UCHAR:                                     /* Zielformat: UCHAR */
      {
        unsigned char *pChar = (pArg->pTarget);          /* Zwischenvariable */
        *pChar = (unsigned char)StrToNumber(pData,
                                            FALSE);             /* Eintragen */
      }
      break;


    case ARG_SHORT:                                     /* Zielformat: SHORT */
      {
        short *pShort = (pArg->pTarget);                 /* Zwischenvariable */
        *pShort = (short)StrToNumber(pData,
                                     TRUE);                     /* Eintragen */
      }
      break;

    case ARG_USHORT:                                   /* Zielformat: USHORT */
      {
        unsigned short *pUShort = (pArg->pTarget);       /* Zwischenvariable */
        *pUShort = (unsigned short)StrToNumber(pData,
                                               FALSE);          /* Eintragen */
      }
      break;

    case ARG_LONG:                                       /* Zielformat: LONG */
      {
        long *pLong = (pArg->pTarget);                   /* Zwischenvariable */
        *pLong = (long)StrToNumber(pData,
                                   TRUE);                       /* Eintragen */
      }
      break;

    case ARG_ULONG:                                     /* Zielformat: ULONG */
      {
        unsigned long *pULong = (pArg->pTarget);         /* Zwischenvariable */
        *pULong = (unsigned long)StrToNumber(pData,
                                             FALSE);            /* Eintragen */
      }
      break;

    case ARG_PSZ:                                       /* Zielformat: ULONG */
      {                   /* Es wird KEINE Kopie des Quellstrings angelegt ! */
        char **ppszStr = (pArg->pTarget);                   /* Hilfevariable */
        *ppszStr = (char *)pData;
      }
      break;

    default:
      return (FALSE);                                     /* Unbekannter Typ */
  }

                     /* Wenn bisherige Aktionen von Erfolg gekr봭t waren ... */
  // *(pArg->pTargetSpecified)++;
  *(pArg->pTargetSpecified) = TRUE;

  return (TRUE);                                     /* R갷kgabewert liefern */
} /* ArgMapToTarget */


/*****************************************************************************
 * Name      : PARGCOMMAND ArgParse
 * Funktion  : Kommanzozeilenparameterauswertung
 * Parameter : int argc, char *argv[]
 * Variablen :
 * Ergebnis  : Fehlercode : 0 - alles OK
 *                         -1 - Fehlerhafter Funktionsparameter
 *                      sonst - Nr. des Parameters n (argv[n]), der
 *                              fehlerhaft ist.
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 07.10.1995 02.23.01]
 *****************************************************************************/

APIRET TOOLAPI _Export ArgParse (int       argc,
                                char      *argv[],
                                PARGUMENT pTabArguments)
{
  PARGUMENT     pArg = NULL;
  int           fSuccess = TRUE;                             /* R갷kgabewert */
  char         *p;
  unsigned long ulArg;
  unsigned long ulTokenLength;                  /* length of parameter token */
  unsigned long ulDefaultCounter;           /* counter for default arguments */
  unsigned long ulDefaultMap;            /* default argument index to map to */

  if (pTabArguments == NULL)                         /* Parameter갶erpr갽ung */
    return (ERROR_TOOLS_INVALID_PARAMETER);          /* Fehler signalisieren */


  for (ulArg            = 1,
       ulDefaultCounter = 0,                /* no default argument found yet */
       p                = argv[1];              /* Alle Parameter abarbeiten */

       ((int)ulArg < argc) && (fSuccess == TRUE);

       ulArg++,
       p = argv[ulArg])
  {
                              /* Erst allgemeine, interne Kommandos abpr갽en */
    for (pArg = pTabArguments;
         (pArg->ucTargetFormat != ARG_ENDOFLIST);
         pArg++)
    {
      ulTokenLength = strlen(pArg->pszToken);   /* query the length of token */

      if (pArg->ucTargetFormat & ARG_DEFAULT)     /* check default attribute */
      {
        if (*(PARGFLAG)pArg->pTargetSpecified == 0) /* and not yet specified */
        {
          ulDefaultCounter++;                    /* increase default counter */
          ulDefaultMap = atoi(pArg->pszToken);        /* query default index */

          if ( ( ulDefaultMap == 0) ||    /* check if argument numbers match */
               ( ulDefaultMap == ulDefaultCounter) )
          {
            ulTokenLength = 0;                     /* parameter has no label */
            break;                                         /* leave the loop */
          }
        }
      }
      else
      {
                                                       /* without parameters */
        if ( (pArg->ucTargetFormat & ARG_MASK_TYPE) == ARG_NULL)
        {
          if ( stricmp (p,                                /* full comparsion */
                        pArg->pszToken) == 0)
            break;
        }
        else
        {
          if ( strnicmp (p,                    /* else check if labels match */
                         pArg->pszToken,
                         ulTokenLength
                ) == 0)                               /* Kommando gefunden ? */
            break;
        }
      }
    }

    if (pArg->ucTargetFormat == ARG_ENDOFLIST)        /* G걄tiges Kommando ? */
      fSuccess = FALSE;                /* Kein brauchbares Kommando gefunden */
    else                         /* Wir haben ein g걄tiges Kommando gefunden */
      fSuccess = ArgMapToTarget (pArg,
                                 p + ulTokenLength);
  }

  if (fSuccess == TRUE)               /* Wenn wir erfolgreich waren, dann OK */
    return (NO_ERROR);                         /* R갷kgabewert zur갷kliefern */
  else
    return ((int)ulArg-1);                     /* R갷kgabewert zur갷kliefern */
}


/*****************************************************************************
 * Name      : APIRET ArgGetMissing
 * Funktion  : Kommanzozeilenparameterauswertung
 * Parameter : PARGUMENT pTabArguments
 * Variablen :
 * Ergebnis  : Fehlercode : 0 - alles OK
 *                         -1 - Fehlerhafter Funktionsparameter
 *                      sonst - Nr. des Parameters n (argv[n]), der
 *                              fehlerhaft ist.
 * Bemerkung : ARG_STRING will be allocated automatically on the heap
 *
 * Autor     : Patrick Haller [Mittwoch, 1997/03/19 02.23.01]
 *****************************************************************************/

APIRET TOOLAPI _Export ArgGetMissing (PARGUMENT pTabArguments)
{
  PARGUMENT     pArg = NULL;
  int           fSuccess = TRUE;                             /* return value */
  char          pszInput[256];             /* reserve 256 bytes input buffer */
  char          *pszDummy;  /* to hold a copy of the input string if ARG_PSZ */
  unsigned long ulArg = 0;                          /* argument index number */
  char          *pszTokenType;        /* points to description of token type */
  int           iAnswer;                           /* for boolean parameters */

  if (pTabArguments == NULL)                         /* Parameter갶erpr갽ung */
    return (ERROR_TOOLS_INVALID_PARAMETER);          /* Fehler signalisieren */


  for (pArg = pTabArguments;
       (pArg->ucTargetFormat != ARG_ENDOFLIST) &&
       (fSuccess == TRUE);
       pArg++)
  {
    if ( (pArg->ucTargetFormat & ARG_MUST) &&          /* mandatory argument */
         (*(PARGFLAG)pArg->pTargetSpecified == 0) ) /* and not yet specified */
    {
      switch (pArg->ucTargetFormat & ARG_MASK_TYPE)          /* get the type */
      {
        case ARG_NULL:   pszTokenType = "Boolean Flag";   break;
        case ARG_CHAR:   pszTokenType = "Signed Char";    break;
        case ARG_UCHAR:  pszTokenType = "Unsigned Char";  break;
        case ARG_SHORT:  pszTokenType = "Signed Short";   break;
        case ARG_USHORT: pszTokenType = "Unsigned Short"; break;
        case ARG_LONG:   pszTokenType = "Signed Long";    break;
        case ARG_ULONG:  pszTokenType = "Unsigned Long";  break;
        case ARG_PSZ:    pszTokenType = "String";         break;
        default:         pszTokenType = "(unknown type)"; break;
      }

      printf ("\n%s %s\n(%s):",                         /* this is out prompt */
              pArg->pszToken,
              pArg->pszDescription,
              pszTokenType);

      if (pArg->ucTargetFormat & ARG_NULL)   /* here we need other functions */
        iAnswer = ToolsConfirmationQuery();            /* get boolean answer */
      else
      {
        iAnswer = 1;                                                  /* YES */

        do
        {
          gets (pszInput);                               /* query the string */
        }
        while (pszInput[0] == 0);         /* we ultimately want some input ! */
      }

      if ( pArg->ucTargetFormat & ARG_PSZ)                       /* string ? */
      {
        pszDummy = strdup(pszInput);          /* copy the string to the heap */
        if (pszDummy == NULL)               /* check if allocation succeeded */
          return (ERROR_TOOLS_NOT_ENOUGH_MEMORY);            /* signal error */

        fSuccess = ArgMapToTarget (pArg,                 /* map dummy string */
                                   pszDummy);
      }
      else
        if (iAnswer == 1)                         /* check for boolean flags */
          fSuccess = ArgMapToTarget (pArg,
                                     pszInput);
    }
  }

  if (fSuccess == TRUE)               /* Wenn wir erfolgreich waren, dann OK */
    return (NO_ERROR);                         /* R갷kgabewert zur갷kliefern */
  else
    return ((int)ulArg-1);                     /* R갷kgabewert zur갷kliefern */
}


/*****************************************************************************
 * Name      : APIRET ArgIsArgumentMissing
 * Funktion  : Check if mandatory argument is missing
 * Parameter : PARGUMENT pTabArguments
 * Variablen :
 * Ergebnis  : Fehlercode : 0 - alles OK
 *                         -2 - Problem
 *                      sonst - Nr. des Parameters n (argv[n]), der
 *                              fehlerhaft ist.
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Mittwoch, 1997/03/19 02.23.01]
 *****************************************************************************/

APIRET TOOLAPI _Export ArgIsArgumentMissing (PARGUMENT pTabArguments)
{
  PARGUMENT     pArg = NULL;
  unsigned long ulArg;

  if (pTabArguments == NULL)                         /* Parameter갶erpr갽ung */
    return (ERROR_TOOLS_INVALID_PARAMETER);          /* Fehler signalisieren */


  for (pArg  = pTabArguments,
       ulArg = 1;

       (pArg->ucTargetFormat != ARG_ENDOFLIST);

       pArg++,
       ulArg++)
  {
    if (pArg->ucTargetFormat & ARG_MUST)               /* mandatory argument */
      if (!*pArg->pTargetSpecified)        /* check whether specified or not */
        return ( (int)ulArg );                  /* return number of argument */
  }

  return(NO_ERROR);                           /* indicate nothing is missing */
}


/*****************************************************************************
 * Name      : APIRET ArgStandard
 * Funktion  : Standard procedure to analyse arguments
 * Parameter : int       argc,
 *             char      argv*[],
 *             PARGUMENT pTabArguments
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Mittwoch, 1997/03/19 02.23.01]
 *****************************************************************************/

APIRET TOOLAPI _Export ArgStandard (int       argc,
                                   char      *argv[],
                                   PARGUMENT pTabArguments,
                                   char *    pfsHelp)
{
  APIRET rc;                                               /* API returncode */

  if (pfsHelp == NULL)                                   /* check parameters */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */


#ifdef __IBMC__
  setvbuf(stdout,                                /* disable stream buffering */
          NULL,
          _IONBF,
          0);
#endif

  rc = ArgParse (argc,
                 argv,
                 pTabArguments);                     /* CLI-Parameter parsen */
  if ( (rc == 0) &&                                     /* if parsing was OK */
       (!*pfsHelp) )
    if ( ArgIsArgumentMissing(pTabArguments) ) /* check for mandatory params */
      rc = ArgGetMissing(pTabArguments);                /* then request them */

  if (rc != NO_ERROR)                                /* Fehler aufgetreten ? */
  {
    if ( (rc != (ULONG)-1) )             /* fehlerhafter Parameter angegeben */
      fprintf(stderr,
              "\nError: Unknown parameter: [%s]",
              argv[rc]);
    else
      fprintf(stderr,
              "\nError: Can't parse command line parameters.");                                               /* Anderer Fehler ... */

    return (ERROR_TOOLS_INVALID_ARGUMENT);                  /* abort program */
  }

  return (NO_ERROR);                                                   /* OK */
}
