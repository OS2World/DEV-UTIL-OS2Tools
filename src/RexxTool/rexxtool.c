/**********************************************************************
*   RexxTool.C                                                        *
*                                                                     *
*   This program extends the REXX language by providing many          *
*   REXX external functions not available in REXXUTIL.                *
*                                                                     *
**********************************************************************/

/* TO DO
 
 "RtSetPathInfo"
 queryinfoblocks
 
 */

/*********************************************************************/
/*  Includes                                                         */
/*********************************************************************/

#define  INCL_VIO
#define  INCL_DOS
#define  INCL_BASE
#define  INCL_DOSFILEMGR
#define  INCL_DOSMEMMGR
#define  INCL_DOSMISC
#define  INCL_ERRORS
#define  INCL_REXXSAA
#include <rexxsaa.h>
#include <os2.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "rexxtool.h"



/*#define DEBUG 1*/
#undef DEBUG
#undef DEBUG_ENABLE

#define MAXPATHLEN 260

/*********************************************************************/
/*  Constants                                                        */
/*********************************************************************/

const static PSZ SZVERSION="0.1.1";


/*********************************************************************/
/*  Declare all exported functions as REXX functions.                */
/*********************************************************************/
RexxFunctionHandler RtDropFuncs;
RexxFunctionHandler RtLoadFuncs;
RexxFunctionHandler RtVersion;
RexxFunctionHandler RtStringSplit;
RexxFunctionHandler RtWordWrap;
RexxFunctionHandler RtQuerySysInfo;
RexxFunctionHandler RtFileCopy;
RexxFunctionHandler RtFileMove;
RexxFunctionHandler RtFileDelete;
RexxFunctionHandler RtProcessKill;
RexxFunctionHandler RtSetPriority;
RexxFunctionHandler RtQueryPathInfo;
RexxFunctionHandler RtQueryCtryInfo;
RexxFunctionHandler RtFileChecksum;
RexxFunctionHandler RtQueryModule;
RexxFunctionHandler RtReplaceModule;
RexxFunctionHandler RtQueryProcess;

/*
RexxFunctionHandler RtSetPathInfo;
RexxFunctionHandler RtQueryAppType;
RexxFunctionHandler RtStemSort
RexxFunctionHandler  RtStemCopy
RexxFunctionHandler  RtStringReplace
*/


/*********************************************************************/
/* RxFncTable                                                        */
/*   Array of names of the RtDLL functions.                       */
/*   This list is used for registration and deregistration.          */
/*********************************************************************/

static PSZ  RxFncTable[] =
   {
      "RtDropFuncs",
      "RtLoadFuncs",
      "RtVersion",
      "RtStringSplit",
      "RtWordWrap",
      "RtQuerySysInfo",
      "RtFileCopy",
      "RtFileMove",
      "RtFileDelete",
      "RtProcessKill",
      "RtSetPriority",
      "RtQueryPathInfo",
      "RtQueryCtryInfo",
      "RtFileChecksum",
      "RtQueryModule",
      "RtReplaceModule",
      "RtQueryProcess"
   };

static APIRET rexxStemInit   (RXSTEMDATA *pldp,
                              RXSTRING   *prxName);

static ULONG rexxStemFinalize(RXSTEMDATA *pldp);

static APIRET rexxStemAppend (RXSTEMDATA *pldp,
                              PSZ        pszData);

static ULONG rexxStemAppend2(RXSTEMDATA *pldp,
                             PSZ        pszData,
                             ULONG      ulLength);

static APIRET toolsCrc32String(PSZ    pszText,
                               PULONG pulCRC);

static APIRET toolsCrc32Buffer(PSZ    pszText,
                               ULONG  ulBufferLength,
                               PULONG pulCRC,
                               PULONG pulCRCCurrent);

static ULONG toolsCrc32BufferFinalize(ULONG pulCRC);

static APIRET toolsMD5Initialize (PMD5_CTX pmdContext);

static APIRET toolsMD5Update (PMD5_CTX pmdContext,
                              PSZ      pszInBuffer,
                              ULONG    ulInBufferLength);

static APIRET toolsMD5Finalize (PMD5_CTX pmdContext);


/*********************************************************************/
/****************  RexxTool Supporting Functions  ********************/
/*********************************************************************/


/******************************************************************************
 * Name      : static APIRET rexxStemInit
 * Funktion  : prepares a Rexx-Stem variable for usage
 * Parameter : RXSTEMDATA *pldp   - pointer to the rexx stem
 *             RXSTRING   prxName - pointer to the rexxstring name
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [1997/11/09]
 ******************************************************************************/

static APIRET rexxStemInit(RXSTEMDATA *pldp,
                           RXSTRING   *prxName)
{
  if ( (pldp    == NULL) ||                              /* check parameters */
       (prxName == NULL) )
     return (ERROR_INVALID_PARAMETER);              /* raise error condition */

  pldp->count = 0;
  strcpy(pldp->varname,                                     /* copy the name */
         prxName->strptr);
  pldp->stemlen = prxName->strlength;
  strupr(pldp->varname);                               /* uppercase the name */

  if (pldp->varname[pldp->stemlen-1] != '.')
    pldp->varname[pldp->stemlen++] = '.';

  return (NO_ERROR);                                                   /* OK */
}


/******************************************************************************
 * Name      : static APIRET rexxStemFinalize
 * Funktion  : close usage of a rexx stem
 * Parameter : RXSTEMDATA *pldp   - pointer to the rexx stem
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [1997/11/09]
 ******************************************************************************/

static ULONG rexxStemFinalize(RXSTEMDATA *pldp)
{
  sprintf(pldp->ibuf,
          "%d",
          pldp->count);
  pldp->varname[pldp->stemlen]    = '0';
  pldp->varname[pldp->stemlen+1]  = 0;
  pldp->shvb.shvnext            = NULL;
  pldp->shvb.shvname.strptr     = pldp->varname;
  pldp->shvb.shvname.strlength  = pldp->stemlen+1;
  pldp->shvb.shvnamelen         = pldp->stemlen+1;
  pldp->shvb.shvvalue.strptr    = pldp->ibuf;
  pldp->shvb.shvvalue.strlength = strlen(pldp->ibuf);
  pldp->shvb.shvvaluelen        = pldp->shvb.shvvalue.strlength;
  pldp->shvb.shvcode            = RXSHV_SET;
  pldp->shvb.shvret             = 0;
  if (RexxVariablePool(&pldp->shvb) == RXSHV_BADN)
    return INVALID_ROUTINE;                             /* error on non-zero */

  return (NO_ERROR);                                                   /* OK */
}


/******************************************************************************
 * Name      : static APIRET rexxStemAppend
 * Funktion  : write an entry to the stem
 * Parameter :          
 * Variablen :
 * Ergebnis  : Rexx-returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [1997/11/09]
 ******************************************************************************/

static ULONG rexxStemAppend(RXSTEMDATA *pldp,
                            PSZ        pszData)
{
  return (rexxStemAppend2(pldp,                        /* simply a forwarder */
                          pszData,
                          strlen(pszData)));
}


/******************************************************************************
 * Name      : static APIRET rexxStemAppend2
 * Funktion  : write an entry to the stem
 * Parameter :          
 * Variablen :
 * Ergebnis  : Rexx-returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [1998/05/27]
 ******************************************************************************/

static ULONG rexxStemAppend2(RXSTEMDATA *pldp,
                             PSZ        pszData,
                             ULONG      ulLength)
{
  /* @@@PH WARNING !!! what to do if ulLength exceeds 255 characters ? */
  memcpy(pldp->ibuf,
         pszData,
         ulLength);
  pldp->vlen = ulLength;

  pldp->count++;                                    /* increase STEM-counter */
  sprintf(pldp->varname+pldp->stemlen,
          "%d",
          pldp->count);

    pldp->shvb.shvnext            = NULL;
    pldp->shvb.shvname.strptr     = pldp->varname;
    pldp->shvb.shvname.strlength  = strlen(pldp->varname);
    pldp->shvb.shvnamelen         = pldp->shvb.shvname.strlength;
    pldp->shvb.shvvalue.strptr    = pldp->ibuf;
    pldp->shvb.shvvalue.strlength = pldp->vlen;
    pldp->shvb.shvvaluelen        = pldp->vlen;
    pldp->shvb.shvcode            = RXSHV_SET;
    pldp->shvb.shvret             = 0;
    if (RexxVariablePool(&pldp->shvb) == RXSHV_BADN)           /* problems ? */
      return INVALID_ROUTINE;                           /* error on non-zero */

  return (NO_ERROR);                                                   /* OK */
}


/*************************************************************************
* Function:  RtDropFuncs                                                 *
*                                                                        *
* Syntax:    call RtDropFuncs                                            *
*                                                                        *
* Return:    NO_UTIL_ERROR - Successful.                                 *
*************************************************************************/

ULONG RtDropFuncs(PUCHAR   name,
                  ULONG    numargs,
                  RXSTRING args[],
                  PSZ      queuename,
                  RXSTRING *retstr)
{
  INT     entries;                     /* Num of entries             */
  INT     j;                           /* Counter                    */

  if (numargs != 0)                    /* no arguments for this      */
    return INVALID_ROUTINE;            /* raise an error             */

  retstr->strlength = 0;               /* return a null string result*/

  entries = sizeof(RxFncTable)/sizeof(PSZ);

  for (j = 0;
       j < entries;
       j++)
    RexxDeregisterFunction(RxFncTable[j]);

  return VALID_ROUTINE;                /* no error on call           */
}



/*************************************************************************
* Function:  RtSplitString                                               *
*                                                                        *
* Syntax:    call RtStringSplit (stem,string,number,delimiter)           *
*                                                                        *
* Params:    stem      - Stem variable name to place results in.         *
*            string    - the source string                               *
*            number    - number of delimiters expected                   *
*            delimiter - the delimiter chars                             *
*                                                                        *
* Return:    NO_UTIL_ERROR   - Successful.                               *
*************************************************************************/

ULONG RtStringSplit(PUCHAR   name,
                    ULONG    numargs,
                    RXSTRING args[],
                    PSZ      queuename,
                    RXSTRING *retstr)
{
  PSZ         pszString;               /* target string              */
  PSZ         pszNumber;               /* number of delimiters       */
  long        lNumber;                 /* numerischer Abfragetyp     */
  ULONG       ulCounter;               /* loop counter               */
  PSZ         pszLast;                 /* loop counter               */
  PSZ         pszDelimiter;            /* the delimiter              */

                                       /* search                     */
  RXSTEMDATA  ldp;                     /* stem data                  */
  PSZ         pszTemp;        /* Tempor„rer Iterator ber den String */
  ULONG       len;                     /* Stringlaenge               */
  APIRET      rc;                                  /* API-returncode */


  BUILDRXSTRING(retstr, NO_UTIL_ERROR);/* pass back result           */
                                       /* validate arguments         */
  if (numargs != 4 ||
      !RXVALIDSTRING(args[0]) ||
      !RXVALIDSTRING(args[1]) ||
      !RXVALIDSTRING(args[2]) ||
      !RXVALIDSTRING(args[3]))
    return INVALID_ROUTINE;                    /* raise an error             */

                                                    /* Process the arguments */
  pszNumber    = args[2].strptr;                 /* Get number of delimiters */
  if ( string2long (pszNumber,&lNumber) == FALSE)         /* Conversion OK ? */
    return INVALID_ROUTINE;

  if (lNumber == 0)                           /* Also kein Counter erwnscht */
    lNumber = 0x7ffffffe;                      /* Auf der sicheren Seite ... */

  pszString    = args[1].strptr;                 /* What shall we work for ? */
  pszDelimiter = args[3].strptr;                     /* The mighty delimiter */
                                               /* Initialize data area       */

  rc = rexxStemInit(&ldp,                             /* initialize our stem */
                    &args[0]);
  if (rc != NO_ERROR)                                    /* check for errors */
     return (rc);                                   /* raise error condition */


  /*********************************************************/
  /* Ab hier sind alle Parameter geprft und es geht los.  */
  /*********************************************************/

  pszTemp = strchr(pszString,pszDelimiter[0]);        /* Stringzeiger setzen */
  pszLast = pszString;
  ulCounter = 0;                           /* Schleifenz„hler initialisieren */
  while ((*pszLast != 0) &&
         (pszTemp != NULL) &&
         (ulCounter < lNumber) )
       {
          len = pszTemp - pszLast;                /* calculate string length */
          rc = rexxStemAppend2(&ldp,                         /* append entry */
                               pszLast,
                               len);
          if (rc != NO_ERROR)
            return (rc);                            /* raise error condition */
         
          pszLast = pszTemp + 1;               /* Naechstes Token anvisieren */
                                                      /* Stringzeiger setzen */
          pszTemp = strchr(pszLast,pszDelimiter[0]);
          ulCounter++;
        }

                                                     /* letztes Token prfen */
  if ( (ulCounter >= lNumber) ||    /* D.h. Abbruchkriterium war der Zaehler */
       (pszTemp == NULL) )
    {
                                /* String in den RexxVariablenPool schreiben */
        len = strlen(pszLast);
        if (len != 0)               /* Wir waren nicht am Ende angelangt ... */
        {
          rc = rexxStemAppend(&ldp,
                              pszLast);                      /* append entry */
          if (rc != NO_ERROR)
            return (rc);                            /* raise error condition */
        }
    }

  rc = rexxStemFinalize(&ldp);                 /* set stem.0 to lines read   */
  if (rc != NO_ERROR)                                        /* check errors */
    return (rc);                                    /* raise error condition */

  return VALID_ROUTINE;                        /* no error on call           */
}


/*************************************************************************
* Function:  RtVersion                                                  *
*                                                                        *
* Syntax:    call RtVersion                                             *
*                                                                        *
* Return:    Version of RtDLL / RtEngine                               *
*************************************************************************/

ULONG RtVersion(PUCHAR   name,
                ULONG    numargs,
                RXSTRING args[],
                PSZ      queuename,
                RXSTRING *retstr)
{
  if (numargs != 0)                    /* validate arg count         */
    return INVALID_ROUTINE;

  strcpy(retstr->strptr,
         SZVERSION);
  retstr->strlength = strlen(retstr->strptr);
  return VALID_ROUTINE;
}


/*************************************************************************
* Function:  RtDLLVersion                                                *
*                                                                        *
* Syntax:    call RtDLLVersion                                           *
*                                                                        *
* Return:    Version of RtDLL / RtEngine                                 *
*************************************************************************/

PSZ RtDLLVersion(void)
{
  return (SZVERSION);                   /* return pointer on static data */
}


/*************************************************************************
* Function:  RtLoadFuncs                                                 *
*                                                                        *
* Syntax:    call RtLoadFuncs [option]                                   *
*                                                                        *
* Params:    none                                                        *
*                                                                        *
* Return:    null string                                                 *
*************************************************************************/

ULONG RtLoadFuncs(PUCHAR   name,
                  ULONG    numargs,
                  RXSTRING args[],
                  PSZ      queuename,
                  RXSTRING *retstr)
{
  INT    entries;                      /* Num of entries             */
  INT    j;                            /* Counter                    */

  retstr->strlength = 0;               /* set return value           */
                                       /* check arguments            */
  if (numargs > 0)
    return INVALID_ROUTINE;

  entries = sizeof(RxFncTable)/sizeof(PSZ);

  for (j = 0;
       j < entries;
       j++)
  {
    RexxRegisterFunctionDll(RxFncTable[j],
                            "REXXTOOL",
                            RxFncTable[j]);
  }
  return VALID_ROUTINE;
}


/*************************************************************************
* Function:  RtWordWrap                                                  *
*                                                                        *
* Syntax:    call RtWordWrap(stem,string,width)                          *
*                                                                        *
* Params:    stem      - Stem variable name to place results in.         *
*            string    - the source string                               *
*            width     - width of the strings to map to                  *
*                                                                        *
* Return:    NO_UTIL_ERROR   - Successful.                               *
*************************************************************************/

ULONG RtWordWrap (PUCHAR   name,
                  ULONG    numargs,
                  RXSTRING args[],
                  PSZ      queuename,
                  RXSTRING *retstr)
{
  PSZ         pszString;               /* target string              */
  PSZ         pszWidth;                /* number of delimiters       */
  long        lWidth;                  /* numerischer Abfragetyp     */
  PSZ         pszDelimiter;            /* delimiting chars           */

                                       /* search                     */
  RXSTEMDATA  ldp;                     /* stem data                  */
  PSZ         pszTemp;        /* Tempor„rer Iterator ber den String */
  PSZ         pszBegin;       /* Tempor„rer Iterator ber den String */
  PSZ         pszLast;        /* Tempor„rer Iterator ber den String */
  ULONG       ulTemp;                           /* temporary counter */
  LONG        iLen;                     /* Stringlaenge               */
  APIRET      rc;                                  /* API-Returncode */


  BUILDRXSTRING(retstr, NO_UTIL_ERROR);                  /* pass back result */
                                                       /* validate arguments */
  if (numargs < 3 ||
      !RXVALIDSTRING(args[0]) ||
      !RXVALIDSTRING(args[1]) ||
      !RXVALIDSTRING(args[2]))
    return INVALID_ROUTINE;                                /* raise an error */

  if (numargs == 4)                             /* 4. Parameter ueberpruefen */
  {
    if (!RXVALIDSTRING(args[3]) )                    /* check this parameter */
       return INVALID_ROUTINE;                             /* raise an error */
    else
      pszDelimiter = args[3].strptr;                 /* get the delimiterset */
  }
  else
    pszDelimiter = " ,;.:-";                      /* hardcoded delimiter set */

                                                    /* Process the arguments */
  pszWidth     = args[2].strptr;                 /* Get number of delimiters */
  if ( string2long (pszWidth,&lWidth) == FALSE)           /* Conversion OK ? */
    return INVALID_ROUTINE;

  if (lWidth == 0)                         /* Fehlerhafte Breite angegeben ? */
  {
    BUILDRXSTRING(retstr, ERROR_INVALID);            /* Fehler signalisieren */
    return VALID_ROUTINE;                                        /* finished */
  }

  pszString    = args[1].strptr;                 /* What shall we work for ? */
                                                     /* Initialize data area */
  rc = rexxStemInit(&ldp,                             /* initialize our stem */
                    &args[0]);
  if (rc != NO_ERROR)                                    /* check for errors */
     return (rc);                                   /* raise error condition */


  /*********************************************************/
  /* Ab hier sind alle Parameter geprft und es geht los.  */
  /*********************************************************/

  pszBegin = pszString;                       /* Basispointer initialisieren */
  pszLast  = NULL;

  while (*pszBegin == ' ')                          /* ignore leading spaces */
    pszBegin++;

  if (*pszBegin != 0)            /* wenn String nicht nur aus Spaces besteht */
  for (pszTemp  = pszBegin,                  /* scan the string char by char */
       ulTemp   = 1;

       *pszTemp != 0;

       pszTemp++,
       ulTemp++)
  {
    if (strchr (pszDelimiter,*pszTemp) )                /* found delimiter ? */
      pszLast = pszTemp+1;               /* store position of last delimiter */

                                               /* check if lWidth is reached */
    if (ulTemp >= lWidth)
    {
                                                       /* lWidth is reached. */
      if (pszLast == NULL)
        pszLast = pszTemp;                    /* ok, then cut current string */

                                  /* now add the string to the variable pool */
                                /* String in den RexxVariablenPool schreiben */
      iLen = pszLast - pszBegin;
      if (iLen > 0)                                 /* usable string is left */
      {
          rc = rexxStemAppend2(&ldp,                         /* append entry */
                               pszBegin,
                               iLen);
          if (rc != NO_ERROR)
            return (rc);                            /* raise error condition */
      }

                                    /* re-initialize for next string segment */
      pszBegin = pszLast;
      while (*pszBegin == ' ')                      /* ignore leading spaces */
        pszBegin++;

      ulTemp   = 1;
      pszTemp  = pszBegin;
      pszLast  = NULL;
    }
  }

                            /* now add remaining string to the variable pool */
                                /* String in den RexxVariablenPool schreiben */
  iLen = strlen(pszBegin);

  if (iLen > 0)                                       /* cut trailing spaces */
    while (pszBegin[iLen-1] == ' ')
      iLen--;

  if (iLen > 0)                     /* Wir waren nicht am Ende angelangt ... */
  {
    rc = rexxStemAppend2(&ldp,                               /* append entry */
                         pszBegin,
                         iLen);
    if (rc != NO_ERROR)
      return (rc);                                  /* raise error condition */
  }


  rc = rexxStemFinalize(&ldp);                 /* set stem.0 to lines read   */
  if (rc != NO_ERROR)                                        /* check errors */
    return (rc);                                    /* raise error condition */

  return VALID_ROUTINE;                                  /* no error on call */
}


/*************************************************************************
* Function:  RtQuerySysInfo                                              *
*                                                                        *
* Syntax:    call RtQuerySysInfo(stem,start,end)                         *
*                                                                        *
* Return:    Query a system value                                        *
*************************************************************************/

ULONG RtQuerySysInfo(PUCHAR    name,
                      ULONG    numargs,
                      RXSTRING args[],
                      PSZ      queuename,
                      RXSTRING *retstr)
{
  APIRET rc;                                               /* API returncode */
  PSZ    pszSysValStart;       /* first index and last index of system value */
  PSZ    pszSysValEnd;
  ULONG  ulSysValStart;
  ULONG  ulSysValEnd;
  CHAR   szBuffer[12];                  /* local buffer for the returnstring */
  ULONG  ulBufferItems;            /* number of items to be stored in buffer */
  ULONG  ulBufferLength;                          /* size of buffer in bytes */
  PULONG pulBuffer;                      /* pointer to array of ULONG values */
  RXSTEMDATA  ldp;                                              /* stem data */
  ULONG       len;                                       /* length of string */
  ULONG       ulCounter;               /* loop counter to iterate over items */

  /*********************************************************/
  /* Parameter conversion and checking section             */
  /*********************************************************/

  BUILDRXSTRING(retstr,
                NO_UTIL_ERROR);                          /* pass back result */
                                                       /* validate arguments */
  if (numargs != 3 ||
      !RXVALIDSTRING(args[0]) ||
      !RXVALIDSTRING(args[1]) ||
      !RXVALIDSTRING(args[2]))
    return INVALID_ROUTINE;                                /* raise an error */

  pszSysValStart = args[1].strptr;
  if ( string2long (pszSysValStart,
                    &ulSysValStart) == FALSE)             /* Conversion OK ? */
    return INVALID_ROUTINE;

  pszSysValEnd   = args[2].strptr;
  if ( string2long (pszSysValEnd,
                    &ulSysValEnd) == FALSE)               /* Conversion OK ? */
    return INVALID_ROUTINE;


  rc = rexxStemInit(&ldp,                             /* initialize our stem */
                    &args[0]);
  if (rc != NO_ERROR)                                    /* check for errors */
     return (rc);                                   /* raise error condition */


  /*********************************************************/
  /* OK, we got our parameter values checked and can go on.*/
  /*********************************************************/

  if (ulSysValStart > ulSysValEnd)                   /* swap the parameteres */
  {
    ULONG ulSwap;

    ulSwap = ulSysValEnd;
    ulSysValEnd = ulSysValStart;
    ulSysValStart = ulSwap;
  }

  ulBufferItems  = 1 + ulSysValEnd - ulSysValStart;   /* allocate the buffer */

         /* intercept nonsense values that would cause too much memory to be */
                                                               /* allocated. */

  if (ulBufferItems > 256)       /* this still leaves enough room for future */
                                       /* DosQuerySysInfo values within OS/2 */
  {
    BUILDRXSTRING(retstr,
                  ERROR_NOMEM);                    /* Fehler signalisieren   */
    return VALID_ROUTINE;                      /* finished                   */
  }

  ulBufferLength = sizeof(ULONG) * ulBufferItems;
  pulBuffer = (PULONG)malloc(ulBufferLength);
  if (pulBuffer == NULL)                          /* check proper allocation */
  {
    BUILDRXSTRING(retstr,
                  ERROR_NOMEM);                    /* Fehler signalisieren   */
    return VALID_ROUTINE;                      /* finished                   */
  }

  rc = DosQuerySysInfo(ulSysValStart,  /* query one particular system value */
                       ulSysValEnd,
                       pulBuffer,
                       ulBufferLength);
  if (rc != NO_ERROR)                             /* check proper allocation */
  {
    free(pulBuffer);                     /* free previously allocated memory */

    sprintf(szBuffer,
            "%d",
            rc);                            /* convert return code to string */

    BUILDRXSTRING(retstr,
                  szBuffer);                       /* Fehler signalisieren   */
    return VALID_ROUTINE;                      /* finished                   */
  }


  /*********************************************************/
  /* OK, now we've got our buffer filled with the info.    */
  /*********************************************************/

  for (ulCounter = 1;
       ulCounter <= ulBufferItems;
       ulCounter++)
  {
                                /* String in den RexxVariablenPool schreiben */
    sprintf(szBuffer,
            "%d",
            pulBuffer[ulCounter - 1]);            /* convert value to string */

    len = strlen(szBuffer);                         /* write value into stem */
    memcpy(ldp.ibuf,
           szBuffer,
           len);
    ldp.vlen = len;

    ldp.count++;                                    /* increase STEM-counter */
    sprintf(ldp.varname+ldp.stemlen,
            "%d",
            ldp.count);

    ldp.shvb.shvnext            = NULL;
    ldp.shvb.shvname.strptr     = ldp.varname;
    ldp.shvb.shvname.strlength  = strlen(ldp.varname);
    ldp.shvb.shvnamelen         = ldp.shvb.shvname.strlength;
    ldp.shvb.shvvalue.strptr    = ldp.ibuf;
    ldp.shvb.shvvalue.strlength = ldp.vlen;
    ldp.shvb.shvvaluelen        = ldp.vlen;
    ldp.shvb.shvcode            = RXSHV_SET;
    ldp.shvb.shvret             = 0;
    if (RexxVariablePool(&ldp.shvb) == RXSHV_BADN)             /* problems ? */
    {
      free (pulBuffer);
      return INVALID_ROUTINE;                           /* error on non-zero */
    }
  }

  free (pulBuffer);                                  /* free the buffer here */
                                            /* set stem.0 to number of items */
  sprintf(ldp.ibuf,
          "%d",
          ldp.count);
  ldp.varname[ldp.stemlen]    = '0';
  ldp.varname[ldp.stemlen+1]  = 0;
  ldp.shvb.shvnext            = NULL;
  ldp.shvb.shvname.strptr     = ldp.varname;
  ldp.shvb.shvname.strlength  = ldp.stemlen+1;
  ldp.shvb.shvnamelen         = ldp.stemlen+1;
  ldp.shvb.shvvalue.strptr    = ldp.ibuf;
  ldp.shvb.shvvalue.strlength = strlen(ldp.ibuf);
  ldp.shvb.shvvaluelen        = ldp.shvb.shvvalue.strlength;
  ldp.shvb.shvcode            = RXSHV_SET;
  ldp.shvb.shvret             = 0;
  if (RexxVariablePool(&ldp.shvb) == RXSHV_BADN)
    return INVALID_ROUTINE;            /* error on non-zero          */


  /*********************************************************/
  /* Das wars ...                                          */
  /*********************************************************/

  strcpy(retstr->strptr,
         NO_UTIL_ERROR);
  retstr->strlength = strlen(retstr->strptr);
  return VALID_ROUTINE;
}


/********************************************************************
* Function:  string2long(string, number)                            *
*                                                                   *
* Purpose:   Validates and converts an ASCII-Z string from string   *
*            form to an unsigned long.  Returns FALSE if the number *
*            is not valid, TRUE if the number was successfully      *
*            converted.                                             *
*                                                                   *
* RC:        TRUE - Good number converted                           *
*            FALSE - Invalid number supplied.                       *
*********************************************************************/

BOOL string2long(PSZ string, LONG *number)
{
  ULONG    accumulator;                /* converted number           */
  INT      length;                     /* length of number           */
  INT      sign;                       /* sign of number             */

  sign = 1;                            /* set default sign           */
  if (*string == '-') {                /* negative?                  */
    sign = -1;                         /* change sign                */
    string++;                          /* step past sign             */
  }

  length = strlen(string);             /* get length of string       */
  if (length == 0 ||                   /* if null string             */
      length > MAX_DIGITS)             /* or too long                */
    return FALSE;                      /* not valid                  */

  accumulator = 0;                     /* start with zero            */

  while (length) {                     /* while more digits          */
    if (!isdigit(*string))             /* not a digit?               */
      return FALSE;                    /* tell caller                */
                                       /* add to accumulator         */
    accumulator = accumulator *10 + (*string - '0');
    length--;                          /* reduce length              */
    string++;                          /* step pointer               */
  }
  *number = accumulator * sign;        /* return the value           */
  return TRUE;                         /* good number                */
}


/********************************************************************
* Function:  StrReplace (pszSource, pszOriginal, pszReplacement)    *
*                                                                   *
* Purpose:   Replaces specified tokens in a string.                 *
*********************************************************************/

#pragma PACK(1)
void StrReplace (PSZ   pszSource,
                 PSZ   pszOriginal,
                 PSZ   pszReplacement,
                 ULONG ulSize)
{
   ULONG ulLenOriginal;
   ULONG ulLenReplacement;
   PSZ   pszTemp;

   ulLenOriginal    = strlen(pszOriginal);
   ulLenReplacement = strlen(pszReplacement);

   pszTemp = pszSource;                                  /* Initialisierung */
                                       /* Bis alle Tokens abgearbeitet sind */
   while ( ( (pszTemp = strstr(pszTemp,pszOriginal)) != NULL) &&
             (pszTemp < pszSource + ulSize
                        - ulLenReplacement
                        - 4                                   /* Sicherheit */
                        + ulLenOriginal) )               /* Overflow-Schutz */
   {
                   /* pszTemp zeigt jetzt auf den gefundenen Originalstring */
      memmove (pszTemp+ulLenReplacement,
               pszTemp+ulLenOriginal,
               strlen(pszTemp + ulLenOriginal)+1);      /* Platz schaffen ! */

      memcpy  (pszTemp,                        /* Neuen Text hineinkopieren */
               pszReplacement,
               ulLenReplacement);
      pszTemp+=1;
   }

   *(pszSource+ulSize-1)=0; /* Sicherheitsterminierung */
}


/******************************************************************************
 * Name      : ULONG RtFileCopy
 * Funktion  : Copy a file to another location using DosCopy
 * Parameter : PUCHAR   name
 *             ULONG    numargs   - number of arguments specified
 *             RXSTRING args[]    - array of arguments passed to this function
 *             PSZ      queuename - name of the queue
 *             RXSTRING *retstr   - string where to return the result
 * Syntax    : rc = RtFileCopy(source filename, destination filename, options)
 *             options - "FAILEA" fail operation if target IFS doesn't support EAs.
 *                       "APPEND" append to existing file
 *                       "EXIST"  overwrite existing file
 * Variablen :
 * Ergebnis  : REXX-Interpreter Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [1997/11/09]
 ******************************************************************************/

ULONG RtFileCopy(PUCHAR   name,
                 ULONG    numargs,
                 RXSTRING args[],
                 PSZ      queuename,
                 RXSTRING *retstr)
{
  APIRET rc;                                               /* API returncode */
  PSZ    pszFileSource;                                  /* source file name */
  PSZ    pszFileDestination;                             /* dest.  file name */
  PSZ    pszOptions;                                     /* options          */
  ULONG  ulOptions;                      /* the bit-mask passed to DosCopy() */
  CHAR   szBuffer[12];                  /* local buffer for the returnstring */


  /*********************************************************/
  /* Parameter conversion and checking section             */
  /*********************************************************/

  BUILDRXSTRING(retstr,
                NO_UTIL_ERROR);                          /* pass back result */
                                                       /* validate arguments */
  if (numargs < 2 ||
      !RXVALIDSTRING(args[0]) ||
      !RXVALIDSTRING(args[1]))
    return INVALID_ROUTINE;                                /* raise an error */

  pszFileSource      = args[0].strptr;
  pszFileDestination = args[1].strptr;

  if (numargs == 3)
    if(!RXVALIDSTRING(args[2]))
      return INVALID_ROUTINE;                              /* raise an error */
    else
      pszOptions = args[2].strptr;              /* map the options parameter */
  else
    pszOptions="";                          /* these are the default options */


                                       /* now we map pszOptions to ulOptions */
  ulOptions = 0;
  strupr(pszOptions);                               /* capitalize the string */
  if (strstr(pszOptions,"FAILEA")) ulOptions |= DCPY_FAILEAS;
  if (strstr(pszOptions,"APPEND")) ulOptions |= DCPY_APPEND;
  if (strstr(pszOptions,"EXIST"))  ulOptions |= DCPY_EXISTING;


  /*********************************************************/
  /* OK, we got our parameter values checked and can go on.*/
  /*********************************************************/

  rc = DosCopy(pszFileSource,                       /* finally copy the file */
               pszFileDestination,
               ulOptions);
  sprintf(szBuffer,
          "%d",
          rc);                              /* convert return code to string */

  BUILDRXSTRING(retstr,
                szBuffer);                         /* Fehler signalisieren   */
  return VALID_ROUTINE;                        /* finished                   */
}


/******************************************************************************
 * Name      : ULONG RtFileMove
 * Funktion  : Move a file to another location using DosMove
 * Parameter : PUCHAR   name
 *             ULONG    numargs   - number of arguments specified
 *             RXSTRING args[]    - array of arguments passed to this function
 *             PSZ      queuename - name of the queue
 *             RXSTRING *retstr   - string where to return the result
 * Syntax    : rc = RtFileMove(source filename, destination filename)
 * Variablen :
 * Ergebnis  : REXX-Interpreter Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [1997/11/09]
 ******************************************************************************/

ULONG RtFileMove(PUCHAR   name,
                 ULONG    numargs,
                 RXSTRING args[],
                 PSZ      queuename,
                 RXSTRING *retstr)
{
  APIRET rc;                                               /* API returncode */
  PSZ    pszFileSource;                                  /* source file name */
  PSZ    pszFileDestination;                             /* dest.  file name */
  CHAR   szBuffer[12];                  /* local buffer for the returnstring */


  /*********************************************************/
  /* Parameter conversion and checking section             */
  /*********************************************************/

  BUILDRXSTRING(retstr,
                NO_UTIL_ERROR);                          /* pass back result */
                                                       /* validate arguments */
  if (numargs != 2 ||
      !RXVALIDSTRING(args[0]) ||
      !RXVALIDSTRING(args[1]))
    return INVALID_ROUTINE;                                /* raise an error */

  pszFileSource      = args[0].strptr;
  pszFileDestination = args[1].strptr;


  /*********************************************************/
  /* OK, we got our parameter values checked and can go on.*/
  /*********************************************************/

  rc = DosMove(pszFileSource,                       /* finally move the file */
               pszFileDestination);
  sprintf(szBuffer,
          "%d",
          rc);                              /* convert return code to string */

  BUILDRXSTRING(retstr,
                szBuffer);                         /* Fehler signalisieren   */
  return VALID_ROUTINE;                        /* finished                   */
}


/******************************************************************************
 * Name      : ULONG RtProcessKill
 * Funktion  : Asynchronously terminate a running process
 * Parameter : PUCHAR   name
 *             ULONG    numargs   - number of arguments specified
 *             RXSTRING args[]    - array of arguments passed to this function
 *             PSZ      queuename - name of the queue
 *             RXSTRING *retstr   - string where to return the result
 * Syntax    : rc = RtProcessKill(PID, action)
 *             action - "TREE" - kill all processes in dependancy tree
 * Variablen :
 * Ergebnis  : REXX-Interpreter Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [1997/11/09]
 ******************************************************************************/

ULONG RtProcessKill(PUCHAR   name,
                    ULONG    numargs,
                    RXSTRING args[],
                    PSZ      queuename,
                    RXSTRING *retstr)
{
  APIRET rc;                                               /* API returncode */
  PSZ    pszPID;                                       /* process identifier */
  ULONG  ulPID;                                        /* process identifier */
  PSZ    pszAction;                                    /* kill options       */
  ULONG  ulAction;                                     /* kill options       */
  CHAR   szBuffer[12];                  /* local buffer for the returnstring */


  /*********************************************************/
  /* Parameter conversion and checking section             */
  /*********************************************************/

  BUILDRXSTRING(retstr,
                NO_UTIL_ERROR);                          /* pass back result */
                                                       /* validate arguments */
  if (numargs < 1 ||
      !RXVALIDSTRING(args[0]))
    return INVALID_ROUTINE;                                /* raise an error */

  pszPID = args[0].strptr;
  if ( string2long (pszPID,&ulPID) == FALSE)              /* Conversion OK ? */
    return INVALID_ROUTINE;

  if (numargs == 2)
    if(!RXVALIDSTRING(args[1]))
      return INVALID_ROUTINE;                              /* raise an error */
    else
      pszAction = args[1].strptr;                         /* get this option */
  else
    pszAction = "";                                        /* default action */


  strupr(pszAction);                                /* capitalize the string */
  if (strstr(pszAction,"TREE"))
    ulAction = DKP_PROCESSTREE;     /* kill all processes in dependency tree */
  else
    ulAction = DKP_PROCESS;                        /* kill this process only */


  /*********************************************************/
  /* OK, we got our parameter values checked and can go on.*/
  /*********************************************************/

  rc = DosKillProcess(ulAction,
                      (PID)ulPID);                      /* kill that process */

  sprintf(szBuffer,
          "%d",
          rc);                              /* convert return code to string */

  BUILDRXSTRING(retstr,
                szBuffer);                         /* Fehler signalisieren   */
  return VALID_ROUTINE;                        /* finished                   */
}


/******************************************************************************
 * Name      : ULONG RtSetPriority
 * Funktion  : Set the processes priority (here the parent CMD.EXE)
 * Parameter : PUCHAR   name
 *             ULONG    numargs   - number of arguments specified
 *             RXSTRING args[]    - array of arguments passed to this function
 *             PSZ      queuename - name of the queue
 *             RXSTRING *retstr   - string where to return the result
 * Syntax    : rc = RtSetPriority(class, delta)
 *             class - "IDLE"
 *                   - "REGULAR"
 *                   - "FOREGROUND"
 *                   - "TIMECRITICAL"
 *                   - "NOCHANGE"
 *             delta - -31 .. +31
 * Variablen :
 * Ergebnis  : REXX-Interpreter Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [1997/11/09]
 ******************************************************************************/

ULONG RtSetPriority(PUCHAR   name,
                    ULONG    numargs,
                    RXSTRING args[],
                    PSZ      queuename,
                    RXSTRING *retstr)
{
  APIRET rc;                                               /* API returncode */
  PSZ    pszClass;                                         /* priority class */
  ULONG  ulClass;
  PSZ    pszDelta;                                         /* priority delta */
  LONG   lDelta;
  CHAR   szBuffer[12];                  /* local buffer for the returnstring */


  /*********************************************************/
  /* Parameter conversion and checking section             */
  /*********************************************************/

  BUILDRXSTRING(retstr,
                NO_UTIL_ERROR);                          /* pass back result */
                                                       /* validate arguments */
  if (numargs != 2 ||
      !RXVALIDSTRING(args[0]) ||
      !RXVALIDSTRING(args[1]))
    return INVALID_ROUTINE;                                /* raise an error */

  pszClass = args[0].strptr;
  pszDelta = args[1].strptr;

  if ( string2long (pszDelta,
                    &lDelta) == FALSE)                    /* Conversion OK ? */
    return INVALID_ROUTINE;

  ulClass = PRTYC_NOCHANGE;                           /* this is the default */
  strupr(pszClass);                                /* capitalize this string */
  if (strstr(pszClass,"NOCHANGE"))
    ulClass = PRTYC_NOCHANGE;
  else
    if (strstr(pszClass,"IDLE"))
      ulClass = PRTYC_IDLETIME;
    else
      if (strstr(pszClass,"REGULAR"))
        ulClass = PRTYC_REGULAR;
      else
        if (strstr(pszClass,"FOREGROUND"))
          ulClass = PRTYC_FOREGROUNDSERVER;
        else
          if (strstr(pszClass,"TIMECRITICAL"))
            ulClass = PRTYC_TIMECRITICAL;
          else
            return INVALID_ROUTINE;                   /* no string did match */


  /*********************************************************/
  /* OK, we got our parameter values checked and can go on.*/
  /*********************************************************/

  rc = DosSetPriority(PRTYS_PROCESS,              /* the parent CMD.EXE only */
                      ulClass,
                      lDelta,
                      0);                        /* current thread / process */
  sprintf(szBuffer,
          "%d",
          rc);                              /* convert return code to string */

  BUILDRXSTRING(retstr,
                szBuffer);                         /* Fehler signalisieren   */
  return VALID_ROUTINE;                        /* finished                   */
}



/******************************************************************************
 * Name      : ULONG RtQueryPathInfo
 * Funktion  : Query Information about a given path / file
 * Parameter : PUCHAR   name
 *             ULONG    numargs   - number of arguments specified
 *             RXSTRING args[]    - array of arguments passed to this function
 *             PSZ      queuename - name of the queue
 *             RXSTRING *retstr   - string where to return the result
 * Syntax    : rc = RtQueryPathInfo(stem,pathname)
 * Variablen :
 * Ergebnis  : REXX-Interpreter Returncode
 *             stem.0 - number of entries
 *                  1 - argument pathname
 *                  2 - fully qualified pathname
 *                  3 - creation date + time
 *                  4 - lastaccess date + time
 *                  5 - lastwrite date + time
 *                  6 - file size in bytes
 *                  7 - allocated file size in bytes
 *                  8 - attributes
 *                  9 - size of EA list
 * Bemerkung :
 *
 * Autor     : Patrick Haller [1997/11/09]
 ******************************************************************************/

ULONG RtQueryPathInfo(PUCHAR   name,
                      ULONG    numargs,
                      RXSTRING args[],
                      PSZ      queuename,
                      RXSTRING *retstr)
{
  APIRET      rc;                                          /* API returncode */
  CHAR        szBuffer[32];             /* local buffer for the returnstring */
  PSZ         pszPath;                              /* the pathname to query */
  FILESTATUS4 fs4;                         /* structure for DosQueryPathInfo */
  CHAR        szFullName[MAXPATHLEN];                       /* full pathname */
  RXSTEMDATA  ldp;                                              /* stem data */

  /*********************************************************/
  /* Parameter conversion and checking section             */
  /*********************************************************/

  BUILDRXSTRING(retstr,
                NO_UTIL_ERROR);                          /* pass back result */
                                                       /* validate arguments */
  if (numargs != 2 ||
      !RXVALIDSTRING(args[0]) ||
      !RXVALIDSTRING(args[1]))
    return INVALID_ROUTINE;                                /* raise an error */

  pszPath = args[1].strptr;

  rc = rexxStemInit(&ldp,                             /* initialize our stem */
                    &args[0]);
  if (rc != NO_ERROR)                                    /* check for errors */
     return (rc);                                   /* raise error condition */

  /*********************************************************/
  /* OK, we got our parameter values checked and can go on.*/
  /*********************************************************/

  rc = DosQueryPathInfo(pszPath,                    /* query full name first */
                        FIL_QUERYFULLNAME,
                        szFullName,
                        sizeof(szFullName));
  if (rc != NO_ERROR)                                /* oops, what's wrong ? */
    goto _err_rtquerypathinfo;

  rc = DosQueryPathInfo(pszPath,                        /* query information */
                        FIL_QUERYEASIZE,
                        &fs4,
                        sizeof(fs4));
  if (rc != NO_ERROR)        /* if we get an error here, we try FIL_STANDARD */
  {
    fs4.cbList = 0;                        /* this field is not valid then ! */
    rc = DosQueryPathInfo(pszPath,
                          FIL_STANDARD,
                          &fs4,
                          sizeof(fs4));
    if (rc != NO_ERROR)                              /* oops, what's wrong ? */
      goto _err_rtquerypathinfo;
  }


  /*********************************************************/
  /* now we got our information and can map it to the stem.*/
  /*********************************************************/

  rc = rexxStemAppend(&ldp, pszPath);       /* put in the argument path name */
  if (rc != NO_ERROR)
     return (rc);                                   /* raise error condition */

  rc = rexxStemAppend(&ldp, szFullName);
  if (rc != NO_ERROR)
     return (rc);                                   /* raise error condition */

  sprintf(szBuffer,                                         /* creation date */
          "%04u/%02u/%02u %02u:%02u.%02u",
          fs4.fdateCreation.year + 1980,
          fs4.fdateCreation.month,
          fs4.fdateCreation.day,
          fs4.ftimeCreation.hours,
          fs4.ftimeCreation.minutes,
          fs4.ftimeCreation.twosecs);
  rc = rexxStemAppend(&ldp, szBuffer);
  if (rc != NO_ERROR)
     return (rc);                                   /* raise error condition */

  sprintf(szBuffer,                                       /* LastAccess date */
          "%04u/%02u/%02u %02u:%02u.%02u",
          fs4.fdateLastAccess.year + 1980,
          fs4.fdateLastAccess.month,
          fs4.fdateLastAccess.day,
          fs4.ftimeLastAccess.hours,
          fs4.ftimeLastAccess.minutes,
          fs4.ftimeLastAccess.twosecs);
  rc = rexxStemAppend(&ldp, szBuffer);
  if (rc != NO_ERROR)
     return (rc);                                   /* raise error condition */

  sprintf(szBuffer,                                        /* LastWrite date */
          "%04u/%02u/%02u %02u:%02u.%02u",
          fs4.fdateLastWrite.year + 1980,
          fs4.fdateLastWrite.month,
          fs4.fdateLastWrite.day,
          fs4.ftimeLastWrite.hours,
          fs4.ftimeLastWrite.minutes,
          fs4.ftimeLastWrite.twosecs);
  rc = rexxStemAppend(&ldp, szBuffer);
  if (rc != NO_ERROR)
     return (rc);                                   /* raise error condition */

  sprintf(szBuffer,
          "%u",
          fs4.cbFile);
  rc = rexxStemAppend(&ldp, szBuffer);
  if (rc != NO_ERROR)
     return (rc);                                   /* raise error condition */

  sprintf(szBuffer,
          "%u",
          fs4.cbFileAlloc);
  rc = rexxStemAppend(&ldp, szBuffer);
  if (rc != NO_ERROR)
     return (rc);                                   /* raise error condition */

  sprintf (szBuffer,
           "%c%c%c%c%c%c",
           (fs4.attrFile & FILE_ARCHIVED)  ? 'A' : '-',
           (fs4.attrFile & FILE_SYSTEM)    ? 'S' : '-',
           (fs4.attrFile & FILE_HIDDEN)    ? 'H' : '-',
           (fs4.attrFile & FILE_READONLY)  ? 'R' : '-',
           (fs4.attrFile & FILE_DIRECTORY) ? 'D' : '-',
           (fs4.cbFileAlloc < fs4.cbFile)  ? 'C' : '-');
  rc = rexxStemAppend(&ldp, szBuffer);
  if (rc != NO_ERROR)
     return (rc);                                   /* raise error condition */

  sprintf(szBuffer,
          "%u",
          fs4.cbList);
  rc = rexxStemAppend(&ldp, szBuffer);
  if (rc != NO_ERROR)
     return (rc);                                   /* raise error condition */

  rc = rexxStemFinalize(&ldp);                            /* close this stem */
  if (rc != NO_ERROR)
     return (rc);                                   /* raise error condition */


  /*********************************************************/
  /* error handler                                         */
  /*********************************************************/

_err_rtquerypathinfo:
  sprintf(szBuffer,
          "%d",
          rc);                              /* convert return code to string */

  BUILDRXSTRING(retstr,
                szBuffer);                         /* Fehler signalisieren   */
  return VALID_ROUTINE;                        /* finished                   */
}


/******************************************************************************
 * Name      : ULONG RtQueryCtryInfo
 * Funktion  : Query Information about a given country / codepage
 * Parameter : PUCHAR   name
 *             ULONG    numargs   - number of arguments specified
 *             RXSTRING args[]    - array of arguments passed to this function
 *             PSZ      queuename - name of the queue
 *             RXSTRING *retstr   - string where to return the result
 * Syntax    : rc = RtQueryCtryInfo(stem,<country,codepage>)
 * Variablen :
 * Ergebnis  : REXX-Interpreter Returncode
 *             stem.0 - number of entries
 *                  1 - country-code / codepage
 *                  2 - date format
 *                  3 - currency symbol
 *                  4 - thousands separator
 *                  5 - decimal separator
 *                  6 - date separator
 *                  7 - time separator
 *                  8 - currency format (bitmask)
 *                  9 - decimal place
 *                 10 - time format
 *                 11 - data list separator
 *                 12 - country-dependent information (?)
 * Bemerkung :
 *
 * Autor     : Patrick Haller [1997/11/09]
 ******************************************************************************/

ULONG RtQueryCtryInfo(PUCHAR   name,
                      ULONG    numargs,
                      RXSTRING args[],
                      PSZ      queuename,
                      RXSTRING *retstr)
{
  APIRET      rc;                                          /* API returncode */
  CHAR        szBuffer[32];             /* local buffer for the returnstring */
  RXSTEMDATA  ldp;                                              /* stem data */
  ULONG       ulCountry;                                     /* country code */
  ULONG       ulCodepage;                                        /* codepage */

  COUNTRYCODE CountryCode;                             /* our NLS structures */
  COUNTRYINFO CountryInfo;
  PSZ         pszTemp;                           /* temporary string pointer */
  ULONG       ulCountryDependent;                  /* country-dependent data */


  /*********************************************************/
  /* Parameter conversion and checking section             */
  /*********************************************************/

  BUILDRXSTRING(retstr,
                NO_UTIL_ERROR);                          /* pass back result */
                                                       /* validate arguments */
  if (numargs < 1 ||
      !RXVALIDSTRING(args[0]))
    return INVALID_ROUTINE;                                /* raise an error */

  rc = rexxStemInit(&ldp,                             /* initialize our stem */
                    &args[0]);
  if (rc != NO_ERROR)                                    /* check for errors */
     return (rc);                                   /* raise error condition */

  if (numargs == 2)                     /* convert the optional country info */
    if (!RXVALIDSTRING(args[1]))
       return INVALID_ROUTINE;
    else
    {
       if ( string2long(args[1].strptr,
                        &ulCountry) == FALSE)             /* Conversion OK ? */
         return INVALID_ROUTINE;
    }
  else
     ulCountry = 0;                                       /* default country */


  if (numargs == 3)                    /* convert the optional codepage info */
    if (!RXVALIDSTRING(args[2]))
       return INVALID_ROUTINE;
    else
    {
       if ( string2long(args[2].strptr,
                        &ulCodepage) == FALSE)           /* Conversion OK ? */
         return INVALID_ROUTINE;
    }
  else
     ulCodepage = 0;                                    /* default codepage */


  /*********************************************************/
  /* OK, we got our parameter values checked and can go on.*/
  /*********************************************************/

  CountryCode.country  = ulCountry;
  CountryCode.codepage = ulCodepage;

  rc = DosQueryCtryInfo(sizeof(CountryInfo),         /* query the NLS system */
                        &CountryCode,
                        &CountryInfo,
                        &ulCountryDependent);
  if (rc != NO_ERROR)                                /* oops, what's wrong ? */
    goto _err_rtqueryctryinfo;


  /*********************************************************/
  /* now we got our information and can map it to the stem.*/
  /*********************************************************/

  sprintf(szBuffer,                              /* countrycode and codepage */
          "%u/%u",
          CountryInfo.country,
          CountryInfo.codepage);
  rc = rexxStemAppend(&ldp, szBuffer);
  if (rc != NO_ERROR)
     return (rc);                                   /* raise error condition */

  switch (CountryInfo.fsDateFmt)
  {
    case 0:  pszTemp="MM/DD/YY"; break;
    case 1:  pszTemp="DD/MM/YY"; break;
    case 2:  pszTemp="YY/MM/DD"; break;
    default: pszTemp="<unknown OS/2 date format>"; break;
  }
  rc = rexxStemAppend(&ldp, pszTemp);
  if (rc != NO_ERROR)
     return (rc);                                   /* raise error condition */

  rc = rexxStemAppend(&ldp, CountryInfo.szCurrency);
  if (rc != NO_ERROR)
     return (rc);                                   /* raise error condition */

  rc = rexxStemAppend(&ldp, CountryInfo.szThousandsSeparator);
  if (rc != NO_ERROR)
     return (rc);                                   /* raise error condition */

  rc = rexxStemAppend(&ldp, CountryInfo.szDecimal);
  if (rc != NO_ERROR)
     return (rc);                                   /* raise error condition */

  rc = rexxStemAppend(&ldp, CountryInfo.szDateSeparator);
  if (rc != NO_ERROR)
     return (rc);                                   /* raise error condition */

  rc = rexxStemAppend(&ldp, CountryInfo.szTimeSeparator);
  if (rc != NO_ERROR)
     return (rc);                                   /* raise error condition */

  sprintf(szBuffer,
          "%d",
          CountryInfo.fsCurrencyFmt);
  rc = rexxStemAppend(&ldp, szBuffer);
  if (rc != NO_ERROR)
     return (rc);                                   /* raise error condition */

  sprintf(szBuffer,
          "%d",
          CountryInfo.cDecimalPlace);
  rc = rexxStemAppend(&ldp, szBuffer);
  if (rc != NO_ERROR)
     return (rc);                                   /* raise error condition */

  switch (CountryInfo.fsTimeFmt)
  {
    case 0:  pszTemp="12"; break;                      /* 12-hour with am/pm */
    case 1:  pszTemp="24"; break;                                 /* 24-hour */
    default: pszTemp="<unknown OS/2 time format>"; break;             /* ??? */
  }
  rc = rexxStemAppend(&ldp, pszTemp);
  if (rc != NO_ERROR)
     return (rc);                                   /* raise error condition */

  rc = rexxStemAppend(&ldp, CountryInfo.szDataSeparator);
  if (rc != NO_ERROR)
     return (rc);                                   /* raise error condition */

  sprintf(szBuffer,
          "%u",
          ulCountryDependent);
  rc = rexxStemAppend(&ldp, szBuffer);
  if (rc != NO_ERROR)
     return (rc);                                   /* raise error condition */


  rc = rexxStemFinalize(&ldp);                            /* close this stem */
  if (rc != NO_ERROR)
     return (rc);                                   /* raise error condition */


  /*********************************************************/
  /* error handler                                         */
  /*********************************************************/

_err_rtqueryctryinfo:
  sprintf(szBuffer,
          "%d",
          rc);                              /* convert return code to string */

  BUILDRXSTRING(retstr,
                szBuffer);                         /* Fehler signalisieren   */
  return VALID_ROUTINE;                        /* finished                   */
}


/******************************************************************************
 * Name      : ULONG RtFileChecksum
 * Funktion  : Query Information about a given country / codepage
 * Parameter : PUCHAR   name
 *             ULONG    numargs   - number of arguments specified
 *             RXSTRING args[]    - array of arguments passed to this function
 *             PSZ      queuename - name of the queue
 *             RXSTRING *retstr   - string where to return the result
 * Syntax    : rc = RtFileChecksum(filename, <algorithm (CRC32,MD5)>)
 * Variablen :
 * Ergebnis  : REXX-Interpreter Returncode
 *             String containing the calculated checksum or
 *             empty string upon error
 *
 * Autor     : Patrick Haller [2002/01/11]
 ******************************************************************************/

enum enumChecksumAlgorithm
{
  CHECKSUM_ALGORITHM_CRC32,
  CHECKSUM_ALGORITHM_MD5
};


static char arrHexDigit[] = {'0', '1', '2', '3',
  '4', '5', '6', '7',
  '8', '9', 'a', 'b',
  'c', 'd', 'e', 'f'};

APIRET workerFileChecksum( PSZ pszFilename,
                           int iAlgorithm,
                           PSZ szBuffer,
                           int iBufferLength)
{
  ULONG       ulAction;
  APIRET      rc;
  HFILE       hFile;
  FILESTATUS4 fs4;  
  ULONG       ulBufferSize;                      /* size of the read buffer */
  ULONG       ulCounter;                          /* temporary loop counter */
  PVOID       pBuffer;                      /* pointer to the target buffer */
  ULONG       ulBytesRead;                          /* number of read bytes */
  ULONG       ulDummy;                                       /* dummy ULONG */
  CHAR        szSize[16];                     /* buffer for the size string */

  PMD5_CTX    pctxMD5 = NULL;           /* MD5 checksum calculation context */
  ULONG       ulCRC32 = 0xFFFFFFFF;   /* CRC32 checksum calculation context */
  
  
  /* first, we terminate the result buffer */
  szBuffer[0] = 0;
  
  /* open the specified file */
  rc = DosOpen(pszFilename,
               &hFile,
               &ulAction,
               0L,
               0L,
               OPEN_ACTION_FAIL_IF_NEW |
               OPEN_ACTION_OPEN_IF_EXISTS,
               OPEN_SHARE_DENYNONE |
               OPEN_ACCESS_READONLY |
               OPEN_FLAGS_SEQUENTIAL,
               NULL);
  if (NO_ERROR != rc)
    return rc;
  
  /* determine file size */
  rc = DosQueryFileInfo(hFile,    
                        FIL_QUERYEASIZE,
                        &fs4,
                        sizeof(fs4));
  if (NO_ERROR != rc)
  {
    DosClose( hFile );
    return rc;
  }
                        

  /* setup checksum contexts if desired */
  if (CHECKSUM_ALGORITHM_MD5 == iAlgorithm)
  {
    pctxMD5 = (PMD5_CTX) malloc( sizeof (MD5_CTX) );

    rc = toolsMD5Initialize(pctxMD5);                  /* initialize context */
    if (rc != NO_ERROR)                                  /* check for errors */
    {
      DosClose( hFile );
      return rc;                                      /* bail out with error */
    }
  }


  ulBufferSize = 63 * 1024;                        /* default buffer is 63kb */

  if (fs4.cbFile == 0)                                   /* 0 byte size file */
  {
    DosClose( hFile );
    return NO_ERROR;                                                   /* OK */
  }

  if (fs4.cbFile < ulBufferSize)                       /* check for filesize */
    ulBufferSize = fs4.cbFile;              /* use smallest necessary buffer */



  rc = DosAllocMem (&pBuffer,               /* pointer to the base address */
                    ulBufferSize,                           /* buffer size */
                    OBJ_TILE   |
                    PAG_COMMIT |                             /* page flags */
                    PAG_READ   |
                    PAG_WRITE);
  if (rc != NO_ERROR)                                  /* check for errors */
  {
    DosClose( hFile );
    return (rc);
  }
  
  
  ulBytesRead  = 1;                                      /* initialization */
                                                  /* stop if done or error */
  while ( (rc == NO_ERROR) && /* as long as there has not been a problem */
          (ulBytesRead != 0) )
  {
    rc = DosRead (hFile,                                        /* read file */
                  pBuffer,
                  ulBufferSize,
                  &ulBytesRead);
    if (rc == NO_ERROR)
    {
    /* check if checksum calculation was desired */
    /* Note: we need at least one read byte ...  */
      if (ulBytesRead > 0)
      {
        if ( pctxMD5 != NULL )
        {
          rc = toolsMD5Update (pctxMD5,
                               pBuffer,
                               ulBytesRead);
          if (rc != NO_ERROR)
          {
            DosClose( hFile );
            return rc;
          }
        }
  
        if (CHECKSUM_ALGORITHM_CRC32 == iAlgorithm)
        {
          rc = toolsCrc32Buffer(pBuffer,
                                ulBytesRead,
                                &ulCRC32,
                                &ulCRC32);
          if (rc != NO_ERROR)
          {
            DosClose( hFile );
            return rc;
          }
        }
      }
    }
  }

  /* ok, close open resources */
  DosClose( hFile );
  DosFreeMem( pBuffer );
    

  /* format the result string */
  if (NULL != pctxMD5)
  {
    PSZ pszBuffer = szBuffer;
    
    rc = toolsMD5Finalize(pctxMD5);
    if (rc != NO_ERROR)
      return rc;

    for (ulCounter = 0;
         ulCounter < 16;                /* this is sizeof(MD5_CTX.digest); */
         ulCounter++)
    {
      /* still enough room for the next hex digit? */
      if (3 < iBufferLength)
      {
        UCHAR ucDigit = pctxMD5->digest[ulCounter];
        
        iBufferLength -= 3;
        *pszBuffer++ = arrHexDigit[ (ucDigit & 0xF0) >> 4 ];
        *pszBuffer++ = arrHexDigit[ (ucDigit & 0x0F)      ];
        
        if (ulCounter < 15)
          *pszBuffer++ = ':';
      }
    }
    
    /* properly terminate the digest string */
    *pszBuffer++ = 0;
    
    free( pctxMD5 );
  }
  else
    if (CHECKSUM_ALGORITHM_CRC32 == iAlgorithm)
    {
      ulCRC32 = toolsCrc32BufferFinalize(ulCRC32);
      
      if (5 < iBufferLength)
        sprintf(szBuffer,
                "%08x",
                ulCRC32);
    }

  return (rc);  
}


ULONG RtFileChecksum(PUCHAR   name,
                     ULONG    numargs,
                     RXSTRING args[],
                     PSZ      queuename,
                     RXSTRING *retstr)
{
  APIRET      rc;                                          /* API returncode */
  CHAR        szBuffer[128];            /* local buffer for the returnstring */
  PSZ         pszAlgorithm;               /* the selected checksum algorithm */
  int         iAlgorithm;


  /*********************************************************/
  /* Parameter conversion and checking section             */
  /*********************************************************/

  BUILDRXSTRING(retstr,
                NO_UTIL_ERROR);                          /* pass back result */
                                                       /* validate arguments */
  if (numargs < 2 ||
      !RXVALIDSTRING(args[0]) ||
      !RXVALIDSTRING(args[1]))
    return INVALID_ROUTINE;                                /* raise an error */
  
  
  /* verify input variables */
  pszAlgorithm = args[1].strptr;
  if (stricmp("CRC32", pszAlgorithm) == 0)
    iAlgorithm = CHECKSUM_ALGORITHM_CRC32;
  else
    if (stricmp("MD5", pszAlgorithm) == 0)
      iAlgorithm = CHECKSUM_ALGORITHM_MD5;
    else
    {
      /* this is an unknown algorithm */
      return INVALID_ROUTINE;
    }
  
  
  /* call workerFileChecksum */
  rc = workerFileChecksum( args[0].strptr,  /* pszFilename */
                           iAlgorithm,      /* selected Checksum algorithm */
                           szBuffer,
                           sizeof( szBuffer ));
  if (rc == NO_ERROR)
  {
    /* build result string */
    BUILDRXSTRING(retstr,
                  szBuffer);  
  }
  else
  {
    /* if an error occured during processing, we just return
     * an empty string.
     */
    BUILDRXSTRING(retstr,
                  "");
  }
  
  return VALID_ROUTINE;                        /* finished                   */
}



/* Crc - 32 BIT ANSI X3.66 CRC checksum files */


/**********************************************************************\
|* Demonstration program to compute the 32-bit CRC used as the frame  *|
|* check sequence in ADCCP (ANSI X3.66, also known as FIPS PUB 71     *|
|* and FED-STD-1003, the U.S. versions of CCITT's X.25 link-level     *|
|* protocol).  The 32-bit FCS was added via the Federal Register,     *|
|* 1 June 1982, p.23798.  I presume but don't know for certain that   *|
|* this polynomial is or will be included in CCITT V.41, which        *|
|* defines the 16-bit CRC (often called CRC-CCITT) polynomial.  FIPS  *|
|* PUB 78 says that the 32-bit FCS reduces otherwise undetected       *|
|* errors by a factor of 10^-5 over 16-bit FCS.                       *|
\**********************************************************************/

/* Copyright (C) 1986 Gary S. Brown.  You may use this program, or
   code or tables extracted from it, as desired without restriction.*/

/* First, the polynomial itself and its table of feedback terms.  The  */
/* polynomial is                                                       */
/* X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0 */
/* Note that we take it "backwards" and put the highest-order term in  */
/* the lowest-order bit.  The X^32 term is "implied"; the LSB is the   */
/* X^31 term, etc.  The X^0 term (usually shown as "+1") results in    */
/* the MSB being 1.                                                    */

/* Note that the usual hardware shift register implementation, which   */
/* is what we're using (we're merely optimizing it by doing eight-bit  */
/* chunks at a time) shifts bits into the lowest-order term.  In our   */
/* implementation, that means shifting towards the right.  Why do we   */
/* do it this way?  Because the calculated CRC must be transmitted in  */
/* order from highest-order term to lowest-order term.  UARTs transmit */
/* characters in order from LSB to MSB.  By storing the CRC this way,  */
/* we hand it to the UART in the order low-byte to high-byte; the UART */
/* sends each low-bit to hight-bit; and the result is transmission bit */
/* by bit from highest- to lowest-order term without requiring any bit */
/* shuffling on our part.  Reception works similarly.                  */

/* The feedback terms table consists of 256, 32-bit entries.  Notes:   */
/*                                                                     */
/*  1. The table can be generated at runtime if desired; code to do so */
/*     is shown later.  It might not be obvious, but the feedback      */
/*     terms simply represent the results of eight shift/xor opera-    */
/*     tions for all combinations of data and CRC register values.     */
/*                                                                     */
/*  2. The CRC accumulation logic is the same for all CRC polynomials, */
/*     be they sixteen or thirty-two bits wide.  You simply choose the */
/*     appropriate table.  Alternatively, because the table can be     */
/*     generated at runtime, you can start by generating the table for */
/*     the polynomial in question and use exactly the same "updcrc",   */
/*     if your application needn't simultaneously handle two CRC       */
/*     polynomials.  (Note, however, that XMODEM is strange.)          */
/*                                                                     */
/*  3. For 16-bit CRCs, the table entries need be only 16 bits wide;   */
/*     of course, 32-bit entries work OK if the high 16 bits are zero. */
/*                                                                     */
/*  4. The values must be right-shifted by eight bits by the "updcrc"  */
/*     logic; the shift must be unsigned (bring in zeroes).  On some   */
/*     hardware you could probably optimize the shift in assembler by  */
/*     using byte-swap instructions.                                   */

static ULONG crc_32_tab[] = { /* CRC polynomial 0xedb88320 */
0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};


/*****************************************************************************
 * Name      : APIRET toolsCrc32String
 * Funktion  : calculates CRC32 of a zero-terminated string
 * Parameter : PSZ    pszString
 *             PULONG pulCRC
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung : 
 *
 * Autor     : Patrick Haller [Monday, 1998/05/31]
 *****************************************************************************/

#define UPDC32(octet, crc) \
          (crc_32_tab[((crc) ^ (octet)) & 0xff] ^ ((crc) >> 8))

static APIRET toolsCrc32String(PSZ    pszText,
                               PULONG pulCRC)
{
  register ULONG ulCRC32;
  register PSZ   pszTemp;
  
  if ( (pszText  == NULL) ||                             /* check parameters */
       (*pszText == 0)    ||
       (pulCRC   == NULL) )
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */
  
  
  ulCRC32 = 0xFFFFFFFF;

  pszTemp = pszText;
  
  while (*pszTemp) 
  {
    ulCRC32 = UPDC32(*pszTemp, ulCRC32);
    ++pszTemp;
  }

  *pulCRC = ~ulCRC32;                                /* pass back the result */
  
  return NO_ERROR;                                                     /* OK */
}


/*****************************************************************************
 * Name      : APIRET toolsCrc32Buffer
 * Funktion  : calculates CRC32 of a buffer
 * Parameter : PSZ    pszBuffer
 *             ULONG  ulBufferLength
 *             PULONG pulCRC
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung : pulCRCCurrent should be the last retrieved CRC if
 *             subsequent buffer calculation is done. If passed NULL,
 *             the routine starts with a CRC initializer value of -1
 *             (0xFFFFFFFF). Otherwise, the variable pointed to by
 *             pulCRCCurrent is supposed to be initialized with -1.
 *
 * Autor     : Patrick Haller [Monday, 1998/05/31]
 *****************************************************************************/

static APIRET toolsCrc32Buffer(PSZ    pszText,
                               ULONG  ulBufferLength,
                               PULONG pulCRC,
                               PULONG pulCRCCurrent)
{
  register ULONG ulCRC32;
  register PSZ   pszTemp;
  
  if ( (pszText        == NULL) ||                       /* check parameters */
       (ulBufferLength == 0)    ||
       (pulCRC         == NULL) )
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */
  

                                   /* initialize or continue the calculation */
  if (NULL != pulCRCCurrent)
      ulCRC32 = *pulCRCCurrent;
  else
      ulCRC32 = 0xFFFFFFFF;

  pszTemp = pszText;
  
  while (ulBufferLength--)
  {
    ulCRC32 = UPDC32(*pszTemp, ulCRC32);
    ++pszTemp;
  }
  
  /* not we may not invert here because
   * a subsequent call could come in
   */
  *pulCRC = ulCRC32;                                 /* pass back the result */
  
  return NO_ERROR;                                                     /* OK */
}


static ULONG toolsCrc32BufferFinalize(ULONG ulCRC32)
{
  return ~ulCRC32;                                  /* pass back the result */
}


#undef UPDC32


/*
 ***********************************************************************
 ** md5.c -- the source code for MD5 routines                         **
 ** RSA Data Security, Inc. MD5 Message-Digest Algorithm              **
 ** Created: 2/17/90 RLR                                              **
 ** Revised: 1/91 SRD,AJ,BSK,JT Reference C ver., 7/10 constant corr. **
 ***********************************************************************
 */

/*
 ***********************************************************************
 ** Copyright (C) 1990, RSA Data Security, Inc. All rights reserved.  **
 **                                                                   **
 ** License to copy and use this software is granted provided that    **
 ** it is identified as the "RSA Data Security, Inc. MD5 Message-     **
 ** Digest Algorithm" in all material mentioning or referencing this  **
 ** software or this function.                                        **
 **                                                                   **
 ** License is also granted to make and use derivative works          **
 ** provided that such works are identified as "derived from the RSA  **
 ** Data Security, Inc. MD5 Message-Digest Algorithm" in all          **
 ** material mentioning or referencing the derived work.              **
 **                                                                   **
 ** RSA Data Security, Inc. makes no representations concerning       **
 ** either the merchantability of this software or the suitability    **
 ** of this software for any particular purpose.  It is provided "as  **
 ** is" without express or implied warranty of any kind.              **
 **                                                                   **
 ** These notices must be retained in any copies of any part of this  **
 ** documentation and/or software.                                    **
 ***********************************************************************
 */

/*
 ***********************************************************************
 **  Message-digest routines:                                         **
 **  To form the message digest for a message M                       **
 **    (1) Initialize a context buffer mdContext using MD5Init        **
 **    (2) Call MD5Update on mdContext and M                          **
 **    (3) Call MD5Final on mdContext                                 **
 **  The message digest is now in mdContext->digest[0...15]           **
 ***********************************************************************
 */

static unsigned char PADDING[64] = 
{
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* F, G, H and I are basic MD5 functions */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4 */
/* Rotation is separate from addition to prevent recomputation */
#define FF(a, b, c, d, x, s, ac) \
  {(a) += F ((b), (c), (d)) + (x) + (ULONG)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) \
  {(a) += G ((b), (c), (d)) + (x) + (ULONG)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) \
  {(a) += H ((b), (c), (d)) + (x) + (ULONG)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) \
  {(a) += I ((b), (c), (d)) + (x) + (ULONG)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }


/* Basic MD5 step. MD5Transforms buf based on in.
 */
static void MD5Transform (PULONG buf, 
                          PULONG in)
{
  ULONG a = buf[0], 
        b = buf[1], 
        c = buf[2], 
        d = buf[3];

  /* Round 1 */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
  FF ( a, b, c, d, in[ 0], S11, 3614090360ul); /* 1 */
  FF ( d, a, b, c, in[ 1], S12, 3905402710ul); /* 2 */
  FF ( c, d, a, b, in[ 2], S13,  606105819ul); /* 3 */
  FF ( b, c, d, a, in[ 3], S14, 3250441966ul); /* 4 */
  FF ( a, b, c, d, in[ 4], S11, 4118548399ul); /* 5 */
  FF ( d, a, b, c, in[ 5], S12, 1200080426ul); /* 6 */
  FF ( c, d, a, b, in[ 6], S13, 2821735955ul); /* 7 */
  FF ( b, c, d, a, in[ 7], S14, 4249261313ul); /* 8 */
  FF ( a, b, c, d, in[ 8], S11, 1770035416ul); /* 9 */
  FF ( d, a, b, c, in[ 9], S12, 2336552879ul); /* 10 */
  FF ( c, d, a, b, in[10], S13, 4294925233ul); /* 11 */
  FF ( b, c, d, a, in[11], S14, 2304563134ul); /* 12 */
  FF ( a, b, c, d, in[12], S11, 1804603682ul); /* 13 */
  FF ( d, a, b, c, in[13], S12, 4254626195ul); /* 14 */
  FF ( c, d, a, b, in[14], S13, 2792965006ul); /* 15 */
  FF ( b, c, d, a, in[15], S14, 1236535329ul); /* 16 */

  /* Round 2 */
#define S21 5
#define S22 9
#define S23 14
#define S24 20
  GG ( a, b, c, d, in[ 1], S21, 4129170786ul); /* 17 */
  GG ( d, a, b, c, in[ 6], S22, 3225465664ul); /* 18 */
  GG ( c, d, a, b, in[11], S23,  643717713ul); /* 19 */
  GG ( b, c, d, a, in[ 0], S24, 3921069994ul); /* 20 */
  GG ( a, b, c, d, in[ 5], S21, 3593408605ul); /* 21 */
  GG ( d, a, b, c, in[10], S22,   38016083ul); /* 22 */
  GG ( c, d, a, b, in[15], S23, 3634488961ul); /* 23 */
  GG ( b, c, d, a, in[ 4], S24, 3889429448ul); /* 24 */
  GG ( a, b, c, d, in[ 9], S21,  568446438ul); /* 25 */
  GG ( d, a, b, c, in[14], S22, 3275163606ul); /* 26 */
  GG ( c, d, a, b, in[ 3], S23, 4107603335ul); /* 27 */
  GG ( b, c, d, a, in[ 8], S24, 1163531501ul); /* 28 */
  GG ( a, b, c, d, in[13], S21, 2850285829ul); /* 29 */
  GG ( d, a, b, c, in[ 2], S22, 4243563512ul); /* 30 */
  GG ( c, d, a, b, in[ 7], S23, 1735328473ul); /* 31 */
  GG ( b, c, d, a, in[12], S24, 2368359562ul); /* 32 */

  /* Round 3 */
#define S31 4
#define S32 11
#define S33 16
#define S34 23
  HH ( a, b, c, d, in[ 5], S31, 4294588738ul); /* 33 */
  HH ( d, a, b, c, in[ 8], S32, 2272392833ul); /* 34 */
  HH ( c, d, a, b, in[11], S33, 1839030562ul); /* 35 */
  HH ( b, c, d, a, in[14], S34, 4259657740ul); /* 36 */
  HH ( a, b, c, d, in[ 1], S31, 2763975236ul); /* 37 */
  HH ( d, a, b, c, in[ 4], S32, 1272893353ul); /* 38 */
  HH ( c, d, a, b, in[ 7], S33, 4139469664ul); /* 39 */
  HH ( b, c, d, a, in[10], S34, 3200236656ul); /* 40 */
  HH ( a, b, c, d, in[13], S31,  681279174ul); /* 41 */
  HH ( d, a, b, c, in[ 0], S32, 3936430074ul); /* 42 */
  HH ( c, d, a, b, in[ 3], S33, 3572445317ul); /* 43 */
  HH ( b, c, d, a, in[ 6], S34,   76029189ul); /* 44 */
  HH ( a, b, c, d, in[ 9], S31, 3654602809ul); /* 45 */
  HH ( d, a, b, c, in[12], S32, 3873151461ul); /* 46 */
  HH ( c, d, a, b, in[15], S33,  530742520ul); /* 47 */
  HH ( b, c, d, a, in[ 2], S34, 3299628645ul); /* 48 */

  /* Round 4 */
#define S41 6
#define S42 10
#define S43 15
#define S44 21
  II ( a, b, c, d, in[ 0], S41, 4096336452ul); /* 49 */
  II ( d, a, b, c, in[ 7], S42, 1126891415ul); /* 50 */
  II ( c, d, a, b, in[14], S43, 2878612391ul); /* 51 */
  II ( b, c, d, a, in[ 5], S44, 4237533241ul); /* 52 */
  II ( a, b, c, d, in[12], S41, 1700485571ul); /* 53 */
  II ( d, a, b, c, in[ 3], S42, 2399980690ul); /* 54 */
  II ( c, d, a, b, in[10], S43, 4293915773ul); /* 55 */
  II ( b, c, d, a, in[ 1], S44, 2240044497ul); /* 56 */
  II ( a, b, c, d, in[ 8], S41, 1873313359ul); /* 57 */
  II ( d, a, b, c, in[15], S42, 4264355552ul); /* 58 */
  II ( c, d, a, b, in[ 6], S43, 2734768916ul); /* 59 */
  II ( b, c, d, a, in[13], S44, 1309151649ul); /* 60 */
  II ( a, b, c, d, in[ 4], S41, 4149444226ul); /* 61 */
  II ( d, a, b, c, in[11], S42, 3174756917ul); /* 62 */
  II ( c, d, a, b, in[ 2], S43,  718787259ul); /* 63 */
  II ( b, c, d, a, in[ 9], S44, 3951481745ul); /* 64 */

  buf[0] += a;
  buf[1] += b;
  buf[2] += c;
  buf[3] += d;
}


/*****************************************************************************
 * Name      : APIRET toolsMD5Initialize
 * Funktion  : The routine initializes the MD5 (message-digest) context
 * Parameter : PMD5_CTX pmd5_ctx
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung : BROKEN
 *
 * Autor     : Patrick Haller [Monday, 1998/05/31]
 *****************************************************************************/

static APIRET toolsMD5Initialize (PMD5_CTX pmdContext)
{
  if (pmdContext == NULL)                                /* check parameters */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */
  
  pmdContext->i[0] = pmdContext->i[1] = 0;

                                     /* Load magic initialization constants. */
  pmdContext->buf[0] = (ULONG)0x67452301;
  pmdContext->buf[1] = (ULONG)0xefcdab89;
  pmdContext->buf[2] = (ULONG)0x98badcfe;
  pmdContext->buf[3] = (ULONG)0x10325476;
  
  return (NO_ERROR);                                                   /* OK */
}


/*****************************************************************************
 * Name      : APIRET toolsMD5Update
 * Funktion  : The routine MD5Update updates the message-digest context to
 *             account for the presence of each of the characters inBuf[0..inLen-1]
 *             in the message whose digest is being computed.
 * Parameter : PMD5_CTX pmd5_ctx
 *             PSZ      inBuffer
 *             ULONG    inBufferLength
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung : BROKEN
 *
 * Autor     : Patrick Haller [Monday, 1998/05/31]
 *****************************************************************************/

static APIRET toolsMD5Update (PMD5_CTX pmdContext,
                              PSZ      pszInBuffer,
                              ULONG    ulInBufferLength)
{
  ULONG in[16];
  int   mdi;
  UINT  i, ii;
  
  if ( (pmdContext       == NULL) ||                     /* check parameters */
       (pszInBuffer      == NULL) ||
       (ulInBufferLength == 0) )
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */
  
  
                                           /* compute number of bytes mod 64 */
  mdi = (int)((pmdContext->i[0] >> 3) & 0x3F);

                                                    /* update number of bits */
  if ((pmdContext->i[0] + (ulInBufferLength << 3)) < pmdContext->i[0])
    pmdContext->i[1]++;
  
  pmdContext->i[0] += (ulInBufferLength << 3);
  pmdContext->i[1] += (ulInBufferLength >> 29);

  while (ulInBufferLength--) 
  {
                               /* add new character to buffer, increment mdi */
    pmdContext->in[mdi++] = *pszInBuffer++;

                                                   /* transform if necessary */
    if (mdi == 0x40) 
    {
      for (i = 0, 
           ii = 0; 
           
           i < 16; 
           
           i++, 
           ii += 4)
        in[i] = ((pmdContext->in[ii+3]) << 24) |
                ((pmdContext->in[ii+2]) << 16) |
                ((pmdContext->in[ii+1]) << 8) |
                (pmdContext->in[ii]);
      MD5Transform (pmdContext->buf, 
                    in);
      mdi = 0;
    }
  }
  
  return (NO_ERROR);                                                   /* OK */
}


/*****************************************************************************
 * Name      : APIRET toolsMD5Finalize
 * Funktion  : The routine MD5Finalize terminates the message-digest computation and
 *             ends with the desired message digest in pmdContext->digest[0...15].
 * Parameter : PMD5_CTX pmd5_ctx
 *             PSZ      inBuffer
 *             ULONG    inBufferLength
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung : BROKEN
 *
 * Autor     : Patrick Haller [Monday, 1998/05/31]
 *****************************************************************************/

static APIRET toolsMD5Finalize (PMD5_CTX pmdContext)
{
  ULONG        in[16];
  int          mdi;
  unsigned int i, ii;
  unsigned int padLen;
  
  if (pmdContext == NULL)                                /* check parameters */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */
  
  
  in[14] = pmdContext->i[0];                          /* save number of bits */
  in[15] = pmdContext->i[1];

                                           /* compute number of bytes mod 64 */
  mdi = (int)((pmdContext->i[0] >> 3) & 0x3F);

                                   
  padLen = (mdi < 56) ? (56 - mdi) : (120 - mdi);    /* pad out to 56 mod 64 */
  
  toolsMD5Update (pmdContext,
                  PADDING,
                  padLen);

                                      /* append length in bits and transform */
  for (i = 0, 
       ii = 0; 
       
       i < 14; 
       
       i++, 
       ii += 4)
    in[i] = ((pmdContext->in[ii+3]) << 24) |
            ((pmdContext->in[ii+2]) << 16) |
            ((pmdContext->in[ii+1]) << 8) |
            (pmdContext->in[ii]);
  
  MD5Transform (pmdContext->buf, 
                in);

                                                   /* store buffer in digest */
  for (i = 0, 
       ii = 0; 
       
       i < 4; 
       
       i++, 
       ii += 4) 
  {
    pmdContext->digest[ii]   = (pmdContext->buf[i] & 0xFF);
    pmdContext->digest[ii+1] = ((pmdContext->buf[i] >> 8) & 0xFF);
    pmdContext->digest[ii+2] = ((pmdContext->buf[i] >> 16) & 0xFF);
    pmdContext->digest[ii+3] = ((pmdContext->buf[i] >> 24) & 0xFF);
  }

  return (NO_ERROR);                                                   /* OK */
}


/******************************************************************************
 * Name      : ULONG RtQueryModule
 * Funktion  : Query information about loaded modules
 * Parameter : PUCHAR   name
 *             ULONG    numargs   - number of arguments specified
 *             RXSTRING args[]    - array of arguments passed to this function
 *             PSZ      queuename - name of the queue
 *             RXSTRING *retstr   - string where to return the result
 * Syntax    : rc = RtQueryModule(stem,module name)
 * Variablen :
 * Ergebnis  : REXX-Interpreter Returncode
 *             stem.0 - fully qualified name of the module found
 *                  :
 *                  :
 *                  n - loaded sub module
 *
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-01-11]
 ******************************************************************************/

extern APIRET APIENTRY DosQuerySysState (ULONG func,
                                         ULONG par1,
                                         ULONG pid,
                                         ULONG _reserved_,
                                         PVOID buf,
                                         ULONG bufsz);

static PQMODULE workerSubModuleQuery(PQTOPLEVEL top,
                                     HMODULE    hModHandle)
{
  PQMODULE m       = top->moddata;
  int      i;

  while (m)
  {
    if (m->hndmod == hModHandle)                   /* is this "our" module ? */
      return (m);                                   /* return this as result */

    m = m->next;                                   /* skip to the next entry */
  }

  return ((PQMODULE)NULL);                     /* return this as our default */
}


static APIRET workerDumpModule(PQTOPLEVEL top,
                               PSZ pszModuleName,
                               RXSTEMDATA* pldp)
{
  PQMODULE pqModule = top->moddata;
  PQMODULE pqModuleIter;
  int      i;
  PSZ      pszTemp;
  APIRET   rc;


  while (pqModule)
  {
    /* find rightmost slash to separate the process name */
    pszTemp = strrchr(pqModule->name, '\\');
    if (NULL == pszTemp)
      pszTemp = pqModule->name;
    else
      /* skip the backslash */
      pszTemp++;

   /* check if we've found the desired module */
    if (NULL != pszModuleName)
    {
      if (stricmp(pszModuleName, pszTemp) == 0 ||
          stricmp(pszModuleName, pqModule->name) == 0)
      {
        rc = rexxStemAppend(pldp, pqModule->name);
        if (rc != NO_ERROR)
          return (rc);                                /* raise error condition */

        for (i=0;
             i < pqModule->refcnt;
             i++)
        {
          pqModuleIter = workerSubModuleQuery(top,
                                              pqModule->modref[i]);
          if (pqModuleIter != NULL)
          {
            rc = rexxStemAppend(pldp, pqModuleIter->name);
            if (rc != NO_ERROR)
              return (rc);                                /* raise error condition */
          }
        }

        /* OK, abort after the first found module */
        return NO_ERROR;
      }
    }
    else
    {
      /* OK, only add current module and we're done */
      rc = rexxStemAppend(pldp, pqModule->name);
      if (rc != NO_ERROR)
        return (rc);                                /* raise error condition */
    }

    pqModule = pqModule->next;
  }

  return NO_ERROR;
}


APIRET workerQueryModule(PSZ pszModuleName,
                         RXSTEMDATA* pldp)
{
  APIRET     rc;                                           /* API returncode */
  PVOID      pBuffer;                      /* buffer for the API information */
  PQTOPLEVEL pqtTop;                            /* the top level information */

#define BUFSIZE 128000l
#define RESERVED 0

  pBuffer = malloc(BUFSIZE);                  /* allocate buffer for the API */
  if (pBuffer == NULL)                        /* check for proper allocation */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */

  memset(pBuffer,                               /* zero-out the whole buffer */
         0,
         BUFSIZE);

  rc = DosQuerySysState(0x3f,                        /* OK, now call the API */
                        RESERVED,
                        RESERVED,
                        RESERVED,
                        (PCHAR)pBuffer,
                        BUFSIZE);
  if (rc == NO_ERROR)                                     /* everything OK ? */
  {
    pqtTop = (PQTOPLEVEL)pBuffer;               /* assign the buffer pointer */

    rc = workerDumpModule (pqtTop,
                           pszModuleName,
                           pldp);
  }

  free (pBuffer);                                         /* free the buffer */

  return (rc);                                                         /* OK */
}


ULONG RtQueryModule(PUCHAR   name,
                    ULONG    numargs,
                    RXSTRING args[],
                    PSZ      queuename,
                    RXSTRING *retstr)
{
  APIRET      rc;                                          /* API returncode */
  APIRET      rc2;                                         /* API returncode */
  CHAR        szBuffer[32];             /* local buffer for the returnstring */
  RXSTEMDATA  ldp;                                              /* stem data */
  PSZ         pszModuleName;             /* name of the module to search for */
  PSZ         pszTemp;                           /* temporary string pointer */


  /*********************************************************/
  /* Parameter conversion and checking section             */
  /*********************************************************/

  BUILDRXSTRING(retstr,
                NO_UTIL_ERROR);                          /* pass back result */
                                                       /* validate arguments */
  if (numargs < 1 ||
      !RXVALIDSTRING(args[0]))
    return INVALID_ROUTINE;                                /* raise an error */
  
  if (numargs == 2 &&
      !RXVALIDSTRING(args[1]))
    return INVALID_ROUTINE;                                /* raise an error */
  
  if (numargs > 2)
    return INVALID_ROUTINE;                                /* raise an error */
  
  rc = rexxStemInit(&ldp,                             /* initialize our stem */
                    &args[0]);
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                                   /* raise error condition */
  
  if (numargs == 2)
    pszModuleName = args[1].strptr;
  else
    pszModuleName = NULL;


  /*********************************************************/
  /* OK, we got our parameter values checked and can go on.*/
  /*********************************************************/
  
  rc = workerQueryModule(pszModuleName,
                         &ldp);

  /*********************************************************/
  /* now we got our information and can map it to the stem.*/
  /*********************************************************/

  rc2 = rexxStemFinalize(&ldp);                           /* close this stem */
  if (rc2 != NO_ERROR)
     return (rc2);                                  /* raise error condition */


  /*********************************************************/
  /* error handler                                         */
  /*********************************************************/

_err_rtqueryctryinfo:
  sprintf(szBuffer,
          "%d",
          rc);                              /* convert return code to string */

  BUILDRXSTRING(retstr,
                szBuffer);                         /* Fehler signalisieren   */
  return VALID_ROUTINE;                        /* finished                   */
}



/*
  DOS32REPLACEMODULE 417

      APIRET APIENTRY DosReplaceModule( PSZ pszOldModule,
                                  PSZ pszNewModule,
                                  PSZ pszBackupModule );

When a DLL or EXE file is in use by the system, the file is locked. It
can not therefore be replaced on the harddisk by a newer copy. The API
DosReplaceModule is to allow the replacement on the disk of the new
module whilst the system continues to run the old module. The contents
of the file pszOldModule are cached by the system, and the file is
closed. A backup copy of the file is created as pszBackupModule for
recovery purposes should the install program fail. The new module
pszModule takes the place of the original module on the disk.

Note - the system will continue to use the cached old module until all
references to it are released. The next reference to the module will
cause a reload from the new module on disk.

Note - only protect mode executable files can be replaced by this API.
This API can not be used for DOS/Windows programs, or for data files.

Note - pszNewModule and pszBackupModule may be NULL pointers.

Note - this API uses the swap file to cache the old module. This API
is expensive in terms of disk space usage.
*/


/******************************************************************************
 * Name      : ULONG RtReplaceModule
 * Funktion  : Query information about loaded modules
 * Parameter : PUCHAR   name
 *             ULONG    numargs   - number of arguments specified
 *             RXSTRING args[]    - array of arguments passed to this function
 *             PSZ      queuename - name of the queue
 *             RXSTRING *retstr   - string where to return the result
 * Syntax    : rc = RtQueryModule(module name)
 * Variablen :
 * Ergebnis  : REXX-Interpreter Returncode / OS/2 return code
 *             0 == NO_ERROR == success
 *
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-01-11]
 ******************************************************************************/

ULONG RtReplaceModule(PUCHAR   name,
                      ULONG    numargs,
                      RXSTRING args[],
                      PSZ      queuename,
                      RXSTRING *retstr)
{
  APIRET      rc;                                          /* API returncode */
  CHAR   szBuffer[12];                  /* local buffer for the returnstring */
  PSZ         pszOld    = NULL;
  PSZ         pszNew    = NULL;
  PSZ         pszBackup = NULL;
  

  /*********************************************************/
  /* Parameter conversion and checking section             */
  /*********************************************************/

  BUILDRXSTRING(retstr,
                NO_UTIL_ERROR);                          /* pass back result */
                                                       /* validate arguments */
  if (numargs < 1 ||
      !RXVALIDSTRING(args[0]))
    return INVALID_ROUTINE;                                /* raise an error */
  else
    pszOld = args[0].strptr;
  
  if (numargs >= 2 &&
      !RXVALIDSTRING(args[1]))
    return INVALID_ROUTINE;                                /* raise an error */
  else
    pszNew = args[1].strptr;
  
  if (numargs == 3 &&
      !RXVALIDSTRING(args[2]))
    return INVALID_ROUTINE;                                /* raise an error */
  else
    pszBackup = args[2].strptr;
  
  
  /* OK, call OS/2 */
  rc = DosReplaceModule(pszOld,
                        pszNew,
                        pszBackup );
  
  sprintf(szBuffer,
          "%d",
          rc);                              /* convert return code to string */

  BUILDRXSTRING(retstr,
                szBuffer);                         /* Fehler signalisieren   */
  return VALID_ROUTINE;                        /* finished                   */
}


/******************************************************************************
 * Name      : ULONG RtQueryProcess
 * Funktion  : Query information about loaded processes
 * Parameter : PUCHAR   name
 *             ULONG    numargs   - number of arguments specified
 *             RXSTRING args[]    - array of arguments passed to this function
 *             PSZ      queuename - name of the queue
 *             RXSTRING *retstr   - string where to return the result
 * Syntax    : rc = RtQueryProcess(stem, [process name or PID])
 * Variablen :
 * Ergebnis  : REXX-Interpreter Returncode
 *             stem.0 - number of entries
 *                  1 - loaded process names
 *                  :
 *                  :
 *                  n - 
 *
 *             If searching for a specific process (name / PID)
 *             stem.0 - number of entries
 *                  1 - process name
 *                  : - further process data
 *                  :
 *                  n - 
 *
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-01-11]
 ******************************************************************************/

static PSZ workerDumpProcessState(ULONG ulState)
{
  switch (ulState)
  {
    case 0x00000000: return ("NORMAL");
    case 0x00000001: return ("EXITLIST");
    case 0x00000010: return ("BACKGRND");
    default:         return ("<unknown>");
  }
}


static PSZ workerDumpProcessType(ULONG ulType)
{
  switch (ulType)
  {
    case 0x00000000: return ("Fullscreen");
    case 0x00000001: return ("VDM");
    case 0x00000002: return ("VIO Window");
    case 0x00000003: return ("PM Session");
    case 0x00000004: return ("Detached");
    default:         return ("<unknown>");
  }
}

static APIRET workerDumpProcess(PQTOPLEVEL top,
                                PSZ pszProcessName,
                                RXSTEMDATA *pldp)
{
  PQPROCESS p = top->procdata;
  PQMODULE  pqmodProcess;
  PQTHREAD  t;
  int       i;
  ULONG     ulPID = (ULONG)0;
  PSZ       pszTemp;
  APIRET    rc;
  char      szBuffer[48];
  
  static const 
    QMODULE   qmodDefault = {NULL,
                             0xFFFF,
                             0xFFFF,
                             0xFFFFFFFF,
                             0xFFFFFFFF,
                             NULL,
                             "<unknown>",
                             0};
  
  
  /* someone specified process name or pid ? */
  if (NULL != pszProcessName)
    ulPID = atol( pszProcessName );
  
  while (p && p->rectype == 1)
  {
    pqmodProcess = workerSubModuleQuery(top,
                                        p->hndmod);
    if (pqmodProcess == NULL)           /* module not found, for some reason */
      pqmodProcess = (PQMODULE)&qmodDefault;          /* then set to default */
    
    /* find rightmost slash to separate the process name */
    pszTemp = strrchr(pqmodProcess->name, '\\');
    if (NULL == pszTemp)
      pszTemp = pqmodProcess->name;
    else
      /* skip the backslash */
      pszTemp++;
    
    /* check if we're looking for a specific process */
    if (NULL != pszProcessName)
    {
      /* PID doesn't exist, so won't hit ever */
      if ( ulPID == p->pid ||
           stricmp( pszProcessName, pszTemp ) == 0 ||
           stricmp( pszProcessName, pqmodProcess->name ) == 0 )
      {
        /* provide detailed information about this process */
        rc = rexxStemAppend(pldp, pqmodProcess->name);
        if (rc != NO_ERROR)
          return (rc);
        
#define RXSTEMADD(a,b) { \
        sprintf(szBuffer, a, b); \
        rc = rexxStemAppend(pldp, szBuffer); \
        if (rc != NO_ERROR) \
          return (rc); \
        }
        
        RXSTEMADD("%d", p->pid);
        RXSTEMADD("%d", p->ppid);
        RXSTEMADD("%d", p->sessid);
        
        rc = rexxStemAppend(pldp, workerDumpProcessType(p->type) );
        if (rc != NO_ERROR)
          return (rc);
        RXSTEMADD("%d", p->type);
        
        rc = rexxStemAppend(pldp, workerDumpProcessState(p->state) );
        if (rc != NO_ERROR)
          return (rc);
        RXSTEMADD("%d", p->state);
        
        RXSTEMADD("%d", p->hndmod);
        RXSTEMADD("%d", p->threadcnt);
        RXSTEMADD("%d", p->sem16cnt);
        RXSTEMADD("%d", p->privsem32cnt);
        RXSTEMADD("%d", p->dllcnt);
        RXSTEMADD("%d", p->shrmemcnt);
        RXSTEMADD("%d", p->fdscnt);
        
        return NO_ERROR;
      }
    }
    else
    {
      /* provide detailed information about this process */
      RXSTEMADD("%d", p->pid);
    }
    
    /* skip to next process */
    t = p->threads;

    for (i=0;                                         /* initialize the loop */
         i < p->threadcnt;
         i++,
         t++)
    { }
    p = (PQPROCESS)t;
  }

  return NO_ERROR;
}

#undef RXSTEMADD


APIRET workerQueryProcess(PSZ pszProcessName,
                          RXSTEMDATA* pldp)
{
  APIRET     rc;                                           /* API returncode */
  PVOID      pBuffer;                      /* buffer for the API information */
  PQTOPLEVEL pqtTop;                            /* the top level information */

#define BUFSIZE 128000l
#define RESERVED 0

  pBuffer = malloc(BUFSIZE);                  /* allocate buffer for the API */
  if (pBuffer == NULL)                        /* check for proper allocation */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */

  memset(pBuffer,                               /* zero-out the whole buffer */
         0,
         BUFSIZE);

  rc = DosQuerySysState(0x3f,                        /* OK, now call the API */
                        RESERVED,
                        RESERVED,
                        RESERVED,
                        (PCHAR)pBuffer,
                        BUFSIZE);
  if (rc == NO_ERROR)                                     /* everything OK ? */
  {
    pqtTop = (PQTOPLEVEL)pBuffer;               /* assign the buffer pointer */

    rc = workerDumpProcess (pqtTop,
                            pszProcessName,
                            pldp);
  }

  free (pBuffer);                                         /* free the buffer */

  return (rc);                                                         /* OK */
}


ULONG RtQueryProcess(PUCHAR   name,
                     ULONG    numargs,
                     RXSTRING args[],
                     PSZ      queuename,
                     RXSTRING *retstr)
{
  APIRET      rc;                                          /* API returncode */
  APIRET      rc2;                                         /* API returncode */
  CHAR        szBuffer[32];             /* local buffer for the returnstring */
  RXSTEMDATA  ldp;                                              /* stem data */
  PSZ         pszProcessName;             /* name of the module to search for */
  PSZ         pszTemp;                           /* temporary string pointer */


  /*********************************************************/
  /* Parameter conversion and checking section             */
  /*********************************************************/

  BUILDRXSTRING(retstr,
                NO_UTIL_ERROR);                          /* pass back result */
                                                       /* validate arguments */
  if (numargs < 1 ||
      !RXVALIDSTRING(args[0]))
    return INVALID_ROUTINE;                                /* raise an error */
  
  if (numargs == 2 &&
      !RXVALIDSTRING(args[1]))
    return INVALID_ROUTINE;                                /* raise an error */
  
  if (numargs > 2)
    return INVALID_ROUTINE;                                /* raise an error */
  
  rc = rexxStemInit(&ldp,                             /* initialize our stem */
                    &args[0]);
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                                   /* raise error condition */
  
  if (numargs == 2)
    pszProcessName = args[1].strptr;
  else
    pszProcessName = NULL;


  /*********************************************************/
  /* OK, we got our parameter values checked and can go on.*/
  /*********************************************************/
  
  rc = workerQueryProcess(pszProcessName,
                         &ldp);

  /*********************************************************/
  /* now we got our information and can map it to the stem.*/
  /*********************************************************/

  rc2 = rexxStemFinalize(&ldp);                           /* close this stem */
  if (rc2 != NO_ERROR)
     return (rc2);                                  /* raise error condition */


  /*********************************************************/
  /* error handler                                         */
  /*********************************************************/

_err_rtqueryctryinfo:
  sprintf(szBuffer,
          "%d",
          rc);                              /* convert return code to string */

  BUILDRXSTRING(retstr,
                szBuffer);                         /* Fehler signalisieren   */
  return VALID_ROUTINE;                        /* finished                   */
}


/******************************************************************************
 * Name      : ULONG RtFileDelete
 * Funktion  : Delete a file using DosDelete or DosForceDelete
 * Parameter : PUCHAR   name
 *             ULONG    numargs   - number of arguments specified
 *             RXSTRING args[]    - array of arguments passed to this function
 *             PSZ      queuename - name of the queue
 *             RXSTRING *retstr   - string where to return the result
 * Syntax    : rc = RtFileDelete(filename, [options])
 *             options - "FORCE" do a force delete
 *                       "ATTRIBUTE" automatically reset read-only attribute
 * Variablen :
 * Ergebnis  : REXX-Interpreter Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-01-18]
 ******************************************************************************/

ULONG RtFileDelete(PUCHAR   name,
                   ULONG    numargs,
                   RXSTRING args[],
                   PSZ      queuename,
                   RXSTRING *retstr)
{
  APIRET rc;                                               /* API returncode */
  PSZ    pszFile;                                               /* file name */
  PSZ    pszOptions;                                     /* options          */
  CHAR   szBuffer[12];                  /* local buffer for the returnstring */
  CHAR   szOptions[64];
  BOOL   flagForce     = FALSE;
  BOOL   flagAttribute = FALSE;
  FILESTATUS fStat;                   /* filestatus structure for attributes */

  /*********************************************************/
  /* Parameter conversion and checking section             */
  /*********************************************************/

  BUILDRXSTRING(retstr,
                NO_UTIL_ERROR);                          /* pass back result */
                                                       /* validate arguments */
  if (numargs < 1 ||
      !RXVALIDSTRING(args[0]))
    return INVALID_ROUTINE;                                /* raise an error */

  pszFile            = args[0].strptr;

  if (numargs == 2)
    if(!RXVALIDSTRING(args[1]))
      return INVALID_ROUTINE;                              /* raise an error */
    else
      pszOptions = args[1].strptr;              /* map the options parameter */
  else
    pszOptions="";                          /* these are the default options */

  
  strncpy(szOptions, pszOptions, sizeof( szOptions ) );
  strupr(szOptions);                                /* capitalize the string */
  if (strstr(szOptions,"FORCE"))     flagForce     = TRUE;
  if (strstr(szOptions,"ATTRIBUTE")) flagAttribute = TRUE;


  /*********************************************************/
  /* OK, we got our parameter values checked and can go on.*/
  /*********************************************************/
  
  
  if (flagForce)
    rc = DosForceDelete(pszFile);            /* file will not be recoverable */
  else
    rc = DosDelete(pszFile);                      /* OK, remove that thing ! */

  if ( (rc == ERROR_ACCESS_DENIED) &&                 /* check for READ-ONLY */
      flagAttribute)
  {
    rc = DosQueryPathInfo (pszFile,                /* query file information */
                           FIL_STANDARD,
                           &fStat,
                           sizeof(fStat));
    if (rc == NO_ERROR)                                  /* check for errors */
    {
      fStat.attrFile = FILE_NORMAL;                  /* reset the attributes */

      rc = DosSetPathInfo (pszFile,                   /* set the information */
                           FIL_STANDARD,
                           &fStat,
                           sizeof(fStat),
                           0L);
    }

    /* now try again */
    if (NO_ERROR == rc)
      if (flagForce)
        rc = DosForceDelete(pszFile);        /* file will not be recoverable */
      else
        rc = DosDelete(pszFile);                  /* OK, remove that thing ! */
  }

  
  sprintf(szBuffer,
          "%d",
          rc);                              /* convert return code to string */

  BUILDRXSTRING(retstr,
                szBuffer);                         /* Fehler signalisieren   */
  
  return VALID_ROUTINE;                        /* finished                   */
}


/* to do

 RtSetPathInfo
 RtStemSort
 RtStemCopy
 RtStringReplace
 RtQueryAppType
 */
