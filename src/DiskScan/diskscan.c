/***********************************************************************
 * Name      : Module DiskScan
 * Funktion  : Scannen eines DASD Volumes
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 02.21.27]
 ***********************************************************************/

/* #define DEBUG */


/*===Includes================================================================*/
#define INCL_NOPMAPI
#define INCL_DOS
#define INCL_DOSERRORS    /* DOS error values    */
#define INCL_DOSFILEMGR   /* File Manager values */
#define INCL_DOSDEVICES   /* Device values    */
#define INCL_DOSDEVIOCTL
#define INCL_DOSMISC
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"
/*===End Includes============================================================*/


#define DisplayText printf


/*===Strukturen==============================================================*/
typedef struct
{
  ULONG ulCylinders;                       /* Anzahl der Zylinder der Platte */
  ULONG ulHeads;                              /* Anzahl der Kîpfe der Platte */
  ULONG ulSectorsPerTrack;                  /* Anzahl der Sektoren pro Track */
} DISKPARAMS, *PDISKPARAMS;


typedef struct                                 /* Struktur fÅr GETLOCKSTATUS */
{
  UCHAR ucCommand;                                         /* IOCtl-Kommando */
  UCHAR usDisk;                                            /* a=0,b=1,c=2,.. */
} DISKPARAMBLOCK, *PDISKPARAMBLOCK;


typedef struct _TrackEntry         /* describes a sector structure for IOCTL */
{
  USHORT usSectorNumber;
  USHORT usSectorSize;
} TRACKENTRY, *PTRACKENTRY;


typedef struct
{
  ARGFLAG fsHelp;                 /* Muss Hilfestellung dargestellt werden ? */
  ARGFLAG fsVerbose;                                  /* produce more output */
  ARGFLAG fsDiskScanAll;                /* DiskScan all found logical drives */
  ARGFLAG fsDiskScanDrives;                 /* DiskScan these logical drives */
  ARGFLAG fsFlushCache;                               /* Flush caches before */
  PSZ     pszDiskScanDrives;                        /* DiskScan these drives */
  USHORT  usPattern;                                   /* erase pattern mode */

  ARGFLAG fsCmdErase;                                 /* erase      the disk */
  ARGFLAG fsCmdPattern;                               /* erase pattern mode  */
  ARGFLAG fsCmdWriteTest;                             /* read/write the disk */
  ARGFLAG fsCmdWriteTest2;                            /* read/write the disk */
  ARGFLAG fsCmdRead;                                        /* read the disk */
  ARGFLAG fsCmdVerify;                                      /* vrfy the disk */
  ARGFLAG fsCmdLogical;                          /* operate on logical disks */
  ARGFLAG fsCmdPhysical;                        /* operate on physical disks */
  ARGFLAG fsConfirmationSkip;         /* no prompts for dangerous operations */
  
  ARGFLAG fsCylinderStart;                              /* starting cylinder */
  ARGFLAG fsCylinderEnd;                                /* ending   cylinder */
  ULONG   ulCylinderStart;
  ULONG   ulCylinderEnd;
  
  ARGFLAG fsIORead;                          /* dump the read data to stdout */

} OPTIONS, *POPTIONS;

/*===End Strukturen==========================================================*/


/*===Prototypen==============================================================*/
APIRET DskDiskPartitionableCount ( PULONG              pulDisks );
APIRET DskDiskGetIOCtlHandle     ( ULONG               ulDiskNumber,
                                   PULONG              pulHandle );
APIRET DskDiskFreeIOCtlHandle    ( ULONG               ulHandle );
APIRET DskGenericIO              ( HFILE               hDisk,
                                   ULONG               ulIOCtlClass,
                                   ULONG               ulCtlCommand,
                                   ULONG               ulDataLen,
                                   PULONG              pulDataRetLen,
                                   PVOID               pData );
APIRET DskDiskGetPhysicalParameters (HFILE             hDisk,
                                     PDISKPARAMS       pDiskParams);
APIRET DskPhysDiskLock           ( HFILE               hDisk );
APIRET DskPhysDiskUnlock         ( HFILE               hDisk );
APIRET DskCacheFlush             ( VOID );
APIRET DiskScanPhysicalDisks      ( VOID );
APIRET DiskScanLogicalDisks       ( PSZ                 pszDrives );
APIRET DiskScanInformation        ( UCHAR               ucDrive );

APIRET DskDiskWriteTest(HFILE       hDisk,
                        ULONG       ulDisk,
                        PDISKPARAMS pDiskParams);


APIRET DskDiskWriteTest2(HFILE       hDisk,
                         ULONG       ulDisk,
                         PDISKPARAMS pDiskParams);

APIRET DskDiskErase    (HFILE       hDisk,
                        ULONG       ulDisk,
                        PDISKPARAMS pDiskParams);


APIRET DskDiskRead(HFILE       hDisk,
                   ULONG       ulDisk,
                   PDISKPARAMS pDiskParams);

APIRET DskDiskVerify(HFILE       hDisk,
                     ULONG       ulDisk,
                     PDISKPARAMS pDiskParams);

APIRET DskDiskIORead(HFILE       hDisk,
                     ULONG       ulDisk,
                     PDISKPARAMS pDiskParams);


/*===Globale Strukturen======================================================*/

OPTIONS Options;

ARGUMENT TabArguments[] =
{ /*Token--Beschreibung--pTarget--ucTargetFormat--pTargetSpecified--*/
  {"/LOGICAL",    "Operate on logical drives.",            NULL,                      ARG_NULL, &Options.fsCmdLogical},
  {"/PHYSICAL",   "Operate on physical drives (default).", NULL,                      ARG_NULL, &Options.fsCmdPhysical},
  {"/WRITE2",     "Perform a short read/write test on the drive.",NULL,               ARG_NULL, &Options.fsCmdWriteTest2},
  {"/WRITE1",     "Perform a thorough read/write test on the drive.",NULL,            ARG_NULL, &Options.fsCmdWriteTest},
  {"/READ",       "Read the disk.",                        NULL,                      ARG_NULL, &Options.fsCmdRead},
  {"/VERIFY",     "Verifies the disk.",                    NULL,                      ARG_NULL, &Options.fsCmdVerify},
  {"/ERASE",      "Erase the disk.",                       NULL,                      ARG_NULL, &Options.fsCmdErase},
  {"/PATTERN=",   "0=zeros, 1=ones, 2=AA55AA55, 3=F6F6F6F6, 4=random",  &Options.usPattern,ARG_USHORT,&Options.fsCmdPattern},
  {"/D=",         "DiskScan these logical drives. /D=CDEF",&Options.pszDiskScanDrives, ARG_PSZ,  &Options.fsDiskScanDrives},
  {"/ALL",        "DiskScan all logical drives.",          NULL,                      ARG_NULL, &Options.fsDiskScanAll},
  {"/NOFLUSH",    "Don't flush the system caches.",        NULL,                      ARG_NULL, &Options.fsFlushCache},
  {"/CYL.START=", "Starting cylinder.",                    &Options.ulCylinderStart,  ARG_ULONG, &Options.fsCylinderStart},
  {"/CYL.END=",   "Ending cylinder.",                      &Options.ulCylinderEnd,    ARG_ULONG, &Options.fsCylinderEnd},
  {"/Y",          "Skip confirmations for dangerous operations.",
                                                           NULL,                 ARG_NULL,       &Options.fsConfirmationSkip},
  {"/IO.READ",    "Read the harddrive and dump data to stdout.", NULL,                ARG_NULL, &Options.fsIORead},
  {"/V",          "Verbose mode, much output.",            NULL,                      ARG_NULL, &Options.fsVerbose},
  {"/?",          "Get help screen.",                      NULL,                      ARG_NULL, &Options.fsHelp},
  {"/H",          "Get help screen.",                      NULL,                      ARG_NULL, &Options.fsHelp},
  ARG_TERMINATE
};

/*===End Globale Strukturen==================================================*/


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
  TOOLVERSION("DiskScan",                                /* application name */
              0x00010009,                            /* application revision */
              0x00010900,           /* minimum required revision of PHSTOOLS */
              0x0001FFFF,        /* maximum recommended revision of PHSTOOLS */
              NULL,                                                /* Remark */
              NULL);                                 /* additional copyright */
}


/***********************************************************************
 * Name      : APIRET hlp_allocateTrackLayout
 * Funktion  : allocates and initializes a track layout table for
 *             low level disk i/o
 * Parameter : ULONG  ulSectoryPerTrack
 *             PPVOID ppBuffer
 *             PULONG pulBufferLength
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 02.27.30]
 ***********************************************************************/

APIRET hlp_allocateTrackLayout(ULONG ulSectorsPerTrack,
                               TRACKLAYOUT** ppBuffer,
                               PULONG pulBufferLength)
{
                                                     /* allocate TRACKLAYOUT */
  ULONG ulTrackLayoutSize = sizeof(TRACKLAYOUT) +
    sizeof(TRACKENTRY) * ulSectorsPerTrack;
  ULONG ulLoopSector;
  PTRACKLAYOUT pTrackLayout;
  
  pTrackLayout = (PVOID) malloc(ulTrackLayoutSize);  /* allocate that memory */
  if (pTrackLayout == NULL)                       /* check proper allocation */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */

  /* build up the sector table */
  pTrackLayout->bCommand      = 1;  /* consecutive sectors, starting with #1 */
  pTrackLayout->usHead        = 0;
  pTrackLayout->usCylinder    = 0;
  pTrackLayout->usFirstSector = 0;
  pTrackLayout->cSectors      = ulSectorsPerTrack;

  for (ulLoopSector = 0;
       ulLoopSector < ulSectorsPerTrack;
       ulLoopSector++)
  {
    pTrackLayout->TrackTable[ulLoopSector].usSectorNumber = ulLoopSector + 1;
    pTrackLayout->TrackTable[ulLoopSector].usSectorSize   = 512;
  }
  
  /* pass back values */
  *ppBuffer = pTrackLayout;
  *pulBufferLength = ulTrackLayoutSize;
  
  return NO_ERROR;
}


/***********************************************************************
 * Name      : ULONG hlp_getCylinderStart
 * Funktion  : Determine from which cylinder to start operation
 * Parameter : PDISKPARAMS pDiskParams
 * Variablen :
 * Ergebnis  : starting cylinder index
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 02.27.30]
 ***********************************************************************/

ULONG hlp_getCylinderStart(PDISKPARAMS pDiskParams)
{
  ULONG ulCylinder = 0;
  
  if (Options.fsCylinderStart)
    ulCylinder = Options.ulCylinderStart;
  
  // if (ulCylinder < 0)
  //  ulCylinder = 0;
  
  if (ulCylinder > pDiskParams->ulCylinders)
    ulCylinder = pDiskParams->ulCylinders;
  
  return ulCylinder;
}



/***********************************************************************
 * Name      : ULONG hlp_getCylinderEnd
 * Funktion  : Determine from which cylinder to end operation
 * Parameter : PDISKPARAMS pDiskParams
 * Variablen :
 * Ergebnis  : ending cylinder index
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 02.27.30]
 ***********************************************************************/

ULONG hlp_getCylinderEnd(PDISKPARAMS pDiskParams)
{
  ULONG ulCylinder = pDiskParams->ulCylinders;
  
  if (Options.fsCylinderEnd)
    ulCylinder = Options.ulCylinderEnd;
  
  // if (ulCylinder < 0)
  //  ulCylinder = 0;
  
  if (ulCylinder > pDiskParams->ulCylinders)
    ulCylinder = pDiskParams->ulCylinders;
  
  return ulCylinder;
}



/***********************************************************************
 * Name      : APIRET DskDiskPartitionableCount
 * Funktion  : ZÑhlt die partitionierbaren Platten.
 * Parameter : PULONG pulDisks
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 02.27.30]
 ***********************************************************************/

APIRET DskDiskPartitionableCount (PULONG pulDisks)
{
  APIRET rc = NO_ERROR;                                /* RÅckgabewert */
  USHORT usNumDrives  = 0;                /* Data return buffer        */
  ULONG  ulDataLen    = sizeof(USHORT);   /* Data return buffer length */

  if ( pulDisks == NULL )                      /* ParameterÅberprÅfung */
    return (ERROR_INVALID_PARAMETER);

  /* Request a count of the number of partitionable disks in the system */
  rc = DosPhysicalDisk(INFO_COUNT_PARTITIONABLE_DISKS,
                        &usNumDrives,
                        ulDataLen,
                        NULL,        /* No parameter for this function */
                        0L);

  if ( rc == NO_ERROR )            /* Wenn kein Fehler aufgetreten ist */
    *pulDisks = (ULONG)usNumDrives;   /* In die Zielvariable schreiben */

  return (rc);                                 /* RÅckgabewert liefern */
} /* APIRET DskDiskPartitionableCount */


/***********************************************************************
 * Name      : APIRET DskDiskGetIOCtlHandle
 * Funktion  : Allokiert ein IOCtl-Handle fÅr das entspr. Laufwerk.
 * Parameter : ULONG ulDiskNumber, PULONG pulHandle
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 02.34.16]
 ***********************************************************************/

APIRET DskDiskGetIOCtlHandle (ULONG ulDiskNumber, PULONG pulHandle)
{
  APIRET rc            = NO_ERROR;                     /* RÅckgabewert */
  USHORT usIOCtlHandle = 0;               /* Data return buffer        */
  ULONG  ulDataLen     = sizeof(USHORT);  /* Data return buffer length */
  UCHAR  pszParamBlock[10];                          /* Parameterblock */
  ULONG  ulParamLen    = 0;                /* Length of parameterblock */

  if ( pulHandle == NULL )                     /* ParameterÅberprÅfung */
    return (ERROR_INVALID_PARAMETER);

                                        /* Prepare the parameter block */
  sprintf(pszParamBlock,"%u:",ulDiskNumber);
  ulParamLen = strlen(pszParamBlock) + 1;

  /* Request a count of the number of partitionable disks in the system */
  rc = DosPhysicalDisk(INFO_GETIOCTLHANDLE,
                        &usIOCtlHandle,
                        ulDataLen,
                        pszParamBlock,/* No parameter for this function */
                        ulParamLen);

  if ( rc == NO_ERROR )            /* Wenn kein Fehler aufgetreten ist */
    *pulHandle = (ULONG)usIOCtlHandle;/* In die Zielvariable schreiben */


  return (rc);                                 /* RÅckgabewert liefern */
} /* APIRET DskDiskGetIOCtlHandle */


/***********************************************************************
 * Name      : APIRET DskDiskFreeIOCtlHandle
 * Funktion  : Freigeben des IOCtl-Handle fÅr das entspr. Laufwerk.
 * Parameter : ULONG ulHandle
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 02.40.01]
 ***********************************************************************/

APIRET DskDiskFreeIOCtlHandle (ULONG ulHandle)
{
  APIRET rc = NO_ERROR;                                /* RÅckgabewert */
  USHORT usIOCtlHandle = 0;               /* Data return buffer        */


  /* Request a count of the number of partitionable disks in the system */
  usIOCtlHandle = (USHORT)ulHandle;
  rc = DosPhysicalDisk(INFO_FREEIOCTLHANDLE,
                        0,
                        0L,
                        &usIOCtlHandle,/* No parameter for this function */
                        sizeof(USHORT));

  return (rc);                                 /* RÅckgabewert liefern */
} /* APIRET DskDiskFreeIOCtlHandle */


/***********************************************************************
 * Name      : APIRET DskGenericIO
 * Funktion  : Abwicklung der generischen IOCtls.
 * Parameter : HFILE hDisk, ULONG ulIOCtlClass, ULONG ulCtlCommand
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung : Spart viel Code...
 *
 * Autor     : Patrick Haller [Montag, 23.10.1995 17.40.32]
 ***********************************************************************/

APIRET DskGenericIO (HFILE  hDisk,
                     ULONG  ulIOCtlClass,
                     ULONG  ulCtlCommand,
                     ULONG  ulDataLen,
                     PULONG pulDataRetLen,
                     PVOID  pData)
{
  ULONG  ulParmRetLen;                          /* ParameterpaketlÑnge */
  ULONG  ulDataRetLen;                          /* ParameterpaketlÑnge */
  APIRET rc;                                           /* RÅckgabewert */
  UCHAR  ucCommand = 0;                                       /* IOCtl */

  if ( (pulDataRetLen == NULL) ||              /* ParameterÅberprÅfung */
       (pData == NULL) )
    return (ERROR_INVALID_PARAMETER);


    /* This is the direct way to communicate with the driver via IOCtl */
  ulParmRetLen = 0;

  rc = DosDevIOCtl(hDisk,                             /* Device-Handle */
         ulIOCtlClass,                                     /* Category */
         ulCtlCommand,                                     /* Function */
         &ucCommand,                         /* Parameterblock-Pointer */
         sizeof(ucCommand),          /* Max. LÑnge der Parameterblocks */
         &ulParmRetLen,       /* Pointer auf LÑnge des Parameterblocks */
         pData,                                          /* Datenblock */
         ulDataLen,                  /* Maximale LÑnge des Datenblocks */
         &ulDataRetLen);          /* Pointer auf LÑnge des Datenblocks */

#ifdef DEBUG
  printf ("\nDebug: DskGenericIO () = #%u",rc);
#endif

  return (rc);                                 /* RÅckgabewert liefern */
} /* APIRET DskGenericIO */


/***********************************************************************
 * Name      : APIRET DskDiskGetPhysicalParameters
 * Funktion  : Ermittelt die physikalische Geometrie des GerÑtes.
 * Parameter : HFILE hDisk, PDISKPARAMS pDiskParams
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 02.58.58]
 ***********************************************************************/

APIRET DskDiskGetPhysicalParameters (HFILE       hDisk,
                                     PDISKPARAMS pDiskParams)
{
  APIRET               rc = NO_ERROR;                  /* RÅckgabewert */
  DEVICEPARAMETERBLOCK DeviceParameterBlock;  /* Geometrie des Devices */
  ULONG                ulDataRetLen;         /* LÑnge des Datenblockes */


  if (pDiskParams == NULL)                     /* ParameterÅberprÅfung */
    return (ERROR_INVALID_PARAMETER);

  ulDataRetLen = sizeof(DeviceParameterBlock);
  rc = DskGenericIO (hDisk,                     /* Nun die Daten holen */
                     IOCTL_PHYSICALDISK,
                     PDSK_GETPHYSDEVICEPARAMS,
                     sizeof (DeviceParameterBlock),
                     &ulDataRetLen,
                     &DeviceParameterBlock);

            /* Nun die gewonnenen Daten in die Zielvariablen schreiben */
  if (rc == NO_ERROR)                    /* Wenn soweit alles i.O. ist */
  {
    pDiskParams->ulCylinders       = (ULONG)DeviceParameterBlock.cCylinders;
    pDiskParams->ulHeads           = (ULONG)DeviceParameterBlock.cHeads;
    pDiskParams->ulSectorsPerTrack = (ULONG)DeviceParameterBlock.cSectorsPerTrack;
  }

  return (rc);                                 /* RÅckgabewert liefern */
} /* APIRET DskDiskGetPhysicalParameters */


/***********************************************************************
 * Name      : APIRET DskPhysDiskLock
 * Funktion  : Allokiert komplette phys. Platte exklusiv.
 * Parameter : HFILE hDisk
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 03.11.45]
 ***********************************************************************/

APIRET DskPhysDiskLock (HFILE hDisk)
{
  APIRET rc = NO_ERROR;                                /* RÅckgabewert */
  UCHAR  pszParam[31];
  ULONG  ulParamLen = sizeof(pszParam);

  memset (pszParam,
          0,
          ulParamLen);

  rc = DskGenericIO ( hDisk,
                      IOCTL_PHYSICALDISK,
                      PDSK_LOCKPHYSDRIVE,
                      ulParamLen,
                      &ulParamLen,
                      pszParam );

  return (rc);                                 /* RÅckgabewert liefern */
} /* APIRET DskPhysDiskLock */



/***********************************************************************
 * Name      : APIRET DskPhysDiskUnLock
 * Funktion  : Gibt komplette phys. Platte frei.
 * Parameter : HFILE hDisk
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 03.11.45]
 ***********************************************************************/

APIRET DskPhysDiskUnlock (HFILE hDisk)
{
  APIRET rc = NO_ERROR;                                /* RÅckgabewert */
  UCHAR  pszParam[31];
  ULONG  ulParamLen = sizeof(pszParam);

  memset (pszParam,0,ulParamLen);
  rc = DskGenericIO ( hDisk,
                      IOCTL_PHYSICALDISK,
                      PDSK_UNLOCKPHYSDRIVE,
                      ulParamLen,
                      &ulParamLen,
                      pszParam );

  return (rc);                                 /* RÅckgabewert liefern */
} /* APIRET DskPhysDiskUnlock */


/***********************************************************************
 * Name      : APIRET DskCacheFlush
 * Funktion  : Flusht die Caches der Filesysteme
 * Parameter :
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 04.32.37]
 ***********************************************************************/

APIRET DskCacheFlush (VOID)
{
  APIRET rc = NO_ERROR;                                /* RÅckgabewert */

  rc = DosShutdown(1L);                               /* Caches leeren */
  return (rc);                                 /* RÅckgabewert liefern */
}


/***********************************************************************
 * Name      : APIRET DiskScanPhysicalDisks
 * Funktion  : Infos ueber physical disks
 * Parameter :
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 04.32.37]
 ***********************************************************************/

APIRET DiskScanPhysicalDisks (void)
{
  ULONG   ulNumDrives  = 0;                  /* Data return buffer        */
  ULONG   i;                                          /* Schleifenzaehler */
  char    szDriveNumber[4];                          /* string for device ID */
  ULONG   hDisk;                                   /* Disk-Handle (IOCtl) */
  APIRET  rc;                                               /* Returncode */

  DISKPARAMS DiskParams;                  /* Angaben zur Plattengeometrie */
  ULONG      ulDriveLast;
  
  
  // @@@PH support disk reading disk -> pipe to netcat
  

  rc = DskDiskPartitionableCount (&ulNumDrives);      /* Wieviele Disks ? */
  if (rc != NO_ERROR)                             /* Fehler aufgetreten ? */
    ToolsErrorDosEx(rc,
                    "DosPhysicalDisk");
  else
    DisplayText("\n%u partitionable disk(s):",
                ulNumDrives);

  for (i = 1;                        /* Alle gefundenen Disks untersuchen */
       i <= ulNumDrives;
       i++)
  {
    DisplayText ("\nPhysical Drive %u:",
                 i);

    rc = DskDiskGetIOCtlHandle(i,
                               &hDisk);                   /* Handle erfragen */
    if (rc != NO_ERROR)                              /* Fehler aufgetreten ? */
      ToolsErrorDosEx(rc,
                      "DskDiskGetIOCtlHandle");
    else                                                      /* Kein Fehler */
    {
                                             /* Now we have a IOCtl handle ! */
      rc = DskDiskGetPhysicalParameters (hDisk,            /* Infos erfragen */
                                         &DiskParams);
     if (rc != NO_ERROR)                             /* Fehler aufgetreten ? */
       DisplayText(" DskDiskGetPhysicalParameters error #%u\n",
                   rc);
     else
     {
       DisplayText (" %5u Cyl, %2u Heads, %3u Sec/Trk.",    /* Info ausgeben */
                    DiskParams.ulCylinders,
                    DiskParams.ulHeads,
                    DiskParams.ulSectorsPerTrack);


       /* check if we've got to operate on that disk */
       ultoa(i,
             szDriveNumber,
             10);


       if (Options.fsDiskScanDrives)
         if (strstr(Options.pszDiskScanDrives,
                    szDriveNumber) == 0)
           continue;

       rc = DskPhysDiskLock (hDisk);    /* PrÅfen, ob Lock auf Disk moeglich */
       if (rc != NO_ERROR)
         DisplayText (" can't lock (%u), ",
                      rc);                      /* kann nicht gelockt werden */
       else
         DisplayText (" lock OK,         ",
                      i);                             /* kann gelockt werden */


       /* do some more dangerous testing ? */
       if (Options.fsCmdErase)
       {
         rc = DskDiskErase(hDisk,
                           i,
                           &DiskParams);
       }
       else
       if (Options.fsCmdWriteTest)
       {
         rc = DskDiskWriteTest(hDisk,
                               i,
                               &DiskParams);
       }
       else
         if (Options.fsCmdWriteTest2)
         {
           rc = DskDiskWriteTest2(hDisk,
                                  i,
                                  &DiskParams);
         }
         else
           {
             if (Options.fsCmdRead)
             {
               if (Options.fsIORead)
                 rc = DskDiskIORead(hDisk,
                                    i,
                                    &DiskParams);
               else
                 rc = DskDiskRead(hDisk,
                                  i,
                                  &DiskParams);
             }
             
             
             if (Options.fsCmdVerify)
               rc = DskDiskVerify(hDisk,
                                  i,
                                  &DiskParams);
           }


       if (rc != NO_ERROR)
         ToolsErrorDos(rc);

       rc = DskPhysDiskUnlock (hDisk);                     /* Unlock prÅfen */
       if (rc != NO_ERROR)
         DisplayText ("can't unlock (%u).", rc);
       else
         DisplayText ("unlock OK.",i);

      }

      rc = DskDiskFreeIOCtlHandle(hDisk);         /* Handle wieder freigeben */
      if (rc != NO_ERROR)
        ToolsErrorDosEx(rc,
                        "DskDiskFreeIOCtlHandle");
    }
  }

  return (rc);                                 /* RÅckgabewert liefern */
}


/***********************************************************************
 * Name      : APIRET DskDiskWriteTest
 * Funktion  : Performs read/write test on the physical drive
 * Parameter : HFILE hDisk
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 02.27.30]
 ***********************************************************************/

APIRET DskDiskWriteTest(HFILE       hDisk,
                        ULONG       ulDisk,
                        PDISKPARAMS pDiskParams)
{
  APIRET     rc;                                           /* API returncode */
  PVOID      pBuffer;                               /* buffer for the tracks */
  PVOID      pBuffer2;                       /* buffer backup for the tracks */
  ULONG      ulBufferLength;                         /* length of the buffer */
  ULONG      ulRetBufferLength;               /* return value from the IOCTL */
  PERFSTRUCT psStart;                                        /* benchmarking */
  PERFSTRUCT psEnd;
  PERFSTRUCT psStartTrk;
  PERFSTRUCT psEndTrk;
  float      fTimeTotal;
  float      fTimeTrk;
  TRACKLAYOUT *pTrackLayout;                            /* the command table */
  ULONG      ulTrackLayoutSize;          /* size of the buffer for the table */
  ULONG      ulRetTrackLayoutSize;            /* return value from the IOCTL */

  ULONG      ulLoopHead;                         /* loop iteration variables */
  ULONG      ulLoopCylinder;
  ULONG      ulLoopSector;

  float      fSpeedMin        = 9999999.99;
  float      fSpeedCurrent;
  float      fSpeedMax        = 0.0;

  int iAnswer;                                   /* user confirmation result */



  if (pDiskParams == NULL)                               /* check parameters */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */


  printf ("\nWrite-testing physical drive %u, %u cylinders, %u heads.",
          ulDisk,
          pDiskParams->ulCylinders,
          pDiskParams->ulHeads);

  /* get confirmation by user */
  if (!Options.fsConfirmationSkip)            /* prompt for every deletion ? */
  {
    printf ("\nThis operation may cause data loss. Backup first. Continue ?\n");


    iAnswer = ToolsConfirmationQuery();                      /* ask the user */
    switch (iAnswer)
    {
      case 0:                                                          /* no */
        return (NO_ERROR);                               /* abort processing */

      case 1:                                                         /* yes */
        break;                                               /* continue ... */

      case 2:                                                      /* escape */
        exit (1);                         /* PH: urgs, terminate the process */
    }
  }


  ulBufferLength = pDiskParams->ulSectorsPerTrack *
                   512; /* 512 Bytes per Sector */
  pBuffer = (PVOID) malloc(ulBufferLength);          /* allocate that memory */
  if (pBuffer == NULL)                            /* check proper allocation */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */

  pBuffer2 = (PVOID) malloc(ulBufferLength);         /* allocate that memory */
  if (pBuffer2 == NULL)                           /* check proper allocation */
  {
    free (pBuffer);                      /* free previously allocated memory */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */
  }

                                                     /* allocate TRACKLAYOUT */
  rc = hlp_allocateTrackLayout(pDiskParams->ulSectorsPerTrack,
                               &pTrackLayout,
                               &ulTrackLayoutSize);
  if ( (rc != NO_ERROR) ||
       (pTrackLayout == NULL) )                   /* check proper allocation */
  {
    free(pBuffer);                       /* free previously allocated memory */
    free(pBuffer2);                      /* free previously allocated memory */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */
  }

  ToolsPerfQuery(&psStart);


  /* read all tracks */
  printf ("\n");

  for (ulLoopCylinder = hlp_getCylinderStart( pDiskParams );
       ulLoopCylinder < hlp_getCylinderEnd( pDiskParams );
       ulLoopCylinder++)
  {
    pTrackLayout->usCylinder    = ulLoopCylinder;

    ToolsPerfQuery(&psStartTrk);                             /* benchmarking */

    for (ulLoopHead = 0;
         ulLoopHead < pDiskParams->ulHeads;
         ulLoopHead++)
    {
      pTrackLayout->usHead        = ulLoopHead;

#if 0
      printf ("\nDEBUG: hDisk                = %u"
              "\n       pTrackLayout         = %08xh"
              "\n         bCommand           = %u"
              "\n         usHead             = %u"
              "\n         usCylinder         = %u"
              "\n         usFirstSector      = %u"
              "\n         cSectors           = %u"
              "\n       ulTrackLayoutSize    = %u"
              "\n       ulRetTrackLayoutSize = %u"
              "\n       pBuffer              = %08xh"
              "\n       ulBufferLength       = %u"
              "\n       ulRetBufferLength    = %u",
              hDisk,
              pTrackLayout,
              pTrackLayout->bCommand,
              pTrackLayout->usHead ,
              pTrackLayout->usCylinder,
              pTrackLayout->usFirstSector,
              pTrackLayout->cSectors,
              ulTrackLayoutSize,
              ulRetTrackLayoutSize,
              ulBufferLength,
              ulRetBufferLength);
#endif

  /*******************
   * 1. verify track *
   *******************/

      rc = DosDevIOCtl(hDisk,                               /* Device-Handle */
                       IOCTL_PHYSICALDISK,
                       PDSK_VERIFYPHYSTRACK,
                       pTrackLayout,
                       ulTrackLayoutSize,
                       &ulRetTrackLayoutSize,
                       NULL,
                       0,
                       &ulRetBufferLength);

      if (rc != NO_ERROR)                                /* check for errors */
        ToolsErrorDosEx(rc,
                        "DosDevIOCtl(PDSK_VERIFYPHYSTRACK)");
      else
      {
        /*******************
         * 2. read track   *
         *******************/

        rc = DosDevIOCtl(hDisk,                               /* Device-Handle */
                         IOCTL_PHYSICALDISK,
                         PDSK_READPHYSTRACK,
                         pTrackLayout,
                         ulTrackLayoutSize,
                         &ulRetTrackLayoutSize,
                         pBuffer,
                         ulBufferLength,
                         &ulRetBufferLength);

        if (rc != NO_ERROR)                                /* check for errors */
          ToolsErrorDosEx(rc,
                          "DosDevIOCtl(PDSK_READPHYSTRACK)");
        else
        {
          /*******************
           * 3. write track  *
           *******************/

          rc = DosDevIOCtl(hDisk,                               /* Device-Handle */
                           IOCTL_PHYSICALDISK,
                           PDSK_WRITEPHYSTRACK,
                           pTrackLayout,
                           ulTrackLayoutSize,
                           &ulRetTrackLayoutSize,
                           pBuffer,
                           ulBufferLength,
                           &ulRetBufferLength);

          if (rc != NO_ERROR)                                /* check for errors */
            ToolsErrorDosEx(rc,
                            "DosDevIOCtl(PDSK_WRITEPHYSTRACK)");
          else
          {
            /*************************
             * 4. verify track again *
             *************************/

            rc = DosDevIOCtl(hDisk,                               /* Device-Handle */
                             IOCTL_PHYSICALDISK,
                             PDSK_VERIFYPHYSTRACK,
                             pTrackLayout,
                             ulTrackLayoutSize,
                             &ulRetTrackLayoutSize,
                             NULL,
                             0,
                             &ulRetBufferLength);

            if (rc != NO_ERROR)                                /* check for errors */
              ToolsErrorDosEx(rc,
                              "DosDevIOCtl(PDSK_VERIFYPHYSTRACK)");
            else
            {
              /******************************************
               * 5. read track again and compare buffer *
               ******************************************/

              rc = DosDevIOCtl(hDisk,                               /* Device-Handle */
                               IOCTL_PHYSICALDISK,
                               PDSK_READPHYSTRACK,
                               pTrackLayout,
                               ulTrackLayoutSize,
                               &ulRetTrackLayoutSize,
                               pBuffer2,
                               ulBufferLength,
                               &ulRetBufferLength);

              if (rc != NO_ERROR)                                /* check for errors */
                ToolsErrorDosEx(rc,
                                "DosDevIOCtl(PDSK_READPHYSTRACK)");
              else
              {
                /* compare buffers */
                if (memcmp(pBuffer,
                           pBuffer2,
                           ulBufferLength) != 0)
                  fprintf(stderr,
                          "ERROR: Buffer mismatch after write ! Data loss !\n");
              }
            }
          }
        }
      }
    }

    ToolsPerfQuery(&psEndTrk);                               /* benchmarking */
    fTimeTrk = psEndTrk.fSeconds - psStartTrk.fSeconds;

    fSpeedCurrent = pDiskParams->ulHeads *
                    pDiskParams->ulSectorsPerTrack / 2 / fTimeTrk;

    if (fSpeedCurrent < fSpeedMin) fSpeedMin = fSpeedCurrent;
    if (fSpeedCurrent > fSpeedMax) fSpeedMax = fSpeedCurrent;

    printf ("\rTesting cylinder %5u, %10.3f/%10.3f/%10.3f kb/s ... ",
            ulLoopCylinder,
            fSpeedMin,
            fSpeedCurrent,
            fSpeedMax);


  }

  ToolsPerfQuery(&psEnd);

  fTimeTotal = psEnd.fSeconds = psStart.fSeconds;    /* calculate total time */

  free (pBuffer);                                         /* free the memory */
  free (pBuffer2);                                        /* free the memory */
  free (pTrackLayout);

  return (NO_ERROR);                                                   /* OK */
}


/***********************************************************************
 * Name      : APIRET DskDiskWriteTest2
 * Funktion  : Performs read/write test on the physical drive
 * Parameter : HFILE hDisk
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung : shorter non-destructive version
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 02.27.30]
 ***********************************************************************/

APIRET DskDiskWriteTest2(HFILE       hDisk,
                         ULONG       ulDisk,
                         PDISKPARAMS pDiskParams)
{
  APIRET     rc;                                           /* API returncode */
  PVOID      pBuffer;                               /* buffer for the tracks */
  PVOID      pBuffer2;                       /* buffer backup for the tracks */
  ULONG      ulBufferLength;                         /* length of the buffer */
  ULONG      ulRetBufferLength;               /* return value from the IOCTL */
  PERFSTRUCT psStart;                                        /* benchmarking */
  PERFSTRUCT psEnd;
  PERFSTRUCT psStartTrk;
  PERFSTRUCT psEndTrk;
  float      fTimeTotal;
  float      fTimeTrk;
  TRACKLAYOUT *pTrackLayout;                            /* the command table */
  ULONG      ulTrackLayoutSize;          /* size of the buffer for the table */
  ULONG      ulRetTrackLayoutSize;            /* return value from the IOCTL */

  ULONG      ulLoopHead;                         /* loop iteration variables */
  ULONG      ulLoopCylinder;
  ULONG      ulLoopSector;

  float      fSpeedMin        = 9999999.99;
  float      fSpeedCurrent;
  float      fSpeedMax        = 0.0;

  int iAnswer;                                   /* user confirmation result */

  if (pDiskParams == NULL)                               /* check parameters */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */


  printf ("\nShort Write-testing physical drive %u, %u cylinders, %u heads.",
          ulDisk,
          pDiskParams->ulCylinders,
          pDiskParams->ulHeads);

  /* get confirmation by user */
  if (!Options.fsConfirmationSkip)            /* prompt for every deletion ? */
  {
    printf ("\nThis operation may cause data loss. Backup first. Continue ?\n");


    iAnswer = ToolsConfirmationQuery();                      /* ask the user */
    switch (iAnswer)
    {
      case 0:                                                          /* no */
        return (NO_ERROR);                               /* abort processing */

      case 1:                                                         /* yes */
        break;                                               /* continue ... */

      case 2:                                                      /* escape */
        exit (1);                         /* PH: urgs, terminate the process */
    }
  }


  ulBufferLength = pDiskParams->ulSectorsPerTrack *
                   512; /* 512 Bytes per Sector */
  pBuffer = (PVOID) malloc(ulBufferLength);          /* allocate that memory */
  if (pBuffer == NULL)                            /* check proper allocation */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */

  pBuffer2 = (PVOID) malloc(ulBufferLength);         /* allocate that memory */
  if (pBuffer2 == NULL)                           /* check proper allocation */
  {
    free (pBuffer);                      /* free previously allocated memory */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */
  }

  
  rc = hlp_allocateTrackLayout(pDiskParams->ulSectorsPerTrack,
                               &pTrackLayout,
                               &ulTrackLayoutSize);
  if ( (rc != NO_ERROR) ||
       (pTrackLayout == NULL) )                   /* check proper allocation */
  {
    free(pBuffer);                       /* free previously allocated memory */
    free(pBuffer2);                      /* free previously allocated memory */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */
  }

  ToolsPerfQuery(&psStart);


  /* read all tracks */
  printf ("\n");

  for (ulLoopCylinder = hlp_getCylinderStart( pDiskParams );
       ulLoopCylinder < hlp_getCylinderEnd( pDiskParams );
       ulLoopCylinder++)
  {
    pTrackLayout->usCylinder    = ulLoopCylinder;

    ToolsPerfQuery(&psStartTrk);                             /* benchmarking */

    for (ulLoopHead = 0;
         ulLoopHead < pDiskParams->ulHeads;
         ulLoopHead++)
    {
      pTrackLayout->usHead        = ulLoopHead;

#if 0
      printf ("\nDEBUG: hDisk                = %u"
              "\n       pTrackLayout         = %08xh"
              "\n         bCommand           = %u"
              "\n         usHead             = %u"
              "\n         usCylinder         = %u"
              "\n         usFirstSector      = %u"
              "\n         cSectors           = %u"
              "\n       ulTrackLayoutSize    = %u"
              "\n       ulRetTrackLayoutSize = %u"
              "\n       pBuffer              = %08xh"
              "\n       ulBufferLength       = %u"
              "\n       ulRetBufferLength    = %u",
              hDisk,
              pTrackLayout,
              pTrackLayout->bCommand,
              pTrackLayout->usHead ,
              pTrackLayout->usCylinder,
              pTrackLayout->usFirstSector,
              pTrackLayout->cSectors,
              ulTrackLayoutSize,
              ulRetTrackLayoutSize,
              ulBufferLength,
              ulRetBufferLength);
#endif

  /*******************
   * 1. read track   *
   *******************/

      rc = DosDevIOCtl(hDisk,                               /* Device-Handle */
                       IOCTL_PHYSICALDISK,
                       PDSK_READPHYSTRACK,
                       pTrackLayout,
                       ulTrackLayoutSize,
                       &ulRetTrackLayoutSize,
                       pBuffer,
                       ulBufferLength,
                       &ulRetBufferLength);

      if (rc != NO_ERROR)                                /* check for errors */
        ToolsErrorDosEx(rc,
                        "DosDevIOCtl(PDSK_READPHYSTRACK)");
      else
      {
        /*******************
         * 2. write track  *
         *******************/

        rc = DosDevIOCtl(hDisk,                               /* Device-Handle */
                         IOCTL_PHYSICALDISK,
                         PDSK_WRITEPHYSTRACK,
                         pTrackLayout,
                         ulTrackLayoutSize,
                         &ulRetTrackLayoutSize,
                         pBuffer,
                         ulBufferLength,
                         &ulRetBufferLength);

        if (rc != NO_ERROR)                                /* check for errors */
          ToolsErrorDosEx(rc,
                          "DosDevIOCtl(PDSK_WRITEPHYSTRACK)");
      }
    }

    ToolsPerfQuery(&psEndTrk);                               /* benchmarking */
    fTimeTrk = psEndTrk.fSeconds - psStartTrk.fSeconds;

    fSpeedCurrent = pDiskParams->ulHeads *
                    pDiskParams->ulSectorsPerTrack / 2 / fTimeTrk;

    if (fSpeedCurrent < fSpeedMin) fSpeedMin = fSpeedCurrent;
    if (fSpeedCurrent > fSpeedMax) fSpeedMax = fSpeedCurrent;

    printf ("\rTesting cylinder %5u, %10.3f/%10.3f/%10.3f kb/s ... ",
            ulLoopCylinder,
            fSpeedMin,
            fSpeedCurrent,
            fSpeedMax);


  }

  ToolsPerfQuery(&psEnd);

  fTimeTotal = psEnd.fSeconds = psStart.fSeconds;    /* calculate total time */

  free (pBuffer);                                         /* free the memory */
  free (pBuffer2);                                        /* free the memory */
  free (pTrackLayout);

  return (NO_ERROR);                                                   /* OK */
}


/***********************************************************************
 * Name      : APIRET DskDiskErase
 * Funktion  : Performs erasure on the physical drive
 * Parameter : HFILE hDisk
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 02.27.30]
 ***********************************************************************/

void BuildBufferPattern(PVOID pBuffer, ULONG ulBufferLength)
{
  PULONG pulBuffer = (PULONG)pBuffer; /* access the buffer as 32-bit dwords */
  PUCHAR pucBuffer = (PUCHAR)pBuffer; /* access the buffer as  8-bit bytes  */
  ULONG  ulFill = 0;
  ULONG  ulIndex;

  if (Options.fsCmdPattern)
  {
      switch (Options.usPattern)
      {
        case 0: /* fill with zeros */
          ulFill = 0x00000000;
          break;

        case 1: /* fill with ones */
          ulFill = 0xFFFFFFFF;
          break;

        case 2: /* fill with alternating bits */
          ulFill = 0xAA55AA55;
          break;

        case 3: /* fill with this pattern */
          ulFill = 0xF6F6F6F6;
          break;

        case 4:
          /* random filler */
          for (ulIndex = 0;
               ulIndex < ulBufferLength;
               ulIndex++)
          {
            *pucBuffer++ = rand() & 0xFF;
          }

          return; /* done */
      }
  }

  /* write the bytes */
  for (ulIndex = 0;
       ulIndex < (ulBufferLength >> 2);
       ulIndex++)
  {
    *pulBuffer++ = ulFill;
  }
}


APIRET DskDiskErase(HFILE       hDisk,
                    ULONG       ulDisk,
                    PDISKPARAMS pDiskParams)
{
  APIRET     rc;                                           /* API returncode */
  PVOID      pBuffer;                               /* buffer for the tracks */
  ULONG      ulBufferLength;                         /* length of the buffer */
  ULONG      ulRetBufferLength;               /* return value from the IOCTL */
  PERFSTRUCT psStart;                                        /* benchmarking */
  PERFSTRUCT psEnd;
  PERFSTRUCT psStartTrk;
  PERFSTRUCT psEndTrk;
  float      fTimeTotal;
  float      fTimeTrk;
  TRACKLAYOUT *pTrackLayout;                            /* the command table */
  ULONG      ulTrackLayoutSize;          /* size of the buffer for the table */
  ULONG      ulRetTrackLayoutSize;            /* return value from the IOCTL */

  ULONG      ulLoopHead;                         /* loop iteration variables */
  ULONG      ulLoopCylinder;
  ULONG      ulLoopSector;

  float      fSpeedMin        = 9999999.99;
  float      fSpeedCurrent;
  float      fSpeedMax        = 0.0;

  int iAnswer;                                   /* user confirmation result */


  if (pDiskParams == NULL)                               /* check parameters */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */


  printf ("\nErasing physical drive %u, %u cylinders, %u heads.",
          ulDisk,
          pDiskParams->ulCylinders,
          pDiskParams->ulHeads);

  /* get confirmation by user */
  if (!Options.fsConfirmationSkip)            /* prompt for every deletion ? */
  {
    printf ("\nThis operation WILL cause data loss. Continue ?\n");


    iAnswer = ToolsConfirmationQuery();                      /* ask the user */
    switch (iAnswer)
    {
      case 0:                                                          /* no */
        return (NO_ERROR);                               /* abort processing */

      case 1:                                                         /* yes */
        break;                                               /* continue ... */

      case 2:                                                      /* escape */
        exit (1);                         /* PH: urgs, terminate the process */
    }
  }


  ulBufferLength = pDiskParams->ulSectorsPerTrack *
                   512; /* 512 Bytes per Sector */
  pBuffer = (PVOID) malloc(ulBufferLength);          /* allocate that memory */
  if (pBuffer == NULL)                            /* check proper allocation */
      return (ERROR_NOT_ENOUGH_MEMORY);             /* raise error condition */

  /*******************************************
   * Build erasure pattern within the buffer *
   *******************************************/

  BuildBufferPattern(pBuffer, ulBufferLength);
  
  rc = hlp_allocateTrackLayout(pDiskParams->ulSectorsPerTrack,
                               &pTrackLayout,
                               &ulTrackLayoutSize);
  if ( (rc != NO_ERROR) ||
       (pTrackLayout == NULL) )                   /* check proper allocation */
  {
    free(pBuffer);                       /* free previously allocated memory */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */
  }

  ToolsPerfQuery(&psStart);


  /* erase all tracks */
  printf ("\n");

  for (ulLoopCylinder = hlp_getCylinderStart( pDiskParams );
       ulLoopCylinder < hlp_getCylinderEnd( pDiskParams );
       ulLoopCylinder++)
  {
    pTrackLayout->usCylinder    = ulLoopCylinder;

    ToolsPerfQuery(&psStartTrk);                             /* benchmarking */

    for (ulLoopHead = 0;
         ulLoopHead < pDiskParams->ulHeads;
         ulLoopHead++)
    {
      pTrackLayout->usHead        = ulLoopHead;

#if 0
      printf ("\nDEBUG: hDisk                = %u"
              "\n       pTrackLayout         = %08xh"
              "\n         bCommand           = %u"
              "\n         usHead             = %u"
              "\n         usCylinder         = %u"
              "\n         usFirstSector      = %u"
              "\n         cSectors           = %u"
              "\n       ulTrackLayoutSize    = %u"
              "\n       ulRetTrackLayoutSize = %u"
              "\n       pBuffer              = %08xh"
              "\n       ulBufferLength       = %u"
              "\n       ulRetBufferLength    = %u",
              hDisk,
              pTrackLayout,
              pTrackLayout->bCommand,
              pTrackLayout->usHead ,
              pTrackLayout->usCylinder,
              pTrackLayout->usFirstSector,
              pTrackLayout->cSectors,
              ulTrackLayoutSize,
              ulRetTrackLayoutSize,
              ulBufferLength,
              ulRetBufferLength);
#endif

          /*******************
           * 3. write track  *
           *******************/

          rc = DosDevIOCtl(hDisk,                               /* Device-Handle */
                           IOCTL_PHYSICALDISK,
                           PDSK_WRITEPHYSTRACK,
                           pTrackLayout,
                           ulTrackLayoutSize,
                           &ulRetTrackLayoutSize,
                           pBuffer,
                           ulBufferLength,
                           &ulRetBufferLength);

          if (rc != NO_ERROR)                                /* check for errors */
            ToolsErrorDosEx(rc,
                            "DosDevIOCtl(PDSK_WRITEPHYSTRACK)");
    }

    ToolsPerfQuery(&psEndTrk);                               /* benchmarking */
    fTimeTrk = psEndTrk.fSeconds - psStartTrk.fSeconds;

    fSpeedCurrent = pDiskParams->ulHeads *
                    pDiskParams->ulSectorsPerTrack / 2 / fTimeTrk;

    if (fSpeedCurrent < fSpeedMin) fSpeedMin = fSpeedCurrent;
    if (fSpeedCurrent > fSpeedMax) fSpeedMax = fSpeedCurrent;

    printf ("\rErasing cylinder %5u, %10.3f/%10.3f/%10.3f kb/s ... ",
            ulLoopCylinder,
            fSpeedMin,
            fSpeedCurrent,
            fSpeedMax);


  }

  ToolsPerfQuery(&psEnd);

  fTimeTotal = psEnd.fSeconds = psStart.fSeconds;    /* calculate total time */

  free (pBuffer);                                         /* free the memory */
  free (pTrackLayout);

  return (NO_ERROR);                                                   /* OK */
}



/***********************************************************************
 * Name      : APIRET DskDiskRead
 * Funktion  : Reads the physical drive
 * Parameter : HFILE hDisk
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 02.27.30]
 ***********************************************************************/

APIRET DskDiskRead(HFILE       hDisk,
                   ULONG       ulDisk,
                   PDISKPARAMS pDiskParams)
{
  APIRET     rc;                                           /* API returncode */
  PVOID      pBuffer;                               /* buffer for the tracks */
  ULONG      ulBufferLength;                         /* length of the buffer */
  ULONG      ulRetBufferLength;               /* return value from the IOCTL */
  PERFSTRUCT psStart;                                        /* benchmarking */
  PERFSTRUCT psEnd;
  PERFSTRUCT psStartTrk;
  PERFSTRUCT psEndTrk;
  float      fTimeTotal;
  float      fTimeTrk;
  TRACKLAYOUT *pTrackLayout;                            /* the command table */
  ULONG      ulTrackLayoutSize;          /* size of the buffer for the table */
  ULONG      ulRetTrackLayoutSize;            /* return value from the IOCTL */

  ULONG      ulLoopHead;                         /* loop iteration variables */
  ULONG      ulLoopCylinder;
  ULONG      ulLoopSector;

  float      fSpeedMin        = 9999999.99;
  float      fSpeedCurrent;
  float      fSpeedMax        = 0.0;

  if (pDiskParams == NULL)                               /* check parameters */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */


  printf ("\nReading physical drive %u, %u cylinders, %u heads.",
          ulDisk,
          pDiskParams->ulCylinders,
          pDiskParams->ulHeads);

  ulBufferLength = pDiskParams->ulSectorsPerTrack *
                   512; /* 512 Bytes per Sector */
  pBuffer = (PVOID) malloc(ulBufferLength);          /* allocate that memory */
  if (pBuffer == NULL)                            /* check proper allocation */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */

  
  rc = hlp_allocateTrackLayout(pDiskParams->ulSectorsPerTrack,
                               &pTrackLayout,
                               &ulTrackLayoutSize);
  if ( (rc != NO_ERROR) ||
       (pTrackLayout == NULL) )                   /* check proper allocation */
  {
    free(pBuffer);                       /* free previously allocated memory */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */
  }

  ToolsPerfQuery(&psStart);

  /* read all tracks */
  printf ("\n");

  for (ulLoopCylinder = hlp_getCylinderStart( pDiskParams );
       ulLoopCylinder < hlp_getCylinderEnd( pDiskParams );
       ulLoopCylinder++)
  {
    pTrackLayout->usCylinder    = ulLoopCylinder;

    ToolsPerfQuery(&psStartTrk);                             /* benchmarking */

    for (ulLoopHead = 0;
         ulLoopHead < pDiskParams->ulHeads;
         ulLoopHead++)
    {
      pTrackLayout->usHead        = ulLoopHead;

#if 0
      printf ("\nDEBUG: hDisk                = %u"
              "\n       pTrackLayout         = %08xh"
              "\n         bCommand           = %u"
              "\n         usHead             = %u"
              "\n         usCylinder         = %u"
              "\n         usFirstSector      = %u"
              "\n         cSectors           = %u"
              "\n       ulTrackLayoutSize    = %u"
              "\n       ulRetTrackLayoutSize = %u"
              "\n       pBuffer              = %08xh"
              "\n       ulBufferLength       = %u"
              "\n       ulRetBufferLength    = %u",
              hDisk,
              pTrackLayout,
              pTrackLayout->bCommand,
              pTrackLayout->usHead ,
              pTrackLayout->usCylinder,
              pTrackLayout->usFirstSector,
              pTrackLayout->cSectors,
              ulTrackLayoutSize,
              ulRetTrackLayoutSize,
              ulBufferLength,
              ulRetBufferLength);
#endif
      rc = DosDevIOCtl(hDisk,                               /* Device-Handle */
                       IOCTL_PHYSICALDISK,
                       PDSK_READPHYSTRACK,
                       pTrackLayout,
                       ulTrackLayoutSize,
                       &ulRetTrackLayoutSize,
                       pBuffer,
                       ulBufferLength,
                       &ulRetBufferLength);

      if (rc != NO_ERROR)                                /* check for errors */
        ToolsErrorDosEx(rc,
                        "DosDevIOCtl(PDSK_READPHYSTRACK)");
    }

    ToolsPerfQuery(&psEndTrk);                               /* benchmarking */
    fTimeTrk = psEndTrk.fSeconds - psStartTrk.fSeconds;

    fSpeedCurrent = pDiskParams->ulHeads *
                    pDiskParams->ulSectorsPerTrack / 2 / fTimeTrk;

    if (fSpeedCurrent < fSpeedMin) fSpeedMin = fSpeedCurrent;
    if (fSpeedCurrent > fSpeedMax) fSpeedMax = fSpeedCurrent;

    printf ("\rReading cylinder %5u, %10.3f/%10.3f/%10.3f kb/s ... ",
            ulLoopCylinder,
            fSpeedMin,
            fSpeedCurrent,
            fSpeedMax);


  }

  ToolsPerfQuery(&psEnd);

  fTimeTotal = psEnd.fSeconds = psStart.fSeconds;    /* calculate total time */

  free (pBuffer);                                         /* free the memory */
  free (pTrackLayout);

  return (NO_ERROR);                                                   /* OK */
}


/***********************************************************************
 * Name      : APIRET DskDiskVerify
 * Funktion  : Verifies the physical drive
 * Parameter : HFILE hDisk
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 02.27.30]
 ***********************************************************************/

APIRET DskDiskVerify(HFILE       hDisk,
                     ULONG       ulDisk,
                     PDISKPARAMS pDiskParams)
{
  APIRET     rc;                                           /* API returncode */
  PERFSTRUCT psStart;                                        /* benchmarking */
  PERFSTRUCT psEnd;
  PERFSTRUCT psStartTrk;
  PERFSTRUCT psEndTrk;
  float      fTimeTotal;
  float      fTimeTrk;
  TRACKLAYOUT *pTrackLayout;                            /* the command table */
  ULONG      ulTrackLayoutSize;          /* size of the buffer for the table */
  ULONG      ulRetTrackLayoutSize;            /* return value from the IOCTL */
  ULONG      ulRetBufferLength;               /* return value from the IOCTL */

  ULONG      ulLoopHead;                         /* loop iteration variables */
  ULONG      ulLoopCylinder;
  ULONG      ulLoopSector;

  float      fSpeedMin        = 9999999.99;
  float      fSpeedCurrent;
  float      fSpeedMax        = 0.0;


  if (pDiskParams == NULL)                               /* check parameters */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */


  printf ("\nVerifying physical drive %u, %u cylinders, %u heads.",
          ulDisk,
          pDiskParams->ulCylinders,
          pDiskParams->ulHeads);

  
  rc = hlp_allocateTrackLayout(pDiskParams->ulSectorsPerTrack,
                               &pTrackLayout,
                               &ulTrackLayoutSize);
  if ( (rc != NO_ERROR) || 
       (pTrackLayout == NULL) )                   /* check proper allocation */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */

  ToolsPerfQuery(&psStart);

  /* read all tracks */
  printf ("\n");

  for (ulLoopCylinder = hlp_getCylinderStart( pDiskParams );
       ulLoopCylinder < hlp_getCylinderEnd( pDiskParams );
       ulLoopCylinder++)
  {
    pTrackLayout->usCylinder    = ulLoopCylinder;

    ToolsPerfQuery(&psStartTrk);                             /* benchmarking */

    for (ulLoopHead = 0;
         ulLoopHead < pDiskParams->ulHeads;
         ulLoopHead++)
    {
      pTrackLayout->usHead        = ulLoopHead;

#if 0
      printf ("\nDEBUG: hDisk                = %u"
              "\n       pTrackLayout         = %08xh"
              "\n         bCommand           = %u"
              "\n         usHead             = %u"
              "\n         usCylinder         = %u"
              "\n         usFirstSector      = %u"
              "\n         cSectors           = %u"
              "\n       ulTrackLayoutSize    = %u"
              "\n       ulRetTrackLayoutSize = %u"
              "\n       pBuffer              = %08xh"
              "\n       ulBufferLength       = %u"
              "\n       ulRetBufferLength    = %u",
              hDisk,
              pTrackLayout,
              pTrackLayout->bCommand,
              pTrackLayout->usHead ,
              pTrackLayout->usCylinder,
              pTrackLayout->usFirstSector,
              pTrackLayout->cSectors,
              ulTrackLayoutSize,
              ulRetTrackLayoutSize,
              ulBufferLength,
              ulRetBufferLength);
#endif
      rc = DosDevIOCtl(hDisk,                               /* Device-Handle */
                       IOCTL_PHYSICALDISK,
                       PDSK_VERIFYPHYSTRACK,
                       pTrackLayout,
                       ulTrackLayoutSize,
                       &ulRetTrackLayoutSize,
                       NULL,
                       0,
                       &ulRetBufferLength);

      if (rc != NO_ERROR)                                /* check for errors */
        ToolsErrorDosEx(rc,
                        "DosDevIOCtl(PDSK_VERIFYPHYSTRACK)");
    }

    ToolsPerfQuery(&psEndTrk);                               /* benchmarking */
    fTimeTrk = psEndTrk.fSeconds - psStartTrk.fSeconds;

    fSpeedCurrent = pDiskParams->ulHeads *
                    pDiskParams->ulSectorsPerTrack / 2 / fTimeTrk;

    if (fSpeedCurrent < fSpeedMin) fSpeedMin = fSpeedCurrent;
    if (fSpeedCurrent > fSpeedMax) fSpeedMax = fSpeedCurrent;

    printf ("\rVerify cylinder %5u, %10.3f/%10.3f/%10.3f kb/s ... ",
            ulLoopCylinder,
            fSpeedMin,
            fSpeedCurrent,
            fSpeedMax);


  }

  ToolsPerfQuery(&psEnd);

  fTimeTotal = psEnd.fSeconds = psStart.fSeconds;    /* calculate total time */

  free (pTrackLayout);

  return (NO_ERROR);                                                   /* OK */
}


/***********************************************************************
 * Name      : APIRET DskDiskIORead
 * Funktion  : Reads the physical drive
 * Parameter : HFILE hDisk
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 02.27.30]
 ***********************************************************************/

APIRET DskDiskIORead(HFILE       hDisk,
                     ULONG       ulDisk,
                     PDISKPARAMS pDiskParams)
{
  APIRET     rc;                                           /* API returncode */
  PERFSTRUCT psStart;                                        /* benchmarking */
  PERFSTRUCT psEnd;
  PERFSTRUCT psStartTrk;
  PERFSTRUCT psEndTrk;
  float      fTimeTotal;
  float      fTimeTrk;
  TRACKLAYOUT *pTrackLayout;                            /* the command table */
  ULONG      ulTrackLayoutSize;          /* size of the buffer for the table */
  ULONG      ulRetTrackLayoutSize;            /* return value from the IOCTL */
  ULONG      ulRetBufferLength;               /* return value from the IOCTL */
  PVOID      pBuffer;                               /* buffer for the tracks */
  ULONG      ulBufferLength;                         /* length of the buffer */

  ULONG      ulLoopHead;                         /* loop iteration variables */
  ULONG      ulLoopCylinder;
  ULONG      ulLoopSector;
  
  ULONG      ulBytesWritten;            /* number of bytes written to stdout */

  if (pDiskParams == NULL)                               /* check parameters */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */
  
  rc = hlp_allocateTrackLayout(pDiskParams->ulSectorsPerTrack,
                               &pTrackLayout,
                               &ulTrackLayoutSize);
  if ( (rc != NO_ERROR) || 
       (pTrackLayout == NULL) )                   /* check proper allocation */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */
  
  
  ulBufferLength = pDiskParams->ulSectorsPerTrack *
                   512; /* 512 Bytes per Sector */
  pBuffer = (PVOID) malloc(ulBufferLength);          /* allocate that memory */
  if (pBuffer == NULL)                            /* check proper allocation */
  {
    free(pTrackLayout);
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */
  }

  /* read all tracks */
  for (ulLoopCylinder = hlp_getCylinderStart( pDiskParams );
       ulLoopCylinder < hlp_getCylinderEnd( pDiskParams );
       ulLoopCylinder++)
  {
    pTrackLayout->usCylinder    = ulLoopCylinder;

    for (ulLoopHead = 0;
         ulLoopHead < pDiskParams->ulHeads;
         ulLoopHead++)
    {
      pTrackLayout->usHead        = ulLoopHead;
      
      // clear buffer memory in case of i/o errors
      memset(pBuffer,
             0,
             ulBufferLength);
      
      rc = DosDevIOCtl(hDisk,                               /* Device-Handle */
                       IOCTL_PHYSICALDISK,
                       PDSK_READPHYSTRACK,
                       pTrackLayout,
                       ulTrackLayoutSize,
                       &ulRetTrackLayoutSize,
                       pBuffer,
                       ulBufferLength,
                       &ulRetBufferLength);
      
      if (rc != NO_ERROR)                                /* check for errors */
        ToolsErrorDosEx(rc,
                        "DosDevIOCtl(PDSK_READPHYSTRACK)");
    
      // dump data on stdout!
      rc = DosWrite( 1, // HF_STDOUT
                    pBuffer,
                    ulBufferLength,
                    &ulBytesWritten);
      if (rc != NO_ERROR)                                /* check for errors */
        ToolsErrorDosEx(rc,
                        "DosWrite(1)");
    }
  }

  free (pTrackLayout);
  free (pBuffer);

  return (NO_ERROR);                                                   /* OK */
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

  setvbuf(stdout,                                /* disable stream buffering */
          NULL,
          _IONBF,
          0);
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
  int rc;                                                    /* RÅckgabewert */

  DosError (FERR_DISABLEHARDERR);                       /* Popups anschalten */

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

  if (Options.fsHelp)                                    /* help requested ? */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }


  DosError (FERR_DISABLEHARDERR);                       /* Popups anschalten */

  if (Options.fsFlushCache == FALSE)                       /* Caches leeren ? */
  {
    DisplayText ("\nFlushing caches ...");                  /* Caches leeren */
    rc = DskCacheFlush ();
    if (rc != NO_ERROR)                              /* Fehler aufgetreten ? */
      ToolsErrorDosEx(rc,
                      "DskCacheFlush");
  }
  

  /* determine operation mode */
  if (Options.fsCmdLogical)
  {
    fprintf(stderr,
            "ERROR: logical operation mode not yet implemented.\n");
  }
  else
  {
    DisplayText ("\nPhysical paritionable disks attached to the system:");
    rc = DiskScanPhysicalDisks();
    DisplayText ("\n");
  }

  return rc;
}
