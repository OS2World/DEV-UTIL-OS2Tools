/***********************************************************************
 * Projekt   : PHS Tools
 * Name      : System Status
 * Funktion  : Information about processes, threads, files, etc.
 * Autor     : Patrick Haller [Montag, 25.09.1995 01.20.20]
 ***********************************************************************/

 /* To Do
    "TOP" mode - show CPU concumption of running processes continuously
  */

/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#define INCL_DOSDEVIOCTL
#define INCL_DOSDEVICES                                 /* DosDevIOCtl */
#define INCL_DOSERRORS                         /* Die Fehlerkonstanten */
#define INCL_DOSMISC                                  /* DosGetMessage */
#define INCL_DOS
#define INCL_NOPMAPI                      /* Kein Presentation Manager */

#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"
#include "sysstate.h"

#define MAXPATHLEN 260


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */

  ARGFLAG fsDontGlobal;          /* bit mask for de-activating some sections */
  ARGFLAG fsDontProcess;
  ARGFLAG fsDontThread;
  ARGFLAG fsDontModule;
  ARGFLAG fsDontSema16;
  ARGFLAG fsDontShrMem;
  ARGFLAG fsDontSema32;
  ARGFLAG fsDontFiles;
} OPTIONS, *POPTIONS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */

ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung----------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",   NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/!GLOBAL",   "Don't display global information.", NULL,  ARG_NULL,       &Options.fsDontGlobal},
  {"/!PROCESS",  "Don't display process information.",NULL,  ARG_NULL,       &Options.fsDontProcess},
  {"/!THREAD",   "Don't display threads information.",NULL,  ARG_NULL,       &Options.fsDontThread},
  {"/!MODULE",   "Don't display module information.", NULL,  ARG_NULL,       &Options.fsDontModule},
  {"/!SEMA16",   "Don't display 16-bit semaphore information.", NULL, ARG_NULL, &Options.fsDontSema16},
  {"/!SEMA32",   "Don't display 32-bit semaphore information.", NULL, ARG_NULL, &Options.fsDontSema32},
  {"/!SHRMEM",   "Don't display shared memory information.",    NULL, ARG_NULL, &Options.fsDontShrMem},
  {"/!FILES",    "Don't display files information.",  NULL,  ARG_NULL,       &Options.fsDontFiles},

  ARG_TERMINATE
};


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/


extern APIRET APIENTRY DosQuerySysState (ULONG func,
                                         ULONG par1,
                                         ULONG pid,
                                         ULONG _reserved_,
                                         PVOID buf,
                                         ULONG bufsz);

PQMODULE SystemModuleQuery(PQTOPLEVEL top,
                           HMODULE    hModHandle);

PQSEMA   SystemSemaQuery  (PQTOPLEVEL top,
                           PQPROCESS  pqProcess,
                           USHORT     hSemaphore);

PQSHRMEM SystemShrMemQuery(PQTOPLEVEL top,
                           USHORT     hHandle);

void   help            ( void );

int    main            ( int           argc,
                         char          *argv[] );


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
  TOOLVERSION("SystemState",                             /* application name */
              0x00010001,                            /* application revision */
              0x00010100,           /* minimum required revision of PHSTOOLS */
              0x0001FFFF,        /* maximum recommended revision of PHSTOOLS */
              NULL,                                                /* Remark */
              "Portions (C) Holger Veit, 1996");     /* additional copyright */
}


/***********************************************************************
 * Name      : void SystemDumpGlobal
 * Funktion  : dumps global system information
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

void SystemDumpGlobal(PQTOPLEVEL top)
{
  PQGLOBAL g = top->gbldata;

  printf ("\n旼System컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�"
          "\n�  Processes: %5u         Threads: %5u         Modules: %5u             �"
          "\n읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�",
          g->proccnt,
          g->threadcnt,
          g->modulecnt);
}


/***********************************************************************
 * Name      : PSZ SystemDumpThreadState
 * Funktion  : returns thread status as alphanumeric string
 * Parameter : ULONG ulState
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

PSZ SystemDumpThreadState(ULONG ulState)
{
  switch (ulState)
  {
    case 0x00000001: return ("READY");
    case 0x00000002: return ("BLOCKED");
    case 0x00000005: return ("RUNNING");
    case 0x00000009: return ("LOADED");
    default:         return ("<unknown>");
  }

  /* states: suspended, critical-section, etc. */
}


/***********************************************************************
 * Name      : PSZ SystemDumpThreadTime
 * Funktion  : returns thread time string
 * Parameter : ULONG ulTime
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

APIRET SystemDumpThreadTime(PSZ   pszBuffer,
                            ULONG ulBuffer,
                            ULONG ulTime)
{
  float fTime = ulTime * 31.25;            /* OS/2 timer tick duration in ms */

  ULONG ulHours    = fTime / 3600000;
  ULONG ulMinutes  = (fTime - ulHours * 3600000) / 60000;
  ULONG ulSeconds  = (fTime - ulMinutes * 60000) / 600;
  ULONG ulmSeconds = (fTime - ulSeconds * 600);

  if (pszBuffer == NULL)                                 /* check parameters */
    return(ERROR_INVALID_PARAMETER);                /* raise error condition */

  if (ulBuffer < 12)                                    /* check buffer size */
    return (ERROR_INSUFFICIENT_BUFFER);             /* raise error condition */

/*
  fprintf (stderr,
           "\nDEBUG: ulTime     = %u, ulHours    = %u"
           "\n       ulMinutes  = %u, ulMinutes  %% 60   = %u"
           "\n       ulSeconds  = %u, ulSeconds  %% 60   = %u"
           "\n       ulmSeconds = %u, ulnSeconds %% 1000 = %u",
           ulTime,
           ulHours,
           ulMinutes,
           ulMinutes % 60,
           ulSeconds,
           ulSeconds % 60,
           ulmSeconds,
           ulmSeconds % 1000);
*/

  sprintf (pszBuffer,
           "%u:%02u:%02u.%03u",
           ulHours,
           ulMinutes % 60,
           ulSeconds % 60,
           ulmSeconds % 1000);

  return (NO_ERROR);                                                   /* OK */
}


/***********************************************************************
 * Name      : PSZ SystemDumpProcessState
 * Funktion  : returns process status as alphanumeric string
 * Parameter : ULONG ulState
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

PSZ SystemDumpProcessState(ULONG ulState)
{
  switch (ulState)
  {
    case 0x00000000: return ("NORMAL");
    case 0x00000001: return ("EXITLIST");
    case 0x00000010: return ("BACKGRND");
    default:         return ("<unknown>");
  }
}


/***********************************************************************
 * Name      : PSZ SystemDumpProcessType
 * Funktion  : returns process type as alphanumeric string
 * Parameter : ULONG ulType
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

PSZ SystemDumpProcessType(ULONG ulType)
{
  switch (ulType)
  {
    case 0x00000000: return ("Fullscreen");
    case 0x00000001: return ("VDM");
    case 0x00000002: return ("VIO Window");
    case 0x00000003: return ("PM Session");
    case 0x00000004: return ("Detached");
    default:         return ("<unknown>");
  }
}


/***********************************************************************
 * Name      : void SystemDumpProcess
 * Funktion  : dumps process information
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

void SystemDumpProcess(PQTOPLEVEL top)
{
  PQPROCESS p = top->procdata;
  PQTHREAD  t;
  int       i;
  CHAR      szTimeThreadSystem[30];    /* buffer for time->string conversion */
  CHAR      szTimeThreadUser  [30];    /* buffer for time->string conversion */
  ULONG     ulTimeProcessSystem;                          /* process timings */
  ULONG     ulTimeProcessUser;
  PQMODULE  pqmodProcess;             /* pointer to the process's own module */
  QMODULE   qmodDefault = {NULL,
                           0xFFFF,
                           0xFFFF,
                           0xFFFFFFFF,
                           0xFFFFFFFF,
                           NULL,
                           "<unknown>",
                           0};

  printf ("\n旼Processes컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커");

  while (p && p->rectype == 1)
  {
    pqmodProcess = SystemModuleQuery(top,
                                     p->hndmod);
    if (pqmodProcess == NULL)           /* module not found, for some reason */
      pqmodProcess = &qmodDefault;                    /* then set to default */

    printf ("\n넬횾rocess #%10u (%8xh)컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커�"
            "\n납%-75s납"
            "\n납Threads:    %10u (%8xh)      Parent-PID: %10u (%8xh) 납"
            "\n납Type:      %11s (%8xh)      State:      %10s (%8xh) 납"
            "\n납Session-ID: %10u (%8xh)      Handle:     %10u (%8xh) 납",
            p->pid,
            p->pid,
            pqmodProcess->name,
            p->threadcnt,
            p->threadcnt,
            p->ppid,
            p->ppid,
            SystemDumpProcessType(p->type),
            p->type,
            SystemDumpProcessState(p->state),
            p->state,
            p->sessid,
            p->sessid,
            p->hndmod,
            p->hndmod);

    printf ("\n납Sem16s:     %10u (%8xh)      Pvt. Sem32s:%10u (%8xh) 납"
            "\n납DLLs:       %10u (%8xh)      Shared Mem: %10u (%8xh) 납"
            "\n납Handles:    %10u (%8xh)      Segments:   %10u (%8xh) 납"
            "\n납References: %10u (%8xh)                                         납",
            p->sem16cnt,
            p->sem16cnt,
            p->privsem32cnt,
            p->privsem32cnt,
            p->dllcnt,
            p->dllcnt,
            p->shrmemcnt,
            p->shrmemcnt,
            p->fdscnt,
            p->fdscnt,
            pqmodProcess->segcnt,
            pqmodProcess->segcnt,
            pqmodProcess->refcnt,
            pqmodProcess->refcnt);

    t = p->threads;

    printf ("\n납旼Threads컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커납"
            "\n납쿟ID   Sleep-ID  Prio State            Slot      System      User         납�");

    for (i=0,                                         /* initialize the loop */
         ulTimeProcessSystem = 0,
         ulTimeProcessUser   = 0;

         i < p->threadcnt;

         i++,
         t++)
    {
      if (!Options.fsDontThread)
      {
        SystemDumpThreadTime(szTimeThreadSystem,        /* convert the times */
                             sizeof(szTimeThreadSystem),
                             t->systime);

        SystemDumpThreadTime(szTimeThreadUser,
                             sizeof(szTimeThreadUser),
                             t->usertime);

        printf ("\n납�%-5u %08xh %04x %-10s %04xh %08xh %s %s  납�",
                t->threadid,
                t->sleepid,
                t->priority,
                SystemDumpThreadState(t->state),
                t->state,
                t->slotid,
                szTimeThreadSystem,
                szTimeThreadUser);

/*
        printf ("\n납旼Thread #%10u %8xh)컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커납"
                "\n납� Sleep ID:   %10u (%8xh)    Priority:  %10u (%8xh) 납�"
                "\n납� State:      %10s (%8xh)    Slot:      %10u (%8xh) 납�"
                "\n납� System Time:%22s    User Time: %22s 납�"
                "\n납읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴冒�",
                t->threadid,
                t->threadid,
                t->sleepid,
                t->sleepid,
                t->priority,
                t->priority,
                SystemDumpThreadState(t->state),
                t->state,
                t->slotid,
                t->slotid,
                szTimeThreadSystem,
                szTimeThreadUser);
*/
      }

      ulTimeProcessSystem += t->systime;     /* sum up ticks for the process */
      ulTimeProcessUser   += t->usertime;
    }
    printf ("\n냅컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸�");

    SystemDumpThreadTime(szTimeThreadSystem,            /* convert the times */
                         sizeof(szTimeThreadSystem),
                         ulTimeProcessSystem);

    SystemDumpThreadTime(szTimeThreadUser,
                         sizeof(szTimeThreadUser),
                         ulTimeProcessUser);


    printf("\n납System Time:  %22s    User Time: %22s  납",
           szTimeThreadSystem,
           szTimeThreadUser);


    /*************************************************************************
     * 16-bit semaphores                                                     *
     *************************************************************************/
    if (p->sem16cnt)
    {
      PQSEMA pqSema;                            /* pointer to semaphore data */

      for (i=0;
           i < p->sem16cnt;
           i++)
      {
        pqSema = SystemSemaQuery(top,
                                 p,
                                 p->sem16s[i]);
        if (pqSema != NULL)                   /* check if module was found */
        {
          printf ("\n납旼16-bit Semaphore %10u (%8xh)컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커납"
                  "\n납� %-71s 납�"
                  "\n납� References: %10u (%8xh)    Flags:    :%10u (%8xh) 납�"
                  "\n납� SysProc Ct: %10u (%8xh)    Index:    :%10u (%8xh) 납�"
                  "\n납읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴冒�",
                  p->sem16s[i],
                  p->sem16s[i],
                  pqSema->name,
                  pqSema->refcnt,
                  pqSema->refcnt,
                  pqSema->sysflags,
                  pqSema->sysflags,
                  pqSema->sysproccnt,
                  pqSema->sysproccnt,
                  pqSema->index,
                  pqSema->index);
        }
      }
    }

    /*************************************************************************
     * modules                                                               *
     *************************************************************************/
    if (p->dllcnt)
    {
      PQMODULE pqModule;

      printf ("\n넬횺odules컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커�"
              "\n납Handle    Module                                             Type Refs Segs납");

      for (i=0;
           i < p->dllcnt;
           i++)
      {
        pqModule = SystemModuleQuery(top,
                                     p->dlls[i]);
        if (pqModule != NULL)                   /* check if module was found */
        {
          printf ("\n납%08xh %-47s %4xh %4xh %4xh납",
                  p->dlls[i],
                  pqModule->name,
                  pqModule->type,
                  pqModule->refcnt,
                  pqModule->segcnt);
/*
          printf ("\n납旼Module %10u (%8xh)컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커납"
                  "\n납� %-71s 납�"
                  "\n납� Type:       %10u (%8xh)    References:%10u (%8xh) 납�"
                  "\n납� Segments:   %10u (%8xh)                                      납�"
                  "\n납읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴冒�",
                  p->dlls[i],
                  p->dlls[i],
                  pqModule->name,
                  pqModule->type,
                  pqModule->type,
                  pqModule->refcnt,
                  pqModule->refcnt,
                  pqModule->segcnt,
                  pqModule->segcnt);
*/
        }
      }
      printf ("\n냅컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸�");
    }

    /*************************************************************************
     * shared memory                                                         *
     *************************************************************************/
    if (p->shrmemcnt)
    {
      PQSHRMEM pqShrMem;

      printf ("\n넬훁hared Memory컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커�"
              "\n납Nr    Sel   Refs  Name                                                     납");

      for (i=0;
           i < p->shrmemcnt;
           i++)
      {
        pqShrMem = SystemShrMemQuery(top,
                                     p->shrmems[i]);
        if (pqShrMem != NULL)                   /* check if module was found */
        {
          printf ("\n납%4xh %4xh %4xh %-56s 납",
                  p->shrmems[i],
                  pqShrMem->selshr,
                  pqShrMem->refcnt,
                  pqShrMem->name);
/*
          printf ("\n납旼Shared Memory %10u (%8xh)컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴엿�"
                  "\n납� \\SHAREMEM\\%-61s 납�"
                  "\n납� Selector:   %10u (%8xh)    References:%10u (%8xh) 납�"
                  "\n납읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴冒�",
                  p->shrmems[i],
                  p->shrmems[i],
                  pqShrMem->name,
                  pqShrMem->selshr,
                  pqShrMem->selshr,
                  pqShrMem->refcnt,
                  pqShrMem->refcnt);
*/
        }
      }
      printf ("\n냅컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸�");
    }


    /*************************************************************************
     * file descriptors                                                      *
     *************************************************************************/
    if ( (p->fdscnt) &&
         (p->fds) )
    {
      for (i=0;
           i < p->fdscnt;
           i++)
        printf ("\n납旼File Descriptor (Handle) %10u (%8xh)컴컴컴컴컴컴컴컴컴컴컴컴커납"
                "\n납읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴冒�",
                p->fds[i],
                p->fds[i]);
    }


    printf ("\n냅컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸�");
    p = (PQPROCESS)t;
  }

  printf ("\n읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�");
}


/***********************************************************************
 * Name      : void SystemProcessQuery
 * Funktion  : returns pointer to the process
 * Parameter : PID pidProcess
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

PQPROCESS SystemProcessQuery(PQTOPLEVEL top,
                             PID        pidProcess)
{
  PQPROCESS p = top->procdata;
  int       i;
  PQTHREAD  t;

  while (p && p->rectype == 1)
  {
    if (p->pid == pidProcess)                      /* is this "our" module ? */
      return (p);                                   /* return this as result */

    t = p->threads;
    for (i=0;                                         /* initialize the loop */

         i < p->threadcnt;

         i++,
         t++);

    p = (PQPROCESS)t;
  }

  return ((PQPROCESS)NULL);                    /* return this as our default */
}


/***********************************************************************
 * Name      : void SystemModuleQuery
 * Funktion  : returns pointer to the module
 * Parameter : HMODULE hModHandle
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

PQMODULE SystemModuleQuery(PQTOPLEVEL top,
                           HMODULE    hModHandle)
{
  PQMODULE m       = top->moddata;
  int      i;

  while (m)
  {
    if (m->hndmod == hModHandle)                   /* is this "our" module ? */
      return (m);                                   /* return this as result */

    m = m->next;                                   /* skip to the next entry */
  }

  return ((PQMODULE)NULL);                     /* return this as our default */
}


/***********************************************************************
 * Name      : void SystemShrMemQuery
 * Funktion  : returns pointer to the shared memory
 * Parameter : HMODULE hModHandle
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

PQSHRMEM SystemShrMemQuery(PQTOPLEVEL top,
                           USHORT     hHandle)
{
  PQSHRMEM m       = top->shrmemdata;
  int      i;

  while (m)
  {
    if (m->hndshr == hHandle)                      /* is this "our" module ? */
      return (m);                                   /* return this as result */

    m = m->next;                                   /* skip to the next entry */
  }

  return ((PQSHRMEM)NULL);                     /* return this as our default */
}


/***********************************************************************
 * Name      : void SystemSemaQuery
 * Funktion  : returns pointer to the semaphore
 * Parameter : HSEM hSemaphore
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

PQSEMA SystemSemaQuery(PQTOPLEVEL top,
                       PQPROCESS  pqProcess,
                       USHORT     hSemaphore)
{
  PQSEMSTRUC s = top->semadata;
  PQSEMA     p = &s->sema;
  int      i;

  for (i=0;

       ( (p != NULL) &&
         (i < pqProcess->sem16cnt) );

        i++,
        p=p->next)
  {
/*
    fprintf (stderr,
             "\nDEBUG: hSem=%u == system=%u ?",
             hSemaphore,
             p->index);
*/
    if (p->index == hSemaphore)                    /* is this "our" module ? */
      return (p);                                   /* return this as result */
  }

  return ((PQSEMA)NULL);                       /* return this as our default */
}


/***********************************************************************
 * Name      : void SystemDumpModule
 * Funktion  : dumps module information
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

void SystemDumpModule(PQTOPLEVEL top)
{
  PQMODULE pqModule = top->moddata;
  PQMODULE pqModuleIter;
  int i;


  printf ("\n旼Modules컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커");

  while (pqModule)
  {
    printf ("\n넬횺odule %10u (%8xh)컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커�"
            "\n납 %-73s 납"
            "\n납 Type:       %10u (%8xh)      References:%10u (%8xh) 납"
            "\n납 Segments:   %10u (%8xh)                                        납",
            pqModule->hndmod,
            pqModule->hndmod,
            pqModule->name,
            pqModule->type,
            pqModule->type,
            pqModule->refcnt,
            pqModule->refcnt,
            pqModule->segcnt,
            pqModule->segcnt);

    for (i=0;
         i < pqModule->refcnt;
         i++)
    {
      pqModuleIter = SystemModuleQuery(top,
                                       pqModule->modref[i]);
      if (pqModuleIter != NULL)
      {
        printf("\n납 %04xh %-67s 납",
               pqModuleIter->hndmod,
               pqModuleIter->name);
      }
    }

    printf ("\n냅컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸�");

    pqModule = pqModule->next;
  }

  printf ("\n읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�");
}


/***********************************************************************
 * Name      : void SystemDumpSema
 * Funktion  : dumps semaphore information
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

void SystemDumpSema(PQTOPLEVEL top)
{
  PQSEMSTRUC s = top->semadata;
  PQSEMA     p = &s->sema;
  int        i;

  printf ("\n旼16-bit Semaphores컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커"
          "\n쿔ndex Name                                       Refs Flags     SysProcCount �");

  while(p)
  {
    printf ("\n�%04xh \\SEM\\%-39s %5u %08xh %10u�",
            p->index,
            p->name,
            p->refcnt,
            p->sysflags,
            p->sysproccnt);

    p = p->next;
  }

  printf ("\n읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�");
}


/***********************************************************************
 * Name      : void SystemDumpShrMem
 * Funktion  : dumps shared memory information
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

void SystemDumpShrMem(PQTOPLEVEL top)
{
  PQSHRMEM s = top->shrmemdata;

  printf ("\n旼Shared Memory컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커"
          "\n쿓andle    Name                                                Selector  Refs �");

  while (s)
  {
    printf ("\n�%08xh \\SHAREMEM\\%-40s %08xh %5u�",
            s->hndshr,
            s->name,
            s->selshr,
            s->refcnt);
    s = s->next;
  }

  printf ("\n읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�");
}


/***********************************************************************
 * Name      : PSZ SystemDumpFileAccess
 * Funktion  : returns file access mode as alphanumeric string
 * Parameter : ULONG ulAccess
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

PSZ SystemDumpFileAccess(ULONG ulAccess)
{
  switch (ulAccess & 0x00000003)
  {
    case OPEN_ACCESS_READONLY:  return ("READ ONLY ");
    case OPEN_ACCESS_WRITEONLY: return ("WRITE ONLY");
    case OPEN_ACCESS_READWRITE: return ("READWRITE ");
    default:return ("<unknown> ");
  }
}


/***********************************************************************
 * Name      : PSZ SystemDumpFileSharing
 * Funktion  : returns file sharing mode as alphanumeric string
 * Parameter : ULONG ulAccess
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

PSZ SystemDumpFileSharing(ULONG ulAccess)
{
  switch (ulAccess & 0x00000070)
  {
    case OPEN_SHARE_DENYREADWRITE: return ("DENY READWRITE");
    case OPEN_SHARE_DENYWRITE:     return ("DENY WRITE    ");
    case OPEN_SHARE_DENYREAD:      return ("DENY READ     ");
    case OPEN_SHARE_DENYNONE:      return ("DENY NONE     ");
    case 0x00000000:               return ("<unknown 0x00>");
    case 0x00000050:               return ("<unknown 0x50>");
    case 0x00000060:               return ("<unknown 0x60>");
    case 0x00000070:               return ("<unknown 0x70>");
    default:                       return ("<unknown>     ");
  }
}


/***********************************************************************
 * Name      : PSZ SystemDumpFileCaching
 * Funktion  : returns file caching mode as alphanumeric string
 * Parameter : ULONG ulFlags
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

PSZ SystemDumpFileCaching(ULONG ulFlags)
{
  switch (ulFlags & 0x00000300)
  {
    case OPEN_FLAGS_NO_LOCALITY:      return ("NO LOCALITY     ");
    case OPEN_FLAGS_SEQUENTIAL:       return ("SEQUENTIAL      ");
    case OPEN_FLAGS_RANDOM:           return ("RANDOM          ");
    case OPEN_FLAGS_RANDOMSEQUENTIAL: return ("RANDOMSEQUENTIAL");
    default:                          return ("<unknown>       ");
  }
}


/***********************************************************************
 * Name      : void SystemDumpFiles
 * Funktion  : dumps MFT/SFT information
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

void SystemDumpFiles(PQTOPLEVEL top)
{
  PQFILE f = (PQFILE)top->filedata;
  PQFDS fd;

  printf ("\n旼File Descriptors / Handles컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�");


  while (f && f->rectype==8)
  {
    fd = f->filedata;

    printf ("\n넬횲ile Handle %10u (%8xh)컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴엿"
            "\n납 %-73s 납"
            "\n납 Open Count: %10u (%8xh)   Filesize:     %10u (%8xh) 납"
            "\n납 References: %10u (%8xh)   Volume Handle:%10u (%8xh) 납",
            fd->sfn,
            fd->sfn,
            f->name,
            f->opencnt,
            f->opencnt,
            fd->filesize,
            fd->filesize,
            fd->refcnt,
            fd->refcnt,
            fd->volhnd,
            fd->volhnd);

    printf ("\n납 Attributes: %c%c%c%c%c      (%8xh)   Access:       %-10s (%8xh) 납"
            "\n납 Flags:      %10u (%8xh)   Sharing:      %-14s         납"
            "\n납 Caching:    %16s                                              납"
            "\n냅컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸�",
            (fd->attrib & FILE_READONLY) ? 'R' : '.',
            (fd->attrib & FILE_HIDDEN)   ? 'H' : '.',
            (fd->attrib & FILE_SYSTEM)   ? 'S' : '.',
            (fd->attrib & FILE_ARCHIVED) ? 'A' : '.',
            (fd->attrib & FILE_DIRECTORY)? 'D' : '.',
            fd->attrib,
            SystemDumpFileAccess(fd->accmode),
            fd->accmode,
            fd->flags,
            fd->flags,
            SystemDumpFileSharing(fd->accmode),
            SystemDumpFileCaching(fd->flags));


/*
   #define OPEN_FLAGS_NOINHERIT           0x00000080
   #define OPEN_FLAGS_NO_CACHE            0x00001000
   #define OPEN_FLAGS_FAIL_ON_ERROR       0x00002000
   #define OPEN_FLAGS_WRITE_THROUGH       0x00004000
   #define OPEN_FLAGS_DASD                0x00008000
   #define OPEN_FLAGS_NONSPOOLED          0x00040000
   #define OPEN_FLAGS_PROTECTED_HANDLE    0x40000000
*/

    f = f->next;
  }

  printf ("\n읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�");
}


/***********************************************************************
 * Name      : void SystemDumpSema32
 * Funktion  : dumps semaphore32 information
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Dienstag, 18.04.1995 22.55.47]
 ***********************************************************************/

void SystemDumpSema32(PQTOPLEVEL top)
{
  PQSEM32STRUC s3 = (PQSEM32STRUC)top->sem32data;

  if (s3 == NULL)
    return;

  printf ("\n旼32-bit Semaphores컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커");

  while (s3)
  {
    printf ("\n넬�32-bit Semaphore컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴엿");
    printf ("\n납 Event Sem:  %10u (%8xh)   Owner PID: %10u (%8h) 납"
            "\n납 Mutex Sem:  %10u (%8xh)   Owner PID: %10u (%8h) 납"
            "\n납 Mux   Sem:  %10u (%8xh)   Owner PID: %10u (%8h) 납",
            s3->evsem.own->opencnt,
            s3->evsem.own->opencnt,
            s3->evsem.own->pid,
            s3->evsem.own->pid,
            s3->muxsem.own->opencnt,
            s3->muxsem.own->opencnt,
            s3->muxsem.own->pid,
            s3->muxsem.own->pid,
            s3->smuxsem.own->opencnt,
            s3->smuxsem.own->opencnt,
            s3->smuxsem.own->pid,
            s3->smuxsem.own->pid);

    s3 = s3->next;
  }

  printf ("\n읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�");
}


/***********************************************************************
 * Name      : APIRET SystemControl
 * Funktion  : Command dispatcher
 * Parameter :
 * Variablen :
 * Ergebnis  : API return code
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 ***********************************************************************/

APIRET SystemControl (void)
{
  APIRET     rc;                                           /* API returncode */
  PVOID      pBuffer;                      /* buffer for the API information */
  PQTOPLEVEL pqtTop;                            /* the top level information */

#define BUFSIZE 128000l
#define RESERVED 0

  pBuffer = malloc(BUFSIZE);                  /* allocate buffer for the API */
  if (pBuffer == NULL)                        /* check for proper allocation */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */


  memset(pBuffer,                               /* zero-out the whole buffer */
         0,
         BUFSIZE);

  rc = DosQuerySysState(0x3f,                        /* OK, now call the API */
                        RESERVED,
                        RESERVED,
                        RESERVED,
                        (PCHAR)pBuffer,
                        BUFSIZE);
  if (rc == NO_ERROR)                                     /* everything OK ? */
  {
    pqtTop = (PQTOPLEVEL)pBuffer;               /* assign the buffer pointer */

    if (!Options.fsDontGlobal)  SystemDumpGlobal (pqtTop);
    if (!Options.fsDontProcess) SystemDumpProcess(pqtTop);
    if (!Options.fsDontModule)  SystemDumpModule (pqtTop);
    if (!Options.fsDontSema16)  SystemDumpSema   (pqtTop);
    if (!Options.fsDontShrMem)  SystemDumpShrMem (pqtTop);
    if (!Options.fsDontSema32)  SystemDumpSema32 (pqtTop);
    if (!Options.fsDontFiles)   SystemDumpFiles  (pqtTop);
  }

  free (pBuffer);                                         /* free the buffer */

  return (NO_ERROR);                                                   /* OK */
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
  int rc;                                                    /* R갷kgabewert */

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

  if ( Options.fsHelp )                                  /* help requested ? */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

                                                /* do some parameter mapping */

  rc = SystemControl();                                        /* Los geht's */
  if (rc != NO_ERROR)
    ToolsErrorDos(rc);

  return (rc);
}
