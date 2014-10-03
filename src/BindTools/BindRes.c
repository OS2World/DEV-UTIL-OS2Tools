/* $Id: BindRes.c,v 1.5 2001/06/11 22:45:45 bird Exp $
 *
 * Bind a resource item.
 *
 * Copyright (c) 2001 knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 *
 * GPL
 *
 */

/*******************************************************************************
*   Defined Constants And Macros                                               *
*******************************************************************************/
#define INCL_BASE

/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include <os2.h>
#include <sql.h>
#include <sqlcodes.h>
#include <sqlenv.h>
#include <stdio.h>
#include "BindLib.h"



/**
 * Bind a bind packet found in a resource item.
 * No bind options is specified. Hence packet and DB defaults are used.
 * @returns NO_ERROR (0) on success.
 *          Appropirate error code on error.
 * @param   hmod    Handle of the resource module. Use NULLHANDLE
 *                  for the executable of the process.
 * @param   idType  Resource type identifier. Usually 805.
 * @param   idName  Name identifier of the resource.
 * @status  Completely implemented.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
int     BindResource(unsigned long hmod, unsigned long idType, unsigned long idName)
{
    struct sqlca    sqlca = {0};
    BOOL            fRc;
    APIRET          rc;
    ULONG           cbBnd;
    PVOID           pvBnd;
    FILE *          phBnd;
    char            szBindFile[CCHMAXPATH];
    CHAR            achsqlopt[sizeof(struct sqlopt) + sizeof(struct sqloptions) * 3] = {0};
    struct sqlopt * psqlopt = (struct sqlopt *)&achsqlopt[0];

    /*
     * Load resource and get its size.
     */
    rc = DosQueryResourceSize((HMODULE)hmod, idType, idName, &cbBnd);
    if (rc != NO_ERROR)
        return rc;
    rc = DosGetResource((HMODULE)hmod, idType, idName, &pvBnd);
    if (rc != NO_ERROR)
        return rc;

    /*
     * Open temporary file.
     */
    phBnd = fopen(BindTmpFile(szBindFile, "bnd"), "wb");
    if (!phBnd)
    {
        DosFreeResource(pvBnd);
        return ERROR_OPEN_FAILED;
    }

    /*
     * Write the bind file data to the temporary file and close it.
     */
    if (fwrite(pvBnd, 1, cbBnd, phBnd) != cbBnd)
    {
        DosFreeResource(pvBnd);
        fclose(phBnd);
        return ERROR_WRITE_FAULT;
    }
    DosFreeResource(pvBnd);
    fclose(phBnd);

    /*
     * Execute bind, delete tempfile and return.
     */
    psqlopt->option[0].type   = SQL_BLOCK_OPT;
    psqlopt->option[0].val    = SQL_BL_ALL;
    psqlopt->option[1].type   = SQL_ISO_OPT;
    psqlopt->option[1].val    = SQL_UNCOM_READ;
    psqlopt->header.allocated = 0;
    psqlopt->header.used      = 0;
    rc = sqlabndx((_SQLOLDCHAR *)szBindFile,
                  "\\dev\\nul",
                  psqlopt,
                  &sqlca);
    if (rc == NO_ERROR && sqlca.sqlcode != SQL_RC_OK)
        rc = sqlca.sqlcode;
    if (rc)
    {
        /*
         * Error occured. Rerun the bind and collect bind messages.
         */
        char            szBuffer[256];
        FILE *          phTmpFile;
        CHAR            szTmpFile[CCHMAXPATH];

        sqlabndx((_SQLOLDCHAR *)szBindFile,
                 BindTmpFile(szTmpFile, NULL),
                 psqlopt,
                 &sqlca);
        phTmpFile = fopen(szTmpFile, "r");
        if (phTmpFile)
        {
            while (fgets(szBuffer, sizeof(szBuffer), phTmpFile))
                BindError("%s", szBuffer);
            fclose(phTmpFile);
        }
        DosDelete(szTmpFile);
        BindError("package=%s sqlcode=%ld sqlstate=%.5s\n",
                  szBindFile, sqlca.sqlcode, sqlca.sqlstate);
        return rc;
    }

    DosDelete(szBindFile);
    return NO_ERROR;
}

