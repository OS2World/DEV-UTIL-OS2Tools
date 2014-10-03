/* $Id: BindPrep.c,v 1.14 2002/03/20 10:08:34 bird Exp $
 *
 * BindPrep utility which uses APIs and a daemon process so we don't have
 * connect for each precompile.
 *
 * Copyright (c) 2001-2002 knut st. osmundsen (bird@anduin.net)
 *
 * GPL.
 *
 */

/*******************************************************************************
*   Defined Constants And Macros                                               *
*******************************************************************************/
#define PIPE_NAME           "\\PIPE\\BINDTOOLS\\BINDPREP-"
//#define DEBUG2

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
#include <process.h>

#include "Bind.h"
#include "BindVer.h"
#include "BindLib.h"

#include <kTypes.h>


/*@Struct**********************************************************************
*   Structures and Typedefs                                                   *
******************************************************************************/
typedef union _mySQLCHAR
{
    SQL_STRUCTURE sqlchar   s;
    char                    dummy[133];
} MYSQLCHAR, *PMYSQLCHAR;

typedef union _mySqlLCharFileName
{
    SQL_STRUCTURE sqlchar   s;
    char                    dummy[CCHMAXPATH+4];
} MYSQLFNM, *PMYSQLFNM;

struct OptionsBindPrep
{
    DBINFO      DBInfo;
    char        szDatabase[32];
    int         cVerbose;
    KBOOL       fExplain;
    KBOOL       fValidateBind;
    KBOOL       fUseDaemon;
    KBOOL       fApplyEOFFix;
    int         iIsolation;
    int         fDateFmt;
    int         iDateFmt;
    int         iBlocking;
    MYSQLCHAR   sSqlOwner;
    MYSQLCHAR   sSqlQualifier;
    MYSQLCHAR   sSqlCollid;
    MYSQLCHAR   sSqlTarget;
    MYSQLFNM    sSqlBind;
    KBOOL       fBind;
    KBOOL       fInsertBuf;
};


typedef struct _MsgHdr
{
    int         cb;
    int         iMsg;
} MSGHDR, *PMSGHDR;


typedef struct _InitMsg
{
    MSGHDR      hdr;
    char        szDatabase[32];
} INITMSG, *PINITMSG;

typedef struct _InitReply
{
    MSGHDR      hdr;
    int         sqlcode;                /* sqlcode from database connect. */
    DBINFO      DBInfo;
} INITREPLY, *PINITREPLY;


struct OptionsBindPrep;
typedef struct _PrepMsg
{
    MSGHDR      hdr;
    struct OptionsBindPrep  options;
    char        szSource[CCHMAXPATH];
    char        szOutput[CCHMAXPATH];
} PREPMSG, *PPREPMSG;

typedef struct _PrepReply
{
    MSGHDR      hdr;
    int         sqlcode;                /* sqlcode from the sqlaprep call. */
    char        szMsg[65535];           /* Only the necessary space is used. */
} PREPREPLY, *PPREPREPLY;


/*******************************************************************************
*   Global Variables                                                           *
*******************************************************************************/
/*
 * Options.
 */
static struct OptionsBindPrep options =
{
     {0,0,0,0},
     "",                                /* szDbAlias     */
     2,                                 /* cVerbose      */
     FALSE,                             /* fExplain      */
     TRUE,                              /* fValidateBind */
     FALSE,                             /* fUseDaemon    */
     TRUE,                              /* fApplyEOFFix  */
     SQL_CURSOR_STAB,                   /* iIsolation    */
     OPTION_SET,                        /* fDateFmt      */
     SQL_DATETIME_ISO,                  /* iDateFmt      */
     SQL_BL_ALL,                        /* iBlocking     */
     0,"",                              /* sSqlOwner     */
     0,"",                              /* sSqlQualifier */
     0,"",                              /* sSqlCollid    */
     0,"",                              /* sSqlTarget    */
     0,"",                              /* sSqlBind      */
     TRUE,                              /* fBind         */
     FALSE                              /* fInsertBuf    */
};


/*
 * Connections stuff.
 */
HPIPE   hpPipe;
HEV     hev;
KBOOL   fThreadRunning;
KBOOL   fThreadStop;


/*
 * SQL CA
 */
struct  sqlca   sqlca;


/*******************************************************************************
*   Internal Functions                                                         *
*******************************************************************************/
static void syntax(void);
static int  Daemon(const char *pszDatabase);
static void DaemonThread(void *pvDatabase);
static int  daemonConnect(const char *pszSelf, const char *pszDatabase);
static int  daemonPrepFile(const char *pszSource, const char *pszOutput);
static int  daemonDisconnect(void);
static int  daemonSendMsg(PMSGHDR pMsg);
static int  daemonGetMsg(PMSGHDR pMsg);
static int  PrepFile(const char *pszSource, const char *pszOutput);
static long fsize(FILE *phFile);
static char*GetExeName(char *pszExeName, const char *argv0);
int         BindPrep(int argc, char **argv);
int         BindFile(const char *pszBindFile);



int main(int argc, char **argv)
{
    int rc;

    BindMsgInit();

    rc = BindPrep(argc, argv);

    BindMsgTerm();

    return rc;
}


int BindPrep(int argc, char **argv)
{
    char *  psz;
    int     argi;
    int     i;
    int     fOptions;
    APIRET  rc;
    const char *    pszSource = NULL;
    const char *    pszOutput = NULL;


    /*
     * Check for daemon start.
     */
    if (argc == 3 && strcmp(argv[2], "Daemonized!!"))
        return Daemon(argv[1]);

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


                case 'b':
                case 'B':
                {
                    char szBuffer[CCHMAXPATH];
                    if (BindGetOption(argc, argv, &argi, &options.sSqlBind.s.data[0], CCHMAXPATH, &fOptions,
                                      FALSE, "a bind file", "Bind File", options.cVerbose))
                        return -1;
                    strcpy(szBuffer, options.sSqlBind.s.data);
                    rc = DosQueryPathInfo(szBuffer, FIL_QUERYFULLNAME, options.sSqlBind.s.data, CCHMAXPATH);
                    if (rc)
                    {
                        BindError("Bad Bind filename '%s'. rc=%d\n", szBuffer, rc);
                        return -1;
                    }
                    options.sSqlBind.s.length = strlen(options.sSqlBind.s.data);
                    break;
                }


                case 'c':
                case 'C':
                    if (BindGetOption(argc, argv, &argi, &options.sSqlCollid.s.data[0], 129, &fOptions,
                                      FALSE, "a collection ID", "Collection ID", options.cVerbose))
                        return -1;
                    options.sSqlCollid.s.length = strlen(strupr(options.sSqlCollid.s.data));
                    break;


                case 'd':
                case 'D':
                    if (BindGetOption(argc, argv, &argi, &options.szDatabase[0], 9, &fOptions,
                                      FALSE, "a database name", "Database", options.cVerbose))
                        return -1;
                    break;


                case 'e':
                case 'E':
                    options.fExplain = argv[argi][2] != '-';
                    if (options.cVerbose >= 3)
                        BindInfo("Explain %s\n", options.fExplain ? "enabled" : "disabled");
                    break;


                case 'f':
                case 'F':
                {
                    char *psz = &argv[argi][2];
                    if ((*psz == '-' || *psz == '*') && psz[1] == '\0')
                    {
                        options.fDateFmt = OPTION_IGNORE;
                        options.iDateFmt = SQL_DATETIME_DEF;
                    }
                    else
                    {
                        options.fDateFmt = OPTION_SET;
                        if (!stricmp(psz, "ISO"))
                            options.iDateFmt = SQL_DATETIME_ISO;
                        else if (!stricmp(psz, "USA"))
                            options.iDateFmt = SQL_DATETIME_USA;
                        else if (!stricmp(psz, "EUR"))
                            options.iDateFmt = SQL_DATETIME_EUR;
                        else if (!stricmp(psz, "JIS"))
                            options.iDateFmt = SQL_DATETIME_JIS;
                        else if (!stricmp(psz, "LOC"))
                            options.iDateFmt = SQL_DATETIME_LOC;
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


                case 'k':
                case 'K':
                {
                    char *psz = &argv[argi][2];
                    if (!stricmp(psz, "ALL"))
                        options.iBlocking = SQL_BL_ALL;
                    else if (!stricmp(psz, "UNAMBIG"))
                        options.iBlocking = SQL_BL_UNAMBIG;
                    else if (!stricmp(psz, "NO"))
                        options.iBlocking = SQL_NO_BL;
                    else
                    {
                        BindSyntaxError(": Option -k requires a valid blocking option: ALL, UNAMBIG or NO\n");
                        return -1;
                    }

                    if (options.cVerbose >= 3)
                        BindInfo("Blocking option '%s'.\n", psz);
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
                        return -1;
                    options.sSqlOwner.s.length = strlen(strupr(options.sSqlOwner.s.data));
                    break;


                case 'q':
                case 'Q':
                    if (BindGetOption(argc, argv, &argi, &options.sSqlQualifier.s.data[0], 129, &fOptions,
                                      FALSE, "a qualifier", "Qualifier", options.cVerbose))
                        return -1;
                    options.sSqlQualifier.s.length = strlen(strupr(options.sSqlQualifier.s.data));
                    break;


                case 's':
                case 'S':
                    #if 0 /* doesn't work */
                    options.fUseDaemon = argv[argi][2] != '-';
                    if (options.cVerbose >= 3)
                        BindInfo(options.fUseDaemon ? "Use daemon\n" : "Not use daemon\n");
                    #endif
                    break;


                case 't':
                case 'T':
                    if (BindGetOption(argc, argv, &argi, &options.sSqlTarget.s.data[0], 129, &fOptions,
                                      FALSE, "a target compiler", "Target Compiler", options.cVerbose))
                        return -1;
                    options.sSqlTarget.s.length = strlen(strupr(options.sSqlTarget.s.data));
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


                case 'y':
                case 'Y':
                {
                    options.fBind = argv[argi][2] != '-';
                    if (options.cVerbose >= 3)
                        BindInfo("Do %sbind after successful precompile.\n", options.fBind ? "" : "NOT ");
                    break;
                }


                default:
                    BindSyntaxError(": Unkown option '%s'.\n", argv[argi]);
                    return -1;
            }
        }
        else
        {
            if (!pszSource)
                pszSource = argv[argi];
            else if (!pszOutput)
                pszOutput = argv[argi];
            else
            {
                BindSyntaxError("Too many files! Only two files, <source> and <target>, are allowed!\n");
                return -1;
            }
        }
    } /* for argi = 1 to argi */


    /*
     * Syntax check.
     */
    if (options.szDatabase[0] == '\0')
    {
        BindFatalError("No database has been specifed yet.\n");
        return -3;
    }
    if (!pszSource)
    {
        BindSyntaxError("No source file given!\n");
        return -1;
    }
    if (!pszOutput)
    {
        BindSyntaxError("No output file given!\n");
        return -1;
    }

    /*
     * Check for target type.
     */
    if (options.sSqlTarget.s.length <= 0)
    {
        const char* pszExt = strrchr(pszSource, '.');
        if (!pszExt)
            pszExt = strrchr(pszOutput, '.');
        if (pszExt++)
        {
            if (    !stricmp(pszExt, "sqx")
                ||  !stricmp(pszExt, "cpp")
                )
                strcpy(options.sSqlTarget.s.data, "CPLUSPLUS");
            else if (   !stricmp(pszExt, "sqc")
                     || !stricmp(pszExt, "c")
                     )
                strcpy(options.sSqlTarget.s.data, "C");
            else if (   !stricmp(pszExt, "sqb")
                     || !stricmp(pszExt, "cbl")
                     )
            {
                /*
                 * Which Cobol Compiler?
                 */
                char szBuffer[CCHMAXPATH];
                if (!DosSearchPath(SEARCH_ENVIRONMENT, "PATH", "..\\bin\\Cob2.EXE", szBuffer, sizeof(szBuffer)))
                    strcpy(options.sSqlTarget.s.data, "IBMCOB");
                else if (!DosSearchPath(SEARCH_ENVIRONMENT, "PATH", "..\\exedll\\Cobol.EXE", szBuffer, sizeof(szBuffer)))
                {
                    ULONG   ulFlags = 0;
                    rc = DosQueryAppType(szBuffer, &ulFlags);
                    if (rc || ulFlags & FAPPTYP_32BIT)
                        strcpy(options.sSqlTarget.s.data, "MFCOB");
                    else
                        strcpy(options.sSqlTarget.s.data, "MFCOB16");
                }
                else
                    strcpy(options.sSqlTarget.s.data, "ANSI_COBOL");
            }
            else if (   !stricmp(pszExt, "sqf")
                     || !stricmp(pszExt, "for")
                     )
                strcpy(options.sSqlTarget.s.data, "FORTRAN");
            options.sSqlTarget.s.length = strlen(options.sSqlTarget.s.data);
        }
    }



    if (options.fUseDaemon)
    {
        char szExeName[CCHMAXPATH];
        /*
         * Connect to daemon.
         */
        rc = daemonConnect(GetExeName(szExeName, argv[0]), options.szDatabase);
        if (rc)
            return rc;

        /*
         * Precompile.
         */
        rc = daemonPrepFile(pszSource, pszOutput);

        /*
         * Disconnect.
         */
        daemonDisconnect();
    }
    else
    {
        /*
         * Connect to database.
         */
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

        /*
         * PreCompile file.
         */
        rc = PrepFile(pszSource, pszOutput);

        /*
         * Disconnect database.
         */
        BindDBDisconnect(options.szDatabase);
    }

    return rc;
}


static void syntax(void)
{
    fputs(
         "BindPrep! - " VER_FULL "\n"
         "---------------------------------\n"
         "Syntax: bindprep.exe <options> <file> <outputfile>\n"
         "  options:\n"
         "   -b<[ ]file|*>     Bind file.                      default: -b*\n"
         "   -d[ ]<database>   Which database to connect to.  (required)\n"
         "   -c<[ ]collid|*>   Collection id.                  default: -c*\n"
         "   -e<[+]|->         Explain.                        default: -e-\n"
         "   -f<ISO|USE|EUR|JIS|LOC|->\n"
         "                     Datetime format.                default: -fISO\n"
         "   -i<RR|CS|UR|RS|NC>\n"
         "                     Isolation.                      default: -iCS\n"
         "   -k<ALL|UNAMBIG|NO|->\n"
         "                     Blocking                        default: -kALL\n"
         "   -l[ ]<*|filename|stderr|stdout>\n"
         "                     Redirect output to logfile.     default: -lstdout\n"
         "   -o<[ ]owner|*>    Owner id.                       default: -o*\n"
         "   -q<[ ]qual|*>     Qualifier id.                   default: -q*\n"
         "   -s<[+]|->         Use background daemon.          default: -s-\n"
         "                     This parameter is currently ignored due to bugs!\n"
         "   -t<[ ]target>     Target type: \n"
         "   -u<[+]|->         Buffered inserts.               default: -u-\n"
         "   -v[0|1|2|3]       Verbosity level.                default: -v2\n"
         "   -y<[+]|->         Bind after successful prep.     default: -y+\n"
         "   -h|?              Help screen (this).\n"
         "\n"
         "This program is published according to the GNU GENERAL PUBLIC LICENSE V2.\n"
         "Copyright (c) 2001-2002 knut st. osmundsen (bird@anduin.net)\n",
         stdout);
}


int Daemon(const char *pszDatabase)
{
    APIRET  rc;
    ULONG   cPost;
    char *  aszFakeArgv[2] = {"-l\\dev\\nul", NULL};
    char    szPipeName[sizeof(PIPE_NAME) + 32];
    int     iFakeArgi = 0;

    /*
     * Assign output to dev\nul and close standard file handles.
     */
    BindMsgOption(1, aszFakeArgv, &iFakeArgi, 0);
    DosClose(0);
    DosClose(1);
    DosClose(2);

    /*
     * Create named pipe.
     */
    rc = DosCreateNPipe(strcat(strcpy(szPipeName, PIPE_NAME), pszDatabase),
                        &hpPipe,
                        NP_NOWRITEBEHIND | NP_NOINHERIT | NP_ACCESS_DUPLEX,
                        NP_WAIT | NP_READMODE_MESSAGE | NP_TYPE_MESSAGE | 1,
                        4096, 4096, 0);
    if (rc != NO_ERROR)
    {
        BindFatalError("Failed to create the named pipe '%s'. rc=%d\n", szPipeName, rc);
        return rc;
    }


    /*
     * Create semaphore.
     */
    rc = DosCreateEventSem(NULL, &hev, 0, TRUE);
    if (rc)
    {
        BindFatalError("Failed to create event semaphore. rc=%d\n", rc);
        return -2;
    }


    /*
     * Create worker thread
     */
    fThreadStop = FALSE;
    fThreadRunning = FALSE;
    if (_beginthread(DaemonThread, NULL, 0x10000, (void*)pszDatabase) == -1)
    {
        BindFatalError("Failed to create the worker thread.\n");
        return -3;
    }


    /*
     * Timeout loop.
     */
    DosSetPriority(PRTYS_THREAD, PRTYC_IDLETIME, 0, 0);
    while (DosWaitEventSem(hev, 60000) == NO_ERROR || fThreadRunning)
        DosResetEventSem(hev, &cPost);

    /*
     * Tell the worker thread to stop working.
     */
    fThreadStop = TRUE;
    DosCloseEventSem(hev);
    while (fThreadRunning)
        DosSleep(0);

    /*
     * Clean up stuff.*/
    DosClose((HFILE)hpPipe);
    if (options.szDatabase[0])
        BindDBDisconnect(options.szDatabase);
    else
        BindDBDisconnect(pszDatabase);

    return 0;
}



void DaemonThread(void *pvDatabase)
{
    int     rc;
    enum {enmConnect, enmInit, enmPrep, enmClose, enmError}
            enmState;
    DBINFO  DBInfo;                     /* Current databse info. */
    char    szDatabase[32];             /* Current database connection. */

    szDatabase[0] = '\0';
    memset(&DBInfo, 0, sizeof(DBInfo));

    DosSetPriority(PRTYS_THREAD, PRTYC_REGULAR, 1, 0);

    enmState = enmConnect;
    while (!fThreadStop)
    {
        switch (enmState)
        {
            /*
             *  Wait for connection
             */
            case enmConnect:
            {
                fThreadRunning = FALSE;
                rc = DosConnectNPipe(hpPipe);
                if (rc == NO_ERROR)
                {
                    fThreadRunning = TRUE;
                    DosPostEventSem(hev);
                    enmState = enmInit;
                }
                else
                {
                    enmState = enmError;
                    BindError("DosConnectNPipe -> rc=%d\n", rc);
                    DosSleep(0);
                }
                break;
            }


            /*
             * Wait for init packet.
             */
            case enmInit:
            {
                INITMSG     InitMsg;

                enmState = enmError;
                rc = daemonGetMsg(&InitMsg.hdr);
                if (rc == NO_ERROR)
                {
                    INITREPLY   InitReply;
                    InitReply.hdr.cb = sizeof(INITREPLY);
                    InitReply.hdr.iMsg = InitMsg.hdr.iMsg;

                    /*
                     * Need to connect to the database?
                     */
                    if (stricmp(szDatabase, InitMsg.szDatabase))
                    {
                        BindDBDisconnect(szDatabase);
                        strcpy(szDatabase, InitMsg.szDatabase);
                        InitReply.sqlcode = BindDBConnect(szDatabase, &DBInfo);
                        if (InitReply.sqlcode != SQL_RC_OK)
                            szDatabase[0] = '\0';
                    }
                    else
                        InitReply.sqlcode = SQL_RC_OK;
                    memcpy(&InitReply.DBInfo, &DBInfo, sizeof(DBInfo));
                    rc = daemonSendMsg(&InitReply.hdr);
                    if (rc == NO_ERROR && InitReply.sqlcode == SQL_RC_OK)
                        enmState = enmPrep;
                }
                break;
            }


            /*
             * Wait for precompile package.
             */
            case enmPrep:
            {
                PREPMSG     PrepMsg;

                enmState = enmError;
                rc = daemonGetMsg(&PrepMsg.hdr);
                if (rc == NO_ERROR)
                {
                    static PREPREPLY    PrepReply;

                    /*
                     * Install client options
                     * Reset error buffer.
                     * Issue prepare call.
                     */
                    memcpy(&options, &PrepMsg.options, sizeof(options));
                    memcpy(&options.DBInfo, &DBInfo, sizeof(DBInfo));
                    BindResetMsgBuffer();
                    BindResetErrorBuffer();
                    PrepReply.sqlcode = PrepFile(PrepMsg.szSource, PrepMsg.szOutput);
                    if (PrepReply.sqlcode && options.cVerbose < 3)
                        BindGetErrorBuffer(PrepReply.szMsg, sizeof(PrepReply.szMsg));
                    else if (options.cVerbose >= 3)
                        BindGetMsgBuffer(PrepReply.szMsg, sizeof(PrepReply.szMsg));
                    else
                        PrepReply.szMsg[0] = '\0';

                    /*
                     * Make and send reply.
                     */
                    PrepReply.hdr.cb = offsetof(PREPREPLY, szMsg) + strlen(PrepReply.szMsg) + 1;
                    PrepReply.hdr.iMsg = PrepMsg.hdr.iMsg;
                    rc = daemonSendMsg(&PrepReply.hdr);
                    if (!rc)
                        enmState = enmClose;
                }
                break;
            }


            /*
             * Disconnect named pipe client and go to connect state.
             */
            case enmClose:
            {
                enmState = enmConnect;
                DosSleep(50);           /* wait for client to get data. */
                DosDisConnectNPipe(hpPipe);
                DosPostEventSem(hev);
                break;
            }

            /*
             * Error occured.
             */
            case enmError:
            {
                enmState = enmConnect;
                DosSleep(50);           /* wait for client to get data. */
                DosDisConnectNPipe(hpPipe);
                DosPostEventSem(hev);
                break;
            }

            default:
                BindFatalError("Invalid state! %d\n", enmState);
        }
    }
}



/**
 * Function which connects the client to the daemon.
 * @returns 0 on success. Error code on error.
 * @param   pszSelf         Pointer to the name of the current executable.
 * @param   pszDatabase     Database name.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
int daemonConnect(const char *pszSelf, const char *pszDatabase)
{
    INITMSG     InitMsg;
    APIRET      rc;
    KBOOL       fStarted = FALSE;
    int         cWait = 20;

    /*
     * Connect to the pipe.
     */
    do
    {
        char    szPipeName[sizeof(PIPE_NAME) + 32];
        ULONG   ulAction = 0;
        rc = DosOpen(strcat(strcpy(szPipeName, PIPE_NAME), pszDatabase),
                     (PHFILE)&hpPipe,
                     &ulAction,
                     0L, FILE_NORMAL,
                     OPEN_ACTION_OPEN_IF_EXISTS,
                     OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE | OPEN_FLAGS_WRITE_THROUGH,
                     NULL);
        if (rc == ERROR_FILE_NOT_FOUND || rc == ERROR_PATH_NOT_FOUND)
        {
            if (!fStarted)
            {
                /*
                 * Start daemon process.
                 */
                char        szArg[CCHMAXPATH + 64];
                RESULTCODES resc = {0, 0};
                APIRET      rc;
                sprintf(szArg, "%s\t%s Daemonize", pszSelf, pszDatabase);
                *strchr(szArg, '\t') = '\0';
                if (options.cVerbose >= 1)
                    BindInfo("Starting daemon...\n");
                rc = DosExecPgm(NULL, 0, EXEC_BACKGROUND, szArg, NULL, &resc, szArg);
                if (rc != NO_ERROR || resc.codeResult != 0 || resc.codeTerminate == 0) /* codeTerminate == pid */
                {
                    BindFatalError("Failed to start daemon. rc=%d codeResult=%d codeTerminate=%d\n",
                                   rc, resc.codeResult, resc.codeTerminate);
                    return rc;
                }
                fStarted = TRUE;
                DosSleep(0);
                continue;
            }
            DosSleep(32);
        }

    } while (   (   (rc == ERROR_FILE_NOT_FOUND || rc == ERROR_PATH_NOT_FOUND)
                 && cWait--
                 )
             || (   rc == ERROR_PIPE_BUSY
                 && (options.cVerbose >= 1 && BindInfo("Waiting for prep server..."))
                 && (rc = DosWaitNPipe(PIPE_NAME, NP_INDEFINITE_WAIT)) == NO_ERROR
                 )
             );

    if (rc != NO_ERROR)
    {
        BindFatalError("Failed to connect to daemon. rc=%d cWait=%d\n", rc, cWait);
        return rc;
    }


    /*
     * Say hi to daemon and tell it which database to use.
     */
    InitMsg.hdr.cb      = sizeof(InitMsg);
    InitMsg.hdr.iMsg    = 0;
    strcpy(InitMsg.szDatabase, pszDatabase);
    rc = daemonSendMsg(&InitMsg.hdr);
    if (rc == NO_ERROR)
    {
        INITREPLY InitReply;
        rc = daemonGetMsg(&InitReply.hdr);
        if (rc == NO_ERROR)
        {
            if (InitReply.sqlcode != SQL_RC_OK)
            {
                BindFatalError("Daemon failed to connect to database '%s'. sqlcode=%d\n",
                               pszDatabase, InitReply.sqlcode);
                return InitReply.sqlcode;
            }
            else if (options.cVerbose >= 1)
                BindInfo("Connected to %s. DB2 v%d.%d.%d%s\n",
                         options.szDatabase,
                         InitReply.DBInfo.iDB2Major,
                         InitReply.DBInfo.iDB2Midi,
                         InitReply.DBInfo.iDB2Minor,
                         InitReply.DBInfo.fMVS ? " - MVS" : "");
        }
    }

    if (rc)
        BindFatalError("Failed to initate connection to daemon. rc=%d\n", rc);

    return rc;
}


/**
 * Function which precompiles a file using the daemon.
 * @returns 0 on succes. Error code on error.
 * @param   pszSource   Name of the file to precompile.
 * @param   pszOutput   Name of the output file of the precompiliation.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
int daemonPrepFile(const char *pszSource, const char *pszOutput)
{
    int         rc;
    PREPMSG     PrepMsg;

    PrepMsg.hdr.cb = sizeof(PREPMSG);
    PrepMsg.hdr.iMsg = 1;
    memcpy(&PrepMsg.options, &options, sizeof(options));
    rc = DosQueryPathInfo((char*)pszSource, FIL_QUERYFULLNAME, PrepMsg.szSource, sizeof(PrepMsg.szSource));
    if (rc)
    {
        BindError("Bad source filename '%s'. rc=%d\n", pszSource, rc);
        return rc;
    }
    rc = DosQueryPathInfo((char*)pszOutput, FIL_QUERYFULLNAME, PrepMsg.szOutput, sizeof(PrepMsg.szOutput));
    if (rc)
    {
        BindError("Bad output filename '%s'. rc=%d\n", pszOutput, rc);
        return rc;
    }
    rc = daemonSendMsg(&PrepMsg.hdr);
    if (rc == NO_ERROR)
    {
        static PREPREPLY    PrepReply;
        PrepReply.szMsg[0] = '\0';
        rc = daemonGetMsg(&PrepReply.hdr);
        if (rc == NO_ERROR)
        {
            rc = PrepReply.sqlcode;
            if (rc != SQL_RC_OK)
            {
                BindError("Precompile failed. sqlcode=%d\n", rc);
                if (PrepReply.szMsg[0])
                    BindError("Messages from daemon:\n%s", PrepReply.szMsg);
            }
            else if (options.cVerbose >= 3 && PrepReply.szMsg[0])
                BindInfo("Messages from daemon:\n%s", PrepReply.szMsg);
            return rc;
        }
    }

    if (rc)
        BindFatalError("Failed to send precompile request to daemon. rc=%d\n", rc);
    return rc;
}


/**
 * Fucntion which the disconnects the client from the daemon.
 * @returns 0 on succes. Errorcode on error.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
int daemonDisconnect(void)
{
    DosClose(hpPipe);
    return -1;
}


/**
 * Sends message thru the pipe.
 * @returns 0 on success. Error code on error.
 * @param   pMsg    Pointer to message.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
int daemonSendMsg(PMSGHDR pMsg)
{
    ULONG   ulActual = 0;
    int     rc;


    rc = DosWrite(hpPipe, pMsg, pMsg->cb, &ulActual);
    if (rc == NO_ERROR && ulActual == pMsg->cb)
        rc = rc;
    else if (rc == NO_ERROR && ulActual == 0)
    {
        /* disconnected pipe */
        BindFatalError("The connection to the daemon is lost!\n");
        rc = ERROR_PIPE_NOT_CONNECTED;
    }
    else
        BindError("Failed to write to pipe. rc=%d ulActual=%d\n", rc, ulActual);

    return rc;
}


/**
 * Gets message from the pipe.
 * @returns 0 on success. Error code on error.
 * @param   pMsg    Pointer to message.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
int daemonGetMsg(PMSGHDR pMsg)
{
    ULONG   ulActual = 0;
    int     rc;
    void *  pv = pMsg;

    do
    {
        rc = DosRead(hpPipe, pv, -1, &ulActual);
        pv = (void*)((char*)pv + ulActual);
    } while (rc == ERROR_MORE_DATA);

    if (rc == NO_ERROR && ulActual != 0)
        rc = rc;
    else if (rc == NO_ERROR && ulActual == 0)
    {
        /* disconnected pipe */
        BindFatalError("The connection to the daemon is lost!\n");
        rc = ERROR_PIPE_NOT_CONNECTED;
    }
    else
        BindError("Failed to read from the pipe. rc=%d ulActual=%d\n", rc, ulActual);

    return rc;
}



/**
 * Precompiles a file.
 * @returns sqlcode, -1, -2, -3, -4, or -5.
 * @param   pszSource   Name of the input file.
 * @param   pszOutput   Name of the output file.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
int PrepFile(const char *pszSource, const char *pszOutput)
{
    int             rc;
    char            szBuffer[256];
    CHAR            szTmpFile[CCHMAXPATH];
    FILE *          phTmpFile;
    CHAR            achsqlopt[sizeof(struct sqlopt) + sizeof(struct sqloptions) * 24] = {0};
    struct sqlopt * psqlopt = (struct sqlopt *)&achsqlopt[0];
    MYSQLFNM        sSqlOutput;
    const char *    pszExt;
    struct sqlca    sqlca;

    /*
     * Build mandatory options.
     */
    psqlopt->option[0].type   = SQL_BLOCK_OPT;
    psqlopt->option[0].val    = SQL_BL_ALL;
    psqlopt->option[1].type   = SQL_DATETIME_OPT;
    psqlopt->option[1].val    = SQL_DATETIME_ISO;
    psqlopt->option[2].type   = SQL_ISO_OPT;
    psqlopt->option[2].val    = options.iIsolation;
    psqlopt->option[3].type   = SQL_BLOCK_OPT;
    psqlopt->option[3].val    = options.iBlocking;
    sSqlOutput.s.length = strlen(pszOutput);
    if (sSqlOutput.s.length + 1 + sizeof(sSqlOutput.s.length) > sizeof(sSqlOutput.dummy))
        return -2;
    strcpy(sSqlOutput.s.data, pszOutput);
    psqlopt->option[4].type   = SQL_PREP_OUTPUT_OPT;
    psqlopt->option[4].val    = (unsigned long)&sSqlOutput;

    /*
     * Add optional options.
     */
    rc = 5;
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
    if (options.fValidateBind && (options.DBInfo.fMVS || options.DBInfo.iDB2Major >= 5)) /* 5?? */
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
    if (options.sSqlTarget.s.length)
    {
        psqlopt->option[rc].type  = SQL_TARGET_OPT;
        psqlopt->option[rc++].val = (unsigned long)&options.sSqlTarget;
    }
    if (options.sSqlBind.s.length)
    {
        psqlopt->option[rc].type  = SQL_BIND_OPT;
        psqlopt->option[rc++].val = (unsigned long)&options.sSqlBind;
        /* doesn't bind why?
         * would this help:
        psqlopt->option[rc].type  = SQL_ACTION_OPT;
        psqlopt->option[rc++].val = SQL_ACTION_REPLACE;
         */
    }
    if (options.fInsertBuf)
    {
        psqlopt->option[rc].type  = SQL_INSERT_OPT;
        psqlopt->option[rc++].val = SQL_INSERT_BUF;
    }

    psqlopt->header.allocated = rc;
    psqlopt->header.used      = rc;


    /*
     * Precompile.
     */
    rc = sqlaprep((_SQLOLDCHAR *)pszSource,
                  options.cVerbose < 3 ? "\\dev\\nul" : BindTmpFile(szTmpFile, NULL),
                  psqlopt,
                  &sqlca);
    if (rc == -1)
    {
        BindError("couldn't start commandprocessor\n");
        return -1;
    }
    if (sqlca.sqlcode == SQLA_RC_PREPWARN)
        sqlca.sqlcode = SQL_RC_OK;

    if (rc != NO_ERROR || sqlca.sqlcode != 0)
    {
        /*
         * Precompile failed.
         */
        if (options.cVerbose < 3)
        {   /* redo precompile to gather error messages */
            rc = sqlaprep((_SQLOLDCHAR *)pszSource,
                          BindTmpFile(szTmpFile, NULL),
                          psqlopt,
                          &sqlca);
        }
        /* read errors from message file */
        phTmpFile = fopen(szTmpFile, "r");
        if (phTmpFile)
        {
            while (fgets(szBuffer, sizeof(szBuffer), phTmpFile))
                BindError("%s", szBuffer);
            fclose(phTmpFile);
        }
        unlink(szTmpFile);
        BindError("%s sqlcode=%ld sqlstate=%.5s\n",
                  pszSource, sqlca.sqlcode, sqlca.sqlstate);
        return rc != NO_ERROR ? rc : sqlca.sqlcode;
    }

    /*
     * Display messages if full verbosity.
     */
    if (options.cVerbose >= 3)
    {
        phTmpFile = fopen(szTmpFile, "r");
        if (phTmpFile)
        {
            while (fgets(szBuffer, sizeof(szBuffer), phTmpFile))
                BindInfo("%s", szBuffer);
            fclose(phTmpFile);
        }
        unlink(szTmpFile);
    }

    rc = NO_ERROR;


    /*
     * Apply EOF fix.
     */
    if (options.fApplyEOFFix)
    {
        FILE * phFile = fopen(pszOutput, "r+b");
        if (phFile)
        {
            long cbFile = fsize(phFile);
            if (cbFile > 0)
            {
                char * pchFile = malloc(cbFile + 1);
                if (pchFile)
                {
                    if (fread(pchFile, 1, cbFile, phFile) == cbFile)
                    {
                        char * pch = pchFile;
                        pchFile[cbFile] = '\0';
                        while ((pch = memchr(pch, 0x1a, cbFile - 1 - (pch - pchFile))) != NULL)
                        {
                            if (fseek(phFile, pch - pchFile, SEEK_SET) == 0)
                                fputc(' ', phFile);
                            else
                            {
                                BindError("Seek to %d(0x%08x) in '%s' failed.\n",
                                          pch - pchFile, pch - pchFile, pszOutput);
                                rc = -5;
                            }
                            pch++;
                        }
                    }
                    else
                    {
                        BindError("Failed to read file '%s'.\n", pszOutput);
                        rc = -4;
                    }
                    free(pchFile);
                }
                else
                {
                    BindError("Failed allocate %d bytes of memory for file '%s'.\n", cbFile + 1, pszOutput);
                    rc = -3;
                }
            }
            else if (cbFile < 0)
            {
                BindError("Failed to determin size of '%s'.\n", pszOutput);
                rc = -2;
            }
            fclose(phFile);
        }
        else
        {
            BindError("Failed to open '%s'.\n", pszOutput);
            rc = -1;
        }
    }

    /*
     * If successful then do bind the file if requested and a .bnd file was genereated.
     */
    if (!rc && options.fBind && options.sSqlBind.s.length > 0)
        rc = BindFile(options.sSqlBind.s.data);

    return rc;
}



/**
 * This function executes (if !fTest) the bind command and collects
 * error information using the error message functions.
 * @returns sqlcode.
 * @param   pszBindFile     Pointer to bind file name.
 * @author  knut st. osmundsen (kosmunds@csc.com)
 */
int BindFile(const char *pszBindFile)
{
    struct  sqlca   sqlca = {0};
    int             rc;
    char            szBuffer[256];
    CHAR            szTmpFile[CCHMAXPATH];
    FILE *          phTmpFile;
    CHAR            achsqlopt[sizeof(struct sqlopt) + sizeof(struct sqloptions) * 14] = {0};
    struct sqlopt * psqlopt = (struct sqlopt *)&achsqlopt[0];

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
        return rc;
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
        return sqlca.sqlcode != 0 ? sqlca.sqlcode : rc;
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

    return SQL_RC_OK;
}


/**
 * Find the size of a file.
 * @returns   Size of file. -1 on error.
 * @param     phFile  File handle.
 */
long fsize(FILE *phFile)
{
    int ipos;
    signed long cb;

    if ((ipos = ftell(phFile)) < 0
        ||
        fseek(phFile, 0, SEEK_END) != 0
        ||
        (cb = ftell(phFile)) < 0
        ||
        fseek(phFile, ipos, SEEK_SET) != 0
        )
        cb = -1;
    return cb;
}


char *GetExeName(char *pszExeName, const char *argv0)
{
    PTIB ptib;
    PPIB ppib;
    DosGetInfoBlocks(&ptib, &ppib);
    if (DosQueryModuleName(ppib->pib_hmte, CCHMAXPATH, pszExeName))
        strcpy(pszExeName, argv0);
    return pszExeName;
}
