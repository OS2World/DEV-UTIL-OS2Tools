/***********************************************************************
 * Name      : Kommentargenerator/2
 * Autor     : Patrick Haller [Freitag,01.04.94 - 16:39:32]
 ***********************************************************************/

#ifndef KOMMGEN_INCLUDES
  #define KOMMGEN_INCLUDES 1

  #define INCL_WIN
  #define INCL_WINSYS
  #define INCL_WINWINDOWMGR
  #define INCL_GPIBITMAPS
  #define INCL_WINLISTBOXES
  #define INCL_DOS
  #define INCL_DOSNLS
  #define INCL_DOSDATETIME

  #include <os2.h>                      /* System Include File      */

  #include <string.h>
  #include <stdio.h>
  #include <malloc.h>
  #include <process.h>
  #include "kommgen.h"                  /* Application Include File */
#endif


#define KGMAINWINDOW "KGMainWindow"
#define KGTEMPWINDOW "KGTemplateWindow"
#define APPININAME   "KommGen.INI"
#define APPNAME      "Kommentargenerator"

#define MAXPATHLEN 260
#define NFILES     32
#define FFBUF      NFILES * 4
#define ID_MLE     101

#define error(a) WinMessageBox (HWND_DESKTOP,HWND_DESKTOP,(a),"Error:",0,MB_OK)


/***********************************************************************
 * Name      : Prototypen
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag,16.11.93 - 16:39:32]
 ***********************************************************************/

MRESULT EXPENTRY MainWinProc    (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY TemplateWinProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY CommentDlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT EXPENTRY AboutDlgProc   (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

void CommentProcess     (HWND hwnd);
void TemplateProcess    ( void );

void ProfileWrite       (void);
void ProfileRead        (void);
void MainFillListbox    (void);
void MainProcessListbox (void);
void MLboxGetPresparams (void);
void MLboxSetPresparams (void);
void CommentFillType    (HWND hwnd);


/* KOMMEDIT.C */
void  MLEInsert         (char *);
void  MLEClear          (void);
BOOL  MLEChanged        (void);
void  MLEReset          (void);
void  FileRead          (char *);
void  FileWrite         (char *);
void  FileNew           (void);
void  TemplateName      (char *);
void  EditRead          (HWND);
void  EditSaveAs        (HWND);
ULONG MLEGetBuffer      (char **);
void  MLEGetPresparams  (void);
void  MLESetPresparams  (void);

/* KOMMCOMM.C */
void  strDate           (char *);
void  strReplace        (char **,ULONG *,char *,char *);
void  CopyToClipboard   (HAB,PSZ);


/***********************************************************************
 * Name      : Globale Variablen
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag,16.11.93 - 16:39:32]
 ***********************************************************************/

extern HAB     hab;
extern HWND    hwndMain;
extern HWND    hwndClientMain;
extern HWND    hwndClientTemp;
extern HWND    hwndMLE;
extern HMODULE hModKommgen;
extern HWND    hwndTemp;
extern HWND    hwndMainListbox;
extern HINI    hInit;

extern BOOL    flTemplateEditorShown;
extern BOOL    flTemplateChanged;
extern BOOL    flArgumentSpecified;
extern PSZ     TTextBuffer; /* Puffer fÅr den Vorlagentext */
extern BOOL    flTemplateNamed;
extern PSZ     szTemplateName;

typedef struct {
	 char fontname[40];
	 RGB2 fgcolor;
	 RGB2 bkcolor;
       } TWINPARAMS;

extern TWINPARAMS MLEPARAMS;
