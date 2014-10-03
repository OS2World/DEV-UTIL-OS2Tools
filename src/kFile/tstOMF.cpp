#include "OMFobj.h"

#include "kTypes.h"
#include "kError.h"
#include "kFile.h"
#include "kFileInterfaces.h"
#include "kFileFormatBase.h"
#include "kFileOMF.h"
#include "kList.h"
#include "kFileOMFLib.h"


void main (int argc,  char **argv)
{
    for (int argi = 1; argi < argc; argi++)
    {
        kFile *     pFile = NULL;

        try
        {
            pFile = new kFile(argv[argi]);
            kFileOMFLib Lib(pFile);
            Lib.dump(&kFile::StdOut);
        }
        catch (kError err)
        {
            if (pFile)
            {
                try
                {
                    pFile = new kFile(argv[argi]);
                    kFileOMF Obj(pFile);
                    Obj.dump(&kFile::StdOut);
                }
                catch (kError err)
                {
                    kFile::StdOut.printf("Failed to open %s - not obj/lib? err=%d\n",
                                         argv[argi],
                                         err.getErrno());
                }
            }
            else
            {
                kFile::StdOut.printf("Failed to open %s. err=%d\n",
                                     argv[argi],
                                     err.getErrno());
            }
        }
    } /* for */
}
