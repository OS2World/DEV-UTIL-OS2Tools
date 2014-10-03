/*************************************************************************

 Ralf Lohmueller

 *************************************************************************/



#define  INCL_DOS
#define  INCL_DOSERRORS
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"                                              /* phstools */


/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG  fsHelp;                       /* help is requested from command line */
  ARGFLAG  fsAgeDelete;                  /* user specified /d=<nr> parameter    */
  ARGFLAG  fsAgeList;                    /* user specified /l=<nr> parameter    */
  ARGFLAG  fsVerbose;                    /* user wants verbose output           */
  ARGFLAG  fsForce;                      /* user wants force delete             */
  ULONG ulAgeDelete;                  /* maximum file date for deletion      */
  ULONG ulAgeList;                    /* maximum file date for selection     */
} OPTIONS, *POPTIONS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */

ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung----------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",
    NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",
    NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/D=",        "Deletes files older than <nr> days, e.g. /D=25.",
    &Options.ulAgeDelete, ARG_ULONG,      &Options.fsAgeDelete},
  {"/L=",        "Lists files older than <nr> days, e.g. /L=25.",
    &Options.ulAgeList,   ARG_ULONG,      &Options.fsAgeList},
  {"/V",         "Verbose output.",
    NULL,                 ARG_NULL,       &Options.fsVerbose},
  {"/FORCE",     "Force delete.",
    NULL,                 ARG_NULL,       &Options.fsForce},
  ARG_TERMINATE
};


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
  TOOLVERSION("XDel",                                   /* application name */
              0x00010000,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              "(c) 1994,1997 Ralf Lohmueller");     /* additional copyright */
}


/***********************************************************************
 * Name      : int main
 * Funktion  : main routine
 * Parameter : int argc, char **argv, char **envp
 * Variablen :
 * Ergebnis  : returncode to the operating system
 * Bemerkung :
 *
 * Autor     : Ralf Lohmueller
 ***********************************************************************/

int main(int argc,
         char *argv[],
         char *envp[])
{
   HDIR         hdir;
   FILEFINDBUF3 findbuf;
   ULONG        cFiles = 1;
   FDATE        datum;
   USHORT       ftag,fmonat,fjahr,cnt;
   int          fdiff,fdiff2;
   ULONG        fsize,fsize2,fattrl,ftage,fheute,sum_alloc,sum_size;
   char         fattr[17];
   char         name[260];
   char         fdel = '*';
   APIRET       rc;
   struct tm    *lt;
   time_t       t;
   BOOL         listflag,delflag;

  rc = ArgStandard (argc,                            /* CLI-Parameter parsen */
                    argv,
                    TabArguments,
                    &Options.fsHelp);
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    exit(1);                                                /* abort program */
  }

  if ( Options.fsHelp ||                     /* check if user specified file */
       (!Options.fsAgeDelete && !Options.fsAgeList)
     )
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }


  /* temporary mapping of command line parameters */
  delflag  = Options.fsAgeDelete;
  listflag = Options.fsAgeList;

  if (Options.fsAgeDelete && Options.fsAgeList)
  {
    fprintf(stderr,
            "\nError: /D= and /L= are mutually exclusive.");                                               /* Anderer Fehler ... */
    return (NO_ERROR);                                      /* abort program */
  }


  /* gefordertes Dateialter aus parameter holen */
  if (Options.fsAgeDelete)
    fdiff = Options.ulAgeDelete;

  if (Options.fsAgeList)
    fdiff = Options.ulAgeList;


   /* Datum von heute, Tagnr */

   t      = time(NULL);
   lt     = localtime(&t);
   fheute = ToolsDateToAge(lt->tm_mday,lt->tm_mon+1,lt->tm_year+1900);


   /* init vars  */

   name[0]   = '\0';
   fattr[0]  = '\0';
   sum_alloc = 0;
   sum_size  = 0;


   /* dir-search */

   hdir = HDIR_SYSTEM;                        /* System soll neuen Handle zur Verfuegung stellen */
   rc = DosFindFirst("*",                     /*                   */
                &hdir,                        /* Pointer auf HDIR  */
                FILE_DIRECTORY | FILE_NORMAL, /* Attributes:       */
                &findbuf,                     /*                   */
                sizeof(findbuf),              /*                   */
                &cFiles,                      /*                   */
                FIL_STANDARD);                /*                   */

   while (rc == NO_ERROR)
   {

     name[0] = '\0';
     strcat(name,findbuf.achName);

     if (strcmp(name,".") && strcmp(name,".."))
      {
      fsize       = findbuf.cbFile;
      fsize2      = findbuf.cbFileAlloc;
      fattrl      = findbuf.attrFile;
      datum       = findbuf.fdateLastWrite;
           ftag   = datum.day;
           fmonat = datum.month;
           fjahr  = datum.year+1980;

      ftage  = ToolsDateToAge(ftag,fmonat,fjahr);

      if ((fheute-ftage)>fdiff)
      {
         fdel = '*';
         sum_alloc  += fsize2;
         sum_size   += fsize;

         if (delflag)
         {
           if (Options.fsForce)                           /* delete the file */
             rc = DosForceDelete(name);
           else
             rc = DosDelete(name);

           if (rc != NO_ERROR)
           {
             fprintf (stderr,                         /* print error message */
                      "\n%s",
                      name);
             ToolsErrorDos(rc);
           }
         }

      }
      else
         {
         fdel = ' ';
         };

      if ( listflag || ((fheute-ftage)>fdiff) )
         if (fattrl & 16)
            fprintf(stdout,"%c %02d.%02d.%4d             <dir> %s\n",
                                     fdel,
                                     ftag,
                                     fmonat,
                                     fjahr,
                             /*      fheute-ftage,          */
                             /*      findbuf.cbFile,        */
                             /*      findbuf.cbFileAlloc,   */
                                     findbuf.achName);
            else
            fprintf(stdout,"%c %02d.%02d.%4d [%5d] %9d %s\n",
                                     fdel,
                                     ftag,
                                     fmonat,
                                     fjahr,
                                     fheute-ftage,
                                     findbuf.cbFile,
                             /*      findbuf.cbFileAlloc,   */
                                     findbuf.achName);
      }

     rc = DosFindNext(hdir,
                 &findbuf, sizeof(findbuf),
                 &cFiles);

     }

   DosFindClose(hdir);

   fprintf(stdout,"\n    ");

   if (delflag) fprintf(stdout,"   xdel deleted ");
      else      fprintf(stdout,"xdel would free ");

   fprintf(stdout," %9d bytes (%d bytes real size)\n",sum_size,sum_alloc);

   return 0;
}
