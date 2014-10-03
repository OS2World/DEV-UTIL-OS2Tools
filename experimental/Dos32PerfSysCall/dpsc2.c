/* 
 This example performs software tracing from a program in ring 3. 
*/
 
 #define INCL_BASE
 
 #include <os2.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <perfutil.h>
 
 int main ( int argc, char *argv[] )
 {
     APIRET    rc;
     BYTE      HookBuffer[256];
     HOOKDATA  HookData = {0,HookBuffer};
     ULONG     ulMajor, ulMinor;
 
     /* Log 3 ULONG values (1, 2, and 3) and a string.   */
 
     *((PULONG)&HookBuffer[0]) = 1;
     *((PULONG)&HookBuffer[4]) = 2;
     *((PULONG)&HookBuffer[8]) = 3;
     strcpy((PSZ)&HookBuffer[12], "Test of 3 ULONG values and a string.")
     HookData.ulLength = 12 + strlen((PSZ)&HookBuffer[12]) + 1;
 
     ulMajor = 0x00b8;        /* Hook major code  */
     ulMinor = 0x0001;        /* Hook minor code  */
 
     rc = DosPerfSysCall( CMD_SOFTTRACE_LOG,
                          ulMajor, ulMinor,
                          (ULONG) &HookData);
     if (rc != NO_ERROR) {
        fprintf(stderr, "CMD_SOFTTRACE_LOG failed:  rc = %u\n", rc);
        return 1;
        }
 
     return NO_ERROR;
 }
 

