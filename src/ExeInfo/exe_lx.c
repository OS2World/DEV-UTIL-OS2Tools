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


/*
  printf ("\n  Entry table offset               : %10u -   %08xh",
          pLX->e_enttab,
          pLX->e_enttab);

  printf ("\n  Module directive table           : %10u -   %08xh",
          pLX->e_dirtab,
          pLX->e_dirtab);

  printf ("\n  Module directive table entries   : %10u -   %08xh",
          pLX->e_dircnt,
          pLX->e_dircnt);

  printf ("\n  Fixup page table offset          : %10u -   %08xh",
          pLX->e_fpagetab,
          pLX->e_fpagetab);

  printf ("\n  Fixup Record table offset        : %10u -   %08xh",
          pLX->e_frectab,
          pLX->e_frectab);
*/



/*****************************************************************************
 * Name      : void ExeAnalyseLXImportProc
 * Funktion  : Dump LX ImportProc Names Information
 * Parameter : PSZ pPointer
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

typedef struct
{
  UINT8  ucLength;                            /* length of the resident name */
  UINT8  szName[1];                     /* string characters, NOT terminated */
} EXEIMAGE_LX_IMPORTPROC, *PEXEIMAGE_LX_IMPORTPROC;


void ExeAnalyseLXImportProc (PSZ pPtr)
{
  PEXEIMAGE_LX_HEADER   pLX = (PEXEIMAGE_LX_HEADER)pPtr;  /* map pointer LX struct */
  ULONG              ulCounter;                 /* loop counter for segments */
  PEXEIMAGE_LX_IMPORTPROC pImp;       /* pointer to import procedure table data */
  char               szBuffer[256];               /* buffer for module names */
  PUINT8             pucNameLength;                    /* length of the name */
  ULONG              ulTotalBytes = 0;            /* counter for total bytes */
  ULONG              ulBytes = 0;                    /* current byte counter */

  printf ("\n\n  Linear Executable Imported Procedures Names Table"
          " (%08x)",
          pLX->e_impproc);
  if (pLX->e_impproc == pLX->e_datapage)
  {
    printf (" not present.");
    return;                                    /* abort function immediately */
  }
  else
    printf("\n    Nr. Len. Name");


  pImp = (PEXEIMAGE_LX_IMPORTPROC) ( (PSZ) pLX +
                                 pLX->e_impproc);

  ulTotalBytes = pLX->e_datapage - pLX->e_impproc;   ;/* total size of table */

  for (ulCounter=0;
       ulBytes < ulTotalBytes;
       ulCounter++)
  {
    pucNameLength = (PUINT8)pImp;

    if (*pucNameLength == 0)                     /* skip this value, unknown */
      return;                                              /* abort function */
    else
    {
      strncpy (szBuffer,
               (PSZ)pucNameLength + sizeof(UINT8),
               *pucNameLength);
      szBuffer[*pucNameLength] = 0;             /* active string termination */

      printf ("\n    %3u. %3u [%s]",
              ulCounter,
              *pucNameLength,
              szBuffer);

      pImp = (PEXEIMAGE_LX_IMPORTPROC) ( (PSZ)pImp +
                                       *pucNameLength +
                                       sizeof(UINT8) );
      ulBytes += sizeof(UINT8) + *pucNameLength;
    }
  }
}


/*****************************************************************************
 * Name      : void ExeAnalyseLXNonResident
 * Funktion  : Dump LX NonResident Names Information
 * Parameter : PSZ pPointer
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

typedef struct
{
  UINT8  ucLength;                            /* length of the resident name */
  UINT8  szName[1];                     /* string characters, NOT terminated */
  UINT16 usID;                                                  /* export ID */
} EXEIMAGE_LX_NONRESIDENT, *PEXEIMAGE_LX_NONRESIDENT;


void ExeAnalyseLXNonResident (PSZ pPtr)
{
  PEXEIMAGE_LX_HEADER   pLX = (PEXEIMAGE_LX_HEADER)pPtr;  /* map pointer LX struct */
  ULONG              ulCounter;                 /* loop counter for segments */
  PEXEIMAGE_LX_NONRESIDENT pNRes;               /* pointer to module table data */
  char               szBuffer[256];               /* buffer for module names */
  UINT8              ucNameLength;                     /* length of the name */
  ULONG              ulTotalBytes = 0;            /* counter for total bytes */
  ULONG              ulBytes = 0;                    /* current byte counter */
  UINT16             usID;                                       /* entry ID */

  printf ("\n\n  Linear Executable Non-Resident Names Table");
  if (pLX->e_cbnrestab == 0)
  {
    printf (" not present.");
    return;                                    /* abort function immediately */
  }
  else
    printf("\n    Nr. ID    Len. Name");


  /* @@@PH !!! WARNING !!!
     for some strange reason, the e_nrestab offset is not relative to
     the "LX" header but absolute to the fileimage !!! This means the
     size of the "MZ" header has to be subtracted
  */


  pNRes = (PEXEIMAGE_LX_NONRESIDENT) ( (PSZ) Globals.pExeBuffer +
                                   pLX->e_nrestab );
  ulTotalBytes = pLX->e_cbnrestab;                    /* total size of table */

  for (ulCounter=0;
       ulBytes < ulTotalBytes;
       ulCounter++)
  {
    ucNameLength = pNRes->ucLength;

    if (ucNameLength == 0)                      /* skip this value, unknown */
    {
      /* @@@PH seems not to be correct
        pNRes = (PEXEIMAGE_LX_NONRESIDENT) ( (PSZ)pNRes +
                                         sizeof(UINT8) );
                                         ulBytes++;
        */
        return;
    }
    else
    {
      strncpy (szBuffer,
               (PSZ)pNRes->szName,
               ucNameLength);
      szBuffer[ucNameLength] = 0;               /* active string termination */

      if (ulCounter == 0)                                     /* 1st entry ? */
        usID = 0; /* ID ermitteln */
      else
        usID = * ( (PUINT16) ( (PSZ)pNRes +
                               ucNameLength +
                               sizeof(UINT8)
                             )
                 );

      printf ("\n    %3u. %5u %3u [%s]",
              ulCounter,
              usID,
              ucNameLength,
              szBuffer);

      pNRes = (PEXEIMAGE_LX_NONRESIDENT) ( (PSZ)pNRes +
                                       ucNameLength +
                                       sizeof(UINT8) +
                                       sizeof(UINT16) );
      ulBytes += sizeof(UINT8) +
                 sizeof(UINT16) +
                 ucNameLength;
    }
  }
}


/*****************************************************************************
 * Name      : void ExeAnalyseLXImport
 * Funktion  : Dump LX Imported Names Information
 * Parameter : PSZ pPointer
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

typedef struct
{
  UINT8 ucLength;                             /* length of the resident name */
  UINT8 szName[1];                      /* string characters, NOT terminated */
} EXEIMAGE_LX_IMPORT, *PEXEIMAGE_LX_IMPORT;


void ExeAnalyseLXImport (PSZ pPtr)
{
  PEXEIMAGE_LX_HEADER pLX = (PEXEIMAGE_LX_HEADER)pPtr;  /* map pointer LX struct */
  ULONG            ulCounter;                 /* loop counter for segments */
  PEXEIMAGE_LX_IMPORT pImp;                   /* pointer to module table data */
  char             szBuffer[256];               /* buffer for module names */
  PUINT8           pucNameLength;                    /* length of the name */

  printf ("\n\n  Linear Executable Imported Names Table");
  if (pLX->e_impmodcnt == 0)
  {
    printf (" not present.");
    return;                                    /* abort function immediately */
  }
  else
    printf("\n    Nr.   Length Index Name");

  pImp = (PEXEIMAGE_LX_IMPORT) ( (PSZ) pLX +
                              pLX->e_impmod);

  for (ulCounter=0;
       ulCounter < pLX->e_impmodcnt;
       ulCounter++)
  {
    pucNameLength = (PUINT8) ( (PSZ)pImp );

    if (*pucNameLength == 0)               /* this indicates end of the list */
      return;

    strncpy (szBuffer,
             (PSZ)pucNameLength + sizeof(UINT8),
             *pucNameLength);
    szBuffer[*pucNameLength] = 0;               /* active string termination */

    printf ("\n    %3u.     %3u   [%s]",
            ulCounter,
            *pucNameLength,
            szBuffer);

    pImp = (PEXEIMAGE_LX_IMPORT) ( (PSZ)pImp + sizeof(UINT8) + *pucNameLength);
  }
}


/*****************************************************************************
 * Name      : void ExeAnalyseLXResident
 * Funktion  : Dump LX Resident Names Information
 * Parameter : PSZ pPointer
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

typedef struct
{
  UINT8 ucLength;                             /* length of the resident name */
  UINT8 szName[1];                      /* string characters, NOT terminated */
  /* UINT16 usIndex; 16-bit index number after the string */
} EXEIMAGE_LX_RESIDENT, *PEXEIMAGE_LX_RESIDENT;


void ExeAnalyseLXResident (PSZ pPtr)
{
  PEXEIMAGE_LX_HEADER   pLX = (PEXEIMAGE_LX_HEADER)pPtr;  /* map pointer LX struct */
  ULONG              ulCounter;                 /* loop counter for segments */
  PEXEIMAGE_LX_RESIDENT pRes;                   /* pointer to module table data */
  char               szBuffer[256];               /* buffer for module names */
  PUINT8             pucNameLength;                    /* length of the name */
  PUINT16            pusIndex;                    /* pointer to index number */
  ULONG              ulTotalBytes = 0;            /* counter for total bytes */
  ULONG              ulBytes = 0;                    /* current byte counter */

  printf ("\n\n  Linear Executable Resident Names Table");
  if (pLX->e_restab == pLX->e_enttab)
  {
    printf (" not present.");
    return;                                    /* abort function immediately */
  }
  else
    printf("\n    Nr.   Length Index Name");

  pRes = (PEXEIMAGE_LX_RESIDENT) ( (PSZ) pLX +
                                pLX->e_restab);
  ulTotalBytes = pLX->e_enttab - pLX->e_restab;       /* total size of table */

  for (ulCounter=0;
       ulBytes < ulTotalBytes;
       ulCounter++)
  {
    pucNameLength = (PUINT8) ( (PSZ)pRes );

    if (*pucNameLength == 0)               /* this indicates end of the list */
      return;

    strncpy (szBuffer,
             (PSZ)pucNameLength + sizeof(UINT8),
             *pucNameLength);
    szBuffer[*pucNameLength] = 0;               /* active string termination */

    pusIndex = (PUINT16)( (PSZ)pRes +
                          sizeof(UINT8) +          /* compensate length byte */
                          *pucNameLength );        /* plus the string length */

    printf ("\n    %3u.     %3u   %3u [%s]",
            ulCounter,
            *pucNameLength,
            *pusIndex,
            szBuffer);

    if (*pusIndex == 0)                         /* is this the first entry ? */
      printf (" (Module Name)");

    pRes = (PEXEIMAGE_LX_RESIDENT) ( (PSZ)pusIndex + sizeof(UINT16) );
    ulBytes += sizeof(UINT8) + sizeof(UINT16) + *pucNameLength;
  }
}


/*****************************************************************************
 * Name      : void ExeAnalyseLXResource
 * Funktion  : Dump LX Resource Information
 * Parameter : PSZ pPointer
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

void ExeAnalyseLXResource (PSZ pPtr)
{
  PEXEIMAGE_LX_HEADER pLX = (PEXEIMAGE_LX_HEADER)pPtr; /* map pointer to LX struct */
  ULONG            ulCounter;                   /* loop counter for objects  */
  PEXEIMAGE_LX_RESOURCE pRes;                 /* pointer to resource table data */
  char             szBuffer[256];                 /* buffer for module names */
  PSZ              pszType;                 /* pointer on resource type name */

  printf ("\n\n  Linear Executable Resource Table");
  if (pLX->e_rsrctab == 0)
  {
    printf (" not present.");
    return;                                    /* abort function immediately */
  }
  else
    printf("\n    Nr.  Type          Name/ID Size                  Obj# Offset");

  pRes = (PEXEIMAGE_LX_RESOURCE) ( (PSZ) pLX +
                                pLX->e_rsrctab);

  for (ulCounter=0;
       ulCounter < pLX->e_rsrccnt;
       ulCounter++)
  {
    switch (pRes->type)
    {
      case EXERT_POINTER:      pszType="POINTER        "; break;
      case EXERT_BITMAP:       pszType="BITMAP         "; break;
      case EXERT_MENU:         pszType="MENU           "; break;
      case EXERT_DIALOG:       pszType="DIALOG         "; break;
      case EXERT_STRING:       pszType="STRINGTABLE    "; break;
      case EXERT_FONTDIR:      pszType="FONTDIRECTORY  "; break;
      case EXERT_FONT:         pszType="FONT           "; break;
      case EXERT_ACCELTABLE:   pszType="ACCELTABLE     "; break;
      case EXERT_RCDATA:       pszType="RCDATA         "; break;
      case EXERT_MESSAGE:      pszType="ERROR MSG TABLE"; break;
      case EXERT_DLGINCLUDE:   pszType="DLGINCLUDE FILE"; break;
      case EXERT_VKEYTBL:      pszType="KEY-VKEY TABLES"; break;
      case EXERT_KEYTBL:       pszType="KEY-UGL TABLES "; break;
      case EXERT_CHARTBL:      pszType="GLYPH-CHARACTER"; break;
      case EXERT_DISPLAYINFO:  pszType="DISPLAY INFO   "; break;
      case EXERT_FKASHORT:     pszType="FUNC KEY (shrt)"; break;
      case EXERT_FKALONG:      pszType="FUNC KEY (long)"; break;
      case EXERT_HELPTABLE:    pszType="HELPTABLE      "; break;
      case EXERT_HELPSUBTABLE: pszType="HELPSUBTABLE   "; break;
      case EXERT_FDDIR:        pszType="DBCS FONT DIR  "; break;
      case EXERT_FD:           pszType="DBCD FONT      "; break;
      default:              sprintf(szBuffer,
                                    "(Unknown %04xh)",
                                    pRes->type);
                            pszType=szBuffer;
               break;
    }

    printf ("\n    %3u. %s %5u %08xh,%10u %5u %08xh",
            ulCounter,
            pszType,
            pRes->name,
            pRes->cb,
            pRes->cb,
            pRes->obj,
            pRes->offset);

    pRes++;
  }
}


/*****************************************************************************
 * Name      : void ExeAnalyseLXObjects
 * Funktion  : Dump LX Object Information
 * Parameter : PSZ pPointer
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

void ExeAnalyseLXObjects (PSZ pPtr)
{
  PEXEIMAGE_LX_HEADER pLX = (PEXEIMAGE_LX_HEADER)pPtr; /* map pointer to LX struct */
  ULONG            ulCounter;                   /* loop counter for objects  */
  ULONG            ulCounter2;                  /* loop counter for objects  */
  PEXEIMAGE_LX_OBJECT pObj;                     /* pointer to object table data */
  PEXEIMAGE_LX_PAGE   pPage;                  /* pointer to page data structure */

  printf ("\n\n  Linear Executable Object Table");
  if (pLX->e_objcnt == 0)
  {
    printf (" not present.");
    return;                                    /* abort function immediately */
  }
  else
    printf("\n    Nr.   Virtual   Base      Attribute  Page Map  Map       Reserved"
           "\n          Size      Address   Flags      Index     Size");

  pObj = (PEXEIMAGE_LX_OBJECT) ( (PSZ) pLX +
                             pLX->e_objtab);
  pPage = (PEXEIMAGE_LX_PAGE) ( (PSZ) pLX +
                            pLX->e_objmap);

  for (ulCounter=0;
       ulCounter < pLX->e_objcnt;
       ulCounter++)
  {
    printf ("\n    %3u.  %08xh %08xh %08xh  %08xh %08xh %08xh"
            "\n          %8u                                 %8u"
            "\n          ",
            ulCounter + 1,
            pObj->o32_size,
            pObj->o32_base,
            pObj->o32_flags,
            pObj->o32_pagemap,
            pObj->o32_mapsize,
            pObj->o32_reserved,
            pObj->o32_size,
            pObj->o32_mapsize);


    if (pObj->o32_flags & OBJREAD) printf ("READ ");
    if (pObj->o32_flags & OBJWRITE) printf ("WRITE ");
    if (pObj->o32_flags & OBJEXEC) printf ("EXECUTABLE ");
    if (pObj->o32_flags & OBJRSRC) printf ("RESOURCE ");
    if (pObj->o32_flags & OBJDISCARD) printf ("DISCARDABLE ");
    if (pObj->o32_flags & OBJSHARED) printf ("SHARED ");
    if (pObj->o32_flags & OBJPRELOAD) printf ("PRELOAD ");
    if (pObj->o32_flags & OBJINVALID) printf ("INVALID ");
    if (pObj->o32_flags & OBJALIAS16) printf ("16:16ALIAS ");
    if (pObj->o32_flags & OBJBIGDEF) printf ("BIG/DEF ");
    if (pObj->o32_flags & OBJIOPL) printf ("IOPL ");
    if (pObj->o32_flags & OBJCONFORM) printf ("CONFORMING ");

    if (pObj->o32_flags & 0xFFFF0800)
      printf ("Unknown (%08x)",
              pObj->o32_flags & 0xFFFF0800);

    printf ("\n         Type: ");
    switch (pObj->o32_flags & OBJTYPEMASK)
    {
      case OBJNONPERM:  printf ("Nonpermanent"); break;
      case OBJPERM:     printf ("Permanent and swappable"); break;
      case OBJRESIDENT: printf ("Permanent and resident"); break;
      case OBJCONTIG:   printf ("Resident and contiguous"); break;
      case OBJDYNAMIC:  printf ("Permanent and long lockable"); break;
      case LNKNONPERM:  printf ("Nonpermanent (Link386)"); break;
      default: printf ("Unknown (%08xh)",
                       pObj->o32_flags & OBJTYPEMASK);
                       break;
    }


    printf ("\n      Nr. Offset      Size          Flags");

    for (ulCounter2 = 0;
         ulCounter2 < pObj->o32_mapsize;
         ulCounter2++)
    {
      printf ("\n      %3u. %08xh  %04xh, %5d  %04xh  ",
              ulCounter2,
              (pPage->o32_pagedataoffset << pLX->e_pageshift)
                + pLX->e_datapage,
              pPage->o32_pagesize,
              pPage->o32_pagesize,
              pPage->o32_pageflags);

      switch (pPage->o32_pageflags)
      {
        case VALID:     printf ("Valid Physical Page in .EXE"); break;
        case ITERDATA:  printf ("Iterated Data Page"); break;
        case INVALID:   printf ("Invalid Page"); break;
        case ZEROED:    printf ("Zero Filled Page"); break;
        case RANGE:     printf ("Range of pages"); break;
        case ITERDATA2: printf ("Iterated Data Page Type 2"); break;
        default:        printf ("Unknown %04xh",
                          pPage->o32_pageflags);
                        break;
      }

      pPage++;
    }

    pObj++;
  }
}


/*****************************************************************************
 * Name      : void ExeAnalyseHeaderLX
 * Funktion  : Dump LX Header Information
 * Parameter : PSZ pPointer
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

void ExeAnalyseHeaderLX (PSZ pPtr)
{
  PEXEIMAGE_LX_HEADER pLX = (PEXEIMAGE_LX_HEADER)pPtr; /* map pointer to LX struct */

  printf ("\n\n컴[LX Header Information (OS/2 2.x and later, Linear eXecutable)]컴컴컴컴컴�"
          "\n  Magic                            :       \"%c%c\" -       %04xh",
          pLX->e_magic & 0xFF,
          pLX->e_magic >> 8,
          pLX->e_magic);

  printf ("\n  The byte ordering for the .EXE   :        %02xh ",
          pLX->e_border);
  if (pLX->e_border == E32LEBO) printf ("- Little Endian Byte Order");
  else
    if (pLX->e_border == E32BEBO) printf ("- Big Endian Byte Order");

  printf ("\n  The word ordering for the .EXE   :        %02xh ",
          pLX->e_worder);
  if (pLX->e_worder == E32LEWO) printf ("- Little Endian Word Order");
  else
    if (pLX->e_worder == E32BEWO) printf ("- Big Endian Word Order");

  printf ("\n  The EXE format level             : %10u -   %08xh",
          pLX->e_level,
          pLX->e_level);

  printf ("\n  The CPU type                     : ");
  switch (pLX->e_cpu)
  {
    case E32CPU286: printf ("Intel 80286 or upwardly compatible"); break;
    case E32CPU386: printf ("Intel 80386 or upwardly compatible"); break;
    case E32CPU486: printf ("Intel 80486 or upwardly compatible"); break;
    default:        printf ("(unknown 0x%04x)",pLX->e_cpu); break;
  }

  printf ("\n  The Operating System type (?)    : ");
  switch (pLX->e_os)
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
              pLX->e_os);
      break;
  }


  printf ("\n  Module version                   : %4u.%02u.%02u",
          pLX->e_ver >> 16,
          pLX->e_ver >> 24,
          pLX->e_ver & 0xFF);


  /***************************************************************************
   * Module Flags                                                            *
   ***************************************************************************/

  printf ("\n  Module flags                     : %10u -   %08xh",
          pLX->e_mflags,
          pLX->e_mflags);

  printf ("\n    Module Type is ");
  switch (pLX->e_mflags & E32MODMASK)
  {
    case E32PROTDLL:   printf("Protected memory library module"); break;
    case E32MODEXE:    printf(".EXE module"); break;
    case E32MODDLL:    printf(".DLL module"); break;
    case E32MODPROTDLL:printf("Protected memory library module"); break;
    case E32MODPDEV:   printf("Physical device driver"); break;
    case E32MODVDEV:   printf("Virtual device driver"); break;
    default:           printf("(unknown type %08xh)",
                              pLX->e_mflags & E32MODMASK); break;
  }

#define E32LXMFLAG(a,b) if(pLX->e_mflags & a) printf("\n    "b);

  /* from OS/2 Books Documentation, Module Table Entry Flags, Devcon 2/1 */
  E32LXMFLAG(0x80000000, "0x80000000 - Debugger Symbols are loaded.");
  E32LXMFLAG(E32LIBTERM, "0x40000000 - per-process library termination");
  E32LXMFLAG(0x20000000, "0x20000000 - newly added module.");
  E32LXMFLAG(0x10000000, "0x10000000 - Module has shared memory protected.");
  E32LXMFLAG(0x08000000, "0x08000000 - porthole module.");
  E32LXMFLAG(0x04000000, "0x04000000 - make code pages swap on load (pre-swap).");
  E32LXMFLAG(0x02000000, "0x02000000 - File Media requires memory below 16M.");
  E32LXMFLAG(0x01000000, "0x01000000 - File Media requires contiguous memory.");
  E32LXMFLAG(0x00800000, "0x00800000 - Module supports long filenames.");
  E32LXMFLAG(0x00400000, "0x00400000 - Filesystem Helper MTE.");
  E32LXMFLAG(0x00200000, "0x00200000 - Filesystem Driver MTE.");
  E32LXMFLAG(0x00100000, "0x00100000 - Allocate specific address.");
/*  E32LXMFLAG(E32NOTSMPSAFE, "0x00080000 - marked as SMP unsafe");*/
  E32LXMFLAG(0x00080000, "0x00080000 - marked as SMP unsafe");
  E32LXMFLAG(E32NOTP,    "0x00008000 - Library Module - not a process");
  E32LXMFLAG(0x00004000, "0x00004000 - device driver module");
  E32LXMFLAG(E32NOLOAD,  "0x00002000 - Module is not loadable (errors in image)");
  E32LXMFLAG(0x00001000, "0x00001000 - File Media permits discarding");
  E32LXMFLAG(0x00000800, "0x00000800 - DOSCALLS -> the OS/2 Kernel module");
  E32LXMFLAG(0x00000080, "0x00000080 - Global Class");
  E32LXMFLAG(0x00000040, "0x00000040 - Program Class");
  E32LXMFLAG(E32NOEXTFIX,"0x00000020 - no external fixups in .EXE");
  E32LXMFLAG(E32NOINTFIX,"0x00000010 - no internal fixups in .EXE");
  E32LXMFLAG(0x00000008, "0x00000008 - Global Init has been setup");
  E32LXMFLAG(E32LIBINIT, "0x00000004 - per-process library initialization");
  E32LXMFLAG(0x00000002, "0x00000002 - Auto DS is not shared");
  E32LXMFLAG(0x00000001, "0x00000001 - Auto DS is shared");


  switch (pLX->e_mflags & E32APPMASK)
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
      printf ("\n    0x00000000 - non-interactive (Device Driver)");
      break;

    default:
      printf ("\n    0x%08x - (unknown)",
             pLX->e_mflags & E32APPMASK);
      break;
  }



  printf ("\n  Module pages                     : %10u -   %08xh",
          pLX->e_mpages,
          pLX->e_mpages);

  printf ("\n  Object # for instruction pointer : %10u -   %08xh",
          pLX->e_startobj,
          pLX->e_startobj);

  printf ("\n  Extended instruction pointer(EIP): %10u -   %08xh",
          pLX->e_eip,
          pLX->e_eip);

  printf ("\n  Object # for stack pointer       : %10u -   %08xh",
          pLX->e_startobj,
          pLX->e_startobj);

  printf ("\n  Extended stack pointer      (ESP): %10u -   %08xh",
          pLX->e_esp,
          pLX->e_esp);

  printf ("\n  Page size                        : %10u -   %08xh",
          pLX->e_pagesize,
          pLX->e_pagesize);

  printf ("\n  Page alignment shift in .EXE     : %10u -   %08xh",
          pLX->e_pageshift,
          pLX->e_pageshift);

  printf ("\n  Fixup section size               : %10u -   %08xh",
          pLX->e_fixupsize,
          pLX->e_fixupsize);

  printf ("\n  Fixup section checksum           : %10u -   %08xh",
          pLX->e_fixupsum,
          pLX->e_fixupsum);

  printf ("\n  Loader section size              : %10u -   %08xh",
          pLX->e_ldrsize,
          pLX->e_ldrsize);

  printf ("\n  Loader section checksum          : %10u -   %08xh",
          pLX->e_ldrsum,
          pLX->e_ldrsum);

  printf ("\n  Object page map offset           : %10u -   %08xh",
          pLX->e_objmap,
          pLX->e_objmap);

  printf ("\n  Object iterated data map offset  : %10u -   %08xh",
          pLX->e_itermap,
          pLX->e_itermap);


  printf ("\n");

  printf ("\n  Object Table     : Off %10u,%08xh  Size    %10u,%08xh (%u)",
          pLX->e_objtab,
          pLX->e_objtab,
          pLX->e_objcnt * sizeof (EXEIMAGE_LX_OBJECT),
          pLX->e_objcnt * sizeof (EXEIMAGE_LX_OBJECT),
          pLX->e_objcnt);

  printf ("\n  Resource Table   : Off %10u,%08xh  Size    %10u,%08xh (%u)",
          pLX->e_rsrctab,
          pLX->e_rsrctab,
          pLX->e_rsrccnt * sizeof (EXEIMAGE_LX_RESOURCE),
          pLX->e_rsrccnt * sizeof (EXEIMAGE_LX_RESOURCE),
          pLX->e_rsrccnt);

  printf ("\n  Import module    : Off %10u,%08xh  Entries %10u,%08xh",
          pLX->e_impmod,
          pLX->e_impmod,
          pLX->e_impmodcnt,
          pLX->e_impmodcnt);

  printf ("\n  Module Directive : Off %10u,%08xh  Entries %10u,%08xh",
          pLX->e_dirtab,
          pLX->e_dirtab,
          pLX->e_dircnt,
          pLX->e_dircnt);

  printf ("\n  Nonresident Names: Off %10u,%08xh  Size    %10u,%08xh",
          pLX->e_nrestab,
          pLX->e_nrestab,
          pLX->e_cbnrestab,
          pLX->e_cbnrestab);

  printf ("\n  Debug Information: Off %10u,%08xh  Size    %10u,%08xh",
          pLX->e_debuginfo,
          pLX->e_debuginfo,
          pLX->e_debuglen,
          pLX->e_debuglen);

  printf ("\n");

  printf ("\n  Resident names table offset      : %10u -   %08xh",
          pLX->e_restab,
          pLX->e_restab);

  printf ("\n  Entry table offset               : %10u -   %08xh",
          pLX->e_enttab,
          pLX->e_enttab);

  printf ("\n  Fixup page table offset          : %10u -   %08xh",
          pLX->e_fpagetab,
          pLX->e_fpagetab);

  printf ("\n  Fixup Record table offset        : %10u -   %08xh",
          pLX->e_frectab,
          pLX->e_frectab);

  printf ("\n  Import procedure name table off. : %10u -   %08xh",
          pLX->e_impproc,
          pLX->e_impproc);

  printf ("\n  Per-Page checksum table offset   : %10u -   %08xh",
          pLX->e_pagesum,
          pLX->e_pagesum);

  printf ("\n  Enumerated data pages table off. : %10u -   %08xh",
          pLX->e_datapage,
          pLX->e_datapage);

  printf ("\n  Number of preload pages          : %10u -   %08xh",
          pLX->e_preload,
          pLX->e_preload);

  printf ("\n  Non-resident name table checksum : %10u -   %08xh",
          pLX->e_nressum,
          pLX->e_nressum);

  printf ("\n  Object # for automatic data      : %10u -   %08xh",
          pLX->e_autodata,
          pLX->e_autodata);

  printf ("\n  Instance pages in preload section: %10u -   %08xh",
          pLX->e_instpreload,
          pLX->e_instpreload);

  printf ("\n  Instance pages in demand load sec: %10u -   %08xh",
          pLX->e_instdemand,
          pLX->e_instdemand);

  printf ("\n  Size of heap (for 16-bit apps)   : %10u -   %08xh",
          pLX->e_heapsize,
          pLX->e_heapsize);

  if (!Options.fsNoObjects)     ExeAnalyseLXObjects    ( (PSZ) pLX);
  if (!Options.fsNoResources)   ExeAnalyseLXResource   ( (PSZ) pLX);
  if (!Options.fsNoResident)    ExeAnalyseLXResident   ( (PSZ) pLX);
  if (!Options.fsNoNonResident) ExeAnalyseLXNonResident( (PSZ) pLX);
  if (!Options.fsNoImport)      ExeAnalyseLXImport     ( (PSZ) pLX);
  if (!Options.fsNoImportProc)  ExeAnalyseLXImportProc ( (PSZ) pLX);


  Globals.pExePointer = NULL;                    /* signal end of processing */
}
