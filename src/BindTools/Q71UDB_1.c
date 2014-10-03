static char sqla_program_id[162] = 
{
 42,0,65,68,65,74,65,72,81,55,49,85,68,66,95,49,87,66,83,84,
 81,66,72,82,48,49,49,49,49,50,32,32,8,0,66,73,78,68,84,79,
 79,76,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 0,0
};

#include "sqladef.h"

static struct sqla_runtime_info sqla_rtinfo = 
{{'S','Q','L','A','R','T','I','N'}, sizeof(wchar_t), 0, {' ',' ',' ',' '}};


#line 1 "Q71UDB_1.sqc"
/* $Id: Q71UDB_1.c,v 1.8 2001/12/04 17:04:50 bird Exp $
 *
 * Queries for UDB DB2 7.1...
 *
 * Copyright (c) 2001 knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 *
 * GPL.
 *
 */

/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#define INCL_BASE
#include <os2.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <process.h>

#include <sql.h>
#include <sqlcodes.h>
#include <sqlenv.h>

#include "BindLib.h"


/*******************************************************************************
*   Global Variables                                                           *
*******************************************************************************/
extern struct  sqlca   sqlca;


/**
 * Dynamically load this function since not all DB2 version have it ( < 6.0 that is).
 * It would cause a SYS2048 if staticly linked.
 */
SQL_API_RC SQL_API_FN  sqlasetda(unsigned short s1, unsigned short s2, unsigned short s3,
                                 struct sqla_setd_list * p1, struct sqla_setds_list * p2, void *pv)
{
    static HMODULE  hmod = NULLHANDLE;
    static PFN      pfn = NULL;

    if (pfn == NULL)
    {
        APIRET  rc1, rc2;
        char    szObj[16];
        if (    (rc1 = DosLoadModule(szObj, sizeof(szObj), "DB2APP", &hmod))
            ||  (rc2 = DosQueryProcAddr(hmod, 0, "sqlasetda", &pfn)))
        {
            BindFatalError("Failed to load DB2APP (rc=%d, %s) or get sqlasetda (rc=%d)\n",
                           rc1, szObj, rc2);
            pfn = NULL;
            exit(-100);
        }
    }

    return pfn(s1, s2, s3, p1, p2, pv);
}


/**
 * Checks for given bind package.
 * @returns TRUE if it has to be binded. FALSE if bind not needed.
 * @param
 *
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
int  BindItCheck_71UDB(const char *pszCreator, const char *pszPlanName,
                       const char *pszTimeStamp, const char *pszVersion,
                       const char *pszCollId, const char *pszQualifier,
                       const char *pszOwner, int iIsolation)
{
    
/*
EXEC SQL BEGIN DECLARE SECTION;
*/

#line 75 "Q71UDB_1.sqc"

    char    szTimeStampHex[17];
    char    szCollId[129];
    char    szPlanName[9];
    char    szIsolation[3];
    char    szOwner[129];
    char    szQualifier[129];             /* Might need to be incremented for 7.x */
    long    cCount;
    short   niCount;
    
/*
EXEC SQL END DECLARE SECTION;
*/

#line 84 "Q71UDB_1.sqc"

    int     cTries = 2;
    char *  psz;
    char *  pszTS;

    pszVersion = pszVersion;

    /*
     * Validate input.
     */
    if (strlen(pszTimeStamp) != 8)
    {
        BindError("Timestamp is not 8 chars long! (%s)\n", pszTimeStamp);
        return 1;
    }
    if (strlen(pszPlanName) > 8)
    {
        BindError("planname is too long! max 8. (%s)\n", pszPlanName);
        return 1;
    }
    if (strlen(pszCreator) > 18)
    {
        BindError("creator is too long! max 18. (%s)\n", pszCreator);
        return 1;
    }
    if (strlen(pszCollId) > 128)
    {
        BindError("collid is too long! max 128. (%s)\n", pszCollId);
        return 1;
    }
    if (strlen(pszOwner) > 128)
    {
        BindError("owner is too long! max 128. (%s)\n", pszOwner);
        return 1;
    }
    if (strlen(pszQualifier) > 128)
    {
        BindError("qualifier is too long! max 128. (%s)\n", pszQualifier);
        return 1;
    }


    /*
     * Fill hostvariables.
     */
    switch (iIsolation)
    {
        case SQL_REP_READ   : strcpy(szIsolation, "RR"); break;
        case SQL_UNCOM_READ : strcpy(szIsolation, "UR"); break;
        case SQL_READ_STAB  : strcpy(szIsolation, "RS"); break;
        case SQL_NO_COMMIT  : strcpy(szIsolation, "NC"); break;
        case SQL_CURSOR_STAB: strcpy(szIsolation, "CS"); break;
        default:
            BindIntError("Isolation level is wrong. %d\n", iIsolation);
            strcpy(szIsolation, "CS");
            break;
    }
    strcpy(szPlanName,  pszPlanName);
    strcpy(szCollId,    pszCollId[0] != '\0'    ? pszCollId     : pszCreator);
    strcpy(szOwner,     pszOwner[0] != '\0'     ? pszOwner      : "");
    strcpy(szQualifier, pszQualifier[0] != '\0' ? pszQualifier  : "");
    for (psz = szTimeStampHex, pszTimeStamp; *pszTimeStamp; psz+=2, pszTimeStamp++)
        _itoa(*pszTimeStamp, psz, 16);
    strupr(szTimeStampHex);


    /*
     * Do bind. (We loop if we need to bind ourselfs.)
     */
    do
        {
        cCount = 0;
        niCount = -1;
        if (szOwner[0] == '\0' && szQualifier[0] == '\0')
        {
            
/*
EXEC SQL
                SELECT  COUNT(*)
                INTO    :cCount :niCount
                FROM    SYSIBM.SYSPLAN
                WHERE   CREATOR   = :szCollId
                    AND NAME      = :szPlanName
                    AND ISOLATION = :szIsolation
                    AND VALID     = 'Y'
                    AND BLOCK     = 'B'
                    AND VALIDATE  = 'B'
                    AND HEX(UNIQUE_ID) = :szTimeStampHex
                    AND BOUNDBY   = USER
                    AND DEFAULT_SCHEMA = USER;
*/

{
#line 171 "Q71UDB_1.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 171 "Q71UDB_1.sqc"
  sqlaaloc(2,4,1,0L);
    {
      struct sqla_setd_list sql_setdlist[4];
#line 171 "Q71UDB_1.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 129;
#line 171 "Q71UDB_1.sqc"
      sql_setdlist[0].sqldata = (void*)szCollId;
#line 171 "Q71UDB_1.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 171 "Q71UDB_1.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 9;
#line 171 "Q71UDB_1.sqc"
      sql_setdlist[1].sqldata = (void*)szPlanName;
#line 171 "Q71UDB_1.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 171 "Q71UDB_1.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 3;
#line 171 "Q71UDB_1.sqc"
      sql_setdlist[2].sqldata = (void*)szIsolation;
#line 171 "Q71UDB_1.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 171 "Q71UDB_1.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 17;
#line 171 "Q71UDB_1.sqc"
      sql_setdlist[3].sqldata = (void*)szTimeStampHex;
#line 171 "Q71UDB_1.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 171 "Q71UDB_1.sqc"
      sqlasetda(2,0,4,sql_setdlist,NULL,0L);
    }
#line 171 "Q71UDB_1.sqc"
  sqlaaloc(3,1,2,0L);
    {
      struct sqla_setd_list sql_setdlist[1];
#line 171 "Q71UDB_1.sqc"
      sql_setdlist[0].sqltype = 497; sql_setdlist[0].sqllen = 4;
#line 171 "Q71UDB_1.sqc"
      sql_setdlist[0].sqldata = (void*)&cCount;
#line 171 "Q71UDB_1.sqc"
      sql_setdlist[0].sqlind = &niCount;
#line 171 "Q71UDB_1.sqc"
      sqlasetda(3,0,1,sql_setdlist,NULL,0L);
    }
#line 171 "Q71UDB_1.sqc"
  sqlacall((unsigned short)24,1,2,3,0L);
#line 171 "Q71UDB_1.sqc"
  sqlastop(0L);
}

#line 171 "Q71UDB_1.sqc"

        }
        else if (szOwner[0] != '\0' && szQualifier[0] != '\0')
        {
            
/*
EXEC SQL
                SELECT  COUNT(*)
                INTO    :cCount :niCount
                FROM    SYSIBM.SYSPLAN
                WHERE   CREATOR   = :szCollId
                    AND NAME      = :szPlanName
                    AND ISOLATION = :szIsolation
                    AND VALID     = 'Y'
                    AND BLOCK     = 'B'
                    AND VALIDATE  = 'B'
                    AND HEX(UNIQUE_ID) = :szTimeStampHex
                    AND BOUNDBY   = :szOwner
                    AND DEFAULT_SCHEMA = :szQualifier;
*/

{
#line 187 "Q71UDB_1.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 187 "Q71UDB_1.sqc"
  sqlaaloc(2,6,3,0L);
    {
      struct sqla_setd_list sql_setdlist[6];
#line 187 "Q71UDB_1.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 129;
#line 187 "Q71UDB_1.sqc"
      sql_setdlist[0].sqldata = (void*)szCollId;
#line 187 "Q71UDB_1.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 187 "Q71UDB_1.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 9;
#line 187 "Q71UDB_1.sqc"
      sql_setdlist[1].sqldata = (void*)szPlanName;
#line 187 "Q71UDB_1.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 187 "Q71UDB_1.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 3;
#line 187 "Q71UDB_1.sqc"
      sql_setdlist[2].sqldata = (void*)szIsolation;
#line 187 "Q71UDB_1.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 187 "Q71UDB_1.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 17;
#line 187 "Q71UDB_1.sqc"
      sql_setdlist[3].sqldata = (void*)szTimeStampHex;
#line 187 "Q71UDB_1.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 187 "Q71UDB_1.sqc"
      sql_setdlist[4].sqltype = 460; sql_setdlist[4].sqllen = 129;
#line 187 "Q71UDB_1.sqc"
      sql_setdlist[4].sqldata = (void*)szOwner;
#line 187 "Q71UDB_1.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 187 "Q71UDB_1.sqc"
      sql_setdlist[5].sqltype = 460; sql_setdlist[5].sqllen = 129;
#line 187 "Q71UDB_1.sqc"
      sql_setdlist[5].sqldata = (void*)szQualifier;
#line 187 "Q71UDB_1.sqc"
      sql_setdlist[5].sqlind = 0L;
#line 187 "Q71UDB_1.sqc"
      sqlasetda(2,0,6,sql_setdlist,NULL,0L);
    }
#line 187 "Q71UDB_1.sqc"
  sqlaaloc(3,1,4,0L);
    {
      struct sqla_setd_list sql_setdlist[1];
#line 187 "Q71UDB_1.sqc"
      sql_setdlist[0].sqltype = 497; sql_setdlist[0].sqllen = 4;
#line 187 "Q71UDB_1.sqc"
      sql_setdlist[0].sqldata = (void*)&cCount;
#line 187 "Q71UDB_1.sqc"
      sql_setdlist[0].sqlind = &niCount;
#line 187 "Q71UDB_1.sqc"
      sqlasetda(3,0,1,sql_setdlist,NULL,0L);
    }
#line 187 "Q71UDB_1.sqc"
  sqlacall((unsigned short)24,2,2,3,0L);
#line 187 "Q71UDB_1.sqc"
  sqlastop(0L);
}

#line 187 "Q71UDB_1.sqc"

        }
        else if (szOwner[0] != '\0')
        {
            
/*
EXEC SQL
                SELECT  COUNT(*)
                INTO    :cCount :niCount
                FROM    SYSIBM.SYSPLAN
                WHERE   CREATOR   = :szCollId
                    AND NAME      = :szPlanName
                    AND ISOLATION = :szIsolation
                    AND VALID     = 'Y'
                    AND BLOCK     = 'B'
                    AND VALIDATE  = 'B'
                    AND HEX(UNIQUE_ID) = :szTimeStampHex
                    AND BOUNDBY   = :szOwner
                    AND DEFAULT_SCHEMA = :szOwner;
*/

{
#line 203 "Q71UDB_1.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 203 "Q71UDB_1.sqc"
  sqlaaloc(2,6,5,0L);
    {
      struct sqla_setd_list sql_setdlist[6];
#line 203 "Q71UDB_1.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 129;
#line 203 "Q71UDB_1.sqc"
      sql_setdlist[0].sqldata = (void*)szCollId;
#line 203 "Q71UDB_1.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 203 "Q71UDB_1.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 9;
#line 203 "Q71UDB_1.sqc"
      sql_setdlist[1].sqldata = (void*)szPlanName;
#line 203 "Q71UDB_1.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 203 "Q71UDB_1.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 3;
#line 203 "Q71UDB_1.sqc"
      sql_setdlist[2].sqldata = (void*)szIsolation;
#line 203 "Q71UDB_1.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 203 "Q71UDB_1.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 17;
#line 203 "Q71UDB_1.sqc"
      sql_setdlist[3].sqldata = (void*)szTimeStampHex;
#line 203 "Q71UDB_1.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 203 "Q71UDB_1.sqc"
      sql_setdlist[4].sqltype = 460; sql_setdlist[4].sqllen = 129;
#line 203 "Q71UDB_1.sqc"
      sql_setdlist[4].sqldata = (void*)szOwner;
#line 203 "Q71UDB_1.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 203 "Q71UDB_1.sqc"
      sql_setdlist[5].sqltype = 460; sql_setdlist[5].sqllen = 129;
#line 203 "Q71UDB_1.sqc"
      sql_setdlist[5].sqldata = (void*)szOwner;
#line 203 "Q71UDB_1.sqc"
      sql_setdlist[5].sqlind = 0L;
#line 203 "Q71UDB_1.sqc"
      sqlasetda(2,0,6,sql_setdlist,NULL,0L);
    }
#line 203 "Q71UDB_1.sqc"
  sqlaaloc(3,1,6,0L);
    {
      struct sqla_setd_list sql_setdlist[1];
#line 203 "Q71UDB_1.sqc"
      sql_setdlist[0].sqltype = 497; sql_setdlist[0].sqllen = 4;
#line 203 "Q71UDB_1.sqc"
      sql_setdlist[0].sqldata = (void*)&cCount;
#line 203 "Q71UDB_1.sqc"
      sql_setdlist[0].sqlind = &niCount;
#line 203 "Q71UDB_1.sqc"
      sqlasetda(3,0,1,sql_setdlist,NULL,0L);
    }
#line 203 "Q71UDB_1.sqc"
  sqlacall((unsigned short)24,3,2,3,0L);
#line 203 "Q71UDB_1.sqc"
  sqlastop(0L);
}

#line 203 "Q71UDB_1.sqc"

        }
        else
        {
            
/*
EXEC SQL
                SELECT  COUNT(*)
                INTO    :cCount :niCount
                FROM    SYSIBM.SYSPLAN
                WHERE   CREATOR   = :szCollId
                    AND NAME      = :szPlanName
                    AND ISOLATION = :szIsolation
                    AND VALID     = 'Y'
                    AND BLOCK     = 'B'
                    AND VALIDATE  = 'B'
                    AND HEX(UNIQUE_ID) = :szTimeStampHex
                    AND BOUNDBY   = USER
                    AND DEFAULT_SCHEMA = :szQualifier;
*/

{
#line 219 "Q71UDB_1.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 219 "Q71UDB_1.sqc"
  sqlaaloc(2,5,7,0L);
    {
      struct sqla_setd_list sql_setdlist[5];
#line 219 "Q71UDB_1.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 129;
#line 219 "Q71UDB_1.sqc"
      sql_setdlist[0].sqldata = (void*)szCollId;
#line 219 "Q71UDB_1.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 219 "Q71UDB_1.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 9;
#line 219 "Q71UDB_1.sqc"
      sql_setdlist[1].sqldata = (void*)szPlanName;
#line 219 "Q71UDB_1.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 219 "Q71UDB_1.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 3;
#line 219 "Q71UDB_1.sqc"
      sql_setdlist[2].sqldata = (void*)szIsolation;
#line 219 "Q71UDB_1.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 219 "Q71UDB_1.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 17;
#line 219 "Q71UDB_1.sqc"
      sql_setdlist[3].sqldata = (void*)szTimeStampHex;
#line 219 "Q71UDB_1.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 219 "Q71UDB_1.sqc"
      sql_setdlist[4].sqltype = 460; sql_setdlist[4].sqllen = 129;
#line 219 "Q71UDB_1.sqc"
      sql_setdlist[4].sqldata = (void*)szQualifier;
#line 219 "Q71UDB_1.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 219 "Q71UDB_1.sqc"
      sqlasetda(2,0,5,sql_setdlist,NULL,0L);
    }
#line 219 "Q71UDB_1.sqc"
  sqlaaloc(3,1,8,0L);
    {
      struct sqla_setd_list sql_setdlist[1];
#line 219 "Q71UDB_1.sqc"
      sql_setdlist[0].sqltype = 497; sql_setdlist[0].sqllen = 4;
#line 219 "Q71UDB_1.sqc"
      sql_setdlist[0].sqldata = (void*)&cCount;
#line 219 "Q71UDB_1.sqc"
      sql_setdlist[0].sqlind = &niCount;
#line 219 "Q71UDB_1.sqc"
      sqlasetda(3,0,1,sql_setdlist,NULL,0L);
    }
#line 219 "Q71UDB_1.sqc"
  sqlacall((unsigned short)24,4,2,3,0L);
#line 219 "Q71UDB_1.sqc"
  sqlastop(0L);
}

#line 219 "Q71UDB_1.sqc"

        }

        if (    sqlca.sqlcode != SQL_RC_E818
            &&  sqlca.sqlcode != SQL_RC_E805)
            break;

        /*
         * Bind package of this program.
         */
        if (BindResource(0, 805, 3))
            {
            BindFatalError("Selfbind failed! " __FILE__); fflush(stdout);
            exit(-805);
            return -1;
            }
        } while (cTries--);


    return sqlca.sqlcode != SQL_RC_OK || niCount < 0 || cCount <= 0;
}


/**
 * Checks if there is a bind package in the database matching a program id.
 * @returns TRUE if there is such a package.
 *          FALSE if there isn't such a package.
 * @param   pszSqlUser      Pointer to the sqluser string from the program id.
 * @param   pszPlanName     Pointer to the planname string from the program id.
 * @param   pszTimeStamp    Pointer to the timestamp string from the program id.
 *
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
int  BindCheckPrgId_71UDB(const char *pszSqlUser, const char *pszPlanName, const char *pszTimeStamp)
{
    
/*
EXEC SQL BEGIN DECLARE SECTION;
*/

#line 254 "Q71UDB_1.sqc"

    char    szTimeStampHex2[17];
    char    szSqlUser[129];
    char    szPlanName2[9];
    long    cCount2;
    short   niCount2;
    
/*
EXEC SQL END DECLARE SECTION;
*/

#line 260 "Q71UDB_1.sqc"

    int     cTries = 2;
    char *  psz;
    char *  pszTS;

    /*
     * Validate input.
     */
    if (strlen(pszTimeStamp) != 8)
    {
        BindError("Timestamp is not 8 chars long! (%s)\n", pszTimeStamp);
        return 1;
    }
    if (strlen(pszPlanName) > 8)
    {
        BindError("planname is too long! max 8. (%s)\n", pszPlanName);
        return 1;
    }
    if (strlen(pszSqlUser) > 128)
    {
        BindError("sqluser is too long! max 128. (%s)\n", pszSqlUser);
        return 1;
    }


    /*
     * Fill hostvariables.
     */
    strcpy(szPlanName2,  pszPlanName);
    strcpy(szSqlUser,   pszSqlUser);
    for (psz = szTimeStampHex2, pszTimeStamp; *pszTimeStamp; psz+=2, pszTimeStamp++)
        _itoa(*pszTimeStamp, psz, 16);
    strupr(szTimeStampHex2);


    /*
     * Do bind. (We loop if we need to bind ourselfs.)
     */
    do
        {
        cCount2 = 0;
        niCount2 = -1;
        
/*
EXEC SQL
            SELECT  COUNT(*)
            INTO    :cCount2 :niCount2
            FROM    SYSIBM.SYSPLAN
            WHERE   NAME      = :szPlanName2
                AND CREATOR   = :szSqlUser
                AND VALID     = 'Y'
                AND HEX(UNIQUE_ID) = :szTimeStampHex2;
*/

{
#line 309 "Q71UDB_1.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 309 "Q71UDB_1.sqc"
  sqlaaloc(2,3,9,0L);
    {
      struct sqla_setd_list sql_setdlist[3];
#line 309 "Q71UDB_1.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 9;
#line 309 "Q71UDB_1.sqc"
      sql_setdlist[0].sqldata = (void*)szPlanName2;
#line 309 "Q71UDB_1.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 309 "Q71UDB_1.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 129;
#line 309 "Q71UDB_1.sqc"
      sql_setdlist[1].sqldata = (void*)szSqlUser;
#line 309 "Q71UDB_1.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 309 "Q71UDB_1.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 17;
#line 309 "Q71UDB_1.sqc"
      sql_setdlist[2].sqldata = (void*)szTimeStampHex2;
#line 309 "Q71UDB_1.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 309 "Q71UDB_1.sqc"
      sqlasetda(2,0,3,sql_setdlist,NULL,0L);
    }
#line 309 "Q71UDB_1.sqc"
  sqlaaloc(3,1,10,0L);
    {
      struct sqla_setd_list sql_setdlist[1];
#line 309 "Q71UDB_1.sqc"
      sql_setdlist[0].sqltype = 497; sql_setdlist[0].sqllen = 4;
#line 309 "Q71UDB_1.sqc"
      sql_setdlist[0].sqldata = (void*)&cCount2;
#line 309 "Q71UDB_1.sqc"
      sql_setdlist[0].sqlind = &niCount2;
#line 309 "Q71UDB_1.sqc"
      sqlasetda(3,0,1,sql_setdlist,NULL,0L);
    }
#line 309 "Q71UDB_1.sqc"
  sqlacall((unsigned short)24,5,2,3,0L);
#line 309 "Q71UDB_1.sqc"
  sqlastop(0L);
}

#line 309 "Q71UDB_1.sqc"


        if (    sqlca.sqlcode != SQL_RC_E818
            &&  sqlca.sqlcode != SQL_RC_E805)
            break;

        /*
         * Bind package of this program.
         */
        if (BindResource(0, 805, 3))
            {
            BindFatalError("Selfbind failed! " __FILE__); fflush(stdout);
            exit(-805);
            return -1;
            }
        } while (cTries--);


    return sqlca.sqlcode == SQL_RC_OK && niCount2 >= 0 && cCount2 >= 1;
}



/**
 * Gets info about evt. package in DB.
 * @returns TRUE if we found a package. FALSE if didn't find a package.
 * @param   pszCreator      Pointer to the collection/creator/sqluser string.
 * @param   pszPlanName     Pointer to the application/planname string.
 * @param   pszTimeStamp    Pointer to the timestamp string.
 * @param   pszVersion      Pointer to the version string.
 * @param   *               Output fields.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 * @remark  The check isn't necesarrily sufficent for bind packages
 *          which is using uncommon fields.
 *          I need feedback on this...
 */
int  BindGetInfo_71UDB(/* input */
                       const char * pszCollId,
                       const char * pszName,
                       const char * pszTimeStamp,
                       const char * pszVersion,
                       /* output */
                       char       * pszCreator,
                       char       * pszQualifier,
                       char       * pszBoundBy,
                       char       * pchValid,
                       char       * pchOperative,
                       char       * pchValidate,
                       char       * pchBlocking,
                       int        * piIsolation)
{
    
/*
EXEC SQL BEGIN DECLARE SECTION;
*/

#line 360 "Q71UDB_1.sqc"

    char    szTimeStampHexI[17];
    char    szCollIdI[129];
    char    szNameI[9];

    char    chValidI;
    char    chBlockI;
    char    chValidateI;
    char    szIsolationI[3];
    char    szBoundByI[129];
    char    szDefinerI[129];
    char    szDefaultSchemaI[129];
    
/*
EXEC SQL END DECLARE SECTION;
*/

#line 372 "Q71UDB_1.sqc"

    int     cTries = 2;
    char *  psz;
    char *  pszTS;

    pszVersion = pszVersion;

    /*
     * Validate input.
     */
    if (strlen(pszTimeStamp) != 8)
    {
        BindError("Timestamp is not 8 chars long! (%s)\n", pszTimeStamp);
        return 1;
    }
    if (strlen(pszName) > 8)
    {
        BindError("planname is too long! max 8. (%s)\n", pszName);
        return 1;
    }
    if (strlen(pszCollId) > 128)
    {
        BindError("collid is too long! max 128. (%s)\n", pszCollId);
        return 1;
    }


    /*
     * Fill hostvariables.
     */
    strcpy(szNameI,     pszName);
    strcpy(szCollIdI,   pszCollId);
    for (psz = szTimeStampHexI, pszTimeStamp; *pszTimeStamp; psz+=2, pszTimeStamp++)
        _itoa(*pszTimeStamp, psz, 16);
    strupr(szTimeStampHexI);


    /*
     * Do bind. (We loop if we need to bind ourselfs.)
     */
    do
    {
        
/*
EXEC SQL
            SELECT  VALID,
                    BLOCK,
                    ISOLATION,
                    BOUNDBY,
                    DEFINER,
                    VALIDATE,
                    DEFAULT_SCHEMA
            INTO    :chValidI,
                    :chBlockI,
                    :szIsolationI,
                    :szBoundByI,
                    :szDefinerI,
                    :chValidateI,
                    :szDefaultSchemaI
            FROM    SYSIBM.SYSPLAN
            WHERE   CREATOR   = :szCollIdI
                AND NAME      = :szNameI
                AND HEX(UNIQUE_ID) = :szTimeStampHexI;
*/

{
#line 432 "Q71UDB_1.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 432 "Q71UDB_1.sqc"
  sqlaaloc(2,3,11,0L);
    {
      struct sqla_setd_list sql_setdlist[3];
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 129;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[0].sqldata = (void*)szCollIdI;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 9;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[1].sqldata = (void*)szNameI;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 17;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[2].sqldata = (void*)szTimeStampHexI;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 432 "Q71UDB_1.sqc"
      sqlasetda(2,0,3,sql_setdlist,NULL,0L);
    }
#line 432 "Q71UDB_1.sqc"
  sqlaaloc(3,7,12,0L);
    {
      struct sqla_setd_list sql_setdlist[7];
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[0].sqltype = 452; sql_setdlist[0].sqllen = 1;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[0].sqldata = (void*)&chValidI;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[1].sqltype = 452; sql_setdlist[1].sqllen = 1;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[1].sqldata = (void*)&chBlockI;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 3;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[2].sqldata = (void*)szIsolationI;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 129;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[3].sqldata = (void*)szBoundByI;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[4].sqltype = 460; sql_setdlist[4].sqllen = 129;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[4].sqldata = (void*)szDefinerI;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[5].sqltype = 452; sql_setdlist[5].sqllen = 1;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[5].sqldata = (void*)&chValidateI;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[5].sqlind = 0L;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[6].sqltype = 460; sql_setdlist[6].sqllen = 129;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[6].sqldata = (void*)szDefaultSchemaI;
#line 432 "Q71UDB_1.sqc"
      sql_setdlist[6].sqlind = 0L;
#line 432 "Q71UDB_1.sqc"
      sqlasetda(3,0,7,sql_setdlist,NULL,0L);
    }
#line 432 "Q71UDB_1.sqc"
  sqlacall((unsigned short)24,6,2,3,0L);
#line 432 "Q71UDB_1.sqc"
  sqlastop(0L);
}

#line 432 "Q71UDB_1.sqc"


        if (    sqlca.sqlcode != SQL_RC_E818
            &&  sqlca.sqlcode != SQL_RC_E805)
            break;

        /*
         * Bind package of this program.
         */
        if (BindResource(0, 805, 3))
        {
            BindFatalError("Selfbind failed! " __FILE__); fflush(stdout);
            exit(-805);
            return -1;
        }
    } while (cTries--);


    if (sqlca.sqlcode == SQL_RC_OK)
    {
        if (pchValid)
            *pchValid = chValidI;
        if (pchBlocking)
            *pchBlocking = chBlockI;
        if (piIsolation)
        {
            if (!strcmp(szIsolationI, "RR"))
                *piIsolation = SQL_REP_READ;
            else if (!strcmp(szIsolationI, "UR"))
                *piIsolation = SQL_UNCOM_READ;
            else if (!strcmp(szIsolationI, "RS"))
                *piIsolation = SQL_READ_STAB;
            else if (!strcmp(szIsolationI, "NC"))
                *piIsolation = SQL_NO_COMMIT;
            else if (!strcmp(szIsolationI, "CS"))
                *piIsolation = SQL_CURSOR_STAB;
            else
            {
                BindIntError("Isolation level is unknown. ISOLATION='%s'\n", szIsolationI);
                *piIsolation = -1;
            }
        }
        if (pszBoundBy)
            strcpy(pszBoundBy, szBoundByI); /* definer? */
        if (pszCreator)
            strcpy(pszCreator, szDefinerI); /* ?? */
        if (pszQualifier)
            strcpy(pszQualifier, szDefaultSchemaI);
        if (pchOperative)
            *pchOperative = chValidI;   /* ?? */
        if (pchValidate)
            *pchValidate = chValidateI;

    }

    return sqlca.sqlcode == SQL_RC_OK;
}



