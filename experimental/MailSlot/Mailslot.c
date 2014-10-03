/* Mailslot test
   (c) 1997 Patrick Haller
 */

#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>

#define _System __syscall

#define PURE_32
#include <netcons.h>
#include <mailslot.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>


typedef struct
{
  HFILE hMailslot; /* Mailslot handle */
  ULONG ulMessageSize; /* Message size */
  ULONG ulMailslotSize; /* Mailslot size */
  ULONG ulNextSize;
  ULONG ulNextPriority;
  ULONG ulMessages; /* number of messages in mailslot */
} MAILSLOT, *PMAILSLOT;


/***********************************************************************
 * Name      : void HexDump
 * Funktion  : Anzeigen der gefundenen Detailinformation
 * Parameter :
 * Variablen :
 * Ergebnis  : Ausgabe auf dem Bildschirm
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.49.57]
 ***********************************************************************/

void HexDump (ULONG ulLength,
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
  
  strcpy (szBuffer,"0000³");                                 /* clear buffer */
  szBuffer2[0] = 0;
  szBuffer3[0] = 0;
  
  ulTotalLength = (ulLength + 0x00000010) & 0xfffffff0;
  
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
      strcat(szBuffer,"³ ");
      strcat(szBuffer, szBuffer3);

      printf ("\n      %s",szBuffer);

      szBuffer2[0] = 0;
      szBuffer3[0] = 0;
      sprintf(szBuffer, "%04x³", ulCounter+1);
    }
  }
}




int main (void)
{
  APIRET   rc;
  MAILSLOT Mailslot;
  PVOID    pBuffer;
  ULONG    ulBufferSize;
  ULONG    ulBytesRead;
  
  
  printf ("\nStart");
  
  
  ulBufferSize = 65536;
  pBuffer = malloc(ulBufferSize);
  
  
  rc = Dos32MakeMailslot("\\MAILSLOT\\BROWSE",
                         4096,
                         32768,
                         &Mailslot.hMailslot);
  
  printf ("\nDosMakeMailslot: hMailslot=%u, rc=%u:",
          Mailslot.hMailslot,
          rc);
  
  getch();
  
  /* --- Dos32MailslotInfo --- */
  rc = Dos32MailslotInfo (Mailslot.hMailslot,
                          &Mailslot.ulMessageSize,
                          &Mailslot.ulMailslotSize,
                          &Mailslot.ulNextSize,
                          &Mailslot.ulNextPriority,
                          &Mailslot.ulMessages);
  printf ("\nDos32MailslotInfo:"
          "\n  rc             = %u"
          "\n  hMailslot      = %u"
          "\n  ulMessageSize  = %u"
          "\n  ulMailslotSize = %u"
          "\n  ulNextSize     = %u"
          "\n  ulNextPriority = %u"
          "\n  ulMessages     = %u",
          rc,
          Mailslot.hMailslot,
          Mailslot.ulMessageSize,
          Mailslot.ulMailslotSize,
          Mailslot.ulNextSize,
          Mailslot.ulNextPriority,
          Mailslot.ulMessages);
  
  
  /* --- Dos32PeekMailslot --- */
  
  /* peek ist nicht-blockierend */
  ulBytesRead = ulBufferSize;
  rc = Dos32PeekMailslot (Mailslot.hMailslot,
                          pBuffer,
                          &ulBytesRead,
                          &Mailslot.ulNextSize,
                          &Mailslot.ulNextPriority);
  printf ("\nDos32PeekMailslot:"
          "\n  rc             = %u"
          "\n  ulBytesRead    = %u"
          "\n  ulNextSize     = %u"
          "\n  ulNextPriority = %u",
          rc,
          ulBytesRead,
          Mailslot.ulNextSize,
          Mailslot.ulNextPriority);
  
  HexDump (ulBytesRead,
           pBuffer);
  
  
  /* --- Dos32ReadMailslot --- */
  
  /* read ist blockierend */
  ulBytesRead = ulBufferSize;
  rc = Dos32ReadMailslot (Mailslot.hMailslot,
                          pBuffer,
                          &ulBytesRead,
                          &Mailslot.ulNextSize,
                          &Mailslot.ulNextPriority,
                          0xFFFFFFFF); /* timeout */
  printf ("\nDos32ReadMailslot:"
          "\n  rc             = %u"
          "\n  ulBytesRead    = %u"
          "\n  ulNextSize     = %u"
          "\n  ulNextPriority = %u",
          rc,
          ulBytesRead,
          Mailslot.ulNextSize,
          Mailslot.ulNextPriority);
  
  HexDump (ulBytesRead,
           pBuffer);
          
  
  
  printf ("\nDos32DeleteMailslot");
  rc = Dos32DeleteMailslot(Mailslot.hMailslot);
  printf (" rc=%u",rc);
  
  free (pBuffer);
  
  return 0;
}