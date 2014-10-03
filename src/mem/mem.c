/*****************************************************
 * DOS-like free / used memory tool.                 *
 * (c) 2001 Patrick Haller                           *
 *****************************************************/

/* To Do
 - show filesystem cache sizes
 - further buffer information?
 - theseus support?
 - Perf-Ctrs support?
 - DosQueryPageUsage support?
 - function for continuous monitoring
 */



#ifdef __OS2__
  #define INCL_DOS
  #define INCL_DOSERRORS
  #define INCL_NOPM
  #include <os2.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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
{ /*Token--------Beschreibung------------------pTarget-ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",           NULL,   ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",           NULL,   ARG_NULL,       &Options.fsHelp},
  ARG_TERMINATE
};


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

APIRET16 APIENTRY16 Dos16MemAvail(PULONG pulAvailMem);


void help       (void);

int  main       (int argc, char *argv[]);


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
  TOOLVERSION("Mem",                                    /* application name */
              0x00010000,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


/***********************************************************************
 * Name      : APIRET memGetSnapshot
 * Funktion  : get current system memory snapshot
 * Parameter : PMEMSNAPSHOT pMemSnapshot
 * Variablen :
 * Ergebnis  : Fehlercode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 12.09.1995 06.56.43]
 ***********************************************************************/

/* not all of these values are defined in elder toolkits */
#ifndef QSV_NUMPROCESSORS
#define QSV_NUMPROCESSORS       26
#endif

#ifndef QSV_MAXHPRMEM
#define QSV_MAXHPRMEM           27
#endif

#ifndef QSV_MAXHSHMEM
#define QSV_MAXHSHMEM           28
#endif

#ifndef QSV_MAXPROCESSES
#define QSV_MAXPROCESSES        29
#endif

#ifndef QSV_VIRTUALADDRESSLIMIT
#define QSV_VIRTUALADDRESSLIMIT 30
#endif

#ifndef QSV_INT10ENABLED
#define QSV_INT10ENABLED        31
#endif

typedef struct tagMEMSNAPSHOT
{
    ULONG ulPageSize;                   /* memory page size (architecture) */

    ULONG ulPhysicalTotal;      /* total physical memory installed          */
    ULONG ulPhysicalUsed;       /* physical memory in use                   */
    ULONG ulPhysicalFree;       /* physical memory free                     */

    ULONG ulVirtualTotal;       /* total virtual memory currently available */
    ULONG ulVirtualUsed;        /* virtual memory in use                    */
    ULONG ulVirtualFree;        /* virtual memory free / available          */
    ULONG ulVirtualLimit;       /* maximum size of user's address space     */

                                /* fully qualified name of active swap file */
    char  pszSwapName[260];
    ULONG ulSwapTotal;          /* maximum current size of available swap   */
    ULONG ulSwapUsed;           /* swap in use                              */
    ULONG ulSwapFree;           /* swap free                                */


    ULONG ulPrivateMax;         /* maximum allowed process-private memory   */
    ULONG ulPrivateHighMax;     /* maximum allowed private high memory      */
    ULONG ulSharedMax;          /* maximum allowed shared memory            */
    ULONG ulSharedHighMax;      /* maximum allowed shared high memory       */
    ULONG ulResident;           /* currently resident memory (non-swappable)*/
} MEMSNAPSHOT, *PMEMSNAPSHOT;


#define QSI(a,b) DosQuerySysInfo(a,a,&b,sizeof(b));


APIRET memGetSnapshot(PMEMSNAPSHOT pMemSnapshot)
{
    APIRET rc;                                     /* 16-bit API return code */

    if (NULL == pMemSnapshot)                            /* check parameters */
        return ERROR_INVALID_PARAMETER;               /* return error status */

    memset(pMemSnapshot,                        /* clear all previous values */
           0,
           sizeof(MEMSNAPSHOT));

    rc = Dos16MemAvail(&pMemSnapshot->ulPhysicalFree);    /* query phys free */
    if (rc != NO_ERROR)                                  /* check for errors */
      return rc;                                        /* return with error */


         /* try "advanced" system values first, not supported on all kernels */
    rc = QSI(QSV_VIRTUALADDRESSLIMIT, pMemSnapshot->ulVirtualLimit);
    rc = QSI(QSV_MAXHSHMEM,           pMemSnapshot->ulSharedHighMax);
    rc = QSI(QSV_MAXHPRMEM,           pMemSnapshot->ulPrivateHighMax);

    rc = QSI(QSV_MAXSHMEM,            pMemSnapshot->ulSharedMax);
    rc = QSI(QSV_MAXPRMEM,            pMemSnapshot->ulPrivateMax);

    rc = QSI(QSV_TOTAVAILMEM,         pMemSnapshot->ulVirtualFree);
    rc = QSI(QSV_TOTRESMEM,           pMemSnapshot->ulResident);
    rc = QSI(QSV_TOTPHYSMEM,          pMemSnapshot->ulPhysicalTotal);
    rc = QSI(QSV_PAGE_SIZE,           pMemSnapshot->ulPageSize);

                             /* calculate some values that cannot be queried */
    pMemSnapshot->ulPhysicalUsed = pMemSnapshot->ulPhysicalTotal -
        pMemSnapshot->ulPhysicalFree;


    /* determine name of swapfile */
    /* determine current size of swap file w/ */
    /* DosFindFirst() / DosQueryPathInfo() */


#if 0
    ULONG ulVirtualTotal;       /* total virtual memory currently available */
    ULONG ulVirtualUsed;        /* virtual memory in use                    */

    PSZ   pszSwapName;          /* fully qualified name of active swap file */
    ULONG ulSwapTotal;          /* maximum current size of available swap   */
    ULONG ulSwapUsed;           /* swap in use                              */
    ULONG ulSwapFree;           /* swap free                                */
#endif

    return NO_ERROR;                                             /* OK, done */
}


/***********************************************************************
 * Name      : APIRET doMem
 * Funktion  : Show current system memory
 * Parameter :
 * Variablen :
 * Ergebnis  : Fehlercode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 12.09.1995 06.56.43]
 ***********************************************************************/

APIRET doMem()
{
  APIRET rc;                                               /* API returncode */
  MEMSNAPSHOT m;                                     /* memory snapshot data */


  rc = memGetSnapshot(&m);                             /* get memory numbers */
  if (rc != NO_ERROR)                                    /* check for errors */
      return rc;                                 /* return with error status */

  /* display the results */
  printf("Virtual Memory Manager\n"
         "  System:   page is %ub, maximum process address space is %uM\n"
         "  Physical: total %8uk   used %8uk   free %8uk\n"
         "  Virtual:  total %8uk   used %8uk   free %8uk\n",
         m.ulPageSize,
         m.ulVirtualLimit,
         m.ulPhysicalTotal >> 10,
         m.ulPhysicalUsed >> 10,
         m.ulPhysicalFree >> 10,
         m.ulVirtualTotal >> 10,
         m.ulVirtualUsed >> 10,
         m.ulVirtualFree >> 10);

  printf("Per-process Memory Management\n"
         "  Private:  max   %8uk   high %8uk\n"
         "  Shared:   max   %8uk   high %8uk\n",
         m.ulPrivateMax >> 10,
         m.ulPrivateHighMax >> 10,
         m.ulSharedMax >> 10,
         m.ulSharedHighMax >> 10);

  printf("Swapper (%s)\n"
         "  Resident:       %8uk\n"
         "  Swapper:  total %8uk   used %8uk   free %8uk\n",
         m.pszSwapName,
         m.ulResident >> 10,
         m.ulSwapTotal >> 10,
         m.ulSwapUsed >> 10,
         m.ulSwapFree >> 10);

  return (NO_ERROR); /* OK, Done */
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
  APIRET rc;                                               /* API returncode */

  rc = ArgStandard (argc,                            /* CLI-Parameter parsen */
                    argv,
                    TabArguments,
                    &Options.fsHelp);
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    exit(1);                                                /* abort program */
  }

  if ( Options.fsHelp)                                 /* user requests help */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  /* Anzeigen der Informationen */
  rc = doMem();
  if (rc != NO_ERROR)
      ToolsErrorDos(rc);

  return rc;
}
