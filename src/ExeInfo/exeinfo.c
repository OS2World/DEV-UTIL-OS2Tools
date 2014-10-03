/*****************************************************
 * EXEINFO Tool.                                     *
 * Reports details on a given executable file.       *
 * (c) 1994-95 Patrick Haller Systemtechnik          *
 *****************************************************/

/* #define DEBUG 1 */

/* Remarks:

 Fertiggestellt:

 Typ Header Tabellen Resourcen Debug Komplett
  MZ     OK       OK        --    --       OK
  NE     OK      90%       90%  Nein     Nein
  LE     OK     Nein      Nein  Nein     Nein
  LX     OK      80%       85%  Nein     Nein
  PE     OK     Nein      Nein  Nein     Nein
*/

/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#ifdef __OS2__
  #define INCL_DOSFILEMGR
  #define INCL_BASE
  #define INCL_NOPMAPI
  #include <os2.h>
#endif

#ifdef _WIN32
  #include <windows.h>
#endif

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <process.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"
#include "exeinfo.h"

#define MAXPATHLEN 260


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/
OPTIONS Options;                /* this structure holds command line options */
GLOBALS Globals;                     /* this structure holds global varibles */

ARGUMENT TabArguments[] =
{ /*Token-----------Beschreibung-------------------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/?",            "Get help screen.",            NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",            "Get help screen.",            NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/NODOS",        "No DOS/MZ header.",           NULL,                 ARG_NULL,       &Options.fsNoMZ},
  {"/NOMZ",         "No DOS/MZ header.",           NULL,                 ARG_NULL,       &Options.fsNoMZ},
  {"/NOSEGMENTS",   "No segments table.",          NULL,                 ARG_NULL,       &Options.fsNoSegments},
  {"/NOENTRIES",    "No entries table.",           NULL,                 ARG_NULL,       &Options.fsNoEntries},
  {"/NOMODULES",    "No modules table.",           NULL,                 ARG_NULL,       &Options.fsNoModules},
  {"/NORESOURCES",  "No resources table.",         NULL,                 ARG_NULL,       &Options.fsNoResources},
  {"/NORESIDENT",   "No resident names table.",    NULL,                 ARG_NULL,       &Options.fsNoResident},
  {"/NOIMPORT",     "No imported names table.",    NULL,                 ARG_NULL,       &Options.fsNoImport},
  {"/NOIMPORTPROC", "No import procedures table.", NULL,                 ARG_NULL,       &Options.fsNoImportProc},
  {"/NONONRESIDENT","No non-resident names table.",NULL,                 ARG_NULL,       &Options.fsNoNonResident},
  {"/NORELOCATIONS","No relocation table.",        NULL,                 ARG_NULL,       &Options.fsNoRelocations},
  {"/NOOBJECTS",    "No object table.",            NULL,                 ARG_NULL,       &Options.fsNoObjects},
  {"1",             "Filename.",                   &Options.pszFile,     ARG_PSZ |
                                                                         ARG_DEFAULT |
                                                                         ARG_MUST,       &Options.fsFile},
  ARG_TERMINATE
};



/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void   help                    (void);

void   initialize              (void);

int    main                    (int,
                                char **);

APIRET display_information     (PSZ pszFile);

void   ExeAnalyse              (void);


void   ExeAnalyseHeaderMZ      (PSZ pPtr);

void   ExeAnalyseHeaderLE      (PSZ pPtr);

void   ExeAnalyseHeaderLX      (PSZ pPtr);

void   ExeAnalyseHeaderNE      (PSZ pPtr);

void   ExeAnalyseHeaderPE      (PSZ pPtr);

void   ExeAnalysePEOptionalNT  (PSZ pPtr);

void   ExeAnalysePEOptionalROM (PSZ pPtr);

void   ExeAnalysePESection     (PSZ pPtr);


/***********************************************************************
 * Name      : void help
 * Funktion  : Anzeigen des Hilfetextes.
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.45.24]
 ***********************************************************************/

void help (void)
{
  TOOLVERSION("ExeInfo",                                /* application name */
              0x00010206,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


/*****************************************************************************
 * Name      : void ExeAnalyse
 * Funktion  : Analyse the executable image, stage one, general information
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

void ExeAnalyse (void)
{
  PUINT16 pPtr  = Globals.pExeBuffer;


  Globals.pExePointer = Globals.pExeBuffer;                /* initialization */

  while (Globals.pExePointer != NULL)         /* until the end of processing */
  {
    pPtr = (PUINT16)Globals.pExePointer;

    switch (* pPtr)
    {
      case EXEIMAGE_DOS_SIGNATURE:    ExeAnalyseHeaderMZ(Globals.pExePointer); break;
      case EXEIMAGE_OS2_SIGNATURE:    ExeAnalyseHeaderNE(Globals.pExePointer); break;
      case EXEIMAGE_OS2_SIGNATURE_LX: ExeAnalyseHeaderLX(Globals.pExePointer); break;
      case EXEIMAGE_VXD_SIGNATURE:    ExeAnalyseHeaderLE(Globals.pExePointer); break;
      case EXEIMAGE_NT_SIGNATURE:     ExeAnalyseHeaderPE(Globals.pExePointer); break;

      /* unknown header */
      default:
      {
        printf ("\nNo executable image or "
                "no known signature (\"%c%c\" - %04xh), aborting.",
                *pPtr & 0xff,
                *pPtr >> 8,
                *pPtr);

        Globals.pExePointer = NULL;              /* signal end of processing */
      }
    } /* end switch */
  }
}


/*****************************************************************************
 * Name      : APIRET display_information
 * Funktion  : Display all information about the file
 * Parameter : PSZ pszFile
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

APIRET display_information (PSZ pszFile)
{
  APIRET      rc;                                         /* API return code */

  rc = ToolsReadFileToBuffer(pszFile,                       /* read the file */
                             &Globals.pExeBuffer,
                             &Globals.ulExeSize);
  if (rc == NO_ERROR)
  {
    printf ("\n[%s, %u bytes]",
            pszFile,
            Globals.ulExeSize);

    ExeAnalyse();                                 /* call analyser functions */

#ifdef __OS2__
    rc = DosFreeMem(Globals.pExeBuffer);           /* free the memory buffer */
#endif

#ifdef _WIN32
    VirtualFree(Globals.pExeBuffer,
                0,
                MEM_RELEASE);
    rc = NO_ERROR;                /* we don't care about the possible errors */
#endif
  }
  return (rc);                                             /* signal success */
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

  memset(&Globals,
         0L,
         sizeof(Globals));
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

  if ( (Options.pszFile == NULL) ||          /* check if user specified file */
       (Options.fsHelp) )
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  rc = display_information (Options.pszFile);
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  return (rc);
}
