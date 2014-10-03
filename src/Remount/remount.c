/***********************************************************************
 * Name      : Module Remount
 * Funktion  : Remount, erweiterter Support fÅr Syquests :)
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


typedef struct
{
  ARGFLAG fsVerbose;                    /* AusfÅhrliche Meldungen ausgeben ? */
  ARGFLAG fsDebug;                /* AusfÅhrliche Debug-Meldungen ausgeben ? */
  ARGFLAG fsHelp;                 /* Muss Hilfestellung dargestellt werden ? */
  ARGFLAG fsErrors;                            /* laengere Fehlermeldungen ? */
  ARGFLAG fsRemountAll;                  /* Remount all found logical drives */
  ARGFLAG fsRemountDrives;                   /* Remount these logical drives */
  ARGFLAG fsFlushCache;                               /* Flush caches before */
  PSZ     pszRemountDrives;                          /* Remount these drives */
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

APIRET DskLogGetIOCtlHandle      ( UCHAR               ulDrive,
                                   PHFILE              pulHandle );
APIRET DskLogMapQuery            ( HFILE               hDisk,
                                   UCHAR               ucDrive,
                                   PUCHAR              pucDriveLast );
APIRET DskLogLockStatus          ( HFILE               hDisk,
                                   USHORT              usDisk,
                                   PUSHORT             pusLockStatus );
APIRET DskLogUnlock              ( HFILE               hDisk );
APIRET DskLogLock                ( HFILE               hDisk );
APIRET DskLogRedetermine         ( HFILE               hDisk );
APIRET DskLogGetParams           ( HFILE               hDisk,
                                   USHORT              usDisk,
                                   PBIOSPARAMETERBLOCK pDiskLogParams );
APIRET DskLogSetParams           ( HFILE               hDisk,
                                   USHORT              usDisk,
                                   PBIOSPARAMETERBLOCK pDiskLogParams );
APIRET DskCacheFlush             ( VOID );
APIRET DskLogSetRemoveable       ( HFILE               hDisk,
                                   USHORT              usDisk,
                                   UCHAR               ucRemoveable );
APIRET DskLogEject               ( HFILE               hDisk,
                                  USHORT              usDisk );
APIRET RemountPhysicalDisks      ( VOID );
APIRET RemountLogicalDisks       ( PSZ                 pszDrives );
APIRET RemountInformation        ( UCHAR               ucDrive );
APIRET SearchDrives              ( PSZ                 pszReturnString );


/*===Globale Strukturen======================================================*/

OPTIONS Options;

ARGUMENT TabArguments[] =
{ /*Token--Beschreibung--pTarget--ucTargetFormat--pTargetSpecified--*/
  {"/D=",      "Remount these logical drives. /D=CDEF",  &Options.pszRemountDrives, ARG_PSZ,  &Options.fsRemountDrives},
  {"/ALL",     "Remount all logical drives.",            NULL,                      ARG_NULL, &Options.fsRemountAll},
  {"/NOFLUSH", "Don't flush the system caches.",         NULL,                      ARG_NULL, &Options.fsFlushCache},
  {"/ERR",     "Descriptive error messages.",            NULL,                      ARG_NULL, &Options.fsErrors},
  {"/DEBUG",   "Debug mode, much output.",               NULL,                      ARG_NULL, &Options.fsDebug},
  {"/V",       "Verbose mode, much output.",             NULL,                      ARG_NULL, &Options.fsVerbose},
  {"/?",       "Get help screen.",                       NULL,                      ARG_NULL, &Options.fsHelp},
  {"/H",       "Get help screen.",                       NULL,                      ARG_NULL, &Options.fsHelp},
  ARG_TERMINATE
};


const PSZ TabDeviceTypes[] =
{/* Describes the physical layout of the device specified,
    and has one of the following values: */
  /* 0 */ "48 TPI low-density diskette drive",
  /* 1 */ "96 TPI high-density diskette drive",
  /* 2 */ "3.5-inch 720KB diskette drive",
  /* 3 */ "8-Inch single-density diskette drive",
  /* 4 */ "8-Inch double-density diskette drive",
  /* 5 */ "Fixed disk",
  /* 6 */ "Tape drive",
  /* 7 */ "Other (includes 1.44MB 3.5-inch diskette drive)",
  /* 8 */ "R/W optical disk",
  /* 9 */ "3.5-inch 4.0MB diskette drive (2.88MB formatted)"
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
  TOOLVERSION("ReMount",                                 /* application name */
              0x00010000,                            /* application revision */
              0x00010100,           /* minimum required revision of PHSTOOLS */
              0x0001FFFF,        /* maximum recommended revision of PHSTOOLS */
              NULL,                                                /* Remark */
              NULL);                                 /* additional copyright */
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

  memset (pszParam,0,ulParamLen);
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
 * Name      : APIRET DskLogGetIOCtlHandle
 * Funktion  : Allokiert ein IOCtl-Handle fÅr eine Partition.
 * Parameter : UCHAR ucDrive ('A','B','C',...) , PHFILE pulHandle
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 03.37.08]
 ***********************************************************************/

APIRET DskLogGetIOCtlHandle (UCHAR ucDrive, PHFILE pulHandle)
{
  APIRET rc = NO_ERROR;                                /* RÅckgabewert */
  PSZ    pszDrive="C:";                 /* Puffer fÅr den Drive-String */
  ULONG  ulAction;                        /* Dummyvariable fÅr DosOpen */


  if ( pulHandle == NULL )                     /* ParameterÅberprÅfung */
    return (ERROR_INVALID_PARAMETER);

  /* Parameterblock zusammenbauen */
  pszDrive[0] = ucDrive;
  rc = DosOpen(pszDrive,
                pulHandle,
                &ulAction,
                0,
                FILE_NORMAL,
                FILE_OPEN,
                OPEN_ACCESS_READONLY | OPEN_FLAGS_DASD | OPEN_SHARE_DENYNONE,
                NULL);


  return (rc);                                 /* RÅckgabewert liefern */
} /* APIRET DskLogGetIOCtlHandle */


/***********************************************************************
 * Name      : APIRET DskLogMapQuery
 * Funktion  : Ermittelt der Mapping der Partitionen und logischen Laufwerke.
 * Parameter : HFILE hDisk, ULONG ulDrive
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 03.43.29]
 ***********************************************************************/

APIRET DskLogMapQuery (HFILE hDisk, UCHAR ucDrive, PUCHAR pucDriveLast)
{
  APIRET rc = NO_ERROR;                                /* RÅckgabewert */
  UCHAR  ucLogDrive = ucDrive - 'A' + 1;  /* Nummer logisches Laufwerk */
  ULONG  ulParamLen = sizeof(ucLogDrive);     /* LÑnge Parameterblocke */

  if ( pucDriveLast == NULL )                  /* ParameterÅberprÅfung */
    return (ERROR_INVALID_PARAMETER);

  rc = DskGenericIO (hDisk,                         /* IOCtl ausfÅhren */
                     IOCTL_DISK,
                     DSK_GETLOGICALMAP,
                     ulParamLen,
                     &ulParamLen,
                     &ucLogDrive);

  /* Ergebnis in die Zielvariable schreiben */
  if (ucLogDrive != 0)                      /* Liegt ein Mapping vor ? */
    *pucDriveLast = ucLogDrive + 'A' - 1;
  else
    *pucDriveLast = 0;

  return (rc);                                 /* RÅckgabewert liefern */
} /* APIRET DskLogMapQuery */



/***********************************************************************
 * Name      : APIRET DskLogMapSet
 * Funktion  : Setzt das Mapping der Partitionen und logischen Laufwerke.
 * Parameter : HFILE hDisk, ULONG ulDriveNew
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 04.23.21]
 ***********************************************************************/

APIRET DskLogMapSet (HFILE hDisk, UCHAR ucDriveNew)
{
  APIRET rc = NO_ERROR;                                /* RÅckgabewert */
  UCHAR  ucLogDrive = ucDriveNew - 'A' + 1;  /* Nummer logisches Laufwerk */
  ULONG  ulParamLen = sizeof(ucLogDrive);     /* LÑnge Parameterblocke */

  rc = DskGenericIO (hDisk,                         /* IOCtl ausfÅhren */
                     IOCTL_DISK,
                     DSK_SETLOGICALMAP,
                     ulParamLen,
                     &ulParamLen,
                     &ucLogDrive);

/* Return auch auswerten */

  return (rc);                                 /* RÅckgabewert liefern */
} /* APIRET DskLogMapSet */


/***********************************************************************
 * Name      : APIRET DskLogLock
 * Funktion  : Allokiert exklusiven Zugriff auf das Volume.
 * Parameter : HFILE hDisk
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 04.32.37]
 ***********************************************************************/

APIRET DskLogLock (HFILE hDisk)
{
  APIRET rc = NO_ERROR;                                /* RÅckgabewert */
  UCHAR  ucParam = 0;                                /* Parameterblock */
  ULONG  ulParamLen = sizeof(ucParam);   /* LÑnge des Parameterblockes */

  rc = DskGenericIO ( hDisk,                  /* Filesystem blockieren */
                      IOCTL_DISK,
                      DSK_LOCKDRIVE,
                      ulParamLen,
                      &ulParamLen,
                      &ucParam );

  return (rc);                                 /* RÅckgabewert liefern */
} /* APIRET DskLogLock */


/***********************************************************************
 * Name      : APIRET DskLogUnlock
 * Funktion  : Freigeben des exklusiven Zugriff auf das Volume.
 * Parameter : HFILE hDisk
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 04.32.37]
 ***********************************************************************/

APIRET DskLogUnlock (HFILE hDisk)
{
  APIRET rc = NO_ERROR;                                /* RÅckgabewert */
  UCHAR  ucParam = 0;                                /* Parameterblock */
  ULONG  ulParamLen = sizeof(ucParam);   /* LÑnge des Parameterblockes */

  rc = DskGenericIO ( hDisk,                  /* Filesystem blockieren */
                      IOCTL_DISK,
                      DSK_UNLOCKDRIVE,
                      ulParamLen,
                      &ulParamLen,
                      &ucParam );

  return (rc);                                 /* RÅckgabewert liefern */
} /* APIRET DskLogUnlock */


/***********************************************************************
 * Name      : APIRET DskLogRedetermine
 * Funktion  : Evaluiert das Filesystem des Mediums neu. (Wechsel FAT<>HPFS, etc).
 * Parameter : HFILE hDisk
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 04.32.37]
 ***********************************************************************/

APIRET DskLogRedetermine (HFILE hDisk)
{
  APIRET rc = NO_ERROR;                                /* RÅckgabewert */
  UCHAR  ucParam = 0;                                /* Parameterblock */
  ULONG  ulParamLen = sizeof(ucParam);   /* LÑnge des Parameterblockes */

  rc = DskLogLock (hDisk);           /* Wir brauchen erst einen Lock ! */
  if (rc != NO_ERROR)                  /* Ist ein Fehler aufgetreten ? */
    return (rc);

  rc = DskGenericIO ( hDisk,              /* Filesystem neu evaluieren */
                      IOCTL_DISK,
                      DSK_REDETERMINEMEDIA,
                      ulParamLen,
                      &ulParamLen,
                      &ucParam );
  if (rc != NO_ERROR)                  /* Ist ein Fehler aufgetreten ? */
    return (rc);

  rc = DskLogUnlock (hDisk);                /* Volume wieder freigeben */

  return (rc);                                 /* RÅckgabewert liefern */
} /* APIRET DskLogRedetermine */



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
 * Name      : APIRET DskLogLockStatus
 * Funktion  : Erfragt Infos Åber Lock/Eject-FunktionalitÑt
 * Parameter :
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 04.32.37]
 ***********************************************************************/

APIRET DskLogLockStatus (HFILE   hDisk,
                         USHORT  usDisk,
                         PUSHORT pusLockStatus)
{
  APIRET          rc = NO_ERROR;                       /* RÅckgabewert */
  ULONG           ulParmRetLen;                 /* ParameterpaketlÑnge */
  ULONG           ulDataRetLen;                     /* DatenblocklÑnge */
  USHORT          usLockStatus = 0;       /* Ergebnis des DosDevIOCtls */
  DISKPARAMBLOCK  LockParameterBlock;           /* Struktur fÅr IOCtls */


  if (pusLockStatus == NULL)                   /* ParameterÅberprÅfung */
    return (ERROR_INVALID_PARAMETER);          /* Fehler signalisieren */

                     /* Nun den Status der Locking-Funktionen erfragen */
  LockParameterBlock.ucCommand = 0;
  LockParameterBlock.usDisk    = usDisk;
  ulParmRetLen                 = sizeof(LockParameterBlock);
  ulDataRetLen                 = sizeof(usLockStatus);

  rc = DosDevIOCtl(hDisk,                             /* Device-Handle */
         IOCTL_DISK,                                       /* Category */
         DSK_GETLOCKSTATUS,                                /* Function */
         &LockParameterBlock,                /* Parameterblock-Pointer */
         sizeof(LockParameterBlock), /* Max. LÑnge der Parameterblocks */
         &ulParmRetLen,       /* Pointer auf LÑnge des Parameterblocks */
         &usLockStatus,                                  /* Datenblock */
         sizeof (usLockStatus),      /* Maximale LÑnge des Datenblocks */
         &ulDataRetLen);          /* Pointer auf LÑnge des Datenblocks */

  *pusLockStatus = usLockStatus;                    /* Ergebnis melden */

  return (rc);                                   /* RÅckgabewert liefern */
}


/***********************************************************************
 * Name      : APIRET DskLogGetParams
 * Funktion  : Erfragt Infos Åber das logische GerÑt
 * Parameter :
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 04.32.37]
 ***********************************************************************/

APIRET DskLogGetParams (HFILE         hDisk,
                        USHORT        usDisk,
                        PBIOSPARAMETERBLOCK pDiskLogParams)
{
  APIRET          rc = NO_ERROR;                       /* RÅckgabewert */
  ULONG           ulParmRetLen;                 /* ParameterpaketlÑnge */
  ULONG           ulDataRetLen;                     /* DatenblocklÑnge */
  DISKPARAMBLOCK  ParameterBlock;               /* Struktur fÅr IOCtls */
  BIOSPARAMETERBLOCK DiskLogParams;     /* Parameter der logischen Platte */

  if (pDiskLogParams == NULL)                  /* ParameterÅberprÅfung */
    return (ERROR_INVALID_PARAMETER);          /* Fehler signalisieren */

                     /* Nun den Status der Locking-Funktionen erfragen */
  ParameterBlock.ucCommand = 0;
  ParameterBlock.usDisk    = usDisk;
  ulParmRetLen             = sizeof(ParameterBlock);
  ulDataRetLen             = sizeof(DiskLogParams);

  rc = DosDevIOCtl(hDisk,                             /* Device-Handle */
         IOCTL_DISK,                                       /* Category */
         DSK_GETDEVICEPARAMS,                              /* Function */
         &ParameterBlock,                    /* Parameterblock-Pointer */
         sizeof(ParameterBlock),     /* Max. LÑnge der Parameterblocks */
         &ulParmRetLen,       /* Pointer auf LÑnge des Parameterblocks */
         &DiskLogParams,                                 /* Datenblock */
         sizeof (DiskLogParams),     /* Maximale LÑnge des Datenblocks */
         &ulDataRetLen);          /* Pointer auf LÑnge des Datenblocks */

  memcpy (pDiskLogParams,&DiskLogParams,ulDataRetLen); /* Daten kopieren */
  return (rc);                                   /* RÅckgabewert liefern */
}


/***********************************************************************
 * Name      : APIRET DskLogSetParams
 * Funktion  : Setzt Infos Åber das logische GerÑt
 * Parameter :
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 04.32.37]
 ***********************************************************************/

APIRET DskLogSetParams (HFILE               hDisk,
                        USHORT              usDisk,
                        PBIOSPARAMETERBLOCK pDiskLogParams)
{
  APIRET          rc = NO_ERROR;                       /* RÅckgabewert */
  ULONG           ulParmRetLen;                 /* ParameterpaketlÑnge */
  ULONG           ulDataRetLen;                     /* DatenblocklÑnge */
  DISKPARAMBLOCK  ParameterBlock;               /* Struktur fÅr IOCtls */

  if (pDiskLogParams == NULL)                  /* ParameterÅberprÅfung */
    return (ERROR_INVALID_PARAMETER);          /* Fehler signalisieren */

                                     /* Nun die GerÑteparameter setzen */
  ParameterBlock.ucCommand = 0;
  ParameterBlock.usDisk    = usDisk;
  ulParmRetLen             = sizeof(ParameterBlock);
  ulDataRetLen             = sizeof(BIOSPARAMETERBLOCK);

  rc = DosDevIOCtl(hDisk,                             /* Device-Handle */
         IOCTL_DISK,                                       /* Category */
         DSK_SETDEVICEPARAMS,                              /* Function */
         &ParameterBlock,                    /* Parameterblock-Pointer */
         sizeof(ParameterBlock),     /* Max. LÑnge der Parameterblocks */
         &ulParmRetLen,       /* Pointer auf LÑnge des Parameterblocks */
         pDiskLogParams,                                 /* Datenblock */
         ulDataRetLen,               /* Maximale LÑnge des Datenblocks */
         &ulDataRetLen);          /* Pointer auf LÑnge des Datenblocks */

  return (rc);                                 /* RÅckgabewert liefern */
}


/***********************************************************************
 * Name      : APIRET DskLogSetRemoveable
 * Funktion  : Setzt/Lîscht Removeable-Bit eines Logischen GerÑtes
 * Parameter :
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung : * Haut voll daneben ... :( *
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 04.32.37]
 ***********************************************************************/

APIRET DskLogSetRemoveable (HFILE  hDisk,
                            USHORT usDisk,
                            UCHAR  ucRemoveable)
{
  APIRET              rc = NO_ERROR;                   /* RÅckgabewert */
  BIOSPARAMETERBLOCK  ParameterBlock;           /* Struktur fÅr IOCtls */

  rc = DskLogGetParams (hDisk,                   /* GerÑtestatus holen */
                        usDisk,
                        &ParameterBlock);
  if (rc != NO_ERROR)                          /* Fehler aufgetreten ? */
    return (rc);                               /* Fehler signalisieren */

                                         /* Das wichtige Bit setzen :) */
  if (ucRemoveable == 0)
  {
    ParameterBlock.fsDeviceAttr &= 0xFFFE;              /* Bit lîschen */
  }
  else
  {
    ParameterBlock.fsDeviceAttr |= 0x0001;               /* Bit setzen */
  }

  rc = DskLogSetParams (hDisk,                  /* GerÑtestatus setzen */
                        usDisk,
                        &ParameterBlock);  return (rc);                                 /* RÅckgabewert liefern */}


/***********************************************************************
 * Name      : APIRET DskLogSetRemoveable
 * Funktion  : Setzt/Lîscht Removeable-Bit eines Logischen GerÑtes
 * Parameter :
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 04.32.37]
 ***********************************************************************/

APIRET DskLogEject (HFILE  hDisk,
                    USHORT usDisk)
{
  APIRET          rc = NO_ERROR;                       /* RÅckgabewert */
  ULONG           ulParmRetLen;                 /* ParameterpaketlÑnge */
  ULONG           ulDataRetLen;                     /* DatenblocklÑnge */
  DISKPARAMBLOCK  ParameterBlock;               /* Struktur fÅr IOCtls */


                     /* Nun den Status der Locking-Funktionen erfragen */
  ParameterBlock.ucCommand = 0;                              /* Unlock */
  ParameterBlock.usDisk    = usDisk;
  ulParmRetLen             = sizeof(ParameterBlock);
  ulDataRetLen             = 0;

/*   rc = DskLogSetRemoveable (hDisk, */
/*                            usDisk, */
/*                            TRUE);  */
/*  printf ("\nrem=%u",rc);           */

  rc = DosDevIOCtl(hDisk,                             /* Device-Handle */
         IOCTL_DISK,                                       /* Category */
         DSK_UNLOCKEJECTMEDIA,                             /* Function */
         &ParameterBlock,                /* Parameterblock-Pointer */
         sizeof(ParameterBlock), /* Max. LÑnge der Parameterblocks */
         &ulParmRetLen,       /* Pointer auf LÑnge des Parameterblocks */
         NULL,                                          /* Datenblock */
         ulDataRetLen,               /* Maximale LÑnge des Datenblocks */
         &ulDataRetLen);          /* Pointer auf LÑnge des Datenblocks */

  printf ("\nunlock=%u",rc);

                     /* Nun den Status der Locking-Funktionen erfragen */
  ParameterBlock.ucCommand = 0x02;                            /* Eject */
  ParameterBlock.usDisk    = usDisk;
  ulParmRetLen             = sizeof(ParameterBlock);
  ulDataRetLen             = 0;

  rc = DosDevIOCtl(hDisk,                             /* Device-Handle */
         IOCTL_DISK,                                       /* Category */
         DSK_UNLOCKEJECTMEDIA,                             /* Function */
         &ParameterBlock,                /* Parameterblock-Pointer */
         sizeof(ParameterBlock), /* Max. LÑnge der Parameterblocks */
         &ulParmRetLen,       /* Pointer auf LÑnge des Parameterblocks */
         NULL,                                          /* Datenblock */
         ulDataRetLen,               /* Maximale LÑnge des Datenblocks */
         &ulDataRetLen);          /* Pointer auf LÑnge des Datenblocks */

  printf ("\neject=%u",rc);

  return (rc);                                 /* RÅckgabewert liefern */
}

/***********************************************************************
 * Name      : APIRET RemountLogicalParams
 * Funktion  : Infos ueber logical disks
 * Parameter :
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 04.32.37]
 ***********************************************************************/
APIRET RemountLogicalParams (HFILE  hDisk,
                             USHORT usDisk)
{
  APIRET       rc = NO_ERROR;                          /* RÅckgabewert */
  BIOSPARAMETERBLOCK DiskLogParams;

  rc = DskLogGetParams (hDisk,                          /* Infos holen */
                        usDisk,
                        &DiskLogParams);

  if (rc == NO_ERROR)                          /* Fehler aufgetreten ? */
  {
     if (Options.fsVerbose == TRUE)        /* AusfÅhrliche Meldungen ? */
     {
       DisplayText ("\n   %5u Bytes per sector,  %5u Sectors per cluster, %5u Sectors"
                    "\n   %5u sectors per track, %5u heads, Media descriptor: 0x%02x"
                    "\n   %5u reserved sectors, %3u FATs,                %5u root entries"
                    "\n   %5u sectors per FAT"
                    "\n   %u hidden sectors, %u large sectors",
                    DiskLogParams.usBytesPerSector,
                    DiskLogParams.bSectorsPerCluster,
                    DiskLogParams.cSectors,
                    DiskLogParams.usSectorsPerTrack,
                    DiskLogParams.cHeads,
                    DiskLogParams.bMedia,
                    DiskLogParams.usReservedSectors,
                    DiskLogParams.cFATs,
                    DiskLogParams.cRootEntries,
                    DiskLogParams.usSectorsPerFAT,
                    DiskLogParams.cHiddenSectors,
                    DiskLogParams.cLargeSectors);

      DisplayText ("\n   Cylinders: %5u\n   Type:%s",  /* Infos ausgeben */
                 DiskLogParams.cCylinders,
                 TabDeviceTypes[DiskLogParams.bDeviceType]);

      if (DiskLogParams.fsDeviceAttr & 0x0001)  /* Removeable Flag */
        DisplayText ("\n   Media cannot be removed.");
      else
        DisplayText ("\n   Media is removeable.");

      if (DiskLogParams.fsDeviceAttr & 0x0002)  /* Changeline Flag */
        DisplayText ("\n   Media change can be detected by device driver.");
      else
        DisplayText ("\n   Media change cannot be detected.");

      if (DiskLogParams.fsDeviceAttr & 0x0004)        /* 16MB Flag */
        DisplayText ("\n   Transfers above 16MB are supported.");
      else
        DisplayText ("\n   Transfers above 16MB are not supported.");
    }
  }
  else
    if (Options.fsDebug == TRUE)                        /* Debugmodus ? */
      ToolsErrorDos(rc);                       /* Fehlermeldung ausgeben */

  return (rc);                                 /* RÅckgabewert liefern */
}



/***********************************************************************
 * Name      : APIRET RemountLogicalStatus
 * Funktion  : Infos ueber logical disks
 * Parameter :
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 04.32.37]
 ***********************************************************************/
APIRET RemountLogicalStatus (HFILE  hDisk,
                             USHORT usDisk)
{
  APIRET rc           = NO_ERROR;                      /* RÅckgabewert */
  USHORT usLockStatus = 0;                /* Ergebnis des DosDevIOCtls */

  rc = DskLogLockStatus (hDisk, usDisk, &usLockStatus); /* Infos holen */
   if (rc == NO_ERROR)                         /* Fehler aufgetreten ? */
   {
             /* Statusinformationen zu Lock/Unlock/Eject und Medium ausgeben */
       DisplayText ("\n   ");
       switch (usLockStatus & 0x0003)  /* Untere 2 Bits untersuchen */
       {
         case 0: DisplayText ("Lock/Unlock/Eject/Status not supported."); break;
         case 1: DisplayText ("Drive locked. Lock/Unlock/Eject/Status supported."); break;
         case 2: DisplayText ("Drive unlocked. Lock/Unlock/Eject/Status supported."); break;
         case 3: DisplayText ("Lock status not supported. Lock/Unlock/Eject supported."); break;
       }

       if (usLockStatus & 0x0004)               /* Medium eingelegt ? */
         DisplayText (" Media inserted.");
       else
         DisplayText (" No media inserted.");
  }
  else
    if (Options.fsDebug == TRUE)                              /* Debugmodus ? */
      ToolsErrorDos(rc);                             /* Fehlermeldung ausgeben */

  return (rc);                                       /* RÅckgabewert liefern */
}

/***********************************************************************
 * Name      : APIRET RemountLogicalDisks
 * Funktion  : Infos ueber logical disks
 * Parameter :
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 04.32.37]
 ***********************************************************************/

       /* Mapping entfaellt. Mapping betrifft bis OS/2 3.x nur
          das Diskettenlaufwerk, welches als A: und B: auftreten kann.
       DisplayText ("\nQuery mapping: ");
       rc = DskLogMapQuery (hDisk, i, &ucDriveLast);
       if (rc != NO_ERROR) {
        DisplayText("DskLogMapQuery error: %u\n", rc);
       }
       else
         if (ucDriveLast == 0)
           DisplayText ("(%c) has no mapping.",i);
         else
           DisplayText ("(%c) maps to (%c).",i,ucDriveLast);
       */

  /* Mapping test */
/*
  Wenn z.B. Diskettenlaufwerk 0 einmal als A: und einmal als B: vorhanden
  ist, dann kann damit die Zuordnung gewechselt werden. Leider nicht
  mehr :(

  i = 'B';
  rc = DskLogGetIOCtlHandle (i, &hDisk);
  if (rc != NO_ERROR) {
    DisplayText("DskLogGetIOCtlHandle error: %u\n", rc);
  }
       DisplayText ("\nQuery mapping: ");
       rc = DskLogMapQuery (hDisk, i, &ucDriveLast);
       if (rc != NO_ERROR) {
        DisplayText("DskLogMapQuery error: %u\n", rc);
       }
       else
         if (ucDriveLast == 0)
           DisplayText ("(%c) has no mapping.",i);
         else
           DisplayText ("(%c) maps to (%c).",i,ucDriveLast);
  rc = DskLogMapSet (hDisk,'A');
       if (rc != NO_ERROR) {
        DisplayText("DskLogMapSet error: %u\n", rc);
       }
*/


APIRET RemountLogicalDisks (PSZ pszDrives)
{
  UINT   i;                                           /* Schleifenzaehler */
  UINT   ulStrLen;                                    /* Schleifenzaehler */
  ULONG  hDisk;                                             /* Diskhandle */
  APIRET rc;                                              /* RÅckgabewert */
  UCHAR  ucDrive;                      /* Nummer des aktuellen Laufwerkes */
  UCHAR  ucDriveLast;

  if (pszDrives == NULL)                          /* ParameterÅberprÅfung */
    return (ERROR_INVALID_PARAMETER);             /* Fehler signalisieren */

  ulStrLen = strlen(pszDrives);         /* Wieviele Laufwerke angegeben ? */
  if (ulStrLen == 0)                               /* Brauchbare Anzahl ? */
    return (ERROR_INVALID_PARAMETER);             /* Fehler signalisieren */

  strupr(pszDrives);                        /* In Grossbuchstaben wandeln */

  for (i = 0;                      /* Alle logischen Laufwerke bearbeiten */
       i < ulStrLen;
       i++)
  {
     ucDrive = pszDrives[i];                        /* Aktuelles Laufwerk */
     DisplayText ("\n\n%c: ",ucDrive);

     RemountInformation(ucDrive);                       /* Infos ausgeben */

     rc = DskLogGetIOCtlHandle (ucDrive, &hDisk);   /* Devicehandle holen */
     if (rc != NO_ERROR)                          /* Fehler aufgetreten ? */
     {
       if (Options.fsDebug == TRUE)                       /* Debug-Modus ? */
         ToolsErrorDos(rc);                       /* Fehlermeldung ausgeben */
       /*DisplayText("DskLogGetIOCtlHandle error: %u\n", rc); */
     }
     else
     {
       RemountLogicalParams (hDisk, ucDrive);             /* Infos ausgeben */
       RemountLogicalStatus (hDisk, ucDrive);             /* Infos ausgeben */
       DskLogEject(hDisk,ucDrive);

       DisplayText ("\n   Remounting ... ");                 /* Status angeben */
       rc = DskLogRedetermine (hDisk);                       /* Remount ! */
       switch (rc)                              /* Je nach Fehlerfall ... */
       {
         case NO_ERROR:                                       /* Alles OK */
           DisplayText ("OK");
           break;

         default:                                     /* alles andere ... */
           ToolsErrorDos(rc);
           /*DisplayText("DskLogRedetermine error: %u\n", rc);*/
       }
       DosClose(hDisk);
     }
  }

  return (rc);                                 /* RÅckgabewert liefern */
}


/***********************************************************************
 * Name      : APIRET RemountPhysicalDisks
 * Funktion  : Infos ueber physical disks
 * Parameter :
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 04.32.37]
 ***********************************************************************/

APIRET RemountPhysicalDisks (void)
{
  ULONG   ulNumDrives  = 0;                  /* Data return buffer        */
  ULONG   i;                                          /* Schleifenzaehler */
  ULONG   hDisk;                                   /* Disk-Handle (IOCtl) */
  APIRET  rc;                                               /* Returncode */

  DISKPARAMS DiskParams;                  /* Angaben zur Plattengeometrie */
  ULONG      ulDriveLast;


  rc = DskDiskPartitionableCount (&ulNumDrives);      /* Wieviele Disks ? */
  if (rc != NO_ERROR)                             /* Fehler aufgetreten ? */
    DisplayText("\nDosPhysicalDisk error #%u\n", rc);
  else
    DisplayText("\n%u partitionable disk(s):",ulNumDrives);

  for (i = 1;                        /* Alle gefundenen Disks untersuchen */
       i <= ulNumDrives;
       i++)
  {
    DisplayText ("\nPhysical Drive %u:",i);

    rc = DskDiskGetIOCtlHandle(i,&hDisk);              /* Handle erfragen */
    if (rc != NO_ERROR)                           /* Fehler aufgetreten ? */
        DisplayText("\nDskDiskGetIOCtlHandle error #%u", rc);
    else                                                   /* Kein Fehler */
    {
                                          /* Now we have a IOCtl handle ! */
      rc = DskDiskGetPhysicalParameters (hDisk,         /* Infos erfragen */
                                         &DiskParams);
     if (rc != NO_ERROR)                          /* Fehler aufgetreten ? */
       DisplayText(" DskDiskGetPhysicalParameters error #%u\n", rc);
     else
     {
       DisplayText (" %5u Cyl, %2u Heads, %3u Sec/Trk.", /* Info ausgeben */
               DiskParams.ulCylinders,
               DiskParams.ulHeads,
               DiskParams.ulSectorsPerTrack);


       rc = DskPhysDiskLock (hDisk);   /* PrÅfen, ob Lock auf Disk moeglich */
       if (rc != NO_ERROR)
         DisplayText (" can't lock (%u), ", rc); /* kann nicht gelockt werden */
       else
         DisplayText (" lock OK,         ",i);       /* kann gelockt werden */

       rc = DskPhysDiskUnlock (hDisk);                     /* Unlock prÅfen */
       if (rc != NO_ERROR)
         DisplayText ("can't unlock (%u).", rc);
       else
         DisplayText ("unlock OK.",i);

    }

    rc = DskDiskFreeIOCtlHandle(hDisk);           /* Handle wieder freigeben */
     if (rc != NO_ERROR)
        DisplayText("\nDskDiskFreeIOCtlHandle error #%u\n", rc);
    }
  }

  return (rc);                                 /* RÅckgabewert liefern */
}


/***********************************************************************
 * Name      : APIRET SearchDrives
 * Funktion  : Sucht nach den vorhandenen Laufwerken.
 * Parameter : PSZ pszReturnString
 * Variablen :
 * Ergebnis  : String mit den Laufwerksbuchstaben der gefundenen Laufwerke.
 * Bemerkung : Ich hasse Laufwerksbuchstaben.
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.46.09]
 ***********************************************************************/

APIRET SearchDrives (PSZ pszReturnString)
{
  APIRET rc = NO_ERROR;                                /* Rueckgabewert */

  ULONG ulDriveCurrent;
  ULONG ulDriveMap;
  ULONG ulPwrof2;
  PSZ   pszTemp;
  UCHAR ucCounter;

  if (pszReturnString == NULL)                /* Parameterueberpruefung */
    return (ERROR_INVALID_PARAMETER);           /* Fehler signalisieren */

           /* Have a look at the drives which shall be scanned normally */
  rc = DosQueryCurrentDisk (&ulDriveCurrent,
                            &ulDriveMap);
  if (rc != NO_ERROR)                   /* Ist ein Fehler aufgetreten ? */
    return (rc);                  /* Wenn ja, dann Fehler signalisieren */

      /* Translate the drive mapszDeviceTypeinto the scan_drives string */
  ulDriveMap &= 0xFFFFFFFc;                      /* Floppies ausblenden */

                                                    /* Count drive bits */
  ulPwrof2 = 1;          /* Zur Maskierung der DriveMap -> ReturnString */
  pszTemp  = pszReturnString; /* Keine Pruefung, ob genug Platz da ist. */

  for (ucCounter=0;                                      /* Bitposition */
       ucCounter<32;
       ucCounter++, ulPwrof2 += ulPwrof2)
    if (ulDriveMap & ulPwrof2)                 /* Laufwerk ausmaskieren */
    {
      *pszTemp = ucCounter + 'A';       /* Laufwerksbuchstabe "basteln" */
      pszTemp++;
    }
  *pszTemp = 0;                                     /* terminate string */

  return (NO_ERROR);                            /* RÅckgabewert liefern */
}


/***********************************************************************
 * Name      : int process_drives
 * Funktion  : Abarbeiten der Detailinformation fÅr jedes Laufwerk.
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.47.28]
 ***********************************************************************/

APIRET RemountInformation (UCHAR ucDrive)
{
  APIRET rc = NO_ERROR;                                /* Rueckgabewert */
  char   pszDeviceName[6];                /* Puffer fÅr den Devicenamen */
  PSZ    pszFSDName;                                        /* FSD-Name */
  PSZ    pszFSDData;                                   /* Attached Data */
  PSZ    pszDeviceType;                                   /* Device-Typ */

#define FSQSIZE 128
  PFSQBUFFER2  fsb;
  ULONG        BufferLength;

  fsb = (PFSQBUFFER2)malloc(FSQSIZE);
  if (!fsb)
  {
    printf ("\nOut of memory.");
    return ERROR_NOT_ENOUGH_MEMORY;
  }


  BufferLength = FSQSIZE;                          /* NÑhere Infos erfragen */
  sprintf (pszDeviceName,"%c:",ucDrive);
  rc = DosQueryFSAttach (pszDeviceName,0,FSAIL_QUERYNAME,fsb,&BufferLength);

    switch (rc)
    {
      case NO_ERROR:
        break;
      case ERROR_NO_MORE_ITEMS:
        #ifdef DEBUG
          printf (" No more items.");
        #endif
        break;
      default:
        return (rc);
    }

  /* Information ausgeben. */
  pszFSDName = fsb->szName + strlen(fsb->szName) + 1;
  pszFSDData = pszFSDName  + strlen(pszFSDName)  + 1;
  switch (fsb->iType)                            /* type of the drive */
  {
    case FSAT_CHARDEV:   pszDeviceType= "Character device       "; break;
    case FSAT_PSEUDODEV: pszDeviceType= "Pseudo-character device"; break;
    case FSAT_LOCALDRV:  pszDeviceType= "Local drive            "; break;
    case FSAT_REMOTEDRV: pszDeviceType= "Remote drive           "; break;
    default:             pszDeviceType= "Unknown:    00000000000";
             sprintf(pszDeviceType,"Unknown:    %011u",fsb->iType);
             break;
  }

  DisplayText ("%-8s %-18s (%s) ",pszFSDName,pszFSDData,pszDeviceType);

  free (fsb);

  return (NO_ERROR);                          /* RÅckgabewert liefern */
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
  int rc;                                                    /* RÅckgabewert */
  char szDrives[40];                /* Puffer fuer die logischen Laufwerke */

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
      DisplayText ("\nDskCacheFlush error %u",
                   rc);                                          /* Ausgeben */
  }

  if (Options.fsVerbose == TRUE)              /* Sollen wir Infos anzeigen ? */
  {
    DisplayText ("\nPhysical paritionable disks attached to the system:");
    rc = RemountPhysicalDisks();
    DisplayText ("\n");
  }

  DisplayText ("\nProcessing logical drives:");                 /* Remounten */
  {
                                                      /* Laufwerke ermitteln */
    if (Options.fsRemountAll == TRUE)                    /* Alles remounten ? */
    {
      rc = SearchDrives (szDrives);                   /* Laufwerke ermitteln */
      if (rc != NO_ERROR)                            /* Fehler aufgetreten ? */
        ToolsErrorDosEx(rc,                           /* yield error message */
                        "SearchDrives");
      else
        Options.pszRemountDrives = szDrives;          /* Parameter ergaenzen */
    }

    if (Options.pszRemountDrives != NULL)         /* Brauchbarer Parameter ? */
      rc = RemountLogicalDisks(Options.pszRemountDrives);
    else
      DisplayText ("\nNothing to remount !");
  }

  return rc;
}
