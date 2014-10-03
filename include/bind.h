/* $Id: bind.h,v 1.9 2002/02/07 09:58:08 bird Exp $
 *
 * Bind structures for DB2.
 *
 * Copyright (c) 2001 knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 *
 * GPL
 *
 */



#ifndef _Bind_h_
#define _Bind_h_



/*******************************************************************************
*   Defined Constants And Macros                                               *
*******************************************************************************/
/*
 * Executable bind struct defines.
 */
#define SQLARTIN            "SQLARTIN"

/* Version < 7.0 */
#define BINDPRGID_MAGIC     "oAABA"
#define BINDPRGID_MAGIC1    ('o' | ('A' << 8) | ('A' << 16) | ('B' << 24))
#define BINDPRGID_MAGIC2    'A'
#define BINDPRGID_CCHMAGIC  5

/* Version 7.0+ */
#define BINDPRGID70_MAGIC     "ADAJAH"
#define BINDPRGID70_MAGIC1    ('A' | ('D' << 8) | ('A' << 16) | ('J' << 24))
#define BINDPRGID70_MAGIC2    ('A' | ('H' << 8))
#define BINDPRGID70_CCHMAGIC  6


/*
 * Header defines.
 */
#define ENDIAN_BIG          'B'         /* if not 'B' then default to little endian. */
#define ENDIAN_LITTLE       'L'         /* if not 'B' then default to little endian. */

#define DATETIME_DEFAULT    0x30        /* '0' */
#define DATETIME_USA        0x31        /* '1' */
#define DATETIME_EUR        0x32        /* '2' */
#define DATETIME_ISO        0x33        /* '3' */
#define DATETIME_JIS        0x34        /* '4' */
#define DATETIME_LOCAL      0x35        /* '5' */

#define STANDARD_SAA        0x30        /* '0' */
#define STANDARD_MIA        '?'         /* Don't know the value... */

#define DEFINED             0           /* Used on the _default fields. */
                                        /* (0 if defined; != 0 if default.) */

#define ISOLATION_RR        0
#define ISOLATION_CS        1
#define ISOLATION_UR        2
#define ISOLATION_RS        3
#define ISOLATION_NC        4

#define BLOCKING_UNAMBIG    0
#define BLOCKING_ALL        1
#define BLOCKING_NO         2

#define BINDID_LENGTH       8
#define BINDID_V10          "\xFF\xFBIND\0\0"
#define BINDID_V20          "BIND V02"
#define BINDID_V50          "BINDV500"  /* guesswork */
#define BINDID_V51          "BINDV510"  /* guesswork */
#define BINDID_V52          "BINDV520"
#define BINDID_V60          "BINDV600"  /* guesswork */
#define BINDID_V61          "BINDV610"
#define BINDID_V70          "BINDV700"  /* guesswork */
#define BINDID_V71          "BINDV710"

#define BINDREL_V1          0x0201
#define BINDREL_V12         0x0202
#define BINDREL_V2          0x0300
#define BINDREL_V5          0x0500
#define BINDREL_V6          0x0600
#define BINDREL_V7          0x0700



/*******************************************************************************
*   Structures and Typedefs                                                    *
*******************************************************************************/
#pragma pack(1)
/*
 * Options.
 */
typedef struct _StringOption
{
    char            reserved;
    char            _default;
    unsigned short  length;
    char            value[1];
} BINDOPTSTR, *PBINDOPTSTR;

typedef struct _ULONGOption
{
    char            reserved;
    char            _default;
    unsigned short  length;
    unsigned long   value;
} BINDOPTUL, *PBINDOPTUL;


/*
 * The bind file header.
 */

/* Version 1.0 */
typedef struct _BINDHDRV10
{
    char            bind_id[8];         /* 0x00 0x08  Bind file identifier */
    unsigned short  relno;              /* 0x08 0x02  Bind file release number */
    char            application[8];     /* 0x0a 0x08  Access package name */
    char            timestamp[8];       /* 0x12 0x08  Access package timestamp */
    char            creator[8];         /* 0x1a 0x08  Bind file creator */
    char            reserved1[0x1a];    /* 0x22 0x1a  */
    unsigned long   statements;         /* 0x3c 0x04  Offset of SQL statements */
    unsigned long   declarel;           /* 0x40 0x04  Size of data declarations */
    unsigned long   declare;            /* 0x44 0x04  Offset of data declarations */
} BINDHDRV10, *PBINDHDRV10;



/* Version 2.0 */
typedef struct _BINDHDRV20
{
    char            bind_id[8];         /* 0x00 0x08  Bind file identifier */
    unsigned short  relno;              /* 0x08 0x02  Bind file release number */
    char            application[8];     /* 0x0a 0x08  Access package name */
    char            timestamp[8];       /* 0x12 0x08  Access package timestamp */
    char            creator[8];         /* 0x1a 0x08  Bind file creator */
    char            reserved1[4];       /* 0x22 0x04  */
    char            endian;             /* 0x26 0x01  Bit representation */
    char            sqlda_doubled;      /* 0x27 0x01  Indicates if SQLDA doubled */
    char            insert;             /* 0x28 0x01  DB2/PE buffered inserts */
    char            reserved2[0xb];     /* 0x29 0x0b */
    unsigned short  num_hostvars;       /* 0x34 0x02  Number of host variables */
    unsigned short  max_sect;           /* 0x36 0x02  Highest section number used */
    unsigned long   num_stmt;           /* 0x38 0x04  Number of SQL statements */
    unsigned long   statements;         /* 0x3c 0x04  Offset of SQL statements */
    unsigned long   declarel;           /* 0x40 0x04  Size of data declarations */
    unsigned long   declare;            /* 0x44 0x04  Offset of data declarations */
    char            prep_id[8];         /* 0x48 0x08  Userid that created bindfile */
    char            date_reverved;      /* 0x50 0x01  */
    char            date_default;       /* 0x51 0x01  Date/Time format default? */
    unsigned short  date_length;        /* 0x52 0x02  Date/Time format length */
    unsigned long   date_value;         /* 0x54 0x04  Date/Time format */
    char            stds_reserved;      /* 0x58 0x01  */
    char            stds_default;       /* 0x59 0x01  Standards Compliance Level default? */
    unsigned short  stds_length;        /* 0x5a 0x02  Standards Compliance Level length */
    unsigned long   stds_value;         /* 0x5c 0x04  Standards Compliance Level */
    char            isol_reserved;      /* 0x60 0x01  */
    char            isol_default;       /* 0x61 0x01  Isolation option default? */
    unsigned short  isol_length;        /* 0x62 0x02  Isolation option length */
    unsigned long   isol_value;         /* 0x64 0x04  Isolation option */
    char            blck_reserved;      /* 0x68 0x01  */
    char            blck_default;       /* 0x69 0x01  Record blocking option default? */
    unsigned short  blck_length;        /* 0x6a 0x02  Record blocking option length */
    unsigned long   blck_value;         /* 0x6c 0x04  Record blocking option */
    char            sqler_reserved;     /* 0x70 0x01  */
    char            sqler_default;      /* 0x71 0x01  SQLERROR option default? */
    unsigned short  sqler_length;       /* 0x72 0x02  SQLERROR option length */
    unsigned long   sqler_value;        /* 0x74 0x04  SQLERROR option */
    char            level_reserved;     /* 0x78 0x01  */
    char            level_default;      /* 0x79 0x01  Level option default? */
    unsigned short  level_length;       /* 0x7a 0x02  Level option in length */
    char            level_value[8];     /* 0x7c 0x08  Level option (2 or 3 bytes are used) */
    char            colid_reserved;     /* 0x84 0x01  */
    char            colid_default;      /* 0x85 0x01  Collection ID option default? */
    unsigned short  colid_length;       /* 0x86 0x01  Collection ID option length. */
    char            colid_value[20];    /* 0x88 0x14  Collection ID option */
    char            vrsn_reserved;      /* 0x9c 0x01  */
    char            vrsn_default;       /* 0x9d 0x01  Version option default. */
    unsigned short  vrsn_length;        /* 0x9e 0x02  Version option length. */
    char            vrsn_value[0x100];  /* 0xa0 0x100 Version option */
    char            owner_reserved;     /*0x1a0 0x01  */
    char            owner_default;      /*0x1a1 0x01  Package owner option default. */
    unsigned short  owner_length;       /*0x1a2 0x02  Package owner option length. */
    char            owner_value[8];     /*0x1a4 0x08  Package owner option */
    char            qual_reserved;      /*0x1ac 0x01  */
    char            qual_default;       /*0x1ad 0x01  Default Qualifier option default. */
    unsigned short  qual_length;        /*0x1ae 0x02  Default Qualifier option length. */
    char            qual_value[8];      /*0x1b0 0x08  Default Qualifier option */
    char            text_reserved;      /*0x1b8 0x01  */
    char            text_default;       /*0x1b9 0x01  Text option default. */
    unsigned short  text_length;        /*0x1ba 0x02  Text option length. */
    char            text_value[0x100];  /*0x1bc 0x100 Text option */
    char            vldte_reserved;     /*0x2bc 0x01  */
    char            vldte_default;      /*0x2bd 0x01  Validate option default. */
    unsigned short  vldte_length;       /*0x2be 0x02  Validate option length. */
    unsigned long   vldte_value;        /*0x2c0 0x04  Validate option */
    char            expln_reserved;     /*0x2c4 0x01  */
    char            expln_default;      /*0x2c5 0x01  Explain option default. */
    unsigned short  expln_length;       /*0x2c6 0x02  Explain option length. */
    unsigned long   expln_value;        /*0x2c8 0x04  Explain option */
    char            actn_reserved;      /*0x2cc 0x01  */
    char            actn_default;       /*0x2cd 0x01  Action option default. */
    unsigned short  actn_length;        /*0x2ce 0x02  Action option length. */
    unsigned long   actn_value;         /*0x2d0 0x04  Action option */
    char            rver_reserved;      /*0x2d4 0x01  */
    char            rver_default;       /*0x2d5 0x01  REPLVER option default. */
    unsigned short  rver_length;        /*0x2d6 0x02  REPLVER option length. */
    char            rver_value[0x100];  /*0x2d8 0x100 REPLVER option */
    char            retn_reserved;      /*0x3d8 0x01  */
    char            retn_default;       /*0x3d9 0x01  Retain option default. */
    unsigned short  retn_length;        /*0x3da 0x02  Retain option length. */
    unsigned long   retn_value;         /*0x3dc 0x04  Retain option */
    char            rlse_reserved;      /*0x3e0 0x01  */
    char            rlse_default;       /*0x3e1 0x01  Release option default. */
    unsigned short  rlse_length;        /*0x3e2 0x02  Release option length. */
    unsigned long   rlse_value;         /*0x3e4 0x04  Release option */
    char            dgr_reserved;       /*0x3e8 0x01  */
    char            dgr_default;        /*0x3e9 0x01  Degree of I/O parallelism default. */
    unsigned short  dgr_length;         /*0x3ea 0x02  Degree of I/O parallelism length. */
    unsigned long   dgr_value;          /*0x3ec 0x04  Degree of I/O parallelism */
    char            str_reserved;       /*0x3f0 0x01  */
    char            str_default;        /*0x3f1 0x01  String delimiter option default. */
    unsigned short  str_length;         /*0x3f2 0x02  String delimiter option length. */
    unsigned long   str_value;          /*0x3f4 0x04  String delimiter option */
    char            decd_reserved;      /*0x3f8 0x01  */
    char            decd_default;       /*0x3f9 0x01  Decimal delimiter option default. */
    unsigned short  decd_length;        /*0x3fa 0x02  Decimal delimiter option length. */
    unsigned long   decd_value;         /*0x3fc 0x04  Decimal delimiter option */
    char            csub_reserved;      /*0x400 0x01  */
    char            csub_default;       /*0x401 0x01  Character subtype option default. */
    unsigned short  csub_length;        /*0x402 0x02  Character subtype option length. */
    unsigned long   csub_value;         /*0x404 0x04  Character subtype option */
    char            ccsids_reserved;    /*0x408 0x01  */
    char            ccsids_default;     /*0x409 0x01  Single byte CCSID option default. */
    unsigned short  ccsids_length;      /*0x40a 0x02  Single byte CCSID option length. */
    unsigned long   ccsids_value;       /*0x40c 0x04  Single byte CCSID option */
    char            ccsidm_reserved;    /*0x410 0x01  */
    char            ccsidm_default;     /*0x411 0x01  Mixed byte CCSID option default. */
    unsigned short  ccsidm_length;      /*0x412 0x02  Mixed byte CCSID option length. */
    unsigned long   ccsidm_value;       /*0x414 0x04  Mixed byte CCSID option */
    char            ccsidg_reserved;    /*0x418 0x01  */
    char            ccsidg_default;     /*0x419 0x01  Double byte CCSID option default. */
    unsigned short  ccsidg_length;      /*0x41a 0x02  Double byte CCSID option length. */
    unsigned long   ccsidg_value;       /*0x41c 0x04  Double byte CCSID option */
    char            decprc_reserved;    /*0x420 0x01  */
    char            decprc_default;     /*0x421 0x01  Decimal precision option default. */
    unsigned short  decprc_length;      /*0x422 0x02  Decimal precision option length. */
    unsigned long   decprc_value;       /*0x424 0x04  Decimal precision option */
    char            dynrul_reserved;    /*0x428 0x01  */
    char            dynrul_default;     /*0x429 0x01  Dynamic rules option default. */
    unsigned short  dynrul_length;      /*0x42a 0x02  Dynamic rules option length. */
    unsigned long   dynrul_value;       /*0x42c 0x04  Dynamic rules option */
    char            insert_reserved;    /*0x430 0x01  */
    char            insert_default;     /*0x431 0x01  DB2/PE buffered inserts default. */
    unsigned short  insert_length;      /*0x432 0x02  DB2/PE buffered inserts length. */
    unsigned long   insert_value;       /*0x434 0x04  DB2/PE buffered inserts */
    char            explsnap_reserved;  /*0x438 0x01  */
    char            explsnap_default;   /*0x439 0x01  Explain snapshot default. */
    unsigned short  explsnap_length;    /*0x43a 0x02  Explain snapshot length. */
    unsigned long   explsnap_value;     /*0x43c 0x04  Explain snapshot */
    char            funcpath_reserved;  /*0x440 0x01  */
    char            funcpath_default;   /*0x441 0x01  UDF function path default. */
    unsigned short  funcpath_length;    /*0x442 0x02  UDF function path length. */
    char            funcpath_value[256];/*0x444 0x100 UDF function path */
    char            sqlwarn_reserved;   /*0x543 0x01  */
    char            sqlwarn_default;    /*0x545 0x01  SQL warnings default. */
    unsigned short  sqlwarn_length;     /*0x546 0x02  SQL warnings length. */
    unsigned long   sqlwarn_value;      /*0x548 0x04  SQL warnings */
    char            queryopt_reserved;  /*0x54c 0x01  */
    char            queryopt_default;   /*0x54d 0x01  Query optimization default. */
    unsigned short  queryopt_length;    /*0x54e 0x02  Query optimization length. */
    unsigned long   queryopt_value;     /*0x550 0x04  Query optimization */

    #if 0
    char            _reserved;          /*0x4 0x01  */
    char            _default;           /*0x4 0x01   default. */
    unsigned short  _length;            /*0x4 0x02   length. */
    unsigned long   _value;             /*0x4 0x04   */

    char            reserved16;         /*0x2 0x01  */
    char            _default;           /*0x2 0x01   default. */
    unsigned short  _length;            /*0x2 0x02   length. */
    char            _value[20];         /*0x2 0x20   */
    #endif

} BINDHDRV20, *PBINDHDRV20;


/* Version 5.2 */
typedef struct _BINDHDRV52
{
    char            bind_id[8];         /* 0x00 0x08  Bind file identifier */
    unsigned short  relno;              /* 0x08 0x02  Bind file release number */
    char            application[8];     /* 0x0a 0x08  Access package name */
    char            timestamp[8];       /* 0x12 0x08  Access package timestamp */
    char            creator[8];         /* 0x1a 0x08  Bind file creator */
    char            reserved1[4];       /* 0x22 0x04  */
    char            endian;             /* 0x26 0x01  Bit representation */
    char            sqlda_doubled;      /* 0x27 0x01  Indicates if SQLDA doubled */
    char            insert;             /* 0x28 0x01  DB2/PE buffered inserts */
    char            reserved2[0xb];     /* 0x29 0x0b */
    unsigned short  num_hostvars;       /* 0x34 0x02  Number of host variables */
    unsigned short  max_sect;           /* 0x36 0x02  Highest section number used */
    unsigned long   num_stmt;           /* 0x38 0x04  Number of SQL statements */
    unsigned long   statements;         /* 0x3c 0x04  Offset of SQL statements */
    unsigned long   declarel;           /* 0x40 0x04  Size of data declarations */
    unsigned long   declare;            /* 0x44 0x04  Offset of data declarations */
    char            prep_id[8];         /* 0x48 0x08  Userid that created bindfile */
    char            date_reverved;      /* 0x50 0x01  */
    char            date_default;       /* 0x51 0x01  Date/Time format default? */
    unsigned short  date_length;        /* 0x52 0x02  Date/Time format length */
    unsigned long   date_value;         /* 0x54 0x04  Date/Time format */
    char            stds_reserved;      /* 0x58 0x01  */
    char            stds_default;       /* 0x59 0x01  Standards Compliance Level default? */
    unsigned short  stds_length;        /* 0x5a 0x02  Standards Compliance Level length */
    unsigned long   stds_value;         /* 0x5c 0x04  Standards Compliance Level */
    char            isol_reserved;      /* 0x60 0x01  */
    char            isol_default;       /* 0x61 0x01  Isolation option default? */
    unsigned short  isol_length;        /* 0x62 0x02  Isolation option length */
    unsigned long   isol_value;         /* 0x64 0x04  Isolation option */
    char            blck_reserved;      /* 0x68 0x01  */
    char            blck_default;       /* 0x69 0x01  Record blocking option default? */
    unsigned short  blck_length;        /* 0x6a 0x02  Record blocking option length */
    unsigned long   blck_value;         /* 0x6c 0x04  Record blocking option */
    char            sqler_reserved;     /* 0x70 0x01  */
    char            sqler_default;      /* 0x71 0x01  SQLERROR option default? */
    unsigned short  sqler_length;       /* 0x72 0x02  SQLERROR option length */
    unsigned long   sqler_value;        /* 0x74 0x04  SQLERROR option */
    char            level_reserved;     /* 0x78 0x01  */
    char            level_default;      /* 0x79 0x01  Level option default? */
    unsigned short  level_length;       /* 0x7a 0x02  Level option in length */
    char            level_value[8];     /* 0x7c 0x08  Level option (2 or 3 bytes are used) */
    char            colid_reserved;     /* 0x84 0x01  */
    char            colid_default;      /* 0x85 0x01  Collection ID option default? */
    unsigned short  colid_length;       /* 0x86 0x01  Collection ID option length. */
    char            colid_value[20];    /* 0x88 0x14  Collection ID option */
    char            vrsn_reserved;      /* 0x9c 0x01  */
    char            vrsn_default;       /* 0x9d 0x01  Version option default. */
    unsigned short  vrsn_length;        /* 0x9e 0x02  Version option length. */
    char            vrsn_value[0x100];  /* 0xa0 0x100 Version option */
    char            owner_reserved;     /*0x1a0 0x01  */
    char            owner_default;      /*0x1a1 0x01  Package owner option default. */
    unsigned short  owner_length;       /*0x1a2 0x02  Package owner option length. */
    char            owner_value[8];     /*0x1a4 0x08  Package owner option */
    /* start - this isn't dumpped on 5.2.. */
    char            qual_old_reserved;  /*0x1ac 0x01  */
    char            qual_old_default;   /*0x1ad 0x01  Default Qualifier option default. */
    unsigned short  qual_old_length;    /*0x1ae 0x02  Default Qualifier option length. */
    char            qual_old_value[8];  /*0x1b0 0x08  Default Qualifier option */
    /* end - this isn't dumpped on 5.2.. */
    char            text_reserved;      /*0x1b8 0x01  */
    char            text_default;       /*0x1b9 0x01  Text option default. */
    unsigned short  text_length;        /*0x1ba 0x02  Text option length. */
    char            text_value[0x100];  /*0x1bc 0x100 Text option */
    char            vldte_reserved;     /*0x2bc 0x01  */
    char            vldte_default;      /*0x2bd 0x01  Validate option default. */
    unsigned short  vldte_length;       /*0x2be 0x02  Validate option length. */
    unsigned long   vldte_value;        /*0x2c0 0x04  Validate option */
    char            expln_reserved;     /*0x2c4 0x01  */
    char            expln_default;      /*0x2c5 0x01  Explain option default. */
    unsigned short  expln_length;       /*0x2c6 0x02  Explain option length. */
    unsigned long   expln_value;        /*0x2c8 0x04  Explain option */
    char            actn_reserved;      /*0x2cc 0x01  */
    char            actn_default;       /*0x2cd 0x01  Action option default. */
    unsigned short  actn_length;        /*0x2ce 0x02  Action option length. */
    unsigned long   actn_value;         /*0x2d0 0x04  Action option */
    char            rver_reserved;      /*0x2d4 0x01  */
    char            rver_default;       /*0x2d5 0x01  REPLVER option default. */
    unsigned short  rver_length;        /*0x2d6 0x02  REPLVER option length. */
    char            rver_value[0x100];  /*0x2d8 0x100 REPLVER option */
    char            retn_reserved;      /*0x3d8 0x01  */
    char            retn_default;       /*0x3d9 0x01  Retain option default. */
    unsigned short  retn_length;        /*0x3da 0x02  Retain option length. */
    unsigned long   retn_value;         /*0x3dc 0x04  Retain option */
    char            rlse_reserved;      /*0x3e0 0x01  */
    char            rlse_default;       /*0x3e1 0x01  Release option default. */
    unsigned short  rlse_length;        /*0x3e2 0x02  Release option length. */
    unsigned long   rlse_value;         /*0x3e4 0x04  Release option */
    char            dgr_reserved;       /*0x3e8 0x01  */
    char            dgr_default;        /*0x3e9 0x01  Degree of I/O parallelism default. */
    unsigned short  dgr_length;         /*0x3ea 0x02  Degree of I/O parallelism length. */
    unsigned long   dgr_value;          /*0x3ec 0x04  Degree of I/O parallelism */
    char            str_reserved;       /*0x3f0 0x01  */
    char            str_default;        /*0x3f1 0x01  String delimiter option default. */
    unsigned short  str_length;         /*0x3f2 0x02  String delimiter option length. */
    unsigned long   str_value;          /*0x3f4 0x04  String delimiter option */
    char            decd_reserved;      /*0x3f8 0x01  */
    char            decd_default;       /*0x3f9 0x01  Decimal delimiter option default. */
    unsigned short  decd_length;        /*0x3fa 0x02  Decimal delimiter option length. */
    unsigned long   decd_value;         /*0x3fc 0x04  Decimal delimiter option */
    char            csub_reserved;      /*0x400 0x01  */
    char            csub_default;       /*0x401 0x01  Character subtype option default. */
    unsigned short  csub_length;        /*0x402 0x02  Character subtype option length. */
    unsigned long   csub_value;         /*0x404 0x04  Character subtype option */
    char            ccsids_reserved;    /*0x408 0x01  */
    char            ccsids_default;     /*0x409 0x01  Single byte CCSID option default. */
    unsigned short  ccsids_length;      /*0x40a 0x02  Single byte CCSID option length. */
    unsigned long   ccsids_value;       /*0x40c 0x04  Single byte CCSID option */
    char            ccsidm_reserved;    /*0x410 0x01  */
    char            ccsidm_default;     /*0x411 0x01  Mixed byte CCSID option default. */
    unsigned short  ccsidm_length;      /*0x412 0x02  Mixed byte CCSID option length. */
    unsigned long   ccsidm_value;       /*0x414 0x04  Mixed byte CCSID option */
    char            ccsidg_reserved;    /*0x418 0x01  */
    char            ccsidg_default;     /*0x419 0x01  Double byte CCSID option default. */
    unsigned short  ccsidg_length;      /*0x41a 0x02  Double byte CCSID option length. */
    unsigned long   ccsidg_value;       /*0x41c 0x04  Double byte CCSID option */
    char            decprc_reserved;    /*0x420 0x01  */
    char            decprc_default;     /*0x421 0x01  Decimal precision option default. */
    unsigned short  decprc_length;      /*0x422 0x02  Decimal precision option length. */
    unsigned long   decprc_value;       /*0x424 0x04  Decimal precision option */
    char            dynrul_reserved;    /*0x428 0x01  */
    char            dynrul_default;     /*0x429 0x01  Dynamic rules option default. */
    unsigned short  dynrul_length;      /*0x42a 0x02  Dynamic rules option length. */
    unsigned long   dynrul_value;       /*0x42c 0x04  Dynamic rules option */
    char            insert_reserved;    /*0x430 0x01  */
    char            insert_default;     /*0x431 0x01  DB2/PE buffered inserts default. */
    unsigned short  insert_length;      /*0x432 0x02  DB2/PE buffered inserts length. */
    unsigned long   insert_value;       /*0x434 0x04  DB2/PE buffered inserts */
    char            explsnap_reserved;  /*0x438 0x01  */
    char            explsnap_default;   /*0x439 0x01  Explain snapshot default. */
    unsigned short  explsnap_length;    /*0x43a 0x02  Explain snapshot length. */
    unsigned long   explsnap_value;     /*0x43c 0x04  Explain snapshot */
    char            funcpath_reserved;  /*0x440 0x01  */
    char            funcpath_default;   /*0x441 0x01  UDF function path default. */
    unsigned short  funcpath_length;    /*0x442 0x02  UDF function path length. */
    char            funcpath_value[256];/*0x444 0x100 UDF function path */
    char            sqlwarn_reserved;   /*0x543 0x01  */
    char            sqlwarn_default;    /*0x545 0x01  SQL warnings default. */
    unsigned short  sqlwarn_length;     /*0x546 0x02  SQL warnings length. */
    unsigned long   sqlwarn_value;      /*0x548 0x04  SQL warnings */
    char            queryopt_reserved;  /*0x54c 0x01  */
    char            queryopt_default;   /*0x54d 0x01  Query optimization default. */
    unsigned short  queryopt_length;    /*0x54e 0x02  Query optimization length. */
    unsigned long   queryopt_value;     /*0x550 0x04  Query optimization */
    char            cnulreqd_reserved;  /*0x554 0x01  */
    char            cnulreqd_default;   /*0x555 0x01  C Null Required Option default. */
    unsigned short  cnulreqd_length;    /*0x556 0x02  C Null Required Option length. */
    unsigned long   cnulreqd_value;     /*0x558 0x04  C Null Required Option */
    char            generic_reserved;   /*0x55c 0x01  */
    char            generic_default;    /*0x55d 0x01  Generic Option default. */
    unsigned short  generic_length;     /*0x55e 0x02  Generic Option length. */
    char            generic_value[1024];/*0x560 0x400 Generic Option */
    char            qual_reserved;      /*0x960 0x01  */
    char            qual_default;       /*0x961 0x01  Default Qualifier option default. */
    unsigned short  qual_length;        /*0x962 0x02  Default Qualifier option length. */
    char            qual_value[20];     /*0x964 0x??  Default Qualifier option */
} BINDHDRV52, *PBINDHDRV52;


/* Version 6.1 */
typedef struct _BINDHDRV61
{
    char            bind_id[8];         /* 0x00 0x08  Bind file identifier */
    unsigned long   headerl;            /* 0x08 0x04  Bind file header length */
    unsigned short  relno;              /* 0x0c 0x02  Bind file release number */
    char            application[8];     /* 0x0e 0x08  Access package name */
    char            timestamp[8];       /* 0x16 0x08  Access package timestamp */
    char            creator[8];         /* 0x1e 0x08  Bind file creator */
    char            reserved1[124];     /* 0x26 0x7c */
    char            endian;             /* 0xa2 0x01  Bit representation */
    char            sqlda_doubled;      /* 0xa3 0x01  Indicates if SQLDA doubled */
    char            insert;             /* 0xa4 0x01  DB2/PE buffered inserts */
    char            reserved2[7];       /* 0xa5 0x07 */
    unsigned short  num_hostvars;       /* 0xac 0x02  Number of host variables */
    unsigned short  max_sect;           /* 0xae 0x02  Highest section number used */
    unsigned long   num_stmt;           /* 0xb0 0x04  Number of SQL statements */
    unsigned long   statements;         /* 0xb4 0x04  Offset of SQL statements */
    unsigned long   declarel;           /* 0xb8 0x04  Size of data declarations */
    unsigned long   declare;            /* 0xbc 0x04  Offset of data declarations */
    char            prep_id[8];         /* 0xc0 0x08  Userid that created bindfile */
    char            reserved4[120];     /* 0xc8 0x78 */
    char            date_reverved;      /*0x140 0x01  */
    char            date_default;       /*0x141 0x01  Date/Time format default? */
    unsigned short  date_length;        /*0x142 0x02  Date/Time format length */
    unsigned long   date_value;         /*0x144 0x04  Date/Time format */
    char            stds_reserved;      /*0x148 0x01  */
    char            stds_default;       /*0x149 0x01  Standards Compliance Level default? */
    unsigned short  stds_length;        /*0x14a 0x02  Standards Compliance Level length */
    unsigned long   stds_value;         /*0x14c 0x04  Standards Compliance Level */
    char            isol_reserved;      /*0x150 0x01  */
    char            isol_default;       /*0x151 0x01  Isolation option default? */
    unsigned short  isol_length;        /*0x152 0x02  Isolation option length */
    unsigned long   isol_value;         /*0x154 0x04  Isolation option */
    char            blck_reserved;      /*0x158 0x01  */
    char            blck_default;       /*0x159 0x01  Record blocking option default? */
    unsigned short  blck_length;        /*0x15a 0x02  Record blocking option length */
    unsigned long   blck_value;         /*0x15c 0x04  Record blocking option */
    char            sqler_reserved;     /*0x160 0x01  */
    char            sqler_default;      /*0x161 0x01  SQLERROR option default? */
    unsigned short  sqler_length;       /*0x162 0x02  SQLERROR option length */
    unsigned long   sqler_value;        /*0x164 0x04  SQLERROR option */
    char            level_reserved;     /*0x168 0x01  */
    char            level_default;      /*0x169 0x01  Level option default? */
    unsigned short  level_length;       /*0x16a 0x02  Level option in length */
    char            level_value[8];     /*0x16c 0x08  Level option (2 or 3 bytes are used) */
    char            colid_reserved;     /*0x174 0x01  */
    char            colid_default;      /*0x175 0x01  Collection ID option default? */
    unsigned short  colid_length;       /*0x176 0x01  Collection ID option length. */
    char            colid_value[128];   /*0x178 0x80? Collection ID option */
    char            vrsn_reserved;      /*0x1f8 0x01  */
    char            vrsn_default;       /*0x1f9 0x01  Version option default. */
    unsigned short  vrsn_length;        /*0x1fa 0x02  Version option length. */
    char            vrsn_value[0x100];  /*0x1fc 0x100 Version option */
    char            owner_reserved;     /*0x2fc 0x01  */
    char            owner_default;      /*0x2fd 0x01  Package owner option default. */
    unsigned short  owner_length;       /*0x2fe 0x02  Package owner option length. */
    char            owner_value[128];   /*0x300 0x80  Package owner option */
    char            qual_reserved;      /*0x380 0x01  */
    char            qual_default;       /*0x381 0x01  Default Qualifier option default. */
    unsigned short  qual_length;        /*0x382 0x02  Default Qualifier option length. */
    char            qual_value[128];    /*0x384 0x80  Default Qualifier option */
    char            text_reserved;      /*0x404 0x01  */
    char            text_default;       /*0x405 0x01  Text option default. */
    unsigned short  text_length;        /*0x406 0x02  Text option length. */
    char            text_value[0x100];  /*0x408 0x100 Text option */
    char            vldte_reserved;     /*0x508 0x01  */
    char            vldte_default;      /*0x509 0x01  Validate option default. */
    unsigned short  vldte_length;       /*0x50a 0x02  Validate option length. */
    unsigned long   vldte_value;        /*0x50c 0x04  Validate option */
    char            expln_reserved;     /*0x510 0x01  */
    char            expln_default;      /*0x511 0x01  Explain option default. */
    unsigned short  expln_length;       /*0x512 0x02  Explain option length. */
    unsigned long   expln_value;        /*0x514 0x04  Explain option */
    char            actn_reserved;      /*0x518 0x01  */
    char            actn_default;       /*0x519 0x01  Action option default. */
    unsigned short  actn_length;        /*0x51a 0x02  Action option length. */
    unsigned long   actn_value;         /*0x51c 0x04  Action option */
    char            rver_reserved;      /*0x520 0x01  */
    char            rver_default;       /*0x521 0x01  REPLVER option default. */
    unsigned short  rver_length;        /*0x522 0x02  REPLVER option length. */
    char            rver_value[0x100];  /*0x524 0x100 REPLVER option */
    char            retn_reserved;      /*0x624 0x01  */
    char            retn_default;       /*0x625 0x01  Retain option default. */
    unsigned short  retn_length;        /*0x626 0x02  Retain option length. */
    unsigned long   retn_value;         /*0x628 0x04  Retain option */
    char            rlse_reserved;      /*0x62c 0x01  */
    char            rlse_default;       /*0x62d 0x01  Release option default. */
    unsigned short  rlse_length;        /*0x62e 0x02  Release option length. */
    unsigned long   rlse_value;         /*0x630 0x04  Release option */
    char            dgr_reserved;       /*0x634 0x01  */
    char            dgr_default;        /*0x635 0x01  Degree of I/O parallelism default. */
    unsigned short  dgr_length;         /*0x636 0x02  Degree of I/O parallelism length. */
    unsigned long   dgr_value;          /*0x638 0x04  Degree of I/O parallelism */
    char            str_reserved;       /*0x63c 0x01  */
    char            str_default;        /*0x63d 0x01  String delimiter option default. */
    unsigned short  str_length;         /*0x63e 0x02  String delimiter option length. */
    unsigned long   str_value;          /*0x640 0x04  String delimiter option */
    char            decd_reserved;      /*0x644 0x01  */
    char            decd_default;       /*0x645 0x01  Decimal delimiter option default. */
    unsigned short  decd_length;        /*0x646 0x02  Decimal delimiter option length. */
    unsigned long   decd_value;         /*0x648 0x04  Decimal delimiter option */
    char            csub_reserved;      /*0x64c 0x01  */
    char            csub_default;       /*0x64d 0x01  Character subtype option default. */
    unsigned short  csub_length;        /*0x64e 0x02  Character subtype option length. */
    unsigned long   csub_value;         /*0x650 0x04  Character subtype option */
    char            ccsids_reserved;    /*0x654 0x01  */
    char            ccsids_default;     /*0x655 0x01  Single byte CCSID option default. */
    unsigned short  ccsids_length;      /*0x656 0x02  Single byte CCSID option length. */
    unsigned long   ccsids_value;       /*0x658 0x04  Single byte CCSID option */
    char            ccsidm_reserved;    /*0x65c 0x01  */
    char            ccsidm_default;     /*0x65d 0x01  Mixed byte CCSID option default. */
    unsigned short  ccsidm_length;      /*0x65e 0x02  Mixed byte CCSID option length. */
    unsigned long   ccsidm_value;       /*0x660 0x04  Mixed byte CCSID option */
    char            ccsidg_reserved;    /*0x664 0x01  */
    char            ccsidg_default;     /*0x665 0x01  Double byte CCSID option default. */
    unsigned short  ccsidg_length;      /*0x666 0x02  Double byte CCSID option length. */
    unsigned long   ccsidg_value;       /*0x668 0x04  Double byte CCSID option */
    char            decprc_reserved;    /*0x66c 0x01  */
    char            decprc_default;     /*0x66d 0x01  Decimal precision option default. */
    unsigned short  decprc_length;      /*0x66e 0x02  Decimal precision option length. */
    unsigned long   decprc_value;       /*0x670 0x04  Decimal precision option */
    char            dynrul_reserved;    /*0x674 0x01  */
    char            dynrul_default;     /*0x675 0x01  Dynamic rules option default. */
    unsigned short  dynrul_length;      /*0x676 0x02  Dynamic rules option length. */
    unsigned long   dynrul_value;       /*0x678 0x04  Dynamic rules option */
    char            insert_reserved;    /*0x67c 0x01  */
    char            insert_default;     /*0x67d 0x01  DB2/PE buffered inserts default. */
    unsigned short  insert_length;      /*0x67e 0x02  DB2/PE buffered inserts length. */
    unsigned long   insert_value;       /*0x680 0x04  DB2/PE buffered inserts */
    char            explsnap_reserved;  /*0x684 0x01  */
    char            explsnap_default;   /*0x685 0x01  Explain snapshot default. */
    unsigned short  explsnap_length;    /*0x686 0x02  Explain snapshot length. */
    unsigned long   explsnap_value;     /*0x688 0x04  Explain snapshot */
    char            funcpath_reserved;  /*0x68c 0x01  */
    char            funcpath_default;   /*0x68d 0x01  UDF function path default. */
    unsigned short  funcpath_length;    /*0x68e 0x02  UDF function path length. */
    char            funcpath_value[256];/*0x690 0x100 UDF function path */
    char            sqlwarn_reserved;   /*0x790 0x01  */
    char            sqlwarn_default;    /*0x791 0x01  SQL warnings default. */
    unsigned short  sqlwarn_length;     /*0x792 0x02  SQL warnings length. */
    unsigned long   sqlwarn_value;      /*0x794 0x04  SQL warnings */
    char            queryopt_reserved;  /*0x798 0x01  */
    char            queryopt_default;   /*0x799 0x01  Query optimization default. */
    unsigned short  queryopt_length;    /*0x79a 0x02  Query optimization length. */
    unsigned long   queryopt_value;     /*0x79c 0x04  Query optimization */
    char            cnulreqd_reserved;  /*0x7a0 0x01  */
    char            cnulreqd_default;   /*0x7a1 0x01  C Null Required Option default. */
    unsigned short  cnulreqd_length;    /*0x7a2 0x02  C Null Required Option length. */
    unsigned long   cnulreqd_value;     /*0x7a4 0x04  C Null Required Option */
    char            generic_reserved;   /*0x7a8 0x01  */
    char            generic_default;    /*0x7a9 0x01  Generic Option default. */
    unsigned short  generic_length;     /*0x7aa 0x02  Generic Option length. */
    char            generic_value[1024];/*0x7ac 0x400 Generic Option */
    char            defprep_reserved;   /*0xbac 0x01  */
    char            defprep_default;    /*0xbad 0x01  Deferred prepare option default. */
    unsigned short  defprep_length;     /*0xbae 0x02  Deferred prepare option length. */
    unsigned long   defprep_value;      /*0xbb0 0x04  Deferred prepare option */
    char            trfgrp_reserved;    /*0xbb4 0x01  */
    char            trfgrp_default;     /*0xbb5 0x01  Transform group option default. */
    unsigned short  trfgrp_length;      /*0xbb6 0x02  Transform group option length. */
    char            trfgrp_value[1024]; /*0xbb8 0x400 Transform group option */

} BINDHDRV61, *PBINDHDRV61;


/* Version 7.1 */
typedef struct _BINDHDRV71
{
    char            bind_id[8];         /* 0x00 0x08  Bind file identifier */
    unsigned long   headerl;            /* 0x08 0x04  Bind file header length */
    unsigned short  relno;              /* 0x0c 0x02  Bind file release number */
    char            application[8];     /* 0x0e 0x08  Access package name */
    char            timestamp[8];       /* 0x16 0x08  Access package timestamp */
    char            creator[32];        /* 0x1e 0x20? Bind file creator */
    char            reserved1[10];      /* 0x3e 0x0a  */
    char            prep_id[32];        /* 0x48 0x20? Userid that created bindfile */
    char            reserved2[58];      /* 0x68 0x3a  */
    char            endian;             /* 0xa2 0x01  Bit representation */
    char            sqlda_doubled;      /* 0xa3 0x01  Indicates if SQLDA doubled */
    char            insert;             /* 0xa4 0x01  DB2/PE buffered inserts */
    char            reserved3[7];       /* 0xa5 0x07 */
    unsigned short  num_hostvars;       /* 0xac 0x02  Number of host variables */
    unsigned short  max_sect;           /* 0xae 0x02  Highest section number used */
    unsigned long   num_stmt;           /* 0xb0 0x04  Number of SQL statements */
    unsigned long   statements;         /* 0xb4 0x04  Offset of SQL statements */
    unsigned long   declarel;           /* 0xb8 0x04  Size of data declarations */
    unsigned long   declare;            /* 0xbc 0x04  Offset of data declarations */
    char            reserved4[128];     /* 0xc0 0x80 */
    char            date_reverved;      /*0x140 0x01  */
    char            date_default;       /*0x141 0x01  Date/Time format default? */
    unsigned short  date_length;        /*0x142 0x02  Date/Time format length */
    unsigned long   date_value;         /*0x144 0x04  Date/Time format */
    char            stds_reserved;      /*0x148 0x01  */
    char            stds_default;       /*0x149 0x01  Standards Compliance Level default? */
    unsigned short  stds_length;        /*0x14a 0x02  Standards Compliance Level length */
    unsigned long   stds_value;         /*0x14c 0x04  Standards Compliance Level */
    char            isol_reserved;      /*0x150 0x01  */
    char            isol_default;       /*0x151 0x01  Isolation option default? */
    unsigned short  isol_length;        /*0x152 0x02  Isolation option length */
    unsigned long   isol_value;         /*0x154 0x04  Isolation option */
    char            blck_reserved;      /*0x158 0x01  */
    char            blck_default;       /*0x159 0x01  Record blocking option default? */
    unsigned short  blck_length;        /*0x15a 0x02  Record blocking option length */
    unsigned long   blck_value;         /*0x15c 0x04  Record blocking option */
    char            sqler_reserved;     /*0x160 0x01  */
    char            sqler_default;      /*0x161 0x01  SQLERROR option default? */
    unsigned short  sqler_length;       /*0x162 0x02  SQLERROR option length */
    unsigned long   sqler_value;        /*0x164 0x04  SQLERROR option */
    char            level_reserved;     /*0x168 0x01  */
    char            level_default;      /*0x169 0x01  Level option default? */
    unsigned short  level_length;       /*0x16a 0x02  Level option in length */
    char            level_value[8];     /*0x16c 0x08  Level option (2 or 3 bytes are used) */
    char            colid_reserved;     /*0x174 0x01  */
    char            colid_default;      /*0x175 0x01  Collection ID option default? */
    unsigned short  colid_length;       /*0x176 0x01  Collection ID option length. */
    char            colid_value[128];   /*0x178 0x80? Collection ID option */
    char            vrsn_reserved;      /*0x1f8 0x01  */
    char            vrsn_default;       /*0x1f9 0x01  Version option default. */
    unsigned short  vrsn_length;        /*0x1fa 0x02  Version option length. */
    char            vrsn_value[0x100];  /*0x1fc 0x100 Version option */
    char            owner_reserved;     /*0x2fc 0x01  */
    char            owner_default;      /*0x2fd 0x01  Package owner option default. */
    unsigned short  owner_length;       /*0x2fe 0x02  Package owner option length. */
    char            owner_value[128];   /*0x300 0x80  Package owner option */
    char            qual_reserved;      /*0x380 0x01  */
    char            qual_default;       /*0x381 0x01  Default Qualifier option default. */
    unsigned short  qual_length;        /*0x382 0x02  Default Qualifier option length. */
    char            qual_value[128];    /*0x384 0x80  Default Qualifier option */
    char            text_reserved;      /*0x404 0x01  */
    char            text_default;       /*0x405 0x01  Text option default. */
    unsigned short  text_length;        /*0x406 0x02  Text option length. */
    char            text_value[0x100];  /*0x408 0x100 Text option */
    char            vldte_reserved;     /*0x508 0x01  */
    char            vldte_default;      /*0x509 0x01  Validate option default. */
    unsigned short  vldte_length;       /*0x50a 0x02  Validate option length. */
    unsigned long   vldte_value;        /*0x50c 0x04  Validate option */
    char            expln_reserved;     /*0x510 0x01  */
    char            expln_default;      /*0x511 0x01  Explain option default. */
    unsigned short  expln_length;       /*0x512 0x02  Explain option length. */
    unsigned long   expln_value;        /*0x514 0x04  Explain option */
    char            actn_reserved;      /*0x518 0x01  */
    char            actn_default;       /*0x519 0x01  Action option default. */
    unsigned short  actn_length;        /*0x51a 0x02  Action option length. */
    unsigned long   actn_value;         /*0x51c 0x04  Action option */
    char            rver_reserved;      /*0x520 0x01  */
    char            rver_default;       /*0x521 0x01  REPLVER option default. */
    unsigned short  rver_length;        /*0x522 0x02  REPLVER option length. */
    char            rver_value[0x100];  /*0x524 0x100 REPLVER option */
    char            retn_reserved;      /*0x624 0x01  */
    char            retn_default;       /*0x625 0x01  Retain option default. */
    unsigned short  retn_length;        /*0x626 0x02  Retain option length. */
    unsigned long   retn_value;         /*0x628 0x04  Retain option */
    char            rlse_reserved;      /*0x62c 0x01  */
    char            rlse_default;       /*0x62d 0x01  Release option default. */
    unsigned short  rlse_length;        /*0x62e 0x02  Release option length. */
    unsigned long   rlse_value;         /*0x630 0x04  Release option */
    char            dgr_reserved;       /*0x634 0x01  */
    char            dgr_default;        /*0x635 0x01  Degree of I/O parallelism default. */
    unsigned short  dgr_length;         /*0x636 0x02  Degree of I/O parallelism length. */
    unsigned long   dgr_value;          /*0x638 0x04  Degree of I/O parallelism */
    char            str_reserved;       /*0x63c 0x01  */
    char            str_default;        /*0x63d 0x01  String delimiter option default. */
    unsigned short  str_length;         /*0x63e 0x02  String delimiter option length. */
    unsigned long   str_value;          /*0x640 0x04  String delimiter option */
    char            decd_reserved;      /*0x644 0x01  */
    char            decd_default;       /*0x645 0x01  Decimal delimiter option default. */
    unsigned short  decd_length;        /*0x646 0x02  Decimal delimiter option length. */
    unsigned long   decd_value;         /*0x648 0x04  Decimal delimiter option */
    char            csub_reserved;      /*0x64c 0x01  */
    char            csub_default;       /*0x64d 0x01  Character subtype option default. */
    unsigned short  csub_length;        /*0x64e 0x02  Character subtype option length. */
    unsigned long   csub_value;         /*0x650 0x04  Character subtype option */
    char            ccsids_reserved;    /*0x654 0x01  */
    char            ccsids_default;     /*0x655 0x01  Single byte CCSID option default. */
    unsigned short  ccsids_length;      /*0x656 0x02  Single byte CCSID option length. */
    unsigned long   ccsids_value;       /*0x658 0x04  Single byte CCSID option */
    char            ccsidm_reserved;    /*0x65c 0x01  */
    char            ccsidm_default;     /*0x65d 0x01  Mixed byte CCSID option default. */
    unsigned short  ccsidm_length;      /*0x65e 0x02  Mixed byte CCSID option length. */
    unsigned long   ccsidm_value;       /*0x660 0x04  Mixed byte CCSID option */
    char            ccsidg_reserved;    /*0x664 0x01  */
    char            ccsidg_default;     /*0x665 0x01  Double byte CCSID option default. */
    unsigned short  ccsidg_length;      /*0x666 0x02  Double byte CCSID option length. */
    unsigned long   ccsidg_value;       /*0x668 0x04  Double byte CCSID option */
    char            decprc_reserved;    /*0x66c 0x01  */
    char            decprc_default;     /*0x66d 0x01  Decimal precision option default. */
    unsigned short  decprc_length;      /*0x66e 0x02  Decimal precision option length. */
    unsigned long   decprc_value;       /*0x670 0x04  Decimal precision option */
    char            dynrul_reserved;    /*0x674 0x01  */
    char            dynrul_default;     /*0x675 0x01  Dynamic rules option default. */
    unsigned short  dynrul_length;      /*0x676 0x02  Dynamic rules option length. */
    unsigned long   dynrul_value;       /*0x678 0x04  Dynamic rules option */
    char            insert_reserved;    /*0x67c 0x01  */
    char            insert_default;     /*0x67d 0x01  DB2/PE buffered inserts default. */
    unsigned short  insert_length;      /*0x67e 0x02  DB2/PE buffered inserts length. */
    unsigned long   insert_value;       /*0x680 0x04  DB2/PE buffered inserts */
    char            explsnap_reserved;  /*0x684 0x01  */
    char            explsnap_default;   /*0x685 0x01  Explain snapshot default. */
    unsigned short  explsnap_length;    /*0x686 0x02  Explain snapshot length. */
    unsigned long   explsnap_value;     /*0x688 0x04  Explain snapshot */
    char            funcpath_reserved;  /*0x68c 0x01  */
    char            funcpath_default;   /*0x68d 0x01  UDF function path default. */
    unsigned short  funcpath_length;    /*0x68e 0x02  UDF function path length. */
    char            funcpath_value[256];/*0x690 0x100 UDF function path */
    char            sqlwarn_reserved;   /*0x790 0x01  */
    char            sqlwarn_default;    /*0x791 0x01  SQL warnings default. */
    unsigned short  sqlwarn_length;     /*0x792 0x02  SQL warnings length. */
    unsigned long   sqlwarn_value;      /*0x794 0x04  SQL warnings */
    char            queryopt_reserved;  /*0x798 0x01  */
    char            queryopt_default;   /*0x799 0x01  Query optimization default. */
    unsigned short  queryopt_length;    /*0x79a 0x02  Query optimization length. */
    unsigned long   queryopt_value;     /*0x79c 0x04  Query optimization */
    char            cnulreqd_reserved;  /*0x7a0 0x01  */
    char            cnulreqd_default;   /*0x7a1 0x01  C Null Required Option default. */
    unsigned short  cnulreqd_length;    /*0x7a2 0x02  C Null Required Option length. */
    unsigned long   cnulreqd_value;     /*0x7a4 0x04  C Null Required Option */
    char            generic_reserved;   /*0x7a8 0x01  */
    char            generic_default;    /*0x7a9 0x01  Generic Option default. */
    unsigned short  generic_length;     /*0x7aa 0x02  Generic Option length. */
    char            generic_value[1024];/*0x7ac 0x400 Generic Option */
    char            defprep_reserved;   /*0xbac 0x01  */
    char            defprep_default;    /*0xbad 0x01  Deferred prepare option default. */
    unsigned short  defprep_length;     /*0xbae 0x02  Deferred prepare option length. */
    unsigned long   defprep_value;      /*0xbb0 0x04  Deferred prepare option */
    char            trfgrp_reserved;    /*0xbb4 0x01  */
    char            trfgrp_default;     /*0xbb5 0x01  Transform group option default. */
    unsigned short  trfgrp_length;      /*0xbb6 0x02  Transform group option length. */
    char            trfgrp_value[1024]; /*0xbb8 0x400 Transform group option */
    char            federated_reserved; /*0xfb8 0x01  */
    char            federated_default;  /*0xfb9 0x01  Federated server option default. */
    unsigned short  federated_length;   /*0xfba 0x02  Federated server option length. */
    unsigned long   federated_value;    /*0xfbc 0x04  Federated server option */

} BINDHDRV71, *PBINDHDRV71;



/*
 * Executable bind data.
 */

/* Version 2.0-6.1 */
typedef struct _BindProgId
{
    unsigned short  length;             /* 0x00 0x02  Length of the struct? (encoded?) */
    unsigned short  rp_rel_num;         /* 0x02 0x02  Release number of this struct - 1.0 (encoded = 'AB')  */
    unsigned short  db_rel_num;         /* 0x04 0x02  Database release number (encoded) */
    unsigned short  bf_rel_num;         /* 0x06 0x02  Bind file release number (encoded) */
    char            sqluser[8];         /* 0x08 0x08  (creator) userid. */
    char            planname[8];        /* 0x10 0x08  Access package name.  */
    char            contoken[8];        /* 0x18 0x08  Access package timestamp. */
    char            buffer[8];          /* 0x20 0x08  Usually the text "011112  ". */
    char            sqlartin[8];        /* 0x28 0x08  Usually the text "SQLARTIN". */
} BINDPROGID, *PBINDPROGID;


/* Version 7.0 */
typedef struct _BindProgId_V70
{
    unsigned short  length;             /* 0x00 0x02  Length of the struct? (encoded?) */
    unsigned short  rp_rel_num;         /* 0x02 0x02  Release number of this struct - 2.0? (encoded = 'AC') */
    unsigned short  db_rel_num;         /* 0x04 0x02  Database release number (encoded) */
    unsigned short  bf_rel_num;         /* 0x06 0x02  Bind file release number (encoded) */
    unsigned short  sqluser_len;        /* 0x08 0x08  (creator) userid. */
    char            sqluser[128];       /* 0x08 0x80  (creator) userid. */
    char            planname[8];        /* 0x90 0x08  Access package name.  */
    char            contoken[8];        /* 0x98 0x08  Access package timestamp. */
    char            buffer[8];          /* 0x30 0x08  Usually the text "011112  ". */
    #if 0
    char            sqlartin[8];        /* 0x38 0x08  Usually the text "SQLARTIN". */
    #endif
} BINDPROGIDV70, *PBINDPROGIDV70;

/* Version 7.1 */
typedef struct _BindProgId_V71
{
    unsigned short  length;             /* 0x00 0x02  Length of the struct. */
    unsigned short  rp_rel_num;         /* 0x02 0x02  Release number of this struct - 3.0 (encoded = 'AD') */
    unsigned short  db_rel_num;         /* 0x04 0x02  Database release number (encoded) */
    unsigned short  bf_rel_num;         /* 0x06 0x02  Bind file release number (encoded) */
    char            planname[8];        /* 0x08 0x08  Access package name.  */
    char            contoken[8];        /* 0x10 0x08  Access package timestamp. */
    char            buffer[8];          /* 0x18 0x08  Usually the text "011112  ". */
    unsigned short  sqluser_len;        /* 0x20 0x02  (creator) userid. */
    char            sqluser[128];       /* 0x22 0x08  (creator) userid. */
    #if 0
    char            sqlartin[8];        /* 0x?? 0x08  Usually the text "SQLARTIN". */
    #endif
} BINDPROGIDV71, *PBINDPROGIDV71;

/* Version 7.1 - MicroFocus 'compessed' edition. */
typedef struct _BindProgId_V71MF
{
    unsigned char   high_length;        /* 0xff 0x01  ?? high byte of the length. */
    unsigned char   length;             /* 0x00 0x01  Length of the struct */
    unsigned char   mflength;           /* 0x00 0x01  Length - 1 of the following string ( = 5) */
    unsigned short  rp_rel_num;         /* 0x02 0x02  Release number of this struct - 3.0 (encoded = 'AD') */
    unsigned short  db_rel_num;         /* 0x04 0x02  Database release number (encoded) */
    unsigned short  bf_rel_num;         /* 0x06 0x02  Bind file release number (encoded) */
    char            chSomething;        /* 0x08 0x01  0x17 */
    char            planname[8];        /* 0x09 0x08  Access package name.  */
    char            contoken[8];        /* 0x11 0x08  Access package timestamp. */
    char            buffer[8];          /* 0x19 0x08  Usually the text "011112  " */
    unsigned short  stuff;              /* 0x21 0x02  0x0000 */
    unsigned char   high_sqluser_len;   /* 0x23 0x01  high byte of sqluser_len? (= 0) */
    unsigned char   sqluser_len;        /* 0x24 0x01  userid (low byte?) */
    unsigned char   mfsqluser;          /* 0x25 0x01  length - 1 of the following string ( = 7 usually) */
    char            sqluser[128];       /* 0x26 0x08  (creator) userid. - it's not this long though 128 is reserved.. */
} BINDPROGIDV71MF, *PBINDPROGIDV71MF;

/* Version 7.1 - MicroFocus 'compessed' type 2 edition - sqluser part. */
typedef struct _BindProgId_V71MF_0_sqluser
{
    unsigned char   achMovEsi[5];       /* 0xbe ?? ?? ?? ?? (4x?? = &rp_rel_num) */
    unsigned char   achMovEdi[5];       /* 0xbf ?? ?? ?? ?? (4x?? = target address) */
    unsigned char   achMovsw[2];        /* 0x66 0xa5 */
    unsigned char   achMovCl7[2];       /* 0xb1 0x07 */
    unsigned char   achRepMovsd[2];     /* 0xf3 0xa5 */
    unsigned char   achMovSqlUserLen[7];/* 0x66 0xc7 0x5 ?? ?? ?? ?? (4x?? = target address) */
    unsigned short  sqluser_len;        /* 0x08 */
    unsigned char   achMovSqlUser1[6];  /* 0xc7 0x5 ?? ?? ?? ?? (4x?? = target address) */
    char            sqluser1[4];        /* 'USER' or whatever. */
    unsigned char   achMovSqlUser2[3];  /* 0xc7 0x47 0x6  */
    char            sqluser2[4];        /* 'ID  ' or whatever. */
} BINDPROGIDV71MFCODE0, *PBINDPROGIDV71MFCODE0;


typedef struct _BindProgId_V71MF_sqluser
{
    unsigned char   achMovEsi[5];       /* 0xbe ?? ?? ?? ?? (4x?? = &rp_rel_num) */
    unsigned char   achMovEdi[5];       /* 0xbf ?? ?? ?? ?? (4x?? = target address) */
    unsigned char   achMovAxEsi[3];     /* 0x66 0x8b 0x6 */
    unsigned char   achMovEdiAx[3];     /* 0x66 0x89 0x7 */
    unsigned char   achAddEsi2[3];      /* 0x83 0xc6 0x2 */
    unsigned char   achAddEdi2[3];      /* 0x83 0xc7 0x2 */
    unsigned char   achMovCl7[2];       /* 0xb1 0x07 */
    unsigned char   achRepMovsd[2];     /* 0xf3 0xa5 */
    unsigned char   achMovSqlUserLen[7];/* 0x66 0xc7 0x5 ?? ?? ?? ?? (4x?? = target address) */
    unsigned short  sqluser_len;        /* 0x08 */
    unsigned char   achMovSqlUser1[6];  /* 0xc7 0x5 ?? ?? ?? ?? (4x?? = target address) */
    char            sqluser1[4];        /* 'USER' or whatever. */
    unsigned char   achMovSqlUser2[3];  /* 0xc7 0x47 0x6  */
    char            sqluser2[4];        /* 'ID  ' or whatever. */
} BINDPROGIDV71MFCODE1, *PBINDPROGIDV71MFCODE1;

typedef union _BindProgId_V71MF_1_sqluser
{
   BINDPROGIDV71MFCODE0 code0;
   BINDPROGIDV71MFCODE1 code1;
} BINDPROGIDV71MFCODE, *PBINDPROGIDV71MFCODE;

#pragma pack()

#endif
