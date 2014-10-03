/* $Id: BindTmpF.c,v 1.2 2001/06/05 15:28:32 bird Exp $
 *
 * Make Temporary file name.
 *
 * Copyright (c) 2001 knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 *
 * GPL.
 *
 */


/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <process.h>
#include "BindLib.h"

/**
 * Generates a temporary filename for use by the bind tools.
 * @returns Pointer to pszFileBuf on success.
 *          NULL on failure (will never happen though).
 * @param   pszFileBuf  Pointer to filename output buffer.
 *                      On successful return the name of the file
 *                      is in this buffer.
 * @param   pszExt      Pointer to extension string. This is optional.
 * @sketch  Get directory for temporary files.
 *          Make temporary file
 * @status
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 * @remark
 */
char *BindTmpFile(char *pszFileBuf, const char *pszExt)
{
    char *  pszTmp;
    int     cchTmp;
    int     i;

    /*
     * Determin Temp directory.
     */
    if (    !(pszTmp = getenv("TMP"))
        &&  !(pszTmp = getenv("TEMP"))
        &&  !(pszTmp = getenv("TEMPDIR"))
        )
        pszTmp = "\\tmp\\";
    cchTmp = strlen(pszTmp);
    strcpy(pszFileBuf, pszTmp);
    if (   pszFileBuf[cchTmp-1] != '\\'
        && pszFileBuf[cchTmp-1] != '/'
        && pszFileBuf[cchTmp-1] != ':')
        strcpy(pszFileBuf + cchTmp++, "\\");


    /*
     * Make temporary filename...
     */
    i = (time(NULL) + getpid() * 0x3456) % 0x100000;
    if (pszExt)
        sprintf(pszFileBuf + cchTmp, "bt%06x.%3.3s",
                i, pszExt);
    else
        sprintf(pszFileBuf + cchTmp, "bt%06x.%03x",
                i, getpid() % 0x1000);

    return pszFileBuf;
}


