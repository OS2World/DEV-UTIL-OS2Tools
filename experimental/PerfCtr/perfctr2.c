/***********************************************************************
 * Projekt   : PHS Tools
 * Name      : OS/2 Performance Monitor
 * Funktion  : Accesses OS/2's Performance Monitors
 * Autor     : Patrick Haller [Montag, 25.09.1995 01.20.20]
 ***********************************************************************/

 /*
  Status:
  
  @@@PH 1998/05/11 Dos32ConfigurePerf kann PerfView an- und ausschalten.
                   (mit Theseus verifiziert, fPerfview %fff10bc0 geht an
                   und aus.
                   
  @@@PH 1998/05/11 Warning 1 - PerfView keeps running (initialized state)
                               even if client process dies.
                   Warning 2 - PerfView does NOT check for any ownership
                               of performance counters. Therefore, any
                               application can deregister foreign counters,
                               e.g. DISK01 or CPU or whatever. There's no way
                               to re-DosRegisterPerfCtrs() them.
                               Subsequent DosAliasPerfCtrs return pointer to
                               next PerfView-queue element.
                   
  */

/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#define INCL_DOSDEVIOCTL
#define INCL_DOSDEVICES                                 /* DosDevIOCtl */
#define INCL_DOSERRORS                         /* Die Fehlerkonstanten */
#define INCL_DOSMISC                                  /* DosGetMessage */
#define INCL_DOS
#define INCL_NOPMAPI                      /* Kein Presentation Manager */
#define INCL_DOSPROFILE

#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "phstypes.h"
#include "phstools.h"
#include "phsarg.h"

/* this include file is only available in the IBM DDK */
#define FAR
#include "pvwxport.h"

#define MAXPATHLEN 260


#include <conio.h>


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
} OPTIONS, *POPTIONS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */

ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung----------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
  ARG_TERMINATE
};


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void   help            ( void );

int    main            ( int           argc,
                         char          *argv[] );


/***********************************************************************
 * Name      : void help
 * Funktion  : Darstellen der Hilfe
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

void help (void)
{
  TOOLVERSION("PerfCtr",                                   /* application name */
              0x00000001,                            /* application revision */
              0x00010100,           /* minimum required revision of PHSTOOLS */
              0x0001FFFF,        /* maximum recommended revision of PHSTOOLS */
              NULL,                                                /* Remark */
              NULL);                                 /* additional copyright */
}


void PrintBlock(PTBH pTextBlk,
                PDBH pDataBlk)
{
  ULONG ulName;
  PTBN  ptbnName;
  PSZ   pszFlag;
  PUCHAR pucData;
  
  if (pTextBlk == NULL) return;
  if (pDataBlk == NULL) return;
  
  printf ("Textblock: %08xh, groupID=%04xh instanceID=%04xh\n",
          pTextBlk->tbh_ulVersion,
          pTextBlk->tbh_bidID.bid_usGroup,
          pTextBlk->tbh_bidID.bid_usInstance);
  
  printf ("Datablock: Group %04xh, Inst %04xh, Size %u, Flags %u\n",
          pDataBlk->dbh_bidID.bid_usGroup,
          pDataBlk->dbh_bidID.bid_usInstance,
          pDataBlk->dbh_ulTotLen,
          pDataBlk->dbh_flFlags);
  pucData = (PUCHAR)pDataBlk + 12; /* skip size, bid, flags */
  
  pszFlag = malloc(64);
  *pszFlag = 0;
  
  if (pTextBlk->tbh_tbnGroup.tbn_ulFlags & TBN_FL_PERS)
    strcat (pszFlag, "Persistent ");
  
  if (pTextBlk->tbh_tbnGroup.tbn_ulFlags & TBN_FL_TRAN)
    strcat (pszFlag, "Transient ");
  
  if (pTextBlk->tbh_tbnGroup.tbn_ulFlags & TBN_FL_KRNL)
    strcat (pszFlag, "Kernel ");
  
  if (pTextBlk->tbh_tbnGroup.tbn_ulFlags & TBN_FL_MULTIPROC)
    strcat (pszFlag, "Multiple CPUs ");
  
  if (pTextBlk->tbh_tbnGroup.tbn_ulFlags & TBN_FL_ADVANCED)
    strcat (pszFlag, "Advanced ");
  
  if (pTextBlk->tbh_tbnGroup.tbn_ulFlags & TBN_FL_HISTOGRAM)
    strcat (pszFlag, "Histogram ");
  
  printf ("   %s: %s (%08xh) Size %uBytes MsgID=%04xh\n",
          pTextBlk->tbh_tbnGroup.tbn_pszName + (ULONG)pTextBlk,
          pszFlag,
          pTextBlk->tbh_tbnGroup.tbn_ulFlags,
          pTextBlk->tbh_tbnGroup.tbn_usSize,
          pTextBlk->tbh_tbnGroup.tbn_usMsgID);
  
  free(pszFlag);
  
  printf ("   Inst : Flags=%08xh Size=%04xh MsgID=%04xh Name=%s\n",
          pTextBlk->tbh_tbnInstance.tbn_ulFlags,
          pTextBlk->tbh_tbnInstance.tbn_usSize,
          pTextBlk->tbh_tbnInstance.tbn_usMsgID,
          pTextBlk->tbh_tbnInstance.tbn_pszName + (ULONG)pTextBlk);
          
  printf ("   Msg=[%s], Help=[%s], Names=%u\n",
          pTextBlk->tbh_pszMsgFile + (ULONG)pTextBlk,
          pTextBlk->tbh_pszHelpFile + (ULONG)pTextBlk,
          pTextBlk->tbh_culName);
  
  for (ulName = 0;
       ulName < pTextBlk->tbh_culName;
       ulName++)
  {
    ptbnName = (PTBN)( (PSZ)&pTextBlk->tbh_atbnName[ulName] + (ULONG)pTextBlk);
    
    
    switch(ptbnName->tbn_ulFlags & TBN_FLM_CT)
    {
      case TBN_FL_CT_CNT:    pszFlag = "Count";          break;
      case TBN_FL_CT_TIMR:   pszFlag = "Timer";          break;
      case TBN_FL_CT_QLEN:   pszFlag = "Queue length";   break;
      case TBN_FL_CT_SUMSQ:  pszFlag = "Sum of Squares"; break;
      case TBN_FL_CT_BULKCT: pszFlag = "Bulk Counter";   break;
      case TBN_FL_CT_UNKNOWN:pszFlag = "Unknown type";   break;
      case TBN_FL_CT_QCT:    pszFlag = "DeltaCount";     break;
      default:               pszFlag = "<invalid>";      break;
    }
    
    printf ("   %4u: %-20s %-20s %2ub, MsgID %04xh: %08xh\n",
            ulName,
            ptbnName->tbn_pszName + (ULONG)pTextBlk,
            pszFlag,
            ptbnName->tbn_usSize,
            ptbnName->tbn_usMsgID,
            *(PULONG)pucData);
    pucData += ptbnName->tbn_usSize;
  }
  
  /* print values ! */
}


/***********************************************************************
 * Name      : APIRET PerfCtrTest
 * Funktion  :
 * Parameter :
 * Variablen :
 * Ergebnis  : API return code
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 ***********************************************************************/

 /*** Perfview API support */
APIRET APIENTRY  Dos32RegisterPerfCtrs(PBYTE pbDataBlk,
                                       PBYTE pbTextBlk,
                                       ULONG flFlags);

APIRET  APIENTRY      Dos32AliasPerfCtrs(ULONG ulRangeType,
                                         ULONG ulInfo,
                                         PBYTE *ppbRangeStart,
                                         ULONG *pulRangeSize);

APIRET  APIENTRY      Dos32ConfigurePerf(ULONG ulEntityType,
                                         ULONG ulConfigType,
                                         ULONG ulInfo1,
                                         ULONG ulInfo2,
                                         PSZ pszConfigSpec,
                                         BOOL32 fExclude);

APIRET  APIENTRY      Dos32DeconPerf(VOID);


APIRET PerfCtrTest(void)
{
  APIRET rc;
  HEV    hevSem;
  PBYTE  pbRangeStart = 0;
  ULONG  ulRangeSize  = 0;
  ULONG  ulBID;
  
  #define NumTimerCounters  7                                 
  
#pragma pack(1)
  UCHAR    GroupName[] = "DISKa 01";/* Name of OS2DASD    */
  UCHAR    str1[] = "ctRDa";        /* Read Counter       */
  UCHAR    str2[] = "qlRDa";        /* Read Queue Counter */
  UCHAR    str3[] = "blRDa";        /* Read Block Count   */
  UCHAR    str4[] = "ctWRa";        /* Write Counter      */
  UCHAR    str5[] = "qlWRa";        /* Write Queue Counters */
  UCHAR    str6[] = "blWRa";        /* Write Block Count  */
  UCHAR    str7[] = "tmBUSYa";      /* Write Block Count  */
  
typedef struct _VDB {
   DBH          pfdbh;
   CNT          NumReads;               /* Read Counter */
   QLEN         Read;     /*   @5242 */ /* Read Queue   */
   CNT          ReadBytes;              /* Read Byte Count */
   CNT          NumWrites;              /* Write Counter */
   QLEN         Write;   /*   @5242 */  /* Write Queue */
   CNT          WriteBytes;             /* Write Byte Count */
   TIMR         Busy;                   /* BUSY timer */             /*@V53255*/
} VDB;
  
  
  VDB PerfViewDB;
  PDBH pDBH;
  PTBH pTBH;

  TBN      NameBlock[]  =
  {
     {PVW_CT_CNT,  sizeof(CNT),  0 , str1},
     {PVW_CT_QLEN, sizeof(QLEN), 0 , str2},
     {PVW_CT_CNT,  sizeof(CNT),  0 , str3},
     {PVW_CT_CNT,  sizeof(CNT),  0 , str4},
     {PVW_CT_QLEN, sizeof(QLEN), 0 , str5},                   
     {PVW_CT_CNT,  sizeof(CNT),  0 , str6},
     {PVW_CT_TIMR, sizeof(TIMR), 0 , str7}                    
  };
  
  TBH PerfViewTB = 
  {
     TBH_VER_2_0_0_0,             /* Version Number */
     0,0,                         /* Block Instance ID, Block Group ID */
     0,0,0,GroupName,             /* Text Block Group Name */
     0,0,0,0,                     /* Text Block Instance Name */
     0,                           /* Message File Name */
     0,                           /* Help file name */
     NumTimerCounters,            /* Number of Timers + Counters */
     &NameBlock[0],               /* Pointer to array of Name Blocks */
  };

  rc = DosCreateEventSem(NULL,
                         &hevSem,
                         DC_SEM_SHARED,
                         0);
  if (rc)
    printf ("createsem rc = %u %08x ...\n",
            rc,
            rc);


  PerfViewDB.pfdbh.dbh_ulTotLen = sizeof(VDB);
  PerfViewDB.pfdbh.dbh_flFlags  = RPC_FL_32BIT | RPC_FL_APP;
  PerfViewDB.pfdbh.dbh_ulSem    = hevSem;

  printf ("Before Perf-Shutdown\n");
  getch();

  /* to close previous session */
  rc = Dos32ConfigurePerf(0,
                        4,
                        0,
                        0,
                        0,
                        0);
  printf ("Dos32ConfigurePerf(TERMINATE) rc = %u.\n",
          rc);

  printf ("Before Perf-Init Allocate Queue (0x100) \n");
  getch();

  /* initialize subsystem */
  rc = Dos32ConfigurePerf(0x100,
                        0,
                        hevSem,
                        0,
                        0,
                        0);
  
  printf ("Dos32ConfigurePerf(INITIALIZE) rc = %u\n",
          rc);

/*
  printf ("textblock: bid: %08xh group=%04xh inst=%04xh\n",
          PerfViewTB.tbh_bidID,
          PerfViewTB.tbh_bidID.bid_usGroup,
          PerfViewTB.tbh_bidID.bid_usInstance);

  rc = DosRegisterPerfCtrs(&PerfViewDB,
                           &PerfViewTB,
                           RPC_FL_APP | RPC_FL_32BIT);
  printf ("registerperf rc = %u %08x ...\n",
          rc,
          rc);
          
 printf ("textblock: bid: %08xh group=%04xh inst=%04xh\n",
          PerfViewTB.tbh_bidID,
          PerfViewTB.tbh_bidID.bid_usGroup,
          PerfViewTB.tbh_bidID.bid_usInstance);

  printf ("configureperf start rc = %u %08x ...\n",
          rc,
          rc);
*/
          

/*  Dieser alias erlaubt scannen EINER gruppe
  rc = Dos32AliasPerfCtrs(0x0f, 
                          0, 
                          &pbRangeStart,
                          &ulRangeSize);
  printf ("aliasperff init rc = %u %08x ...\n",
          rc,
          rc);

  printf ("pbRangeStart = %08xh, ulRangeSize = %u\n",
          pbRangeStart,
          ulRangeSize);
*/


#define TESTAP(a,b) rc=Dos32AliasPerfCtrs(a,b,&pbRangeStart,&ulRangeSize); \
                    printf ("ap(#%u,#%u) rc=%3u", a,b,rc);                 \
                    printf (" pbRangeStart = %08xh, ulRangeSize = %u\n",    \
                    pbRangeStart,                                          \
                    ulRangeSize);

/*
  TESTAP(0,0);
  TESTAP(1,0);
  TESTAP(2,0);
  TESTAP(3,0);
  TESTAP(4,0);
  TESTAP(5,0);
  TESTAP(6,0);
  TESTAP(7,0);
  TESTAP(8,0);
  TESTAP(9,0);
  TESTAP(10,0);
  TESTAP(11,0);
  TESTAP(12,0);
  TESTAP(13,0);
  TESTAP(14,0);
  TESTAP(15,0);
  TESTAP(16,0);
  TESTAP(17,0);
  TESTAP(18,0);
  TESTAP(19,0);
  TESTAP(20,0);
  TESTAP(21,0);
  TESTAP(22,0);
  TESTAP(23,0);
  TESTAP(24,0);
  TESTAP(25,0);
  TESTAP(26,0);
  TESTAP(27,0);
  TESTAP(28,0);
  TESTAP(29,0);
  TESTAP(30,0);
  TESTAP(31,0);
  TESTAP(32,0);
*/

  pTBH = &PerfViewTB;
  pDBH = &PerfViewDB;


  TESTAP(11,0);

  pTBH = (PTBH)pbRangeStart;

  TESTAP(19,0);
  pDBH = (PDBH)pbRangeStart;

  ulBID = pTBH->tbh_bidID.bid_usInstance | (pTBH->tbh_bidID.bid_usGroup << 16);

  rc = Dos32ConfigurePerf(1, /* allocate queue */
                          1,
                          ulBID,   /* block ID */
                          0,
                          0,
                          0);
    printf ("configureperf (%08xh) start rc = %u %08x ...\n",
         ulBID,
         rc,
         rc);

/*
  pbRangeStart = malloc(4096);
  ulRangeSize  = 4096;
  TESTAP(32,0);
*/


  while( !kbhit() )
  {
    DosSleep(1000);
/*
    ToolsDumpHex( pbRangeStart,
                  ulRangeSize,
                  pbRangeStart);
    printf("\n");
*/
    PrintBlock(pTBH,
               pDBH);

    rc = Dos32ConfigurePerf(1, /* allocate queue */
                            1,
                            ulBID, /* block ID */
                            0,
                            0,
                            0);
    printf ("configureperf start rc = %u %08x ...\n",
            rc,
            rc);
  }

/*
  rc = DosRegisterPerfCtrs(&PerfViewDB,
                           &PerfViewTB,
                           RPC_FL_APP | RPC_FL_32BIT | RPC_FL_DEREG);
  printf ("deregisterperf rc = %u %08x ...\n",
          rc,
          rc);

 printf ("textblock: bid: %08xh group=%04xh inst=%04xh\n",
          PerfViewTB.tbh_bidID,
          PerfViewTB.tbh_bidID.bid_usGroup,
          PerfViewTB.tbh_bidID.bid_usInstance);
*/


  rc = Dos32ConfigurePerf(0,
                        4,
                        0,
                        0,
                        0,
                        0);
  printf ("configureperf term rc = %u %08x ...\n",
          rc,
          rc);


  return rc;
}


/***********************************************************************
 * Name      : void initialize
 * Funktion  : Initialisierung einiger Variablen
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 ***********************************************************************/

void initialize (void)
{
  memset(&Options,
         0L,
         sizeof(Options));
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
  int rc;                                                    /* Rckgabewert */

  initialize ();                                          /* Initialisierung */

  rc = ArgStandard (argc,                            /* CLI-Parameter parsen */
                    argv,
                    TabArguments,
                    &Options.fsHelp);
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    exit(1);                                                /* abort program */
  }

  if ( Options.fsHelp )                                  /* help requested ? */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }


  rc = PerfCtrTest();                                            /* Los geht's */
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  return (rc);
}
