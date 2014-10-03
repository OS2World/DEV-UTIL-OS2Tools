/* $Id: robin.c,v 1.1 2001/02/15 15:55:58 phaller Exp $ 
 */

/*
 * Robin - advanced command interpreter for OS/2
 * (c) 2000-2001 Patrick Haller <patrick.haller@innotek.de>
 *
 * Note: this work is based on work of the FreeDOS / FreeCOM and Reactos team.
 *       (GPL applies)
 *
 * $Log: robin.c,v $
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

#define CRLF "\r\n"


 /****************************************************************************
 * Static, global variables
 ****************************************************************************/

static CMD_SESSION globalCmdSession = {0};

typedef struct
{
  PSZ  pszUser;                                 /* name of the current user */
  PSZ  pszHome;                       /* home directory of the current user */
  BOOL flagQuiet;                                    /* display logo or not */
} OPTIONS, *POPTIONS;
static OPTIONS Options;


/****************************************************************************
 * Implementation
 ****************************************************************************/

APIRET mainSetupSession(void)
{
  globalCmdSession.ulType = SESSION_STANDARD;               /* session type */
  globalCmdSession.hOut   = 0;                          /* standard handles */
  globalCmdSession.hIn    = 1;
  globalCmdSession.hOut   = 2;
  globalCmdSession.flagTerminate = FALSE;
  
  globalCmdSession.pStackDirectory = NULL;              /* empty at startup */
  
                                       /* create the static completion objects */
  globalCmdSession.pCompletionEnvironment = completionGetEnvironment();
  globalCmdSession.pCompletionCommand     = completionGetCommand();
  globalCmdSession.pCompletionHistory     = completionGetHistory(Options.pszHome);
  
  return NO_ERROR;
}


APIRET mainParseParameters(int argc,
                           char **argv)
{
  Options.flagQuiet = FALSE;
  return NO_ERROR;
}


void mainTitle(PCMD_SESSION pSession)
{
  ConsoleOutString(pSession,
                   "ROBIN "VERSION_MAJOR"."VERSION_MINOR" "
                   "- Advanced Command Interpreter for OS/2"CRLF
                   "(c) 2000-2001 Patrick Haller <patrick.haller@innotek.de>"CRLF);
  
#ifdef DEBUG
  ConsoleOutString(pSession,
                   "Build %s, internal version %2u.%02u\n",
                   __DATE__,
                   __TIME__,
                   VERSION_BUILD);
#endif
}


int main(int argc,
         char **argv)
{
  APIRET rc;                                       /* operation return code */
  
  rc = mainSetupSession();                 /* parse command line parameters */
  if (rc != NO_ERROR)                                   /* check for errors */
    return rc;                                            /* abort on error */

  rc = mainParseParameters(argc, argv);    /* parse command line parameters */
  if (rc != NO_ERROR)                                   /* check for errors */
    return rc;                                            /* abort on error */

  if (Options.flagQuiet == FALSE)              /* display welcome message ? */
    mainTitle(&globalCmdSession);
  
  rc = parseEngine(&globalCmdSession);       /* call the main parser engine */
  if (rc != NO_ERROR)                                   /* check for errors */
    return rc;                                            /* abort on error */
  
  return NO_ERROR;                                /* OK, exit shell process */
}