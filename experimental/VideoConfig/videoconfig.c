/*****************************************************************************
 * Projekt   : OS/2 Video Configuration Manager
 * Name      : VIDEOCONFIG.C
 * Funktion  : Providing an user interface to OS/2s built in video manager
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung : You need SVGADEFS.H from the IBM DDK!
 *
 * Autor     : Patrick Haller [Freitag, 11.10.1996 05.34.20]
 *****************************************************************************/

#define INCL_DOSERRORS
#include <os2.h>

//#define VIDEOPMI_30_LEVEL 1

#include <svgadefs.h>                          /* SVGADEFS.H of the OS/2 DDK */
#include <stdio.h>
#include <stdlib.h>


/*****************************************************************************
 * Projekt   : OS/2 Video Configuration Manager
 * Name      : dumpmonitornum
 * Funktion  : Gets number of available monitor definitions on MONITOR.DIF
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 11.10.1996 05.34.20]
 *****************************************************************************/

void dumpmonitornum(void)
{
  ULONG  ulMonitors;                                   /* number of monitors */
  APIRET rc;                                               /* API-Returncode */
  
  rc = QueryNumMonitors(&ulMonitors);                      /* get the number */
  if (rc == NO_ERROR)                                        /* check errors */
    printf ("\nMonitors definitions available: %u", 
            ulMonitors);
  else
    printf ("\ndumpmonitornum::QueryNumMonitors returned rc=%u",
            rc);
}


/*****************************************************************************
 * Projekt   : OS/2 Video Configuration Manager
 * Name      : dumpcurrentconfiguration
 * Funktion  : Gets current monitor and adapter information
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 11.10.1996 05.34.20]
 *****************************************************************************/

void dumpcurrentconfiguration(void)
{
  MONITORINFO MonitorInfo;                            /* Monitor Information */
  ADAPTERINFO AdapterInfo;                      /* Video Adapter Information */
  APIRET      rc;                                          /* API-Returncode */
  ULONG       ulMode;             /* loop counter for monitor mode detection */
  
  rc = GetCurrentCfg(&AdapterInfo,       /* get the pointers to the registry */
                     &MonitorInfo);
  if (rc != NO_ERROR)
  {
    printf ("\ndumpcurrentconfiguration::GetCurrentCfg returned rc=%u",
            rc);
    return;
  }
  
  printf("\nAdapter"
         "\n-------\n");

  printf ("\n  Adapter ID       : %u"
          "\n  Adapter OEM      : %s"
          "\n  Adapter DAC      : %s"
          "\n  Adapter Revision : %s"
          "\n  Total Memory     : %s"
          "\n  MMIOBase Address : %08x"
          "\n  PIOBase  Address : %08x"
          "\n  Bus Type         : %u"
          "\n  Endian           : %u"
          "\n  Device Bus ID    : %u"
          "\n  Vendor Bus ID    : %u"
          "\n  Slot       ID    : %u",
          AdapterInfo.ulAdapterID,
          AdapterInfo.szOEMString,
          AdapterInfo.szDACString,
          AdapterInfo.szRevision,
          AdapterInfo.ulTotalMemory,
          AdapterInfo.ulMMIOBaseAddress,
          AdapterInfo.ulPIOBaseAddress,
          AdapterInfo.bBusType,
          AdapterInfo.bEndian,
          AdapterInfo.usDeviceBusID,
          AdapterInfo.usVendorBusID,
          AdapterInfo.SlotID);

  
  printf ("\nMonitor"
          "\n-------\n");
  
  printf ("\n  Monitor Name                 : %s",
          MonitorInfo.szMonitor);
  
  printf ("\nModes"
          "\n-----\n");
  for (ulMode = 0;
       ulMode < 10;
       ulMode++)
  {
    printf ("\n  [Mode %u]"
            "\n    %5u x %5u at %3uHz vertical, %3ukHz horizontal"
            "\n    Vertical Polarity Position   : %u"
            "\n    Horizontal Polarity Position : %u",
            ulMode,
            MonitorInfo.ModeInfo[ulMode].usXResolution,
            MonitorInfo.ModeInfo[ulMode].usYResolution,
            MonitorInfo.ModeInfo[ulMode].bVertRefresh,
            MonitorInfo.ModeInfo[ulMode].bHorizRefresh,
            MonitorInfo.ModeInfo[ulMode].bVPolarityPos,
            MonitorInfo.ModeInfo[ulMode].bHPolarityPos);
  }
}


void dumpinfo(void)
{
  printf ("\nTestsuite for OS/2 Video Configuration Manager"
          "\n----------------------------------------------\n");

  dumpcurrentconfiguration();
  dumpmonitornum(); 
}

int main (int argc, char *argv[])
{
  dumpinfo();
}