/* $Id: memory.c,v 1.1 2001/02/15 15:55:58 phaller Exp $ 
 */

/*
 * Robin - advanced command interpreter for OS/2
 * (c) 2000-2001 Patrick Haller <patrick.haller@innotek.de>
 *
 * Note: this work is based on work of the FreeDOS / FreeCOM and Reactos team.
 *       (GPL applies)
 *
 * $Log: memory.c,v $
 * Revision 1.1  2001/02/15 15:55:58  phaller
 * .
 *
 */


/****************************************************************************
 * Includes
 ****************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "robin_version.h"
#include "robin.h"


/****************************************************************************
 * Type definitions
 ****************************************************************************/

 
 /****************************************************************************
 * Static, global variables
 ****************************************************************************/

/****************************************************************************
 * Implementation
 ****************************************************************************/

PVOID memAlloc(ULONG ulSize)
{
  return malloc(ulSize);
}

void memFree(PVOID pvObject)
{
  if (NULL != pvObject)
    free(pvObject);
}

void memCopy(PVOID pvDest, PVOID pvSource, ULONG ulLength)
{
  memcpy(pvDest,pvSource,ulLength);
}
