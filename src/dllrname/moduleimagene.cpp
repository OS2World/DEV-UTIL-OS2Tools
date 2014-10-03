/* $Id: moduleimagene.cpp,v 1.1 2002/01/10 16:24:49 phaller Exp $
 *
 * DLL Rename Utility
 *
 * Copyright (C) 2002 InnoTek Systemberatung GmbH
 *
 */


/****************************************************************************
 * Includes
 ****************************************************************************/

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <string.h>
#include <stdio.h>

#include "moduleimagene.h"


typedef unsigned char  UINT8;
typedef UINT8* PUINT8;
typedef unsigned short UINT16;
typedef unsigned long  UINT32;


/*****************************************************************************
 * "NE" header - OS/2 1.x / Windows 3.x                                      *
 *****************************************************************************/

typedef struct                                             /* New .EXE header */
{
  UINT16      e_magic;                              /* Magic number NE_MAGIC */
  UINT8       e_ver;                                       /* Version number */
  UINT8       e_rev;                                      /* Revision number */
  UINT16      e_enttab;                             /* Offset of Entry Table */
  UINT16      e_cbenttab;                  /* Number of bytes in Entry Table */
  UINT32      e_crc;                               /* Checksum of whole file */
  UINT16      e_flags;                                          /* Flag word */
  UINT16      e_autodata;                   /* Automatic data segment number */
  UINT16      e_heap;                             /* Initial heap allocation */
  UINT16      e_stack;                           /* Initial stack allocation */
  UINT32      e_csip;                               /* Initial CS:IP setting */
  UINT32      e_sssp;                               /* Initial SS:SP setting */
  UINT16      e_cseg;                              /* Count of file segments */
  UINT16      e_cmod;                   /* Entries in Module Reference Table */
  UINT16      e_cbnrestab;                /* Size of non-resident name table */
  UINT16      e_segtab;                           /* Offset of Segment Table */
  UINT16      e_rsrctab;                         /* Offset of Resource Table */
  UINT16      e_restab;                     /* Offset of resident name table */
  UINT16      e_modtab;                  /* Offset of Module Reference Table */
  UINT16      e_imptab;                    /* Offset of Imported Names Table */
  UINT32      e_nrestab;               /* Offset of Non-resident Names Table */
  UINT16      e_cmovent;                         /* Count of movable entries */
  UINT16      e_align;                      /* Segment alignment shift count */
  UINT16      e_cres;                           /* Count of resource entries */
  UINT8       e_exetyp;                           /* Target operating system */
  UINT8       e_flagsothers;                             /* Other .EXE flags */
  UINT16      e_pretthunks;               /* offset to return thunks         */
  UINT16      e_psegrefbytes;             /* offset to segment ref. bytes    */
  UINT16      e_swaparea;                 /* Minimum code swap area size     */
  UINT16      e_expver;                   /* Expected Windows version number */
} EXEIMAGE_NE_HEADER, *PEXEIMAGE_NE_HEADER;



/*
 *  Target operating systems
 */

#define NE_UNKNOWN      0x0             /* Unknown (any "new-format" OS) */
#define NE_OS2          0x1             /* OS/2 (default)  */
#define NE_WINDOWS      0x2             /* Windows */
#define NE_DOS4         0x3             /* DOS 4.x */
#define NE_DEV386       0x4             /* Windows 386 */


/*
 *  Format of NE_FLAGS(x):
 *
 *  p                                   Not-a-process
 *   x                                  Unused
 *    e                                 Errors in image
 *     x                                Unused
 *      b                               Bound Family/API
 *       ttt                            Application type
 *          f                           Floating-point instructions
 *           3                          386 instructions
 *            2                         286 instructions
 *             0                        8086 instructions
 *              P                       Protected mode only
 *               p                      Per-process library initialization
 *                i                     Instance data
 *                 s                    Solo data
 */
#define NENOTP          0x8000          /* Not a process */
#define NEIERR          0x2000          /* Errors in image */
#define NEBOUND         0x0800          /* Bound Family/API */
#define NEAPPTYP        0x0700          /* Application type mask */
#define NENOTWINCOMPAT  0x0100          /* Not compatible with P.M. Windowing */
#define NEWINCOMPAT     0x0200          /* Compatible with P.M. Windowing */
#define NEWINAPI        0x0300          /* Uses P.M. Windowing API */
#define NEFLTP          0x0080          /* Floating-point instructions */
#define NEI386          0x0040          /* 386 instructions */
#define NEI286          0x0020          /* 286 instructions */
#define NEI086          0x0010          /* 8086 instructions */
#define NEPROT          0x0008          /* Runs in protected mode only */
#define NEPPLI          0x0004          /* Per-Process Library Initialization */
#define NEINST          0x0002          /* Instance data */
#define NESOLO          0x0001          /* Solo data */

/*
 *  Format of NE_FLAGSOTHERS(x):
 *
 *      7 6 5 4 3 2 1 0  - bit no
 *      |         | | |
 *      |         | | +---------------- Support for long file names
 *      |         | +------------------ Windows 2.x app runs in prot mode
 *      |         +-------------------- Windows 2.x app gets prop. font
 *      +------------------------------ WLO appl on OS/2 (markwlo.exe)
 *
 */

#define NELONGNAMES     0x01
#define NEWINISPROT     0x02
#define NEWINGETPROPFON 0x04
#define NEWLOAPPL       0x80

/* 0x08 is Windows 2.x Gangload Area */

typedef struct new_seg                       /* New .EXE segment table entry */
{
    UINT16      ns_sector;      /* File sector of start of segment */
    UINT16      ns_cbseg;       /* Number of bytes in file */
    UINT16      ns_flags;       /* Attribute flags */
    UINT16      ns_minalloc;    /* Minimum allocation in bytes */
} EXEIMAGE_NE_SEGMENT, *PEXEIMAGE_NE_SEGMENT;

/*
 *  Format of NS_FLAGS(x)
 *
 *  Flag word has the following format:
 *
 *      15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0  - bit no
 *          |  |  |  |  | | | | | | | | | | |
 *          |  |  |  |  | | | | | | | | +-+-+--- Segment type DATA/CODE
 *          |  |  |  |  | | | | | | | +--------- Iterated segment
 *          |  |  |  |  | | | | | | +----------- Movable segment
 *          |  |  |  |  | | | | | +------------- Segment can be shared
 *          |  |  |  |  | | | | +--------------- Preload segment
 *          |  |  |  |  | | | +----------------- Execute/read-only for code/data segment
 *          |  |  |  |  | | +------------------- Segment has relocations
 *          |  |  |  |  | +--------------------- Code conforming/Data is expand down
 *          |  |  |  +--+----------------------- I/O privilege level
 *          |  |  +----------------------------- Discardable segment
 *          |  +-------------------------------- 32-bit code segment
 *          +----------------------------------- Huge segment/GDT allocation requested
 *
 */

#define NSTYPE          0x0007          /* Segment type mask */

#define NSCODE          0x0000          /* Code segment */
#define NSDATA          0x0001          /* Data segment */
#define NSITER          0x0008          /* Iterated segment flag */
#define NSMOVE          0x0010          /* Movable segment flag */
#define NSSHARED        0x0020          /* Shared segment flag */
#define NSPRELOAD       0x0040          /* Preload segment flag */
#define NSEXRD          0x0080          /* Execute-only (code segment), or
                                        *  read-only (data segment)
                                        */
#define NSRELOC         0x0100          /* Segment has relocations */
#define NSCONFORM       0x0200          /* Conforming segment */
#define NSEXPDOWN       0x0200          /* Data segment is expand down */
#define NSDPL           0x0C00          /* I/O privilege level (286 DPL bits) */
#define SHIFTDPL        10              /* Left shift count for SEGDPL field */
#define NSDISCARD       0x1000          /* Segment is discardable */
#define NS32BIT         0x2000          /* 32-bit code segment */
#define NSHUGE          0x4000          /* Huge memory segment, length of
                                         * segment and minimum allocation
                                         * size are in segment sector units
                                         */
#define NSGDT           0x8000          /* GDT allocation requested */

#define NSPURE          NSSHARED        /* For compatibility */

#define NSALIGN 9       /* Segment data aligned on 512 byte boundaries */

#define NSLOADED    0x0004      /* ns_sector field contains memory addr */


typedef struct new_segdata                                   /* Segment data */
{
  union
  {
    struct
    {
      UINT16 ns_niter;                               /* number of iterations */
      UINT16 ns_nbytes;                                   /* number of bytes */
      char   ns_iterdata;                             /* iterated data bytes */
    } ns_iter;
    
    struct
    {
      char   ns_data;                                          /* data bytes */
    } ns_noniter;
  } ns_union;
} EXEIMAGE_NE_SEGDATA, *PEXEIMAGE_NE_SEGDATA;

typedef union  new_rlcinfo                                /* Relocation info */
{
  UINT16 nr_nreloc;                     /* number of relocation items that */
  
  struct
  {
    UINT8 nr_nreloc;
    UINT8 nr_seg;
  } nSeg;
}                                                                  /* follow */
EXEIMAGE_NE_RLCINFO, *PEXEIMAGE_NE_RLCINFO;

#pragma pack(1)


typedef struct new_rlc                                    /* Relocation item */
{
  UINT8  nr_stype;                                            /* Source type */
  
  union
  {
    struct
    {
      UINT8  nr_segno;                              /* Target segment number */
      UINT8  nr_res;                                             /* Reserved */
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

#pragma pack()


/*
 *  Format of NR_STYPE(x) and R32_STYPE(x):
 *
 *       7 6 5 4 3 2 1 0  - bit no
 *               | | | |
 *               +-+-+-+--- source type
 *
 */

#define NRSTYP          0x0f            /* Source type mask */
#define NRSBYT          0x00            /* lo byte (8-bits)*/
#define NRSSEG          0x02            /* 16-bit segment (16-bits) */
#define NRSPTR          0x03            /* 16:16 pointer (32-bits) */
#define NRSOFF          0x05            /* 16-bit offset (16-bits) */
#define NRPTR48         0x06            /* 16:32 pointer (48-bits) */
#define NROFF32         0x07            /* 32-bit offset (32-bits) */
#define NRSOFF32        0x08            /* 32-bit self-relative offset (32-bits) */


/*
 *  Format of NR_FLAGS(x) and R32_FLAGS(x):
 *
 *       7 6 5 4 3 2 1 0  - bit no
 *                 | | |
 *                 | +-+--- Reference type
 *                 +------- Additive fixup
 */

#define NRADD           0x04            /* Additive fixup */
#define NRRTYP          0x03            /* Reference type mask */
#define NRRINT          0x00            /* Internal reference */
#define NRRORD          0x01            /* Import by ordinal */
#define NRRNAM          0x02            /* Import by name */
#define NRROSF          0x03            /* Operating system fixup */


/* Resource type or name string */
struct rsrc_string
    {
    char rs_len;            /* number of bytes in string */
    char rs_string[ 1 ];    /* text of string */
    };

/* Resource type information block */
typedef struct rsrc_typeinfo
{
  UINT16 rt_id;
  UINT16 rt_nres;
  long rt_proc;
} EXEIMAGE_NE_RESOURCE_TYPE, *PEXEIMAGE_NE_RESOURCE_TYPE;

/* Resource name information block */
typedef struct rsrc_nameinfo
{
  /* The following two fields must be shifted left by the value of  */
  /* the rs_align field to compute their actual value.  This allows */
  /* resources to be larger than 64k, but they do not need to be    */
  /* aligned on 512 byte boundaries, the way segments are           */
  UINT16 rn_offset;   /* file offset to resource data */
  UINT16 rn_length;   /* length of resource data */
  UINT16 rn_flags;    /* resource flags */
  UINT16 rn_id;       /* resource name id */
  UINT16 rn_handle;   /* If loaded, then global handle */
  UINT16 rn_usage;    /* Initially zero.  Number of times */
                              /* the handle for this resource has */
                              /* been given out */
} EXEIMAGE_NE_RESOURCE_NAME, *PEXEIMAGE_NE_RESOURCE_NAME;

#define RSORDID     0x8000      /* if high bit of ID set then integer id */
                                /* otherwise ID is offset of string from
                                   the beginning of the resource table */

                                /* Ideally these are the same as the */
                                /* corresponding segment flags */
#define RNMOVE      0x0010      /* Moveable resource */
#define RNPURE      0x0020      /* Pure (read-only) resource */
#define RNPRELOAD   0x0040      /* Preloaded resource */
#define RNDISCARD   0xF000      /* Discard priority level for resource */

/* Resource table */
typedef struct new_rsrc
{
  UINT16 rs_align;                    /* alignment shift count for resources */
  struct rsrc_typeinfo rs_typeinfo;
} EXEIMAGE_NE_RESOURCE, *PEXEIMAGE_NE_RESOURCE;



/***********************************************************************
 * Name      :
 * Purpose   :
 * Parameter : 
 * Variables :
 * Result    :
 * Remark    :
 *
 * Author    : Patrick Haller [2002-01-10]
 ***********************************************************************/

ModuleImageNE::ModuleImageNE(void* _pBuffer,
                             int _iBufferSize)
  : ModuleImage(_pBuffer, _iBufferSize)
{
}


ModuleImageNE::~ModuleImageNE()
{
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
  UINT16 nr_nameoffset;
} EXEIMAGE_NE_MODULE, *PEXEIMAGE_NE_MODULE;


int ModuleImageNE::changeImportModuleName(char* pszOldName,
                                          char* pszNewName)
{
  int iOldNameLength = strlen( pszOldName );
  int iNewNameLength = strlen( pszNewName );
  int iChanges = 0;
  
  // check if name lengths match
  if (iOldNameLength != iNewNameLength)
    // invalid names - no changes
    return 0;
  
  PEXEIMAGE_NE_HEADER pNE = (PEXEIMAGE_NE_HEADER)getModuleHeaderPointer();
  ULONG            ulCounter;                   /* loop counter for segments */
  PEXEIMAGE_NE_MODULE pMod;                     /* pointer to module table data */
  PUINT8           pucNameLength;                      /* length of the name */

  // check if New Executable Module Table is present
  if (pNE->e_cmod == 0)
    // no table - no changes
    return 0;
  
  
  pMod = (PEXEIMAGE_NE_MODULE) ( (PSZ) pNE + pNE->e_modtab);

  for (ulCounter=0;
       ulCounter < pNE->e_cmod;
       ulCounter++)
  {
    pucNameLength = (PUINT8) ( (PSZ)pMod +
                                    pNE->e_cmod * sizeof(EXEIMAGE_NE_MODULE) +
                                    pMod[ulCounter].nr_nameoffset );
    
    
    // check if the current name matches pszOldName
    if (*pucNameLength == iOldNameLength)
    {
      PSZ pszImport = (PSZ)pucNameLength + sizeof(UINT8);
      
      if ( strnicmp(pszOldName,
                    pszImport,
                    iOldNameLength) == 0)
      {
        // OK, match
        // replace the names
        memcpy(pszImport,
               pszNewName,
               iNewNameLength);
        
        iChanges++;
      }
    }
  }
  
  
  // Done
  return iChanges;
}


int ModuleImageNE::displayImportModuleNames(void)
{
  char szBuffer[260];
  PEXEIMAGE_NE_HEADER pNE = (PEXEIMAGE_NE_HEADER)getModuleHeaderPointer();
  ULONG            ulCounter;                   /* loop counter for segments */
  PEXEIMAGE_NE_MODULE pMod;                     /* pointer to module table data */
  PUINT8           pucNameLength;                      /* length of the name */
  
  // check if New Executable Module Table is present
  if (pNE->e_cmod == 0)
  {
    printf("  no new executable module table present\n");
    return 0;
  }
  
  // OK, proceed ...
  pMod = (PEXEIMAGE_NE_MODULE) ( (PSZ) pNE + pNE->e_modtab);

  for (ulCounter=0;
       ulCounter < pNE->e_cmod;
       ulCounter++)
  {
    pucNameLength = (PUINT8) ( (PSZ)pMod +
                                    pNE->e_cmod * sizeof(EXEIMAGE_NE_MODULE) +
                                    pMod[ulCounter].nr_nameoffset );

    PSZ pszImport = (PSZ)pucNameLength + sizeof(UINT8);

    strncpy(szBuffer,
            pszImport,
            *pucNameLength);
    szBuffer[ *pucNameLength ] = 0;

    printf("  %d. %s\n",
           ulCounter,
           szBuffer);
  }
  
  // Done
  return ulCounter;
}
