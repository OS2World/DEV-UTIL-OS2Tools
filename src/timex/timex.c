/***********************************************************************
 * Name      : Timex
 * Funktion  : Measure total execution time of a program
 *
 * Autor     : Patrick Haller [2002-07-28]
 ***********************************************************************/



/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <io.h>
#include <process.h>

#include "tooltypes.h"
#include "tools.h"


#define MAXPATHLEN 260


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/


typedef struct
{
  PERFSTRUCT   psStart;                       /* for performance measurement */
  PERFSTRUCT   psEnd;
  double       dTimeTotal;                   /* total time needed in seconds */
} GLOBALS, *PGLOBALS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/
GLOBALS Globals;                /* this structure holds global variables     */


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/
void   help              (void);

int    main              (int,
                          char **);


/***********************************************************************
 * Name      : void help
 * Funktion  : Anzeigen des Hilfetextes
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:43:33]
 ***********************************************************************/

void help (void)
{
  TOOLVERSION("Timex",                                   /* application name */
              0x00010000,                            /* application revision */
              0x00010100,           /* minimum required revision of PHSTOOLS */
              0x0001FFFF,        /* maximum recommended revision of PHSTOOLS */
              NULL,                                                /* Remark */
              NULL);                                 /* additional copyright */

  printf("\ntimex [command line]\n");
}


/***********************************************************************
 * Name      : APIRET ExecuteCommand
 * Funktion  : 
 * Parameter : int argc, char *argv[]
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-07-28]
 ***********************************************************************/

APIRET ExecuteCommand(int argc, char *argv[])
{
    /* execute the child command */
    APIRET rc;

    /* build command line */
    PSZ pszCommandLine = (PSZ)malloc( 4000 );
    int i;

    *pszCommandLine = 0;

    for (i = 1;
         i < argc;
         i++)
    {
        /* add up the elements of a command line */
        PSZ pszToken = argv[ i ];

        /* check if there is a space or quotes contained in the argument ? */
        if (strchr(pszToken, ' '))
        {
            strcat(pszCommandLine, "\"");
            strcat(pszCommandLine, pszToken);
            strcat(pszCommandLine, "\"");
        }
        else
            strcat(pszCommandLine, pszToken);

        if (i < argc - 1)
            strcat(pszCommandLine, " ");
    }

//    printf("DEBUG: [%s]\n",
//           pszCommandLine);


    ToolsPerfQuery(&Globals.psStart);                          /* benchmarking */

    rc = system(pszCommandLine);

    ToolsPerfQuery(&Globals.psEnd);                            /* benchmarking */

    Globals.dTimeTotal = Globals.psEnd.fSeconds - Globals.psStart.fSeconds;

    printf ("Executed in %10.5f, return code is %d (%8xh)\n",
            Globals.dTimeTotal,
            rc,
            rc);

    free( pszCommandLine );

    return rc;
}


/***********************************************************************
 * Name      : int main
 * Funktion  : Hauptroutine
 * Parameter : int argc, char *argv[]
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 ***********************************************************************/

int main (int argc, char *argv[])
{
  int rc;                                                    /* Rckgabewert */

  memset (&Globals,
          0,
          sizeof(Globals));

  if (argc <= 1)                                       /* user requests help */
  {
    help();
    return (NO_ERROR);
  }


  /***************************************************************************
   * Map arguments                                                           *
   ***************************************************************************/

  rc = ExecuteCommand(argc, argv);                           /* process them */

  return (rc);
}
