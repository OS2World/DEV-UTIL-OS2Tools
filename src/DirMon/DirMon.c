/***********************************************************************
 * Name      : Directory Monitor
 * Funktion  : Beobachtet Veraenderungen in einem Verzeichnis
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:25:17]
 ***********************************************************************/

/* #define DEBUG */


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#ifdef __OS2__
  #define INCL_DOSFILEMGR
  #define INCL_BASE
  #define INCL_DOS
  #define INCL_NOPMAPI
  #include <os2.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <io.h>
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

typedef PSZ *PPSZ;

typedef struct
{
  int dummy;
} GLOBALS, *PGLOBALS;


/*****************************************************************************
 * Undocumented                                                              *
 *****************************************************************************/

#pragma pack(1)
typedef struct _CNPATH {      /* CHANGENOTIFYPATH */
   ULONG   oNextEntryOffset;
   ULONG   wFlags;
   USHORT  cbName;
   CHAR    szName[1];
} CNPATH;

typedef CNPATH *PCNPATH;

typedef struct _CNINFO {      /* CHANGENOTIFYINFO */
   ULONG   oNextEntryOffset;
   CHAR    bAction;
   USHORT  cbName;
   CHAR    szName[1];
} CNINFO;

typedef CNINFO *PCNINFO;
#pragma pack()

/* Equates for ChangeNotifyInfo baction field */

#define             RCNF_FILE_ADDED        0x0001
#define             RCNF_FILE_DELETED      0x0002
#define             RCNF_DIR_ADDED         0x0003
#define             RCNF_DIR_DELETED       0x0004
#define             RCNF_MOVED_IN          0x0005
#define             RCNF_MOVED_OUT         0x0006
#define             RCNF_CHANGED           0x0007
#define             RCNF_OLDNAME           0x0008
#define             RCNF_NEWNAME           0x0009
#define             RCNF_DEVICE_ATTACHED   0x000A
#define             RCNF_DEVICE_DETACHED   0x000B

APIRET  APIENTRY DosOpenChangeNotify(PCNPATH PathBuf,
                                     ULONG   LogSize,
                                     PHDIR   hdir,
                                     ULONG   ulReserved);

APIRET  APIENTRY DosResetChangeNotify(PCNINFO LogBuf,
                                      ULONG   BufferSize,
                                      PULONG  LogCount,
                                      HDIR    hdir);

APIRET  APIENTRY DosCloseChangeNotify(HDIR    hdir);


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/
OPTIONS Options;                /* this structure holds command line options */
GLOBALS Globals;                /* this structure holds global variables     */


ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung---------------pTarget----------------ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",        NULL,                  ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",        NULL,                  ARG_NULL,       &Options.fsHelp},
  ARG_TERMINATE
};


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/
void  help               (void);
int   main               (int, char **);


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

void help ()
{
  TOOLVERSION("DirMonitor",                              /* application name */
              0x00010000,                            /* application revision */
              0x00010100,           /* minimum required revision of PHSTOOLS */
              0x0001FFFF,        /* maximum recommended revision of PHSTOOLS */
              NULL,                                                /* Remark */
              NULL);                                 /* additional copyright */
}


/***********************************************************************
 * Name      : APIRET DirWatch
 * Funktion  : This routine monitors changes to the directory
 * Parameter :
 * Variablen :
 * Ergebnis  : API return code
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 ***********************************************************************/

APIRET DirWatch(void)
{
  APIRET  rc;                                              /* API returncode */
  HDIR    hDir = HDIR_SYSTEM;                 /* the directory being watched */
  PCNPATH pcnPath;                                     /* the logging buffer */
  PCNPATH pcnPathIterator;             /* iterator over the change log table */
  PCNINFO pcnInfo;                                     /* the logging buffer */
  PCNINFO pcnInfoIterator;             /* iterator over the change log table */
  ULONG   ulChanges;                                       /* change counter */
  PSZ     pszAction;                            /* the action string pointer */

#define LOGBUFSIZE 65535

  pcnPath = malloc(LOGBUFSIZE);       /* allocate 64k for the logging buffer */
  if (pcnPath == NULL)                        /* check for proper allocation */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */

  pcnInfo = malloc(LOGBUFSIZE);       /* allocate 64k for the logging buffer */
  if (pcnInfo == NULL)                        /* check for proper allocation */
  {
    free(pcnPath);                       /* free previously allocated memory */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */
  }


  /* do we have to fill the cnp structure ? */
  memset (pcnPath,
          0,
          LOGBUFSIZE);
  /* not neccessary to fill the buffer at all.
     The WPS doesn't do it neither. */

  rc = DosOpenChangeNotify(pcnPath,
                           LOGBUFSIZE,
                           &hDir,
                           0);
  /* fprintf (stderr,
           "\nDEBUG: OpenNotify = %u",
           rc);
   */

  if (rc == ERROR_ACCESS_DENIED)
  {
    fprintf (stderr,
             "\nError: Workplace Shell running ? (access denied)");
  }

#if 0
  if (rc != NO_ERROR)                                    /* check for errors */
    return(rc);                                /* then raise error condition */
#endif

  for(;!kbhit();)
  {
    ulChanges=1;
    rc = DosResetChangeNotify(pcnInfo,
                              LOGBUFSIZE,
                              &ulChanges,
                              hDir);
    if (rc)
      fprintf (stderr,
               "\nDEBUG: ResetNotify = %u, change=%u",
               rc,
               ulChanges);

    if (rc != NO_ERROR)                                  /* check for errors */
      return(rc);                              /* then raise error condition */

    /* display results */
    pcnInfoIterator = pcnInfo;

    do
    {
      switch (pcnInfoIterator->bAction)
      {
        case RCNF_FILE_ADDED:     pszAction = "File added"; break;
        case RCNF_FILE_DELETED:   pszAction = "File deleted"; break;
        case RCNF_DIR_ADDED:      pszAction = "Dir added"; break;
        case RCNF_DIR_DELETED:    pszAction = "Dir deleted"; break;
        case RCNF_MOVED_IN:       pszAction = "Moved in"; break;
        case RCNF_MOVED_OUT:      pszAction = "Moved out"; break;
        case RCNF_CHANGED:        pszAction = "Changed"; break;
        case RCNF_OLDNAME:        pszAction = "Old name"; break;
        case RCNF_NEWNAME:        pszAction = "New name"; break;
        case RCNF_DEVICE_ATTACHED:pszAction = "Device attached"; break;
        case RCNF_DEVICE_DETACHED:pszAction = "Device detached"; break;
        default:                  pszAction = "<unknown>"; break;
      }

      printf ("\n%-16s %s",
              pszAction,
              pcnInfoIterator->szName);

      if (pcnInfoIterator->oNextEntryOffset == 0)
        ulChanges=0;

      pcnInfoIterator = (CNINFO *) ((BYTE*)pcnInfoIterator +
                                    pcnInfoIterator->oNextEntryOffset);
    }
    while (ulChanges);
  }

  rc = DosCloseChangeNotify(hDir);               /* close the notifier again */

/*
  fprintf (stderr,
           "\nDEBUG: CloseNotify = %u",
           rc);
*/
  return (rc);                                                         /* OK */
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

  memset (&Options,                      /* initialize the global structures */
          0,
          sizeof(Options));

  memset (&Globals,
          0,
          sizeof(Globals));

  rc = ArgStandard (argc,                            /* CLI-Parameter parsen */
                    argv,
                    TabArguments,
                    &Options.fsHelp);
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    exit(1);                                                /* abort program */
  }

  if (Options.fsHelp)                                  /* user requests help */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  /***************************************************************************
   * Map arguments                                                           *
   ***************************************************************************/

  rc = DirWatch();
  if (rc != NO_ERROR)                                 /* print error message */
    ToolsErrorDos(rc);

  return (rc);
}
