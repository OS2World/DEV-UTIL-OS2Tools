/*****************************************************
 * Unix-like DF Tool.                                *
 * Reports free space on all available (hard)drives. *
 * (c) 1994-95 Patrick Haller Systemtechnik          *
 *****************************************************/

/* #define DEBUG 1 */
#undef DEBUG


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#ifdef __OS2__
  #define INCL_DOSMISC
  #define INCL_DOSFILEMGR
  #define INCL_BASE
  #define INCL_NOPMAPI
  #include <os2.h>
#endif

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <process.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"


/*****************************************************************************
 * Defines                                                                   *
 *****************************************************************************/

#define MAXDRIVES 30

/* Display modes */
#define DSPMODE_STD    0
#define DSPMODE_DETAIL 1

#define MAXPATHLEN 260

#define DRV_NO_FLOPPIES 0xFFFFFFFC /* Bit 0, 1 cleared */

#define MAXPATHLEN 260


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsFloppy;                   /* scan floppy drives                  */
  ARGFLAG fsDetailed;                 /* more detailed output                */
  ARGFLAG fsDrivers;                  /* scan device drivers                 */
  ARGFLAG fsASCII;                    /* usage of ASCII characters           */
  ARGFLAG fsDrives;                   /* drive string specified              */

  PSZ  pszDrives;                                            /* drive string */
} OPTIONS, *POPTIONS;


typedef struct
{
  char display_mode;
  char scanned_drives[MAXDRIVES];
  PSZ  pszScanDrives;
} GLOBALS, *PGLOBALS;


typedef struct
{
    ULONG volSerialNo;
    BYTE  volNameLen;
    CHAR  volName[MAXPATHLEN];
} FSInfoLevel2;


typedef struct
{
    ULONG  FSid;
    ULONG  SecUnit;
    ULONG  UnitsTotal;
    ULONG  UnitsFree;
    USHORT BytesSec;
} FSInfoLevel1;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */
GLOBALS Globals;                /* this structure holds global variables     */

ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung-----------------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",          NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",          NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/F",         "Scans floppy drives.",      NULL,                 ARG_NULL,       &Options.fsFloppy},
  {"/V",         "Scan device drivers.",      NULL,                 ARG_NULL,       &Options.fsDrivers},
  {"/D",         "More detailed information.",NULL,                 ARG_NULL,       &Options.fsDetailed},
  {"/ASCII",     "Uses ASCII characters.",    NULL,                 ARG_NULL,       &Options.fsASCII},
  {"/A",         "Uses ASCII characters.",    NULL,                 ARG_NULL,       &Options.fsASCII},
  {"1",          "Drives, e.g. CDEF.",        &Options.pszDrives,   ARG_PSZ |
                                                                    ARG_DEFAULT,    &Options.fsDrives},
  ARG_TERMINATE
};



char *str_title; /* Pointer auf untenstehende Strings */
char *str_mask;
char *str_skip;
char *str_detail;
char *str_attdata;
char *str_dtitle;
char *str_dmask;

char str_dbtitle[] = "\nDeviceƒƒƒƒƒƒƒƒ¬Devicetypeƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ¬ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ";
char str_dbmask[]  = "\n%-13s ≥ %-40s ≥ %s";
char str_datitle[] = "\nDevice--------+Devicetype--------------------------------+---------------------";
char str_damask[]  = "\n%-13s | %-40s | %s";

char str_btitle[]  = "Drv¬Filesystemƒ¬Totalƒƒƒ¬Freeƒƒƒƒƒƒƒƒƒƒƒ¬Usedƒƒƒƒƒƒƒƒƒƒƒ¬Nameƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ";
char str_bmask[]   = "\n%c:%c≥%11s≥%8s≥%8s %5.1f%%≥%8s %5.1f%%≥[%s]";
char str_bskip[]   = "\nƒƒƒ≈ƒƒƒƒƒƒƒƒƒƒƒ≈ƒƒƒƒƒƒƒƒ≈ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ≈ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ≈ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ";
char str_bdetail[] = "\n   ≥Clusters:  ≥%8u≥%9u      ≥%9u      ≥%s"
                     "\n   ≥Sec/Clu%4u≥        ≥Byte/Sec%7u≥Byte/Clu%7u≥Serial %8x";
char str_battdata[]= "\n   ≥[";


char str_atitle[]= "Drv+Filesystem-+Total---+Free-----------+Used-----------+Name------------------";
char str_amask[] = "\n%c:%c|%11s|%8s|%8s %5.1f%%|%8s %5.1f%%|[%s]";
char str_askip[] = "\n---+-----------+--------+---------------+---------------+----------------------";
char str_adetail[] = "\n   |Clusters:  |%8u|%9u      |%9u      |%s"
                     "\n   |Sec/Clu%4u|        |Byte/Sec%7u|Byte/Clu%7u|Serial %8x";
char str_aattdata[]= "\n   |[";



double dTotalSpace = 0.0;
double dTotalFree  = 0.0;


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void   help                (void);

APIRET SearchDrives        (PSZ);

int    process_drives      (void);

void   DisplayInformation  (FSInfoLevel1 *,
                            FSInfoLevel2 *,
                            FSQBUFFER2 *,
                            char,
                            char);

APIRET ParamsMap           (void);

VOID   Initialize          (VOID);

int    main                (int,
                            char **);


/***********************************************************************
 * Name      : void help
 * Funktion  : Anzeigen des Hilfetextes.
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.45.24]
 ***********************************************************************/

void help (void)
{
  TOOLVERSION("DiskFree",                               /* application name */
              0x00010004,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
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
  if (!Options.fsFloppy)    /* Soll nach Floppies gesucht werden ? */
    ulDriveMap &= DRV_NO_FLOPPIES;

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
 * Name      : APIRET ParamsMap
 * Funktion  : Temporary mapping of the parameters
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.46.49]
 ***********************************************************************/

APIRET ParamsMap (void)
{
  PSZ    p;
  APIRET rc = NO_ERROR;                               /* Rueckgabewert */

  if (Options.fsASCII)
  {
    str_title   = str_atitle;
    str_mask    = str_amask;
    str_skip    = str_askip;
    str_detail  = str_adetail;
    str_attdata = str_aattdata;

    str_dmask   = str_damask;
    str_dtitle  = str_datitle;
  }

  if (Options.fsDetailed)
    Globals.display_mode = DSPMODE_DETAIL;
  else
    Globals.display_mode = DSPMODE_STD;

  if (Options.fsDrives)
    Globals.pszScanDrives = Options.pszDrives;
  else
  {
    rc = SearchDrives (Globals.scanned_drives);       /* Laufwerke ermitteln */
    if (rc != NO_ERROR)                              /* Fehler aufgetreten ? */
      return (rc);                                   /* Fehler signalisieren */
    Globals.pszScanDrives = Globals.scanned_drives;
  }

  strupr (Globals.pszScanDrives);
  p= Globals.pszScanDrives;
  while (*p)
    if ((*p < 'A') || (*p > 'Z'))
    {
      printf ("\nInvalid drive specified at [%s]",p);
      return ERROR_INVALID_PARAMETER;
    }
    else
      p++;

  return (NO_ERROR);                          /* RÅckgabewert liefern */
}


#ifdef DEBUG
void prtfsi1 (FSInfoLevel1 *f)
{
  printf ("\nFSInfoLevel1: FSid:      [%u]"
     "\n              SecUnit:   [%u]"
     "\n              UnitsTotal:[%u]"
     "\n              UnitsFree: [%u]"
     "\n              BytesSec:  [%d]",
     f->FSid,
     f->SecUnit,
     f->UnitsTotal,
     f->UnitsFree,
     f->BytesSec);
}

void prtfsi2 (FSInfoLevel2 *f)
{
  printf ("\nFSInfoLevel2: volSerialNo:[%u]"
     "\n              volNameLen: [%d]"
     "\n              volName:    [%s]",
     f->volSerialNo,
     f->volNameLen,
     f->volName);
}

void prtfsq2 (FSQBUFFER2 *f)
{
  printf ("\nFSQBUFFER2: iType:    [%hu]"
     "\n            cbName:   [%hu]"
     "\n            cbFSDName:[%hu]"
     "\n            cbFSAData:[%hu]"
     "\n            szName:   [%s]"
     "\n            rzFSDName:[%s]",
     f->iType,
     f->cbName,
     f->cbFSDName,
     f->cbFSAData,
     f->szName,
     f->szName+strlen(f->szName)+1);
}
#endif


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

int process_drives (void)
{
  char         pszDeviceName[20];        /* Puffer fÅr den Devicenamen */

#define FSQSIZE 128
  APIRET       rc = NO_ERROR;
  char         *drive;
  ULONG        ulDrive = 0;
  ULONG        ulBootDrive;
  FSInfoLevel1 fsi1;
  FSInfoLevel2 fsi2;
  PFSQBUFFER2  fsb;
  char         cBootDrive;
  ULONG        BufferLength;


  printf (str_title);

  fsb = malloc(FSQSIZE);
  if (!fsb)
  {
    printf ("\nOut of memory.");
    return ERROR_NOT_ENOUGH_MEMORY;
  }

  DosQuerySysInfo (QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,
         &ulBootDrive, sizeof(ulBootDrive));

  drive = Globals.pszScanDrives;

_next:
  while (*drive)
  {
    ulDrive = *drive - 'A' + 1;                    /* Das ist falsch ! */

#ifdef DEBUG
  printf ("\nExamining: %c: as %u",*drive,ulDrive);
#endif

    rc = DosQueryFSInfo (ulDrive,FSIL_ALLOC, &fsi1,sizeof(fsi1));
#ifdef DEBUG
    prtfsi1(&fsi1);
#endif
    switch (rc)
    {
       case NO_ERROR:
         break;

       case ERROR_NOT_READY:
         printf ("\n%c: - drive is not ready",*drive);
         drive++;
         goto _next;

       case ERROR_DISK_CHANGE:
         printf ("\n%c: - drive B: not valid",*drive);
         drive++;
         goto _next;

       case ERROR_DRIVE_LOCKED:
         printf ("\n%c: - drive is locked by another process",*drive);
         drive++;
         goto _next;

       case ERROR_INVALID_DRIVE:
         printf ("\n%c: - drive is invalid",*drive);
         drive++;
         goto _next;

       case ERROR_GEN_FAILURE:
         printf ("\n%c: - drive is not functioning",*drive);
         drive++;
         goto _next;

       case ERROR_SECTOR_NOT_FOUND:
         printf ("\n%c: - sector not found",*drive);
         drive++;
         goto _next;

      case ERROR_DEVICE_IN_USE:
         printf ("\n%c: - device in use",*drive);
         drive++;
         goto _next;

      case ERROR_BAD_COMMAND:
         printf ("\n%c: - device doesn't recognize command",*drive);
         drive++;
         goto _next;


       case 627: /* HPFS_VOLUME_DIRTY */
         printf ("\n%c: - volume is dirty, CHKDSK required",*drive);
         drive++;
         goto _next;

       case ERROR_SEEK:
         printf("\n%c: - cannot locate a specific area or track on the disk.", *drive);
         drive++;
         goto _next;

       default:
         printf ("\n%c: - error #%u occured",*drive,rc);
         drive++;
         goto _next;
    }

    rc = DosQueryFSInfo (ulDrive,
                         FSIL_VOLSER,
                         &fsi2,
                         sizeof(fsi2));

#ifdef DEBUG
    printf ("\nDosQueryFSInfo rc=%u",rc);
    prtfsi2(&fsi2);
#endif

    if (rc != NO_ERROR)                                      /* check errors */
      return (rc);

    /* NÑhere Infos erfragen */
    BufferLength = FSQSIZE;
    sprintf (pszDeviceName,
             "%c:",
             *drive);
    rc = DosQueryFSAttach (pszDeviceName,
                           0,
                           FSAIL_QUERYNAME,
                           fsb,
                           &BufferLength);

#ifdef DEBUG
    printf ("\nDosQueryFSAttach rc=%u",rc);
    prtfsq2 (fsb);
#endif

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

    /* OK - display gathered information */
    if (ulDrive == ulBootDrive)
      cBootDrive = '*';
    else
      cBootDrive = ' ';

    DisplayInformation (&fsi1,
                        &fsi2,
                        fsb,
                        cBootDrive,
                        *drive);

    drive++;
  }

  /* display totals */
  DisplayInformation (NULL,
                      NULL,
                      NULL,
                      ' ',
                      ' ');

  free (fsb);

  return (NO_ERROR);                          /* RÅckgabewert liefern */
}


/***********************************************************************
 * Name      : void str_size
 * Funktion  : Erzeugt einen String, in dem die Åbergebene Zahl mit Kilo,Mega versehen ist.
 * Parameter : char *str, ULONG value, char mode
 * Variablen :
 * Ergebnis  :
 * Bemerkung : mode schaltet die Genauigkeit um.
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.48.30]
 ***********************************************************************/

void str_size (char *str,
               double dValue,
               char mode)
{
  /* dValue means a size in bytes */
  if (!mode)
  {
    if (dValue < 10000)    /* 10000 bytes */
    {
      sprintf (str,"%6.0fb",dValue);
      return;
    }

    if (dValue < 10240000) /* 1024 kB */
    {
      sprintf (str,"%6.1fk",dValue / 1024.0);
      return;
    }

    sprintf (str,"%6.1fM",dValue / 1048576.0);
  }
  else /* value means size in kbytes */
  {
    if (dValue < 10000)    /* 10000 bytes */
    {
      sprintf (str,"%6.0fk",dValue);
      return;
    }

    if (dValue < 10240000) /* 1024 kB */
    {
      sprintf (str,"%6.1fM",dValue / 1024.0);
      return;
    }

    sprintf (str,"%6.1fG",dValue / 1048576.0);
  }
}


/***********************************************************************
 * Name      : void DisplayInformation
 * Funktion  : Anzeigen der gefundenen Detailinformation
 * Parameter : FSInfoLevel1 *fs1, FSInfoLevel2 *fs2, FSQBUFFER2 *fsb,
 *             char cBootDrive, char drive
 * Variablen :
 * Ergebnis  : Ausgabe auf dem Bildschirm
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.49.57]
 ***********************************************************************/

void DisplayInformation (FSInfoLevel1 *fs1,
                         FSInfoLevel2 *fs2,
                         FSQBUFFER2 *fsb,
                         char cBootDrive,
                         char drive)
{
  double dPercentFree;                                /* Prozentangaben */
  double dPercentUsed;
  double dTemp;          /* Temporaere Variable zur Zwischenspeicherung */

  char  UnitsTotal[8];
  char  UnitsFree[8];
  char  UnitsUsed[8];
  PSZ   pszFSDName;
  PSZ   pszFSDData;
  PSZ   pszDeviceType;
  ULONG i;


  /* display totals ? */
  if (fs1 == NULL)
  {
    if (Globals.display_mode == DSPMODE_STD)
      printf (str_skip);

    if (dTotalSpace > 0)                                 /* Prozentangaben */
    {
      dPercentFree = 100.0 * (dTotalFree / dTotalSpace);
      dPercentUsed = 100.0 - dPercentFree;
    }
    else
    {
      dPercentFree = 0.0;
      dPercentUsed = 0.0;
    }

    str_size (UnitsTotal,dTotalSpace,1);
    str_size (UnitsFree, dTotalFree,1);
    str_size (UnitsUsed, dTotalSpace - dTotalFree,1);

    printf (str_mask, '*', ' ', "totals",
            UnitsTotal,
            UnitsFree,
            dPercentFree,
            UnitsUsed,
            dPercentUsed,
            "all");
    return;                                  /* Funktion hier abbrechen */
  }

               /* update totals , totals in kB to avoid overflow at 4GB */
  dTotalSpace += (double)fs1->UnitsTotal *
                 (double)fs1->SecUnit *
                 (double)fs1->BytesSec /
                 1024.0;

  dTotalFree  += (double)fs1->UnitsFree  *
                 (double)fs1->SecUnit *
                 (double)fs1->BytesSec /
                 1024.0;

  if (fs1->UnitsTotal > 0)              /* Div 0 muss verhindert werden */
  {
    dPercentFree = 100.0 * ((double)fs1->UnitsFree / (double)fs1->UnitsTotal);
    dPercentUsed = 100.0 - dPercentFree;
  }
  else
  {
    dPercentFree = 0.0;
    dPercentUsed = 0.0;
  }

                 /* Berechnung der Prozentangaben Benutzt, Frei, Gesamt */
  dTemp = ((double)fs1->SecUnit * (double)fs1->BytesSec) / 1024.0;
  str_size (UnitsTotal,fs1->UnitsTotal                    * dTemp,1);
  str_size (UnitsFree ,fs1->UnitsFree                     * dTemp,1);
  str_size (UnitsUsed ,(fs1->UnitsTotal - fs1->UnitsFree) * dTemp,1);

                /* Calculate File System Driver Names and attached Date */
  pszFSDName = fsb->szName + strlen(fsb->szName) + 1;
  pszFSDData = pszFSDName  + strlen(pszFSDName)  + 1;

                                /* Ausgeben einer kompletten Datenzeile */
  printf (str_mask,
          drive,
          cBootDrive,
          pszFSDName,
          UnitsTotal,
          UnitsFree,
          dPercentFree,
          UnitsUsed,
          dPercentUsed,
          fs2->volName);

  if (Globals.display_mode == DSPMODE_DETAIL)
  {
    switch (fsb->iType)                            /* type of the drive */
    {
      case FSAT_CHARDEV:   pszDeviceType= "Character device";        break;
      case FSAT_PSEUDODEV: pszDeviceType= "Pseudo-character device"; break;
      case FSAT_LOCALDRV:  pszDeviceType= "Local drive";             break;
      case FSAT_REMOTEDRV: pszDeviceType= "Remote drive";            break;
      default: pszDeviceType= "Unknown device type: 00000000000";
               sprintf(pszDeviceType,"Unknown device type: %u",fsb->iType);
          break;
    }

    printf (str_detail,                         /* display the clusters */
        fs1->UnitsTotal,
        fs1->UnitsFree,
             fs1->UnitsTotal - fs1->UnitsFree,
             pszDeviceType,
        fs1->SecUnit,
        fs1->BytesSec,
        fs1->SecUnit * fs1->BytesSec,
        fs2->volSerialNo);


    if (fsb->cbFSAData)      /* is there additional data from the FSD ? */
    {
      printf (str_attdata);
      for (i=0, pszDeviceType = pszFSDData;
           i < fsb->cbFSAData;
           i++, pszDeviceType++)
        printf ("%c",*pszDeviceType);

      printf ("] ");
      for (i=0, pszDeviceType = pszFSDData;
           i < fsb->cbFSAData;
           i++, pszDeviceType++)
        printf ("%02x ",*pszDeviceType);
    }

    printf (str_skip);                        /* Zeilentrenner ausgeben */
  }
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

VOID Initialize ( VOID )
{
  /* No Harderr popups */
  DosError (FERR_DISABLEHARDERR | FERR_ENABLEEXCEPTION);

  memset (&Options,
          0,
          sizeof(Options));

  memset (&Globals,
          0,
          sizeof(Globals));

  str_title   = str_btitle;
  str_mask    = str_bmask;
  str_skip    = str_bskip;
  str_detail  = str_bdetail;
  str_attdata = str_battdata;

  str_dtitle  = str_dbtitle;
  str_dmask   = str_dbmask;
}


/***********************************************************************
 * Name      : APIRET ScanDevices
 * Funktion  : Scannt GerÑte/Devicedriver ab
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 ***********************************************************************/
APIRET ScanDevices (void)
{
    ULONG      ulBufferSize = MAXPATHLEN;               /* PufferlÑnge */
    UCHAR      achBuffer[MAXPATHLEN];                        /* Puffer */
    FSQBUFFER2 *fsb = (FSQBUFFER2 *)achBuffer;
    ULONG      i;                                    /* ZÑhlervariable */
    APIRET     rc;                                     /* RÅckgabewert */
    PSZ        pszDeviceType;                            /* GerÑtetype */
    PSZ        pszFSDName;             /* Name des angeschlossenen FSD */
    PSZ        pszFSDData;                    /* Weitere Daten vom FSD */
    PSZ        p;                                      /* Laufvariable */

    /* Start with the first driver in the list */
    i=1;

    printf ("\n");
    printf (str_dtitle);

    /* Query each driver in OS/2's list, one at a time */
    while (! (rc = DosQueryFSAttach(0,
                                    i,
                                    FSAIL_DEVNUMBER,
                                    (FSQBUFFER2 *)achBuffer,
                                    &ulBufferSize)))
    {
      /* Calculate File System Driver Names and attached Date */
      pszFSDName = fsb->szName + strlen(fsb->szName) + 1;
      pszFSDData = pszFSDName + strlen(pszFSDName) + 1;

      /* type of the drive */
      switch (fsb->iType)
      {
        case FSAT_CHARDEV:    pszDeviceType= "Character device";        break;
        case FSAT_PSEUDODEV:  pszDeviceType= "Pseudo-character device"; break;
        case FSAT_LOCALDRV:   pszDeviceType= "Local drive";             break;
        case FSAT_REMOTEDRV:  pszDeviceType= "Remote drive";            break;
        default:              pszDeviceType= "OS/2 Error - unknown device type 00000.";
                              sprintf (pszDeviceType+33,"%05u",fsb->iType);
                              break;
      }

      printf (str_dmask,fsb->szName,pszDeviceType,pszFSDName);

      /* is there additional data from the FSD ? */
      if (fsb->cbFSAData)
      {
        printf (str_attdata);
        for (i=0,p=pszFSDData;
             i < fsb->cbFSAData;
             i++,p++)
          printf ("%c",*p);

        printf ("] ");
        for (i=0,p=pszFSDData;
             i < fsb->cbFSAData;
             i++,p++)
          printf ("%02x ",*p);
      }

      /* Next driver */
      i++;
      ulBufferSize = MAXPATHLEN;
    }

    if (rc == ERROR_NO_MORE_ITEMS)                      /* Alles i.O. */
      rc = NO_ERROR;

    return (rc);
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

  Initialize ();                                          /* Initialisierung */

  rc = ArgStandard (argc,                            /* CLI-Parameter parsen */
                    argv,
                    TabArguments,
                    &Options.fsHelp);
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    exit(1);                                                /* abort program */
  }

  if ( Options.fsHelp )                      /* check if user specified file */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  rc = ParamsMap();                                    /* map the parameters */
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  rc = process_drives ();            /* Alle Laufwerke / Devices untersuchen */
  if ( (rc == NO_ERROR)  && Options.fsDrivers )
    rc = ScanDevices();

  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  return (rc);
}
