/*****************************************************************************
 * Projekt   : OS2TOOLS General C Library
 * Name      : Modul os2types
 * Funktion  : Datentypen
 *
 * Autor     : Patrick Haller [Dienstag, 26.09.1995 17.57.39]
 *****************************************************************************/

#ifndef MODULE_OS2TOOLS_TYPES
#define MODULE_OS2TOOLS_TYPES

#ifdef __cplusplus
      extern "C" {
#endif


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/
        
#pragma pack(1)
        
        
#ifdef __OS2__
  #define INCL_BASE
  #define INCL_DOS
  #define INCL_DOSERRORS
  #include <os2.h>
#endif


/*****************************************************************************
 * Definitions                                                               *
 *****************************************************************************/

#ifndef __OS2__

typedef unsigned char     UINT8;
typedef unsigned short    UINT16;
typedef unsigned long int UINT32;
typedef signed char       INT8;
typedef signed short      INT16;
typedef signed long int   INT32;

typedef UINT8  *PUINT8;
typedef UINT16 *PUINT16;
typedef UINT32 *PUINT32;
typedef INT8   *PINT8;
typedef INT16  *PINT16;
typedef INT32  *PINT32;

typedef PUINT8  *PPUINT8;
typedef PUINT16 *PPUINT16;
typedef PUINT32 *PPUINT32;
typedef PINT8   *PPINT8;
typedef PINT16  *PPINT16;
typedef PINT32  *PPINT32;

typedef UINT8 UCHAR;
typedef UCHAR *PUCHAR;
typedef INT8  CHAR;
typedef CHAR *PCHAR;
typedef UINT16 USHORT;
typedef USHORT *PUSHORT;
typedef INT16 SHORT;
typedef SHORT *PSHORT;
typedef UINT32 ULONG;
typedef ULONG *PULONG;
typedef INT32 LONG;
typedef LONG *PLONG;

typedef char *PSZ;

typedef UINT32 BOOL;
typedef void *PVOID;
typedef PVOID *PPVOID;

typedef struct
{
  USHORT year;
  USHORT month;
  USHORT day;
} FDATE, *PFDATE;

typedef struct
{
  USHORT hours;
  USHORT minutes;
  USHORT twosecs;
} FTIME, *PFTIME;

typedef UINT32 APIRET;

#endif


#ifndef _WIN32
typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef void *LPVOID;
#endif


#ifndef NULL
  #define NULL 0L
#endif

typedef PSZ *PPSZ;


/*****************************************************************************
 * Definitions                                                               *
 *****************************************************************************/

#ifdef __OS2__
  #ifdef __BORLANDC__
    #define TOOLAPI __stdcall
  #endif

  #ifdef __IBMCPP__
    #define TOOLAPI _Optlink
  #endif

  #ifdef __IBMC__
    #define TOOLAPI _Optlink
  #endif
#endif

#ifdef _WIN32
  #define TOOLAPI WINAPI
#endif


/* determine which compiler was used */
#ifdef __BORLANDC__
#define __TOOLSCOMPILER__ "Borland C++"
#endif

#if defined(__IBMC__) || (__IBMCPP__)
#define __TOOLSCOMPILER__ "IBM VAC++"
#endif

#ifdef _MSC_VER
#define __TOOLSCOMPILER__ "MS Visual C"
#endif

#ifndef __TOOLSCOMPILER__
#define __TOOLSCOMPILER__ "unknown compiler"
#endif


/* determine which operating system is target platform */
#ifdef __OS2__
#define __TOOLSOS__ "OS/2"
#endif

#ifdef _WIN32
#define __TOOLSOS__ "Win32"
#endif

#ifndef __TOOLSOS__
#define __TOOLSOS__ "unknown OS"
#endif

#define __TOOLSREVISION__ "built "__DATE__", "__TIME__ \
        " with "__TOOLSCOMPILER__" for "__TOOLSOS__"."
#define __TOOLSCOPYRIGHT__ "(c) 1994-2002 Patrick Haller"

#define __TOOLSVERSION__ 0x00011100
#define __iTOOLSREVISION__ "OS2Tools     1.11.00 "__TOOLSREVISION__

#ifdef __cplusplus
      }
#endif


#endif /* MODULE_OS2TOOLS_TYPES */

