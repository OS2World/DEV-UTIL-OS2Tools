static char sqla_program_id[40] = 
{111,65,65,66,65,71,65,68,66,73,78,68,84,79,79,76,81,54,49,77,
86,83,95,49,57,65,49,76,82,69,77,82,48,49,49,49,49,32,32,32};


#include "sqladef.h"

static struct sqla_runtime_info sqla_rtinfo = 
{{'S','Q','L','A','R','T','I','N'}, sizeof(wchar_t), 0, {' ',' ',' ',' '}};


#line 1 "Q61MVS_1.sqc"
/* $Id: Q61MVS_1.c,v 1.5 2001/12/04 17:04:50 bird Exp $
 *
 * Queries for MVS DB2...
 *
 * Copyright (c) 2001 knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 *
 * GPL.
 *
 */

/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
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
 * Checks for given bind package.
 * @returns TRUE if it has to be binded. FALSE if bind not needed.
 * @param   pFileEntry      Pointer to the fileentry we gonna lookup
 *                          in the database.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
int  BindItCheck_61MVS(const char *pszCreator, const char *pszPlanName,
                       const char *pszTimeStamp, const char *pszVersion,
                       const char *pszCollId, const char *pszQualifier,
                       const char *pszOwner, int iIsolation)
{
    
/*
EXEC SQL BEGIN DECLARE SECTION;
*/

#line 45 "Q61MVS_1.sqc"

    char    szTimeStampHex[17];
    char    szCollId[19];
    char    szPlanName[9];
    char    chIsolation;
    char    szOwner[9];                 /* Might need to be incremented for 7.x */
    char    szQualifier[9];             /* Might need to be incremented for 7.x */
    char    szVersion[65];
    long    cCount;
    short   niCount;
    
/*
EXEC SQL END DECLARE SECTION;
*/

#line 55 "Q61MVS_1.sqc"

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
    if (strlen(pszCreator) > 18)
    {
        BindError("creator is too long! max 18. (%s)\n", pszCreator);
        return 1;
    }
    if (strlen(pszCollId) > 18)
    {
        BindError("collid is too long! max 18. (%s)\n", pszCollId);
        return 1;
    }
    if (strlen(pszOwner) > 8)
    {
        BindError("owner is too long! max 8. (%s)\n", pszOwner);
        return 1;
    }
    if (strlen(pszQualifier) > 8)
    {
        BindError("qualifier is too long! max 8. (%s)\n", pszQualifier);
        return 1;
    }
    if (strlen(pszVersion) > 64)
    {
        BindError("version is too long! max 64. (%s)\n", pszVersion);
        return 1;
    }


    /*
     * Fill hostvariables.
     */
    switch (iIsolation)
    {
        case SQL_NO_COMMIT  : chIsolation = 'U'; break;
        case SQL_UNCOM_READ : chIsolation = 'U'; break;
        case SQL_READ_STAB  : chIsolation = 'T'; break;
        case SQL_REP_READ   : chIsolation = 'R'; break;
        case SQL_CURSOR_STAB: chIsolation = 'S'; break;
        default:
            BindIntError("Isolation lever is wrong. %d\n", iIsolation);
            chIsolation = 'S';
            break;
    }
    strcpy(szPlanName,  pszPlanName);
    strcpy(szCollId,    pszCollId[0] != '\0'    ? pszCollId     : pszCreator);
    strcpy(szOwner,     pszOwner[0] != '\0'     ? pszOwner      : "");
    strcpy(szQualifier, pszQualifier[0] != '\0' ? pszQualifier  : "");
    strcpy(szVersion,   pszVersion);
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
                FROM    SYSIBM.SYSPACKAGE
                WHERE   COLLID    = :szCollId
                    AND NAME      = :szPlanName
                    AND ISOLATION = :chIsolation
                    AND VALID     = 'Y'
                    AND OPERATIVE = 'Y'
                    AND VALIDATE  = 'B'
                    AND LOCATION  = ' '
                    AND VERSION   = :szVersion
                    AND HEX(CONTOKEN) = :szTimeStampHex
                    AND OWNER     = USER
                    AND QUALIFIER = USER;
*/

{
#line 148 "Q61MVS_1.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 148 "Q61MVS_1.sqc"
  sqlaaloc(2,5,1,0L);
    {
      struct sqla_setd_list sql_setdlist[5];
#line 148 "Q61MVS_1.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 19;
#line 148 "Q61MVS_1.sqc"
      sql_setdlist[0].sqldata = (void*)szCollId;
#line 148 "Q61MVS_1.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 148 "Q61MVS_1.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 9;
#line 148 "Q61MVS_1.sqc"
      sql_setdlist[1].sqldata = (void*)szPlanName;
#line 148 "Q61MVS_1.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 148 "Q61MVS_1.sqc"
      sql_setdlist[2].sqltype = 452; sql_setdlist[2].sqllen = 1;
#line 148 "Q61MVS_1.sqc"
      sql_setdlist[2].sqldata = (void*)&chIsolation;
#line 148 "Q61MVS_1.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 148 "Q61MVS_1.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 65;
#line 148 "Q61MVS_1.sqc"
      sql_setdlist[3].sqldata = (void*)szVersion;
#line 148 "Q61MVS_1.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 148 "Q61MVS_1.sqc"
      sql_setdlist[4].sqltype = 460; sql_setdlist[4].sqllen = 17;
#line 148 "Q61MVS_1.sqc"
      sql_setdlist[4].sqldata = (void*)szTimeStampHex;
#line 148 "Q61MVS_1.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 148 "Q61MVS_1.sqc"
      sqlasetd(2,0,5,sql_setdlist,0L);
    }
#line 148 "Q61MVS_1.sqc"
  sqlaaloc(3,1,2,0L);
    {
      struct sqla_setd_list sql_setdlist[1];
#line 148 "Q61MVS_1.sqc"
      sql_setdlist[0].sqltype = 497; sql_setdlist[0].sqllen = 4;
#line 148 "Q61MVS_1.sqc"
      sql_setdlist[0].sqldata = (void*)&cCount;
#line 148 "Q61MVS_1.sqc"
      sql_setdlist[0].sqlind = &niCount;
#line 148 "Q61MVS_1.sqc"
      sqlasetd(3,0,1,sql_setdlist,0L);
    }
#line 148 "Q61MVS_1.sqc"
  sqlacall((unsigned short)24,1,2,3,0L);
#line 148 "Q61MVS_1.sqc"
  sqlastop(0L);
}

#line 148 "Q61MVS_1.sqc"

        }
        else if (szOwner[0] != '\0' && szQualifier[0] != '\0')
        {
            
/*
EXEC SQL
                SELECT  COUNT(*)
                INTO    :cCount :niCount
                FROM    SYSIBM.SYSPACKAGE
                WHERE   COLLID    = :szCollId
                    AND NAME      = :szPlanName
                    AND ISOLATION = :chIsolation
                    AND VALID     = 'Y'
                    AND OPERATIVE = 'Y'
                    AND VALIDATE  = 'B'
                    AND LOCATION  = ' '
                    AND VERSION   = :szVersion
                    AND HEX(CONTOKEN) = :szTimeStampHex
                    AND OWNER     = :szOwner
                    AND QUALIFIER = :szQualifier;
*/

{
#line 166 "Q61MVS_1.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 166 "Q61MVS_1.sqc"
  sqlaaloc(2,7,3,0L);
    {
      struct sqla_setd_list sql_setdlist[7];
#line 166 "Q61MVS_1.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 19;
#line 166 "Q61MVS_1.sqc"
      sql_setdlist[0].sqldata = (void*)szCollId;
#line 166 "Q61MVS_1.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 166 "Q61MVS_1.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 9;
#line 166 "Q61MVS_1.sqc"
      sql_setdlist[1].sqldata = (void*)szPlanName;
#line 166 "Q61MVS_1.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 166 "Q61MVS_1.sqc"
      sql_setdlist[2].sqltype = 452; sql_setdlist[2].sqllen = 1;
#line 166 "Q61MVS_1.sqc"
      sql_setdlist[2].sqldata = (void*)&chIsolation;
#line 166 "Q61MVS_1.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 166 "Q61MVS_1.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 65;
#line 166 "Q61MVS_1.sqc"
      sql_setdlist[3].sqldata = (void*)szVersion;
#line 166 "Q61MVS_1.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 166 "Q61MVS_1.sqc"
      sql_setdlist[4].sqltype = 460; sql_setdlist[4].sqllen = 17;
#line 166 "Q61MVS_1.sqc"
      sql_setdlist[4].sqldata = (void*)szTimeStampHex;
#line 166 "Q61MVS_1.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 166 "Q61MVS_1.sqc"
      sql_setdlist[5].sqltype = 460; sql_setdlist[5].sqllen = 9;
#line 166 "Q61MVS_1.sqc"
      sql_setdlist[5].sqldata = (void*)szOwner;
#line 166 "Q61MVS_1.sqc"
      sql_setdlist[5].sqlind = 0L;
#line 166 "Q61MVS_1.sqc"
      sql_setdlist[6].sqltype = 460; sql_setdlist[6].sqllen = 9;
#line 166 "Q61MVS_1.sqc"
      sql_setdlist[6].sqldata = (void*)szQualifier;
#line 166 "Q61MVS_1.sqc"
      sql_setdlist[6].sqlind = 0L;
#line 166 "Q61MVS_1.sqc"
      sqlasetd(2,0,7,sql_setdlist,0L);
    }
#line 166 "Q61MVS_1.sqc"
  sqlaaloc(3,1,4,0L);
    {
      struct sqla_setd_list sql_setdlist[1];
#line 166 "Q61MVS_1.sqc"
      sql_setdlist[0].sqltype = 497; sql_setdlist[0].sqllen = 4;
#line 166 "Q61MVS_1.sqc"
      sql_setdlist[0].sqldata = (void*)&cCount;
#line 166 "Q61MVS_1.sqc"
      sql_setdlist[0].sqlind = &niCount;
#line 166 "Q61MVS_1.sqc"
      sqlasetd(3,0,1,sql_setdlist,0L);
    }
#line 166 "Q61MVS_1.sqc"
  sqlacall((unsigned short)24,2,2,3,0L);
#line 166 "Q61MVS_1.sqc"
  sqlastop(0L);
}

#line 166 "Q61MVS_1.sqc"

        }
        else if (szOwner[0] != '\0')
        {
            
/*
EXEC SQL
                SELECT  COUNT(*)
                INTO    :cCount :niCount
                FROM    SYSIBM.SYSPACKAGE
                WHERE   COLLID    = :szCollId
                    AND NAME      = :szPlanName
                    AND ISOLATION = :chIsolation
                    AND VALID     = 'Y'
                    AND OPERATIVE = 'Y'
                    AND VALIDATE  = 'B'
                    AND LOCATION  = ' '
                    AND VERSION   = :szVersion
                    AND HEX(CONTOKEN) = :szTimeStampHex
                    AND OWNER     = :szOwner
                    AND QUALIFIER = :szOwner;
*/

{
#line 184 "Q61MVS_1.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 184 "Q61MVS_1.sqc"
  sqlaaloc(2,7,5,0L);
    {
      struct sqla_setd_list sql_setdlist[7];
#line 184 "Q61MVS_1.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 19;
#line 184 "Q61MVS_1.sqc"
      sql_setdlist[0].sqldata = (void*)szCollId;
#line 184 "Q61MVS_1.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 184 "Q61MVS_1.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 9;
#line 184 "Q61MVS_1.sqc"
      sql_setdlist[1].sqldata = (void*)szPlanName;
#line 184 "Q61MVS_1.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 184 "Q61MVS_1.sqc"
      sql_setdlist[2].sqltype = 452; sql_setdlist[2].sqllen = 1;
#line 184 "Q61MVS_1.sqc"
      sql_setdlist[2].sqldata = (void*)&chIsolation;
#line 184 "Q61MVS_1.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 184 "Q61MVS_1.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 65;
#line 184 "Q61MVS_1.sqc"
      sql_setdlist[3].sqldata = (void*)szVersion;
#line 184 "Q61MVS_1.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 184 "Q61MVS_1.sqc"
      sql_setdlist[4].sqltype = 460; sql_setdlist[4].sqllen = 17;
#line 184 "Q61MVS_1.sqc"
      sql_setdlist[4].sqldata = (void*)szTimeStampHex;
#line 184 "Q61MVS_1.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 184 "Q61MVS_1.sqc"
      sql_setdlist[5].sqltype = 460; sql_setdlist[5].sqllen = 9;
#line 184 "Q61MVS_1.sqc"
      sql_setdlist[5].sqldata = (void*)szOwner;
#line 184 "Q61MVS_1.sqc"
      sql_setdlist[5].sqlind = 0L;
#line 184 "Q61MVS_1.sqc"
      sql_setdlist[6].sqltype = 460; sql_setdlist[6].sqllen = 9;
#line 184 "Q61MVS_1.sqc"
      sql_setdlist[6].sqldata = (void*)szOwner;
#line 184 "Q61MVS_1.sqc"
      sql_setdlist[6].sqlind = 0L;
#line 184 "Q61MVS_1.sqc"
      sqlasetd(2,0,7,sql_setdlist,0L);
    }
#line 184 "Q61MVS_1.sqc"
  sqlaaloc(3,1,6,0L);
    {
      struct sqla_setd_list sql_setdlist[1];
#line 184 "Q61MVS_1.sqc"
      sql_setdlist[0].sqltype = 497; sql_setdlist[0].sqllen = 4;
#line 184 "Q61MVS_1.sqc"
      sql_setdlist[0].sqldata = (void*)&cCount;
#line 184 "Q61MVS_1.sqc"
      sql_setdlist[0].sqlind = &niCount;
#line 184 "Q61MVS_1.sqc"
      sqlasetd(3,0,1,sql_setdlist,0L);
    }
#line 184 "Q61MVS_1.sqc"
  sqlacall((unsigned short)24,3,2,3,0L);
#line 184 "Q61MVS_1.sqc"
  sqlastop(0L);
}

#line 184 "Q61MVS_1.sqc"

        }
        else
        {
            
/*
EXEC SQL
                SELECT  COUNT(*)
                INTO    :cCount :niCount
                FROM    SYSIBM.SYSPACKAGE
                WHERE   COLLID    = :szCollId
                    AND NAME      = :szPlanName
                    AND ISOLATION = :chIsolation
                    AND VALID     = 'Y'
                    AND OPERATIVE = 'Y'
                    AND VALIDATE  = 'B'
                    AND LOCATION  = ' '
                    AND VERSION   = :szVersion
                    AND HEX(CONTOKEN) = :szTimeStampHex
                    AND OWNER     = USER
                    AND QUALIFIER = :szQualifier;
*/

{
#line 202 "Q61MVS_1.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 202 "Q61MVS_1.sqc"
  sqlaaloc(2,6,7,0L);
    {
      struct sqla_setd_list sql_setdlist[6];
#line 202 "Q61MVS_1.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 19;
#line 202 "Q61MVS_1.sqc"
      sql_setdlist[0].sqldata = (void*)szCollId;
#line 202 "Q61MVS_1.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 202 "Q61MVS_1.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 9;
#line 202 "Q61MVS_1.sqc"
      sql_setdlist[1].sqldata = (void*)szPlanName;
#line 202 "Q61MVS_1.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 202 "Q61MVS_1.sqc"
      sql_setdlist[2].sqltype = 452; sql_setdlist[2].sqllen = 1;
#line 202 "Q61MVS_1.sqc"
      sql_setdlist[2].sqldata = (void*)&chIsolation;
#line 202 "Q61MVS_1.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 202 "Q61MVS_1.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 65;
#line 202 "Q61MVS_1.sqc"
      sql_setdlist[3].sqldata = (void*)szVersion;
#line 202 "Q61MVS_1.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 202 "Q61MVS_1.sqc"
      sql_setdlist[4].sqltype = 460; sql_setdlist[4].sqllen = 17;
#line 202 "Q61MVS_1.sqc"
      sql_setdlist[4].sqldata = (void*)szTimeStampHex;
#line 202 "Q61MVS_1.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 202 "Q61MVS_1.sqc"
      sql_setdlist[5].sqltype = 460; sql_setdlist[5].sqllen = 9;
#line 202 "Q61MVS_1.sqc"
      sql_setdlist[5].sqldata = (void*)szQualifier;
#line 202 "Q61MVS_1.sqc"
      sql_setdlist[5].sqlind = 0L;
#line 202 "Q61MVS_1.sqc"
      sqlasetd(2,0,6,sql_setdlist,0L);
    }
#line 202 "Q61MVS_1.sqc"
  sqlaaloc(3,1,8,0L);
    {
      struct sqla_setd_list sql_setdlist[1];
#line 202 "Q61MVS_1.sqc"
      sql_setdlist[0].sqltype = 497; sql_setdlist[0].sqllen = 4;
#line 202 "Q61MVS_1.sqc"
      sql_setdlist[0].sqldata = (void*)&cCount;
#line 202 "Q61MVS_1.sqc"
      sql_setdlist[0].sqlind = &niCount;
#line 202 "Q61MVS_1.sqc"
      sqlasetd(3,0,1,sql_setdlist,0L);
    }
#line 202 "Q61MVS_1.sqc"
  sqlacall((unsigned short)24,4,2,3,0L);
#line 202 "Q61MVS_1.sqc"
  sqlastop(0L);
}

#line 202 "Q61MVS_1.sqc"

        }

        if (    sqlca.sqlcode != SQL_RC_E818
            &&  sqlca.sqlcode != SQL_RC_E805)
            break;

        /*
         * Bind package of this program.
         */
        if (BindResource(0, 805, 2))
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
int  BindCheckPrgId_61MVS(const char *pszSqlUser, const char *pszPlanName, const char *pszTimeStamp)
{
    
/*
EXEC SQL BEGIN DECLARE SECTION;
*/

#line 239 "Q61MVS_1.sqc"

    char    szTimeStampHex2[17];
    char    szSqlUser[19];
    char    szPlanName2[9];
    long    cCount2;
    short   niCount2;
    
/*
EXEC SQL END DECLARE SECTION;
*/

#line 245 "Q61MVS_1.sqc"

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
    if (strlen(pszSqlUser) > 18)
    {
        BindError("sqluser is too long! max 18. (%s)\n", pszSqlUser);
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
            FROM    SYSIBM.SYSPACKAGE
            WHERE   NAME      = :szPlanName2
                AND COLLID    = :szSqlUser
                AND VALID     = 'Y'
                AND OPERATIVE = 'Y'
                AND HEX(CONTOKEN) = :szTimeStampHex2;
*/

{
#line 295 "Q61MVS_1.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 295 "Q61MVS_1.sqc"
  sqlaaloc(2,3,9,0L);
    {
      struct sqla_setd_list sql_setdlist[3];
#line 295 "Q61MVS_1.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 9;
#line 295 "Q61MVS_1.sqc"
      sql_setdlist[0].sqldata = (void*)szPlanName2;
#line 295 "Q61MVS_1.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 295 "Q61MVS_1.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 19;
#line 295 "Q61MVS_1.sqc"
      sql_setdlist[1].sqldata = (void*)szSqlUser;
#line 295 "Q61MVS_1.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 295 "Q61MVS_1.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 17;
#line 295 "Q61MVS_1.sqc"
      sql_setdlist[2].sqldata = (void*)szTimeStampHex2;
#line 295 "Q61MVS_1.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 295 "Q61MVS_1.sqc"
      sqlasetd(2,0,3,sql_setdlist,0L);
    }
#line 295 "Q61MVS_1.sqc"
  sqlaaloc(3,1,10,0L);
    {
      struct sqla_setd_list sql_setdlist[1];
#line 295 "Q61MVS_1.sqc"
      sql_setdlist[0].sqltype = 497; sql_setdlist[0].sqllen = 4;
#line 295 "Q61MVS_1.sqc"
      sql_setdlist[0].sqldata = (void*)&cCount2;
#line 295 "Q61MVS_1.sqc"
      sql_setdlist[0].sqlind = &niCount2;
#line 295 "Q61MVS_1.sqc"
      sqlasetd(3,0,1,sql_setdlist,0L);
    }
#line 295 "Q61MVS_1.sqc"
  sqlacall((unsigned short)24,5,2,3,0L);
#line 295 "Q61MVS_1.sqc"
  sqlastop(0L);
}

#line 295 "Q61MVS_1.sqc"


        if (    sqlca.sqlcode != SQL_RC_E818
            &&  sqlca.sqlcode != SQL_RC_E805)
            break;

        /*
         * Bind package of this program.
         */
        if (BindResource(0, 805, 2))
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
int  BindGetInfo_61MVS(/* input */
                       const char * pszCollId,
                       const char * pszName,
                       const char * pszTimeStamp,
                       const char * pszVersion,
                       /* output */
                       char       * pszCreator,
                       char       * pszQualifier,
                       char       * pszOwner,
                       char       * pchValid,
                       char       * pchOperative,
                       char       * pchValidate,
                       char       * pchBlocking,
                       int        * piIsolation)
{
    
/*
EXEC SQL BEGIN DECLARE SECTION;
*/

#line 346 "Q61MVS_1.sqc"

    char    szVersionI[65];
    char    szCollIdI[19];
    char    szNameI[9];
    char    szTimeStampHexI[17];

    char    chValidI;
    char    chIsolationI;
    char    szCreatorI[9];
    char    szOwnerI[9];
    char    chOperativeI;
    char    chValidateI;
    char    szQualifierI[9];
    
/*
EXEC SQL END DECLARE SECTION;
*/

#line 359 "Q61MVS_1.sqc"

    int     cTries = 2;
    char *  psz;
    char *  pszTS;

    /*
     * Validate input.
     */
    if (strlen(pszVersion) > 64)
    {
        BindError("version is too long! max 64. (%s)\n", pszVersion);
        return 1;
    }
    if (strlen(pszName) > 8)
    {
        BindError("planname is too long! max 8. (%s)\n", pszName);
        return 1;
    }
    if (strlen(pszCollId) > 18)
    {
        BindError("collid is too long! max 18. (%s)\n", pszCollId);
        return 1;
    }
    if (strlen(pszTimeStamp) != 8)
    {
        BindError("Timestamp is not 8 chars long! (%s)\n", pszTimeStamp);
        return 1;
    }


    /*
     * Fill hostvariables.
     */
    strcpy(szCollIdI,   pszCollId);
    strcpy(szNameI,     pszName);
    for (psz = szTimeStampHexI, pszTimeStamp; *pszTimeStamp; psz+=2, pszTimeStamp++)
        _itoa(*pszTimeStamp, psz, 16);
    strupr(szTimeStampHexI);
    strcpy(szVersionI,  pszVersion);


    /*
     * Do bind. (We loop if we need to bind ourselfs.)
     */
    do
    {
        
/*
EXEC SQL
            SELECT  VALID,
                    ISOLATION,
                    CREATOR,
                    OWNER,
                    OPERATIVE,
                    VALIDATE,
                    QUALIFIER
            INTO    :chValidI,
                    :chIsolationI,
                    :szCreatorI,
                    :szOwnerI,
                    :chOperativeI,
                    :chValidateI,
                    :szQualifierI
            FROM    SYSIBM.SYSPACKAGE
            WHERE   COLLID    = :szCollIdI
                AND NAME      = :szNameI
                AND VERSION   = :szVersionI
                AND HEX(CONTOKEN) = :szTimeStampHexI
                AND LOCATION  = ' ';
*/

{
#line 425 "Q61MVS_1.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 425 "Q61MVS_1.sqc"
  sqlaaloc(2,4,11,0L);
    {
      struct sqla_setd_list sql_setdlist[4];
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 19;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[0].sqldata = (void*)szCollIdI;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 9;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[1].sqldata = (void*)szNameI;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 65;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[2].sqldata = (void*)szVersionI;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 17;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[3].sqldata = (void*)szTimeStampHexI;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 425 "Q61MVS_1.sqc"
      sqlasetd(2,0,4,sql_setdlist,0L);
    }
#line 425 "Q61MVS_1.sqc"
  sqlaaloc(3,7,12,0L);
    {
      struct sqla_setd_list sql_setdlist[7];
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[0].sqltype = 452; sql_setdlist[0].sqllen = 1;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[0].sqldata = (void*)&chValidI;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[1].sqltype = 452; sql_setdlist[1].sqllen = 1;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[1].sqldata = (void*)&chIsolationI;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 9;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[2].sqldata = (void*)szCreatorI;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 9;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[3].sqldata = (void*)szOwnerI;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[4].sqltype = 452; sql_setdlist[4].sqllen = 1;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[4].sqldata = (void*)&chOperativeI;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[4].sqlind = 0L;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[5].sqltype = 452; sql_setdlist[5].sqllen = 1;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[5].sqldata = (void*)&chValidateI;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[5].sqlind = 0L;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[6].sqltype = 460; sql_setdlist[6].sqllen = 9;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[6].sqldata = (void*)szQualifierI;
#line 425 "Q61MVS_1.sqc"
      sql_setdlist[6].sqlind = 0L;
#line 425 "Q61MVS_1.sqc"
      sqlasetd(3,0,7,sql_setdlist,0L);
    }
#line 425 "Q61MVS_1.sqc"
  sqlacall((unsigned short)24,6,2,3,0L);
#line 425 "Q61MVS_1.sqc"
  sqlastop(0L);
}

#line 425 "Q61MVS_1.sqc"


        if (    sqlca.sqlcode != SQL_RC_E818
            &&  sqlca.sqlcode != SQL_RC_E805)
            break;

        /*
         * Bind package of this program.
         */
        if (BindResource(0, 805, 2))
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
        if (piIsolation)
        {
            if (chIsolationI == 'U')
                *piIsolation = SQL_NO_COMMIT;
            /*else if (chIsolationI == 'U')
                *piIsolation = SQL_UNCOM_READ;*/
            else if (chIsolationI == 'T')
                *piIsolation = SQL_READ_STAB;
            else if (chIsolationI == 'R')
                *piIsolation = SQL_REP_READ;
            else if (chIsolationI == 'S')
                *piIsolation = SQL_CURSOR_STAB;
            else
            {
                BindIntError("Isolation lever is unknown. ISOLATION='%c'\n", chIsolationI);
                *piIsolation = -1;
            }
        }
        if (pszCreator)
            strcpy(pszCreator, szCreatorI);
        if (pszOwner)
            strcpy(pszOwner, szOwnerI);
        if (pchOperative)
            *pchOperative = chOperativeI;
        if (pchValidate)
            *pchValidate = chValidateI;
        if (pszQualifier)
            strcpy(pszQualifier, szQualifierI);

        if (pchBlocking)
            *pchBlocking = '\0'; /* ?? */
    }

    return sqlca.sqlcode == SQL_RC_OK;
}

