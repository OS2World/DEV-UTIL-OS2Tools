/*****************************************************************************
 * Name      : Module Config
 * Funktion  : Behandlung von Konfigurationsscripts.
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [(null), 19.05.1996 22.31.53]
 *****************************************************************************/


/****************** Include-Dateien ******************************************/
#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rconfig.h"
#include "phstools.h"
#include "reflect.h"

/*****************************************************************************/


/****************** Definition der Modul-Globalen Variablen ******************/
                                                     /* (Nullbasierter Wert) */
static COMMAND TabCommands[] =
{
  { "help",           0, &CmdHelp,          "Help pages."},
  { "include",        1, &CmdInclude,       "Include a file in the script."},
  { "set",            2, &CmdSet,           "Assigns a value to a variable."},
  { NULL,             0, NULL,           NULL}          /* Listenterminierung */
};


/*****************************************************************************
 * Macros                                                                    *
 *****************************************************************************/


/*****************************************************************************
 * Name      : StrTrimcfg
 * Funktion  : SÑubert Strings, Kommentare abtrennen, etc.
 * Parameter : PSZ pszString
 * Variablen :
 * Ergebnis  : void
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 06.04.1996 19.47.02]
 *****************************************************************************/

void StrTrimcfg (PSZ pszString)
{
  PSZ pszTemp;                                 /* temporaerer Stringiterator */

                                            /* Ist da Åberhaupt ein String ? */
  if ((!pszString) || (*pszString == 0)) return;

                                    /* FÅhrende Leerzeichen/Tabs abschneiden */
  pszTemp = pszString;
  while (*pszTemp && (*pszTemp <= ' ')) pszTemp++;
  if (pszTemp != pszString)
    memcpy (pszString,pszTemp,strlen(pszTemp)+1);

                                     /* hintere Leerzeichen & LF abschneiden */
  pszTemp = pszString + strlen(pszString);
  while ((*pszTemp <= ' ') && (pszTemp >= pszString)) pszTemp--;
  *(pszTemp+1) = 0;                                  /* explicit termination */

                                                    /* Kommentar abschneiden */
  pszTemp = strchr (pszString,'%');
  if (pszTemp) *pszTemp = 0;
  else
  {
    pszTemp = strchr (pszString,'#');
    if (pszTemp) *pszTemp = 0;
    else
    {
      pszTemp = strchr (pszString,';');
      if (pszTemp) *pszTemp = 0;
    }
  }

                                                /* UngÅltige Zeichen filtern */
  for (pszTemp=pszString;*pszTemp;pszTemp++)
    if (*pszTemp < ' ') *pszTemp=' ';
}


/*****************************************************************************
 * Funktion:            APIRET ParseCommand(PSZ pszCommand,
 *                                          PSZ pszParameter)
 *
 * Kurzbeschreibung:    Aufbereitung eines Kommandos fuer die Verarbeitung
 *
 * Import-Parameter:    PSZ pszCommand
 *                      PSZ pszParameter
 *
 * Export-Parameter:    -
 *
 * Ergebnis:            API-Fehlercode bzw. Fehlercode der bearbeiteten
 *                      Kommandofunktion. Besonderheit: RC_EXIT. Damit
 *                      wird signalisiert, dass das Programm beendet
 *                      werden soll.
 *****************************************************************************/
APIRET ParseCommand (PSZ pszCommand, PSZ pszParameter)
{
  APIRET rc = NO_ERROR;                                     /* Rueckgabewert */
  ULONG  ulCommand;                        /* Index des gefundenen Kommandos */
  ULONG  ulParameters;                   /* Anzahl der angegebenen Parameter */
  STACK  StackLocal;                     /* Lokaler Stack fuer die Parameter */

  if (pszCommand == NULL)                          /* Parameterueberpruefung */
    return (ERROR_INVALID_PARAMETER);                /* Fehler signalisieren */

  rc = StackInit (&StackLocal);          /* Den lokalen Stack initialisierem */
  if (rc == NO_ERROR)                        /* Ist ein Fehler aufgetreten ? */
  {
    rc = CommandSearch (pszCommand,&ulCommand);       /* Kommando raussuchen */
    if (rc == NO_ERROR)                      /* Ist ein Fehler aufgetreten ? */
    {
                                               /* Parameterstring bearbeiten */
         rc = CommandParameterGet (&StackLocal,          /* der lokale Stack */
                                   pszParameter,          /* Parameterstring */
                                   &ulParameters);       /* Anzahl Parameter */

         if (rc == NO_ERROR)      /* Alles i.O., dann also Funktion aufrufen */
         {
           rc = CommandRun (&StackLocal,               /* Kommando ausfÅhren */
                            ulCommand);
         }
    }
  }

  return (rc);                                      /* Rueckgabewert liefern */
}


/*****************************************************************************
 * Name      : APIRET ConfigProcess
 * Funktion  : Konfigurationsdatei abarbeiten
 * Parameter :
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 06.04.1996 19.55.40]
 * Mod       : 96/11/06 PH - corrected handling of the last line in a
 *                           configuration file
 *****************************************************************************/

APIRET ConfigProcess (PSZ pszConfigFile)
{
  FILE   *fConfig;                                          /* Dateiselektor */
  char   szBuffer[256];                       /* Zeilenpuffer fÅr die Config */
  APIRET rc = NO_ERROR;                                    /* API-Returncode */
  PSZ    pszParameter;                                     /* Stringiterator */
  PSZ    pszCommand;                                       /* Stringiterator */
  ULONG  ulLine = 0;       /* line counter for the configuration script file */



  if (pszConfigFile == NULL)                         /* ParameterÅberprÅfung */
    return (ERROR_INVALID_PARAMETER);                /* Fehler signalisieren */

  fConfig = fopen (pszConfigFile,"r");                       /* Datei îffnen */
  if (fConfig == NULL)                               /* Fehler aufgetreten ? */
     return (ERROR_OPEN_FAILED);                     /* Fehler signalisieren */

  while (!feof(fConfig))                   /* Solange noch Daten da sind ... */
  {
    fgets (szBuffer, 
           sizeof(szBuffer), 
           fConfig);                                       /* Zeile einlesen */
    
    if (feof(fConfig))                              /* Dateieende erreicht ? */
      break;                                               /* Dann abbrechen */
    /* @@@PH - letzte Zeile auswerten */
    
    ulLine++;                                   /* increase the line counter */


    StrTrimcfg (szBuffer);                                      /* AufrÑumen */


    if (szBuffer[0] != '\0')               /* Wenn brauchbarer String da ist */
    {
       pszParameter = strchr (szBuffer, ' ');    /* Parameterblock abtrennen */

       if (pszParameter != NULL)               /* Sind Parameter angegeben ? */
       {
                /* Pruefen, ob uns ein Leerzeichen am Zeilenende irre fuehrt */
         if (*(pszParameter+1) == '\0')    /* Ist die Zeile nach " " zu Ende */
           pszParameter = NULL;      /* Dann liegen auch keine Parameter vor */
        else
          pszParameter++;          /* sonst fuehrende Leerzeichen ignorieren */
       }

       pszCommand = strtok(szBuffer," ");            /* " " als Trennzeichen */
       if (pszCommand != NULL)                  /* Liegt was Sinnvolles an ? */
       {
         StrTrim (pszParameter);           /* Den Parameterstring aufraeumen */
                                                      /* Kommando bearbeiten */
         rc = ParseCommand(pszCommand,pszParameter);
         if (rc != NO_ERROR)                         /* Fehler aufgetreten ? */
           ToolsErrorDosEx(rc,
                           "ConfigProcess->ParseCommand");
       }
    }
  }

  fclose (fConfig);                                /* Config-Datei schlie·en */

  return (NO_ERROR);                                 /* RÅckgabewert liefern */
}


/*****************************************************************************
 * Funktion:            APIRET CommandSearch (pszCommand)
 *                      PSZ pszCommand
 *
 * Kurzbeschreibung:    Sucht nach einem Kommandonamen in der
 *                      Kommandotabelle
 *
 * Ergebnis:            Fehlercode der Bearbeitung.
 *                      ERROR_INVALID_PARAMETER: Fehlerhafter Funktions-
 *                                               parameter
 *                      ERROR_UNKNOWN_COMMAND:   Kommando unbekannt.
 *                      NO_ERROR:                alles OK
 *
 * Import-Parameter:    PSZ pszCommand
 *
 * Export-Parameter:    PULONG pulIndex - Index auf das gefundene
 *                                        Kommando in obiger Tabelle
 *
 * Mod.glob.Var.:       TabComands - die Kommandotabelle
 *
 *****************************************************************************/
APIRET CommandSearch (PSZ pszCommand, PULONG pulIndex)
{
  PCOMMAND pCommand;                   /* Iterator ueber die Kommandotabelle */
  ULONG    ulCommand;                       /* Index auf die Kommandotabelle */

  if ( (pszCommand == NULL) ||                     /* Parameterueberpruefung */
       (pulIndex   == NULL) )
    return (ERROR_INVALID_PARAMETER);                /* Fehler signalisieren */

  for (pCommand = TabCommands,                       /* Bearbeitungsschleife */
       ulCommand = 0;                                /* Index initialisieren */
       (pCommand->pszCommand != NULL);         /* Primaeres Abbruchkriterium */
       pCommand++,                                   /* Iterator inkremieren */
       ulCommand++)                                     /* Index inkremieren */
  {                                  /* Nach dem angegebenen Kommando suchen */
    if (strcmpi (pCommand->pszCommand, pszCommand) == 0)
    {
      *pulIndex = ulCommand;                       /* Ergebniswert eintragen */
      return (NO_ERROR);          /* Diese Schleife muss leider so verlassen */
           /* werden, da stricmp compilerabhaengig bei der Evaluierung der   */
           /* kompletten Abbruchbedingung (pCommand->pszCommand != NULL) &&  */
           /* (stricmp (pCommand->pszCommand, pszCommand) != NULL) eine      */
           /* Segmentation Fault ausloesen koennte.                          */

            /* Naeher an der Aufgabenstellung ist folgendes Abbruchkriterium */
            /* orientiert: (stricmp(pCommand->pszCommand,"exit") != NULL) && */
            /*         (stricmp(pCommand->pszCommand,pszCommand) != NULL)    */
    }
  }

  return (ERROR_INVALID_FUNCTION);                 /* Rueckgabewert liefern */
}


/*****************************************************************************
 * Funktion:            APIRET CommandParameterGet
 *
 * Kurzbeschreibung:    Analysiert die uebergebene Parameterzeile betr.
 *                      der an der ermittelte Kommando zu uebergebenden
 *                      Parameter. Dann werden die Parameter als Strings
 *                      auf den Stack geschoben.
 * Ergebnis:            API-Fehlercode
 *                      NO_ERROR                - alles OK
 *                      ERROR_INVALID_PARAMETER - interner Fehler:
 *                                                ungueltiger Funktions-
 *                                                parameter.
 *
 * Import-Parameter:    PSTACK  pStack        - der verwendete Stack
 *                      PSZ    pszParameter   - die Parameterzeile
 *                      ULONG  ulCommandIndex - das gefundene Kommando
 *
 * Export-Parameter:    PULONG pulReturnParameters - Anzahl der tat-
 *                                      saechlich gelieferten Parameter
 *
 * Mod.glob.Var.:       TabCommands - die Kommandotabelle
 *
 *****************************************************************************/
APIRET CommandParameterGet (PSTACK pStack,
                            PSZ    pszParameter,
                            PULONG pulReturnParameters)
{
  APIRET rc = NO_ERROR;                                     /* Rueckgabewert */
  ULONG  ulParameters = 0;             /* Anzahl Parameter fuer die Funktion */
  PSZ    pszToken;               /* Zeiger auf die einzelnen Parametertokens */
  PSZ    pszSpace;     /* temporÑrer Stringiterator zeigt auf naechtes SPACE */
  BOOL   fInQuotes;                          /* indicates position of quotes */
  BOOL   fLastSpace;                         /* has the last char been a " " */
  PSZ    pszTemp;                                /* temporaerer Stringzeiger */



                                         /* Parameter auf den Stack schieben */
  if (pszParameter != NULL)                        /* Liegen Parameter vor ? */
  {
    pszToken = pszParameter;                      /* pszToken initialisieren */
    StrTrim (pszParameter);                    /* clean the parameter string */

    if (pszParameter[0] == 0)                             /* Leerer String ? */
      return (NO_ERROR);                              /* dann hier abbrechen */


    fLastSpace = TRUE;                                  /* initialize status */
    fInQuotes  = FALSE;

    for (pszSpace = pszParameter;

         *pszSpace != 0;                              /* until end of string */

         pszSpace++)                                    /* skip to next char */
    {
      switch (*pszSpace)                        /* react on the current char */
      {
        case '"':
          fInQuotes = !fInQuotes;                  /* toggle quote indicator */
          break;

        case ' ':
          if (fInQuotes == FALSE)              /* if the space is NOT quoted */
          {
            if (fLastSpace == FALSE)              /* We had a space before ? */
            {
              ulParameters++;         /* Anzahl geparster Parameter erhoehen */
              
              *pszSpace = 0;          /* explicit termination of this string */

              while (*pszToken == ' ')
                pszToken++;                       /* skip the leading spaces */
              
              if (*pszToken == '"')                /* skip one leading quote */
                pszToken++;
              
              pszTemp = pszSpace - 1;               /* remove trailing quote */
              while ( (pszTemp > pszToken) &&
                      (*pszTemp == ' ') )
                pszTemp--;                    /* step backward in the string */
              
              if (*pszTemp == '"')                 /* cut one trailing quote */
                pszTemp--;
              
              pszTemp++;
              *pszTemp = 0;                          /* terminate the string */
              
                               /* Geparsten Parameter auf den Stack schieben */
              rc = StackPush (pStack,
                              pszToken,
                              pszTemp-pszToken+1);      /* calculated strlen */
              if (rc != NO_ERROR)       /* Ist hier ein Fehler aufgetreten ? */
                return (rc);            /* Dann Schleife ebenfalls verlassen */

              pszToken = pszSpace + 1;    /* set token beginning to next one */
              fInQuotes = FALSE;   /* token's end is reached, no quotes left */
            }

            fLastSpace = TRUE;               /* Yes, last char was a space ! */
          }
          break;

        default:
          fLastSpace = FALSE;                  /* Last char is NOT a space ! */
          break;
      }
    }


    ulParameters++;                   /* Anzahl geparster Parameter erhoehen */

    StrTrim (pszToken);                           /* clean up in data string */

    if (*pszToken == '"')                                /* check for quotes */
    {
      pszToken++;                        /* Skip to next char, ignore quotes */
      pszTemp = pszToken;
      while ( (*pszTemp != 0) &&          /* do not parse behind strings end */
              (*pszTemp != '"') )
        pszTemp++;
      *pszTemp = 0;                                 /* terminate this string */
    }
    else
      pszTemp = pszSpace;                           /* remove trailing quote */
                               /* Geparsten Parameter auf den Stack schieben */
    rc = StackPush (pStack,
                    pszToken,
                    pszTemp-pszToken+1);                /* calculated strlen */
  }

         /* Wenn keine Parameter geliefert wurden, dann ist ulParameters = 0 */
  *pulReturnParameters = ulParameters;       /* Anzahl angegebener Parameter */

  return (rc);                                      /* Rueckgabewert liefern */
}


/*****************************************************************************
 * Funktion:            APIRET CommandRun
 *
 * Kurzbeschreibung:    Ruft die gewuenschte Funktion aus der Tabelle
 *                      mit den auf dem Stack liegenden Parametern auf.
 *
 * Ergebnis:            API-Fehlercode:
 *                      ERROR_INVALID_PARAMETERS - interner Fehler.
 *                      sonst: Fehlercode aufgerufener kommandofunktion
 *
 * Import-Parameter:    PSTACK pStack        - der benutzte Stack
 *                      ULONG  ulCommand    - Nummer des Kommandos
 *                      ULONG  ulParameters - derzeit unbenutzt
 *
 * Export-Parameter:    -
 *
 * Mod.glob.Var.:       TabCommands - die Kommandotabelle
 *
 *****************************************************************************/
APIRET CommandRun (PSTACK pStack, ULONG ulCommand)
{
  APIRET rc = NO_ERROR;                                     /* Rueckgabewert */
  PFNI   pfnFunction;                         /* dynamischer Funktionszeiger */

  pfnFunction = TabCommands[ulCommand].pfFunction;        /* Funktionszeiger */
  rc = pfnFunction(pStack);                     /* Nun die Funktion aufrufen */

                    /* clear the parameter stack after the function returned */
  StackFlush(pStack);                       /* returncode is not of interest */

  return (rc);                                      /* Rueckgabewert liefern */
}


/*****************************************************************************
 * Funktion:            APIRET CmdHelp (PSTACK pStackParameters)
 *                      PSTACK   pStackParameters - der Stack der ueber-
 *                                              gebenen Parameter
 *
 * Kurzbeschreibung:    Ausgeben der verfuegbaren Funktionen.
 *
 * Ergebnis:            Fehlercode der Bearbeitung.
 *
 * Import-Parameter:    PSTACK  pStackParameters
 *
 * Export-Parameter:
 *
 * Mod.glob.Var.:
 *
 *****************************************************************************/
APIRET CmdHelp(PSTACK pStackParameters)
{
  PCOMMAND pCommand;                    /* Iterator Åber die Kommandotabelle */

  printf("\nCommand          Parms Description");
  printf("\nƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ ƒƒƒƒƒ ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ");
  for (pCommand = TabCommands;
       pCommand->pszCommand != NULL;
       pCommand++)
    printf ("\n%-15s %2u     %s",
                   pCommand->pszCommand,
                   pCommand->ulParameters,
                   pCommand->pszHelp);
  printf ("\n");

  return (NO_ERROR);                                 /* RÅckgabewert liefern */
}


/*****************************************************************************
 * Funktion:            APIRET CmdInclude
 *                                     (PSTACK pStackParameters)
 *                      PSTACK   pStackParameters - der Stack der ueber-
 *                                              gebenen Parameter
 *
 * Ergebnis:            Fehlercode der Bearbeitung.
 *
 * Import-Parameter:    PSTACK  pStackParameters
 *
 * Export-Parameter:
 *
 * Mod.glob.Var.:
 *
 *****************************************************************************/
APIRET CmdInclude (PSTACK pStackParameters)
{
  APIRET rc = NO_ERROR;                                      /* RÅckgabewert */
  char   szScriptFile [256];                       /* Name der Kontrolldatei */
  ULONG  ulDataLength;                             /* Groesse obigen Puffers */

                 /* Laenge des Parameters, der vom Stack gelesen werden soll */
  ulDataLength = sizeof(szScriptFile);
                                      /* Den Stringparameter vom Stack lesen */
  rc = StackPop  (pStackParameters,szScriptFile, &ulDataLength);
  if (rc != NO_ERROR)                        /* Ist ein Fehler aufgetreten ? */
    return (rc);                                    /* Rueckgabewert liefern */

  rc = ConfigProcess(szScriptFile);                              /* Einlesen */

  return (rc);                                /* Sinnvolles Ergebnis liefern */
}


/*****************************************************************************
 * Funktion:            APIRET CmdSet  (PSTACK pStackParameters)
 *                      PSTACK   pStackParameters - der Stack der ueber-
 *                                              gebenen Parameter
 *
 * Ergebnis:            Fehlercode der Bearbeitung.
 *
 * Import-Parameter:    PSTACK  pStackParameters
 *
 * Export-Parameter:
 *
 *****************************************************************************/
APIRET CmdSet (PSTACK pStackParameters)
{
  APIRET rc = NO_ERROR;                                      /* RÅckgabewert */
  char   szVariable [64];                           /* Name der Zielvariable */
  char   szValue    [128];          /* Wert der Zielvariable zugewiesen wird */
  ULONG  ulDataLength;                             /* Groesse obigen Puffers */

                 /* Laenge des Parameters, der vom Stack gelesen werden soll */
  ulDataLength = sizeof(szValue);
                                      /* Den Stringparameter vom Stack lesen */
  rc = StackPop  (pStackParameters,
                  szValue,
                  &ulDataLength);
  if (rc != NO_ERROR)                        /* Ist ein Fehler aufgetreten ? */
    return (rc);                                    /* Rueckgabewert liefern */


                 /* Laenge des Parameters, der vom Stack gelesen werden soll */
  ulDataLength = sizeof(szVariable);

  rc = StackPop  (pStackParameters,
                  szVariable,
                  &ulDataLength);
  if (rc != NO_ERROR)                        /* Ist ein Fehler aufgetreten ? */
    return (rc);                                    /* Rueckgabewert liefern */


          /* Nun wird angegebener Variable der entsprechende Wert zugewiesen */
                                               /* add value to variable list */
  rc = VarListAdd (&Globals.pConfiguration,
                   szVariable,                   /* wrapper for VarListAdd() */
                   szValue);

  return (rc);                                /* Sinnvolles Ergebnis liefern */
}


/*****************************************************************************
 * Name      : APIRET VarListAdd
 * Funktion  : Linked list of system configuration variables
 * Parameter : PPVARLIST pVarList - the pointer to the linked list
 *             PSZ       pszName  - name of the variable
 *             PSZ       pszValue - value for the variable
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 20.05.1996 22.31.53]
 *****************************************************************************/

APIRET VarListAdd (PPVARLIST ppVarList,         /* pointer to linked varlist */
                   PSZ       pszName,                /* name of the variable */
                   PSZ       pszValue)                    /* value to assign */
{
  PVARLIST pVarTemp;             /* temporary iterator on the linked varlist */


  if ( (ppVarList == NULL) ||                            /* check parameters */
       (pszName   == NULL) ||
       (pszValue  == NULL) )
    return (ERROR_INVALID_PARAMETER);                      /* signal problem */


  for (pVarTemp = *ppVarList;                      /* Iterate on the varlist */
       pVarTemp != NULL;              /* unless we reach the end of the list */
       pVarTemp = (PVARLIST)pVarTemp->pNext)     /* jump to the next element */
  {
    if ( strcmpi (pszName,              /* look, if "pszName" already exists */
                  pVarTemp->pszName) == 0)
    {
      if (pVarTemp->pszValue != NULL)           /* if there is a valid entry */
        free (pVarTemp->pszValue);                                /* free it */

      pVarTemp->pszValue = strdup(pszValue);               /* copy the value */
      if (pVarTemp->pszValue == NULL)           /* has there been an error ? */
        return (ERROR_NOT_ENOUGH_MEMORY);                /* signal the error */
      else
        return (NO_ERROR);           /* OK, we're finished properly. Return. */
    }
  }

                           /* "pszName" does not exist, create a new entry ! */
  pVarTemp = (PVARLIST)malloc( sizeof(VARLIST) );  /* allocate new structure */
  if (pVarTemp == NULL)                         /* has there been an error ? */
    return (ERROR_NOT_ENOUGH_MEMORY);                    /* signal the error */

  pVarTemp->pszName  = strdup(pszName);                     /* copy the name */
  if (pVarTemp->pszName == NULL)                /* has there been an error ? */
  {
    free (pVarTemp);                     /* free previously allocated memory */
    return (ERROR_NOT_ENOUGH_MEMORY);                    /* signal the error */
  }

  pVarTemp->pszValue = strdup(pszValue);                   /* copy the value */
  if (pVarTemp->pszValue == NULL)               /* has there been an error ? */
  {
    free (pVarTemp);                     /* free previously allocated memory */
    free (pVarTemp->pszName);            /* free previously allocated memory */
    return (ERROR_NOT_ENOUGH_MEMORY);                    /* signal the error */
  }

                               /* Now put the new entry into the linked list */
  pVarTemp->pNext = *ppVarList;                        /* link to next entry */
  *ppVarList = pVarTemp;                             /* set new 1.st element */

  return (NO_ERROR);                                   /* OK, we're done ... */
}


/*****************************************************************************
 * Name      : PSZ VarListQuery
 * Funktion  : Querys a value from the linked list
 * Parameter : PVARLIST pVarList  - the pointer to the linked list
 *             PSZ      pszName   - name of the variable
 * Variablen :
 * Ergebnis  : Pointer to the value or NULL if it fails.
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 20.05.1996 22.31.53]
 *****************************************************************************/

PSZ VarListQuery (PVARLIST pVarList,            /* pointer to linked varlist */
                  PSZ      pszName)                  /* name of the variable */
{
  PVARLIST pVarTemp;             /* temporary iterator on the linked varlist */

  if ( (pVarList == NULL) ||                             /* check parameters */
       (pszName  == NULL) )
    return (NULL);                                         /* signal problem */


  for (pVarTemp = pVarList;                        /* Iterate on the varlist */
       pVarTemp != NULL;              /* unless we reach the end of the list */
       pVarTemp = (PVARLIST)pVarTemp->pNext)     /* jump to the next element */
  {
    if ( strcmpi (pszName,              /* look, if "pszName" already exists */
                  pVarTemp->pszName) == 0)
      return (pVarTemp->pszValue);             /* return pointer to the data */
  }

  return (NULL);                                /* value has not been found. */
}

