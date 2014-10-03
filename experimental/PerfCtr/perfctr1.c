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


void PrintTextBlock(PTBH pTextBlk)
{
  ULONG ulName;
  PTBN  ptbnName;
  
  if (pTextBlk == NULL) return;
  
  printf ("Textblock %08xh groupID=%04xh instanceID=%04xh\n",
          pTextBlk->tbh_ulVersion,
          pTextBlk->tbh_bidID.bid_usGroup,
          pTextBlk->tbh_bidID.bid_usInstance);
  
  printf ("   Group: Flags=%08xh Size=%04xh MsgID=%04xh Name=%s\n",
          pTextBlk->tbh_tbnGroup.tbn_ulFlags,
          pTextBlk->tbh_tbnGroup.tbn_usSize,
          pTextBlk->tbh_tbnGroup.tbn_usMsgID,
          pTextBlk->tbh_tbnGroup.tbn_pszName + (ULONG)pTextBlk);
  
  printf ("   Inst : Flags=%08xh Size=%04xh MsgID=%04xh Name=%s\n",
          pTextBlk->tbh_tbnInstance.tbn_ulFlags,
          pTextBlk->tbh_tbnInstance.tbn_usSize,
          pTextBlk->tbh_tbnInstance.tbn_usMsgID,
          pTextBlk->tbh_tbnInstance.tbn_pszName + (ULONG)pTextBlk);
          
  printf ("   Msg=%s, Help=%s, Names=%u\n",
          pTextBlk->tbh_pszMsgFile + (ULONG)pTextBlk,
          pTextBlk->tbh_pszHelpFile + (ULONG)pTextBlk,
          pTextBlk->tbh_culName);
  
  for (ulName = 0;
       ulName < pTextBlk->tbh_culName;
       ulName++)
  {
    ptbnName = (PTBN)( (PSZ)&pTextBlk->tbh_atbnName[ulName] + (ULONG)pTextBlk);
    
    printf ("   %04u : Flags=%08xh Size=%04xh MsgID=%04xh Name=%s\n",
            ulName,
            ptbnName->tbn_ulFlags,
            ptbnName->tbn_usSize,
            ptbnName->tbn_usMsgID,
            ptbnName->tbn_pszName + (ULONG)pTextBlk);
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
  
  #define NumTimerCounters  7                                 
  
#pragma pack(1)
  UCHAR    GroupName[] = "DISK 01";/* Name of OS2DASD    */
  UCHAR    str1[] = "ctRD";        /* Read Counter       */
  UCHAR    str2[] = "qlRD";        /* Read Queue Counter */   
  UCHAR    str3[] = "blRD";        /* Read Block Count   */
  UCHAR    str4[] = "ctWR";        /* Write Counter      */
  UCHAR    str5[] = "qlWR";        /* Write Queue Counters */
  UCHAR    str6[] = "blWR";        /* Write Block Count  */
  UCHAR    str7[] = "tmBUSY";      /* Write Block Count  */   
  
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
  printf ("createsem rc = %u %08x ...\n",
          rc,
          rc);


  PerfViewDB.pfdbh.dbh_ulTotLen = sizeof(VDB);
  PerfViewDB.pfdbh.dbh_flFlags  = RPC_FL_32BIT | RPC_FL_APP;
  PerfViewDB.pfdbh.dbh_ulSem    = hevSem;

  rc = Dos32ConfigurePerf(0,
                        4,
                        0,
                        0,
                        0,
                        0);
  printf ("configureperf term rc = %u %08x ...\n",
          rc,
          rc);


  printf ("Configuring PerfView ...\n");

  /* initialize subsystem */
  rc = Dos32ConfigurePerf(100,
                        0,
                        hevSem,
                        0,
                        0,
                        0);
  
  printf ("configureperf init rc = %u %08x ...\n",
          rc,
          rc);


  printf ("textblock: bid: %08xh group=%04xh inst=%04xh\n",
          PerfViewTB.tbh_bidID,
          PerfViewTB.tbh_bidID.bid_usGroup,
          PerfViewTB.tbh_bidID.bid_usInstance);
/*
  rc = DosRegisterPerfCtrs(&PerfViewDB,
                           &PerfViewTB,
                           RPC_FL_APP | RPC_FL_32BIT | RPC_FL_DEREG);
  printf ("registerperf rc = %u %08x ...\n",
          rc,
          rc);

 printf ("textblock: bid: %08xh group=%04xh inst=%04xh\n",
          PerfViewTB.tbh_bidID,
          PerfViewTB.tbh_bidID.bid_usGroup,
          PerfViewTB.tbh_bidID.bid_usInstance);
*/
  
  printf ("configureperf start rc = %u %08x ...\n",
          rc,
          rc);

  rc = Dos32AliasPerfCtrs(0x01, /* rangetype */
                          0, /* size of snapshot */
                          &pbRangeStart,
                          &ulRangeSize);
  printf ("aliasperf1 init rc = %u %08x ...\n",
          rc,
          rc);

  printf ("pbRangeStart = %08xh, ulRangeSize = %u\n",
          pbRangeStart,
          ulRangeSize);


  rc = Dos32AliasPerfCtrs(0x03, /* rangetype */
                          0, /* size of snapshot */
                          &pbRangeStart,
                          &ulRangeSize);
  printf ("aliasperf2 init rc = %u %08x ...\n",
          rc,
          rc);

  printf ("pbRangeStart = %08xh, ulRangeSize = %u\n",
          pbRangeStart,
          ulRangeSize);


  rc = Dos32AliasPerfCtrs(0x07, /* rangetype */
                          0, /* size of snapshot */
                          &pbRangeStart,
                          &ulRangeSize);
  printf ("aliasperf3 init rc = %u %08x ...\n",
          rc,
          rc);

  printf ("pbRangeStart = %08xh, ulRangeSize = %u\n",
          pbRangeStart,
          ulRangeSize);


  rc = Dos32AliasPerfCtrs(0x0f, /* rangetype */
                          0, /* size of snapshot */
                          &pbRangeStart,
                          &ulRangeSize);
  printf ("aliasperf4 init rc = %u %08x ...\n",
          rc,
          rc);

  printf ("pbRangeStart = %08xh, ulRangeSize = %u\n",
          pbRangeStart,
          ulRangeSize);

#if 0
  rc = Dos32AliasPerfCtrs(0x17, /* rangetype */
                          0, /* size of snapshot */
                          &pbRangeStart,
                          &ulRangeSize);
  printf ("aliasperf1 init rc = %u %08x ...\n",
          rc,
          rc);

  printf ("pbRangeStart = %08xh, ulRangeSize = %u\n",
          pbRangeStart,
          ulRangeSize);

  rc = Dos32AliasPerfCtrs(0x1f, /* rangetype */
                          0, /* size of snapshot */
                          &pbRangeStart,
                          &ulRangeSize);
  printf ("aliasperf1 init rc = %u %08x ...\n",
          rc,
          rc);

  printf ("pbRangeStart = %08xh, ulRangeSize = %u\n",
          pbRangeStart,
          ulRangeSize);


  rc = Dos32AliasPerfCtrs(0x11, /* rangetype */
                          0, /* size of snapshot */
                          &pbRangeStart,
                          &ulRangeSize);
  printf ("aliasperf1 init rc = %u %08x ...\n",
          rc,
          rc);

  printf ("pbRangeStart = %08xh, ulRangeSize = %u\n",
          pbRangeStart,
          ulRangeSize);
#endif

  rc = Dos32AliasPerfCtrs(0x20, /* rangetype */
                          4096, /* size of snapshot */
                          &pbRangeStart,
                          &ulRangeSize);
  printf ("aliasperf1 init rc = %u %08x ...\n",
          rc,
          rc);

  printf ("pbRangeStart = %08xh, ulRangeSize = %u\n",
          pbRangeStart,
          ulRangeSize);

  pDBH = (PDBH)pbRangeStart;



  rc = Dos32ConfigurePerf(1,
                        1,
                        100,
                        0,
                        0,
                        0);


  ulRangeSize <<= 1;

  while( !kbhit() )
  {
    DosSleep(1000);
    
    ToolsDumpHex(pbRangeStart,
                 ulRangeSize,
                 pbRangeStart);
    PrintTextBlock( (PTBH) pbRangeStart);
    printf ("\nDEBUG\n");
  }



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
