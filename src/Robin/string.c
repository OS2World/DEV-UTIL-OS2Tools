/* $Id: string.c,v 1.1 2001/02/15 15:55:59 phaller Exp $ 
 */

/*
 * Robin - advanced command interpreter for OS/2
 * (c) 2000-2001 Patrick Haller <patrick.haller@innotek.de>
 *
 * Note: this work is based on work of the FreeDOS / FreeCOM and Reactos team.
 *       (GPL applies)
 *
 * $Log: string.c,v $
 * Revision 1.1  2001/02/15 15:55:59  phaller
 * .
 *
 */


/****************************************************************************
 * Includes
 ****************************************************************************/

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

ULONG stringLength(PSZ pszString)
{
  register ULONG ulCount;
  
  for (ulCount = 0;
       
       *pszString != 0;
       
       pszString++,
       ulCount++)
    ;
  
  return ulCount;
}

