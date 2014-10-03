/*****************************************************************************
 * Projekt   : PHS CPVersion
 * Name      : Modul MAIN
 * Funktion  : Hauptmodul
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 05.09.1995 22.50.30]
 *****************************************************************************/


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include "cpversion.h"


/*****************************************************************************
 * Defines                                                                   *
 *****************************************************************************/

typedef char *PSZ;
typedef unsigned long int ULONG;


/*****************************************************************************
 * This structure holds all global variables                                 *
 *****************************************************************************/

static const PSZ pszRevHeader = "\n/****************************************"
                                "\n * This file was automatically created by"
                                "\n * CPVersion ("__DATE__" "__TIME__
                                "\n * Do not put code in here, nor alter the"
                                "\n * keyword tokens below. Only modify the"
                                "\n * revision token values."
                                "\n * Suggestions welcome:"
                                "\n * haller@zebra.fh-weingarten.de"
                                "\n ****************************************/"
                                "\n"
                                "\n#ifndef MODULE_REVISION"
                                "\n#define MODULE_REVISION"
                                "\n\n";

static const PSZ pszRevFooter = "\n"
                                "#endif /* MODULE_REVISION */"
                                "\n\n";


/*****************************************************************************
 * This structure is the keyword table.                                      *
 *****************************************************************************/

typedef struct _KEYWORD
{
  PSZ   pszToken;
  PSZ   pszDescription;
  ULONG ulType;
} KEYWORD, * PKEYWORD;

enum { KT_END,
       KT_DATE,                                             /* keyword types */
       KT_TIME,
       KT_IGNORE,
       KT_DIR,
       KT_VER_MAJOR,
       KT_VER_MINOR,
       KT_VER_BUILD,
       KT_VER
     };

static const KEYWORD ktKeywords[] =
{
  /*--pszToken----------pszDescription--------------------ulType-------------*/
  {"REV_COMPILE_DATE",  "Compilation date.",              KT_DATE},
  {"REV_COMPILE_TIME",  "Compilation time.",              KT_TIME},
  {"REV_COMPILE_NAME",  "Name of author.",                KT_IGNORE},
  {"REV_COMPILER",      "Name of compiler.",              KT_IGNORE},
  {"REV_PROJECT_NAME",  "Name of project.",               KT_IGNORE},
  {"REV_PROJECT_MODULE","Name of module.",                KT_IGNORE},
  {"REV_COMPILE_DIR",   "Directory of compilation.",      KT_DIR},
  {"REV_VERSION_MAJOR", "Major version number.",          KT_VER_MAJOR},
  {"REV_VERSION_MINOR", "Minor version number.",          KT_VER_MINOR},
  {"REV_VERSION_BUILD", "Build version number.",          KT_VER_BUILD},
  {"REV_VERSION",       "Version 012.3.45",               KT_VER},
    
  {NULL,                NULL,                             KT_END}
};


/*****************************************************************************
 * Local prototypes                                                          *
 *****************************************************************************/


/*****************************************************************************
 * Projekt   : PHS CPVersion
 * Name      : int MainProcessFile
 * Funktion  : Kapselt komplette Bearbeitung einer Revisionsdatei
 * Parameter : 
 * Variablen :
 * Ergebnis  : Fehlercode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Mittwoch, 16.10.1996 15.24.34]
 *****************************************************************************/

int MainProcessFile (PSZ pszRevisionFile)
{
  /* 1. Revisionsdatei ”ffnen */
  /* 2. alte Werte parsen */
  /* 3. neue Werte berechnen */
  /* 4. neue Revisionsdatei schreiben */
  /* 5. Revisionsdatei schlieáen */

  return 0;
}


/*****************************************************************************
 * Projekt   : PHS CPVersion
 * Name      : int main
 * Funktion  : Hauptroutine
 * Parameter : 
 * Variablen :
 * Ergebnis  : Fehlercode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Mittwoch, 16.10.1996 15.24.34]
 *****************************************************************************/

int main(int argc, char *argv[])
{
  int iReturn;                                         /* Library returncode */
  
  
  if (argc != 2)                             /* check command line arguments */
  {
    printf ("\nCPVersion v0.01 (c) 1996 Patrick Haller Systemtechnik"
            "\n"
            "\nUsage: CPVERSION <revision file.rev>");
    exit (1);                               /* abort program with returncode */
  }
  
  iReturn = MainProcessFile (argv[1]);          /* process the revision file */
  return (iReturn);               /* return the code to the operating system */
}
