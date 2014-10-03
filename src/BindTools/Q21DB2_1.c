static char sqla_program_id[40] = 
{111,65,65,66,65,71,65,68,66,73,78,68,84,79,79,76,81,50,49,68,
66,50,95,49,50,65,100,76,82,69,77,82,48,49,49,49,49,32,32,32};


#include "sqladef.h"

static struct sqla_runtime_info sqla_rtinfo = 
{{'S','Q','L','A','R','T','I','N'}, sizeof(wchar_t), 0, {' ',' ',' ',' '}};


#line 1 "Q21DB2_1.sqc"
/* $Id: Q21DB2_1.c,v 1.6 2001/12/04 17:04:49 bird Exp $
 *
 * Queries for DB2 2.12, and other PC based versions...
 *
 * Copyright (c) 2001 knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 *
 * GPL.
 *
 */

/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include <string.h>
#include <stdlib.h>
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
 * @param   pszCreator      Pointer to the collection/creator/sqluser string.
 * @param   pszPlanName     Pointer to the application/planname string.
 * @param   pszTimeStamp    Pointer to the timestamp string.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 * @remark  The check isn't necesarrily sufficent for bind packages
 *          which is using uncommon fields.
 *          I need feedback on this...
 */
int  BindItCheck_21DB2(const char *pszCreator, const char *pszPlanName,
                       const char *pszTimeStamp, const char *pszVersion,
                       const char *pszCollId, const char *pszQualifier,
                       const char *pszOwner, int iIsolation)
{
    
/*
EXEC SQL BEGIN DECLARE SECTION;
*/

#line 48 "Q21DB2_1.sqc"

    char    szCreator[9];
    char    szPlanName[9];
    char    szTimeStamp[9];
    char    szIsolation[3];
    long    cCount;
    short   niCount;
    
/*
EXEC SQL END DECLARE SECTION;
*/

#line 55 "Q21DB2_1.sqc"

    int     cTries = 1;

    pszVersion = pszVersion;
    pszCollId = pszCollId;
    pszQualifier = pszQualifier;
    pszOwner = pszOwner;

    /*
     * Validate input.
     */
    if (strlen(pszTimeStamp) != 8)
    {
        BindError("Timestamp is not 8 chars long! (%s)\n", pszTimeStamp);
        return 0;
    }
    if (strlen(pszPlanName) > 8)
    {
        BindError("planname is too long! max 8. (%s)\n", pszPlanName);
        return 0;
    }
    if (strlen(pszCreator) > 9)
    {
        BindError("creator is too long! max 8. (%s)\n", pszCreator);
        return 0;
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
    strcpy(szCreator, pszCreator);
    strcpy(szPlanName, pszPlanName);
    strcpy(szTimeStamp, pszTimeStamp);


    /*
     * Do bind. (We loop if we need to bind ourselfs.)
     */
    do
        {
        cCount = 0;
        niCount = -1;
        
/*
EXEC SQL
            SELECT  COUNT(*)
            INTO    :cCount :niCount
            FROM    SYSCAT.PACKAGES
            WHERE   PKGSCHEMA = :szCreator
                AND PKGNAME   = :szPlanName
                AND UNIQUE_ID = :szTimeStamp
                AND ISOLATION = :szIsolation
                AND BLOCKING  = 'B'
                AND VALID     = 'Y';
*/

{
#line 119 "Q21DB2_1.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 119 "Q21DB2_1.sqc"
  sqlaaloc(2,4,1,0L);
    {
      struct sqla_setd_list sql_setdlist[4];
#line 119 "Q21DB2_1.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 9;
#line 119 "Q21DB2_1.sqc"
      sql_setdlist[0].sqldata = (void*)szCreator;
#line 119 "Q21DB2_1.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 119 "Q21DB2_1.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 9;
#line 119 "Q21DB2_1.sqc"
      sql_setdlist[1].sqldata = (void*)szPlanName;
#line 119 "Q21DB2_1.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 119 "Q21DB2_1.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 9;
#line 119 "Q21DB2_1.sqc"
      sql_setdlist[2].sqldata = (void*)szTimeStamp;
#line 119 "Q21DB2_1.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 119 "Q21DB2_1.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 3;
#line 119 "Q21DB2_1.sqc"
      sql_setdlist[3].sqldata = (void*)szIsolation;
#line 119 "Q21DB2_1.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 119 "Q21DB2_1.sqc"
      sqlasetd(2,0,4,sql_setdlist,0L);
    }
#line 119 "Q21DB2_1.sqc"
  sqlaaloc(3,1,2,0L);
    {
      struct sqla_setd_list sql_setdlist[1];
#line 119 "Q21DB2_1.sqc"
      sql_setdlist[0].sqltype = 497; sql_setdlist[0].sqllen = 4;
#line 119 "Q21DB2_1.sqc"
      sql_setdlist[0].sqldata = (void*)&cCount;
#line 119 "Q21DB2_1.sqc"
      sql_setdlist[0].sqlind = &niCount;
#line 119 "Q21DB2_1.sqc"
      sqlasetd(3,0,1,sql_setdlist,0L);
    }
#line 119 "Q21DB2_1.sqc"
  sqlacall((unsigned short)24,1,2,3,0L);
#line 119 "Q21DB2_1.sqc"
  sqlastop(0L);
}

#line 119 "Q21DB2_1.sqc"


        if (    (   sqlca.sqlcode != SQL_RC_E818
                 && sqlca.sqlcode != SQL_RC_E805)
            ||  cTries <= 0)
            break;

        /*
         * Bind package of this program.
         */
        if (BindResource(0, 805, 1))
            {
            BindFatalError("Selfbind failed! " __FILE__); fflush(stdout);
            exit(-805);
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
int  BindCheckPrgId_21DB2(const char *pszSqlUser, const char *pszPlanName, const char *pszTimeStamp)
{
    
/*
EXEC SQL BEGIN DECLARE SECTION;
*/

#line 153 "Q21DB2_1.sqc"

    char    szTimeStampHex2[17];
    char    szSqlUser[9];
    char    szPlanName2[9];
    long    cCount2;
    short   niCount2;
    
/*
EXEC SQL END DECLARE SECTION;
*/

#line 159 "Q21DB2_1.sqc"

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
    if (strlen(pszSqlUser) > 8)
    {
        BindError("sqluser is too long! max 8. (%s)\n", pszSqlUser);
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
            FROM    SYSCAT.PACKAGES
            WHERE   PKGNAME   = :szPlanName2
                AND PKGSCHEMA = :szSqlUser
                AND VALID     = 'Y'
                AND HEX(UNIQUE_ID) = :szTimeStampHex2;
*/

{
#line 208 "Q21DB2_1.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 208 "Q21DB2_1.sqc"
  sqlaaloc(2,3,3,0L);
    {
      struct sqla_setd_list sql_setdlist[3];
#line 208 "Q21DB2_1.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 9;
#line 208 "Q21DB2_1.sqc"
      sql_setdlist[0].sqldata = (void*)szPlanName2;
#line 208 "Q21DB2_1.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 208 "Q21DB2_1.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 9;
#line 208 "Q21DB2_1.sqc"
      sql_setdlist[1].sqldata = (void*)szSqlUser;
#line 208 "Q21DB2_1.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 208 "Q21DB2_1.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 17;
#line 208 "Q21DB2_1.sqc"
      sql_setdlist[2].sqldata = (void*)szTimeStampHex2;
#line 208 "Q21DB2_1.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 208 "Q21DB2_1.sqc"
      sqlasetd(2,0,3,sql_setdlist,0L);
    }
#line 208 "Q21DB2_1.sqc"
  sqlaaloc(3,1,4,0L);
    {
      struct sqla_setd_list sql_setdlist[1];
#line 208 "Q21DB2_1.sqc"
      sql_setdlist[0].sqltype = 497; sql_setdlist[0].sqllen = 4;
#line 208 "Q21DB2_1.sqc"
      sql_setdlist[0].sqldata = (void*)&cCount2;
#line 208 "Q21DB2_1.sqc"
      sql_setdlist[0].sqlind = &niCount2;
#line 208 "Q21DB2_1.sqc"
      sqlasetd(3,0,1,sql_setdlist,0L);
    }
#line 208 "Q21DB2_1.sqc"
  sqlacall((unsigned short)24,2,2,3,0L);
#line 208 "Q21DB2_1.sqc"
  sqlastop(0L);
}

#line 208 "Q21DB2_1.sqc"


        if (    sqlca.sqlcode != SQL_RC_E818
            &&  sqlca.sqlcode != SQL_RC_E805)
            break;

        /*
         * Bind package of this program.
         */
        if (BindResource(0, 805, 1))
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
 * @param   *               Output fields.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 * @remark  The check isn't necesarrily sufficent for bind packages
 *          which is using uncommon fields.
 *          I need feedback on this...
 */
int  BindGetInfo_21DB2(/* input */
                       const char * pszCreator,
                       const char * pszPlanName,
                       const char * pszTimeStamp,
                       const char * pszVersion,
                       /* output */
                       char       * pszCollId,
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

#line 258 "Q21DB2_1.sqc"

    char    szCreatorI[9];
    char    szPlanNameI[9];
    char    szTimeStampI[9];

    char    szBoundByI[9];
    char    szIsolationI[3];
    char    chValidI;
    char    chBlockingI;
    
/*
EXEC SQL END DECLARE SECTION;
*/

#line 267 "Q21DB2_1.sqc"

    int     cTries = 1;

    pszVersion = pszVersion;
    pszCollId = pszCollId;
    pszQualifier = pszQualifier;


    /*
     * Validate input.
     */
    if (strlen(pszTimeStamp) != 8)
    {
        BindError("Timestamp is not 8 chars long! (%s)\n", pszTimeStamp);
        return 0;
    }
    if (strlen(pszPlanName) > 8)
    {
        BindError("planname is too long! max 8. (%s)\n", pszPlanName);
        return 0;
    }
    if (strlen(pszCreator) > 9)
    {
        BindError("creator is too long! max 8. (%s)\n", pszCreator);
        return 0;
    }


    /*
     * Fill hostvariables.
     */
    strcpy(szCreatorI, pszCreator);
    strcpy(szPlanNameI, pszPlanName);
    strcpy(szTimeStampI, pszTimeStamp);


    /*
     * Do bind. (We loop if we need to bind ourselfs.)
     */
    do
    {
        chValidI = chBlockingI = szIsolationI[0] = '\0';
        
/*
EXEC SQL
            SELECT  VALID,
                    BLOCKING,
                    ISOLATION,
                    BOUNDBY
            INTO    :chValidI,
                    :chBlockingI,
                    :szIsolationI,
                    :szBoundByI
            FROM    SYSCAT.PACKAGES
            WHERE   PKGSCHEMA = :szCreatorI
                AND PKGNAME   = :szPlanNameI
                AND UNIQUE_ID = :szTimeStampI;
*/

{
#line 321 "Q21DB2_1.sqc"
  sqlastrt(sqla_program_id, &sqla_rtinfo, &sqlca);
#line 321 "Q21DB2_1.sqc"
  sqlaaloc(2,3,5,0L);
    {
      struct sqla_setd_list sql_setdlist[3];
#line 321 "Q21DB2_1.sqc"
      sql_setdlist[0].sqltype = 460; sql_setdlist[0].sqllen = 9;
#line 321 "Q21DB2_1.sqc"
      sql_setdlist[0].sqldata = (void*)szCreatorI;
#line 321 "Q21DB2_1.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 321 "Q21DB2_1.sqc"
      sql_setdlist[1].sqltype = 460; sql_setdlist[1].sqllen = 9;
#line 321 "Q21DB2_1.sqc"
      sql_setdlist[1].sqldata = (void*)szPlanNameI;
#line 321 "Q21DB2_1.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 321 "Q21DB2_1.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 9;
#line 321 "Q21DB2_1.sqc"
      sql_setdlist[2].sqldata = (void*)szTimeStampI;
#line 321 "Q21DB2_1.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 321 "Q21DB2_1.sqc"
      sqlasetd(2,0,3,sql_setdlist,0L);
    }
#line 321 "Q21DB2_1.sqc"
  sqlaaloc(3,4,6,0L);
    {
      struct sqla_setd_list sql_setdlist[4];
#line 321 "Q21DB2_1.sqc"
      sql_setdlist[0].sqltype = 452; sql_setdlist[0].sqllen = 1;
#line 321 "Q21DB2_1.sqc"
      sql_setdlist[0].sqldata = (void*)&chValidI;
#line 321 "Q21DB2_1.sqc"
      sql_setdlist[0].sqlind = 0L;
#line 321 "Q21DB2_1.sqc"
      sql_setdlist[1].sqltype = 452; sql_setdlist[1].sqllen = 1;
#line 321 "Q21DB2_1.sqc"
      sql_setdlist[1].sqldata = (void*)&chBlockingI;
#line 321 "Q21DB2_1.sqc"
      sql_setdlist[1].sqlind = 0L;
#line 321 "Q21DB2_1.sqc"
      sql_setdlist[2].sqltype = 460; sql_setdlist[2].sqllen = 3;
#line 321 "Q21DB2_1.sqc"
      sql_setdlist[2].sqldata = (void*)szIsolationI;
#line 321 "Q21DB2_1.sqc"
      sql_setdlist[2].sqlind = 0L;
#line 321 "Q21DB2_1.sqc"
      sql_setdlist[3].sqltype = 460; sql_setdlist[3].sqllen = 9;
#line 321 "Q21DB2_1.sqc"
      sql_setdlist[3].sqldata = (void*)szBoundByI;
#line 321 "Q21DB2_1.sqc"
      sql_setdlist[3].sqlind = 0L;
#line 321 "Q21DB2_1.sqc"
      sqlasetd(3,0,4,sql_setdlist,0L);
    }
#line 321 "Q21DB2_1.sqc"
  sqlacall((unsigned short)24,3,2,3,0L);
#line 321 "Q21DB2_1.sqc"
  sqlastop(0L);
}

#line 321 "Q21DB2_1.sqc"


        if (    (   sqlca.sqlcode != SQL_RC_E818
                 && sqlca.sqlcode != SQL_RC_E805)
            ||  cTries <= 0)
            break;

        /*
         * Bind package of this program.
         */
        if (BindResource(0, 805, 1))
        {
            BindFatalError("Selfbind failed! " __FILE__); fflush(stdout);
            exit(-805);
        }
    } while (cTries--);


    if (sqlca.sqlcode == SQL_RC_OK)
    {
        if (pchValid)
            *pchValid = chValidI;
        if (pchBlocking)
            *pchBlocking = chBlockingI;
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
            strcpy(pszBoundBy, szBoundByI);
        if (pszCollId)
            strcpy(pszCollId, szCreatorI); /* ?? */
        if (pszQualifier)
            *pszQualifier = '\0';
        if (pchOperative)
            *pchOperative = chValidI;   /* ?? */
        if (pchValidate)
            *pchValidate = 'B';         /* ?? */
    }

    return sqlca.sqlcode == SQL_RC_OK;
}




