/***********************************************************************
 * Name      : Module Stack
 * Funktion  : Implementation eines abstrakten Stackobjektes.
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [(null), 19.05.1996 22.37.56]
 ***********************************************************************/

#ifndef MODULE_STACK
#define MODULE_STACK

#ifdef __cplusplus
      extern "C" {
#endif


#ifdef DEBUG
  #include <stdio.h>
#endif

#define INCL_DOS
#define INCL_NOPMAPI
#include <os2.h>


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
APIRET StackPop     (PSTACK pStack,
                     PVOID  pData,
                     PULONG pulDataLength);

APIRET StackPush    (PSTACK pStack,
                     PVOID  pData,
                     ULONG  ulDataLength);

BOOL   StackIsEmpty (PSTACK pStack);

APIRET StackInit    (PSTACK pStack);

APIRET StackFlush   (PSTACK pStack);


#ifdef DEBUG
void   StackDump    (PSTACK pStack,
                     FILE   *fFile);
#endif


#ifdef __cplusplus
      }
#endif


#endif /* MODULE_STACK */

