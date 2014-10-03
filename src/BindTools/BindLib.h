/* $Id: BindLib.h,v 1.16 2001/12/04 17:04:48 bird Exp $
 *
 * Bind Helper Library Header.
 *
 * Copyright (c) 2001 knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 *
 * GPL
 *
 */

#ifndef _BindLib_h_
#define _BindLib_h_

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Timestamp structure.
 */
typedef struct _TimeStamp
{
    int     iYear;
    int     iMonth;
    int     iDay;
    int     iHour;
    int     iMinutte;
    int     iSecond;
    int     i100;
} TIMESTAMP, *PTIMESTAMP;

int     BindCvtTS(const char *pszTimeStamp, PTIMESTAMP pTS);
int     BindMakeTS(char *pszTimeStamp, PTIMESTAMP pTS);
int     BindMakeTSISO(char *pszTimeStamp, const char *pszISO);
int     BindToNum(const char *pszNum, int cchNum);

char *  BindTmpFile(char *pszFileBuf, const char *pszExt);
int     BindResource(unsigned long hmod, unsigned long idType, unsigned long idName);

/*
 * Database info.
 */
typedef struct _DBInfo
{
    int     iDB2Major;
    int     iDB2Midi;
    int     iDB2Minor;
    int     fMVS;
} DBINFO , *PDBINFO;
int     BindDBConnect(const char *pszDatabase, PDBINFO pDbInfo);
int     BindDBDisconnect(const char *pszDatabase);


/*
 * Check bind package functions.
 */
int     BindItCheck_21DB2(const char *pszCreator, const char *pszPlanName,
                          const char *pszTimeStamp, const char *pszVersion,
                          const char *pszCollId, const char *pszQualifier,
                          const char *pszOwner, int fCS);
int     BindItCheck_61MVS(const char *pszCreator, const char *pszPlanName,
                          const char *pszTimeStamp, const char *pszVersion,
                          const char *pszCollId, const char *pszQualifier,
                          const char *pszOwner, int fCS);
int     BindItCheck_71UDB(const char *pszCreator, const char *pszPlanName,
                          const char *pszTimeStamp, const char *pszVersion,
                          const char *pszCollId, const char *pszQualifier,
                          const char *pszOwner, int fCS);

int     BindCheckPrgId_21DB2(const char *pszSqlUser, const char *pszPlanName, const char *pszTimeStamp);
int     BindCheckPrgId_61MVS(const char *pszSqlUser, const char *pszPlanName, const char *pszTimeStamp);
int     BindCheckPrgId_71UDB(const char *pszSqlUser, const char *pszPlanName, const char *pszTimeStamp);


int     BindGetInfo_21DB2(/* input */
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
                        int        * piIsolation);
int    BindGetInfo_61MVS(/* input */
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
                        int        * piIsolation);
int    BindGetInfo_71UDB(/* input */
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
                        int        * piIsolation);


/*
 * Argument helpers.
 */
int     BindGetOption(int argc, char **argv, int *pargi, char *pszOption, int cchMax, int *pfOption,
                      int fClearAllowed, const char *pszDesc, const char *pszVerbose, int cVerbose);
/* Option flags. */
#define OPTION_CLEAR    -1
#define OPTION_SET      1
#define OPTION_IGNORE   0


/*
 * Message handling.
 */
void    BindMsgInit(void);
void    BindMsgTerm(void);
int     BindMsgOption(int argc, char **argv, int *pargi, int cVerbose);
int     BindError(const char *pszFormat, ...);
int     BindFatalError(const char *pszFormat, ...);
int     BindIntError(const char *pszFormat, ...);
int     BindSyntaxError(const char *pszFormat, ...);
int     BindWarning(const char *pszFormat, ...);
int     BindInfo(const char *pszFormat, ...);
int     BindStatus(const char *pszFormat, ...);
void    BindResetMsgBuffer(void);
int     BindGetMsgBuffer(char *pszBuffer, int cchBuffer);
void    BindErrorSummary(const char *pszFile);
void    BindResetErrorBuffer(void);
int     BindGetErrorBuffer(char *pszBuffer, int cchBuffer);
int     BindDelayedReset(void);
int     BindDelayedError(const char *pszFormat, ...);
int     BindDelayedWarning(const char *pszFormat, ...);
int     BindDelayedInfo(const char *pszFormat, ...);
int     BindDelayedFlush(void);



#ifdef __cplusplus
}
#endif
#endif /* _BINDCMN_H_ */

