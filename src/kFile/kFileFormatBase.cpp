/* $Id: kFileFormatBase.cpp,v 1.3 2001/12/14 21:33:12 bird Exp $
 *
 * kFileFormatBase - Base class for kFile<format> classes.
 *
 * Copyright (c) 1999-2001 knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 *
 */



/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include "kTypes.h"
#include "kError.h"
#include "kFile.h"
#include "kFileInterfaces.h"
#include "kFileFormatBase.h"



/**
 * Constructor.
 * Saves the file object pointer.
 * @param   pFile   This will be deleted if the object is
 *                  successfully constructed (by the destructor).
 * @status  completely implemented.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
kFileFormatBase::kFileFormatBase(kFile *pFile)
    : pFile(pFile)
{
}


/**
 * Destructor.
 * Deletes the file object if it exists.
 * @status  completely implemented.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
kFileFormatBase::~kFileFormatBase()
{
    if (pFile)
        delete pFile;
}


/**
 * Dump function.
 * @returns Successindicator.
 * @param   pOut    Output file.
 */
KBOOL   kFileFormatBase::dump(kFile *pOut)
{
    pOut->printf("Sorry, dump() is not implemented for this file format.\n");
    return FALSE;
}

