/* 
 This example uses DosPerfSysCall to obtain CPU Utilization information on a 
 uniprocessor. 
*/

 #define INCL_BASE
 
 #include <os2.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>

 /* parameters are:
   PID, call level, BUFFER, BUFFER SIZE at best guess */

APIRET _System Dos32QueryPageUsage(ULONG  ulSubFunc,
                           PVOID  pvInfoBuffer,
                           PULONG pulBufferLength);


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


#define QPU_REGAPP      0
#define QPU_GETAPP      1
#define QPU_REGSYS      2
#define QPU_GETSYS      3
#define QPU_DEREG       4
#define QPU_GETGSR      5
#define QPU_GETHIGH     6


CHAR   szBuffer[32768];
CHAR   szData[32768];
 
int main (void)
{
  APIRET rc;
  ULONG  ulTest;
  
  //printf("\nQPU_DEREG");
  //rc = Dos32QueryPageUsage(QPU_DEREG, 0, 0);
  //printf ("rc = %u\n",rc);
  
  
  memset(szBuffer,
         0,
         sizeof(szBuffer));
  
  /* set application name */
  strcpy (szBuffer,
          "D:\\OS2\\CMD.EXE");
  
  /* setup data region */
  memset (szData, 0, sizeof(szData));
  *( (PULONG)szData) = sizeof(szData);
  
  printf ("\nQPU_REGAPP: ");
  rc = Dos32QueryPageUsage(QPU_REGAPP, szBuffer, szData);
  printf ("rc = %u\n",rc);

  ToolsDumpHex(0, sizeof(szBuffer), szBuffer);
  
  
  printf ("\nQPU_GETAPP: ");
  *( (PULONG)szData) = sizeof(szData);
  rc = Dos32QueryPageUsage(QPU_GETAPP, szBuffer, szData);
  printf ("rc = %u\n",rc);

  ToolsDumpHex(0, sizeof(szData), szData);
  
  printf("\nQPU_DEREG");
  rc = Dos32QueryPageUsage(QPU_DEREG, 0, 0);
  printf ("rc = %u\n",rc);
  
}