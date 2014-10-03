/* $Id: BindMsg.c,v 1.8 2001/12/04 17:04:48 bird Exp $
 *
 * Message handling.
 *
 * Copyright (c) 2001 knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 *
 * GPL.
 *
 */

/*******************************************************************************
*   Defined Constants And Macros                                               *
*******************************************************************************/
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <process.h>
#include <signal.h>

#include "BindLib.h"


/*@Struct**********************************************************************
*   Structures and Typedefs                                                   *
******************************************************************************/
typedef struct _DelayedMsg
{
    struct _DelayedMsg *        pNext;
    struct _DelayedMsg *        pPrev;
    enum {enmError, enmWarning, enmInfo}
                                enmType;
    char                        szMsg[1];
} DELAYEDMSG, *PDELAYEDMSG;


/*******************************************************************************
*   Global Variables                                                           *
*******************************************************************************/
static char *   pszError = NULL;
static char *   pszErrorEnd = NULL;
static char *   pszErrorCur = NULL;

static char *   pszMsg = NULL;
static char *   pszMsgEnd = NULL;
static char *   pszMsgCur = NULL;

static PDELAYEDMSG  pDelayedMsgs = NULL;
static PDELAYEDMSG  pDelayedMsgsEnd = NULL;

static FILE *   phLog = NULL;

/*******************************************************************************
*   Internal Functions                                                         *
*******************************************************************************/
_Inline int   WriteMsg(const char *pszPrefix, const char *pszFormat, va_list args);
_Inline void  SaveMsg(const char *pszPrefix, const char *pszFormat, va_list args, int cch);
_Inline void  SaveErrorMsg(const char *pszPrefix, const char *pszFormat, va_list args, int cch);
static  void      dbHandler(int sig);

void SigHandler(int sig)
{
    #if defined(__IBMC__) || defined(__IBMCPP__)
    (*(unsigned short *)((char*)phLog + 0xC))--;
    #endif

    fflush(phLog);

    #if defined(__IBMC__) || defined(__IBMCPP__)
    (*(unsigned short *)((char*)phLog + 0xC))++;
    #endif
}

void    BindMsgInit(void)
{
    phLog = stdout;

    signal(SIGBREAK, SigHandler);
    signal(SIGINT,   SigHandler);
    signal(SIGTERM,  SigHandler);
    signal(SIGABRT,  SigHandler);
    signal(SIGSEGV,  SigHandler);
    signal(SIGILL,   SigHandler);
}


void    BindMsgTerm(void)
{
    fflush(phLog);
    if (phLog != stderr && phLog != stdout)
        fclose(phLog);
}


int     BindMsgOption(int argc, char **argv, int *pargi, int cVerbose)
{
    //"   -l[ ]<*|filename|stderr|stdout> Redirect output to logfile.   default: -lstdout"
    //"   -s[[ ]filename]                 Error summary"
    char *  pszOpt = argv[*pargi];
    char *  psz;
    int     cch;

    if (argv[*pargi][2] == '\0' && argc <= *pargi + 1)
    {
        BindSyntaxError("Option -l requires a logfile name.\n");
        return -1;
    }

    psz = argv[*pargi][2] != '\0' ? &argv[*pargi][2] : argv[++*pargi];
    cch = strlen(psz);
    if (cch <= 0)
    {
        BindSyntaxError("Option -l requires a log file name.\n");
        return -1;
    }

    if (cch == 1 && *psz == '*')
        phLog = stdout;
    else if (cch == 6 && !stricmp(psz, "stdout"))
        phLog = stdout;
    else if (cch == 6 && !stricmp(psz, "stderr"))
        phLog = stderr;
    else
    {
        phLog = fopen(psz, "w");
        if (!phLog)
        {
            fprintf(stdout, "Fatal error: Failed to open log file '%s'.\n", psz);
            return -1;
        }
    }

    if (cVerbose >= 3)
    {
        if (phLog == stdout)
            BindInfo("Output is sent to stdout\n");
        else if (phLog == stderr)
            BindInfo("Output is sent to stderr\n");
        else
            BindInfo("Output is sent to '%s'\n", psz);
    }

    return 0;
}


int     BindError(const char *pszFormat, ...)
{
    va_list arg;
    int     cch;
    va_start(arg, pszFormat);
    cch = WriteMsg("Error: ", pszFormat, arg);
    SaveMsg("Error: ", pszFormat, arg, cch);
    SaveErrorMsg("Error: ", pszFormat, arg, cch);
    va_end(arg);
    return cch;
}

int     BindSyntaxError(const char *pszFormat, ...)
{
    va_list arg;
    int     cch;
    va_start(arg, pszFormat);
    cch = WriteMsg("Syntax error: ", pszFormat, arg);
    SaveMsg("Syntax error: ", pszFormat, arg, cch);
    SaveErrorMsg("Syntax error: ", pszFormat, arg, cch);
    va_end(arg);
    return cch;
}


int     BindFatalError(const char *pszFormat, ...)
{
    va_list arg;
    int     cch;
    va_start(arg, pszFormat);
    cch = WriteMsg("Fatal error: ", pszFormat, arg);
    SaveMsg("Fatal error: ", pszFormat, arg, cch);
    SaveErrorMsg("Fatal error: ", pszFormat, arg, cch);
    va_end(arg);
    return cch;
}


int     BindIntError(const char *pszFormat, ...)
{
    va_list arg;
    int     cch;
    va_start(arg, pszFormat);
    cch = WriteMsg("Internal error: ", pszFormat, arg);
    SaveMsg("Internal error: ", pszFormat, arg, cch);
    SaveErrorMsg("Internal error: ", pszFormat, arg, cch);
    va_end(arg);
    return cch;
}



int     BindWarning(const char *pszFormat, ...)
{
    va_list arg;
    int     cch;
    va_start(arg, pszFormat);
    cch = WriteMsg("Warning: ", pszFormat, arg);
    SaveMsg("Warning: ", pszFormat, arg, cch);
    va_end(arg);
    return cch;
}


int     BindInfo(const char *pszFormat, ...)
{
    va_list arg;
    int     cch;
    va_start(arg, pszFormat);
    cch = WriteMsg("Info:  ", pszFormat, arg);
    SaveMsg("Info:  ", pszFormat, arg, cch);
    va_end(arg);
    return cch;
}


int     BindStatus(const char *pszFormat, ...)
{
    va_list arg;
    int     cch;
    va_start(arg, pszFormat);
    cch = WriteMsg("Status:\n", pszFormat, arg);
    SaveMsg("Status: ", pszFormat, arg, cch);
    va_end(arg);
    return cch;
}



_Inline int WriteMsg(const char *pszPrefix, const char *pszFormat, va_list args)
{
    int cchPrefix = strlen(pszPrefix);
    fputs(pszPrefix, phLog);
    cchPrefix += vfprintf(phLog, pszFormat, args) + 1;
    cchPrefix *= 1.02; //Add VAT ;-)
    return cchPrefix;
}


_Inline void SaveMsg(const char *pszPrefix, const char *pszFormat, va_list args, int cch)
{
    if (cch + 1 >= pszMsgEnd - pszMsgCur)
    {
        int     cchAlloc = (cch + 0x1fff) & ~(0x1fff);
        void *  pv = realloc(pszMsg, pszMsgEnd - pszMsg + cchAlloc);
        if (!pv)
        {
            fprintf(phLog, "Fatal Error: Out of Memory!!!\n");
            if (phLog != stdout)
                fprintf(stdout, "Fatal Error: Out of Memory!!!\n");
            exit(-1);
        }
        pszMsgEnd += (char*)pv - pszMsg + cchAlloc;
        pszMsgCur += (char*)pv - pszMsg;
        pszMsg = (char*)pv;
    }

    strcpy(pszMsgCur, pszPrefix);
    pszMsgCur += strlen(pszPrefix);
    pszMsgCur += vsprintf(pszMsgCur, pszFormat, args);
}


_Inline void SaveErrorMsg(const char *pszPrefix, const char *pszFormat, va_list args, int cch)
{
    if (cch + 1 >= pszErrorEnd - pszErrorCur)
    {
        int     cchAlloc = (cch + 0x1fff) & ~(0x1fff);
        void *pv = realloc(pszError, pszErrorEnd - pszError + cchAlloc);
        if (!pv)
        {
            fprintf(phLog, "Fatal Error: Out of Memory!!!\n");
            if (phLog != stdout)
                fprintf(stdout, "Fatal Error: Out of Memory!!!\n");
            exit(-1);
        }
        pszErrorEnd += (char*)pv - pszError + cchAlloc;
        pszErrorCur += (char*)pv - pszError;
        pszError = (char*)pv;
    }

    strcpy(pszErrorCur, pszPrefix);
    pszErrorCur += strlen(pszPrefix);
    pszErrorCur += vsprintf(pszErrorCur, pszFormat, args);
}



void    BindErrorSummary(const char *pszFile)
{
    if (pszErrorCur > pszError)
    {
        FILE *  phFile = pszFile && *pszFile ? fopen(pszFile, "w") : stdout;

        if (!phFile)
        {
            fprintf(stderr, "Failed to open error summary file '%s'.\n", pszFile);
            return;
        }

        fputs("Error Summary:\n", phFile);
        fwrite(pszError, pszErrorCur - pszError, 1, phFile);
        if (phFile != stdout && phFile != stderr)
            fclose(phFile);
    }
}


/**
 * Resets the internal message buffer.
 * @returns 0.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
void    BindResetMsgBuffer(void)
{
    pszMsgCur = pszMsg;
}

/**
 * Copies the internal message buffer to the passed in buffer.
 * @returns 0 on succes. -1 on buffer overflow.
 * @param   pszBuffer   Pointer to buffer.
 * @param   cchBuffer   Size of the buffer.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
int     BindGetMsgBuffer(char *pszBuffer, int cchBuffer)
{
    int rc = 0;
    if (pszMsgCur - pszMsg + 1 < cchBuffer)
        cchBuffer = pszMsgCur - pszMsg;
    else
        rc = -1;
    memcpy(pszBuffer, pszMsg, cchBuffer);
    pszBuffer[cchBuffer] = '\0';
    return rc;
}


/**
 * Resets the internal error buffer.
 * @returns 0.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
void    BindResetErrorBuffer(void)
{
    pszErrorCur = pszError;
}

/**
 * Copies the internal error buffer to the passed in buffer.
 * @returns 0 on succes. -1 on buffer overflow.
 * @param   pszBuffer   Pointer to buffer.
 * @param   cchBuffer   Size of the buffer.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
int     BindGetErrorBuffer(char *pszBuffer, int cchBuffer)
{
    int rc = 0;
    if (pszErrorCur - pszError + 1 < cchBuffer)
        cchBuffer = pszErrorCur - pszError;
    else
        rc = -1;
    memcpy(pszBuffer, pszError, cchBuffer);
    pszBuffer[cchBuffer] = '\0';
    return rc;
}



/**
 * Removes all delayed messages.
 * @returns 0 on success.
 * @author  knut st. osmundsen (kosmunds@csc.com)
 */
int     BindDelayedReset(void)
{
    PDELAYEDMSG pCur = pDelayedMsgs;
    pDelayedMsgs = pDelayedMsgsEnd = NULL;
    while (pCur)
    {
        void *pv = pCur;
        pCur = pCur->pNext;
        free(pv);
    }

    return 0;
}


/**
 * Registeres a delayed error message.
 * @returns size of message.
 * @param   pszFormat   Message format.
 * @param   ...         Message parameters.
 * @author  knut st. osmundsen (kosmunds@csc.com)
 */
int     BindDelayedError(const char *pszFormat, ...)
{
    PDELAYEDMSG pCur;
    char    szBuffer[1024];
    va_list arg;
    int     cch;

    va_start(arg, pszFormat);
    cch = vsprintf(szBuffer, pszFormat, arg);
    va_end(arg);

    pCur = malloc(sizeof(*pCur) + cch);
    memcpy(pCur->szMsg, szBuffer, cch + 1);
    pCur->enmType = enmError;
    pCur->pNext = NULL;
    pCur->pPrev = pDelayedMsgsEnd;

    if (pDelayedMsgsEnd)
        pDelayedMsgsEnd->pNext = pCur;
    else
        pDelayedMsgsEnd = pDelayedMsgs = pCur;

    return cch;
}


/**
 * Registeres a delayed warning message.
 * @returns size of message.
 * @param   pszFormat   Message format.
 * @param   ...         Message parameters.
 * @author  knut st. osmundsen (kosmunds@csc.com)
 */
int     BindDelayedWarning(const char *pszFormat, ...)
{
    PDELAYEDMSG pCur;
    char    szBuffer[1024];
    va_list arg;
    int     cch;

    va_start(arg, pszFormat);
    cch = vsprintf(szBuffer, pszFormat, arg);
    va_end(arg);

    pCur = malloc(sizeof(*pCur) + cch);
    memcpy(pCur->szMsg, szBuffer, cch + 1);
    pCur->enmType = enmWarning;
    pCur->pNext = NULL;
    pCur->pPrev = pDelayedMsgsEnd;

    if (pDelayedMsgsEnd)
        pDelayedMsgsEnd->pNext = pCur;
    else
        pDelayedMsgsEnd = pDelayedMsgs = pCur;

    return cch;
}


/**
 * Registeres a delayed info message.
 * @returns size of message.
 * @param   pszFormat   Message format.
 * @param   ...         Message parameters.
 * @author  knut st. osmundsen (kosmunds@csc.com)
 */
int     BindDelayedInfo(const char *pszFormat, ...)
{
    PDELAYEDMSG pCur;
    char    szBuffer[1024];
    va_list arg;
    int     cch;

    va_start(arg, pszFormat);
    cch = vsprintf(szBuffer, pszFormat, arg);
    va_end(arg);

    pCur = malloc(sizeof(*pCur) + cch);
    memcpy(pCur->szMsg, szBuffer, cch + 1);
    pCur->enmType = enmInfo;
    pCur->pNext = NULL;
    pCur->pPrev = pDelayedMsgsEnd;

    if (pDelayedMsgsEnd)
        pDelayedMsgsEnd->pNext = pCur;
    else
        pDelayedMsgsEnd = pDelayedMsgs = pCur;

    return cch;
}


/**
 * Flushes the delayed message.
 * @returns 0 on success.
 * @author  knut st. osmundsen (kosmunds@csc.com)
 */
int     BindDelayedFlush(void)
{
    PDELAYEDMSG pCur = pDelayedMsgs;
    pDelayedMsgs = pDelayedMsgsEnd = NULL;
    while (pCur)
    {
        void *pv = pCur;

        switch (pCur->enmType)
        {
            case enmError:
                BindError("%s", pCur->szMsg);
                break;

            case enmWarning:
                BindWarning("%s", pCur->szMsg);
                break;

            case enmInfo:
                BindInfo("%s", pCur->szMsg);
                break;

            #ifdef DEBUG
            default:
                BindIntError("BindDelayedFlush: Invalid delayed message type!\n");
            #endif
        }

        /* next + free prev */
        pCur = pCur->pNext;
        free(pv);
    }

    return 0;
}


