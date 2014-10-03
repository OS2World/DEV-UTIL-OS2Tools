/* $Id: BindCnct.c,v 1.2 2001/06/11 22:45:44 bird Exp $
 *
 * Connect to database and disconnect from database operations.
 *
 * Copyright (c) 2001 knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 *
 * GPL.
 *
 */


/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include <os2.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sql.h>
#include <sqlcodes.h>
#include <sqlenv.h>

#include "BindLib.h"


/*******************************************************************************
*   Global Variables                                                           *
*******************************************************************************/
extern struct sqlca sqlca;



/**
 * Connects to a database using the sqlca and the provided database name.
 * If you specify pointer to a DBINFO struct version info of the database is
 * returned as well.
 * @returns sqlcode of the connect.
 * @param   pszDatabase     Database name.
 * @param   pDBInfo         Pointer database info structure.
 *                          NULL is allowed.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
int BindDBConnect(const char *pszDatabase, PDBINFO pDBInfo)
{
    sqlestrd((char*)pszDatabase, SQL_USE_SHR, &sqlca);
    if (sqlca.sqlcode == SQL_RC_OK && pDBInfo)
    {
        char    sz[3];
        sz[2] = '\0';
        memcpy(sz, &sqlca.sqlerrp[3], 2);
        pDBInfo->iDB2Major = atoi(sz);
        memcpy(sz, &sqlca.sqlerrp[5], 2);
        pDBInfo->iDB2Midi  = atoi(sz);
        memcpy(sz, &sqlca.sqlerrp[7], 2);
        pDBInfo->iDB2Minor = atoi(sz);
        if (!memcmp(&sqlca.sqlerrp[0], "DSN", 3))
            pDBInfo->fMVS = TRUE;
        else if (!memcmp(&sqlca.sqlerrp[0], "SQL", 3))
            pDBInfo->fMVS = FALSE;
        else
        {
            BindWarning("Don't know what host type we're connected to '%.8s'.\n",
                        sqlca.sqlerrp);
            pDBInfo->fMVS = FALSE;
        }
    }

    return sqlca.sqlcode;
}


/**
 * Disconnects from the database which currently connected to.
 * @returns sqlcode of the disconnect operation.
 * @param   pszDatabase     Name of the database we're connected to...
 *                          Used only to check if there is a connection.
 *                          If empty string, then we're not connected.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
int BindDBDisconnect(const char *pszDatabase)
{
    if (!*pszDatabase)
        return SQL_RC_OK;

    sqlestpd(&sqlca);
    return sqlca.sqlcode;
}

