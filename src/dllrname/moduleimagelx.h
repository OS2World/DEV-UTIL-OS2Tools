/* $Id: moduleimagelx.h,v 1.1 2002/01/10 16:24:49 phaller Exp $
 *
 * DLL Rename Utility
 *
 * Copyright (C) 2002 InnoTek Systemberatung GmbH
 *
 */

#ifndef _MODULEIMAGELX_H_
#define _MODULEIMAGELX_H_


/****************************************************************************
 * Includes
 ****************************************************************************/

#include "moduleimage.h"


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

class ModuleImageLX : public ModuleImage
{
public:
  ModuleImageLX(void* _pBuffer,
                int _iBufferSize);
  ~ModuleImageLX();
  
  int changeImportModuleName(char* pszOldName,
                             char* pszNewName);

  int displayImportModuleNames(void);
};



#endif /* _MODULEIMAGELX_H_ */
