/*****************************************************************************
 * Name      : PHSErrorTable
 * Purpose   : Class / Functions for universal error mapping table 
 * Parameter : 
 * Variables :
 * Result    : 
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/


 /*****************************************************************************
 * Includes                                                                  *         
 *****************************************************************************/

#include "phserror.h"



/*****************************************************************************
 * Name      : PHSErrorTable::PHSErrorTable
 * Purpose   : Table constructor
 * Parameter : PERROTABLE pNewErrorTable
 * Variables :
 * Result    : 
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

PHSErrorTable::PHSErrorTable(PERRORTABLE pNewErrorTable) 
{
  pErrorTable = pNewErrorTable;
}


/*****************************************************************************
 * Name      : PHSErrorTable::~PHSErrorTable
 * Purpose   : Table destructor
 * Parameter : 
 * Variables :
 * Result    : 
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

PHSErrorTable::~PHSErrorTable(void) 
{  
}


/*****************************************************************************
 * Name      : APIRET PHSErrorTable::ErrorTableSet
 * Purpose   : Set new table pointer
 * Parameter : PERROTABLE pNewErrorTable
 * Variables :
 * Result    : 
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

APIRET PHSErrorTable::ErrorTableSet(PERRORTABLE pNewErrorTable) 
{
  if (pNewErrorTable == NULL)                            /* check parameters */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */

  pErrorTable = pNewErrorTable;                     /* set new table pointer */

  return (NO_ERROR);                                  /* deliver return code */
}


/*****************************************************************************
 * Name      : PHSERRORTABLE PHSErrorTable::ErrorTableQuery
 * Purpose   : Return current table pointer
 * Parameter : 
 * Variables :
 * Result    : 
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

PERRORTABLE PHSErrorTable::ErrorTableQuery(void) 
{
  return (pErrorTable);                      /* return current table pointer */
}


/*****************************************************************************
 * Name      : APIRET PHSErrorTable::QueryFromOSError
 * Purpose   : Map OS dependent error code to independent error code
 *             and deliver pointer to entry in message table 
 * Parameter : int  iErrorCode  - OS error code
 *             PPSZ ppszMessage - pointer to pointer to receive string
 * Variables :
 * Result    : APIRET - operating system independent error code
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

APIRET PHSErrorTable::QueryFromOSError (int  iErrorCode,
										PPSZ ppszMessage)
{
  PERRORTABLE pTableIterator;                       /* loop pointer on table */

  if (pErrorTable == NULL)    /* check wheter valid table is attached or not */
    return (ERROR_MESSAGE_NO_TABLE);                /* raise error condition */

  for (pTableIterator = pErrorTable;
       pTableIterator->pszMessage != NULL;             /* marks end of table */
	   pTableIterator++)
  {
    if (pTableIterator->iOSError == iErrorCode)           /* find error code */
	{
      if (ppszMessage != NULL)        /* check whether string result desired */
     	*ppszMessage = pTableIterator->pszMessage;         /* return pointer */

	  return (pTableIterator->rcError);          /* return mapped error code */
	}
  }

  if (ppszMessage != NULL)            /* check whether string result desired */
	*ppszMessage = "<unknown error>";                         /* set to NULL */

  return (ERROR_MESSAGE_NOT_FOUND);               /* error message not found */
}


/*****************************************************************************
 * Name      : APIRET PHSErrorTable::Query
 * Purpose   : Map OS independent error code to entry in message table 
 * Parameter : APIRET rc        - OS error code
 *             PPSZ ppszMessage - pointer to pointer to receive string
 * Variables :
 * Result    : APIRET - API return code
 * Remark    :
 * Author    : Patrick Haller [1997/05/28]
 *****************************************************************************/

APIRET PHSErrorTable::Query (APIRET rc,
							 PPSZ ppszMessage)
{
  PERRORTABLE pTableIterator;                       /* loop pointer on table */

  if (pErrorTable == NULL)    /* check wheter valid table is attached or not */
    return (ERROR_MESSAGE_NO_TABLE);                /* raise error condition */

  for (pTableIterator = pErrorTable;
       pTableIterator->pszMessage != NULL;             /* marks end of table */
	   pTableIterator++)
  {
    if (pTableIterator->rcError == rc)                    /* find error code */
	{
      if (ppszMessage != NULL)        /* check whether string result desired */
     	*ppszMessage = pTableIterator->pszMessage;         /* return pointer */

	  return (NO_ERROR);                         /* return mapped error code */
	}
  }

  if (ppszMessage != NULL)            /* check whether string result desired */
	*ppszMessage = "<unknown error>";                         /* set to NULL */

  return (ERROR_MESSAGE_NOT_FOUND);               /* error message not found */
}
