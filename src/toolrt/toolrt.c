/*****************************************************************************
 * Projekt   : OS2TOOLS General C Library
 * Name      : Tools
 * Funktion  : Sammlung gemeinsamer Funktionen
 *
 * Autor     : Patrick Haller [Dienstag, 1997/04/29]
 *****************************************************************************/

#define SZLOGAPP "TOOL"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include <stdarg.h>
#include <time.h>
#include <ctype.h>

#include "tools.h"
#include "toolerror.h"


/***********************************************************************
 * Name      : Globale Variablen
 * Bemerkung : VORSICHT bei Mehrfachnutzung !
 *
 * Autor     : Patrick Haller [Freitag, 12.08.1994 05.20.48]
 ***********************************************************************/

static struct _Locals
{
  UCHAR         ucLoggingLevel;
  HFILE         hLogFile;
  char          pszLogApp[4];
} Locals;


/*****************************************************************************
 * Name      : ULONG ToolsVersion
 * Funktion  : returns numerical revision of library
 * Parameter : -
 * Variablen : -
 * Ergebnis  : ULONG - version number of the library
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 07.10.1995 03.22.44]
 *****************************************************************************/

ULONG TOOLAPI _Export ToolsVersion(void)
{
  return (__TOOLSVERSION__);
}


/*****************************************************************************
 * Name      : PSZ ToolsRevision
 * Funktion  : return pointer to static revision string
 * Parameter : -
 * Variablen : -
 * Ergebnis  : PSZ - pointer to revision string
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 07.10.1995 03.22.44]
 *****************************************************************************/

PSZ TOOLAPI _Export  ToolsRevision(void)
{
  static PSZ pszRevision = __iTOOLSREVISION__
                           "\n             "
                           __TOOLSCOPYRIGHT__;

  return (pszRevision);                             /* return static pointer */
}


/*****************************************************************************
 * Name      : APIRET ToolsPrintTitle
 * Funktion  : Prints the tool title on stdout
 * Parameter : -
 * Variablen : -
 * Ergebnis  : PTOOLAPPVERSION pToolAppVersion
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 07.10.1995 03.22.44]
 *****************************************************************************/

APIRET TOOLAPI _Export ToolsPrintTitle (PTOOLAPPVERSION pToolAppVersion)
{
//  const PSZ pszDonatedBy =
//    "\x1b[0www.\x1b[34;1minno\x1b[37;1mtek\x1b[0m.de";
  
  if (pToolAppVersion == NULL)                           /* check parameters */
    return (ERROR_TOOLS_INVALID_PARAMETER);         /* raise error condition */

  switch (pToolAppVersion->ucStructureVersion)    /* check structure version */
  {
    /*************************************************************************
     * this is the first release of the structure                            *
     *************************************************************************/
    case 1:
    {
      PTOOLAPPVERSION1 pToolAppVersion1 = (PTOOLAPPVERSION1)pToolAppVersion;

      fprintf (stdout,
               "\rอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ"
//               "\rอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ[ %s ]"
               "\n%-12s %x.%02x.%02x %s",
//               pszDonatedBy,
               pToolAppVersion1->pszName,
               pToolAppVersion1->ulVersion >> 16,
               (pToolAppVersion1->ulVersion & 0x0000FF00) >> 8,
               pToolAppVersion1->ulVersion & 0x000000FF,
               pToolAppVersion1->pszBuild);

      if (pToolAppVersion1->pszCopyright != NULL)/* check for copyright line */
        fprintf (stdout,
                 "\n             %s",
                 pToolAppVersion1->pszCopyright);

      if (pToolAppVersion1->pszRemark != NULL)      /* check for remark line */
        fprintf (stdout,
                 "\n             %s",
                 pToolAppVersion1->pszRemark);

      fprintf (stdout,                          /* print revision of library */
               "\n%s"
               "\nอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ",
               ToolsRevision() );

      /* warn user of old structure */
      if (pToolAppVersion1->ucStructureVersion != __TOOLSAPPVERSIONSTRUCT__)
        fprintf (stderr,
                 "\nWarning: This application uses a backlevel identification structure."
                 "\n         A newer release is recommended.");
    }
    break;


    /*************************************************************************
     * this is the 2nd   release of the structure                            *
     *************************************************************************/
    case 2:
      fprintf (stdout,
             "\rอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ"
//               "\rอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ[ %s ]"
               "\n%-12s %x.%02x.%02x %s",
//               pszDonatedBy,
               pToolAppVersion->pszName,
               pToolAppVersion->ulVersion >> 16,
               (pToolAppVersion->ulVersion & 0x0000FF00) >> 8,
               pToolAppVersion->ulVersion & 0x000000FF,
               pToolAppVersion->pszBuild);

      if (pToolAppVersion->pszCopyright != NULL) /* check for copyright line */
        fprintf (stdout,
                 "\n             %s",
                 pToolAppVersion->pszCopyright);

      if (pToolAppVersion->pszRemark != NULL)       /* check for remark line */
        fprintf (stdout,
                 "\n             %s",
                 pToolAppVersion->pszRemark);

      fprintf (stdout,                          /* print revision of library */
               "\n%s"
               "\nอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ",
               ToolsRevision() );

      /* check whether library and application are compatible */
      if (pToolAppVersion->ulVersionMin > __TOOLSVERSION__)
      {
        fprintf (stderr,
                 "\nError: Application requires at least TOOLRT %x.%02x.%02x.",
                 pToolAppVersion->ulVersionMin >> 16,
                 (pToolAppVersion->ulVersionMin & 0x0000FF00) >> 8,
                 pToolAppVersion->ulVersionMin & 0x000000FF);
        return (ERROR_TOOLS_LIBRARY_BACKLEVEL);                         /* raise error condition */
      }

      if (pToolAppVersion->ulVersionMax < __TOOLSVERSION__)
      {
        fprintf (stderr,
                 "\nWarning: Application only compatible up to TOOLRT %x.%02x.%02x.",
                 pToolAppVersion->ulVersionMax >> 16,
                 (pToolAppVersion->ulVersionMax & 0x0000FF00) >> 8,
                 pToolAppVersion->ulVersionMax & 0x000000FF);
        return (ERROR_TOOLS_LIBRARY_INCOMPATIBLE);                      /* raise error condition */
      }
      break;


    /*************************************************************************
     * this is an unknown release of the structure                           *
     *************************************************************************/

    default:
      fprintf (stderr,
               "\nError: This application is not compatible with the current version of"
               "\n       the TOOLRT library.");

      return (ERROR_TOOLS_INVALID_PARAMETER);       /* raise error condition */
  }

  return (NO_ERROR);                                                   /* OK */
}


/***********************************************************************
 * Name      : void ToolsErrorDos
 * Funktion  : Ausgeben einer Fehlermeldung aus OSO0001.MSG
 * Parameter : int nError
 * Variablen :
 * Ergebnis  :
 * Bemerkung : Statische Fehlermeldungsliste.
 *             Obviously Microsoft Visual C++ has problems in either
 *             multithreaded environment or optimization errors.
 *
 * Autor     : Patrick Haller [Dienstag, 13.06.1995 22.56.56]
 ***********************************************************************/

APIRET TOOLAPI _Export ToolsErrorDos ( APIRET rc )
{

#ifdef _WIN32
  LPVOID lpMsgBuf = NULL;                   /* pointer to the message buffer */

  if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
	            FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    rc,
                    MAKELANGID(LANG_NEUTRAL, 
                               SUBLANG_DEFAULT), // Default language
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL) != 0)
  {
    fputs(lpMsgBuf,                          /* write the message to stderr */
		  stderr);    /* use fputs instead fprintf to avoid resolving of %s */

    LocalFree( lpMsgBuf );                          /* free the local buffer */
  }
  else
	fprintf (stderr,                       /* display standard error message */
		     "\nError: unknown operating system error #%i.",
		     rc);

  return (NO_ERROR);                                       /* default for NT */
#endif

#ifdef __OS2__
  char  msg[4000];
  ULONG bc;
  APIRET rc2;                                              /* API returncode */

  rc2 = DosGetMessage(NULL,
                      0,
                      msg,
                      sizeof(msg),
                      rc,
                      "OSO001.MSG",
                      &bc);
  if (rc2 != NO_ERROR)                                   /* check for errors */
  {
    fprintf (stderr,                       /* display standard error message */
      	     "\nError: unknown operating system error #%i.",
             rc);
    return (rc2);                                   /* raise error condition */
  }

  fprintf (stderr,
           "\n");

  rc2 = DosPutMessage(2,
                      bc,
                      msg );
  return (rc2);                                       /* deliver return code */
#endif
}

/***********************************************************************
 * Name      : void ToolsErrorDosEx
 * Funktion  : Ausgeben einer Fehlermeldung aus OSO0001.MSG
 * Parameter : int nError, PSZ pszMessage
 * Variablen :
 * Ergebnis  :
 * Bemerkung : Statische Fehlermeldungsliste.
 *             Obviously Microsoft Visual C++ has problems in either
 *             multithreaded environment or optimization errors.
 *
 * Autor     : Patrick Haller [Dienstag, 13.06.1995 22.56.56]
 ***********************************************************************/

APIRET TOOLAPI _Export ToolsErrorDosEx (APIRET rc,
                                       PSZ    pszMessage )
{
  APIRET rc2;                                              /* API returncode */

  rc2 = ToolsErrorDos(rc);                     /* first the standard message */

  if (pszMessage != NULL)                      /* if a message was supplied, */
    fprintf (stderr,                                           /* display it */
             pszMessage);

  return (rc2);                                 /* pass thru the return code */
}


/*****************************************************************************
 * Name      : APIRET ToolsPerfQuery
 * Funktion  : Timer mit 840ns Aufl”sung
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Mittwoch, 11.05.1994 14.04.33]
 *****************************************************************************/

APIRET TOOLAPI _Export ToolsPerfQuery(PERFSTRUCT *t)
{
#ifdef __OS2__
  APIRET rc;                                               /* API Returncode */
  QWORD  qwTime;                                              /* timer value */
  static ULONG ulFrequency = 0;                           /* timer frequency */

  if (ulFrequency == 0)                              /* not yet determined ? */
  {
    rc = DosTmrQueryFreq (&ulFrequency);      /* query hires timer frequency */
    if (rc != NO_ERROR)                                      /* check errors */
      return (rc);                                   /* return API errorcode */
  }

  rc = DosTmrQueryTime(&qwTime);                  /* query hires timer value */
  if (rc != NO_ERROR)                                        /* check errors */
    return (rc);                                    /* return API returncode */

  t->fSeconds = (double)qwTime.ulHi;
  t->fSeconds *= 65536.0;                                            /* 2~32 */
  t->fSeconds *= 65536.0;
  t->fSeconds += (double)qwTime.ulLo;

  t->fSeconds = t->fSeconds / (double)ulFrequency;

  return NO_ERROR;                                       /* OK, we succeeded */
#endif

#ifdef _WIN32
  static LARGE_INTEGER liFrequency;                /* 64-bit timer frequency */
  LARGE_INTEGER liCounter;                     /* 64-bit timer counter value */

  if (liFrequency.QuadPart == 0)           /* frequency not yet determined ? */
  {
    if (FALSE == QueryPerformanceFrequency (&liFrequency) )
      return (ERROR_NOT_SUPPORTED);            /* hi-res timer not supported */
  }

  if (FALSE == QueryPerformanceCounter (&liCounter) )
    return (ERROR_NOT_SUPPORTED);              /* hi-res timer not supported */

                                  /* 64-bit integer arithmetic is required ! */
  t->fSeconds = (double)liCounter.QuadPart / (double)liFrequency.QuadPart;

  return (NO_ERROR);
#endif
}


/***********************************************************************
 * Name      : APIRET ToolsDumpHex
 * Funktion  : Anzeigen der gefundenen Detailinformation
 * Parameter :
 * Variablen :
 * Ergebnis  : Ausgabe auf dem Bildschirm
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.49.57]
 ***********************************************************************/

APIRET TOOLAPI _Export ToolsDumpHex (ULONG ulPosition,
                                    ULONG ulLength,
                                    PVOID pSource)
{
  ULONG ulCounter;                                 /* temporary loop counter */
  CHAR  szBuffer[128];            /* local, temporary buffer for debug lines */
  CHAR  szBuffer2[80];            /* local, temporary buffer for debug lines */
  CHAR  szBuffer3[80];            /* local, temporary buffer for debug lines */
  CHAR  szBuffer4[20];            /* local, temporary buffer for debug lines */
  UCHAR ucData;                                                 /* data byte */
  PCHAR pchTemp;                                /* pointer to that data byte */
  ULONG ulTotalLength;           /* total length of printed buffer + padding */

  sprintf(szBuffer, "%08xณ", ulPosition);

  szBuffer2[0] = 0;
  szBuffer3[0] = 0;

  ulTotalLength = (ulLength + 0x0000000f) & 0xfffffff0;

  for (ulCounter = 0;
       ulCounter < ulTotalLength;                           /* round up to 8 */
       ulCounter++)
  {
    if (ulCounter < ulLength)
    {
      pchTemp = (PCHAR)pSource + ulCounter;
      ucData = *pchTemp;
      sprintf(szBuffer4, "%02x ", ucData);
      strcat (szBuffer2, szBuffer4);                        /* add hex token */

      if (ucData < ' ')
        strcat (szBuffer3,"๚");                     /* unprintable character */
      else
      {
        szBuffer4[0] = ucData;
        szBuffer4[1] = 0;
        strcat(szBuffer3, szBuffer4);
      }
    }
    else
    {
      strcat(szBuffer2,"๚๚ ");                    /* insert dummy characters */
      strcat(szBuffer3," ");
    }

    if (ulCounter % 0x00000010 == 0x0000000f)         /* check for linebreak */
    {
      strcat(szBuffer, szBuffer2);
      strcat(szBuffer,"ณ");
      strcat(szBuffer, szBuffer3);

      printf ("\n%s",szBuffer);

      szBuffer2[0] = 0;
      szBuffer3[0] = 0;
      sprintf(szBuffer, "%08xณ", ulCounter+1+ulPosition);
    }
  }

  return (NO_ERROR);                                                   /* OK */
}


/***********************************************************************
 * Name      : APIRET TOOLAPI _Export ToolsDumpAscii
 * Funktion  : Anzeigen der gefundenen Detailinformation
 * Parameter :
 * Variablen :
 * Ergebnis  : Ausgabe auf dem Bildschirm
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.49.57]
 ***********************************************************************/

#define ASCIIWIDTH 64

APIRET TOOLAPI _Export ToolsDumpAscii (ULONG ulPosition,
                                      ULONG ulLength,
                                      PVOID pSource)
{
  ULONG ulCounter;                                 /* temporary loop counter */
  CHAR  szBuffer[128];            /* local, temporary buffer for debug lines */
  CHAR  szBuffer3[80];            /* local, temporary buffer for debug lines */
  PSZ   pszSource;                              /* pointer to that data byte */
  PSZ   pszTemp;
  ULONG ulTotalLength;           /* total length of printed buffer + padding */

  sprintf(szBuffer,
          "%08xณ",
          ulPosition);

  pszTemp      = szBuffer3;                                /* initialization */

  ulTotalLength = (ulLength + ASCIIWIDTH - 1) & (-ASCIIWIDTH);   /* round up */

  for (ulCounter = 0,
       pszSource = pSource;
       ulCounter < ulTotalLength;                           /* round up to 8 */
       ulCounter++,
       pszSource++)
  {
    if (ulCounter < ulLength)          /* check if we are within valid range */
      if (*pszSource > ' ')                    /* check for valid characters */
        *pszTemp = *pszSource;                        /* then simply copy it */
      else
        *pszTemp = '๚';                          /* else fill up with spaces */
    else
      *pszTemp = ' ';                                  /* behind valid range */

    pszTemp++;                    /* proceed to the next formatted character */

    if ( (ulCounter % ASCIIWIDTH == ASCIIWIDTH - 1) ||             /* Output */
         (ulCounter == ulTotalLength) )
    {
      *pszTemp = 0;                                       /* set termination */
      printf ("\n%s%s",
              szBuffer,
              szBuffer3);

      sprintf(szBuffer,
              "%08xณ",
              ulCounter+1+ulPosition);

      pszTemp = szBuffer3;                     /* re-initialize this pointer */
    }
  }

  return (NO_ERROR);                                                   /* OK */
}


/*****************************************************************************
 * Name      : APIRET TOOLAPI _Export StrTrim
 * Funktion  : Raeumt einen String auf = Leerzeichen abtrennen
 * Parameter : PSZ pszSource
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 07.10.1995 03.22.44]
 *****************************************************************************/

APIRET TOOLAPI _Export StrTrim (PSZ pszSource)
{
  PSZ pszTemp;

  if (pszSource == NULL)                                 /* check parameters */
    return (ERROR_TOOLS_INVALID_PARAMETER);         /* raise error condition */

  for (pszTemp=pszSource;                              /* Cut leading spaces */
       *pszTemp && (*pszTemp <= ' ');
       pszTemp++ )
  {}
  
  if (pszTemp != pszSource)
    memcpy (pszSource,
            pszTemp,
            strlen(pszTemp)+1);

  for (pszTemp = pszSource+strlen(pszSource)-1;       /* Cut trailing spaces */
       (pszTemp >= pszSource) && (*pszTemp <= ' ');
       *pszTemp-- = 0)
  {}

  return (NO_ERROR);                                                   /* OK */
}


/*****************************************************************************
 * Name      : ULONG StrToNumber
 * Funktion  : Converts a string to a number, accepts also hex entry
 * Parameter : PSZ pszString
 * Variablen :
 * Ergebnis  : ULONG ulTheMagicNumber
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 07.10.1995 03.22.44]
 *****************************************************************************/

ULONG TOOLAPI _Export StrToNumber(PSZ  pszString,
                                  BOOL fSigned)
{
  UCHAR ucBase   = 10;                 /* the base for the number conversion */
  UCHAR ucOffset = 0;                         /* offset into the data string */
  ULONG ulNumber;                                /* the parsed number itself */
  ULONG ulMultiplier = 1;                        /* an additional multiplier */
  size_t iMultiplierPos;              /* position of the multiplier (if any) */
  
  if (pszString == NULL)                                 /* check parameters */
    return 0;                                 /* then return this as default */

  if (pszString[0] == 0)                                 /* check parameters */
    return 0;                                 /* then return this as default */


  if (pszString[0] == '$')                   /* check whether decimal or hex */
  {
    ucBase   = 16;                                             /* hex number */
    ucOffset = 1;                                         /* skip the dollar */
  }
  else
    if ( (pszString[0] == '0') &&            /* check whether decimal or hex */
         ( (pszString[1] == 'X') ||
           (pszString[1] == 'x') ) )
    {
      ucBase   = 16;                                           /* hex number */
      ucOffset = 2;                                           /* skip the 0x */
    }
  
  /* multiplier suffix */
  iMultiplierPos = strspn(pszString + ucOffset,
                          "0123456789");
  if (0 != iMultiplierPos)
  {
    PSZ pszMultiplier = pszString + ucOffset + iMultiplierPos;
    
    if (stricmp("k", pszMultiplier) == 0)
      ulMultiplier = 1024;
    else
    if (stricmp("m", pszMultiplier) == 0)
      ulMultiplier = 1024 * 1024;
    else
    if (stricmp("g", pszMultiplier) == 0)
      ulMultiplier = 1024 * 1024 * 1024;
  }
  
  if (fSigned == TRUE)                            /* take care of the sign ? */
    ulNumber = strtol(pszString + ucOffset,         /* do the hex processing */
                      NULL,
                      ucBase);
  else
    ulNumber = strtoul(pszString + ucOffset,        /* do the hex processing */
                       NULL,
                       ucBase);

  return ulNumber * ulMultiplier;
}


/*****************************************************************************
 * Name      : APIRET ReadFileToBuffer
 * Funktion  : Reads a single file completely into a memory buffer
 * Parameter : PSZ pszFile, PPVOID ppBuffer, PULONG pulBuffer
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 07.10.1995 03.22.44]
 *****************************************************************************/

APIRET TOOLAPI _Export ToolsReadFileToBuffer (PSZ    pszFile,
                                             PPVOID ppBuffer,
                                             PULONG pulBuffer)
{
#ifdef __OS2__
  APIRET      rc;                                          /* API-Returncode */
  FILESTATUS3 fs3Status;                               /* Dateiinformationen */
  HFILE       hFileInput;                               /* input file handle */
  ULONG       ulAction;                       /* dummy parameter for DosOpen */
  ULONG       ulReadSize;                /* number of bytes to read per call */
  ULONG       ulReadTotal = 0;                 /* total number of bytes read */

  if ( (pszFile   == NULL) ||                            /* check parameters */
       (ppBuffer  == NULL) ||
       (pulBuffer == NULL) )
    return (ERROR_TOOLS_INVALID_PARAMETER);                  /* signal error */


  rc = DosQueryPathInfo (pszFile,                          /* Infos einholen */
                         FIL_STANDARD,
                         &fs3Status,
                         sizeof(fs3Status));
  if (rc != NO_ERROR)                           /* check if an error occured */
    return (rc);                                             /* signal error */

                                         /* allocate buffer RAM for the file */
  *pulBuffer = fs3Status.cbFile;              /* return the size of the file */
  rc = DosAllocMem(ppBuffer,                          /* allocate the buffer */
                   fs3Status.cbFile,
                   PAG_WRITE |
                   PAG_READ  |
                   PAG_COMMIT);
  if (rc != NO_ERROR)                      /* check if the allocation failed */
    return (rc);                                             /* signal error */

  ulReadSize = OS2READSIZE;       /* for OS/2 2.x, 3.x, 4.0 best performance */
  if (fs3Status.cbFile < ulReadSize) /* only if file is smaller, we decrease */
    ulReadSize = fs3Status.cbFile;

  rc = DosOpen(pszFile,                                    /* File path name */
               &hFileInput,                                   /* File handle */
               &ulAction,                                    /* Action taken */
               0L,                                /* File primary allocation */
               FILE_ARCHIVED |
               FILE_NORMAL,                                /* File attribute */
               OPEN_ACTION_FAIL_IF_NEW |
               OPEN_ACTION_OPEN_IF_EXISTS,             /* Open function type */
               OPEN_FLAGS_NOINHERIT |
               OPEN_FLAGS_SEQUENTIAL|
               OPEN_SHARE_DENYNONE  |
               OPEN_ACCESS_READONLY,                /* Open mode of the file */
               0L);                                 /* No extended attribute */
  if (rc == NO_ERROR)                                    /* check for errors */
  {
    ULONG ulRead;                                    /* number of read bytes */
    PSZ   pReadPosition = *ppBuffer;                      /* target position */

    do
    {
#if 0
      fprintf (stderr,
               "\nDEBUG: hInput=%08xh, pRead=%08xh, ulSize=%u, &ulRead=%08xh",
               hFileInput,
               pReadPosition,
               ulReadSize,
               &ulRead);
#endif

      rc = DosRead (hFileInput,
                    pReadPosition,
                    ulReadSize,
                    &ulRead);

      pReadPosition += ulRead;                     /* move the read position */
      ulReadTotal   += ulRead;                         /* update the counter */

      ulReadSize = fs3Status.cbFile - ulReadTotal;     /* the remaining part */
      if (ulReadSize > OS2READSIZE)                 /* bigger than optimum ? */
        ulReadSize = OS2READSIZE;                /* then cut down to default */
    }
    while ( (rc == NO_ERROR) &&
            (ulRead != 0) &&
            (ulReadTotal != fs3Status.cbFile)
          );

    DosClose (hFileInput);               /* close the filehandle in any case */
  }

  if (rc != NO_ERROR)                                    /* check for errors */
  {
    DosFreeMem (*ppBuffer);                     /* free the allocated memory */
    *ppBuffer  = NULL;                       /* reset the passed back values */
    *pulBuffer = 0;
  }

  return (rc);                                                /* return code */
#endif



#ifdef _WIN32
  HANDLE      hFileInput;                               /* input file handle */
  ULONG       ulAction;                       /* dummy parameter for DosOpen */
  ULONG       ulFileSize;                    /* the size of the file to read */

  if ( (pszFile   == NULL) ||                            /* check parameters */
       (ppBuffer  == NULL) ||
       (pulBuffer == NULL) )
    return (ERROR_TOOLS_INVALID_PARAMETER);                  /* signal error */


  hFileInput = CreateFile(pszFile,                               /* filename */
                          GENERIC_READ,               /* desired access mode */
                          FILE_SHARE_READ,                     /* share mode */
                          NULL,               /* default security descriptor */
                          OPEN_EXISTING,                        /* open mode */
                          FILE_ATTRIBUTE_NORMAL |         /* file attributes */
                          FILE_FLAG_SEQUENTIAL_SCAN,
                          (HANDLE)0);                     /* template handle */
  if (hFileInput == INVALID_HANDLE_VALUE)                /* check for errors */
    return (GetLastError());                        /* raise error condition */


  ulFileSize = GetFileSize(hFileInput,           /* get the size of the file */
                           NULL);
  if (0xFFFFFFFF == ulFileSize)                 /* check if an error occured */
  {
    CloseHandle(hFileInput);                               /* close the file */
    return (ERROR_OPEN_FAILED);                              /* signal error */
  }


                                         /* allocate buffer RAM for the file */
  *pulBuffer = ulFileSize;                    /* return the size of the file */
  *ppBuffer = VirtualAlloc(NULL,                 /* desired starting address */
                           ulFileSize,                 /* size of allocation */
                           MEM_COMMIT,                   /* allocation flags */
                           PAGE_READWRITE);             /* read/write access */
  if (*ppBuffer == NULL)                                 /* check for errors */
  {
    CloseHandle(hFileInput);                               /* close the file */
    return (ERROR_OPEN_FAILED);                              /* signal error */
  }

                                     /* now, finally read the file to memory */
  if (ReadFile(hFileInput,
               *ppBuffer,
               ulFileSize,
               &ulAction,
               NULL) == FALSE)                 /* read file and check errors */
  {
    VirtualFree(*ppBuffer,                                /* free the memory */
                0,
                MEM_RELEASE);
    ulAction = GetLastError();                 /* save the error information */
    CloseHandle(hFileInput);                               /* close the file */
    *ppBuffer  = NULL;                       /* reset the passed back values */
    *pulBuffer = 0;
    return (ulAction);                                       /* signal error */
  }

  CloseHandle(hFileInput);                     /* close the file, rest is OK */

  return (NO_ERROR);                                          /* return code */
#endif
}


/******************************************************************************
 * Name      : void StrValueToSize
 * Funktion  : Creates a string with the formatted valued passed
 * Parameter : PSZtr, ULONG value
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:48:00]
 ******************************************************************************/

void TOOLAPI _Export StrValueToSize (PSZ   pszString,
                                    ULONG ulValue)
{
  double fValue;                                            /* converted value */

                                               /* value means a size in bytes */
  if (ulValue < 10000)                                         /* 10000 bytes */
  {
    sprintf (pszString,                       /* convert value to byte string */
             "%6ub",
             ulValue);
    return;
  }

  if (ulValue < 10240000)                                          /* 1024 kB */
  {
    fValue = (double)ulValue / 1024.0;               /* convert value to double */

    sprintf (pszString,         /* convert temporary value to kilobyte string */
             "%6.1fk",
             fValue);
    return;
  }

  fValue = (double)ulValue / 1048576.0;                /* calculate mega bytes */
  sprintf (pszString,                     /* convert value to megabyte string */
           "%6.1fM",
           fValue);
}


/*****************************************************************************
 * Name      : void StrValueToSizeFloat
 * Funktion  : Creates a string with the formatted valued passed
 * Parameter : PSZtr, float value
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag,09.03.01 - 09:48:00]
 *****************************************************************************/

void TOOLAPI _Export StrValueToSizeFloat (PSZ   pszString,
                                          float fValue)
{
  if (fValue < 10000)                                         /* 10000 bytes */
  {
    sprintf (pszString,                      /* convert value to byte string */
             "%6.0fb",
             fValue);
    return;
  }

  if (fValue < 10240000)                                         /* 10000 kB */
  {
    sprintf (pszString,        /* convert temporary value to kilobyte string */
             "%6.1fk",
             fValue / 1024.0);
    return;
  }

  if (fValue < 10485760000.0)                                    /* 10000 MB */
  {
    sprintf (pszString,        /* convert temporary value to kilobyte string */
             "%6.1fM",
             fValue / 1048576.0);
    return;
  }

  sprintf (pszString,                    /* convert value to megabyte string */
           "%6.1fG",
           fValue / 1073741824.0);
}


/******************************************************************************
 * Name      : ToolsConfirmationQuery
 * Funktion  : Get confirmation from user
 * Parameter :
 * Variablen :
 * Ergebnis  : 0 - no, 1 - yes, 2 - escape, 10 - unknown, error
 * Bemerkung : Perhaps adapt to full NLS support.
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:48:00]
 ******************************************************************************/

int TOOLAPI _Export ToolsConfirmationQuery (void)
{
  int ch;

  printf ("[Y/N/Esc]");

  for(;;)
  {
    ch = getch ();
    if ( (ch == 'Y') ||
         (ch == 'y') ||
         (ch == 'N') ||
         (ch == 'n') ||
         (ch ==  27) )                                                /* ESC */
      break;
  }

  switch (ch)
  {
    case 'N':
    case 'n':
      printf ("No.");
      return 0;

    case 'Y':
    case 'y':
      printf ("Yes.");
      return 1;

    case  27:
      printf ("Aborting.");
      return 2;
  }

  /* dummy */
  return 10;
}


/***********************************************************************
 * Name      : PSZ StrFDateTimeToString
 * Funktion  : Konvertieren eines kompletten Datums in einen String
 * Parameter : FDATE fDate, FTIME fTime, PSZ szBuffer
 * Variablen :
 * Ergebnis  : in szBuffer
 * Bemerkung : NLS Support fehlt noch.
 *
 * Autor     : Patrick Haller [Donnerstag, 04.08.1995 00.49.57]
 ***********************************************************************/
PSZ TOOLAPI _Export StrFDateTimeToString (FDATE fDate,
                                 FTIME fTime,
                                 PSZ   szBuffer)
{
  if (szBuffer == NULL)                                    /* Parametercheck */
    return NULL;

  if ((fDate.day     == 0) &&                 /* Does the FSD support this ? */
      (fDate.month   == 0) &&
      (fDate.year    == 0) &&
      (fTime.hours   == 0) &&
      (fTime.minutes == 0) &&
      (fTime.twosecs == 0))
    strcpy (szBuffer,
            "* not supplied *");
  else
    /* XXXDay, xx.yy.zzzz, 11:22 a.m. */
    sprintf (szBuffer,
           "%4u/%02u/%02u %02u:%02u",
           fDate.year + 1980,          /* with correction of the date origin */
           fDate.month,
           fDate.day,
           fTime.hours,
           fTime.minutes);

  return (szBuffer);
}


#ifdef __OS2__
/* not yet ported to NT */

/***********************************************************************
 * Name      : PSZ StrFAttrToString
 * Funktion  : Konvertieren eines kompletten Attributsatzes in String
 * Parameter : ULONG fAttr, PSZ szBuffer
 * Variablen :
 * Ergebnis  : in szBuffer
 * Bemerkung : NLS Support fehlt noch.
 *
 * Autor     : Patrick Haller [Donnerstag, 04.08.1995 00.49.57]
 ***********************************************************************/

PSZ TOOLAPI _Export StrFAttrToString (ULONG fAttr,
                                     PSZ   szBuffer)
{
  if (szBuffer == NULL)                              /* Parametercheck */
    return NULL;

  *szBuffer = 0;                   /* Target string becomes terminated */

  if (fAttr & FILE_READONLY)
    strcat (szBuffer,"Read-only ");

  if (fAttr & FILE_ARCHIVED)
    strcat (szBuffer,"Archived ");

  if (fAttr & FILE_HIDDEN)
    strcat (szBuffer,"Hidden ");

  if (fAttr & FILE_DIRECTORY)
    strcat (szBuffer,"Directory ");

  if (fAttr & FILE_SYSTEM)
    strcat (szBuffer,"System ");

  return (szBuffer);
}


/***********************************************************************
 * Name      : PSZ StrFAttrToStringShort
 * Funktion  : Konvertieren eines kompletten Attributsatzes in String
 * Parameter : ULONG fAttr, PSZ szBuffer
 * Variablen :
 * Ergebnis  : in szBuffer
 * Bemerkung : NLS Support fehlt noch.
 *
 * Autor     : Patrick Haller [Donnerstag, 04.08.1995 00.49.57]
 ***********************************************************************/

PSZ TOOLAPI _Export StrFAttrToStringShort (ULONG fAttr,
                                  PSZ   szBuffer)
{
  if (szBuffer == NULL)                                    /* Parametercheck */
    return NULL;

  szBuffer[5]= 0;                        /* Target string becomes terminated */

  if (fAttr & FILE_READONLY ) szBuffer[0]='R'; else szBuffer[0]='.';
  if (fAttr & FILE_ARCHIVED ) szBuffer[1]='A'; else szBuffer[1]='.';
  if (fAttr & FILE_HIDDEN   ) szBuffer[2]='H'; else szBuffer[2]='.';
  if (fAttr & FILE_SYSTEM   ) szBuffer[3]='S'; else szBuffer[3]='.';
  if (fAttr & FILE_DIRECTORY) szBuffer[4]='D'; else szBuffer[4]='.';

  return (szBuffer);
}
#endif


/***********************************************************************
 * Name      : ULONG ToolsDateToAge
 * Funktion  : julianisches Datum ermitteln
 * Parameter : USHORT tag, USHORT monat, USHORT jahr
 * Variablen :
 * Ergebnis  : ULONG ulDayNumber
 * Bemerkung :
 *
 * Autor     : Ralf Lohmueller
 ***********************************************************************/

ULONG TOOLAPI _Export ToolsDateToAge (USHORT tag,
                                     USHORT monat,
                                     USHORT jahr)
{
  ULONG  tagnr;
  SHORT  sJahrDim = jahr-1;

  const signed char calendar[12] = {0,1,-1,0,0,1,1,2,3,3,4,4};

  tagnr  = ( sJahrDim * 365L ) ;
  tagnr += ( sJahrDim >> 2 ) ;
  tagnr -= ( (sJahrDim - sJahrDim >> 2 ) / 100) ;
  tagnr += ( calendar[monat-1] + ( (monat-1) * 30 ) ) ;
  tagnr += (
             ( (jahr % 4 == 0) && (jahr % 100 != 0) )
             || ( jahr % 400 == 0 )
           )
             && ( monat > 2 ) ;
  tagnr += tag ;

  return(tagnr);
}


/*****************************************************************************
 * Name      : int ToolsDateCompare
 * Funktion  : compares two file dates
 * Parameter : FDATE fdate1,
 *             FTIME ftime1,
 *             FDATE fdate2,
 *             FTIME ftime2
 * Variablen :
 * Ergebnis  : integer value describing the date difference
 * Bemerkung : result < 0, when file 1 older than file 2
 *             result = 0, when files are equal age
 *             result > 0, when file 1 newer than file 2
 *
 * Autor     : Patrick Haller
 *****************************************************************************/

int TOOLAPI _Export ToolsDateCompare (FDATE fdate1,        /* compares two file dates */
                             FTIME ftime1,
                             FDATE fdate2,
                             FTIME ftime2)
{
  double fFile1;                                      /* date value of file 1 */
  double fFile2;                                      /* date value of file 2 */

  fFile1 = (double)ToolsDateToAge(fdate1.day,             /* calculate file 1 */
                                 fdate1.month,
                                 fdate1.year) * 86400.0 +
           (double)ftime1.hours * 3600.0 +
           (double)ftime1.minutes * 60.0 +
           (double)ftime1.twosecs *  2.0;

  fFile2 = (double)ToolsDateToAge(fdate2.day,             /* calculate file 2 */
                                 fdate2.month,
                                 fdate2.year) * 86400.0 +
           (double)ftime2.hours * 3600.0 +
           (double)ftime2.minutes * 60.0 +
           (double)ftime2.twosecs *  2.0;

  if (fFile1 == fFile2)                                  /* exactly same age */
    return (0);

  if (fFile1 < fFile2)
    return (-1);                                 /* file 1 older than file 2 */
  else
    return (1);                                  /* file 1 newer than file 2 */
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
 * Name      : APIRET ToolsCrc32String
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

APIRET TOOLAPI _Export ToolsCrc32String(PSZ    pszText,
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
 * Name      : APIRET ToolsCrc32Buffer
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

APIRET TOOLAPI _Export ToolsCrc32Buffer(PSZ    pszText,
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
  
  /* Note: because of possible subsequent buffer calls,
   * we may not yet invert the result!
   */
  *pulCRC = ulCRC32;                                 /* pass back the result */
  
  return NO_ERROR;                                                     /* OK */
}

ULONG TOOLAPI _Export ToolsCrc32BufferFinalize(ULONG ulCRC32)
{
  return ~ulCRC32;
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
 * Name      : APIRET ToolsMD5Initialize
 * Funktion  : The routine initializes the MD5 (message-digest) context
 * Parameter : PMD5_CTX pmd5_ctx
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung : BROKEN
 *
 * Autor     : Patrick Haller [Monday, 1998/05/31]
 *****************************************************************************/

APIRET TOOLAPI _Export ToolsMD5Initialize (PMD5_CTX pmdContext)
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
 * Name      : APIRET ToolsMD5Update
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

APIRET TOOLAPI _Export ToolsMD5Update (PMD5_CTX pmdContext,
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
 * Name      : APIRET ToolsMD5Finalize
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

APIRET TOOLAPI _Export ToolsMD5Finalize (PMD5_CTX pmdContext)
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
  
  ToolsMD5Update (pmdContext,
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


/***********************************************************************
 * Name      : APIRET StrInsert
 * Funktion  : Fgt eine Zeichenkette an die aktuelle Stringposition ein.
 * Parameter : PSZ pszPos, PSZ pszInsert
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 22.10.93 - 20:12:00]
 ***********************************************************************/

APIRET TOOLAPI _Export StrInsert (      PSZ pszPos,
                                 const PSZ pszInsert)
{
  ULONG ulStringLength;                     /* Laenge der Zeichenkette */

  if ( (pszPos    == NULL) ||                  /* Parameterberprfung */
       (pszInsert == NULL) )
    return (ERROR_INVALID_PARAMETER);          /* Fehler signalisieren */

  ulStringLength = strlen(pszInsert);              /* Laenge ermitteln */

  memmove (pszPos + ulStringLength,
           pszPos,
           strlen(pszPos) + 1);                      /* Platz schaffen */
  memcpy  (pszPos,                           /* Zeichenkette einfuegen */
           pszInsert,
           ulStringLength);

  return (NO_ERROR);                           /* Rckgabewert liefern */
}


/***********************************************************************
 * Name      : APIRET StrDelete
 * Funktion  : L”schen von Zeichen aus einem String
 * Parameter : PSZworkstr,USHORT pos, USHORT num
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag,16.09.93 - 16:40:39]
 ***********************************************************************/

APIRET TOOLAPI _Export StrDelete (PSZ   pszString,
                                 ULONG ulPosition,
                                 ULONG ulNumber)
{
  PSZ pszDest;                            /* Destination - Zieladresse */
  PSZ pszSrc;

  if ( (pszString == NULL) ||                  /* Parameterberprfung */
       (ulNumber  == 0) )
    return (ERROR_INVALID_PARAMETER);          /* Fehler signalisieren */

  pszDest = pszString + ulPosition;           /* Zieladresse berechnen */
  pszSrc  = pszDest   + ulNumber;            /* Quelladresse berechnen */
  memmove (pszDest,                          /* Zeichen ueberschreiben */
           pszSrc,
           strlen(pszSrc) + 1);
  
  return (NO_ERROR);                                                   /* OK */
}


/***********************************************************************
 * Name      : void StrReplace2
 * Funktion  : Search & Replace, bufferresizing
 * Parameter : PSZ*buffer, ULONG *bufsize, PSZreplace, PSZby
 * Variablen : 
 * Ergebnis  : neuer String
 * Bemerkung : soll(te) automatisch den Puffer vergr”แern, wenn notwendig.
 *
 * Autor     : Patrick Haller [Samstag, 15.04.1994 00.39.42]
 ***********************************************************************/

APIRET TOOLAPI _Export StrSearchReplace (PPSZ   ppszBuffer,
                                        PULONG pulBufferSize,
                                        PSZ    pszOriginal,
                                        PSZ    pszReplacement)
{
#if 0
  ULONG slen_rep;
  ULONG slen_by;
  ULONG ulLengthBuffer;
  int   delta;

  PSZ p;
  PSZ pszTemp;

  p = *buffer;
  slen_rep = strlen(replace);
  slen_by  = strlen(by);
  ulLengthBuffer = strlen(*buffer);

  while ( (p = strstr (*buffer,
                       replace)) != NULL )
  {
    delta = slen_by - slen_rep;
    pszTemp = p + delta;

    if (ulLengthBuffer + delta >= *bufsize)
    {
      *bufsize+=1024; /* increase buffer by 1kB */
      *buffer = (PSZ)realloc(*buffer,
                             *bufsize);
      if (!(*buffer))
      {
        *buffer = NULL; /* error */
        return;
      }
    }

    ulLengthBuffer += delta;

    /* replacement */

    if (!delta);
    else
      if (delta < 0)
        memmove (p,
                 p-delta,
                 strlen (p - delta) + 1);
      else
        memmove (pszTemp,
                 p, 
                 strlen (p) + 1);
    
    memcpy (p,
            by,
            slen_by);
  }
#endif
  return (NO_ERROR);                                                   /* OK */
}


/***********************************************************************
 * Name      : void Strreplace
 * Funktion  : Ersetzen eines Strings in einem anderen String
 * Parameter : PSZworkstr,PSZinsstr,USHORT pos
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag,16.09.93 - 16:40:39]
 ***********************************************************************/

APIRET TOOLAPI _Export StrReplace (PSZ   pszDestination,
                                  PSZ   pszInsert,
                                  ULONG ulPosition)
{
  PSZ   pszTempDest;                        /* temporary destination pointer */
  PSZ   pszTempSource;                      /* temporary source pointer      */
  ULONG ulStringLength;                    /* length of the string to insert */
  
  if ( (pszDestination == NULL) ||                       /* check parameters */
       (pszInsert      == NULL) )
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */
  
  ulStringLength = strlen(pszInsert);                   /* get string length */
  
                                                        /* calculate offsets */
  pszTempDest    = pszDestination + ulPosition + ulStringLength - 1;
  pszTempSource  = pszDestination + ulPosition;

  memmove (pszTempDest,
           pszTempSource,
           strlen(pszDestination) - ulPosition + 1);
  
  strncpy (pszTempSource,
           pszInsert,
           ulStringLength);
  
  return (NO_ERROR);                                                   /* OK */
}


/***********************************************************************
 * Name      : APIRET StrRealcase
 * Funktion  : Groแ/Kleinschreibung einer Zeichenkette
 * Parameter : PSZ pszString
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 26.11.1993 - 16:10:23]
 ***********************************************************************/

APIRET TOOLAPI _Export StrRealcase (PSZ pszString)
{
  UCHAR ucLast;
  
  if (pszString == NULL)                                 /* check parameters */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */
  
  ucLast = 0;
  
  while (*pszString)
  {
    if ((ucLast) && (*pszString >= 'A') && (*pszString <= 'Z')) 
      *pszString+=32;
    
    if (isalpha (*pszString)) 
      ucLast = 1;
    else 
      ucLast = 0;
    
    pszString++;
  }
  
  return (NO_ERROR);                                                   /* OK */
}
