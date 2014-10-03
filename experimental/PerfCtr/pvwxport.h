/*static char *SCCSID = "@(#)pvwxport.h 6.2 92/02/05";*/
/*      SCCSID = "@(#)pvwxport.h        6.2 92/02/05"       */

/****************************** Module Header ******************************\
*
* Module Name: PVWXPORT.H
*
* OS/2 Perfview (PVW) Definitions File
*
* Copyright (c) International Business Machines Corporation 1981, 1988-1992
*
* ===========================================================================
*
*  This include file contains the definitions and constants for the
*  registration of a performance counter group. This header file should be
*  included when DosRegisterPerfCtrs(), dh_RegisterPerfCtrs(),
*  fsh_RegisterPerfCtrs() are needed.
*
*
\***************************************************************************/

/* Miscellaneous constants */

/* Types */

/*** Quad-Word (qw) definitions */

struct qw_s {
        ULONG   qw_ulLo;
        ULONG   qw_ulHi;
};

typedef struct  qw_s QWRD;      /* qw  */
typedef QWRD    FAR *PQWRD;     /* pqw */

#define QW_MAX_HI       0x7fffffff
#define QW_MAX_LO       0xffffffff
#define QW_MIN_HI       0x80000000
#define QW_MIN_LO       0x00000000

typedef ULONG CNT;              /* ct                   */
typedef struct  qw_s TIMR;      /* tm                   */
typedef struct  qw_s SUMSQ;     /* ss -- Sum of Squares */
typedef struct  qw_s BULKCNT;   /* bc -- Bulk Count     */
typedef ULONG QCT;              /* qct that can go up or down */

typedef CNT     FAR * PCNT;     /* pct                  */
typedef TIMR    FAR * PTIMR;    /* ptm                  */
typedef SUMSQ   FAR * PSUMSQ;   /* pss                  */
typedef BULKCNT FAR * PBULKCNT; /* pbc                  */
typedef QCT     FAR * PQCT;     /* pqct                 */


/*** Queue Length (ql) definitions */

struct qlen_s {
        TIMR    ql_tm;
        CNT     ql_ct;
};

typedef struct  qlen_s qlen_t;  /* ql  */
typedef struct  qlen_s QLEN;
typedef QLEN    FAR *  PQLEN;   /* pql */

/* This definition satisfies MASM happy when it sees the results of H2INC; it
 * provides a convenient way to define a pointer size.
 */

struct ul_s {                   /* ul */
        ULONG   ul_ul;
};

/**************************************************************************/

/* RegisterPerfCtrs (RPC) definitions
 *
 *      For DosRegisterPerfCtrs(), dh_RegisterPerfCtrs(),
 *      fsh_RegisterPerfCtrs(), vdh_RegisterPerfCtrs().
 */

/*      FLAGS   RegisterPerfCtrs Flags -- valid values for 'flFlags' of Data
 *              block are:
 *
 *              {RPC_FL_16BIT | RPC_FL_32BIT} |
 *              {RPC_FL_PERS | RPC_FL_TRAN} |
 *              {RPC_FL_KRNL | RPC_FL_DD | RPC_FL_FSD | RPC_FL_VDD | RPC_FL_APP} |
 *              RPC_FL_TMR_INIT |
 *              RPC_FL_DEREG
 */

#define RPC_FL_16BIT      0x0000        /* 16-bit interface             */
#define RPC_FL_32BIT      0x0001        /* 32-bit interface             */

#define RPC_FL_PERS       0x0002        /* Persistent entity type       */
#define RPC_FL_TRAN       0x0004        /* Transient entity type        */

#define RPC_FL_KRNL       0x0008
#define RPC_FL_DD         0x0010        /* Device driver entity type    */
#define RPC_FL_FSD        0x0020        /* FSD entity type              */
#define RPC_FL_VDD        0x0040        /* VDD entity type              */
#define RPC_FL_APP        0x0080        /* Application entity type      */

#define RPC_FL_TMR_INIT   0x0100        /* Init Tmr function ptrs at
                                         * registration time.
                                         */

#define RPC_FL_DEREG      0x0200        /* Deregister entity            */

/*      The following flags are reserved for private use by the kernel; they
 *      are cleared at registration time.
 */

#define RPC_FL_CONFIG     0x0400        /* Configured                   */

/*              Flag Masks
 */

#define RPC_FLM_KRNL_PERS (RPC_FL_KRNL | RPC_FL_PERS)
#define RPC_FLM_KRNL_TRAN (RPC_FL_KRNL | RPC_FL_TRAN)

#define RPC_FLM_DRV (RPC_FL_DD | RPC_FL_FSD | RPC_FL_VDD) /* Driver entities */

#define RPC_FLM_PRIVATE (RPC_FL_CONFIG) /* Private flags for kernel use */

#define RPC_FLM_ET (RPC_FL_KRNL | RPC_FL_DD | RPC_FL_FSD | RPC_FL_VDD | RPC_FL_APP) /* Entity Type */



/*      Perfview Block IDentifier (bid)
 */

struct pvbid_s {
        USHORT  bid_usInstance; /* Block Instance ID                    */
        USHORT  bid_usGroup;    /* Block Group ID                       */
};

typedef struct pvbid_s  pvbid_t;        /* Block ID                     */
typedef struct pvbid_s  BID;

typedef pvbid_t FAR *   ppvbid_t;       /* Ptr to Block ID              */
typedef pvbid_t FAR *   PBID;

#define RPC_MAX_ID      0xffff          /* Maximum ID must fit in USHORT */



/*      PerfView Text Block Name (tbn)
 *
 *      FLAGS   Valid values for 'tbn_ulFlags' are:
 *
 *              for Counter Names:
 *
 *              {TBN_FL_CT_UNKNOWN | TBN_FL_CT_CNT | TBN_FL_CT_TIMR |
 *              TBN_FL_CT_QLEN | TBN_FL_CT_SUMSQ | TBN_FL_BULKCT | TBN_FL_CT_QCT}
 *              TBN_FL_CT_QCT
 *              for Group and Instance Names:
 *
 *              {TBN_FL_PERS | TBN_FL_TRAN} |
 *              {TBN_FL_KRNL} |
 *              {TBN_FL_MULTIPROC}
 *
 *      NOTES   The structure members take on slightly different meanings
 *              when they are used to describe a Transient Entity Instance.
 *              In this case, the tbh_atbnName member of the Text Block
 *              Header (TBH) serves as a pointer to an array of instance
 *              descriptor blocks which have the same form as pvtbName_s, but
 *              rather than describing counters in a separate Data Block,
 *              they are self-describing:  tbn_pszName describes the contents
 *              of tbn_ulFlags, tbn_usMsgID would retain its usual meaning,
 *              and tbn_usSize would be unused.
 *
 *              For example, if tbn_pszName = "PID", then tbn_ulFlags will
 *              contain the pid of the process which created this instance.
 *              Typical descriptors for kernel transient instances would be
 *              "PID", "TID", and "SLOT #".  By convention, the first TBN
 *              block of the tbh_atbnName array will contain the name of the
 *              creating process in tbn_pszName; in this case, tbn_ulFlags
 *              will be unused.
 */

struct pvtbName_s {
        ULONG   tbn_ulFlags;    /* Flags, see TBN_FL_***                */
        USHORT  tbn_usSize;     /* Counter Size                         */
        USHORT  tbn_usMsgID;    /* Message ID                           */
        PSZ     tbn_pszName;    /* Name (default message)               */
};

typedef struct pvtbName_s       pvtbName_t;  /* Text Block Name         */
typedef struct pvtbName_s       TBN;

typedef pvtbName_t FAR *        ppvtbName_t; /* Ptr to Text Block Name  */
typedef pvtbName_t FAR *        PTBN;        /* Ptr to Text Block Name  */


#define _PVWNAME                pvtbName_s
typedef struct pvtbName_s       PVWNAME;


/*      Flags values (tbn_ulFlags)
 */

#define TBN_FLM_CT          0x000F /* Counter Flag Mask (values 0 -> 0xF)*/
#define TBN_FL_CT_CNT       0x0000 /*   Count                            */
#define TBN_FL_CT_TIMR      0x0001 /*   Timer                            */
#define TBN_FL_CT_QLEN      0x0002 /*   Queue length                     */
#define TBN_FL_CT_SUMSQ     0x0003 /*   Sum of Squares                   */
#define TBN_FL_CT_BULKCT    0x0004 /*   Bulk Counter                     */
#define TBN_FL_CT_UNKNOWN   0x0005 /*   Unknown type                     */
#define TBN_FL_CT_QCT       0x0006 /*   Count that increases or decreases */

                                   /* These are used in the tbh_tbnGroup and
                                    * tbh_tbnInstance Name blocks (see pvtbh_s).
                                    */
#define TBN_FL_PERS         0x0010 /* Persistent Entity                  */
#define TBN_FL_TRAN         0x0020 /* Transient Entity                   */
#define TBN_FL_KRNL         0x0040 /* Kernel Entity                      */
#define TBN_FL_MULTIPROC    0x0080 /* Multiple Processor Entity          */

                                   /* General purpose (per counter) flags*/
#define TBN_FL_ADVANCED     0x0100 /* Advanced property                  */
#define TBN_FL_HISTOGRAM    0x0200 /* Start/End histogram counters       */

#define TBN_FLM_UNKNOWN     0xF000 /* For Unknown counter types
                                    * (TBN_FL_CT_UNKNOWN), these flags are
                                    * reserved for use by the Registrant.
                                    */


/*      PerfView Text Block Header (tbh)
 *
 *      NOTES   Values for the version number, tbh_ulVersion, are defined
 *              below (TBH_VER_...).
 *
 *              See NOTES for Perfview Text Block Name (tbn) concerning
 *              the tbh_atbnName member.
 */

struct pvtbh_s {
        ULONG   tbh_ulVersion;  /* Version number (MUST BE FIRST MEMBER)   */
        BID     tbh_bidID;      /* Block IDentifier                        */
        TBN     tbh_tbnGroup;   /* Text block Group name                   */
        TBN     tbh_tbnInstance; /* Text block Instance name               */
        PSZ     tbh_pszMsgFile; /* Message File name                       */
        PSZ     tbh_pszHelpFile; /* Help File name                         */
        ULONG   tbh_culName;    /* Number of counters Names                */
        PTBN    tbh_atbnName;   /* Pointer to array of counter Name blocks */
};

typedef struct pvtbh_s  pvtbh_t;        /* Text Block Header            */
typedef struct pvtbh_s  TBH;

#define _PVWFSDTEXTH    pvtbh_s         /* for File System Driver (FSD) */
typedef struct pvtbh_s  PVWFSDTEXTH;

typedef pvtbh_t FAR *   ppvtbh_t;       /* Pointer to Text Block Header */
typedef pvtbh_t FAR *   PTBH;

#define NUM_TBH_PSZ     4               /* Number of ASCIIZ ptrs in TBH */


/*      Version number values (tbh_ulVersion)
 *
 *      Format: A.B.C.D where   A = Major version #
 *                              B = Major release #
 *                              C = Minor release #
 *                              D = Development revision #
 */

#define TBH_VER_2_0_0_0 0x02000000              /* 2.0.0.0 */

/* XLATOFF */
#define TBH_VER_CURRENT TBH_VER_2_0_0_0         /* Currently supported  */
/* XLATON */



/*      PerfView Data Block Header (dbh)
 *
 *      FLAGS   Valid values for 'dbh_ulFlags' are:
 *
 *              {RPC_FL_16BIT | RPC_FL_32BIT} |
 *              {RPC_FL_PERS | RPC_FL_TRAN} |
 *              {RPC_FL_KRNL | RPC_FL_DD | RPC_FL_FSD | RPC_FL_VDD | RPC_FL_APP} |
 *
 *      NOTES   If modifying pvdbh_s, be sure to change both the 'C' AND
 *              MASM definitions below.  They are separated because h2inc.exe
 *              doesn't handle the pfnTmr declaration conversion properly.
 *
 *              Semaphore:  Kernel counters and Drivers use 'dbh_ulSem' as a
 *              "data modified" flag (increment each time block modified).
 *              Applications use 'dbh_ulSem' as a semaphore handle
 *              (semaphore is created by the Application).
 */

/* XLATOFF */ /* pfnTmr members are treated as INT rather than PFN by h2inc */
struct pvdbh_s {
        ULONG   dbh_ulTotLen;   /* Total Length of data block (including hdr)*/
        BID     dbh_bidID;      /* Block IDentifier                          */
        ULONG   dbh_flFlags;    /* Flags                                     */
        ULONG   dbh_ulSem;      /* Semaphore or Semaphore handle             */
        INT     (FAR *dbh_pfnTmrAdd)(PTIMR); /* Ptr to timer Addition function */
        INT     (FAR *dbh_pfnTmrSub)(PTIMR); /* Ptr to timer Subtract function */
};
/* XLATON */

/*ASM
pvdbh_s STRUC
dbh_ulTotLen    DD      ?       ;  Total Length of data block (including hdr)
dbh_bidID       DB SIZE BID DUP (?) ;  IDentifier (Block ID)
dbh_flFlags     DD      ?       ;  Flags
dbh_ulSem       DD      ?       ;  Semaphore or Semaphore handle
dbh_pfnTmrAdd   DD      ?       ;  Ptr to timer Addition function
dbh_pfnTmrSub   DD      ?       ;  Ptr to timer Subtract function
pvdbh_s ENDS
*/

typedef struct pvdbh_s  pvdbh_t;        /* Data Block Header            */
typedef struct pvdbh_s  DBH;

#define _PVWFSDDATAH    pvdbh_s         /* for File System Driver (FSD) */
typedef struct pvdbh_s  PVWFSDDATAH;

typedef pvdbh_t FAR *   ppvdbh_t;       /* Pointer to Data Block Header */
typedef pvdbh_t FAR *   PDBH;


/***************************************************************************/

/*      Counter Types (CT) (************ -- USE TBN_FL_... ***)
 */

#define PVW_CT_UNKNOWN    TBN_FL_CT_UNKNOWN
#define PVW_CT_CNT        TBN_FL_CT_CNT
#define PVW_CT_TIMR       TBN_FL_CT_TIMR
#define PVW_CT_QLEN       TBN_FL_CT_QLEN
#define PVW_CT_SUMSQ      TBN_FL_CT_SUMSQ
#define PVW_CT_BULKCT     TBN_FL_CT_BULKCT
#define PVW_CT_QCT        TBN_FL_CT_QCT
