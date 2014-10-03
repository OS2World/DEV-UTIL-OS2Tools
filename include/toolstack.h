/***********************************************************************
 * Projekt   : OS2TOOLS General C Library
 * Name      : Module os2stack
 * Funktion  : Implementation eines abstrakten Stackobjektes.
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [(null), 19.05.1996 22.37.56]
 ***********************************************************************/

#ifndef MODULE_OS2TOOLS_STACK
#define MODULE_OS2TOOLS_STACK

#ifdef __cplusplus
      extern "C" {
#endif

#pragma pack(1)

/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/
        
#include <stdio.h>
#include "tooltypes.h"                                  /* type definitions */
        
        
/***************************************
 * Datenstruktur eines Stack-Elementes *
 ***************************************/
typedef struct
{
  PVOID pPreviousElement;                  /* Zeiger auf das letzte Element */
  PVOID pData;                                                /* Datenblock */
  ULONG ulDataLength;                              /* Groesse des Elementes */
} STACKELEMENT, *PSTACKELEMENT;

typedef struct
{
  ULONG         ulElements;     /* Zaehler ueber die eingetragenen Elemente */
  PSTACKELEMENT pStackElement;         /* Zeiger auf das erste Stackelement */
} STACK, *PSTACK;


/**************************
 * Prototypen aus STACK.C *
 **************************/
APIRET TOOLAPI StackPop     (PSTACK pStack,
                             PVOID  pData,
                             PULONG pulDataLength);

APIRET TOOLAPI StackPush    (PSTACK pStack,
                             PVOID  pData,
                             ULONG  ulDataLength);

BOOL   TOOLAPI StackIsEmpty (PSTACK pStack);

APIRET TOOLAPI StackInit    (PSTACK pStack);

APIRET TOOLAPI StackFlush   (PSTACK pStack);

void   TOOLAPI StackDump    (PSTACK pStack,
                             FILE   *fFile);


#ifdef __cplusplus
      }
#endif


#endif /* MODULE_OS2TOOLS_STACK */

