/* $Id: kFileOMF.cpp,v 1.6 2001/12/21 04:10:44 bird Exp $
 *
 * kFileOMF -  files.
 *
 * Copyright (c) 2001 knut st. osmundsen
 *
 */


/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "kTypes.h"
#include "kError.h"
#include "kFile.h"
#include "kFileInterfaces.h"
#include "kFileFormatBase.h"
#include "OMFobj.h"
#include "kFileOMF.h"


/*******************************************************************************
*   Global Variables                                                           *
*******************************************************************************/
static char *   aszTypes[256] =
{
          "?",       "?",       "?",       "?",       "?",       "?",       "?",       "?",   /* 00h-08h */
          "?",       "?",       "?",       "?",       "?",       "?",       "?",       "?",   /* 08h-10h */
          "?",       "?",       "?",       "?",       "?",       "?",       "?",       "?",   /* 10h-18h */
          "?",       "?",       "?",       "?",       "?",       "?",       "?",       "?",   /* 18h-20h */
          "?",       "?",       "?",       "?",       "?",       "?",       "?",       "?",   /* 20h-28h */
          "?",       "?",       "?",       "?",       "?",       "?",       "?",       "?",   /* 28h-30h */
          "?",       "?",       "?",       "?",       "?",       "?",       "?",       "?",   /* 30h-38h */
          "?",       "?",       "?",       "?",       "?",       "?",       "?",       "?",   /* 38h-40h */
          "?",       "?",       "?",       "?",       "?",       "?",       "?",       "?",   /* 40h-48h */
          "?",       "?",       "?",       "?",       "?",       "?",       "?",       "?",   /* 48h-50h */
          "?",       "?",       "?",       "?",       "?",       "?",       "?",       "?",   /* 50h-58h */
          "?",       "?",       "?",       "?",       "?",       "?",       "?",       "?",   /* 58h-60h */
          "?",       "?",       "?",       "?",       "?",       "?",       "?",       "?",   /* 60h-68h */
          "?",       "?",       "?",       "?",       "?",       "?", "!RHEADR",       "?",   /* 68h-70h */
    "!REGINT",       "?", "!REDATA",       "?", "!RIDATA",       "?", "!OVLDEF",       "?",   /* 70h-78h */
    "!ENDREC",       "?", "!BLKDEF",       "?", "!BLKEND",       "?", "!DEBSYM",       "?",   /* 78h-80h */
     "THEADR",       "?",  "LHEADR",       "?", "!PEDATA",       "?", "!PIDATA",       "?",   /* 80h-88h */
     "COMENT",       "?",  "MODEND","MODEND32",  "EXTDEF",       "?",  "TYPDEF",       "?",   /* 88h-90h */
     "PUBDEF","PUBDEF32", "!LOCSYM",       "?",  "LINNUM","LINNUM32",  "LNAMES",       "?",   /* 90h-98h */
     "SEGDEF","SEGDEF32",  "GRPDEF",       "?",  "FIXUPP","FIXUPP32","!unused!",       "?",   /* 98h-a0h */
     "LEDATA","LEDATA32",  "LIDATA","LIDATA32",  "LIBHED",       "?",  "LIBNAM",       "?",   /* a0h-a8h */
     "LIBLOC",       "?",  "LIBDIC",       "?",       "?",       "?",       "?",       "?",   /* a8h-b0h */
     "COMDEF",       "?",  "BAKPAT","BAKPAT32", "LEXTDEF","LEXTDEF32","LPUBDEF","LPUBDEF32",  /* b0h-b8h */
    "LCOMDEF",       "?",  "COMFIX","COMFIX32", "CEXTDEF",       "?",       "?",       "?",   /* b8h-c0h */
     "SELDEF",       "?",  "COMDAT","COMDAT32",  "LINSYM","LINSYM32",   "ALIAS",       "?",   /* c0h-c8h */
     "NBKPAT","NBKPAT32", "LLNAMES",       "?",       "?",       "?",       "?",       "?",   /* c8h-d0h */
          "?",       "?",       "?",       "?",       "?",       "?",       "?",       "?",   /* d0h-d8h */
          "?",       "?",       "?",       "?",       "?",       "?",       "?",       "?",   /* d8h-e0h */
          "?",       "?",       "?",       "?",       "?",       "?",       "?",       "?",   /* e0h-e8h */
          "?",       "?",       "?",       "?",       "?",       "?",       "?",       "?",   /* e8h-f0h */
     "LIBHDR",  "LIBEND",       "?", "LIBHDR2",       "?",       "?",       "?",       "?",   /* f0h-f8h */
          "?",       "?",       "?",       "?",       "?",       "?",       "?",       "?"    /* f8h-100h */

};



kFileOMF::kFileOMF(kFile *pFile)
    : kFileFormatBase(pFile)
{
    //throw(kError(kError::NOT_SUPPORTED));
    pvFile = pFile->mapFile();
    cbFile = pFile->getSize();
}


kFileOMF::kFileOMF(void *pvFile, unsigned long cbFile) :
    kFileFormatBase(NULL), pvFile(pvFile), cbFile(cbFile)
{
//parse();
}


kFileOMF::~kFileOMF()
{
    if (pvFile)
        kFile::mapFree(pvFile);
    pvFile = NULL;
}


KBOOL kFileOMF::dump(kFile *pOut)
{
    char *  pch1;
    char *  pch2;
    char *  pch3;
    int     i1;
    int     i2;
    int     i3;
    char *  pch = (char*)pvFile;
    char *  pchEndFile = pch + cbFile;

    while (pch + 3 < pchEndFile)
    {
        POMFHDR pHdr = (POMFHDR)pch;
        char *  pchEnd = pHdr->cch + 3 + pch;


        pOut->printf("%s: type=%02x size=%04x ",
                     aszTypes[pHdr->chType], pHdr->chType, pHdr->cch);
        switch (pHdr->chType)
        {
            case THEADR:
                break;

            case MODEND:
            case MODEND32:
                pch = pchEndFile;
                break;

            case PUBDEF:
            case PUBDEF32:
            case LPUBDEF:
            case LPUBDEF32:
                pch1 = pch + 3 + INDEX_SIZE(pch + 3);
                pch2 = pch1 + INDEX_SIZE(pch1);
                if (!*pch1)
                    pch2 += 2;
                //pOut->printf("PUBDEF%s: segno=%d seggrp=%d \n",
                //             pHdr->chType & 0x1 ? "32" : "");
                for (pchEnd = pch + pHdr->cch; pch2 + 1 < pchEnd; )
                {
                    pOut->printf("\r\n    0x%08x %.*s",
                                 pHdr->chType & 0x1 ? OMF_DWORD(pch2, *pch2 + 1) : OMF_WORD(pch2, *pch2 + 1),
                                 *pch2, pch2 + 1);
                    pch2 += *pch2 + 1 + (pHdr->chType & 0x1 ? 4 : 2);
                    pch2 += INDEX_SIZE(pch2);
                }
                break;

            case COMENT:
                #if 0 //ext+pub only
                switch (*((unsigned char*)(pch+4)))
                {
                    case CMTCLASS_A0:
                        switch (*((unsigned char*)(pch+5)))
                        {
                            case CMTA0_IMPDEF:
                            {
                                int             fOrd = *((unsigned char*)((int)pch + 6));
                                unsigned char   cchIntName  = *((unsigned char*)((int)pch + 7));
                                char           *pachIntName = (char*)((int)pch + 8);
                                unsigned char   cchDllName  = *((unsigned char*)((int)pch + 8 + cchIntName));
                                char           *pachDllName = (char*)((int)pch + 9 + cchIntName);
                                unsigned char   cchExtName  = *((unsigned char*)((int)pch + 9 + cchIntName + cchDllName));
                                char           *pachExtName = (char*)((int)pch +10 + cchIntName + cchDllName);
                                unsigned short  usOrdinal   = *((unsigned short*)((int)pch + 9 + cchIntName + cchDllName));

                                pOut->printf("\r\nIMPDEF: fOrd=%d; intName=%.*s; dllName=%.*s; ",
                                             fOrd, cchIntName, pachIntName, cchDllName, pachDllName);
                                if (fOrd)
                                    pOut->printf("%d", usOrdinal);
                                else
                                    pOut->printf("%.*s", cchExtName, pachExtName);
                                break;
                            }
                        } //switch
                        break;

                } //switch
                #endif
                break;

            case LNAMES:
            {
                #if 0 //ext+pub only
                char *  pchName = pch + 3;
                int     i = 1;
                while (pchName < pch + pHdr->cch + 2)
                {
                    pOut->printf("\r\n    %2d,\"%.*s\"", i, *pchName, pchName+1);
                    pchName += *pchName + 1;
                    i++;
                }
                #endif
                break;
            }

            case SEGDEF:
            case SEGDEF32:
                #if 0 //ext+pub only
                pOut->printf("segdef ACBP=%x,%x,%x,%x len=0x%x sni=%d cni=%d oni=%d",
                             (pch[3] >> 5) & 7,
                             (pch[3] >> 2) & 7,
                             (pch[3] >> 1) & 1,
                             pch[3] & 1,
                             *(int*)(pch+4),
                             *(pch+8), /* these are not correct. but works for small 32-bit stuff. */
                             *(pch+9),
                             *(pch+10));
                #endif
                break;

            case LEXTDEF:
            case LEXTDEF32:
            case EXTDEF:
            {
                char *  pchName = pch + 3;
                int     i = 1;
                while (pchName < pch + pHdr->cch + 2)
                {
                    pOut->printf("\r\n    %2d,\"%.*s\"  type %d",
                                 i,
                                 *pchName, pchName + 1,
                                 INDEX_VALUE(pchName + *pchName + 1)
                                 );
                    /* next */
                    pchName += *pchName + 1;
                    pchName += INDEX_SIZE(pchName);
                    i++;
                }
                break;
            }

            case LHEADR:
            case TYPDEF:
            case LINNUM:
            case LINNUM32:
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

            case LIBEND:
                throw(kError(kError::BAD_FORMAT));

            default:
                kFile::StdOut.printf("debug:unknown record type %x length %x\n", pHdr->chType, pHdr->chType);
                throw(kError(kError::BAD_FORMAT));
        }
        pOut->printf("\r\n");
        pch += pHdr->cch + 3;
    }

    return TRUE;
}
