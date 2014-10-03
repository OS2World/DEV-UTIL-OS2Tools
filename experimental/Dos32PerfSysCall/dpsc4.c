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
   extern "\nC" {
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


/***********************************************************************
 * Name      : APIRET ToolsDumpHex
 * Funktion  : Anzeigen der gefundenen Detailinformation
 * Parameter :
 * Variablen :
 * Ergebnis  : Ausgabe auf dem Bildschirm
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.49.57]
 ***********************************************************************/

APIRET        ToolsDumpHex (ULONG ulPosition,
                            ULONG ulLength,
                            PVOID pSource)
{
  ULONG ulCounter;                                 /* temporary loop counter */
  CHAR  szBuffer[128];            /* local, temporary buffer for debug lines */
  CHAR  szBuffer2[80];            /* local, temporary buffer for debug lines */
  CHAR  szBuffer3[80];            /* local, temporary buffer for debug lines */
  CHAR  szBuffer4[20];            /* local, temporary buffer for debug lines */
  UCHAR ucData;                                                 /* data byte */
  PCHAR pchTemp;                                /* pointer to that data byte */
  ULONG ulTotalLength;           /* total length of printed buffer + padding */
  
  sprintf(szBuffer, "%08x³", ulPosition);

  szBuffer2[0] = 0;
  szBuffer3[0] = 0;
  
  ulTotalLength = (ulLength + 0x0000000f) & 0xfffffff0;
  
  for (ulCounter = 0;
       ulCounter < ulTotalLength;                           /* round up to 8 */
       ulCounter++)
  {
    if (ulCounter < ulLength)
    {
      pchTemp = (PCHAR)pSource + ulCounter;
      ucData = *pchTemp;
      sprintf(szBuffer4, "%02x ", ucData);
      strcat (szBuffer2, szBuffer4);                        /* add hex token */

      if (ucData < ' ')
        strcat (szBuffer3,"ú");                     /* unprintable character */
      else
      {
        szBuffer4[0] = ucData;
        szBuffer4[1] = 0;
        strcat(szBuffer3, szBuffer4);
      }
    }
    else
    {
      strcat(szBuffer2,"úú ");                    /* insert dummy characters */
      strcat(szBuffer3," ");
    }

    if (ulCounter % 0x00000010 == 0x0000000f)         /* check for linebreak */
    {
      strcat(szBuffer, szBuffer2);
      strcat(szBuffer,"³");
      strcat(szBuffer, szBuffer3);

      printf ("\n%s",szBuffer);

      szBuffer2[0] = 0;
      szBuffer3[0] = 0;
      sprintf(szBuffer, "%08x³", ulCounter+1+ulPosition);
    }
  }
  
  return (NO_ERROR);                                                   /* OK */
}

 
 /* This is a 1 processor example */
 
void main (int argc, char *argv[])
{
  APIRET      rc;
  PSZ    szBuffer;
  ULONG  ulSize;
  ULONG  ulCPUs;
  ULONG  ulTest;
  PSZ    ppPointers[1025];
  
  memset(ppPointers,
         0,
         sizeof(ppPointers));
  
  /* pPDA is system wide ! */
  rc = DosPerfSysCall(CMD_FREEPERFBUFFER,
                      0,
                      0,
                      0);
  
  rc = DosPerfSysCall(CMD_INITPERFBUFFER,
                      (ULONG)&ppPointers,
                      sizeof(ppPointers),
                      0);
  if (rc) 
      fprintf(stderr, "\nCMD_INITPERFBUFFER failed rc = %d\n",rc);
  
  rc = DosPerfSysCall(CMD_INFOPERFBUFFER,
                      (ULONG)&ulSize,
                      (ULONG)&ulCPUs,
                      0);
  printf ("\nSize=%u (%08xh), CPUs=%u, ppPointers=%08xh", 
          ulSize, 
          ulSize,
          ulCPUs,
          ppPointers);
  if (rc) 
      fprintf(stderr, "\nCMD_INFOPERFBUFFER failed rc = %d\n",rc);
  
  rc = DosPerfSysCall(CMD_GETPERFBUFFERHEADERS,
                      (ULONG)&ppPointers,
                      sizeof(ppPointers),
                      0);
  if (rc) 
      fprintf(stderr, "\nCMD_GETPERFBUFFERHEADERS failed rc = %d\n",rc);
printf ("\nGetPerfBufferHeaders");
  
  ToolsDumpHex(0,
               sizeof(ppPointers),
               ppPointers);
  
  
  for (ulTest = 0;
       ulTest < 12;
       ulTest++)
    printf ("\nPtr[%2u] = %08xh",
            ulTest,
            ppPointers[ulTest]);

  rc = DosPerfSysCall(CMD_GETPERFBUFFER,
                      0,
                      (ULONG)&ppPointers,
                      sizeof(ppPointers));
  if (rc) 
      fprintf(stderr, "\nCMD_GETPERFBUFFER failed rc = %d\n",rc);
  
printf ("\nGetPerfBuffer");
  ToolsDumpHex(0,
               sizeof(ppPointers),
               ppPointers);
  
  
  rc = DosPerfSysCall(CMD_FREEPERFBUFFER,
                      0,
                      0,
                      0);
  if (rc) 
      fprintf(stderr, "\nCMD_FREEPERFBUFFER failed rc = %d\n",rc);
}
