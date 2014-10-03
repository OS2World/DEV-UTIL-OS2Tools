/* $Id: rm.c,v 1.9 2002/09/20 05:00:52 bird Exp $
 *
 * Mini rm for the build system.
 *
 * The only intention is to be *much* faster than
 * the EMX base port of GNU rm.
 *
 * Copyright (c) 2002 knut st. osmundsen <bird@anduin.net>
 *
 *
 * This file is part of Generic Buildsystem.
 *
 * Generic Buildsystem is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Generic Buildsystem is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Generic Buildsystem; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*******************************************************************************
*   Defined Constants And Macros                                               *
*******************************************************************************/
//#define DEBUGTRACE
#if defined(DEBUGTRACE)
#define dprintf(a)  do {printf("dbg - "); printf a; fflush(stdout);} while (0)
#else
#define dprintf(a)  do {} while(0)
#endif


/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#ifdef OS2
#define INCL_BASE
#define INCL_ERRORS
#include <os2.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

/*******************************************************************************
*   Global Variables                                                           *
*******************************************************************************/
#ifdef OS2
#pragma data_seg(BUFFER64K)
char achFileBuf[0x10000] = {0};
#pragma data_seg()
#endif

int fTest = 0;                          /* -t flag. */
int fForce = 0;                         /* -f flag. */
int fRecursive = 0;                     /* -R flag. */
int fVerbose = 0;                       /* -v flag. */
#ifdef OS2
int fForceAPI = 0;                      /* -F flag. */
#endif


/*******************************************************************************
*   Internal Functions                                                         *
*******************************************************************************/
int DeleteFiles(const char *pszFiles, int fEmptyOk);
int DeleteDirs(const char *pszDir, int fFirst);

int DeleteFiles(const char *pszFiles, int fEmptyOk)
{
    /*
     * file mask or file.
     */
    #ifdef OS2
    static  char achFile[CCHMAXPATH];
    ULONG   cFiles = 0x1000;
    HDIR    hDir = HDIR_CREATE;
    APIRET  rcApi;
    #endif
    char *  psz;
    int     rc = 0;
    dprintf(("DeleteFiles(%s)\n", pszFiles));

    /*
     * Parse out the directory name.
     */
    psz = strrchr(pszFiles, '\\');
    if (!psz)
        psz = strrchr(pszFiles, '/');
    if (!psz)
        psz = (char*)&pszFiles[pszFiles[1] == ':' ? 1 : -1];
    psz++;
    strncpy(&achFile[0], pszFiles, psz - pszFiles);
    psz = &achFile[psz - pszFiles]; /* Now points to where the filename is to be added! */
    dprintf(("dir=%s\n", &achFile[0], *psz = '\0'));

    /*
     * OS specific search and delete loop.
     */
    #ifdef OS2
    /* Query OS loop */
    rcApi = DosFindFirst((char*)pszFiles,
                         &hDir,
                         fForce ? FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM | FILE_ARCHIVED : FILE_NORMAL,
                         &achFileBuf[0], sizeof(achFileBuf),
                         &cFiles,
                         FIL_STANDARD);
    dprintf(("findfirst: rc=%d files=%d\n", rcApi, cFiles));
    if (!rcApi)
    {
        do
        {
            /* Buffer loop */
            PFILEFINDBUF3   pBuf;
            for (pBuf = (PFILEFINDBUF3)&achFileBuf[0];
                 cFiles > 0;
                 cFiles--, pBuf = (PFILEFINDBUF3)((char*)pBuf + pBuf->oNextEntryOffset)
                 )
            {
                strcpy(psz, pBuf->achName);
                dprintf(("file: %s\n", &achFile[0]));
                if (!(pBuf->attrFile & FILE_DIRECTORY))
                {
                    if (fVerbose) puts(&achFile[0]);
                    if (!fTest)
                    {
                        if (fForceAPI)
                            rcApi = DosForceDelete(&achFile[0]); /* this is way faster! */
                        else
                            rcApi = DosDelete(&achFile[0]);
                        if (rcApi)
                        {
                            if (fForce && rcApi == ERROR_ACCESS_DENIED)
                            {   /* turn off the readonly, system and hidden attributtes */
                                FILESTATUS3     fsts3 = {0};
                                if (!DosQueryPathInfo(&achFile[0], FIL_STANDARD, &fsts3, sizeof(fsts3)))
                                {
                                    fsts3.attrFile = (fsts3.attrFile & ~(FILE_READONLY|FILE_SYSTEM|FILE_HIDDEN));
                                    rcApi = DosSetPathInfo(&achFile[0], FIL_STANDARD, &fsts3, sizeof(fsts3), 0);
                                    if (fForceAPI)
                                        rcApi = DosForceDelete(&achFile[0]); /* this is way faster! */
                                    else
                                        rcApi = DosDelete(&achFile[0]);
                                }
                            }

                            if (rcApi)
                            {
                                printf("error: rc=%d deleting file '%s'\n", rcApi, &achFile[0]);
                                rc = 8;
                            }
                        }
                    }
                }
                else dprintf(("directory: %s\n", achFile));
            }

            /* next chunk */
            cFiles = 0x1000;
            rcApi = DosFindNext(hDir, &achFileBuf[0], sizeof(achFileBuf), &cFiles);
            dprintf(("findnext: rc=%d files=%d\n", rcApi, cFiles));
        } while (!rcApi);
        /* close dir handle */
        DosFindClose(hDir);

        /* check for unexpected errors from DosFindNext */
        if (rcApi != ERROR_NO_MORE_FILES)
        {
            printf("error: DosFindNext rc=%d files '%s'\n", rcApi, pszFiles);
            rc = 8;
        }
    }
    else
    {   /* find first failed
         * If fForce is set we will keep quiet if any type of file not found error!
         */
        if (    !fEmptyOk
            ||  (   rcApi != ERROR_FILE_NOT_FOUND
                 && rcApi != ERROR_NO_MORE_FILES
                 && rcApi != ERROR_PATH_NOT_FOUND
                 )
            )
        {
            printf("error: DosFindFirst rc=%d files '%s'\n", rcApi, pszFiles);
            rc = 8;
        }
    }
    #endif
    return rc;
}


int DeleteDirs(const char *pszDir, int fFirst)
{
    /*
     * file mask or file.
     */
    #ifdef OS2
    char    achDir[CCHMAXPATH];
    char    achBuffer[1024];
    ULONG   cDirs = 0x1000;
    HDIR    hDir = HDIR_CREATE;
    APIRET  rcApi;
    #endif
    int     fWildchar = 1;
    char *  psz;
    int     rc;
    dprintf(("DeleteDirs(%s)\n", pszDir));
    if (fVerbose) puts(pszDir);

    /*
     * Prepare directory part.
     * If we have a wildchar search for files and directories we'll
     * proceed with the search expr passed in.
     * Else we append a search expr to the string passed in.
     */
    psz = strrchr(pszDir, '\\');
    if (!psz)
        psz = strrchr(pszDir, '/');
    if (!psz)
        psz = (char*)&pszDir[pszDir[1] == ':' ? 1 : -1];
    psz++;

    strcpy(&achDir[0], pszDir);
    if (!strchr(psz, '*') && !strchr(psz, '?'))
    {   /* no wildchars! The last part must be used as directory name. */
        psz = &achDir[strlen(&achDir[0])];
        if (psz[-1] != ':' && psz[-1] != '\\' && psz[-1] != '/')
            *psz++ = '\\';
        dprintf(("dir-1: %s\n", &achDir[0], *psz = '\0'));
        psz[0] = '*';
        psz[1] = '\0';                      /* Now points where we should add subdirectory names. */
        fWildchar = 0;
    }
    else
    {   /* wildchar expr passed in */
        psz = &achDir[psz - pszDir]; /* Now points to where the filename is to be added! */
        dprintf(("dir-2: %s\n", &achDir[0]));
    }


    /*
     * Remove all files in the directory or the files specified.
     */
    rc = DeleteFiles(&achDir[0], 1);
    if (rc > 8)                         /* accept all non fatal errors. */
        return rc;


    /*
     * OS specific search and delete loop.
     */
    #ifdef OS2
    /* Query OS loop */
    rcApi = DosFindFirst(&achDir[0],
                         &hDir,
                         MUST_HAVE_DIRECTORY | (fForce ? FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM | FILE_ARCHIVED : 0),
                         &achBuffer[0], sizeof(achBuffer),
                         &cDirs,
                         FIL_STANDARD);
    dprintf(("findfirst-d: rc=%d dirs=%d\n", rcApi, cDirs));
    if (!rcApi)
    {
        do
        {
            /* Buffer loop */
            PFILEFINDBUF3   pBuf;
            for (pBuf = (PFILEFINDBUF3)&achBuffer[0];
                 cDirs > 0;
                 cDirs--, pBuf = (PFILEFINDBUF3)((char*)pBuf + pBuf->oNextEntryOffset)
                 )
            {
                /*
                 * Skip files, '.' and '..'.
                 */
                if (pBuf->attrFile & FILE_DIRECTORY
                    && !(   pBuf->achName[0] == '.'
                         && (   pBuf->achName[1] == '\0'
                             || (pBuf->achName[1] == '.' && pBuf->achName[2] == '\0')
                             )
                         )
                    )
                {
                    /*
                     * Make fullname.
                     */
                    strcpy(psz, pBuf->achName);
                    dprintf(("dir-d: %s\n", &achDir[0]));

                    /*
                     * Process subdirectories - skip '.' and '..'.
                     */
                    rc = DeleteDirs(&achDir[0], 0);
                    if (rc > 8)
                    {
                        DosFindClose(hDir);
                        return rc;
                    }

                    /*
                     * Remove the directory.
                     */
                    if (!fTest)
                    {
                        rcApi = DosDeleteDir(&achDir[0]);
                        if (rcApi && rcApi != ERROR_PATH_NOT_FOUND)
                        {
                            printf("error: rc=%d deleting dir '%s'\n", rcApi, &achDir[0]);
                            rc = 8;
                        }
                    }
                }
            }

            /* next chunk */
            cDirs = 0x1000;
            rcApi = DosFindNext(hDir, &achBuffer[0], sizeof(achBuffer), &cDirs);
            dprintf(("findnext-d: rc=%d dirs=%d\n", rcApi, cDirs));
        } while (!rcApi);
        /* close dir handle */
        DosFindClose(hDir);

        /* check for unexpected errors from DosFindNext */
        if (rcApi != ERROR_NO_MORE_FILES)
        {
            printf("error: DosFindNext rc=%d dir '%s'\n", rcApi, pszDir);
            rc = 8;
        }
    }
    else
    {   /* find first failed
         * If fForce is set we will keep quiet if any type of file not found error!
         */
        if (    fFirst
            &&  (
                     !fForce
                ||  (   rcApi != ERROR_FILE_NOT_FOUND
                     && rcApi != ERROR_NO_MORE_FILES
                     && rcApi != ERROR_PATH_NOT_FOUND
                     )
                 )
            )
        {
            printf("error: DosFindFirst rc=%d dir '%s'\n", rcApi, pszDir);
            rc = 8;
        }
    }
    #endif


    /*
     * Remove the directory if no wildchars was specified.
     */
    if (rc <= 8 && !fWildchar)
    {
        if (!fTest)
        {
            psz[-1] = '\0';
            rcApi = DosDeleteDir(&achDir[0]);
            if (rcApi && rcApi != ERROR_PATH_NOT_FOUND)
            {
                printf("error: rc=%d deleting directory '%s'\n", rcApi, &achDir[0]);
                rc = 8;
            }
        }
    }

    return rc;
}


int syntax(void)
{
    printf("mini rm clone v1.2\n"
           "\n"
           "Syntax: rm [-f] [-d] [-?|-h|--help] [files] [@<respfile>]\n"
           "\n"
           "    -f, --force             Don't complain about missing files.\n"
           "                            Delete readonly, hidden and system files and subdirs.\n"
           "    -r, -R, --recursive     Delete subdirectories recursivly.\n"
           "    -v, --verbose           Show what's being done.\n"
           "    -t, --test              Nondestructive mode.\n"
           "    -?, -h, --help          This stuff and exit.\n"
           "        --version           Version string and exit.\n"
#ifdef OS2
           "    -F, --os2-force         Use the fast force api.\n"
           "                            This is 100x faster.\n"
#endif
           "\n"
           "    files     0 or more filenames to delete.\n"
           "              Filenames can contain wildchars even when quoted.\n"
           "    respfile  Response file. (if the commandline isn't long enough)\n"
           "              Multiple nested files are supported (careful!). \n"
           "\n"
           "WARNING! Arguments are processed as passed in!\n"
           "\n"
           "#include <strongest disclaimer>\n"
           "The author isn't responsible for anything!\n"
           "\n"
           "Copyright (c) 2002 knut st. osmundsen (bird@anduin.net)\n"
           );
    return 4;
}


int main(int argc, char **argv)
{
    int rc = 0;
    int argi;

    #ifdef OS2
    /*
     * The difference between DosForceDelete and DosDelete
     * is essentially a check for DELDIR + some other slowdowns for DosDelete.
     * If no DELDIR DosDelete will call DosForceDelete.
     */
    if (!getenv("DELDIR"))
        fForceAPI = 1;
    #endif

    /*
     * Work thru the parameters.
     */
    for (argi = 1; argi < argc; argi++)
    {
        dprintf(("arg%d: %s\n", argi, argv[argi]));
        if (argv[argi][0] != '-' && argv[argi][0] != '@')
        {
            int rc2;
            if (fRecursive)
            {
                int rc3;
                rc2 = DeleteDirs(argv[argi], 1);
                if (rc2 && rc > rc2)
                    rc = rc2;
                rc2 = DeleteFiles(argv[argi], 1);
            }
            else
                rc2 = DeleteFiles(argv[argi], fForce);
            if (rc2 && rc > rc2)
                rc = rc2;
        }
        else if (argv[argi][0] == '@')
        {   /*
             * Parameter file:
             *    Create a textbuffer of the entrie file.
             *    Parse the file and create a new parameter vector.
             *    Set argv to the new parameter vector, argi to 0 and argc to
             *      the parameter count.
             *    Restrictions: Stupid users is allowed to loop parameter files.
             */
            char *pszBuffer = NULL;
            FILE *phFile = fopen(&argv[argi][1], "rb");
            dprintf(("respfile: %s\n", &argv[argi][1]));
            if (phFile)
            {
                signed long cbFile;
                fseek(phFile, 0, SEEK_END);
                cbFile = ftell(phFile);
                if (cbFile >= 0)
                {
                    pszBuffer = (char*)malloc(cbFile + 2);
                    if (pszBuffer)
                    {
                        memset(pszBuffer, 0, cbFile + 2); /* !parser relies on two null terminators! */
                        fseek(phFile, 0, SEEK_SET);
                        if (cbFile > 0 && fread(pszBuffer, 1, cbFile, phFile) == 0)
                        {   /* failed! */
                            free(pszBuffer);
                            pszBuffer = NULL;
                        }
                    }
                    else
                    {
                        puts("fatal error: out of memory reading responsefile.\n");
                        return 16;
                    }
                }
                fclose(phFile);
            }

            if (pszBuffer)
            {
                char ** papszArgs = NULL;
                char *  psz = pszBuffer;
                int     iArg = 0;

                dprintf(("parsing responsefile\n"));
                while (*psz != '\0')
                {
                    char *pszEnd;
                    char  ch = *psz;

                    /* skip the start */
                    while (    (ch == ' ')
                            || (ch == '\t')
                            || (ch == '\n')
                            || (ch == '\r')
                          )
                        ch = *++psz;

                    /* find end of parameter word */
                    if (ch == '\"' || ch == '\'')
                    {   /* quoted */
                        char chQuoted = ch;
                        pszEnd = ++psz;
                        do
                        {
                            ch = *++pszEnd;
                        } while (ch != chQuoted && ch != '\n' && ch != '\r' && ch != '\0');
                    }
                    else
                    {   /* non quoted */
                        pszEnd = psz + 1;
                        while (    (ch != ' ')
                                && (ch != '\t')
                                && (ch != '\n')
                                && (ch != '\r')
                                && (ch != '\0')
                               )
                            ch = *++pszEnd;
                    }

                    /* skip empty arguments */
                    *pszEnd = '\0';
                    if (*psz)
                    {
                        /* allocate more arg array space? */
                        if ((iArg % 512) == 0)
                        {
                            papszArgs = (char**)realloc(papszArgs, sizeof(char*) * (512 + iArg));
                            if (papszArgs == NULL)
                            {
                                puts("fatal error: out of memory reading responsefile.\n");
                                return 16;
                            }
                        }
                        papszArgs[iArg++] = psz;
                        dprintf(("Arg-%02d: '%s' (%p %p\n", iArg, psz, psz, pszEnd));
                    }

                    /* next */
                    psz = pszEnd + 1;
                }
                dprintf(("completed parsing, iArg=%d\n", iArg));


                /*
                 * Add the remaining args.
                 */
                for (argi++; argi < argc; argi++)
                {
                    /* allocate more arg array space? */
                    if ((iArg % 512) == 0)
                    {
                        papszArgs = (char**)realloc(papszArgs, sizeof(char*) * (512 + iArg));
                        if (papszArgs == NULL)
                        {
                            puts("fatal error: out of memory reading responsefile.\n");
                            return 16;
                        }
                    }
                    papszArgs[iArg++] = argv[argi];
                }

                /*
                 * Install the new argument array and start processing it.
                 */
                argi = -1;
                argc = iArg;
                argv = papszArgs;
                dprintf(("new argi=%d\n", argi));
            }
            else
            {
                printf("error: could not open parameter file '%s'\n", &argv[argi][1]);
                return 8;
            }
        }
        else
        {   /*
             * Options
             */
            if (argv[argi][1] != '-')
            {
                int iOpt;
                for (iOpt = 1; argv[argi][iOpt]; iOpt++)
                {
                    switch (argv[argi][iOpt])
                    {
                        case 'f':   fForce = 1; break;
                        #ifdef OS2
                        case 'F':   fForceAPI = 1; break;
                        #endif
                        case 'r':
                        case 'R':   fRecursive = 1; break;
                        case 'v':   fVerbose = 1;   break;
                        case 't':   fTest = 1;  break;
                        case '?':
                        case 'h':   return syntax();
                        default:
                            printf("fatal error: invalid option! '%s'\n", argv[argi]);
                            return 12;
                    }
                }
            }
            else
            {   /* long options */
                if (!strcmp(&argv[argi][2], "force"))
                    fForce = 1;
                #ifdef OS2
                else if (!strcmp(&argv[argi][2], "os2-force"))
                    fForceAPI = 1;
                #endif
                else if (!strcmp(&argv[argi][2], "verbose"))
                    fVerbose = 1;
                else if (!strcmp(&argv[argi][2], "recursive"))
                    fRecursive = 1;
                else if (!strcmp(&argv[argi][2], "test"))
                    fTest = 1;
                else if (!strcmp(&argv[argi][2], "version"))
                {
                    printf("mini rm - v1.1\n");
                    return 0;
                }
                else
                {
                    printf("fatal error: invalid option! '%s'\n", argv[argi]);
                    return 12;
                }
            } /* short vs. long option */
        }
    }

    return rc;
}



