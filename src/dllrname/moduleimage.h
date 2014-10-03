/* $Id: moduleimage.h,v 1.1 2002/01/10 16:24:48 phaller Exp $
 *
 * DLL Rename Utility
 *
 * Copyright (C) 2002 InnoTek Systemberatung GmbH
 *
 */

#ifndef _MODULEIMAGE_H_
#define _MODULEIMAGE_H_


/****************************************************************************
 * Includes
 ****************************************************************************/


/****************************************************************************
 * Implementation
 ****************************************************************************/

enum ModuleType
{
  MODULE_UNKNOWN = 0,
  MODULE_MZ,
  MODULE_NE,
  MODULE_LE,
  MODULE_LX,
  MODULE_PE
};


class ModuleImage
{
  protected:
    void* pBuffer;
    int iBufferSize;
  
  public:
    ModuleImage(void* _pBuffer,
                int _iBufferSize);
    ~ModuleImage();
  
    void setBuffer(void* _pBuffer);
  
    void* getBuffer(void);
  
    void setBufferSize(int iBufferSize);
  
    int getBufferSize(void);
  
    void* getModuleHeaderPointer(void);
  
    int getModuleType(void);
  
    BOOL checkByte(void* p,
                   int iOffset,
                   char b);
  
    BOOL checkString(void* p,
                     int iOffset,
                     char* s);
  
    virtual int changeImportModuleName(char* pszOldName,
                                       char* pszNewName);
  
    virtual int displayImportModuleNames(void);
};


#endif /* _MODULEIMAGE_H_ */
