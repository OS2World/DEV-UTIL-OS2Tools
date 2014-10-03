/***********************************************************************
 * Name      : Sort
 * Funktion  : Sorting a textfile
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:25:17]
 ***********************************************************************/


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#ifdef __OS2__
  #define INCL_DOSFILEMGR
  #define INCL_BASE
  #define INCL_NOPMAPI
  #include <os2.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <io.h>


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
  ARGFLAG fsFileInput;                 /* user specified the input file name */
  ARGFLAG fsFileOutput;               /* user specified the output file name */
  ARGFLAG fsSortStart;                               /* first column to sort */
  ARGFLAG fsReverse;                                   /* reverse sort order */
  ARGFLAG fsVerbose;                                       /* verbose output */
  ARGFLAG fsIgnoreCase;                          /* case insensitive sorting */
  ARGFLAG fsNumeric;                            /* converts string to number */

  PSZ   pszFileInput;                          /* this is the input filename */
  PSZ   pszFileOutput;                    /* and this is the output filename */
  ULONG ulSortStart;                                 /* first column to sort */
} OPTIONS, *POPTIONS;

typedef PSZ *PPSZ;

typedef struct
{
  ULONG ulFileSize;                                /* total size of the file */
  PSZ   pszFileBuffer;        /* points to the memory allocated for the file */

  PPSZ  parrStrings;              /* pointer to dynamic string pointer array */
  ULONG ulStrings;                             /* number of string in buffer */
  ULONG ulStringsCurrent;                 /* index of current string pointer */
} GLOBALS, *PGLOBALS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/
OPTIONS Options;                /* this structure holds command line options */
GLOBALS Globals;                /* this structure holds global variables     */


ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung---------------pTarget----------------ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",        NULL,                  ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",        NULL,                  ARG_NULL,       &Options.fsHelp},
  {"/VERBOSE",   "Verbose output.",         NULL,                  ARG_NULL,       &Options.fsVerbose},
  {"/IN=",       "Input file.",             &Options.pszFileInput ,ARG_PSZ,        &Options.fsFileInput},
  {"/OUT=",      "Output file.",            &Options.pszFileOutput,ARG_PSZ,        &Options.fsFileOutput},
  {"/FIRST=",    "First column to sort.",   &Options.ulSortStart  ,ARG_ULONG,      &Options.fsSortStart},
  {"/+",         "First column",            &Options.ulSortStart  ,ARG_ULONG|
                                                                   ARG_HIDDEN,     &Options.fsSortStart},
  {"/R",         "Reverse",                 NULL,                  ARG_NULL |
                                                                   ARG_HIDDEN,       &Options.fsReverse},
  {"/REVERSE",   "Reverse sort order.",     NULL,                  ARG_NULL,       &Options.fsReverse},
  {"/I",         "Ignore case.",            NULL,                  ARG_NULL,       &Options.fsIgnoreCase},
  {"/NUMERIC",   "Converts sort argument to numerical value.",
                                            NULL,                  ARG_NULL,       &Options.fsNumeric},
  ARG_TERMINATE
};


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/
void  help               (void);
int   main               (int, char **);


/***********************************************************************
 * Name      : void help
 * Funktion  : Anzeigen des Hilfetextes
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag,24.09.93 - 13:43:33]
 ***********************************************************************/

void help ()
{
  TOOLVERSION("Sort",                                   /* application name */
              0x00010006,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
}


/*****************************************************************************
 * Name      : APIRET SortProcess
 * Funktion  : build pointer list
 * Parameter :
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/


int sort_function_fast (const void *a, const void *b)
{
  register signed char cResult;

  cResult = **(signed char **)a - **(signed char **)b;
  if (!cResult)
    return (strcmp(*(PPSZ)a, *(PPSZ)b));
  else
    return ( (int)cResult);
}


int sort_function (const void *a, const void *b)
{
  PSZ   pszA = *(PPSZ)a;
  PSZ   pszB = *(PPSZ)b;

  int   iLengthA = strlen(pszA);               /* length of the both strings */
  int   iLengthB = strlen(pszB);

  if (Options.ulSortStart)                       /* check for special values */
  {
    if ( (iLengthA > Options.ulSortStart) &&          /* check stringlengths */
         (iLengthB > Options.ulSortStart) )
    {
      pszA += Options.ulSortStart;
      pszB += Options.ulSortStart;
    }
    else
      return (iLengthA - iLengthB);            /* compare stringlengths then */
  }

#if 0
  fprintf (stderr,
           "\nDEBUG: pszA=[%s,%i]"
           "\n       pszB=[%s,%i]"
           "\n       indt=%u",
           pszA,atoi(pszA),
           pszB,atoi(pszB),
           Options.ulSortStart);
#endif


  if (Options.fsNumeric)                                /* numerical sorting */
    if (!Options.fsReverse)
      return (atoi(pszA) - atoi(pszB));
    else
      return (atoi(pszB) - atoi(pszA));
  else                                             /* alphanumerical sorting */
    if (Options.fsIgnoreCase)
      if (!Options.fsReverse)
        return (stricmp(pszA, pszB));
      else
        return (stricmp(pszB, pszA));
    else
      if (!Options.fsReverse)
        return (strcmp(pszA, pszB));
      else
        return (strcmp(pszB, pszA));
}


APIRET SortProcess (void)
{
  PSZ   pszTemp;                   /* iterator over the complete file buffer */
  PSZ   pszString;  /* points to the beginning of currently processed string */
  ULONG ulTemp;                     /* counter over the complete file buffer */

  PVOID pBuffer;                                        /* the output buffer */
  ULONG ulBufferPosition;              /* current position within the buffer */
  ULONG ulBufferSize;                                         /* buffer size */
  ULONG ulStringLength;          /* length of the currently processed string */
  ULONG ulDummy;                            /* unsigned long int dummy value */
  APIRET rc = NO_ERROR;                                    /* API returncode */
  HFILE hFileOutput;                                   /* output file stream */



  Globals.parrStrings = malloc(1024 * sizeof(PSZ) );       /* allocate array */
  Globals.ulStrings   = 1024;
  Globals.ulStringsCurrent = 0;                  /* currently active pointer */

  if (Globals.parrStrings == NULL)                       /* check allocation */
    return (ERROR_NOT_ENOUGH_MEMORY);               /* raise error condition */

  pszTemp   = Globals.pszFileBuffer;              /* initialize this pointer */
  pszString = pszTemp;


  /***************************************************************************
   * building pointer array                                                  *
   ***************************************************************************/

  if (Options.fsVerbose)                                 /* verbose output ? */
    printf ("\n- building pointer array");

                                /* now iterate over the complete file buffer */
  for (ulTemp = 0;
       ulTemp < Globals.ulFileSize;)
  {
    switch (*pszTemp)                           /* examine current character */
    {
      case 0x0d: /* CR LF */
        if ( *(pszTemp+1) == 0x0a)               /* check for following 0x0a */
        {
          *pszTemp = 0;                              /* terminate the string */
          pszTemp++;                             /* skip following character */
          ulTemp++;

          /* !!! NO BREAK !!! enter next case !!! */
        }
        else
          break;

      case 0x0a: /* LF */
        *pszTemp = 0;                 /* apply artificial string termination */

             /* OK, now we have to enter our new string in the pointer array */
        if (Globals.ulStringsCurrent >= Globals.ulStrings)    /* check space */
        {
          Globals.ulStrings += 1024;                   /* increase the array */
          Globals.parrStrings = realloc (Globals.parrStrings,
                                         Globals.ulStrings * sizeof (PSZ) );
          if (Globals.parrStrings == NULL)               /* check allocation */
            return (ERROR_NOT_ENOUGH_MEMORY);       /* raise error condition */
        }

        Globals.parrStrings[Globals.ulStringsCurrent] = pszString;
        Globals.ulStringsCurrent++;                    /* skip to next entry */

        pszString = pszTemp + 1;  /* set to the beginning of the next string */

        break;
    }

    ulTemp++;
    pszTemp++;
  }


  /***************************************************************************
   * qsort the pointer array                                                 *
   ***************************************************************************/

  if (Options.fsVerbose)                                 /* verbose output ? */
    printf ("\n- sorting pointers");

  if ( (!Options.fsReverse) &&
       (!Options.ulSortStart) &&
       (!Options.fsIgnoreCase) &&
       (!Options.fsNumeric)
     )
    qsort(Globals.parrStrings,                     /* faster for the default */
          Globals.ulStringsCurrent,
          sizeof(PSZ),
          sort_function_fast);
  else
    qsort(Globals.parrStrings,
          Globals.ulStringsCurrent,
          sizeof(PSZ),
          sort_function);


  /***************************************************************************
   * writing the output file                                                 *
   ***************************************************************************/

  if (Options.fsVerbose)                                 /* verbose output ? */
    printf ("\n- writing output (%u lines)",
            Globals.ulStringsCurrent);

  {
    if (!Options.fsFileOutput)
      hFileOutput = 1;        /* HF_STDOUT */    /* default output is stdout */
    else
    {
      rc = DosOpen (Options.pszFileOutput,
                    &hFileOutput,
                    &ulDummy,
                    Globals.ulFileSize,                          /* Filesize */
                    0L,                                   /* File attributes */
                    OPEN_ACTION_REPLACE_IF_EXISTS |
                    OPEN_ACTION_CREATE_IF_NEW,
                    OPEN_SHARE_DENYWRITE  |
                    OPEN_ACCESS_WRITEONLY |
                    OPEN_FLAGS_FAIL_ON_ERROR |
                    OPEN_FLAGS_SEQUENTIAL,
                    NULL);
      if (rc != NO_ERROR)                                /* check for errors */
        goto sort_bail_out;
    }


    ulBufferPosition = 0;
    ulBufferSize     = 65535;
    pBuffer = malloc(ulBufferSize);            /* allocate the output buffer */
    if (pBuffer == NULL)                                 /* check for errors */
    {
      rc = ERROR_NOT_ENOUGH_MEMORY;                 /* raise error condition */
      goto sort_bail_out;                           /* OK, we're out of here */
    }

                       /* iterate over the pointer array and write all lines */
    for (ulTemp=0;
         ulTemp<Globals.ulStringsCurrent;
         ulTemp++)
    {
      ulStringLength = strlen(Globals.parrStrings[ulTemp]);
                                                 /* would we exceed buffer ? */
      if (ulBufferPosition + ulStringLength > ulBufferSize - 2)
      {
        /* flush current buffer */
        rc = DosWrite (hFileOutput,
                       pBuffer,
                       ulBufferPosition,
                       &ulDummy);
        if (rc != NO_ERROR)                              /* check for errors */
          goto sort_bail_out;

        ulBufferPosition = 0;                    /* declare buffer as cleared */
      }

                                             /* add the string to the buffer */
      memcpy ((PSZ)pBuffer + ulBufferPosition,
              Globals.parrStrings[ulTemp],
              ulStringLength);

      ulBufferPosition += ulStringLength;

      *(PULONG)((PSZ)pBuffer +
                     ulBufferPosition) = 0x00000a0d;         /* add linefeed */

      ulBufferPosition += 2;                        /* set new buffer cursor */
    }

    /* flush current buffer */
    rc = DosWrite (hFileOutput,
                   pBuffer,
                   ulBufferPosition,
                   &ulDummy);
    free(pBuffer);         /* free memory used for the file output buffering */
  }

sort_bail_out:
  if (Options.fsFileOutput)                           /* do not close stdout */
    DosClose(hFileOutput);                          /* close the file handle */

  free (Globals.parrStrings);                          /* free pointer array */
  return (rc);                                                         /* OK */
}


/*****************************************************************************
 * Name      : APIRET SortProcessSTDIN
 * Funktion  : Sort data read from stdin
 * Parameter :
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

APIRET SortProcessSTDIN (void)
{
  APIRET      rc = NO_ERROR;                              /* API return code */
  PERFSTRUCT  psSortStart;
  PERFSTRUCT  psSortEnd;
  PSZ         pStdinBuffer;                       /* pointer to stdin buffer */
  ULONG       ulStdinBufferSize = 65536;                /* stdin buffer size */
  ULONG       ulReadSize = 65535;                           /* standard size */
  ULONG       ulStdinPosition;                      /* current read position */
  ULONG       ulReadBytes;           /* bytes read during last I/O operation */

  if (Options.fsVerbose)                                 /* verbose output ? */
    printf ("\nSorting ..."
            "\n- reading from \\DEV\\STDIN\n");

  rc = DosAllocMem ((PPVOID)&pStdinBuffer,      /* initial memory allocation */
                    ulStdinBufferSize,              /* but not yet committed */
                    PAG_READ | PAG_WRITE | PAG_COMMIT);
  if (rc != NO_ERROR)                                    /* check for errors */
    return (rc);                                    /* raise error condition */

  ulStdinPosition = 0;                                     /* initialization */

  do                                               /* read to dynamic buffer */
  {
#if 0
    rc = DosRead (0 /* HFILE_STDIN */,              /* read from standard in */
                  &pStdinBuffer[ulStdinPosition],
                  ulReadSize,
                  &ulReadBytes);
    if (rc == NO_ERROR)                            /* check if error occured */
#endif

    ulReadBytes = read (0,
                        &pStdinBuffer[ulStdinPosition],
                        ulReadSize);

    if (errno == 0)
    {
      ulStdinPosition += ulReadBytes;             /* calculate next position */

                                  /* running out of committed memory pages ? */
      if (ulStdinPosition + ulReadSize > ulStdinBufferSize)
      {
        PVOID pNewBuffer;               /* pointer to newly allocated buffer */

        rc = DosAllocMem (&pNewBuffer,         /* re-allocate memory block ! */
                          2 * ulStdinBufferSize,
                          PAG_READ | PAG_WRITE | PAG_COMMIT);
        if (rc != NO_ERROR)                              /* check for errors */
        {
          DosFreeMem(pStdinBuffer);      /* free previously allocated memory */
          return (rc);
        }

        memcpy (pNewBuffer,                           /* copy the data bytes */
                pStdinBuffer,
                ulStdinPosition);

        DosFreeMem(pStdinBuffer);                     /* free the old buffer */

        pStdinBuffer = pNewBuffer;                    /* adjust the pointers */
        ulStdinBufferSize *= 2;                 /* double the reserved range */
      }
    }
  }
  while ( (rc == NO_ERROR) & (ulReadBytes != 0) );

  Globals.pszFileBuffer = (PSZ)pStdinBuffer;  /* adjust the global variables */
  Globals.ulFileSize    = ulStdinPosition;

  ToolsPerfQuery(&psSortStart);             /* query the performance counter */
  SortProcess();                                      /* call sort functions */
  ToolsPerfQuery(&psSortEnd);               /* query the performance counter */


  if (Options.fsVerbose)                                 /* verbose output ? */
  {
    float fSort  = psSortEnd.fSeconds - psSortStart.fSeconds;

    if (fSort != 0.0)                              /* avoid division by zero */
      printf ("\nSorting time  is %10.3fs"
              "\nSorting speed is %10.3f lines per second.",
              fSort,
              Globals.ulStringsCurrent / fSort);
  }

  return (rc);                                             /* signal success */
}


/*****************************************************************************
 * Name      : APIRET SortProcessFile
 * Funktion  : Map file into memory
 * Parameter : PSZ pszFile
 * Variablen :
 * Ergebnis  : API returncode
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag, 04.05.1995 00.53.13]
 *****************************************************************************/

APIRET SortProcessFile (PSZ pszFile)
{
  APIRET      rc;                                         /* API return code */
  PERFSTRUCT  psStart;                     /* performance counter structures */
  PERFSTRUCT  psSortStart;
  PERFSTRUCT  psSortEnd;
  PERFSTRUCT  psEnd;

  if (pszFile == NULL)                                   /* check parameters */
    return (ERROR_INVALID_PARAMETER);                   /* return error code */

  if (Options.fsVerbose)                                 /* verbose output ? */
    printf ("\nSorting ..."
            "\n- reading the file");

  ToolsPerfQuery(&psStart);                 /* query the performance counter */

  rc = ToolsReadFileToBuffer(pszFile,                       /* read the file */
                             (PPVOID)&Globals.pszFileBuffer,
                             &Globals.ulFileSize);
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                   /* check for errors */
    return (rc);                               /* abort function immediately */
  }


  ToolsPerfQuery(&psSortStart);             /* query the performance counter */
  SortProcess();                                      /* call sort functions */
  ToolsPerfQuery(&psSortEnd);               /* query the performance counter */


  rc = DosFreeMem(Globals.pszFileBuffer);          /* free the memory buffer */
  ToolsPerfQuery(&psEnd);                   /* query the performance counter */

  if (Options.fsVerbose)                                 /* verbose output ? */
  {
    float fSort  = psSortEnd.fSeconds - psSortStart.fSeconds;

    printf   ("\nTotal time    is %10.3fs.",
            psEnd.fSeconds - psStart.fSeconds);

    if (fSort != 0.0)                              /* avoid division by zero */
      printf ("\nSorting time  is %10.3fs"
              "\nSorting speed is %10.3f lines per second.",
              fSort,
              Globals.ulStringsCurrent / fSort);
  }

  return (rc);                                             /* signal success */
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

  memset (&Options,                      /* initialize the global structures */
          0,
          sizeof(Options));

  memset (&Globals,
          0,
          sizeof(Globals));


  rc = ArgStandard (argc,                            /* CLI-Parameter parsen */
                    argv,
                    TabArguments,
                    &Options.fsHelp);
  if (rc != NO_ERROR)
  {
    ToolsErrorDos(rc);                                /* print error message */
    exit(1);                                                /* abort program */
  }

  if (Options.fsHelp)                                  /* user requests help */
  {
    help();
    ArgHelp(TabArguments);
    return (NO_ERROR);
  }

  if (Options.fsFileInput)  /* check whether sorting from stdin of from file */
    rc = SortProcessFile(Options.pszFileInput);
  else
    rc = SortProcessSTDIN();                              /* sort from stdin */

  if (rc != NO_ERROR)                                 /* print error message */
    ToolsErrorDos(rc);

  return (rc);
}
