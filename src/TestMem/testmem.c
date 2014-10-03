/*****************************************************************************
 * TestMem Tool.                                                             *
 * Test the system memory.                                                   *
 * (c) 1997 Patrick Haller Systemtechnik                                     *
 *****************************************************************************/

/* @@@PH - Exception Handler !
         - support for named / unnamed shared memory
         - Information about shared memory, etc.
         - Dump
*/

/* #define DEBUG 1 */

/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#define INCL_DOSFILEMGR
#define INCL_DOS
#define INCL_BASE
#define INCL_NOPMAPI
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

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
  ARGFLAG fsMemSize;          /* Size of the memory objects.                 */
  ARGFLAG fsMemNumber;        /* Number of memory objects.                   */
  ARGFLAG fsMemCommit;        /* Assign PAG_COMMIT attribute to the objects. */
  ARGFLAG fsMemWrite;         /* Assign PAG_WRITE  attribute to the objects. */
  ARGFLAG fsMemRead;          /* Assign PAG_READ   attribute to the objects. */
  ARGFLAG fsMemTile;          /* Assign OBJ_TILE   attribute to the objects. */
  ARGFLAG fsMemExecute;       /* Assign PAG_EXEC   attribute to the objects. */
  ARGFLAG fsOpQuery;          /* Perform query operations on the objects.    */
  ARGFLAG fsOpDump;           /* Perform memory dump.                        */
  ARGFLAG fsOpWrite;          /* Perform write operations on the objects.    */
  ARGFLAG fsOpRead;           /* Perform read  operations on the objects.    */
  ARGFLAG fsOpCheck;          /* Perform memory r/w tests on the objects.    */
  ARGFLAG fsOpForward;        /* Operation direction is forward.             */
  ARGFLAG fsOpBackward;       /* Operation direction is backward.            */
  ARGFLAG fsOpMixed;          /* Operation direction is mixed.               */
  ARGFLAG fsOpRandom;         /* Operation uses random addresses.            */
  ARGFLAG fsDumpStart;        /* Dump start address specified.               */
  ARGFLAG fsDumpEnd;          /* Dump end   address specified.               */
  ARGFLAG fsDumpMode;         /* Dump mode (HEX/ASCII) specified.            */

  PSZ   pszMemSize;                  /* memory size as number or "MAX", etc. */
  ULONG ulMemNumber;                              /* number of memory blocks */
  ULONG ulDumpStart;                            /* memory dump start address */
  ULONG ulDumpEnd;                                /* memory dump end address */
  PSZ   pszDumpMode;                                     /* memory dump mode */
} OPTIONS, *POPTIONS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */

ARGUMENT TabArguments[] =
{ /*Token-----------Beschreibung---------------------------------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/MEM.SIZE=",    "Size of the memory objects.",               &Options.pszMemSize,  ARG_PSZ,        &Options.fsMemSize},
  {"/MEM.NUMBER=",  "Number of memory objects.",                 &Options.ulMemNumber, ARG_ULONG,      &Options.fsMemNumber},
  {"/MEM.COMMIT",   "Assign PAG_COMMIT attribute to the objects.",NULL,                ARG_NULL,       &Options.fsMemCommit},
  {"/MEM.WRITE",    "Assign PAG_WRITE  attribute to the objects.",NULL,                ARG_NULL,       &Options.fsMemWrite},
  {"/MEM.READ",     "Assign PAG_READ   attribute to the objects.",NULL,                ARG_NULL,       &Options.fsMemRead},
  {"/MEM.TILE",     "Assign OBJ_TILE   attribute to the objects.",NULL,                ARG_NULL,       &Options.fsMemTile},
  {"/MEM.EXECUTE",  "Assign PAG_EXEC   attribute to the objects.",NULL,                ARG_NULL,       &Options.fsMemExecute},
  {"/OP.QUERY",     "Query memory attributes only.",              NULL,                ARG_NULL,       &Options.fsOpQuery},
  {"/OP.DUMP",      "Dumps every readable memory page.",          NULL,                ARG_NULL,       &Options.fsOpDump},
  {"/OP.WRITE",     "Perform write operations on the objects.",   NULL,                ARG_NULL,       &Options.fsOpWrite},
  {"/OP.READ",      "Perform read  operations on the objects.",   NULL,                ARG_NULL,       &Options.fsOpRead},
  {"/OP.CHECK",     "Perform memory r/w tests on the objects.",   NULL,                ARG_NULL,       &Options.fsOpCheck},
  {"/OP.FORWARD",   "Operation direction is forward.",            NULL,                ARG_NULL,       &Options.fsOpForward},
  {"/OP.BACKWARD",  "Operation direction is backward.",           NULL,                ARG_NULL,       &Options.fsOpBackward},
  {"/OP.MIXED",     "Operation direction is mixed.",              NULL,                ARG_NULL,       &Options.fsOpMixed},
  {"/OP.RANDOM",    "Operation uses random addresses.",           NULL,                ARG_NULL,       &Options.fsOpRandom},
  {"/DUMP.START",   "Dump range start address (if any).",         &Options.ulDumpStart,ARG_ULONG,      &Options.fsDumpStart},
  {"/DUMP.END",     "Dump range end address (if any).",           &Options.ulDumpEnd,  ARG_ULONG,      &Options.fsDumpEnd},
  {"/DUMP.MODE=",   "Dump mode, HEX or ASCII",                    &Options.pszDumpMode,ARG_PSZ,        &Options.fsDumpMode},
  {"/?",            "Get help screen.",                           NULL,                ARG_NULL,       &Options.fsHelp},
  {"/H",            "Get help screen.",                           NULL,                ARG_NULL,       &Options.fsHelp},
  ARG_TERMINATE
};


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void   help                (void);
void   initialize          (void);
APIRET TestMemory          (void);


/*****************************************************************************
 * Name      : void help
 * Funktion  : Anzeigen des Hilfetextes.
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.45.24]
 *****************************************************************************/

void help (void)
{
  TOOLVERSION("TestMem",                                /* application name */
              0x00000101,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


/*****************************************************************************
 * Name      : APIRET TestMemory
 * Funktion  : Worker routine
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

APIRET TestMemory(void)
{
  APIRET rc = NO_ERROR;                                   /* API Return Code */
  ULONG  ulMemorySize;                 /* size of one memory object in bytes */
  ULONG  ulMemoryNumber;                         /* number of memory objects */
  ULONG  ulMemoryFlags;             /* allocation attributes for the objects */

  struct
  {
    ULONG ulPageSize;                                    /* system page size */
    ULONG ulTotalPhysicalMemory;                    /* total physical memory */
    ULONG ulTotalResidentMemory;                    /* total resident memory */
    ULONG ulTotalAvailableMemory;                  /* total available memory */
    ULONG ulMaximumPrivateMemory;        /* maximum available private memory */
    ULONG ulMaximumSharedMemory;          /* maximum available shared memory */
  } sSystemValues;


  /***************************************************************************
   * Query information about memory manager from the system                  *
   * calculating all necessary parameters and values                         *
   ***************************************************************************/

  rc = DosQuerySysInfo (QSV_PAGE_SIZE,
                        QSV_PAGE_SIZE,
                        &sSystemValues.ulPageSize,
                        sizeof(sSystemValues.ulPageSize));
  if (rc != NO_ERROR)
    return (rc);                                         /* abort with error */

  rc = DosQuerySysInfo (QSV_TOTPHYSMEM,
                        QSV_MAXSHMEM,
                        &sSystemValues.ulTotalPhysicalMemory,
                        5 * sizeof(sSystemValues.ulTotalPhysicalMemory)
                       );
  if (rc != NO_ERROR)
    return (rc);                                         /* abort with error */

#define PMEM(a) sSystemValues.a, \
  (float)sSystemValues.a / 1024.0, \
  (float)sSystemValues.a / 1024.0 / 1024.0

  printf ("\nMemory Information:"
          "\n  System page size        (PAGE) : %10ub   %10.3fk   %10.3fM"
          "\n  Total physical memory   (PHYS) : %10ub   %10.3fk   %10.3fM",
          PMEM(ulPageSize),
          PMEM(ulTotalPhysicalMemory));

  printf ("\n  Total resident memory          : %10ub   %10.3fk   %10.3fM"
          "\n  Total available memory (AVAIL) : %10ub   %10.3fk   %10.3fM",
          PMEM(ulTotalResidentMemory),
          PMEM(ulTotalAvailableMemory));

  printf ("\n  Available private memory (MAX) : %10ub   %10.3fk   %10.3fM"
          "\n  Available shared  memory (SHR) : %10ub   %10.3fk   %10.3fM",
          PMEM(ulMaximumPrivateMemory),
          PMEM(ulMaximumSharedMemory));
#undef PMEM

  if (Options.fsMemSize)
  {
    if (stricmp(Options.pszMemSize,
                "MAX") == 0)
      ulMemorySize = sSystemValues.ulMaximumPrivateMemory;
    else
    {
      if (stricmp(Options.pszMemSize,
                  "PHYS") == 0)
        ulMemorySize = sSystemValues.ulTotalPhysicalMemory;
      else
      {
        if (stricmp(Options.pszMemSize,
                    "AVAIL") == 0)
          ulMemorySize = sSystemValues.ulTotalAvailableMemory;
        else
        {
          if (stricmp(Options.pszMemSize,
                      "SHR") == 0)
            ulMemorySize = sSystemValues.ulMaximumSharedMemory;
          else
          {
            if (stricmp(Options.pszMemSize,
                        "PAGE") == 0)
              ulMemorySize = sSystemValues.ulPageSize;
            else
              ulMemorySize = atoi(Options.pszMemSize);
          }
        }
      }
    }
  }
  else
    ulMemorySize = 1024 * 1024;                            /* 1MB is default */

  if (Options.fsMemNumber)
    ulMemoryNumber = Options.ulMemNumber;
  else
    ulMemoryNumber = 1;                             /* one object is default */

  printf ("\n\nAllocating %u objects at size of %u bytes each, %u in total.",
          ulMemoryNumber,
          ulMemorySize,
          ulMemoryNumber * ulMemorySize);

  ulMemoryFlags = 0;
  printf ("\nMemory object attributes:");
#define MEMA(a,b) {ulMemoryFlags |= a; printf (b);}
  if (Options.fsMemCommit ) MEMA(PAG_COMMIT, " PAG_COMMIT");
  if (Options.fsMemWrite  ) MEMA(PAG_WRITE,  " PAG_WRITE");
  if (Options.fsMemRead   ) MEMA(PAG_READ,   " PAG_READ");
  if (Options.fsMemExecute) MEMA(PAG_EXECUTE," PAG_EXECUTE");
  if (Options.fsMemTile   ) MEMA(OBJ_TILE,   " OBJ_TILE");
#undef MEMA


  /***************************************************************************
   * Now allocating the memory.                                              *
   ***************************************************************************/


  /***************************************************************************
   * Now performing certain operations on that memory.                       *
   ***************************************************************************/


#if 0
  ARGFLAG fsOpQuery;          /* Perform query operations on the objects.    */
  ARGFLAG fsOpWrite;          /* Perform write operations on the objects.    */
  ARGFLAG fsOpRead;           /* Perform read  operations on the objects.    */
  ARGFLAG fsOpCheck;          /* Perform memory r/w tests on the objects.    */
  ARGFLAG fsOpForward;        /* Operation direction is forward.             */
  ARGFLAG fsOpBackward;       /* Operation direction is backward.            */
  ARGFLAG fsOpMixed;          /* Operation direction is mixed.               */
  ARGFLAG fsOpRandom;         /* Operation uses random addresses.            */
#endif


  return (NO_ERROR);                     /* return value to the main routine */
}


/*****************************************************************************
 * Name      : void initialize
 * Funktion  : Initialisierung einiger Variablen
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

void initialize (void)
{
  memset(&Options,
         0L,
         sizeof(Options));
}


/*****************************************************************************
 * Name      : int main
 * Funktion  : Hauptroutine
 * Parameter : int argc, char *argv[]
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

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

  if (Options.fsHelp)                    /* check if help is to be displayed */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  rc = TestMemory();                             /* Perform the memory tests */
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  return (rc);
}
