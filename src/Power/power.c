/***********************************************************************
 * Projekt   : PHS Tools
 * Name      : Module Power Management
 * Funktion  : Accesses OS/2's Advanced Power Management from command line
 * Autor     : Patrick Haller [Montag, 25.09.1995 01.20.20]
 ***********************************************************************/

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

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"

#define MAXPATHLEN 260


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsCmdEnable;                            /* enable power management */
  ARGFLAG fsCmdDisable;                          /* disable power management */
  ARGFLAG fsCmdBIOS;                                /* restore BIOS defaults */
  ARGFLAG fsCmdLow;                                /* send battery low event */
  ARGFLAG fsCmdNormal;                           /* send normal resume event */
  ARGFLAG fsCmdCritical;                       /* send critical resume event */
  ARGFLAG fsCmdInfo;                                     /* query APM system */

  ARGFLAG fsCommand;                                  /* command specified ? */
  ARGFLAG fsDevice;                              /* the device was specified */
  PSZ     pszDevice;                                      /* the device name */
  PSZ     pszCommand;                                     /* the command str */
} OPTIONS, *POPTIONS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */

ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung----------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/ENABLE",    "Enable APM functions.",NULL,               ARG_NULL,       &Options.fsCmdEnable},
  {"/DISABLE",   "Disable APM functions.",NULL,              ARG_NULL,       &Options.fsCmdDisable},
  {"/BIOS",      "Restore APM BIOS defaults.",NULL,          ARG_NULL,       &Options.fsCmdBIOS},
  {"/LOW",       "Send battery low event.",NULL,             ARG_NULL,       &Options.fsCmdLow},
  {"/NORMAL",    "Send normal resume event.",NULL,           ARG_NULL,       &Options.fsCmdNormal},
  {"/CRITICAL",  "Send critical resume event.",NULL,         ARG_NULL,       &Options.fsCmdCritical},
  {"/INFO",      "Query APM status.",  NULL,                 ARG_NULL,       &Options.fsCmdInfo},
  {"1",          "Device BIOS,ALL,DISPLAY.x,STORAGE.x,PARALLEL.x,SERIAL.x",
                                       &Options.pszDevice,   ARG_PSZ |
                                                             ARG_DEFAULT,    &Options.fsDevice},
  {"2",          "FULL,STANDBY,SUSPEND,OFF",
                                       &Options.pszCommand,  ARG_PSZ |
                                                             ARG_DEFAULT,    &Options.fsCommand},
  ARG_TERMINATE
};


#define CMD_POWER_FULL    0
#define CMD_POWER_STANDBY 1
#define CMD_POWER_SUSPEND 2
#define CMD_POWER_OFF     3
#define CMD_POWER_INVALID -1


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
  TOOLVERSION("Power",                                   /* application name */
              0x00010002,                            /* application revision */
              0x00010100,           /* minimum required revision of PHSTOOLS */
              0x0001FFFF,        /* maximum recommended revision of PHSTOOLS */
              NULL,                                                /* Remark */
              NULL);                                 /* additional copyright */
}


/***********************************************************************
 * Name      : void ErrorPower
 * Funktion  : Stellt Fehlermeldungen des APM-Treibers zur VerfÅgung.
 * Parameter : APIRET rc
 * Variablen :
 * Ergebnis  : kenies
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 21.10.1995 18.53.20]
 ***********************************************************************/

void ErrorPower (APIRET rc)
{
  ULONG ulMsg;                                        /* Messagenummer */

  const static PSZ szError[] = {
    /* 00 */ "No error",
    /* 01 */ "Bad Sub ID",
    /* 02 */ "Bad Reserved",
    /* 03 */ "Bad Device ID",
    /* 04 */ "Bad Power State",
    /* 05 */ "powererror 05",
    /* 06 */ "powererror 06",
    /* 07 */ "powererror 07",
    /* 08 */ "powererror 08",
    /* 09 */ "Advanced Power Management disabled",
    /* 0a */ "powererror 0a",
    /* 0b */ "powererror 0b",
    /* 0c */ "powererror 0c",
    /* 0d */ "powererror 0d",
    /* 0e */ "powererror 0e",
    /* 0f */ "Request not supported",
    /* 10 */ "powererror 10",
    /* 11 */ "powererror 11",
    /* 12 */ "powererror 12",
    /* 13 */ "powererror 13",
    /* 14 */ "powererror 14",
    /* 15 */ "powererror 15",
    /* 16 */ "powererror 16",
    /* 17 */ "powererror 17",
    /* 18 */ "powererror 18",
    /* 19 */ "powererror 19",
    /* 1a */ "powererror 1a",
    /* 1b */ "powererror 1b",
    /* 1c */ "powererror 1c",
    /* 1d */ "powererror 1d",
    /* 1e */ "powererror 1e",
    /* 1f */ "powererror 1f"};

  ulMsg = rc & 0x00ff;                      /* Treibermeldung filtern */

  if (ulMsg < 0x20)                      /* Innerhalb des Bereiches ? */
    printf ("\nError: %04x - (%s).",
        ulMsg,
        szError[ulMsg]);
}


/***********************************************************************
 * Name      : APIRET PowerSendEventShort
 * Funktion  : Send an APM event
 * Parameter : HFILE hPower
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung : Spart viel Code...
 *
 * Autor     : Patrick Haller [Montag, 23.10.1995 17.40.32]
 ***********************************************************************/

APIRET PowerSendEventShort (HFILE hPower,
                            ULONG ulParam1)
{
  ULONG  ulParmRetLen;                                /* ParameterpaketlÑnge */
  ULONG  ulDataRetLen;                                /* ParameterpaketlÑnge */
  APIRET rc;                                                 /* RÅckgabewert */

  struct _ParamBlock
  {
    ULONG ulParam1;                          /* parameter 1 in request block */
    ULONG ulParam2;                          /* parameter 2 in request block */
  } sParamBlock;

  struct _DataBlock
  {
    USHORT usReturn;                         /* returncode in the data block */
  } sDataBlock;

          /* This is the direct way to communicate with the driver via IOCtl */
  ulParmRetLen = sizeof(sParamBlock);
  ulDataRetLen = sizeof(sDataBlock);

  sParamBlock.ulParam1 = ulParam1;
  sParamBlock.ulParam2 = 0;

  rc = DosDevIOCtl(hPower,                                  /* Device-Handle */
                   IOCTL_POWER,                                  /* Category */
                   POWER_SENDPOWEREVENT,                         /* Function */
                   (PVOID)&sParamBlock,            /* Parameterblock-Pointer */
                   sizeof(sParamBlock),    /* Max. LÑnge der Parameterblocks */
                   &ulParmRetLen,   /* Pointer auf LÑnge des Parameterblocks */
                   (PVOID)&sDataBlock,                         /* Datenblock */
                   sizeof(sDataBlock),     /* Maximale LÑnge des Datenblocks */
                   &ulDataRetLen);      /* Pointer auf LÑnge des Datenblocks */

  if (rc == ERROR_INVALID_PARAMETER)         /* check data block return code */
  {
    if (sDataBlock.usReturn != NO_ERROR)      /* check for subsequent errors */
      ErrorPower(sDataBlock.usReturn);                /* yield error message */
  }

  return (rc);                                       /* RÅckgabewert liefern */
} /* APIRET PowerSendEventShort */


/***********************************************************************
 * Name      : APIRET PowerSendEvent
 * Funktion  : Send an APM event
 * Parameter : HFILE hPower
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung : Spart viel Code...
 *
 * Autor     : Patrick Haller [Montag, 23.10.1995 17.40.32]
 ***********************************************************************/

APIRET PowerSendEvent (HFILE hPower,
                       ULONG ulParam1,
                       ULONG ulParam2)
{
  ULONG  ulParmRetLen;                                /* ParameterpaketlÑnge */
  ULONG  ulDataRetLen;                                /* ParameterpaketlÑnge */
  APIRET rc;                                                 /* RÅckgabewert */

  struct _ParamBlock
  {
    ULONG ulParam1;                          /* parameter 1 in request block */
    ULONG ulParam2;                          /* parameter 2 in request block */
  } sParamBlock;

  struct _DataBlock
  {
    USHORT usReturn;                         /* returncode in the data block */
  } sDataBlock;

          /* This is the direct way to communicate with the driver via IOCtl */
  ulParmRetLen = sizeof(sParamBlock);
  ulDataRetLen = sizeof(sDataBlock);

  sParamBlock.ulParam1 = ulParam1;
  sParamBlock.ulParam2 = ulParam2;

  rc = DosDevIOCtl(hPower,                                  /* Device-Handle */
                   IOCTL_POWER,                                  /* Category */
                   POWER_SENDPOWEREVENT,                         /* Function */
                   (PVOID)&sParamBlock,            /* Parameterblock-Pointer */
                   sizeof(sParamBlock),    /* Max. LÑnge der Parameterblocks */
                   &ulParmRetLen,   /* Pointer auf LÑnge des Parameterblocks */
                   (PVOID)&sDataBlock,                         /* Datenblock */
                   sizeof(sDataBlock),     /* Maximale LÑnge des Datenblocks */
                   &ulDataRetLen);      /* Pointer auf LÑnge des Datenblocks */

  if (rc == ERROR_INVALID_PARAMETER)         /* check data block return code */
  {
    if (sDataBlock.usReturn != NO_ERROR)      /* check for subsequent errors */
      ErrorPower(sDataBlock.usReturn);                /* yield error message */
  }

  return (rc);                                       /* RÅckgabewert liefern */
} /* APIRET PowerSendEvent */


/***********************************************************************
 * Name      : ULONG PowerParseCommand
 * Funktion  : Convert a command string to command code
 * Parameter : PSZ pszCommand
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 23.10.1995 17.40.32]
 ***********************************************************************/

ULONG PowerParseCommand (PSZ pszCommand)
{
  if (stricmp("FULL",   pszCommand) == 0) return CMD_POWER_FULL;
  if (stricmp("STANDBY",pszCommand) == 0) return CMD_POWER_STANDBY;
  if (stricmp("SUSPEND",pszCommand) == 0) return CMD_POWER_SUSPEND;
  if (stricmp("OFF",    pszCommand) == 0) return CMD_POWER_OFF;
  return (CMD_POWER_INVALID);
}


/***********************************************************************
 * Name      : PSZ PowerSubIDToString
 * Funktion  : Convert a SubID to a string
 * Parameter : ULONG ulSubID
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 23.10.1995 17.40.32]
 ***********************************************************************/

PSZ PowerSubIDToString (ULONG ulSubID)
{
  const static PSZ pszSubIDTable[] =
  {/* 03 */ "Enable Power Management Functions",
   /* 04 */ "Disable Power Management Functions",
   /* 05 */ "Restore BIOS Defaults",
   /* 06 */ "Set Power State"};

  if ( (ulSubID < 3) ||                             /* check the index first */
       (ulSubID > 6) )
    return ("<invalid>");
  else
    return (pszSubIDTable[ulSubID-3]);             /* else return from table */
}


/***********************************************************************
 * Name      : PSZ PowerStateToString
 * Funktion  : Convert a State to a string
 * Parameter : ULONG ulState
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 23.10.1995 17.40.32]
 ***********************************************************************/

PSZ PowerStateToString (ULONG ulState)
{
  const static PSZ pszStateTable[] =
  {/* 00 */ "READY",
   /* 01 */ "STANDBY",
   /* 02 */ "SUSPEND",
   /* 03 */ "OFF"};

  if ( ulState > 3)
    return ("<invalid>");
  else
    return (pszStateTable[ulState]);               /* else return from table */
}


/***********************************************************************
 * Name      : PSZ PowerDevIDToString
 * Funktion  : Convert a DevID to a string
 * Parameter : ULONG ulDevID
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung : NO MULTITHREADING !
 *
 * Autor     : Patrick Haller [Montag, 23.10.1995 17.40.32]
 ***********************************************************************/

PSZ PowerDevIDToString (ULONG ulDevID)
{
  USHORT usDevID = ulDevID & 0x0000FFFF;

  static PSZ pszDevDisplay = "Display Device #00\0\0\0\0\0\0\0\0";
  static PSZ pszDevStorage = "Storage Device #00\0\0\0\0\0\0\0\0";
  static PSZ pszDevParallel= "Parallel Device #00\0\0\0\0\0\0\0\0";
  static PSZ pszDevSerial  = "Serial Device #00\0\0\0\0\0\0\0\0";


  if (usDevID == 0x0000)          /* the power management system BIOS itself */
    return ("Power Management System BIOS");

  if (usDevID == 0x0001)          /* devices managed by the power management */
    return ("Devices managed by the Power Management BIOS");

  if (usDevID & 0x0100)                                  /* display device ? */
  {
    sprintf (pszDevDisplay+16,
             "%2u",
             usDevID & 0x00FF);

    return (pszDevDisplay);
  }

  if (usDevID & 0x0200)                                  /* storage device ? */
  {
    sprintf (pszDevStorage+16,
             "%2u",
             usDevID & 0x00FF);

    return (pszDevStorage);
  }

  if (usDevID & 0x0300)                                  /* storage device ? */
  {
    sprintf (pszDevParallel+17,
             "%2u",
             usDevID & 0x00FF);

    return (pszDevParallel);
  }

  if (usDevID & 0x0400)                                  /* storage device ? */
  {
    sprintf (pszDevSerial+15,
             "%2u",
             usDevID & 0x00FF);

    return (pszDevSerial);
  }

  return ("<unknown>");                                    /* unknown device */
}


/***********************************************************************
 * Name      : USHORT PowerStringToDevID
 * Funktion  : Convert a string to a DevID
 * Parameter : PSZ pszDevice
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung : NO MULTITHREADING !
 *
 * Autor     : Patrick Haller [Montag, 23.10.1995 17.40.32]
 ***********************************************************************/

USHORT PowerStringToDevID (PSZ pszString)
{
  USHORT usDevice;                                      /* the device number */
  PSZ    pszDot;               /* points to the dot within the device string */

  if (stricmp("BIOS", pszString) == 0) return 0x0000;            /* APM BIOS */
  if (stricmp("ALL",  pszString) == 0) return 0x0001;  /* power managed devs */

  pszDot = strchr(pszString,'.');                            /* find the dot */
  if (pszDot != NULL)                                   /* if there is a dot */
    usDevice = atoi(pszDot+1);                   /* then convert to a number */
  else
    usDevice = 0x0000;                        /* else we default to device 0 */

  if (strnicmp("DISPLAY", pszString,7) == 0) return (0x0100 | usDevice);
  if (strnicmp("STORAGE", pszString,7) == 0) return (0x0200 | usDevice);
  if (strnicmp("DISK",    pszString,4) == 0) return (0x0200 | usDevice);
  if (strnicmp("HD",      pszString,2) == 0) return (0x0200 | usDevice);
  if (strnicmp("HARDDISK",pszString,8) == 0) return (0x0200 | usDevice);
  if (strnicmp("PARALLEL",pszString,8) == 0) return (0x0300 | usDevice);
  if (strnicmp("SERIAL",  pszString,6) == 0) return (0x0400 | usDevice);

  return (0xFFFF);                          /* this indicates unknown device */
}


/***********************************************************************
 * Name      : APIRET PowerGetInfo
 * Funktion  : Query APM status
 * Parameter : HFILE hPower
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung : Spart viel Code...
 *
 * Autor     : Patrick Haller [Montag, 23.10.1995 17.40.32]
 ***********************************************************************/

APIRET PowerGetInfo (HFILE hPower)
{
  ULONG  ulParmRetLen;                                /* ParameterpaketlÑnge */
  ULONG  ulDataRetLen;                                /* ParameterpaketlÑnge */
  APIRET rc;                                                 /* RÅckgabewert */

  struct _ParamBlock1
  {
    USHORT usParamLen;                          /* length of parameter block */
    USHORT usBIOSFlags;                                        /* BIOS Flags */
    USHORT usBIOSVersion;                                    /* BIOS Version */
    USHORT usSubSysVersion;                             /* Subsystem Version */
  } sParamBlock1;

  struct _ParamBlock2
  {
    USHORT usParamLen;                          /* length of parameter block */
    USHORT usPowerFlags;
    UCHAR  ucACStatus;
    UCHAR  ucBatteryStatus;
    UCHAR  ucBatteryLife;
  } sParamBlock2;

  struct _ParamBlock3
  {
    USHORT usParamLen;                          /* length of parameter block */
    USHORT usMessageCount;              /* remaining events in the APM queue */
    ULONG  ulParam1;                     /* parameter 1 as in PowerSendEvent */
    ULONG  ulParam2;                     /* parameter 2 as in PowerSendEvent */
  } sParamBlock3;


  struct _DataBlock1
  {
    USHORT usReturn;                         /* returncode in the data block */
  } sDataBlock1;


  /***************************************************************************
   * Query APM system for general information                                *
   ***************************************************************************/

  ulParmRetLen = sizeof(sParamBlock1);
  ulDataRetLen = sizeof(sDataBlock1);

  sParamBlock1.usParamLen = sizeof(sParamBlock1);

  rc = DosDevIOCtl(hPower,                                  /* Device-Handle */
                   IOCTL_POWER,                                  /* Category */
                   POWER_GETPOWERINFO,                           /* Function */
                   (PVOID)&sParamBlock1,           /* Parameterblock-Pointer */
                   sizeof(sParamBlock1),   /* Max. LÑnge der Parameterblocks */
                   &ulParmRetLen,   /* Pointer auf LÑnge des Parameterblocks */
                   (PVOID)&sDataBlock1,                        /* Datenblock */
                   sizeof(sDataBlock1),    /* Maximale LÑnge des Datenblocks */
                   &ulDataRetLen);      /* Pointer auf LÑnge des Datenblocks */

  if (rc == ERROR_INVALID_PARAMETER)         /* check data block return code */
  {
    if (sDataBlock1.usReturn != NO_ERROR)     /* check for subsequent errors */
      ErrorPower(sDataBlock1.usReturn);               /* yield error message */
  }
  else
  {
    /*************************************************************************
     * Query APM system for current status                                   *
     *************************************************************************/

    ulParmRetLen = sizeof(sParamBlock2);
    ulDataRetLen = sizeof(sDataBlock1);

    sParamBlock2.usParamLen = sizeof(sParamBlock2);

    rc = DosDevIOCtl(hPower,                                /* Device-Handle */
                     IOCTL_POWER,                                /* Category */
                     POWER_GETPOWERSTATUS,                       /* Function */
                     (PVOID)&sParamBlock2,         /* Parameterblock-Pointer */
                     sizeof(sParamBlock2), /* Max. LÑnge der Parameterblocks */
                     &ulParmRetLen, /* Pointer auf LÑnge des Parameterblocks */
                     (PVOID)&sDataBlock1,                      /* Datenblock */
                     sizeof(sDataBlock1),  /* Maximale LÑnge des Datenblocks */
                     &ulDataRetLen);    /* Pointer auf LÑnge des Datenblocks */

    if (rc == ERROR_INVALID_PARAMETER)       /* check data block return code */
    {
      if (sDataBlock1.usReturn != NO_ERROR)   /* check for subsequent errors */
        ErrorPower(sDataBlock1.usReturn);             /* yield error message */
    }
    else
    {
      PSZ pszAC;                                 /* temporary string pointer */
      PSZ pszBattery;                            /* temporary string pointer */

      printf ("\nAPM Information:"
              "\n  BIOS Flags        0x%08x"
              "\n  BIOS Version      %u.%u"
              "\n  Subsystem Version %u.%u",
              sParamBlock1.usBIOSFlags,
              sParamBlock1.usBIOSVersion >> 8,
              sParamBlock1.usBIOSVersion & 0x00FF,
              sParamBlock1.usSubSysVersion >> 8,
              sParamBlock1.usSubSysVersion & 0x00FF);

      switch (sParamBlock2.ucACStatus)
      {
        case 0:   pszAC = "AC offline"; break;
        case 1:   pszAC = "AC online";  break;
        case 255: pszAC = "unknown";    break;
        default:  pszAC = "<invalid>";  break;
      }

      switch (sParamBlock2.ucBatteryStatus)
      {
        case 0:   pszBattery = "High";      break;
        case 1:   pszBattery = "Low";       break;
        case 2:   pszBattery = "Critical";  break;
        case 3:   pszBattery = "Charging";  break;
        case 255: pszBattery = "unknown";   break;
        default:  pszBattery = "<invalid>"; break;
      }

      printf ("\nAPM Status:"
              "\n  AC Status         %s (%u)"
              "\n  Power Flags       0x%08x",
              pszAC,
              sParamBlock2.ucACStatus,
              sParamBlock2.usPowerFlags);

      printf ("\n    Power Management is ");

      if (sParamBlock2.usPowerFlags & 0x0001)
        printf ("ENABLED.");
      else
        printf ("DISABLED.");

      if (sParamBlock2.ucACStatus != 1)          /* PowerSupply not online ! */
        printf("\n  Battery Status    %s (%u)"
               "\n  Battery Life      %u%%",
               pszBattery,
               sParamBlock2.ucBatteryStatus,
               sParamBlock2.ucBatteryLife);
    }
  }


  /***************************************************************************
   * Query APM system for queued power events                                *
   ***************************************************************************/

  ulParmRetLen = sizeof(sParamBlock3);
  ulDataRetLen = sizeof(sDataBlock1);

  sParamBlock3.usParamLen = sizeof(sParamBlock3);

  printf ("\nAPM Messages:");

  rc = DosDevIOCtl(hPower,                                /* Device-Handle */
                   IOCTL_POWER,                                /* Category */
                   POWER_GETPOWERINFO,                         /* Function */
                   (PVOID)&sParamBlock3,         /* Parameterblock-Pointer */
                   sizeof(sParamBlock3), /* Max. LÑnge der Parameterblocks */
                   &ulParmRetLen, /* Pointer auf LÑnge des Parameterblocks */
                   (PVOID)&sDataBlock1,                      /* Datenblock */
                   sizeof(sDataBlock1),  /* Maximale LÑnge des Datenblocks */
                   &ulDataRetLen);    /* Pointer auf LÑnge des Datenblocks */

  if (rc == ERROR_INVALID_PARAMETER)       /* check data block return code */
  {
    if (sDataBlock1.usReturn != NO_ERROR)   /* check for subsequent errors */
      ErrorPower(sDataBlock1.usReturn);             /* yield error message */
  }

  printf ("\n  %3u 0x%08x %-16s",
          sParamBlock3.usMessageCount,
          sParamBlock3.ulParam1,
          PowerSubIDToString(sParamBlock3.ulParam1));
  if (sParamBlock3.ulParam1 & 0x0000FFFF == 0x0006)   /* set power state ? */
    printf ("  Device-ID: 0x%04x %s to state 0x%4x %s",
            sParamBlock3.ulParam2 & 0xFFFF,
            PowerDevIDToString(sParamBlock3.ulParam2 & 0xFFFF),
            sParamBlock3.ulParam2 >> 16,
            PowerStateToString( (sParamBlock3.ulParam2 >> 16) & 0xFFFF) );

  return (rc);                                       /* RÅckgabewert liefern */
} /* APIRET PowerSendEvent */


/***********************************************************************
 * Name      : APIRET PowerControl
 * Funktion  : Wrapper fÅr alle CD-Unterfunktionen
 * Parameter : ULONG ulCommand
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 25.09.1995 01.31.18]
 ***********************************************************************/

APIRET PowerControl (void)
{
  APIRET rc;                                    /* RÅckgabewert - Fehlercode */
  HFILE  hPower;                                     /* Handle fÅr das Power */
  ULONG  ulAction;                               /* Dummy fÅr DosOpen-Aktion */

  rc = DosOpen("\\DEV\\APM$",
               &hPower,
               &ulAction,
               0,
               FILE_NORMAL,
               FILE_OPEN,
               OPEN_ACCESS_READONLY |
               OPEN_SHARE_DENYNONE,
               NULL);
  if (rc != NO_ERROR)                                    /* check for errors */
  {
    fprintf (stderr,                            /* yet another error message */
             "\nError: Cannot access Advanced Power Management driver.");
    return (rc);
  }


  /***************************************************************************
   * process commands                                                        *
   ***************************************************************************/

  if (Options.fsCmdEnable)
  {
    printf ("\nEnabling Power Management functions.");
    rc = PowerSendEventShort(hPower,                      /* send this event */
                             0x00000003);
    if (rc != NO_ERROR)                                  /* check for errors */
      ToolsErrorDos(rc);                              /* yield error message */
  }


  if (Options.fsCmdDisable)
  {
    printf ("\nDisabling Power Management functions.");
    rc = PowerSendEventShort(hPower,                      /* send this event */
                             0x00000004);                        /* reserved */
    if (rc != NO_ERROR)                                  /* check for errors */
      ToolsErrorDos(rc);                              /* yield error message */
  }


  if (Options.fsCmdBIOS)
  {
    printf ("\nRestore BIOS defaults.");
    rc = PowerSendEventShort(hPower,                      /* send this event */
                             0x00000005);                        /* reserved */
    if (rc != NO_ERROR)                                  /* check for errors */
      ToolsErrorDos(rc);                              /* yield error message */
  }


  if (Options.fsCmdLow)
  {
    printf ("\nSending battery low event.");
    rc = PowerSendEventShort(hPower,                      /* send this event */
                             0x00000007);                        /* reserved */
    if (rc != NO_ERROR)                                  /* check for errors */
      ToolsErrorDos(rc);                              /* yield error message */
  }


  if (Options.fsCmdNormal)
  {
    printf ("\nSending normal resume event.");
    rc = PowerSendEventShort(hPower,                      /* send this event */
                            0x00000008);
    if (rc != NO_ERROR)                                  /* check for errors */
      ToolsErrorDos(rc);                              /* yield error message */
  }


  if (Options.fsCmdCritical)
  {
    printf ("\nSending critical resume event.");
    rc = PowerSendEventShort(hPower,                      /* send this event */
                             0x00000009);
    if (rc != NO_ERROR)                                  /* check for errors */
      ToolsErrorDos(rc);                              /* yield error message */
  }


  if (Options.fsCmdInfo)
  {
    rc = PowerGetInfo(hPower);                            /* send this event */
    if (rc != NO_ERROR)                                  /* check for errors */
      ToolsErrorDos(rc);                              /* yield error message */
  }


  if (Options.fsDevice)                                /* device specified ? */
  {
    if (Options.fsCommand)                           /* however no command ? */
    {
      ULONG ulDevice;                               /* the APM device number */
      ULONG ulCommand;                               /* the APM command code */

      ulDevice = PowerStringToDevID(Options.pszDevice);    /* convert device */

      if (ulDevice != 0x0000FFFF)                        /* check for errors */
      {
        ulCommand = PowerParseCommand(Options.pszCommand);    /* get command */
        if (ulCommand != CMD_POWER_INVALID)
        {
          printf ("\nSetting %s to %s.",
                  PowerDevIDToString(ulDevice),
                  PowerStateToString(ulCommand));

          if ( (ulDevice & 0x0200) &&                    /* storage device ? */
               (ulCommand != CMD_POWER_FULL) )           /* and power-down ? */
          {
            printf ("\n Flushing filesystem caches ...");   /* message first */
            rc = DosShutdown(1L);          /* flushing the filesystem caches */
            if (rc != NO_ERROR)                          /* check for errors */
              ToolsErrorDos(rc);                      /* yield error message */
          }

          rc = PowerSendEvent(hPower,
                              0x00000006,
                              ulCommand << 16 | ulDevice);
          if (rc != NO_ERROR)                            /* check for errors */
            ToolsErrorDos(rc);                        /* yield error message */
        }
        else
          fprintf (stderr,
                   "\nError: unknown command specified.");
      }
      else
        fprintf (stderr,
                 "\nError: unknown device specified.");
    }
    else
    {
      fprintf(stderr,                                 /* yield error message */
              "\nError: no command specified for the device.");
    }
  }


  DosClose (hPower);

  return (NO_ERROR);                                 /* RÅckgabewert liefern */
} /* APIRET PowerControl */


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

                                                /* do some parameter mapping */
  if (!Options.fsCmdEnable &&
      !Options.fsCmdDisable &&
      !Options.fsCmdBIOS &&
      !Options.fsCmdLow &&
      !Options.fsCmdNormal &&
      !Options.fsCmdCritical &&
      !Options.fsCommand)
    Options.fsCmdInfo = TRUE;         /* then turn this on as default action */

  rc = PowerControl();                                         /* Los geht's */
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  return (rc);
}
