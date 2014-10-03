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
 * Name      : void ExeAnalyseNENonResident
 * Funktion  : Dump NE NonResident Names Information
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
} EXEIMAGE_NE_NONRESIDENT, *PEXEIMAGE_NE_NONRESIDENT;



void   HexDump                 (ULONG ulLength,
                                PVOID pSource);


void ExeAnalyseNENonResident (PSZ pPtr)
{
  PEXEIMAGE_NE_HEADER   pNE = (PEXEIMAGE_NE_HEADER)pPtr;  /* map pointer NE struct */
  ULONG              ulCounter;                 /* loop counter for segments */
  PEXEIMAGE_NE_NONRESIDENT pNRes;               /* pointer to module table data */
  char               szBuffer[256];               /* buffer for module names */
  PUINT8             pucNameLength;                    /* length of the name */
  ULONG              ulTotalBytes = 0;            /* counter for total bytes */
  ULONG              ulBytes = 0;                    /* current byte counter */

  printf ("\n\n  New Executable Non-Resident Names Table");
  if (pNE->e_cbnrestab == 0)
  {
    printf (" not present.");
    return;                                    /* abort function immediately */
  }
  else
    printf("\n    Nr. Len. Name");


  /* @@@PH !!! WARNING !!!
     for some strange reason, the e_nrestab offset is not relative to
     the "NE" header but absolute to the fileimage !!! This means the
     size of the "MZ" header has to be subtracted
  */


  pNRes = (PEXEIMAGE_NE_NONRESIDENT) ( (PSZ) Globals.pExeBuffer +
                                    pNE->e_nrestab);
  ulTotalBytes = pNE->e_cbnrestab;                    /* total size of table */

  for (ulCounter=0;
       ulBytes < ulTotalBytes;
       ulCounter++)
  {
    pucNameLength = (PUINT8)pNRes;

    if (*pucNameLength == 0)                     /* skip this value, unknown */
    {
      /* @@@PH seems not to be correct
        pNRes = (PEXEIMAGE_NE_NONRESIDENT) ( (PSZ)pNRes +
                                         sizeof(UINT8) );
        ulBytes++;
      */
      return;                                              /* abort function */
    }
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

      pNRes = (PEXEIMAGE_NE_NONRESIDENT) ( (PSZ)pNRes +
                                       *pucNameLength +
                                       sizeof(UINT8) );
      ulBytes += sizeof(UINT8) + *pucNameLength;
    }
  }
}


/*****************************************************************************
 * Name      : void ExeAnalyseNEImport
 * Funktion  : Dump NE Imported Names Information
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
} EXEIMAGE_NE_IMPORT, *PEXEIMAGE_NE_IMPORT;


void ExeAnalyseNEImport (PSZ pPtr)
{
  PEXEIMAGE_NE_HEADER pNE = (PEXEIMAGE_NE_HEADER)pPtr;  /* map pointer NE struct */
  ULONG            ulCounter;                 /* loop counter for segments */
  PEXEIMAGE_NE_IMPORT pImp;                   /* pointer to module table data */
  char             szBuffer[256];               /* buffer for module names */
  PUINT8           pucNameLength;                    /* length of the name */
  ULONG            ulTotalBytes = 0;            /* counter for total bytes */
  ULONG            ulBytes = 0;                    /* current byte counter */

  printf ("\n\n  New Executable Imported Names Table");
  if (pNE->e_imptab == pNE->e_enttab)
  {
    printf (" not present.");
    return;                                    /* abort function immediately */
  }
  else
    printf("\n    Nr.   Length Index Name");

  pImp = (PEXEIMAGE_NE_IMPORT) ( (PSZ) pNE +
                              pNE->e_imptab
                             + sizeof(UINT8) ); /* 1st byte to skip. Unknown */
  ulTotalBytes = pNE->e_enttab - pNE->e_imptab;       /* total size of table */
  ulBytes++;                        /* we skipped the 1st byte of this table */

  for (ulCounter=0;
       ulBytes < ulTotalBytes;
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

    pImp = (PEXEIMAGE_NE_IMPORT) ( (PSZ)pImp + sizeof(UINT8) + *pucNameLength);
    ulBytes += sizeof(UINT8) + *pucNameLength;
  }
}


/*****************************************************************************
 * Name      : void ExeAnalyseNEResident
 * Funktion  : Dump NE Resident Names Information
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
} EXEIMAGE_NE_RESIDENT, *PEXEIMAGE_NE_RESIDENT;


void ExeAnalyseNEResident (PSZ pPtr)
{
  PEXEIMAGE_NE_HEADER   pNE = (PEXEIMAGE_NE_HEADER)pPtr;  /* map pointer NE struct */
  ULONG              ulCounter;                 /* loop counter for segments */
  PEXEIMAGE_NE_RESIDENT pRes;                   /* pointer to module table data */
  char               szBuffer[256];               /* buffer for module names */
  PUINT8             pucNameLength;                    /* length of the name */
  PUINT16            pusIndex;                    /* pointer to index number */
  ULONG              ulTotalBytes = 0;            /* counter for total bytes */
  ULONG              ulBytes = 0;                    /* current byte counter */

  printf ("\n\n  New Executable Resident Names Table");
  if (pNE->e_restab == pNE->e_modtab)
  {
    printf (" not present.");
    return;                                    /* abort function immediately */
  }
  else
    printf("\n    Nr.   Length Index Name");

  pRes = (PEXEIMAGE_NE_RESIDENT) ( (PSZ) pNE +
                                pNE->e_restab);
  ulTotalBytes = pNE->e_modtab - pNE->e_restab;       /* total size of table */

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

    pRes = (PEXEIMAGE_NE_RESIDENT) ( (PSZ)pusIndex + sizeof(UINT16) );
    ulBytes += sizeof(UINT8) + sizeof(UINT16) + *pucNameLength;
  }
}


/*****************************************************************************
 * Name      : void ExeAnalyseNEResources
 * Funktion  : Dump NE Resource Information
 * Parameter : PSZ pPointer
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

void ExeAnalyseNEResources (PSZ pPtr)
{
  PEXEIMAGE_NE_HEADER   pNE = (PEXEIMAGE_NE_HEADER)pPtr;  /* map pointer NE struct */
  ULONG              ulCounter;                 /* loop counter for segments */
  PEXEIMAGE_NE_RESOURCE pRes;                   /* pointer to module table data */
  char               szBuffer[256];               /* buffer for module names */

  PEXEIMAGE_NE_RESOURCE_TYPE pType;
  PEXEIMAGE_NE_RESOURCE_NAME pName;
  ULONG                   ulTotalBytes;
  ULONG                   ulCounter2;
  PSZ                     pszGroup;
  PUINT8                  pNameLength;
  PSZ                     pszName;


#define PRINTINFO(var,mask,text) \
  if (var & mask) \
    printf (text);



  printf ("\n\n  New Executable Resource Table");
  if (pNE->e_rsrctab == pNE->e_restab)
  {
    printf (" not present.");
    return;                                    /* abort function immediately */
  }

  pRes = (PEXEIMAGE_NE_RESOURCE) ( (PSZ) pNE +
                                pNE->e_rsrctab);
  printf ("\n    Alignment %u = %u bytes.",
          pRes->rs_align,
          1 << pRes->rs_align);

                              /* get pointer to the resource type information*/
  pType = &(pRes->rs_typeinfo);

  ulTotalBytes = pNE->e_restab - pNE->e_rsrctab;

  for (ulCounter=2;                                           /* is rs_align */
       ulCounter < ulTotalBytes;
      )
  {
    if (pType->rt_id == 0)                              /* end of the list ? */
      break;

    if (pType->rt_id & RSORDID)
      switch (pType->rt_id & 0x7FFF)
      {
        case 0x0001: pszGroup="CURSOR"; break;
        case 0x0002: pszGroup="BITMAP"; break;
        case 0x0003: pszGroup="ICON"; break;
        case 0x0004: pszGroup="MENU"; break;
        case 0x0005: pszGroup="DIALOG"; break;
        case 0x0006: pszGroup="STRINGTABLE"; break;
        case 0x0007: pszGroup="FONTDIRECTORY"; break;
        case 0x0008: pszGroup="FONT"; break;
        case 0x0009: pszGroup="ACCELTABLE"; break;
        case 0x000a: pszGroup="RCDATA"; break;
        case 0x000c: pszGroup="Group CURSOR"; break;
        case 0x000e: pszGroup="Group ICON"; break;
        case 0x000f: pszGroup="NAMETABLE"; break;
        case 0x0010: pszGroup="VS_VERSION_INFO"; break;
        default: sprintf(szBuffer,
                         "(unknown %04xh)",
                         pType->rt_id & 0x7FFF);
                 pszGroup=szBuffer;
                 break;
      }
    else /* ID is offset to stringtable */
    {
      pNameLength= (PUINT8)pRes + (ULONG)(pType->rt_id & 0x7FFF);
      pszName = (PSZ)pNameLength + 1;

      strncpy (szBuffer,
               pszName,
               *pNameLength);
      pszGroup=szBuffer;
      szBuffer[*pNameLength]=0;               /* explicit string termination */
    }

    printf ("\n    %-17s Entries %04x   Reserved %08x  ",
            pszGroup,
            pType->rt_nres,
            pType->rt_proc);

    pName = (PEXEIMAGE_NE_RESOURCE_NAME) ( (PSZ)pType +
                                        sizeof(EXEIMAGE_NE_RESOURCE_TYPE) );

    for (ulCounter2 = 0;
         ulCounter2 < pType->rt_nres;
         ulCounter2++)
    {
      if (pName->rn_id & RSORDID)              /* what type of ID is there ? */
        sprintf (szBuffer,
                 "#%u",
                 pName->rn_id & 0x7FFF);
      else
      {
        pNameLength= (PUINT8)pRes + (ULONG)(pName->rn_id & 0x7FFF);
        pszName = (PSZ)pNameLength + 1;

        strncpy (szBuffer,
                 pszName,
                 *pNameLength);
        szBuffer[*pNameLength]=0;             /* explicit string termination */
      }
      printf ("\n      %-16s   Offset %04xh   Length %04xh,%5u",
              szBuffer,
              pName->rn_offset << pRes->rs_align,
              pName->rn_length << pRes->rs_align,
              pName->rn_length << pRes->rs_align);

      printf("\n          ");
      PRINTINFO(pName->rn_flags,RNMOVE,"MOVEABLE ");
      PRINTINFO(pName->rn_flags,RNPURE,"PURE/SHAREABLE ");
      PRINTINFO(pName->rn_flags,RNPRELOAD,"PRELOAD ");
      if (pName->rn_flags & 0x0F8F)
        printf ("(unknown %04xh) ",
                pName->rn_flags & 0x0F8F);

      printf("   Discard priority %u",
             (pName->rn_flags & RNDISCARD) >> 12);

      ulCounter += sizeof(EXEIMAGE_NE_RESOURCE_NAME);
      pName++;
    }

    pType = (PEXEIMAGE_NE_RESOURCE_TYPE) pName;
    ulCounter += sizeof(EXEIMAGE_NE_RESOURCE_TYPE);
  }

#undef PRINTINFO
}


/*****************************************************************************
 * Name      : void ExeAnalyseNEModules
 * Funktion  : Dump NE Module Information
 * Parameter : PSZ pPointer
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

typedef struct
{
  UINT16 nr_nameoffset;


} EXEIMAGE_NE_MODULE, *PEXEIMAGE_NE_MODULE;


void ExeAnalyseNEModules (PSZ pPtr)
{
  PEXEIMAGE_NE_HEADER pNE = (PEXEIMAGE_NE_HEADER)pPtr; /* map pointer to NE struct */
  ULONG            ulCounter;                   /* loop counter for segments */
  PEXEIMAGE_NE_MODULE pMod;                     /* pointer to module table data */
  char             szBuffer[256];                 /* buffer for module names */
  PUINT8           pucNameLength;                      /* length of the name */

  printf ("\n\n  New Executable Module Table");
  if (pNE->e_cmod == 0)
  {
    printf (" not present.");
    return;                                    /* abort function immediately */
  }
  else
    printf("\n    Nr.   Offset Length Name");

  pMod = (PEXEIMAGE_NE_MODULE) ( (PSZ) pNE +
                             pNE->e_modtab);

  for (ulCounter=0;
       ulCounter < pNE->e_cmod;
       ulCounter++)
  {
    printf ("\n    %3u.  %04x   ",
            ulCounter,
            pMod[ulCounter].nr_nameoffset);

    pucNameLength = (PUINT8) ( (PSZ)pMod +
                                    pNE->e_cmod * sizeof(EXEIMAGE_NE_MODULE) +
                                    pMod[ulCounter].nr_nameoffset );

    strncpy (szBuffer,
             (PSZ)pucNameLength + sizeof(UINT8),
             *pucNameLength);
    szBuffer[*pucNameLength] = 0;               /* active string termination */

    printf ("%3u    [%s]",
            *pucNameLength,
            szBuffer);
  }
}


/*****************************************************************************
 * Name      : void ExeAnalyseNEEntries
 * Funktion  : Dump NE Relocation Information
 * Parameter : PSZ pPointer
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

void ExeAnalyseNEEntries (PSZ pPtr)
{
  PEXEIMAGE_NE_HEADER pNE = (PEXEIMAGE_NE_HEADER)pPtr; /* map pointer to NE struct */
  ULONG            ulCounter;                   /* loop counter for segments */
  ULONG            ulRelCounter;       /* inner loop counter for relocations */
  ULONG            ulRelLength;          /* holds length of relocation entry */
  ULONG            ulRelTotalCounter = 0;   /* counts total number of relocs */
  ULONG            ulByteTotalCounter = 0;   /* counts total number of bytes */
  PEXEIMAGE_NE_NEWRLC pRel;                      /* pointer to relocation entry */
  PEXEIMAGE_NE_RLCINFO pInfo;


  printf ("\n\n  New Executable Entry Table");
  if (pNE->e_cbenttab == 0)
  {
    printf (" not present.");
    return;                                    /* abort function immediately */
  }
  else
    printf("\n    Nr.   Type Flags Offset");

  pInfo = (PEXEIMAGE_NE_RLCINFO) ( (PSZ) pNE +
                                pNE->e_enttab );

  for (ulCounter = 0;
       ulByteTotalCounter < pNE->e_cbenttab;
       ulCounter ++)
  {
    if (pInfo->nSeg.nr_nreloc == 0)                      /* end of the table */
      return;                                                  /* then abort */

    printf ("\n    Segment %02xh has %4u relocations",
            pInfo->nSeg.nr_seg,
            pInfo->nSeg.nr_nreloc);
    pRel = (PEXEIMAGE_NE_NEWRLC) ( (PSZ)pInfo + sizeof(EXEIMAGE_NE_RLCINFO) );
    ulByteTotalCounter += sizeof(EXEIMAGE_NE_RLCINFO);

    for (ulRelCounter = 0;
         ulRelCounter < pInfo->nSeg.nr_nreloc;
         ulRelCounter++)
    {
      printf ("\n      %3u. %02xh ",
              ulRelTotalCounter,
              pRel->nr_stype);

      if (pRel->nr_stype & NRADD)                        /* additive fixup ? */
        printf ("additive ");
      else
        printf ("absolute ");


      switch (pRel->nr_stype & NRRTYP)       /* mask out the relocation type */
      {
        case NRRINT:
          printf ("internal reference,     segment %02xh (%02xh), entry %04xh",
                  pRel->nr_union.nr_intref.nr_segno,
                  pRel->nr_union.nr_intref.nr_res,
                  pRel->nr_union.nr_intref.nr_entry);
          ulRelLength = 5;                    /* unknown length, problematic ! */
          break;

        case NRRORD:
          printf ("import by ordinal,      module %04xh, ordinal %5u",
                  pRel->nr_union.nr_import.nr_mod,
                  pRel->nr_union.nr_import.nr_proc);
          ulRelLength = 5;                    /* unknown length, problematic ! */
          break;

        case NRRNAM:
          printf ("import by name,         module %04xh, name offset %04xh",
                  pRel->nr_union.nr_import.nr_mod,
                  pRel->nr_union.nr_import.nr_proc);
          ulRelLength = 5;                    /* unknown length, problematic ! */
          break;

        case NRROSF:
          printf ("operating system fixup, type %04xh",
                  pRel->nr_union.nr_osfix.nr_ostype);
          ulRelLength = 3;                    /* unknown length, problematic ! */
          break;

        default:
          printf ("(undefined %02xh)",
                  pRel->nr_stype & NRRTYP);
          ulRelLength = 1;                    /* unknown length, problematic ! */
          break;
      }


      pRel = (PEXEIMAGE_NE_NEWRLC) ( (PSZ) pRel +
                                 ulRelLength);
      ulByteTotalCounter += ulRelLength;
      ulRelTotalCounter++;
    }

    pInfo = (PEXEIMAGE_NE_RLCINFO)pRel;
  }
}



#if 0
          switch (pRel->nr_stype & NRSTYP)
      {
      case NRSBYT:
        printf ("lo byte (8-bits)                     ");
        printf ("%02xh",
                pRel->nr_union.nr_sbyt.nr_offset8);
        ulRelLength = 2;              /* relocation entry length is 2 bytes  */
        break;

      case NRSSEG:
        printf ("16-bit segment (16-bits)             ");
        printf ("%04xh",
                pRel->nr_union.nr_sseg.nr_segment);
        ulRelLength = 3;              /* relocation entry length is 3 bytes  */
        break;

      case NRSPTR:
        printf ("16:16 pointer (32-bits)              ");
        printf ("%04xh:%04xh",
                pRel->nr_union.nr_sptr.nr_segment,
                pRel->nr_union.nr_sptr.nr_offset);
        ulRelLength = 5;              /* relocation entry length is 5 bytes  */
        break;

      case NRSOFF:
        printf ("16-bit offset (16-bits)              ");
        printf ("%04xh",
                pRel->nr_union.nr_soff.nr_offset);
        ulRelLength = 3;              /* relocation entry length is 3 bytes  */
        break;

      case NRPTR48:
        printf ("16:32 pointer (48-bits)              ");
        printf ("%04xh%08xh",
                pRel->nr_union.nr_sptr48.nr_segment,
                pRel->nr_union.nr_sptr48.nr_offset);
        ulRelLength = 7;              /* relocation entry length is 7 bytes  */
        break;

      case NROFF32:
        printf ("32-bit offset (32-bits)              ");
        printf ("%08xh",
                pRel->nr_union.nr_soff32.nr_offset);
        ulRelLength = 5;              /* relocation entry length is 5 bytes  */
        break;

      case NRSOFF32:
        printf ("32-bit self-relative offset (32-bits)");
        printf ("%08xh",
                pRel->nr_union.nr_ssoff32.nr_offset);
        ulRelLength = 5;              /* relocation entry length is 5 bytes  */
        break;


      if (pRel->nr_stype & 0xf0)
        printf ("\n             Unknown Bits: %02xh",
                pRel->nr_stype & 0xf0);

      printf ("\n        Flags: ");
      switch (pRel->nr_flags)
      {
        case NRADD: printf ("additive fixup"); break;
        case NRRTYP: printf ("reference type mask"); break;
        case NRRINT: printf ("internal reference"); break;
        case NRRORD: printf ("import by ordinal"); break;
        case NRRNAM: printf ("import by name"); break;
        default: printf ("(unknown %02xh)",
                         pRel->nr_flags & 0xf8); break;
      }

  typedef struct new_rlc                                    /* Relocation item */
{
  char   nr_stype;                                            /* Source type */
  char   nr_flags;                                              /* Flag byte */
  UINT16 nr_soff;                                           /* Source offset */

  union
  {
    struct
    {
      char   nr_segno;                              /* Target segment number */
      char   nr_res;                                             /* Reserved */
      UINT16 nr_entry;                          /* Target Entry Table offset */
    } nr_intref;                                       /* Internal reference */

    struct
    {
      UINT16 nr_mod;                    /* Index into Module Reference Table */
      UINT16 nr_proc;                    /* Procedure ordinal or name offset */
    } nr_import;                                                   /* Import */

    struct
    {
      UINT16 nr_ostype;                                      /* OSFIXUP type */
      UINT16 nr_osres;                                           /* reserved */
    } nr_osfix;                                    /* Operating system fixup */
  } nr_union;                                                       /* Union */
} EXEIMAGE_NE_NEWRLC, *PEXEIMAGE_NE_NEWRLC;
#endif


/*****************************************************************************
 * Name      : void ExeAnalyseNESegments
 * Funktion  : Dump NE Header Information
 * Parameter : PSZ pPointer
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

void ExeAnalyseNESegments (PSZ pPtr)
{
  PEXEIMAGE_NE_HEADER pNE = (PEXEIMAGE_NE_HEADER)pPtr; /* map pointer to NE struct */
  ULONG            ulCounter;                   /* loop counter for segments */
  PEXEIMAGE_NE_SEGMENT pSeg;                        /* pointer to segment entry */

#define PRINTINFO(var,mask,text) \
  if (var & mask) \
    printf (text);


  printf ("\n\n  New Executable Segment Entries");
  if (pNE->e_cseg == 0)
  {
    printf (" not present.");
    return;                                    /* abort function immediately */
  }
  else
    printf("\n    Nr.   Sector Size  Flags Allocation");

  for (ulCounter = 0;
       ulCounter < pNE->e_cseg;
       ulCounter++)
  {
    pSeg = (PEXEIMAGE_NE_SEGMENT) ( (PSZ)pNE +
                                pNE->e_segtab +
                                sizeof(EXEIMAGE_NE_SEGMENT) * ulCounter
                               );

    printf ("\n\n    %4u. Sector %04xh    Length %04xh   Flags %04xh   "\
            "Alloc %04xh\n          ",
            ulCounter + 1,
            pSeg->ns_sector,
            pSeg->ns_cbseg,
            pSeg->ns_flags,
            pSeg->ns_minalloc);

    if (pSeg->ns_flags & NSDATA)
    {
      printf ("DATA ");
      PRINTINFO(pSeg->ns_flags,NSEXRD,"READONLY ")
      PRINTINFO(pSeg->ns_flags,NSCONFORM,"EXPANDDOWN ")
    }
    else
    {
      printf ("CODE ");
      PRINTINFO(pSeg->ns_flags,NSEXRD,"EXECUTEONLY ")
      PRINTINFO(pSeg->ns_flags,NSCONFORM,"CONFORMING ")
    }


    PRINTINFO(pSeg->ns_flags,NSITER,"ITERATED ")
    PRINTINFO(pSeg->ns_flags,NSMOVE,"MOVEABLE ")
    PRINTINFO(pSeg->ns_flags,NSSHARED,"SHARED ")
    PRINTINFO(pSeg->ns_flags,NSPRELOAD,"PRELOAD ")
    PRINTINFO(pSeg->ns_flags,NSRELOC,"RELOCATIONS ")
    PRINTINFO(pSeg->ns_flags,NSDISCARD,"DISCARD ")
    PRINTINFO(pSeg->ns_flags,NS32BIT,"32BITCODE ")
    PRINTINFO(pSeg->ns_flags,NSHUGE,"HUGE ")
    PRINTINFO(pSeg->ns_flags,NSGDT,"GDTALLOC ")


    printf ("\n          ");
    PRINTINFO(pSeg->ns_flags,0x0400,"0x0400 ")
    PRINTINFO(pSeg->ns_flags,0x0002,"0x0002 ")
    PRINTINFO(pSeg->ns_flags,NSLOADED,"Sector field contains memory address.")

    printf ("   286 IOPL / DPL bits: %02xh",
            (pSeg->ns_flags & NSDPL) >> SHIFTDPL);

  }

  #undef PRINTINFO
}


/*****************************************************************************
 * Name      : void ExeAnalyseHeaderNE
 * Funktion  : Dump NE Header Information
 * Parameter : PSZ pPointer
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

void ExeAnalyseHeaderNE (PSZ pPtr)
{
  PEXEIMAGE_NE_HEADER pNE = (PEXEIMAGE_NE_HEADER)pPtr; /* map pointer to NE struct */

  printf ("\n\n컴[NE Header Information (OS/2 1.x, Windows 3.x, New Executable)]컴컴컴컴컴�"
          "\n  Magic                            :       \"%c%c\" -       %04xh",
          pNE->e_magic & 0xFF,
          pNE->e_magic >> 8,
          pNE->e_magic);


  printf ("\n  Version, Revision                : %u.%u",
          pNE->e_ver,
          pNE->e_rev);

  printf ("\n  Checksum (header), CRC           : %10u -       %04xh",
          pNE->e_crc,
          pNE->e_crc);

  printf ("\n           (calculated, NOT YET)   : %10u -       %04xh",
          0,
          0);

  printf ("\n  Flags                            : %10u -       %04xh",
          pNE->e_flags,
          pNE->e_flags);

                                                         /* decode the flags */
  if (pNE->e_flags & NENOTP)
    printf ("\n        0x8000 - not a process");

/*  if (pNE->e_flags & NENOTSMPSAFE)*/
  if (pNE->e_flags & 0x4000)
    printf ("\n        0x4000 - not SMP safe");

  if (pNE->e_flags & NEIERR)
    printf ("\n        0x2000 - errors in image");
  else
    printf ("\n    not 0x2000 - no errors in image");

  if (pNE->e_flags & 0x1000)
    printf ("\n        0x1000 - (unused)");

  if (pNE->e_flags & NEBOUND)
    printf ("\n        0x0800 - Bound Family/API");
  else
    printf ("\n    not 0x0800 - no Bound Family/API");

  switch (pNE->e_flags & NEAPPTYP)
  {
    case NENOTWINCOMPAT:
      printf ("\n        0x0100 - not windowing compatible");
      break;

    case NEWINCOMPAT:
      printf ("\n        0x0200 - windowing compatible");
      break;

    case NEWINAPI:
      printf ("\n        0x0300 - uses windowing API (PM)");
      break;

    default:
      printf ("\n        0x%04x - (unknown)",
             pNE->e_flags & NEAPPTYP);
      break;
  }

  if (pNE->e_flags & NEFLTP)
    printf ("\n        0x0080 - floating point instructions");
  else
    printf ("\n    not 0x0080 - no floating point instructions");

  if (pNE->e_flags & NEI386)
    printf ("\n        0x0040 - 80386 instructions");
  else
    printf ("\n    not 0x0040 - no 80386 instructions");

  if (pNE->e_flags & NEI286)
    printf ("\n        0x0020 - 80286 instructions");
  else
    printf ("\n    not 0x0020 - no 80286 instructions");

  if (pNE->e_flags & NEI086)
    printf ("\n        0x0010 - 8086 instructions");
  else
    printf ("\n    not 0x0010 - no 8086 instructions");

  if (pNE->e_flags & NEPROT)
    printf ("\n        0x0008 - runs in protected mode only");
  else
    printf ("\n    not 0x0008 - runs in real mode and protected mode");

  if (pNE->e_flags & NEPPLI)
    printf ("\n        0x0004 - per-process library initialization");
  else
    printf ("\n    not 0x0004 - no per-process library initialization");

  if (pNE->e_flags & NEINST)
    printf ("\n        0x0002 - instance data");
  else
    printf ("\n    not 0x0002 - no instance data");

  if (pNE->e_flags & NESOLO)
    printf ("\n        0x0001 - solo data");
  else
    printf ("\n    not 0x0001 - no solo data");



  printf ("\n  Automatic data segment number    : %10u -       %04xh",
          pNE->e_autodata,
          pNE->e_autodata);

  printf ("\n  Initial heap allocation          : %10u -       %04xh",
          pNE->e_heap,
          pNE->e_heap);

  printf ("\n  Initial stack allocation         : %10u -       %04xh",
          pNE->e_stack,
          pNE->e_stack);

  printf ("\n  Initial            CS:IP value   :      CS:IP - %04xh:%04xh",
          pNE->e_csip >> 16,
          pNE->e_csip & 0xFFFF);

  printf ("\n  Initial (relative) SS:SP value   :      SS:SP - %04xh:%04xh",
          pNE->e_sssp >> 16,
          pNE->e_sssp & 0xFFFF);

  printf ("\n");


  printf ("\n  Segment Table            : Offset %5u, %04xh   Size %5u, %04xh  (%u)",
          pNE->e_segtab,
          pNE->e_segtab,
          pNE->e_cseg * sizeof (EXEIMAGE_NE_SEGMENT),
          pNE->e_cseg * sizeof (EXEIMAGE_NE_SEGMENT),
          pNE->e_cseg);

  printf ("\n  Resource Table           : Offset %5u, %04xh   Size %5u, %04xh",
          pNE->e_rsrctab,
          pNE->e_rsrctab,
          pNE->e_restab - pNE->e_rsrctab,
          pNE->e_restab - pNE->e_rsrctab);

  printf ("\n  Resident Names Table     : Offset %5u, %04xh   Size %5u, %04xh  (%u)",
          pNE->e_restab,
          pNE->e_restab,
          pNE->e_modtab - pNE->e_restab,
          pNE->e_modtab - pNE->e_restab,
          pNE->e_cres);

  printf ("\n  Module Reference Table   : Offset %5u, %04xh   Size %5u, %04xh  (%u)",
          pNE->e_modtab,
          pNE->e_modtab,
          pNE->e_cmod * 2, /* @ */
          pNE->e_cmod * 2,
          pNE->e_cmod);


  printf ("\n  Imported Names Table     : Offset %5u, %04xh   Size %5u, %04xh",
          pNE->e_imptab,
          pNE->e_imptab,
          pNE->e_enttab - pNE->e_imptab,

          pNE->e_enttab - pNE->e_imptab);

  printf ("\n  Entry Table              : Offset %5u, %04xh   Size %5u, %04xh",
          pNE->e_enttab,
          pNE->e_enttab,
          pNE->e_cbenttab,
          pNE->e_cbenttab);

  /* for some strange reason, the e_nrestab offset is not relative to
     the "NE" header but absolute to the fileimage !!! This means the
     size of the "MZ" header has to be subtracted */

  printf ("\n  Non-Resident Names Table : Offset %5u, %04xh   Size %5u, %04xh",
          pNE->e_nrestab,
          pNE->e_nrestab,
          pNE->e_cbnrestab,
          pNE->e_cbnrestab);

  printf ("\n");

  printf ("\n  Count of movable entries         : %10u -       %04xh",
          pNE->e_cmovent,
          pNE->e_cmovent);

  printf ("\n  Segment alignment shift count    : %10u -       %04xh",
          pNE->e_align,
          pNE->e_align);

  printf ("\n  Target Operating System          : ");

  switch (pNE->e_exetyp)
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
      printf ("(undefined operating system %02x)",
              pNE->e_exetyp);
      break;
  }

  if (pNE->e_expver != 0)      /* if there is a valid entry for the expected */
                                                 /* operating system version */
    printf (" (%2u.%02u)",
            pNE->e_expver >> 8,
            pNE->e_expver & 0xff);


  printf ("\n  Other Flags                      : %10u -         %02xh",
          pNE->e_flagsothers,
          pNE->e_flagsothers);

                                                         /* decode the flags */
  if (pNE->e_flagsothers & NELONGNAMES)
    printf ("\n    0x01 - support for long file names");

  if (pNE->e_flagsothers & NEWINISPROT)
    printf ("\n    0x02 - Windows 2.x app runs in prot mode");

  if (pNE->e_flagsothers & NEWINGETPROPFON)
    printf ("\n    0x04 - Windows 2.x app gets proportional font");

  if (pNE->e_flagsothers & 0x08)
    printf ("\n    0x08 - Windows 2.x / 3.x Gangload Area");

  if (pNE->e_flagsothers & 0x70)
    printf ("\n    0x70 - (unknown additional flags)");

  if (pNE->e_flagsothers & NEWLOAPPL)
    printf ("\n    0x80 - WLO application on OS/2 (MARKWLO.EXE)");



  printf ("\n  Offset to return thunks          : %10u -       %04xh",
          pNE->e_pretthunks,
          pNE->e_pretthunks);

  printf ("\n  Offset to segment ref. bytes     : %10u -       %04xh",
          pNE->e_psegrefbytes,
          pNE->e_psegrefbytes);

  printf ("\n  Minimum code swap area size      : %10u -       %04xh",
          pNE->e_swaparea,
          pNE->e_swaparea);

  if (!Options.fsNoSegments)    ExeAnalyseNESegments( (PSZ) pNE);
  if (!Options.fsNoEntries)     ExeAnalyseNEEntries( (PSZ) pNE);
  if (!Options.fsNoModules)     ExeAnalyseNEModules( (PSZ) pNE);
  if (!Options.fsNoResources)   ExeAnalyseNEResources( (PSZ) pNE);
  if (!Options.fsNoResident)    ExeAnalyseNEResident( (PSZ) pNE);
  if (!Options.fsNoImport)      ExeAnalyseNEImport( (PSZ) pNE);
  if (!Options.fsNoNonResident) ExeAnalyseNENonResident( (PSZ) pNE);

  Globals.pExePointer = NULL;                    /* signal end of processing */
}
