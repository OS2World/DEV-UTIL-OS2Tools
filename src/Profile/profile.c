/***********************************************************************
 * Projekt   : PHS Tools
 * Name      : Profiling
 * Funktion  : Accesses OS/2's Profiling Facility
 * Autor     : Patrick Haller [Montag, 25.09.1995 01.20.20]
 ***********************************************************************/

/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#define INCL_DOSDEVIOCTL
#define INCL_DOSDEVICES                                 /* DosDevIOCtl */
#define INCL_DOSERRORS                         /* Die Fehlerkonstanten */
#define INCL_DOSMISC                                  /* DosGetMessage */
#define INCL_DOS
#define INCL_NOPMAPI                      /* Kein Presentation Manager */

#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <process.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"

#define MAXPATHLEN 260


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

void   help            ( void );

int    main            ( int           argc,
                         char          *argv[] );


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
  TOOLVERSION("Profile",                                   /* application name */
              0x00000001,                            /* application revision */
              0x00010100,           /* minimum required revision of PHSTOOLS */
              0x0001FFFF,        /* maximum recommended revision of PHSTOOLS */
              NULL,                                                /* Remark */
              NULL);                                 /* additional copyright */
}


/***********************************************************************
 * Name      : APIRET ProfileTest
 * Funktion  :
 * Parameter :
 * Variablen :
 * Ergebnis  : API return code
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 ***********************************************************************/

APIRET APIENTRY Dos32Profile(ULONG ulType,
                             ULONG ulFunc,
                             PID   pidProcess,
                             ULONG ul2);

APIRET ProfileTest(void)
{
  APIRET rc;

  ULONG  ul1;
  ULONG  ul2;

  PVOID  pBuffer;
  PVOID  ppBuffer = &pBuffer;
#if 0
   /* DosProfile usType */
   #define PROF_SYSTEM           0
   #define PROF_USER             1
   #define PROF_USEDD            2
   #define PROF_KERNEL           4
   #define PROF_VERBOSE          8
   #define PROF_ENABLE          16

   /* DosProfile usFunc */
   #define PROF_ALLOC            0
   #define PROF_CLEAR            1
   #define PROF_ON               2
   #define PROF_OFF              3
   #define PROF_DUMP             4
   #define PROF_FREE             5

   /* DosProfile tic count granularity (DWORD) */
   #define PROF_SHIFT            2

   /* DosProfile module name string length */
   #define PROF_MOD_NAME_SIZE   10

   /* DosProfile error code for end of data */
   #define PROF_END_OF_DATA     13
#endif

  /* have to call system first, initialization */
  rc = DosAllocSharedMem(&pBuffer,
                         "\\SHAREMEM\\PROFILE",
                         8192,
                         PAG_READ | PAG_WRITE | PAG_COMMIT);
  fprintf (stderr,
           "\nDEBUG Dos32AllocMem (alloc) = %u",
           rc);


  rc = Dos32Profile(PROF_SYSTEM,
                    0,
                    getpid(),
                    (ULONG)pBuffer);
  fprintf (stderr,
           "\nDEBUG Dos32Profile (alloc) = %u",
           rc);

  rc = Dos32Profile(PROF_KERNEL,
                    PROF_ALLOC,
                    getpid(),
                    (ULONG)0);
  fprintf (stderr,
           "\nDEBUG Dos32Profile (alloc) = %u",
           rc);


  rc = Dos32Profile(PROF_KERNEL,
                    PROF_CLEAR,
                    0,
                    0);
  fprintf (stderr,
           "\nDEBUG Dos32Profile (clear) = %u",
           rc);

  rc = Dos32Profile(PROF_KERNEL,
                    PROF_ON,
                    0,
                    0);
  fprintf (stderr,
           "\nDEBUG Dos32Profile (on) = %u",
           rc);

  rc = Dos32Profile(PROF_KERNEL,
                    PROF_OFF,
                    0,
                    0);
  fprintf (stderr,
           "\nDEBUG Dos32Profile (off) = %u",
           rc);

  rc = Dos32Profile(PROF_KERNEL,
                    PROF_DUMP,
                    0,
                    0);
  fprintf (stderr,
           "\nDEBUG Dos32Profile (dump) = %u",
           rc);


  rc = Dos32Profile(PROF_KERNEL,
                    PROF_FREE,
                    0,
                    0);
  fprintf (stderr,
           "\nDEBUG Dos32Profile (free) = %u",
           rc);
}


/***********************************************************************
 * Name      : void initialize
 * Funktion  : Initialisierung einiger Variablen
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 ***********************************************************************/

void initialize (void)
{
  memset(&Options,
         0L,
         sizeof(Options));
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
  int rc;                                                    /* RÅckgabewert */

  initialize ();                                          /* Initialisierung */

  rc = ArgStandard (argc,                            /* CLI-Parameter parsen */
                    argv,
                    TabArguments,
                    &Options.fsHelp);
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    exit(1);                                                /* abort program */
  }

  if ( Options.fsHelp )                                  /* help requested ? */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }


  rc = ProfileTest();                                            /* Los geht's */
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  return (rc);
}
