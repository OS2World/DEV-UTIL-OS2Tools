/***********************************************************************
 * Name      : Module OS2Info
 * Funktion  : Queries OEMHLP$
 * Autor     : Patrick Haller [Montag, 25.09.1995 01.20.20]
 *
 * Note      : You need SVGADEFS.H from the IBM DDK or Toolkit!
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
#include <malloc.h>
#include <conio.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"


#define CHIPS_INCL
#include <svgadefs.h>                                   /* from IBM OS/2 DDK */


#define MAXPATHLEN 260


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

void   ErrorDD         ( APIRET        rc );

APIRET OemHlpInfo      ( void );

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
  TOOLVERSION("OS2Info",                                 /* application name */
              0x00010000,                            /* application revision */
              0x00010100,           /* minimum required revision of PHSTOOLS */
              0x0001FFFF,        /* maximum recommended revision of PHSTOOLS */
              NULL,                                                /* Remark */
              NULL);                                 /* additional copyright */
}


#pragma pack(1)

/***********************************************************************
 * Name      : void ErrorDD
 * Funktion  : Stellt Fehlermeldungen des CDROM-Treibers zur VerfÅgung.
 * Parameter : APIRET rc
 * Variablen :
 * Ergebnis  : kenies
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 21.10.1995 18.53.20]
 ***********************************************************************/

void ErrorDD (APIRET rc)
{
  ULONG ulMsg;                                        /* Messagenummer */

  const static PSZ szError[] = {
    /* 00 */ "dderror 00",
    /* 01 */ "dderror 01",
    /* 02 */ "Device not ready or controller does not respond",
    /* 03 */ "Action not supported, device is not a CDROM",
    /* 04 */ "dderror 04",
    /* 05 */ "dderror 05",
    /* 06 */ "Seek error: The drive does not seek",
    /* 07 */ "Unknown media, CDROM-format does not match CD Redbook specifications",
    /* 08 */ "Sector not found",
    /* 09 */ "dderror 09",
    /* 0a */ "dderror 0a",
    /* 0b */ "dderror 0b",
    /* 0c */ "dderror 0c",
    /* 0d */ "dderror 0d",
    /* 0e */ "dderror 0e",
    /* 0f */ "dderror 0f",
    /* 10 */ "Uncertain media",
    /* 11 */ "dderror 11",
    /* 12 */ "dderror 12",
    /* 13 */ "Unsupported parameter, signature is wrong (internal error)",
    /* 14 */ "Device is already in use",
    /* 15 */ "dderror 15",
    /* 16 */ "dderror 16",
    /* 17 */ "dderror 17",
    /* 18 */ "dderror 18",
    /* 19 */ "dderror 19",
    /* 1a */ "dderror 1a",
    /* 1b */ "dderror 1b",
    /* 1c */ "dderror 1c",
    /* 1d */ "dderror 1d",
    /* 1e */ "dderror 1e",
    /* 1f */ "dderror 1f"};

  ulMsg = rc & 0x00ff;                      /* Treibermeldung filtern */

  if (ulMsg < 0x20)                      /* Innerhalb des Bereiches ? */
    printf ("\nError: %04x - (%s).",
        ulMsg,
        szError[ulMsg]);
}


/***********************************************************************
 * Name      : APIRET OemAdaption
 * Funktion  : Query OS/2 OEM Adaption Information
 * Parameter : HFILE hOemHlp
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 23.10.1995 17.40.32]
 ***********************************************************************/

APIRET OemAdaption (HFILE hOemHlp)
{
  ULONG  ulParmRetLen;                                /* ParameterpaketlÑnge */
  PVOID  pParam;
  ULONG  ulDataRetLen;                                /* ParameterpaketlÑnge */
  APIRET rc;                                                 /* RÅckgabewert */

  struct PDATA
  {
    UCHAR ucOemName[20];
    UCHAR ucRevision[10];
  } Data;

          /* This is the direct way to communicate with the driver via IOCtl */
  ulParmRetLen = 0;
  pParam       = NULL;
  ulDataRetLen = sizeof(Data);

  rc = DosDevIOCtl(hOemHlp,                                  /* Device-Handle */
                   IOCTL_OEMHLP,                                 /* Category */
                   OEMHLP_GETOEMADAPTIONINFO,                    /* Function */
                   NULL,                           /* Parameterblock-Pointer */
                   0,                      /* Max. LÑnge der Parameterblocks */
                   &ulParmRetLen,   /* Pointer auf LÑnge des Parameterblocks */
                   &Data,                                      /* Datenblock */
                   sizeof(Data),           /* Maximale LÑnge des Datenblocks */
                   &ulDataRetLen);      /* Pointer auf LÑnge des Datenblocks */
  if (rc == NO_ERROR)                                                  /* OK */
    printf ("OS/2: %s (Build %s)\n",
            Data.ucOemName,
            Data.ucRevision);

  return (rc);                                       /* RÅckgabewert liefern */
} /* APIRET OemAdaption */


/***********************************************************************
 * Name      : APIRET OemMachineInfo
 * Funktion  : Query machine information
 * Parameter : HFILE hOemHlp
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 23.10.1995 17.40.32]
 ***********************************************************************/

APIRET OemMachineInfo (HFILE hOemHlp)
{
  ULONG  ulParmRetLen;                                /* ParameterpaketlÑnge */
  PVOID  pParam;
  ULONG  ulDataRetLen;                                /* ParameterpaketlÑnge */
  APIRET rc;                                                 /* RÅckgabewert */

  struct PDATA
  {
    UCHAR ucManufacturer[20];
    UCHAR ucModel[10];
    UCHAR ucROM[10];
  } Data;

          /* This is the direct way to communicate with the driver via IOCtl */
  ulParmRetLen = 0;
  pParam       = NULL;
  ulDataRetLen = sizeof(Data);

  rc = DosDevIOCtl(hOemHlp,                                  /* Device-Handle */
                   IOCTL_OEMHLP,                                 /* Category */
                   OEMHLP_GETMACHINEINFO,                        /* Function */
                   NULL,                           /* Parameterblock-Pointer */
                   0,                      /* Max. LÑnge der Parameterblocks */
                   &ulParmRetLen,   /* Pointer auf LÑnge des Parameterblocks */
                   &Data,                                      /* Datenblock */
                   sizeof(Data),           /* Maximale LÑnge des Datenblocks */
                   &ulDataRetLen);      /* Pointer auf LÑnge des Datenblocks */
  if (rc == NO_ERROR)                                                  /* OK */
    printf ("Machine: Manufacturer [%s], Model [%s], ROM-Revision [%s].\n",
            Data.ucManufacturer,
            Data.ucModel,
            Data.ucROM);

  return (rc);                                       /* RÅckgabewert liefern */
} /* APIRET OemMachineInfo */


/***********************************************************************
 * Name      : APIRET OemDisplayCombinationCode
 * Funktion  : Query display combination code
 * Parameter : HFILE hOemHlp
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung : INT 10h (AH=1Ah)
 *
 * Autor     : Patrick Haller [Montag, 23.10.1995 17.40.32]
 ***********************************************************************/

APIRET OemDisplayCombinationCode (HFILE hOemHlp)
{
  ULONG  ulParmRetLen;                                /* ParameterpaketlÑnge */
  PVOID  pParam;
  ULONG  ulDataRetLen;                                /* ParameterpaketlÑnge */
  APIRET rc;                                                 /* RÅckgabewert */

  struct PDATA
  {
    UCHAR ucDisplay;
  } Data;

          /* This is the direct way to communicate with the driver via IOCtl */
  ulParmRetLen = 0;
  pParam       = NULL;
  ulDataRetLen = sizeof(Data);

  rc = DosDevIOCtl(hOemHlp,                                  /* Device-Handle */
                   IOCTL_OEMHLP,                                 /* Category */
                   OEMHLP_GETDISPLAYCOMBCODE,                    /* Function */
                   NULL,                           /* Parameterblock-Pointer */
                   0,                      /* Max. LÑnge der Parameterblocks */
                   &ulParmRetLen,   /* Pointer auf LÑnge des Parameterblocks */
                   &Data,                                      /* Datenblock */
                   sizeof(Data),           /* Maximale LÑnge des Datenblocks */
                   &ulDataRetLen);      /* Pointer auf LÑnge des Datenblocks */
  if (rc == NO_ERROR)                                                  /* OK */
    printf ("Display: %02xh.\n",
            Data.ucDisplay);

  return (rc);                                       /* RÅckgabewert liefern */
} /* APIRET OemDisplayCombinationCode */


/***********************************************************************
 * Name      : APIRET OemROMBIOSInfo
 * Funktion  : Query ROM BIOS Information
 * Parameter : HFILE hOemHlp
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 23.10.1995 17.40.32]
 ***********************************************************************/

APIRET OemROMBIOSInfo (HFILE hOemHlp)
{
  ULONG  ulParmRetLen;                                /* ParameterpaketlÑnge */
  PVOID  pParam;
  ULONG  ulDataRetLen;                                /* ParameterpaketlÑnge */
  APIRET rc;                                                 /* RÅckgabewert */

  struct PDATA
  {
    USHORT usModel;
    USHORT usSubmodel;
    USHORT usBIOSRevision;
    USHORT usBIOSFlags;
  } Data;

          /* This is the direct way to communicate with the driver via IOCtl */
  ulParmRetLen = 0;
  pParam       = NULL;
  ulDataRetLen = sizeof(Data);

  rc = DosDevIOCtl(hOemHlp,                                  /* Device-Handle */
                   IOCTL_OEMHLP,                                 /* Category */
                   OEMHLP_GETROMBIOSINFO,                        /* Function */
                   NULL,                           /* Parameterblock-Pointer */
                   0,                      /* Max. LÑnge der Parameterblocks */
                   &ulParmRetLen,   /* Pointer auf LÑnge des Parameterblocks */
                   &Data,                                      /* Datenblock */
                   sizeof(Data),           /* Maximale LÑnge des Datenblocks */
                   &ulDataRetLen);      /* Pointer auf LÑnge des Datenblocks */
  if (rc == NO_ERROR)                                                  /* OK */
    printf ("BIOS: Model %04xh.%04xh, BIOS Revision %04xh, Flags %04xh %s\n",
            Data.usModel,
            Data.usSubmodel,
            Data.usBIOSRevision,
            Data.usBIOSFlags,
            Data.usBIOSFlags & 0x0001 ? "(ABIOS present)" : "");

  return (rc);                                       /* RÅckgabewert liefern */
} /* APIRET OemROMBIOSInfo */


/***********************************************************************
 * Name      : APIRET OemMiscVideoInfo
 * Funktion  : Query Miscellaneous Video Information
 * Parameter : HFILE hOemHlp
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 23.10.1995 17.40.32]
 ***********************************************************************/

APIRET OemMiscVideoInfo (HFILE hOemHlp)
{
  ULONG  ulParmRetLen;                                /* ParameterpaketlÑnge */
  PVOID  pParam;
  ULONG  ulDataRetLen;                                /* ParameterpaketlÑnge */
  APIRET rc;                                                 /* RÅckgabewert */

  struct PDATA
  {
    UCHAR ucVideo;
  } Data;

          /* This is the direct way to communicate with the driver via IOCtl */
  ulParmRetLen = 0;
  pParam       = NULL;
  ulDataRetLen = sizeof(Data);

  rc = DosDevIOCtl(hOemHlp,                                  /* Device-Handle */
                   IOCTL_OEMHLP,                                 /* Category */
                   OEMHLP_GETMISCVIDEOINFO,                      /* Function */
                   NULL,                           /* Parameterblock-Pointer */
                   0,                      /* Max. LÑnge der Parameterblocks */
                   &ulParmRetLen,   /* Pointer auf LÑnge des Parameterblocks */
                   &Data,                                      /* Datenblock */
                   sizeof(Data),           /* Maximale LÑnge des Datenblocks */
                   &ulDataRetLen);      /* Pointer auf LÑnge des Datenblocks */
  if (rc == NO_ERROR)                                                  /* OK */
    printf ("Video: Reserved bit is               %s\n"
            "       P70 video adapter is          %s\n"
            "       Video attribute is            %s\n"
            "       Cursor emulation is           %s\n"
            "       Palette loading is            %s\n"
            "       Monochrome display is         %s\n"
            "       Summing capability is         %s\n"
            "       All modes on all displays are %s\n",
            Data.ucVideo & 0x80 ? "set"                  : "clear",
            Data.ucVideo & 0x40 ? "active"               : "inactive",
            Data.ucVideo & 0x20 ? "background intensity" : "blinking",
            Data.ucVideo & 0x10 ? "active"               : "inactive",
            Data.ucVideo & 0x08 ? "disabled"             : "enabled",
            Data.ucVideo & 0x04 ? "attached"             : "not attached",
            Data.ucVideo & 0x02 ? "active"               : "inactive",
            Data.ucVideo & 0x01 ? "active"               : "inactive");

  return (rc);                                       /* RÅckgabewert liefern */
} /* APIRET OemMiscVideoInfo */


/***********************************************************************
 * Name      : APIRET OemVideoAdapter
 * Funktion  : Query Video Adapter
 * Parameter : HFILE hOemHlp
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 23.10.1995 17.40.32]
 ***********************************************************************/

APIRET OemVideoAdapter (HFILE hOemHlp)
{
  ULONG  ulParmRetLen;                                /* ParameterpaketlÑnge */
  PVOID  pParam;
  ULONG  ulDataRetLen;                                /* ParameterpaketlÑnge */
  APIRET rc;                                                 /* RÅckgabewert */

  struct PDATA
  {
    UCHAR ucVideoAdapter;
  } Data;

          /* This is the direct way to communicate with the driver via IOCtl */
  ulParmRetLen = 0;
  pParam       = NULL;
  ulDataRetLen = sizeof(Data);

  rc = DosDevIOCtl(hOemHlp,                                  /* Device-Handle */
                   IOCTL_OEMHLP,                                 /* Category */
                   OEMHLP_GETVIDEOADAPTER,                       /* Function */
                   NULL,                           /* Parameterblock-Pointer */
                   0,                      /* Max. LÑnge der Parameterblocks */
                   &ulParmRetLen,   /* Pointer auf LÑnge des Parameterblocks */
                   &Data,                                      /* Datenblock */
                   sizeof(Data),           /* Maximale LÑnge des Datenblocks */
                   &ulDataRetLen);      /* Pointer auf LÑnge des Datenblocks */
  if (rc == NO_ERROR)                                                  /* OK */
    printf ("Video: MPA   is %s\n"
            "       CGA   is %s\n"
            "       EGA   is %s\n"
            "       VGA   is %s\n"
            "       Reserved %02xh\n",
            Data.ucVideoAdapter & 0x01 ? "present" : "not present",
            Data.ucVideoAdapter & 0x02 ? "present" : "not present",
            Data.ucVideoAdapter & 0x04 ? "present" : "not present",
            Data.ucVideoAdapter & 0x08 ? "present" : "not present",
            Data.ucVideoAdapter & 0xf0);

  return (rc);                                       /* RÅckgabewert liefern */
} /* APIRET OemVideoAdapter */


/***********************************************************************
 * Name      : APIRET OemSVGAInfo
 * Funktion  : Query SVGA Information
 * Parameter : HFILE hOemHlp
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung : -> DDK:SVGADEFS.H
 *
 * Autor     : Patrick Haller [Montag, 23.10.1995 17.40.32]
 ***********************************************************************/

typedef struct _CHIPSET
{
  ULONG ulManufacturerID;
  ULONG ulNumberOfChipsets;

  PCHIPNAMES TabChipNames;
} CHIPSET, *PCHIPSET;


CHIPNAMES ppszNullChipNames [MAX_NULL_CHIP] =
{ /* NULL */
  "<unknown chipset>"
};

CHIPSET csUnknown     = {UNKNOWN_ADAPTER,        MAX_NULL_CHIP,           ppszNullChipNames};
CHIPSET csVideo7      = {VIDEO7_ADAPTER,         MAX_VIDEO7_CHIP,         ppszVideo7ChipNames};
CHIPSET csTrident     = {TRIDENT_ADAPTER,        MAX_TRIDENT_CHIP,        ppszTridentChipNames};
CHIPSET csTSENG       = {TSENG_ADAPTER,          MAX_TSENG_CHIP,          ppszTsengChipNames};
CHIPSET csWD          = {WESTERNDIG_ADAPTER,     MAX_WESTERNDIG_CHIP,     ppszWDChipNames};
CHIPSET csATI         = {ATI_ADAPTER,            MAX_ATI_CHIP,            ppszATIChipNames};
CHIPSET csIBM         = {IBM_ADAPTER,            MAX_IBM_CHIP,            ppszIBMChipNames};
CHIPSET csCirrus      = {CIRRUS_ADAPTER,         MAX_CIRRUS_CHIP,         ppszCirrusChipNames};
CHIPSET csS3          = {S3_ADAPTER,             MAX_S3_CHIP,             ppszS3ChipNames};
CHIPSET csChips       = {CHIPS_ADAPTER,          MAX_CHIPS_CHIP,          ppszNullChipNames}; /* !!! */
CHIPSET csWeitek      = {WEITEK_ADAPTER,         MAX_WEITEK_CHIP,         ppszWeitekChipNames};
CHIPSET csNumber9     = {NUMBER9_ADAPTER,        MAX_NULL_CHIP,           ppszNullChipNames};
CHIPSET csGenericPCI  = {GENERIC_PCISVGA_ADAPTER,MAX_NULL_CHIP,           ppszNullChipNames};
CHIPSET csOak         = {OAK_ADAPTER,            MAX_NULL_CHIP,           ppszNullChipNames};
CHIPSET csMatrox      = {MATROX_ADAPTER,         MAX_NULL_CHIP,           ppszNullChipNames};
CHIPSET csBrooktree   = {BROOKTREE_ADAPTER,      MAX_NULL_CHIP,           ppszNullChipNames};
CHIPSET csNVidia      = {NVIDIA_ADAPTER,         MAX_NULL_CHIP,           ppszNullChipNames};
CHIPSET csAlliance    = {ALLIANCE_ADAPTER,       MAX_NULL_CHIP,           ppszNullChipNames};
CHIPSET csAvance      = {AVANCE_ADAPTER,         MAX_NULL_CHIP,           ppszNullChipNames};
CHIPSET csMediaVision = {MEDIAVISION_ADAPTER,    MAX_NULL_CHIP,           ppszNullChipNames};
CHIPSET csArkLogic    = {ARKLOGIC_ADAPTER,       MAX_NULL_CHIP,           ppszNullChipNames};
CHIPSET csRadius      = {RADIUS_ADAPTER,         MAX_NULL_CHIP,           ppszNullChipNames};
CHIPSET csThreeDLabs  = {THREE_D_LABS_ADAPTER,   MAX_NULL_CHIP,           ppszNullChipNames};
CHIPSET csNCR         = {NCR_ADAPTER,            MAX_NULL_CHIP,           ppszNullChipNames};
CHIPSET csIIT         = {IIT_ADAPTER,            MAX_NULL_CHIP,           ppszNullChipNames};
CHIPSET csAppian      = {APPIAN_ADAPTER,         MAX_NULL_CHIP,           ppszNullChipNames};
CHIPSET csSierra      = {SIERRA_ADAPTER,         MAX_NULL_CHIP,           ppszNullChipNames};
CHIPSET csCornerstone = {CORNERSTONE_ADAPTER,    MAX_NULL_CHIP,           ppszNullChipNames};
CHIPSET csDigital     = {DIGITAL_ADAPTER,        MAX_NULL_CHIP,           ppszNullChipNames};
CHIPSET csCompaq      = {COMPAQ_ADAPTER,         MAX_NULL_CHIP,           ppszNullChipNames};
CHIPSET csInfotronic  = {INFOTRONIC_ADAPTER,     MAX_NULL_CHIP,           ppszNullChipNames};
CHIPSET csOpti        = {OPTI_ADAPTER,           MAX_NULL_CHIP,           ppszNullChipNames};
CHIPSET csNULL        = {NULL_ADAPTER,           MAX_NULL_CHIP,           ppszNullChipNames};

PCHIPSET pTabChipSets[] =
{
  &csUnknown,
  &csVideo7,
  &csTrident,
  &csTSENG,
  &csWD,
  &csATI,
  &csIBM,
  &csCirrus,
  &csS3,
  &csChips,
  &csWeitek,
  &csNumber9,
  &csGenericPCI,
  &csOak,
  &csMatrox,
  &csBrooktree,
  &csNVidia,
  &csAlliance,
  &csAvance,
  &csMediaVision,
  &csArkLogic,
  &csRadius,
  &csThreeDLabs,
  &csNCR,
  &csIIT,
  &csAppian,
  &csSierra,
  &csCornerstone,
  &csDigital,
  &csCompaq,
  &csInfotronic,
  &csOpti,
  &csNULL
};


APIRET OemSVGAInfo (HFILE hOemHlp)
{
  ULONG  ulParmRetLen;                                /* ParameterpaketlÑnge */
  PVOID  pParam;
  ULONG  ulDataRetLen;                                /* ParameterpaketlÑnge */
  APIRET rc;                                                 /* RÅckgabewert */

  struct PDATA
  {
    USHORT usAdapterType;
    USHORT usChipType;
    ULONG  ulVideoMemory;
  } Data;

          /* This is the direct way to communicate with the driver via IOCtl */
  ulParmRetLen = 0;
  pParam       = NULL;
  ulDataRetLen = sizeof(Data);

  rc = DosDevIOCtl(hOemHlp,                                  /* Device-Handle */
                   IOCTL_OEMHLP,                                 /* Category */
                   OEMHLP_GETSVGAINFO,                           /* Function */
                   NULL,                           /* Parameterblock-Pointer */
                   0,                      /* Max. LÑnge der Parameterblocks */
                   &ulParmRetLen,   /* Pointer auf LÑnge des Parameterblocks */
                   &Data,                                      /* Datenblock */
                   sizeof(Data),           /* Maximale LÑnge des Datenblocks */
                   &ulDataRetLen);      /* Pointer auf LÑnge des Datenblocks */
  if (rc == NO_ERROR)                                                  /* OK */
  {
    ULONG ulLoop;

    if (Data.usAdapterType > SVGA_LASTADAPTER)    /* limit to array boundary */
      Data.usAdapterType = SVGA_LASTADAPTER;

    /* determine chipset */
    for (ulLoop = 0;
         pTabChipSets[ulLoop]->ulManufacturerID != NULL_ADAPTER;
         ulLoop++)
    {
      if (pTabChipSets[ulLoop]->ulManufacturerID == Data.usAdapterType)
      {
        if (Data.usChipType > pTabChipSets[ulLoop]->ulNumberOfChipsets)
          Data.usChipType = (USHORT)pTabChipSets[ulLoop]->ulNumberOfChipsets;

        break;
      }
    }

    printf ("Video: Memory       %8ub\n"
            "       Adapter      %2u: %s\n"
            "       Manufacturer %2u: %s\n"
            "       Chipset      %2u: %s\n",
            Data.ulVideoMemory,
            Data.usAdapterType,
            Adapters[Data.usAdapterType].Name,
            Data.usAdapterType,
            Adapters[Data.usAdapterType].Manufacturer,
            Data.usChipType,
            pTabChipSets[ulLoop]->TabChipNames[Data.usChipType - 1]);
  }

  return (rc);                                       /* RÅckgabewert liefern */
} /* APIRET OemVideoSVGAInfo */


/***********************************************************************
 * Name      : APIRET OemMemInfo
 * Funktion  : Query Memory Information
 * Parameter : HFILE hOemHlp
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 23.10.1995 17.40.32]
 ***********************************************************************/

APIRET OemMemInfo (HFILE hOemHlp)
{
  ULONG  ulParmRetLen;                                /* ParameterpaketlÑnge */
  PVOID  pParam;
  ULONG  ulDataRetLen;                                /* ParameterpaketlÑnge */
  APIRET rc;                                                 /* RÅckgabewert */

  struct PDATA
  {
    USHORT usLowMem;
    ULONG  ulHighMem;
  } Data;

          /* This is the direct way to communicate with the driver via IOCtl */
  ulParmRetLen = 0;
  pParam       = NULL;
  ulDataRetLen = sizeof(Data);

  rc = DosDevIOCtl(hOemHlp,                                  /* Device-Handle */
                   IOCTL_OEMHLP,                                 /* Category */
                   OEMHLP_GETMEMINFO,                            /* Function */
                   NULL,                           /* Parameterblock-Pointer */
                   0,                      /* Max. LÑnge der Parameterblocks */
                   &ulParmRetLen,   /* Pointer auf LÑnge des Parameterblocks */
                   &Data,                                      /* Datenblock */
                   sizeof(Data),           /* Maximale LÑnge des Datenblocks */
                   &ulDataRetLen);      /* Pointer auf LÑnge des Datenblocks */
  if (rc == NO_ERROR)                                                  /* OK */
    printf ("Memory: low  memory %10uk\n"
            "        high memory %10uk %5uM\n",
            Data.usLowMem,
            Data.ulHighMem,
            (Data.ulHighMem >> 10) );

  return (rc);                                       /* RÅckgabewert liefern */
} /* APIRET OemMemInfo */


/***********************************************************************
 * Name      : APIRET OemHlpInfo
 * Funktion  : Wrapper for all information subfunctions
 * Parameter : ULONG ulCommand
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 25.09.1995 01.31.18]
 ***********************************************************************/

APIRET OemHlpInfo (void)
{
  APIRET rc;                                    /* RÅckgabewert - Fehlercode */
  HFILE  hOemHlp;                                     /* Handle fÅr das CDROM */
  ULONG  ulAction;                               /* Dummy fÅr DosOpen-Aktion */
  ULONG  ulParmLen;                                   /* ParameterpaketlÑnge */
  ULONG  ulDataLen;                                       /* DatenblocklÑnge */


  rc = DosOpen("\\DEV\\OEMHLP$",                              /* open driver */
               &hOemHlp,
               &ulAction,
               0,
               FILE_NORMAL,
               FILE_OPEN,
               OPEN_ACCESS_WRITEONLY    |
               OPEN_FLAGS_FAIL_ON_ERROR |
               OPEN_SHARE_DENYNONE,
               NULL);
  if (rc == NO_ERROR)
  {
    rc = OemAdaption(hOemHlp);
    rc = OemMachineInfo(hOemHlp);
    rc = OemDisplayCombinationCode(hOemHlp);
    rc = OemROMBIOSInfo(hOemHlp);
    rc = OemMiscVideoInfo(hOemHlp);
    rc = OemVideoAdapter(hOemHlp);
    rc = OemSVGAInfo(hOemHlp);
    rc = OemMemInfo(hOemHlp);

    DosClose (hOemHlp);
  }

  return (rc);                                 /* RÅckgabewert liefern */
} /* APIRET OemHlpInfo */


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

  if ( Options.fsHelp )                                    /* display help ? */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  rc = OemHlpInfo();                                           /* Los geht's */
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  return (rc);
}
