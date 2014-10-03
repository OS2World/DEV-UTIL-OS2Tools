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


typedef struct
{
  HFILE hMailslot; /* Mailslot handle */
  ULONG ulMessageSize; /* Message size */
  ULONG ulMailslotSize; /* Mailslot size */
  ULONG ulNextSize;
  ULONG ulNextPriority;
  ULONG ulMessages; /* number of messages in mailslot */
} MAILSLOT, *PMAILSLOT;


int main (void)
{
  APIRET   rc;
  PSZ      pBuffer = "This is a test message via mailslot.";
  ULONG    ulBufferSize;
  
  
  printf ("\nStart");
  
  ulBufferSize = strlen(pBuffer);
  rc = Dos32WriteMailslot ("\\\\T22114\\MAILSLOT\\MESSNGR",
                           pBuffer,
                           ulBufferSize,
                           1,           /* ulPriority */
                           1,           /* ulClass */
                           0xFFFFFFFF); /* timeout */
  
  printf ("\nrc=%u",rc);
  
  return 0;
}