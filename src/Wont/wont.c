/***********************************************************************
 * Name      : Module Wont
 * Funktion  : Echoing parameters and doing nothing.
 * Bemerkung : 
 *
 * Autor     : Patrick Haller [Samstag, 28.10.1995 14.54.33]
 ***********************************************************************/


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

int  main ( int    argc,
            char  *argv[] );


/***********************************************************************
 * Name      : int main
 * Funktion  : Hauptroutine
 * Parameter : int argc, char *argv[]
 * Variablen :
 * Ergebnis  : RÅckgabewert ans Betriebssystem
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 28.10.1995 14.55.13]
 ***********************************************************************/

int main (int argc, char *argv[])
{
  int iCounter;                                     /* loop counter variable */
  
  printf ("\nWon't ");
  
  /* Bug: BC1.0 passes the name of the runtime DLL instead of executable
          name in argv[0], e.g. \DLL\C2MT.DLL for dynamically linked files */
  
  for (iCounter = 1;
       iCounter < argc;
       iCounter++)
    printf("%s ",
           argv[iCounter]);
  
  return 0;                           /* return code to the operating system */
} /* int main */
