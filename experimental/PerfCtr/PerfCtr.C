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


/*** Perfview API support */
APIRET APIENTRY Dos32RegisterPerfCtrs(PBYTE pbDataBlk,
                                      PBYTE pbTextBlk,
                                      ULONG flFlags);

APIRET APIENTRY Dos32AliasPerfCtrs   (ULONG  ulRangeType,
                                      ULONG  ulInfo,
                                      PBYTE  *ppbRangeStart,
                                      ULONG  *pulRangeSize);

#define DAPCRT_SCAN             0x00000001
#define DAPCRT_SCAN_BACKWARD    0x00000002
#define DAPCRT_SCAN_FORWARD     0x00000004
#define DAPCRT_SELECT_TEXTBLOCK 0x00000008
#define DAPCRT_SELECT_DATABLOCK 0x00000010
#define DAPCRT_SNAPSHOT         0x00000020

/*
 * ulRangeType:
 * 0x00000000 not allowed
 * 0x00000001 mandatory - either 0x01 or 0x20
 * 0x00000002 ? stars from head of list
 * 0x00000004 ? iterates over the linked list within the queue
 * 0x00000008 either 0x08 or 0x10
 * 0x00000010   provide different offset into the block structure
 *
 * 0x00000020 - checks target buffer size (size in ulInfo)
 *              if insufficient buffer size, pulRangeSize returns minimum size
 *            - creates a snapshot of the current PerfView-Queue
 *            - no other flags allowed
 *            - returns the whole queue of registered performance counters
 *              with textblock headers and data block headers - no data though!
 * 0x0000000B get (last) textblock header -> 0x08 / 0x10 select either
 * 0x00000013 get (last) datablock header    datablock or textblock
 */


APIRET APIENTRY Dos32ConfigurePerf   (ULONG  ulEntityType,
                                      ULONG  ulConfigType,
                                      ULONG  ulInfo1,
                                      ULONG  ulInfo2,
                                      PSZ    pszConfigSpec,
                                      BOOL32 fExclude);

APIRET APIENTRY Dos32DeconPerf       (VOID);


#define DCP_CONFIG_INITIALIZE  0
#define DCP_CONFIG_COLLECT     1
#define DCP_CONFIG_CALIBRATE   3
#define DCP_CONFIG_TERMINATE   4

/* ulEntityType:
 * DCP_CONFIG_INIT (ulConfigType = 0):
 *   ulEntityType
 *   0x00000020 arm hooks, init persistent and transient kernel entities
 *   0x00000100 allocate perfview queue
 *              ulInfo1 = event semaphore handle
 *
 * DCP_COLLECT_SAMPLE: (ulConfigType = 1)
 *   ulEntityType  ulInfo1
 *   0x00000001 !  PvwRPDescriptor (->pvwFindRPDesc)
 *
 * DCP_CALIBRATE_TIMERS (ulConfigType = 3)
 *   ulEntityType = 0 !
 *
 * DCP_CONFIG_TERM: (ulConfigType = 4)
 *   ulEntityType = 0 !
 */



/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsPerfViewOff;                  /* turn off the perfview subsystem */
} OPTIONS, *POPTIONS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */

ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung----------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/OFF",       "Turn off PerfView kernel hooks.", NULL,    ARG_NULL,       &Options.fsPerfViewOff},
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
              0x00000002,                            /* application revision */
              0x00010100,           /* minimum required revision of PHSTOOLS */
              0x0001FFFF,        /* maximum recommended revision of PHSTOOLS */
              NULL,                                                /* Remark */
              NULL);                                 /* additional copyright */
}


/***********************************************************************
 * Name      : void PrintBlock
 * Funktion  : Prints text- and datablock
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/


void PrintBlock(PTBH  pTextBlk,
                PDBH  pDataBlk,
                BOOL  fDataValid)
{
  ULONG  ulName;
  PTBN   ptbnName;
  PSZ    pszFlag;
  PUCHAR pucData;
  ULONG  ulData;
  
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
  
  pucData = (PUCHAR)pDataBlk + sizeof(DBH); /* skip size, bid, flags */
  
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
    
    if (fDataValid)
      ulData = *(PULONG)pucData;
    else
      ulData = 0xFFFFFFFF;
    
    printf ("   %4u: %-20s %-20s %2ub, MsgID %04xh: %08xh\n",
            ulName,
            ptbnName->tbn_pszName + (ULONG)pTextBlk,
            pszFlag,
            ptbnName->tbn_usSize,
            ptbnName->tbn_usMsgID,
            ulData);
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


APIRET PerfCtrTest(void)
{
  APIRET rc;
  HEV    hevSem;
  PBYTE  pbRangeStart = 0;
  PBYTE  pbData;
  ULONG  ulRangeSize  = 0;
  ULONG  ulBID;
  PTBH   pTBH;
  PDBH   pDBH;
  
  /* create event semaphore */
  rc = DosCreateEventSem(NULL,
                         &hevSem,
                         DC_SEM_SHARED,
                         0);
  if (rc)
    printf ("createsem rc = %u %08x ...\n",
            rc,
            rc);


  /* to close previous session */
  rc = Dos32ConfigurePerf(0, 4, 0, 0, 0, 0);
  printf ("Dos32ConfigurePerf(TERMINATE) rc = %u.\n",
          rc);
  
  /* note: it is important to allocate the queue first
           and launch the collector threads afterwards */
  
  /* initialize subsystem */
  rc = Dos32ConfigurePerf(0x100,
                        0,
                        hevSem,
                        0,
                        0,
                        0);
  
  printf ("Dos32ConfigurePerf(INITIALIZE, ALLOC QUEUE) rc = %u\n",
          rc);

  /* initialize subsystem */
  rc = Dos32ConfigurePerf(0x020,
                        0,
                        hevSem,
                        0,
                        0,
                        0);
  
  printf ("Dos32ConfigurePerf(INITIALIZE, START THDS) rc = %u\n",
          rc);


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
                    printf ("ap(#%08x,#%08x) rc=%3u", a,b,rc);                 \
                    printf (" pbRangeStart = %08xh, ulRangeSize = %u\n",    \
                    pbRangeStart,                                          \
                    ulRangeSize);

/*
  TESTAP(0,0);   TESTAP(1,0);    TESTAP(2,0);   TESTAP(3,0);
  TESTAP(4,0);   TESTAP(5,0);    TESTAP(6,0);   TESTAP(7,0);
  TESTAP(8,0);   TESTAP(9,0);    TESTAP(10,0);  TESTAP(11,0);
  TESTAP(12,0);  TESTAP(13,0);   TESTAP(14,0);  TESTAP(15,0);
  TESTAP(16,0);  TESTAP(17,0);   TESTAP(18,0);  TESTAP(19,0);
  TESTAP(20,0);  TESTAP(21,0);   TESTAP(22,0);  TESTAP(23,0);
  TESTAP(24,0);  TESTAP(25,0);   TESTAP(26,0);  TESTAP(27,0);
  TESTAP(28,0);  TESTAP(29,0);   TESTAP(30,0);  TESTAP(31,0);
  TESTAP(32,0);

  rc = Dos32ConfigurePerf(1,
                          1,
                          ulBID, 
                          0,
                          0,
                          0);
    printf ("configureperf (%08xh) start rc = %u %08x ...\n",
         ulBID,
         rc,
         rc);
  
  */
    
/*
  TESTAP(11,0);

  pTBH = (PTBH)pbRangeStart;

  TESTAP(19,0);
  pDBH = (PDBH)pbRangeStart;

  ulBID = pTBH->tbh_bidID.bid_usInstance | (pTBH->tbh_bidID.bid_usGroup << 16);
*/
  
  ulRangeSize  = 65536;
  pbRangeStart = malloc(ulRangeSize);
  printf ("pbRangeStart = %08xh\n",pbRangeStart);
  
  while( !kbhit() )
  {
    ULONG ulSize;
    PBYTE pBlock;
    ULONG ulLoop;
    
    //DosSleep(1000);
    printf("* waiting for PerfView message\n ");
    rc = DosWaitEventSem(hevSem,
                        1000);
    if (rc == NO_ERROR)
    {
      printf("* got PerfView message\n ");
      
      printf ("***QUEUE SNAPSHOT***\n");
      TESTAP(32,ulRangeSize);
      
      /* try to calculate blocks from messages in queue */
      for (pBlock = pbRangeStart,
           ulSize = *(PULONG)pbRangeStart;
           
           ulSize != 0;
           
           pBlock += ulSize,
           ulSize = *(PULONG)pBlock)
      {
        printf("\n**BLOCK %08xh, SIZE %u (%08xh)**\n",
               pBlock,
               ulSize,
               ulSize);
        
        /*
        ToolsDumpHex(pBlock,
                     0x14,
                     pBlock);
        */
        
        pDBH = pBlock;
        pTBH = pBlock + 0x14; /* skip size */
/*
        PrintBlock (pTBH,
                    pDBH,
                    FALSE);
*/
        
        
        if (pDBH->dbh_flFlags & RPC_FL_KRNL)
          if (pDBH->dbh_flFlags & RPC_FL_TRAN)
            continue;
        
        printf ("\n**ALIASING BLOCK %04x:%04x**\n",
                pTBH->tbh_bidID.bid_usGroup,
                pTBH->tbh_bidID.bid_usInstance);
        ulBID = pTBH->tbh_bidID.bid_usInstance | 
                (pTBH->tbh_bidID.bid_usGroup << 16);
        
/*
        rc = Dos32ConfigurePerf(1,
                                1,
                                ulBID, 
                                0,
                                0,
                                0);
          printf ("configureperf (%08xh) start rc = %u %08x ...\n",
               ulBID,
               rc,
               rc);
*/
        
        TESTAP(0x11, ulBID);
        pDBH = (PDBH)pbRangeStart;
        
        PrintBlock(pTBH, pDBH, TRUE);
        
        for (ulLoop = 0;
             ulLoop < 9;
             ulLoop++)
        {
          rc = Dos32ConfigurePerf(1,
                                  1,
                                  ulBID, 
                                  0,
                                  0,
                                  0);
          if (rc)
            printf ("configureperf (%08xh) start rc = %u %08x ...\n",
                 ulBID,
                 rc,
                 rc);
          
          PrintBlock(pTBH, pDBH, TRUE);        
        }
      }
      
/*
      PrintBlock(pTBH,
                 pDBH);
      
      ToolsDumpHex( pbRangeStart,
                    ulRangeSize,
                    pbRangeStart);
      printf("\n");
*/
      ulRangeSize = 65536;
    }
    else
      printf ("* nothing yet\n");
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


/*****************************************************************************
 * Name      : APIRET PerfViewOff
 * Funktion  : turn off perfview system, terminate kernel entities and
 *             deallocate the perfview queue
 * Parameter :
 * Variablen :
 * Ergebnis  : API return code
 * Bemerkung : this might prevent some traps resulting from flawed logic
 *             in VMAliasToAlias and some PerfView functions
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET PerfViewOff(void)
{
  APIRET rc;

  /* to close previous session since PerfView is NOT terminated, when        */
  /* the client process dies.                                                */
  rc = Dos32ConfigurePerf(0, 
                          DCP_CONFIG_TERMINATE, 
                          0, 
                          0, 
                          0, 
                          0);
  if (rc == NO_ERROR)
    printf ("OS/2 PerfView subsystem terminated.\n");
  
  return (rc);
}


/*****************************************************************************
 * Name      : APIRET PerfViewControl
 * Funktion  : 
 * Parameter :
 * Variablen :
 * Ergebnis  : API return code
 * Bemerkung : 
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET PerfViewControl(void)
{
  APIRET rc;
  
  if (Options.fsPerfViewOff)
    rc = PerfViewOff();
  else
    rc = PerfCtrTest();
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


  rc = PerfViewControl();                                        /* Los geht's */
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  return (rc);
}
