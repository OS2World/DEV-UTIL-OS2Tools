/* $Id: completion.c,v 1.1 2001/02/15 15:55:57 phaller Exp $ 
 */

/*
 * Robin - advanced command interpreter for OS/2
 * (c) 2000-2001 Patrick Haller <patrick.haller@innotek.de>
 *
 * Note: this work is based on work of the FreeDOS / FreeCOM and Reactos team.
 *       (GPL applies)
 *
 * $Log: completion.c,v $
 * Revision 1.1  2001/02/15 15:55:57  phaller
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


/* get completion from available environment variables */
PCOMPLETION completionGetEnvironment(void)
{
  PCOMPLETION pCompletionEnvironment = completionNew("Environment");
  
  return pCompletionEnvironment;
}


/* get completion object from static command table */
PCOMPLETION completionGetCommand(void)
{
  PCOMPLETION pCompletionCommand = completionNew("Commands");
  
  return pCompletionCommand;
}


/* read the history file and build a completion object from that */
PCOMPLETION completionGetHistory(PSZ pszHome)
{
  PCOMPLETION pCompletionHistory = completionNew("History");
  
  return pCompletionHistory;
}


/* free completion object */
void completionDelete(PCOMPLETION pCompletion)
{
  ULONG ulIndex;
  
  if (pCompletion != NULL)
  {
                                               /* free all registered items */
    for (ulIndex = 0;
         ulIndex < pCompletion->ulItems;
         ulIndex++)
    {
      memFree(pCompletion->arrpItems[ulIndex]);
    }
    
    memFree(pCompletion->arrpItems);
    memFree(pCompletion);
  }
}


PCOMPLETION completionNew(PSZ pszName)
{
  PCOMPLETION pCompletion;                      /* the object to be created */
  
  if (NULL == pszName)                                 /* verify parameters */
    return NULL;                                         /* signal problems */
  
  pCompletion = memAlloc( sizeof(COMPLETION) );
  if (NULL == pCompletion)                             /* verify allocation */
    return NULL;                                         /* signal problems */
  
  pCompletion->pszName = pszName;             /* keep reference of the name */
  pCompletion->flagValid = FALSE;      /* don't use empty completion object */
  pCompletion->ulItems = 0;                             /* no attached data */
  pCompletion->arrpItems = NULL;
  
  return pCompletion;                                               /* Done */
}