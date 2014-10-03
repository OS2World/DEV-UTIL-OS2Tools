/*****************************************************
 * Unix-like NetCat Tool.                            *
 *                                                   *
 * (c) 2002 Patrick Haller                           *
 *****************************************************/

 /* NOTES:
  - iostream \\.\Physical_Disk1,NULL: works with current OS/2 kernels!
  - iostream \\.\C:,NULL: works with current OS/2 kernels!
  - be EXTREMELY careful with these options
  
  * TODO:
  - pipe stream
  - HEX filter
  - TEXT filter
  - follow mode ???
  - signal handler (ctrl-c)
  */
  

// #define DEBUG 1
// #undef DEBUG


/*****************************************************************************
 * Includes                                                                  *
 *****************************************************************************/

#ifdef __OS2__
  #define INCL_DOSMISC
  #define INCL_DOSFILEMGR
  #define INCL_BASE
  #define INCL_NOPMAPI
  #include <os2.h>
#endif

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <process.h>
#include <ctype.h>

#include "tooltypes.h"
#include "tools.h"
#include "toolarg.h"


#include "iostream.h"


/*****************************************************************************
 * Defines                                                                   *
 *****************************************************************************/

/* TCP/IP header additions (to equalize backlevel headers */
#ifndef SO_REUSEPORT
#define SO_REUSEPORT 0x1000
#endif


#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif
#ifdef MAXHOSTNAMELEN
#undef MAXHOSTNAMELEN		/* might be too small on aix, so fix it */
#endif
#define MAXHOSTNAMELEN 256



/*****************************************************************************
 * Structures                                                                *
 *****************************************************************************/

typedef struct
{
  ARGFLAG fsHelp;                     /* help is requested from command line */
  ARGFLAG fsVerbose;                                    /* verbose operation */
  ARGFLAG fsTestStreams;                         /* test stream availability */
  ARGFLAG fsContinueOnErrors;              /* continue operation upon errors */
  ARGFLAG fsTimeout;             /* timeout for connects and final net reads */
  ULONG   ulTimeout;
  ARGFLAG fsStreams;                                  /* stream object chain */
  PSZ     pszStreams;                                 /* stream object chain */
  ARGFLAG fsOverwrite;                             /* overwrite target files */
  ARGFLAG fsBufferSize;                                   /* i/o buffer size */
  ULONG   ulBufferSize;
} OPTIONS, *POPTIONS;


/*****************************************************************************
 * Global                                                                    *
 *****************************************************************************/

OPTIONS Options;                /* this structure holds command line options */
GLOBALS Globals;                /* this structure holds global variables     */


ARGUMENT TabArguments[] =
{ /*Token--------Beschreibung-----------------pTarget---------------ucTargetFormat--pTargetSpecified--*/
  {"/?",         "Get help screen.",          NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/H",         "Get help screen.",          NULL,                 ARG_NULL,       &Options.fsHelp},
  {"/V",         "Verbose operation.",        NULL,                 ARG_NULL,       &Options.fsVerbose},
  {"/A",         "Test stream type availability.",  NULL,           ARG_NULL,       &Options.fsTestStreams},
  {"/C",         "Continue operation on errors.", NULL,             ARG_NULL,       &Options.fsContinueOnErrors},
  {"/O",         "Overwrite target files.",   NULL,                 ARG_NULL,       &Options.fsOverwrite},
  {"/W=",        "Timeout for connects and final net reads", &Options.ulTimeout, ARG_ULONG, &Options.fsTimeout},
  {"/B=",        "I/O buffer size (64k default)", &Options.ulBufferSize, ARG_ULONG, &Options.fsBufferSize},
  {"1",          "Stream object chain",       &Options.pszStreams,  ARG_PSZ |
                                                                    ARG_MUST |
                                                                    ARG_DEFAULT,    &Options.fsStreams},
  ARG_TERMINATE
};


/*****************************************************************************
 * Prototypes                                                                *
 *****************************************************************************/

void   help                (void);

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
  TOOLVERSION("IOStream",                               /* application name */
              0x00010004,                           /* application revision */
              0x00010100,          /* minimum required revision of PHSTOOLS */
              0x0001FFFF,       /* maximum recommended revision of PHSTOOLS */
              NULL,                                               /* Remark */
              NULL);                                /* additional copyright */
  
  printf("\nAdditional information:\n"
         "  'iostream \\\\.\\Physical_Disk1,NULL:' and\n"
         "  'iostream \\\\.\\C:,NULL:' work fine with modern OS/2 kernels!\n"
         "   * !! USE WITH EXTREME CAUTION !! *\n"
         "\n"
         "Input Streams                           Output Streams\n"
         "--------------------------------------- ---------------------------------------\n"
         "STDIN:   standard input stream          STDOUT: standard output stream\n"
         "ONE:     stream sends binary ones       NULL:   stream swallows everything\n"
         "ZERO:    stream sends binary zeros\n"
         "RANDOM:  stream sends random input\n"
         "TCP:port read from TCP connection       TCP:host:port connect to host on port\n"
         "LOG:C:   read logical C: partition      LOG:D:  write to logical D: partition\n"
         "\n"
         "Anything else tries to open the specified parameter as a file.\n"
         "Stream concatenation is (will be) possible like\n"
         "'iostream STDIN:,GZIP:,STDOUT:'\n");
}





 /*
  
  IOStream
  +---IOStreamInput
  !   +--IOStreamInputStdIn   OK
  !   +--IOStreamInputTCP
  !   +--IOStreamInputFile
  !   +--IOStreamInputDASD
  !   +--IOStreamInputOne     OK
  !   +--IOStreamInputZero    OK
  !   +--IOStreamInputRandom  OK
  +---IOStreamOutput
      +--IOStreamOutputStdOut OK
      +--IOStreamOutputTCP
      +--IOStreamOutputFile
      +--IOStreamOutputGZIP
      +--IOStreamOutputUnGZIP
      +--IOStreamOutputBZIP2
      +--IOStreamOutputUnBZIP2
      +--IOStreamOutputDASD
      +--IOStreamOutputNull   OK
      */



/***********************************************************************
 * Name      : Main I/O transfer function
 * Funktion  : 
 * Parameter : - input stream object
 *             - output stream object
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-03-07]
 ***********************************************************************/

int IOTransfer(IOStreamInput *in,
               IOStreamOutput *out)
{
  ULONG      ulIOBufferSize = OS2READSIZE;
  BOOL       flagFinished = FALSE;
  APIRET     rc;
  PERFSTRUCT ts1;
  PERFSTRUCT ts2;
  float      fSecondsRead = 0;
  float      fSecondsWrite = 0;
  ULONG      ulIOTotal = 0;
  ULONG      ulIOTotalEstimated;
  float      fIOTotalEstimated100 = 0.0;
  char       szSizeBuffer[32];
  
  Debug(("Opening i/o stream objects"))
    
  // setup i/o buffer size
  if (Options.fsBufferSize)
  {
    if (Options.ulBufferSize > 0)
      ulIOBufferSize = Options.ulBufferSize;
  }
  
  // open input stream
  rc = in->connect();
  if (NO_ERROR != rc)
  {
    ToolsErrorDosEx(rc,
                    "Opening input stream object");
    return rc;
  }
  
  // @@@PH add 64-bit support
  ulIOTotalEstimated = in->estimatedStreamLength();
  fIOTotalEstimated100 = ulIOTotalEstimated / 100.0;
  
  if (Options.fsVerbose)
  {
    StrValueToSize(szSizeBuffer,
                   ulIOTotalEstimated);
    printf("%s expected from input stream.\n",
           szSizeBuffer);
  }
  
  
  // open output stream
  rc = out->connect( in );
  if (NO_ERROR != rc)
  {
    ToolsErrorDosEx(rc,
                    "Opening output stream object");
    return rc;
  }
  
  Debug(("Setting up i/o stream buffers"))
  
  IOBuffer* ioBuffer1 = new IOBuffer( ulIOBufferSize );
  
  // do the transfer
  do
  {
    ToolsPerfQuery (&ts1);
    rc = in->read( ioBuffer1 );
    ToolsPerfQuery (&ts2);
    fSecondsRead += ts2.fSeconds - ts1.fSeconds;
    
    if (NO_ERROR != rc)
    {
      ToolsErrorDosEx(rc,
                      "Reading from input object");
      
      // abort on errors?
      if (!Options.fsContinueOnErrors)
        return rc;
    }
    else
    {
      // sum up the total of the transferred bytes
      ulIOTotal += ioBuffer1->getValid();
    }
    
    // if no data came in, terminate on the next opportunity
    if (ioBuffer1->getValid() == 0)
      flagFinished = 1;
    
    ToolsPerfQuery (&ts1);
    rc = out->write( ioBuffer1 );
    ToolsPerfQuery (&ts2);
    fSecondsWrite += ts2.fSeconds - ts1.fSeconds;
    
    if (NO_ERROR != rc)
    {
      ToolsErrorDosEx(rc,
                      "Writing to output object");
      
      // abort on errors?
      if (!Options.fsContinueOnErrors)
        return rc;
    }
    
    if (Options.fsVerbose)
    {
      float fRead  = ulIOTotal / fSecondsRead / 1024.0;
      float fWrite = ulIOTotal / fSecondsWrite / 1024.0;
      float fTotal = ulIOTotal / (fSecondsRead + fSecondsWrite) / 1024.0;
      
      StrValueToSize(szSizeBuffer,
                     ulIOTotal);
      
      // if we know how much data to expect, we can calculate
      // the current percentage
      if (ulIOTotalEstimated)
      {
        float fPercent = ulIOTotal / fIOTotalEstimated100;
        printf("\r%%%5.1f: %-10s,r %10.2f kb/s,w %10.2f kb/s,tot %10.2f kb/s",
               fPercent,
               szSizeBuffer,
               fRead,
               fWrite,
               fTotal);
      }
      else
        printf("\r%12ub,r %10.3f kb/s,w %10.3f kb/s,tot %10.3f kb/s",
               ulIOTotal,
               fRead,
               fWrite,
               fTotal);
    }
  }
  while (!flagFinished);
  
  Debug(("Closing down i/o stream objects"))
    
    
  rc = in->disconnect();
  if (NO_ERROR != rc)
    ToolsErrorDosEx(rc,
                    "Closing input stream object");

  rc = out->disconnect();
  if (NO_ERROR != rc)
    ToolsErrorDosEx(rc,
                    "Closing output stream object");
  
  Debug(("Closing down i/o buffers"))
    
  delete ioBuffer1;
  
  return NO_ERROR;
}


/***********************************************************************
 * Name      : I/O stream chain setup method
 * Funktion  : 
 * Parameter : 
 * Variablen :
 * Ergebnis  : - input stream object
 *             - output stream object
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-03-07]
 ***********************************************************************/

APIRET IOStreamSetup(PSZ pszStreams,
                     IOStreamInput **  pin,
                     IOStreamOutput ** pout)
{
  if (NULL == pszStreams)
    return ERROR_INVALID_PARAMETER;
  
  IOStreamOutput* lastStream = NULL;
  IOStreamInput*  in         = NULL;
  IOStreamOutput* out        = NULL;
  IOStreamOutput* outTemp    = NULL;
  
  PSZ pszStreamToken = strtok(pszStreams, ",");
  while (pszStreamToken)
  {
    // Debug(("StreamToken '%s' found'",
    //       pszStreamToken))

    // now determine what kind of stream we have
    // and how to chain it in
    if (NULL == in)
    {
      // if no input stream was yet assigned, get the first one
      if (0 == strnicmp("STDIN:", pszStreamToken, 6))
        in = new IOStreamInputStdIn();
      else
      if (0 == strnicmp("ONE:", pszStreamToken, 4))
        in = new IOStreamInputOne();
      else
      if (0 == strnicmp("ZERO:", pszStreamToken, 5))
        in = new IOStreamInputZero();
      else
      if (0 == strnicmp("RANDOM:", pszStreamToken, 7))
        in = new IOStreamInputRandom();
      else
      if (0 == strnicmp("TCP:", pszStreamToken, 4))
        in = new IOStreamInputTCP( pszStreamToken );
      else
      if (0 == strnicmp("LOG:", pszStreamToken, 4))
        in = new IOStreamInputDASDLogical( pszStreamToken );
      else
//      if (strnicmp, "DASD:", pszStreamToken, 5)
//        in = new IOStreamInputDASD( pszStreamToken );
//      else
        // Default
        in = new IOStreamInputFile( pszStreamToken );
    }
    else
    {
      // create yet another output stream
      if (0 == strnicmp("STDOUT:", pszStreamToken, 7))
        outTemp = new IOStreamOutputStdOut();
      else
      if (0 == strnicmp("NULL:", pszStreamToken, 5))
        outTemp = new IOStreamOutputNull();
      else
      if (0 == strnicmp("TCP:", pszStreamToken, 4))
        outTemp = new IOStreamOutputTCP( pszStreamToken );
      else
      if (0 == strnicmp("LOG:", pszStreamToken, 4))
        outTemp = new IOStreamOutputDASDLogical( pszStreamToken );
      else
//      if (strnicmp, "DASD:", pszStreamToken, 5)
//        outTemp = new IOStreamOutputDASD( pszStreamToken );
//      else
        // Default
        outTemp = new IOStreamOutputFile( pszStreamToken );
      
      // if there was another output stream before,
      // chain them
      if (NULL == out)
        // first in output stream chain
        out = outTemp; 
      else
        // lastStream cannot be zero at this point
        lastStream->associate( outTemp );
      
      lastStream = outTemp;
    }
      
    // get next stream token
    pszStreamToken = strtok(NULL, ",");
  }
  
  
  // if no input stream was given, revert to stdin
  if (NULL == in)
    in = new IOStreamInputStdIn();
  
  // if no output stream was given, revert to stdout
  if (NULL == out)
    out= new IOStreamOutputStdOut();
  
  // verify if the initialized streams are available
  // @@@PH
  if (in->isAvailable() == FALSE)
  {
    ToolsErrorDosEx(ERROR_NOT_SUPPORTED,
                    in->getStreamName());
    return ERROR_NOT_SUPPORTED;
  }
  
  if (out->isAvailable() == FALSE)
  {
    ToolsErrorDosEx(ERROR_NOT_SUPPORTED,
                    out->getStreamName());
    return ERROR_NOT_SUPPORTED;
  }
  
  
  *pin  = in;
  *pout = out;
  
  return NO_ERROR;
}


/***********************************************************************
 * Name      : I/O stream availability test
 * Funktion  : 
 * Parameter : 
 * Variablen :
 * Ergebnis  : 
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-03-07]
 ***********************************************************************/

APIRET testStreamAvailability( void )
{
#define NR_OF_STREAMS 12
  IOStream *stream[ NR_OF_STREAMS ];
  
  // create all known stream types
  stream[ 0] = new IOStreamInputStdIn();
  stream[ 1] = new IOStreamInputOne();
  stream[ 2] = new IOStreamInputZero();
  stream[ 3] = new IOStreamInputRandom();
  stream[ 4] = new IOStreamInputTCP("TCP:0.0.0.0");
  stream[ 5] = new IOStreamInputDASDLogical("LOG:0");
  stream[ 6] = new IOStreamInputFile("nofile");
  stream[ 7] = new IOStreamOutputStdOut();
  stream[ 8] = new IOStreamOutputNull();
  stream[ 9] = new IOStreamOutputTCP("TCP:0.0.0.0");
  stream[10] = new IOStreamOutputDASDLogical("LOG:0");
  stream[11] = new IOStreamOutputFile("nofile");
  
  printf("Stream type availability:\n"
         "-------------------------\n");
  
  for (int i = 0;
       i < NR_OF_STREAMS;
       i++)
  {
    // test all streams
    printf("%2d %s %-25s %-12s %s\n",
           i,
           stream[i]->isAvailable() ? "OK" : "--",
           stream[i]->getStreamName(),
           stream[i]->getStreamType(),
           stream[i]->getStreamDevice());
    
    // delete this stream
    delete stream[i];
  }
  
  
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

  memset(&Options,
         0,
         sizeof( Options ));
  
  memset(&Globals,
         0,
         sizeof( Globals ));
  
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
  
  if ( Options.fsTestStreams )
  {
    testStreamAvailability();
    return NO_ERROR;
  }
  
  /* copy global configuration values */
  Globals.fsOverwrite = (BOOL)Options.fsOverwrite;
  
  /* @@@PH setup stream chain */
  IOStreamInput*  in; 
  IOStreamOutput* out;
  
  rc = IOStreamSetup(Options.pszStreams,
                     &in,
                     &out);
  if (rc != NO_ERROR)
  {
    ToolsErrorDosEx(rc,
                    "Setting up stream objects");
    return rc;
  }
  
  rc = IOTransfer(in, out);

  if (rc != NO_ERROR)
    ToolsErrorDos(rc);
  
  delete in;
  delete out;
  
  return (rc);
}