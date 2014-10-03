#define INCL_DOS
#include <os2.h>

/* Prototypes */
   APIRET  APIENTRY        OS2AliasPerfCtrs(ULONG ulRangeType,
                                            ULONG ulInfo,
                                            PBYTE *ppbRangeStart,
                                            ULONG *pulRangeSize);

   APIRET  APIENTRY        OS2ConfigurePerf(ULONG ulEntityType,
                                            ULONG ulConfigType,
                                            ULONG ulInfo1,
                                            ULONG ulInfo2,
                                            PSZ pszConfigSpec,
                                            BOOL32 fExclude);

APIRET  APIENTRY        Dos32AliasPerfCtrs(ULONG ulRangeType,
                                           ULONG ulInfo,
                                           PBYTE *ppbRangeStart,
                                           ULONG *pulRangeSize)
{
  APIRET rc;
  
  rc = OS2AliasPerfCtrs(ulRangeType,
                        ulInfo,
                        ppbRangeStart,
                        pulRangeSize);
  
  /* break */
  
  return (rc);
}

APIRET  APIENTRY      Dos32ConfigurePerf(ULONG  ulEntityType,
                                         ULONG  ulConfigType,
                                         ULONG  ulInfo1,
                                         ULONG  ulInfo2,
                                         PSZ    pszConfigSpec,
                                         BOOL32 fExclude)
{
  APIRET rc;
  
  rc = OS2ConfigurePerf(ulEntityType,
                        ulConfigType,
                        ulInfo1,
                        ulInfo2,
                        pszConfigSpec,
                        fExclude);
  
  return (rc);
}

