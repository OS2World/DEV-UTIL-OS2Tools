/* $Id: OMFobj.h,v 1.2 2001/06/07 03:31:29 bird Exp $ */
/*
 * Relocatable Object Module Format (OMF) definitions and declarations.
 *
 * Copyright (c) 1999-2001 knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 *
 */

#ifndef _OMF_H_
#define _OMF_H_

/*
 * Record type flag.
 */
#define FLAGS_32                0x01    /* The least significant bit of the type is  */
                                        /* used to signal Link386 records. */

/*
 * Object record types, classes and subtypes.
 */
/* Common IBM/M$ stuff */
#define THEADR                  0x80    /* Translator Header Record */
#define LHEADR                  0x82    /* Library Module Header Record */

#define COMENT                  0x88    /* Comment Record */
#define CMTCLASS_A0              0xa0
#define CMTA0_IMPDEF               0x01 /* Import Definition Record (Comment Class A0, Subtype 01) */
#define CMTA0_EXPDEF               0x02 /* Export Definition Record (Comment Class A0, Subtype 02) */
#define CMTA0_INCDEF               0x03 /* Incremental Compilation Record (Cmnt Class A0, Sub 03) */
#define CMTA0_LNKDIR               0x05 /* C++ Directives Record (Comment Class A0, Subtype 05) */
#define CMTCLASS_LIBMOD          0xa3   /* Library Module Name Record (Comment Class A3) */
#define CMTCLASS_EXESTR          0xa4   /* Executable String Record (Comment Class A4) */
#define CMTCLASS_INCERR          0xa6   /* Incremental Compilation Error (Comment Class A6) */
#define CMTCLASS_NOPAD           0xa7   /* No Segment Padding (Comment Class A7) */
#define CMTCLASS_WKEXT           0xa8   /* Weak Extern Record (Comment Class A8) */
#define CMTCLASS_LZEXT           0xa9   /* Lazy Extern Record (Comment Class A9) */
#define CMTCLASS_AA              0xaa   /* PharLap Format Record (Comment Class AA) */

#define MODEND                  0x8a    /* Module End Record */
#define MODEND32                0x8b    /* Module End Record */
#define EXTDEF                  0x8c    /* External Names Definition Record */
#define TYPDEF                  0x8e    /* Type Definition Record */
#define PUBDEF                  0x90    /* Public Names Definition Record */
#define PUBDEF32                0x91    /* Public Names Definition Record */
#define LINNUM                  0x94    /* Line Numbers Record */
#define LINNUM32                0x95    /* Line Numbers Record */
#define LNAMES                  0x96    /* List of Names Record */
#define SEGDEF                  0x98    /* Segment Definition Record */
#define SEGDEF32                0x99    /* Segment Definition Record */
#define GRPDEF                  0x9a    /* Group Definition Record */
#define FIXUPP                  0x9c    /* Fixup Record */
#define FIXUPP32                0x9d    /* Fixup Record */
#define LEDATA                  0xa0    /* Logical Enumerated Data Record */
#define LEDATA32                0xa1    /* Logical Enumerated Data Record */
#define LIDATA                  0xa2    /* Logical Iterated Data Record */
#define LIDATA32                0xa3    /* Logical Iterated Data Record */
#define COMDEF                  0xb0    /* Communal Names Definition Record */
#define BAKPAT                  0xb2    /* Backpatch Record */
#define BAKPAT32                0xb3    /* Backpatch Record */
#define LEXTDEF                 0xb4    /* Local External Names Definition Record */
#define LEXTDEF32               0xb5    /* Local External Names Definition Record */
#define LPUBDEF                 0xb6    /* Local Public Names Definition Record */
#define LPUBDEF32               0xb7    /* Local Public Names Definition Record */
#define LCOMDEF                 0xb8    /* Local Communal Names Definition Record */
#define COMDAT                  0xc2    /* Initialized Communal Data Record */
#define COMDAT32                0xc3    /* Initialized Communal Data Record */
#define LINSYM                  0xc4    /* Symbol Line Numbers Record */
#define LINSYM32                0xc5    /* Symbol Line Numbers Record */
#define ALIAS                   0xc6    /* Alias Definition Record */
#define NBKPAT                  0xc8    /* Named Backpatch Record */
#define NBKPAT32                0xc9    /* Named Backpatch Record */
#define LLNAMES                 0xca    /* Local Logical Names Definition Record */

#define COMFIX                  0xba    /* Communal Fixup Record */
#define COMFIX32                0xbb
#define CEXTDEF                 0xbc    /* COMDAT External Names Definition Record */

/* Not implemented (specified by Intel) and Obosolete. */
#define RHEADR                  0x6E    /* R-Module Header Record */
#define REGINT                  0x70    /* Register Initialization Record */
#define REDATA                  0x72    /* Relocatable Enumerated Data Record */
#define RIDATA                  0x74    /* Relocatable Iterated Data Record */
#define OVLDEF                  0x76    /* Overlay Definition Record */
#define ENDREC                  0x78    /* End Record */
#define BLKDEF                  0x7A    /* Block Definition Record */
#define BLKEND                  0x7C    /* Block End Record */
#define DEBSYM                  0x7E    /* Debug Symbols Record */
#define PEDATA                  0x84    /* Physical Enumerated Data Record */
#define PIDATA                  0x86    /* Physical Iterated Data Record */
#define LOCSYM                  0x92    /* Local Symbols Record */
#define RESERVED                0x9E    /* Unnamed record */
#define LIBHED                  0xA4    /* Library Header Record */
#define LIBNAM                  0xA6    /* Library Module Names Record */
#define LIBLOC                  0xA8    /* Library Module Locations Record */
#define LIBDIC                  0xAA    /* Library Dictionary Record */
#define SELDEF                  0xC0    /* Selector Definition Record */


/*
 * Library record types.
 */
#define LIBHDR                  0xF0
#define LIBEND                  0xF1

/* IBM extentions */
#define LIBHDR2                 0xF3


/**
 * Access macro for reading index value.
 * @return  index value. 0 - 32768.
 * @param   pch     Pointer to the first byte of the index.
 */
#define INDEX_VALUE(pch)    ( *(char*)(pch) & 0x80 \
                                ? (*(char *)(pch) & 0x7f) * 0x100 + *((char*)(pch)+1) \
                                : *((char*)(pch)) )

/**
 * Access macro for determin the size of an index.
 * @return  index size in bytes. 1 or 2.
 * @param   pch     Pointer to the first byte of the index.
 */
#define INDEX_SIZE(pch)     ( *(char*)(pch) & 0x80 ? 2 : 1 )


/**
 * Get a byte at a given offset.
 * @return  byte at given offset.
 * @param   pch     Pointer to address relativ to.
 * @param   off     Byte offset from pch.
 */
#define OMF_BYTE(pch, off)  ( *((char*)(pch) + (int)(off)) )


/**
 * Get a word (unsigned short) at a given offset.
 * @return  word (unsigned short) at given offset.
 * @param   pch     Pointer to address relativ to.
 * @param   off     Byte offset from pch.
 */
#define OMF_WORD(pch, off)  ( *(unsigned short*)((char*)(pch) + (int)(off)) )


/**
 * Get a dword (unsigned long) at a given offset.
 * @return  dword (unsigned long) at given offset.
 * @param   pch     Pointer to address relativ to.
 * @param   off     Byte offset from pch.
 */
#define OMF_DWORD(pch, off)  ( *(unsigned long*)((char*)(pch) + (int)(off)) )

#pragma pack(1)
typedef struct _OMFHdr
{
    char            chType;             /* Record type. */
    unsigned short  cch;                /* Size of record. (not including this header.) */
} OMFHDR, *POMFHDR;
#pragma pack()

#endif

