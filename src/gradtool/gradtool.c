/***********************************************************************
 * Name      : Module GRADD Tool
 * Funktion  : Information about GRADD chain,
 *             Configuration of GRADD drivers
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 30.10.1995 02.21.27]
 ***********************************************************************/

/* #define DEBUG */


/*===Includes================================================================*/
#define INCL_NOPMAPI
#define INCL_DOS
#define INCL_DOSERRORS    /* DOS error values    */
#define INCL_DOSFILEMGR   /* File Manager values */
#define INCL_DOSDEVICES   /* Device values    */
#define INCL_DOSDEVIOCTL
#define INCL_DOSMISC
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

#include <pmbitmap.h>
#include <svgadefs.h>
#include <gradd.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"
/*===End Includes============================================================*/


/*===Strukturen==============================================================*/
typedef struct
{
  ARGFLAG fsHelp;                 /* Muss Hilfestellung dargestellt werden ? */
  ARGFLAG fsVerbose;                                  /* produce more output */

} OPTIONS, *POPTIONS;

/*===End Strukturen==========================================================*/


/*===Prototypen==============================================================*/

/*===Globale Strukturen======================================================*/

OPTIONS Options;

ARGUMENT TabArguments[] =
{ /*Token--Beschreibung--pTarget--ucTargetFormat--pTargetSpecified--*/
  {"/?",          "Get help screen.",                      NULL,                      ARG_NULL,   &Options.fsHelp},
  {"/H",          "Get help screen.",                      NULL,                      ARG_NULL,   &Options.fsHelp},
  ARG_TERMINATE
};

/*===End Globale Strukturen==================================================*/


/***********************************************************************
 * Name      : void help
 * Funktion  : Darstellen der Hilfe
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

void help (void)
{
  TOOLVERSION("GRADD Tool",                              /* application name */
              0x00010000,                            /* application revision */
              0x00010900,           /* minimum required revision of PHSTOOLS */
              0x0001FFFF,        /* maximum recommended revision of PHSTOOLS */
              NULL,                                                /* Remark */
              NULL);                                 /* additional copyright */
}


/***********************************************************************
 * Name      : void initialize
 * Funktion  : Initialisierung einiger Variablen
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 ***********************************************************************/

void initialize (void)
{
  memset(&Options,
         0L,
         sizeof(Options));

  setvbuf(stdout,                                /* disable stream buffering */
          NULL,
          _IONBF,
          0);
}



PSZ GraddReturnCode( ULONG rc )
{
  switch (rc)
  {
    case RC_SUCCESS:                  return "OK";
    case RC_SIMULATE:                 return "simulate";
    case RC_UNSUPPORTED:              return "unsupported";
    case RC_ERROR:                    return "error";
    case RC_ERROR_IN_BACKGROUND:      return "error in background";
    case RC_ERROR_NO_HANDLE:          return "error no handle";
    case RC_ERROR_RESOURCE_NOT_FOUND: return "error resource not found";
    case RC_ERROR_INVALID_PARAMETER:  return "error invalid parameter";
    case RC_ERROR_OUT_OF_MEMORY:      return "error out of memory";
    case RC_DISABLED:                 return "disabled";
    case RC_ENABLED:                  return "enabled";
    default:                          return "(unknown)";
  }
}
            



/***********************************************************************
 * Name      : GraddDisplayChain
 * Funktion  : 
 * Parameter : 
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 ***********************************************************************/

APIRET GraddDisplayChain( void )
{
  ULONG        rc;
  VMIQCI       vmiQCI;
  PCHAININFO   pciHead;
  PGRADDINFO   pGraddList;
  ULONG        ulIndexChain = 0;
  ULONG        ulIndexGradd;
  int          i;
  ULONG        ulCaps;
  PGDDMODEINFO pMode;
    
  rc = VMIEntry((GID)NULL,/* GID  */
                VMI_CMD_QUERYCHAININFO,
                NULL,     /* pIn  */
                &vmiQCI); /* pOut */
  if (RC_SUCCESS != rc)
  {
    printf("Error: VMIQueryChainInfo failed (%s)\n",
           GraddReturnCode(rc));
    return rc;
  }
  
  /* display the chain */
  printf("Display GRADD driver chain:\n");
  pciHead = vmiQCI.pciHead;
  
  do
  {
    ulIndexChain++;
    
    printf("%3d CID            = %08xh [%s]\n",
           ulIndexChain,
           pciHead->cid,
           pciHead->pszChainName);
    
    // display HW entry
    printf("    Chain HW Entry = %08xh\n",
           pciHead->pChainHWEntry);
    
    // display GRADD chain
    
    ulIndexGradd = 0;
    pGraddList = pciHead->pGraddList;
    do
    {
      printf("   %3d GID = %08xh [%s]\n",
             ulIndexGradd,
             pGraddList->gid,
             pGraddList->pszGraddName);
      printf("       Gradd Entry = %08xh\n"
             "       Chain Entry = %08xh\n",
             pGraddList->pGraddEntry,
             pGraddList->pChainEntry);
      
      printf("       Modes       = %d\n",
             pGraddList->cModes);
      
      if (pGraddList->cModes)
      {
        printf("      Nr  Id         BPP/Colors         Mode            Aperture           Scanline VRAM\n");
        pMode = pGraddList->pModeInfo;
        for (i = 0;
             i < pGraddList->cModes;
             i++,
             pMode++)
        {
          printf("      %3d (%08x) %3d/%c%c%c%c %8d %4dx%4d@%3dhz %6dkb@%08xh %5d     %dkb\n",
                 i,
                 pMode->ulModeId,
                 pMode->ulBpp,
                 (pMode->fccColorEncoding & 0x000000ff),
                 (pMode->fccColorEncoding & 0x0000ff00) >> 8,
                 (pMode->fccColorEncoding & 0x00ff0000) >> 16,
                 (pMode->fccColorEncoding & 0xff000000) >> 24,
                 pMode->cColors,
                 pMode->ulHorizResolution,
                 pMode->ulVertResolution,
                 pMode->ulRefreshRate,
                 pMode->ulApertureSize / 1024,
                 pMode->pbVRAMPhys,
                 pMode->ulScanLineSize,
                 pMode->ulTotalVRAMSize / 1024);
        }
      }
      
      ulCaps = pGraddList->pCapsInfo->ulFCFlags;
      printf("      Capabilities = [%s] (%08xh)\n",
             pGraddList->pCapsInfo->pszFunctionClassID,
             ulCaps);
      
      if (ulCaps & GC_SEND_MEM_TO_MEM)         printf("         GRADD wants to see memory-to-memory blits\n");
      if (ulCaps & GC_SIM_SRC_PAT_ROPS)        printf("         GRADD wants to simulate 3way rops as sequence of 2way rops.\n");
      if (ulCaps & GC_ALPHA_SUPPORT)           printf("         GRADD supports alpha blending rules.\n");
      if (ulCaps & GC_SRC_STRETCH)             printf("         GRADD handles stretchblts.\n");
      if (ulCaps & GC_POLYGON_SIZE_ANY)        printf("         GRADD can handle polygon (concave or convex).\n");
      if (ulCaps & GC_CLIP)                    printf("         GRADD can handle single clip rect of polygons and source bitmaps.\n");
      if (ulCaps & GC_CLIP_COMPLEX)            printf("         GRADD can handle clipping with more than one clip rect.\n");
      if (ulCaps & GC_TEXTBLT_DOWNLOADABLE)    printf("         GRADD supports downloadable fonts.\n");
      if (ulCaps & GC_TEXTBLT_CLIPABLE)        printf("         GRADD supports clippable fonts.\n");
      if (ulCaps & GC_TEXTBLT_DS_DEVICE_FONTS) printf("         GRADD device has Hardware Fonts.\n");
      if (ulCaps & GC_SIMPLE_LINES)            printf("         GRADD handles simple lines only.\n");
      if (ulCaps & GC_SRC_CONVERT)             printf("         GRADD can do device-independent bitmap conversion itself.\n");
      if (ulCaps & GC_POLYGON_FILL)            printf("         GRADD can handle polygon fills.\n");
      
      // @@@PH
      // query user caps
      {
        USERCAPSIN  userCapsIn;
        ULONG       ulNumberOfUserCaps;
        DRIVERCAPS* pDriverCaps;
        PULONG      pUserCapsBuffer;
        int         j;
        
        userCapsIn.ulLength   = sizeof(userCapsIn);
        userCapsIn.ulFunction = QUERYCAPS;
        userCapsIn.ulSize     = 16384;
        
        pUserCapsBuffer = malloc( userCapsIn.ulSize );
        
        printf("    User capabilities:\n");
        rc = VMIEntry(pGraddList->gid,
                      VMI_CMD_USERCAPS,
                      &userCapsIn,
                      pUserCapsBuffer);
        if (RC_SUCCESS != rc)
          printf("      (failed: %s)\n",
                 GraddReturnCode(rc));
        else
        {
          // determine number of capabilities
          ulNumberOfUserCaps = *pUserCapsBuffer;
          
          pDriverCaps = (PDRIVERCAPS)( pUserCapsBuffer + 1 );
          for (j = 0;
               j < ulNumberOfUserCaps;
               j++)
          {
            PSZ  pszCapsType;
            CHAR szCurrentValue[64];
            PSZ  pszCurrentValue = szCurrentValue;
            CHAR szDefaultValue[64];
            PSZ  pszDefaultValue = szDefaultValue;
            
            switch (pDriverCaps->ulCapsType)
            {
              case CAPSTYPE_BOOLEAN:          
                pszCapsType = "boolean";
              
                if (NULL == pDriverCaps->pCurrentValue)
                  pszCurrentValue = "not supported";
                else
                  if ( *(PBOOL)(pDriverCaps->pCurrentValue) == TRUE)
                    pszCurrentValue = "true";
                  else
                    pszCurrentValue = "false";
              
              
                if (NULL == pDriverCaps->pDefaultValue)
                  pszDefaultValue = "not supported";
                else
                  if ( *(PBOOL)(pDriverCaps->pDefaultValue) == TRUE)
                    pszDefaultValue = "true";
                  else
                    pszDefaultValue = "false";
              
                break;
                
              case CAPSTYPE_AGGREGATE_INT:    
                pszCapsType = "int"; 
              
                if (NULL == pDriverCaps->pCurrentValue)
                  pszCurrentValue = "not supported";
                else
                  sprintf(szCurrentValue,
                          "%d",
                          *(PINT)(pDriverCaps->pCurrentValue));

                if (NULL == pDriverCaps->pDefaultValue)
                  pszDefaultValue = "not supported";
                else
                  sprintf(szDefaultValue,
                          "%d",
                          *(PINT)(pDriverCaps->pDefaultValue));
              
                break;
                
              case CAPSTYPE_AGGREGATE_STRING: 
                pszCapsType = "string";
              
                if (NULL == pDriverCaps->pCurrentValue)
                  pszCurrentValue = "not supported";
                else
                  pszCurrentValue = (PSZ)pDriverCaps->pCurrentValue;
              
                if (NULL == pDriverCaps->pDefaultValue)
                  pszDefaultValue = "not supported";
                else
                  pszDefaultValue = (PSZ)pDriverCaps->pDefaultValue;
                break;
                
              default: 
                pszCapsType = "unknown";
                szCurrentValue[0] = 0;
                szDefaultValue[0] = 0;
                break;
            }
            
            printf("      Capability [%s] (%s.%08xh)\n",
                   pDriverCaps->szCapsDesc,
                   (NULL == pDriverCaps->szHelpFileName) 
                     ? "(no helpfile)"
                     : pDriverCaps->szHelpFileName,
                   pDriverCaps->ulHelpId);
            printf("         Type                = %s (%d)\n"
                   "         Member size         = %d\n"
                   "         Number of members   = %d\n",
                   pszCapsType,
                   pDriverCaps->ulCapsType,
                   pDriverCaps->ulValueMemberSize,
                   pDriverCaps->ulNumValueMember);
            printf("         Value list addr     = %08xh\n"
                   "         Current value       = %s\n"
                   "         Default value       = %s\n"
                   "         Default value sup   = %s\n"
                   "         Static (reboot req) = %s\n",
                   pDriverCaps->pValueList,
                   pszCurrentValue,
                   pszDefaultValue,
                   pDriverCaps->bDefaultValueSupported ? "yes" : "no",
                   pDriverCaps->bStaticCaps            ? "yes" : "no");
                   

            pDriverCaps++;
          }
        }
        
        if (NULL != pUserCapsBuffer)
          free( pUserCapsBuffer );
      }
      
      ulIndexGradd++;
      pGraddList = pGraddList->pNextGraddInfo;
    }
    while (NULL != pGraddList);
    
    // skip to next structure
    pciHead = pciHead->pNextChainInfo;
  }
  while (NULL != pciHead);
  
  return NO_ERROR;
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
  int rc;                                                    /* RÅckgabewert */

  DosError (FERR_DISABLEHARDERR);                       /* Popups anschalten */

  initialize ();                                          /* Initialisierung */

  rc = ArgStandard (argc,                            /* CLI-Parameter parsen */
                    argv,
                    TabArguments,
                    &Options.fsHelp);
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    exit(1);                                                /* abort program */
  }

  if (Options.fsHelp)                                    /* help requested ? */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }


  DosError (FERR_DISABLEHARDERR);                       /* Popups anschalten */

  /* @@@PH */
  rc = GraddDisplayChain();

  return rc;
}
