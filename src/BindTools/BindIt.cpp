/* $Id: BindIt.cpp,v 1.18 2002/03/14 15:27:17 bird Exp $
 *
 * Bind utility - should be faster than most others...
 *
 * Copyright (c) 2001-2002 knut st. osmundsen (bird@anduin.net)
 *
 * GPL.
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "Bind.h"
#include "BindVer.h"
#include "BindLib.h"

#include <kTypes.h>
#include <kError.h>
#include <kFile.h>
#include <kFileFormatBase.h>
#include <kFileBnd.h>


/*******************************************************************************
*   Global Variables                                                           *
*******************************************************************************/
/*
 * Statistics.
 */
ULONG cTotalFiles = 0;
ULONG cBindFiles  = 0;
ULONG cSuccess    = 0;
ULONG cErrors     = 0;


/*
 * Options.
 */
typedef union _mySQLCHAR
{
    SQL_STRUCTURE sqlchar   s;
    char                    dummy[133];
} MYSQLCHAR, *PMYSQLCHAR;

static struct _OptionsBindIt
{
    DBINFO      DBInfo;
    char        szDatabase[32];
    KBOOL       fExplain;
    KBOOL       fForceAll;
    int         cVerbose;
    KBOOL       fValidateBind;
    int         iIsolation;
    KBOOL       fTest;
    int         fSummary;
    char        szSummary[CCHMAXPATH];
    MYSQLCHAR   sSqlOwner;
    MYSQLCHAR   sSqlQualifier;
    MYSQLCHAR   sSqlCollid;
    KBOOL       fInsertBuf;
} options =
{
     {0,0,0,0},
     "",                                /* szDbAlias     */
     FALSE,                             /* fExplain      */
     FALSE,                             /* fForceAll     */
     2,                                 /* cVerbose      */
     TRUE,                              /* fValidateBind */
     SQL_CURSOR_STAB,                   /* iIsolation    */
     FALSE,                             /* fTest         */
     OPTION_IGNORE,                     /* fSummary      */
     "",                                /* szSummary     */
     0,"",                              /* sSqlOwner     */
     0,"",                              /* sSqlQualifier */
     0,"",                              /* sSqlCollid    */
     FALSE                              /* fInsertBuf    */
};


/*
 * SQL Communication area ++
 */
struct  sqlca     sqlca;


/*******************************************************************************
*   Internal Functions                                                         *
*******************************************************************************/
static int   BindIt(int argc, char **argv);
static void  syntax(void);
static KBOOL ProcessFiles(const char *pszFilePattern);
static KBOOL CheckBindFileAndDatabase(const char *pszBindFile);
static KBOOL BindFile(const char *pszBindFile);


int main(int argc, char **argv)
{
    int rc;

    BindMsgInit();

    rc = BindIt(argc, argv);

    BindMsgTerm();

    return rc;
}


int BindIt(int argc, char **argv)
{
    char *  psz;
    int     argi;
    int     i;
    int     fOptions;
    APIRET  rc;


    /*
     * Parse commandline
     */
    if (argc == 1)
    {
        syntax();
        return -1;
    }

    for (argi = 1; argi < argc; argi++)
    {
        if (argv[argi][0] == '-')
        {
            switch (argv[argi][1])
            {
                case 'h':
                case 'H':
                case '?':
                case '-':
                    syntax();
                    BindDBDisconnect(options.szDatabase);
                    return 0;


                case 'a':
                case 'A':
                    options.fForceAll = argv[argi][2] != '-';
                    if (options.cVerbose >= 3)
                        BindInfo("%s\n", options.fForceAll ? "Bind All" : "Bind When Needed");
                    break;


                case 'c':
                case 'C':
                    if (BindGetOption(argc, argv, &argi, &options.sSqlCollid.s.data[0], 129, &fOptions,
                                      FALSE, "a collection ID", "Collection ID", options.cVerbose))
                    {
                        BindDBDisconnect(options.szDatabase);
                        return -1;
                    }
                    options.sSqlCollid.s.length = strlen(strupr(options.sSqlCollid.s.data));
                    break;


                case 'd':
                case 'D':
                    BindDBDisconnect(options.szDatabase);
                    if (BindGetOption(argc, argv, &argi, &options.szDatabase[0], 9, &fOptions,
                                      FALSE, "a database name", "Database", options.cVerbose))
                        return -1;

                    rc = BindDBConnect(options.szDatabase, &options.DBInfo);
                    if (rc)
                    {
                        BindFatalError("Failed to connect to database %s. sqlcode=%d\n",
                                       options.szDatabase, rc);
                        return -2;
                    }
                    if (options.cVerbose >= 1)
                        BindInfo("Connected to %s. DB2 v%d.%d.%d%s\n",
                                 options.szDatabase,
                                 options.DBInfo.iDB2Major,
                                 options.DBInfo.iDB2Midi,
                                 options.DBInfo.iDB2Minor,
                                 options.DBInfo.fMVS ? " - MVS" : "");
                    break;


                case 'e':
                case 'E':
                    options.fExplain = argv[argi][2] != '-';
                    if (options.cVerbose >= 3)
                        BindInfo("Explain %s\n", options.fExplain ? "enabled" : "disabled");
                    break;


                case 'i':
                case 'I':
                {
                    char *psz = &argv[argi][2];
                    if (!stricmp(psz, "RR"))
                        options.iIsolation = SQL_REP_READ;
                    else if (!stricmp(psz, "CS"))
                        options.iIsolation = SQL_CURSOR_STAB;
                    else if (!stricmp(psz, "UR"))
                        options.iIsolation = SQL_UNCOM_READ;
                    else if (!stricmp(psz, "RS"))
                        options.iIsolation = SQL_READ_STAB;
                    else if (!stricmp(psz, "NC"))
                        options.iIsolation = SQL_NO_COMMIT;
                    else
                    {
                        BindSyntaxError(": Option -i requires a valid isolation level.\n"
                                        "              RR, CS, UR, RS, or NC.");
                        return -1;
                    }

                    if (options.cVerbose >= 3)
                        BindInfo("Isolation level is set to '%s'.\n", psz);
                    break;
                }


                case 'l':
                case 'L':
                    if (BindMsgOption(argc, argv, &argi, options.cVerbose))
                        return -1;
                    break;


                case 'o':
                case 'O':
                    if (BindGetOption(argc, argv, &argi, &options.sSqlOwner.s.data[0], 129, &fOptions,
                                      FALSE, "an owner", "Owner", options.cVerbose))
                    {
                        BindDBDisconnect(options.szDatabase);
                        return -1;
                    }
                    options.sSqlOwner.s.length = strlen(strupr(options.sSqlOwner.s.data));
                    break;


                case 'q':
                case 'Q':
                    if (BindGetOption(argc, argv, &argi, &options.sSqlQualifier.s.data[0], 129, &fOptions,
                                      FALSE, "a qualifier", "Qualifier", options.cVerbose))
                    {
                        BindDBDisconnect(options.szDatabase);
                        return -1;
                    }
                    options.sSqlQualifier.s.length = strlen(strupr(options.sSqlQualifier.s.data));
                    break;


                case 's':
                case 'S':
                    options.szSummary[0] = '\0';
                    if (argv[argi][2] == '+')
                        options.fSummary = OPTION_SET;
                    else if (argv[argi][2] == '-')
                        options.fSummary = OPTION_IGNORE;
                    else
                    {
                        if (BindGetOption(argc, argv, &argi, &options.szSummary[0], CCHMAXPATH,
                                          &options.fSummary, TRUE, "a filename", "Error summary is written to", options.cVerbose))
                            return -1;
                    }
                    break;


                case 't':
                case 'T':
                    options.fTest = argv[argi][2] != '-';
                    if (options.cVerbose >= 3)
                        BindInfo("%s\n", options.fTest ? "Do NOT Bind" : "Do Bind");
                    break;


                case 'u':
                case 'U':
                    options.fInsertBuf = argv[argi][2] != '-';
                    if (options.cVerbose >= 3)
                        BindInfo("%s\n", options.fInsertBuf ? "NOT Buffered Inserts" : "Buffered Inserts");
                    break;


                case 'v':
                case 'V':
                {
                    int cVerbose;
                    switch (argv[argi][2])
                    {
                        case '0':   cVerbose = 0; break;
                        case '1':   cVerbose = 1; break;
                        case '2':   cVerbose = 2; break;
                        case '\0':
                        case '3':   cVerbose = 3; break;
                        default:
                            BindSyntaxError(": misformed option -v.\n");
                            syntax();
                            return -1;
                    }
                    if (options.cVerbose >= 3 || cVerbose >= 3)
                        BindInfo("Verbosity level %d.\n", cVerbose);
                    options.cVerbose = cVerbose;
                    break;
                }


                default:
                    BindSyntaxError(": Unkown option '%s'.\n", argv[argi]);
                    BindDBDisconnect(options.szDatabase);
                    return -1;
            }
        }
        else
        {
            /*
             * Bind files.
             */
            if (options.szDatabase[0] == '\0')
            {
                BindFatalError("No database has been specifed yet.\n");
                return -3;
            }
            rc = ProcessFiles(argv[argi]);
            if (rc)
            {
                BindDBDisconnect(options.szDatabase);
                BindFatalError("Failed to gather files '%s'. SYS%04d\n",
                               argv[argi], rc);
                return -4;
            }
        }
    }


    /*
     * Display status.
     */
    if (options.fSummary == OPTION_SET)
        BindErrorSummary(options.szSummary);
    if (options.cVerbose >= 1)
        BindStatus("%8d file(s) examined\n"
                   "%8d file(s) needed to be binded\n"
                   "%8d successful bind(s)\n"
                   "%8d bind(s) failed\n",
                   cTotalFiles,
                   cBindFiles,
                   cSuccess,
                   cErrors);

    /*
     * Close Database connection.
     */
    BindDBDisconnect(options.szDatabase);

    return cErrors;
}


void syntax(void)
{
    fputs(
         "BindIt! - " VER_FULL "\n"
         "-------------------------------\n"
         "Syntax: bindit.exe <options> <files..> [[options] <files> ...]\n"
         "  options:\n"
         "   -d[ ]<database> Which database to connect to.  (required)\n"
         "   -a<[+]|->       Force bind of all packages.     default: -a-\n"
         "   -c<[ ]collid|*> Collection id.                  default: -c*\n"
         "   -e<[+]|->       Explain.                        default: -e-\n"
         "   -f<ISO|...>     Datetime format.                default: -fISO\n"
         "   -i<RR|CS|UR|RS|NC>\n"
         "                   Isolation.                      default: -iCS\n"
         "   -l[ ]<*|filename|stderr|stdout>\n"
         "                   Redirect output to logfile.     default: -lstdout\n"
         "   -o<[ ]owner|*>  Owner id.                       default: -o*\n"
         "   -q<[ ]qual|*>   Qualifier id.                   default: -q*\n"
         "   -s<+|-|[ ]file> Write error summary to stdout or file.\n"
         "                                                   default: -s-\n"
         "   -t<[+]|->       Test run - no bind performed.   default: -t-\n"
         "   -u<[+]|->       Buffered inserts.               default: -u-\n"
         "   -v[0|1|2|3]     Verbosity level.                default: -v2\n"
         "   -h|?            Help screen (this).\n"
         "\n"
         "This program is published according to the GNU GENERAL PUBLIC LICENSE V2.\n"
         "Copyright (c) 2001-2002 knut st. osmundsen (bird@anduin.net)\n",
         stdout);
}



/**
 * Process a file pattern.
 * @returns 0 on success.
 *          error code on fatal failure.
 * @param   pszFilePattern  File pattern to search.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
KBOOL ProcessFiles(const char *pszFilePattern)
{
    char            szFilename[CCHMAXPATH];
    char *          pszFilename;
    const char *    pszPath;
    const char *    pszPath1;
    const char *    pszPath2;
    APIRET          rc;
    ULONG           cFiles = 500;
    HDIR            hDir = HDIR_CREATE;
    char            achfindbuf[1024*16];
    PFILEFINDBUF3   pfindbuf = (PFILEFINDBUF3)&achfindbuf[0];


    if (options.cVerbose >= 1)
        BindInfo("Processing %s\n", pszFilePattern);

    /*
     * Extract path from pattern.
     */
    pszPath1 = strrchr(pszFilePattern, '\\');
    pszPath2 = strrchr(pszFilePattern, '/');
    if (pszPath1 != NULL && pszPath1 > pszPath2)
        pszPath = pszPath1+1;
    else if (pszPath2 != NULL && pszPath2 > pszPath1)
        pszPath = pszPath2+1;
    else if (pszFilePattern[1] == ':')
        pszPath = &pszFilePattern[2];
    else
        pszPath = &pszFilePattern[0];
    memcpy(szFilename, pszFilePattern, pszPath - pszFilePattern);
    pszFilename = &szFilename[pszPath - pszFilePattern];
    *pszFilename = '\0';

    /*
     * First search.
     */
    rc = DosFindFirst((PSZ)pszFilePattern,
                      &hDir,
                      0,
                      pfindbuf,
                      sizeof(achfindbuf),
                      &cFiles,
                      FIL_STANDARD);
    while (rc == NO_ERROR && cFiles > 0)
    {
        PFILEFINDBUF3   pCur = pfindbuf;

        /*
         * Loop thru the result.
         */
        while (pCur && cFiles--)
        {
            /*
             * Make filename.
             * Check the bind file.
             * Check if need bind.
             */
            strcpy(pszFilename, pCur->achName);
            if (CheckBindFileAndDatabase(szFilename))
            {
                /*
                 * Bind needed.
                 */
                cBindFiles++;
                if (BindFile(szFilename))
                    cSuccess++;
                else
                    cErrors++;
            }
            cTotalFiles++;

            /*
             * Next file.
             */
            pCur = pCur->oNextEntryOffset == 0
                    ? NULL
                    : (PFILEFINDBUF3)((char*)pCur + pCur->oNextEntryOffset);
        }

        /*
         * Next chunk.
         */
        cFiles = 500;
        rc = DosFindNext(hDir, pfindbuf, sizeof(achfindbuf), &cFiles);
    }

    /*
     * Terminate find.
     */
    DosFindClose(hDir);

    return rc == ERROR_NO_MORE_FILES ? NO_ERROR : rc;
}


KBOOL CheckBindFileAndDatabase(const char *pszBindFile)
{
    kFileBND *  pFileBND;

    try
    {
        KBOOL   fBind = options.fForceAll || options.fExplain;
        pFileBND = kFileBND::Open(pszBindFile, TRUE);

        char szCreator[136];
        if (   !pFileBND->queryColid(szCreator)
            && !pFileBND->queryCreator(szCreator))
        {
            BindWarning("%s - invalid bind file? failed to get creator/collid.\n", pszBindFile);
            delete pFileBND;
            return TRUE;
        }

        char szApplication[16];
        if (!pFileBND->queryApplication(szApplication))
        {
            BindWarning("%s - invalid bind file? failed to get application name.\n", pszBindFile);
            delete pFileBND;
            return TRUE;
        }

        char szTimeStamp[16];
        if (!pFileBND->queryTimeStamp(szTimeStamp))
        {
            BindWarning("%s - invalid bind file? failed to get timestamp.\n", pszBindFile);
            delete pFileBND;
            return TRUE;
        }

        char szVersion[0x101];
        if (!pFileBND->queryVrsn(szVersion))
            szVersion[0] = '\0';

        if (!fBind)
        {
            if (options.DBInfo.fMVS)
                fBind = BindItCheck_61MVS(szCreator,
                                          szApplication,
                                          szTimeStamp,
                                          szVersion,
                                          options.sSqlCollid.s.data,
                                          options.sSqlQualifier.s.data,
                                          options.sSqlOwner.s.data,
                                          options.iIsolation);
            else if (options.DBInfo.iDB2Major < 7)
                fBind = BindItCheck_21DB2(szCreator,
                                          szApplication,
                                          szTimeStamp,
                                          szVersion,
                                          options.sSqlCollid.s.data,
                                          options.sSqlQualifier.s.data,
                                          options.sSqlOwner.s.data,
                                          options.iIsolation);
            else
                fBind = BindItCheck_71UDB(szCreator,
                                          szApplication,
                                          szTimeStamp,
                                          szVersion,
                                          options.sSqlCollid.s.data,
                                          options.sSqlQualifier.s.data,
                                          options.sSqlOwner.s.data,
                                          options.iIsolation);
        }

        if (options.cVerbose >= 1)
            BindInfo("%s - %s\n",
                     pszBindFile,
                     fBind ? "Bind Needed" : "Bind Not Needed");
        if (options.cVerbose >= 2)
        {
            TIMESTAMP TS = {0,0,0,0,0,0,0};
            BindCvtTS(szTimeStamp, &TS);
            BindInfo("ts: %s (%04d-%02d-%02d-%02d.%02d.%02d.%02d)  app: %s  creator: %s\n",
                     szTimeStamp,
                     TS.iYear, TS.iMonth, TS.iDay,
                     TS.iHour, TS.iMinutte, TS.iSecond, TS.i100,
                     szApplication,
                     szCreator);
        }

        delete pFileBND;
        return fBind;
    }
    catch (kError err)
    {
        BindWarning("%s is not a valid bind file. (err=%d)\n", pszBindFile, err.getErrno());
        return FALSE;
    }
    return TRUE;
}


/**
 * This function executes (if !fTest) the bind command and collects
 * error information using the error message functions.
 * @returns Success indicator.
 * @param   pszBindFile     Pointer to bind file name.
 * @author  knut st. osmundsen (kosmunds@csc.com)
 */
KBOOL BindFile(const char *pszBindFile)
{
    struct  sqlca   sqlca = {0};
    int             rc;
    char            szBuffer[256];
    CHAR            szTmpFile[CCHMAXPATH];
    FILE *          phTmpFile;
    CHAR            achsqlopt[sizeof(struct sqlopt) + sizeof(struct sqloptions) * 14] = {0};
    struct sqlopt * psqlopt = (struct sqlopt *)&achsqlopt[0];

    if (options.fTest)
        return TRUE;

    psqlopt->option[0].type   = SQL_BLOCK_OPT;
    psqlopt->option[0].val    = SQL_BL_ALL;
    psqlopt->option[1].type   = SQL_DATETIME_OPT;
    psqlopt->option[1].val    = SQL_DATETIME_ISO;
    psqlopt->option[2].type   = SQL_ISO_OPT;
    psqlopt->option[2].val    = options.iIsolation;
    rc = 3;
    if (options.fExplain)
    {
        if (!options.DBInfo.fMVS)
        {
            psqlopt->option[rc].type  = SQL_EXPLAIN_OPT;
            psqlopt->option[rc++].val = SQL_EXPLAIN_ALL;
            psqlopt->option[rc].type  = SQL_EXPLSNAP_OPT;
            psqlopt->option[rc++].val = SQL_EXPLSNAP_ALL;
        }
        else
        {
            psqlopt->option[rc].type  = SQL_EXPLAIN_OPT;
            psqlopt->option[rc++].val = SQL_EXPLAIN_YES;
        }
    }
    if (options.fValidateBind)
    {
        psqlopt->option[rc].type  = SQL_VALIDATE_OPT;
        psqlopt->option[rc++].val = SQL_VALIDATE_BIND;
    }
    if (options.sSqlOwner.s.length)
    {
        psqlopt->option[rc].type  = SQL_OWNER_OPT;
        psqlopt->option[rc++].val = (unsigned long)&options.sSqlOwner;
    }
    if (options.sSqlQualifier.s.length)
    {
        psqlopt->option[rc].type  = SQL_QUALIFIER_OPT;
        psqlopt->option[rc++].val = (unsigned long)&options.sSqlQualifier;
    }
    if (options.sSqlCollid.s.length)
    {
        psqlopt->option[rc].type  = SQL_COLLECTION_OPT;
        psqlopt->option[rc++].val = (unsigned long)&options.sSqlCollid;
    }
    if (options.fInsertBuf)
    {
        psqlopt->option[rc].type  = SQL_INSERT_OPT;
        psqlopt->option[rc++].val = SQL_INSERT_BUF;
    }
    psqlopt->header.allocated = rc;
    psqlopt->header.used      = rc;

    rc = sqlabndx((_SQLOLDCHAR *)pszBindFile,
                  options.cVerbose < 3 ? "\\dev\\nul" : BindTmpFile(szTmpFile, NULL),
                  psqlopt,
                  &sqlca);
    if (rc == -1)
    {
        BindError("couldn't start commandprocessor\n");
        return FALSE;
    }
    if (sqlca.sqlcode == SQLA_RC_BINDWARN)
        sqlca.sqlcode = SQL_RC_OK;

    if (rc != NO_ERROR || sqlca.sqlcode != 0)
    {
        if (options.cVerbose < 3)
        {
            rc = sqlabndx((_SQLOLDCHAR *)pszBindFile,
                          BindTmpFile(szTmpFile, NULL),
                          psqlopt,
                          &sqlca);
        }
        phTmpFile = fopen(szTmpFile, "r");
        if (phTmpFile)
        {
            while (fgets(szBuffer, sizeof(szBuffer), phTmpFile))
                BindError("%s", szBuffer);
            fclose(phTmpFile);
        }
        unlink(szTmpFile);
        BindError("package=%s sqlcode=%ld sqlstate=%.5s\n",
                  pszBindFile, sqlca.sqlcode, sqlca.sqlstate);
        return FALSE;
    }

    if (options.cVerbose >= 3)
    {
        phTmpFile = fopen(szTmpFile, "r");
        if (phTmpFile)
        {
            while (fgets(szBuffer, sizeof(szBuffer), phTmpFile))
                fputs(szBuffer, stdout);
            fclose(phTmpFile);
        }
        unlink(szTmpFile);
    }

    return TRUE;
}

