/* $Id: moduleimage.cpp,v 1.1 2002/01/10 16:24:48 phaller Exp $
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
#include <string.h>

#include "moduleimage.h"


/****************************************************************************
 * Implementation
 ****************************************************************************/


/*****************************************************************************
 * "MZ" header - DOS / Stub                                                  *
 *****************************************************************************/

typedef unsigned short UINT16;
typedef unsigned long  UINT32;

typedef struct                                    /* DOS 1, 2, 3 .EXE header */
{
    UINT16      e_magic;                                     /* Magic number */
    UINT16      e_cblp;                        /* Bytes on last page of file */
    UINT16      e_cp;                                       /* Pages in file */
    UINT16      e_crlc;                                       /* Relocations */
    UINT16      e_cparhdr;                   /* Size of header in paragraphs */
    UINT16      e_minalloc;               /* Minimum extra paragraphs needed */
    UINT16      e_maxalloc;               /* Maximum extra paragraphs needed */
    UINT16      e_ss;                         /* Initial (relative) SS value */
    UINT16      e_sp;                                    /* Initial SP value */
    UINT16      e_csum;                                          /* Checksum */
    UINT16      e_ip;                                    /* Initial IP value */
    UINT16      e_cs;                         /* Initial (relative) CS value */
    UINT16      e_lfarlc;                /* File address of relocation table */
    UINT16      e_ovno;                                    /* Overlay number */
    /*      the following fields may not be present.
     *              ereloff = 28            not present
     *                      = 30            exe.ever present and valid
     *                      = 32            exe.ever field contains garbage
     *              ereloff > 32            exe.ever present and valid
     *                                              = 0 if "don't know"
     */
    UINT16      e_ver;                      /* version # of producing linker */
    UINT16      e_res0;                                            /* unused */
    /*      the following fields may not be present - if the exe.ereloff
     *      value encompasses the fields then they are present and valid.
     */
    UINT16      e_bb;                                       /* behavior bits */
    UINT16      e_res1;                                    /* Reserved words */
    UINT16      e_oemid;                   /* OEM identifier (for e_oeminfo) */
    UINT16      e_oeminfo;              /* OEM information; e_oemid specific */
    UINT16      e_res2[10];                                /* Reserved words */
    long        e_lfanew;                  /* File address of new exe header */
} EXEIMAGE_MZ_HEADER, *PEXEIMAGE_MZ_HEADER;



ModuleImage::ModuleImage(void* _pBuffer,
                         int _iBufferSize)
{
  setBuffer( _pBuffer );
  setBufferSize( _iBufferSize );
}


ModuleImage::~ModuleImage()
{
}


void ModuleImage::setBuffer(void* _pBuffer)
{
  pBuffer = _pBuffer;
}

void* ModuleImage::getBuffer(void)
{
  return pBuffer;
}


void ModuleImage::setBufferSize(int _iBufferSize)
{
  iBufferSize = _iBufferSize;
}


int ModuleImage::getBufferSize(void)
{
  return iBufferSize;
}


BOOL ModuleImage::checkByte(void* p,
                            int iOffset,
                            char b)
{
  // prevent exceeding buffer boundary
  if (iOffset > getBufferSize() )
    return FALSE;
  
  return ( (PSZ)p )[iOffset] == b;
}


BOOL ModuleImage::checkString(void* p,
                              int iOffset,
                              char* s)
{
  int iLength = strlen( s );
  
  // prevent exceeding buffer boundary
  if ( (iOffset + iLength - 1) > getBufferSize() )
    return FALSE;
  
  return strncmp(s, 
                 ((PSZ)p) + iOffset,
                 iLength) == 0;
}


void* ModuleImage::getModuleHeaderPointer(void)
{
  PBYTE pBase = (PBYTE)getBuffer();
  
  // a module must at least be n bytes long
  if ( getBufferSize() < 4 )
    return pBase;
  
  // check for the vanilla MZ executable
  if ( checkString(pBase, 0, "MZ") )
  {
    // check for further image headers
    PEXEIMAGE_MZ_HEADER pMZ = (PEXEIMAGE_MZ_HEADER)pBase;
    /* if there is a value different from NULL */
    /* if is difficult to decide whether e_lfanew is valid or not. So we */
    /* just add the value there to the Global pointer */
    if (pMZ->e_lfanew == 0)
      return pBase;
    else
    {
      if ( (UINT32)(pMZ->e_lfanew) >= getBufferSize())
      {
        // File address of new exe header is definately invalid.
        return pBase;
      }
      else
        pBase += (UINT32)(pMZ->e_lfanew);
    }
  }
  
  return pBase;
}


int ModuleImage::getModuleType(void)
{
  int   iModuleType = MODULE_UNKNOWN;
  PBYTE pHeader = (PBYTE)getModuleHeaderPointer();
  
  // a module must at least be n bytes long
  if ( getBufferSize() < 128 )
    return MODULE_UNKNOWN;
  
  // check for the vanilla MZ executable
  if ( checkString(pHeader, 0, "MZ") ) return MODULE_MZ;
  if ( checkString(pHeader, 0, "LE") ) return MODULE_LE;
  if ( checkString(pHeader, 0, "LX") ) return MODULE_LX;
  if ( checkString(pHeader, 0, "NE") ) return MODULE_NE;
  if ( checkString(pHeader, 0, "PE00") ) return MODULE_PE;
  
  // chained new header is unknown
  return MODULE_UNKNOWN;
}


int ModuleImage::changeImportModuleName(char* pszOldName,
                                        char* pszNewName)
{
  // This method is only present to prevent a
  // pure virtual function call, yet it virtually
  // does nothing.
  
  return 0;
}


int ModuleImage::displayImportModuleNames(void)
{
  // This method is only present to prevent a
  // pure virtual function call, yet it virtually
  // does nothing.
  
  return 0;
}
