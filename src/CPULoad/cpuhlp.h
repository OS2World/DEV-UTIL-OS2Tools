/* $Id: cpuhlp.h,v 1.2 2002/08/08 13:02:57 phaller Exp $ */

#ifndef _CPUHLP_H_
#define _CPUHLP_H_

#include <float.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BIT(a)	(1<<a)

#define CPUID_FPU_PRESENT		  BIT(0)  // FPU
#define CPUID_VMMODE_EXTENSIONS		  BIT(1)  // VME
#define CPUID_DBG_EXTENSIONS		  BIT(2)  // DE
#define CPUID_PAGE_SIZE_EXTENSIONS	  BIT(3)  // PSE
#define CPUID_TIME_STAMP_COUNTER	  BIT(4)  // TSC
#define CPUID_K86_MODEL_REGS		  BIT(5)  // MSR
#define CPUID_PHYSICAL_ADDRESS_EXTENSIONS BIT(6)  // PAE
#define CPUID_MACHINE_CHECK_EXCEPTION	  BIT(7)  // MCE
#define CPUID_CMPXCHG8B_INSTRUCTION	  BIT(8)  // CX8
#define CPUID_APIC			  BIT(9)  // APIC
#define CPUID_RESERVED_1                  BIT(10) //
#define CPUID_SYSENTER_SYSEXIT            BIT(11) // SEP
#define CPUID_MEMORY_TYPE_RANGE_REGS	  BIT(12) // MTRR
#define CPUID_GLOBAL_PAGING_EXTENSIONS	  BIT(13) // PGE
#define CPUID_MACHINE_CHECK_ARCHITECTURE  BIT(14) // MCA
#define CPUID_CONDITIONAL_MOVE		  BIT(15) // CMOV
#define CPUID_PAGE_ATTRIBUTE_TABLE        BIT(16) // PAT
#define CPUID_PAGE_SIZE_EXTENSION_2       BIT(17) // PSE
#define CPUID_PROCESSOR_SERIAL_NUMBER     BIT(18) // PSN
#define CPUID_CACHELINE_FLUSH         	  BIT(19) // CFLSH
#define CPUID_RESERVED_2                  BIT(20) //
#define CPUID_DEBUG_STORE                 BIT(21) // DS
#define CPUID_THERMAL_MONITOR_AND_CLOCK   BIT(22) // ACPI
#define CPUID_MMX			  BIT(23) // MMX
#define CPUID_FXSR                        BIT(24) // FXSR
#define CPUID_SSE                         BIT(25) // SS
#define CPUID_SSE2                        BIT(26) // SSE2
#define CPUID_SELF_SNOOP                  BIT(27) // SS
#define CPUID_HYPER_THREADING             BIT(28) // HTT
#define CPUID_THERMAL_MONITOR             BIT(29) // TM
#define CPUID_RESERVED_3                  BIT(30) //
#define CPUID_PENDING_BREAK_ENABLE        BIT(31) // PBE



BOOL  CDECL SupportsCPUID();

void  CDECL GetCPUVendorString(char *vendor);
ULONG CDECL GetCPUFeatures();
ULONG CDECL GetCPUSignature();
void  CDECL GetTSC(LONG *high, LONG *low);
void  CDECL GetCPUGenericCPUID(unsigned long* data, unsigned long ulFunction);


#if (__IBMC__ >= 360) || (__IBMCPP__ >= 360)
#define CONTROL87(a,b)  __control87(a, b)
#else
#if (__IBMCPP__ == 300) || (__IBMC__ == 300)
#define CONTROL87(a,b)  _control87(a, b)
#else
#ifdef __WATCOMC__
#define CONTROL87(a,b)  _control87(a, b)
#else
#error  CONTROL87 undefined
#endif
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* _CPUHLP_H_ */

