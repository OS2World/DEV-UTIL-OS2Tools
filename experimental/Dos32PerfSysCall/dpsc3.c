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
     
     
     
     /* PH */
#define CMD_GETPERFBUFFER 0x40
#define CMD_INFOPERFBUFFER 0x41
#define CMD_INITPERFBUFFER 0x42
#define CMD_FREEPERFBUFFER 0x43
#define CMD_GETPERFBUFFERHEADERS 0x52
#define CMD_KI_ENABLE 0x60
#define CMD_KI_DISABLE 0x61
     
     
     /* getperfbufferheaders: pro CPU 512 Bytes puffer */
     
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

  rc = DosPerfSysCall(CMD_KI_DISABLE,0,0,0);
  if (rc) 
  {
      fprintf(stderr, "CMD_KI_DISABLE failed rc = %d\n",rc);
      exit(1);
  }
  
  DosSleep(10000);
  
  rc = DosPerfSysCall(CMD_KI_ENABLE,0,0,0);
  if (rc) 
  {
      fprintf(stderr, "CMD_KI_ENABLE failed rc = %d\n",rc);
      exit(1);
  }
  
}
