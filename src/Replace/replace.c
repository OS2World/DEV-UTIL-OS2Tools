/*****************************************************
 * Replace Tool.                                     *
 * Replaces complete strings                         *
 * (c) 1994-95 Patrick Haller Systemtechnik          *
 *****************************************************/

/* To Do

   - schneller
   - leistungsfaehigere regeln
   - stdin / stdout
   - /IN:* /OUT:* (filenames ueberlappen)
   - /IN:* /OUT:<dir>\*
*/


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#ifdef __OS2__
  #define INCL_NOPMAPI
  #define INCL_DOS
  #define INCL_DOSERRORS
  #define INCL_DOSFILEMGR                             /* File Manager values */
  #include <os2.h>
#endif


#include <string.H>
#include <process.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"


#define MAXPATHLEN   260
#define IOBUFFERSIZE 16384

#ifdef __BORLANDC__
#define _Inline
#endif

#ifdef _MSC_VER
#define _Inline
#endif


/*****************************************************************************
 * Structures and definitions                                                *
 *****************************************************************************/

typedef struct
{
  PVOID *pNext;        /* pointer to the next entry in the replacement table */

  PSZ   pszOriginal;                              /* original string pattern */
  ULONG ulLengthOriginal;             /* length of original pattern in chars */

  PSZ   pszReplacement;                        /* replacement string pattern */
  ULONG ulLengthReplacement;          /* length of replace. pattern in chars */
} REPLENTRY, *PREPLENTRY;


typedef struct
{
  PREPLENTRY *pReplacementRoot;       /* root entry for the replacement list */
  ULONG      ulProcessedFiles;                  /* number of processed files */
  ULONG      ulProcessedLines;                  /* number of processed lines */
  ULONG      ulProcessedBytes;                  /* number of processed bytes */
  ULONG      ulRules;                             /* number of applied rules */

  PVOID      pBufferHeader;              /* points to the header file buffer */
  ULONG      ulHeaderSize;                      /* size of the header buffer */
  PVOID      pBufferFooter;              /* points to the footer file buffer */
  ULONG      ulFooterSize;                      /* size of the footer buffer */

  PVOID      piobufInput;                  /* I/O buffer for file operations */
  PVOID      piobufOutput;
} GLOBALS, *PGLOBALS;


typedef struct                    /* table of command line parameter options */
{
  ARGFLAG fFileInput;              /* indicates if input files are specified */
  PSZ     pszFileInput;            /* this points to the specified parameter */

  ARGFLAG fFileOutput;            /* indicates if output files are specified */
  PSZ     pszFileOutput;           /* this points to the specified parameter */

  ARGFLAG fFileRules;              /* indicates if rules files are specified */
  PSZ     pszFileRules;            /* this points to the specified parameter */

  ARGFLAG fFileHeader;             /* indicates if header file is specified  */
  PSZ     pszFileHeader;           /* this points to the specified parameter */

  ARGFLAG fFileFooter;             /* indicates if footer file is specified  */
  PSZ     pszFileFooter;           /* this points to the specified parameter */


  ARGFLAG fFileBackup;         /* indicates if backup files shall be created */

  ARGFLAG fQuiet;                       /* quiet operation, no screen output */
  ARGFLAG fHelp;                                  /* help shall be displayed */
  ARGFLAG fShowRules;    /* indicates if rule table has to be shown at first */
} OPTIONS, *POPTIONS;


static GLOBALS Globals;  /* export the global variable table for this module */
static OPTIONS Options; /* export the options variable table for this module */


/*****************************************************************************
 * parameter table                                                           *
 *****************************************************************************/

static ARGUMENT TabArguments[] =
{ /*Token-------Beschreibung--pTarget--ucTargetFormat--pTargetSpecified--*/
  {"/IN:",      "<input file name>",                      &Options.pszFileInput, ARG_MUST | ARG_PSZ, &Options.fFileInput},
  {"/OUT:",     "<output file name (pattern)>",           &Options.pszFileOutput,           ARG_PSZ, &Options.fFileOutput},
  {"/RULES:",   "<rule file name>",                       &Options.pszFileRules, ARG_MUST | ARG_PSZ, &Options.fFileRules},
  {"/HEADER:",  "<header file name>",                     &Options.pszFileHeader,           ARG_PSZ, &Options.fFileHeader},
  {"/FOOTER:",  "<footer file name>",                     &Options.pszFileFooter,           ARG_PSZ, &Options.fFileFooter},
  {"/SHOWRULES","<flag indicates if rule table is shown.",NULL,                  ARG_NULL,           &Options.fShowRules},
  {"/Q",        "Quiet Operation, no output.",            NULL,                  ARG_NULL,           &Options.fQuiet},
  {"/?",        "Get help screen.",                       NULL,                  ARG_NULL,           &Options.fHelp},
  {"/H",        "Get help screen.",                       NULL,                  ARG_NULL,           &Options.fHelp},
  ARG_TERMINATE
};

/* HTML-Mode: skip any (unknown, i.e. not processed)
              chars between < and > ???         */


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void   myStrReplace       (PSZ        pszSource,
                         PREPLENTRY pReplEntry);

APIRET ProcessFile      (PSZ        pszPath,
                         PSZ        pszFile);

APIRET ProcessFiles     (void);

void   RulesApply       (PREPLENTRY pReplRoot,
                         PSZ        pszString);

APIRET RulesAdd         (PREPLENTRY *ppEntry,
                         PSZ        pszOriginal,
                         PSZ        pszReplacement);

APIRET RulesRead        (PSZ        pszRuleFile,
                         PREPLENTRY *ppRoot);

int    main             (int        argc,
                         char       *argv[]);


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
  TOOLVERSION("Replace",                                /* application name */
              0x00010002,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


/*****************************************************************************
 * Name      : void myStrReplace
 * Funktion  : Ersetzt alle Vorkommen von pszOriginal durch pszReplacement
 *             innerhalb von pszSource
 * Parameter : PSZ pszSource,
 *             PSZ pszOriginal,
 *             PSZ pszReplacement
 * Variablen :
 * Ergebnis  : -
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 07.10.1995 03.22.44]
 *****************************************************************************/

void _Inline myStrReplace (PSZ        pszSource,
                         PREPLENTRY pReplEntry)
{
  PSZ   pszTemp;

#ifdef DEBUG
  if ( (pszSource      == NULL) ||                       /* check parameters */
       (pReplEntry     == NULL) )
    return;                                         /* abort processing here */
#endif

                                        /* Bis alle Tokens abgearbeitet sind */

  pszTemp = pszSource;     /* a little speedup and prevention of recursion ! */
  while ( (pszTemp = strstr(pszTemp,
                            pReplEntry->pszOriginal)) != NULL)
  {
                    /* pszTemp zeigt jetzt auf den gefundenen Originalstring */
    memmove (pszTemp+pReplEntry->ulLengthReplacement,
             pszTemp+pReplEntry->ulLengthOriginal,
             strlen(pszTemp + pReplEntry->ulLengthOriginal)+1);
                                                         /* Platz schaffen ! */
    memcpy  (pszTemp,                           /* Neuen Text hineinkopieren */
             pReplEntry->pszReplacement,
             pReplEntry->ulLengthReplacement);

    pszTemp += pReplEntry->ulLengthReplacement; /* ignore replacement tokens */
  }
}


/*****************************************************************************
 * Name      : APIRET ProcessFile
 * Funktion  : Processes a single file
 * Parameter : PSZ pszFile
 * Variablen :
 * Ergebnis  : -
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 07.10.1995 03.22.44]
 *****************************************************************************/

APIRET ProcessFile (PSZ pszPath,
                    PSZ pszFile)
{
  APIRET rc                      = NO_ERROR;    /* this is an API-returncode */
  char   szFileName[MAXPATHLEN];    /* local buffer for a modified file name */
  char   szFileTemp[MAXPATHLEN];    /* local buffer for a modified file name */
  FILE   *fileInput;                            /* handle for the input file */
  FILE   *fileOutput;                          /* handle for the output file */
  char   szLineBuffer[4096];                /* 4k buffer for line processing */


  if ( (pszPath == NULL) ||                              /* check parameters */
       (pszFile == NULL) )
    return (ERROR_INVALID_PARAMETER);                        /* signal error */


  if (Options.fFileOutput == TRUE)   /* check if output pattern is specified */
  {
               /* first we open all needed files, especially the output file */
    rc = DosEditName(1,                 /* OS/2 1.2 filename semantics apply */
                     pszFile,                              /* source pattern */
                     Options.pszFileOutput,          /* modification pattern */
                     szFileName,                                   /* buffer */
                     sizeof(szFileName));                  /* size of buffer */
    if (rc != NO_ERROR)                         /* check if an error occured */
      return (rc);                                           /* signal error */
  }
  else
  {
               /* first we open all needed files, especially the output file */
    rc = DosEditName(1,                 /* OS/2 1.2 filename semantics apply */
                     pszFile,                              /* source pattern */
                     "*.NEW",                        /* modification pattern */
                     szFileName,                                   /* buffer */
                     sizeof(szFileName));                  /* size of buffer */
    if (rc != NO_ERROR)                         /* check if an error occured */
      return (rc);                                           /* signal error */
  }


  strcpy (szFileTemp,                      /* copy the path information data */
          pszPath);
  strcat (szFileTemp,                          /* this is the open file name */
          pszFile);

  if (Options.fQuiet != TRUE)              /* check if output is appreciated */
    printf ("\n[%-34s] -> ",szFileTemp);

  fileInput = fopen(szFileTemp,"r");                     /* open for reading */
  if (fileInput == NULL)                          /* check if opening failed */
    return (ERROR_OPEN_FAILED);                              /* signal error */



  strcpy (szFileTemp,                      /* copy the path information data */
          pszPath);
  strcat (szFileTemp,                         /* this is the write file name */
          szFileName);

  if (Options.fQuiet != TRUE)              /* check if output is appreciated */
    printf ("[%-34s]",szFileTemp);
                                                /* now open the needed files */
  fileOutput = fopen(szFileTemp, "wb");                  /* open for writing */
  if (fileOutput == NULL)                         /* check if opening failed */
  {
    fclose(fileInput);                        /* close the input file handle */
    return (ERROR_OPEN_FAILED);                              /* signal error */
  }

                                         /* set buffers for input and output */
  setvbuf (fileInput,
           Globals.piobufInput,
           _IOFBF,
           IOBUFFERSIZE);

  setvbuf (fileOutput,
           Globals.piobufOutput,
           _IOFBF,
           IOBUFFERSIZE);


  if (Globals.pBufferHeader != NULL)        /* copy the header file at first */
    fwrite(Globals.pBufferHeader,
           Globals.ulHeaderSize,
           1,
           fileOutput);

  /* apply the replacement rules linewise */
  while (!feof(fileInput))       /* until we reach the end of the input file */
  {
    fgets(szLineBuffer,                                   /* read a new line */
          sizeof(szLineBuffer),
          fileInput);
    if (feof(fileInput))    /* abort processing here if last line is reached */
      break;

    Globals.ulProcessedLines++;                    /* correct the statistics */

    RulesApply (Globals.pReplacementRoot,     /* apply the replacement rules */
                szLineBuffer);

    fwrite(szLineBuffer,            /* write the new line to the output file */
           strlen(szLineBuffer),
           1,
           fileOutput);
  }


  if (Globals.pBufferFooter != NULL)       /* append the footer file at last */
    fwrite(Globals.pBufferFooter,
           Globals.ulFooterSize,
           1,
           fileOutput);

  fclose (fileOutput);                        /* close the used file handles */
  fclose (fileInput);

  return (rc);                                                /* return code */
}


/*****************************************************************************
 * Name      : APIRET ProcessFiles
 * Funktion  : Traverses the wildcard specification
 * Parameter : PSZ pszFiles
 * Variablen :
 * Ergebnis  : -
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 07.10.1995 03.22.44]
 *****************************************************************************/

#define NFILES  512
#define FFBUF   16384

APIRET ProcessFiles (void)
{
  APIRET       rc = NO_ERROR;                   /* this is an API-returncode */
  HDIR         dirHandle = HDIR_CREATE;             /* directory scan handle */
  FILEFINDBUF3 *fileData;
  PVOID        pDirectoryBuffer;         /* pointer to directory data buffer */
  ULONG        ulCount;         /* how many files have been found on drive ? */
  PSZ          pszFiles = Options.pszFileInput; /* pointer to variable space */
  char         szFilePath[MAXPATHLEN];    /* local buffer for filename xform */
  PSZ          pszTemp;        /* used for determination if path was passed. */

  pDirectoryBuffer = malloc(FFBUF);
  if (pDirectoryBuffer == NULL)        /* check if allocation was successful */
    return (ERROR_NOT_ENOUGH_MEMORY);                        /* signal error */

  ulCount = NFILES;         /* set how many files shall be read per API call */
  fileData = (FILEFINDBUF3 *)pDirectoryBuffer;              /* map to buffer */
  rc = DosFindFirst(pszFiles,            /* call to the first directory scan */
                    &dirHandle,
                    FILE_ARCHIVED |
                    FILE_NORMAL,
                    fileData,
                    FFBUF,
                    &ulCount,
                    FIL_STANDARD);


  if (rc != NO_ERROR)                    /* check if there has been an error */
    return (rc);                                             /* signal error */


                                                 /* modify the path buffer ! */
  strcpy(szFilePath,                                  /* copy the data first */
         Options.pszFileInput);

  pszTemp = strrchr(szFilePath,                   /* scan for the last slash */
                    '/');
  if (pszTemp == NULL)                       /* the slash has not been found */
  {
    pszTemp = strrchr(szFilePath,             /* scan for the last backslash */
                      '\\');
    if (pszTemp == NULL)                 /* the backslash has not been found */
      pszTemp = strrchr(szFilePath,               /* scan for the last colon */
                        ':');
  }

  if (pszTemp != NULL)       /* a path delimiting character has been found ! */
    *(pszTemp+1) = 0;                           /* cut the path string there */
  else
    szFilePath[0] = 0;                  /* then no additional path applies ! */



  while (rc == NO_ERROR)                      /* as long as everything is OK */
  {
  printf ("\nDEBUG: [%u scanned]",ulCount);
    while (ulCount != 0)                  /* as long as there are files left */
    {
      Globals.ulProcessedFiles++;                   /* adjust the statistics */
      Globals.ulProcessedBytes += fileData->cbFile;

      rc = ProcessFile(szFilePath,           /* now process this single file */
                       fileData->achName);
      if (rc != NO_ERROR)                     /* check if an error occured ? */
        goto __label_process_exit;                /* abort processing here ! */

      ulCount--;                                    /* skip to the next file */
      fileData = (FILEFINDBUF3 *) ((PBYTE)fileData +
                                   fileData->oNextEntryOffset);
    }
    ulCount   = NFILES;
    fileData = (FILEFINDBUF3 *)pDirectoryBuffer;
    rc = DosFindNext (dirHandle,              /* now call the next few files */
                      fileData,
                      FFBUF,
                      &ulCount);
  }


__label_process_exit:           /* here begins the routine's exit processing */
  DosFindClose(dirHandle);

  free(pDirectoryBuffer);
  return (rc);                                             /* signal success */
}


/*****************************************************************************
 * Name      : void RulesApply
 * Funktion  : This routine applies all known rules on a string
 * Parameter : PREPLENTRY pReplRoot,
 *             PSZ        pszString
 * Variablen :
 * Ergebnis  :
 * Bemerkung : no boundary check on pszString can be done. Here may occur
 *             traps !!!
 *
 * Autor     : Patrick Haller [Samstag, 07.10.1995 03.22.44]
 *****************************************************************************/

void RulesApply (PREPLENTRY pReplRoot,
                 PSZ        pszString)
{
  PREPLENTRY pReplTemp;          /* temporary replacement entry list pointer */

  if ( (pReplRoot      == NULL) ||                       /* check parameters */
       (pszString      == NULL) )
    return;                                         /* abort processing here */

  for (pReplTemp = pReplRoot;   /* this loop iterates over the complete list */
       pReplTemp != NULL;       /* of known rules and applies them all :)    */
       pReplTemp = (PREPLENTRY) pReplTemp->pNext)
  {
    myStrReplace (pszString,       /* apply the rule = replace some characters */
                pReplTemp);
  }
}


/*****************************************************************************
 * Name      : void RulesAdd
 * Funktion  : This routine adds a rule to the rule table
 * Parameter : PREPLENTRY *ppEntry,
 *             PSZ        pszOriginal,
 *             PSZ        pszReplacement
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 07.10.1995 03.22.44]
 *****************************************************************************/

APIRET RulesAdd (PREPLENTRY *ppEntry,
                 PSZ        pszOriginal,
                 PSZ        pszReplacement)
{
  PREPLENTRY pReplTemp;          /* temporary replacement entry list pointer */

  if ( (ppEntry        == NULL) ||                       /* check parameters */
       (pszOriginal    == NULL) ||
       (pszReplacement == NULL) )
    return (ERROR_INVALID_PARAMETER);                        /* signal error */

  pReplTemp = malloc(sizeof(REPLENTRY));        /* allocate new element then */
  if (pReplTemp == NULL)                       /* check if allocation failed */
    return (ERROR_NOT_ENOUGH_MEMORY);                        /* signal error */

  pReplTemp->pNext = (PVOID)*ppEntry;    /* save link to the succeeding node */
  pReplTemp->pszOriginal    = strdup(pszOriginal);          /* copy the data */
  pReplTemp->pszReplacement = strdup(pszReplacement);
  pReplTemp->ulLengthOriginal    = strlen(pszOriginal);
  pReplTemp->ulLengthReplacement = strlen(pszReplacement);

  *ppEntry = pReplTemp;               /* pass back the newly allocated entry */
  Globals.ulRules++;                           /* increase the rules counter */

  return (NO_ERROR);                                       /* signal success */
}


/*****************************************************************************
 * Name      : APIRET RulesRead
 * Funktion  : This routine analyses the passed rule file and builds the list
 * Parameter :
 * Variablen :
 * Ergebnis  : Returncode an das Betriebssystem
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 07.10.1995 03.22.44]
 *****************************************************************************/

APIRET RulesRead (PSZ        pszRuleFile,
                  PREPLENTRY *ppRoot)
{
  APIRET rc;                                               /* API-Returncode */
  FILE   *fileRules;                                  /* rule file structure */
  char   szBuffer[1024];                       /* buffers 1k  chars per line */
  char   chDelimiter;      /* this delimits original string from replacement */
  PSZ    pszOriginal;        /* this points to the original part of the rule */
  PSZ    pszReplacement;     /* this points to the replace. part of the rule */
  PSZ    pszTemp;                               /* temporary string iterator */

             /* a rule has this syntax:
                <delimiter> <original> <delimiter> <replacement> <delimiter> */

  if ( (pszRuleFile == NULL) ||                          /* check parameters */
       (ppRoot      == NULL) )
    return (ERROR_INVALID_PARAMETER);                        /* signal error */


  fileRules = fopen(pszRuleFile,"r");          /* open rule file for reading */
  if (fileRules == NULL)                                    /* open failed ? */
    return (ERROR_OPEN_FAILED);                              /* signal error */

  while (!feof(fileRules))        /* until we reach the end of the rule file */
  {
    fgets(szBuffer,                        /* get a line from the rules file */
          sizeof(szBuffer),
          fileRules);

    StrTrim(szBuffer);                                  /* clean up the line */
    if (szBuffer[0] != 0)               /* check if an usable string is left */
    {

          /* now we have to analyse the rule and put it into the linked list */
      chDelimiter=szBuffer[0];               /* this char delimites our rule */
      pszOriginal=szBuffer + 1;               /* it begins at buf+1 always ! */
      pszReplacement = strchr(pszOriginal,      /* find the second delimiter */
                              (int)chDelimiter);
      if (pszReplacement != NULL)       /* there seems to be a 2nd delimiter */
      {
        *pszReplacement = 0;        /* terminate the pszOriginal string here */
        pszReplacement++;                      /* skip to the next character */
        pszTemp = strchr(pszReplacement,           /* find the 3rd delimiter */
                         (int)chDelimiter);
        if (pszTemp != NULL)                               /* if we found it */
          *pszTemp = 0;  /* terminate the replacement string here explicitly */

        /* now we should have a valid pszOriginal and valid pszReplacement ! */
        rc = RulesAdd (ppRoot,                   /* add the rule to the list */
                       pszOriginal,
                       pszReplacement);
        /* @@@PH: error handling should/could be applied */

        if (Options.fShowRules == TRUE)
          printf ("\n[%-34s] -> [%-34s]",
                  pszOriginal,
                  pszReplacement);
      }
    }
  }

  fclose (fileRules);                                 /* close the rule file */

  return (NO_ERROR);                                       /* signal success */
}


/*****************************************************************************
 * Name      : int main
 * Funktion  : Hauptroutine, Command Line Auswertung
 * Parameter :
 * Variablen :
 * Ergebnis  : Returncode an das Betriebssystem
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 07.10.1995 03.22.44]
 *****************************************************************************/

int main (int argc, char *argv[])
{
  APIRET     rc = NO_ERROR;                   /* RÅckgabewert der Funktionen */
  PERFSTRUCT tsStart;
  PERFSTRUCT tsEnd;
  float      dSeconds;

  memset (&Options,0,sizeof(Options));                    /* Initialisierung */
  memset (&Globals,0,sizeof(Globals));                    /* Initialisierung */

  Globals.piobufInput = malloc(IOBUFFERSIZE);         /* allocate I/O buffer */
  if (Globals.piobufInput == NULL)                       /* check allocation */
    return (ERROR_NOT_ENOUGH_MEMORY);

  Globals.piobufOutput = malloc(IOBUFFERSIZE);        /* allocate I/O buffer */
  if (Globals.piobufOutput == NULL)                      /* check allocation */
    return (ERROR_NOT_ENOUGH_MEMORY);


  rc = ArgStandard (argc,                            /* CLI-Parameter parsen */
                    argv,
                    TabArguments,
                    &Options.fHelp);
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    exit(1);                                                /* abort program */
  }

  if (Options.fHelp)                         /* check if user specified help */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  rc = RulesRead(Options.pszFileRules,
                 &Globals.pReplacementRoot);               /* scan the rules */
  if (rc == NO_ERROR)                             /* check if anything is OK */
  {
    if (Options.fQuiet != TRUE)                   /* check if output is OK ? */
      printf ("\n%u rules apply.",   /* print message about rules that apply */
              Globals.ulRules);

                             /* preload header and footer files if necessary */
    if (Options.fFileHeader == TRUE)
    {
                                                    /* memory map the header */
      rc = ToolsReadFileToBuffer (Options.pszFileHeader,
                                  &Globals.pBufferHeader,
                                  &Globals.ulHeaderSize);
      if (rc != NO_ERROR)                                /* check for errors */
        ToolsErrorDos(rc);                          /* display error message */
    }

    if (Options.fFileFooter == TRUE)
    {
                                                    /* memory map the header */
      rc = ToolsReadFileToBuffer (Options.pszFileFooter,
                                  &Globals.pBufferFooter,
                                  &Globals.ulFooterSize);
      if (rc != NO_ERROR)                                /* check for errors */
        ToolsErrorDos(rc);                          /* display error message */
    }
    
    if (!Options.fFileOutput)
    {
      // if nothing else is specified, write output back to the input file
      Options.pszFileOutput = Options.pszFileInput;
      Options.fFileOutput = TRUE;
    }
    

                            /* now process the files (wildcards shall apply) */
    ToolsPerfQuery(&tsStart);                       /* measure starting time */
    rc = ProcessFiles ();                               /* process the files */
    ToolsPerfQuery(&tsEnd);                           /* measure ending time */

    if (rc != NO_ERROR)                                  /* check for errors */
      ToolsErrorDos(rc);                            /* display error message */

    dSeconds = tsEnd.fSeconds - tsStart.fSeconds;      /* calculate duration */
    if (dSeconds == 0.0)                          /* prevent divison by zero */
      dSeconds = 0.001;                         /* assume this minimum value */

      /* @@@PH */

    if (Options.fQuiet != TRUE)                   /* check if output is OK ? */
      printf ("\n%10u files processed. (%12.3f per second)"
              "\n%10u lines processed. (%12.3f per second)"
              "\n%10u bytes processed. (%12.3f per second)",
              Globals.ulProcessedFiles,
              Globals.ulProcessedFiles / dSeconds,
              Globals.ulProcessedLines,
              Globals.ulProcessedLines / dSeconds,
              Globals.ulProcessedBytes,
              Globals.ulProcessedBytes / dSeconds);


    if (Globals.pBufferHeader != NULL)      /* check if memory was allocated */
      free (Globals.pBufferHeader);                      /* free that memory */

    if (Globals.pBufferFooter != NULL)      /* check if memory was allocated */
      free (Globals.pBufferFooter);                      /* free that memory */
  }

  free (Globals.piobufInput);                /* free allocated I/O buffering */
  free (Globals.piobufOutput);

  return (rc);                /* deliver return code to the operating system */
}
