/* $Id: BindChk.cpp,v 1.16 2002/03/14 15:27:17 bird Exp $
 *
 * Bind Check - compare bindpackages and executables.
 *            - compare executables and database.
 *            - compare bindpackages and database.
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
#include <kFileInterfaces.h>
#include <kFileBnd.h>
#include <kFileLX.h>


/*******************************************************************************
*   Global Variables                                                           *
*******************************************************************************/
/*
 * Statistics.
 */
unsigned long cTotalFiles   = 0;
unsigned long cMatched      = 0;
unsigned long cMisMatched   = 0;
unsigned long cNotFound     = 0;
unsigned long cErrors       = 0;
unsigned long cWrongFormat  = 0;


/*
 * Options.
 */
static struct _OptionsBindChk
{
    DBINFO      DBInfo;
    char        szDatabase[32];
    char        szBindPath[4096];
    int         cVerbose;
    int         fSummary;
    char        szSummary[CCHMAXPATH];
} options =
{
     {0,0,0,0},                         /* DBInfo        */
     "",                                /* szDatabase    */
     "",                                /* szBindPath    */
     2,                                 /* cVerbose      */
     OPTION_IGNORE,                     /* fSummary      */
     ""                                 /* szSummary     */
};


/*
 * SQL Communication area ++
 */
struct  sqlca     sqlca;


/*******************************************************************************
*   Internal Functions                                                         *
*******************************************************************************/
static void     syntax(void);
static int      ProcessFiles(const char *pszFilePattern);
static int      ProcessFile(const char *pszBindFile);
static int      ExeCallBack(kFileBNDPrgId *pPrgId, unsigned ulAddress, unsigned long ulUser);
static int      CompareProgIdAndBindFile(const char *pszPlanName, kFileBNDPrgId * pPrgId);
static KBOOL    CompareBindFileAndDatabase(kFileBND *  pFileBND, const char *pszBindFile);
int             BindChk(int argc, char **argv);


int main(int argc, char **argv)
{
    int rc;

    BindMsgInit();

    rc = BindChk(argc, argv);

    BindMsgTerm();

    return rc;
}

int BindChk(int argc, char **argv)
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


                case 'b':
                case 'B':
                    if (BindGetOption(argc, argv, &argi, &options.szBindPath[0], sizeof(options.szBindPath),
                                      &fOptions, FALSE, "a bind path", "Bind Path", options.cVerbose))
                    {
                        BindDBDisconnect(options.szDatabase);
                        return -1;
                    }
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


                case 'l':
                case 'L':
                    if (BindMsgOption(argc, argv, &argi, options.cVerbose))
                        return -1;
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
             * Files.
             */
            if (options.szDatabase[0] == '\0' && options.szBindPath[0] == '\0')
            {
                BindFatalError("Neither database nor bind path has been specifed yet.\n");
                return -3;
            }
            rc = ProcessFiles(argv[argi]);
            if (rc)
            {
                BindDBDisconnect(options.szDatabase);
                BindFatalError("Failed to process files '%s'. SYS%04d\n", argv[argi], rc);
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
                   "%8d compare(s) matched\n"
                   "%8d compare(s) didn't match\n"
                   "%8d bindfile(s) not found\n"
                   "%8d file(s) wrong format\n"
                   "%8d file(s) failed\n",
                   cTotalFiles,
                   cMatched,
                   cMisMatched,
                   cNotFound,
                   cWrongFormat,
                   cErrors);

    /*
     * Close Database connection.
     */
    BindDBDisconnect(options.szDatabase);

    return cErrors + cMisMatched;
}


void syntax(void)
{
    fputs(
         "BindChk! - " VER_FULL "\n"
         "--------------------------------\n"
         "Syntax: bindchk.exe <options> <files..> [[options] <files> ...]\n"
         "  options:\n"
         "   -b[ ]<bindpath|*> Where to find the bind files.   default: -b*\n"
         "   -d[ ]<database|*> Which database to connect to.   default: -d*\n"
         "   -l[ ]<*|filename|stderr|stdout>\n"
         "                     Redirect output to logfile.     default: -lstdout\n"
         "   -s<+|-|[ ]file>   Write error summary to stdout or file.\n"
         "                                                     default: -s-\n"
         "   -v[0|1|2|3]       Verbosity level.                default: -v2\n"
         "   -h|?              Help screen (this).\n"
         "\n"
         "Three checks may be performed:\n"
         "  1. Executables and bindfiles are compared.\n"
         "  2. Executables and database are compared.\n"
         "  3. Bindfiles and database are compared.\n"
         "If a database (-d<db>) is spesified all files following will be compared with\n"
         "the database.\n"
         "If a bindpath (-b<path>) is specified all executables will be compared to\n"
         "bindfiles in that path.\n"
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
int ProcessFiles(const char *pszFilePattern)
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
             */
            strcpy(pszFilename, pCur->achName);
            rc = ProcessFile(szFilename);
            if (rc == -2)
                cWrongFormat++;
            else if (rc != 0)
                cErrors++;
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

/**
 * Processes a single file.
 * @returns 0 if successfully processed.
 *          -2 if wrong format.
 *          other error code on failure.
 * @param   pszBindFile     Name of the file to process.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
int ProcessFile(const char *pszBindFile)
{
    int rc;

    BindDelayedReset();
    try
    {
        /*
         * Try open the file as a bind file.
         * If it succeeds we'll compare the bind file to the package in the database.
         * If mismatch we'll allways complain about.
         * Display info according to verbosity level.
         */
        kFileBND *  pFileBND = kFileBND::Open(pszBindFile);
        if (options.szDatabase[0] == '\0')
        {
            BindError("No database connection. Unable to check '%s'.\n", pszBindFile);
            return -1;
        }
        KBOOL   fMatched = CompareBindFileAndDatabase(pFileBND, pszBindFile);
        if (fMatched)
            cMatched++;
        else
            cMisMatched++;

        if (options.cVerbose >= 2 || !fMatched)
        {
            if (fMatched)
                BindInfo("BINDFILE: %s  -  Match (with Database)\n", pszBindFile);
            else
                BindError("BINDFILE: %s  -  MISMATCH (with Database)\n", pszBindFile);
        }
        if (options.cVerbose >= 3)
        {
            TIMESTAMP   TS;
            char        szTS[9];
            char        szApplication[9];
            char        szCreator[64];
            char        szCollid[132];
            char        szOwner[132];
            char        szIsol[32];
            char        szDateFmt[32];

            pFileBND->queryTimeStamp(szTS);
            BindCvtTS(szTS, &TS);

            BindInfo("ts: %s (%04d-%02d-%02d-%02d.%02d.%02d.%02d)  app: %s  creator: %s\n",
                     szTS,
                     TS.iYear, TS.iMonth, TS.iDay,
                     TS.iHour, TS.iMinutte, TS.iSecond, TS.i100,
                     pFileBND->queryApplication(szApplication),
                     pFileBND->queryCreator(szCreator));
            BindInfo("rel: %x.%02x  date: %s  isol: %s  colid: %s  owner: %s \n",
                     pFileBND->queryRelNo() / 0x100, pFileBND->queryRelNo() % 0x100,
                     kFileBND::queryDateShortName(pFileBND->queryDate(), szDateFmt),
                     kFileBND::queryIsoShortName(pFileBND->queryIsol(), szIsol),
                     pFileBND->queryColid(szCollid) ? szCollid : "<Undef/Deflt>",
                     pFileBND->queryOwner(szOwner)  ? szOwner  : "<Undef/Deflt>");
        }

        delete pFileBND;
    }
    catch (kError err)
    {
        /*
         * If it's a bind file we'll won't try opening it as an executable file.
         */
        if (err == kError::INCORRECT_VERSION)
        {
            BindError("The bindfile %s have an unsupported structure.\n", pszBindFile);
            return -1;
        }
        if (err != kError::INVALID_SIGNATURE)
        {
            BindError("Error %d occured while reading the bind file %s.\n", err.getErrno(), pszBindFile);
            return -1;
        }


        /*
         * It wasn't a Bind file. We'll try an open it as an executable.
         */
        try
        {
            kFileLX *   pFileLX = new kFileLX(new kFile(pszBindFile));
            if (options.cVerbose >= 2)
                BindInfo("Executable: %s\n", pszBindFile);

            rc = kFileBNDPrgId::scanExecutable(pFileLX, FALSE, ExeCallBack, (unsigned long)pszBindFile);

            delete pFileLX;
        }
        catch (kError err)
        {
            if (err == kError::INVALID_EXE_SIGNATURE)
            {
                /*
                 * Check if this wasn't a LX file.
                 */
                if (options.cVerbose >= 1)
                    BindWarning("%s is neither a bind nor a LX executable file.\n", pszBindFile);
                rc = -2;
            }
            else
            {
                /*
                 * It was a LX file but some other error occured. report it.
                 */
                BindError("%s error. (err=%d)\n", pszBindFile, err.getErrno());
                rc = -1;
            }
        }
    }

    BindDelayedFlush();

    return rc;
}


/**
 * Callback function which will compare the program id filed with database or/and bindfile..
 * @returns 0 on success.
 *          error code (-1) on failure.
 * @param   pPrgId      Pointer to an program id object.
 * @param   ulAddress   Address in the executable this program id was found.
 * @equiv   ulUser      User defined parameter.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
int ExeCallBack(kFileBNDPrgId *pPrgId, unsigned ulAddress, unsigned long ulUser)
{
    const char *pszBindFile = (const char*)(void*)ulUser;
    int         rc;
    int         fMatchedDB = -1;
    int         fMatchedBind = -1;
    int         fGetInfo = -1;
    char        szCreator[136];
    char        szQualifier[136];
    char        szOwner[136];
    char        chValid;
    char        chOperative;
    char        chValidate;
    char        chBlocking;
    int         iIsolation;
    char        szContoken[9];
    char        szPlanname[9];
    char        szSqlUser[136];

    if (   !pPrgId->querySqlUser(szSqlUser)
        || !pPrgId->queryPlanname(szPlanname)
        || !pPrgId->queryContoken(szContoken))
    {
        BindIntError("Failed to get sqluser, contoken or planname. ulAddress=0x%08x file=%s\n",
                     ulAddress, pszBindFile);
        return -1;
    }

    if (options.szBindPath[0] != '\0')
        fMatchedBind = CompareProgIdAndBindFile(szPlanname, pPrgId);

    if (options.szDatabase[0] != '\0')
    {
        if (options.DBInfo.fMVS)
            fGetInfo = BindGetInfo_61MVS(szSqlUser,
                                         szPlanname,
                                         szContoken,
                                         "",
                                         &szCreator[0],
                                         &szQualifier[0],
                                         &szOwner[0],
                                         &chValid,
                                         &chOperative,
                                         &chValidate,
                                         &chBlocking,
                                         &iIsolation);

        else if (options.DBInfo.iDB2Major < 7)
            fGetInfo = BindGetInfo_21DB2(szSqlUser,
                                         szPlanname,
                                         szContoken,
                                         "",
                                         &szCreator[0],
                                         &szQualifier[0],
                                         &szOwner[0],
                                         &chValid,
                                         &chOperative,
                                         &chValidate,
                                         &chBlocking,
                                         &iIsolation);
        else
            fGetInfo = BindGetInfo_71UDB(szSqlUser,
                                         szPlanname,
                                         szContoken,
                                         "",
                                         &szCreator[0],
                                         &szQualifier[0],
                                         &szOwner[0],
                                         &chValid,
                                         &chOperative,
                                         &chValidate,
                                         &chBlocking,
                                         &iIsolation);

        fMatchedDB = chValid != 'N' && chOperative != 'N' && fGetInfo;
    }

    if (!fMatchedDB || !fMatchedBind)
        cMisMatched++;
    else if (fMatchedBind == -2)
        cNotFound++;
    else
        cMatched++;

    if (options.cVerbose >= 1 || !fMatchedDB || !fMatchedBind || fMatchedBind == -2)
    {
        char *pszBindMsg;
        char *pszDBMsg;

        pszBindMsg =  !fMatchedBind         ? "MISMATCH"
                     : fMatchedBind > 0     ? "Match"
                     : fMatchedBind == -1   ? "Untested"
                                            : "File not found";
        pszDBMsg   =  !fMatchedDB           ? "MISMATCH"
                     : fMatchedDB != -1     ? "Match   "
                                            : "Untested";

        if (!fMatchedDB || !fMatchedBind || fMatchedBind == -2)
            BindError("planname: %s  db: %s  bind: %s\n",
                      szPlanname, pszDBMsg, pszBindMsg);
        else
            BindInfo( "planname: %s  db: %s  bind: %s\n",
                      szPlanname, pszDBMsg, pszBindMsg);

        if (options.szDatabase[0] && fGetInfo)
        {
            if (chValid == 'N')
                BindError("package is marked NOT VALID.\n");
            if (chOperative == 'N')
                BindError("package is marked NOT OPERATIVE.\n");
        }
        else
            BindError("package is probably not binded.\n");

        if (options.cVerbose >= 2)
        {
            TIMESTAMP TS;
            BindCvtTS(szContoken, &TS);
            if (options.cVerbose >= 3)
                BindInfo("0x%08x  sqluser: %s  ts: %s (%04d-%02d-%02d-%02d.%02d.%02d.%02d)\n",
                         szPlanname,
                         szSqlUser,
                         szContoken,
                         TS.iYear, TS.iMonth, TS.iDay,
                         TS.iHour, TS.iMinutte, TS.iSecond, TS.i100);
            else
                BindInfo("sqluser:  %s  ts: %s (%04d-%02d-%02d-%02d.%02d.%02d.%02d)\n",
                         szSqlUser,
                         szContoken,
                         TS.iYear, TS.iMonth, TS.iDay,
                         TS.iHour, TS.iMinutte, TS.iSecond, TS.i100);
        }

        if (options.szDatabase[0] && options.cVerbose >= 3 && fGetInfo)
        {
            char szIsolation[16];
            BindInfo("binded with qual='%s'  owner='%s'  vldt='%c'  isol='%s'\n",
                     &szQualifier[0], &szOwner[0], chValidate,
                     kFileBND::queryIsoShortName(iIsolation, szIsolation));
        }
    }

    return 0;
}


/**
 * Compare a program ID and a bindfile.
 * @returns 1 on match.
 *          0 on mismatch
 *          -2 on file not found.
 * @param   pszPlanName     Planname/application from the PrgId. This is postfixed with
 *                          a .bnd extention and the bindpath is searched for that name.
 *                          So the bind filename must match the planname/application.
 * @param   pPrgId          Pointer to program id object to compare with the bind file.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
int CompareProgIdAndBindFile(const char *pszPlanName, kFileBNDPrgId * pPrgId)
{
    char    szFilename[12];
    char    szBindFile[CCHMAXPATH];
    int     rc;

    /*
     * Make filename and search the bindpath for it.
     */
    rc = DosSearchPath(SEARCH_IGNORENETERRS, options.szBindPath,
                       strcat(strcpy(szFilename, pszPlanName), ".bnd"),
                       szBindFile, sizeof(szBindFile));
    if (rc != NO_ERROR)
        return -2;

    /*
     * We found a file. Lets try open it and compare it with the program id object.
     */
    try
    {
        kFileBND *  pFileBND = kFileBND::Open(szBindFile);
        rc = *pPrgId == *pFileBND;
        delete pFileBND;
    }
    catch (kError err)
    {
        /*
         * Error occured, let's complain. We return file not found.
         */
        if (err == kError::INCORRECT_VERSION)
            BindError("The bindfile %s have an unsupported structure.\n", szBindFile);
        else if (err != kError::INVALID_SIGNATURE)
            BindError("Error %d occured while reading the bind file %s.\n", err.getErrno(), szBindFile);
        else
            BindError("The file %s is not a valid bind file.\n", szBindFile);
        rc = -2;
    }
    return rc;
}


/**
 * Compares a bind file and the bind packages in the database.
 * @returns 1 if the bind file and bind package matched
 *          0 if they don't match.
 * @param   pFileBND        Pointer to the bind file objec.t
 * @param   pszBindFile     Pointer to bind file name. Used when reporting error.s
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
KBOOL CompareBindFileAndDatabase(kFileBND *  pFileBND, const char *pszBindFile)
{
    char    szCreator[136];
    char    szCollid[136];
    char    szVersion[0x101];
    char    szQualifier[136];
    char    szOwner[136];
    char    chValid;
    char    chValidate;
    char    chBlocking;
    char    chOperative;
    int     iIsolation;


    if (   !pFileBND->queryColid(szCollid)
        && !pFileBND->queryCreator(szCollid))
    {
        BindWarning("%s - invalid bind file? failed to get creator/collid.\n", pszBindFile);
        return FALSE;
    }

    char szApplication[16];
    if (!pFileBND->queryApplication(szApplication))
    {
        BindWarning("%s - invalid bind file? failed to get application name.\n", pszBindFile);
        return FALSE;
    }

    char szTimeStamp[16];
    if (!pFileBND->queryTimeStamp(szTimeStamp))
    {
        BindWarning("%s - invalid bind file? failed to get timestamp.\n", pszBindFile);
        return FALSE;
    }

    if (!pFileBND->queryVrsn(szVersion))
        szVersion[0] = '\0';


    KBOOL   fBind;
    if (options.DBInfo.fMVS)
        fBind = BindGetInfo_61MVS(szCollid,
                                  szApplication,
                                  szTimeStamp,
                                  szVersion,
                                  &szCreator[0],
                                  &szQualifier[0],
                                  &szOwner[0],
                                  &chValid,
                                  &chOperative,
                                  &chValidate,
                                  &chBlocking,
                                  &iIsolation);

    else if (options.DBInfo.iDB2Major < 7)
        fBind = BindGetInfo_21DB2(szCollid,
                                  szApplication,
                                  szTimeStamp,
                                  szVersion,
                                  &szCreator[0],
                                  &szQualifier[0],
                                  &szOwner[0],
                                  &chValid,
                                  &chOperative,
                                  &chValidate,
                                  &chBlocking,
                                  &iIsolation);
    else
        fBind = BindGetInfo_71UDB(szCollid,
                                  szApplication,
                                  szTimeStamp,
                                  szVersion,
                                  &szCreator[0],
                                  &szQualifier[0],
                                  &szOwner[0],
                                  &chValid,
                                  &chOperative,
                                  &chValidate,
                                  &chBlocking,
                                  &iIsolation);

    if (fBind)
    {
        if (chValid == 'N')
        {
            BindDelayedError("package is marked NOT VALID.\n");
            fBind = FALSE;
        }
        if (chOperative == 'N')
        {
            BindDelayedError("package is marked NOT OPERATIVE.\n");
            fBind = FALSE;
        }
        if (options.cVerbose == 3)
        {
            char szIsolation[16];
            BindDelayedInfo("binded with qual='%s'  owner='%s'  vldt='%c'  isol='%s'\n",
                            &szQualifier[0], &szOwner[0], chValidate,
                            kFileBND::queryIsoShortName(iIsolation, szIsolation));
        }
    }
    else
        BindDelayedError("package probably not binded.\n");

    return fBind;
}

