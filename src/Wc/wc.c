/***********************************************************************
 * Name      : WC
 * Funktion  : Building a dictionary over several text files
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:25:17]
 ***********************************************************************/

 /* REMARK:
  - stdin wieder einbauen
  - algorithmus anpassen, damit ergebnisse mit standard WC uebereinstimmen

  */

//#define DEBUGPRINT(a,b) fprintf(stdout,"\nDEBUG:" a " (%u)",b);
#define DEBUGPRINT(a,b)


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#ifdef __OS2__
  #define INCL_DOSFILEMGR
  #define INCL_BASE
  #define INCL_NOPMAPI
  #include <os2.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <io.h>
#include <process.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"


#define MAXPATHLEN 260


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct _Report
{
  ULONG ulChars;                         /* number of characters in the file */
  ULONG ulWords;                              /* number of words in the file */
  ULONG ulLines;                              /* number of lines in the file */
} REPORT, *PREPORT;


typedef struct _IOBuffer
{
  ULONG ulSizeInitial;                       /* the total size of the buffer */
  ULONG ulSizeWork;                         /* the "used" area of the buffer */
  PSZ   pszBuffer;                                  /* pointer to the buffer */
  UCHAR fProcessed;                                  /* buffer was processed */
  UCHAR fRead;                           /* buffer is filled with valid data */
  UCHAR fLast;                                      /* last buffer of a file */
  PSZ   pszFile;      /* the filename, only valid on the last buffer read in */
  HMTX  hmtxLocked;                     /* locking semaphore for this buffer */
} IOBUFFER, *PIOBUFFER, **PPIOBUFFER;


typedef struct _BufferArray
{
  ULONG     ulBuffers;                                  /* number of buffers */
  HEV       hevBufferRead;               /* posted whenever a buffer is read */
  HEV       hevBufferProcessed; /* posted whenever a buffer became processed */
  TID       tidWorker;                     /* thread ID of the worker thread */
  BOOL      fTerminate;                /* flag to set for thread termination */
  PIOBUFFER pIOBuffer;                        /* pointer to the buffer array */
} BUFFERARRAY, *PBUFFERARRAY, **PPBUFFERARRAY;



typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsFileInput;                 /* user specified the input file name */
  ARGFLAG fsFileOutput;               /* user specified the output file name */
  ARGFLAG fsRecursive;             /* recursion through the subdirectories ? */
  ARGFLAG fsFind;                  /* for compatibility with old filesystems */
  ARGFLAG fsBuffers;               /* number of I/O buffers for the thread   */
  ARGFLAG fsBufferSize;            /* size per I/O buffer for the thread     */
  ARGFLAG fsVerbose;                                       /* verbose output */

  ARGFLAG fsSpeed;        /* perform read operation only, test reading speed */

  PSZ   pszFileInput;                          /* this is the input filename */

  ULONG   ulFindNumberMaximum;                        /* maximum find number */

  ULONG   ulBuffers;                                /* number of I/O buffers */
  ULONG   ulBufferSize;                                   /* size per buffer */
} OPTIONS, *POPTIONS;



typedef struct
{
  ULONG ulFileSize;                                /* total size of the file */
  PSZ   pszFileBuffer;        /* points to the memory allocated for the file */
  ULONG ulFileFindBufferSize;                 /* size of the filefind buffer */
  ULONG ulFilesScanned;                           /* number of files scanned */
  ULONG ulFileMask;                  /* file attribute bits for the filefind */

  PPSZ  parrStrings;              /* pointer to dynamic string pointer array */
  ULONG ulStrings;                             /* number of string in buffer */
  ULONG ulStringsCurrent;                 /* index of current string pointer */

  REPORT ReportTotal;                                   /* the global report */

  ULONG  ulReportFiles;                           /* number of files scanned */

  PBUFFERARRAY pBufferArray;             /* the I/O buffer control structure */

  PERFSTRUCT   psStart;                       /* for performance measurement */
  PERFSTRUCT   psEnd;
  double       dTimeTotal;                   /* total time needed in seconds */
} GLOBALS, *PGLOBALS;


typedef struct _wcprocessdata
{
  PSZ     pszBuffer;
  PREPORT pReport;
  ULONG   ulSize;
  BOOL    fInWord;                                /* we start outside a word */
} WCPROCESSDATA, *PWCPROCESSDATA;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/
OPTIONS Options;                /* this structure holds command line options */
GLOBALS Globals;                /* this structure holds global variables     */


ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung---------------pTarget----------------ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",        NULL,                  ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",        NULL,                  ARG_NULL,       &Options.fsHelp},
  {"/CRAP",      "Compatibility to some Windows 95 requester",
                                            NULL,                  ARG_NULL,       &Options.fsFind},
  {"/V",         "Verbose output.",         NULL,                  ARG_NULL |
                                                                   ARG_HIDDEN,     &Options.fsVerbose},
  {"/VERBOSE",   "Verbose output.",         NULL,                  ARG_NULL,       &Options.fsVerbose},
  {"/SPEED",     "Read data only, no further operation.",
                                            NULL,                  ARG_NULL,       &Options.fsSpeed},
  {"/S",         "Recurse through subdirectories.",
                                            NULL,                  ARG_NULL,       &Options.fsRecursive},
  {"/BUFFERS=",  "Number of I/O buffers. (4)",
                                            &Options.ulBuffers,    ARG_ULONG,      &Options.fsBuffers},
  {"/BUFFERSIZE=","Size of I/O buffer. (65535)",
                                            &Options.ulBufferSize, ARG_ULONG,      &Options.fsBufferSize},
  {"1",          "Input file(s).",          &Options.pszFileInput, ARG_PSZ  |
                                                                   ARG_MUST |
                                                                   ARG_DEFAULT,    &Options.fsFileInput},
  ARG_TERMINATE
};


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/
void   help              (void);

int    main              (int,
                          char **);

void   WCProcess         (PWCPROCESSDATA pData);

APIRET WCProcessFiles    (PSZ pszFile);

void   ThdWCProcess      (PVOID pParameters);

void   ThdWCProcessSpeed (PVOID pParameters);


/***********************************************************************
 * Name      : void help
 * Funktion  : Anzeigen des Hilfetextes
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:43:33]
 ***********************************************************************/

void help (void)
{
  TOOLVERSION("WordCount",                               /* application name */
              0x00010206,                            /* application revision */
              0x00010100,           /* minimum required revision of PHSTOOLS */
              0x0001FFFF,        /* maximum recommended revision of PHSTOOLS */
              NULL,                                                /* Remark */
              NULL);                                 /* additional copyright */
}


/*****************************************************************************
 * Name      : APIRET WCProcess
 * Funktion  : build pointer list
 * Parameter :
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

/* set to 1 if characters is a word delimiter */
static const UCHAR tabWordCharacters[] =
{
  /* 0x00 .. 0x0f */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /* 0x10 .. 0x1f */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /* 0x20 .. 0x2f */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /* 0x30 .. 0x3f */ 0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,
  /* 0x40 .. 0x4f */ 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* 0x50 .. 0x5f */ 0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,
  /* 0x60 .. 0x6f */ 1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  /* 0x70 .. 0x7f */ 0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,
  /* 0x80 .. 0x8f */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /* 0x90 .. 0x9f */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /* 0xa0 .. 0xaf */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /* 0xb0 .. 0xbf */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /* 0xc0 .. 0xcf */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /* 0xd0 .. 0xdf */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /* 0xe0 .. 0xef */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  /* 0xf0 .. 0xff */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};


void _Optlink WCProcess (PWCPROCESSDATA pData)
{
  register ULONG ulLocalCount;
  register PSZ   pszLocalTemp;
  register BOOL  fInWord = pData->fInWord;
  register ULONG ulLocalWords = 0;

  pData->pReport->ulChars += pData->ulSize;   /* number of characters in the buffer */

  for (ulLocalCount = pData->ulSize,
       pszLocalTemp = pData->pszBuffer;

       ulLocalCount;

       ulLocalCount--,
       pszLocalTemp++)
  {
                                   /* check if character determinates a word */
    if (tabWordCharacters[*pszLocalTemp])
    {
      if (*pszLocalTemp == 0x0a)                                 /* LINEFEED */
        pData->pReport->ulLines++;
                                                                /* word scan */

      if (fInWord)                         /* if we have been outside a word */
        fInWord = FALSE;                             /* still outside a word */
    }
    else
    {
      if(!fInWord)
      {
        ulLocalWords++;                                   /* count this word */
        fInWord = TRUE;                       /* then we start a new one now */
      }
    }
  }

  pData->fInWord           = fInWord;                     /* pass back value */
  pData->pReport->ulWords += ulLocalWords;
}


/*****************************************************************************
 * Name      : APIRET BufferManAllocate
 * Funktion  : allocates and prepares the I/O buffers
 * Parameter : ULONG ulSize   - size per buffer
 *             ULONG ulNumber - number of buffers
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

APIRET BufferManAllocate(ULONG         ulSize,
                         ULONG         ulNumber,
                         PPBUFFERARRAY ppBuffers)
{
  PBUFFERARRAY pBufferArray;                     /* pointer to control array */
  PIOBUFFER    pIOBuffer;          /* pointer to IOBuffer controls structure */
  HMTX         hMutex;               /* handle to mutial exclusion semaphore */
  ULONG        ulCount;                        /* the loop counting variable */
  APIRET       rc;                                         /* API returncode */
  HEV   hevBufferRead;                   /* posted whenever a buffer is read */
  HEV   hevBufferProcessed;     /* posted whenever a buffer became processed */

  if ( (ulSize    == 0) ||                               /* check parameters */
       (ulNumber  == 0) ||
       (ppBuffers == NULL) )
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */


  rc = DosCreateEventSem(NULL,                    /* unnamed event semaphore */
                         &hevBufferRead,
                         0,                                    /* non shared */
                         0);                          /* initial state RESET */
  if (rc != NO_ERROR)                                        /* check errors */
    return (rc);                                    /* raise error condition */

  rc = DosCreateEventSem(NULL,                    /* unnamed event semaphore */
                         &hevBufferProcessed,
                         0,                                    /* non shared */
                         0);                          /* initial state RESET */
  if (rc != NO_ERROR)                                        /* check errors */
  {
    DosCloseEventSem(hevBufferRead);                 /* free event semaphore */
    return (rc);                                    /* raise error condition */
  }


  pBufferArray = (PBUFFERARRAY)malloc(sizeof(BUFFERARRAY));
  if (pBufferArray == NULL)                   /* check for proper allocation */
  {
    DosCloseEventSem(hevBufferRead);                 /* free event semaphore */
    DosCloseEventSem(hevBufferProcessed);            /* free event semaphore */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */
  }

  pBufferArray->ulBuffers = ulNumber;        /* put in the number of buffers */


  pIOBuffer = (PIOBUFFER)malloc(sizeof(IOBUFFER) * ulNumber);
  if (pIOBuffer == NULL)                      /* check for proper allocation */
  {
    DosCloseEventSem(hevBufferRead);                 /* free event semaphore */
    DosCloseEventSem(hevBufferProcessed);            /* free event semaphore */
    free(pBufferArray);                  /* free previously allocated memory */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */
  }

  pBufferArray->pIOBuffer          = pIOBuffer;        /* put it the pointer */
  pBufferArray->hevBufferRead      = hevBufferRead; /* the event sem handles */
  pBufferArray->hevBufferProcessed = hevBufferProcessed;

  for (ulCount = 0;
       ulCount < ulNumber;
       ulCount++)
  {
    rc = DosCreateMutexSem(NULL,                        /* unnamed semaphore */
                           &hMutex,
                           0,                                       /* flags */
                           0);                              /* unowned state */

    if (rc == NO_ERROR)                 /* if semaphore was created properly */
      pIOBuffer[ulCount].pszBuffer = (PSZ)malloc(ulSize);   /* allocate buf. */

    if (pIOBuffer[ulCount].pszBuffer == NULL)            /* check allocation */
      rc = ERROR_NOT_ENOUGH_MEMORY;                   /* set the return code */

    if (rc != NO_ERROR)                              /* check for all errors */
    {
      ULONG ulTemp;                         /* temporary, local loop counter */

      for (ulTemp = 0;            /* deallocate previously allocated buffers */
           ulTemp < ulCount;
           ulTemp++)
      {
        DosCloseMutexSem(pIOBuffer[ulTemp].hmtxLocked);    /* free semaphore */
        free(pIOBuffer[ulTemp].pszBuffer);         /* free that memory again */
      }

      free(pIOBuffer);                              /* free the array itself */
      free(pBufferArray);                       /* and the control structure */

      DosCloseMutexSem(hMutex);               /* release the mutex semaphore */

      DosCloseEventSem(hevBufferRead);               /* free event semaphore */
      DosCloseEventSem(hevBufferProcessed);          /* free event semaphore */

      return (rc);                                  /* raise error condition */
    }

    pIOBuffer[ulCount].ulSizeInitial = ulSize;
    pIOBuffer[ulCount].ulSizeWork    = 0;
    pIOBuffer[ulCount].fProcessed    = FALSE;
    pIOBuffer[ulCount].fRead         = FALSE;
    pIOBuffer[ulCount].hmtxLocked    = hMutex;   /* and the semaphore handle */
  }

  *ppBuffers = pBufferArray;                        /* pass back the pointer */

  return (NO_ERROR);                                            /* that's it */
}


/*****************************************************************************
 * Name      : APIRET BufferManFree
 * Funktion  : frees all the I/O buffers
 * Parameter : PBUFFERARRAY pBuffers
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

APIRET BufferManFree(PBUFFERARRAY pBuffers)
{
  ULONG        ulCount;                        /* the loop counting variable */

  if (pBuffers == NULL)                                  /* check parameters */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */

  for (ulCount = 0;                          /* loop over all the IO buffers */
       ulCount < pBuffers->ulBuffers;
       ulCount++)
  {
    DosCloseMutexSem(pBuffers->pIOBuffer[ulCount].hmtxLocked);       /* sema */
    free(pBuffers->pIOBuffer[ulCount].pszBuffer);       /* and the structure */
  }

  DosCloseEventSem(pBuffers->hevBufferRead);         /* free event semaphore */
  DosCloseEventSem(pBuffers->hevBufferProcessed);    /* free event semaphore */

  free (pBuffers->pIOBuffer);                     /* and the array structure */
  free (pBuffers);                              /* and the control structure */

  return (NO_ERROR);                                            /* that's it */
}


/*****************************************************************************
 * Name      : void _stdcall ThdWCProcess
 * Funktion  : a worker thread for asynchronous processing of the buffers
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

void ThdWCProcess (PVOID pParameters)
{
  PBUFFERARRAY pBuffer = (PBUFFERARRAY)pParameters;
  APIRET       rc;                                         /* API returncode */
  ULONG        ulPostCounter;           /* the post counter of the semaphore */
  ULONG        ulCount = 0;                    /* counts the processed bytes */
  PIOBUFFER    pIOBuffer = pBuffer->pIOBuffer; /* pointer to list of buffers */
  WCPROCESSDATA wcData;                                    /* Data structure */
  REPORT       Report;                         /* result reporting structure */

  Report.ulChars = 0;
  Report.ulWords = 0;
  Report.ulLines = 0;

  wcData.pReport        = &Report;
  wcData.ulSize         = pIOBuffer->ulSizeWork;
  wcData.pszBuffer      = pIOBuffer->pszBuffer;
  wcData.fInWord        = FALSE;


  while (pBuffer->fTerminate == FALSE)
  {
    DEBUGPRINT("Thd: Waiting for new buffer",0);
    rc = DosWaitEventSem(pBuffer->hevBufferRead,    /* wait for a new buffer */
                         SEM_INDEFINITE_WAIT);
    if (rc != NO_ERROR)                                  /* check for errors */
      ToolsErrorDosEx(rc,                             /* yield error message */
                      "Thd::DosWaitEventSem(1)");

    rc = DosResetEventSem(pBuffer->hevBufferRead,     /* reset the semaphore */
                          &ulPostCounter);
    if (rc != NO_ERROR)                                  /* check for errors */
      ToolsErrorDosEx(rc,                             /* yield error message */
                      "Thd::DosResetEventSem(1)");

    /* find out the recently read buffer(s) and process them */
    for (ulCount   = 0,
         pIOBuffer = pBuffer->pIOBuffer;

         ulCount < pBuffer->ulBuffers;

         ulCount++,
         pIOBuffer++)
    {
      if ( (pIOBuffer->fRead      == TRUE) &&          /* search the buffers */
           (pIOBuffer->fProcessed == FALSE) )
      {
        DEBUGPRINT("Thd: Waiting to lock the buffer",ulCount);

        rc = DosRequestMutexSem(pIOBuffer->hmtxLocked,
                                SEM_INDEFINITE_WAIT);
        if (rc != NO_ERROR)                              /* check for errors */
          ToolsErrorDosEx(rc,
                          "Thd::DosRequestMutexSem(1)");

        DEBUGPRINT("Thd: processing buffer",ulCount);


        wcData.pszBuffer = pIOBuffer->pszBuffer;
        wcData.ulSize    = pIOBuffer->ulSizeWork;
        WCProcess(&wcData);                           /* process that buffer */

        pIOBuffer->fRead      = FALSE;           /* switch the buffer status */
        pIOBuffer->fProcessed = TRUE;

        if (pIOBuffer->fLast == TRUE)
        {
          printf ("%8u %8u %8u %s \n",                   /* print the report */
            Report.ulChars,
            Report.ulWords,
            Report.ulLines,
            pIOBuffer->pszFile);
                                                 /* update global statistics */
          Globals.ReportTotal.ulChars += Report.ulChars;
          Globals.ReportTotal.ulWords += Report.ulWords;
          Globals.ReportTotal.ulLines += Report.ulLines;
          Globals.ulReportFiles++;                      /* register the file */

          Report.ulChars = 0;              /* reset structure for next files */
          Report.ulWords = 0;
          Report.ulLines = 0;

          if (pIOBuffer->pszFile != NULL) /* properly allcated ? then free ! */
            free (pIOBuffer->pszFile);

          pIOBuffer->fLast   = FALSE;
          pIOBuffer->pszFile = NULL;
        }
        else
          if (Options.fsVerbose)
            printf ("%8u %8u %8u %s \r",                 /* print the report */
              Report.ulChars,
              Report.ulWords,
              Report.ulLines,
              pIOBuffer->pszFile);



        DEBUGPRINT("Thd: unlocking buffer",ulCount);
        rc = DosReleaseMutexSem(pIOBuffer->hmtxLocked); /* release the mutex */
        if (rc != NO_ERROR)                              /* check for errors */
          ToolsErrorDosEx(rc,
                          "Thd::DosReleaseMutexSem(1)");

        DEBUGPRINT("Thd: signal reader thd",0);
        rc = DosPostEventSem(pBuffer->hevBufferProcessed);         /* signal */
        if ( (rc != NO_ERROR) &&                         /* check for errors */
             (rc != ERROR_ALREADY_POSTED) )
          ToolsErrorDosEx(rc,
                          "Thd::DosPostEventSem(1)");

      }
    }
  }

  DEBUGPRINT("Thd: quitting",0);
}


/*****************************************************************************
 * Name      : void _stdcall ThdWCProcessSpeed
 * Funktion  : a worker thread for asynchronous processing of the buffers
 *             this is a dummy version to measure maximum speed
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 1998/07/07 14.53.13]
 *****************************************************************************/

void ThdWCProcessSpeed (PVOID pParameters)
{
  PBUFFERARRAY pBuffer = (PBUFFERARRAY)pParameters;
  APIRET       rc;                                         /* API returncode */
  ULONG        ulPostCounter;           /* the post counter of the semaphore */
  ULONG        ulCount = 0;                    /* counts the processed bytes */
  PIOBUFFER    pIOBuffer = pBuffer->pIOBuffer; /* pointer to list of buffers */
  WCPROCESSDATA wcData;                                    /* Data structure */
  REPORT       Report;                         /* result reporting structure */

  Report.ulChars = 0;
  Report.ulWords = 0;
  Report.ulLines = 0;

  wcData.pReport        = &Report;
  wcData.ulSize         = pIOBuffer->ulSizeWork;
  wcData.pszBuffer      = pIOBuffer->pszBuffer;
  wcData.fInWord        = FALSE;


  while (pBuffer->fTerminate == FALSE)
  {
    DEBUGPRINT("Thd: Waiting for new buffer",0);
    rc = DosWaitEventSem(pBuffer->hevBufferRead,    /* wait for a new buffer */
                         SEM_INDEFINITE_WAIT);
    if (rc != NO_ERROR)                                  /* check for errors */
      ToolsErrorDosEx(rc,                             /* yield error message */
                      "Thd::DosWaitEventSem(1)");

    rc = DosResetEventSem(pBuffer->hevBufferRead,     /* reset the semaphore */
                          &ulPostCounter);
    if (rc != NO_ERROR)                                  /* check for errors */
      ToolsErrorDosEx(rc,                             /* yield error message */
                      "Thd::DosResetEventSem(1)");

    /* find out the recently read buffer(s) and process them */
    for (ulCount   = 0,
         pIOBuffer = pBuffer->pIOBuffer;

         ulCount < pBuffer->ulBuffers;

         ulCount++,
         pIOBuffer++)
    {
      if ( (pIOBuffer->fRead      == TRUE) &&          /* search the buffers */
           (pIOBuffer->fProcessed == FALSE) )
      {
        DEBUGPRINT("Thd: Waiting to lock the buffer",ulCount);

        rc = DosRequestMutexSem(pIOBuffer->hmtxLocked,
                                SEM_INDEFINITE_WAIT);
        if (rc != NO_ERROR)                              /* check for errors */
          ToolsErrorDosEx(rc,
                          "Thd::DosRequestMutexSem(1)");

        DEBUGPRINT("Thd: processing buffer",ulCount);


        wcData.pszBuffer = pIOBuffer->pszBuffer;
        wcData.ulSize    = pIOBuffer->ulSizeWork;

        /* dummy statistics instead of real processing */
        wcData.pReport->ulChars += wcData.ulSize;
        wcData.pReport->ulLines = 0;
        wcData.pReport->ulWords = 0;

        pIOBuffer->fRead      = FALSE;           /* switch the buffer status */
        pIOBuffer->fProcessed = TRUE;

        if (pIOBuffer->fLast == TRUE)
        {
          printf ("%8u %8u %8u %s \n",                   /* print the report */
            Report.ulChars,
            Report.ulWords,
            Report.ulLines,
            pIOBuffer->pszFile);
                                                 /* update global statistics */
          Globals.ReportTotal.ulChars += Report.ulChars;
          Globals.ReportTotal.ulWords += Report.ulWords;
          Globals.ReportTotal.ulLines += Report.ulLines;
          Globals.ulReportFiles++;                      /* register the file */

          Report.ulChars = 0;              /* reset structure for next files */
          Report.ulWords = 0;
          Report.ulLines = 0;

          if (pIOBuffer->pszFile != NULL) /* properly allcated ? then free ! */
            free (pIOBuffer->pszFile);

          pIOBuffer->fLast   = FALSE;
          pIOBuffer->pszFile = NULL;
        }
        else
          if (Options.fsVerbose)
            printf ("%8u %8u %8u %s \r",                 /* print the report */
              Report.ulChars,
              Report.ulWords,
              Report.ulLines,
              pIOBuffer->pszFile);



        DEBUGPRINT("Thd: unlocking buffer",ulCount);
        rc = DosReleaseMutexSem(pIOBuffer->hmtxLocked); /* release the mutex */
        if (rc != NO_ERROR)                              /* check for errors */
          ToolsErrorDosEx(rc,
                          "Thd::DosReleaseMutexSem(1)");

        DEBUGPRINT("Thd: signal reader thd",0);
        rc = DosPostEventSem(pBuffer->hevBufferProcessed);         /* signal */
        if ( (rc != NO_ERROR) &&                         /* check for errors */
             (rc != ERROR_ALREADY_POSTED) )
          ToolsErrorDosEx(rc,
                          "Thd::DosPostEventSem(1)");

      }
    }
  }

  DEBUGPRINT("Thd: quitting",0);
}


/*****************************************************************************
 * Name      : APIRET WCProcessMTWait
 * Funktion  : Wait for the thread to complete pending buffers
 * Parameter :
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

APIRET WCProcessMTWait (PBUFFERARRAY pBufferArray)
{
  ULONG     ulCount;                                /* counters for the loop */
  ULONG     ulTemp;
  PIOBUFFER pIOBuffer;                      /* pointer to the current buffer */
  APIRET    rc;                                            /* API returncode */
  ULONG     ulPostCounter;                         /* semaphore post counter */

  if (pBufferArray == NULL)                              /* check parameters */
    return (ERROR_INVALID_PARAMETER);               /* raise error condition */

  pIOBuffer = pBufferArray->pIOBuffer;

  do
  {
    ulCount = 0;
    DEBUGPRINT("Work: Waiting for thread to complete all.",0);
    for (ulTemp = 0,
         pIOBuffer = pBufferArray->pIOBuffer;

         ulTemp < pBufferArray->ulBuffers;

         ulTemp++,
         pIOBuffer++)
    {
      if ( (pIOBuffer->fRead      == TRUE) &&   /* read but not yet processed*/
           (pIOBuffer->fProcessed == FALSE) )
        ulCount++;                                 /* then count this buffer */
    }

    if (ulCount != 0)                         /* if no free buffer was found */
    {
      DEBUGPRINT("Work: waiting for thread to complete work.",ulTemp);
      rc = DosWaitEventSem(pBufferArray->hevBufferProcessed,
                           SEM_INDEFINITE_WAIT);
      if (rc != NO_ERROR)
        ToolsErrorDosEx(rc,
                        "Work::DosWaitEventSem(3)");

      rc = DosResetEventSem(pBufferArray->hevBufferProcessed,
                            &ulPostCounter);
      if (rc != NO_ERROR)
        ToolsErrorDosEx(rc,
                        "Work::DosWaitEventSem(3)");
    }
  }
  while (ulCount != 0);              /* until last buffer has been processed */

  return (NO_ERROR);                                                   /* OK */
}


/*****************************************************************************
 * Name      : APIRET WCProcessMT
 * Funktion  : build pointer list
 * Parameter :
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

APIRET WCProcessFileMT (PBUFFERARRAY pBufferArray,
                        HFILE        hFile,
                        PSZ          pszFile,
                        ULONG        ulSize)
{
  ULONG         ulBuffersFree;             /* counter for free buffers found */
  ULONG         ulTemp;                 /* loop counter over the file buffer */
  ULONG         ulCount;                /* loop counter over the file buffer */
  ULONG         ulCountBytes;           /* loop counter over the file buffer */
  APIRET        rc;                                        /* API returncode */
  ULONG         ulPostCounter;          /* the post counter of the semaphore */
  PIOBUFFER     pIOBuffer;                 /* pointer to the list of buffers */


  /***************************************************************************
   * now read the file in chunks and post/reset the semaphores               *
   ***************************************************************************/

  rc           = NO_ERROR;
  ulCountBytes = 0;

  while ( (rc == NO_ERROR) &&         /* end is reached when error occurs or */
          (ulCountBytes < ulSize) )   /* file buffers processed              */
  {
    ulBuffersFree = 0;                     /* counter for free buffers found */

    for (ulCount = 0,
         pIOBuffer = pBufferArray->pIOBuffer;

         (ulCount < pBufferArray->ulBuffers) &&
         (ulCountBytes < ulSize);

         ulCount++,
         pIOBuffer++)
    {
                                /* OK, find a free buffer and read some data */
      if (pIOBuffer->fRead == FALSE)
      {
        DEBUGPRINT("Work: locking buffer",ulCount);
        rc = DosRequestMutexSem(pIOBuffer->hmtxLocked,    /* lock the buffer */
                                SEM_INDEFINITE_WAIT);
        if (rc != NO_ERROR)                              /* check for errors */
          ToolsErrorDosEx(rc,                             /* exit processing */
                          "Work::DosRequestMutexSem");


        DEBUGPRINT("Work: Reading buffer",ulCount);
        rc = DosRead(hFile,                               /* fill the buffer */
                     pIOBuffer->pszBuffer,
                     pIOBuffer->ulSizeInitial,
                     &pIOBuffer->ulSizeWork);
        if (rc != NO_ERROR)                              /* check for errors */
          ToolsErrorDosEx(rc,                             /* exit processing */
                          pszFile);

        ulCountBytes += pIOBuffer->ulSizeWork;             /* adjust counter */


        pIOBuffer->fRead      = TRUE;         /* OK initialize buffer status */
        pIOBuffer->fProcessed = FALSE;

        if (ulCountBytes == ulSize)             /* last buffer of the file ? */
        {
          pIOBuffer->fLast = TRUE;       /* indicate this is the last buffer */
          pIOBuffer->pszFile = strdup(pszFile);         /* copy the filename */
        }

        DEBUGPRINT("Work: unlocking buffer",ulCount);
        rc = DosReleaseMutexSem(pIOBuffer->hmtxLocked); /* unlock the buffer */
        if (rc != NO_ERROR)                              /* check for errors */
          ToolsErrorDosEx(rc,                             /* exit processing */
                          "Work::DosReleaseMutexSem");


        DEBUGPRINT("Work: announcing new buffer",0);
        rc = DosPostEventSem(pBufferArray->hevBufferRead);       /* post sem */
        if ( (rc != NO_ERROR)  &&                        /* check for errors */
             (rc != ERROR_ALREADY_POSTED) )
          ToolsErrorDosEx(rc,
                          "Work::DosPostEventSem(1)");
        else
          rc = NO_ERROR;                            /* ignore ALREADY POSTED */

        if (ulCountBytes == ulSize)             /* if this file is completed */
          goto _wait_for_thread;                 /* then leave the procedure */


        ulBuffersFree++;                    /* say we've found a free buffer */
      }
    }

                /* if all buffers are filled, wait for semaphore from thread */
    if (ulBuffersFree == 0)                        /* no free buffer found ? */
    {
      DEBUGPRINT("Waiting for free buffer",0);
      rc = DosWaitEventSem(pBufferArray->hevBufferProcessed, /* wait for thd */
                           SEM_INDEFINITE_WAIT);
      if (rc != NO_ERROR)
        ToolsErrorDosEx(rc,
                        "Work::DosWaitEventSem(2)");

      rc = DosResetEventSem(pBufferArray->hevBufferProcessed, /* wait for thd */
                            &ulPostCounter);
      if (rc != NO_ERROR)
        ToolsErrorDosEx(rc,
                        "Work::DosResetEventSem(2)");

    }
  }

           /* check if all buffers have been processed and skip to next file */
  _wait_for_thread:

  /* WCProcessMTWait */

  DEBUGPRINT("Work: OK, that's it",0);
  return (rc);                                     /* pass thru return value */
}


/*****************************************************************************
 * Name      : APIRET WCProcessFile
 * Funktion  : Map file into memory
 * Parameter : PSZ pszFile
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

APIRET WCProcessFile (PBUFFERARRAY pBufferArray,
                      PSZ          pszFile,
                      ULONG        ulSize)
{
  APIRET      rc;                                         /* API return code */
  HFILE       hFileInput;                   /* file handle of the input file */
  ULONG       ulAction;                  /* action code from the DosOpen API */

#ifdef DEBUG
  fprintf (stderr,
           "\nWCProcessFile(%s)",
           pszFile);
#endif


  if ( (pszFile      == NULL)  ||                        /* check parameters */
       (pBufferArray == NULL) )
    return (ERROR_INVALID_PARAMETER);                   /* return error code */

  if (ulSize != 0)                                       /* zero-sized files */
  {
    rc = DosOpen(pszFile,                                  /* File path name */
                 &hFileInput,                                 /* File handle */
                 &ulAction,                                  /* Action taken */
                 0L,                              /* File primary allocation */
                 FILE_ARCHIVED |
                 FILE_NORMAL,                              /* File attribute */
                 OPEN_ACTION_FAIL_IF_NEW |
                 OPEN_ACTION_OPEN_IF_EXISTS,           /* Open function type */
                 OPEN_FLAGS_NOINHERIT |
                 OPEN_FLAGS_SEQUENTIAL|
                 OPEN_SHARE_DENYNONE  |
                 OPEN_ACCESS_READONLY,              /* Open mode of the file */
                 0L);                               /* No extended attribute */
    if (rc != NO_ERROR)
    {
      ToolsErrorDosEx(rc,                                /* check for errors */
                      pszFile);
      return (rc);                             /* abort function immediately */
    }

    WCProcessFileMT(pBufferArray,
                    hFileInput,
                    pszFile,
                    ulSize);                          /* call cwlc functions */

    rc = DosClose(hFileInput);                       /* close the input file */
  }
  else
    rc = NO_ERROR;

  return (rc);                                             /* signal success */
}


/******************************************************************************
 * Name      : APIRET ProcessScanSpeed
 * Funktion  : Scan directories ad max. speed
 * Parameter : PSZ pszPath      - the current path
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:48:00]
 ******************************************************************************/

APIRET WCProcessScan (PSZ pszPath,
                       PSZ pszWildcard)
{
  APIRET          rc;
  HDIR            hDirectory = HDIR_CREATE;
  FILEFINDBUF3    *pFileInfo;
  PSZ             pFileFindBuffer;
  PSZ             pszPathNext;
  PSZ             pszTemp;                       /* temporary string pointer */
  ULONG           ulFindCount;
  ULONG           ulPathLength;

#ifdef DEBUG
  fprintf (stderr,
           "\nWCProcessScan(%s,%s)",
           pszPath,
           pszWildcard);
#endif

  pFileFindBuffer = (PSZ)malloc(Globals.ulFileFindBufferSize);
  if (pFileFindBuffer == NULL)              /* check if allocation succeeded */
    return(ERROR_NOT_ENOUGH_MEMORY);

  ulPathLength = strlen(pszPath);                 /* OK, get the origin path */
  if (ulPathLength > MAXPATHLEN)
  {
    free (pFileFindBuffer);              /* free previously allocated memory */
    return(ERROR_FILENAME_EXCED_RANGE);      /* Bail out, subdir is too long */
  }

  pszPathNext = (PSZ)malloc(MAXPATHLEN);         /* for copying the pathname */
  if (pszPathNext == NULL)                                /* out of memory ? */
  {
    free (pFileFindBuffer);              /* free previously allocated memory */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */
  }

  memmove ((PSZ)pszPathNext,                         /* copy the path string */
           (PSZ)pszPath,
           ulPathLength);

  pszTemp = pszPathNext + ulPathLength;           /* calculate this position */

  if (( pszPathNext[ulPathLength-1] == '\\' )|| /* ignore trailing backslash */
      ( pszPathNext[ulPathLength-1] == '/' ))   /* ignore trailing slash     */
  {
    ulPathLength--;
    pszPath[ulPathLength] = 0;                         /* cut trailing slash */
  }

                                                       /* "\*" -> 0x00002a5b */
  *( (PSHORT)pszTemp ) = 0x005c;                               /* append "\" */
  strcat (pszTemp,                    /* and concatenate the wildcard string */
          pszWildcard);

                                                           /* OK, los geht's */
  ulFindCount    = Options.ulFindNumberMaximum;

  pFileInfo = (FILEFINDBUF3 *)pFileFindBuffer;

  rc = DosFindFirst(pszPathNext,
                    &hDirectory,
                    Globals.ulFileMask,
                    pFileInfo,
                    Globals.ulFileFindBufferSize,
                    &ulFindCount,
                    FIL_STANDARD);
  if ( rc != NO_ERROR)                          /* check for error condition */
    return (rc);                                        /* return error code */

  do
  {
    Globals.ulFilesScanned += ulFindCount;

    while (ulFindCount)
    {
      strcpy (pszPathNext,                            /* build the full path */
              pszPath);

      ulPathLength = strlen(pszPathNext);        /* query the strings length */
                                                /* ignore trailing backslash */
      * ( (PUSHORT)(pszPathNext + ulPathLength) ) = 0x005c;             /* \ */

      strcat (pszPathNext,                                /* append new name */
              pFileInfo->achName);


      if ( !(pFileInfo->attrFile & FILE_DIRECTORY))           /* ignore dirs */
      {
        rc = WCProcessFile(Globals.pBufferArray,
                           pszPathNext,             /* then proceed as usual */
                           pFileInfo->cbFile);
        if (rc != NO_ERROR)
            ToolsErrorDosEx(rc,
                            pszPathNext);
      }

      ulFindCount--;
      pFileInfo = (FILEFINDBUF3 *) ((BYTE*)pFileInfo + pFileInfo->oNextEntryOffset);
    }

    ulFindCount = Options.ulFindNumberMaximum;
    pFileInfo = (FILEFINDBUF3 *)pFileFindBuffer;
    rc = DosFindNext (hDirectory,
                      pFileInfo,
                      Globals.ulFileFindBufferSize,
                      &ulFindCount);
  }
  while (rc == NO_ERROR);

  free((void *)pFileFindBuffer);
  free((void *)pszPathNext);

  return(DosFindClose(hDirectory));
}


/******************************************************************************
 * Name      : APIRET WCProcessScanDir
 * Funktion  : Scan directories ad max. speed
 * Parameter : PSZ pszPath      - the current path
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:48:00]
 ******************************************************************************/

APIRET WCProcessScanDir (PSZ pszPath,
                          PSZ pszWildcard)
{
  APIRET          rc;
  HDIR            hDirectory = HDIR_CREATE;
  FILEFINDBUF3    *pFileInfo;
  PSZ             pFileFindBuffer;
  PSZ             pszPathNext;
  PSZ             pszTemp;                       /* temporary string pointer */
  ULONG           ulFindCount;
  ULONG           ulPathLength;

#ifdef DEBUG
  fprintf (stderr,
           "\nWCProcessScanDir(%s,%s)",
           pszPath,
           pszWildcard);
#endif

  rc = WCProcessScan(pszPath,                   /* scan the directory first */
                      pszWildcard);

  pFileFindBuffer = (PSZ)malloc(Globals.ulFileFindBufferSize);
  if (pFileFindBuffer == NULL)              /* check if allocation succeeded */
    return(ERROR_NOT_ENOUGH_MEMORY);

  ulPathLength = strlen(pszPath);                 /* OK, get the origin path */
  if (ulPathLength > MAXPATHLEN)
  {
    free (pFileFindBuffer);              /* free previously allocated memory */
    return(ERROR_FILENAME_EXCED_RANGE);      /* Bail out, subdir is too long */
  }

  pszPathNext = (PSZ)malloc(MAXPATHLEN);         /* for copying the pathname */
  if (pszPathNext == NULL)                                /* out of memory ? */
  {
    free (pFileFindBuffer);              /* free previously allocated memory */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */
  }

  memmove ((PSZ)pszPathNext,                         /* copy the path string */
           (PSZ)pszPath,
           ulPathLength);

  pszTemp = pszPathNext + ulPathLength;           /* calculate this position */

  if (( pszPathNext[ulPathLength-1] == '\\' )|| /* ignore trailing backslash */
      ( pszPathNext[ulPathLength-1] == '/' ))   /* ignore trailing slash     */
  {
    ulPathLength--;
    pszPath[ulPathLength] = 0;                         /* cut trailing slash */
  }

                                                       /* "\*" -> 0x00002a5b */
  *( (PULONG)pszTemp ) = 0x00002a5c;                          /* append "\*" */

                                                           /* OK, los geht's */
  ulFindCount    = Options.ulFindNumberMaximum;

  pFileInfo = (FILEFINDBUF3 *)pFileFindBuffer;

  rc = DosFindFirst(pszPathNext,
                    &hDirectory,
                    MUST_HAVE_DIRECTORY,
                    pFileInfo,
                    Globals.ulFileFindBufferSize,
                    &ulFindCount,
                    FIL_STANDARD);
  if ( (rc != NO_ERROR) &&                      /* check for error condition */
       (rc != ERROR_NO_MORE_FILES) )
    return (rc);                                        /* return error code */

  do
  {
    Globals.ulFilesScanned += ulFindCount;

    while (ulFindCount)
    {
      strcpy (pszPathNext,                            /* build the full path */
              pszPath);

      ulPathLength = strlen(pszPathNext);        /* query the strings length */
                                                /* ignore trailing backslash */
      * ( (PUSHORT)(pszPathNext + ulPathLength) ) = 0x005c;             /* \ */

      strcat (pszPathNext,                                /* append new name */
              pFileInfo->achName);

      if ((pFileInfo->achName[0] == '.') &&               /* ignore . and .. */
          (( !pFileInfo->achName[1] ||
          ((pFileInfo->achName[1] == '.') && !pFileInfo->achName[2])) ));
      else
      {
        rc = WCProcessScanDir(pszPathNext,
                               pszWildcard);                    /* recursion */
        if (rc != NO_ERROR)
            ToolsErrorDosEx(rc,
                            pszPathNext);
      }

      ulFindCount--;
      pFileInfo = (FILEFINDBUF3 *) ((BYTE*)pFileInfo + pFileInfo->oNextEntryOffset);
    }

    ulFindCount = Options.ulFindNumberMaximum;
    pFileInfo = (FILEFINDBUF3 *)pFileFindBuffer;
    rc = DosFindNext (hDirectory,
                      pFileInfo,
                      Globals.ulFileFindBufferSize,
                      &ulFindCount);
  }
  while (rc == NO_ERROR);

  free((void *)pFileFindBuffer);
  free((void *)pszPathNext);

  return(DosFindClose(hDirectory));
}


/*****************************************************************************
 * Name      : APIRET WCProcessFilesMain
 * Funktion  : Separate all the filenames
 * Parameter : PSZ pszFile
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

APIRET WCProcessFilesMain (PSZ pszFile)
{
  APIRET rc;                                              /* API return code */

  rc = BufferManAllocate(Options.ulBufferSize,  /* initialize buffer manager */
                         Options.ulBuffers,
                         &Globals.pBufferArray);
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                                    /* raise error condition */

  Globals.pBufferArray->fTerminate = FALSE;             /* rest of the setup */

  if (Options.fsSpeed)
  {
    /* measure reading speed only */

    printf ("Measuring reading speed only, won't count words and lines.\n");

  #ifdef __BORLANDC__
    Globals.pBufferArray->tidWorker  = _beginthread(ThdWCProcessSpeed,
                                                    16384,
                                                    (PVOID)Globals.pBufferArray);
  #endif

  #ifdef __IBMC__
    Globals.pBufferArray->tidWorker  = _beginthread(ThdWCProcessSpeed,
                                                    NULL,
                                                    16384,
                                                    (PVOID)Globals.pBufferArray);
  #endif

  }
  else
  {
    /* do real processing */
  #ifdef __BORLANDC__
    Globals.pBufferArray->tidWorker  = _beginthread(ThdWCProcess,
                                                    16384,
                                                    (PVOID)Globals.pBufferArray);
  #endif

  #ifdef __IBMC__
    Globals.pBufferArray->tidWorker  = _beginthread(ThdWCProcess,
                                                    NULL,
                                                    16384,
                                                    (PVOID)Globals.pBufferArray);
  #endif
  }

  rc = WCProcessFiles(pszFile);                     /* call client procedure */

  BufferManFree(Globals.pBufferArray);      /* close down the buffer manager */

  return (rc);                                /* return error code to caller */
}

/*****************************************************************************
 * Name      : APIRET WCProcessFiles
 * Funktion  : Separate all the filenames
 * Parameter : PSZ pszFile
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

APIRET WCProcessFiles (PSZ pszFile)
{
  APIRET      rc;                                         /* API return code */
  PSZ         pszTemp;                           /* temporary string pointer */
  PSZ         pszTemp2;                          /* temporary string pointer */
  FILESTATUS3 fs3;          /* filestatus 3 structure for the given filename */
  PSZ         pszWildcard = "*";   /* points to wildcard section of filename */
  CHAR        szRoot[MAXPATHLEN];     /* buffer for qualified root path name */
  ULONG       ulTemp;                      /* temporary calculation variable */


  if (pszFile == NULL)                                   /* check parameters */
    return (ERROR_INVALID_PARAMETER);                   /* return error code */

#ifdef DEBUG
  fprintf (stderr,
           "\nWCProcessFiles(%s)",
           pszFile);
#endif

  /* 1. loop -> tokenizer */
  pszTemp = strtok(pszFile,                 /* tokenize the string by commas */
                   ",");
  while (pszTemp)
  {
    /* 2. loop -> scan wildcards */
    rc = DosQueryPathInfo(pszFile,             /* if file is a normal "FILE" */
                          FIL_STANDARD,/* then don't try to expand wildcards */
                          &fs3,
                          sizeof(fs3));

#ifdef DEBUG
  fprintf (stderr,
           "\nDosQueryPathInfo(%s) = #%u",
           pszFile,
           rc);
#endif

    if (rc != NO_ERROR)
    {
                                            /* query root path and file part */
                     /* scan the last part of the filename for the file only */
      pszWildcard = strrchr(pszTemp,'\\');    /* point to last part of fname */
      pszTemp2 = strrchr(pszTemp,'/');       /* scan for alternate delimiter */

      if (pszTemp2 > pszWildcard)                 /* take tha last character */
        pszWildcard = pszTemp2;

      if (pszWildcard == NULL)                 /* still no delimiter found ? */
        pszWildcard = strrchr(pszTemp,':');       /* then the drive letter ? */

      if (pszWildcard == NULL)   /* if still no path part found, assume file */
      {
        pszWildcard = pszTemp;      /* then the remaining part is wildcard ! */
        pszTemp = ".";
      }
      else
      {
        ulTemp = pszWildcard - pszTemp + 1;       /* calculate string length */

        strncpy (szRoot,                               /* copy the path part */
                 pszTemp,
                 ulTemp);
        szRoot[ulTemp] = 0;                          /* terminate the string */
        pszTemp = szRoot;                  /* and set pointer to root buffer */

        pszWildcard++;                             /* skip to next character */
        if (*pszWildcard == 0)        /* check if string termination reached */
          pszWildcard = "*";                 /* then assume default wildcard */
      }

                                       /* cut forbidden trailing backslashes */
      ulTemp = strlen(pszTemp) - 1;        /* note: strlen(pszTemp) != 0 !!! */
      if (pszTemp[ulTemp-1] != ':')                 /* I HATE DRIVELETTERS ! */
        if ( ( (pszTemp[ulTemp] == '\\') ||
               (pszTemp[ulTemp] == '/' ) ) &&
             (ulTemp > 0) )
          pszTemp[ulTemp] = 0;                         /* cut this backslash */

      /* 2. loop -> scan wildcards */
      rc = DosQueryPathInfo(pszTemp,           /* if file is a normal "FILE" */
                            FIL_STANDARD,   /* then don't try expand wildcrds*/
                            &fs3,
                            sizeof(fs3));

#ifdef DEBUG
    fprintf (stderr,
           "\nDosQueryPathInfo2(%s) = #%u, pszWildcard=%s",
           pszTemp,
           rc,
           pszWildcard);
#endif

      if (rc != NO_ERROR)                                /* check for errors */
      {
        ToolsErrorDosEx(rc,                           /* yield error message */
                        pszTemp);                               /* and abort */
        return (rc);
      }
    }

    if (fs3.attrFile & FILE_DIRECTORY)                  /* is it a directory */
    {
      if (Options.fsRecursive)
        rc = WCProcessScanDir(pszTemp,       /* recursively scan directories */
                               pszWildcard);
      else
        rc = WCProcessScan(pszTemp,                                  /* path */
                           pszWildcard);
    }
    else
      rc = WCProcessFile(Globals.pBufferArray,
                         pszTemp,
                         fs3.cbFile);                      /* this file only */

    if (rc != NO_ERROR)                                  /* check for errors */
      ToolsErrorDosEx(rc,                             /* yield error message */
                      "WCProcessFiles");

    pszTemp = strtok(NULL,                  /* tokenize the string by commas */
                     ",");
  }

                 /* wait for the asynchronous thread to complete all buffers */
  rc = WCProcessMTWait(Globals.pBufferArray);
  if (rc != NO_ERROR)                                    /* check for errors */
    ToolsErrorDosEx(rc,                               /* yield error message */
                    "WCProcessFiles::WCProcessMTWait");

  return (NO_ERROR);                                                   /* OK */
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
  int rc;                                                    /* RÅckgabewert */

  memset (&Options,                      /* initialize the global structures */
          0,
          sizeof(Options));

  memset (&Globals,
          0,
          sizeof(Globals));


  rc = ArgStandard (argc,                            /* CLI-Parameter parsen */
                    argv,
                    TabArguments,
                    &Options.fsHelp);
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    exit(1);                                                /* abort program */
  }

  if (Options.fsHelp)                                  /* user requests help */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  /***************************************************************************
   * Map arguments                                                           *
   ***************************************************************************/

  if (!Options.fsFind)           /* user did not specify maximum find number */
    Options.ulFindNumberMaximum = 65535 / sizeof(FILEFINDBUF3);
  else
    Options.ulFindNumberMaximum = 1;        /* for some Windows 95 requester */
 /* OS/2 2.x, 3.x provide by far best performance when avoiding 64k switches */

  Globals.ulFileFindBufferSize = sizeof(FILEFINDBUF3) * /* our target struct */
                                 Options.ulFindNumberMaximum;

  Globals.ulFileMask = FILE_NORMAL    |
                       FILE_READONLY;

  if ( (!Options.fsBuffers) || (Options.ulBuffers == 0) )
    Options.ulBuffers = 4;

  if ( (!Options.fsBufferSize) || (Options.ulBufferSize == 0) )
    Options.ulBufferSize = 65535;



  ToolsPerfQuery(&Globals.psStart);                          /* benchmarking */

  rc = WCProcessFilesMain(Options.pszFileInput);             /* process them */

  ToolsPerfQuery(&Globals.psEnd);                            /* benchmarking */

  if (rc == NO_ERROR)
  {
    Globals.dTimeTotal = Globals.psEnd.fSeconds - Globals.psStart.fSeconds;

    printf ("\n\n%8u %8u %8u (all %u files)",
            Globals.ReportTotal.ulChars,
            Globals.ReportTotal.ulWords,
            Globals.ReportTotal.ulLines,
            Globals.ulReportFiles);

    printf ("\n\n%10.3f chars/s %10.3f words/s    total %10.3f sec"
            "\n%10.3f lines/s %10.3f files/s",
            Globals.ReportTotal.ulChars / Globals.dTimeTotal,
            Globals.ReportTotal.ulWords / Globals.dTimeTotal,
            Globals.dTimeTotal,
            Globals.ReportTotal.ulLines / Globals.dTimeTotal,
            Globals.ulReportFiles / Globals.dTimeTotal);
  }
  else
    ToolsErrorDos(rc);                                /* yield error message */

  return (rc);
}
