/* $Id: binaryfile.h,v 1.1 2002/01/10 16:24:47 phaller Exp $
 *
 * DLL Rename Utility
 *
 * Copyright (C) 2002 InnoTek Systemberatung GmbH
 *
 */

#ifndef _BINARYFILE_H_
#define _BINARYFILE_H_


/****************************************************************************
 * Includes
 ****************************************************************************/


/****************************************************************************
 * Implementation
 ****************************************************************************/

class BinaryFile
{
  protected:
    // file metadata
    char* pszFilename;
  
    // pointer to memory buffer containing file
    void* pBuffer;
  
    // size of above buffer
    int iBufferSize;
  
  
  public:
    BinaryFile();
    BinaryFile(char* pszName);
    ~BinaryFile();
  
    // get current filename
    char* getFilename(void) { return pszFilename; }
  
    // set current filename
    void setFilename(char* pszName) { pszFilename = pszName; }
  
    // load file into memory
    APIRET load(int* piBytesLoaded);
  
    // save memory buffer back to file
    APIRET save(void);
  
    // generate backup filename
    void generateBackupFilename(char* pszTarget,
                                int iTargetLength);
  
    // get pointer to buffer
    void* getBuffer(void) { return pBuffer; }
  
    // get buffer size
    int getBufferSize(void) { return iBufferSize; }
  
    // clear the internal buffers
    void clear();
  
    // verify if the file is writable
    APIRET canWrite();
  
    // create a backup of the current file
    APIRET BinaryFile::backup(BOOL flagOverwrite);
};


#endif /* _BINARYFILE_H_ */
