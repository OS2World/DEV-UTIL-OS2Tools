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
 * Name      : void ExeAnalyseHeaderLE
 * Funktion  : Dump LE Header Information
 * Parameter : PSZ pPointer
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

void ExeAnalyseHeaderLE (PSZ pPtr)
{
  PEXEIMAGE_LE_HEADER pLE = (PEXEIMAGE_LE_HEADER)pPtr; /* map pointer to LE struct */

  printf ("\n\n컴[LE Header Information (Windows 95 virtual device drivers, VxD)]컴컴컴컴컴"
          "\n  Magic                            :       \"%c%c\" -       %04xh",
          pLE->e_magic & 0xFF,
          pLE->e_magic >> 8,
          pLE->e_magic);


  printf ("\n  The byte ordering for the .EXE   :        %02xh ",
          pLE->e_border);
  if (pLE->e_border == E32LEBO) printf ("- Little Endian Byte Order");
  else
    if (pLE->e_border == E32BEBO) printf ("- Big Endian Byte Order");

  printf ("\n  The word ordering for the .EXE   :        %02xh ",
          pLE->e_worder);
  if (pLE->e_worder == E32LEWO) printf ("- Little Endian Word Order");
  else
    if (pLE->e_worder == E32BEWO) printf ("- Big Endian Word Order");

  printf ("\n  The EXE format level             : %10u -   %08xh",
          pLE->e_level,
          pLE->e_level);

  printf ("\n  The CPU type                     : ");
  switch (pLE->e_cpu)
  {
    case E32CPU286: printf ("Intel 80286 or upwardly compatible"); break;
    case E32CPU386: printf ("Intel 80386 or upwardly compatible"); break;
    case E32CPU486: printf ("Intel 80486 or upwardly compatible"); break;
    default:        printf ("(unknown 0x%04x)",pLE->e_cpu); break;
  }

  printf ("\n  The Operating System type (?)    : ");
  switch (pLE->e_os)
  {
    case NE_UNKNOWN:
      printf ("unknown / new-format operating system");
      break;

    case NE_OS2:
      printf ("Operating System/2");
      break;

    case NE_WINDOWS:
      printf ("Windows");
      break;

    case NE_DOS4:
      printf ("DOS 4.x");
      break;

    case NE_DEV386:
      printf ("Windows 386");
      break;

    default:
      printf ("(undefined operating system %04x)",
              pLE->e_os);
      break;
  }


  printf ("\n  Module version                   : %4u.%02u.%02u",
          pLE->e_ver >> 16,
          pLE->e_ver >> 24,
          pLE->e_ver & 0xFF);

  printf ("\n  Module flags                     : %10u -   %08xh",
          pLE->e_mflags,
          pLE->e_mflags);

  if (pLE->e_mflags & 0xFBFC0000)
    printf ("\n    Unknown flags (%08xh)",pLE->e_mflags & 0xFBFC0000);

  printf ("\n    Module Type is ");
  switch (pLE->e_mflags & E32MODMASK)
  {
    case E32PROTDLL:   printf("Protected memory library module"); break;
    case E32MODEXE:    printf(".EXE module"); break;
    case E32MODDLL:    printf(".DLL module"); break;
    case E32MODPROTDLL:printf("Protected memory library module"); break;
    case E32MODPDEV:   printf("Physical device driver"); break;
    case E32MODVDEV:   printf("Virtual device driver"); break;
    default:           printf("(unknown type %08xh)",
                              pLE->e_mflags & E32MODMASK); break;
  }

  if (pLE->e_mflags & E32NOTP)
    printf ("\n    0x00008000 - Library Module - not a process");

  if (pLE->e_mflags & 0x00004000)
    printf ("\n    0x00004000 - (unknown)");

  if (pLE->e_mflags & E32NOLOAD)
    printf ("\n    0x00002000 - Module is not loadable (errors in EXEIMAGE)");

  if (pLE->e_mflags & 0x00001800)
    printf ("\n    0x00001800 - (unknown %08xh)",
            pLE->e_mflags & 0x00001800);

  switch (pLE->e_mflags & E32APPMASK)
  {
    case E32NOPMW:
      printf ("\n    0x00000100 - not windowing compatible");
      break;

    case E32PMW:
      printf ("\n    0x00000200 - windowing compatible with PM");
      break;

    case E32PMAPI:
      printf ("\n    0x00000300 - uses windowing API (PM)");
      break;

    case 0x00000000:
      printf ("\n    0x00000300 - non-interactive (Device Driver)");
      break;

    default:
      printf ("\n    0x%08x - (unknown)",
             pLE->e_mflags & E32APPMASK);
      break;
  }

  if (pLE->e_mflags & 0x000000CB)
    printf ("\n    0x000000CB - (unknown %08xh)",
            pLE->e_mflags & 0x000000CB);

  if (pLE->e_mflags & E32NOEXTFIX)
    printf ("\n    0x00000020 - no external fixups in .EXE");

  if (pLE->e_mflags & E32NOINTFIX)
    printf ("\n    0x00000010 - no internal fixups in .EXE");

  if (pLE->e_mflags & E32LIBINIT)
    printf ("\n    0x00000004 - per-process library initialization");

  if (pLE->e_mflags & E32LIBTERM)
    printf ("\n    0x40000000 - per-process library termination");


  printf ("\n  Module pages                     : %10u -   %08xh",
          pLE->e_mpages,
          pLE->e_mpages);

  printf ("\n  Object # for instruction pointer : %10u -   %08xh",
          pLE->e_startobj,
          pLE->e_startobj);

  printf ("\n  Extended instruction pointer(EIP): %10u -   %08xh",
          pLE->e_eip,
          pLE->e_eip);

  printf ("\n  Object # for stack pointer       : %10u -   %08xh",
          pLE->e_startobj,
          pLE->e_startobj);

  printf ("\n  Extended stack pointer      (ESP): %10u -   %08xh",
          pLE->e_esp,
          pLE->e_esp);

  printf ("\n  Page size                        : %10u -   %08xh",
          pLE->e_pagesize,
          pLE->e_pagesize);

  printf ("\n  Last page size in VXD            : %10u -   %08xh",
          pLE->e_lastpagesize,
          pLE->e_lastpagesize);

  printf ("\n  Fixup section size               : %10u -   %08xh",
          pLE->e_fixupsize,
          pLE->e_fixupsize);

  printf ("\n  Fixup section checksum           : %10u -   %08xh",
          pLE->e_fixupsum,
          pLE->e_fixupsum);

  printf ("\n  Loader section size              : %10u -   %08xh",
          pLE->e_ldrsize,
          pLE->e_ldrsize);

  printf ("\n  Loader section checksum          : %10u -   %08xh",
          pLE->e_ldrsum,
          pLE->e_ldrsum);

  printf ("\n  Object table offset              : %10u -   %08xh",
          pLE->e_objtab,
          pLE->e_objtab);

  printf ("\n  Number of objects in module      : %10u -   %08xh",
          pLE->e_objcnt,
          pLE->e_objcnt);

  printf ("\n  Object page map offset           : %10u -   %08xh",
          pLE->e_objmap,
          pLE->e_objmap);

  printf ("\n  Object iterated data map offset  : %10u -   %08xh",
          pLE->e_itermap,
          pLE->e_itermap);

  printf ("\n  Resource table offset            : %10u -   %08xh",
          pLE->e_rsrctab,
          pLE->e_rsrctab);

  printf ("\n  Resource table entries           : %10u -   %08xh",
          pLE->e_rsrccnt,
          pLE->e_rsrccnt);

  printf ("\n  Resident names table offset      : %10u -   %08xh",
          pLE->e_restab,
          pLE->e_restab);

  printf ("\n  Entry table offset               : %10u -   %08xh",
          pLE->e_enttab,
          pLE->e_enttab);

  printf ("\n  Module directive table           : %10u -   %08xh",
          pLE->e_dirtab,
          pLE->e_dirtab);

  printf ("\n  Module directive table entries   : %10u -   %08xh",
          pLE->e_dircnt,
          pLE->e_dircnt);

  printf ("\n  Fixup page table offset          : %10u -   %08xh",
          pLE->e_fpagetab,
          pLE->e_fpagetab);

  printf ("\n  Fixup Record table offset        : %10u -   %08xh",
          pLE->e_frectab,
          pLE->e_frectab);

  printf ("\n  Import module name table offset  : %10u -   %08xh",
          pLE->e_impmod,
          pLE->e_impmod);

  printf ("\n  Import module name table entries : %10u -   %08xh",
          pLE->e_impmodcnt,
          pLE->e_impmodcnt);

  printf ("\n  Import procedure name table off. : %10u -   %08xh",
          pLE->e_impproc,
          pLE->e_impproc);

  printf ("\n  Per-Page checksum table offset   : %10u -   %08xh",
          pLE->e_pagesum,
          pLE->e_pagesum);

  printf ("\n  Enumerated data pages table off. : %10u -   %08xh",
          pLE->e_datapage,
          pLE->e_datapage);

  printf ("\n  Number of preload pages          : %10u -   %08xh",
          pLE->e_preload,
          pLE->e_preload);

  printf ("\n  Non-resident names table offset  : %10u -   %08xh",
          pLE->e_nrestab,
          pLE->e_nrestab);

  printf ("\n  Non-resident names table size    : %10u -   %08xh",
          pLE->e_cbnrestab,
          pLE->e_cbnrestab);

  printf ("\n  Non-resident name table checksum : %10u -   %08xh",
          pLE->e_nressum,
          pLE->e_nressum);

  printf ("\n  Object # for automatic data      : %10u -   %08xh",
          pLE->e_autodata,
          pLE->e_autodata);


  printf ("\n  Debugging information offset     : %10u -   %08xh",
          pLE->e_debuginfo,
          pLE->e_debuginfo);

  printf ("\n  Debugging information length     : %10u -   %08xh",
          pLE->e_debuglen,
          pLE->e_debuglen);

  printf ("\n  Instance pages in preload section: %10u -   %08xh",
          pLE->e_instpreload,
          pLE->e_instpreload);

  printf ("\n  Instance pages in demand load sec: %10u -   %08xh",
          pLE->e_instdemand,
          pLE->e_instdemand);

  printf ("\n  Size of heap (for 16-bit apps)   : %10u -   %08xh",
          pLE->e_heapsize,
          pLE->e_heapsize);

  printf ("\n  Windows Res Offset               : %10u -   %08xh",
          pLE->e_winresoff,
          pLE->e_winresoff);

  printf ("\n  Windows Res Length               : %10u -   %08xh",
          pLE->e_winreslen,
          pLE->e_winreslen);

  printf ("\n  Device ID for VxD                : %10u -       %04xh",
          pLE->e_devid,
          pLE->e_devid);

  printf ("\n  Windows DDK version for VxD      : %2u.%02u",
           pLE->e_ddkver >> 8,
           pLE->e_ddkver & 0xff);

  Globals.pExePointer = NULL;                    /* signal end of processing */
}
