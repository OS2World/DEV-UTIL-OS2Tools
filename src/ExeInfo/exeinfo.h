/*****************************************************************************
 * Name      : EXEINFO.H
 * Funktion  : Definition of all necessary exe information headers
 * Parameter : 
 * Variablen :
 * Ergebnis  :
 * Bemerkung : Gathered together from Win NT Toolkit, IBM Toolkit
 *
 * Autor     : Patrick Haller [Dienstag, 13.06.1995 22.56.56]
 *****************************************************************************/

#ifndef MODULE_EXEINFO
#define MODULE_EXEINFO


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


#ifdef __cplusplus
extern "C" {
#endif
  
/*****************************************************************************
 * Types                                                                     *
 *****************************************************************************/

typedef unsigned char  UINT8;
typedef unsigned short UINT16;

#ifndef _WIN32
typedef unsigned int   UINT32;
#endif 

typedef signed   char  INT8;
typedef signed   short INT16;

#ifndef _WIN32 
typedef signed   int   INT32;
#endif

typedef unsigned char  *PUINT8;
typedef unsigned short *PUINT16;

#ifndef _WIN32
typedef unsigned int   *PUINT32;
#endif

typedef signed   char  *PINT8;
typedef signed   short *PINT16;

#ifndef _WIN32
typedef signed   int   *PINT32;
#endif
  
  

/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsFile;          /* indicates file was specified from command line */
  ARGFLAG fsNoMZ;                       /* skip MZ header, do NOT display it */
  ARGFLAG fsNoSegments;  /* these flags set if user wants to skip the tables */
  ARGFLAG fsNoEntries;
  ARGFLAG fsNoModules;
  ARGFLAG fsNoResources;
  ARGFLAG fsNoResident;
  ARGFLAG fsNoImport;
  ARGFLAG fsNoImportProc;
  ARGFLAG fsNoNonResident;
  ARGFLAG fsNoRelocations;
  ARGFLAG fsNoObjects;
  
  PSZ  pszFile;                          /* name of the file to be processed */
} OPTIONS, *POPTIONS;
extern OPTIONS Options;


typedef struct
{
  PVOID  pExeBuffer;                     /* pointer to the executable buffer */
  ULONG  ulExeSize;                         /* size of the executable buffer */
  PUINT8 pExePointer;               /* pointer to the current analysis point */
} GLOBALS, *PGLOBALS;
extern GLOBALS Globals;


  
/*****************************************************************************
 * Header IDs                                                                *
 *****************************************************************************/

#define EXEIMAGE_DOS_SIGNATURE                 0x5A4D      // MZ
#define EXEIMAGE_OS2_SIGNATURE                 0x454E      // NE
#define EXEIMAGE_OS2_SIGNATURE_LE              0x454C      // LE
#define EXEIMAGE_OS2_SIGNATURE_LX              0x584C      // LX
#define EXEIMAGE_VXD_SIGNATURE                 0x454C      // LE
#define EXEIMAGE_NT_SIGNATURE                  0x4550      // PE
#define EXEIMAGE_NT_SIGNATURE_00               0x00004550  // PE00

#define EXERT_POINTER      1   /* mouse pointer shape */
#define EXERT_BITMAP       2   /* bitmap */
#define EXERT_MENU         3   /* menu template */
#define EXERT_DIALOG       4   /* dialog template */
#define EXERT_STRING       5   /* string tables */
#define EXERT_FONTDIR      6   /* font directory */
#define EXERT_FONT         7   /* font */
#define EXERT_ACCELTABLE   8   /* accelerator tables */
#define EXERT_RCDATA       9   /* binary data */
#define EXERT_MESSAGE      10  /* error msg     tables */
#define EXERT_DLGINCLUDE   11  /* dialog include file name */
#define EXERT_VKEYTBL      12  /* key to vkey tables */
#define EXERT_KEYTBL       13  /* key to UGL tables */
#define EXERT_CHARTBL      14  /* glyph to character tables */
#define EXERT_DISPLAYINFO  15  /* screen display information */

#define EXERT_FKASHORT     16  /* function key area short form */
#define EXERT_FKALONG      17  /* function key area long form */

#define EXERT_HELPTABLE    18  /* Help table for Cary Help manager */
#define EXERT_HELPSUBTABLE 19  /* Help subtable for Cary Help manager */

#define EXERT_FDDIR        20  /* DBCS uniq/font driver directory */
#define EXERT_FD           21  /* DBCS uniq/font driver */

#define EXERT_MAX          22  /* 1st unused Resource Type */


#define EXERF_ORDINALID    0x80000000L  /* ordinal id flag in resource table */


/*****************************************************************************
 * "MZ" header - DOS / Stub                                                  *
 *****************************************************************************/

typedef struct                                    /* DOS 1, 2, 3 .EXE header */
{
    UINT16      e_magic;                                     /* Magic number */
    UINT16      e_cblp;                        /* Bytes on last page of file */
    UINT16      e_cp;                                       /* Pages in file */
    UINT16      e_crlc;                                       /* Relocations */
    UINT16      e_cparhdr;                   /* Size of header in paragraphs */
    UINT16      e_minalloc;               /* Minimum extra paragraphs needed */
    UINT16      e_maxalloc;               /* Maximum extra paragraphs needed */
    UINT16      e_ss;                         /* Initial (relative) SS value */
    UINT16      e_sp;                                    /* Initial SP value */
    UINT16      e_csum;                                          /* Checksum */
    UINT16      e_ip;                                    /* Initial IP value */
    UINT16      e_cs;                         /* Initial (relative) CS value */
    UINT16      e_lfarlc;                /* File address of relocation table */
    UINT16      e_ovno;                                    /* Overlay number */
    /*      the following fields may not be present.
     *              ereloff = 28            not present
     *                      = 30            exe.ever present and valid
     *                      = 32            exe.ever field contains garbage
     *              ereloff > 32            exe.ever present and valid
     *                                              = 0 if "don't know"
     */
    UINT16      e_ver;                      /* version # of producing linker */
    UINT16      e_res0;                                            /* unused */
    /*      the following fields may not be present - if the exe.ereloff
     *      value encompasses the fields then they are present and valid.
     */
    UINT16      e_bb;                                       /* behavior bits */
    UINT16      e_res1;                                    /* Reserved words */
    UINT16      e_oemid;                   /* OEM identifier (for e_oeminfo) */
    UINT16      e_oeminfo;              /* OEM information; e_oemid specific */
    UINT16      e_res2[10];                                /* Reserved words */
    long        e_lfanew;                  /* File address of new exe header */
} EXEIMAGE_MZ_HEADER, *PEXEIMAGE_MZ_HEADER;


typedef struct
{
  UINT16 mr_offset;  
  UINT16 mr_segment;
} EXEIMAGE_MZ_RELOCATION, *PEXEIMAGE_MZ_RELOCATION;

  
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


  
/*****************************************************************************
 * "LX" header - OS/2 2.x, 3.x, 4.0 Intel                                    *
 *****************************************************************************/

#define BITPERWORD      16
#define BITPERBYTE      8
#define OBJPAGELEN      4096
#define E32MAGIC1       'L'             /* New magic number  "LX" */
#define E32MAGIC2       'X'             /* New magic number  "LX" */
#define E32MAGIC        0x584c          /* New magic number  "LX" */
#define E32RESBYTES1    0               /* First bytes reserved */
#define E32RESBYTES2    0               /* Second bytes reserved */
#define E32RESBYTES3    24              /* Third bytes reserved */
#define E32LEBO         0x00            /* Little Endian Byte Order */
#define E32BEBO         0x01            /* Big Endian Byte Order */
#define E32LEWO         0x00            /* Little Endian Word Order */
#define E32BEWO         0x01            /* Big Endian Word Order */
#define E32LEVEL        0L              /* 32-bit EXE format level */
#define E32CPU286       0x001           /* Intel 80286 or upwardly compatibile */
#define E32CPU386       0x002           /* Intel 80386 or upwardly compatibile */
#define E32CPU486       0x003           /* Intel 80486 or upwardly compatibile */



typedef struct                          /* New 32-bit .EXE header */
{
    UINT16      e_magic;      /* Magic number E32_MAGIC */
    UINT8       e_border;     /* The byte ordering for the .EXE */
    UINT8       e_worder;     /* The word ordering for the .EXE */
    UINT32      e_level;      /* The EXE format level for now = 0 */
    UINT16      e_cpu;        /* The CPU type */
    UINT16      e_os;         /* The OS type */
    UINT32      e_ver;        /* Module version */
    UINT32      e_mflags;     /* Module flags */
    UINT32      e_mpages;     /* Module # pages */
    UINT32      e_startobj;   /* Object # for instruction pointer */
    UINT32      e_eip;        /* Extended instruction pointer */
    UINT32      e_stackobj;   /* Object # for stack pointer */
    UINT32      e_esp;        /* Extended stack pointer */
    UINT32      e_pagesize;   /* .EXE page size */
    UINT32      e_pageshift;  /* Page alignment shift in .EXE */
    UINT32      e_fixupsize;  /* Fixup section size */
    UINT32      e_fixupsum;   /* Fixup section checksum */
    UINT32      e_ldrsize;    /* Loader section size */
    UINT32      e_ldrsum;     /* Loader section checksum */
    UINT32      e_objtab;     /* Object table offset */
    UINT32      e_objcnt;     /* Number of objects in module */
    UINT32      e_objmap;     /* Object page map offset */
    UINT32      e_itermap;    /* Object iterated data map offset */
    UINT32      e_rsrctab;    /* Offset of Resource Table */
    UINT32      e_rsrccnt;    /* Number of resource entries */
    UINT32      e_restab;     /* Offset of resident name table */
    UINT32      e_enttab;     /* Offset of Entry Table */
    UINT32      e_dirtab;     /* Offset of Module Directive Table */
    UINT32      e_dircnt;     /* Number of module directives */
    UINT32      e_fpagetab;   /* Offset of Fixup Page Table */
    UINT32      e_frectab;    /* Offset of Fixup Record Table */
    UINT32      e_impmod;     /* Offset of Import Module Name Table */
    UINT32      e_impmodcnt;  /* Number of entries in Import Module Name Table */
    UINT32      e_impproc;    /* Offset of Import Procedure Name Table */
    UINT32      e_pagesum;    /* Offset of Per-Page Checksum Table */
    UINT32      e_datapage;   /* Offset of Enumerated Data Pages */
    UINT32      e_preload;    /* Number of preload pages */
    UINT32      e_nrestab;    /* Offset of Non-resident Names Table */
    UINT32      e_cbnrestab;  /* Size of Non-resident Name Table */
    UINT32      e_nressum;    /* Non-resident Name Table Checksum */
    UINT32      e_autodata;   /* Object # for automatic data object */
    UINT32      e_debuginfo;  /* Offset of the debugging information */
    UINT32      e_debuglen;   /* The length of the debugging info. in bytes */
    UINT32      e_instpreload;/* Number of instance pages in preload section of .EXE file */
    UINT32      e_instdemand; /* Number of instance pages in demand load section of .EXE file */
    UINT32      e_heapsize;   /* Size of heap - for 16-bit apps */
    UINT8       e_res3[E32RESBYTES3];   /* Pad structure to 196 bytes */
} EXEIMAGE_LX_HEADER, *PEXEIMAGE_LX_HEADER;



/*
 *  Format of E32_MFLAGS(x):
 *
 *  Low word has the following format:
 *
 *  15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0  - bit no
 *   |     |          | |     | |   |
 *   |     |          | |     | |   +------- Per-Process Library Initialization
 *   |     |          | |     | +----------- No Internal Fixups for Module in .EXE
 *   |     |          | |     +------------- No External Fixups for Module in .EXE
 *   |     |          | +------------------- Incompatible with PM Windowing
 *   |     |          +--------------------- Compatible with PM Windowing
 *   |     |                                 Uses PM Windowing API
 *   |     +-------------------------------- Module not Loadable
 *   +-------------------------------------- Library Module
 */


#define E32NOTP          0x8000L        /* Library Module - used as NENOTP */
#define E32NOLOAD        0x2000L        /* Module not Loadable */
#define E32PMAPI         0x0300L        /* Uses PM Windowing API */
#define E32PMW           0x0200L        /* Compatible with PM Windowing */
#define E32NOPMW         0x0100L        /* Incompatible with PM Windowing */
#define E32NOEXTFIX      0x0020L        /* NO External Fixups in .EXE */
#define E32NOINTFIX      0x0010L        /* NO Internal Fixups in .EXE */
#define E32LIBINIT       0x0004L        /* Per-Process Library Initialization */

#define E32LIBTERM       0x40000000L    /* Per-Process Library Termination */
#define E32APPMASK       0x0700L        /* Application Type Mask */


/*
 *  Format of E32_MFLAGS(x):
 *
 *  High word has the following format:
 *
 *  15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0  - bit no
 *                                    | |
 *                                    | +--- Protected memory library module
 *                                    +----- Device driver
 */

#define E32PROTDLL       0x10000L       /* Protected memory library module */
#define E32DEVICE        0x20000L       /* Device driver                   */
#define E32MODEXE        0x00000L       /* .EXE module                     */
#define E32MODDLL        0x08000L       /* .DLL module                     */
#define E32MODPROTDLL    0x18000L       /* Protected memory library module */
#define E32MODPDEV       0x20000L       /* Physical device driver          */
#define E32MODVDEV       0x28000L       /* Virtual device driver           */
#define E32MODMASK       0x38000L       /* Module type mask                */

/*
 *  RELOCATION DEFINITIONS - RUN-TIME FIXUPS
 */


#pragma pack(1)                         /* This data must be packed */


typedef union _offset
{
    UINT16 offset16;
    UINT32  offset32;
}
    offset;                             /* 16-bit or 32-bit offset */


/***ET+ r32_rlc - Relocation item */

struct r32_rlc                          /* Relocation item */
{
    UINT8       nr_stype;       /* Source type - field shared with new_rlc */
    UINT8       nr_flags;       /* Flag byte - field shared with new_rlc */
    short               r32_soff;       /* Source offset */
    UINT16      r32_objmod;     /* Target object number or Module ordinal */

    union targetid
    {
        offset             intref;      /* Internal fixup */

        union extfixup
        {
            offset         proc;        /* Procedure name offset */
            UINT32  ord;         /* Procedure odrinal */
        }
                           extref;      /* External fixup */

        struct addfixup
        {
            UINT16 entry;       /* Entry ordinal */
            offset         addval;      /* Value added to the address */
        }
                           addfix;      /* Additive fixup */
    }
                        r32_target;     /* Target data */
    UINT16      r32_srccount;   /* Number of chained fixup records */
    UINT16      r32_chain;      /* Chain head */
};


#pragma pack()                          /* Stop packing */


/*
 *  In 32-bit .EXE file run-time relocations are written as varying size
 *  records, so we need many size definitions.
 */

#define RINTSIZE16      8
#define RINTSIZE32      10
#define RORDSIZE        8
#define RNAMSIZE16      8
#define RNAMSIZE32      10
#define RADDSIZE16      10
#define RADDSIZE32      12



/*
 *  Access macros defined in NEWEXE.H !!!
 */
#define NR_STYPE(x)      (x).nr_stype
#define NR_FLAGS(x)      (x).nr_flags

#define R32_SOFF(x)      (x).r32_soff
#define R32_OBJNO(x)     (x).r32_objmod
#define R32_MODORD(x)    (x).r32_objmod
#define R32_OFFSET16(x)  (x).r32_target.intref.offset16
#define R32_OFFSET32(x)  (x).r32_target.intref.offset32
#define R32_PROCOFF16(x) (x).r32_target.extref.proc.offset16
#define R32_PROCOFF32(x) (x).r32_target.extref.proc.offset32
#define R32_PROCORD(x)   (x).r32_target.extref.ord
#define R32_ENTRY(x)     (x).r32_target.addfix.entry
#define R32_ADDVAL16(x)  (x).r32_target.addfix.addval.offset16
#define R32_ADDVAL32(x)  (x).r32_target.addfix.addval.offset32
#define R32_SRCCNT(x)    (x).r32_srccount
#define R32_CHAIN(x)     (x).r32_chain



/*
 *  Format of NR_STYPE(x)
 *
 *       7 6 5 4 3 2 1 0  - bit no
 *           | | | | | |
 *           | | +-+-+-+--- Source type
 *           | +----------- Fixup to 16:16 alias
 *           +------------- List of source offset follows fixup record
 */


            /* DEFINED in newexe.h !!! */

#define NRSTYP          0x0f            /* Source type mask */
#define NRSBYT          0x00            /* lo byte (8-bits)*/
#define NRSSEG          0x02            /* 16-bit segment (16-bits) */
#define NRSPTR          0x03            /* 16:16 pointer (32-bits) */
#define NRSOFF          0x05            /* 16-bit offset (16-bits) */
#define NRPTR48         0x06            /* 16:32 pointer (48-bits) */
#define NROFF32         0x07            /* 32-bit offset (32-bits) */
#define NRSOFF32        0x08            /* 32-bit self-relative offset (32-bits) */


#define NRSRCMASK       0x0f            /* Source type mask */
#define NRALIAS         0x10            /* Fixup to alias */
#define NRCHAIN         0x20            /* List of source offset follows */
                                        /* fixup record, source offset field */
                                        /* in fixup record contains number */
                                        /* of elements in list */

/*
 *  Format of NR_FLAGS(x) and R32_FLAGS(x):
 *
 *       7 6 5 4 3 2 1 0  - bit no
 *       | | | |   | | |
 *       | | | |   | +-+--- Reference type
 *       | | | |   +------- Additive fixup
 *       | | | +----------- 32-bit Target Offset Flag (1 - 32-bit; 0 - 16-bit)
 *       | | +------------- 32-bit Additive Flag (1 - 32-bit; 0 - 16-bit)
 *       | +--------------- 16-bit Object/Module ordinal (1 - 16-bit; 0 - 8-bit)
 *       +----------------- 8-bit import ordinal (1 - 8-bit;
 *                                                0 - NR32BITOFF toggles
 *                                                    between 16 and 32 bit
 *                                                    ordinal)
 */


            /* DEFINED in newexe.h !!! */

#define NRRTYP          0x03            /* Reference type mask */
#define NRRINT          0x00            /* Internal reference */
#define NRRORD          0x01            /* Import by ordinal */
#define NRRNAM          0x02            /* Import by name */
#define NRADD           0x04            /* Additive fixup */

#define NRRENT          0x03            /* Internal entry table fixup */

#define NR32BITOFF      0x10            /* 32-bit Target Offset */
#define NR32BITADD      0x20            /* 32-bit Additive fixup */
#define NR16OBJMOD      0x40            /* 16-bit Object/Module ordinal */
#define NR8BITORD       0x80            /* 8-bit import ordinal */
/*end*/

/*
 *  Data structures for storing run-time fixups in linker virtual memory.

 *
 *  Each object has a list of Object Page Directories which specify
 *  fixups for given page. Each page has its own hash table which is
 *  used to detect fixups to the same target.
 */

#define PAGEPERDIR      62
#define LG2DIR          7


typedef struct _OBJPAGEDIR
{
    UINT32   next;               /* Virtual pointer to next dir on list */
    UINT16  ht[PAGEPERDIR];     /* Pointers to individual hash tables */
}
    OBJPAGEDIR;



/*
 *  OBJECT TABLE
 */

/***ET+ o32_obj Object Table Entry */

typedef struct o32_obj                       /* Flat .EXE object table entry */
{
    UINT32       o32_size;       /* Object virtual size */
    UINT32       o32_base;       /* Object base virtual address */
    UINT32       o32_flags;      /* Attribute flags */
    UINT32       o32_pagemap;    /* Object page map index */
    UINT32       o32_mapsize;    /* Number of entries in object page map */
    UINT32       o32_reserved;   /* Reserved */
} EXEIMAGE_LX_OBJECT, *PEXEIMAGE_LX_OBJECT;


/*
 *  Format of O32_FLAGS(x)
 *
 *  High word of dword flag field is not used for now.
 *  Low word has the following format:
 *
 *  15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0  - bit no
 *   |  |  |  |     | | | | | | | | | | |
 *   |  |  |  |     | | | | | | | | | | +--- Readable Object
 *   |  |  |  |     | | | | | | | | | +----- Writeable Object
 *   |  |  |  |     | | | | | | | | +------- Executable Object
 *   |  |  |  |     | | | | | | | +--------- Resource Object
 *   |  |  |  |     | | | | | | +----------- Object is Discardable
 *   |  |  |  |     | | | | | +------------- Object is Shared
 *   |  |  |  |     | | | | +--------------- Object has preload pages
 *   |  |  |  |     | | | +----------------- Object has invalid pages
 *   |  |  |  |     | | +------------------- Object is permanent and swappable
 *   |  |  |  |     | +--------------------- Object is permanent and resident
 *   |  |  |  |     +----------------------- Object is permanent and long lockable
 *   |  |  |  +----------------------------- 16:16 alias required (80x86 specific)
 *   |  |  +-------------------------------- Big/Default bit setting (80x86 specific)
 *   |  +----------------------------------- Object is conforming for code (80x86 specific)
 *   +-------------------------------------- Object I/O privilege level (80x86 specific)
 *
 */

#define OBJREAD         0x0001L             /* Readable Object   */
#define OBJWRITE        0x0002L             /* Writeable Object  */
#define OBJRSRC         0x0008L             /* Resource Object   */
#define OBJINVALID      0x0080L             /* Object has invalid pages  */
#define LNKNONPERM      0x0600L             /* Object is nonpermanent - should be */
#define OBJNONPERM      0x0000L             /* zero in the .EXE but LINK386 uses 6 */
#define OBJPERM         0x0100L             /* Object is permanent and swappable */
#define OBJRESIDENT     0x0200L             /* Object is permanent and resident */
#define OBJCONTIG       0x0300L             /* Object is resident and contiguous */
#define OBJDYNAMIC      0x0400L             /* Object is permanent and long locable */
#define OBJTYPEMASK     0x0700L             /* Object type mask */
#define OBJALIAS16      0x1000L             /* 16:16 alias required (80x86 specific)           */
#define OBJBIGDEF       0x2000L             /* Big/Default bit setting (80x86 specific)        */
#define OBJIOPL         0x8000L             /* Object I/O privilege level (80x86 specific)     */
/*
 *  Name these flags differently for EXEHDR.EXE - avoid conflicts with 286 version
 */
#define OBJDISCARD       0x0010L            /* Object is Discardable */
#define OBJSHARED        0x0020L            /* Object is Shared */
#define OBJPRELOAD       0x0040L            /* Object has preload pages  */
#define OBJEXEC          0x0004L            /* Executable Object */
#define OBJCONFORM       0x4000L            /* Object is conforming for code (80x86 specific)  */
/*
 *  Life will be easier, if we keep the same names for the following flags:
 */
#define NS32DISCARD       0x0010L             /* Object is Discardable */
#define NS32MOVE          NS32DISCARD         /* Moveable object is for sure Discardable */
#define NS32SHARED        0x0020L             /* Object is Shared */
#define NS32PRELOAD       0x0040L             /* Object has preload pages  */
#define NS32EXRD          0x0004L             /* Executable Object */
#define NS32CONFORM       0x4000L             /* Object is conforming for code (80x86 specific)  */
/*end*/

/***ET+ o32_map - Object Page Map entry */

typedef struct o32_map                            /* Object Page Table entry */
{
    UINT32   o32_pagedataoffset;     /* file offset of page */
    UINT16  o32_pagesize;           /* # bytes of page data */
    UINT16  o32_pageflags;          /* Per-Page attributes */
} EXEIMAGE_LX_PAGE, *PEXEIMAGE_LX_PAGE;


#define GETPAGEIDX(x)    ((x).o32_pagedataoffset)

#define PUTPAGEIDX(x,i)  ((x).o32_pagedataoffset = ((UINT32)(i)))

#define PUTPAGESIZ(x,i)  ((x).o32_pagesize = ((unsigned int)(i)))

#define GETPAGESIZ(x)    ((x).o32_pagesize)

#define PAGEFLAGS(x)    (x).o32_pageflags


#define VALID           0x0000                /* Valid Physical Page in .EXE */
#define ITERDATA        0x0001                /* Iterated Data Page */
#define INVALID         0x0002                /* Invalid Page */
#define ZEROED          0x0003                /* Zero Filled Page */
#define RANGE           0x0004                /* Range of pages */
#define ITERDATA2       0x0005                /* Iterated Data Page Type II */
/*end*/

/*
 *  RESOURCE TABLE
 */

/***ET+ rsrc32 - Resource Table Entry */

typedef struct rsrc32                                /* Resource Table Entry */
{
    UINT16 type;                                            /* Resource type */
    UINT16 name;                                            /* Resource name */
    UINT32 cb;                                              /* Resource size */
    UINT16 obj;                                             /* Object number */
    UINT32 offset;                                   /* Offset within object */
} EXEIMAGE_LX_RESOURCE, *PEXEIMAGE_LX_RESOURCE;


#pragma pack(1)                         /* This data must be packed */

 /*
  * Iteration Record format for 'EXEPACK'ed pages. (DCR1346)
  */
struct LX_Iter
{
    UINT16 LX_nIter;            /* number of iterations */
    UINT16 LX_nBytes;           /* number of bytes */
    UINT8  LX_Iterdata;         /* iterated data byte(s) */
};


/*
 *  ENTRY TABLE DEFINITIONS
 */

/***ET+ b32_bundle - Entry Table */

struct b32_bundle
{
    UINT8       b32_cnt;        /* Number of entries in this bundle */
    UINT8       b32_type;       /* Bundle type */
    UINT16      b32_obj;        /* Object number */
};                                      /* Follows entry types */

struct e32_entry
{
    UINT8       e32_flags;      /* Entry point flags */
    union entrykind
    {
        offset          e32_offset;     /* 16-bit/32-bit offset entry */
        struct callgate
        {
            UINT16 offset;      /* Offset in segment */
            UINT16 callgate;    /* Callgate selector */
        }
                        e32_callgate;   /* 286 (16-bit) call gate */
        struct fwd
        {
            UINT16  modord;     /* Module ordinal number */
            UINT32   value;      /* Proc name offset or ordinal */
        }
                        e32_fwd;        /* Forwarder */
    }
                        e32_variant;    /* Entry variant */
};

#pragma pack()                          /* Stop packing */


#define B32_CNT(x)      (x).b32_cnt
#define B32_TYPE(x)     (x).b32_type
#define B32_OBJ(x)      (x).b32_obj

#define E32_EFLAGS(x)   (x).e32_flags
#define E32_OFFSET16(x) (x).e32_variant.e32_offset.offset16
#define E32_OFFSET32(x) (x).e32_variant.e32_offset.offset32
#define E32_GATEOFF(x)  (x).e32_variant.e32_callgate.offset
#define E32_GATE(x)     (x).e32_variant.e32_callgate.callgate
#define E32_MODORD(x)   (x).e32_variant.e32_fwd.modord
#define E32_VALUE(x)    (x).e32_variant.e32_fwd.value

#define FIXENT16        3
#define FIXENT32        5
#define GATEENT16       5
#define FWDENT          7

/*
 *  BUNDLE TYPES
 */

#define EMPTY        0x00               /* Empty bundle */
#define ENTRY16      0x01               /* 16-bit offset entry point */
#define GATE16       0x02               /* 286 call gate (16-bit IOPL) */
#define ENTRY32      0x03               /* 32-bit offset entry point */
#define ENTRYFWD     0x04               /* Forwarder entry point */
#define TYPEINFO     0x80               /* Typing information present flag */


/*
 *  Format for E32_EFLAGS(x)
 *
 *       7 6 5 4 3 2 1 0  - bit no
 *       | | | | | | | |
 *       | | | | | | | +--- exported entry
 *       | | | | | | +----- uses shared data
 *       +-+-+-+-+-+------- parameter word count
 */

#define E32EXPORT       0x01            /* Exported entry */
#define E32SHARED       0x02            /* Uses shared data */
#define E32PARAMS       0xf8            /* Parameter word count mask */

/*
 *  Flags for forwarders only:
 */

#define FWD_ORDINAL     0x01            /* Imported by ordinal */
/*end*/




/*++ BUILD Version: 0093     Increment this if a change has global effects

Copyright (c) 1990-1996  Microsoft Corporation

Module Name:

    winnt.h

Abstract:

    This module defines the 32-Bit Windows types and constants that are
    defined by NT, but exposed through the Win32 API.

Revision History:
--*/

#define APPLICATION_ERROR_MASK       0x20000000
#define ERROR_SEVERITY_SUCCESS       0x00000000
#define ERROR_SEVERITY_INFORMATIONAL 0x40000000
#define ERROR_SEVERITY_WARNING       0x80000000
#define ERROR_SEVERITY_ERROR         0xC0000000


/*****************************************************************************
 * "LE" header - Windows 95 VxD device driver                                *
 *****************************************************************************/

typedef struct _EXEIMAGE_VXD_HEADER {      // Windows VXD header
    UINT16  e_magic;                   // Magic number
    UINT8   e_border;                  // The byte ordering for the VXD
    UINT8   e_worder;                  // The word ordering for the VXD
    UINT32  e_level;                   // The EXE format level for now = 0
    UINT16  e_cpu;                     // The CPU type
    UINT16  e_os;                      // The OS type
    UINT32  e_ver;                     // Module version
    UINT32  e_mflags;                  // Module flags
    UINT32  e_mpages;                  // Module # pages
    UINT32  e_startobj;                // Object # for instruction pointer
    UINT32  e_eip;                     // Extended instruction pointer
    UINT32  e_stackobj;                // Object # for stack pointer
    UINT32  e_esp;                     // Extended stack pointer
    UINT32  e_pagesize;                // VXD page size
    UINT32  e_lastpagesize;            // Last page size in VXD
    UINT32  e_fixupsize;               // Fixup section size
    UINT32  e_fixupsum;                // Fixup section checksum
    UINT32  e_ldrsize;                 // Loader section size
    UINT32  e_ldrsum;                  // Loader section checksum
    UINT32  e_objtab;                  // Object table offset
    UINT32  e_objcnt;                  // Number of objects in module
    UINT32  e_objmap;                  // Object page map offset
    UINT32  e_itermap;                 // Object iterated data map offset
    UINT32  e_rsrctab;                 // Offset of Resource Table
    UINT32  e_rsrccnt;                 // Number of resource entries
    UINT32  e_restab;                  // Offset of resident name table
    UINT32  e_enttab;                  // Offset of Entry Table
    UINT32  e_dirtab;                  // Offset of Module Directive Table
    UINT32  e_dircnt;                  // Number of module directives
    UINT32  e_fpagetab;                // Offset of Fixup Page Table
    UINT32  e_frectab;                 // Offset of Fixup Record Table
    UINT32  e_impmod;                  // Offset of Import Module Name Table
    UINT32  e_impmodcnt;               // Number of entries in Import Module Name Table
    UINT32  e_impproc;                 // Offset of Import Procedure Name Table
    UINT32  e_pagesum;                 // Offset of Per-Page Checksum Table
    UINT32  e_datapage;                // Offset of Enumerated Data Pages
    UINT32  e_preload;                 // Number of preload pages
    UINT32  e_nrestab;                 // Offset of Non-resident Names Table
    UINT32  e_cbnrestab;               // Size of Non-resident Name Table
    UINT32  e_nressum;                 // Non-resident Name Table Checksum
    UINT32  e_autodata;                // Object # for automatic data object
    UINT32  e_debuginfo;               // Offset of the debugging information
    UINT32  e_debuglen;                // The length of the debugging info. in bytes
    UINT32  e_instpreload;             // Number of instance pages in preload section of VXD file
    UINT32  e_instdemand;              // Number of instance pages in demand load section of VXD file
    UINT32  e_heapsize;                // Size of heap - for 16-bit apps
    UINT8   e_res3[12];                // Reserved words
    UINT32  e_winresoff;
    UINT32  e_winreslen;
    UINT16  e_devid;                   // Device ID for VxD
    UINT16  e_ddkver;                  // DDK version for VxD
  } EXEIMAGE_VXD_HEADER, *PEXEIMAGE_VXD_HEADER;
#define EXEIMAGE_LE_HEADER EXEIMAGE_VXD_HEADER
#define PEXEIMAGE_LE_HEADER PEXEIMAGE_VXD_HEADER


/*****************************************************************************
 * "PE" header - Windows NT, Windows 95 executables                          *
 *****************************************************************************/

typedef struct _EXEIMAGE_FILE_HEADER 
{
    UINT16 Machine;
    UINT16 NumberOfSections;
    UINT32 TimeDateStamp;
    UINT32 PointerToSymbolTable;
    UINT32 NumberOfSymbols;
    UINT16 SizeOfOptionalHeader;
    UINT16 Characteristics;
} EXEIMAGE_FILE_HEADER, *PEXEIMAGE_FILE_HEADER;

#define EXEIMAGE_SIZEOF_FILE_HEADER             20

#define EXEIMAGE_FILE_RELOCS_STRIPPED           0x0001  // Relocation info stripped from file.
#define EXEIMAGE_FILE_EXECUTABLE_EXEIMAGE          0x0002  // File is executable  (i.e. no unresolved externel references).
#define EXEIMAGE_FILE_LINE_NUMS_STRIPPED        0x0004  // Line nunbers stripped from file.
#define EXEIMAGE_FILE_LOCAL_SYMS_STRIPPED       0x0008  // Local symbols stripped from file.
#define EXEIMAGE_FILE_BYTES_REVERSED_LO         0x0080  // Bytes of machine word are reversed.
#define EXEIMAGE_FILE_32BIT_MACHINE             0x0100  // 32 bit word machine.
#define EXEIMAGE_FILE_DEBUG_STRIPPED            0x0200  // Debugging info stripped from file in .DBG file
#define EXEIMAGE_FILE_REMOVABLE_RUN_FROM_SWAP   0x0400  // If Image is on removable media, copy and run from the swap file.
#define EXEIMAGE_FILE_NET_RUN_FROM_SWAP         0x0800  // If Image is on Net, copy and run from the swap file.
#define EXEIMAGE_FILE_SYSTEM                    0x1000  // System File.
#define EXEIMAGE_FILE_DLL                       0x2000  // File is a DLL.
#define EXEIMAGE_FILE_UP_SYSTEM_ONLY            0x4000  // File should only be run on a UP machine
#define EXEIMAGE_FILE_BYTES_REVERSED_HI         0x8000  // Bytes of machine word are reversed.

#define EXEIMAGE_FILE_MACHINE_UNKNOWN           0
#define EXEIMAGE_FILE_MACHINE_I386              0x14c   // Intel 386.
#define EXEIMAGE_FILE_MACHINE_R3000             0x162   // MIPS little-endian, 0x160 big-endian
#define EXEIMAGE_FILE_MACHINE_R4000             0x166   // MIPS little-endian
#define EXEIMAGE_FILE_MACHINE_R10000            0x168   // MIPS little-endian
#define EXEIMAGE_FILE_MACHINE_ALPHA             0x184   // Alpha_AXP
#define EXEIMAGE_FILE_MACHINE_POWERPC           0x1F0   // IBM PowerPC Little-Endian

//
// Directory format.
//

typedef struct _EXEIMAGE_DATA_DIRECTORY {
    UINT32   VirtualAddress;
    UINT32   Size;
} EXEIMAGE_DATA_DIRECTORY, *PEXEIMAGE_DATA_DIRECTORY;

#define EXEIMAGE_NUMBEROF_DIRECTORY_ENTRIES    16

//
// Optional header format.
//

typedef struct _EXEIMAGE_OPTIONAL_HEADER {
    //
    // Standard fields.
    //

    UINT16     Magic;
    UINT8    MajorLinkerVersion;
    UINT8    MinorLinkerVersion;
    UINT32   SizeOfCode;
    UINT32   SizeOfInitializedData;
    UINT32   SizeOfUninitializedData;
    UINT32   AddressOfEntryPoint;
    UINT32   BaseOfCode;
    UINT32   BaseOfData;

    //
    // NT additional fields.
    //

    UINT32   ImageBase;
    UINT32   SectionAlignment;
    UINT32   FileAlignment;
    UINT16     MajorOperatingSystemVersion;
    UINT16     MinorOperatingSystemVersion;
    UINT16     MajorImageVersion;
    UINT16     MinorImageVersion;
    UINT16     MajorSubsystemVersion;
    UINT16     MinorSubsystemVersion;
    UINT32   Win32VersionValue;
    UINT32   SizeOfImage;
    UINT32   SizeOfHeaders;
    UINT32   CheckSum;
    UINT16     Subsystem;
    UINT16     DllCharacteristics;
    UINT32   SizeOfStackReserve;
    UINT32   SizeOfStackCommit;
    UINT32   SizeOfHeapReserve;
    UINT32   SizeOfHeapCommit;
    UINT32   LoaderFlags;
    UINT32   NumberOfRvaAndSizes;
    EXEIMAGE_DATA_DIRECTORY DataDirectory[EXEIMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} EXEIMAGE_OPTIONAL_HEADER, *PEXEIMAGE_OPTIONAL_HEADER;

typedef struct _EXEIMAGE_ROM_OPTIONAL_HEADER {
    UINT16    Magic;
    UINT8   MajorLinkerVersion;
    UINT8   MinorLinkerVersion;
    UINT32  SizeOfCode;
    UINT32  SizeOfInitializedData;
    UINT32  SizeOfUninitializedData;
    UINT32  AddressOfEntryPoint;
    UINT32  BaseOfCode;
    UINT32  BaseOfData;
    UINT32  BaseOfBss;
    UINT32  GprMask;
    UINT32  CprMask[4];
    UINT32  GpValue;
} EXEIMAGE_ROM_OPTIONAL_HEADER, *PEXEIMAGE_ROM_OPTIONAL_HEADER;

#define EXEIMAGE_SIZEOF_ROM_OPTIONAL_HEADER      56
#define EXEIMAGE_SIZEOF_STD_OPTIONAL_HEADER      28
#define EXEIMAGE_SIZEOF_NT_OPTIONAL_HEADER      224

#define EXEIMAGE_NT_OPTIONAL_HDR_MAGIC        0x10b
#define EXEIMAGE_ROM_OPTIONAL_HDR_MAGIC       0x107

typedef struct _EXEIMAGE_NT_HEADERS 
{
  UINT32                Signature;
  EXEIMAGE_FILE_HEADER     FileHeader;
  EXEIMAGE_OPTIONAL_HEADER OptionalHeader;
} EXEIMAGE_NT_HEADERS, *PEXEIMAGE_NT_HEADERS;
#define EXEIMAGE_PE_HEADER EXEIMAGE_NT_HEADERS
#define PEXEIMAGE_PE_HEADER PEXEIMAGE_NT_HEADERS

typedef struct _EXEIMAGE_ROM_HEADERS {
    EXEIMAGE_FILE_HEADER FileHeader;
    EXEIMAGE_ROM_OPTIONAL_HEADER OptionalHeader;
} EXEIMAGE_ROM_HEADERS, *PEXEIMAGE_ROM_HEADERS;

#define EXEIMAGE_FIRST_SECTION( ntheader ) ((PEXEIMAGE_SECTION_HEADER)        \
    ((DWORD)ntheader +                                                  \
     FIELD_OFFSET( EXEIMAGE_NT_HEADERS, OptionalHeader ) +                 \
     ((PEXEIMAGE_NT_HEADERS)(ntheader))->FileHeader.SizeOfOptionalHeader   \
    ))


// Subsystem Values

#define EXEIMAGE_SUBSYSTEM_UNKNOWN              0   // Unknown subsystem.
#define EXEIMAGE_SUBSYSTEM_NATIVE               1   // Image doesn't require a subsystem.
#define EXEIMAGE_SUBSYSTEM_WINDOWS_GUI          2   // Image runs in the Windows GUI subsystem.
#define EXEIMAGE_SUBSYSTEM_WINDOWS_CUI          3   // Image runs in the Windows character subsystem.
#define EXEIMAGE_SUBSYSTEM_OS2_CUI              5   // image runs in the OS/2 character subsystem.
#define EXEIMAGE_SUBSYSTEM_POSIX_CUI            7   // image run  in the Posix character subsystem.
#define EXEIMAGE_SUBSYSTEM_RESERVED8            8   // image run  in the 8 subsystem.


// Directory Entries

#define EXEIMAGE_DIRECTORY_ENTRY_EXPORT         0   // Export Directory
#define EXEIMAGE_DIRECTORY_ENTRY_IMPORT         1   // Import Directory
#define EXEIMAGE_DIRECTORY_ENTRY_RESOURCE       2   // Resource Directory
#define EXEIMAGE_DIRECTORY_ENTRY_EXCEPTION      3   // Exception Directory
#define EXEIMAGE_DIRECTORY_ENTRY_SECURITY       4   // Security Directory
#define EXEIMAGE_DIRECTORY_ENTRY_BASERELOC      5   // Base Relocation Table
#define EXEIMAGE_DIRECTORY_ENTRY_DEBUG          6   // Debug Directory
#define EXEIMAGE_DIRECTORY_ENTRY_COPYRIGHT      7   // Description String
#define EXEIMAGE_DIRECTORY_ENTRY_GLOBALPTR      8   // Machine Value (MIPS GP)
#define EXEIMAGE_DIRECTORY_ENTRY_TLS            9   // TLS Directory
#define EXEIMAGE_DIRECTORY_ENTRY_LOAD_CONFIG   10   // Load Configuration Directory
#define EXEIMAGE_DIRECTORY_ENTRY_BOUND_IMPORT  11   // Bound Import Directory in headers
#define EXEIMAGE_DIRECTORY_ENTRY_IAT           12   // Import Address Table

//
// Section header format.
//

#define EXEIMAGE_SIZEOF_SHOEXERT_NAME              8

typedef struct _EXEIMAGE_SECTION_HEADER {
    UINT8    Name[EXEIMAGE_SIZEOF_SHOEXERT_NAME];
    union {
            UINT32   PhysicalAddress;
            UINT32   VirtualSize;
    } Misc;
    UINT32   VirtualAddress;
    UINT32   SizeOfRawData;
    UINT32   PointerToRawData;
    UINT32   PointerToRelocations;
    UINT32   PointerToLinenumbers;
    UINT16     NumberOfRelocations;
    UINT16     NumberOfLinenumbers;
    UINT32   Characteristics;
} EXEIMAGE_SECTION_HEADER, *PEXEIMAGE_SECTION_HEADER;

#define EXEIMAGE_SIZEOF_SECTION_HEADER          40

//
// Section characteristics.
//

#define EXEIMAGE_SCN_TYPE_REG                   0x00000000  // Reserved.
#define EXEIMAGE_SCN_TYPE_DSECT                 0x00000001  // Reserved.
#define EXEIMAGE_SCN_TYPE_NOLOAD                0x00000002  // Reserved.
#define EXEIMAGE_SCN_TYPE_GROUP                 0x00000004  // Reserved.
#define EXEIMAGE_SCN_TYPE_NO_PAD                0x00000008  // Reserved.
#define EXEIMAGE_SCN_TYPE_COPY                  0x00000010  // Reserved.

#define EXEIMAGE_SCN_CNT_CODE                   0x00000020  // Section contains code.
#define EXEIMAGE_SCN_CNT_INITIALIZED_DATA       0x00000040  // Section contains initialized data.
#define EXEIMAGE_SCN_CNT_UNINITIALIZED_DATA     0x00000080  // Section contains uninitialized data.

#define EXEIMAGE_SCN_LNK_OTHER                  0x00000100  // Reserved.
#define EXEIMAGE_SCN_LNK_INFO                   0x00000200  // Section contains comments or some other type of information.
#define EXEIMAGE_SCN_TYPE_OVER                  0x00000400  // Reserved.
#define EXEIMAGE_SCN_LNK_REMOVE                 0x00000800  // Section contents will not become part of image.
#define EXEIMAGE_SCN_LNK_COMDAT                 0x00001000  // Section contents comdat.

#define EXEIMAGE_SCN_MEM_PROTECTED              0x00004000
#define EXEIMAGE_SCN_MEM_FARDATA                0x00008000
#define EXEIMAGE_SCN_MEM_SYSHEAP                0x00010000
#define EXEIMAGE_SCN_MEM_PURGEABLE              0x00020000
#define EXEIMAGE_SCN_MEM_16BIT                  0x00020000
#define EXEIMAGE_SCN_MEM_LOCKED                 0x00040000
#define EXEIMAGE_SCN_MEM_PRELOAD                0x00080000

#define EXEIMAGE_SCN_ALIGN_1BYTES               0x00100000  //
#define EXEIMAGE_SCN_ALIGN_2BYTES               0x00200000  //
#define EXEIMAGE_SCN_ALIGN_4BYTES               0x00300000  //
#define EXEIMAGE_SCN_ALIGN_8BYTES               0x00400000  //
#define EXEIMAGE_SCN_ALIGN_16BYTES              0x00500000  // Default alignment if no others are specified.
#define EXEIMAGE_SCN_ALIGN_32BYTES              0x00600000  //
#define EXEIMAGE_SCN_ALIGN_64BYTES              0x00700000  //
// Unused                                    0x00800000

#define EXEIMAGE_SCN_LNK_NRELOC_OVFL            0x01000000  // Section contains extended relocations.
#define EXEIMAGE_SCN_MEM_DISCARDABLE            0x02000000  // Section can be discarded.
#define EXEIMAGE_SCN_MEM_NOT_CACHED             0x04000000  // Section is not cachable.
#define EXEIMAGE_SCN_MEM_NOT_PAGED              0x08000000  // Section is not pageable.
#define EXEIMAGE_SCN_MEM_SHARED                 0x10000000  // Section is shareable.
#define EXEIMAGE_SCN_MEM_EXECUTE                0x20000000  // Section is executable.
#define EXEIMAGE_SCN_MEM_READ                   0x40000000  // Section is readable.
#define EXEIMAGE_SCN_MEM_WRITE                  0x80000000  // Section is writeable.


//
// Symbol format.
//

typedef struct _EXEIMAGE_SYMBOL {
    union {
        UINT8    ShortName[8];
        struct {
            UINT32   Short;     // if 0, use LongName
            UINT32   Long;      // offset into string table
        } Name;
        PBYTE   LongName[2];
    } N;
    UINT32   Value;
    SHORT   SectionNumber;
    UINT16     Type;
    UINT8    StorageClass;
    UINT8    NumberOfAuxSymbols;
} EXEIMAGE_SYMBOL;
typedef EXEIMAGE_SYMBOL *PEXEIMAGE_SYMBOL;


#define EXEIMAGE_SIZEOF_SYMBOL                  18

//
// Section values.
//
// Symbols have a section number of the section in which they are
// defined. Otherwise, section numbers have the following meanings:
//

#define EXEIMAGE_SYM_UNDEFINED           (SHORT)0          // Symbol is undefined or is common.
#define EXEIMAGE_SYM_ABSOLUTE            (SHORT)-1         // Symbol is an absolute value.
#define EXEIMAGE_SYM_DEBUG               (SHORT)-2         // Symbol is a special debug item.

//
// Type (fundamental) values.
//

#define EXEIMAGE_SYM_TYPE_NULL                 0x0000  // no type.
#define EXEIMAGE_SYM_TYPE_VOID                 0x0001  //
#define EXEIMAGE_SYM_TYPE_CHAR                 0x0002  // type character.
#define EXEIMAGE_SYM_TYPE_SHORT                0x0003  // type short integer.
#define EXEIMAGE_SYM_TYPE_INT                  0x0004  //
#define EXEIMAGE_SYM_TYPE_LONG                 0x0005  //
#define EXEIMAGE_SYM_TYPE_FLOAT                0x0006  //
#define EXEIMAGE_SYM_TYPE_DOUBLE               0x0007  //
#define EXEIMAGE_SYM_TYPE_STRUCT               0x0008  //
#define EXEIMAGE_SYM_TYPE_UNION                0x0009  //
#define EXEIMAGE_SYM_TYPE_ENUM                 0x000A  // enumeration.
#define EXEIMAGE_SYM_TYPE_MOE                  0x000B  // member of enumeration.
#define EXEIMAGE_SYM_TYPE_BYTE                 0x000C  //
#define EXEIMAGE_SYM_TYPE_WORD                 0x000D  //
#define EXEIMAGE_SYM_TYPE_UINT                 0x000E  //
#define EXEIMAGE_SYM_TYPE_DWORD                0x000F  //
#define EXEIMAGE_SYM_TYPE_PCODE                0x8000  //

//
// Type (derived) values.
//

#define EXEIMAGE_SYM_DTYPE_NULL                0       // no derived type.
#define EXEIMAGE_SYM_DTYPE_POINTER             1       // pointer.
#define EXEIMAGE_SYM_DTYPE_FUNCTION            2       // function.
#define EXEIMAGE_SYM_DTYPE_ARRAY               3       // array.

//
// Storage classes.
//

#define EXEIMAGE_SYM_CLASS_END_OF_FUNCTION     (BYTE )-1
#define EXEIMAGE_SYM_CLASS_NULL                0x0000
#define EXEIMAGE_SYM_CLASS_AUTOMATIC           0x0001
#define EXEIMAGE_SYM_CLASS_EXTERNAL            0x0002
#define EXEIMAGE_SYM_CLASS_STATIC              0x0003
#define EXEIMAGE_SYM_CLASS_REGISTER            0x0004
#define EXEIMAGE_SYM_CLASS_EXTERNAL_DEF        0x0005
#define EXEIMAGE_SYM_CLASS_LABEL               0x0006
#define EXEIMAGE_SYM_CLASS_UNDEFINED_LABEL     0x0007
#define EXEIMAGE_SYM_CLASS_MEMBER_OF_STRUCT    0x0008
#define EXEIMAGE_SYM_CLASS_ARGUMENT            0x0009
#define EXEIMAGE_SYM_CLASS_STRUCT_TAG          0x000A
#define EXEIMAGE_SYM_CLASS_MEMBER_OF_UNION     0x000B
#define EXEIMAGE_SYM_CLASS_UNION_TAG           0x000C
#define EXEIMAGE_SYM_CLASS_TYPE_DEFINITION     0x000D
#define EXEIMAGE_SYM_CLASS_UNDEFINED_STATIC    0x000E
#define EXEIMAGE_SYM_CLASS_ENUM_TAG            0x000F
#define EXEIMAGE_SYM_CLASS_MEMBER_OF_ENUM      0x0010
#define EXEIMAGE_SYM_CLASS_REGISTER_PARAM      0x0011
#define EXEIMAGE_SYM_CLASS_BIT_FIELD           0x0012

#define EXEIMAGE_SYM_CLASS_FAR_EXTERNAL        0x0044  //

#define EXEIMAGE_SYM_CLASS_BLOCK               0x0064
#define EXEIMAGE_SYM_CLASS_FUNCTION            0x0065
#define EXEIMAGE_SYM_CLASS_END_OF_STRUCT       0x0066
#define EXEIMAGE_SYM_CLASS_FILE                0x0067
// new
#define EXEIMAGE_SYM_CLASS_SECTION             0x0068
#define EXEIMAGE_SYM_CLASS_WEAK_EXTERNAL       0x0069

// type packing constants

#define N_BTMASK                            0x000F
#define N_TMASK                             0x0030
#define N_TMASK1                            0x00C0
#define N_TMASK2                            0x00F0
#define N_BTSHFT                            4
#define N_TSHIFT                            2

// MACROS

// Basic Type of  x
#define BTYPE(x) ((x) & N_BTMASK)

// Is x a pointer?
#ifndef ISPTR
#define ISPTR(x) (((x) & N_TMASK) == (EXEIMAGE_SYM_DTYPE_POINTER << N_BTSHFT))
#endif

// Is x a function?
#ifndef ISFCN
#define ISFCN(x) (((x) & N_TMASK) == (EXEIMAGE_SYM_DTYPE_FUNCTION << N_BTSHFT))
#endif

// Is x an array?

#ifndef ISARY
#define ISARY(x) (((x) & N_TMASK) == (EXEIMAGE_SYM_DTYPE_ARRAY << N_BTSHFT))
#endif

// Is x a structure, union, or enumeration TAG?
#ifndef ISTAG
#define ISTAG(x) ((x)==EXEIMAGE_SYM_CLASS_STRUCT_TAG || (x)==EXEIMAGE_SYM_CLASS_UNION_TAG || (x)==EXEIMAGE_SYM_CLASS_ENUM_TAG)
#endif

#ifndef INCREF
#define INCREF(x) ((((x)&~N_BTMASK)<<N_TSHIFT)|(EXEIMAGE_SYM_DTYPE_POINTER<<N_BTSHFT)|((x)&N_BTMASK))
#endif
#ifndef DECREF
#define DECREF(x) ((((x)>>N_TSHIFT)&~N_BTMASK)|((x)&N_BTMASK))
#endif

//
// Auxiliary entry format.
//

typedef union _EXEIMAGE_AUX_SYMBOL {
    struct {
        UINT32    TagIndex;                      // struct, union, or enum tag index
        union {
            struct {
                UINT16     Linenumber;             // declaration line number
                UINT16     Size;                   // size of struct, union, or enum
            } LnSz;
           UINT32    TotalSize;
        } Misc;
        union {
            struct {                            // if ISFCN, tag, or .bb
                UINT32    PointerToLinenumber;
                UINT32    PointerToNextFunction;
            } Function;
            struct {                            // if ISARY, up to 4 dimen.
                UINT16      Dimension[4];
            } Array;
        } FcnAry;
        UINT16     TvIndex;                        // tv index
    } Sym;
    struct {
        UINT8    Name[EXEIMAGE_SIZEOF_SYMBOL];
    } File;
    struct {
        UINT32   Length;                         // section length
        UINT16     NumberOfRelocations;            // number of relocation entries
        UINT16     NumberOfLinenumbers;            // number of line numbers
        UINT32   CheckSum;                       // checksum for communal
        SHORT   Number;                         // section number to associate with
        UINT8    Selection;                      // communal selection type
    } Section;
} EXEIMAGE_AUX_SYMBOL;
typedef EXEIMAGE_AUX_SYMBOL *PEXEIMAGE_AUX_SYMBOL;

#define EXEIMAGE_SIZEOF_AUX_SYMBOL             18

//
// Communal selection types.
//

#define EXEIMAGE_COMDAT_SELECT_NODUPLICATES    1
#define EXEIMAGE_COMDAT_SELECT_ANY             2
#define EXEIMAGE_COMDAT_SELECT_SAME_SIZE       3
#define EXEIMAGE_COMDAT_SELECT_EXACT_MATCH     4
#define EXEIMAGE_COMDAT_SELECT_ASSOCIATIVE     5
#define EXEIMAGE_COMDAT_SELECT_LARGEST         6
#define EXEIMAGE_COMDAT_SELECT_NEWEST          7

#define EXEIMAGE_WEAK_EXTERN_SEARCH_NOLIBRARY  1
#define EXEIMAGE_WEAK_EXTERN_SEARCH_LIBRARY    2
#define EXEIMAGE_WEAK_EXTERN_SEARCH_ALIAS      3

//
// Relocation format.
//

typedef struct _EXEIMAGE_RELOCATION 
{
  union

  {
    UINT32 VirtualAddress;
    UINT32 RelocCount; // Set to the real count when EXEIMAGE_SCN_LNK_NRELOC_OVFL is set
  } uRelovation;
  UINT32 SymbolTableIndex;
  UINT16 Type;
} EXEIMAGE_RELOCATION;
typedef EXEIMAGE_RELOCATION *PEXEIMAGE_RELOCATION;

#define EXEIMAGE_SIZEOF_RELOCATION         10

//
// I386 relocation types.
//

#define EXEIMAGE_REL_I386_ABSOLUTE         0x0000  // Reference is absolute, no relocation is necessary
#define EXEIMAGE_REL_I386_DIR16            0x0001  // Direct 16-bit reference to the symbols virtual address
#define EXEIMAGE_REL_I386_REL16            0x0002  // PC-relative 16-bit reference to the symbols virtual address
#define EXEIMAGE_REL_I386_DIR32            0x0006  // Direct 32-bit reference to the symbols virtual address
#define EXEIMAGE_REL_I386_DIR32NB          0x0007  // Direct 32-bit reference to the symbols virtual address, base not included
#define EXEIMAGE_REL_I386_SEG12            0x0009  // Direct 16-bit reference to the segment-selector bits of a 32-bit virtual address
#define EXEIMAGE_REL_I386_SECTION          0x000A
#define EXEIMAGE_REL_I386_SECREL           0x000B
#define EXEIMAGE_REL_I386_REL32            0x0014  // PC-relative 32-bit reference to the symbols virtual address

//
// MIPS relocation types.
//

#define EXEIMAGE_REL_MIPS_ABSOLUTE         0x0000  // Reference is absolute, no relocation is necessary
#define EXEIMAGE_REL_MIPS_REFHALF          0x0001
#define EXEIMAGE_REL_MIPS_REFWORD          0x0002
#define EXEIMAGE_REL_MIPS_JMPADDR          0x0003
#define EXEIMAGE_REL_MIPS_REFHI            0x0004
#define EXEIMAGE_REL_MIPS_REFLO            0x0005
#define EXEIMAGE_REL_MIPS_GPREL            0x0006
#define EXEIMAGE_REL_MIPS_LITERAL          0x0007
#define EXEIMAGE_REL_MIPS_SECTION          0x000A
#define EXEIMAGE_REL_MIPS_SECREL           0x000B
#define EXEIMAGE_REL_MIPS_SECRELLO         0x000C  // Low 16-bit section relative referemce (used for >32k TLS)
#define EXEIMAGE_REL_MIPS_SECRELHI         0x000D  // High 16-bit section relative reference (used for >32k TLS)
#define EXEIMAGE_REL_MIPS_REFWORDNB        0x0022
#define EXEIMAGE_REL_MIPS_PAIR             0x0025

//
// Alpha Relocation types.
//

#define EXEIMAGE_REL_ALPHA_ABSOLUTE        0x0000
#define EXEIMAGE_REL_ALPHA_REFLONG         0x0001
#define EXEIMAGE_REL_ALPHA_REFQUAD         0x0002
#define EXEIMAGE_REL_ALPHA_GPREL32         0x0003
#define EXEIMAGE_REL_ALPHA_LITERAL         0x0004
#define EXEIMAGE_REL_ALPHA_LITUSE          0x0005
#define EXEIMAGE_REL_ALPHA_GPDISP          0x0006
#define EXEIMAGE_REL_ALPHA_BRADDR          0x0007
#define EXEIMAGE_REL_ALPHA_HINT            0x0008
#define EXEIMAGE_REL_ALPHA_INLINE_REFLONG  0x0009
#define EXEIMAGE_REL_ALPHA_REFHI           0x000A
#define EXEIMAGE_REL_ALPHA_REFLO           0x000B
#define EXEIMAGE_REL_ALPHA_PAIR            0x000C
#define EXEIMAGE_REL_ALPHA_MATCH           0x000D
#define EXEIMAGE_REL_ALPHA_SECTION         0x000E
#define EXEIMAGE_REL_ALPHA_SECREL          0x000F
#define EXEIMAGE_REL_ALPHA_REFLONGNB       0x0010
#define EXEIMAGE_REL_ALPHA_SECRELLO        0x0011  // Low 16-bit section relative reference
#define EXEIMAGE_REL_ALPHA_SECRELHI        0x0012  // High 16-bit section relative reference

//
// IBM PowerPC relocation types.
//

#define EXEIMAGE_REL_PPC_ABSOLUTE          0x0000  // NOP
#define EXEIMAGE_REL_PPC_ADDR64            0x0001  // 64-bit address
#define EXEIMAGE_REL_PPC_ADDR32            0x0002  // 32-bit address
#define EXEIMAGE_REL_PPC_ADDR24            0x0003  // 26-bit address, shifted left 2 (branch absolute)
#define EXEIMAGE_REL_PPC_ADDR16            0x0004  // 16-bit address
#define EXEIMAGE_REL_PPC_ADDR14            0x0005  // 16-bit address, shifted left 2 (load doubleword)
#define EXEIMAGE_REL_PPC_REL24             0x0006  // 26-bit PC-relative offset, shifted left 2 (branch relative)
#define EXEIMAGE_REL_PPC_REL14             0x0007  // 16-bit PC-relative offset, shifted left 2 (br cond relative)
#define EXEIMAGE_REL_PPC_TOCREL16          0x0008  // 16-bit offset from TOC base
#define EXEIMAGE_REL_PPC_TOCREL14          0x0009  // 16-bit offset from TOC base, shifted left 2 (load doubleword)

#define EXEIMAGE_REL_PPC_ADDR32NB          0x000A  // 32-bit addr w/o image base
#define EXEIMAGE_REL_PPC_SECREL            0x000B  // va of containing section (as in an image sectionhdr)
#define EXEIMAGE_REL_PPC_SECTION           0x000C  // sectionheader number
#define EXEIMAGE_REL_PPC_IFGLUE            0x000D  // substitute TOC restore instruction iff symbol is glue code
#define EXEIMAGE_REL_PPC_IMGLUE            0x000E  // symbol is glue code; virtual address is TOC restore instruction
#define EXEIMAGE_REL_PPC_SECREL16          0x000F  // va of containing section (limited to 16 bits)
#define EXEIMAGE_REL_PPC_REFHI             0x0010
#define EXEIMAGE_REL_PPC_REFLO             0x0011
#define EXEIMAGE_REL_PPC_PAIR              0x0012
#define EXEIMAGE_REL_PPC_SECRELLO          0x0013  // Low 16-bit section relative reference (used for >32k TLS)
#define EXEIMAGE_REL_PPC_SECRELHI          0x0014  // High 16-bit section relative reference (used for >32k TLS)

#define EXEIMAGE_REL_PPC_TYPEMASK          0x00FF  // mask to isolate above values in EXEIMAGE_RELOCATION.Type

// Flag bits in EXEIMAGE_RELOCATION.TYPE

#define EXEIMAGE_REL_PPC_NEG               0x0100  // subtract reloc value rather than adding it
#define EXEIMAGE_REL_PPC_BRTAKEN           0x0200  // fix branch prediction bit to predict branch taken
#define EXEIMAGE_REL_PPC_BRNTAKEN          0x0400  // fix branch prediction bit to predict branch not taken
#define EXEIMAGE_REL_PPC_TOCDEFN           0x0800  // toc slot defined in file (or, data in toc)

//
// Line number format.
//

typedef struct _EXEIMAGE_LINENUMBER {
    union {
        UINT32   SymbolTableIndex;               // Symbol table index of function name if Linenumber is 0.
        UINT32   VirtualAddress;                 // Virtual address of line number.
    } Type;
    UINT16     Linenumber;                         // Line number.
} EXEIMAGE_LINENUMBER;
typedef EXEIMAGE_LINENUMBER *PEXEIMAGE_LINENUMBER;

#define EXEIMAGE_SIZEOF_LINENUMBER              6


//
// Based relocation format.
//

typedef struct _EXEIMAGE_BASE_RELOCATION {
    UINT32   VirtualAddress;
    UINT32   SizeOfBlock;
//  UINT16     TypeOffset[1];
} EXEIMAGE_BASE_RELOCATION, *PEXEIMAGE_BASE_RELOCATION;

#define EXEIMAGE_SIZEOF_BASE_RELOCATION         8

//
// Based relocation types.
//

#define EXEIMAGE_REL_BASED_ABSOLUTE              0
#define EXEIMAGE_REL_BASED_HIGH                  1
#define EXEIMAGE_REL_BASED_LOW                   2
#define EXEIMAGE_REL_BASED_HIGHLOW               3
#define EXEIMAGE_REL_BASED_HIGHADJ               4
#define EXEIMAGE_REL_BASED_MIPS_JMPADDR          5

//
// Archive format.
//

#define EXEIMAGE_ARCHIVE_STAEXERT_SIZE             8
#define EXEIMAGE_ARCHIVE_START                  "!<arch>\n"
#define EXEIMAGE_ARCHIVE_END                    "`\n"
#define EXEIMAGE_ARCHIVE_PAD                    "\n"
#define EXEIMAGE_ARCHIVE_LINKER_MEMBER          "/               "
#define EXEIMAGE_ARCHIVE_LONGNAMES_MEMBER       "//              "

typedef struct _EXEIMAGE_ARCHIVE_MEMBER_HEADER {
    UINT8     Name[16];                          // File member name - `/' terminated.
    UINT8     Date[12];                          // File member date - decimal.
    UINT8     UserID[6];                         // File member user id - decimal.
    UINT8     GroupID[6];                        // File member group id - decimal.
    UINT8     Mode[8];                           // File member mode - octal.
    UINT8     Size[10];                          // File member size - decimal.
    UINT8     EndHeader[2];                      // String to end header.
} EXEIMAGE_ARCHIVE_MEMBER_HEADER, *PEXEIMAGE_ARCHIVE_MEMBER_HEADER;

#define EXEIMAGE_SIZEOF_ARCHIVE_MEMBER_HDR      60

//
// DLL support.
//

//
// Export Format
//

typedef struct _EXEIMAGE_EXPOEXERT_DIRECTORY {
    UINT32   Characteristics;
    UINT32   TimeDateStamp;
    UINT16   MajorVersion;
    UINT16   MinorVersion;
    UINT32   Name;
    UINT32   Base;
    UINT32   NumberOfFunctions;
    UINT32   NumberOfNames;
    PUINT32  *AddressOfFunctions;
    PUINT32  *AddressOfNames;
    PUINT16  *AddressOfNameOrdinals;
} EXEIMAGE_EXPOEXERT_DIRECTORY, *PEXEIMAGE_EXPOEXERT_DIRECTORY;

//
// Import Format
//

typedef struct _EXEIMAGE_IMPOEXERT_BY_NAME {
    UINT16     Hint;
    UINT8    Name[1];
} EXEIMAGE_IMPOEXERT_BY_NAME, *PEXEIMAGE_IMPOEXERT_BY_NAME;

typedef struct _EXEIMAGE_THUNK_DATA {
    union {
        PBYTE  ForwarderString;
        PUINT32 Function;
        UINT32 Ordinal;
        PEXEIMAGE_IMPOEXERT_BY_NAME AddressOfData;
    } u1;
} EXEIMAGE_THUNK_DATA;
typedef EXEIMAGE_THUNK_DATA * PEXEIMAGE_THUNK_DATA;

#define EXEIMAGE_ORDINAL_FLAG 0x80000000
#define EXEIMAGE_SNAP_BY_ORDINAL(Ordinal) ((Ordinal & EXEIMAGE_ORDINAL_FLAG) != 0)
#define EXEIMAGE_ORDINAL(Ordinal) (Ordinal & 0xffff)

typedef struct _EXEIMAGE_IMPOEXERT_DESCRIPTOR {
    union {
        UINT32   Characteristics;                // 0 for terminating null import descriptor
        PEXEIMAGE_THUNK_DATA OriginalFirstThunk;   // RVA to original unbound IAT
    } uThunkData;
    UINT32   TimeDateStamp;                  // 0 if not bound,
                                            // -1 if bound, and real date\time stamp
                                            //     in EXEIMAGE_DIRECTORY_ENTRY_BOUND_IMPORT (new BIND)
                                            // O.W. date/time stamp of DLL bound to (Old BIND)

    UINT32   ForwarderChain;                 // -1 if no forwarders
    UINT32   Name;
    PEXEIMAGE_THUNK_DATA FirstThunk;           // RVA to IAT (if bound this IAT has actual addresses)
} EXEIMAGE_IMPOEXERT_DESCRIPTOR;
typedef EXEIMAGE_IMPOEXERT_DESCRIPTOR *PEXEIMAGE_IMPOEXERT_DESCRIPTOR;

//
// New format import descriptors pointed to by DataDirectory[ EXEIMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ]
//

typedef struct _EXEIMAGE_BOUND_IMPOEXERT_DESCRIPTOR {
    UINT32   TimeDateStamp;
    UINT16     OffsetModuleName;
    UINT16     NumberOfModuleForwarderRefs;
// Array of zero or more EXEIMAGE_BOUND_FORWARDER_REF follows
} EXEIMAGE_BOUND_IMPOEXERT_DESCRIPTOR,  *PEXEIMAGE_BOUND_IMPOEXERT_DESCRIPTOR;

typedef struct _EXEIMAGE_BOUND_FORWARDER_REF {
    UINT32   TimeDateStamp;
    UINT16     OffsetModuleName;
    UINT16     Reserved;
} EXEIMAGE_BOUND_FORWARDER_REF, *PEXEIMAGE_BOUND_FORWARDER_REF;


//
// Thread Local Storage
//

typedef VOID
(*PEXEIMAGE_TLS_CALLBACK) (
    PVOID DllHandle,
    UINT32 Reason,
    PVOID Reserved
    );

typedef struct _EXEIMAGE_TLS_DIRECTORY {
    UINT32   StartAddressOfRawData;
    UINT32   EndAddressOfRawData;
    PUINT32  AddressOfIndex;
    PEXEIMAGE_TLS_CALLBACK *AddressOfCallBacks;
    UINT32   SizeOfZeroFill;
    UINT32   Characteristics;
} EXEIMAGE_TLS_DIRECTORY, *PEXEIMAGE_TLS_DIRECTORY;


//
// Resource Format.
//

//
// Resource directory consists of two counts, following by a variable length
// array of directory entries.  The first count is the number of entries at
// beginning of the array that have actual names associated with each entry.
// The entries are in ascending order, case insensitive strings.  The second
// count is the number of entries that immediately follow the named entries.
// This second count identifies the number of entries that have 16-bit integer
// Ids as their name.  These entries are also sorted in ascending order.
//
// This structure allows fast lookup by either name or number, but for any
// given resource entry only one form of lookup is supported, not both.
// This is consistant with the syntax of the .RC file and the .RES file.
//

typedef struct _EXEIMAGE_RESOURCE_DIRECTORY {
    UINT32   Characteristics;
    UINT32   TimeDateStamp;
    UINT16     MajorVersion;
    UINT16     MinorVersion;
    UINT16     NumberOfNamedEntries;
    UINT16     NumberOfIdEntries;
//  EXEIMAGE_RESOURCE_DIRECTORY_ENTRY DirectoryEntries[];
} EXEIMAGE_RESOURCE_DIRECTORY, *PEXEIMAGE_RESOURCE_DIRECTORY;

#define EXEIMAGE_RESOURCE_NAME_IS_STRING        0x80000000
#define EXEIMAGE_RESOURCE_DATA_IS_DIRECTORY     0x80000000

//
// Each directory contains the 32-bit Name of the entry and an offset,
// relative to the beginning of the resource directory of the data associated
// with this directory entry.  If the name of the entry is an actual text
// string instead of an integer Id, then the high order bit of the name field
// is set to one and the low order 31-bits are an offset, relative to the
// beginning of the resource directory of the string, which is of type
// EXEIMAGE_RESOURCE_DIRECTORY_STRING.  Otherwise the high bit is clear and the
// low-order 16-bits are the integer Id that identify this resource directory
// entry. If the directory entry is yet another resource directory (i.e. a
// subdirectory), then the high order bit of the offset field will be
// set to indicate this.  Otherwise the high bit is clear and the offset
// field points to a resource data entry.
//

typedef struct _EXEIMAGE_RESOURCE_DIRECTORY_ENTRY {
    union {
        struct {
            UINT32 NameOffset:31;
            UINT32 NameIsString:1;
        } uName;
        UINT32   Name;
        UINT16   Id;
    } uNameID;
    union {
        UINT32   OffsetToData;
        struct {
            UINT32   OffsetToDirectory:31;
            UINT32   DataIsDirectory:1;
        } uDirectoryOffset;
    } uDirectory;
} EXEIMAGE_RESOURCE_DIRECTORY_ENTRY, *PEXEIMAGE_RESOURCE_DIRECTORY_ENTRY;

//
// For resource directory entries that have actual string names, the Name
// field of the directory entry points to an object of the following type.
// All of these string objects are stored together after the last resource
// directory entry and before the first resource data object.  This minimizes
// the impact of these variable length objects on the alignment of the fixed
// size directory entry objects.
//

typedef struct _EXEIMAGE_RESOURCE_DIRECTORY_STRING {
    UINT16     Length;
    CHAR    NameString[ 1 ];
} EXEIMAGE_RESOURCE_DIRECTORY_STRING, *PEXEIMAGE_RESOURCE_DIRECTORY_STRING;


typedef struct _EXEIMAGE_RESOURCE_DIR_STRING_U {
    UINT16     Length;
    UINT8   NameString[ 1 ];
} EXEIMAGE_RESOURCE_DIR_STRING_U, *PEXEIMAGE_RESOURCE_DIR_STRING_U;


//
// Each resource data entry describes a leaf node in the resource directory
// tree.  It contains an offset, relative to the beginning of the resource
// directory of the data for the resource, a size field that gives the number
// of bytes of data at that offset, a CodePage that should be used when
// decoding code point values within the resource data.  Typically for new
// applications the code page would be the unicode code page.
//

typedef struct _EXEIMAGE_RESOURCE_DATA_ENTRY {
    UINT32   OffsetToData;
    UINT32   Size;
    UINT32   CodePage;
    UINT32   Reserved;
} EXEIMAGE_RESOURCE_DATA_ENTRY, *PEXEIMAGE_RESOURCE_DATA_ENTRY;

//
// Load Configuration Directory Entry
//

typedef struct _EXEIMAGE_LOAD_CONFIG_DIRECTORY {
    UINT32   Characteristics;
    UINT32   TimeDateStamp;
    UINT16     MajorVersion;
    UINT16     MinorVersion;
    UINT32   GlobalFlagsClear;
    UINT32   GlobalFlagsSet;
    UINT32   CriticalSectionDefaultTimeout;
    UINT32   DeCommitFreeBlockThreshold;
    UINT32   DeCommitTotalFreeThreshold;
    PVOID   LockPrefixTable;
    UINT32   MaximumAllocationSize;
    UINT32   VirtualMemoryThreshold;
    UINT32   ProcessHeapFlags;
    UINT32   ProcessAffinityMask;
    UINT32   Reserved[ 3 ];
} EXEIMAGE_LOAD_CONFIG_DIRECTORY, *PEXEIMAGE_LOAD_CONFIG_DIRECTORY;


//
// Function table entry format for MIPS/ALPHA images.  Function table is
// pointed to by the EXEIMAGE_DIRECTORY_ENTRY_EXCEPTION directory entry.
// This definition duplicates ones in ntmips.h and ntalpha.h for use
// by portable image file mungers.
//

typedef struct _EXEIMAGE_RUNTIME_FUNCTION_ENTRY {
    UINT32 BeginAddress;
    UINT32 EndAddress;
    PVOID ExceptionHandler;
    PVOID HandlerData;
    UINT32 PrologEndAddress;
} EXEIMAGE_RUNTIME_FUNCTION_ENTRY, *PEXEIMAGE_RUNTIME_FUNCTION_ENTRY;

//
// Debug Format
//

typedef struct _EXEIMAGE_DEBUG_DIRECTORY {
    UINT32   Characteristics;
    UINT32   TimeDateStamp;
    UINT16     MajorVersion;
    UINT16     MinorVersion;
    UINT32   Type;
    UINT32   SizeOfData;
    UINT32   AddressOfRawData;
    UINT32   PointerToRawData;
} EXEIMAGE_DEBUG_DIRECTORY, *PEXEIMAGE_DEBUG_DIRECTORY;

#define EXEIMAGE_DEBUG_TYPE_UNKNOWN          0
#define EXEIMAGE_DEBUG_TYPE_COFF             1
#define EXEIMAGE_DEBUG_TYPE_CODEVIEW         2
#define EXEIMAGE_DEBUG_TYPE_FPO              3
#define EXEIMAGE_DEBUG_TYPE_MISC             4
#define EXEIMAGE_DEBUG_TYPE_EXCEPTION        5
#define EXEIMAGE_DEBUG_TYPE_FIXUP            6
#define EXEIMAGE_DEBUG_TYPE_OMAP_TO_SRC      7
#define EXEIMAGE_DEBUG_TYPE_OMAP_FROM_SRC    8


typedef struct _EXEIMAGE_COFF_SYMBOLS_HEADER {
    UINT32   NumberOfSymbols;
    UINT32   LvaToFirstSymbol;
    UINT32   NumberOfLinenumbers;
    UINT32   LvaToFirstLinenumber;
    UINT32   RvaToFirstByteOfCode;
    UINT32   RvaToLastByteOfCode;
    UINT32   RvaToFirstByteOfData;
    UINT32   RvaToLastByteOfData;
} EXEIMAGE_COFF_SYMBOLS_HEADER, *PEXEIMAGE_COFF_SYMBOLS_HEADER;

#define FRAME_FPO       0
#define FRAME_TRAP      1
#define FRAME_TSS       2
#define FRAME_NONFPO    3

#ifndef _WIN32
typedef struct _FPO_DATA {
    UINT32       ulOffStart;             // offset 1st byte of function code
    UINT32       cbProcSize;             // # bytes in function
    UINT32       cdwLocals;              // # bytes in locals/4
    UINT16         cdwParams;              // # bytes in params/4
    UINT16         cbProlog : 8;           // # bytes in prolog
    UINT16         cbRegs   : 3;           // # regs saved
    UINT16         fHasSEH  : 1;           // TRUE if SEH in func
    UINT16         fUseBP   : 1;           // TRUE if EBP has been allocated
    UINT16         reserved : 1;           // reserved for future use
    UINT16         cbFrame  : 2;           // frame type
} FPO_DATA, *PFPO_DATA;
#define SIZEOF_RFPO_DATA 16
#endif


#define EXEIMAGE_DEBUG_MISC_EXENAME    1

typedef struct _EXEIMAGE_DEBUG_MISC {
    UINT32       DataType;               // type of misc data, see defines
    UINT32       Length;                 // total length of record, rounded to four
                                        // byte multiple.
    UINT8     Unicode;                // TRUE if data is unicode string
    UINT8        Reserved[ 3 ];
    UINT8        Data[ 1 ];              // Actual data
} EXEIMAGE_DEBUG_MISC, *PEXEIMAGE_DEBUG_MISC;


//
// Function table extracted from MIPS/ALPHA images.  Does not contain
// information needed only for runtime support.  Just those fields for
// each entry needed by a debugger.
//

typedef struct _EXEIMAGE_FUNCTION_ENTRY {
    UINT32   StartingAddress;
    UINT32   EndingAddress;
    UINT32   EndOfPrologue;
} EXEIMAGE_FUNCTION_ENTRY, *PEXEIMAGE_FUNCTION_ENTRY;

//
// Debugging information can be stripped from an image file and placed
// in a separate .DBG file, whose file name part is the same as the
// image file name part (e.g. symbols for CMD.EXE could be stripped
// and placed in CMD.DBG).  This is indicated by the EXEIMAGE_FILE_DEBUG_STRIPPED
// flag in the Characteristics field of the file header.  The beginning of
// the .DBG file contains the following structure which captures certain
// information from the image file.  This allows a debug to proceed even if
// the original image file is not accessable.  This header is followed by
// zero of more EXEIMAGE_SECTION_HEADER structures, followed by zero or more
// EXEIMAGE_DEBUG_DIRECTORY structures.  The latter structures and those in
// the image file contain file offsets relative to the beginning of the
// .DBG file.
//
// If symbols have been stripped from an image, the EXEIMAGE_DEBUG_MISC structure
// is left in the image file, but not mapped.  This allows a debugger to
// compute the name of the .DBG file, from the name of the image in the
// EXEIMAGE_DEBUG_MISC structure.
//

typedef struct _EXEIMAGE_SEPARATE_DEBUG_HEADER {
    UINT16         Signature;
    UINT16         Flags;
    UINT16         Machine;
    UINT16         Characteristics;
    UINT32       TimeDateStamp;
    UINT32       CheckSum;
    UINT32       ImageBase;
    UINT32       SizeOfImage;
    UINT32       NumberOfSections;
    UINT32       ExportedNamesSize;
    UINT32       DebugDirectorySize;
    UINT32       SectionAlignment;
    UINT32       Reserved[2];
} EXEIMAGE_SEPARATE_DEBUG_HEADER, *PEXEIMAGE_SEPARATE_DEBUG_HEADER;

#define EXEIMAGE_SEPARATE_DEBUG_SIGNATURE  0x4944

#define EXEIMAGE_SEPARATE_DEBUG_FLAGS_MASK 0x8000
#define EXEIMAGE_SEPARATE_DEBUG_MISMATCH   0x8000  // when DBG was updated, the
                                                // old checksum didn't match.




#ifdef __cplusplus
}
#endif


#endif /* MODULE_EXEINFO */

