/* $Id: kFileOMFLib.cpp,v 1.4 2001/12/21 04:09:08 bird Exp $
 *
 * kFileOMF -  files.
 *
 * Copyright (c) 2001 knut st. osmundsen
 *
 */


/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include <string.h>

#include "OMFobj.h"

#include "kTypes.h"
#include "kError.h"
#include "kFile.h"
#include "kFileInterfaces.h"
#include "kFileFormatBase.h"
#include "kFileOMF.h"
#include "kList.h"
#include "kFileOMFLib.h"


/*******************************************************************************
*   Global Variables                                                           *
*******************************************************************************/
#if 0
static kFileOMFLib tst((kFile*)NULL);
#endif


/**
 * Creator.
 * @param   pFile   Pointer to file object.
 * @status  competely implemented.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
kFileOMFLib::kFileOMFLib(kFile *pFile) :
    kFileFormatBase(pFile) , pvFile(NULL)
{
    pFile->setThrowOnErrors();
    pvFile = pFile->mapFile();
    parse();
}


/**
 * Destructor.
 * @status  completely implemented.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
kFileOMFLib::~kFileOMFLib()
{
    if (pvFile)
        kFile::mapFree(pvFile);
    #ifdef DEBUG
    pvFile = (void*)-1;
    cbFile = -1;
    #endif
}


/**
 * Dumps this library to pOut.
 * @return  Success indicator.
 * @param   pOut    Pointer to output file.
 */
KBOOL kFileOMFLib::dump(kFile *pOut)
{




    /*
     * Dump individual OMF modules.
     */
    kFileOMFLib_Member  *   pMember = Members.getFirst();
    while (pMember)
    {
        /*
         * Object module.
         */
        pOut->printf(
            " Library member: %s\n"
            "------------------------------------------------------------\n",
            pMember->pszName);

        try
        {
            if (!pMember->pOBJ)
                pMember->pOBJ = new kFileOMF(pMember->pvFile, pMember->cbFile);
            pMember->pOBJ->dump(pOut);
        }
        catch (kError err)
        {
            pOut->printf("  Failed to create object for this OMF module. err=%d\n",
                         err.getErrno());
        }
        pMember = (kFileOMFLib_Member*)pMember->getNext();
    }


    return TRUE;
}



/**
 * This function parses the library file.
 * @sketch
 * @status  completely implemented.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 * @remark
 */
void kFileOMFLib::parse(void)
{
    POMFHDR                 pHdr = (POMFHDR)pvFile;
    char *                  pch = (char*)pvFile;
    char *                  pchEndFile;
    kFileOMFLib_Member  *   pMember;
    int                     cbRec;
    int                     cchPage;

    /*
     * Library file header.
     */
    if (   (pHdr->chType != LIBHDR && pHdr->chType != LIBHDR2)
        ||  pHdr->cch <= sizeof(OMFHDR))
        throw (kError(kError::INVALID_SIGNATURE));
    if (pHdr->chType == LIBHDR2)
    {
        parseNew();
        return;
    }
    pchEndFile = pch + OMF_DWORD(pch, 3);
    cchPage = pHdr->cch + 3;

    /*
     * Skip records to the first object module.
     */
    do
    {
        pch += pHdr->cch + 3;
        pHdr = (POMFHDR)pch;
    } while (pch + 3 < pchEndFile && pHdr->chType != THEADR);


    /*
     * Loop thru all members.
     */
    pMember = NULL;
    while (pch + 3 < pchEndFile)
    {
        pHdr = (POMFHDR)pch;
        switch (pHdr->chType)
        {
            case THEADR:
                if (pMember)
                    throw(kError(kError::BAD_FORMAT));
                pMember = new kFileOMFLib_Member;
                pMember->cbFile = -1;
                pMember->pOBJ = NULL;
                pMember->pvFile = pch;
                pMember->pszName = new char[OMF_BYTE(pch, 3) + 1];
                memcpy(pMember->pszName, pch + 4, OMF_BYTE(pch, 3));
                pMember->pszName[OMF_BYTE(pch, 3)] = '\0';
                break;

            case MODEND:
            case MODEND32:
                if (!pMember)
                    throw(kError(kError::BAD_FORMAT));
                pMember->cbFile = pch + pHdr->cch + 3 - (char*)pMember->pvFile;
                Members.insert(pMember);
                pMember = NULL;

                /* align next */
                pch = (char*)pvFile +  ((pch - (char*)pvFile + pHdr->cch + 3 + cchPage - 1) & ~(cchPage-1)) - (pHdr->cch + 3);
                break;

            case LIBEND:
                if (pMember)
                    throw(kError(kError::BAD_FORMAT));
                pch = pchEndFile;
                break;

            case LHEADR:
            case COMENT:
            case EXTDEF:
            case TYPDEF:
            case PUBDEF:
            case PUBDEF32:
            case LINNUM:
            case LINNUM32:
            case LNAMES:
            case SEGDEF:
            case SEGDEF32:
            case GRPDEF:
            case FIXUPP:
            case FIXUPP32:
            case LEDATA:
            case LEDATA32:
            case LIDATA:
            case LIDATA32:
            case COMDEF:
            case BAKPAT:
            case BAKPAT32:
            case LEXTDEF:
            case LEXTDEF32:
            case LPUBDEF:
            case LPUBDEF32:
            case LCOMDEF:
            case COMDAT:
            case COMDAT32:
            case LINSYM:
            case LINSYM32:
            case ALIAS :
            case NBKPAT:
            case NBKPAT32:
            case LLNAMES:
            case COMFIX:
            case COMFIX32:
            case CEXTDEF:
                break;

            default:
                kFile::StdOut.printf("debug:unknown record type %x length %x\n", pHdr->chType, pHdr->chType);
                throw(kError(kError::BAD_FORMAT));
        }
        pch += pHdr->cch + 3;
    }

    if (pHdr->chType != LIBEND)
        throw(kError(kError::BAD_FORMAT));


    /*
     * Extended dictionary. (ignored currently)
     */

}



/**
 * This function parses the new library files generated by the VAC compilers.
 * @sketch
 * @status  completely implemented.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 * @remark
Starts with a F3h record (LIBHDR2).

   1     2        1         4                   2           2
   F3    Record   Case      Size of names       Number of   Unknown
         Length   Sensitive                     Modules

   4                        4                   4
   Size of public names.    Unknown             Offset to some kind of array.

Followed by an zero string name listing. <string>'\0'<string>'\0'...
The size if given by the DWORD at offset 4 of LIBHDR2.

Then follows an offset table for the modules. This is an array of dwords.

*/
void kFileOMFLib::parseNew(void)
{
    POMFHDR                 pHdr = (POMFHDR)pvFile;
    char *                  pch = (char*)pvFile;
    char *                  pchEndFile;
    kFileOMFLib_Member  *   pMember;
    int                     cbRec;
    unsigned long *         paOff;
    int                     cMembers;

    /*
     * Library file header.
     */
    if (pHdr->chType != LIBHDR2 || pHdr->cch <= sizeof(OMFHDR))
        throw (kError(kError::INVALID_SIGNATURE));

    paOff = (unsigned long*)(pch + pHdr->cch + 3 + OMF_DWORD(pvFile, 4));
    cMembers = OMF_WORD(pvFile, 8);


    /*
     * Loop thru all members.
     */
    for (int i = 0; i < cMembers; i++)
    {
        pMember = new kFileOMFLib_Member;
        pMember->cbFile = -1;
        pMember->pOBJ = NULL;
        pMember->pvFile = (char*)pvFile + paOff[i];

        pHdr = (POMFHDR)pMember->pvFile;
        if (pHdr->chType != THEADR)
        {
            delete pMember;
            throw(kError(kError::BAD_FORMAT));
        }
        pch = (char*)pHdr;
        pMember->pszName = new char[OMF_BYTE(pch, 4) + 1];
        memcpy(pMember->pszName, pch + 4, OMF_BYTE(pch, 4));
        pMember->pszName[OMF_BYTE(pch, 4)] = '\0';

        /*
         * Skip to end of file.
         */
        while (pHdr->chType != MODEND && pHdr->chType != MODEND32)
        {
            switch (pHdr->chType)
            {
                case LIBEND:
                    if (pMember)
                        delete pMember;
                    throw(kError(kError::BAD_FORMAT));

                case THEADR:
                case LHEADR:
                case COMENT:
                case EXTDEF:
                case TYPDEF:
                case PUBDEF:
                case PUBDEF32:
                case LINNUM:
                case LINNUM32:
                case LNAMES:
                case SEGDEF:
                case SEGDEF32:
                case GRPDEF:
                case FIXUPP:
                case FIXUPP32:
                case LEDATA:
                case LEDATA32:
                case LIDATA:
                case LIDATA32:
                case COMDEF:
                case BAKPAT:
                case BAKPAT32:
                case LEXTDEF:
                case LEXTDEF32:
                case LPUBDEF:
                case LPUBDEF32:
                case LCOMDEF:
                case COMDAT:
                case COMDAT32:
                case LINSYM:
                case LINSYM32:
                case ALIAS :
                case NBKPAT:
                case NBKPAT32:
                case LLNAMES:
                case COMFIX:
                case COMFIX32:
                case CEXTDEF:
                    break;

                default:
                    kFile::StdOut.printf("debug:unknown record type %x length %x\n", pHdr->chType, pHdr->chType);
                    throw(kError(kError::BAD_FORMAT));
            }
            pHdr = (POMFHDR)((char*)pHdr + pHdr->cch + 3);
        }

        /*
         * Finish member object and add it to the list.
         */
        pMember->cbFile = (char*)pHdr + pHdr->cch + 3 - (char*)pMember->pvFile;
        Members.insert(pMember);
    }

}




kFileOMFLib_Member::~kFileOMFLib_Member()
{
    delete pszName;
}


#include "kList.cpp"
