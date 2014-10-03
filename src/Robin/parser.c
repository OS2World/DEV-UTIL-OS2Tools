/* $Id: parser.c,v 1.1 2001/02/15 15:55:58 phaller Exp $ 
 */

/*
 * Robin - advanced command interpreter for OS/2
 * (c) 2000-2001 Patrick Haller <patrick.haller@innotek.de>
 *
 * Note: this work is based on work of the FreeDOS / FreeCOM and Reactos team.
 *       (GPL applies)
 *
 * $Log: parser.c,v $
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


/* "fully" get a command line from the user's keyboard */
APIRET parseGetCommandLine(PCMD_SESSION pSession,
                           PSZ pszLine,
                           ULONG ulLineLength)
{
  ULONG ulRead = ulLineLength;
  
  /* display the prompt */
  ConsoleOutString(pSession, ":>");
  
  /* @@@PH shortcut */
  return ConsoleReadString(pSession,
                           pszLine,
                           &ulRead);
}


/* main entry to the parser engine
 * reentrant for script processing!
 */
APIRET parseEngine(PCMD_SESSION pSession)
{
  APIRET rc;                                       /* operation return code */
  CHAR   szLineBuffer[2048];

  
  do
  {
                                             /* 1 - get parsed command line */
    rc = parseGetCommandLine(pSession,
                             szLineBuffer,
                             sizeof(szLineBuffer));
    if (rc != NO_ERROR)                                 /* check for errors */
      return rc;
    
         /* 2 - process alias definitions and resolve environment variables */
    
    
#ifdef DEBUG
    ConsoleOutString(pSession,
                     "DEBUG: cooked command line is ");
    ConsoleOutString(pSession,
                     szLineBuffer);
#endif
                                      /* 3 - check with registered commands */
    
                             /* 4 - pass on to the image execution handlers */
    
  }
  while (!pSession->flagTerminate);
  
  return NO_ERROR; /* Done */
}