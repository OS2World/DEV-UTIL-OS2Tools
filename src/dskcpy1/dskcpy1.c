/***********************************************************************
 * Name      : Module DiskCopy
 * Funktion  : Kopieren eines DASD Volumes
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

  ARGFLAG fsSource;                                  
  ARGFLAG fsDestination;                             
  UCHAR   ucSource;                                  
  UCHAR   ucDestination;                             
  
  ARGFLAG fsCylinderStart;
  ULONG   ulCylinderStart;
  
  ARGFLAG fsCmdCopy;                            /* copy or comapre operation */
  ARGFLAG fsConfirmationSkip;         /* no prompts for dangerous operations */

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
APIRET DiskScanInformation        ( UCHAR               ucDrive );


APIRET DskDiskCopy(HFILE       hdskSource,
                   HFILE       hdskDest,
                   PDISKPARAMS pDiskParams);
APIRET DskDiskCompare(HFILE       hdskSource1,
                      HFILE       hdskSource2,
                      PDISKPARAMS pDiskParams);

/*===Globale Strukturen======================================================*/

OPTIONS Options;

ARGUMENT TabArguments[] =
{ /*Token--Beschreibung--pTarget--ucTargetFormat--pTargetSpecified--*/
  {"/SOURCE=",    "Source physical disk (1, 2, ..)",       &Options.ucSource,         ARG_UCHAR 
                                                                                      | ARG_MUST, &Options.fsSource},
  {"/DEST=",      "Destination physical disk (1, 2, ..)",  &Options.ucDestination,    ARG_UCHAR      
                                                                                      | ARG_MUST, &Options.fsDestination},
  {"/STARTCYL=",  "Starting cylinder (default 0)",         &Options.ulCylinderStart,  ARG_ULONG,  &Options.fsCylinderStart},
  {"/COPY",       "Copy instead of compare operation",     NULL,                      ARG_NULL,   &Options.fsCmdCopy},
  {"/Y",          "Skip confirmations for dangerous operations.",NULL,                ARG_NULL,   &Options.fsConfirmationSkip},
  {"/V",          "Verbose mode, much output.",            NULL,                      ARG_NULL,   &Options.fsVerbose},
  {"/?",          "Get help screen.",                      NULL,                      ARG_NULL,   &Options.fsHelp},
  {"/H",          "Get help screen.",                      NULL,                      ARG_NULL,   &Options.fsHelp},
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
  TOOLVERSION("DiskCopy",                                /* application name */
              0x00010002,                            /* application revision */
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
  HFILE   hdskSource;
  HFILE   hdskDest;
  APIRET  rc;                                               /* Returncode */

  DISKPARAMS DiskParams;                  /* Angaben zur Plattengeometrie */
  ULONG      ulDriveLast;
  
  
  /* obtain source handle */
  rc = DskDiskGetIOCtlHandle(Options.ucSource,
                             &hdskSource);              /* Handle erfragen */
  if (rc != NO_ERROR)                              /* Fehler aufgetreten ? */
  {
    ToolsErrorDosEx(rc,
                    "DskDiskGetIOCtlHandle(source)");
    
    return rc;
  }
  
  
  /* obtain target handle */
  rc = DskDiskGetIOCtlHandle(Options.ucDestination,
                             &hdskDest);                /* Handle erfragen */
  if (rc != NO_ERROR)                              /* Fehler aufgetreten ? */
  {
    ToolsErrorDosEx(rc,
                    "DskDiskGetIOCtlHandle(source)");
    
    return rc;
  }
  
  
  /* source info */
  rc = DskDiskGetPhysicalParameters (hdskSource,            /* Infos erfragen */
                                     &DiskParams);
  if (rc != NO_ERROR)                             /* Fehler aufgetreten ? */
    DisplayText(" DskDiskGetPhysicalParameters error #%u\n",
                rc);
  else
    DisplayText ("Source      Disk %u (%08xh): %5u Cyl, %2u Heads, %3u Sec/Trk.",    /* Info ausgeben */
                 Options.ucSource,
                 hdskSource,
                 DiskParams.ulCylinders,
                 DiskParams.ulHeads,
                 DiskParams.ulSectorsPerTrack);

  rc = DskPhysDiskLock (hdskSource);    /* PrÅfen, ob Lock auf Disk moeglich */
  if (rc != NO_ERROR)
    DisplayText (" can't lock (%u),\n",
                 rc);                      /* kann nicht gelockt werden */
  else
    DisplayText (" lock OK,\n");                             /* kann gelockt werden */
  
  
  /* destination info */
  rc = DskDiskGetPhysicalParameters (hdskDest,            /* Infos erfragen */
                                     &DiskParams);
  if (rc != NO_ERROR)                             /* Fehler aufgetreten ? */
    DisplayText(" DskDiskGetPhysicalParameters error #%u\n",
                rc);
  else
    DisplayText ("Destination Disk %u (%08xh): %5u Cyl, %2u Heads, %3u Sec/Trk.",    /* Info ausgeben */
                 Options.ucDestination,
                 hdskDest,
                 DiskParams.ulCylinders,
                 DiskParams.ulHeads,
                 DiskParams.ulSectorsPerTrack);

  rc = DskPhysDiskLock (hdskDest);    /* PrÅfen, ob Lock auf Disk moeglich */
  if (rc != NO_ERROR)
    DisplayText (" can't lock (%u),\n",
                 rc);                      /* kann nicht gelockt werden */
  else
    DisplayText (" lock OK\n");                             /* kann gelockt werden */
  
  /* do the operation */
  /* Note: we call the copy operation with the disk parameters of the source drive!
   * Therefore, the target drive is supposedly be larger than the source
   */
  if (Options.fsCmdCopy)
    rc = DskDiskCopy(hdskSource,
                     hdskDest,
                     &DiskParams);
  else
    rc = DskDiskCompare(hdskSource,
                        hdskDest,
                        &DiskParams);
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);
  
  
  /* unlock source */
  rc = DskPhysDiskUnlock (hdskSource);                     /* Unlock prÅfen */
  if (rc != NO_ERROR)
    DisplayText ("Source: can't unlock (%u).\n", rc);
  else
    DisplayText ("Source: unlock OK.\n");

  rc = DskDiskFreeIOCtlHandle(hdskSource);         /* Handle wieder freigeben */
  if (rc != NO_ERROR)
    ToolsErrorDosEx(rc,
                    "DskDiskFreeIOCtlHandle");
  
  /* unlock destination */
  rc = DskPhysDiskUnlock (hdskDest);                     /* Unlock prÅfen */
  if (rc != NO_ERROR)
    DisplayText ("Destination: can't unlock (%u).\n", rc);
  else
    DisplayText ("Destination: unlock OK.\n");

  rc = DskDiskFreeIOCtlHandle(hdskDest);         /* Handle wieder freigeben */
  if (rc != NO_ERROR)
    ToolsErrorDosEx(rc,
                    "DskDiskFreeIOCtlHandle");
  
  return (rc);                                 /* RÅckgabewert liefern */
}



/***********************************************************************
 * Name      : APIRET DskDiskCopy
 * Funktion  : Copies the physical drive
 * Parameter : HFILE hDisk
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 02.27.30]
 ***********************************************************************/

APIRET DskDiskCopy(HFILE       hdskSource,
                   HFILE       hdskDest,
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
  
  int        iAnswer;

  if (pDiskParams == NULL)                               /* check parameters */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */
  
  
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

  for (ulLoopCylinder = Options.ulCylinderStart;
       ulLoopCylinder < pDiskParams->ulCylinders;
       ulLoopCylinder++)
  {
    pTrackLayout->usCylinder    = ulLoopCylinder;

    ToolsPerfQuery(&psStartTrk);                             /* benchmarking */

    for (ulLoopHead = 0;
         ulLoopHead < pDiskParams->ulHeads;
         ulLoopHead++)
    {
      pTrackLayout->usHead        = ulLoopHead;

      rc = DosDevIOCtl(hdskSource,                          /* Device-Handle */
                       IOCTL_PHYSICALDISK,
                       PDSK_READPHYSTRACK,
                       pTrackLayout,
                       ulTrackLayoutSize,
                       &ulRetTrackLayoutSize,
                       pBuffer,
                       ulBufferLength,
                       &ulRetBufferLength);
      if (rc != NO_ERROR)
      {
        /* clear the buffer in case of read errors */
        memset(pBuffer,
               0,
               ulBufferLength);
      }

      if (rc != NO_ERROR)                                /* check for errors */
        fprintf(stderr,
                "\nREAD ERROR on handle %08xh at cylinder %d, head %d, error %d\n",
                hdskSource,
                ulLoopCylinder,
                ulLoopHead,
                rc);
    
    
      /* OK, now send the read data to the target device */
      rc = DosDevIOCtl(hdskDest,                            /* Device-Handle */
                       IOCTL_PHYSICALDISK,
                       PDSK_WRITEPHYSTRACK,
                       pTrackLayout,
                       ulTrackLayoutSize,
                       &ulRetTrackLayoutSize,
                       pBuffer,
                       ulBufferLength,
                       &ulRetBufferLength);

      if (rc != NO_ERROR)                                /* check for errors */
        fprintf(stderr,
                "\nWRITE ERROR on handle %08xh at cylinder %d, head %d, error %d\n",
                hdskDest,
                ulLoopCylinder,
                ulLoopHead,
                rc);
    }

    ToolsPerfQuery(&psEndTrk);                               /* benchmarking */
    fTimeTrk = psEndTrk.fSeconds - psStartTrk.fSeconds;

    fSpeedCurrent = pDiskParams->ulHeads *
                    pDiskParams->ulSectorsPerTrack / 2 / fTimeTrk;

    if (fSpeedCurrent < fSpeedMin) fSpeedMin = fSpeedCurrent;
    if (fSpeedCurrent > fSpeedMax) fSpeedMax = fSpeedCurrent;

    printf ("\rCopying cylinder %5u, %10.3f/%10.3f/%10.3f kb/s ... ",
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
 * Name      : APIRET DskDiskCompare
 * Funktion  : Compare the physical drive
 * Parameter : HFILE hDisk
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 02.27.30]
 ***********************************************************************/

APIRET DskDiskCompare(HFILE       hdskSource1,
                      HFILE       hdskSource2,
                      PDISKPARAMS pDiskParams)
{
  APIRET     rc;                                           /* API returncode */
  PVOID      pBuffer;                               /* buffer for the tracks */
  PVOID      pBuffer2;                              /* buffer for the tracks */
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
  
  int        iAnswer;

  if (pDiskParams == NULL)                               /* check parameters */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */
  
  
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
  
  pBuffer2 = (PVOID) malloc(ulBufferLength);          /* allocate that memory */
  if (pBuffer2 == NULL)                            /* check proper allocation */
  {
    free(pBuffer);
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

  for (ulLoopCylinder = Options.ulCylinderStart;
       ulLoopCylinder < pDiskParams->ulCylinders;
       ulLoopCylinder++)
  {
    pTrackLayout->usCylinder    = ulLoopCylinder;

    ToolsPerfQuery(&psStartTrk);                             /* benchmarking */

    for (ulLoopHead = 0;
         ulLoopHead < pDiskParams->ulHeads;
         ulLoopHead++)
    {
      pTrackLayout->usHead        = ulLoopHead;

      rc = DosDevIOCtl(hdskSource1,                         /* Device-Handle */
                       IOCTL_PHYSICALDISK,
                       PDSK_READPHYSTRACK,
                       pTrackLayout,
                       ulTrackLayoutSize,
                       &ulRetTrackLayoutSize,
                       pBuffer,
                       ulBufferLength,
                       &ulRetBufferLength);

      if (rc != NO_ERROR)                                /* check for errors */
        fprintf(stderr,
                "\nREAD ERROR on handle %08xh at cylinder %d, head %d, error %d\n",
                hdskSource1,
                ulLoopCylinder,
                ulLoopHead,
                rc);
    
      /* OK, now send the read data to the target device */
      rc = DosDevIOCtl(hdskSource2,                         /* Device-Handle */
                       IOCTL_PHYSICALDISK,
                       PDSK_READPHYSTRACK,
                       pTrackLayout,
                       ulTrackLayoutSize,
                       &ulRetTrackLayoutSize,
                       pBuffer2,
                       ulBufferLength,
                       &ulRetBufferLength);

      if (rc != NO_ERROR)                                /* check for errors */
        fprintf(stderr,
                "\nREAD ERROR on handle %08xh at cylinder %d, head %d, error %d\n",
                hdskSource2,
                ulLoopCylinder,
                ulLoopHead,
                rc);
      
      /* compare the buffer */
      if (memcmp(pBuffer,
                 pBuffer2,
                 ulBufferLength) != 0)
      {
        fprintf(stderr,
                "\nDATA MISMATCH ERROR at cylinder %d, head %d\n",
                ulLoopCylinder,
                ulLoopHead);
      }
    }

    ToolsPerfQuery(&psEndTrk);                               /* benchmarking */
    fTimeTrk = psEndTrk.fSeconds - psStartTrk.fSeconds;

    fSpeedCurrent = pDiskParams->ulHeads *
                    pDiskParams->ulSectorsPerTrack / 2 / fTimeTrk;

    if (fSpeedCurrent < fSpeedMin) fSpeedMin = fSpeedCurrent;
    if (fSpeedCurrent > fSpeedMax) fSpeedMax = fSpeedCurrent;

    printf ("\rCopying cylinder %5u, %10.3f/%10.3f/%10.3f kb/s ... ",
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

  rc = DskCacheFlush ();
  if (rc != NO_ERROR)                              /* Fehler aufgetreten ? */
    ToolsErrorDosEx(rc,
                    "DskCacheFlush");
  

  /* determine operation mode */
  rc = DiskScanPhysicalDisks();

  return rc;
}
