/*****************************************************************************
 * Projekt   : PHS Tools
 * Name      : CPULoad
 * Funktion  : Monitors CPU Load
 * Autor     : Patrick Haller [Montag, 25.09.1995 01.20.20]
 * Remark    : This program requires OS/2 Warp 4.0 or SMP Kernel
 *****************************************************************************/


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#define INCL_BASE
#define INCL_DOSERRORS                               /* Die Fehlerkonstanten */
#define INCL_NOPMAPI                            /* Kein Presentation Manager */
#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>


#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"

#define MAXPATHLEN 260

#define CDECL __cdecl
#include "cpuhlp.h"

#define FLOATTYPE float



/*
 DosPerfSysCall is a general purpose performance function.  This function accepts
 four parameters.  The first parameter is the command requested. The other three
 parameters are command specific.  DosPerfSysCall is intended to be extended by
 IBM in the future for other performance related functions.

 Some functions of DosPerfSysCall may have a dependency on Intel Pentium or
 Pentium-Pro support.  If a function cannot be provided because OS/2 is not
 running on an processor with the required features, a return code will indicate
 an attempt to use an unsupported function.

 The perfutil.h file referenced in the example code may or may not be shipped
 as part of the Toolkit.  In the event that it is not, its contents are:
*/

 #ifdef __cplusplus
   extern "C" {
 #endif

 #ifndef  PERFCALL_INCLUDED
 #define  PERFCALL_INCLUDED

 /*
    DosPerfSysCall Function Prototype
 */

 /* The  ordinal for DosPerfSysCall (in BSEORD.H) */
 /* is defined as ORD_DOS32PERFSYSCALL         */

 APIRET APIENTRY DosPerfSysCall(ULONG ulCommand, ULONG ulParm1, ULONG ulParm2, ULONG ulParm3);

 /***
  *
  * Software Tracing
  * ----------------
  *
  **/

 #define   CMD_SOFTTRACE_LOG (0x14)

  typedef struct _HookData {
     ULONG ulLength;
     PBYTE pData;
  } HOOKDATA;
  typedef HOOKDATA * PHOOKDATA;


 /***
  *
  * CPU Utilization
  * ---------------
  *
  **/

 #define   CMD_KI_RDCNT    (0x63)
     /* PH */
 #define CMD_GETPERFBUFFER 0x40
 #define CMD_INFOPERFBUFFER 0x41
 #define CMD_INITPERFBUFFER 0x42
 #define CMD_FREEPERFBUFFER 0x43
 #define CMD_GETPERFBUFFERHEADERS 0x52
 #define CMD_KI_ENABLE 0x60
 #define CMD_KI_DISABLE 0x61

 typedef struct _CPUUTIL
 {
   ULONG ulTimeLow;     /* Low 32 bits of time stamp      */
   ULONG ulTimeHigh;    /* High 32 bits of time stamp     */
   ULONG ulIdleLow;     /* Low 32 bits of idle time       */
   ULONG ulIdleHigh;    /* High 32 bits of idle time      */
   ULONG ulBusyLow;     /* Low 32 bits of busy time       */
   ULONG ulBusyHigh;    /* High 32 bits of busy time      */
   ULONG ulIntrLow;     /* Low 32 bits of interrupt time  */
   ULONG ulIntrHigh;    /* High 32 bits of interrupt time */
 } CPUUTIL, *PCPUUTIL;

#endif

 #ifdef __cplusplus
   }
 #endif

/* It may also be necessary to define the ordinal for this function: */

 #define ORD_DOS32PERFSYSCALL            976





 /*
    Convert 8-byte (low, high) time value to FLOATTYPE
 */
 #define LL2F(high, low) (4294967296.0*(high)+(low))



/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsCmdOn;                                    /* enable the KI hooks */
  ARGFLAG fsCmdOff;                                  /* disable the KI hooks */
  ARGFLAG fsCmdMonitor;                                  /* monitor the CPUs */
  ARGFLAG fsCmdInfo;                                        /* give CPU info */
  ARGFLAG fsScanTime;                      /* time between CPU scans in ms   */

  ARGFLAG fsCPUTest1;                                 /* CPU test #1 desired */

  ULONG   ulScanTime;                      /* time between CPU scans in ms   */
} OPTIONS, *POPTIONS;


typedef struct _CPUPERF
{
  /* values */
  FLOATTYPE      ts_val,   ts_val_prev;
  FLOATTYPE      idle_val, idle_val_prev;
  FLOATTYPE      busy_val, busy_val_prev;
  FLOATTYPE      intr_val, intr_val_prev;

  ULONG       ulIterations;                             /* iteration counter */

  /* percentage */
  FLOATTYPE      idle_val_perc;
  FLOATTYPE      busy_val_perc;
  FLOATTYPE      intr_val_perc;

  /* maxima */
  FLOATTYPE      idle_val_max;
  FLOATTYPE      busy_val_max;
  FLOATTYPE      intr_val_max;

  /* minima */
  FLOATTYPE      idle_val_min;
  FLOATTYPE      busy_val_min;
  FLOATTYPE      intr_val_min;

  /* average */
  FLOATTYPE      idle_val_sum;
  FLOATTYPE      busy_val_sum;
  FLOATTYPE      intr_val_sum;
} CPUPERF, *PCPUPERF;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */

ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung------------pTarget-ucTargetFormat-pTargetSpecified--*/
  {"/?",         "Get help screen.",     NULL,   ARG_NULL,      &Options.fsHelp},
  {"/H",         "Get help screen.",     NULL,   ARG_NULL,      &Options.fsHelp},
  {"/CPUTEST1",  "Mearure CPU frequency.", NULL, ARG_NULL,      &Options.fsCPUTest1},
  {"/ON",        "Enable Kernel Hooks.", NULL,   ARG_NULL,      &Options.fsCmdOn},
  {"/OFF",       "Disable Kernel Hooks.",NULL,   ARG_NULL,      &Options.fsCmdOff},
  {"/MONITOR",   "Monitor CPU Load.",    NULL,   ARG_NULL,      &Options.fsCmdMonitor},
  {"/INFO",      "Display some CPU info.",NULL,  ARG_NULL,      &Options.fsCmdInfo},
  {"/TIME=",     "Time between CPU scans in milliseconds.",
                          &Options.ulScanTime,   ARG_ULONG,     &Options.fsScanTime},
  ARG_TERMINATE
};


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void   help                  (void);

APIRET displayCPUInfo        (void);

APIRET PerfKIDisable         (void);

APIRET PerfKIEnable          (void);

APIRET PerfKIMonitor         (ULONG  ulScanTime);

void   GenerateExecutionLoop (ULONG  ulInstructions,
                              PUCHAR pucPage);

APIRET AllocateExecutionLoop (PPVOID ppBase,
                              ULONG  ulSize);

double CPUDetermineHZ        (ULONG features);

int    CPUTest1              (void);

int    main                  (int    argc,
                              char   *argv[] );


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
  TOOLVERSION("CPULoad",                                 /* application name */
              0x00010006,                            /* application revision */
              0x00010100,           /* minimum required revision of PHSTOOLS */
              0x0001FFFF,        /* maximum recommended revision of PHSTOOLS */
              "OS/2 Warp 4, Warp 3 SMP or later required.",        /* Remark */
              NULL);                                 /* additional copyright */
}



/***********************************************************************
 * Name      : APIRET displayCPUInfo()
 * Funktion  : Display some information about the kernel's view of the CPU
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/


APIRET APIENTRY DosSysCtl(ULONG ulFunc, PVOID pBuf);

#define SC_CPUTYPE      9
#define SC_IODELAYCOUNT 10
#define SC_NUMBEROFCPUS 22
#define SC_CURRENTCPU   24

#define CPU_286         0
#define CPU_386         1
#define CPU_486         2
#define CPU_PENTIUM     3
#define CPU_PENTIUM_PRO 4

// self-defined
#define CPU_PENTIUM_II  5

#define CPU_MAX CPU_PENTIUM_II

static PSZ arrCPUTypes[] = {"286-Class",
                            "386-Class",
                            "486-Class",
                            "Pentium-Class",
                            "Pentium-Pro-Class",
                            "Pentium-II-Class"};

APIRET displayCPUInfo(void)
{
    APIRET rc;
    ULONG  ulCpus;
    ULONG  ulCpuCurrent;
    ULONG  ulCpuType;
    ULONG  ulIODelayCount;
    PSZ    pszCpuType;

    printf("CPU Information:\n");

#define my_sysctl(a,b,c)   \
    rc = DosSysCtl(a, &b); \
    if (NO_ERROR != rc)    \
      ToolsErrorDosEx(rc, c);

    my_sysctl(SC_NUMBEROFCPUS, ulCpus,        "querying number of installed CPUs");
    my_sysctl(SC_CURRENTCPU,   ulCpuCurrent,  "querying current CPU");
    my_sysctl(SC_CPUTYPE,      ulCpuType,     "querying CPU type");
    my_sysctl(SC_IODELAYCOUNT, ulIODelayCount,"querying I/O delay count");
  
    // Note: ulCpuCurrent is one-based
    printf("  Installed CPUs  : %d (this process is on CPU_%d)\n",
           ulCpus,
           ulCpuCurrent-1);


    if (ulCpuType <= CPU_MAX)
        pszCpuType = arrCPUTypes[ulCpuType];
    else
        pszCpuType = "(unknown)";

    printf("  CPU type        : %08xh (%s)\n",
           ulCpuType,
           pszCpuType);

    printf("  I/O Delay Count : %08xh %8d\n",
           ulIODelayCount,
           ulIODelayCount);

    // special new information
    if (!SupportsCPUID())
    {
        printf("  CPUID instruction not supported\n");
        return NO_ERROR;
    }
    else
    {
        char szVendor[32];
        ULONG ulCPUFeatures = GetCPUFeatures();
        ULONG ulCPUSignature = GetCPUSignature();

        printf("  CPUID instruction is supported\n");

        memset(szVendor, 0, sizeof( szVendor ) );
        GetCPUVendorString( szVendor );
        printf("  CPU vendor      : %s\n",
               szVendor);

        printf("  CPU features    : %08xh\n",
               ulCPUFeatures);

        // ...
#define CPUFEATURE(a,b)                             \
        printf("                  : %s%s\n",        \
               (ulCPUFeatures & a) ? "   " : "no ", b)

        CPUFEATURE(CPUID_FPU_PRESENT,                 "FPU    On-chip FPU");
        CPUFEATURE(CPUID_VMMODE_EXTENSIONS,           "VME    VMMode Extensions (virtual interrupts)");
        CPUFEATURE(CPUID_DBG_EXTENSIONS,              "DE     Debugging Extensions (I/O breakpoints)");
        CPUFEATURE(CPUID_PAGE_SIZE_EXTENSIONS,        "PSE    Page Size Extensions (4MB pages)");
        CPUFEATURE(CPUID_TIME_STAMP_COUNTER,          "TSC    Time Stamp Counter");
        CPUFEATURE(CPUID_K86_MODEL_REGS,              "MSR    K86 Model Registers");
        CPUFEATURE(CPUID_PHYSICAL_ADDRESS_EXTENSIONS, "PAE    Physical Address Extension (2MB pages)"),
        CPUFEATURE(CPUID_MACHINE_CHECK_EXCEPTION,     "MCE    Machine Check Exception");
        CPUFEATURE(CPUID_CMPXCHG8B_INSTRUCTION,       "CM8    8-byte compare exchange (cmpxchg8b)");
        CPUFEATURE(CPUID_APIC,                        "APIC   On-chip Advanced Programmable Interrupt Control");
        CPUFEATURE(CPUID_RESERVED_1,                  "---    (reserved bit 10)");
        CPUFEATURE(CPUID_SYSENTER_SYSEXIT,            "SEP    SYSENTER and SYSEXIT support");
        CPUFEATURE(CPUID_MEMORY_TYPE_RANGE_REGS,      "MTRR   Memory Type Range Registers");
        CPUFEATURE(CPUID_GLOBAL_PAGING_EXTENSIONS,    "PGE    Global Paging Extensions (shared TLB entries)");
        CPUFEATURE(CPUID_MACHINE_CHECK_ARCHITECTURE,  "MCA    Machine Check Architecture (error reporting)");
        CPUFEATURE(CPUID_CONDITIONAL_MOVE,            "CMOV   Conditional Move and compare");
        CPUFEATURE(CPUID_PAGE_ATTRIBUTE_TABLE,        "PAT    Page Attribute Table (MTRR improvement)");
        CPUFEATURE(CPUID_PAGE_SIZE_EXTENSION_2,       "PSE    Page Size Extension (2) (64GB memory, 36-bit bus)");
        CPUFEATURE(CPUID_PROCESSOR_SERIAL_NUMBER,     "PSN    96-bit Processor Serial Number support");
        CPUFEATURE(CPUID_CACHELINE_FLUSH,             "CFLSH  CLFLUSH instruction (cacheline flush)");
        CPUFEATURE(CPUID_RESERVED_2,                  "---    (reserved bit 20)");
        CPUFEATURE(CPUID_DEBUG_STORE,                 "DS     Debug Store");
        CPUFEATURE(CPUID_THERMAL_MONITOR_AND_CLOCK,   "ACPI   Thermal Monitor and Clock Control");
        CPUFEATURE(CPUID_MMX,                         "MMX    Intel MMX Multimedia Extensions");
        CPUFEATURE(CPUID_FXSR,                        "FXSR   FXSAVE and FXRESTOR instructions (FPU state)");
        CPUFEATURE(CPUID_SSE,                         "SSE    Streaming SIMD Extensions");
        CPUFEATURE(CPUID_SSE2,                        "SSE2   Streaming SIMD Extensions 2");
        CPUFEATURE(CPUID_SELF_SNOOP,                  "SS     Self Snoop (conflicting memory type mgmt)");
        CPUFEATURE(CPUID_HYPER_THREADING,             "HTT    Hyper-Threading Technology");
        CPUFEATURE(CPUID_THERMAL_MONITOR,             "TM     Thermal Monitor (automatic thermal control)");
        CPUFEATURE(CPUID_RESERVED_3,                  "---    (reserved bit 30)");
        CPUFEATURE(CPUID_PENDING_BREAK_ENABLE,        "PBE    Pending Break Enable (interrupt wakeup)");
      
      
        printf("  CPU signature   : %08xh\n",
               ulCPUSignature);
      
        // decode CPU signature
        {
          ULONG ulSteppingID =  ulCPUSignature & 0x0000000f;
          ULONG ulModel      = (ulCPUSignature & 0x000000f0) >> 4;
          ULONG ulFamily     = (ulCPUSignature & 0x00000f00) >> 8;
          ULONG ulType       = (ulCPUSignature & 0x00003000) >> 12;
          ULONG ulModelEx    = (ulCPUSignature & 0x000f0000) >> 16;
          ULONG ulFamilyEx   = (ulCPUSignature & 0x00ff0000) >> 20;
          
          PSZ arrpszType[4] =
          {
            "Original OEM Processor",
            "Intel Overdrive Processor",
            "Dual Processor",
            "(reserved)"
          };
          
          printf("             type : %s\n",
                 arrpszType[ ulType ]);
          printf("            model : %d (extended: %d)\n",
                 ulModel,
                 ulModelEx);
          printf("           family : %d (extended: %d)\n",
                 ulFamily,
                 ulFamilyEx);
          printf("      stepping ID : %d\n",
                 ulSteppingID);
        }
      
        if (ulCPUFeatures & CPUID_TIME_STAMP_COUNTER)
        {
            ULONG lTSCHi;
            ULONG lTSCLo;
            double dblCyclesPerSecond;
            double dblCPUCycles;
            double dblPowerOnSeconds;
            ULONG r_days;
            ULONG r_hours;
            ULONG r_minutes;
            ULONG r_seconds;
          
            GetTSC((LONG*)&lTSCHi, (LONG*)&lTSCLo);
            printf("  CPU timestamp   : %08x:%08xh\n",
                   lTSCHi,
                   lTSCLo);
          
            dblCyclesPerSecond = CPUDetermineHZ(ulCPUFeatures);
            dblCPUCycles = lTSCHi * 65536.0 * 65536.0 + lTSCLo;
            dblPowerOnSeconds = dblCPUCycles / dblCyclesPerSecond;
          
            // determine timestamp counter frequency
            printf("  CPU speed       : %d MHz\n",
                   (ULONG)(dblCyclesPerSecond / 1000000.0) );
          
            r_days    = (ULONG)(dblPowerOnSeconds / 86400);
            r_hours   = (ULONG)( (dblPowerOnSeconds / 3600)) % 24;
            r_minutes = (ULONG)( (dblPowerOnSeconds / 60)) % 60;
            r_seconds = (ULONG)(dblPowerOnSeconds) % 60;
          
            printf("  Power on time   : %03ud %02uh %02um %02us (no-idle mode eqvalent)\n",
                   r_days,
                   r_hours,
                   r_minutes,
                   r_seconds);
        }
    }
  
  
  // more specific CPU information
  {
    ULONG cpudata[12];
    ULONG ulCPUIDMax;
    ULONG ulCPUIDMaxExtended;
    
    // determine extended CPUID information
    memset(cpudata, 0, sizeof( cpudata ) );
    GetCPUGenericCPUID(cpudata, 0);
    ulCPUIDMax = cpudata[0];
    printf("\nMaximum Input value for CPUID information:\n"
           "  max          = %08xh\n",
           ulCPUIDMax);
    
    memset(cpudata, 0, sizeof( cpudata ) );
    GetCPUGenericCPUID(cpudata, 0x80000000);
    ulCPUIDMaxExtended = cpudata[0] & 0x7ffffff;
    printf("  max extended = %08xh\n",
           ulCPUIDMaxExtended);
    
    if (ulCPUIDMax >= 1)
    {
      PSZ arrpszBrand[] =
      {
        "not supported feature",
        "Celeron processor",
        "Pentium /// processor",
        "Intel Pentium /// Xeon processor",
        "(reseved 4)",
        "(reseved 5)",
        "(reseved 6)",
        "(reseved 7)",
        "Intel Pentium 4 processor"
      };
      UCHAR ucBrand;
      
      // obtain miscellaneous information
      memset(cpudata, 0, sizeof( cpudata ) );
      GetCPUGenericCPUID(cpudata, 1);
      ucBrand = cpudata[1] & 0x000000ff;
      printf("      brand index : %d (introduced Intel Pentium /// Xeon)\n",
             ucBrand);
      if (ucBrand < 8) // size of above string array
        printf("                  : %s\n",
               arrpszBrand[ ucBrand ]);
      
      printf("     CLFLUSH size : %d (introduced Intel Pentium 4, cacheline size)\n",
             (cpudata[1] & 0x0000ff00) >> 8);
      printf("     Logical CPUs : %d (per physical processor)\n",
             (cpudata[1] & 0x00ff0000) >> 16);
      printf("    Local APIC ID : %d (introduced Intel Pentium 4)\n",
             (cpudata[1] & 0xff000000) >> 24);
    }
    
    
    if (ulCPUIDMax >= 2)
    {
      // obtain cache and TLB information
      memset(cpudata, 0, sizeof( cpudata ) );
      GetCPUGenericCPUID(cpudata, 2);
      
      // @@@PH incomplete! see Intel spec.
      printf("\n\nCache and TLB information\n");
      printf("  %08xh %08xh %08xh %08xh\n",
             cpudata[0],
             cpudata[1],
             cpudata[2],
             cpudata[3]);
    }
    
    if (ulCPUIDMax >= 3)
    {
      // obtain processor serial number
      memset(cpudata, 0, sizeof( cpudata ) );
      GetCPUGenericCPUID(cpudata, 3);
      printf("\nProcessor serial number (Intel Pentium III only):\n"
             "  %08xh:%08xh\n",
             cpudata[3],
             cpudata[2]);
    }
    
    
    if (ulCPUIDMaxExtended >= 1)
    {
      // obtain extended processor signature and extended feature bits
      memset(cpudata, 0, sizeof( cpudata ) );
      GetCPUGenericCPUID(cpudata, 0x8000001);
      printf("\nExtended Processor Signature and Extended Feature Bits:\n"
             "  %08xh\n",
             cpudata[0]);
    }
    
    // Pentium 4 function
    if (ulCPUIDMaxExtended >= 4)
    {
      // obtain processor brand string
      memset(cpudata, 0, sizeof( cpudata ) );
      GetCPUGenericCPUID(cpudata,     0x8000002);
      GetCPUGenericCPUID(&cpudata[4], 0x8000003);
      GetCPUGenericCPUID(&cpudata[8], 0x8000004);
      
      // Should be something like
      //     EAX  EBX  ECX  EDX
      // 2: "    :    :    :  In"
      // 3: "tel(:R) P:enti:um(R"
      // 4: ") 4 :CPU :1500:MHz\0"
      
      printf("\nProcessor Brand String:\n"
             "  %s\n",
             cpudata);
    }
    
    // further info w/o spec
    if (ulCPUIDMaxExtended >= 6)
    {
      int i;
      
      printf("\nAdditional CPUID data:\n");
      for (i = 5;
           i <= ulCPUIDMaxExtended;
           i++)
      {
        // obtain processor brand string
        memset(cpudata, 0, sizeof( cpudata ) );
        GetCPUGenericCPUID(cpudata,     0x8000002);
        
        printf ("  %2d: %08xh:%08xh:%08xh:%08xh\n",
                i,
                cpudata[0],
                cpudata[1],
                cpudata[2],
                cpudata[3]);
      }
    }
  }
  
  return NO_ERROR;
}



double CPUDetermineHZ(ULONG features)
{
    if(features & CPUID_TIME_STAMP_COUNTER)
    {
        LONG tsc1_LowPart;
        LONG tsc1_HighPart;
        LONG tsc2_LowPart;
        LONG tsc2_HighPart;

        PERFSTRUCT time1;
        PERFSTRUCT time2;

        double clockticks, millisec, tmp, tmp1, mhertz, fCompensation;

        // determine compensation
        ToolsPerfQuery(&time1);
        GetTSC((LONG *)&tsc1_HighPart, &tsc1_LowPart);
        GetTSC((LONG *)&tsc2_HighPart, &tsc2_LowPart);
        ToolsPerfQuery(&time2);
        fCompensation = time2.fSeconds - time1.fSeconds;
        if (fCompensation < 0.0)
            fCompensation = 0.0;

        // real measurement
        ToolsPerfQuery(&time1);
        GetTSC((LONG *)&tsc1_HighPart, &tsc1_LowPart);
        DosSleep(32);  
        GetTSC((LONG *)&tsc2_HighPart, &tsc2_LowPart);
        ToolsPerfQuery(&time2);

        // now compare TSC-delta and time-telds
        millisec = time2.fSeconds - time1.fSeconds - fCompensation;

        tmp  = (double)tsc2_LowPart + (double)tsc2_HighPart*4.0*1024.0*1024.0;
        tmp1 = (double)tsc1_LowPart + (double)tsc1_HighPart*4.0*1024.0*1024.0;
        clockticks = tmp - tmp1;

        tmp = 1000 / millisec;
        clockticks = clockticks * tmp;	//ticks per second

        return clockticks / 1000.0;
    }
    else
        return 0.0;
}



/***********************************************************************
 * Name      : APIRET PerfKIDisable()
 * Funktion  : Disable the kernel performance hooks
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

APIRET PerfKIDisable(void)
{
  return(DosPerfSysCall(CMD_KI_DISABLE,
                        0,
                        0,
                        0));
}


/***********************************************************************
 * Name      : APIRET PerfKIEnable()
 * Funktion  : Enable the kernel performance hooks
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

APIRET PerfKIEnable(void)
{
  return(DosPerfSysCall(CMD_KI_ENABLE,
                        0,
                        0,
                        0));
}


/*****************************************************************************
 * Name      : APIRET PerfKIMonitor()
 * Funktion  : Monitor the CPU Load
 * Parameter : ULONG ulScanTime - milliseconds between iteration
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 *****************************************************************************/

APIRET PerfKIMonitor(ULONG ulScanTime)
{
  APIRET      rc;                                          /* API returncode */
  PCPUUTIL    pCPUUtil;            /* pointer to array of CPUUTIL structures */
  PCPUPERF    pCPUPerf;            /* pointer to array of CPUPERF structures */
  ULONG       ulCPUs;                        /* number of CPUs in the system */
  ULONG       ulTemp;                         /* temporary iterator variable */
  BOOL        fFirstIteration=TRUE;          /* indicates validity of values */
  PCPUUTIL    pCPUUtilCurrent;              /* pointer to current array item */
  PCPUPERF    pCPUPerfCurrent;              /* pointer to current array item */
  FLOATTYPE   ftDummy;           /* temporary register used for calculations */

#define QSV_NUMPROCESSORS       26

  rc = DosQuerySysInfo (QSV_NUMPROCESSORS,
                        QSV_NUMPROCESSORS,            /* 26 = Num processors */
                        &ulCPUs,
                        sizeof(ulCPUs));
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                   /* either not supported or kernel too old */

  printf ("\n%u CPUs reported by kernel.",                  /* print message */
          ulCPUs);

  pCPUUtil = malloc(ulCPUs * sizeof(CPUUTIL) );   /* allocate buffer for CPUs*/
  if (pCPUUtil == NULL)                           /* check proper allocation */
    return (ERROR_NOT_ENOUGH_MEMORY);                   /* else report error */

  pCPUPerf = malloc(ulCPUs * sizeof(CPUPERF) );   /* allocate buffer for CPUs*/
  if (pCPUPerf == NULL)                           /* check proper allocation */
  {
    free(pCPUUtil);                      /* free previously allocated memory */
    return (ERROR_NOT_ENOUGH_MEMORY);                   /* else report error */
  }


  do
  {
    rc = DosPerfSysCall(CMD_KI_RDCNT,               /* read the kernel times */
                        (ULONG)pCPUUtil,
                        0,
                        0);
    if (rc != NO_ERROR)                                  /* check for errors */
    {
      free(pCPUUtil);                    /* free previously allocated memory */
      free(pCPUPerf);                    /* free previously allocated memory */
      return (rc);                                  /* raise error condition */
    }

    DosSleep(ulScanTime);                                   /* wait the time */

    /* calculate the timings */
    for (ulTemp = 0;
         ulTemp < ulCPUs;
         ulTemp++)
    {
      pCPUUtilCurrent = &pCPUUtil[ulTemp];
      pCPUPerfCurrent = &pCPUPerf[ulTemp];

      pCPUPerfCurrent->ts_val   = LL2F(pCPUUtilCurrent->ulTimeHigh,
                                       pCPUUtilCurrent->ulTimeLow);
      pCPUPerfCurrent->idle_val = LL2F(pCPUUtilCurrent->ulIdleHigh,
                                       pCPUUtilCurrent->ulIdleLow);
      pCPUPerfCurrent->busy_val = LL2F(pCPUUtilCurrent->ulBusyHigh,
                                       pCPUUtilCurrent->ulBusyLow);
      pCPUPerfCurrent->intr_val = LL2F(pCPUUtilCurrent->ulIntrHigh,
                                       pCPUUtilCurrent->ulIntrLow);


      if (fFirstIteration == FALSE)
      {
        FLOATTYPE  ts_delta = pCPUPerfCurrent->ts_val -
                              pCPUPerfCurrent->ts_val_prev;

        pCPUPerfCurrent->idle_val_perc = (pCPUPerfCurrent->idle_val -
                                          pCPUPerfCurrent->idle_val_prev)
                                           / ts_delta * 100.0;

        pCPUPerfCurrent->busy_val_perc = (pCPUPerfCurrent->busy_val -
                                          pCPUPerfCurrent->busy_val_prev)
                                           / ts_delta * 100.0;

        pCPUPerfCurrent->intr_val_perc = (pCPUPerfCurrent->intr_val -
                                          pCPUPerfCurrent->intr_val_prev)
                                           / ts_delta * 100.0;

        printf("\nCPU_%u: idle %6.2f%%  busy %6.2f%%  intr %6.2f%%",
               ulTemp,
               pCPUPerfCurrent->idle_val_perc,
               pCPUPerfCurrent->busy_val_perc,
               pCPUPerfCurrent->intr_val_perc);

        /* calculate the other statistics */
        pCPUPerfCurrent->ulIterations++;

  #define CPUPERFMAX(a,b) if (pCPUPerfCurrent->a > pCPUPerfCurrent->b) \
                               pCPUPerfCurrent->b = pCPUPerfCurrent->a; \

  #define CPUPERFMIN(a,b) if (pCPUPerfCurrent->a < pCPUPerfCurrent->b) \
                               pCPUPerfCurrent->b = pCPUPerfCurrent->a;

  #define CPUPERFSUM(a,b) pCPUPerfCurrent->b += pCPUPerfCurrent->a;

        CPUPERFMAX(idle_val_perc,idle_val_max)
        CPUPERFMAX(busy_val_perc,busy_val_max)
        CPUPERFMAX(intr_val_perc,intr_val_max)

        CPUPERFMIN(idle_val_perc,idle_val_min)
        CPUPERFMIN(busy_val_perc,busy_val_min)
        CPUPERFMIN(intr_val_perc,intr_val_min)

        CPUPERFSUM(idle_val_perc,idle_val_sum)
        CPUPERFSUM(busy_val_perc,busy_val_sum)
        CPUPERFSUM(intr_val_perc,intr_val_sum)

        /* shift back the values */
        pCPUPerfCurrent->ts_val_prev   = pCPUPerfCurrent->ts_val;
        pCPUPerfCurrent->idle_val_prev = pCPUPerfCurrent->idle_val;
        pCPUPerfCurrent->busy_val_prev = pCPUPerfCurrent->busy_val;
        pCPUPerfCurrent->intr_val_prev = pCPUPerfCurrent->intr_val;
      }
    }

    if (fFirstIteration == TRUE)
    {
      fFirstIteration = FALSE;

      pCPUPerfCurrent->busy_val_min=100.0;       /* initialize the structure */
      pCPUPerfCurrent->intr_val_min=100.0;
      pCPUPerfCurrent->idle_val_min=100.0;
      pCPUPerfCurrent->busy_val_max=  0.0;
      pCPUPerfCurrent->intr_val_max=  0.0;
      pCPUPerfCurrent->idle_val_max=  0.0;
      pCPUPerfCurrent->busy_val_sum=  0.0;
      pCPUPerfCurrent->intr_val_sum=  0.0;
      pCPUPerfCurrent->idle_val_sum=  0.0;
      pCPUPerfCurrent->ulIterations=  0;
    }

  }
  while(!kbhit());                                       /* until keypressed */


                                               /* now print the final report */
  if (pCPUPerfCurrent->ulIterations != 0)                   /* prevent DIV 0 */
  {
    printf ("\nÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿"
            "\n³  Summary after %9u iterations                                      ³"
            "\nÃÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´"
            "\n³ CPU ³Max                   ³Min                   ³Average               ³"
            "\n³ Nr. ³Idle    Busy    Intr  ³Idle    Busy    Intr  ³Idle    Busy    Intr  ³"
            "\nÆÍÍÍÍÍØÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍØÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍØÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍµ",
            pCPUPerf[0].ulIterations);

    for (ulTemp = 0;
         ulTemp < ulCPUs;
         ulTemp++)
    {
      pCPUPerfCurrent = &pCPUPerf[ulTemp];

      printf ("\n³ %3u ³ %6.2f %6.2f %6.2f",
              ulTemp,
              pCPUPerfCurrent->idle_val_max,
              pCPUPerfCurrent->busy_val_max,
              pCPUPerfCurrent->intr_val_max);

      printf (" ³ %6.2f %6.2f %6.2f ³ ",
              pCPUPerfCurrent->idle_val_min,
              pCPUPerfCurrent->busy_val_min,
              pCPUPerfCurrent->intr_val_min);

      printf ("%6.2f %6.2f %6.2f ³",
              pCPUPerfCurrent->idle_val_sum / (FLOATTYPE)pCPUPerfCurrent->ulIterations,
              pCPUPerfCurrent->busy_val_sum / (FLOATTYPE)pCPUPerfCurrent->ulIterations,
              pCPUPerfCurrent->intr_val_sum / (FLOATTYPE)pCPUPerfCurrent->ulIterations);
    }

    printf ("\nÀÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ");
  }

  return (NO_ERROR);                                                   /* OK */
}


/*
 * CPU-Tester
 * (c) 1998 Patrick Haller
 *
 * 2 different methods:
 *
 * a) - create a memory page with EXECUTE attribute set
 *    - generate a loop of instructions with known cycles
 *    - ensure empty loop is in L1/L2 cache
 *    - execute empty loop and measure timer overhead
 *    - ensure filled loop is in L1/L2 cache
 *    - execute filled loop, measure time and calculate netto execution time
 *
 * b) - create high priority timer thread and wait for semaphore
 *    - create lower priority measure thread and wait for same semaphore
 *    - post semaphore
 *    - timer thread will do a DosSleep() and kill measurement thread
 *      afterwards
 *    - measurement thread increases a counter within a loop,
 *      this counter variable MUST not be assigned to a CPU register
 *    - compare total execution time and counter value to calculate
 *      CPU speed.
 *
 * this program implements type a).
 */


/* Allocate EXECUTABLE page and create loop of MOV reg,reg instructions */
/* which will take 2 cycles on 386 and 1 cycle on 486 and later.        */
/* The instruction cannot be parallelized within the pipelines and no   */
/* bus access will occur so we're able to determine raw CPU frequency.  */


/***********************************************************************
 * Name      : APIRET PerfKIEnable()
 * Funktion  : Enable the kernel performance hooks
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

void GenerateExecutionLoop(ULONG  ulInstructions,
                           PUCHAR pucPage)
{
  ULONG ulCounter;

  for (ulCounter = 0;
       ulCounter < ulInstructions;
       ulCounter++)
  {
    /* @@@PH AMD K5,K6 and PII can pipeline this instruction */

    *pucPage++ = 0x89; /* MOV   */
    *pucPage++ = 0xD8; /* AX,BX */
  }

  *pucPage = 0xC3;     /* RET   */
}


/***********************************************************************
 * Name      : APIRET PerfKIEnable()
 * Funktion  : Enable the kernel performance hooks
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

APIRET AllocateExecutionLoop(PPVOID ppBase,
                             ULONG  ulSize)
{
  return(DosAllocMem(ppBase,
                     ulSize * 2 + 1,           /* calculate instruction size */
                     PAG_WRITE  |
                     PAG_READ   |
                     PAG_COMMIT |
                     PAG_EXECUTE));
}


/***********************************************************************
 * Name      : APIRET PerfKIEnable()
 * Funktion  : Enable the kernel performance hooks
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

typedef void (*PFNTEST)(void);

#define LOOP_SIZE  1000                           /* DON'T EXCEED L1 CACHE ! */
#define LOOP_COUNT 100000
#define LOOP_EMPTY 20000

int CPUTest1(void)
{
  PVOID   pExecution;                   /* pointer to execution loop page(s) */
  APIRET  rc;                                             /* API return code */
  PFNTEST pfnExecution;                     /* pointer to execution function */
  QWORD   qwStart;
  QWORD   qwEnd;
  int     i;
  float   fOverhead = 0.0;
  float   fTotal    = 0.0;
  ULONG   ulTimerFrequency;                /* builtin 64-bit timer frequency */
  float   fCPUFrequency;                         /* calculated CPU frequency */


  printf ("Testing CPU frequency ...\n\n"
          "(please note modern processors like AMD K5, K6 or Intel Pentium II\n"
          " do pipeline the used MOV AX,BX instruction. Therefore, the result\n"
          " of this test might be the actual frequency multiplied by the level\n"
          " of pipelining. For CPUs with exceptionally small L1 caches, the\n"
          " measurement loop might exceed it and yield significally lower results.)\n\n");

  rc = AllocateExecutionLoop(&pExecution,
                             LOOP_SIZE);
  if (rc != NO_ERROR)                                    /* check for errors */
  {
    fprintf (stderr,
             "ERROR: SYS%04x during memory allocation.\n",
             rc);
    ToolsErrorDos(rc);
    return(rc);                                                 /* exit here */
  }


  GenerateExecutionLoop(0,
                        (PUCHAR)pExecution);          /* generate empty loop */


  pfnExecution = (PFNTEST)pExecution;    /* obtain execution pointer to loop */

  pfnExecution();                               /* get it into the CPU cache */


  DosSetPriority(PRTYS_PROCESS,              /* boost our process to the max */
                 PRTYC_TIMECRITICAL,
                 PRTYD_MAXIMUM,
                 0);

  /******************************
   * now measure timer overhead *
   ******************************/

  DosTmrQueryTime(&qwStart);

  for (i = 0;
       i < LOOP_EMPTY;
       i++)
    pfnExecution();

  DosTmrQueryTime(&qwEnd);

  fOverhead += (qwEnd.ulLo - qwStart.ulLo);            /* add timer cycles */

  printf ("Timer overhead is %10.5f timer cycles in total.\n",
          fOverhead);


  /******************************
   * now measure execution loop *
   ******************************/

  GenerateExecutionLoop(LOOP_SIZE,
                        (PUCHAR)pExecution);          /* generate empty loop */


  pfnExecution = (PFNTEST)pExecution;    /* obtain execution pointer to loop */

  pfnExecution();                               /* get it into the CPU cache */


  DosTmrQueryTime(&qwStart);
  for (i = 0;
       i < LOOP_COUNT;
       i++)
  {
    pfnExecution();
  }

  DosTmrQueryTime(&qwEnd);
  fTotal += (qwEnd.ulLo - qwStart.ulLo);                 /* add timer cycles */
  fTotal -= fOverhead;
  fTotal /= LOOP_COUNT;

  /* We don't measure the time the operating system spends on                */
  /* other tasks and interrupt processing. Just too difficult to do this     */
  /* unless Pentium Machine State Registers and such are used.               */

  printf ("Execution time is %10.5f timer cycles per loop.\n",
          fTotal);


                                           /* now we calculate CPU frequency */
  DosTmrQueryFreq(&ulTimerFrequency);                 /* get timer frequency */

  fCPUFrequency = (float)ulTimerFrequency
                  * (float)LOOP_SIZE
                  / fTotal;

                  /* well, on i386 systems MOV AX,BX would take 2 cycles ... */
  printf ("Therefore, calculated CPU frequency is %10.3f MHz.",
          fCPUFrequency / 1000000);

  DosFreeMem(pExecution);


  return NO_ERROR;
}


/*****************************************************************************
 * Name      : APIRET CPUControl
 * Funktion  : Central dispatcher routine
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 *****************************************************************************/

APIRET CPUControl(void)
{
  APIRET rc;                                               /* API returncode */

  if (Options.fsCmdInfo)                          /* display CPU information */
      displayCPUInfo();

  if (Options.fsCPUTest1)
  {
    return (CPUTest1());                                 /* do the test only */
  }

  if (Options.fsCmdOn)                                    /* process command */
  {
    printf ("\nSwitching on kernel performance hooks.");          /* message */
    rc = PerfKIEnable();
    if (rc != NO_ERROR)
      ToolsErrorDosEx(rc,                             /* yield error message */
                      "PerfKIEnable");
  }

  if (Options.fsCmdOff)                                   /* process command */
  {
    printf ("\nSwitching off kernel performance hooks.");         /* message */
    rc = PerfKIDisable();
    if (rc != NO_ERROR)
      ToolsErrorDosEx(rc,                             /* yield error message */
                      "PerfKIDisable");
  }


  if (Options.fsCmdMonitor)                               /* process command */
  {
    printf ("\nMonitoring CPU load...");                          /* message */
    rc = PerfKIMonitor(Options.ulScanTime);
    if (rc != NO_ERROR)
      ToolsErrorDosEx(rc,                             /* yield error message */
                      "PerfKIMonitor");
  }


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

  Options.ulScanTime = 1000;                              /* 1000ms per scan */
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
  int rc;                                                    /* Rckgabewert */

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

  if (Options.fsHelp)                                              /* help ? */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  if (!Options.fsCmdOn  &&                 /* perform some parameter mapping */
      !Options.fsCmdOff &&
      !Options.fsCmdInfo &&
      !Options.fsCmdMonitor)
    Options.fsCmdMonitor = TRUE;           /* then switch this on as default */


  rc = CPUControl();                                           /* Los geht's */
  return (rc);
}
