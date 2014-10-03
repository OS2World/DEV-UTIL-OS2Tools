/*****************************************************
 * Keyboard Set Tool.                                *
 * Configures OS/2 keyboards.                        *
 * (c) 1997 Patrick Haller Systemtechnik             *
 *****************************************************/

 /*
  * This program uses a semi-undefined mouse driver ioctl to change
  * the dynamic movement of that device.  I decided to try this after
  * browsing the DDSK CDROM source code for the mouse device driver,
  * where I saw this ioctl hidden away.  The structure that it uses
  * is,  however, documented in bsesub.h.
  *
  * Note that this program uses the ioctl defines for the Get/Set
  * "hotkey". It appears that these are no longer supported for that
  * function,  and are instead used to get/set the threshold
  * multiplier buffer.
  *
  * To change the mouse behavior of the OS/2 2.x PM desktop,  this
  * program must be run from a vio window.  It is a sample program
  * only,  with hard-coded values for the new multipliers.
  *
  * Peter Fitzsimmons, Thu  94-06-09
  */


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#define INCL_BASE
#define INCL_NOPMAPI
#define INCL_DOSDEVIOCTL
#define INCL_KBD
#define INCL_DOSERRORS
#include <os2.h>

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <process.h>


#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"

#define MAXPATHLEN 260


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsLev1Mult;                                /* 1st level multiplier */
  ARGFLAG fsLev1;                                      /* 1st movement level */
  ARGFLAG fsLev2Mult;                                /* 2nd level multiplier */
  ARGFLAG fsLev2;                                      /* 2nd movement level */

  USHORT usLev1Mult;                                 /* 1st level multiplier */
  USHORT usLev1;                                       /* 1st movement level */
  USHORT usLev2Mult;                                 /* 2nd level multiplier */
  USHORT usLev2;                                       /* 2nd movement level */

} OPTIONS, *POPTIONS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */

ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung-----------------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",          NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",          NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/L1=",       "Set level 1 trigger speed.",&Options.usLev1,      ARG_USHORT,     &Options.fsLev1},
  {"/M1=",       "Set level 1 multiplier.",   &Options.usLev1Mult,  ARG_USHORT,     &Options.fsLev1Mult},
  {"/L2=",       "Set level 2 trigger speed.",&Options.usLev2,      ARG_USHORT,     &Options.fsLev2},
  {"/M2=",       "Set level 2 multiplier.",   &Options.usLev2Mult,  ARG_USHORT,     &Options.fsLev2Mult},
  ARG_TERMINATE
};


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void   help                (void);

void   initialize          (void);

APIRET MouQueryBallistic   (HMOU       hMouse,
                            PTHRESHOLD pThreshold);

APIRET MouSetBallistic     (HMOU       hMouse,
                            PTHRESHOLD pThreshold);

APIRET MouProcess          (void);


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
  TOOLVERSION("MouseSet",                               /* application name */
              0x00010000,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */

  printf ("\nTo change your mouse ballistics of a Presentation Manager"
          "session,\nrun this program windowed.\n");
}


/***********************************************************************
 * Name      : APIRET MouQueryBallistic
 * Funktion  : Erfragt die Level-Multiplier fr die Mausballistik.
 * Parameter : HMOU hMouse, PTHRESHOLD pThreshold
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 18.09.1995 23.02.58]
 ***********************************************************************/

APIRET MouQueryBallistic (HMOU hMouse,
                          PTHRESHOLD pThreshold)
{
   APIRET rc;                                          /* Rckgabewert */
   ULONG  ulParmLen;                            /* Parameterpaketl„nge */
   ULONG  ulDataLen;                                /* Datenblockl„nge */

  if (pThreshold == NULL)                      /* Parameterberprfung */
     return (ERROR_INVALID_PARAMETER);

    /* This is the direct way to communicate with the driver via IOCtl */
  memset(pThreshold,
         0,
         sizeof(THRESHOLD));
  pThreshold->Length = sizeof(THRESHOLD);

  ulParmLen = 0;
  ulDataLen = sizeof(THRESHOLD);

  rc = DosDevIOCtl(hMouse,
                   IOCTL_POINTINGDEVICE,
                   MOU_GETHOTKEYBUTTON,
                   NULL,
                   0,
                   &ulParmLen,
                   pThreshold,
                   sizeof(THRESHOLD),
                   &ulDataLen);

        /* This is the "official" way to do it. But it fails under PM !
  rc = MouGetThreshold (pThreshold, hMouse);
  */

  return (rc);                                 /* Rckgabewert liefern */
} /* APIRET MouQueryBallistic */


/***********************************************************************
 * Name      : APIRET MouSetBallistic
 * Funktion  : Setzt die Level-Multiplier fr die Mausballistik.
 * Parameter : HMOU hMouse, PTHRESHOLD pThreshold, USHORT level 1,
 *             USHORT level 2
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 18.09.1995 23.02.58]
 ***********************************************************************/

APIRET MouSetBallistic (HMOU       hMouse,
                        PTHRESHOLD pThreshold)
{
   APIRET rc;                                          /* Rckgabewert */
   ULONG  ulParmLen;                            /* Parameterpaketl„nge */
   ULONG  ulDataLen;                                /* Datenblockl„nge */

  if (pThreshold == NULL)                      /* Parameterberprfung */
     return (ERROR_INVALID_PARAMETER);

    /* This is the direct way to communicate with the driver via IOCtl */
  ulParmLen = 0;
  ulDataLen = sizeof(THRESHOLD);

  rc = DosDevIOCtl(hMouse,
                   IOCTL_POINTINGDEVICE,
                   MOU_SETHOTKEYBUTTON,
                   pThreshold,
                   sizeof(THRESHOLD),
                   &ulParmLen,
                   NULL,
                   0,
                   &ulDataLen);

  /* This is the "official" way to do it. But it fails under PM !
  rc = MouSetThreshold (pThreshold, hMouse);
  */

  return (rc);                                 /* Rckgabewert liefern */
} /* APIRET MouQueryBallistic */


/***********************************************************************
 * Name      : APIRET MouProcess
 * Funktion  :
 * Parameter : int argc, char *argv[]
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 18.09.1995 23.22.22]
 ***********************************************************************/

APIRET MouProcess(void)
{
  HFILE      hMouse;                                /* Handle zum Kbdtreiber */
  APIRET     rc;                                  /* Rckgabewert            */
  THRESHOLD  MouThreshold;
  ULONG      ulDummy;                                         /* dummy value */

                                           /* Get access to the keybd driver */
  rc = DosOpen("\\DEV\\MOUSE$",                    /* open the device driver */
               &hMouse,
               &ulDummy,
               0L,
               FILE_NORMAL,
               FILE_OPEN,
               OPEN_ACCESS_WRITEONLY |
               OPEN_SHARE_DENYNONE |
               OPEN_FLAGS_FAIL_ON_ERROR,
               NULL);
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                                    /* raise error condition */

                                                 /* Alte Mausballistic Lesen */
  rc = MouQueryBallistic (hMouse,
                          &MouThreshold);
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                                    /* raise error condition */

  /* this is a fix to a bug in the Toolkit */
#define Lev2Mult lev2Mult

  printf ("\nÚÄCurrent Mouse ValuesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿"
       "\n³ 1st movement level: %5d               1st level multiplier: %5d         ³"
       "\n³ 2nd movement level: %5d               2nd level multiplier: %5d         ³"
       "\nÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ",
       MouThreshold.Level1,
       MouThreshold.Lev1Mult,
       MouThreshold.Level2,
       MouThreshold.Lev2Mult);

                      /* Ist Level 1 auch unbedingt kleiner-gleich Level 2 ? */
  if ( Options.fsLev1Mult && Options.fsLev2Mult )
    if (Options.usLev1Mult > Options.usLev2Mult)
    {
       USHORT usDummy;                       /* Nein, also Werte austauschen */

       usDummy            = Options.usLev1Mult;
       Options.usLev1Mult = Options.usLev2Mult;
       Options.usLev2Mult = usDummy;
       printf ("\nLevel multipliers were exchanged.");
    }


                      /* Ist Level 1 auch unbedingt kleiner-gleich Level 2 ? */
  if ( Options.fsLev1 && Options.fsLev2 )
    if (Options.usLev1 > Options.usLev2)
    {
       USHORT usDummy;                       /* Nein, also Werte austauschen */

       usDummy        = Options.usLev1;
       Options.usLev1 = Options.usLev2;
       Options.usLev2 = usDummy;
       printf ("\nMovement levels were exchanged.");
    }


  if (Options.fsLev1Mult ||              /* Sollte nur Info anzeigt werden ? */
      Options.fsLev1     ||
      Options.fsLev2Mult ||
      Options.fsLev2)
  {
                                /* Werte in die Threshold-Structur eintragen */
    if (Options.usLev1Mult) MouThreshold.Lev1Mult = Options.usLev1Mult;
    if (Options.usLev1    ) MouThreshold.Level1   = Options.usLev1;
    if (Options.usLev2Mult) MouThreshold.Lev2Mult = Options.usLev2Mult;
    if (Options.usLev2    ) MouThreshold.Level2   = Options.usLev2;

                                                        /* Neue Werte setzen */
    rc = MouSetBallistic (hMouse,
                          &MouThreshold);
    if (rc != NO_ERROR)                                  /* check for errors */
      return (rc);                                  /* raise error condition */

    printf ("\nÚÄNew Mouse ValuesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿"
         "\n³ 1st movement level: %5d               1st level multiplier: %5d         ³"
         "\n³ 2nd movement level: %5d               2nd level multiplier: %5d         ³"
         "\nÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ",
         MouThreshold.Level1,
         MouThreshold.Lev1Mult,
         MouThreshold.Level2,
         MouThreshold.Lev2Mult);
  }

  DosClose (hMouse);

  return NO_ERROR;
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
  int rc;                                                    /* Rckgabewert */

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

  if ( Options.fsHelp )                      /* check if user specified file */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  rc = MouProcess();                                          /* do the work */
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  return (rc);
}
