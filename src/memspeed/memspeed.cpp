/*
 * Memory Speed Testing Utility
 *
 * (C)'02 Patrick Haller. All rights reserved.
 *
 * Memory transfer routines are from SSSBBAC
 * Copyright (c) 2001 Takayuki 'January June' Suwa
 */


/****************************************************************************
 * Compilation conditionals
 ****************************************************************************/


/****************************************************************************
 * Includes
 ****************************************************************************/

#define INCL_OS2
#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <process.h>
#include <float.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"

#include "util.h"


/****************************************************************************
 * Defines
 ****************************************************************************/

#define MAXPATHLEN 260

#define CPUFEATURE_FPU 0x00000001L  /* FPU on-chip */
#define CPUFEATURE_MMX 0x00800000L  /* MMX */
#define CPUFEATURE_SSE 0x02000000L  /* Streaming SIMD Extensions */



/****************************************************************************
 * Externals
 ****************************************************************************/

extern ULONG _Optlink IdentifyCpuFeature(VOID);

extern VOID _Optlink TransferScanline_MMX(PVOID pbDst,   /* eax */
                                          PVOID pbSrc,   /* edx */
                                          ULONG ulBytes  /* ecx */);

extern VOID _Optlink TransferScanline_MMX2(PVOID pbDst,   /* eax */
                                           PVOID pbSrc,   /* edx */
                                           ULONG ulBytes  /* ecx */);

extern VOID _Optlink TransferScanline_Generic(PVOID pbDst,   /* eax */
                                              PVOID pbSrc,   /* edx */
                                              ULONG ulBytes  /* ecx */);


/****************************************************************************
 * Implementation
 ****************************************************************************/



/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsNoFPU;                                /* don't test FPU transfer */
  ARGFLAG fsNoMMX;                                /* don't test MMX transfer */
  ARGFLAG fsNoSSE;                                /* don't test SSE transfer */
} OPTIONS, *POPTIONS;


typedef struct tagMemBench
{
    ULONG ulBlockSize;
    double dblTime;
} MEMBENCH, *PMEMBENCH;


// benchmark function definition
typedef VOID _Optlink FNMEMOP(PVOID pBuffer1, PVOID pBuffer2, ULONG ulSize);
typedef FNMEMOP* PFNMEMOP;


// forwarder declarations
extern VOID _Optlink mbOverhead(PVOID pBuffer1, PVOID pBuffer2, ULONG ulSize);
extern VOID _Optlink mbCPUCopy (PVOID pBuffer1, PVOID pBuffer2, ULONG ulSize);
extern VOID _Optlink mbCPUMove (PVOID pBuffer1, PVOID pBuffer2, ULONG ulSize);
extern VOID _Optlink mbFPU     (PVOID pBuffer1, PVOID pBuffer2, ULONG ulSize);
extern VOID _Optlink mbMMX     (PVOID pBuffer1, PVOID pBuffer2, ULONG ulSize);
extern VOID _Optlink mbSSE     (PVOID pBuffer1, PVOID pBuffer2, ULONG ulSize);


#define MEMBENCHES 14
typedef struct tagMemOp
{
    PSZ      pszName;
    ULONG    ulRequiredCPUFeatures;
    PFNMEMOP pfnMemOp;
    MEMBENCH arrMemBench[MEMBENCHES];
} MEMOP, *PMEMOP;


#define MEMBENCH_SIZE(a) {a,0.0}
#define __MEMBENCHES   { \
MEMBENCH_SIZE(16),       \
MEMBENCH_SIZE(32),       \
MEMBENCH_SIZE(64),       \
MEMBENCH_SIZE(256),      \
MEMBENCH_SIZE(512),      \
MEMBENCH_SIZE(1024),     \
MEMBENCH_SIZE(2048),     \
MEMBENCH_SIZE(4096),     \
MEMBENCH_SIZE(16384),    \
MEMBENCH_SIZE(65536),    \
MEMBENCH_SIZE(262144),   \
MEMBENCH_SIZE(1048576),  \
MEMBENCH_SIZE(4*1048576),\
MEMBENCH_SIZE(8*1048576) }


#define MEMOPS 6
MEMOP arrMemoryOperations[ MEMOPS ] =
{
    {"Overhead", 0,               mbOverhead, __MEMBENCHES},
    {"CPU copy", 0,               mbCPUCopy,  __MEMBENCHES},
    {"CPU move", 0,               mbCPUMove,  __MEMBENCHES},
    {"FPU",      CPUFEATURE_FPU,  mbFPU,      __MEMBENCHES},
    {"MMX",      CPUFEATURE_MMX,  mbMMX,      __MEMBENCHES},
    {"SSE",      CPUFEATURE_SSE,  mbSSE,      __MEMBENCHES}
};


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */


ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung---------------pTarget----------------ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",        NULL,                  ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",        NULL,                  ARG_NULL,       &Options.fsHelp},
  {"/!FPU",      "DOn't test FPU transfer.",NULL,                  ARG_NULL,       &Options.fsNoFPU},
  {"/!MMX",      "DOn't test MMX transfer.",NULL,                  ARG_NULL,       &Options.fsNoMMX},
  {"/!SSE",      "DOn't test SSE transfer.",NULL,                  ARG_NULL,       &Options.fsNoSSE},
  ARG_TERMINATE
};


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
  TOOLVERSION("MemSpeed",                                /* application name */
              0x00010001,                            /* application revision */
              0x00010100,           /* minimum required revision of PHSTOOLS */
              0x0001FFFF,        /* maximum recommended revision of PHSTOOLS */
              NULL,                                                /* Remark */
              NULL);                                 /* additional copyright */
}


/***********************************************************************
 * Name      : MemBenchmark
 * Funktion  : run that single specified benchmark
 * Parameter : 
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-08-07]
 ***********************************************************************/

void _Optlink mbOverhead(PVOID pBuffer1, PVOID pBuffer2, ULONG ulSize)
{
}


void _Optlink mbCPUCopy(PVOID pBuffer1, PVOID pBuffer2, ULONG ulSize)
{
    memcpy(pBuffer1, pBuffer2, ulSize);
}

void _Optlink mbCPUMove(PVOID pBuffer1, PVOID pBuffer2, ULONG ulSize)
{
    memmove(pBuffer1, pBuffer2, ulSize);
}

void _Optlink mbMMX    (PVOID pBuffer1, PVOID pBuffer2, ULONG ulSize)
{
    TransferScanline_MMX(pBuffer1, pBuffer2, ulSize);
}

void _Optlink mbSSE    (PVOID pBuffer1, PVOID pBuffer2, ULONG ulSize)
{
    TransferScanline_MMX2(pBuffer1, pBuffer2, ulSize);
}

void _Optlink mbFPU    (PVOID pBuffer1, PVOID pBuffer2, ULONG ulSize)
{
    TransferScanline_Generic(pBuffer1, pBuffer2, ulSize);
}


void MemBenchmark(PMEMOP    pOp,
                  PMEMBENCH pMemBench)
{
    // measure timing overhead
    ULONG ul1Hi;
    ULONG ul1Lo;
    ULONG ul2Hi;
    ULONG ul2Lo;
    ULONG ulSize = pMemBench->ulBlockSize;
    double dblDelta = 0.0;
    double dblOverhead = 0.0;
    int    i;

    PVOID pBuffer1 = malloc( ulSize );
    PVOID pBuffer2 = malloc( ulSize );

    // touch the buffers to avoid paging
    memset(pBuffer1, 1, ulSize);
    memset(pBuffer2, 2, ulSize);


    // load the benchmark function into CPU cache
    pOp->pfnMemOp(pBuffer1, pBuffer2, ulSize);


    // close out other tasks
    DosSetPriority(PRTYS_PROCESS,
                   PRTYC_TIMECRITICAL,
                   PRTYD_MAXIMUM,
                   getpid());


#define MEM_RUNS 100

    // run the overhead determinator
    for (i = 0;
         i < MEM_RUNS;
         i++)
    {
        // call the benchmark function
        ProfileGetTimestamp(&ul1Hi, &ul1Lo);
        ProfileGetTimestamp(&ul2Hi, &ul2Lo);

        // sum up the time
        dblDelta += ul2Lo - ul1Lo;
        dblDelta += ((ul2Hi - ul1Hi) * 65536.0 * 65536.0);
    }
    dblOverhead = dblDelta / MEM_RUNS;
    dblDelta = 0.0;

    // quickly schedule other threads
    DosSleep(1L);

    // run the benchmark
    for (i = 0;
         i < MEM_RUNS;
         i++)
    {
        // call the benchmark function
        ProfileGetTimestamp(&ul1Hi, &ul1Lo);
        pOp->pfnMemOp(pBuffer1, pBuffer2, ulSize);
        ProfileGetTimestamp(&ul2Hi, &ul2Lo);

        // reset the FPU after FTP/MMX transfers
        _fpreset();

        // sum up the time
        dblDelta += ul2Lo - ul1Lo;
        dblDelta += ((ul2Hi - ul1Hi) * 65536.0 * 65536.0);
    }

    // write the result into the target structure
    pMemBench->dblTime = dblDelta / MEM_RUNS - dblOverhead;
#undef MEM_RUNS

    // close out other tasks
    DosSetPriority(PRTYS_PROCESS,
                   PRTYC_REGULAR,
                   PRTYD_MINIMUM,
                   getpid());

    free( pBuffer1 );
    free( pBuffer2 );
}


/***********************************************************************
 * Name      : PrintBenchResult
 * Funktion  : print formatted output of benchmark result
 * Parameter : 
 * Variablen :
 * Ergebnis  :
 * Bemerkung : output is 10 characters wide
 *
 * Autor     : Patrick Haller [2002-08-07]
 ***********************************************************************/

void PrintBenchResult(PMEMOP    pOp,
                      PMEMBENCH pMemBench)
{
    printf("%9.0f ", pMemBench->dblTime);
}


/***********************************************************************
 * Name      : MemSpeed
 * Funktion  : 
 * Parameter : 
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-08-07]
 ***********************************************************************/

void MemSpeed(void)
{
    // identify CPU features
    ULONG ulCpuFeature = IdentifyCpuFeature();

    printf("CPU Identification:\n"
           "  Floating Point Unit on-chip (FPU): %s\n"
           "  Multimedia Extensions       (MMX): %s\n"
           "  Streaming SIMD Extensions   (SSE): %s\n\n",
           (ulCpuFeature & CPUFEATURE_FPU) ? "yes" : "no",
           (ulCpuFeature & CPUFEATURE_MMX) ? "yes" : "no",
           (ulCpuFeature & CPUFEATURE_SSE) ? "yes" : "no");


    // Turn off deselected tests
    if (Options.fsNoFPU) ulCpuFeature &= ~CPUFEATURE_FPU;
    if (Options.fsNoMMX) ulCpuFeature &= ~CPUFEATURE_MMX;
    if (Options.fsNoSSE) ulCpuFeature &= ~CPUFEATURE_SSE;


    // measure timing overhead
    ULONG ul1Hi;
    ULONG ul1Lo;
    ULONG ul2Hi;
    ULONG ul2Lo;
    ULONG ulDelta = 0;


    // close out other tasks
    DosSetPriority(PRTYS_PROCESS,
                   PRTYC_TIMECRITICAL,
                   PRTYD_MAXIMUM,
                   getpid());

    for (int i = 0;
         i < 1000;
         i++)
    {
        ProfileGetTimestamp(&ul1Hi, &ul1Lo);
        ProfileGetTimestamp(&ul2Hi, &ul2Lo);

        ulDelta += ul2Lo - ul1Lo;
    }

    // close out other tasks
    DosSetPriority(PRTYS_PROCESS,
                   PRTYC_REGULAR,
                   PRTYD_MINIMUM,
                   getpid());

    printf("  Timing overhead      (CPU ticks) : %d\n",
           ulDelta / 1000);


    /*
     * Start that circus maximus
     */

    // print title line
    printf("Size     ");
    for(int iOp = 0;
        iOp < MEMOPS;
        iOp++)
    {
        PMEMOP pOp = &arrMemoryOperations[ iOp ];

        printf("%-10s", pOp->pszName);
    }
    printf("\n");



    for(int iBench = 0;
        iBench < MEMBENCHES;
        iBench++)
    {
        printf("%8d ",
               arrMemoryOperations[ 0 ].arrMemBench[ iBench ].ulBlockSize);

        for(int iOp = 0;
            iOp < MEMOPS;
            iOp++)
        {
            PMEMOP pOp = &arrMemoryOperations[ iOp ];
            PMEMBENCH pMemBench = &pOp->arrMemBench[ iBench ];

            // verify if operation is supported
            if ( (ulCpuFeature & pOp->ulRequiredCPUFeatures)
                 == pOp->ulRequiredCPUFeatures)
            {
                // do the benchmark
                MemBenchmark(pOp,
                             pMemBench);

                // print the result
                PrintBenchResult(pOp,
                                 pMemBench);
            }
            else
                printf("-- n/a -- ");
        }

        printf("\n");
    }
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

  MemSpeed();

  return (NO_ERROR);
}
