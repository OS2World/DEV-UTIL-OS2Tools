/***********************************************************************
 * Name      : Module MACInfo
 * Funktion  : Command-line frontend for the protocol manager
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 02.21.27]
 ***********************************************************************/

/* #define DEBUG */

/*
 */


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

/*===Strukturen==============================================================*/
typedef struct
{
  ARGFLAG fsHelp;                 /* Muss Hilfestellung dargestellt werden ? */
  ARGFLAG fsVerbose;                                  /* produce more output */
} OPTIONS, *POPTIONS;

typedef void* LPFUN;
typedef char* LPBUF;
typedef PSZ LPSZ;

#pragma pack(1)
// Common Characteristics
typedef struct _CommonCharacteristics
{
  // Size of common characteristics table (bytes)
  WORD      wLength;             
  
  // Major NDIS Version (2 BCD digits - 02 for this version)
  BYTE      bVersionNDISMajor;   
  
  // Minor NDIS Version (2 BCD digits - 00 for this version)
  BYTE      bVersionNDISMinor;   
  
  WORD      Reserved;
    
  // Major Module Version (2 BCD digits)
  BYTE      bVersionModuleMajor;
  
  // Minor Module Version (2 BCD digits)
  BYTE      bVersionModuleMinor; 
  
  // Module function flags, a bit mask :
  //  0 - Binding at upper boundary supported
  //  1 - Binding at lower boundary supported
  //  2 - Dynamically bound (i.e., this module can be swapped out)
  //  3-31 - Reserved, must be zero
  DWORD     dwFunctions;
  
  // Module name - ASCIIZ format
  BYTE      szModuleName[16];
  
  // Protocol level at upper boundary of module:
  //  1 - MAC
  //  2 - Data link
  //  3 - Network
  //  4 - Transport
  //  5 - Session
  // -1 - Not specified
  BYTE      bUpperLevel;
  
  // Type of interface at upper boundary of module:
  //   For MAC's:       1 => MAC
  //   For Data Links:  To be defined
  //   For Transports:  To be defined
  //   For Session:     1 => NCB
  //   For any level:   0 => private (ISV defined)
  BYTE      bUpperInterface;
  
  // Protocol level at lower boundary of module
  //  0 - Physical
  //  1 - MAC
  //  2 - Data link
  //  3 - Network
  //  4 - Transport
  //  5 - Session
  //  -1 - Not specified
  BYTE      bLowerLevel;
  
  // Type of interface at lower boundary of module:
  //   For MAC:          1 => MAC
  //   For Data Link:    To be defined
  //   For Transport:    To be defined
  //   For Session:      1 => NCB
  //   For any level:    0 => private (ISV defined)
  BYTE      bLowerInterface;
  
  // Module ID filled in by Protocol Manager on return from RegisterModule
  WORD      hPMHandle;
  
  // Module data segment selector
  WORD      segModuleDS;
  
  // System request dispatch entry point
  LPFUN     pfDispatch;
  
  
  // Pointer to service-specific characteristics (NULL if none)
  LPBUF     pServiceCharacteristics;
  
  // Pointer to service-specific status (NULL if none)
  LPBUF     pServiceStatus;
  
  // Pointer to upper dispatch table (see below; NULL if none)
  LPBUF     pUpperDispatchTable;

  // Pointer to lower dispatch table (see below; NULL if none)
  LPBUF     pLowerDispatchTable;
  
  // Reserved for future expansion, must be NULL
  LPBUF     pReserved1;
  
  // Reserved for future expansion, must be NULL
  LPBUF     pReserved2;

  // NOTE:    LPSZ   Long pointer to an ASCIIZ string
  //          LPBUF  Long pointer to a data buffer
  //          LPFUN  Long pointer to a function
} COMMONCHARACTERISTICS, *PCOMMONCHARACTERISTICS;



// MAC Service-Specific Characteristics
typedef struct _ServiceCharacteristics
{
  // All MAC's use the following format for this table.  This table
  // contains volatile information (like the current station address)
  // which may be updated by the MAC during the course of operation.
  // Other modules may read this table directly during execution.
  
  // Length of MAC service-specific characteristics table
  WORD        wLength;
  
  // Type name of MAC, ASCIIZ format:
  // 802.3, 802.4, 802.5, 802.6, DIX, DIX+802.3, APPLETALK,
  // ARCNET, FDDI, SDLC, BSC, HDLC, ISDN
  BYTE        szMACType[16];
  
  // Length of station addresses in bytes
  WORD        wAddressLength;
  
  // Permanent station address
  BYTE        addrPermanent[16];
  
  // Current station address
  BYTE        addrCurrent[16];
  
  // Current functional address of adapter (0 if none)
  DWORD       dwFunctionalAddress;

  // Multicast Address List (structure defined below)
  LPBUF       pMulticastAddressList;

  // Link speed (bits/sec)
  DWORD       dwLinkSpeed;
  
  // Service flags, a bit mask:
  //   0 - broadcast supported
  //   1 - multicast supported
  //   2 - functional/group addressing supported
  //   3 - promiscuous mode supported
  //   4 - software settable station address
  //   5 - statistics are always current in service-specific status table
  //   6 - InitiateDiagnostics supported
  //   7 - Loopback supported
  //   8 - Type of receives
  //   0 - MAC does primarily ReceiveLookahead indications
  //   1 - MAC does primarily ReceiveChain indications
  //   9 - IBM Source routing supported
  //   10 - Reset MAC supported
  //   11 - Open / Close Adapter supported
  //   12 - Interrupt Request supported
  //   13 - Source Routing Bridge supported
  //   14 - GDT virtual addresses supported
  //   15 - Multiple TransferDatas permitted during a single indication (V2.0.1 and later)
  //   16 - Mac normally sets FrameSize = 0 in ReceiveLookahead (V2.0.1 and later)
  //   17-31 - Reserved, must be 0  
  DWORD       dwServiceFlags;
  
  // Maximum frame size which may be both sent and received
  WORD        wMaximumFrameSize;
  
  // Total transmission buffer capacity in the driver (bytes)
  DWORD       dwTransmissionBuffer;
  
  // Transmission buffer allocation block size (bytes)
  WORD        wTransmissionBufferBlockSize;
  
  // Total reception buffer capacity in the driver (bytes)
  DWORD       dwReceptionBuffer;
  
  // Reception buffer allocation block size (bytes)
  WORD        wReceptionBufferBlockSize;
  
  // IEEE Vendor code
  CHAR        arrIEEEVendorCode[3];
  
  // Vendor Adapter code
  CHAR        cVendorAdapterCode;
  
  // Vendor Adapter description
  LPSZ        pszVendorAdapterDescription;
  
  // IRQ Interrupt level used by adapter (V2.0.1 and later)
  WORD        wIRQ;
  
  // Transmit Queue Depth (V2.0.1 and later)
  WORD        wTransmissionQueueDepth;
  
  // Maximum number of data blocks in buffer descriptors supported (V2.0.1 and later)
  WORD        wMaximumDataBlocks;
} SERVICECHARACTERISTICS, *PSERVICECHARACTERISTICS;


// Multicast Address List is a buffer formatted as follows:
typedef struct _MulticastAddressList
{
  // Maximum number of multicast addresses
  WORD      wMaximumMulticastAddresses;

  // Current number of multicast addresses
  WORD      wCurrentMulticastAddresses;
  
  // Multicast address 1
  BYTE      arrMulticastAddresses[1][16];
  // BYTE[16]  Multicast address 2
  // BYTE[16]  Multicast Address N
} MULTICASTADDRESSLIST, *PMULTICASTADDRESSLIST;


typedef struct _Globals
{
  // Protocol Manager handle
  HFILE hProtMan;
  
} GLOBALS, *PGLOBALS;

/*
Protocol Manager Request Block
*/
typedef struct _PMReqBlock
{
  USHORT   opcode;
  USHORT   status;
  PCHAR16  pointer1;
  PCHAR16  pointer2;
  USHORT   word1;
} PMREQBLOCK, *PPMREQBLOCK;


/* PM IOCTL request packet parameters
 */
#define  IOCTL_CAT_LAN_MANAGER   0x81  // LAN Manager Category code
#define  IOCTL_FUN_PM            0x58  // Protocol Manager Function code

/* Protocol Manager request Opcodes
 */
#define  PM_OP_GET_PM_INFO            1
#define  PM_OP_REGISTER_MODULE        2
#define  PM_OP_GET_PROTOCOL_INI_PATH  5

// Protocol Manager status codes
#define STATUS_SUCCESS                       0x0000
#define STATUS_INVALID_FUNCTION              0x0008
#define STATUS_DRIVER_NOT_INITIALIZED        0x0022
#define STATUS_HARDWARE_NOT_FOUND            0x0023
#define STATUS_HARDWARE_FAILURE              0x0024
#define STATUS_CONFIGURATION_FAILURE         0x0025
#define STATUS_INTERRUPT_CONFLICT            0x0026
#define STATUS_INCOMPATIBLE_MAC              0x0027
#define STATUS_INITIALIZATION_FAILED         0x0028
#define STATUS_NETWORK_MAY_NOT_BE_CONNECTED  0x002A
#define STATUS_INCOMPATIBLE_OS_VERSION       0x002B
#define STATUS_INFO_NOT_FOUND                0x002f
#define STATUS_GENERAL_FAILURE               0x00ff


#pragma pack()

/*===End Strukturen==========================================================*/

#define PROTMAN "\\DEV\\PROTMAN$"


/*===Prototypen==============================================================*/


/*===Globale Strukturen======================================================*/

OPTIONS Options;
GLOBALS Globals;

ARGUMENT TabArguments[] =
{ /*Token--Beschreibung--pTarget--ucTargetFormat--pTargetSpecified--*/
  {"/V",          "Verbose mode, much output.",            NULL,                      ARG_NULL, &Options.fsVerbose},
  {"/H",          "Get help screen.",                      NULL,                      ARG_NULL, &Options.fsHelp},
  {"/?",          "Get help screen.",                      NULL,                      ARG_NULL | ARG_HIDDEN, &Options.fsHelp},
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
  TOOLVERSION("MACInfo",                                 /* application name */
              0x00010000,                            /* application revision */
              0x00010900,           /* minimum required revision of PHSTOOLS */
              0x0001FFFF,        /* maximum recommended revision of PHSTOOLS */
              NULL,                                                /* Remark */
              NULL);                                 /* additional copyright */
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
  
  memset(&Globals,
         0L,
         sizeof(Globals));

  setvbuf(stdout,                               /* disable stream buffering */
          NULL,
          _IONBF,
          0);
}


/***********************************************************************
 * Name      : 
 * Funktion  : 
 * Parameter : 
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-03-04]
 ***********************************************************************/

APIRET ProtManOpen(void)
{
  APIRET rc;
  ULONG  ulAction;
  
  rc = DosOpen(PROTMAN,
               &Globals.hProtMan, 
               &ulAction, 
               0, 
               FILE_NORMAL, 
               FILE_OPEN,
               OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE, 
               0L);
  return rc;
}


APIRET ProtManGetProtocolManagerInfo()
{
  APIRET rc;
  PMREQBLOCK rb = {0};
  ULONG  ulParamsLength = sizeof( rb );
  PVOID  pParams = &rb;
  ULONG  ulDataLength = 0;
  PVOID  pData = NULL;
  
  rb.opcode = PM_OP_GET_PM_INFO;
  
  rc = DosDevIOCtl(Globals.hProtMan,
                   IOCTL_CAT_LAN_MANAGER,
                   IOCTL_FUN_PM,
                   pParams,
                   ulParamsLength,
                   &ulParamsLength,
                   pData,
                   ulDataLength,
                   &ulDataLength);
  if (NO_ERROR != rc)
    return rc;
  
  // check status field in RequestBlock
  if (0 != rb.status)
    return rb.status;
  
  return rc;
}


APIRET ProtManGetProtocolIniPath()
{
  APIRET rc;
  PMREQBLOCK rb = {0};
  ULONG  ulParamsLength = sizeof( rb );
  PVOID  pParams = &rb;
  ULONG  ulDataLength = 0;
  PVOID  pData = NULL;
  CHAR   szProtocolIni[ 260 ];
  
  rb.opcode   = PM_OP_GET_PROTOCOL_INI_PATH;
  rb.pointer1 = szProtocolIni;
  rb.word1    = sizeof( szProtocolIni );
  
  rc = DosDevIOCtl(Globals.hProtMan,
                   IOCTL_CAT_LAN_MANAGER,
                   IOCTL_FUN_PM,
                   pParams,
                   ulParamsLength,
                   &ulParamsLength,
                   pData,
                   ulDataLength,
                   &ulDataLength);
  if (NO_ERROR != rc)
    return rc;
  
  // check status field in RequestBlock
  if (0 != rb.status)
    return rb.status;
  
  return rc;
}




APIRET ProtManClose(void)
{
  return DosClose( Globals.hProtMan );
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
  APIRET rc;                                                    /* RÅckgabewert */
  APIRET rc2;
  ULONG ulCmd;

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
  
  
  // Open the protocol manager
  rc = ProtManOpen();
  if (NO_ERROR != rc)
    ToolsErrorDosEx(rc,
                    "Opening protocol manager driver");
  else
  {
    // @@@PH
    rc = ProtManGetProtocolIniPath();
    rc = ProtManGetProtocolManagerInfo();
    
    // close the protocol manager
    rc2 = ProtManClose();
    if (NO_ERROR != rc2)
      ToolsErrorDosEx(rc2,
                      "Closing protocol manager driver");
  }
  
  return rc;
}
