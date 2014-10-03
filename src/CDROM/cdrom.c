/***********************************************************************
 * Name      : Module CDROM
 * Funktion  : CDROM-Steuerung via Commandline.
 * Autor     : Patrick Haller [Montag, 25.09.1995 01.20.20]
 ***********************************************************************/

 /*
  Jeff Patton, 1:2410/242 in OS2PROG:

  The 'eject' IOCTL (cat 8, fn 40h) does not require a handle -- you can pass
  -1 as the handle and put the drive number (A=0) in the second byte of
  ParamList.  I would think 'close tray' would have the same option.
*/

/* CM_SEEK
   Audio Commands */

/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#define INCL_DOSDEVIOCTL
#define INCL_DOSDEVICES                                 /* DosDevIOCtl */
#define INCL_DOSERRORS                         /* Die Fehlerkonstanten */
#define INCL_DOSMISC                                  /* DosGetMessage */
#define INCL_DOS
#define INCL_NOPMAPI                      /* Kein Presentation Manager */

#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <conio.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"

#define MAXPATHLEN 260



/* ==================== Strukturdefinitionen ========================= */
typedef struct
{
  PSZ   pszFeature;                      /* Wie hei·t dieses Feature ? */
  PSZ   pszSupported;                        /* If it is supported ... */
  PSZ   pszNotSupported;                 /* If it is not supported ... */
  ULONG ulBitMask;                        /* Bit-mask for this feature */
} CDDEVSUPPORT, *PCDDEVSUPPORT;

#define CDSUPPORTED       "supported"    /* Default string for support */
#define CDNOTSUPPORTED    "not supported"              /* the opposite */
#define CDACTIVE          "active"       /* Default string for support */
#define CDINACTIVE        "inactive"                   /* the opposite */
#define CDYES             "yes"                             /* "Ja" :) */
#define CDNO              "no"                                 /* Nein */


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsCDROM;                              /* desired cdrom specified ? */
  ARGFLAG fsCmdEject;                            /* specified command tokens */
  ARGFLAG fsCmdPVD;
  ARGFLAG fsCmdReset;
  ARGFLAG fsCmdChange;
  ARGFLAG fsCmdCloseTray;
  ARGFLAG fsCmdLock;
  ARGFLAG fsCmdUnlock;
  ARGFLAG fsCmdStatus;
  ARGFLAG fsCmdSeek;
  ARGFLAG fsCmdVol0;
  ARGFLAG fsCmdVol1;
  ARGFLAG fsCmdVol2;
  ARGFLAG fsCmdVol3;
  ARGFLAG fsCmdPlay;
  ARGFLAG fsCmdStop;
  ARGFLAG fsCmdResume;
  ARGFLAG fsCmdInfo;

  PSZ     pszSeek;                                         /* parameter data */
  UCHAR   ucVol0;
  UCHAR   ucVol1;
  UCHAR   ucVol2;
  UCHAR   ucVol3;
  PSZ     pszCDROM;
} OPTIONS, *POPTIONS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */

ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung----------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/PVD",       "Read PVD sector and dump the information.",
                                       NULL,                 ARG_NULL,       &Options.fsCmdPVD},

  {"/EJECT",     "Eject the CD from the drive.",
                                       NULL,                 ARG_NULL,       &Options.fsCmdEject},
  {"/CLOSE",     "Close the CDROM tray.",
                                       NULL,                 ARG_NULL,       &Options.fsCmdCloseTray},
  {"/RESET",     "Reset the CDROM drive.",
                                       NULL,                 ARG_NULL,       &Options.fsCmdReset},
  {"/CHANGE",    "Ejects CDROM and closes tray afterwards.",
                                       NULL,                 ARG_NULL,       &Options.fsCmdChange},
  {"/LOCK",      "Locks the CDROM drive.",
                                       NULL,                 ARG_NULL,       &Options.fsCmdLock},
  {"/UNLOCK",    "Unlocks the CDROM drive.",
                                       NULL,                 ARG_NULL,       &Options.fsCmdUnlock},
  {"/STATUS",    "Information about the current drive status.",
                                       NULL,                 ARG_NULL,       &Options.fsCmdStatus},
  {"/SEEK=",     "Seek to specific sector or minute:second.",
                                       &Options.pszSeek,     ARG_PSZ,        &Options.fsCmdSeek},
  {"/VOL0=",     "Set volume for channel 0 (0..255)",
                                       &Options.ucVol0,      ARG_UCHAR,      &Options.fsCmdVol0},
  {"/VOL1=",     "Set volume for channel 1 (0..255)",
                                       &Options.ucVol1,      ARG_UCHAR,      &Options.fsCmdVol1},
  {"/VOL2=",     "Set volume for channel 2 (0..255)",
                                       &Options.ucVol2,      ARG_UCHAR,      &Options.fsCmdVol2},
  {"/VOL3=",     "Set volume for channel 3 (0..255)",
                                       &Options.ucVol3,      ARG_UCHAR,      &Options.fsCmdVol3},
  {"/PLAY",      "Start playing audio.",
                                       NULL,                 ARG_NULL,       &Options.fsCmdPlay},
  {"/STOP",      "Stops playing audio.",
                                       NULL,                 ARG_NULL,       &Options.fsCmdStop},
  {"/RESUME",    "Resume playing audio.",
                                       NULL,                 ARG_NULL,       &Options.fsCmdResume},
  {"/INFO",      "Get information about the inserted CD.",
                                       NULL,                 ARG_NULL,       &Options.fsCmdInfo},

  {"1",          "CDROM drive (C:,D:,E:,0,1,2)",
                                       &Options.pszCDROM,    ARG_PSZ |
                                                             ARG_DEFAULT |
                                                             ARG_MUST,       &Options.fsCDROM},
  ARG_TERMINATE
};


/* ============== Tablelle fÅr den CDROM-Status ====================== */
CDDEVSUPPORT TabCDSupport[] =
{ {"Reading of CD-DA sectors",  CDSUPPORTED,CDNOTSUPPORTED,0x40000000},
  {"Reading of mode 2 sectors", CDSUPPORTED,CDNOTSUPPORTED,0x00000400},
  {"Reading of data, audio and/or video",
                                CDSUPPORTED,"data only",   0x00000010},
  {"Supported sector sizes",    "2048 and 2352 byte sectors",
            "2048 byte sectors only",  0x00000004},
  {"Audio playback mode",       CDACTIVE,   CDINACTIVE,    0x00001000},
  {"Audio channel manipulation",CDSUPPORTED,CDNOTSUPPORTED,0x00000100},
  {"Addressing modes",          "logical-block and minute-second frame",
            "logical-block only, no minute-second frame",
                                            0x00000200},
  {"Internal prefetching",      CDSUPPORTED,CDNOTSUPPORTED,0x00000080},
  {"ISO9660 interleaving size and skip",
                                CDSUPPORTED,CDNOTSUPPORTED,0x00000040},
  {"Drive can",                 "read and write (CDWORM)",
            "only read (normal CDROM)",0x00000008},
  {"Disc in drive",             CDNO,       CDYES,         0x00000800},
  {"Drive door is",             "unlocked", "locked",      0x00000002},
  {"Drive door is",             "open",     "closed",      0x00000001},
  {NULL,                        NULL,       NULL,          0x00000000},
};


/* ===================== Globale Variablen =========================== */
static const UCHAR pszCDROMSig[4] = "CD01";              /* Standardsignatur */


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void   help            ( void );

void   ErrorCD         ( APIRET        rc );

APIRET CDGenericIO     ( HFILE         hCDROM,
                         ULONG         ulIOCtlClass,
                         ULONG         ulCtlCommand );

APIRET CDEject         ( HFILE         hCDROM );

APIRET CDReset         ( HFILE         hCDROM );

APIRET CDCloseTray     ( HFILE         hCDROM );

APIRET CDLockUnlock    ( HFILE         hCDROM,
                         BYTE          bAction );

VOID   CDDeviceStatusTable
                       ( ULONG         ulCDStatus,
                         PCDDEVSUPPORT pCDDevSupport );

APIRET CDDeviceStatus  ( HFILE         hCDROM );

APIRET CDSeek          ( HFILE         hCDROM,
                         ULONG         ulSeek );

APIRET CDAudioStop     ( HFILE         hCDROM );

APIRET CDAudioResume   ( HFILE         hCDROM );

APIRET CDROMControl    ( void );

APIRET CDDiskControl   ( UCHAR         ucUnit,
                         UCHAR         ucCommand );


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
  TOOLVERSION("CDROM",                                   /* application name */
              0x00010003,                            /* application revision */
              0x00010100,           /* minimum required revision of PHSTOOLS */
              0x0001FFFF,        /* maximum recommended revision of PHSTOOLS */
              NULL,                                                /* Remark */
              NULL);                                 /* additional copyright */
}


/***********************************************************************
 * Name      : void ErrorCD
 * Funktion  : Stellt Fehlermeldungen des CDROM-Treibers zur VerfÅgung.
 * Parameter : APIRET rc
 * Variablen :
 * Ergebnis  : kenies
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 21.10.1995 18.53.20]
 ***********************************************************************/

void ErrorCD (APIRET rc)
{
  ULONG ulMsg;                                        /* Messagenummer */

  const static PSZ szError[] = {
    /* 00 */ "cderror 00",
    /* 01 */ "cderror 01",
    /* 02 */ "Device not ready or controller does not respond",
    /* 03 */ "Action not supported, device is not a CDROM",
    /* 04 */ "cderror 04",
    /* 05 */ "cderror 05",
    /* 06 */ "Seek error: The drive does not seek",
    /* 07 */ "Unknown media, CDROM-format does not match CD Redbook specifications",
    /* 08 */ "Sector not found",
    /* 09 */ "cderror 09",
    /* 0a */ "cderror 0a",
    /* 0b */ "cderror 0b",
    /* 0c */ "cderror 0c",
    /* 0d */ "cderror 0d",
    /* 0e */ "cderror 0e",
    /* 0f */ "cderror 0f",
    /* 10 */ "Uncertain media",
    /* 11 */ "cderror 11",
    /* 12 */ "cderror 12",
    /* 13 */ "Unsupported parameter, signature is wrong (internal error)",
    /* 14 */ "Device is already in use",
    /* 15 */ "cderror 15",
    /* 16 */ "cderror 16",
    /* 17 */ "cderror 17",
    /* 18 */ "cderror 18",
    /* 19 */ "cderror 19",
    /* 1a */ "cderror 1a",
    /* 1b */ "cderror 1b",
    /* 1c */ "cderror 1c",
    /* 1d */ "cderror 1d",
    /* 1e */ "cderror 1e",
    /* 1f */ "cderror 1f"};

  ulMsg = rc & 0x00ff;                      /* Treibermeldung filtern */

  if (ulMsg < 0x20)                      /* Innerhalb des Bereiches ? */
    printf ("\nError: %04x - (%s).",
        ulMsg,
        szError[ulMsg]);
}


/***********************************************************************
 * Name      : APIRET CDGenericIO
 * Funktion  : Abwicklung der generischen CD-IOCtls.
 * Parameter : HFILE hCDROM, ULONG ulIOCtlClass, ULONG ulCtlCommand
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung : Spart viel Code...
 *
 * Autor     : Patrick Haller [Montag, 23.10.1995 17.40.32]
 ***********************************************************************/

APIRET CDGenericIO (HFILE hCDROM,
                    ULONG ulIOCtlClass,
                    ULONG ulCtlCommand)
{
  ULONG  ulParmRetLen;                                /* ParameterpaketlÑnge */
  ULONG  ulDataRetLen;                                /* ParameterpaketlÑnge */
  APIRET rc;                                                 /* RÅckgabewert */

          /* This is the direct way to communicate with the driver via IOCtl */
  ulParmRetLen = 4;
  ulDataRetLen = 0;

  rc = DosDevIOCtl(hCDROM,                                  /* Device-Handle */
               ulIOCtlClass,                                     /* Category */
               ulCtlCommand,                                     /* Function */
               (PVOID)pszCDROMSig,                 /* Parameterblock-Pointer */
               sizeof(pszCDROMSig),        /* Max. LÑnge der Parameterblocks */
               &ulParmRetLen,       /* Pointer auf LÑnge des Parameterblocks */
               NULL,                                           /* Datenblock */
               0L,                         /* Maximale LÑnge des Datenblocks */
               &ulDataRetLen);          /* Pointer auf LÑnge des Datenblocks */

  return (rc);                                       /* RÅckgabewert liefern */
} /* APIRET CDGenericIO */

/***********************************************************************
 * Name      : APIRET CDEject
 * Funktion  : Auswerfen der CD.
 * Parameter : VOID
 * Variablen :
 * Ergebnis  : API-Fehlercode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 03.10.1995 18.20.18]
 ***********************************************************************/

APIRET CDEject (HFILE hCDROM)
{
  return (CDGenericIO (hCDROM,
                       IOCTL_CDROMDISK,
                       CDROMDISK_EJECTDISK));
} /* APIRET CDEject */


/***********************************************************************
 * Name      : APIRET CDReset
 * Funktion  : Laufwerk resetten
 * Parameter : HFILE hCDROM
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung : Sieht DosDevIOCtl
 *
 * Autor     : Patrick Haller [Montag, 16.10.1995 03.12.12]
 ***********************************************************************/

APIRET CDReset (HFILE hCDROM)
{
  return (CDGenericIO (hCDROM,
                       IOCTL_CDROMDISK,
                       CDROMDISK_RESETDRIVE));
} /* APIRET CDReset */


/***********************************************************************
 * Name      : APIRET CDCloseTray
 * Funktion  : Closes the tray on drives with tray loading mechanism.
 * Parameter : HFILE hCDROM
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung : Wenn Laufwerk bereits offen ist, kann kein Handle
 *             ueber das CDROM erfragt werden :(
 *
 * Autor     : Patrick Haller [Montag, 16.10.1995 03.15.13]
 ***********************************************************************/

APIRET CDCloseTray (HFILE hCDROM)
{
#define CDROMDISK_CLOSETRAY 0x45                     /* Bug in toolkit */
  APIRET rc = NO_ERROR;                                /* RÅckgabewert */

  printf ("\nEjecting media ...");            /* Erst Medium auswerfen */
  rc = CDGenericIO (hCDROM,
                    IOCTL_CDROMDISK,
                    CDROMDISK_EJECTDISK);
  if (rc == NO_ERROR)                          /* Fehler aufgetreten ? */
  {
    printf ("\nChange media and press any key ...");
    getch();                                          /* Wait for user */
    rc = CDGenericIO (hCDROM,
                      IOCTL_CDROMDISK,
                      CDROMDISK_CLOSETRAY);
  }

  return (rc);                                 /* RÅckgabewert liefern */
} /* APIRET CDCloseTray */


/***********************************************************************
 * Name      : APIRET CDLockUnlock
 * Funktion  : (Un)locks the CDROM drive.
 * Parameter : HFILE hCDROM, BYTE bAction
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung : Siehe DosDevIOCtl
 *
 * Autor     : Patrick Haller [Montag, 16.10.1995 03.16.58]
 ***********************************************************************/

APIRET CDLockUnlock (HFILE hCDROM, BYTE bAction)
{
  APIRET rc = NO_ERROR;                                /* RÅckgabewert */
  ULONG  ulParmRetLen;                          /* ParameterpaketlÑnge */
  ULONG  ulDataRetLen;                          /* ParameterpaketlÑnge */

  UCHAR  pszParamBlock[5] = "CD010";                 /* Parameterblock */

    /* This is the direct way to communicate with the driver via IOCtl */
  ulParmRetLen = 5;
  ulDataRetLen = 0;

            /* Shall we lock or unlock the drive ? */
  pszParamBlock[4] = bAction;                  /* 0 = unlock, 1 = lock */

  rc = DosDevIOCtl(hCDROM,                          /* Device-Handle */
    IOCTL_CDROMDISK,                                  /* Category */
    CDROMDISK_LOCKUNLOCKDOOR,                         /* Function */
    pszParamBlock,                      /* Parameterblock-Pointer */
    sizeof(pszParamBlock),      /* Max. LÑnge der Parameterblocks */
    &ulParmRetLen,       /* Pointer auf LÑnge des Parameterblocks */
    NULL,                                           /* Datenblock */
    0L,                         /* Maximale LÑnge des Datenblocks */
    &ulDataRetLen);          /* Pointer auf LÑnge des Datenblocks */

  return (rc);                                 /* RÅckgabewert liefern */
} /* APIRET CDLockUnlock */


/***********************************************************************
 * Name      : VOID CDDeviceStatusTable
 * Funktion  : Auswerten des Status-ULONGs anhand der Tabelle.
 * Parameter : ULONG ulCDStatus, PCDDEVSUPPORT pCDDevSupport
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 21.10.1995 19.13.35]
 ***********************************************************************/

VOID CDDeviceStatusTable (ULONG ulCDStatus, PCDDEVSUPPORT pCDDevSupport)
{
  PCDDEVSUPPORT pSupport;               /* Laufzeiger Åber die Tabelle */

  if (pCDDevSupport == NULL)                   /* ParameterÅberprÅfung */
    return;

  for (pSupport = pCDDevSupport;
       pSupport->pszFeature;  /* Solange Listenende nicht erreicht ist */
       pSupport++)
       {
    if (ulCDStatus & pSupport->ulBitMask)        /* Feasture an ? */
      printf ("\n* %-37s: %s",
         pSupport->pszFeature,
         pSupport->pszSupported);
    else
      printf ("\n* %-37s: %s",             /* Nicht unterstÅtzt */
         pSupport->pszFeature,
         pSupport->pszNotSupported);       }

} /* VOID CDDeviceStatusTable */


/***********************************************************************
 * Name      : APIRET CDDeviceStatus
 * Funktion  : Gathers information about the drive's status.
 * Parameter : HFILE hCDROM
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung : Siehe DosDevIOCtl
 *
 * Autor     : Patrick Haller [Montag, 16.10.1995 03.35.43]
 ***********************************************************************/

APIRET CDDeviceStatus (HFILE hCDROM)
{
  APIRET rc = NO_ERROR;                                /* RÅckgabewert */
  ULONG  ulParmRetLen;                          /* ParameterpaketlÑnge */
  ULONG  ulDataRetLen;                          /* ParameterpaketlÑnge */
  ULONG  ulData;                         /* ULONG data parameter block */
  USHORT usData;                          /* WORD data parameter block */

  ULONG  ulSectors;                          /* Volume size in sectors */
  ULONG  ulSectorSize;                  /* The size of a single sector */
  double dSize;                               /* Volume size in MBytes */


  /* This is the direct way to communicate with the driver via IOCtl */
  ulParmRetLen = 4;
  ulDataRetLen = sizeof(ulData);

  rc = DosDevIOCtl(hCDROM,                                  /* Device-Handle */
               IOCTL_CDROMDISK,                                  /* Category */
               CDROMDISK_DEVICESTATUS,                           /* Function */
               (PVOID)pszCDROMSig,                 /* Parameterblock-Pointer */
               sizeof(pszCDROMSig),        /* Max. LÑnge der Parameterblocks */
               &ulParmRetLen,       /* Pointer auf LÑnge des Parameterblocks */
               &ulData,                                        /* Datenblock */
               sizeof(ulData),             /* Maximale LÑnge des Datenblocks */
               &ulDataRetLen);          /* Pointer auf LÑnge des Datenblocks */

  if (rc == NO_ERROR)                    /* Wenn soweit alles i.O. ist */
  {
    printf ("\nDevice status:");

    CDDeviceStatusTable (ulData, TabCDSupport);  /* Status analysieren */
  }
  else
    return (rc);              /* Ein Fehler ist aufgetreten, abbrechen */


  /* This is the direct way to communicate with the driver via IOCtl */
  ulParmRetLen = 4;
  ulDataRetLen = sizeof(usData);

  /* Get sector size */
  rc = DosDevIOCtl(hCDROM,                                  /* Device-Handle */
                IOCTL_CDROMDISK,                                 /* Category */
                CDROMDISK_GETSECTORSIZE,                         /* Function */
                (PVOID)pszCDROMSig,                /* Parameterblock-Pointer */
                sizeof(pszCDROMSig),       /* Max. LÑnge der Parameterblocks */
                &ulParmRetLen,      /* Pointer auf LÑnge des Parameterblocks */
                &usData,                                       /* Datenblock */
                sizeof(usData),            /* Maximale LÑnge des Datenblocks */
                &ulDataRetLen);         /* Pointer auf LÑnge des Datenblocks */
  if (rc == NO_ERROR)                                   /* Soweit alles i.O. */
  {
    printf ("\nSector size is %hu bytes.",usData);
    ulSectorSize = usData;
  }
  else
    return (rc);                /* Ein Fehler ist aufgetreten, Abbruch */

  /* IOCTL_GETHEADLOC */


  /* This is the direct way to communicate with the driver via IOCtl */
  ulParmRetLen = 4;
  ulDataRetLen = sizeof(ulData);

  /* Get sector size */
  rc = DosDevIOCtl(hCDROM,                                  /* Device-Handle */
               IOCTL_CDROMDISK,                                  /* Category */
               CDROMDISK_GETVOLUMESIZE,                          /* Function */
               (PVOID)pszCDROMSig,                 /* Parameterblock-Pointer */
               sizeof(pszCDROMSig),        /* Max. LÑnge der Parameterblocks */
               &ulParmRetLen,       /* Pointer auf LÑnge des Parameterblocks */
               &ulData,                                        /* Datenblock */
               sizeof(ulData),             /* Maximale LÑnge des Datenblocks */
               &ulDataRetLen);          /* Pointer auf LÑnge des Datenblocks */
  if (rc == NO_ERROR)                                   /* Soweit alles i.O. */
  {
    ulSectors = ulData;
    dSize = (double)ulSectors * (double)ulSectorSize / 1024 / 1024;
    printf ("\nVolume size is %u sectors, %6.2f MBytes.",
       ulSectors,
       dSize);
  }
  else
    return (rc);                /* Ein Fehler ist aufgetreten, Abbruch */

  /* Universal Product Code auslesen ist recht uninteressant*/

  return (rc);                                 /* RÅckgabewert liefern */
} /* APIRET CDDeviceStatus */


/***********************************************************************
 * Name      : APIRET CDSeek
 * Funktion  : Suchen/Spulen auf der CDROM.
 * Parameter : HFILE hCDROM, ULONG ulSeek
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 23.10.1995 16.53.45]
 ***********************************************************************/

APIRET CDSeek (HFILE hCDROM, ULONG ulSeek)
{
  APIRET rc = NO_ERROR;                                /* RÅckgabewert */
  ULONG  ulParmRetLen;                          /* ParameterpaketlÑnge */
  ULONG  ulDataRetLen;                          /* ParameterpaketlÑnge */
  UCHAR  bAddressingMode;                        /* Adressierungsmodus */

  UCHAR  pszParamBlock[9] = "CD0100000";             /* Parameterblock */

    /* This is the direct way to communicate with the driver via IOCtl */
  ulParmRetLen = sizeof(pszParamBlock);
  ulDataRetLen = 0;

  bAddressingMode = 0;                /* @@@PH Vorerst fix eingestellt */
  pszParamBlock[4] = bAddressingMode;  /* 0 = logical block, 1 = mm:ss */

  *((PULONG)pszParamBlock+5) = ulSeek;

  rc = DosDevIOCtl(hCDROM,                            /* Device-Handle */
    IOCTL_CDROMDISK,                                  /* Category */
    CDROMDISK_SEEK,                                   /* Function */
    pszParamBlock,                      /* Parameterblock-Pointer */
    sizeof(pszParamBlock),      /* Max. LÑnge der Parameterblocks */
    &ulParmRetLen,       /* Pointer auf LÑnge des Parameterblocks */
    NULL,                                           /* Datenblock */
    0L,                         /* Maximale LÑnge des Datenblocks */
    &ulDataRetLen);          /* Pointer auf LÑnge des Datenblocks */


  return (rc);                                 /* RÅckgabewert liefern */
} /* APIRET CDSeek */


/***********************************************************************
 * Name      : APIRET CDAudioStop
 * Funktion  : Bricht das Audioabspielen ab.
 * Parameter : HFILE hCDROM
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 23.10.1995 17.12.57]
 ***********************************************************************/

APIRET CDAudioStop (HFILE hCDROM)
{
  return (CDGenericIO (hCDROM,
                       IOCTL_CDROMAUDIO,
                       CDROMAUDIO_STOPAUDIO));
} /* APIRET CDAudioStop */


/***********************************************************************
 * Name      : APIRET CDAudioPlay
 * Funktion  : Start playing audio
 * Parameter : HFILE hCDROM
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 23.10.1995 17.12.57]
 ***********************************************************************/

APIRET CDAudioPlay (HFILE hCDROM,
                    UCHAR ucAddressing,
                    ULONG ulLSNStart,
                    ULONG ulLSNEnd)
{
  APIRET rc = NO_ERROR;                                      /* RÅckgabewert */
  ULONG  ulParmRetLen;                                /* ParameterpaketlÑnge */
  ULONG  ulDataRetLen;                                /* ParameterpaketlÑnge */

  struct _sParamBlock
  {
    CHAR  szSignature[4];                     /* CD request packet signature */
    UCHAR ucAddressing;                                   /* addressing mode */
    ULONG ulLSNStart;                             /* sector to start playing */
    ULONG ulLSNEnd;                                /* sector to stop playing */
  } sParamBlock;

          /* This is the direct way to communicate with the driver via IOCtl */
  ulParmRetLen = 13;
  ulDataRetLen = 0;

  sParamBlock.szSignature[0] = 'C';
  sParamBlock.szSignature[1] = 'D';
  sParamBlock.szSignature[2] = '0';
  sParamBlock.szSignature[3] = '1';
  sParamBlock.ucAddressing = ucAddressing;           /* 0-sector, 1-mm/ss/ff */
  sParamBlock.ulLSNStart   = ulLSNStart;
  sParamBlock.ulLSNEnd     = ulLSNEnd;

  rc = DosDevIOCtl(hCDROM,                                  /* Device-Handle */
               IOCTL_CDROMAUDIO,                                 /* Category */
               CDROMAUDIO_PLAYAUDIO,                             /* Function */
               &sParamBlock,                       /* Parameterblock-Pointer */
               sizeof(sParamBlock),        /* Max. LÑnge der Parameterblocks */
               &ulParmRetLen,       /* Pointer auf LÑnge des Parameterblocks */
               NULL,                                           /* Datenblock */
               0L,                         /* Maximale LÑnge des Datenblocks */
               &ulDataRetLen);          /* Pointer auf LÑnge des Datenblocks */

  return (rc);                                       /* RÅckgabewert liefern */
} /* APIRET CDAudioPlay */


/***********************************************************************
 * Name      : APIRET CDAudioResume
 * Funktion  : Setzt das Audioabspielen fort.
 * Parameter : HFILE hCDROM
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 23.10.1995 17.12.57]
 ***********************************************************************/

APIRET CDAudioResume (HFILE hCDROM)
{
  return (CDGenericIO (hCDROM,
                       IOCTL_CDROMAUDIO,
                       CDROMAUDIO_RESUMEAUDIO));
} /* APIRET CDAudioResume */


/***********************************************************************
 * Name      : APIRET CDROMControl
 * Funktion  : Wrapper fÅr alle CD-Unterfunktionen
 * Parameter : ULONG ulCommand
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 25.09.1995 01.31.18]
 ***********************************************************************/

APIRET CDDiskControl (UCHAR ucUnit,
                      UCHAR ucCommand)
{
  APIRET rc;                                    /* RÅckgabewert - Fehlercode */
  ULONG  ulDataRetLen;                                 /* data return length */
  ULONG  ulParmRetLen;                                 /* parm return length */
  USHORT usData;                                            /* the data word */

  ulParmRetLen = 2;
  ulDataRetLen = 0;

  usData = ( (ucUnit - '0') << 8) | ucCommand;           /* build data block */

  rc = DosDevIOCtl(0,                                       /* Device-Handle */
                   IOCTL_DISK,                                   /* Category */
                   DSK_UNLOCKEJECTMEDIA,                         /* Function */
                   &usData,                        /* Parameterblock-Pointer */
                   sizeof(usData),         /* Max. LÑnge der Parameterblocks */
                   &ulParmRetLen,   /* Pointer auf LÑnge des Parameterblocks */
                   NULL,                                       /* Datenblock */
                   0L,                     /* Maximale LÑnge des Datenblocks */
                   &ulDataRetLen);      /* Pointer auf LÑnge des Datenblocks */
  if (rc != NO_ERROR)                                    /* check for errors */
    ToolsErrorDos(rc);

  return (rc);
}


/***********************************************************************
 * Name      : APIRET CDInfoPVD
 * Funktion  : Reads PVD and dumps information
 * Parameter : ULONG ulCommand
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 25.09.1995 01.31.18]
 ***********************************************************************/

/* reads the first primary volume descriptor on a ISO CD-ROM into            */
/* the structures given below (for reading the actual - that is the          */
/* last - PVD, the function would have to be modified in a way that it loops */
/* through all PVDs written onto the multi-session-CD-ROM)                   */

#define DATA_SECTORSIZE 2048            /* actual size of data in one sector */
#pragma pack ( 1 )

                          /* Struktur eines Verzeichniseintrags fÅr ISO-9660 */
typedef struct _PVD_ISODIR
{
   UCHAR  ucDirRecLen;               /* LÑnge Verzeichniseintrag             */
   UCHAR  ucXARLen;                  /* LÑnge XAR                            */
   ULONG  ulLocExtLSB;               /* Start der Daten (Block)              */
   ULONG  ulLocExtMSB;               /*       "                              */
   ULONG  ulDataLenLSB;              /* Dateigrî·e                           */
   ULONG  ulDataLenMSB;              /*       "                              */
   UCHAR  ucDateYear;                /* Jahr (seit 1900)                     */
   UCHAR  ucDateMonth;               /* Monat                                */
   UCHAR  ucDateDay;                 /* Tag                                  */
   UCHAR  ucDateHour;                /* Stunde                               */
   UCHAR  ucDateMinute;              /* Minute                               */
   UCHAR  ucDateSecond;              /* Sekunde                              */
   UCHAR  ucDateGMT;                 /* Offset zu GMT in 15M. Intervallen    */
   UCHAR  ucFileFlags;               /* Dateiflags                           */
   UCHAR  ucFileUnitSize;            /* Einheitengrî·e bei Interleaving      */
   UCHAR  ucIlGapSize;               /* Grî·e der LÅcke bei Interleaving     */
   USHORT usVolSeqLSB;               /* Set, in dem die Datei zu finden ist  */
   USHORT usVolSeqMSB;               /*       "                              */
   UCHAR  ucFileIDLen;               /* LÑnge Dateinamen                     */
   UCHAR  ucFileID;                  /* Dateiname                            */
} PVD_ISODIR, *PPVD_ISODIR;

                     /* Struktur des Primary Volume Descriptors fÅr ISO 9660 */
typedef struct _PVD_ISOVOLDESC
{
   UCHAR     ucType;                 /* Typ des PVD                          */
   UCHAR     ucID[5];                /* ISO "CD001"                          */
   UCHAR     ucVersion;              /* Version des Standards                */
   UCHAR     ucReserved1;
   UCHAR     ucSysID[32];            /* System-Identifikation                */
   UCHAR     ucVolID[32];            /* Volume label                         */
   UCHAR     ucReserved2[8];
   ULONG     ulVolSizeLSB;           /* Grî·e des Volumes in Blocks          */
   ULONG     ulVolSizeMSB;           /*       "                              */
   UCHAR     ucReserved3[32];
   USHORT    usSetSizeLSB;           /* Anzahl CDs im Set                    */
   USHORT    usSetSizeMSB;           /*       "                              */
   USHORT    usSetSeqLSB;            /* Folgenummer im Set                   */
   USHORT    usSetSeqMSB;            /*       "                              */
   USHORT    usBlkSizeLSB;           /* Logische Blockgrî·e                  */
   USHORT    usBlkSizeMSB;           /*       "                              */
   ULONG     ulPathTabSizeLSB;       /* LÑnge der Pfadtabelle in Bytes       */
   ULONG     ulPathTabSizeMSB;       /*       "                              */
   ULONG     ulPathTabLocLSB;        /* Start der Pfadtabelle (Block)        */
   ULONG     ulPathTabLocOptLSB;     /* Start der optionalen Pfadtabelle     */
   ULONG     ulPathTabLocMSB;        /* Start der Pfadtabelle (Block)        */
   ULONG     ulPathTabLocOptMSB;     /* Start der optionalen Pfadtabelle     */
   PVD_ISODIR idRootDir;             /* Hauptverzeichnis                     */
   UCHAR     ucVolSetID[128];        /* Name des Sets                        */
   UCHAR     ucPubID[128];           /* Herausgeber                          */
   UCHAR     ucPrepID[128];          /* Datenaufbereiter                     */
   UCHAR     ucAppID[128];           /* Anwendung                            */
   UCHAR     ucCopyRightID[37];      /* Copyright                            */
   UCHAR     ucAbstractID[37];       /* Kurzbeschreibung                     */
   UCHAR     ucBiblioID[37];         /* Bibliographie                        */
   UCHAR     ucCreateDate[17];       /* Erstellungsdatum                     */
   UCHAR     ucModDate[17];          /* énderungsdatum                       */
   UCHAR     ucExpDate[17];          /* Ablaufdatum                          */
   UCHAR     ucEffDate[17];          /* GÅltigkeitsdatum                     */
   UCHAR     ucStdVer;               /*                                      */
   UCHAR     ucReserved4[193];
   UCHAR     ucXA_ID[5];             /* "CD-XA"                              */
} PVD_ISOVOLDESC, *PPVD_ISOVOLDESC;

typedef struct _PVD_PVDSECTOR
{
   UCHAR       ucSync[12];           /* sync at beginning of raw sector      */
   UCHAR       ucHeader[4];          /* header data                          */
   PVD_ISOVOLDESC ivdPVD;            /* the primary volume descriptor data   */
   UCHAR       ucLeftOver[DATA_SECTORSIZE-sizeof(PVD_ISOVOLDESC)];
               /* data area that is not used (just to match the sector size) */
   UCHAR       ucECC[288];           /*  error corecction codes              */
} PVD_PVDSECTOR, *PPVD_PVDSECTOR;


APIRET CDPvdRead(HFILE          hCDROM,
                 PPVD_PVDSECTOR ppvdPVDBuffer)
{
  #pragma pack ( 1 )  // align byte-wise
  struct                                                  /* parameter block */
  {
    UCHAR   ucSignature [4];
    UCHAR   ucAdrMode;
    USHORT  usSectorCount;
    ULONG   ulStartSector;
    UCHAR   ucRes_1;
    UCHAR   ucInterleave;
  } ReadPVDParams = {'C','D','0','1',
                     0x00,
                     0x0001,
                     0x00000010,
                     0x00,
                     0x00
                    };
  ULONG  ulReadPVDParmLen = sizeof (ReadPVDParams);  /* size parameter block */
  APIRET rc = NO_ERROR;                                        /* error code */
  ULONG  ulDataLen = sizeof(PVD_PVDSECTOR);         /* size of sector buffer */
  ULONG  ulBytesRead,
         ulNewLoc;

  memset (ppvdPVDBuffer,
          0x00,
          ulReadPVDParmLen);

  rc = DosSetFilePtr( hCDROM,                                 /* seek to PVD */
                      0x10*2048l,
                      FILE_BEGIN,
                      &ulNewLoc);
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                                    /* raise error condition */


  rc = DosRead( hCDROM,
                (PVOID)&ppvdPVDBuffer->ivdPVD,
                sizeof(PVD_ISOVOLDESC),
                &ulBytesRead );

  return (rc);                                      /* raise error condition */

/*  Alternate methow with read raw -> leads to problems sometimes

    rc = DosDevIOCtl(hfCDHandle,                 // Handle to device
                     (ULONG)IOCTL_CDROMDISK,     // Category of request
                     (ULONG)CDROMDISK_READLONG,  // Function being requested

                     (PVOID) &ReadPVDParams, // Input/Output parameter list
                     ulReadPVDParmLen,       // Maximum output parameter size
                     &ulReadPVDParmLen,      // Input:  size of parameter list
                                               // Output: size of parameters returned

                     (PVOID) ppvdPVDBuffer, // Input/Output data area
                     ulDataLen,             // Maximum output data size
                     &ulDataLen);           // Input:  size of input data area
                                              // Output: size of data returned
*/
}


/***********************************************************************
 * Name      : APIRET CDPvdInfo
 * Funktion  : Reads PVD and dumps information
 * Parameter : ULONG ulCommand
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 25.09.1995 01.31.18]
 ***********************************************************************/

APIRET CDPvdInfo(HFILE hCDROM)
{
  APIRET rc;                                               /* API-returncode */
  PVD_PVDSECTOR pvd;                                       /* Our PVD sector */

  printf ("\nReading PVD ...");

  rc = CDPvdRead(hCDROM,                                     /* read the pvd */
                 &pvd);
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                                    /* raise error condition */


  /************
   * PVD-Info *
   ************/

  printf ("\n[Primary Volume Descriptor]");

  pvd.ivdPVD.ucID[3] = 0;     StrTrim(pvd.ivdPVD.ucID);
  pvd.ivdPVD.ucSysID[31] = 0; StrTrim(pvd.ivdPVD.ucSysID);
  pvd.ivdPVD.ucVolID[31] = 0; StrTrim(pvd.ivdPVD.ucVolID);

  printf ("\n  Type      : %02xh"
          "\n  ID        : [%s]"
          "\n  Version   : %u"
          "\n  System ID : [%s]"
          "\n  Volume ID : [%s]",
          pvd.ivdPVD.ucType,
          pvd.ivdPVD.ucID,
          pvd.ivdPVD.ucVersion,
          pvd.ivdPVD.ucSysID,
          pvd.ivdPVD.ucVolID);
  printf ("\n  Volume Size (blocks) LSB: %u"
          "\n                       MSB: %u"
          "\n  CD Set Size             : %u"
          "\n  CD Set Sequence Number  : %u"
          "\n  Logical Blocksize       : %u",
          pvd.ivdPVD.ulVolSizeLSB,
          pvd.ivdPVD.ulVolSizeMSB,
          pvd.ivdPVD.usSetSizeMSB << 16 + pvd.ivdPVD.usSetSizeLSB,
          pvd.ivdPVD.usSetSeqMSB << 16  + pvd.ivdPVD.usSetSeqLSB,
          pvd.ivdPVD.usBlkSizeMSB << 16 + pvd.ivdPVD.usBlkSizeLSB);

  printf ("\n  Path Table Size      LSB: %u"
          "\n                       MSB: %u"
          "\n  Path Table Start     LSB: %08xh"
          "\n                       MSB: %08xh"
          "\n  Opt. Path Table      LSB: %08xh"
          "\n                       MSB: %08xh",
          pvd.ivdPVD.ulPathTabSizeLSB,
          pvd.ivdPVD.ulPathTabSizeMSB,
          pvd.ivdPVD.ulPathTabLocLSB,
          pvd.ivdPVD.ulPathTabLocMSB,
          pvd.ivdPVD.ulPathTabLocOptLSB,
          pvd.ivdPVD.ulPathTabLocOptMSB);

  /* @@@PH Maybe we're loosing the last character here ... */
  pvd.ivdPVD.ucVolSetID[127]   = 0; StrTrim(pvd.ivdPVD.ucVolSetID);
  pvd.ivdPVD.ucPubID[127]      = 0; StrTrim(pvd.ivdPVD.ucPubID);
  pvd.ivdPVD.ucPrepID[127]     = 0; StrTrim(pvd.ivdPVD.ucPrepID);
  pvd.ivdPVD.ucAppID[127]      = 0; StrTrim(pvd.ivdPVD.ucAppID);
  pvd.ivdPVD.ucCopyRightID[36] = 0; StrTrim(pvd.ivdPVD.ucCopyRightID);
  pvd.ivdPVD.ucAbstractID[36]  = 0; StrTrim(pvd.ivdPVD.ucAbstractID);
  pvd.ivdPVD.ucBiblioID[36]    = 0; StrTrim(pvd.ivdPVD.ucBiblioID);
  pvd.ivdPVD.ucCreateDate[16]  = 0; StrTrim(pvd.ivdPVD.ucCreateDate);
  pvd.ivdPVD.ucModDate[16]     = 0; StrTrim(pvd.ivdPVD.ucModDate);
  pvd.ivdPVD.ucExpDate[16]     = 0; StrTrim(pvd.ivdPVD.ucExpDate);
  pvd.ivdPVD.ucEffDate[16]     = 0; StrTrim(pvd.ivdPVD.ucEffDate);

  printf ("\n\n[Author Information]"
          "\n  Volume Set   [%s]"
          "\n  Publisher    [%s]"
          "\n  Preparer     [%s]"
          "\n  Application  [%s]"
          "\n  Copyright    [%s]"
          "\n  Abstract     [%s]"
          "\n  Brief desc.  [%s]"
          "\n  Creation     [%s]"
          "\n  Modification [%s]"
          "\n  Expiry       [%s]"
          "\n  Eff. date    [%s]"
          "\n  Standard ver: %u",
          pvd.ivdPVD.ucVolSetID,
          pvd.ivdPVD.ucPubID,
          pvd.ivdPVD.ucPrepID,
          pvd.ivdPVD.ucAppID,
          pvd.ivdPVD.ucCopyRightID,
          pvd.ivdPVD.ucAbstractID,
          pvd.ivdPVD.ucBiblioID,
          pvd.ivdPVD.ucCreateDate,
          pvd.ivdPVD.ucModDate,
          pvd.ivdPVD.ucExpDate,
          pvd.ivdPVD.ucEffDate,
          pvd.ivdPVD.ucStdVer);

  printf ("\n\n[Root Directory]"
          "\n  Directory Entry Length: %u"
          "\n  XAR Length            : %u"
          "\n  Start Data Extent  LSB: %08xh"
          "\n                     MSB: %08xh"
          "\n  Filesize           LSB: %08xh"
          "\n                     MSB: %08xh"
          "\n  Date (YYYY/MM/DD)     : %04u/%02u/%02u"
          "\n  Time (HH/MM/SS)       : %02u:%02u.%02u",
          pvd.ivdPVD.idRootDir.ucDirRecLen,
          pvd.ivdPVD.idRootDir.ucXARLen,
          pvd.ivdPVD.idRootDir.ulLocExtLSB,
          pvd.ivdPVD.idRootDir.ulLocExtMSB,
          pvd.ivdPVD.idRootDir.ulDataLenLSB,
          pvd.ivdPVD.idRootDir.ulDataLenMSB,
          pvd.ivdPVD.idRootDir.ucDateYear + 1900,
          pvd.ivdPVD.idRootDir.ucDateMonth,
          pvd.ivdPVD.idRootDir.ucDateDay,
          pvd.ivdPVD.idRootDir.ucDateHour,
          pvd.ivdPVD.idRootDir.ucDateMinute,
          pvd.ivdPVD.idRootDir.ucDateSecond);

  printf ("\n  Offset to GMT         : %uh %um",
          pvd.ivdPVD.idRootDir.ucDateGMT >> 2,
          15 * (pvd.ivdPVD.idRootDir.ucDateGMT & 0x03));

  printf ("\n  Flags                 : %08xh"
          "\n  File Unit Size        : %u"
          "\n  Interleave Gap Size   : %u"
          "\n  Volume Sequence    LSB: %08xh"
          "\n                     MSB: %08xh",
          pvd.ivdPVD.idRootDir.ucFileFlags,
          pvd.ivdPVD.idRootDir.ucFileUnitSize,
          pvd.ivdPVD.idRootDir.ucIlGapSize,
          pvd.ivdPVD.idRootDir.usVolSeqLSB,
          pvd.ivdPVD.idRootDir.usVolSeqMSB);

  return (NO_ERROR); /* OK */
}



/***********************************************************************
 * Name      : APIRET CDROMControl
 * Funktion  : Wrapper fÅr alle CD-Unterfunktionen
 * Parameter : ULONG ulCommand
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 25.09.1995 01.31.18]
 ***********************************************************************/

APIRET CDROMControl (void)
{
  APIRET rc;                                    /* RÅckgabewert - Fehlercode */
  HFILE  hCDROM;                                     /* Handle fÅr das CDROM */
  ULONG  ulAction;                               /* Dummy fÅr DosOpen-Aktion */
  ULONG  ulParmLen;                                   /* ParameterpaketlÑnge */
  ULONG  ulDataLen;                                       /* DatenblocklÑnge */

                       /* Steht in pszCDROM bereits eine brauchbare Angabe ? */
  if (Options.pszCDROM == NULL)
  {
    printf ("\nNo valid CDROM device specified.");
    return (ERROR_INVALID_PARAMETER);
  }

                                         /* a) Handle fÅr das Device holen ! */
  printf ("\nOpening [%s] as CD-ROM...",
          Options.pszCDROM);

  if ( (Options.pszCDROM[0] >= '0') &&        /* check available options for */
       (Options.pszCDROM[0] <= '9') )       /* this alternate operation mode */
  {
    if (Options.fsCmdEject)                                 /* check command */
    {
      rc = CDDiskControl(Options.pszCDROM[0],
                         0x02);                               /* eject cdrom */
      if (rc != NO_ERROR)                                /* check for errors */
        ErrorCD(rc);
    }

    if (Options.fsCmdCloseTray)                             /* check command */
    {
      rc = CDDiskControl(Options.pszCDROM[0],
                         0x03);                               /* close cdrom */
      if (rc != NO_ERROR)                                /* check for errors */
        ErrorCD(rc);
    }

    if (Options.fsCmdLock)                                  /* check command */
    {
      rc = CDDiskControl(Options.pszCDROM[0],
                         0x01);                               /* lock  cdrom */
      if (rc != NO_ERROR)                                /* check for errors */
        ErrorCD(rc);
    }

    if (Options.fsCmdUnlock)                                /* check command */
    {
      rc = CDDiskControl(Options.pszCDROM[0],
                         0x00);                              /* unlock cdrom */
      if (rc != NO_ERROR)                                /* check for errors */
        ErrorCD(rc);
    }

    return (rc);                                           /* OK, abort here */
  }

  rc = DosOpen(Options.pszCDROM,                              /* open driver */
               &hCDROM,
               &ulAction,
               0,
               FILE_NORMAL,
               FILE_OPEN,
               OPEN_ACCESS_READONLY |
               OPEN_FLAGS_DASD      |
               OPEN_SHARE_DENYNONE,
               NULL);

  if (rc == NO_ERROR)                            /* Wenn alles i.o. dann ... */
  {
    if (Options.fsCmdEject)                                 /* check command */
    {
      rc = CDEject(hCDROM);                                   /* eject cdrom */
      if (rc != NO_ERROR)                                /* check for errors */
        ErrorCD(rc);
    }

   if (Options.fsCmdPVD)                                    /* check command */
    {
      rc = CDPvdInfo(hCDROM);                                    /* read PVD */
      if (rc != NO_ERROR)                                /* check for errors */
        ErrorCD(rc);
    }


    if (Options.fsCmdReset)                                 /* check command */
    {
      rc = CDReset(hCDROM);                                   /* reset cdrom */
      if (rc != NO_ERROR)                                /* check for errors */
        ErrorCD(rc);
    }

    if (Options.fsCmdCloseTray)                             /* check command */
    {
      rc = CDCloseTray(hCDROM);                               /* close cdrom */
      if (rc != NO_ERROR)                                /* check for errors */
        ErrorCD(rc);
    }

    if (Options.fsCmdLock)                                  /* check command */
    {
      rc = CDLockUnlock(hCDROM,1);                            /* lock  cdrom */
      if (rc != NO_ERROR)                                /* check for errors */
        ErrorCD(rc);
    }

    if (Options.fsCmdUnlock)                                /* check command */
    {
      rc = CDLockUnlock(hCDROM,0);                           /* unlock cdrom */
      if (rc != NO_ERROR)                                /* check for errors */
        ErrorCD(rc);
    }

    if (Options.fsCmdSeek)                                  /* check command */
    {
      rc = CDSeek(hCDROM,atoi(Options.pszSeek));             /* seek for trk */
      if (rc != NO_ERROR)                                /* check for errors */
        ErrorCD(rc);
    }

    if (Options.fsCmdStatus)                                /* check command */
    {
      rc = CDDeviceStatus(hCDROM);                           /* status cdrom */
      if (rc != NO_ERROR)                                /* check for errors */
        ErrorCD(rc);
    }

    if (Options.fsCmdPlay)                                  /* check command */
    {
      rc = CDAudioPlay(hCDROM,
                       0,
                       0,
                       0xFFFFFFFF);                            /* play audio */
      if (rc != NO_ERROR)                                /* check for errors */
        ErrorCD(rc);
    }

    if (Options.fsCmdStop)                                  /* check command */
    {
      rc = CDAudioStop(hCDROM);                                /* stop audio */
      if (rc != NO_ERROR)                                /* check for errors */
        ErrorCD(rc);
    }

    if (Options.fsCmdResume)                                /* check command */
    {
      rc = CDAudioResume(hCDROM);                            /* resume audio */
      if (rc != NO_ERROR)                                /* check for errors */
        ErrorCD(rc);
    }

    if (Options.fsCmdStop)                                  /* check command */
    {
      rc = CDAudioStop(hCDROM);                                /* stop audio */
      if (rc != NO_ERROR)                                /* check for errors */
        ErrorCD(rc);
    }

    if (Options.fsCmdVol0)
    {
      /* @@@PH */
    }

    if (Options.fsCmdVol1)
    {
      /* @@@PH */
    }

    if (Options.fsCmdVol2)
    {
      /* @@@PH */
    }

    if (Options.fsCmdVol3)
    {
      /* @@@PH */
    }

    DosClose (hCDROM);
  }
  else
    ToolsErrorDos (rc);

  return (rc);                                 /* RÅckgabewert liefern */
} /* APIRET CDROMControl */


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

  if ( (Options.pszCDROM == NULL) ||         /* check if user specified file */
       (Options.fsHelp) )
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  rc = CDROMControl();                                         /* Los geht's */
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  return (rc);
}
