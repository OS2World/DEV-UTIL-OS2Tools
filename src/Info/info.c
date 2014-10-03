/*****************************************************
 * INFO Tool.                                        *
 * Reports details on a given file.                  *
 * (c) 1994-95 Patrick Haller Systemtechnik          *
 *****************************************************/

/* #define DEBUG 1 */

/* TODO:
    - DASD Info (maybe) DosOpen(OPEN_FLAGS_DASD)
    - EAs bei Directories (EAUTIL debuggen)
    - SFT (system file table) auswerten
    - CRC-32 calculation is defective (depends on read chunk size?)
    - via DOsFSCtl() auf HPFS FNode-Nummern etc. ermitteln
    */

/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#define INCL_DOSFILEMGR
#define INCL_BASE
#define INCL_NOPMAPI
#include <os2.h>

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <process.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"

#define MAXPATHLEN 260


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  USHORT usType;
  PSZ    pszType;
} EATYPE, *PEATYPE;

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsVerbose;                          /* provide more verbose output */
  ARGFLAG fsDumpEA;                                    /* dump values of EAs */
  ARGFLAG fsFile;          /* indicates file was specified from command line */
  ARGFLAG fsRead;  /* indicates file shall be read completely with benchmark */
  ARGFLAG fsReadNumber;           /* number of reads requested was specified */
  ARGFLAG fsReadSize;                   /* maximum buffer size was specified */
  ARGFLAG fsCache;          /* specified whether file shall be cached or not */
  ARGFLAG fsMD5;                 /* calculate MD5 checksum when reading file */
  ARGFLAG fsCRC32;             /* calculate CRC32 checksum when reading file */

  PSZ  pszFile;                          /* name of the file to be processed */
  ULONG ulReadNumber;
  ULONG ulReadSize;
} OPTIONS, *POPTIONS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

EATYPE TabEATypes[] =
{
  {EAT_BINARY,  "length preceeded binary"},
  {EAT_ASCII,   "length preceeded ASCII"},
  {EAT_BITMAP,  "length preceeded bitmap"},
  {EAT_METAFILE,"length preceeded metafile"},
  {EAT_ICON,    "length preceeded icon"},
  {EAT_EA,      "ASCII (include file)"},
  {EAT_MVMT,    "multi-valued, multi-typed"},
  {EAT_MVST,    "multi-valued, single-typed"},
  {EAT_ASN1,    "ASN.1 field"},
  {0,           NULL}
};

OPTIONS Options;                /* this structure holds command line options */

ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung----------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/EA",        "Dump EA values.",    NULL,                 ARG_NULL,       &Options.fsDumpEA},
  {"/?",         "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/V",         "Verbose output.",    NULL,                 ARG_NULL,       &Options.fsVerbose},
  {"/MD5",       "Calculate MD5 when reading file (/READ).", NULL,   ARG_NULL, &Options.fsMD5},
  {"/CRC32",     "Calculate CRC32 when reading file (/READ).", NULL, ARG_NULL, &Options.fsCRC32},
  {"/READ.NR=",  "Number of reads.",   &Options.ulReadNumber,ARG_ULONG,      &Options.fsReadNumber},
  {"/READ.SIZE=","Readbuffer size.",   &Options.ulReadSize  ,ARG_ULONG,      &Options.fsReadSize},
  {"/READ.CACHE","Use cache for xfer.",NULL,                 ARG_NULL,       &Options.fsCache},
  {"/READ",      "Benchmark reading.", NULL,                 ARG_NULL,       &Options.fsRead},
  {"1",          "Filename.",          &Options.pszFile,     ARG_PSZ |
                                                             ARG_DEFAULT |
                                                             ARG_MUST,       &Options.fsFile},
  ARG_TERMINATE
};



/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void   help                (void);

void   initialize          (void);

int    display_information (PSZ);

APIRET display_directory   (PSZ          pszDirectory);

int    display_info_path_EA (PSZ          pszDirectory,
                             PFILESTATUS4 pfs4);

APIRET display_speedtest   (HFILE        hInfoFile,
                            PFILESTATUS4 pfs4);

int    main                (int,
                            char **);

APIRET CheckLock           (PSZ pszFilename);


/***********************************************************************
 * Name      : void help
 * Funktion  : Anzeigen des Hilfetextes.
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.45.24]
 ***********************************************************************/

void help (void)
{
  TOOLVERSION("Info",                                   /* application name */
              0x00010006,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


/***********************************************************************
 * Name      : APIRET CheckLock
 * Funktion  : Ist die Datei durch einen anderen Prozess gelockt ?
 * Parameter : PSZ pszFilename
 * Variablen :
 * Ergebnis  : Ausgabe auf dem Bildschirm
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 02.11.1995 10.49.57]
 ***********************************************************************/

APIRET CheckLock (PSZ pszFilename)
{
  APIRET rc;                                      /* R갷kgabeparameter */

  if (pszFilename == NULL)                     /* Parameter갶erpr갽ung */
    return (ERROR_INVALID_PARAMETER);

  rc = DosMove (pszFilename,pszFilename);    /* Umbenennen/Verschieben */

  printf ("\n   Locking                : ");
  if (rc == NO_ERROR)                  /* Keine Fehler wurden gemeldet */
    printf ("not locked.");
  else
  {
    /* Ist die Datei nun gelockt ? */
    if (rc == ERROR_SHARING_VIOLATION)
      printf ("locked (used by another process).");
  }

  return (rc);
}

/***********************************************************************
 * Name      : PSZ QueryAttributeType
 * Funktion  : Returns pointer to descriptive string on attribute type
 * Parameter : USHORT usType
 * Variablen :
 * Ergebnis  : Pointer to the string
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.49.57]
 ***********************************************************************/


PSZ QueryAttributeType (USHORT usSearchType)
{
  static PSZ pszUserDefined = "<unknown: user defined>";
  static PSZ pszReserved    = "<unknown: reserved>";
  PEATYPE pEAType;

                                                      /* search in the table */
  for (pEAType = TabEATypes;
       pEAType->pszType != NULL;
       pEAType++)
  {
    if (pEAType->usType == usSearchType)    /* if value is found, return ptr */
      return (pEAType->pszType);
  }

                     /* if we get here, then the EAType was not recognized ! */
  if (usSearchType <= 0x7fff)      /* if type is user defined, return ptr to */
    return (pszUserDefined);                                  /* static text */
  else
    return (pszReserved);
}


/***********************************************************************
 * Name      : void display_info_path_EA
 * Funktion  : Anzeigen der gefundenen Directory-EA-Detailinformation
 * Parameter : PSZ pszDirectory
 * Variablen :
 * Ergebnis  : Ausgabe auf dem Bildschirm
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.49.57]
 ***********************************************************************/

int display_info_path_EA (PSZ          pszDirectory,
                         PFILESTATUS4 pfs4)
{
  int   rc;                                                /* API-Returncode */
  PVOID  pEABuffer;                            /* buffer to retrieve EA data */
  ULONG  ulEnumCount = 0xFFFFFFFF;                /* number of EAs requested */
  PDENA2 pEAInfo;                                      /* pointer to EA Info */
  PSZ    pTemp;                          /* temporary pointer for conversion */
  ULONG ulCounter;                                 /* temporary loop counter */

  EAOP2     eaEAOP2;             /* query structure for requesting EA values */
  PGEA2LIST pGEAList;
  PGEA2     pGEAIterator;
  PFEA2LIST pFEAList;
  PFEA2     pFEAIterator;

  PSZ       pszAttributeType;   /* pointer to string defining attribute type */
  USHORT    usSearchType;                /* numeric type of the EA attribute */


  if (pszDirectory == NULL)                              /* check parameters */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */


  pEABuffer = malloc(pfs4->cbList);                       /* allocate buffer */
  if (pEABuffer == NULL)             /* check if allocation succeeded or not */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */

  pGEAList = malloc(pfs4->cbList);                        /* allocate buffer */
  if (pGEAList == NULL)              /* check if allocation succeeded or not */
  {
    free (pEABuffer);                    /* free previously allocated memory */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */
  }

  pFEAList = malloc(pfs4->cbList);                        /* allocate buffer */
  if (pFEAList == NULL)              /* check if allocation succeeded or not */
  {
    free (pEABuffer);                    /* free previously allocated memory */
    free (pGEAList);
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */
  }

  rc = DosEnumAttribute(ENUMEA_REFTYPE_PATH,
                        pszDirectory,
                        1,
                        pEABuffer,
                        pfs4->cbList,
                        &ulEnumCount,
                        ENUMEA_LEVEL_NO_VALUE);
  if ( (rc != NO_ERROR) ||                         /* check if error occured */
       (ulEnumCount == 0) )
  {
    free(pEABuffer);                     /* free previously allocated memory */
    free(pGEAList),
    free(pFEAList);
    return (rc);                                             /* return value */
  }


  printf ("\n컴횱xtended Attributes컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�");

  for (ulCounter = 1,
       pEAInfo   = pEABuffer;                   /* start iteration on buffer */

       ulCounter <= ulEnumCount;

       ulCounter++)
  {
    printf ("\n   %2u %-32s: %5u bytes",
            ulCounter,
            pEAInfo->szName,
            pEAInfo->cbValue);

    pTemp = (PSZ)pEAInfo;
    pTemp += pEAInfo->oNextEntryOffset;
    pEAInfo = (PDENA2)pTemp;
  }


  if (Options.fsDumpEA)
  {
              /* now request the EA value from the list via DosQueryFileInfo */
                                   /* first build a valid GEA2LIST structure */
    pGEAList->cbList = pfs4->cbList;        /* total length of the structure */
    for (ulCounter = 1,
         pEAInfo   = pEABuffer,                 /* start iteration on buffer */
         pGEAIterator = &pGEAList->list[0];

         ulCounter <= ulEnumCount;

         ulCounter++)
    {
              /* calculates the value for oNextEntryOffset from pEAInfo list */
      if (pEAInfo->oNextEntryOffset != 0)
      {
        pGEAIterator->oNextEntryOffset = pEAInfo->oNextEntryOffset - 3;
        pGEAIterator->oNextEntryOffset &= 0xFFFFFFFc;           /* alignment */
      }
      else
        pGEAIterator->oNextEntryOffset = 0;

      pGEAIterator->cbName=pEAInfo->cbName;
      memcpy (pGEAIterator->szName,                             /* copy name */
              pEAInfo->szName,
              pEAInfo->cbName + 1);

      pTemp = (PSZ)pEAInfo;
      pTemp += pEAInfo->oNextEntryOffset;
      pEAInfo = (PDENA2)pTemp;

      pTemp = (PSZ)pGEAIterator;
      pTemp += pGEAIterator->oNextEntryOffset;
      pGEAIterator = (PGEA2)pTemp;
    }

                                     /* now setup the header in the FEA2LIST */
    pFEAList->cbList = pfs4->cbList;               /* length of total buffer */
    eaEAOP2.fpGEA2List = pGEAList;
    eaEAOP2.fpFEA2List = pFEAList;

    rc = DosQueryPathInfo (pszDirectory,
                           FIL_QUERYEASFROMLIST,
                           &eaEAOP2,
                           sizeof(eaEAOP2));
    if (rc == NO_ERROR)                /* check if we succeeded on last call */
    {
      printf ("\n  Values:");

                                           /* now process retrieved FEA2LIST */
      for (ulCounter = 1,
           pFEAIterator = &pFEAList->list[0];

           ulCounter <= ulEnumCount;

           ulCounter++)
      {
      /* the first two bytes in the value array define what type of attribute*/
        pTemp = (PSZ)pFEAIterator + pFEAIterator->cbName + 9;

        usSearchType = *( (PUSHORT)pTemp );
        pszAttributeType = QueryAttributeType (usSearchType);

        printf ("\n   %2u %-32s: %5u bytes, %s",
                ulCounter,
                pFEAIterator->szName,
                pFEAIterator->cbValue,
                pszAttributeType);

                                                                /* hexdump ! */
        ToolsDumpHex (pFEAIterator->cbValue,
                      pFEAIterator->cbValue,
                      pTemp);

        pTemp = (PSZ)pFEAIterator;
        pTemp += pFEAIterator->oNextEntryOffset;
        pFEAIterator = (PFEA2)pTemp;
      }
    }
  }

  free(pEABuffer);                       /* free previously allocated memory */
  free(pGEAList);
  free(pFEAList);

  return (NO_ERROR);                                       /* return success */
}


/***************************************************************************
 * Name      : void display_speedtest
 * Funktion  : read the whole file, benchmark it and check for proper
 *             readability
 * Parameter :
 * Variablen :
 * Ergebnis  : API-Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.49.57]
 ***************************************************************************/

APIRET display_speedtest(HFILE        hInfoFile,
                         PFILESTATUS4 pfs4)
{
  PERFSTRUCT TS_Start;                  /* timestruct : start of benchmark */
  PERFSTRUCT TS_End;                    /* timestruct : end   of benchmark */
  float      fSeconds;                /* elapsed seconds for the benchmark */
  ULONG      ulBufferSize;                      /* size of the read buffer */
  ULONG      ulCounter;                          /* temporary loop counter */
  ULONG      ulBufferRead = 0;  /* number of times the files is to be read */
  ULONG      ulBytesTotal;                  /* counter of total read bytes */
  PVOID      pBuffer;                      /* pointer to the target buffer */
  APIRET     rc;                                         /* API returncode */
  ULONG      ulBytesRead;                          /* number of read bytes */
  ULONG      ulDummy;                                       /* dummy ULONG */
  CHAR       szSize[16];                     /* buffer for the size string */

  PMD5_CTX   pctxMD5 = NULL;           /* MD5 checksum calculation context */
  ULONG      ulCRC32 = 0xFFFFFFFF;   /* CRC32 checksum calculation context */

  if (pfs4 == NULL)                                    /* check parameters */
    return (ERROR_INVALID_PARAMETER);             /* raise error condition */

  /* setup checksum contexts if desired */
  if (Options.fsMD5)
  {
      pctxMD5 = (PMD5_CTX) malloc( sizeof (MD5_CTX) );

      rc = ToolsMD5Initialize(pctxMD5);              /* initialize context */
      if (rc != NO_ERROR)                              /* check for errors */
          return rc;                                /* bail out with error */
  }


  if (Options.fsReadSize)                           /* allocate buffer ... */
    ulBufferSize = Options.ulReadSize;            /* use this if specified */
  else
    ulBufferSize = 1024 * 1024;                   /* default buffer is 1MB */

  if (pfs4->cbFile == 0)                                 /* 0 byte size file */
  {
    printf ("\n  File has no content, can't read from it.");
    return NO_ERROR;                                                   /* OK */
  }

  if (pfs4->cbFile < ulBufferSize)                   /* check for filesize */
    ulBufferSize = pfs4->cbFile;          /* use smallest necessary buffer */


  if (Options.fsReadNumber)          /* check if buffer read was specified */
    ulBufferRead = Options.ulReadNumber;
  else
    ulBufferRead = 1;                                           /* default */


  printf ("\n컴훂ransfer컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴");

  rc = DosAllocMem (&pBuffer,               /* pointer to the base address */
                    ulBufferSize,                           /* buffer size */
                    OBJ_TILE   |
                    PAG_COMMIT |                             /* page flags */
                    PAG_READ   |
                    PAG_WRITE);
  if (rc != NO_ERROR)                                  /* check for errors */
  {
    ToolsErrorDos(rc);                                /* print error message */
    return (rc);
  }

                                   /* opt: number of reads, size of buffer */
  printf ("\n   Buffer size     :  %12ub   Number of reads    : %6u",
          ulBufferSize,
          ulBufferRead);


  ulBytesRead  = 1;                                      /* initialization */
  ulBytesTotal = 0;
  rc           = NO_ERROR;

                                                        /* start benchmark */
  ToolsPerfQuery (&TS_Start);                     /* exact time measurement */

  for (ulCounter = 0;                         /* run as often as requested */
       (ulCounter < ulBufferRead) &&
       (rc == NO_ERROR);
       ulCounter++)
  {
    rc = DosSetFilePtr (hInfoFile,                          /* file handle */
                        0,                                     /* distance */
                        FILE_BEGIN,                          /* from where */
                        &ulDummy);                                /* dummy */
    ulBytesRead = 1;                                        /* initializer */


    if (Options.fsVerbose)
    {
      StrValueToSize(szSize,                        /* calculate size string */
                     ulBytesTotal);
      ToolsPerfQuery (&TS_End);                    /* exact time measurement */

      fSeconds = TS_End.fSeconds - TS_Start.fSeconds;  /* calculate duration */
      if (fSeconds == 0)                         /* prevent division by zero */
        fSeconds = 0.01;

      printf ("\nRead : %12s %4.1f%%, Time : %12.4fs,  Average : %12.4fkb/s",
              szSize,
              100.0 * (float)ulBytesTotal / (float)pfs4->cbFile,
              fSeconds,
              (float)ulBytesTotal / fSeconds / 1024.0);
    }


                                                  /* stop if done or error */
    while ( (rc == NO_ERROR) && /* as long as there has not been a problem */
            (ulBytesRead != 0) )
    {
      rc = DosRead (hInfoFile,                                /* read file */
                    pBuffer,
                    ulBufferSize,
                    &ulBytesRead);

      if (rc == NO_ERROR)
      {
          ulBytesTotal += ulBytesRead;                  /* update read counter */


          /* check if checksum calculation was desired */
          /* Note: we need at least one read byte ...  */
          if (ulBytesRead > 0)
          {
            if ( (Options.fsMD5) && (pctxMD5 != NULL) )
            {
                rc = ToolsMD5Update (pctxMD5,
                                     pBuffer,
                                     ulBytesRead);
                if (rc != NO_ERROR)
                    ToolsErrorDosEx(rc,
                                    "Calculation of MD5 message digest");
            }
  
            if (Options.fsCRC32)
            {
                rc = ToolsCrc32Buffer(pBuffer,
                                      ulBytesRead,
                                      &ulCRC32,
                                      &ulCRC32);
                if (rc != NO_ERROR)
                    ToolsErrorDosEx(rc,
                                    "Calculation of CRC32 checksum");
            }
          }
      }

      if (Options.fsVerbose)
      {
        ToolsPerfQuery (&TS_End);                  /* exact time measurement */
        StrValueToSize(szSize,                      /* calculate size string */
                       ulBytesTotal);

        fSeconds = TS_End.fSeconds - TS_Start.fSeconds;/* calculate duration */
        if (fSeconds == 0)                       /* prevent division by zero */
          fSeconds = 0.01;

        printf ("\rRead : %12s %4.1f%%, Time : %12.4fs,  Average : %12.4fkb/s",
                szSize,
                100.0 * (float)ulBytesTotal / (float)pfs4->cbFile,
                fSeconds,
                (float)ulBytesTotal / fSeconds / 1024.0);
      }
    }
  }

  ToolsPerfQuery (&TS_End);                        /* exact time measurement */
  fSeconds = TS_End.fSeconds - TS_Start.fSeconds;      /* calculate duration */

  if (fSeconds == 0)                           /* prevent division by zero */
    fSeconds = 0.01;

  printf ("\n   Time to read    :  %12.4fs   Average Throughput :  %12.4fkb/s"
          "\n   Bytes read      :  %12.4fkb",
          fSeconds,
          (float)ulBytesTotal / fSeconds / 1024.0,
          (float)ulBytesTotal / 1024.0);
  if (rc != NO_ERROR)                       /* check if there was an error */
  {
    printf ("\nError during reading:\n");
    ToolsErrorDos(rc);                                /* print error message */
  }

                         /* print information about the calculated checksums */
  if ( (Options.fsMD5) || (Options.fsCRC32) )
  {
    printf ("\n컴횮hecksums컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�");
  
    if (Options.fsMD5)
    {
        rc = ToolsMD5Finalize(pctxMD5);
        if (rc != NO_ERROR)
            ToolsErrorDosEx(rc,
                            "Finalizing MD5 message digest calculation");
  
        printf("\n   MD5             :");
        for (ulCounter = 0;
             ulCounter < 16;                /* this is sizeof(MD5_CTX.digest); */
             ulCounter++)
        {
            printf (" %02x", pctxMD5->digest[ulCounter]);
        }
    }
  
    if (Options.fsCRC32)
    {
        printf("\n   CRC-32          : %08xh",
               ulCRC32);
    }
  }


  rc = DosFreeMem(pBuffer);
  if (rc != NO_ERROR)                                  /* check for errors */
    ToolsErrorDos(rc);                                /* print error message */

  return (rc);                                    /* indicate we succeeded */
}


/***************************************************************************
 * Name      : void display_directory
 * Funktion  : Anzeigen der gefundenen Directoryinformation
 * Parameter :
 * Variablen :
 * Ergebnis  : Ausgabe auf dem Bildschirm
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.49.57]
 ***************************************************************************/

APIRET display_directory (PSZ pszDirectory)
{
  int   rc;

  char  szFCreation  [40];    /* Buffers for the conversion from time/date */
  char  szFLastWrite [40];
  char  szFLastAccess[40];
  char  szBuffer     [128];
  char  szQualifiedName [MAXPATHLEN];   /* buffer for fully qualified name */

  FILESTATUS4 fs4;       /* Structure to hold the second level information */


  if (pszDirectory == NULL)                        /* Parameter갶erpr갽ung */
    return (ERROR_INVALID_PARAMETER);


  rc = DosQueryPathInfo(pszDirectory,                /* Gather information */
                        FIL_QUERYEASIZE,
                        &fs4,
                        sizeof(fs4));
  if (rc != NO_ERROR)                              /* was there an error ? */
    ToolsErrorDos(rc);
  else
  {
    rc = DosQueryPathInfo(pszDirectory,                 /* query full name */
                          FIL_QUERYFULLNAME,
                          szQualifiedName,
                          sizeof(szQualifiedName));
    if (rc != NO_ERROR)                          /* check if error occured */
      return (rc);                                     /* abort with error */

    printf ("\n컴횳eneral Information컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�"
            "\n   Full name              : %s"
            "\n   Actual file size       : [%12u]"
            "\n   Allocated size         : [%12u]"
            "   --> slack space : [%12i]",
            szQualifiedName,
            fs4.cbFile,
            fs4.cbFileAlloc,
            fs4.cbFileAlloc - fs4.cbFile);

    if ( (fs4.cbFileAlloc < fs4.cbFile) &&
         (fs4.cbFileAlloc != 0) )
      printf ("\n   Compression            : 1:%2.2f",
              (float)fs4.cbFile / (float)fs4.cbFileAlloc);

    if (fs4.cbList > 4)
      printf ("\n   Extended Attribute size: [%12u] (buffer)",
              fs4.cbList >> 1);

    StrFDateTimeToString(fs4.fdateCreation,
                         fs4.ftimeCreation,
                         szFCreation);

    StrFDateTimeToString(fs4.fdateLastWrite,
                         fs4.ftimeLastWrite,
                         szFLastWrite);

    StrFDateTimeToString(fs4.fdateLastAccess,
                         fs4.ftimeLastAccess,
                         szFLastAccess);

    printf ("\n컴횫ccess컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴"
            "\n   Creation               : %s"
            "\n   Last write             : %s"
            "\n   Last access            : %s",
            szFCreation,
            szFLastWrite,
            szFLastAccess
           );

    StrFAttrToString (fs4.attrFile,
                      szBuffer);

    printf ("\n   Attributes             : %s",
            szBuffer);


    /* @@@PH: how many files, speedtest scanning, etc ... */

    rc = display_info_path_EA (pszDirectory,   /* show additional information */
                                &fs4);
    if (rc != NO_ERROR)                            /* check if error occured */
      ToolsErrorDos(rc);
  }

  return (NO_ERROR);                                       /* return success */
}


/***************************************************************************
 * Name      : void display_information
 * Funktion  : Anzeigen der gefundenen Detailinformation
 * Parameter :
 * Variablen :
 * Ergebnis  : Ausgabe auf dem Bildschirm
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.49.57]
 ***************************************************************************/

int display_information (PSZ szFilename)
{
  int   rc;
  HFILE hInfoFile;            /* Filehandle f걊 die zu untersuchende Datei */
  ULONG ulAction;
  ULONG ulFlags;                                 /* flags for DosOpen call */

  char  szFCreation  [40];    /* Buffers for the conversion from time/date */
  char  szFLastWrite [40];
  char  szFLastAccess[40];
  char  szBuffer     [128];
  char  szQualifiedName [MAXPATHLEN];   /* buffer for fully qualified name */

  FILESTATUS4 fs4;       /* Structure to hold the second level information */


  if (szFilename == NULL)                          /* Parameter갶erpr갽ung */
    return (ERROR_INVALID_PARAMETER);


  /* ToDo:

      * Info 갶er Sharing Modes (schon offen, etc.)
      * Info 갶er Directories
      * Info 갶er NPipes
      * Info 갶er DASDs
      * Unterst걎zung v. Wildcards
      * gescheite Formatierung der Ausgabe

      * DosQueryHType
      * NLS-Support

  */


  printf ("\n[%s]",szFilename);


  printf ("\n컴횸ocking컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�");
  rc = CheckLock (szFilename);                      /* Datei gelockt ? */

  /* if (rc == ERROR_SHARING_VIOLATION)
       return */

  ulFlags = OPEN_SHARE_DENYNONE |
            OPEN_ACCESS_READONLY;
  if (Options.fsCache) /* cache or not ? */
  {
    ulFlags |= OPEN_FLAGS_SEQUENTIAL;
  }
  else
  {
    ulFlags |= OPEN_FLAGS_NO_CACHE;
  }

  rc = DosOpen (szFilename,
                &hInfoFile,
                &ulAction,
                0L,                                            /* Filesize */
                0L,                                     /* File attributes */
                OPEN_ACTION_FAIL_IF_NEW |
                OPEN_ACTION_OPEN_IF_EXISTS,
                ulFlags,
                NULL);


  if (rc == NO_ERROR)
  {

    rc = DosQueryFileInfo(hInfoFile,             /* Gather information */
                          FIL_QUERYEASIZE,
                          &fs4,
                          sizeof(fs4));
    if (rc != NO_ERROR)                        /* was there an error ? */
      ToolsErrorDos(rc);
    else
    {
      rc = DosQueryPathInfo(szFilename,                   /* query full name */
                            FIL_QUERYFULLNAME,
                            szQualifiedName,
                            sizeof(szQualifiedName));
      if (rc != NO_ERROR)                          /* check if error occured */
        return (rc);                                     /* abort with error */

      printf ("\n컴횳eneral Information컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�"
              "\n   Full name              : %s"
              "\n   Actual file size       : [%12u]"
              "\n   Allocated size         : [%12u]"
              "   --> slack space : [%12i]",
              szQualifiedName,
              fs4.cbFile,
              fs4.cbFileAlloc,
              fs4.cbFileAlloc - fs4.cbFile);

      if ( (fs4.cbFileAlloc < fs4.cbFile) &&
           (fs4.cbFileAlloc != 0) )
        printf ("\n   Compression            : 1:%2.2f",
                (float)fs4.cbFile / (float)fs4.cbFileAlloc);

      if (fs4.cbList > 4)
        printf ("\n   Extended Attribute size: [%12u] (buffer)",
                fs4.cbList >> 1);

      StrFDateTimeToString(fs4.fdateCreation,
                           fs4.ftimeCreation,
                           szFCreation);

      StrFDateTimeToString(fs4.fdateLastWrite,
                           fs4.ftimeLastWrite,
                           szFLastWrite);

      StrFDateTimeToString(fs4.fdateLastAccess,
                           fs4.ftimeLastAccess,
                           szFLastAccess);

      printf ("\n컴횫ccess컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴"
              "\n   Creation               : %s"
              "\n   Last write             : %s"
              "\n   Last access            : %s",
              szFCreation,
              szFLastWrite,
              szFLastAccess
              );

      StrFAttrToString (fs4.attrFile,szBuffer);
      printf ("\n   Attributes             : %s",
              szBuffer);

      /* show additional information */
      rc = display_info_path_EA (szFilename,
                                 &fs4);
      if (rc != NO_ERROR)                        /* check if error occured */
        ToolsErrorDos(rc);


      if (Options.fsRead)           /* see if we have to run the benchmark */
      {
        rc = display_speedtest(hInfoFile,
                               &fs4);
        if (rc != NO_ERROR)                      /* check if error occured */
          ToolsErrorDos(rc);
      }
    }

    rc = DosClose(hInfoFile);                                /* Close file */
  }
  else
    if (rc == ERROR_ACCESS_DENIED)
    {
                /* if we recieve access denied, there might be a directory */
      rc = display_directory(szFilename);
    }

  return (rc);                                 /* R갷kgabewert liefern */
}


/***********************************************************************
 * Name      : void initialize
 * Funktion  : Initialisierung einiger Variablen
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 ***********************************************************************/

void initialize (void)
{
  memset(&Options,
         0L,
         sizeof(Options));
}


/***********************************************************************
 * Name      : int main
 * Funktion  : Hauptroutine
 * Parameter : int argc, char *argv[]
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 ***********************************************************************/

int main (int argc, char *argv[])
{
  int rc;                                                    /* R갷kgabewert */

  initialize ();                                          /* Initialisierung */

  rc = ArgStandard (argc,                            /* CLI-Parameter parsen */
                    argv,
                    TabArguments,
                    &Options.fsHelp);
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    exit(1);                                                /* abort program */
  }

  if ( (Options.pszFile == NULL) ||          /* check if user specified file */
       (Options.fsHelp) )
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  /* map some options */
  if (Options.fsMD5 || Options.fsCRC32)
  {
      /* then also enable reading and cached reads */
      Options.fsRead = TRUE;
      Options.fsCache = TRUE;
  }


  rc = display_information (Options.pszFile);
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  return (rc);
}
