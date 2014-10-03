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

APIRET Dos32QueryMemState(ULONG ulParam1,
                          ULONG ulParam2,
                          ULONG ulParam3,
                          ULONG ulParam4);


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


int main (void)
{
  APIRET rc;
  CHAR   szBuffer[64];
  ULONG  ulTest;
  
  memset(szBuffer,
         0,
         sizeof(szBuffer));
  
  for (ulTest = 0;
       ulTest < 100;
       ulTest++)
  {
    rc = Dos32QueryMemState(ulTest,
                            (ULONG)szBuffer,
                            (ULONG)szBuffer,
                            sizeof(szBuffer));
    printf ("\nparam1=%u rc=%u",
            ulTest,
            rc);
    getch();
  }
  
    ToolsDumpHex(0,
                 sizeof(szBuffer),
                 szBuffer);
}