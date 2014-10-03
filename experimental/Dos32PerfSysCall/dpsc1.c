/* 
 This example uses DosPerfSysCall to obtain CPU Utilization information on a 
 uniprocessor. 
*/

 #define INCL_BASE
 
 #include <os2.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>

/* #include <perfutil.h>*/

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

  typedef struct _CPUUTIL {
    ULONG ulTimeLow;     /* Low 32 bits of time stamp      */
    ULONG ulTimeHigh;    /* High 32 bits of time stamp     */
    ULONG ulIdleLow;     /* Low 32 bits of idle time       */
    ULONG ulIdleHigh;    /* High 32 bits of idle time      */
    ULONG ulBusyLow;     /* Low 32 bits of busy time       */
    ULONG ulBusyHigh;    /* High 32 bits of busy time      */
    ULONG ulIntrLow;     /* Low 32 bits of interrupt time  */
    ULONG ulIntrHigh;    /* High 32 bits of interrupt time */
   } CPUUTIL;

  typedef CPUUTIL *PCPUUTIL;
#endif

 #ifdef __cplusplus
   }
 #endif

/* It may also be necessary to define the ordinal for this function: */

 #define ORD_DOS32PERFSYSCALL            976





 /*
    Convert 8-byte (low, high) time value to double
 */
 #define LL2F(high, low) (4294967296.0*(high)+(low))
 
 
 /* This is a 1 processor example */
 
 void main (int argc, char *argv[])
 {
     APIRET      rc;
     int         i, iter, sleep_sec;
     double      ts_val, ts_val_prev;
     double      idle_val, idle_val_prev;
     double      busy_val, busy_val_prev;
     double      intr_val, intr_val_prev;
     CPUUTIL     CPUUtil;
 
     if ((argc < 2) || (*argv[1] < '1') || (*argv[1] > '9')) {
         fprintf(stderr, "usage: %s [1-9]\n", argv[0]);
         exit(0);
     }
     sleep_sec = *argv[1] - '0';
 
     iter = 0;
     do {
         rc = DosPerfSysCall(CMD_KI_RDCNT,(ULONG) &CPUUtil,0,0);
         if (rc) {
             fprintf(stderr, "CMD_KI_RDCNT failed rc = %d\n",rc);
             exit(1);
         }
         ts_val = LL2F(CPUUtil.ulTimeHigh, CPUUtil.ulTimeLow);
         idle_val = LL2F(CPUUtil.ulIdleHigh, CPUUtil.ulIdleLow);
         busy_val = LL2F(CPUUtil.ulBusyHigh, CPUUtil.ulBusyLow);
         intr_val = LL2F(CPUUtil.ulIntrHigh, CPUUtil.ulIntrLow);
 
         if (iter > 0) {
             double  ts_delta = ts_val - ts_val_prev;
           printf("idle: %4.2f%%  busy: %4.2f%%  intr: %4.2f%%\n",
                    (idle_val - idle_val_prev)/ts_delta*100.0,
                    (busy_val - busy_val_prev)/ts_delta*100.0,
                    (intr_val - intr_val_prev)/ts_delta*100.0);
         }
 
         ts_val_prev = ts_val;
         idle_val_prev = idle_val;
         busy_val_prev = busy_val;
         intr_val_prev = intr_val;
 
         iter++;
         DosSleep(1000*sleep_sec);
 
     } while (1);
 }
