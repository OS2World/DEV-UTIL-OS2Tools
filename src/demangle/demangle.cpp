/***********************************************************************
 * Name      : Module Demangle
 * Funktion  : Demangle IBM VAC C++ mangled names
 * Bemerkung : 
 *
 * Autor     : Patrick Haller [Samstag, 28.10.1995 14.54.33]
 ***********************************************************************/


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#define INCL_DOSERRORS                         /* Die Fehlerkonstanten */
#define INCL_DOS
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <demangle.h>
#include <string.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
} OPTIONS, *POPTIONS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */

ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung----------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
  ARG_TERMINATE
};


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void help            ( void );

BOOL demangleName( PSZ pszNameBuffer,
                   ULONG ulNameBufferLength );

int  main ( int    argc,
            char  *argv[] );


/***********************************************************************
 * Name      : void help
 * Funktion  : Darstellen der Hilfe
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

void help (void)
{
  TOOLVERSION("Demangle",                                /* application name */
              0x00010000,                            /* application revision */
              0x00010100,           /* minimum required revision of PHSTOOLS */
              0x0001FFFF,        /* maximum recommended revision of PHSTOOLS */
              NULL,                                                /* Remark */
              NULL);                                 /* additional copyright */
}




/***********************************************************************
 * Name      : BOOL demangleName
 * Funktion  : Demangle VAC-style C++ name
 * Parameter : PSZ   pszNameBuffer      
 *              in:  mangled name
 *              out: return buffer for the demangled name
 *             ULONG ulNameBufferLength 
 *                   length of the return buffer
 * Variablen :
 * Ergebnis  : TRUE if name was demangled, FALSE if not
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2001/11/28]
 ***********************************************************************/

BOOL demangleName(PSZ pszNameBuffer,
                  ULONG ulNameBufferLength)
{
  // check if we can demangle a C++ name
  char* rest;
  Name* name = Demangle(pszNameBuffer, rest);
  if (name != NULL)
  {
    strncpy(pszNameBuffer,
            name->Text(),
            ulNameBufferLength);
    delete name;
    
    // name could be demangled
    return TRUE;
  }
  
  // not a mangled name
  return FALSE;
}


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
  
  printf ("\nDemangling C++ names:\n");
  
  if (argc <= 1)
  {
    help();
  }
  else
  {
    for (iCounter = 1;
         iCounter < argc;
         iCounter++)
    {
      CHAR szBuf[260];
      
      strcpy (szBuf,
              argv[iCounter]);
              
      if (TRUE == demangleName(szBuf,
                               sizeof(szBuf)) )
      {
        printf("  %-25s : %s\n",
               argv[iCounter],
               szBuf);
      }
      else
      {
        printf("  %-25s : not mangled name\n",
               argv[iCounter]);
      }
    }
  }
  
  return 0;                           /* return code to the operating system */
} /* int main */
