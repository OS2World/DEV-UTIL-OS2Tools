/******************************************************************************
 * Name      : File Tree.C
 * Funktion  : Display Directory Tree
 * Bemerkung : (c) Copyright 1993 Patrick Haller
 *
 * Autor     : Patrick Haller [Mittwoch, 09.06.1994 02.03.43]
 ******************************************************************************/


 /*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/


#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"


/******************************************************************************
 * Structures                                                                 *
 ******************************************************************************/


typedef struct
{
  ULONG ulRecursionDepth;

} GLOBALS, *PGLOBALS;


typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsPath;                                    /* path supplied or not */
  PSZ     pszPath;                               /* pointer to the path data */

  ARGFLAG fsASCII;                                   /* use ASCII characters */
  ULONG   ulRecursionDepthMaximum;                /* maximum recursion depth */
  ARGFLAG fsRecursion;                  /* maximum recursion depth specified */
  ULONG   ulFindNumberMaximum;                        /* maximum find number */
  ARGFLAG fsFind;                           /* maximum find number specified */
} OPTIONS, *POPTIONS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/
OPTIONS Options;                /* this structure holds command line options */
GLOBALS Globals;                     /* this structure holds global varibles */

ARGUMENT TabArguments[] =
{ /*Token-------Beschreibung-----------------------pTarget--------------------------ucTargetFmt-pTargetSpecified--*/
  {"/?",        "Get help screen.",                NULL,                            ARG_NULL,   &Options.fsHelp},
  {"/H",        "Get help screen.",                NULL,                            ARG_NULL,   &Options.fsHelp},
  {"/A",        "Use plain ASCII characters.",     NULL,                            ARG_NULL |
                                                                                    ARG_HIDDEN, &Options.fsASCII},
  {"/R",        "Maximum recursion depth e.g. /R3",&Options.ulRecursionDepthMaximum,ARG_ULONG,  &Options.fsRecursion},
  {"1",         "Directory, pathname.",            &Options.pszPath,                ARG_PSZ |
                                                                                    ARG_DEFAULT,&Options.fsPath},
  ARG_TERMINATE
};






// ***DEFINES
#define MAXPATHLEN      260     // OS/2 max pathlength 260 chars
#define MATCH1          "\\*"
#define MATCH2          "*"
// ***TYPEDEFINITIONS
//typedef unsigned char UCHAR;
typedef char CHARSET[2][2][4];

// ***GLOBAL VARIABLES
static char  home_path[MAXPATHLEN];
static char  old_path [MAXPATHLEN];
static char  buf_path [MAXPATHLEN];
static ULONG old_drive;
static UCHAR fl_ascii = 0;
static CHARSET cs_chars = {"³  \0","ÃÄÄ\0",
                           "!  \0","+--\0"};


/******************************************************************************
 * Prototypes                                                                 *
 ******************************************************************************/

void   help            (void);

void   tree_error   (APIRET);

void   handle_dirs  (void);

void   tree_process (char *,char *);


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
  TOOLVERSION("Tree",                                   /* application name */
              0x00010002,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


/**************************\
 * void tree_error (void) *
 * Terminate program with *
 * (occured) error info.  *
\**************************/

void tree_error (APIRET rc)
{
  switch (rc)
  {
    case NO_ERROR:                      printf ("\nInternal Error (none).\n");
                                        break;
    case ERROR_FILE_NOT_FOUND:          printf ("\nFile not found error.\n");
                                        break;
    case ERROR_PATH_NOT_FOUND:          printf ("\nPath not found error.\n");
                                        break;
    case ERROR_ACCESS_DENIED:           printf ("\nAccess denied error.\n");
                                        break;
    case ERROR_NOT_ENOUGH_MEMORY:       printf ("\nNot enough memory error.\n");
                                        break;
    case ERROR_NOT_READY:               printf ("\nDrive not ready error.\n");
                                        break;
    case ERROR_INVALID_DRIVE:           printf ("\nInvalid drive error.\n");
                                        break;
    case ERROR_NOT_DOS_DISK:            printf ("\nNot a DOS disk error.\n");
                                        break;
    case ERROR_DRIVE_LOCKED:            printf ("\nDrive locked error.\n");
                                        break;
    case ERROR_BUFFER_OVERFLOW:         printf ("\nInternal buffer overflow error.\n");
                                        break;
    default:                            printf ("\nUndetermined Error: %lu",rc);
  }
  exit (EXIT_FAILURE);
}


/***************************\
 * void handle_dirs (void) *
 * Constructs return dir-  *
 * ectory for tree.c       *
\***************************/

void handle_dirs (void)
{
  APIRET rc;
  ULONG  drive_map;
  ULONG  dirpathlen;
  ULONG  new_drive;

  dirpathlen = MAXPATHLEN;
  DosQueryCurrentDisk (&old_drive,&drive_map);
  rc = DosQueryCurrentDir  (old_drive,buf_path,&dirpathlen);
  if (rc) tree_error(rc);

  strcpy (old_path,"\\");
  strcat (old_path,buf_path);

  if (home_path[strlen(home_path)-1] == '\\')
    home_path[strlen(home_path)-1] = 0;

  if (!strlen(home_path))
  {
     home_path[0] = (UCHAR)(old_drive + 64);
     home_path[1] = ':';
     home_path[2] = 0;
  }
  else
  {
    if (home_path[1] == ':')
    {
      new_drive = (ULONG)(toupper(home_path[0]) - 64);
      rc = DosSetDefaultDisk (new_drive);
      if (rc) tree_error(rc);
    }
    else
    {
      buf_path[0] = (UCHAR)(old_drive + 64);
      buf_path[1] = ':';
      buf_path[2] = 0;
      strcat (buf_path,home_path);
      strcpy (home_path,buf_path);
    }
  }

  printf ("\n%s\n",home_path);
}


void tree_process (char *path,char *depth)
{
  APIRET       rc;
  HDIR         dirHandle = HDIR_CREATE;
  FILEFINDBUF3 fileData;
  ULONG        count;
  char         *fullname,*nextpath,*nextdepth;
  ULONG        i;
  UINT         j;

  // allocating memory for buffer
  fullname = (char *)malloc(strlen(path)+5);
  if (fullname == NULL) tree_error (ERROR_NOT_ENOUGH_MEMORY);

  strcpy (fullname,path);
  if (fullname[0] == '\\') strcat (fullname,MATCH2);
  else strcat (fullname,MATCH1);

  count = 1;

  rc = DosFindFirst(fullname,   // search mask
                    &dirHandle,
                    MUST_HAVE_DIRECTORY,
                    &fileData,
                    sizeof(FILEFINDBUF3),
                    &count,
                    FIL_STANDARD);
  if (rc) tree_error (rc);

  while (!rc)
  {
      if ((strcmp(fileData.achName,".")  != 0) &&
          (strcmp(fileData.achName,"..") != 0))
      {
        // recurse

        nextpath = (char *)malloc(MAXPATHLEN);
        if (nextpath == NULL) tree_error (ERROR_NOT_ENOUGH_MEMORY);
        strcpy (nextpath,path);
        if (nextpath[strlen(nextpath)-1] != '\\')       strcat (nextpath,"\\");
        strcat (nextpath,fileData.achName);

        printf ("%s%s%s\n",depth,cs_chars[fl_ascii][1],fileData.achName);

        nextdepth = (char *)malloc(MAXPATHLEN);
        if (nextdepth == NULL) tree_error (ERROR_NOT_ENOUGH_MEMORY);
        strcpy (nextdepth,depth);
        strcat (nextdepth,cs_chars[fl_ascii][0]);

        tree_process(nextpath,nextdepth);
        free (nextdepth);
        free (nextpath);
      }

    count   = 1;

    rc = DosFindNext (dirHandle,
                      &fileData,
                      sizeof(FILEFINDBUF3),
                      &count);
  }

  DosFindClose(dirHandle);
  free(fullname);
}


/*********************************************\
 * void _cdecl main (int argc, char *argv[]) *
 * Main program routine, checks for params   *
\*********************************************/
void main (int argc, char *argv[])
{
  register UCHAR i;
  APIRET         rc;


  /****************************************************************************
   * Initialize Globals                                                       *
   ****************************************************************************/

  memset (&Globals,                              /* reset variable structure */
          0,
          sizeof(GLOBALS));

  Options.ulRecursionDepthMaximum = -1;                           /* maximum */


  /***************************************************************************
   * Parse arguments                                                         *
   ***************************************************************************/

  rc = ArgStandard (argc,                            /* CLI-Parameter parsen */
                    argv,
                    TabArguments,
                    &Options.fsHelp);
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    exit(1);                                                /* abort program */
  }

  if (Options.fsHelp)                        /* check if user specified help */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  /* translate parameters to legagy code */
  if (Options.fsASCII)
      fl_ascii = 1;
  
  // check if the root path was specified!
  if (Options.fsPath)
    strcpy (home_path, Options.pszPath);
  else
    strcpy (home_path, ".");

  handle_dirs ();

  /* process treeing */
  tree_process(home_path,"");

  rc = DosSetDefaultDisk (old_drive);
  if (rc) tree_error(rc);

  rc = DosSetCurrentDir (old_path);
  if (rc) tree_error(rc);
}
