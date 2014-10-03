/* $Id: console.c,v 1.1 2001/02/15 15:55:58 phaller Exp $ 
 */

/*
 * Robin - advanced command interpreter for OS/2
 * (c) 2000-2001 Patrick Haller <patrick.haller@innotek.de>
 *
 * Note: this work is based on work of the FreeDOS / FreeCOM and Reactos team.
 *       (GPL applies)
 *
 * $Log: console.c,v $
 * Revision 1.1  2001/02/15 15:55:58  phaller
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


/* send a string to standard out */
APIRET ConsoleOutString(PCMD_SESSION pSession,
                        PSZ pszString)
{
  ULONG ulWritten;
  
  return DosWrite(pSession->hOut,
                  pszString,
                  stringLength(pszString),
                  &ulWritten);
}


/* send a formatted string to standard out */
APIRET ConsoleOutPrint(PCMD_SESSION pSession,
                    PSZ pszFormat,
                    ...)
{
  return ConsoleOutString(pSession,
                          pszFormat);
}


/* read a line from the stdin channel */
APIRET ConsoleReadString(PCMD_SESSION pSession,
                         PSZ pszLine,
                         PULONG pulLineLength)
{
  return DosRead(pSession->hIn,
                 pszLine,
                 *pulLineLength,
                 pulLineLength);
}