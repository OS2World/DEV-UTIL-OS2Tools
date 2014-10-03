#include <os2.h>
#include <stdio.h>

#include "LXexe.h"

#include "kTypes.h"
#include "kError.h"
#include "kFile.h"
#include "kFileInterfaces.h"
#include "kFileFormatBase.h"
#include "kFileLX.h"

/*******************************************************************************
*   Internal Functions                                                         *
*******************************************************************************/
void hexdump(char *pch, int cch, int offset);

extern ULONG EXPENTRY EP2_32_Decompress(ULONG cbSource,
                                        ULONG cbDest,
                                        char  *pSource,
                                        char  *pDest);

void main (int argc,  char **argv)
{
    #if 0
    //for (int argi = 1; argi < argc; argi++)
    //{
        kFile *     pFile = NULL;

        try
        {
            pFile = new kFile(argv[0]);
            kFileLX LX(pFile);
            char achPage[0x1004];
            int rc = LX.pageGet(&achPage[0], 0x1B000);
            if (!rc)
            {
                if (memcmp(achPage, (void*)0x1B000, 0x1000))
                    kFile::StdOut.printf("compare failed!\r\n");
                else
                    kFile::StdOut.printf("compare successfull!\r\n");
            }
            else
                kFile::StdOut.printf("failed to read page! rc=%d\r\n", rc);

            rc = LX.pageGet(&achPage[0], 0x10000);
            if (!rc)
            {
                if (memcmp(achPage, (void*)0x10000, 0x1000))
                    kFile::StdOut.printf("compare failed!\r\n");
                else
                    kFile::StdOut.printf("compare successfull!\r\n");
            }
            else
                kFile::StdOut.printf("failed to read page! rc=%d\r\n", rc);
        }
        catch (kError err)
        {
            if (!pFile)
            {
                kFile::StdOut.printf("Failed to open %s. err=%d\r\n",
                                     argv[0],
                                     err.getErrno());
            }
        }
    //} /* for */
    #else
    for (int argi = 1; argi < argc; argi++)
    {
        kFile *     pFile = NULL;

        try
        {
            kFileLX LX(new kFile(argv[argi]));
            int     cObjs = LX.getObjectCount();
            for (int iObj = 0; iObj < cObjs; iObj++)
            {
                char                achPage[0x1004];
                struct o32_obj *    pObj = LX.getObject(iObj);

                for (int iPage = 0; iPage < pObj->o32_mapsize; iPage++)
                {
                    printf("iObj=%2d  iPage=%2d\n", iObj, iPage);
                    int rc = LX.pageGet(&achPage[0], iObj, iPage * 0x1000);
                    if (rc)
                        printf("pageGet -> rc=%d\n", rc);
                    #if 1
                    rc = LX.pagePut(&achPage[0], iObj, iPage * 0x1000);
                    if (rc)
                        printf("pagePut -> rc=%d\n", rc);
                    #else
                    hexdump(&achPage[0], 0x1000, pObj->o32_base + iPage*0x1000);
                    #endif
                }
            }

            #if 1
            kRelocEntry reloc;
            KBOOL       fRc;

            fRc = LX.relocFindFirst(0, 0, &reloc);
            while (fRc)
            {
                if (reloc.isName())
                    printf("%04x:%08x %2d/%d  flags=%08x  %s.%s %d\n",
                           reloc.ulSegment,
                           reloc.offSegment,
                           reloc.offSegment / 0x1000,
                           reloc.ulSegment,
                           reloc.fFlags,
                           reloc.Info.Name.pszModule ? reloc.Info.Name.pszModule : "<NULL>",
                           reloc.Info.Name.pszName ? reloc.Info.Name.pszName : "<NULL>",
                           reloc.Info.Name.ulOrdinal);
                else
                    printf("%04x:%08x %2d/%d  flags=%08x  %04x:%08x\n",
                           reloc.ulSegment,
                           reloc.offSegment,
                           reloc.offSegment / 0x1000,
                           reloc.ulSegment,
                           reloc.fFlags,
                           reloc.Info.Internal.ulSegment,
                           reloc.Info.Internal.offSegment);

                /* next */
                fRc = LX.relocFindNext(&reloc);
            }

            LX.relocFindClose(&reloc);

            #endif
            flushall();
        }
        catch (kError err)
        {
            if (!pFile)
            {
                kFile::StdOut.printf("Failed to open %s. err=%d\r\n",
                                     argv[argi],
                                     err.getErrno());
            }
            flushall();
        }
    } /* for */

    #endif
}



void hexdump(char *pch, int cch, int offset)
{
    char * pchStart = pch;
    while (cch > 0)
    {
        int i;
        printf("%08x ", (int)pch - (int)pchStart + offset);

        for (i = 0; i < 32 && cch > 0; i++, pch++, cch--)
            printf(" %02x", (int)*pch);
        printf("\n");
    }
}
