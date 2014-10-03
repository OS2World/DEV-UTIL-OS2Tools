/*****************************************************
 * FixACPs Tool                                      *
 * Checks existing ACPs                              *
 * (c) 1997    Patrick Haller Systemtechnik          *
 *****************************************************/

/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#ifdef __OS2__
  #define INCL_DOSFILEMGR
  #define INCL_BASE
  #define INCL_NOPMAPI
  #include <os2.h>

  #define PURE_32
  #include <netcons.h>                               /* Lan Server Constants */
  #include <access.h>                                   /* Lan Server Access */
  #include <neterr.h>                              /* Lan Server Error Codes */
  #include <process.h>
  #include <wksta.h>

  #define NETRET API_RET_TYPE
#endif

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"

#ifndef MAXPATHLEN
  #define MAXPATHLEN 260
#endif


/*****************************************************************************
 * Defines                                                                   *
 *****************************************************************************/

/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsNoPrompt;       /* don't prompt for access control list deletion */
  ARGFLAG fsList;    /* only list the ACPs marked for deletion, don't delete */
  ARGFLAG fsVerbose;                                       /* verbose output */

  ARGFLAG fsServer;                                 /* server name specified */
  PSZ     pszServer;                                    /* the server's name */
} OPTIONS, *POPTIONS;


typedef struct _Globals
{
  int iDummy;
} GLOBALS, *PGLOBALS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */
GLOBALS Globals;

ARGUMENT TabArguments[] =
{ /*Token-----------Beschreibung--------pTarget--------------------ucTargetFormat--pTargetSpecified--*/
  {"/?",            "Get help screen.", NULL,                      ARG_NULL,       &Options.fsHelp},
  {"/H",            "Get help screen.", NULL,                      ARG_NULL,       &Options.fsHelp},
  {"/V",            "Verbose output.",  NULL,                      ARG_NULL,       &Options.fsVerbose},
  {"/Y",            "Does not prompt for deletion.", NULL,         ARG_NULL,       &Options.fsNoPrompt},
  {"/LIST",         "Only list ACPs marked for deletion.", NULL,   ARG_NULL,       &Options.fsList},
  {"1",             "Server name.",     &Options.pszServer,        ARG_PSZ |
                                                                   ARG_DEFAULT,    &Options.fsServer},

  ARG_TERMINATE
};



/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void   help                  (void);

APIRET initialize            (void);

int    main                  (int,
                              char **);


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
  TOOLVERSION("FixACPs",                                /* application name */
              0x00000100,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


/***********************************************************************
 * Name      : APIRET FixACPs
 * Funktion  : Add the user
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung : IBM docs recommend to do the recursion on our own.
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 ***********************************************************************/

typedef struct access_info_0 ACCESSINFO0;
typedef ACCESSINFO0 *PACCESSINFO0;
typedef struct access_info_1 ACCESSINFO1;
typedef ACCESSINFO1 *PACCESSINFO1;
typedef struct access_list ACCESSLIST;
typedef ACCESSLIST *PACCESSLIST;


#ifndef MAXPATHLEN
#define MAXPATHLEN 260
#endif

APIRET ACPRemove(ULONG ulCount,
                 PSZ   pszResource)
{
  NETRET      rc;                                      /* NET_API returncode */
  FILESTATUS3 fsStatus3;                           /* for DosQueryPathInfo() */


    /* skip known resources */
    if ( (pszResource[1] == ':') &&                      /* skip drive resources */
         (pszResource[2] == 0) )
      return (NO_ERROR);

    if (pszResource[0] == '\\')                            /* \COMM \PRINT \PIPE */
      return (NO_ERROR);


    rc = DosQueryPathInfo(pszResource,                      /* query full name */
                          FIL_STANDARD,
                          &fsStatus3,
                          sizeof(fsStatus3));
    switch (rc)
    {
      case ERROR_PATH_NOT_FOUND:
      case ERROR_FILE_NOT_FOUND:
        {
          printf ("DEL %4u: %s\n",
                  ulCount,
                  pszResource);

          if (Options.fsList)                                 /* list only ? */
            break;

          if (!Options.fsNoPrompt)
          {
            int iAnswer;

            iAnswer = ToolsConfirmationQuery();              /* ask the user */
            switch (iAnswer)
            {
              case 0:                                                  /* no */
                return (NO_ERROR);                       /* abort processing */

              case 1:                                                 /* yes */
                break;                                       /* continue ... */

              case 2:                                              /* escape */
                exit (1);                 /* PH: urgs, terminate the process */
            }
          }

          rc = Net32AccessDel(Options.pszServer,      /* delete the resource */
                              pszResource);
          if (rc != NO_ERROR)                            /* check for errors */
            printf ("          error %u occured.\n",rc);

        }
        break;


      default:
        printf ("OK  %4u: %s\n",
                ulCount,
                pszResource);
        break;
    }

  return (rc);
}


/***********************************************************************
 * Name      : APIRET FixACPs
 * Funktion  : Add the user
 * Parameter : void
 * Variablen :
 * Ergebnis  :
 * Bemerkung : IBM docs recommend to do the recursion on our own.
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.52.41]
 ***********************************************************************/


APIRET FixACPs (PSZ pszRoot)
{
  NETRET rc;                                           /* NET_API returncode */
  ULONG  ulEntriesReturned;                      /* number of ACPs delivered */
  ULONG  ulEntriesAvailable;                         /* total number of ACPs */
  ULONG  ulCount;                                            /* loop counter */
  ULONG  ulCount2;                                           /* loop counter */

  ULONG        ulACPBufferSize;                            /* size of buffer */
  PACCESSINFO0 pACPBuffer;                                     /* the buffer */
  PSZ          pACPTemp;                                       /* the buffer */
  PSZ          pszTemp;                          /* temporary string pointer */

/*
  printf ("FixACPs(%s)",
          pszRoot);
  */

                         /* determine number of ACPs in the control database */
  rc = Net32AccessEnum(Options.pszServer,                       /* pszServer */
                       pszRoot,                               /* pszResource */
                       TRUE,                             /* recursive on/off */
                       0,                                     /* query level */
                       NULL,                                       /* buffer */
                       0,                                   /* buffer length */
                       &ulEntriesReturned,
                       &ulEntriesAvailable);
  if (rc != NO_ERROR) /* check errors */
    //  return (rc);
    ;

/*
  printf("rc=%u\n",rc);
  printf("Entries returned : %u\n", ulEntriesReturned);
  printf("Entries available: %u\n", ulEntriesAvailable);

*/
                                 /* allocate buffer and retrieve all entries */
  ulACPBufferSize = ulEntriesAvailable *
                    (sizeof(ACCESSINFO0) + MAXPATHLEN);
  pACPBuffer = malloc(ulACPBufferSize);
  if (pACPBuffer == NULL)                                    /* check errors */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */

  rc = Net32AccessEnum(Options.pszServer,                       /* pszServer */
                       pszRoot,                               /* pszResource */
                       TRUE,                             /* recursive on/off */
                       0,                                     /* query level */
                       (PSZ)pACPBuffer,                            /* buffer */
                       ulACPBufferSize,                     /* buffer length */
                       &ulEntriesReturned,
                       &ulEntriesAvailable);
  if (rc != NO_ERROR) /* check errors */
    //  return (rc);
    ;

/*
  printf("rc=%u\n",rc);
  printf("Entries returned : %u\n", ulEntriesReturned);
  printf("Entries available: %u\n", ulEntriesAvailable);
*/

  /* OK, now loop over all retrieved entries */
  for (ulCount = 0,
       pACPTemp = (PSZ)pACPBuffer;

       ulCount < ulEntriesReturned;

       ulCount++,
       pACPTemp += sizeof(ACCESSINFO0) )

  {
    PACCESSINFO0 pACPResource;

    pACPResource = (PACCESSINFO0)pACPTemp;                   /* convert pointer */

#if 0
    PACCESSLIST  pACPList;

    for (ulCount2 = 0,
         pACPList = pACPTemp + sizeof(ACCESSINFO1);

         ulCount2 < pACPResource->acc1_count;

         ulCount2++,
         pACPTemp += sizeof(ACCESSLIST),
         pACPList++
        )
      if (Options.fsVerbose)                     /* verbose output desired ? */
        printf ("      %-50s %04xh\n",
                pACPList->acl_ugname,
                pACPList->acl_access);
#endif

    /* now check the ACP */
    pszTemp = pACPResource->acc0_resource_name;

    rc = ACPRemove(ulCount,
                   pszTemp);          /* check & remove the ACL if necessary */
  }


  free (pACPBuffer);                                          /* free buffer */

  return (rc);
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

APIRET initialize (void)
{
  memset(&Options,
         0L,
         sizeof(Options));

  memset(&Globals,
         0L,
         sizeof(Globals));

  return (NO_ERROR);                                                   /* OK */
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

  rc = initialize ();                                     /* Initialisierung */
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    exit(1);                                                /* abort program */
  }

  rc = ArgStandard (argc,                          /* CLI-Parameter parsen */
                    argv,
                    TabArguments,
                    &Options.fsHelp);
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    exit(1);                                                /* abort program */
  }

  if ( Options.fsHelp )                                /* user requests help */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  rc = FixACPs(NULL);                            /* this is our main routine */
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  return (rc);
}
