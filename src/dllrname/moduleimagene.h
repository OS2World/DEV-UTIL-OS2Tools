/* $Id: moduleimagene.h,v 1.1 2002/01/10 16:24:49 phaller Exp $
 *
 * DLL Rename Utility
 *
 * Copyright (C) 2002 InnoTek Systemberatung GmbH
 *
 */

#ifndef _MODULEIMAGENE_H_
#define _MODULEIMAGENE_H_


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

class ModuleImageNE : public ModuleImage
{
public:
  ModuleImageNE(void* _pBuffer,
                int _iBufferSize);
  ~ModuleImageNE();
  
  int changeImportModuleName(char* pszOldName,
                             char* pszNewName);
  
  int displayImportModuleNames(void);
};



#endif /* _MODULEIMAGENE_H_ */
