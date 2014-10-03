/*****************************************************************************
 * Projekt   : OS2TOOLS General C Library
 * Name      : Modul os2tools
 * Funktion  : Sammlung gemeinsamer Funktionen
 *
 * Autor     : Patrick Haller [Dienstag, 26.09.1995 17.57.39]
 *****************************************************************************/

 /* todo

  - @FILENAME als primaerquelle fuer die parameter
    -> ArgParseFile
  - Environmentvariablen fuer die parameter
    -> ArgParseEnvironment
  - VIO Dialog zur parametereingabe
  - strtrim, strinsert, strdelete

  */

#ifndef MODULE_OS2TOOLS_TOOLS
#define MODULE_OS2TOOLS_TOOLS

#ifdef __cplusplus
      extern "C" {
#endif


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#ifdef __OS2__
  #define INCL_DOSFILEMGR
  #define INCL_BASE
  #define INCL_NOPMAPI
  #include <os2.h>

  #define OS2READSIZE 61440
#endif

#ifdef _WIN32
  #include <windows.h>
#endif


#include "tooltypes.h"                                   /* type definitions */


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/
        
#pragma pack(1)
        
/* old, backlevel structure */
typedef struct _TOOLAPPVERSION1
{
  UCHAR ucStructureVersion;               /* structure number of this record */
  PSZ   pszName;                                  /* name of the application */
  ULONG ulVersion;        /* version of the application 0x00010203 - 1.02.03 */
  PSZ   pszRemark;                       /* special remark line, can be NULL */
  PSZ   pszBuild;                                /* this describes the build */
  PSZ   pszCopyright;                                  /* the copyright line */
} TOOLAPPVERSION1, *PTOOLAPPVERSION1;


typedef struct _TOOLAPPVERSION
{
  UCHAR ucStructureVersion;               /* structure number of this record */
  PSZ   pszName;                                  /* name of the application */
  ULONG ulVersion;        /* version of the application 0x00010203 - 1.02.03 */
  ULONG ulVersionMin;                 /* minimum required version of the DLL */
  ULONG ulVersionMax;              /* maximum recommended version of the DLL */
  PSZ   pszRemark;                       /* special remark line, can be NULL */
  PSZ   pszBuild;                                /* this describes the build */
  PSZ   pszCopyright;                                  /* the copyright line */
} TOOLAPPVERSION, *PTOOLAPPVERSION;


typedef struct             /* structure for high resolution time measurement */
{
  double fSeconds;
} PERFSTRUCT, *PPERFSTRUCT;


/* Data structure for MD5 (Message-Digest) computation */
typedef struct 
{
  ULONG i[2];                           /* number of _bits_ handled mod 2^64 */
  ULONG buf[4];                                            /* scratch buffer */
  UCHAR in[64];                                              /* input buffer */
  UCHAR digest[16];                     /* actual digest after MD5Final call */
} MD5_CTX, *PMD5_CTX;

#pragma pack()


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

ULONG  TOOLAPI ToolsVersion(void);         /* returns version of this library */

PSZ    TOOLAPI ToolsRevision(void);   /* returns build string of this library */

                             /* print the title of the application to stdout */
APIRET TOOLAPI ToolsPrintTitle (PTOOLAPPVERSION pToolAppVersion);

APIRET TOOLAPI ToolsErrorDos (APIRET rc);    /* print message from OSO001.MSG */

APIRET TOOLAPI ToolsErrorDosEx (APIRET rc,   /* print error message + message */
                               PSZ    pszMessage);

APIRET TOOLAPI ToolsPerfQuery(PERFSTRUCT *t);          /* performance counter */

APIRET TOOLAPI ToolsDumpHex   (ULONG ulPosition,          /* hexdump a buffer */
                              ULONG ulLength,
                              PVOID pSource);

APIRET TOOLAPI ToolsDumpAscii (ULONG ulPosition,        /* asciidump a buffer */
                              ULONG ulLength,
                              PVOID pSource);

APIRET TOOLAPI ToolsReadFileToBuffer (PSZ    pszFile,   /* read a file to mem */
                                     PPVOID ppBuffer,
                                     PULONG pulBuffer);

int    TOOLAPI ToolsConfirmationQuery (void); /* query confirmation from user */

ULONG  TOOLAPI ToolsDateToAge (USHORT tag,         /* calculate canonical age */
                              USHORT monat,
                              USHORT jahr);

int    TOOLAPI ToolsDateCompare (FDATE fdate1,     /* compares two file dates */
                                FTIME ftime1,
                                FDATE fdate2,
                                FTIME ftime2);

APIRET TOOLAPI ToolsCrc32String (PSZ    pszText,           /* calculate CRC32 */
                                PULONG pulCRC);

APIRET TOOLAPI ToolsCrc32Buffer (PSZ    pszText,           /* calculate CRC32 */
                                 ULONG  ulBufferLength,
                                 PULONG pulCRC,
                                 PULONG pulCRCCurrent);

APIRET TOOLAPI ToolsMD5Initialize (PMD5_CTX pmdContext);          /* MD5 init */

APIRET TOOLAPI ToolsMD5Update     (PMD5_CTX pmdContext,           /* MD5 upd. */
                                  PSZ      pszInBuffer,
                                  ULONG    ulInBufferLength);

APIRET TOOLAPI ToolsMD5Finalize   (PMD5_CTX pmdContext);        /* MD5 final. */



APIRET TOOLAPI StrTrim (PSZ pszSource);                     /* clean a string */

void   TOOLAPI StrValueToSize (PSZ   pszString,        /* transform to string */
                              ULONG ulValue);

void   TOOLAPI StrValueToSizeFloat (PSZ   pszString,   /* transform to string */
                                    float fValue);

PSZ    TOOLAPI StrFDateTimeToString    (FDATE fDate,    /* datetime to string */
                                       FTIME fTime,
                                       PSZ   szBuffer);

PSZ    TOOLAPI StrFAttrToString (ULONG fAttr,  /* convert file attributes str */
                                PSZ   szBuffer);

PSZ    TOOLAPI StrFAttrToStringShort(ULONG fAttr,          /* dto. short form */
                                    PSZ   szBuffer);

ULONG  TOOLAPI StrToNumber(PSZ  pszString,      /* convert a string to number */
                          BOOL fSigned);

APIRET TOOLAPI StrInsert  (      PSZ pszPos,  /* insert string into other str */
                          const PSZ pszInsert);

APIRET TOOLAPI StrDelete  (PSZ   pszString,  /* delete characters from string */
                          ULONG ulPosition,
                          ULONG ulNumber);

APIRET TOOLAPI StrSearchReplace (PPSZ   ppszBuffer,       /* search'n'replace */
                                PULONG pulBufferSize,
                                PSZ    pszOriginal,
                                PSZ    pszReplacement);

APIRET TOOLAPI StrReplace (PSZ   pszDestination,                   /* replace */
                          PSZ   pszInsert,
                          ULONG ulPosition);

APIRET TOOLAPI StrRealcase (PSZ pszString);            /* convert string case */


/*****************************************************************************
 * Additional macros                                                         *
 *****************************************************************************/

#define __TOOLSAPPVERSIONSTRUCT__ 2      /* current structure version number */
#define TOOLVERSION(a,b,c,d,e,f) { \
  static TOOLAPPVERSION \
  ToolAppVersion = {__TOOLSAPPVERSIONSTRUCT__,\
                   a,b,c,d,e,\
                   __TOOLSREVISION__,\
                   f}; \
  ToolsPrintTitle(&ToolAppVersion); }


/*****************************************************************************
 * Defines and constants                                                     *
 *****************************************************************************/


/*****************************************************************************
 * Error Codes                                                               *
 *****************************************************************************/


#ifdef __cplusplus
      }
#endif


#endif /* MODULE_OS2TOOLS_TOOLS */

