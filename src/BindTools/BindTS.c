/* $Id: BindTS.c,v 1.3 2001/06/26 21:04:35 bird Exp $
 *
 * Timestamp converter.
 *
 * Copyright (c) 2001 knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 *
 * GPL
 *
 */


/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include <string.h>

#include "BindLib.h"


/*******************************************************************************
*   Global Variables                                                           *
*******************************************************************************/
static const char szConv[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890@#$_";


/**
 * Maps a singel timestamp character to a number value.
 * @returns -1 on error.
 *          Number (positive) which the character (ch) represents.
 * @param   ch  Character to map to number.
 */
_Inline int ToNum(char ch)
{
    char *psz;
    if (ch == '\0')
        return -1;
    psz = strchr(szConv, ch);
    if (psz == NULL)
        return -1;
    return psz - szConv;
}


/**
 * Maps a number value to a single timestamp character.
 * @returns valid character on success. '\0' on error.
 * @param   i   Value to convert. Must be less than 67!
 */
_Inline char ToChar(int i)
{
    return i < 67 ? szConv[i] : '\0';
}


/**
 * Converts a DB2 Bind timestamp to a human readable date.
 * @returns 0 on success.
 *          -1 on error.(invalid timestamp)
 * @param   pszTimeStamp    Input timestamp string. Assumes it is 8 chars long.
 * @param   pTS             Output structure.
 * @sketch
 * @status  Completely implemented.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 * @remark  Let's hope this don't change for later releases of DB2.
 */
int BindCvtTS(const char *pszTimeStamp, PTIMESTAMP pTS)
{
    int i;
    int rc = 0;

    pTS->iYear   = (i= ToNum(pszTimeStamp[7])) + 1984;
    if (i < 0)  rc = -1;
    pTS->iMonth  = i = ToNum(pszTimeStamp[6]);
    if (i < 0)  rc = -1;
    pTS->iDay    = i = ToNum(pszTimeStamp[5]);
    if (i < 0)  rc = -1;
    pTS->iHour   = i = ToNum(pszTimeStamp[4]);
    if (i < 0)  rc = -1;
    pTS->iMinutte= i = ToNum(pszTimeStamp[3]);
    if (i < 0)  rc = -1;
    pTS->iSecond = i = ToNum(pszTimeStamp[2]);
    if (i < 0)  rc = -1;
    pTS->i100    = i = ToNum(pszTimeStamp[1]) * 62;
    if (i < 0)  rc = -1;
    pTS->i100   += (i= ToNum(pszTimeStamp[0]));
    if (i < 0)  rc = -1;

    return rc;
}


/**
 * Converts a DB2 Bind timestamp to a human readable date.
 * @returns 0 on success.
 *          -1 on error.(invalid timestamp)
 * @param   pszTimeStamp    Input timestamp string. Assumes it is 8 chars long.
 * @param   pTS             Output structure.
 * @sketch
 * @status  Completely implemented.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 * @remark  Let's hope this don't change for later releases of DB2.
 */
int BindMakeTS(char *pszTimeStamp, PTIMESTAMP pTS)
{
    int i;
    int rc = 0;

    /*
     * Validate
     */
    if (    pTS->iYear < 1984
        ||  pTS->iYear > 1984+67
        ||  pTS->iMonth < 1
        ||  pTS->iMonth > 12
        ||  pTS->iDay < 1
        ||  pTS->iDay > 31
        ||  pTS->iHour > 23
        ||  pTS->iHour < 0
        ||  pTS->iMinutte > 59
        ||  pTS->iMinutte < 0
        ||  pTS->iSecond > 59
        ||  pTS->iSecond < 0
        ||  pTS->i100 < 0
        ||  pTS->i100 > 99
        )
        return -1;

    pszTimeStamp[7] = ToChar(pTS->iYear - 1984);
    pszTimeStamp[6] = ToChar(pTS->iMonth);
    pszTimeStamp[5] = ToChar(pTS->iDay);
    pszTimeStamp[4] = ToChar(pTS->iHour);
    pszTimeStamp[3] = ToChar(pTS->iMinutte);
    pszTimeStamp[2] = ToChar(pTS->iSecond);
    pszTimeStamp[1] = ToChar(pTS->i100 / 62);
    pszTimeStamp[0] = ToChar(pTS->i100 % 62);

    return rc;
}


/**
 * Converts a number string to a number.
 * @returns The number which the string represent. (Positive integer)
 *          -1 on error.
 * @param   psz     Pointer to the string.
 * @param   cch     Number of chars.
 * @param   iMin    Min value for this number.
 * @param   iMax    Max value for this number.
 */
_Inline int Num(const char *psz, int cch, int iMin, int iMax)
{
    int i = 0;

    while (cch-- > 0)
    {
        if (*psz > '9' || *psz < '0')
            return -1;
        i *= 10;
        i += *psz++ - '0';
    }

    if (iMax < i)
        return -1;

    return i;
}



int BindMakeTSISO(char *pszTimeStamp, const char *pszISO)
{
    int         cchISO = strlen(pszISO);
    TIMESTAMP   TS;

    /*
     * Validate ISO timestamp.
     * yyyy-mm-dd-hh.mm.ss.xxxxxx
     * 012345678901234567890123456
     */
    if (    cchISO != 26
        ||  pszISO[4]  != '-'
        ||  pszISO[7]  != '-'
        ||  pszISO[10] != '-'
        ||  pszISO[13] != '.'
        ||  pszISO[16] != '.'
        ||  pszISO[19] != '.'
        )
    {
        return -1;
    }

    /*
     * Read the ISO timestamp.
     */
    TS.iYear    = Num(pszISO +  0, 4, 1, 9999);
    if (TS.iYear < 0)       return TS.iYear;
    TS.iMonth   = Num(pszISO +  5, 2, 1, 12);
    if (TS.iMonth < 0)      return TS.iMonth;
    TS.iDay     = Num(pszISO +  8, 2, 1, 31);
    if (TS.iDay < 0)        return TS.iDay;
    TS.iHour    = Num(pszISO + 11, 2, 0, 23);
    if (TS.iHour < 0)       return TS.iHour;
    TS.iMinutte = Num(pszISO + 14, 2, 0, 59);
    if (TS.iMinutte < 0)    return TS.iMinutte;
    TS.iSecond  = Num(pszISO + 17, 2, 0, 59);
    if (TS.iSecond < 0)     return TS.iSecond;
    TS.i100     = Num(pszISO + 20, 6, 1, 999999); /* validation only */
    if (TS.i100 < 0)        return TS.i100;
    TS.i100     = Num(pszISO + 20, 2, 1, 99);
    if (TS.i100 < 0)        return TS.i100;

    /*
     * Make bind timestamp.
     */
    return BindMakeTS(pszTimeStamp, &TS);
}



/**
 * Converts an encoded number to a interger.
 * @returns The decoded number (positiv integer).
 *          -1 on error.
 * @param   pszNum  Pointer to the string.
 * @param   cchNum  Size of the string part which is to be converted.
 *                  If <= 0 then we'll convert the entrie string.
 * @status  Completely implemented.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 * @remark  Suitble for ProgId size and rev numbers.
 */
int     BindToNum(const char *pszNum, int cchNum)
{
    int         iNum = 0;
    const char *psz = pszNum + (cchNum <= 0 ? strlen(pszNum) : cchNum);

    while (pszNum < psz--)
    {
        int j = ToNum(*psz);
        if (j < 0)
            return -1;
        iNum *= 62;
        iNum += j;
    }

    return iNum;
}

