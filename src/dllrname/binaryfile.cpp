/* $Id: binaryfile.cpp,v 1.1 2002/01/10 16:24:47 phaller Exp $
 *
 * DLL Rename Utility
 *
 * Copyright (C) 2002 InnoTek Systemberatung GmbH
 *
 */


/****************************************************************************
 * Includes
 ****************************************************************************/

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "binaryfile.h"


/****************************************************************************
 * Implementation
 ****************************************************************************/


/***********************************************************************
 * Name      :
 * Purpose   :
 * Parameter : 
 * Variables :
 * Result    :
 * Remark    :
 *
 * Author    : Patrick Haller [2002-01-10]
 ***********************************************************************/

BinaryFile::BinaryFile(void)
{
  // no buffer allocated yet
  pBuffer     = NULL;
  iBufferSize = 0;
  
  setFilename( NULL );
}


BinaryFile::BinaryFile(char* pszName)
{
  // no buffer allocated yet
  pBuffer     = NULL;
  iBufferSize = 0;
  
  setFilename( pszName );
}


void BinaryFile::clear()
{
  // free file buffer if one was allocated
  if (NULL != pBuffer)
    free( pBuffer );
  
  pBuffer = NULL;
  iBufferSize = 0;
}


BinaryFile::~BinaryFile()
{
  // clear any allocated buffer
  clear();
}


APIRET BinaryFile::load(int* piBytesLoaded)
{
  APIRET      rc;
  HFILE       hFile;
  FILESTATUS4 fs4;        /* Structure to hold the second level information */
  ULONG       ulAction;                              /* DosOpen action code */
  
  // if another file was loaded, clear buffer first
  clear();
  
  // check if file exists
  rc = DosQueryPathInfo(getFilename(),
                        FIL_QUERYEASIZE,
                        &fs4,
                        sizeof(fs4));
  if (NO_ERROR != rc)
    return rc;
  
  // verify memory consumption and create required buffer
  pBuffer = malloc( fs4.cbFile );
  if (NULL == pBuffer)
  {
    // gracefully handle out-of-memory case
    iBufferSize = 0;
    return ERROR_NOT_ENOUGH_MEMORY;
  }
  
  iBufferSize = fs4.cbFile;
  
  // open file for reading (shared)
  rc = DosOpen (getFilename(),
                &hFile,
                &ulAction,
                0L,                                            /* Filesize */
                0L,                                     /* File attributes */
                OPEN_ACTION_FAIL_IF_NEW |
                OPEN_ACTION_OPEN_IF_EXISTS,
                OPEN_SHARE_DENYNONE |
                OPEN_ACCESS_READONLY |
                OPEN_FLAGS_SEQUENTIAL,
                NULL);
  if (NO_ERROR == rc)
  {
    ULONG ulBytesRead;
    
    // read whole file into buffer
    rc = DosRead(hFile,
                 pBuffer,
                 (ULONG) iBufferSize,
                 &ulBytesRead);
    
    // check if all bytes were read
    if (ulBytesRead != iBufferSize)
      rc = ERROR_READ_FAULT;
    
    // close the file
    DosClose( hFile );
  }
  
  // if everything was OK,
  if (NO_ERROR != rc)
    clear();
  
  // return number of bytes read
  if (NULL != piBytesLoaded)
    *piBytesLoaded = iBufferSize;
  
  return rc;
}


APIRET BinaryFile::save(void)
{
  APIRET      rc;
  HFILE       hFile;
  ULONG       ulAction;                              /* DosOpen action code */

  // open file for reading (shared)
  rc = DosOpen (getFilename(),
                &hFile,
                &ulAction,
                0L,                                            /* Filesize */
                0L,                                     /* File attributes */
                OPEN_ACTION_CREATE_IF_NEW |
                OPEN_ACTION_OPEN_IF_EXISTS,
                OPEN_SHARE_DENYWRITE |
                OPEN_ACCESS_READWRITE |
                OPEN_FLAGS_SEQUENTIAL,
                NULL);
  if (NO_ERROR == rc)
  {
    ULONG ulBytesWritten;
    
    // write whole buffer to file
    rc = DosWrite(hFile,
                  pBuffer,
                  (ULONG) iBufferSize,
                  &ulBytesWritten);
    
    // check if all bytes were written
    if (ulBytesWritten != iBufferSize)
      rc = ERROR_WRITE_FAULT;
    
    // close the file
    DosClose( hFile );
  }
  
  return rc;
}


APIRET BinaryFile::canWrite(void)
{
  APIRET      rc;
  HFILE       hFile;
  ULONG       ulAction;                              /* DosOpen action code */

  // open file for reading (shared)
  rc = DosOpen (getFilename(),
                &hFile,
                &ulAction,
                0L,                                            /* Filesize */
                0L,                                     /* File attributes */
                OPEN_ACTION_FAIL_IF_NEW |
                OPEN_ACTION_OPEN_IF_EXISTS,
                OPEN_SHARE_DENYWRITE |
                OPEN_ACCESS_READWRITE |
                OPEN_FLAGS_SEQUENTIAL,
                NULL);
  if (NO_ERROR == rc)
  {
    // close the file
    DosClose( hFile );
  }
  
  return rc;
}


// generate backup filename
void BinaryFile::generateBackupFilename(char* pszTarget,
                                        int iTargetLength)
{
  char szBuffer[260];
  
  // get the original current name first
  strncpy(szBuffer,
          getFilename(),
          sizeof( szBuffer ));
  
  // transform it into a backup name
  char* pszLastDot = strrchr(szBuffer, '.');
  if (NULL == pszLastDot)
    pszLastDot = szBuffer + strlen(szBuffer) - 1;
  
  // append the .bak extension
  strcpy(pszLastDot,
         ".bak");
  
  // ok, copy back the generated name
  strncpy(pszTarget,
          szBuffer,
          iTargetLength);
}


APIRET BinaryFile::backup(BOOL flagOverwrite)
{
  APIRET rc;
  char   szBuffer[260];
  
  generateBackupFilename( szBuffer, sizeof( szBuffer ) );
  
  // don't fail if EAs can't be copied
  // don't append to already existing file
  // fail if file already exists
  rc = DosCopy(getFilename(),
               szBuffer,
               (flagOverwrite == TRUE) ? DCPY_EXISTING : 0);
  
  return rc;
}