/*****************************************************
 * EXEINFO Tool.                                     *
 * Reports details on a given executable file.       *
 * (c) 1994-95 Patrick Haller Systemtechnik          *
 *****************************************************/

/* #define DEBUG 1 */

/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#ifdef __OS2__
  #define INCL_DOSFILEMGR
  #define INCL_BASE
  #define INCL_NOPMAPI
  #include <os2.h>
#endif

#ifdef _WIN32
  #include <windows.h>
#endif

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include "toolarg.h"
#include "exeinfo.h"

#define MAXPATHLEN 260



/*****************************************************************************
 * Name      : void ExeAnalysePEOptionalNT
 * Funktion  : Dump PE-Image, OptionalNT Record information
 * Parameter : PSZ pPointer
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

void ExeAnalysePEOptionalNT  (PSZ pPtr)
{
  PEXEIMAGE_OPTIONAL_HEADER pOpt;
  ULONG                  ulCounter;                /* temporary loop counter */

  static PSZ pszTableDirectory[] =
  {
    "Export Directory",                  /* EXEIMAGE_DIRECTORY_ENTRY_EXPORT         0 */
    "Import Directory",                  /* EXEIMAGE_DIRECTORY_ENTRY_IMPORT         1 */
    "Resource Directory",                /* EXEIMAGE_DIRECTORY_ENTRY_RESOURCE       2 */
    "Exception Directory",               /* EXEIMAGE_DIRECTORY_ENTRY_EXCEPTION      3 */
    "Security Directory",                /* EXEIMAGE_DIRECTORY_ENTRY_SECURITY       4 */
    "Base Relocation Table",             /* EXEIMAGE_DIRECTORY_ENTRY_BASERELOC      5 */
    "Debug Directory",                   /* EXEIMAGE_DIRECTORY_ENTRY_DEBUG          6 */
    "Description String",                /* EXEIMAGE_DIRECTORY_ENTRY_COPYRIGHT      7 */
    "Machine Value (MIPS GP)",           /* EXEIMAGE_DIRECTORY_ENTRY_GLOBALPTR      8 */
    "TLS Directory",                     /* EXEIMAGE_DIRECTORY_ENTRY_TLS            9 */
    "Load Configuration Directory",      /* EXEIMAGE_DIRECTORY_ENTRY_LOAD_CONFIG   10 */
    "Bound Import Directory in headers", /* EXEIMAGE_DIRECTORY_ENTRY_BOUND_IMPORT  11 */
    "Import Address Table"               /* EXEIMAGE_DIRECTORY_ENTRY_IAT           12 */
  };

  pOpt = (PEXEIMAGE_OPTIONAL_HEADER)((PCHAR)pPtr + sizeof(EXEIMAGE_FILE_HEADER) + 4);

  printf ("\n      NT Optional Header record.");

  printf ("\n      Linker version               : %10u.%u",
          pOpt->MajorLinkerVersion,
          pOpt->MinorLinkerVersion);

  printf ("\n      Size of code                 : %10u -   %08xh",
          pOpt->SizeOfCode,
          pOpt->SizeOfCode);

  printf ("\n      Size of initialized data     : %10u -   %08xh",
          pOpt->SizeOfInitializedData,
          pOpt->SizeOfInitializedData);

  printf ("\n      Size of uninitialized data   : %10u -   %08xh",
          pOpt->SizeOfUninitializedData,
          pOpt->SizeOfUninitializedData);

  printf ("\n      Address of entry point       : %10u -   %08xh",
          pOpt->AddressOfEntryPoint,
          pOpt->AddressOfEntryPoint);

  printf ("\n      Base of code                 : %10u -   %08xh",
          pOpt->BaseOfCode,
          pOpt->BaseOfCode);

  printf ("\n      Base of data                 : %10u -   %08xh",
          pOpt->BaseOfData,
          pOpt->BaseOfData);

  printf ("\n      Image base                   : %10u -   %08xh",
          pOpt->ImageBase,
          pOpt->ImageBase);

  printf ("\n      Section alignment            : %10u -   %08xh",
          pOpt->SectionAlignment,
          pOpt->SectionAlignment);

  printf ("\n      File alignment               : %10u -   %08xh",
          pOpt->FileAlignment,
          pOpt->FileAlignment);

  printf ("\n      Operating system version     : %4u.%02u",
          pOpt->MajorOperatingSystemVersion,
          pOpt->MinorOperatingSystemVersion);

  printf ("\n      Image version                : %4u.%02u",
          pOpt->MajorImageVersion,
          pOpt->MinorImageVersion);

  printf ("\n      Subsystem version            : %4u.%02u",
          pOpt->MajorSubsystemVersion,
          pOpt->MinorSubsystemVersion);

  printf ("\n      Win32 version                : %4u.%02u",
          pOpt->Win32VersionValue >> 16,
          pOpt->Win32VersionValue & 0xFFFF);

  printf ("\n      Size of image                : %10u -   %08xh",
          pOpt->SizeOfImage,
          pOpt->SizeOfImage);

  printf ("\n      Size of headers              : %10u -   %08xh",
          pOpt->SizeOfHeaders,
          pOpt->SizeOfHeaders);

  printf ("\n      Checksum (from image)        : %10u -   %08xh",
          pOpt->CheckSum,
          pOpt->CheckSum);

  printf ("\n      Subsystem                    : %10u -       %04xh\n        ",
          pOpt->Subsystem,
          pOpt->Subsystem);

  switch (pOpt->Subsystem)
  {
    case EXEIMAGE_SUBSYSTEM_UNKNOWN:    printf("Unknown subsystem."); break;
    case EXEIMAGE_SUBSYSTEM_NATIVE:     printf("Image doesn't require a subsystem."); break;
    case EXEIMAGE_SUBSYSTEM_WINDOWS_GUI:printf("Image runs in the Windows GUI subsystem."); break;
    case EXEIMAGE_SUBSYSTEM_WINDOWS_CUI:printf("Image runs in the Windows character subsystem."); break;
    case EXEIMAGE_SUBSYSTEM_OS2_CUI:    printf("image runs in the OS/2 character subsystem."); break;
    case EXEIMAGE_SUBSYSTEM_POSIX_CUI:  printf("image run  in the Posix character subsystem."); break;
    case EXEIMAGE_SUBSYSTEM_RESERVED8:  printf("image run  in the reserved8 subsystem."); break;
    default: printf("(undefined subsystem)"); break;
  }

  printf ("\n      DLL Characteristics          : %10u -       %04xh",
          pOpt->DllCharacteristics,
          pOpt->DllCharacteristics);

  /* @@@PH */

  printf ("\n      Size of stack reserve        : %10u -   %08xh",
          pOpt->SizeOfStackReserve,
          pOpt->SizeOfStackReserve);

  printf ("\n      Size of stack commit         : %10u -   %08xh",
          pOpt->SizeOfStackCommit,
          pOpt->SizeOfStackCommit);

  printf ("\n      Size of heap reserve         : %10u -   %08xh",
          pOpt->SizeOfHeapReserve,
          pOpt->SizeOfHeapReserve);

  printf ("\n      Size of heap commit          : %10u -   %08xh",
          pOpt->SizeOfHeapCommit,
          pOpt->SizeOfHeapCommit);

  printf ("\n      Loader flags                 : %10u -   %08xh",
          pOpt->LoaderFlags,
          pOpt->LoaderFlags);

  /* @@@PH */

  printf ("\n      Number of rva and sizes      : %10u -   %08xh",
          pOpt->NumberOfRvaAndSizes,
          pOpt->NumberOfRvaAndSizes);

  printf ("\n\n      Data directory");

  printf ("\n        Nr. Virt Addr.  Size                Description");
  for (ulCounter = 0;
       ulCounter < pOpt->NumberOfRvaAndSizes;
       ulCounter++)
  {
    printf ("\n        %2u. %08xh %10u,%08xh  ",
            ulCounter,
            pOpt->DataDirectory[ulCounter].VirtualAddress,
            pOpt->DataDirectory[ulCounter].Size,
            pOpt->DataDirectory[ulCounter].Size);

    if (ulCounter <= EXEIMAGE_DIRECTORY_ENTRY_IAT)
      printf (pszTableDirectory[ulCounter]);
    else
      printf ("<unknown directory>");
  }
}


/*****************************************************************************
 * Name      : void ExeAnalysePEOptionalROM
 * Funktion  : Dump PE-Image, OptionalROM Record information
 * Parameter : PSZ pPointer
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

void ExeAnalysePEOptionalROM  (PSZ pPtr)
{
  PEXEIMAGE_ROM_OPTIONAL_HEADER pOpt;

  pOpt = (PEXEIMAGE_ROM_OPTIONAL_HEADER)((PCHAR)pPtr +
                                      sizeof(EXEIMAGE_FILE_HEADER) + 4);

  printf ("\n      ROM Optional Header record.");

  printf ("\n      Linker version               : %10u.%u",
          pOpt->MajorLinkerVersion,
          pOpt->MinorLinkerVersion);

  printf ("\n      Size of code                 : %10u -   %08xh",
          pOpt->SizeOfCode,
          pOpt->SizeOfCode);

  printf ("\n      Size of initialized data     : %10u -   %08xh",
          pOpt->SizeOfInitializedData,
          pOpt->SizeOfInitializedData);

  printf ("\n      Size of uninitialized data   : %10u -   %08xh",
          pOpt->SizeOfUninitializedData,
          pOpt->SizeOfUninitializedData);

  printf ("\n      Address of entry point       : %10u -   %08xh",
          pOpt->AddressOfEntryPoint,
          pOpt->AddressOfEntryPoint);

  printf ("\n      Base of code                 : %10u -   %08xh",
          pOpt->BaseOfCode,
          pOpt->BaseOfCode);

  printf ("\n      Base of data                 : %10u -   %08xh",
          pOpt->BaseOfData,
          pOpt->BaseOfData);

  printf ("\n      Base of BSS (stack ?)        : %10u -   %08xh",
          pOpt->BaseOfBss,
          pOpt->BaseOfBss);

  printf ("\n      GprMask                      : %10u -   %08xh",
          pOpt->GprMask,
          pOpt->GprMask);

  printf ("\n      CprMask #1                   : %10u -   %08xh",
          pOpt->CprMask[0],
          pOpt->CprMask[0]);

  printf ("\n      CprMask #2                   : %10u -   %08xh",
          pOpt->CprMask[1],
          pOpt->CprMask[1]);

  printf ("\n      CprMask #3                   : %10u -   %08xh",
          pOpt->CprMask[2],
          pOpt->CprMask[2]);

  printf ("\n      CprMask #4                   : %10u -   %08xh",
          pOpt->CprMask[3],
          pOpt->CprMask[3]);

  printf ("\n      GpValue                      : %10u -   %08xh",
          pOpt->GpValue,
          pOpt->GpValue);
}


/*****************************************************************************
 * Name      : void ExeAnalysePESection
 * Funktion  : Dump PE Section Information
 * Parameter : PSZ pPointer
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

void ExeAnalysePESection (PSZ pPtr)
{
  PEXEIMAGE_PE_HEADER pPE = (PEXEIMAGE_PE_HEADER)pPtr; /* map pointer to PE struct */
  ULONG            ulCounter;         /* temporary loop counter for sections */
  PEXEIMAGE_SECTION_HEADER pSec;       /* pointer to section header information */


#define PRINTINFO(var,mask,text) \
  if (var & mask) \
    printf ("\n                  %08x - " \
            text,mask);


  printf ("\n\n  Sections Section");

  for (ulCounter = 0;
       ulCounter < pPE->FileHeader.NumberOfSections;
       ulCounter++)
  {
    pSec = (PEXEIMAGE_SECTION_HEADER)
             ( (PCHAR)pPE +
               sizeof(EXEIMAGE_FILE_HEADER) +
               sizeof(UINT32) +                  /* correction for signature */
               pPE->FileHeader.SizeOfOptionalHeader +
               ulCounter * EXEIMAGE_SIZEOF_SECTION_HEADER
           );

    printf ("\n    [%-8s] Virtual Size / Physical Address : %10u - %08xh"
            "\n               Virtual  Address                : %10u - %08xh"
            "\n               Size of raw data                : %10u - %08xh"
            "\n               Pointer to raw data             : %10u - %08xh"
            "\n               Pointer to relocations          : %10u - %08xh"
            "\n               Pointer to linenumbers          : %10u - %08xh"
            "\n               Number of relocations           : %10u -     %04xh"
            "\n               Number of linenumbers           : %10u -     %04xh"
            "\n               Characteristics                 : %10u - %08xh",
            pSec->Name,
            pSec->Misc.VirtualSize,
            pSec->Misc.VirtualSize,
            pSec->VirtualAddress,
            pSec->VirtualAddress,
            pSec->SizeOfRawData,
            pSec->SizeOfRawData,
            pSec->PointerToRawData,
            pSec->PointerToRawData,
            pSec->PointerToRelocations,
            pSec->PointerToRelocations,
            pSec->PointerToLinenumbers,
            pSec->PointerToLinenumbers,
            pSec->NumberOfRelocations,
            pSec->NumberOfRelocations,
            pSec->NumberOfLinenumbers,
            pSec->NumberOfLinenumbers,
            pSec->Characteristics,
            pSec->Characteristics);

  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_TYPE_REG,    "(reserved) REG")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_TYPE_DSECT,  "(reserved) DSECT")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_TYPE_NOLOAD, "(reserved) NOLOAD")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_TYPE_GROUP,  "(reserved) GROUP")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_TYPE_NO_PAD, "(reserved) no pad")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_TYPE_COPY ,  "(reserved) COPY")

  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_CNT_CODE,              "Section contains code.")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_CNT_INITIALIZED_DATA,  "Section contains initialized data.")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_CNT_UNINITIALIZED_DATA,"Section contains uninitialized data.")

  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_LNK_OTHER, "(reserved) Linkage.")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_LNK_INFO,  "Section contains comments or some other type of information.")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_TYPE_OVER, "(reserved) Linkage.")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_LNK_REMOVE,"Section contents will not become part of image.")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_LNK_COMDAT,"Section contents comdat.")
  PRINTINFO(pSec->Characteristics,0x00002000,          "(reserved) Linkage.")

  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_MEM_PROTECTED, "(reserved) Memory is protected.")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_MEM_FARDATA,   "Memory is FAR data.")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_MEM_SYSHEAP,   "(reserved) Memory is system heap.")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_MEM_PURGEABLE, "Memory is purgeable.")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_MEM_16BIT,     "Memory is 16 bit.")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_MEM_LOCKED,    "Memory is locked.")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_MEM_PRELOAD,   "Memory is preloaded.")

  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_ALIGN_1BYTES,  "Alignment on  1 byte.")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_ALIGN_2BYTES,  "Alignment on  2 byte.")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_ALIGN_4BYTES,  "Alignment on  4 byte.")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_ALIGN_8BYTES,  "Alignment on  8 byte.")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_ALIGN_16BYTES, "Alignment on 16 byte.")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_ALIGN_32BYTES, "Alignment on 32 byte.")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_ALIGN_64BYTES, "Alignment on 64 byte.")
  PRINTINFO(pSec->Characteristics,0x00800000,              "(unused Alignment bit).")

  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_LNK_NRELOC_OVFL,  "Section contains extended relocations.")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_MEM_DISCARDABLE,  "Section can be discarded.")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_MEM_NOT_CACHED,   "Section is not cachable.")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_MEM_NOT_PAGED,    "Section is not pageable.")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_MEM_SHARED,       "Section is shareable.")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_MEM_EXECUTE,      "Section is executable.")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_MEM_READ,         "Section is readable.")
  PRINTINFO(pSec->Characteristics,EXEIMAGE_SCN_MEM_WRITE,        "Section is writeable.")
  }

#undef PRINTINFO
}


/*****************************************************************************
 * Name      : void ExeAnalyseHeaderPE
 * Funktion  : Dump PE Header Information
 * Parameter : PSZ pPointer
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

void ExeAnalyseHeaderPE (PSZ pPtr)
{
  PEXEIMAGE_PE_HEADER pPE = (PEXEIMAGE_PE_HEADER)pPtr; /* map pointer to PE struct */

  printf ("\n\nÄÄ[PE Header Information (Windows 95, Windows NT, Protected Executable)]ÄÄÄÄ"
          "\n  Magic                            :     \"%c%c%c%c\" -   %08xh",
          pPE->Signature & 0xFF,
          (pPE->Signature >> 8) & 0xFF,
          (pPE->Signature >> 16) & 0xFF,
          (pPE->Signature >> 24) & 0xFF,
          pPE->Signature);


  /***************************************************************************
   * File Header Section                                                     *
   ***************************************************************************/

  printf ("\n  File Header Section");

  printf ("\n    Machine                        : %10u -       %04xh\n      ",
          pPE->FileHeader.Machine,
          pPE->FileHeader.Machine);

  switch (pPE->FileHeader.Machine)
  {
    case EXEIMAGE_FILE_MACHINE_UNKNOWN: printf("Machine type is unknown."); break;
    case EXEIMAGE_FILE_MACHINE_I386:    printf("Intel 386."); break;
    case EXEIMAGE_FILE_MACHINE_R3000:   printf("MIPS little-endian, 0x160 big-endian"); break;
    case EXEIMAGE_FILE_MACHINE_R4000:   printf("MIPS little-endian"); break;
    case EXEIMAGE_FILE_MACHINE_R10000:  printf("MIPS little-endian"); break;
    case EXEIMAGE_FILE_MACHINE_ALPHA:   printf("Alpha_AXP"); break;
    case EXEIMAGE_FILE_MACHINE_POWERPC: printf("IBM PowerPC Little-Endian"); break;
  }

  printf ("\n    Number of sections             : %10u -       %04xh",
          pPE->FileHeader.NumberOfSections,
          pPE->FileHeader.NumberOfSections);

  /* @@@PH i hope this is correct */
  printf ("\n    Timestamp                      : %s",
          ctime(&pPE->FileHeader.TimeDateStamp));

  printf ("\n    Pointer to symbol table        : %10u -   %08xh",
          pPE->FileHeader.PointerToSymbolTable,
          pPE->FileHeader.PointerToSymbolTable);

  printf ("\n    Number of symbols              : %10u -   %08xh",
          pPE->FileHeader.NumberOfSymbols,
          pPE->FileHeader.NumberOfSymbols);

  printf ("\n    Size of optional header        : %10u -       %04xh",
          pPE->FileHeader.SizeOfOptionalHeader,
          pPE->FileHeader.SizeOfOptionalHeader);

  printf ("\n    Characteristics                : %10u -       %04xh",
          pPE->FileHeader.Characteristics,
          pPE->FileHeader.Characteristics);

  if (pPE->FileHeader.Characteristics & EXEIMAGE_FILE_RELOCS_STRIPPED)
    printf ("\n          0x0001 Relocation info stripped from file.");
  else
    printf ("\n      not 0x0001 Relocation info is in the file.");

  if (pPE->FileHeader.Characteristics & EXEIMAGE_FILE_EXECUTABLE_EXEIMAGE)
    printf ("\n          0x0002 File is executable (i.e. no unresolved externel refs.).");
  else
    printf ("\n      not 0x0002 File is not executable (i.e. unresolved externel refs.).");

  if (pPE->FileHeader.Characteristics & EXEIMAGE_FILE_LINE_NUMS_STRIPPED)
    printf ("\n          0x0004 Line numbers stripped from file.");
  else
    printf ("\n      not 0x0004 Line numbers are in the file.");

  if (pPE->FileHeader.Characteristics & EXEIMAGE_FILE_LOCAL_SYMS_STRIPPED)
    printf ("\n          0x0008 Local symbols stripped from file.");
  else
    printf ("\n      not 0x0008 Local symbols are in the file.");

  if (pPE->FileHeader.Characteristics & 0x0030)
    printf ("\n          0x0030 (unknown flags %04xh).",
            pPE->FileHeader.Characteristics & 0x0030);

  if (pPE->FileHeader.Characteristics & EXEIMAGE_FILE_BYTES_REVERSED_LO)
    printf ("\n          0x0080 Bytes of machine word are reversed.");
  else
    printf ("\n      not 0x0080 Bytes of machine word are normal.");

  if (pPE->FileHeader.Characteristics & EXEIMAGE_FILE_BYTES_REVERSED_HI)
    printf ("\n          0x8000 Bytes of machine word are reversed.");
  else
    printf ("\n      not 0x8000 Bytes of machine word are normal.");


  if (pPE->FileHeader.Characteristics & EXEIMAGE_FILE_32BIT_MACHINE)
    printf ("\n          0x0100 32 bit word machine.");
  else
    printf ("\n      not 0x0100 not a 32 bit word machine.");

  if (pPE->FileHeader.Characteristics & EXEIMAGE_FILE_DEBUG_STRIPPED)
    printf ("\n          0x0200 Debugging info stripped from file in .DBG file.");
  else
    printf ("\n      not 0x0200 Debugging info is in the file.");

  if (pPE->FileHeader.Characteristics & EXEIMAGE_FILE_REMOVABLE_RUN_FROM_SWAP)
    printf ("\n          0x0400 If Image is on removable media, copy and run from the swap file.");

  if (pPE->FileHeader.Characteristics & EXEIMAGE_FILE_NET_RUN_FROM_SWAP)
    printf ("\n          0x0800 If Image is on Net, copy and run from the swap file.");

  if (pPE->FileHeader.Characteristics & EXEIMAGE_FILE_SYSTEM)
    printf ("\n          0x1000 System File.");

  if (pPE->FileHeader.Characteristics & EXEIMAGE_FILE_DLL)
    printf ("\n          0x2000 File is a DLL.");

  if (pPE->FileHeader.Characteristics & EXEIMAGE_FILE_UP_SYSTEM_ONLY)
    printf ("\n          0x4000 File should only be run on a UP machine.");


  /***************************************************************************
   * Optional Header Section                                                 *
   ***************************************************************************/

  printf ("\n  Optional Header Section");

  printf ("\n    Magic                          : %10u -       %04xh",
          pPE->OptionalHeader.Magic,
          pPE->OptionalHeader.Magic);

  if (pPE->OptionalHeader.Magic == EXEIMAGE_NT_OPTIONAL_HDR_MAGIC)
    ExeAnalysePEOptionalNT((PSZ)pPE);
  else
    if (pPE->OptionalHeader.Magic == EXEIMAGE_ROM_OPTIONAL_HDR_MAGIC)
      ExeAnalysePEOptionalROM((PSZ)pPE);
    else
      printf ("Unknown Optional Header record.");


  /***************************************************************************
   * Sections Header Section                                                 *
   ***************************************************************************/

  ExeAnalysePESection((PSZ)pPE);


  Globals.pExePointer = NULL;                    /* signal end of processing */
}

