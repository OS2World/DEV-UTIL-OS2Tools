/* $Id: robin.h,v 1.2 2001/03/02 11:10:07 phaller Exp $ 
 */

/*
 * Robin - advanced command interpreter for OS/2
 * (c) 2000-2001 Patrick Haller <patrick.haller@innotek.de>
 *
 * Note: this work is based on work of the FreeDOS / FreeCOM and Reactos team.
 *       (GPL applies)
 *
 * $Log: robin.h,v $
 * Revision 1.2  2001/03/02 11:10:07  phaller
 * .
 *
 * Revision 1.1  2001/02/15 15:55:58  phaller
 * .
 *
 */


#ifndef _ROBIN_H_
#define _ROBIN_H_


/****************************************************************************
 * Includes
 ****************************************************************************/

#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>

#include "toolstack.h"


/****************************************************************************
 * Defines
 ****************************************************************************/

#define SESSION_STANDARD   0
#define SESSION_REDIRECTED 1


/****************************************************************************
 * Type definitions
 ****************************************************************************/

                                    /* create an alist for the file handles */
typedef HFILE HANDLE;


/* this is a simple 64-bit timestamp as returned by OS/2's hires timer APIs */
typedef struct
{
  ULONG ulLo;
  ULONG ulHi;
} TIMESTAMP, *PTIMESTAMP;


                    /* this is a short descriptor for a command's parameter */
typedef struct
{
  PSZ   pszParamLong;                         /* long form of the parameter */
  PSZ   pszParamShort;                       /* short form of the parameter */
  ULONG ID;                       /* parameter identifier for message table */
} CMD_PARAMETER, *PCMD_PARAMETER;


       /* this is a complete command descriptor including function pointers */
typedef struct
{
  PSZ   pszCommand;                       /* command name token (uppercase) */
  ULONG ID;                         /* command identifier for message table */
  
  PCMD_PARAMETER *arrpCmdParameters;        /* list of available parameters */
  
  void *pfnExecute;                        /* actually execute this command */
  void *pfnProgress;                        /* progress indication callback */
  void *pfnDialog;                /* dialog procedure for specified command */

  ULONG ulStackSize;             /* required stack size for async execution */
} CMD_COMMAND, *PCMD_COMMAND;


                             /* this structure models a parsed command line */
typedef struct
{
  PSZ pszCommandLine;                /* the original, complete command line */
  
  ULONG ulTokens;      /* total number of parsed tokens of the command line */
  PSZ *arrpTokens;                    /* pointer array to the parsed tokens */
} CMD_LINE, *PCMD_LINE;


                  /* this models a completion to lookup for command completion */
typedef struct
{
  PSZ   pszName;                              /* name of the completion object */
  BOOL  flagValid;      /* set to false if any other command invalidates it */
                            /* such as adding an environment variable, etc. */
  ULONG ulItems;             /* total number of items in the completion object */
  PSZ   *arrpItems;                  /* array of ascii strings of the items */
} COMPLETION, *PCOMPLETION;


typedef struct
{
  ULONG  ulType;                            /* type of this command session */
  
  HANDLE hOut;               /* handles to send output to / read input from */
  HANDLE hIn;              /* these may be altered during command execution */
  HANDLE hError;                     /* (foreground / background switching) */
                         /* probably jump scrolling can be implemented, too */
  
  BOOL flagTerminate;            /* set to true as termination is requested */
  
  ULONG ulErrorLevel;  /* store the error code of the last executed command */
  PSZ   pszPrompt;                             /* the current prompt string */
  BOOL  flagBreak;                                /* enable / disable break */
  
  PSTACK   pStackDirectory;                              /* directory stack */
  PCOMPLETION pCompletionEnvironment;      /* environment completion object */
  PCOMPLETION pCompletionCommand;              /* command completion object */
  PCOMPLETION pCompletionHistory;             /* typed commands of the user */
} CMD_SESSION, *PCMD_SESSION;

                          /* this is the complete command execution context */
typedef struct
{
  BOOL flagDebug;          /* enable debugging for this command execution ? */
  BOOL flagSimulate;                 /* only simulate execution, do nothing */
  BOOL flagAsynchronous;       /* true if this is an asynchronous execution */
  
  TIMESTAMP tsStart;                           /* for benchmarking purposes */
  TIMESTAMP tsEnd;
  
  ULONG ulReturnCode;     /* the return code of the (last) executed command */
  
  HANDLE hOut;               /* handles to send output to / read input from */
  HANDLE hIn;              /* these may be altered during command execution */
  HANDLE hError;                     /* (foreground / background switching) */
                         /* probably jump scrolling can be implemented, too */
  
  PCMD_COMMAND pCmdCommand;         /* the (current) command to be executed */
  PCMD_LINE pCmdLine;     /* the (parsed and prepared) current command line */
} CMD_CONTEXT, *PCMD_CONTEXT;


/****************************************************************************
 * Prototypes
 ****************************************************************************/

/* memory management */
void* memAlloc(ULONG ulSize);
void memFree(PVOID pvObject);
void memCopy(PVOID pvDest, PVOID pvSource, ULONG ulLength);

/* string functions */
ULONG stringLength(PSZ pszString);

/* completion management */
PCOMPLETION completionGetEnvironment(void);
PCOMPLETION completionGetCommand(void);
PCOMPLETION completionGetHistory(PSZ pszHome);
void completionDelete(PCOMPLETION pCompletion);
PCOMPLETION completionNew(PSZ pszName);

/* console stream management */
APIRET ConsoleOutString(PCMD_SESSION pSession, PSZ pszString);
APIRET ConsoleOutPrint(PCMD_SESSION pSession, PSZ pszFormat, ...);
APIRET ConsoleReadString(PCMD_SESSION pSession, PSZ pszLine, PULONG pulLineLength);

#endif /* _ROBIN_H_ */
