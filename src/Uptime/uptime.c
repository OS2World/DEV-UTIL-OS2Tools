/*****************************************************
 * Unix-like Uptime Tool.                            *
 * Prints the time the system is running so far.     *
 * (c) 1994-95 Patrick Haller Systemtechnik          *
 *****************************************************/

 /* ToDo:
  Laptops takten den Counter runter ->
  wieder Zeit merken bzw. DosQuerySysInfo
  */

#ifdef __OS2__
  #define INCL_DOS
  #define INCL_DOSERRORS
  #define INCL_NOPM
  #include <os2.h>
#endif

#ifdef _WIN32
  #include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsTimer;                              /* display timer information */
  ARGFLAG fsVerbose;                                      /* verbose display */

#ifdef __OS2__
  ARGFLAG fsSystem;                            /* display system information */
#endif

} OPTIONS, *POPTIONS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/
OPTIONS Options;                /* this structure holds command line options */

ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung------------------pTarget-ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",           NULL,   ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",           NULL,   ARG_NULL,       &Options.fsHelp},
  {"/T",         "Display timer information.", NULL,   ARG_NULL,       &Options.fsTimer},
  {"/V",         "Verbose display.",           NULL,   ARG_NULL,       &Options.fsVerbose},
#ifdef __OS2__
  {"/S",         "Display system information.",NULL,   ARG_NULL,       &Options.fsSystem},
#endif
  ARG_TERMINATE
};


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

int  InfoTimer  (void);
void help       (void);
int  InfoUptime (void);


#ifdef __OS2__
int  InfoSystem (void);
#endif


int  main       (int argc, char *argv[]);



/***********************************************************************
 * Name      : int InfoTimer
 * Funktion  : Ermitteln der Daten ber den Timer.
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 12.09.1995 06.13.44]
 ***********************************************************************/

int InfoTimer (void)
{
  double  realtime;

#ifdef __OS2__
  ULONG   ulFreq;
  QWORD   qwTime;
  APIRET  rc;                                          /* Rckgabewert */

  rc = DosTmrQueryFreq (&ulFreq);
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);
    return (rc);
  }

  if ((rc = DosTmrQueryTime(&qwTime)) != NO_ERROR)
  {
    ToolsErrorDos(rc);
    return (rc);
  }

  realtime = (double)qwTime.ulHi;
  realtime = realtime * 256.0 * 256.0 * 256.0 * 256.0;
  realtime += (double)qwTime.ulLo;

  realtime = realtime / (double)ulFreq;
  printf ("Timer runs at %u Hz, is up since %f sec.\n",
          ulFreq,
          realtime);

  return (NO_ERROR);
#endif

#ifdef _WIN32
  LARGE_INTEGER liFrequency;
  LARGE_INTEGER liTime;

  if (FALSE == QueryPerformanceFrequency(&liFrequency))
  {
        ToolsErrorDos(ERROR_NOT_SUPPORTED);
        return (ERROR_NOT_SUPPORTED);
  }

  if (FALSE == QueryPerformanceCounter(&liTime))
  {
        ToolsErrorDos(ERROR_NOT_SUPPORTED);
        return (ERROR_NOT_SUPPORTED);
  }

  realtime = (double)liTime.QuadPart / (double)liFrequency.QuadPart;

  printf ("Timer runs at %u Hz, is up since %12.3f sec.\n",
          liFrequency.LowPart,
          realtime);

  return (NO_ERROR);
#endif

} /* int InfoTimer */


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
  TOOLVERSION("Uptime",                                 /* application name */
              0x00010004,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


/*****************************************************************************
 * Name      : int InfoUptime
 * Funktion  : Ermitteln und anzeigen der Systemlaufzeit
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 12.09.1995 06.07.59]
 *****************************************************************************/

#ifdef __OS2__
int InfoUptime (void)
{
  APIRET rc;                                                 /* Rckgabewert */
  PSZ    pszClockMessage = NULL;           /* special messages for the timer */
  double dTimerDifference;                      /* difference between timers */

  time_t t;
  QWORD   qwTime;
  double  realtime;
  double  dSystemTime;
  ULONG   ulSeconds;
  ULONG   ulFreq;

  USHORT r_days;
  USHORT r_hours;
  USHORT r_minutes;
  USHORT r_seconds;

  struct {
         ULONG OSMajor;
         ULONG OSMinor;
         ULONG OSRev;
         ULONG mscount;
       } buffer;


  rc = DosQuerySysInfo (QSV_VERSION_MAJOR,
                        QSV_MS_COUNT,
                        &buffer,
                        sizeof(buffer));

  if (rc == NO_ERROR)
  {
    rc = DosTmrQueryFreq (&ulFreq);
    if (rc != NO_ERROR)
      return (rc);

    if ((rc = DosTmrQueryTime(&qwTime)) != NO_ERROR)
      return (rc);

    realtime = (double)qwTime.ulHi;
    realtime = realtime * 256.0 * 256.0 * 256.0 * 256.0;
    realtime += (double)qwTime.ulLo;

    realtime = realtime / (double)ulFreq;

    ulSeconds = (ULONG)realtime;


    dSystemTime = buffer.mscount / 1000;    /* calculate seconds from sysval */

    if ( (dSystemTime != 0) &&                                /* div 0 check */
          (realtime    != 0) )
    {
      if (dSystemTime - realtime > 100)      /* more than 100 sec difference */
      {
        ulSeconds = (ULONG)dSystemTime;
        pszClockMessage = "(840ns Timer is unreliable or powersaved)\n";
      }

      if (realtime - dSystemTime > 100)      /* more than 100 sec difference */
      {
        ulSeconds = (ULONG)realtime;
        pszClockMessage = "(1000ns Timer overflow)\n";
      }
    }

    r_days    = (ulSeconds / 86400);
    r_hours   = (ulSeconds / 3600) % 24;
    r_minutes = (ulSeconds / 60) % 60;
    r_seconds = ulSeconds % 60;

    printf ("\rOS/2 %u.%u.%u running %03ud %02uh %02um %02us.",
            buffer.OSMajor / 10,
            buffer.OSMinor,
            buffer.OSRev,
            r_days,
            r_hours,
            r_minutes,
            r_seconds);

    if (Options.fsVerbose)
    {
      t = time(NULL);
      t -= ulSeconds;
      printf ("   Boot: %s",
              ctime(&t));

      if (pszClockMessage != NULL)    /* check if there is a special message */
        printf (pszClockMessage);

      if (Options.fsTimer)             /* provide more information on timers */
      {
        dTimerDifference = dSystemTime - realtime;

        if (dTimerDifference > 0)
          printf ("System timer is %10.3fs ahead hardware timer.",
                 dTimerDifference);
        else
          printf ("Hardware timer is %10.3fs ahead system timer.",
                  -dTimerDifference);
      }
    }
  }
  else
    ToolsErrorDos(rc);

  return (rc);                                 /* Rckgabewert liefern */
} /* int InfoUptime */
#endif


#ifdef _WIN32
int InfoUptime (void)
{
  LARGE_INTEGER liFrequency;
  LARGE_INTEGER liTime;
  double        realtime;
  double        fTime;
  double        fFrequency;
  ULONG         ulSeconds;
  ULONG         r_days,
                    r_hours,
                                r_minutes,
                                r_seconds;
  OSVERSIONINFO osvi;                    /* Win32 Version information record */
  PSZ           pszVersion;                  /* points to the Win32 platform */

  if (FALSE == QueryPerformanceFrequency(&liFrequency))
  {
        ToolsErrorDos(ERROR_NOT_SUPPORTED);
        return (ERROR_NOT_SUPPORTED);
  }

  if (FALSE == QueryPerformanceCounter(&liTime))
  {
        ToolsErrorDos(ERROR_NOT_SUPPORTED);
        return (ERROR_NOT_SUPPORTED);
  }

  fTime      = 0.0;
  fTime      = liTime.QuadPart;
  fFrequency = liFrequency.QuadPart;
  realtime   = fTime / fFrequency;

  ulSeconds  = (ULONG)realtime;
  r_days    = (ulSeconds / 86400);
  r_hours   = (ulSeconds / 3600) % 24;
  r_minutes = (ulSeconds / 60) % 60;
  r_seconds = ulSeconds % 60;

  osvi.dwOSVersionInfoSize = sizeof(osvi);      /* query the windows version */
  if (GetVersionEx(&osvi) == 0)
        return (GetLastError());

  switch (osvi.dwPlatformId)
  {
    case VER_PLATFORM_WIN32s:        pszVersion = "3.1 (Win32s)"; break;
    case VER_PLATFORM_WIN32_WINDOWS: pszVersion = "95";           break;
    case VER_PLATFORM_WIN32_NT:      pszVersion = "NT";           break;
    default:                         pszVersion = "<unknown>";    break;
  }

  printf ("\rMicrosoft Windows %s %u.%u (Build %u, %s) running %03ud %02uh %02um %02us.",
              pszVersion,
          osvi.dwMajorVersion,
                  osvi.dwMinorVersion,
                  osvi.dwBuildNumber,
                  osvi.szCSDVersion,
          r_days,
          r_hours,
          r_minutes,
          r_seconds);

  return (NO_ERROR);
}
#endif


/***********************************************************************
 * Name      : int InfoSystem
 * Funktion  : Ermitteln und Anzeigen einer Systemwerte
 * Parameter :
 * Variablen :
 * Ergebnis  : Fehlercode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 12.09.1995 06.56.43]
 ***********************************************************************/

#ifdef __OS2__
int InfoSystem (void)
{
  PSZ    pszPriVar;
  PSZ    pszOSVer;
  PSZ    pszCPUImage;
  PSZ    pszInt10Enabled;
  APIRET rc;                                               /* API returncode */
  USHORT usInformationLevel;                            /* information level */

#define INFOLEVEL_STANDARD 1L         /* standard OS/2 2.x information level */
#define INFOLEVEL_SMPV2    2l              /* OS/2 2.x SMP information level */
#define INFOLEVEL_SMPV3    3l              /* OS/2 3.x SMP information level */
#define INFOLEVEL_SMPV4    4l              /* OS/2 4.x SMP information level */

  struct {
         ULONG ulMaxPathLength;        /* Maximum length of file paths */
         ULONG ulMaxTextSessions;   /* Maximum number of text sessions */
         ULONG ulMaxPMSessions;     /* Maximum number of PM   sessions */
         ULONG ulMaxVDMSessions;    /* Maximum number of VDM  sessions */

         ULONG ulBootDrive;          /* Which drive did we boot from ? */
         ULONG ulPriorityVariation;    /* How is the sceduler set up ? */
         ULONG ulMaxWait;                   /* Maximum wait in seconds */
         ULONG ulMinSlice;                       /* Minimum time slice */
         ULONG ulMaxSlice;                       /* Maximum time slice */
         ULONG ulPageSize;                       /* Page size in bytes */
         ULONG ulVerMajor;                     /* Version number major */
         ULONG ulVerMinor;                     /* Version number minor */
         ULONG ulVerRevision;                  /* Version number rev.  */

         ULONG ulMsCount;             /* already handled by InfoUptime */
         ULONG ulTimeLow;
         ULONG ulTimeHigh;

         ULONG ulTotPhysMem;                  /* Total physical memory */
         ULONG ulTotResMem;                   /* Total resident memory */
         ULONG ulTotAvailMem;                /* Total available memory */
         ULONG ulMaxPrivateMem;              /* Maximum private memory */
         ULONG ulMaxSharedMem;                /* Maximum shared memory */

         ULONG ulTimerInterval;                /* timer interval in ms */
         ULONG ulMaxCompLength;             /* maximum filename length */

         ULONG ulIDFSession;
         ULONG ulIDFPID;

         ULONG ulCPUs;                /* Numbers of processors in the system */
         ULONG ulMaxHighPrivateArena; /* maximum amount of free space in the */
                                           /* private arena for this process */
         ULONG ulMaxHighSharedArena;      /* maximum amount of free space in */
                                   /* the high shared arena for this process */
         ULONG ulMaxProcesses;     /* maximum number of concurrent processes */
         ULONG ulVirtualLimit;         /* size of user's address space in MB */

         ULONG ulInt10Enabled;                /* Int10 service VDM available */
  } SystemQueryBuffer;

#ifndef QSV_NUMPROCESSORS
#define QSV_NUMPROCESSORS       26
#endif

#ifndef QSV_MAXHPRMEM
#define QSV_MAXHPRMEM           27
#endif

#ifndef QSV_MAXHSHMEM
#define QSV_MAXHSHMEM           28
#endif

#ifndef QSV_MAXPROCESSES
#define QSV_MAXPROCESSES        29
#endif

#ifndef QSV_VIRTUALADDRESSLIMIT
#define QSV_VIRTUALADDRESSLIMIT 30
#endif

#ifndef QSV_INT10ENABLED
#define QSV_INT10ENABLED        31
#endif

  memset(&SystemQueryBuffer,
         0,
         sizeof(SystemQueryBuffer));

  usInformationLevel = INFOLEVEL_SMPV4;
  rc = DosQuerySysInfo(QSV_MAX_PATH_LENGTH,    /* query advanced information */
                       QSV_INT10ENABLED,
                       &SystemQueryBuffer,
                       sizeof(SystemQueryBuffer));
                                /* this OS/2 can't give advanced information */
  if (rc == ERROR_INVALID_PARAMETER)
  {
    usInformationLevel = INFOLEVEL_SMPV3;

    rc = DosQuerySysInfo(QSV_MAX_PATH_LENGTH,    /* query advanced information */
                         QSV_MAXHSHMEM,
                         &SystemQueryBuffer,
                         sizeof(SystemQueryBuffer));
                                  /* this OS/2 can't give advanced information */
    if (rc == ERROR_INVALID_PARAMETER)
    {
      usInformationLevel = INFOLEVEL_SMPV2;
  
      rc = DosQuerySysInfo (QSV_MAX_PATH_LENGTH,
                            QSV_NUMPROCESSORS,          /* 26 = Num processors */
                            &SystemQueryBuffer,
                            sizeof(SystemQueryBuffer));
      if (rc == ERROR_INVALID_PARAMETER)     /* OS version can't give all info */
      {
        usInformationLevel = INFOLEVEL_STANDARD;
        rc = DosQuerySysInfo (QSV_MAX_PATH_LENGTH,
                              QSV_MAX_COMP_LENGTH,
                              &SystemQueryBuffer,
                              sizeof(SystemQueryBuffer));
      }
    }
  }


  /* How many CPUs have we found ? */
  if ( (SystemQueryBuffer.ulCPUs == 0) ||
       (SystemQueryBuffer.ulCPUs == 1) )
  {
    SystemQueryBuffer.ulCPUs = 1;
    pszCPUImage = "         Uniprocessor Image";
  }
  else
    pszCPUImage = "Multiprocessing Image (SMP)";


  if (rc == NO_ERROR)
  {
    pszOSVer = "             ";                 /* default is a blank string */

     /* Determine operating system version */
     switch (SystemQueryBuffer.ulVerMajor)
     {
       case 20:
         switch (SystemQueryBuffer.ulVerMinor)
         {
           case 00: pszOSVer = "OS/2 2.0     "; break;
           case 10: pszOSVer = "OS/2 2.1     "; break;
           case 11: pszOSVer = "OS/2 2.11    "; break;
           case 99: pszOSVer = "OS/2 2.99   b"; break;
           case 30: pszOSVer = "OS/2 Warp 3.0"; break;
           case 40: pszOSVer = "OS/2 Warp 4.0"; break;
           case 45: pszOSVer = "OS/2 Warp 4.5"; break;
         }
         break;


       case 30:
         pszOSVer = "OS/2 PowerPC "; break;


       case 90:
         switch (SystemQueryBuffer.ulVerMinor)
         {
           case 99: pszOSVer = "OS/2 Merlin b"; break;
         }
         break;
     }


     /* determine status of INT10 service VDM */

     if (usInformationLevel >= INFOLEVEL_SMPV4)
     {
         if (SystemQueryBuffer.ulInt10Enabled != 0)
             pszInt10Enabled = "enabled                  ";
         else
             pszInt10Enabled = "disabled                 ";
     }
     else
         pszInt10Enabled = "(information unavailable)";


     /* Sessions */
     printf ("ÚÄSessionsÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿"
             "\n³  Textmode: %04u      Presentation Managers: %04u     Virtual Machines: %04u ³"
             "\nÃÄOperating systemÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´"
             "\n³  OS/2 %u.%u.%u (%s)   Processors:   %02u %s ³"
             "\n³  Bootdrive: %c:   Maximum path length: %04u   Maximum component length: %04u ³"
             "\n³  Int 10 service VDM: %s                              ³"
             "\nÃÄMemory ManagementÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´"
             "\n³  Page size:                    %02uk (%012u bytes)                     ³"
             "\n³  Physical  memory:       %08uk (%012u bytes)                     ³"
             "\n³  Resident  memory:       %08uk (%012u bytes)                     ³"
             "\n³  Available memory:       %08uk (%012u bytes)                     ³"
             "\n³  Maximum private memory: %08uk (%012u bytes)                     ³"
             "\n³  Maximum shared  memory: %08uk (%012u bytes)                     ³",

             SystemQueryBuffer.ulMaxTextSessions,
             SystemQueryBuffer.ulMaxPMSessions,
             SystemQueryBuffer.ulMaxVDMSessions,

             SystemQueryBuffer.ulVerMajor / 10,
             SystemQueryBuffer.ulVerMinor,
             SystemQueryBuffer.ulVerRevision,
             pszOSVer,
             SystemQueryBuffer.ulCPUs,
             pszCPUImage,
             (UCHAR)(SystemQueryBuffer.ulBootDrive + 'A' - 1),
             SystemQueryBuffer.ulMaxPathLength,
             SystemQueryBuffer.ulMaxCompLength,
             pszInt10Enabled,

             SystemQueryBuffer.ulPageSize / 1024,
             SystemQueryBuffer.ulPageSize,
             SystemQueryBuffer.ulTotPhysMem / 1024,
             SystemQueryBuffer.ulTotPhysMem,
             SystemQueryBuffer.ulTotResMem / 1024,
             SystemQueryBuffer.ulTotResMem,
             SystemQueryBuffer.ulTotAvailMem / 1024,
             SystemQueryBuffer.ulTotAvailMem,
             SystemQueryBuffer.ulMaxPrivateMem / 1024,
             SystemQueryBuffer.ulMaxPrivateMem,
             SystemQueryBuffer.ulMaxSharedMem / 1024,
             SystemQueryBuffer.ulMaxSharedMem
            );

    if (usInformationLevel >= INFOLEVEL_SMPV3)
      printf("\n³  Max. high private mem.: %08uk (%012u bytes)                     ³"
             "\n³  Max. high shared  mem.: %08uk (%012u bytes)                     ³"
             "\n³  Maximum processes     : %8u                                           ³"
             "\n³  User's address space .: %08uM (%012u GBytes)                    ³",
               SystemQueryBuffer.ulMaxHighPrivateArena / 1024,
               SystemQueryBuffer.ulMaxHighPrivateArena,
               SystemQueryBuffer.ulMaxHighSharedArena / 1024,
               SystemQueryBuffer.ulMaxHighSharedArena,
               SystemQueryBuffer.ulMaxProcesses,
               SystemQueryBuffer.ulVirtualLimit,
               SystemQueryBuffer.ulVirtualLimit / 1024);


     /* Determine priority variation */
     switch(SystemQueryBuffer.ulPriorityVariation)
     {
       case 0:  pszPriVar = "absolute"; break;
       case 1:  pszPriVar = "dynamic "; break;
       default: pszPriVar = "unknown "; break;
     }

     printf ("\nÃÄSchedulerÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´"
             "\n³  Timer Interval:     %4.1f ms         Maximum Wait:      %04u seconds        ³"
             "\n³  Minimum Timeslice:  %04u ms         Maximum Timeslice: %04u ms             ³"
             "\n³  Priority Variation: %s (%1u)    Last PID:          %05u               ³"
             "\nÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ\n",
             (float)(SystemQueryBuffer.ulTimerInterval / 10),
             SystemQueryBuffer.ulMaxWait,
             SystemQueryBuffer.ulMinSlice,
             SystemQueryBuffer.ulMaxSlice,
             pszPriVar,
             SystemQueryBuffer.ulPriorityVariation,
             SystemQueryBuffer.ulIDFPID
             );
  }
  else
    ToolsErrorDos(rc);

  return (rc);
} /* int InfoSystem */
#endif


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
  APIRET rc;                                               /* API returncode */

  rc = ArgStandard (argc,                            /* CLI-Parameter parsen */
                    argv,
                    TabArguments,
                    &Options.fsHelp);
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    exit(1);                                                /* abort program */
  }

  if ( Options.fsHelp)                                 /* user requests help */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  /* Anzeigen der Informationen */
#ifdef __OS2__
  if (Options.fsSystem) rc = InfoSystem();
#endif
  if (Options.fsTimer ) rc = InfoTimer();
  rc = InfoUptime();

  return rc;
}
