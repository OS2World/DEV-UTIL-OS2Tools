/***********************************************************************
 * Projekt   : PHS Tools
 * Name      : Dos32SysCtl
 * Funktion  : 
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

#include <conio.h>

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
  TOOLVERSION("SystemControl",                           /* application name */
              0x00000001,                            /* application revision */
              0x00010100,           /* minimum required revision of PHSTOOLS */
              0x0001FFFF,        /* maximum recommended revision of PHSTOOLS */
              NULL,                                                /* Remark */
              NULL);                                 /* additional copyright */
}


/***********************************************************************
 * Name      : APIRET HyberTest
 * Funktion  :
 * Parameter :
 * Variablen :
 * Ergebnis  : API return code
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 ***********************************************************************/

APIRET APIENTRY Dos32SysCtl(ULONG ulFunction,
                            PVOID pParameter);

APIRET HyberTest(void)
{
  APIRET rc;
  CHAR   szSwappath[260];
  ULONG  ulCPUType;
  ULONG  ulIODelay;

#define SYSCTL_SWAPMANAGERRESET1   0x00000001
#define SYSCTL_UNKNOWN1            0x00000002
#define SYSCTL_NOP                 0x00000003
#define SYSCTL_MINIVDMCREATE       0x00000004
#define SYSCTL_MINIVDMDESTROY      0x00000005
#define SYSCTL_MINIVDMISSUEREQUEST 0x00000006 /* ulParam */
#define SYSCTL_MINIVDMTHAWREQUEST  0x00000007 /* ulParam */
#define SYSCTL_QUERYFSDBUFFER1     0x00000008 /* ulParam -> pBuffer */
#define SYSCTL_QUERYCPUTYPE        0x00000009 /* ulParam -> &ulCPUType */
#define SYSCTL_QUERYIODELAY        0x0000000a /* ulParam -> &ulIODelay */
#define SYSCTL_HYBERNATE_STATUS    0x0000000b /* ulParam -> pBuffer */
#define SYSCTL_HYBERNATE           0x0000000c /* ulParam -> pBuffer */
#define SYSCTL_HYBERNATE_FREEZE2   0x0000000d
#define SYSCTL_HYBERNATE_FREEZE1   0x0000000e /* ulParam -> pBuffer */
#define SYSCTL_IFS_FLUSHCACHE      0x0000000f /* ulParam -> pBuffer */
#define SYSCTL_HYBEROUT            0x00000010
#define SYSCTL_HYBEROK             0x00000011 /* ulParam -> pBuffer */
#define SYSCTL_QUERYSWAPPATH       0x00000012 /* ulParam -> pszBuffer */
#define SYSCTL_HYBERFILECHECK      0x00000013 /* ulParam -> pszHyberFile ? */
#define SYSCTL_HYBERSAVERESTORE    0x00000014 /* ? */
#define SYSCTL_VMFREECONTEXT       0x00000015  /* ulParam = plinBaseAddress */


  
  rc = Dos32SysCtl(SYSCTL_QUERYCPUTYPE,
                   &ulCPUType);

  printf ("\nSYSCTL_QUERYCPUTYPE = %u (%08x)\n",
          rc,
          ulCPUType);
  
  /* from IBM DDK, vpmx2.h */
  #define VPMCPU_286            0               /* processor is a 286 */
  #define VPMCPU_386            1               /* processor is a 386 */
  #define VPMCPU_486            2               /* processor is a 486 */
  #define VPMCPU_586            3               /* processor is a 586 */  /* 149007 */

  
  getch();
  
  rc = Dos32SysCtl(SYSCTL_QUERYIODELAY,
                   &ulIODelay);

  printf ("\nSYSCTL_QUERYIODELAY = %u (%08x)\n",
          rc,
          ulIODelay);
  
  getch();

  rc = Dos32SysCtl(SYSCTL_QUERYSWAPPATH,
                   szSwappath);

  printf ("\nSYSCTL_QUERYSWAPPATH = %u (%s)\n",
          rc,
          szSwappath);
  
  getch();
  
  printf ("\n");

  return (0);
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


  rc = HyberTest();                                            /* Los geht's */
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  return (rc);
}
