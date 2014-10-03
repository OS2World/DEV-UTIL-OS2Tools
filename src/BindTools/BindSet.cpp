/* $Id: BindSet.cpp,v 1.16 2002/03/14 15:27:18 bird Exp $
 *
 * Set utility - Modify bind files and executables.
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
#include <kFileInterfaces.h>
#include <kFileFormatBase.h>
#include <kFileBnd.h>
#include <kFileLX.h>

/*******************************************************************************
*   Global Variables                                                           *
*******************************************************************************/
/*
 * Statistics.
 */
unsigned long cTotalFiles = 0;
unsigned long cSuccess = 0;
unsigned long cErrors = 0;
unsigned long cWrongFormat = 0;


/*
 * Options.
 */
static struct _OptionsBindIt
{
    BOOL        fTest;
    int         cVerbose;
    int         fTimestamp;
    char        szTimestamp[9];
    int         fCreator;
    char        szCreator[132];
    int         fOwner;
    char        szOwner[132];
    int         fCollid;
    char        szCollid[132];
    int         fQualifier;
    char        szQualifier[132];
    int         fIsolation;
    int         iIsolation;
    int         fDateFmt;
    int         iDateFmt;
    int         fSummary;
    char        szSummary[CCHMAXPATH];
    BOOL        fShowOnly;
} options =
{
    FALSE,                              /* fTest            */
    2,                                  /* cVerbose         */
    OPTION_IGNORE,                      /* fTimestamp       */
    "",                                 /* szTimestamp[9]   */
    OPTION_IGNORE,                      /* fCreator         */
    "",                                 /* szCreator[132]   */
    OPTION_IGNORE,                      /* fOwner           */
    "",                                 /* szOwner[132]     */
    OPTION_IGNORE,                      /* fCollid          */
    "",                                 /* szCollid[132]    */
    OPTION_IGNORE,                      /* fQualifier       */
    "",                                 /* szQualifier[132] */
    OPTION_IGNORE,                      /* fIsolation       */
    0,                                  /* iIsolation       */
    OPTION_IGNORE,                      /* fDateFmt         */
    0,                                  /* iDateFmt         */
    OPTION_IGNORE,                      /* fSummary         */
    "",                                 /* szSummary        */
    FALSE                               /* fShowOnly        */
};


/*******************************************************************************
*   Internal Functions                                                         *
*******************************************************************************/
static void syntax(void);
static int  ProcessFiles(const char *pszFilePattern);
static int  ProcessFile(const char *pszBindFile);
static int  ExeCallBack(kFileBNDPrgId *pPrgId, unsigned ulAddress, unsigned long ulUser);
int         BindSet(int argc, char **argv);


int main(int argc, char **argv)
{
    int rc;

    BindMsgInit();

    rc = BindSet(argc, argv);

    BindMsgTerm();

    return rc;
}

int BindSet(int argc, char **argv)
{
    char *  psz;
    int     argi;
    int     i;
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
                    return 0;


                case 'c':
                case 'C':
                    if (BindGetOption(argc, argv, &argi, options.szCollid, 129, &options.fCollid,
                                      TRUE, "a collection ID", "Collection ID", options.cVerbose))
                        return -1;
                    strupr(options.szCollid);
                    break;


                case 'd':
                case 'D':
                {
                    char *psz = &argv[argi][2];
                    if (*psz == '*' && psz[1] == '\0')
                    {
                        options.fDateFmt = OPTION_IGNORE;
                        options.iDateFmt = kFileBND::date_default;
                    }
                    else if (*psz == '-' && psz[1] == '\0')
                    {
                        options.fDateFmt = OPTION_CLEAR;
                        options.iDateFmt = kFileBND::date_default;
                    }
                    else
                    {
                        options.fDateFmt = OPTION_SET;
                        if (!stricmp(psz, "ISO"))
                            options.iDateFmt = kFileBND::date_iso;
                        else if (!stricmp(psz, "USA"))
                            options.iDateFmt = kFileBND::date_usa;
                        else if (!stricmp(psz, "EUR"))
                            options.iDateFmt = kFileBND::date_eur;
                        else if (!stricmp(psz, "JIS"))
                            options.iDateFmt = kFileBND::date_jis;
                        else if (!stricmp(psz, "LOC"))
                            options.iDateFmt = kFileBND::date_local;
                        else
                        {
                            BindSyntaxError("Option -d requires a valid datetime format. (ISO, USA, EUR, JIS or LOC)\n");
                            return -1;
                        }
                    }

                    if (options.cVerbose >= 3)
                    {
                        if (options.fDateFmt == OPTION_SET)
                            BindInfo("Datetime format is set to '%s'.\n", psz);
                        else if (options.fDateFmt == OPTION_CLEAR)
                            BindInfo("Datetime format is to be cleared.\n");
                        else
                            BindInfo("Datetime format isn't changed.\n");
                    }
                    break;
                }


                case 'i':
                case 'I':
                {
                    char *psz = &argv[argi][2];
                    if (*psz == '*' && psz[1] == '\0')
                    {
                        options.fIsolation = OPTION_IGNORE;
                        options.iIsolation = kFileBND::notset;
                    }
                    else if (*psz == '-' && psz[1] == '\0')
                    {
                        options.fIsolation = OPTION_CLEAR;
                        options.iIsolation = kFileBND::notset;
                    }
                    else
                    {
                        options.fIsolation = OPTION_SET;
                        if (!stricmp(psz, "RR"))
                            options.iIsolation = kFileBND::iso_rr;
                        else if (!stricmp(psz, "CS"))
                            options.iIsolation = kFileBND::iso_cs;
                        else if (!stricmp(psz, "UR"))
                            options.iIsolation = kFileBND::iso_ur;
                        else if (!stricmp(psz, "RS"))
                            options.iIsolation = kFileBND::iso_rs;
                        else if (!stricmp(psz, "NC"))
                            options.iIsolation = kFileBND::iso_nc;
                        else
                        {
                            BindSyntaxError("Option -i requires a valid isolation level. (RR, CS, UR, RS or NC)\n");
                            return -1;
                        }
                    }

                    if (options.cVerbose >= 3)
                    {
                        if (options.fIsolation == OPTION_SET)
                            BindInfo("Isolation level is set to '%s'.\n", psz);
                        else if (options.fIsolation == OPTION_CLEAR)
                            BindInfo("Isolation level is to be cleared.\n");
                        else
                            BindInfo("Isolation level isn't changed.\n");
                    }
                    break;
                }


                case 'l':
                case 'L':
                    if (BindMsgOption(argc, argv, &argi, options.cVerbose))
                        return -1;
                    break;


                case 'k':
                case 'K':
                    if (BindGetOption(argc, argv, &argi, options.szCreator, 33, &options.fCreator,
                                      FALSE, "a creator", "Creator", options.cVerbose))
                        return -1;
                    strupr(options.szCreator);
                    break;


                case 'o':
                case 'O':
                    if (BindGetOption(argc, argv, &argi, options.szOwner, 129, &options.fOwner,
                                      TRUE, "an owner", "Owner", options.cVerbose))
                        return -1;
                    strupr(options.szOwner);
                    break;


                case 'q':
                case 'Q':
                    if (BindGetOption(argc, argv, &argi, options.szOwner, 129, &options.fOwner,
                                      TRUE, "a qualifier", "Qualifier", options.cVerbose))
                        return -1;
                    strupr(options.szQualifier);
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
                            BindSyntaxError("Misformed options -v.\n");
                            syntax();
                            return -1;
                    }
                    if (options.cVerbose >= 3 || cVerbose >= 3)
                        BindInfo("Verbosity level %d.\n", cVerbose);
                    options.cVerbose = cVerbose;
                    break;
                }


                case 'x':
                case 'X':
                {
                    options.fShowOnly = argv[argi][2] != '-';
                    if (options.cVerbose >= 3)
                        BindInfo("%s.\n", options.fTest ? "Do nothing and show CURRENT values ONLY" : "Do work and show NEW values");
                    break;
                }


                case 'z':
                case 'Z':
                {
                    char szISO[32];

                    if (BindGetOption(argc, argv, &argi, &szISO[0], 27, &options.fTimestamp, FALSE,
                                      "a ISO Timestamp", "Qualifier", options.cVerbose))
                    {
                        BindSyntaxError("(yyyy-mm-dd-hh.mm.ss.xxxxxx)\n");
                        return -1;
                    }
                    if (options.fTimestamp != OPTION_SET)
                        break;

                    if (strlen(szISO) != 26)
                    {
                        BindSyntaxError("Option -z should be followed by a 26 chars long timestamp.\n"
                                        "              (yyyy-mm-dd-hh.mm.ss.xxxxxx).\n");
                        return -1;
                    }

                    if (BindMakeTSISO(options.szTimestamp, szISO))
                    {
                        BindSyntaxError("bad timestamp '%s'. Use ISO format: yyyy-mm-dd-hh.mm.ss.xxxxxx.\n"
                                        "              Year range from 1984-2050\n",
                                        szISO);
                        return -1;
                    }
                    break;
                }


                default:
                    BindSyntaxError("Unknown options '%s'.\n", argv[argi]);
                    return -1;
            }
        }
        else
        {
            /*
             * Bind files.
             */
            rc = ProcessFiles(argv[argi]);
            if (rc)
            {
                BindFatalError("Failed to process files '%s'. SYS%04d\n",
                        argv[argi], rc);
                return -4;
            }
        }
    }


    /*
     * Display status.
     */
    if (options.fSummary == OPTION_SET)
        BindErrorSummary(&options.szSummary[0]);
    if (options.cVerbose >= 1)
        BindStatus("%8d file(s) examined\n"
                   "%8d file(s) successful processed\n"
                   "%8d file(s) failed\n"
                   "%8d file(s) wrong format\n",
                   cTotalFiles,
                   cSuccess,
                   cErrors,
                   cWrongFormat
                   );
    BindMsgTerm();

    return cErrors;
}


void syntax(void)
{
    fputs(
         "BindSet! - " VER_FULL "\n"
         "--------------------------------\n"
         "Syntax: bindset.exe <options> <files..> [[options] <files> ...]\n"
         "  options:\n"
         "   -c<[ ]collid|*|-> Collection id.                  default: -c*  b)\n"
         "   -d<ISO|USE|EUR|JIS|LOC|->\n"
         "                     Datetime format.                default: -d*  b)\n"
         "   -l[ ]<*|filename|stderr|stdout>\n"
         "                     Redirect output to logfile.     default: -lstdout\n"
         "   -k<[ ]creator|*>  Creator id.                     default: -k*\n"
         "   -i<RR|CS|UR|RS|NC|*|->\n"
         "                     Isolation.                      default: -i*  b)\n"
         "   -o<[ ]owner|*|->  Owner id.                       default: -o*  b)\n"
         "   -q<[ ]qual|*|->   Qualifier id.                   default: -q*  b)\n"
         "   -s<+|-|[ ]file>   Write error summary to stdout\n"
         "                     or file.                        default: -s-\n"
         "   -t<[+]|->         Test run - no change performed. default: -t-\n"
         "   -v[0|1|2|3]       Verbosity level.                default: -v2\n"
         "   -x<[+]|->         Show current value, no changes\n"
         "                     are done.                       default: -x-\n"
         "   -z<[ ]ts|*>       Timestamp (ISO format).         default: -z*\n"
         "   -h|?              Help screen (this).\n"
         "\n"
         "  *  means ignoring this option - i.e. don't touch.\n"
         "  -  means clear the field. DB2 will use default value for it.\n"
         "  b) means that this only applies to bind files.\n"
         "\n"
         "Note 1: Don't mix collid and creator. When a bind package doesn't have\n"
         "        an collid, the creator will be used for the collection id.\n"
         "Note 2: Executables don't have a collection id but a creator, or really \n"
         "        it's called a sqluser. For newer DB2 ver.7.1+ the field is expanded,\n"
         "        but its length is variable. We can't therefore NOT change 7.1+\n"
         "        compiled executables to have a creator longer than the one they\n"
         "        was assigned on compile time.\n"
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
    int             cPrevTotalFiles = cTotalFiles;
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


    if (options.cVerbose >= 1 || options.fShowOnly)
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
             * Process the file.
             * Maintain statistics.
             */
            strcpy(pszFilename, pCur->achName);
            rc = ProcessFile(szFilename);
            if (rc == -2)
                cWrongFormat++;
            else if (rc == 0)
                cSuccess++;
            else
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

    /*
     * Warning about no files and return.
     */
    if (cTotalFiles == cPrevTotalFiles)
        BindWarning("No files found matching the pattern: %s\n", pszFilePattern);
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
    int rc = 0;

    try
    {
        kFileBND *  pFileBND = kFileBND::Open(pszBindFile, options.fTest);

        /*
         * Change the bind file.
         */
        if (!options.fShowOnly)
        {
            if (options.fTimestamp == OPTION_SET)
            {
                rc = pFileBND->setTimeStamp(options.szTimestamp);
                if (rc)
                {
                    BindError("Failed to set timestamp of file %s to '%s'. rc=%d\n",
                              pszBindFile, options.szTimestamp, rc);
                    delete pFileBND;
                    return rc;
                }
            }

            if (options.fCreator == OPTION_SET)
            {
                rc = pFileBND->setCreator(options.szCreator);
                if (rc)
                {
                    BindError("Failed to set creator of file %s to '%s'. rc=%d\n",
                              pszBindFile, options.szCreator, rc);
                    delete pFileBND;
                    return rc;
                }
            }

            if (options.fOwner != OPTION_IGNORE)
            {
                rc = pFileBND->setOwner(options.szOwner);
                if (rc)
                {
                    BindError("Failed to set owner of file %s to '%s'. rc=%d\n",
                              pszBindFile, options.szOwner, rc);
                    delete pFileBND;
                    return rc;
                }
            }

            if (options.fCollid != OPTION_IGNORE)
            {
                rc = pFileBND->setColid(options.szCollid);
                if (rc)
                {
                    BindError("Failed to set collection id of file %s to '%s'. rc=%d\n",
                              pszBindFile, options.szCollid, rc);
                    delete pFileBND;
                    return rc;
                }
            }

            if (options.fQualifier != OPTION_IGNORE)
            {
                rc = pFileBND->setQual(options.szQualifier);
                if (rc)
                {
                    BindError("Failed to set qualifier of file %s to '%s'. rc=%d\n",
                              pszBindFile, options.szQualifier, rc);
                    delete pFileBND;
                    return rc;
                }
            }

            if (options.fIsolation != OPTION_IGNORE)
            {
                rc = pFileBND->setIsol(options.iIsolation);
                if (rc)
                {
                    BindError("Failed to set isolation of file %s (to %d). rc=%d\n",
                              pszBindFile, options.iIsolation, rc);
                    delete pFileBND;
                    return rc;
                }
            }

            if (options.fDateFmt != OPTION_IGNORE)
            {
                rc = pFileBND->setDate(options.iDateFmt);
                if (rc)
                {
                    BindError("Failed to set dateformat of file %s (to %d). rc=%d\n",
                              pszBindFile, options.iDateFmt, rc);
                    delete pFileBND;
                    return rc;
                }
            }

            /*
             * Commit the changes.
             */
            if (!options.fTest)
            {
                rc = pFileBND->Commit();
                if (rc)
                {
                    BindError("Failed to write changes to file %s. rc=%d\n", pszBindFile, rc);
                    delete pFileBND;
                    return rc;
                }
            }
        } /* !options.fShowOnly */


        /*
         * Display info.
         */
        if (options.cVerbose == 2)
        {
            BindInfo("BindFile: %s - %s\n", pszBindFile,
                    options.fTest ? "Test run" : "Updated");
        }
        if (options.cVerbose >= 3 || options.fShowOnly)
        {
            TIMESTAMP   TS;
            char        szTS[9];
            char        szApplication[9];
            char        szCreator[64];
            char        szCollid[132];
            char        szOwner[132];
            char        szIsol[32];
            int         iIsol = pFileBND->queryIsol();
            char        szDateFmt[32];
            int         iDateFmt = pFileBND->queryDate();

            pFileBND->queryTimeStamp(szTS);
            BindCvtTS(szTS, &TS);
            switch (iIsol)
            {
                case kFileBND::iso_rr:  strcpy(szIsol, "RR"); break;
                case kFileBND::iso_cs:  strcpy(szIsol, "CS"); break;
                case kFileBND::iso_ur:  strcpy(szIsol, "UR"); break;
                case kFileBND::iso_rs:  strcpy(szIsol, "RS"); break;
                case kFileBND::iso_nc:  strcpy(szIsol, "NC"); break;
                case kFileBND::notset:  strcpy(szIsol, "<Undef>"); break;
                default:                strcpy(szIsol, "<Unknw>"); break;
            }
            switch (iDateFmt)
            {
                case kFileBND::date_default:strcpy(szDateFmt, "DEF"); break;
                case kFileBND::date_usa:    strcpy(szDateFmt, "USA"); break;
                case kFileBND::date_eur:    strcpy(szDateFmt, "EUR"); break;
                case kFileBND::date_iso:    strcpy(szDateFmt, "ISO"); break;
                case kFileBND::date_jis:    strcpy(szDateFmt, "JIS"); break;
                case kFileBND::date_local:  strcpy(szDateFmt, "LOC"); break;
                default:                    strcpy(szDateFmt, "<Unknw>"); break;
            }

            BindInfo("BindFile: %s - %s\n",
                    pszBindFile, options.fTest ? "Test run" : "Updated");
            BindInfo("ts: %s (%04d-%02d-%02d-%02d.%02d.%02d.%02d)  app: %s  creator: %s\n",
                    szTS,
                    TS.iYear, TS.iMonth, TS.iDay,
                    TS.iHour, TS.iMinutte, TS.iSecond, TS.i100,
                    pFileBND->queryApplication(szApplication),
                    pFileBND->queryCreator(szCreator));
            BindInfo("rel: %x.%02x  date: %s  isol: %s  colid: %s  owner: %s  classnm: %s\n",
                    pFileBND->queryRelNo() / 0x100, pFileBND->queryRelNo() % 0x100,
                    szDateFmt,
                    szIsol,
                    pFileBND->queryColid(szCollid) ? szCollid : "<Undef/Deflt>",
                    pFileBND->queryOwner(szOwner)  ? szOwner  : "<Undef/Deflt>",
                    pFileBND->queryClassNm());
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
            kFileLX *   pFileLX = new kFileLX(new kFile(pszBindFile, options.fTest));
            if (options.cVerbose >= 2 || options.fShowOnly)
                BindInfo("Executable: %s\n", pszBindFile);

            rc = kFileBNDPrgId::scanExecutable(pFileLX, options.fTest,
                                               ExeCallBack, (unsigned long)pszBindFile);

            delete pFileLX;
        }
        catch (kError err)
        {
            if (err == kError::INVALID_EXE_SIGNATURE)
            {
                /*
                 * Check if this wasn't a LX file.
                 */
                if (options.cVerbose >= 1 || options.fShowOnly)
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

    return rc;
}


/**
 * Callback function which should work the Program Id.
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

    /*
     * Do Changes.
     */
    if (!options.fShowOnly)
    {
        if (options.fTimestamp == OPTION_SET)
        {
            rc = pPrgId->setContoken(options.szTimestamp);
            if (rc)
            {
                BindIntError("Failed to set connect token. rc=%d ulAddress=0x%08x file=%s\n",
                             rc, ulAddress, pszBindFile);
                return rc;
            }
        }

        if (options.fCreator == OPTION_SET)
        {
            rc = pPrgId->setSqlUser(options.szCreator);
            if (rc)
            {
                BindError("Failed to set creator/sqluser. rc=%d ulAddress=0x%08x file=%s\n",
                          rc, ulAddress, pszBindFile);
                return rc;
            }
        }

        #if 0
        if (options.fApplication == OPTION_SET)
        {
            rc = pPrgId->setPlanname(options.szApplication);
            if (rc)
            {
                BindError("Failed to set application/planname. rc=%d ulAddress=0x%08x file=%s\n",
                          rc, ulAddress, pszBindFile);
                return rc;
            }
        }
        #endif
    } /* !options.fShowOnly */


    /*
     * Display info.
     */
    if (options.cVerbose >= 2 || options.fShowOnly)
    {
        char szContoken[9];
        char szPlanname[9];
        char szSqlUser[136];
        if (   !pPrgId->querySqlUser(szSqlUser)
            || !pPrgId->queryPlanname(szPlanname)
            || !pPrgId->queryContoken(szContoken))
        {
            BindIntError("Failed to get sqluser, contoken or planname. ulAddress=0x%08x file=%s\n",
                    ulAddress, pszBindFile);
            return -1;
        }

        TIMESTAMP TS;
        BindCvtTS(szContoken, &TS);
        if (options.cVerbose >= 3 || options.fShowOnly)
            BindInfo("0x%08x ts: %s (%04d-%02d-%02d-%02d.%02d.%02d.%02d)  app: %s  creator: %s  classnm: %s\n",
                    ulAddress,
                    szContoken,
                    TS.iYear, TS.iMonth, TS.iDay,
                    TS.iHour, TS.iMinutte, TS.iSecond, TS.i100,
                    szPlanname, szSqlUser, pPrgId->queryClassNm());
        else
            BindInfo("ts: %s (%04d-%02d-%02d-%02d.%02d.%02d.%02d)  app: %s  creator: %s\n",
                    szContoken,
                    TS.iYear, TS.iMonth, TS.iDay,
                    TS.iHour, TS.iMinutte, TS.iSecond, TS.i100,
                    szPlanname, szSqlUser);
    }

    return 0;
}
