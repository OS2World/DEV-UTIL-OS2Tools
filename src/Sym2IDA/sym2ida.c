/*****************************************************
 * Symbolic Debug Information to IDA                 *
 * (c) 1998    Patrick Haller Systemtechnik          *
 *****************************************************/

#ifdef __OS2__
  #define INCL_DOS
  #define INCL_DOSERRORS
  #define INCL_NOPM
  #include <os2.h>
#endif

#ifdef _WIN32
  #include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"

#include "sym.h"


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsVerbose;                                      /* verbose display */
  ARGFLAG fsFileInput;                               /* input file specified */

  PSZ     pszFileInput;                                    /* input filename */
} OPTIONS, *POPTIONS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/
OPTIONS Options;                /* this structure holds command line options */

ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung------------------pTarget----------------ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",           NULL,                  ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",           NULL,                  ARG_NULL,       &Options.fsHelp},
  {"/V",         "Verbose display.",           NULL,                  ARG_NULL,       &Options.fsVerbose},
  {"1",          "Input filename.",            &Options.pszFileInput, ARG_PSZ  |
                                                                      ARG_MUST |
                                                                      ARG_DEFAULT,    &Options.fsFileInput},
  ARG_TERMINATE
};


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void help (void);
int  main (int argc, char *argv[]);


/***********************************************************************
 * Name      : void help
 * Funktion  : Anzeigen des Hilfetextes.
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.45.24]
 ***********************************************************************/

void help (void)
{
  TOOLVERSION("Sym2Ida",                                /* application name */
              0x00010000,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


/***********************************************************************
 * Name      : APIRET ConvertSymbolsToIDA
 * Funktion  : Konvertiert CodeVIEW4-Debuginformation nach
 *             IDA-IDC'style (Interactive Disassembler Command)
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.45.24]
 ***********************************************************************/

#define DUMPSTRUC 1

typedef MAPDEF*      PMAPDEF;
typedef LAST_MAPDEF* PLASTMAPDEF;
typedef SEGDEF*      PSEGDEF;
typedef SYMDEF32*    PSYMDEF32;
typedef SYMDEF16*    PSYMDEF16;


void DumpMap(PMAPDEF pMapDef)
{
  char szModuleName[256];

  strncpy (szModuleName,
           pMapDef->achModName,
           pMapDef->cbModName);
  szModuleName[pMapDef->cbModName] = 0;         /* active string termination */

  printf ("\n"
          "// *******************************\n"
          "// * Module %-20s *\n"
          "// *******************************\n"
          "\n",
          szModuleName);

  printf ("  // maximum symbol length is %u characters.\n"
          "  // flags: %02u, %u segments, %u constants.\n",
          pMapDef->cbMaxSym,
          pMapDef->bFlags,
          pMapDef->cSegs,
          pMapDef->cConsts);

#if 0
  printf("\n"
         "// unsigned short int ppNextMap =0x%04xh;\t/* paragraph pointer to next map      */\n"
         "// unsigned char      bFlags    =0x%02xh;  \t/* symbol types                       */\n"
         "// unsigned char      bReserved1=0x%04xh;\t/* reserved                           */\n"
         "// unsigned short int pSegEntry =0x%04xh;\t/* segment entry point value          */\n"
         "// unsigned short int cConsts   =0x%04xh;\t/* count of constants in map          */\n"
         "// unsigned short int pConstDef =0x%04xh;\t/* pointer to constant chain          */\n"
         "// unsigned short int cSegs     =0x%04xh;\t/* count of segments in map           */\n"
         "// unsigned short int ppSegDef  =0x%04xh;\t/* paragraph pointer to first segment */\n"
         "// unsigned char      cbMaxSym  =0x%02xh;  \t/* maximum symbol-name length         */\n"
         "\n",
         pMapDef->ppNextMap,
         pMapDef->bFlags,
         pMapDef->bReserved1,
         pMapDef->pSegEntry,
         pMapDef->cConsts,
         pMapDef->pConstDef,
         pMapDef->cSegs,
         pMapDef->ppSegDef,
         pMapDef->cbMaxSym);
#endif

}


void DumpLastMap(PLASTMAPDEF pLastMapDef)
{
  printf ("\n  // Symbolic Debug Information Version %u.%02u"
          "\n",
          pLastMapDef->version,
          pLastMapDef->release);
#if 0
  printf("\n"
         "// unsigned short int ppNextMap =0x%04xh;\t/* always zero                        */\n"
         "// unsigned char      release   =0x%02xh;  \t/* release number (minor version num) */\n"
         "// unsigned char      version   =0x%04xh;\t/* major version number               */\n"
         "\n",
         pLastMapDef->ppNextMap,
         pLastMapDef->release,
         pLastMapDef->version);
#endif
}


void SegmentQueryName(PSEGDEF pSegDef,
                      PSZ     pszTarget)
{
  strncpy (pszTarget,
           pSegDef->achSegName,
           pSegDef->cbSegName);
  pszTarget[pSegDef->cbSegName] = 0;        /* active string termination */
}


void DumpSegment(ULONG   ulSegmentNumber,
                 PSEGDEF pSegDef)
{
  char szSegmentName[256];

  SegmentQueryName(pSegDef,
                   szSegmentName);

  printf ("// Segment #%02u: %-20s  (%5u symbols, flags: %02x, internal #%u)\n",
          ulSegmentNumber,
          szSegmentName,
          pSegDef->cSymbols,
          pSegDef->bFlags,
          pSegDef->wReserved1);

#if 0
    printf("  // unsigned short int ppNextSeg  =0x%04xh;\t/* paragraph pointer to next segment     */\n"
           "  // unsigned short int cSymbols   =0x%04xh;\t/* count of symbols in list              */\n"
           "  // unsigned short int pSymDef    =0x%04xh;\t/* offset of symbol chain                */\n"
           "  // unsigned short int wReserved1 =0x%04xh;\t/* reserved                              */\n"
           "  // unsigned short int wReserved2 =0x%04xh;\t/* reserved                              */\n"
           "  // unsigned short int wReserved3 =0x%04xh;\t/* reserved                              */\n"
           "  // unsigned short int wReserved4 =0x%04xh;\t/* reserved                              */\n"
           "  // unsigned char      bFlags     =0x%02xh;  \t/* symbol types                          */\n"
           "  // unsigned char      bReserved1 =0x%02xh;  \t/* reserved                              */\n"
           "  // unsigned short int ppLineDef  =0x%04xh;\t/* offset of line number record          */\n"
           "  // unsigned char      bReserved2 =0x%02xh;  \t/* reserved                              */\n"
           "  // unsigned char      bReserved3 =0x%02xh;  \t/* reserved                              */\n",
           pSegDef->ppNextSeg,
           pSegDef->cSymbols,
           pSegDef->pSymDef,
           pSegDef->wReserved1,
           pSegDef->wReserved2,
           pSegDef->wReserved3,
           pSegDef->wReserved4,
           pSegDef->bFlags,
           pSegDef->bReserved1,
           pSegDef->ppLineDef,
           pSegDef->bReserved2,
           pSegDef->bReserved3);
#endif
}


APIRET ConvertSymbolsToIDA (void)
{
  APIRET rc;                                               /* API Returncode */

  PUCHAR pucFileBuffer;                            /* pointer to file buffer */
  ULONG  ulFileBufferSize;                  /* size of allocated file buffer */

  PUCHAR  pucFilePointer;             /* current "read" position within file */
  PUCHAR  pucTemp;

  PMAPDEF pMapDef;                              /* pointer to map definition */
  ULONG   ulMapDefs;                            /* number of map definitions */

  SEGDEF   *pSegDef;
  SYMDEF32 *pSymDef32;
  SYMDEF16 *pSymDef16;

  char    szBuffer[256];

  int     SegNum,
          SymNum;

  unsigned int SegOffset,
               SymOffset,
               SymPtrOffset;


  rc = ToolsReadFileToBuffer (Options.pszFileInput,       /* memory map file */
                              (PPVOID)&pucFileBuffer,
                              &ulFileBufferSize);
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);

  pucFilePointer = pucFileBuffer;                                /* "rewind" */


  /****************************
   * process all map sections *
   ****************************/

  pMapDef = (PMAPDEF)pucFileBuffer;                        /* "fread Mapdef" */

  for (ulMapDefs = 0;
       pMapDef->ppNextMap != 0;
       ulMapDefs++)
  {
    DumpMap(pMapDef);
    pucFilePointer += (pMapDef->ppNextMap * 16);
    pMapDef = (PMAPDEF)pucFilePointer;
  }

  DumpLastMap((PLASTMAPDEF)pMapDef);


  /*******************************
   * write IDC header            *
   *******************************/

  printf ("\n"
          "#include <idc.idc>\n"
          "\n");

  /*
          "static main(void)\n"
          "{\n"
          "  auto s;\n"
          "  s = FirstSeg();\n");
*/

  /********************************
   * process the segment sections *
   ********************************/

  pMapDef = (PMAPDEF)pucFileBuffer;                        /* "fread Mapdef" */
  pucFilePointer = pucFileBuffer;

  for (ulMapDefs = 0;
       pMapDef->ppNextMap != 0;
       ulMapDefs++)
  {
    SegOffset = pMapDef->ppSegDef * 16;

    /* now process the segment */
    for (SegNum = 0;
         SegNum < pMapDef->cSegs;
         SegNum++)
    {
      pSegDef = (PSEGDEF)(pucFileBuffer + SegOffset);

      DumpSegment(SegNum,
                  pSegDef);

      SegOffset = pSegDef->ppNextSeg * 16;
    }

    pucFilePointer += (pMapDef->ppNextMap * 16);
    pMapDef = (PMAPDEF)pucFilePointer;
  }


  /*******************************
   * process the symbol section  *
   *******************************/

  pMapDef = (PMAPDEF)pucFileBuffer;                        /* "fread Mapdef" */
  pucFilePointer = pucFileBuffer;

  for (ulMapDefs = 0;
       pMapDef->ppNextMap != 0;
       ulMapDefs++)
  {
    SegOffset = pMapDef->ppSegDef * 16;

    /* now process the segment */
    for (SegNum = 0;
         SegNum < pMapDef->cSegs;
         SegNum++)
    {
      pSegDef = (PSEGDEF)(pucFileBuffer + SegOffset);

      DumpSegment(SegNum,
                  pSegDef);


      SegmentQueryName(pSegDef,
                       szBuffer);

      /* procedure header */
      printf ("\n"
              "static Seg%04x(s)\n"
              "{\n"
              "  SegRename(s, \"%s\");\n",
              SegNum,
              szBuffer);

      for (SymNum = 0;
           SymNum < pSegDef->cSymbols;
           SymNum++)
      {
        /* calculate symbol pointer offset */
        pucTemp = ( (PUCHAR) pSegDef +
                   pSegDef->pSymDef +
                   SymNum * sizeof(USHORT) );

        SymOffset = *(PUSHORT)pucTemp;            /* calculate symbol offset */

        pucTemp = (PUCHAR)pSegDef + SymOffset;

/*
        fprintf (stderr,
                 "DEBUG: SymOffset = %5u pucTemp(%u) = %08xh\n",
                 SymOffset,
                 SymNum,
                 pucTemp);
*/
        if (pSegDef->bFlags & 0x01)        /* check segment definition flags */
        {
          pSymDef32 = (PSYMDEF32)pucTemp;

          memcpy(szBuffer,
                 pSymDef32->achSymName,
                 pSymDef32->cbSymName);
          szBuffer[pSymDef32->cbSymName]=0x00;         /* string termination */

          printf("  MakeName(s+0x%p,\"%s\");\n",
                 pSymDef32->wSymVal,
                 szBuffer);
        }
        else
        {
          pSymDef16 = (PSYMDEF16)pucTemp;

          memcpy(szBuffer,
                 pSymDef16->achSymName,
                 pSymDef16->cbSymName);
          szBuffer[pSymDef16->cbSymName]=0x00;         /* string termination */

          printf("  MakeName(s+0x%p,\"%s\");\n",
                 pSymDef16->wSymVal,
                 szBuffer);
        } /* endif */
      }

      /* procedure footer */
      printf ("}\n"
              "\n");

      SegOffset = (pSegDef->ppNextSeg * 16);
    }

    pucFilePointer += (pMapDef->ppNextMap * 16);
    pMapDef = (PMAPDEF)pucFilePointer;
  }


  /*******************************
   * write main routine          *
   *******************************/

  pMapDef = (PMAPDEF)pucFileBuffer;                        /* "fread Mapdef" */
  pucFilePointer = pucFileBuffer;

  printf("\n"
         "static main()\n"
         "{\n"
         "  auto s;\n"
         "\n"
         "  s = FirstSeg();\n"
         "\n");

  for (ulMapDefs = 0;
       pMapDef->ppNextMap != 0;
       ulMapDefs++)
  {
    SegOffset = pMapDef->ppSegDef * 16;

    /* now process the segment */
    for (SegNum = 0;
         SegNum < pMapDef->cSegs;
         SegNum++)
    {
      pSegDef = (PSEGDEF)(pucFileBuffer + SegOffset);

      SegmentQueryName(pSegDef,
                       szBuffer);

      /* procedure header */
      printf ("  Seg%04x(s); \ts=NextSeg(s); \t// call %s\n",
              SegNum,
              szBuffer,
              szBuffer);

      SegOffset = (pSegDef->ppNextSeg * 16);
    }

    pucFilePointer += (pMapDef->ppNextMap * 16);
    pMapDef = (PMAPDEF)pucFilePointer;
  }

  printf ("}\n");


  return 0;

#if 0
    for (SymNum=0;
         SymNum<pSegDef->cSymbols;
         SymNum++)
    {
    }

    SegOffset=NEXTSEGDEFOFFSET( (*pSegDef) );
    printf("  s=NextSeg(s);\n");
  } /* endwhile */

  printf("}\n");
#endif

  return (NO_ERROR);
}


/***********************************************************************
 * Name      : int main
 * Funktion  : Hauptroutine
 * Parameter : int argc, char *argv[]
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 ***********************************************************************/

int main (int argc, char *argv[])
{
  APIRET rc;                                               /* API returncode */

  rc = ArgStandard (argc,                            /* CLI-Parameter parsen */
                    argv,
                    TabArguments,
                    &Options.fsHelp);
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    exit(1);                                                /* abort program */
  }

  if ( Options.fsHelp)                                 /* user requests help */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  rc = ConvertSymbolsToIDA();                      /* OK, let's do some work */
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);                                /* print error message */

  return rc;
}
