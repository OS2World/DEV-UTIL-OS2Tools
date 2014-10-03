/*****************************************************
 * RStart tool, advanced session starting.           *
 * (c) 1997    Patrick Haller Systemtechnik          *
 *****************************************************/

/* #define DEBUG 1 */

/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#define INCL_DOSFILEMGR
#define INCL_BASE
#define INCL_NOPMAPI
#include <os2.h>

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <process.h>
#include "tools.h"
#include "toolarg.h"

#define MAXPATHLEN 260


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsName;
  ARGFLAG fsTitle;
  ARGFLAG fsParams;
  ARGFLAG fsEnv;
  ARGFLAG fsIcon;
  ARGFLAG fsPosX;
  ARGFLAG fsPosY;
  ARGFLAG fsSizeX;
  ARGFLAG fsSizeY;
  ARGFLAG fsWait;
  ARGFLAG fsPriority;
  ARGFLAG fsNoClose;
  ARGFLAG fsMaximize;
  ARGFLAG fsMinimize;
  ARGFLAG fsChild;
  ARGFLAG fsInherit;
  ARGFLAG fsInvisible;
  ARGFLAG fsForeground;

  PSZ pszName;
  PSZ pszTitle;
  PSZ pszParams;
  PSZ pszEnv;
  PSZ pszIcon;
  ULONG ulPosX;
  ULONG ulPosY;
  ULONG ulSizeX;
  ULONG ulSizeY;
  ULONG ulPriority;
} OPTIONS, *POPTIONS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */

ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung----------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/TITLE=",    "Process title.",     &Options.pszTitle,    ARG_PSZ,        &Options.fsTitle},
  {"/PARAM=",    "Parameters for that program.",
                                       &Options.pszParams,   ARG_PSZ,        &Options.fsParams},
  {"/ENV=",      "Environment for that program.",
                                       &Options.pszEnv,      ARG_PSZ,        &Options.fsEnv},
  {"/ICON=",     "Icon for that program.",
                                       &Options.pszIcon,     ARG_PSZ,        &Options.fsIcon},
  {"/POS.X=",    "X-position for a PM session.",
                                       &Options.ulPosX,      ARG_ULONG,      &Options.fsPosX},
  {"/POS.Y=",    "Y-position for a PM session.",
                                      &Options.ulPosY,       ARG_ULONG,      &Options.fsPosY},
  {"/SIZE.X=",   "X-size for a PM session.",
                                       &Options.ulSizeX,     ARG_ULONG,      &Options.fsSizeX},
  {"/SIZE.Y=",   "Y-size for a PM session.",
                                      &Options.ulSizeY,      ARG_ULONG,      &Options.fsSizeY},
  {"/CHILD",      "Launch as a child window.", NULL,         ARG_NULL,       &Options.fsChild},
  {"/FOREGROUND", "Launch in foreground.",     NULL,         ARG_NULL,       &Options.fsForeground},
  {"/INHERIT",    "Inherit handles from parent.", NULL,      ARG_NULL,       &Options.fsInherit},
  {"/INVISIBLE",  "Launch invisible session.",    NULL,      ARG_NULL,       &Options.fsInvisible},
  {"/MAX",        "Maximize session.",            NULL,      ARG_NULL,       &Options.fsMaximize},
  {"/MIN",        "Minimize session.",            NULL,      ARG_NULL,       &Options.fsMinimize},
  {"/NOCLOSE",    "Don't auto-close session.",    NULL,      ARG_NULL,       &Options.fsNoClose},
  {"/WAIT",       "Wait for termination.",        NULL,      ARG_NULL,       &Options.fsWait},
  {"/PRIORITY=",  "Child session's priority.",    &Options.ulPriority, ARG_ULONG, &Options.fsPriority},

  {"#1",          "Program name.",    &Options.pszName,      ARG_PSZ |
                                                             ARG_DEFAULT,    &Options.fsName},
  ARG_TERMINATE
};



/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void   help                (void);

void   initialize          (void);

int    main                (int,
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
  TOOLVERSION("RStart",                                 /* application name */
              0x00010000,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


/***********************************************************************
 * Name      : int createchild
 * Funktion  : Erzeugt den Child-Proze·
 * Parameter : ...
 * Variablen : ...
 * Ergebnis  : Returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 23.09.23]
 ***********************************************************************/

int createchild (void)
{
  ULONG       SessID;       /* Session ID (returned) */
  PID         pidPID;       /* Process ID (returned) */
  UCHAR       ObjBuf[260];  /* Object buffer */
  APIRET      rc;           /* Return code */
  STARTDATA   StartData;                    /* structure for DosStartSession */

  /*  Specify the various session start parameters  */
#if 0
 typedef struct _STARTDATA {
   USHORT     TraceOpt;       /*  An indicator which specifies whether the program started in the new session should be executed under conditions for tracing. */
   PBYTE      TermQ;          /*  Either 0 or the address of an ASCIIZ string that contains the file specification of a system queue. */
   USHORT     SessionType;    /*  The type of session that should be created for this program. */
   ULONG      PgmHandle;      /*  Either 0 or the program handle. */
 } STARTDATA;
#endif


  StartData.Length      = sizeof(STARTDATA); /* Length of STARTDATA structure */
  StartData.PgmTitle    = Options.pszTitle;       /* Session Title string          */
  StartData.PgmName     = Options.pszName;        /* Program path-name string      */
  StartData.PgmInputs   = Options.pszParams;      /* be passed to the program      */
  StartData.Environment = Options.pszEnv;
  StartData.IconFile    = Options.pszIcon;        /* specific icon file            */
  StartData.InitXPos    = Options.ulPosX;
  StartData.InitYPos    = Options.ulPosY;
  StartData.InitXSize   = Options.ulSizeX;    /* Initial window coordinates */
  StartData.InitYSize   = Options.ulSizeY;    /*   and size                 */

  if (Options.fsChild)
     StartData.Related = SSF_RELATED_CHILD;
  else
     StartData.Related = SSF_RELATED_INDEPENDENT;

  if (Options.fsForeground)
     StartData.FgBg = SSF_FGBG_FORE;
  else
     StartData.FgBg = SSF_FGBG_BACK;


  if (Options.fsInherit)
     StartData.InheritOpt = SSF_INHERTOPT_PARENT;
  else
     StartData.InheritOpt = SSF_INHERTOPT_SHELL;

  StartData.TraceOpt = SSF_TRACEOPT_NONE;   /* Don't trace session */


            /* Inherit environment and open */
            /*   file handles from parent   */
  StartData.SessionType = SSF_TYPE_DEFAULT;

#if 0
     0        SSF_TYPE_DEFAULT                   Use the PgmHandle data, or allow the Shell to establish the session type.
     1        SSF_TYPE_FULLSCREEN                Start the program in a full-screen session.
     2        SSF_TYPE_WINDOWABLEVIO             Start the program in a windowed session for programs using the Base Video Subsystem.
     3        SSF_TYPE_PM.                       Start the program in a windowed session for programs using the Presentation Manager services (including AVIO calls).
     4        SSF_TYPE_VDM                       Start the program in a full-screen DOS session.
     5        SSF_TYPE_GROUP
     6        SSF_TYPE_DLL
     7        SSF_TYPE_WINDOWEDVDM               Start the program in a windowed DOS session.
     8        SSF_TYPE_PDD
     9        SSF_TYPE_VDD
#endif


            /* Allow the Shell to establish */
            /*   the session type           */
  StartData.PgmControl = SSF_CONTROL_VISIBLE; /* == 0 */

  if (Options.fsPosX || Options.fsPosY || Options.fsSizeX || Options.fsSizeY) StartData.PgmControl |= SSF_CONTROL_SETPOS;
  if (Options.fsNoClose)   StartData.PgmControl |= SSF_CONTROL_NOAUTOCLOSE;
  if (Options.fsMaximize)  StartData.PgmControl |= SSF_CONTROL_MAXIMIZE;
  if (Options.fsMinimize)  StartData.PgmControl |= SSF_CONTROL_MINIMIZE;
  if (Options.fsInvisible) StartData.PgmControl |= SSF_CONTROL_INVISIBLE;


  StartData.TermQ     = 0;                    /* Assume no termination queue */
  StartData.PgmHandle = 0;               /* Do not use the installation file */
  StartData.Reserved  = 0;                         /* Reserved, must be zero */
  StartData.ObjectBuffer = ObjBuf;
            /* Object buffer to hold DosExecPgm */
            /*   failure causes                 */
  StartData.ObjectBuffLen = sizeof(ObjBuf);
            /* Size of object buffer */

#ifdef DEBUG
  printf ("\nint createchild (void)"
     "\n  StartData.Length = %u",StartData.Length);
  printf ("\n  StartData.PgmTitle = %s",StartData.PgmTitle);
  printf ("\n  StartData.PgmName = %s",StartData.PgmName);
  printf ("\n  StartData.Iconfile= %s",StartData.IconFile);

  printf ("\n  StartData.Pos  = %u,%u",StartData.InitXPos,StartData.InitYPos);
  printf ("\n  StartData.Size = %u,%u",StartData.InitXSize,StartData.InitYSize);

  if (pszPgmParam)
  {
    printf ("\n  StartData.PgmInputs:");
    p = pszPgmParam;
    while (*p)
    {
      printf ("\n[%s]",p);
      p+=strlen(p)+1;
    }
  }

  if (pszPgmEnv)
  {
    printf ("\n  StartData.PgmEnvironment:");
    p = pszPgmEnv;
    while (*p)
    {
      printf ("\n[%s]",p);
      p+=strlen(p)+1;
    }
  }

#endif

  rc = DosStartSession(&StartData,
                       &SessID,
                       &pidPID);
           /* On successful return, the variable  */
           /*   SessID contains the session ID    */
           /*   of the new session, and the       */
           /*   variable PID contains the process */
           /*   ID of the new process             */

  if (rc != 0)
     ToolsErrorDos(rc);
  else
    printf ("\nProcess %u started successfully.\n",
            pidPID);
  return (rc);
} /* int createchild */


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
  int rc;                                                    /* RÅckgabewert */

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

  if (Options.fsHelp)                                      /* provide help ? */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  /* get_args */
  rc = createchild();
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  return (rc);
}
