/* $Id: BindArgs.c,v 1.4 2001/06/11 22:45:44 bird Exp $
 *
 * Bind Common Argument Routines.
 *
 * Copyright (c) 2001 knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 *
 * GPL.
 *
 */

/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include <string.h>
#include <stdio.h>

#include "BindLib.h"


/**
 * This function reads and validates a string input option.
 * examples: -cCOLLECTIONID or -o OWNER.
 * @returns 0 on success.
 *          -1 on error.
 */
int BindGetOption(int argc, char **argv, int *pargi,
                  char *pszOption, int cchMax, int *pfOption, int fClearAllowed,
                  const char *pszDesc, const char *pszVerbose, int cVerbose)
{
    char *  pszOpt = argv[*pargi];
    char *  psz;
    int     cch;

    if (argv[*pargi][2] == '\0' && argc <= *pargi + 1)
    {
        BindSyntaxError("Option %.2s requires %s.\n", pszOpt, pszDesc);
        return -1;
    }

    psz = argv[*pargi][2] != '\0' ? &argv[*pargi][2] : argv[++*pargi];
    cch = strlen(psz);
    if (cch > cchMax)
    {
        BindSyntaxError("Option %.2s don't accept %s that is longer than %d chars.\n",
                        pszOpt, pszDesc, cchMax - 1);
        return -1;
    }

    if (cch <= 0)
    {
        BindSyntaxError("Option %.2s requires %s.\n", pszOpt, pszDesc);
        return -1;
    }

    *pfOption = OPTION_SET;
    if (cch == 1)
    {
        if (*psz == '*')
            *pfOption = OPTION_IGNORE;
        else if (fClearAllowed && *psz == '-')
            *pfOption = OPTION_CLEAR;
    }

    if (*pfOption == OPTION_SET)
    {
        memcpy(pszOption, psz, cch + 1);
        strupr(pszOption);
    }
    else
        *pszOption = '\0';

    if (cVerbose >= 3)
    {
        if (*pfOption == OPTION_SET)
            BindInfo("%s is set to '%s'.\n", pszVerbose, pszOption);
        else if (*pfOption == OPTION_CLEAR)
            BindInfo("%s is to be cleared.\n", pszVerbose);
        else
            BindInfo("%s isn't changed.\n", pszVerbose);
    }

    return 0;
}


