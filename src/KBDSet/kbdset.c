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

#ifdef __OS2__
  #define INCL_DOS
  #define INCL_NOPMAPI
  #define INCL_DOSDEVIOCTL
  #define INCL_KBD
  #define INCL_DOSERRORS
  #include <os2.h>
#endif

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"

#define MAXPATHLEN 260


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct _KbdCodepage
{
  USHORT usDataLength;
  USHORT usKbdCP;
  CHAR   szCountry[64];                         /* to reserve enough space */
  CHAR   szSubCountry[64];
} KBDCODEPAGE, *PKBDCODEPAGE;

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsCapslockOn;                        /* the states of the CAPSLOCK */
  ARGFLAG fsCapslockOff;
  ARGFLAG fsScrolllockOn;                    /* the states of the SCROLLLOCK */
  ARGFLAG fsScrolllockOff;
  ARGFLAG fsNumlockOn;                          /* the states of the NUMLOCK */
  ARGFLAG fsNumlockOff;
} OPTIONS, *POPTIONS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */

ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung--------------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",       NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",       NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/CAPS-",     "Turn CAPSLOCK off.",     NULL,                 ARG_NULL,       &Options.fsCapslockOff},
  {"/CAPS+",     "Turn CAPSLOCK on.",      NULL,                 ARG_NULL,       &Options.fsCapslockOn},
  {"/SCROLL-",   "Turn SCROLLLOCK off.",   NULL,                 ARG_NULL,       &Options.fsScrolllockOff},
  {"/SCROLL+",   "Turn SCROLLLOCK on.",    NULL,                 ARG_NULL,       &Options.fsScrolllockOn},
  {"/NUM-",      "Turn NUMLOCK off.",      NULL,                 ARG_NULL,       &Options.fsNumlockOff},
  {"/NUM+",      "Turn NUMLOCK on.",       NULL,                 ARG_NULL,       &Options.fsNumlockOn},
  ARG_TERMINATE
};

/*
 - typematic rate
 - hotkeys
 - mode ascii/binary
 */

/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void   help                (void);

void   initialize          (void);

APIRET KbdQueryState       (HKBD        hKbd,
                            PSHIFTSTATE pKbdShiftState);

APIRET KbdSetState         (HKBD        hKbd,
                            PSHIFTSTATE pKbdShiftState);

void   KbdShowState        (USHORT      fsState,
                            PSZ         pszField,
                            USHORT      usMask);

APIRET KbdProcess          (void);


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
  TOOLVERSION("KBDSet",                                 /* application name */
              0x00010001,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */

  printf ("\n\nAs OS/2's Presentation Manager keeps the keyboard locked for"
          " it's\nPM-session(s), it is impossible to switch the keyboard"
          " state in PM-sessions,\nnor can the keyboard lights be switched"
          " when running this program in a windowed\nsession altough the"
          " keyboard state is definately switched for non-PM-sessions.\n");
}


/***********************************************************************
 * Name      : APIRET KbdQueryState
 * Funktion  : Erfragt den Keyboardstatus via IOCtl
 * Parameter : HKBD hKbd, PSHIFTSTATE pKbdShiftState
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 18.09.1995 23.02.58]
 ***********************************************************************/

APIRET KbdQueryState  (HKBD        hKbd,
                       PSHIFTSTATE pKbdShiftState)
{
   APIRET rc;                                          /* Rckgabewert */
   ULONG  ulParmLen;                            /* Parameterpaketl„nge */
   ULONG  ulDataLen;                                /* Datenblockl„nge */

  if (pKbdShiftState == NULL)                        /* Parameterberprfung */
     return (ERROR_INVALID_PARAMETER);

    /* This is the direct way to communicate with the driver via IOCtl */
  memset(pKbdShiftState,
         0,
         sizeof(SHIFTSTATE));

  ulParmLen = 0;
  ulDataLen = sizeof(SHIFTSTATE);

  rc = DosDevIOCtl(hKbd,
                   IOCTL_KEYBOARD,
                   KBD_GETSHIFTSTATE,
                   NULL,
                   0,
                   &ulParmLen,
                   pKbdShiftState,
                   sizeof(SHIFTSTATE),
                   &ulDataLen);

  return (rc);                                 /* Rckgabewert liefern */
} /* APIRET KbdQueryState */


/***********************************************************************
 * Name      : APIRET KbdQueryKeyboardType
 * Funktion  : Erfragt den Keyboardtyp via IOCtl
 * Parameter : HKBD hKbd
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 18.09.1995 23.02.58]
 ***********************************************************************/

APIRET KbdQueryKeyboardType  (HKBD   hKbd,
                              PULONG pulType)
{
  APIRET rc;                                                 /* Rckgabewert */
  ULONG  ulParmLen;                                   /* Parameterpaketl„nge */
  ULONG  ulDataLen;                                       /* Datenblockl„nge */

  if (pulType == NULL)                               /* Parameterberprfung */
     return (ERROR_INVALID_PARAMETER);

    /* This is the direct way to communicate with the driver via IOCtl */

  ulParmLen = 0;
  ulDataLen = sizeof(ULONG);

  *pulType = 0x00000004;

  rc = DosDevIOCtl(hKbd,
                   IOCTL_KEYBOARD,
                   KBD_QUERYKBDHARDWAREID,
                   NULL,
                   0,
                   &ulParmLen,
                   pulType,
                   sizeof(ULONG),
                   &ulDataLen);

  *pulType >>= 16;

  return (rc);                                 /* Rckgabewert liefern */
} /* APIRET KbdQueryState */


/***********************************************************************
 * Name      : APIRET KbdQueryKeyboardCodepage
 * Funktion  : Erfragt die Keyboardcodepage via IOCtl
 * Parameter : HKBD hKbd
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 18.09.1995 23.02.58]
 ***********************************************************************/

APIRET KbdQueryKeyboardCodepage (HKBD         hKbd,
                                 PKBDCODEPAGE pKbdCodePage)
{
  APIRET rc;                                                 /* Rckgabewert */
  ULONG  ulParmLen;                                   /* Parameterpaketl„nge */
  ULONG  ulDataLen;                                       /* Datenblockl„nge */

  if (pKbdCodePage == NULL)                          /* Parameterberprfung */
     return (ERROR_INVALID_PARAMETER);

    /* This is the direct way to communicate with the driver via IOCtl */

  ulParmLen = 0;
  ulDataLen = sizeof(KBDCODEPAGE);
  pKbdCodePage->usDataLength = sizeof(KBDCODEPAGE);

  rc = DosDevIOCtl(hKbd,
                   IOCTL_KEYBOARD,
                   KBD_QUERYKBDCODEPAGESUPPORT,
                   NULL,
                   0,
                   &ulParmLen,
                   pKbdCodePage,
                   sizeof(KBDCODEPAGE),
                   &ulDataLen);

  return (rc);                                 /* Rckgabewert liefern */
} /* APIRET KbdQueryState */



/***********************************************************************
 * Name      : APIRET KbdSetState
 * Funktion  : Setzt den Keyboardstatus via IOCtl
 * Parameter : HKBD hKbd, PSHIFTSTATE pKbdShiftState
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 18.09.1995 23.02.58]
 ***********************************************************************/

APIRET KbdSetState  (HKBD        hKbd,
                     PSHIFTSTATE pKbdShiftState)
{
  APIRET rc;                                          /* Rckgabewert */
  ULONG  ulParmLen;                            /* Parameterpaketl„nge */
  ULONG  ulDataLen;                                /* Datenblockl„nge */

  if (pKbdShiftState == NULL)                      /* Parameterberprfung */
     return (ERROR_INVALID_PARAMETER);

    /* This is the direct way to communicate with the driver via IOCtl */
  ulParmLen = 0;
  ulDataLen = sizeof(SHIFTSTATE);

  rc = DosDevIOCtl(hKbd,
                   IOCTL_KEYBOARD,
                   KBD_SETSHIFTSTATE,
                   pKbdShiftState,
                   sizeof(SHIFTSTATE),
                   &ulParmLen,
                   NULL,
                   0,
                   &ulDataLen);

  return (rc);                                 /* Rckgabewert liefern */
} /* APIRET KbdSetState */


/***********************************************************************
 * Name      : void KbdShowState
 * Funktion  : Anzeigen eines speziellen Tastaturstatus.
 * Parameter : USHORT fsState, PSZ pszField, USHORT usMask
 * Variablen :
 * Ergebnis  : Ausgabe einer entspr Meldung "Right Shift: OFF"
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Mittwoch, 20.09.1995 09.37.35]
 ***********************************************************************/

void KbdShowState (USHORT fsState,
                   PSZ    pszField,
                   USHORT usMask)
{
  const PSZ pszOn  = "ON ";
  const PSZ pszOff = "OFF";

  PSZ   pszState;

  if (pszField == NULL)                              /* Parameterberprfung */
    return;

  /* Pointer setzen */
  pszState = (fsState & usMask) ? pszOn : pszOff;
  printf ("\n³ %-40s %s ³",
          pszField,
          pszState);
} /* void KbdShowState */


/***********************************************************************
 * Name      : APIRET KbdProcess
 * Funktion  :
 * Parameter : int argc, char *argv[]
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Montag, 18.09.1995 23.22.22]
 ***********************************************************************/

APIRET KbdProcess(void)
{
  HFILE      hKbd;                                  /* Handle zum Kbdtreiber */
  APIRET     rc;                                  /* Rckgabewert            */
  SHIFTSTATE KbdShiftState;                               /* keyboard status */
  ULONG      ulDummy;                                         /* dummy value */
  ULONG      ulKbdType;
  KBDCODEPAGE KbdCodepage;
  PSZ         pszTemp;
                                           /* Get access to the keybd driver */
  rc = DosOpen("\\DEV\\KBD$",                      /* open the device driver */
               &hKbd,
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


  rc = KbdQueryState (hKbd,                            /* read current state */
                      &KbdShiftState);
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                                    /* raise error condition */


  rc = KbdQueryKeyboardType (hKbd,
                             &ulKbdType);
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                                    /* raise error condition */


  rc = KbdQueryKeyboardCodepage (hKbd,
                                 &KbdCodepage);
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                                    /* raise error condition */


  printf ("\nÚÄCurrent Keyboard ValuesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿");
  KbdShowState (KbdShiftState.fsState, "Scrolllock",SCROLLLOCK_ON);
  KbdShowState (KbdShiftState.fsState, "Numlock",NUMLOCK_ON);
  KbdShowState (KbdShiftState.fsState, "Capslock",CAPSLOCK_ON);

  switch (ulKbdType)
  {
    case 0x00000001: printf ("\n³ PC/AT Standard Keyboard                      ³"); break;
    case 0x0000ab41: printf ("\n³ 101/102-Key Enhanced Keyboard                ³"); break;
    case 0x0000ab54: printf ("\n³ 88/89-Key   Enhanced Keyboard                ³"); break;
    case 0x0000ab86: printf ("\n³ 122-Key Mainframe Interactive (MFI) Keyboard ³"); break;
  }

  printf ("\n³ Active Codepage   : %03u                      ³",
          KbdCodepage.usKbdCP);

  pszTemp = KbdCodepage.szCountry;
  printf ("\n³ Active Country    : %-24s ³",
          pszTemp);
  pszTemp+=strlen(pszTemp)+1;
  printf ("\n³ Active Subcountry : %-24s ³",
          pszTemp);

  printf ("\nÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ");

/* Weitere Kbd_Tokens:
RIGHTSHIFT,LEFTSHIFT,CONTROL,ALT,SCROLLLOCK_ON,NUMLOCK_ON,CAPSLOCK_ON,
INSERT_ON,LEFTCONTROL,LEFTALT,RIGHTCONTROL,RIGHTALT,SCROLLLOCK,NUMLOCK,
CAPSLOCK,SYSREQ */

                                                /* Keyboardstatus umschalten */
  if (Options.fsNumlockOn)     KbdShiftState.fsState |= NUMLOCK_ON;
  if (Options.fsNumlockOff)    KbdShiftState.fsState &= ~NUMLOCK_ON;
  if (Options.fsCapslockOn)    KbdShiftState.fsState |= CAPSLOCK_ON;
  if (Options.fsCapslockOff)   KbdShiftState.fsState &= ~CAPSLOCK_ON;
  if (Options.fsScrolllockOn)  KbdShiftState.fsState |= SCROLLLOCK_ON;
  if (Options.fsScrolllockOff) KbdShiftState.fsState &= ~SCROLLLOCK_ON;


  rc = KbdSetState (hKbd,                               /* Neue Werte setzen */
                    &KbdShiftState);
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                                    /* raise error condition */


                                   /* Wurden Ver„nderungen vorgenommen ? */
  if (Options.fsNumlockOn    || Options.fsNumlockOff    ||
      Options.fsCapslockOn   || Options.fsCapslockOff   ||
      Options.fsScrolllockOn || Options.fsScrolllockOff)
  {
    printf ("\nÚÄNew Keyboard ValuesÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿");

    KbdShowState (KbdShiftState.fsState, "Scrolllock",SCROLLLOCK_ON);
    KbdShowState (KbdShiftState.fsState, "Numlock",NUMLOCK_ON);
    KbdShowState (KbdShiftState.fsState, "Capslock",CAPSLOCK_ON);

    printf ("\nÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ");
  }

  DosClose (hKbd);

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

int  main (int argc, char *argv[])
{
  int rc;                                                    /* Rckgabewert */

  initialize ();                                          /* Initialisierung */

  rc = ArgStandard(argc,                    /* parse command line parameters */
                   argv,
                   TabArguments,
                   &Options.fsHelp);
  if ( Options.fsHelp )                      /* check if user specified file */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  rc = KbdProcess();                                          /* do the work */
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  return (rc);
}
