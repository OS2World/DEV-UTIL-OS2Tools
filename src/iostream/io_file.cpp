/*****************************************************
 * Unix-like NetCat Tool.                            *
 *                                                   *
 * (c) 2002 Patrick Haller                           *
 *****************************************************/

/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#ifdef __OS2__
  #define INCL_DOSMISC
  #define INCL_DOSFILEMGR
  #define INCL_BASE
  #define INCL_NOPMAPI
  #define INCL_DOSDEVIOCTL
  #include <os2.h>
#endif

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <process.h>
#include <ctype.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"


#include "iostream.h"


/*****************************************************************************
 * Externals                                                                 *
 *****************************************************************************/

extern GLOBALS Globals;


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/


IOStreamOutputStdOut::IOStreamOutputStdOut( )
: IOStreamOutput("IOStreamOutputStdOut",
                 "Output stream to standard out",
                 "STDOUT")
{
}


IOStreamOutputStdOut::~IOStreamOutputStdOut( )
{
}


/* setup connection to 'device' */
APIRET IOStreamOutputStdOut::connect( IOStreamInput* in )
{
  setConnected( TRUE );
  return NO_ERROR;
}


/* terminate existing connection to 'device' */
APIRET IOStreamOutputStdOut::disconnect()
{
  setConnected( FALSE );
  return NO_ERROR;
}
  

APIRET IOStreamOutputStdOut::write(IOBuffer *ioBuffer)
{
  ULONG  ulBytesWritten;
  APIRET rc;
  PVOID  pData = ioBuffer->acquire();

  rc = DosWrite(1,
                pData,
                ioBuffer->getValid(),
                    &ulBytesWritten);
  if (ulBytesWritten != ioBuffer->getValid())
    rc = ERROR_WRITE_FAULT;

  ioBuffer->release();

  return rc;
}



IOStreamOutputFile::IOStreamOutputFile( PSZ pszStreamToken )
: IOStreamOutput("IOStreamFile",
                 "Output stream to file",
                 pszStreamToken)
{
  hFileOutput = NULLHANDLE;
  pszFilename = strdup( pszStreamToken );
}
  

IOStreamOutputFile::~IOStreamOutputFile( )
{
  if (NULLHANDLE != hFileOutput)
  {
    Debug(("~IOStreamOutputFile called with open file (%s)!",
           pszFilename))

      DosClose( hFileOutput );
  }

  if (NULL != pszFilename)
    free( pszFilename );
}
  
  
/* setup connection to 'device' */
APIRET IOStreamOutputFile::connect( IOStreamInput* in )
{
  APIRET rc;
  ULONG  ulAction;
  ULONG  ulFlags;

  if (Globals.fsOverwrite)
    ulFlags = OPEN_ACTION_REPLACE_IF_EXISTS |
    OPEN_ACTION_CREATE_IF_NEW;
  else
    ulFlags = OPEN_ACTION_FAIL_IF_EXISTS |
    OPEN_ACTION_CREATE_IF_NEW;

  rc = DosOpen (pszFilename,
                &hFileOutput,
                &ulAction,
                in->estimatedStreamLength(),
                0L,           /* File attributes */
                ulFlags,
                OPEN_SHARE_DENYNONE  |
                OPEN_ACCESS_WRITEONLY |
                OPEN_FLAGS_FAIL_ON_ERROR |
                OPEN_FLAGS_SEQUENTIAL,
                NULL);
  if (NO_ERROR == rc)
    setConnected( TRUE );

  return rc;
}


/* terminate existing connection to 'device' */
APIRET IOStreamOutputFile::disconnect()
{
  APIRET rc = DosClose( hFileOutput );
  if (NO_ERROR == rc)
  {
    hFileOutput = NULLHANDLE;
    setConnected( FALSE );
  }

  return rc;
}
  

APIRET IOStreamOutputFile::write(IOBuffer *ioBuffer)
{
  ULONG  ulBytesWritten;
  APIRET rc;
  PVOID  pData = ioBuffer->acquire();

  rc = DosWrite(hFileOutput,
                pData,
                ioBuffer->getValid(),
                &ulBytesWritten);
  if (ulBytesWritten != ioBuffer->getValid())
    rc = ERROR_WRITE_FAULT;

  ioBuffer->release();

  return rc;
}


IOStreamOutputNull::IOStreamOutputNull( )
    : IOStreamOutput("IOStreamOutputNull",
                     "Output stream to NULL",
                     "OUTNULL")
{
}


IOStreamOutputNull::~IOStreamOutputNull( )
{
}
  
  
/* setup connection to 'device' */
APIRET IOStreamOutputNull::connect( IOStreamInput *in )
{
  setConnected( TRUE );
  return NO_ERROR;
}


/* terminate existing connection to 'device' */
APIRET IOStreamOutputNull::disconnect()
{
  setConnected( FALSE );
  return NO_ERROR;
}
  

APIRET IOStreamOutputNull::write(IOBuffer *ioBuffer)
{
  // just discard the data
  return NO_ERROR;
}


IOStreamOutputDASDLogical::IOStreamOutputDASDLogical( PSZ pszStreamToken )
: IOStreamOutput("IOStreamOutputDASDLogical",
                 "Output stream to logical DASD partition",
                 pszStreamToken)
{
  hDASDOutput = NULLHANDLE;

  // cut off leading "LOG:" signature
  if (0 == strnicmp("LOG:", pszStreamToken, 4))
    pszStreamToken += 4;

  pszPartitionName = strdup( pszStreamToken );
}


IOStreamOutputDASDLogical::~IOStreamOutputDASDLogical( )
{
  if (NULLHANDLE != hDASDOutput)
  {
    Debug(("~IOStreamOutputDASDLogical called with open partition (%s)!",
           pszPartitionName))

      DosClose( hDASDOutput );
  }

  if (NULL != pszPartitionName)
    free( pszPartitionName );
}


/* setup connection to 'device' */
APIRET IOStreamOutputDASDLogical::connect( IOStreamInput* in )
{
  APIRET rc;
  ULONG  ulAction;

  rc = DosOpen (pszPartitionName,
                &hDASDOutput,
                &ulAction,
                in->estimatedStreamLength(),
                FILE_NORMAL,           /* File attributes */
                FILE_OPEN,
                OPEN_SHARE_DENYNONE  |
                OPEN_ACCESS_WRITEONLY |
                OPEN_FLAGS_DASD |
                OPEN_FLAGS_FAIL_ON_ERROR,
                NULL);
  if (NO_ERROR == rc)
  {
      // to obtain write access, the drive needs to be locked
      BYTE  cmd = 0;
      ULONG ulParmLengthInOut = sizeof(cmd),
            ulDataLengthInOut = 0;
      rc = DosDevIOCtl(hDASDOutput,
                       IOCTL_DISK, DSK_LOCKDRIVE,
                       &cmd,
                       sizeof(cmd),
                       &ulParmLengthInOut,
                       0,
                       0,
                       &ulDataLengthInOut);
      if (NO_ERROR != rc)
          DosClose(hDASDOutput);
  }

  if (NO_ERROR == rc)
    setConnected( TRUE );

  return rc;
}


/* terminate existing connection to 'device' */
APIRET IOStreamOutputDASDLogical::disconnect()
{
    BYTE  cmd = 0;
    ULONG ulParmLengthInOut = sizeof(cmd),
          ulDataLengthInOut = 0;

    APIRET rc = DosDevIOCtl(hDASDOutput,
                            IOCTL_DISK,
                            DSK_UNLOCKDRIVE,
                            &cmd,
                            sizeof(cmd),
                            &ulParmLengthInOut,
                            0,
                            0,
                            &ulDataLengthInOut);

  rc = DosClose( hDASDOutput );
  if (NO_ERROR == rc)
  {
    hDASDOutput = NULLHANDLE;
    setConnected( FALSE );
  }

  return rc;
}


APIRET IOStreamOutputDASDLogical::write(IOBuffer *ioBuffer)
{
  ULONG  ulBytesWritten;
  APIRET rc;
  PVOID  pData = ioBuffer->acquire();

  rc = DosWrite(hDASDOutput,
                pData,
                ioBuffer->getValid(),
                &ulBytesWritten);
  if (ulBytesWritten != ioBuffer->getValid())
    rc = ERROR_WRITE_FAULT;

  ioBuffer->release();

  return rc;
}


IOStreamInputFile::IOStreamInputFile( PSZ pszStreamToken )
: IOStreamInput("IOStreamInputFile",
                "Input stream from file",
                pszStreamToken)
{
  hFileInput  = NULLHANDLE;
  pszFilename = strdup( pszStreamToken );
}
  

IOStreamInputFile::~IOStreamInputFile( )
{
  if (NULLHANDLE != hFileInput)
  {
    Debug(("~IOStreamInputFile called with open file (%s)!",
              pszFilename))

      DosClose( hFileInput );
  }

  if (NULL != pszFilename)
    free( pszFilename );
}


/* setup connection to 'device' */
APIRET IOStreamInputFile::connect()
{
  APIRET rc;
  ULONG  ulAction;

  rc = DosOpen (pszFilename,
                &hFileInput,
                &ulAction,
                0L,
                0L,           /* File attributes */
                OPEN_ACTION_OPEN_IF_EXISTS |
                OPEN_ACTION_FAIL_IF_NEW,
                OPEN_SHARE_DENYNONE  |
                OPEN_ACCESS_READONLY |
                OPEN_FLAGS_FAIL_ON_ERROR |
                OPEN_FLAGS_SEQUENTIAL,
                NULL);
  if (NO_ERROR == rc)
    setConnected( TRUE );

  return rc;
}


/* terminate existing connection to 'device' */
APIRET IOStreamInputFile::disconnect()
{
  APIRET rc = DosClose( hFileInput );
  if (NO_ERROR == rc)
  {
    hFileInput = NULLHANDLE;
    setConnected( FALSE );
  }

  return rc;
}
  
  
/* estimate how much data is contained in the stream */
ULONG IOStreamInputFile::estimatedStreamLength(PULONGLONG pull)
{
  // stream size is exactly filelength
  FILESTATUS4 fs4;
  APIRET rc;
  
  // @@@PH add 64-bit support
  
  rc = DosQueryFileInfo(hFileInput,             /* Gather information */
                        FIL_QUERYEASIZE,
                        &fs4,
                        sizeof(fs4));
  if (NO_ERROR != rc)
    return 0;
  else
    return fs4.cbFile;
}


APIRET IOStreamInputFile::read(IOBuffer *ioBuffer)
{
  ULONG  ulBytesRead;
  APIRET rc;
  PVOID  pData = ioBuffer->acquire();

  rc = DosRead(hFileInput,
               pData,
               ioBuffer->getSize(),
               &ulBytesRead);
  if (NO_ERROR != rc)
    ulBytesRead = 0;

  ioBuffer->setValid( ulBytesRead );

  ioBuffer->release();

  return rc;
}


IOStreamInputStdIn::IOStreamInputStdIn( )
: IOStreamInput("IOStreamInputStdIn",
                "Input stream from standard in",
                "STDIN")
{
}
  

IOStreamInputStdIn::~IOStreamInputStdIn( )
{
}
  
  
/* setup connection to 'device' */
APIRET IOStreamInputStdIn::connect()
{
  setConnected( TRUE );
  return NO_ERROR;
}
  
  
/* terminate existing connection to 'device' */
APIRET IOStreamInputStdIn::disconnect()
{
  setConnected( FALSE );
  return NO_ERROR;
}


APIRET IOStreamInputStdIn::read(IOBuffer *ioBuffer)
{
  ULONG  ulBytesRead;
  APIRET rc;
  PVOID  pData = ioBuffer->acquire();

  rc = DosRead(0,
               pData,
               ioBuffer->getSize(),
               &ulBytesRead);
  if (NO_ERROR != rc)
        ulBytesRead = 0;

  ioBuffer->setValid( ulBytesRead );

  ioBuffer->release();

  return rc;
}


IOStreamInputZero::IOStreamInputZero( )
: IOStreamInput("IOStreamInputZero",
                "Input stream from pseudo zero-generator",
                "INZERO")
{
}
  

IOStreamInputZero::~IOStreamInputZero( )
{
}
  
  
/* setup connection to 'device' */
APIRET IOStreamInputZero::connect()
{
  setConnected( TRUE );
  return NO_ERROR;
}
  
  
/* terminate existing connection to 'device' */
APIRET IOStreamInputZero::disconnect()
{
  setConnected( FALSE );
  return NO_ERROR;
}
  

APIRET IOStreamInputZero::read(IOBuffer *ioBuffer)
{
  // fill the buffer with zeros and return
  PVOID  pData = ioBuffer->acquire();

  memset(pData,
         0,
         ioBuffer->getSize() );

  ioBuffer->setValid( ioBuffer->getSize() );
      
  ioBuffer->release();

  return NO_ERROR;
}


IOStreamInputOne::IOStreamInputOne( )
: IOStreamInput("IOStreamInputOne",
                "Input stream from pseudo one-generator",
                "INONE")
{
}
  

IOStreamInputOne::~IOStreamInputOne( )
{
}

/* setup connection to 'device' */
APIRET IOStreamInputOne::connect()
{
  setConnected( TRUE );
  return NO_ERROR;
}
  
  
/* terminate existing connection to 'device' */
APIRET IOStreamInputOne::disconnect()
{
  setConnected( FALSE );
  return NO_ERROR;
}
  

APIRET IOStreamInputOne::read(IOBuffer *ioBuffer)
{
  // fill the buffer with ones and return
  PVOID  pData = ioBuffer->acquire();

  memset(pData,
         1,
         ioBuffer->getSize() );

  ioBuffer->setValid( ioBuffer->getSize() );

  ioBuffer->release();

  return NO_ERROR;
}


IOStreamInputRandom::IOStreamInputRandom( )
: IOStreamInput("IOStreamInputRandom",
                "Input stream from pseudo random-generator",
                "INRANDOM")
{
  // setup random generator from system clock, etc.
  ULONG arrSysInfo[ 6 ];
  int iNumber = sizeof( arrSysInfo ) / sizeof( ULONG );
  int iSeed;

  DosQuerySysInfo(QSV_MS_COUNT,
                  QSV_TOTAVAILMEM,
                  arrSysInfo,
                  iNumber);

  // calculate sum of obtained values
  for (int i = 0;
       i < iNumber;
       i++)
    iSeed += arrSysInfo[ i ];

  // initialize the random number generator
  srand( iSeed );
}


IOStreamInputRandom::~IOStreamInputRandom( )
{
}
  
  
/* setup connection to 'device' */
APIRET IOStreamInputRandom::connect()
{
  setConnected( TRUE );
  return NO_ERROR;
}


/* terminate existing connection to 'device' */
APIRET IOStreamInputRandom::disconnect()
{
  setConnected( FALSE );
  return NO_ERROR;
}
  

APIRET IOStreamInputRandom::read(IOBuffer *ioBuffer)
{
  // fill the buffer with random values and return
  PVOID  pData = ioBuffer->acquire();
  PUCHAR pucByte = (PUCHAR)pData;

  for (int i = 0;
       i < ioBuffer->getSize();
       i++)
  {
    // insert one randomized byte
    *pucByte++ = rand() & 0xff;
  }

  ioBuffer->setValid( ioBuffer->getSize() );

  ioBuffer->release();

  return NO_ERROR;
}


IOStreamInputDASDLogical::IOStreamInputDASDLogical( PSZ pszStreamToken )
: IOStreamInput("IOStreamInputDASDLogical",
                "Input stream from logical partition",
                pszStreamToken)
{
  hDASDInput       = NULLHANDLE;

  // cut off leading "LOG:" signature
  if (0 == strnicmp("LOG:", pszStreamToken, 4))
    pszStreamToken += 4;

  pszPartitionName = strdup( pszStreamToken );
}
  

IOStreamInputDASDLogical::~IOStreamInputDASDLogical( )
{
  if (NULLHANDLE != hDASDInput)
  {
    Debug(("~IOStreamInputDASDLogical called with open partition (%s)!",
           pszPartitionName))

      DosClose( hDASDInput );
  }

  if (NULL != pszPartitionName)
    free( pszPartitionName );
}


/* setup connection to 'device' */
APIRET IOStreamInputDASDLogical::connect()
{
  APIRET rc;
  ULONG  ulAction;

  rc = DosOpen (pszPartitionName,
                &hDASDInput,
                &ulAction,
                0L,
                0L,           /* File attributes */
                OPEN_ACTION_OPEN_IF_EXISTS |
                OPEN_ACTION_FAIL_IF_NEW,
                OPEN_SHARE_DENYNONE  |
                OPEN_ACCESS_READONLY |
                OPEN_FLAGS_DASD |
                OPEN_FLAGS_FAIL_ON_ERROR |
                OPEN_FLAGS_SEQUENTIAL,
                NULL);
  if (NO_ERROR == rc)
    setConnected( TRUE );

  return rc;
}
  
  
/* terminate existing connection to 'device' */
APIRET IOStreamInputDASDLogical::disconnect()
{
  APIRET rc = DosClose( hDASDInput );
  if (NO_ERROR == rc)
  {
    hDASDInput = NULLHANDLE;
    setConnected( FALSE );
  }

  return rc;
}
  
  
/* estimate how much data is contained in the stream */
ULONG IOStreamInputDASDLogical::estimatedStreamLength(PULONGLONG pull)
{
  // stream size is exactly partition size
  APIRET rc;
  FSALLOCATE fsalloc;
  
  // @@@PH add 64-bit support

  ULONG ulDrive = toupper(pszPartitionName[0]) - 'A' + 1;

  rc = DosQueryFSInfo (ulDrive,
                       FSIL_ALLOC,
                       &fsalloc,
                       sizeof( FSALLOCATE ));

  if (NO_ERROR != rc)
    return 0;
  else
  {
    Debug(("IOStreamInputDASDLogical::estimatedStreamLength:\n"
           "sectors = %d, sectorunit=%d, units=%d\n",
           fsalloc.cbSector,
           fsalloc.cSectorUnit,
           fsalloc.cUnit))

      // @@@PH 64-bit support desirable
      return fsalloc.cbSector * fsalloc.cSectorUnit * fsalloc.cUnit;
  }
}

  
APIRET IOStreamInputDASDLogical::read(IOBuffer *ioBuffer)
{
  ULONG  ulBytesRead;
  APIRET rc;
  PVOID  pData = ioBuffer->acquire();

  rc = DosRead(hDASDInput,
               pData,
               ioBuffer->getSize(),
               &ulBytesRead);
  if (NO_ERROR != rc)
    ulBytesRead = 0;

  ioBuffer->setValid( ulBytesRead );

  ioBuffer->release();

  return rc;
}
