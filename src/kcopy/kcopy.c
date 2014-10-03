/* $Id: kcopy.c,v 1.4 2002/09/22 21:22:43 bird Exp $
 *
 * kCopy - funny little file copying program.
 *
 * Copyright (c) 2002 knut st. osmundsen <bird@anduin.net>
 *
 *
 * This file is part of kCopy.
 *
 * kCopy is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * kCopy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with kCopy; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#ifdef OS2
#define INCL_BASE
#define INCL_ERRORS
#include <os2.h>
#else
#include <errno.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>


/*******************************************************************************
*   Defined Constants And Macros                                               *
*******************************************************************************/
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif


/*
 * Config.
 */
#define CB_READ     0x18000             /* 96KB at the time. */
#define CHUNKS_YIELDMARK         8      /* don't know if this helps... */
#define CHUNKS_HIGHWATERMARK     16

#ifdef OS2
#define MAX_FILENAME    CCHMAXPATH
#else
//#define MAX_FILENAME
#endif

/*
 * Chunk type.
 */
#define TYPE_DATA   1
#define TYPE_EA____                     /* todo */
#define TYPE_EOF    0xE0F               /* End of file. */

/*
 * Semaphore abstration.
 */
#ifdef OS2

#define PSEMMTX                 HMTX

#define SemMtxCreate(ppMtx)     DosCreateMutexSem(NULL, ppMtx, 0, FALSE)
#define SemMtxDestroy(pMtx)     DosCloseMutexSem(pMtx);
#define SemMtxUp(pMtx)          DosRequestMutexSem(pMtx, SEM_INDEFINITE_WAIT)
#define SemMtxDown(pMtx)        DosReleaseMutexSem(pMtx)

ULONG   ulPostCountDummy;               /* Dummy variable for DosResetEventSem. */
#define PSEMEVT                 HEV
#define SemEvtCreate(ppEvt)     DosCreateEventSem(NULL, ppEvt, 0, FALSE)
#define SemEvtDestroy(pEvt)     DosCloseEventSem(pEvt);
#define SemEvtPost(pEvt)        DosPostEventSem(pEvt)
#define SemEvtReset(pEvt)       DosResetEventSem(pEvt, &ulPostCountDummy)
#define SemEvtWait(pEvt)        DosWaitEventSem(pEvt, SEM_INDEFINITE_WAIT)

#else

#error "requires semaphores for this OS!"

#endif


/*
 * Debug tracing.
 */
#ifdef DEBUG
#   define dtrace(a)   DebugTrace a
#else
#   define dtrace(a)   do {} while(0)
#endif


/*******************************************************************************
*   Structures and Typedefs                                                    *
*******************************************************************************/
typedef struct _DataChunk DATACHUNK, *PDATACHUNK;

struct _DataChunk
{
    PDATACHUNK      pNext;
    int             iType;              /* content type. */
    unsigned long   cbData;
    int             rc;
    char            achData[1];
};


/*******************************************************************************
*   Global Variables                                                           *
*******************************************************************************/
int     fTerminate = FALSE;

/*
 * Reader / Writer globals.
 */
PSEMEVT     evtReader;                  /* Reader event sem. */
PSEMEVT     evtReader2;                 /* Reader event sem. - used for highwatermark checks */
PSEMEVT     evtWriter;                  /* Writer event sem. */
#ifdef OS2
HFILE       hSource;
#else
FILE       *phSource;
#endif


/*
 * List of packets to write.
 */
PSEMMTX     mtxList;                    /* Must be held before touching the list! */
PDATACHUNK  pHead = NULL;
PDATACHUNK  pTail = NULL;
unsigned    cChunks = 0;


/*******************************************************************************
*   Internal Functions                                                         *
*******************************************************************************/
#ifdef DEBUG
void    DebugTrace(const char *pszFormat, ...);
#endif
int     CopyFileToFiles(const char *pszSource, char ** const papszTargets, int cTargets);
void    reader(void *pv);
int     IsDirectory(const char *pszName);
int     EnumFiles(const char *pszSearch, char **ppszFilename);
char *  FileFilename(char *pszFile);
unsigned long  FileSize(const char *pszFile);
unsigned long gettimems(void);
void    syntax(void);


/**
 * Debug trace facility.
 * @param   pszFormat   Format string.
 * @param   ...         Extra arguments.
 */
#ifdef DEBUG
void    DebugTrace(const char *pszFormat, ...)
{
    static FILE *phTraceFile = NULL;

    if (!phTraceFile)
        phTraceFile = fopen("kcopy.log", "w");
    if (phTraceFile)
    {
        va_list arg;
        va_start(arg, pszFormat);
        vfprintf(phTraceFile, pszFormat, arg);
        va_end(arg);
        fflush(phTraceFile);
    }
}
#endif


/**
 * Copies one single file to many targets.
 *
 * @returns 0 on success.
 *          Appropriate returncode on error.
 * @param   pszSource       Source filename.
 * @param   papszTargets    Array of target filenames.
 * @param   cTargets        Number of target filenames.
 */
int     CopyFileToFiles(const char *pszSource, char ** const papszTargets, int cTargets)
{
    #ifdef OS2
    ULONG   ul;
    HFILE * pahTargets = malloc(cTargets * sizeof(HFILE));
    #else
    FILE ** paphTargets = malloc(cTargets * sizeof(FILE));
    #endif
    int     i;
    int     rc;
    int     fEOF;
    unsigned long cbSourceFile = FileSize(pszSource);

    dtrace((__FUNCTION__"%s,%p,%d\n", pszSource, papszTargets, cTargets));

    /*
     * Open the source file.
     */
    #ifdef OS2
    ul = 0;
    rc = DosOpen(pszSource, &hSource, &ul, 0, FILE_NORMAL,
                 OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_FLAGS_SEQUENTIAL | OPEN_SHARE_DENYWRITE | OPEN_ACCESS_READONLY,
                 NULL);
    #else
    phSource = fopen(pszSource, "rb");
    rc = phSource ? 0 : errno;
    #endif
    if (rc)
    {
        fprintf(stderr, "error: Failed to open source file '%s'. rc=%d\n", pszSource, rc);
        return rc;
    }


    /*
     * Open the target files.
     */
    for (i = 0; i < cTargets; i++)
    {
        #ifdef OS2
        ul = 0;
        rc = DosOpen(papszTargets[i], &pahTargets[i], &ul, cbSourceFile, FILE_NORMAL,
                     OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS,
                     OPEN_FLAGS_SEQUENTIAL | OPEN_SHARE_DENYWRITE | OPEN_ACCESS_WRITEONLY,
                     NULL);
        #else
        paphTargets = fopen(papszTargets[i], "wb");
        rc = paphTarget ? 0 : errno;
        #endif
        if (rc)
        {
            fprintf(stderr, "error: Failed to open target file '%s'. rc=%d\n", papszTargets[i], rc);
            #ifdef OS2
            while (i-- > 0) DosClose(pahTargets[i]);
            DosClose(hSource);
            #else
            while (i-- > 0) DosClose(paphTargets[i]);
            fclose(phSource);
            #endif
            return rc;
        }
    }


    /*
     * Init the state variables and tell the reader to start reading.
     */
    SemEvtReset(evtWriter);
    SemEvtPost(evtReader);
    dtrace((__FUNCTION__": Posted reader\n"));


    /*
     * Write loop.
     */
    rc = 0;
    fEOF = 0;
    while (!fTerminate && !fEOF && !rc)
    {
        /*
         * Wait for data.
         */
        SemEvtWait(evtWriter);
        dtrace((__FUNCTION__": Wakeup\n"));

        /*
         * Work while theres work to be done.
         */
        while (!fTerminate && !fEOF)
        {
            PDATACHUNK  pData;
            unsigned    cLocalChunks;

            /*
             * Unqueue a packet.
             */
            SemMtxUp(mtxList);
            pData = pHead;
            if (pData)
            {
                cLocalChunks = --cChunks;
                pHead = pData->pNext;
                if (!pHead)
                    pTail = NULL;
            }
            SemEvtReset(evtWriter);
            SemEvtPost(evtReader2);
            SemMtxDown(mtxList);
            dtrace((__FUNCTION__": Data packet %p; %d waiting\n", pData, cLocalChunks));
            if (cLocalChunks < 1)
                DosSleep(1);

            /*
             * If no data then wait for it.
             */
            if (!pData)
                break;

            /*
             * Process packet.
             * Check for errors first.
             */
            if (pData->rc)
            {
                rc = pData->rc;
                free(pData);
                break;
            }

            switch (pData->iType)
            {
                case TYPE_DATA:
                {
                    for (i = 0; i < cTargets; i++)
                    {
                        unsigned long ulTime = gettimems();
                        #ifdef OS2
                        ULONG   cbRead = 0;
                        rc = DosWrite(pahTargets[i], &pData->achData[0], pData->cbData, &cbRead);
                        if (!rc && cbRead != pData->cbData)
                            rc = -1;
                        #else
                        rc = fwrite(&pData->achData[0], 1, pData->cbData, paphTargets[i]);
                        if (rc != pData->cbData)
                            rc = -1;
                        #endif
                        if (rc)
                        {
                            fprintf(stderr, "error: failed to write to '%s'. rc=%d\n", papszTargets[i], rc);
                            break;
                        }
                        dtrace((__FUNCTION__": Wrote %d bytes in %d ms\n", pData->cbData, gettimems() - ulTime));
                    }
                    break;
                }

                case TYPE_EOF:
                    fEOF = TRUE;
                    dtrace((__FUNCTION__": EOF\n", pData->cbData));
                    break;

                default:
                    fprintf(stderr, "internal error: invalid data chunk type - %d\n", pData->iType);
                    __interrupt(3);
            }
            free(pData);
        } /* packet processor loop */
    } /* big loop */


    /*
     * Close the files and exit.
     */
    i = cTargets;
    #ifdef OS2
    while (i-- > 0) DosClose(pahTargets[i]);
    DosClose(hSource);
    #else
    while (i-- > 0) DosClose(paphTargets[i]);
    fclose(phSource);
    #endif

    dtrace((__FUNCTION__": returning rc=%d\n", rc));
    return rc;
}


/**
 * File reader thread function.
 */
void reader(void *pv)
{
    #ifdef OS2
    ULONG   ul;
    #endif

    dtrace((__FUNCTION__": started\n"));
    while (!fTerminate)
    {
        int rc;

        rc = SemEvtWait(evtReader);
        dtrace((__FUNCTION__": wakeup rc=%d\n", rc));
        if (!rc && !fTerminate)
        {   /*
             * Allocate data buffer.
             */
            PDATACHUNK  pData = malloc(CB_READ + sizeof(DATACHUNK));
            if (pData)
            {
                /*
                 * Read a chunk into the data buffer.
                 */
                unsigned long   cbRead = 0;
                int             fEOF = 0;
                unsigned        cLocalChunks;
                unsigned long   ulTime = gettimems();

                #ifdef OS2
                rc = DosRead(hSource, &pData->achData[0], CB_READ, &cbRead);
                if (rc == ERROR_MORE_DATA)
                    rc = NO_ERROR;
                else if (cbRead == 0)
                {
                    rc = NO_ERROR;
                    fEOF = 1;
                }
                #else
                rc = 0;
                cbRead = fread(&pData->achData[0], 1, CB_READ, phSource);
                if (cbRead != CB_READ)
                {
                    fEOF = feof(phSource);
                    if (!fEOF)
                        rc = ferror(phSource);
                }
                #endif
                dtrace((__FUNCTION__": read %d bytes in %d ms\n", cbRead, gettimems() - ulTime));

                /*
                 * Put result in packet.
                 */
                pData->iType = TYPE_DATA;
                pData->cbData = cbRead;
                pData->rc = rc;


                /*
                 * Queue the packet for writing and notify the writer.
                 */
                pData->pNext = NULL;
                SemMtxUp(mtxList);
                if (pTail)
                    pTail = pTail->pNext = pData;
                else
                    pHead = pTail = pData;
                cLocalChunks = cChunks++;
                SemEvtPost(evtWriter);
                SemMtxDown(mtxList);
                dtrace((__FUNCTION__": sent packet, %d waiting\n", cLocalChunks + 1));

                /*
                 * If EOF we'll post a separate EOF packet.
                 */
                if (fEOF)
                {
                    pData = malloc(sizeof(DATACHUNK));
                    pData->iType = TYPE_EOF;
                    pData->rc = 0;
                    pData->pNext = NULL;
                    SemMtxUp(mtxList);
                    if (pTail)
                        pTail = pTail->pNext = pData;
                    else
                        pHead = pTail = pData;
                    cChunks++;
                    SemEvtPost(evtWriter);
                    SemMtxDown(mtxList);
                    dtrace((__FUNCTION__": sent EOF packet\n"));
                }

                /*
                 * If EOF or error we'll stop reading.
                 */
                if (fEOF || rc)
                {
                    SemEvtReset(evtReader);
                    dtrace((__FUNCTION__": reset evtReader\n"));
                }

                /*
                 * Else: Check for high water mark
                 */
                #ifdef OS2
                else if (cLocalChunks > CHUNKS_HIGHWATERMARK)
                {
                    do
                    {
                        dtrace((__FUNCTION__": waiting for writer cLocalChunks=%d\n", cLocalChunks));
                        SemEvtWait(evtReader2);
                        if (fTerminate)
                            break;
                        SemMtxUp(mtxList);
                        cLocalChunks = cChunks++;
                        SemEvtReset(evtReader2);
                        SemMtxDown(mtxList);
                    } while (cLocalChunks < CHUNKS_HIGHWATERMARK);
                    dtrace((__FUNCTION__": read more cLocalChunks=%d\n", cLocalChunks));
                }
                #ifdef OS2
                else if (cLocalChunks > CHUNKS_YIELDMARK)
                    DosSleep(0);
                #endif
                #endif
            }
            else
            {
                fprintf(stderr, "fatal error: failed to allocate new datachunk!\n");
                fTerminate = TRUE;
                SemEvtPost(evtWriter);
                break;
            }
        } /* if normal wakeup */
    } /* while not terminating */

    dtrace((__FUNCTION__": terminate\n"));
    pv = pv;
}


/**
 * Checks if the given name is the name of a directory or not.
 * @returns TRUE if directory.
 *          FLASE if not directory.
 * @param   pszName     Name to check.
 */
int IsDirectory(const char *pszName)
{
    #ifdef OS2
    FILESTATUS3 fsts3;
    if (    !DosQueryPathInfo(pszName, FIL_STANDARD, &fsts3, sizeof(fsts3))
        && (fsts3.attrFile & FILE_DIRECTORY))
        return TRUE;
    #else
    struct stat s;
    if (!stat(pszName, &s) && (s.st_mode & S_IFDIR))
        return TRUE;
    #endif
    return FALSE;
}


/**
 * Enumerates a file search expression.
 * This function only supports one enumeration at the time!
 *
 * @returns 0 on success.
 *          !0 on failure.
 * @param   pszSearch       Search pattern is new enumeration.
 *                          NULL if we're to get the next file.
 * @param   ppszFilename    Where to place the filename pointer.
 */
int EnumFiles(const char *pszSearch, char **ppszFilename)
{
    #ifdef OS2
    static char     szFilename[MAX_FILENAME];
    static char    *pszEndPath;
    static char     achBuffer[0xc000];
    static HDIR     hDir = HDIR_CREATE;
    static ULONG    cFiles;
    static PFILEFINDBUF3    pFile;
    int     rc = 0;

    if (pszSearch)
    {   /*
         * A New search.
         */
        if (hDir != HDIR_CREATE)
            DosFindClose(hDir);
        hDir = HDIR_CREATE;
        cFiles = 0x1000;
        rc = DosFindFirst(pszSearch,
                          &hDir,
                          FILE_NORMAL | FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM | FILE_ARCHIVED,
                          &achBuffer[0], sizeof(achBuffer),
                          &cFiles,
                          FIL_STANDARD);
        if (!rc)
        {
            if (cFiles)
            {
                pFile = (PFILEFINDBUF3)&achBuffer[0];
                strcpy(szFilename, pszSearch);
                pszEndPath = FileFilename(szFilename);

                strcpy(pszEndPath, &pFile->achName[0]);
                *ppszFilename = szFilename;
            }
            else
                rc = ERROR_NO_MORE_FILES;
        }
    }
    else
    {   /*
         * Get next file.
         */
        if (--cFiles)
        {
            pFile = (PFILEFINDBUF3)((char*)pFile + pFile->oNextEntryOffset);
            strcpy(pszEndPath, &pFile->achName[0]);
            *ppszFilename = szFilename;
        }
        else
        {
            cFiles = 0x1000;
            rc = DosFindNext(hDir, &achBuffer[0], sizeof(achBuffer), &cFiles);
            if (!rc)
            {
                pFile = (PFILEFINDBUF3)&achBuffer[0];
                strcpy(pszEndPath, &pFile->achName[0]);
                *ppszFilename = szFilename;
            }
            else
            {   /* finished */
                DosFindClose(hDir);
                hDir = HDIR_CREATE;
                *ppszFilename = NULL;
            }
        }
    }

    return rc;
    #else
    #   error TODO
    return -1;
    #endif
}


/**
 * Finds the filename part of the passed in file specifier.
 * @returns Pointer to filename part (after any path).
 *
 * @param   pszFile     Filename specifier.
 */
char *FileFilename(char *pszFile)
{
    char *psz;
    char *psz2;

    psz = strrchr(pszFile, '\\');
    psz2 = strrchr(pszFile, '/');
    if (psz2 > psz)
        psz = psz2;
    psz2 = strrchr(pszFile, ':');
    if (psz2 > psz)
        psz = psz2;

    return psz ? psz + 1 : pszFile;
}


/**
 * Get the file size in bytes.
 * @returns File size.
 * @param   szFile  File which file size we wanna get.
 */
unsigned long  FileSize(const char *pszFile)
{
    #ifdef OS2
    FILESTATUS3 fsts3;
    if (    !DosQueryPathInfo(pszFile, FIL_STANDARD, &fsts3, sizeof(fsts3))
        && !(fsts3.attrFile & FILE_DIRECTORY))
        return fsts3.cbFile;
    #else
    struct stat s;
    if (!stat(pszFile, &s) && !(s.st_mode & S_IFREG))
        return s.st_size;
    #endif
    return 0;
}


/**
 * Gets a MS timestamp, used measuring copy actions.
 * @returns timestamp in milliseconds.
 */
unsigned long gettimems(void)
{
#ifdef OS2
    unsigned long ulMs = 0;
    #if 0
    DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT, &ulMs, sizeof(ulMs));
    #else
    static ULONG hrtimerHz = 0;
    QWORD           qwTime;

    if (hrtimerHz == 0)
    {
        DosTmrQueryFreq(&hrtimerHz);
        hrtimerHz /= 1000;
    }
    DosTmrQueryTime(&qwTime);
    ulMs = (ULONG)((((long double)qwTime.ulHi * (long double)0x10000000 * (long double)0x10 + (long double)qwTime.ulLo) / (long double)hrtimerHz));
    #endif
    return ulMs;
#else
#endif
}


/**
 * Display syntax,
 */
void syntax(void)
{
    printf("syntax: kCopy [options] sources [<-> [targets]]\n"
           "\n"
           "    sources     Source specifiers. Wildchars allowed.\n"
           "    -           Marks the start of the target specifiers.\n"
           "    targets     Target specifiers. Either filenames or directories.\n"
           "                kCopy assumes directories if more than one source file\n"
           "                or recursive.\n"
           "Specifiers ending with a slash is assumes directories.\n"
           "\n"
           "Options:\n"
           "    -q          Don't display filenames.\n"
           "    -n          Don't display totals.\n"
           "    -<r|s>      Recursive - not implemented!\n"
           "\n"
           );
}


int main(int argc, char **argv)
{
    int     iRc = 0;                    /* kCopy return code. */
    int     rc = 0;
    int     argi;
    int     iSrc;
    int     i;
    int     fTargets = FALSE;
    int    *pacchTrgs;                  /* Lengths of base target names. */

    /*
     * Arguments.
     */
    int     fRecursive = FALSE;
    int     fNoStatus = FALSE;
    int     fQuiet = FALSE;
    char  **papszSrcs = NULL;
    int     cSrcs = 0;
    char  **papszTrgs = NULL;
    int     cTrgs = 0;

    /*
     * Statistics
     */
    unsigned long   ulTotalDuration = 0;
    long double     lrdTotalBytes = 0.0;
    unsigned long   cTotalFiles = 0;


    /*
     * Parse arguments.
     */
    for (argi = 1; argi < argc; argi++)
    {
        /*
         * Option.
         */
        if (argv[argi][0] == '/' || argv[argi][0] == '-')
        {
            switch (argv[argi][1])
            {
                case 'r':
                case 'R':
                case 's':
                case 'S':
                    fRecursive = 1;
                    break;

                case 'q':
                case 'Q':
                    fQuiet = 1;
                    break;

                case 'n':
                case 'N':
                    fNoStatus = 1;
                    break;

                case '\0':
                    fTargets = 1;
                    break;

                default:
                    fprintf(stderr, "syntax error: Invalid option '%s'\n", argv[argi]);
                case 'h':
                case 'H':
                case '?':
                    syntax();
                    return 12;
            }
        }
        /*
         * File.
         */
        else
        {
            char ***ppapszFiles = &papszSrcs;
            int    *pcFiles = &cSrcs;
            if (fTargets)
            {
                ppapszFiles = &papszTrgs;
                pcFiles = &cTrgs;
                if (    strchr(argv[argi], '*')
                    ||  strchr(argv[argi], '?'))
                {
                    fprintf(stderr, "syntax error: wildchars not allowed in target specifier!\n");
                    return 12;
                }
            }

            /*
             * Reallocate the array.
             */
            *ppapszFiles = realloc(*ppapszFiles, (*pcFiles + 2) * sizeof(char*));
            if (!*ppapszFiles)
            {
                fprintf(stderr, "fatal error: failed to allocate memory for files.\n");
                return -16;
            }

            /*
             * Add the file and marking the end with a NULL.
             */
            (*ppapszFiles)[*pcFiles] = argv[argi];
            (*pcFiles)++;
            (*ppapszFiles)[*pcFiles] = NULL;
        }
    }  /* arg loop */

    /*
     * Check that we have source files.
     */
    if (!papszSrcs)
    {
        fprintf(stderr, "syntax error: no source files specified.\n");
        return 12;
    }

    /*
     * If not targets specified they are copied to the current directory.
     */
    if (!papszTrgs)
    {
        papszTrgs = malloc(sizeof(char *) * 2);
        if (!papszTrgs)
        {
            fprintf(stderr, "fatal error: malloc failed\n");
            return 16;
        }
        papszTrgs[0] = ".";
        papszTrgs[1] = NULL;
        cTrgs = 1;
    }


    /*
     * Preprocess the targets making them either valid filenames or directories.
     */
    pacchTrgs = malloc(cTrgs * sizeof(int));
    if (!pacchTrgs)
    {
        fprintf(stderr, "fatal error: malloc failed (3)\n");
        return 16;
    }

    for (i = 0; i < cTrgs; i++)
    {
        char *  psz = papszTrgs[i];
        int     cch = strlen(psz);

        /*
         * Allocate a max filename buffer and copy the target
         * specifier to it.
         */
        papszTrgs[i] = malloc(MAX_FILENAME);
        if (!papszTrgs)
        {
            fprintf(stderr, "fatal error: malloc failed (2)\n");
            return 16;
        }

        strcpy(papszTrgs[i], psz);

        /*
         * Directory:
         */
        if (    psz[cch-1] == '\\' || psz[cch-1] == '/' || psz[cch-1] == ':' ||  cSrcs > 1
            ||  IsDirectory(psz))
        {
            if (psz[cch-1] == ':')
            {
                #ifdef OS2
                ULONG cchBuffer;
                #endif
                if (cch != 2)
                {
                    fprintf(stderr, "error: invalid target name '%s'\n", psz);
                    return 12;
                }
                #ifdef OS2
                cchBuffer = MAX_FILENAME;
                rc = DosQueryCurrentDir(toupper(*psz) - 'A' + 1, papszTrgs[i], &cchBuffer);
                #endif
                if (rc)
                {
                    fprintf(stderr, "error: failed to get current dir for '%s'. rc=%d\n", psz, rc);
                    return 12;
                }
            }
            else if (psz[cch-1] != '\\' && psz[cch-1] != '/')
                strcat(papszTrgs[i], "\\");

            /*
             * Set the base length.
             */
            pacchTrgs[i] = strlen(papszTrgs[i]);
        }
        /*
         * File: base length of -1 to indicate file.
         */
        else
            pacchTrgs[i] = -1;
    }


    /*
     * Create semaphores.
     */
    if (    SemEvtCreate(&evtReader)
        ||  SemEvtCreate(&evtReader2)
        ||  SemEvtCreate(&evtWriter)
        ||  SemMtxCreate(&mtxList))
    {
        fprintf(stderr, "fatal error: failed to create one or more sempahores.\n");
        return 16;
    }


    /*
     * Start reader thread.
     */
    if (_beginthread(reader, NULL, 0x10000, NULL) < 0)
    {
        fprintf(stderr, "fatal error: failed to start reader thread. errno=%d\n", errno);
        return 16;
    }


    /*
     * Copy the source files specified.
     */
    for (iSrc = 0; iSrc < cSrcs; iSrc++)
    {
        char    szSearch[MAX_FILENAME];
        char    ch;
        char   *pszFile;

        strcpy(szSearch, papszSrcs[iSrc]);

        /*
         * Directory specifier? (ie. ending with a slash or being a directory)
         */
        if (   (ch = szSearch[strlen(szSearch) - 1] == '\\')
            || ch == '/'
            || ch == ':'
            || IsDirectory(szSearch)
            )
        {
            if (ch == '\\' || ch == '/')
                strcat(szSearch, "*");
            else
                strcat(szSearch, "\\*");
        }

        /*
         * Search for source files.
         */
        rc = EnumFiles(szSearch, &pszFile);
        if (!rc)
        {
            do
            {
                long double     lrdBytes;
                unsigned long   ulDuration;
                char *pszFileNoPath = FileFilename(pszFile);

                if (!fQuiet)
                    printf("%s -> ", pszFile);

                /*
                 * Build array of targets.
                 */
                for (i = 0; i < cTrgs; i++)
                {
                    if (pacchTrgs[i] > 0)
                        strcpy(&papszTrgs[i][pacchTrgs[i]], pszFileNoPath);
                    if (!fQuiet)
                        printf(i ? ", %s" : "%s", papszTrgs[i]);
                }
                if (!fQuiet)
                    printf("\n"); fflush(stdout);

                /*
                 * Do copy.
                 */
                ulDuration = gettimems();
                rc = CopyFileToFiles(pszFile, papszTrgs, cTrgs);
                ulDuration = gettimems() - ulDuration;
                if (rc)
                    break;

                /*
                 * Statistics.
                 */
                lrdBytes = FileSize(pszFile);
                cTotalFiles++;
                ulTotalDuration += ulDuration;
                lrdTotalBytes += lrdBytes * cTrgs;
                if (!ulDuration) ulDuration = 1;
                if (!fQuiet)
                    printf(" Copied %ld bytes in %ld msec - %.2f MB/s\n",
                           (unsigned long)lrdBytes,
                           ulDuration,
                           (double)(lrdBytes / (ulDuration * 1000)));

            } while (!EnumFiles(NULL, &pszFile));
        }
        else
        {
            fprintf(stderr, "error: %s didn't result in any files\n", szSearch);
            iRc = 8;
        }
    }

    /*
     * Try make read thread terminate nicely.
     */
    fTerminate = TRUE;
    SemEvtPost(evtReader);
    SemEvtPost(evtReader2);
    #ifdef OS2
    DosSleep(0);
    #endif

    /*
     * Print statistics
     */
    if (!fNoStatus)
    {
        if (!ulTotalDuration) ulTotalDuration = 1;
        printf(" Copied %.0f bytes / %ld files in %d msec - %.2f MB/s\n",
               (double)lrdTotalBytes,
               cTotalFiles,
               ulTotalDuration,
               (double)(lrdTotalBytes / (ulTotalDuration * 1000)));
    }

    return iRc;
}
