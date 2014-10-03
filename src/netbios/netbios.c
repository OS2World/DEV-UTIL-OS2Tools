/***********************************************************************
 * Name      : Module NetBios
 * Funktion  : Command-line frontend for the NetBios protocol
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 02.21.27]
 ***********************************************************************/

/* #define DEBUG */

/*
 Notes:
 
 - NetBios32Enum on a REMOTE server causes XCPT_BAD_STACK inside netbios lib
 - same for NetBios32GetInfo
 - a netbios name comprised of spaces only will hang the netbios driver
 
 
 To implement:
 - NCBCANCEL to unhang stuck commands?
 - NCBHANGUP to close a (stuck) session
 - NCBRESET
 - NCBSSTAT
 - NCBTRACE
 
 - eventually a netbios send / receive utility
 
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

#define E32TO16
#define PURE_32

#pragma pack(1)
#include <netcons.h>
#include <lan_7_c.h>
#include <ncb.h>
#include <netbios.h>
#include <netb_1_c.h>
#include <netb_2_c.h>
#include <netb_4_c.h>
#pragma pack()

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"
/*===End Includes============================================================*/


// _CU_ means unique computer name
// _DU_ means unique domain name
// _UU_ means unique user name

// \\computer_name[00h]
//   Registered by the Workstation Service on the WINS Client
#define NBNAME_CU_WORKSTATION 0x00

// \\computer_name[03h]
//  Registered by the Messenger Service on the WINS Client
#define NBNAME_CU_MESSENGER   0x03

// \\computer_name[06h]
// Registered by the Remote Access Service (RAS), when started on a RAS Server.
#define NBNAME_CU_RASSERVER   0x06

// \\computer_name[1Fh]
// Registered by the Network Dynamic Data Exchange (NetDDE) services, will 
// only appear if the NetDDE services are started on the computer. By 
// default under Windows NT 3.51, the NetDDE services are
// not automatically started.
#define NBNAME_CU_NETDDE      0x1f

// \\computer_name[20h]
// Registered by the Server Service on the WINS Client.
#define NBNAME_CU_SERVER      0x20

// \\computer_name[21h]
// Registered by the RAS Client Service, when started on a RAS Client.
#define NBNAME_CU_RASCLIENT   0x21

// \\computer_name[BEh]
// Registered by the Network Monitoring Agent Serviceˇwill only appear if 
// the service is started on the computer. If the computer name is not a 
// full 15 characters, the name will be padded with plus (+) symbols.
#define NBNAME_CU_NETMONAGENT 0xBE

// \\computer_name[BFh]
// Registered by the Network Monitoring Utility (included with Microsoft 
// Systems Management Server). If the computer name is not a full 15 
// characters, the name will be padded with plus (+) symbols.
#define NBNAME_CU_NETMONUTIL  0xBF

// \\username[03h]
// User names for the currently logged on users are registered in the WINS 
// database. The user name is registered by the Server component so that 
// the user can receive any "net send" commands sent to their
// user name. If more than one user is logged on with the same user name, 
// only the first computer at which a user logged on with the user name 
// will register the name.
#define NBNAME_UU_USER        0x03

// \\domain_name[1Bh]
// Registered by the Windows NT Server primary domain controller (PDC) that 
// is running as the Domain Master Browser and is used to allow remote 
// browsing of domains. When a WINS server is queried for
// this name, a WINS server returns the IP address of the computer that 
// registered this name.
#define NBNAME_DU_PDC         0x1b // domain at PDC

// \\domain_name[1Dh]
// Registered only by the Master Browser, of which there can be only one for 
// each subnet. This name is used by the Backup Browsers to communicate with 
// the Master Browser to retrieve the list of available
// servers from the Master Browser.
// WINS servers always return a positive registration response for 
// domain_name[1D], even though the WINS server does not "register" this 
// name in its database. Therefore, when a WINS server is queried
// for the domain_name[1D], the WINS server returns a negative response, 
// which will cause the client to broadcast to resolve the name.
#define NBNAME_DU_BROWSER     0x1d // domain at browser


// Group Names

// \\domain_name[00h]
// Registered by the Workstation Service so that it can receive browser 
// broadcasts from LAN Manager-based computers.
#define NBNAME_DG_WORKSTATION 0x00

// \\domain_name[1Ch]
// Registered for use by the domain controllers within the domain and can 
// contain up to 25 IP addresses. One IP address will be that of the primary 
// domain controller (PDC) and the other 24 will be the IP
// addresses of backup domain controllers (BDCs).
#define NBNAME_DG_PDC         0x1c

// \\domain_name[1Eh]
// Registered for browsing purposes and used by the browsers to elect a 
// Master Browser (this is how a statically mapped group name registers 
// itself). When a WINS server receives a name query for a name
// ending with [1E], the WINS server will always return the network 
// broadcast address for the requesting client's local network.
#define NBNAME_DG_BROWSER     0x1e

// \\--__MSBROWSE__[01h]
// Registered by the Master Browser for each subnet. When a WINS server 
// receives a name query for this name, the WINS server will always return 
// the network broadcast address for the requesting client's local network.
#define NBNAME_MSBROWSE       0x01



/*===Strukturen==============================================================*/
typedef NCB *PNCB;

typedef struct
{
  ARGFLAG fsHelp;                 /* Muss Hilfestellung dargestellt werden ? */
  ARGFLAG fsVerbose;                                  /* produce more output */
  
  ARGFLAG fsServer;                           /* send request to this server */
  PSZ     pszServer;                          /* send request to this server */
  ARGFLAG fsAdapter;                                     /* adapter argument */
  PSZ     pszAdapter;                                    /* adapter argument */
  ARGFLAG fsName;                                           /* name argument */
  PSZ     pszName;                                          /* name argument */
  ARGFLAG fsNameType;                                  /* name type argument */
  UCHAR   ucNameType;                                  /* name type argument */
  
  ARGFLAG fsCommand;                                    /* the command token */
  PSZ     pszCommand;                                   /* the command token */
  
} OPTIONS, *POPTIONS;


typedef struct _NBCOMMAND
{
  ULONG ulCommand;
  PSZ   pszCommand;
} NBCOMMAND, *PNBCOMMAND;


enum
{
  CMD_GETINFO,
  CMD_LISTADAPTERS,
  CMD_FINDNAME,
  CMD_LISTNAMES,
  CMD_DELNAME,
  CMD_ADDNAME,
  CMD_ADDGRNAME
};


static NBCOMMAND arrCommands[] =
{
  {CMD_GETINFO,      "GETINFO"},
  {CMD_LISTADAPTERS, "LISTADAPTERS"},
  {CMD_FINDNAME,     "FINDNAME"},
  {CMD_LISTNAMES,    "LISTNAMES"},
  {CMD_DELNAME,      "DELNAME"},
  {CMD_ADDNAME,      "ADDNAME"},
  {CMD_ADDGRNAME,    "ADDGRNAME"},
  0
};

/*===End Strukturen==========================================================*/


/*===Prototypen==============================================================*/

void listCommands(void);


/*===Globale Strukturen======================================================*/

OPTIONS Options;

ARGUMENT TabArguments[] =
{ /*Token--Beschreibung--pTarget--ucTargetFormat--pTargetSpecified--*/
  {"/V",          "Verbose mode, much output.",            NULL,                      ARG_NULL, &Options.fsVerbose},
  {"/SERVER=",    "Send requests to this server machine.", &Options.pszServer,        ARG_PSZ,  &Options.fsServer},
  {"/ADAPTER=",   "Adapter for some operations (net1).",   &Options.pszAdapter,       ARG_PSZ,  &Options.fsAdapter},
  {"/NAME=",      "Name for some operations.",             &Options.pszName,          ARG_PSZ,  &Options.fsName},
  {"/TYPE=",      "Name type for some operations.",        &Options.ucNameType,       ARG_UCHAR,&Options.fsNameType},
  {"/?",          "Get help screen.",                      NULL,                      ARG_NULL, &Options.fsHelp},
  {"/H",          "Get help screen.",                      NULL,                      ARG_NULL, &Options.fsHelp},
  {"1",           "Command to perform.",                   &Options.pszCommand,       ARG_PSZ |
                                                                                      ARG_MUST|
                                                                                      ARG_DEFAULT, &Options.fsCommand},
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
  TOOLVERSION("NetBios",                                 /* application name */
              0x00010002,                            /* application revision */
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

  setvbuf(stdout,                                /* disable stream buffering */
          NULL,
          _IONBF,
          0);
}


void printNBI(PSZ pszObject,
              struct netbios_info_1* pNBI)
{
  PSZ pszDriverType;
  
  // determine driver type
  switch (pNBI->nb1_driver_type)
  {
    case 1:
      pszDriverType = "network control block";
      break;
    
    case 2:
      pszDriverType = "message control block";
      break;
    
    default:
      pszDriverType = "unknown";
      break;
  }
    
  printf("Information on %s:\n"
         "  net name           = [%s]\n"
         "  driver name        = [%s]\n"
         "  lan adapter number = %d\n"
         "  (pad 1)            = %02xh\n"
         "  driver type        = %04xh (%s)\n"
         "  net status         = %04xh\n"
         "  net bandwidth      = %08xh (bit/sec?)\n"
         "  maximum sessions   = %d\n"
         "  maximum NCBs       = %d\n"
         "  maximum names      = %d\n",
         pszObject,
         pNBI->nb1_net_name,
         pNBI->nb1_driver_name,
         pNBI->nb1_lana_num,
         pNBI->nb1_pad_1,
         pNBI->nb1_driver_type, pszDriverType,
         pNBI->nb1_net_status,
         pNBI->nb1_net_bandwidth,
         pNBI->nb1_max_sess,
         pNBI->nb1_max_ncbs,
         pNBI->nb1_max_names);
  
  printf("  Status is: \n");
  
#define STATUS(a,b)                                    \
  printf("    %-50s: %s\n",                            \
         a,                                            \
         ((pNBI->nb1_net_status & b) == b) ? "yes" : "no");

#define STATUS2(a,b)                                   \
  if ( ((pNBI->nb1_net_status & b) == b) )             \
    printf("    %-50s: is set\n",                       \
         a);
  
  STATUS("LAN is managed by redirector",                NB_LAN_MANAGED)
  STATUS("LAN is a loopback driver",                    NB_LAN_LOOPBACK)
  STATUS("LAN allows SendNoAck NCBs",                   NB_LAN_SENDNOACK)
  STATUS("LAN supports LAN Manager extended NCBs",      NB_LAN_LMEXT)
  STATUS("LAN allows NCB submission at interrupt time", NB_LAN_INTNCB)
  STATUS2("undefined bit 0x0020",                        0x20)
  STATUS("LAN does not allow reset NCBs",               NB_LAN_NORESET)
  STATUS2("undefined bit 0x0080",                        0x80)
  STATUS2("undefined bit 0x0100",                        0x100)
  STATUS2("undefined bit 0x0200",                        0x200)
  STATUS2("undefined bit 0x0400",                        0x400)
  STATUS2("undefined bit 0x0800",                        0x800)
  STATUS2("undefined bit 0x1000",                        0x1000)
  STATUS2("undefined bit 0x2000",                        0x2000)
    
  switch ( (pNBI->nb1_net_status & NB_OPEN_MODE_MASK) )
  {
    case 0:
      printf("    The network software is not started.\n");
      break;
    
    case NB_OPEN_REGULAR:
      printf("    The software is operating in regular mode.\n");
      break;
    
    case NB_OPEN_PRIVILEGED:
      printf("    The software is operating in privileged mode.\n");
      break;
    
    case NB_OPEN_EXCLUSIVE:
      printf("    The software is operating in exclusive mode.\n");
      break;
  }

#undef STATUS
#undef STATUS2
}


void init_NCB(PNCB  pncb,
              UCHAR ncb_command,
              ULONG ulLanAdapterNumber)
{
  memset(pncb, 0, sizeof(NCB) );
  
  pncb->ncb_command  = ncb_command;
  pncb->ncb_retcode  = NRC_GOODRET;
  pncb->ncb_lana_num = ulLanAdapterNumber;
  pncb->ncb_rto      = 120 * 2; // 500msec units
  pncb->ncb_sto      = 120 * 2; // 500msec units
}


void buildNetbiosName(PCHAR pNameTarget,
                      PSZ   pszName,
                      UCHAR ucNameType)
{
  int iLen;
  
  if (NULL != pszName)
  {
    strncpy(pNameTarget,
            pszName,
            NCBNAMSZ);
    
    strupr(pNameTarget);
    iLen = strlen(pszName);
  }
  else
  {
    // Note:
    // a netbios name filled with spaces only will send the software
    // into an infinite, uninterruptable state, NETBIOS.EXE will be stuck.
    pNameTarget[0] = '?';
    pNameTarget[1] = 0;
    iLen = 1;
  }
  
  
  while (iLen < NCBNAMSZ - 1)
  {
    pNameTarget[ iLen ] = ' ';
    iLen++;
  }
  
  pNameTarget[ NCBNAMSZ-1 ] = ucNameType;
}

void HexString(PSZ pszBuffer,
               PUCHAR pucSource,
               ULONG ulSourceLength)
{
  int i = 0;
  char szBuffer[8];
  
  pszBuffer[0] = 0;
  
  for (i = 0;
       i < ulSourceLength;
       i++)
  {
    if (i != ulSourceLength)
      sprintf(szBuffer, "%02x ", *pucSource);
    else
      sprintf(szBuffer, "%02x", *pucSource);
    
    strcat(pszBuffer, szBuffer);
  }
}


APIRET i_NetBios32Submit(PNCB pncb)
{
  UCHAR rc;
  ULONG hNetbios = 0;
  
  if (NULL != Options.pszAdapter)
  {
    rc = NetBios32Open(Options.pszAdapter,
                       NULL,
                       NB_REGULAR,
                       &hNetbios);
    if (rc != NO_ERROR)
      return rc;
  }
  
  // if no adapter is selected, use the first one
  rc = NetBios32Submit(hNetbios,
                       0, // single NCB
                       pncb);
  
  if (NULL != Options.pszAdapter)
    NetBios32Close(hNetbios,
                   0);
  
  return rc;
}

/***********************************************************************
 * Name      : 
 * Funktion  : 
 * Parameter : 
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-02-28]
 ***********************************************************************/

APIRET netbios_EnumAdapters(void)
{
  ULONG ulEntriesReturned  = 0;
  ULONG ulEntriesAvailable = 0;
  ULONG rc;
  PVOID pBuffer;
  ULONG ulBufferLength;
  struct netbios_info_1* pNBI1;
  ULONG ulCount;
  char szBuf[64];
    
  // determine number of available entries
  rc = NetBios32Enum(Options.pszServer,
                     0,
                     NULL,
                     0,
                     &ulEntriesReturned,
                     &ulEntriesAvailable);
    
  // if network adapters are available ...
  if (0 == ulEntriesAvailable)
  {
    printf("No NetBIOS adapters available\n");
    return NO_ERROR;
  }
  
  printf("List of available NetBIOS adapters:\n");
  
  // allocate buffer of sufficient size
  ulBufferLength = ulEntriesAvailable * sizeof( struct netbios_info_1 );
  pBuffer = malloc( ulBufferLength );
  if (NULL == pBuffer)
    return ERROR_NOT_ENOUGH_MEMORY;

  // enumerate all network drivers (net1, net2, ...)
  // and build the array of LANA numbers
  rc = NetBios32Enum(Options.pszServer,
                     1,
                     (PUCHAR)pBuffer,
                     ulBufferLength,
                     &ulEntriesReturned,
                     &ulEntriesAvailable);
  
  pNBI1 = (struct netbios_info_1 *)pBuffer;
  
  for (ulCount = 0;
       ulCount < ulEntriesReturned;
       ulCount++)
  {
    sprintf(szBuf,
            "Adapter %d",
            ulCount);
    
    printNBI(szBuf,
             pNBI1);
    
    pNBI1++;
  }

  free( pBuffer );
  
  return NO_ERROR;
}


/***********************************************************************
 * Name      : 
 * Funktion  : 
 * Parameter : 
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-02-28]
 ***********************************************************************/

APIRET netbios_GetInfo(void)
{
  ULONG ulBytesRequired = 0;
  ULONG rc;
  PVOID pBuffer;
  ULONG ulCount;
  struct netbios_info_1* pNBI1;
  
  if (NULL == Options.pszAdapter)
  {
    printf("Please specify NetBIOS adapter name, such as 'net1'\n");
    return ERROR_INVALID_PARAMETER;
  }
  
  // determine number of available entries
  rc = NetBios32GetInfo(Options.pszServer,
                        Options.pszAdapter,
                        1,
                        NULL,
                        0,
                        &ulBytesRequired);
  /* @@@PH error handling */
    
  // if no information is available ...
  if (0 == ulBytesRequired)
  {
    printf("No information available for %s\n",
           Options.pszAdapter);
    return NO_ERROR;
  }
  
  printf("NetBIOS information for %s on %s:\n",
         (Options.pszServer == NULL) ? "(local)" : Options.pszServer,
         Options.pszAdapter);
  
  // allocate buffer of sufficient size
  pBuffer = malloc( ulBytesRequired );
  if (NULL == pBuffer)
    return ERROR_NOT_ENOUGH_MEMORY;

  // enumerate all network drivers (net1, net2, ...)
  // and build the array of LANA numbers
  rc = NetBios32GetInfo(Options.pszServer,
                        Options.pszAdapter,
                        1,
                        (PUCHAR)pBuffer,
                        ulBytesRequired,
                        &ulBytesRequired);
                        
  /* @@@PH error handling */
  
  pNBI1 = (struct netbios_info_1 *)pBuffer;
  
  printNBI(Options.pszAdapter,
           pNBI1 );
  
  free( pBuffer );
  
  return NO_ERROR;
}


/***********************************************************************
 * Name      : 
 * Funktion  : 
 * Parameter : 
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-02-28]
 ***********************************************************************/

APIRET netbios_FindName(void)
{
  NCB   ncb;
  UCHAR rc;
  char  szBuffer[ 512 ]; // NETBIOS crashes if that's too large
  PSZ   pszNameStatus;
  struct ncb_find_name_info*   pFindHeader;
  struct ncb_lan_header_entry* pFindBuffer;
  char  szMACDestination[64];
  char  szMACSource[64];
  char  szRouteInfo[64];
  int   i;
  
  
  // setup the network control block
  // @@@PH LANA number
  init_NCB( &ncb, NB_FIND_NAMEWAIT, 0);
  ncb.ncb_buffer  = szBuffer;
  ncb.ncb_length  = sizeof( szBuffer );
  
  // build netbios name
  buildNetbiosName(ncb.ncb_callname, Options.pszName, Options.ucNameType);
  
  // send NCB
  rc = i_NetBios32Submit( &ncb );
  
  printf("netbios returned %d\n",
         rc);
  
  // display found information
  pFindHeader = (struct ncb_find_name_info *)ncb.ncb_buffer;
  switch (pFindHeader->name_status)
  {
    case 0: pszNameStatus = "unique"; break;
    case 1: pszNameStatus = "group"; break;
    default: pszNameStatus = "unknown"; break;
  }
  
  printf("Find Name:\n"
         "  nodes responding        = %d\n"
         "  reserved                = %d\n"
         "  name status             = %02xh (%s)\n",
         pFindHeader->nodes_responding,
         pFindHeader->reserved,
         pFindHeader->name_status,
         pszNameStatus);
  
  pFindBuffer = (struct ncb_lan_header_entry*) 
                  (ncb.ncb_buffer + sizeof(struct ncb_find_name_info) );
  
  for (i = 0;
       i < pFindHeader->nodes_responding;
       i++)
  {
    HexString(szMACDestination, pFindBuffer->lan_destination_addr, 6);
    HexString(szMACSource,      pFindBuffer->lan_source_addr,      6);
    HexString(szRouteInfo,      pFindBuffer->lan_destination_addr, 18);

    printf("  Name entry %d:\n"
           "    length                = %d\n"
           "    access control        = %02xh\n"
           "    frame  control        = %02xh\n",
           i,
           pFindBuffer->lan_entry_length,
           pFindBuffer->lan_pcf0,
           pFindBuffer->lan_pcf1);
    
    if (pFindBuffer->lan_entry_length >= 8)
      printf("    destination MAC       = %s\n",
             szMACDestination);
                  
    if (pFindBuffer->lan_entry_length >= 14)
      printf("    source MAC            = %s\n",
           szMACSource);
             
    if (pFindBuffer->lan_entry_length >= 32)
      printf("    routing info          = %s\n",
             szRouteInfo);
  }

  return NO_ERROR;
}


/***********************************************************************
 * Name      : 
 * Funktion  : 
 * Parameter : 
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-02-28]
 ***********************************************************************/

// This structure is missing in IBMs Toolkit
#pragma pack(1)
typedef struct _NAME_BUFFER 
{
  UCHAR name[NCBNAMSZ];
  UCHAR name_num;
  UCHAR name_flags;
} NAME_BUFFER, *PNAME_BUFFER;
#pragma pack()

//  values for name_flags bits.

#define NAME_FLAGS_MASK 0x87

#define NAME_SCOPE      0xf0
#define NAME_STATUS     0x0f
#define GROUP_NAME      0x80
#define UNIQUE_NAME     0x00

#define REGISTERING     0x00
#define REGISTERED      0x04
#define DEREGISTERED    0x05
#define DUPLICATE       0x06
#define DUPLICATE_DEREG 0x07



APIRET netbios_ListNames(void)
{
  NCB   ncb;
  UCHAR rc;
  char  szBuffer[ 2048 ]; // NETBIOS crashes if that's too large
  PSZ   pszNameStatus;
  int   i;
  struct ncb_status_information* pAdapterStatus;
  struct ncb_extended_status* pAdapterStatusEx;
  PNAME_BUFFER pNameBuffer;
  char  szMAC[64];
  char  szMACLocal[64];
  PSZ   pszAdapterType;
  char  szRevision[8];
  PSZ   pszNameScope;
  CHAR  szName[NCBNAMSZ];
  CHAR  cNameType;
  PSZ   pszNameType;
  

  // setup the network control block
  // @@@PH LANA number
  init_NCB( &ncb, NB_STATUS_WAIT, 0);
  ncb.ncb_buffer  = szBuffer;
  ncb.ncb_length  = sizeof( szBuffer );
  
  // build netbios name
  buildNetbiosName(ncb.ncb_callname, Options.pszName, Options.ucNameType);
  
  // send NCB
  rc = i_NetBios32Submit( &ncb );
  
  printf("netbios returned %d\n",
         rc);
  
  // display found information
  pAdapterStatus = (struct ncb_status_information *)ncb.ncb_buffer;
  
  HexString(szMAC, pAdapterStatus->burned_in_addr, 6);
  
  switch (pAdapterStatus->software_level_number & 0x00ff)
  {
    case 0xfe: pszAdapterType = "Ethernet adapter"; break;
    case 0xff: pszAdapterType = "Token Ring adapter"; break;
    default:   pszAdapterType = "unknown adapter"; break;
  }
  
  // according to M$ documentation, software revision builds from:
  sprintf(szRevision, 
          "%d.%d", 
          (pAdapterStatus->software_level_number >> 8),
          pAdapterStatus->reserved1[0]);
  
  printf("Adapter Status:\n"
         "  Adapter's burned in addr = %s\n"
         "  reserved                 = %02xh %02xh\n"
         "  software revision        = %s\n"
         "  software level number    = %04xh (%s)\n"
         "  reporting period (min)   = %d\n"
         "  frmr frames received     = %d\n"
         "  frmr frames sent         = %d\n",
         szMAC,
         pAdapterStatus->reserved1[0],
         pAdapterStatus->reserved1[1],
         szRevision,
         pAdapterStatus->software_level_number,
         pszAdapterType,
         pAdapterStatus->reporting_period,
         pAdapterStatus->frmr_frames_received,
         pAdapterStatus->frmr_frames_sent);
         
  printf("  bad iframes received     = %d\n"
         "  aborted transmissions    = %d\n"
         "  packets transmitted      = %d\n"
         "  packets received         = %d\n"
         "  bad iframes transmitted  = %d\n"
         "  lost SAP buffer data cnt = %d\n"
         "  number of T1 expirations = %d\n"
         "  number of Ti expirations = %d\n",
         pAdapterStatus->bad_iframes_received,
         pAdapterStatus->aborted_transmissions,
         pAdapterStatus->packets_transmitted,
         pAdapterStatus->packets_received,
         pAdapterStatus->bad_iframes_transmitted,
         pAdapterStatus->lost_data_count,
         pAdapterStatus->t1_expiration_count,
         pAdapterStatus->ti_expiration_count);
         
  printf("  number of free NCBs      = %d\n"
         "  max configured NCBs      = %d\n"
         "  max allowed NCBs         = %d\n"
         "  busy condition count     = %d\n"
         "  max datagram size        = %d\n"
         "  pending sessions         = %d\n"
         "  max configured sessions  = %d\n"
         "  max allowed sessions     = %d\n"
         "  max data packet size     = %d\n"
         "  number of names present  = %d\n",
         pAdapterStatus->number_of_free_ncbs,
         pAdapterStatus->max_configured_ncbs,
         pAdapterStatus->max_allowed_ncbs,
         pAdapterStatus->busy_condition_count,
         pAdapterStatus->max_datagram_size,
         pAdapterStatus->pending_sessions,
         pAdapterStatus->max_configured_sessions,
         pAdapterStatus->max_allowed_sessions,
         pAdapterStatus->max_data_packet_size,
         pAdapterStatus->number_of_names_present);
  
  if (pAdapterStatus->extended_status_table != NULL)
  {
    pAdapterStatusEx = (struct ncb_extended_status*) pAdapterStatus->extended_status_table;
    HexString(szMAC, pAdapterStatusEx->local_adapter_address, 6);
    printf("Extended status:\n"
           "  local adapter address    = %s\n",
           szMACLocal);
  }
  
  
  pNameBuffer = (PNAME_BUFFER)(pAdapterStatus + 1);
  for (i = 0;
       i < pAdapterStatus->number_of_names_present;
       i++)
  {
    // extract ASCII part of name
    strncpy(szName,
            pNameBuffer->name,
            NCBNAMSZ);
    szName[NCBNAMSZ-1] = 0;
    cNameType = pNameBuffer->name[NCBNAMSZ-1];
    
    if (pNameBuffer->name_flags & GROUP_NAME)
    {
      switch (cNameType)
      {
        case NBNAME_DG_WORKSTATION: pszNameType = "Domain: LM browser proxy"; break;
        case NBNAME_DG_PDC:         pszNameType = "Domain: PDC+BDC"; break;
        case NBNAME_DG_BROWSER:     pszNameType = "Domain: Browser Election"; break;
        case NBNAME_MSBROWSE:       pszNameType = "Domain: Master Browser"; break;
        case 0x2f: pszNameType = "Lotus Notes (multicast)"; break;
        case 0x33: pszNameType = "Lotus Notes (nameserver)"; break;
        default: pszNameType = "(unknown name type)"; break;
      }
    }
    else
    {
      switch (cNameType)
      {
        case NBNAME_CU_WORKSTATION: pszNameType = "workstation"; break;
        case NBNAME_CU_MESSENGER:   pszNameType = "messenger / user"; break;
        case NBNAME_CU_RASSERVER:   pszNameType = "RAS server"; break;
        case NBNAME_CU_NETDDE:      pszNameType = "NET DDE"; break;
        case NBNAME_CU_SERVER:      pszNameType = "Server"; break;
        case NBNAME_CU_RASCLIENT:   pszNameType = "RAS client"; break;
        case NBNAME_CU_NETMONAGENT: pszNameType = "Network Monitoring Agent"; break;
        case NBNAME_CU_NETMONUTIL:  pszNameType = "Network Monitoring Utility"; break;
        case NBNAME_DU_PDC:         pszNameType = "Primary Domain Controller"; break;
        case NBNAME_DU_BROWSER:     pszNameType = "Backup Browser"; break;
        
        case 0x01: pszNameType = "Messenger Service"; break;
        case 0x22: pszNameType = "Exchange Interchange"; break;
        case 0x23: pszNameType = "Exchange Store"; break;
        case 0x24: pszNameType = "Exchange Directory"; break;
        case 0x30: pszNameType = "Modem Sharing Server Service"; break;
        case 0x31: pszNameType = "Modem Sharing Client Service"; break;
        case 0x43: pszNameType = "SMS Client Remote Control"; break;
        case 0x44: pszNameType = "SMS Admin Remote Control"; break;
        case 0x45: pszNameType = "SMS Client Remote Chat"; break;
        case 0x46: pszNameType = "SMS Client Remote Transfer"; break;
        case 0x4c: pszNameType = "DEC Pathworks TCPIP Service"; break;
        case 0x52: pszNameType = "DEC Pathworks TCPIP Service"; break;
        case 0x87: pszNameType = "Exchange MTA"; break;
        case 0x6a: pszNameType = "Exchange IMC"; break;
        case 0x2b: pszNameType = "Lotus Notes Server"; break;
      }
    }

    switch (pNameBuffer->name_flags & NAME_SCOPE)
    {
      case GROUP_NAME:  pszNameScope = "Group "; break;
      case UNIQUE_NAME: pszNameScope = "Unique"; break;
      default:          pszNameScope = "(???) "; break;
    }
    
    switch (pNameBuffer->name_flags & NAME_STATUS)
    {
      case REGISTERING: pszNameStatus = "Registering"; break;
      case REGISTERED:  pszNameStatus = "Registered"; break;
      case DEREGISTERED: pszNameStatus = "Deregistered"; break;
      case DUPLICATE: pszNameStatus = "Duplicate"; break;
      case DUPLICATE_DEREG: pszNameStatus = "Duplicate deregistered"; break;
      default: pszNameStatus = "(unknown status)"; break;
    }

    printf("  %d. [%-15s] %02x-%-25s %02xh-%s %s\n",
           pNameBuffer->name_num,
           szName,
           cNameType,
           pszNameType,
           pNameBuffer->name_flags,
           pszNameScope,
           pszNameStatus);
    
    pNameBuffer++;
  }
  
  return NO_ERROR;
}



/***********************************************************************
 * Name      : 
 * Funktion  : 
 * Parameter : 
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-02-28]
 ***********************************************************************/

APIRET netbios_DelName(void)
{
  NCB   ncb;
  UCHAR rc;
  char  szBuffer[ 512 ]; // NETBIOS crashes if that's too large
  
  
  // setup the network control block
  // @@@PH LANA number
  init_NCB( &ncb, NB_DELETE_NAME_WAIT, 0);
  ncb.ncb_buffer  = szBuffer;
  ncb.ncb_length  = sizeof( szBuffer );
  
  // build netbios name
  buildNetbiosName(ncb.ncb_name, Options.pszName, Options.ucNameType);
  
  // send NCB
  rc = i_NetBios32Submit( &ncb );
  
  printf("netbios returned %d\n",
         rc);
  
  return NO_ERROR;
}


/***********************************************************************
 * Name      : 
 * Funktion  : 
 * Parameter : 
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-02-28]
 ***********************************************************************/

APIRET netbios_AddName(void)
{
  NCB   ncb;
  UCHAR rc;
  char  szBuffer[ 512 ]; // NETBIOS crashes if that's too large
  
  
  // setup the network control block
  // @@@PH LANA number
  init_NCB( &ncb, NB_ADD_NAME_WAIT, 0);
  ncb.ncb_buffer  = szBuffer;
  ncb.ncb_length  = sizeof( szBuffer );
  
  // build netbios name
  buildNetbiosName(ncb.ncb_name, Options.pszName, Options.ucNameType);
  
  // send NCB
  rc = i_NetBios32Submit( &ncb );
  
  printf("netbios returned %d\n",
         rc);
  
  return NO_ERROR;
}


/***********************************************************************
 * Name      : 
 * Funktion  : 
 * Parameter : 
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-02-28]
 ***********************************************************************/

APIRET netbios_AddGroupName(void)
{
  NCB   ncb;
  UCHAR rc;
  char  szBuffer[ 512 ]; // NETBIOS crashes if that's too large
  
  
  // setup the network control block
  // @@@PH LANA number
  init_NCB( &ncb, NB_ADD_GROUP_NAME_WAIT, 0);
  ncb.ncb_buffer  = szBuffer;
  ncb.ncb_length  = sizeof( szBuffer );
  
  // build netbios name
  buildNetbiosName(ncb.ncb_name, Options.pszName, Options.ucNameType);
  
  // send NCB
  rc = i_NetBios32Submit( &ncb );
  
  printf("netbios returned %d\n",
         rc);
  
  return NO_ERROR;
}


/***********************************************************************
 * Name      : 
 * Funktion  : 
 * Parameter : 
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-02-28]
 ***********************************************************************/




ULONG getCommandCode(PSZ pszCommand)
{
  PNBCOMMAND pCmd = arrCommands;
  
  for (;
       pCmd->pszCommand != NULL;
       pCmd++)
  {
    if (0 == stricmp(pszCommand,
                     pCmd->pszCommand) )
      return pCmd->ulCommand;
  }
  
  return -1;
}


void listCommands(void)
{
  int i = 0;
  PNBCOMMAND pCmd = arrCommands;
  
  printf("\nSupported Commands:\n  ");
  
  for (;
       pCmd->pszCommand != NULL;
       pCmd++)
  {
    printf("%-19s",
           pCmd->pszCommand);
    
    i++;
    if (i % 4 == 0)
      printf("\n  ");
  }
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
    
    listCommands();
    
    return (NO_ERROR);
  }
  
  
  /* command router */
  ulCmd = getCommandCode( Options.pszCommand );
  switch (ulCmd)
  {
    case CMD_GETINFO:
      rc = netbios_GetInfo();
      break;
    
    case CMD_LISTADAPTERS:
      rc = netbios_EnumAdapters();
      break;
  
    case CMD_FINDNAME:
      rc = netbios_FindName();
      break;
  
    case CMD_LISTNAMES:
      rc = netbios_ListNames();
      break;
  
    case CMD_DELNAME:
      rc = netbios_DelName();
      break;
  
    case CMD_ADDNAME:
      rc = netbios_AddName();
      break;
    
    case CMD_ADDGRNAME:
      rc = netbios_AddGroupName();
      break;

    default:
      printf("ERROR: '%s' is not a known command.\n",
             Options.pszCommand);
      rc = ERROR_INVALID_FUNCTION;
  }
  
  if (NO_ERROR != rc)
    ToolsErrorDos ( rc );
  
  return rc;
}
