/*****************************************************
 * EXEINFO Tool.                                     *
 * Reports details on a given executable file.       *
 * (c) 1994-95 Patrick Haller Systemtechnik          *
 *****************************************************/

/* #define DEBUG 1 */

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
#include "toolarg.h"
#include "exeinfo.h"

#define MAXPATHLEN 260


/*****************************************************************************
 * Name      : void ExeAnalyseMZRelocations
 * Funktion  : Dump MZ Relocation Information
 * Parameter : PSZ pMZPointer
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

void ExeAnalyseMZRelocation (PSZ pPtr)
{
  PEXEIMAGE_MZ_HEADER pMZ = (PEXEIMAGE_MZ_HEADER)pPtr; /* map pointer to MZ struct */
  ULONG            ulCounter;                /* loop counter for relocations */
  PEXEIMAGE_MZ_RELOCATION pRel;                   /* pointer to relocation data */

  printf ("\n\n  Executable Relocations (%5u - %04xh relocations)",
          pMZ->e_crlc,
          pMZ->e_crlc);

  pRel = (PEXEIMAGE_MZ_RELOCATION) ( (PSZ) pMZ +
                                  pMZ->e_lfarlc );

  for (ulCounter = 0;
       ulCounter < pMZ->e_crlc;
       ulCounter++)
  {
    if (ulCounter % 4 == 0)                            /* do some linebreaks */
      printf ("\n    %4u:", ulCounter);

    printf ("    %04xh:%04xh",
            pRel[ulCounter].mr_segment,
            pRel[ulCounter].mr_offset);
  }
}

/*****************************************************************************
 * Name      : void ExeAnalyseHeaderMZ
 * Funktion  : Dump MZ Header Information
 * Parameter : PSZ pMZPointer
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

void ExeAnalyseHeaderMZ (PSZ pPtr)
{
  PEXEIMAGE_MZ_HEADER pMZ = (PEXEIMAGE_MZ_HEADER)pPtr; /* map pointer to MZ struct */

  if (!Options.fsNoMZ)  /* only to display if MZ header is not to be ignored */
  {

    printf ("\n\n컴[MZ Header Information (DOS)]컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�"
            "\n  Magic                            :       \"%c%c\" -       %04xh",
            pMZ->e_magic & 0xFF,
            pMZ->e_magic >> 8,
            pMZ->e_magic);

    printf ("\n  Bytes on last page of file       : %10u -       %04xh",
            pMZ->e_cblp,
            pMZ->e_cblp);

    printf ("\n  Pages in file                    : %10u -       %04xh",
            pMZ->e_cp,
            pMZ->e_cp);

    printf ("\n  Relocations                      : %10u -       %04xh",
            pMZ->e_crlc,
            pMZ->e_crlc);

    printf ("\n  Size of header in paragraphs     : %10u -       %04xh",
            pMZ->e_cparhdr,
            pMZ->e_cparhdr);

    printf ("\n  Minimum extra paragraphs needed  : %10u -       %04xh",
            pMZ->e_minalloc,
            pMZ->e_minalloc);

    printf ("\n  Maximum extra paragraphs needed  : %10u -       %04xh",
            pMZ->e_maxalloc,
            pMZ->e_maxalloc);

    printf ("\n  Initial (relative) SS:SP value   :      SS:SP - %04xh:%04xh",
            pMZ->e_ss,
            pMZ->e_sp);

    printf ("\n  Initial            CS:IP value   :      CS:IP - %04xh:%04xh",
            pMZ->e_cs,
            pMZ->e_ip);

    printf ("\n  Checksum (header)                : %10u -       %04xh",
            pMZ->e_csum,
            pMZ->e_csum);

    printf ("\n           (calculated, NOT YET)   : %10u -       %04xh",
            0,
            0);

    printf ("\n  File address of relocation table : %10u -       %04xh",
            pMZ->e_lfarlc,
            pMZ->e_lfarlc);

    printf ("\n  Overlay number                   : %10u -       %04xh",
            pMZ->e_ovno,
            pMZ->e_ovno);

    printf ("\n  Linker version                   : %10u.%u",
            pMZ->e_ver >> 8,
            pMZ->e_ver & 0xff);

    printf ("\n  Behaviour bits                   : %10u -       %04xh",
            pMZ->e_bb,
            pMZ->e_bb);

    printf ("\n  OEM identifier                   : %10u -       %04xh",
            pMZ->e_oemid,
            pMZ->e_oemid);

    printf ("\n  OEM information                  : %10u -       %04xh",
            pMZ->e_oeminfo,
            pMZ->e_oeminfo);

    printf ("\n  File address of new exe header   : %10u -   %08xh",
            pMZ->e_lfanew,
            pMZ->e_lfanew);

    if (!Options.fsNoRelocations) ExeAnalyseMZRelocation( (PSZ) pMZ);
  }

  if (pMZ->e_lfanew != 0)         /* if there is a value different from NULL */
        /* if is difficult to decide whether e_lfanew is valid or not. So we */
        /* just add the value there to the Global pointer */
  {
    if ( (UINT32)(pMZ->e_lfanew) >= Globals.ulExeSize)
    {
      printf ("\n  File address of new exe header is definately invalid.");
      Globals.pExePointer = NULL;                /* signal end of processing */
    }
    else
      Globals.pExePointer += (UINT32)(pMZ->e_lfanew);
  }
  else
    Globals.pExePointer = NULL;                  /* signal end of processing */
}

