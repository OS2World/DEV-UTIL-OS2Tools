/* $Id: autochkdsk.c,v 1.2 2003/04/23 23:21:46 bird Exp $
 *
 * Simple program which queries volums and run an appropriate chkdsk on
 * them if they aren't ready.
 *
 * NOTE: use with caution, only tested with JFS and HPFS.
 *
 * Copyright (c) 2002 knut st. osmundsen <bird@anduin.net>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with This program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#define INCL_BASE
#include <os2.h>
#include <process.h>
#include <stdio.h>
#include <string.h>


/**
 * Checks if a disk needs chkdsk.
 * @returns TRUE if needed.
 * @returns FALSE if ok.
 * @param   chDrive
 */
BOOL needChkDsk(char chDrive)
{
    APIRET  rc;
    char    achBuffer[1024];
#if 0
    ULONG   cFiles = 1;
    HDIR    hDir = HDIR_CREATE;
    char    szSearch[16] = "c:\\*.*";
    szSearch[0] = chDrive;
    rc = DosFindFirst(&szSearch[0], &hDir, 0, &achBuffer[0], sizeof(achBuffer), &cFiles, FIL_STANDARD);
    if (rc == NO_ERROR || rc == ERROR_FILE_NOT_FOUND || rc == ERROR_NO_MORE_FILES)
    {
        DosFindClose(hDir);
        return FALSE;
    }
#else
    char    szPath[16] = "c:\\";
    szPath[0] = chDrive;
    rc = DosQueryPathInfo(szPath, FIL_STANDARD, &achBuffer[0], sizeof(achBuffer));
    if (rc == NO_ERROR)
    {
        return FALSE;
    }
#endif

    return TRUE;
}


/**
 * Gets the IFS name owning a volume (drive letter).
 * @returns Success indicator.
 * @param   chDrive     Drive in question.
 * @param   pszFSName   Pointer to output buffer.
 *                      sizeof() >= 8 chars!
 */
BOOL queryFSName(char chDrive, char * pszFSName)
{
    FSINFO      fsinfo;
    FSALLOCATE  fsalloc;
    union
    {
        FSQBUFFER2  fsqbuf2;
        char        buffer[256];
    }           ufsqbuf2;
    ULONG       cbfsqbuf2 = sizeof(ufsqbuf2);
    PSZ         pFSDName = NULL;
    CHAR        szDevNm[3] = "C:";
    APIRET      rc;

    szDevNm[0] = chDrive;
    rc = DosQueryFSAttach(&szDevNm[0], 0, FSAIL_QUERYNAME,
                          &ufsqbuf2.fsqbuf2, &cbfsqbuf2);
    if (!rc)
    {
        pFSDName = &ufsqbuf2.fsqbuf2.szName[ufsqbuf2.fsqbuf2.cbName] + 1;
        strcpy(pszFSName, pFSDName);
        return TRUE;
    }
    *pszFSName = '\0';
    return FALSE;
}


int main(void)
{
    int     chDrive;

    DosError(FERR_DISABLEHARDERR|FERR_DISABLEEXCEPTION);

    for (chDrive = 'C'; chDrive <= 'Z'; chDrive++)
    {
        CHAR szName[16];
        if (queryFSName(chDrive, &szName[0]))
        {
            char szChkDsk[80];
            printf("%c: [%s] - ", chDrive, &szName[0]);
            if (needChkDsk(chDrive))
            {
                char    szDrv[10];
                char *argvChild[10];

                printf("bad, chkdsk required! ");
                sprintf(szDrv, "%c:", chDrive);
                if (    !strcmp("FAT", &szName[0])
                    /* PARANOIA!!! */
                    ||  !strcmp("CDFS", &szName[0])
                    ||  !strcmp("NDFS", &szName[0])
                    ||  !strcmp("TVFS", &szName[0])
                    ||  !strcmp("LAN", &szName[0])
                    )
                {
                    argvChild[0] = NULL;
                    printf(" %s drive, skipping it!\n", szName[0]);
                }
                else if (!strcmp("JFS", &szName[0]))
                {
                    argvChild[0] = "jfschk32.exe";
                    argvChild[1] = szDrv;
                    argvChild[2] = "/F";
                    argvChild[3] = "/C";
                    argvChild[4] = NULL;
                }
                else
                {
                    argvChild[0] = "chkdsk.com";
                    argvChild[1] = szDrv;
                    argvChild[2] = "/F";
                    argvChild[3] = NULL;
                }

                if (argvChild[0])
                {
                    int rc;
                    printf("invoking: %s %s\n", argvChild[0], argvChild[1]);
                    rc = spawnvp(P_WAIT, argvChild[0], argvChild);
                    if (rc)
                        printf("spawnvp returned rc=%d drive=%c\n", rc, chDrive);
                }
            }
            else
                printf("ok\n");
        }
    }

    return 0;
}

