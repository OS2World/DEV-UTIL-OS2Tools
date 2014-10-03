/***********************************************************************
 * Name      : Kommentargenerator/2
 * Autor     : Patrick Haller [Freitag,01.04.94 - 16:39:32]
 ***********************************************************************/

#include "KommIncl.h"

/***********************************************************************
 * Name      : Globale Variablen
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag,16.11.93 - 16:39:32]
 ***********************************************************************/

HAB     hab             = NULLHANDLE;
HWND    hwndMain        = NULLHANDLE;
HWND    hwndClientMain  = NULLHANDLE;
HMODULE hModKommgen     = 0L;
HWND    hwndMainListbox = NULLHANDLE;
HINI    hInit           = NULLHANDLE;

struct  /* Window Positions, etc. */
{
  SWP PosMain;
  SWP PosTemp;
  SWP PosComment;
} Options;

TWINPARAMS MLBOXPARAMS;


/***********************************************************************
 * Name      : void main
 * Funktion  : Initialisieren der Applikation
 * Parameter : int argc, char*argv[]
 * Ergebnis  : -
 * Bemerkung : Start via DnD -> Auswerten der Kommandozeile.
 *
 * Autor     : Patrick Haller [Donnerstag,16.11.93 - 16:39:32]
 ***********************************************************************/

void main(void)
{
    QMSG qmsg;               /* Message Queue                */
    HMQ  hmq;                /* Handle to Queue              */
    ULONG flCreate1;         /* Creation flags               */
    ULONG flCreate3;

    HPOINTER hptrMain;       /* Icons                        */
    HPOINTER hptrTemp;

    hab = WinInitialize((USHORT) NULL); /* Intialize PM and obtain anchor block */
    hmq = WinCreateMsgQueue(hab,0);     /* Create queue to receive messsages */

    ProfileRead (); /* Get a profile */

    /* Register the window class KGMAINWINDOW with procedure MainWinProc */
    WinRegisterClass(hab,KGMAINWINDOW,(PFNWP) MainWinProc,
                     CS_SIZEREDRAW,0);

    /* Register the window class KGTEMPWINDOW with procedure TemplateWinProc */
    WinRegisterClass(hab,KGTEMPWINDOW,(PFNWP) TemplateWinProc,
           CS_SIZEREDRAW,0);

    flCreate1= FCF_AUTOICON | FCF_MAXBUTTON | FCF_MENU | FCF_MINBUTTON |
               FCF_SIZEBORDER | FCF_SYSMENU | FCF_TASKLIST | FCF_TITLEBAR;

    flCreate3= FCF_MAXBUTTON | FCF_MENU | FCF_MINBUTTON | FCF_SIZEBORDER |
               FCF_AUTOICON | FCF_SYSMENU | FCF_TITLEBAR;

    /* Create a standard window */
    hwndMain = WinCreateStdWindow(HWND_DESKTOP,
                 0L, &flCreate1, KGMAINWINDOW,
                 "Kommentar Generator/2", 0L, hModKommgen,
                 ID_DLGMAIN, (PHWND) &hwndClientMain);

    /* Create a standard template window */
    hwndTemp = WinCreateStdWindow(HWND_DESKTOP,
                 0L, /* Window is NOT visible */
                 &flCreate3, KGTEMPWINDOW,
                 "Vorlageneditor", 0L, hModKommgen,
                 ID_DLGTEMP,(PHWND) &hwndClientTemp);

    WinSetWindowPos(hwndMain,HWND_TOP,
                    Options.PosMain.x, Options.PosMain.y,
                    Options.PosMain.cx,Options.PosMain.cy,
                    SWP_ACTIVATE | SWP_SIZE | SWP_MOVE | SWP_SHOW);
    WinSetWindowPos(hwndTemp,HWND_TOP,
                    Options.PosTemp.x, Options.PosTemp.y,
                    Options.PosTemp.cx,Options.PosTemp.cy,
                    SWP_MOVE | SWP_SIZE);

    /* set an icon for the dialog */
    hptrMain = WinLoadPointer(HWND_DESKTOP, 0,ID_DLGMAIN );
    WinSendMsg( hwndMain, WM_SETICON, (MPARAM)hptrMain, 0L );
    hptrTemp = WinLoadPointer(HWND_DESKTOP, 0,ID_DLGTEMP );
    WinSendMsg( hwndTemp, WM_SETICON, (MPARAM)hptrTemp, 0L );

    /* Message Loop */
    while ( WinGetMsg(hab,(PQMSG) &qmsg,(HWND) NULL,0,0))
       WinDispatchMsg(hab,(PQMSG) &qmsg );

    /* Get the editor's presentation parameters */
    MLEGetPresparams ();
    MLboxGetPresparams ();

    WinDestroyPointer(hptrMain); /* Destroy the icons               */
    WinDestroyPointer(hptrTemp); /* Destroy the icons               */
    WinDestroyWindow(hwndMain);  /* Destroy the window              */
    WinDestroyWindow(hwndTemp);  /* Destroy the window              */

    ProfileWrite ();

    if (TTextBuffer)
      free (TTextBuffer);

    WinDestroyMsgQueue( hmq );   /* Destroy the message queue       */
    WinTerminate( hab );         /* Terminate and release resources */
}


/***********************************************************************
 * Name      : MRESULT EXPENTRY MainWinProc
 * Funktion  : Messagehandling des Hauptfensters
 * Parameter : HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag,16.11.93 - 16:39:32]
 ***********************************************************************/

MRESULT EXPENTRY MainWinProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    RECTL rectl;             /* Dimensions of window         */

    switch(msg)
    {
        case WM_COMMAND:
   switch (SHORT1FROMMP(mp1))
   {
       case CM_FILENEW:    /* ~Neu */
              FileNew ();
              break;

       case CM_FILEOPEN:    /* ~ôffnen */
              EditRead (hwnd);
              break;

       case CM_FILESAVE:    /* ~Speichern */
              if (flTemplateNamed)
                FileWrite (szTemplateName);
              else
                EditSaveAs (hwnd);
              break;

       case CM_FILESAVEAS:    /* Speichern ~unter ... */
              EditSaveAs (hwnd);
              MainFillListbox ();
         break;

       case CM_EXIT:    /* ~Beenden */
                if (MLEChanged() == TRUE)
                  WinPostMsg (hwnd,WM_QUIT,(MPARAM)0,(MPARAM)0);
              break;

       case CM_TEMPLATE:    /* ~Vorlage ... */
         TemplateProcess ();
              break;

       case CM_COMMENT:    /* ~Kommentar ... */
         CommentProcess(hwnd);
              break;

       case CM_HELPABOUT:    /* ~Produktinformation */
         WinDlgBox (HWND_DESKTOP,hwnd,(PFNWP) AboutDlgProc,hModKommgen,
          ID_DLGABOUT,NULL);
              break;
   }
   break;

    case WM_CONTROL:
   switch (SHORT1FROMMP(mp1))
   {
     case ID_MAINLISTBOX:
            switch (SHORT2FROMMP(mp1))
            {
              case LN_ENTER: /* OK, now load the template */
                MainProcessListbox ();
                break;
            }
     break;
   }
   break;

    case WM_SIZE:
        /* Resize the listbox */
        WinQueryWindowRect (hwndClientMain,&rectl);
        WinSetWindowPos (hwndMainListbox,hwndMain,
          rectl.xLeft,rectl.yBottom,
                         rectl.xRight,rectl.yTop,
                         SWP_MOVE | SWP_SIZE | SWP_SHOW);
        break;

    case WM_CREATE:
       /* Create the WC_LISTBOX, ""  */
       WinCreateWindow(hwnd,
             WC_LISTBOX,
             "",
                            LS_NOADJUSTPOS,
             0,
             0,
             373,
             250,
             hwnd,
             HWND_TOP,
             ID_MAINLISTBOX,
             0L,
             NULL);
            hwndMainListbox = WinWindowFromID (hwnd,ID_MAINLISTBOX);
            MainFillListbox();
            MLboxSetPresparams ();
            WinSetFocus (HWND_DESKTOP,hwndMainListbox);
       break;

    case WM_CLOSE: /* WM_QUIT Message kommt nicht an ?!? */
      if (MLEChanged() == FALSE) return ((MPARAM)TRUE);
      else
   return(WinDefWindowProc(hwnd,msg,mp1,mp2));

    case WM_DESTROY:
      WinQueryWindowPos (hwndMain,&(Options.PosMain));
      WinQueryWindowPos (hwndTemp,&(Options.PosTemp));
      break;

    case WM_ERASEBACKGROUND:
         return (MPARAM)FALSE;

    default:
   return(WinDefWindowProc(hwnd,msg,mp1,mp2));
  }
  return(FALSE);
}


/***********************************************************************
 * Name      : MRESULT EXPENTRY CommentDlgProc
 * Funktion  : Dialogprocedure des Kommentardia
 * Parameter : HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 15.04.1994 00.47.36]
 ***********************************************************************/

MRESULT EXPENTRY CommentDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
 CHAR CMF_Name[61];
 CHAR CMF_Parameter[81];
 CHAR CMF_Variables[81];
 CHAR CMF_Result[81];
 CHAR CMF_Author[61];
 CHAR CMF_Remark[81];
 CHAR CMF_Type[81];
 CHAR CMF_Function[81];

 CHAR szDate[80];

 char *buffer;
 ULONG buffersize;

    switch(msg)
    {
        case WM_COMMAND:
            switch (SHORT1FROMMP(mp1))
            {
      case PB_COPY:

          /* Read from entry field EF_NAME and place 60 characters in  CMF_Name */
          WinQueryDlgItemText(hwnd,
                                        EF_NAME,
                                        61,
                                        CMF_Name);

                    /* Read from entry field EF_PARAMETER and place 80 characters in  CMF_Parameter */
                    WinQueryDlgItemText(hwnd,
               EF_PARAMETER,
               81,
               CMF_Parameter);

          /* Read from entry field EF_VARIABLES and place 80 characters in  CMF_Variables */
          WinQueryDlgItemText(hwnd,
               EF_VARIABLES,
               81,
               CMF_Variables);

          /* Read from entry field EF_RESULT and place 80 characters in  CMF_Result */
          WinQueryDlgItemText(hwnd,
               EF_RESULT,
               81,
               CMF_Result);

          /* Read from entry field EF_AUTHOR and place 60 characters in  CMF_Author */
          WinQueryDlgItemText(hwnd,
               EF_AUTHOR,
               61,
               CMF_Author);

                    /* Read from entry field EF_FUNCTION and place 80 characters in  CMF_Author */
          WinQueryDlgItemText(hwnd,
                                        EF_FUNCTION,
                                        81,
                                        CMF_Function);

                    /* Read from entry field EF_REMARK and place 80 characters in  CMF_Remark */
          WinQueryDlgItemText(hwnd,
               EF_REMARK,
               81,
               CMF_Remark);

          /* Read from entry field CB_TYPE and place VARLENGTH characters in  VARNAME */
          WinQueryDlgItemText(hwnd,
               CB_TYPE,
               81,
               CMF_Type);

          /* OK, this is the work ! */
                    strDate (szDate);

          /* Get the text */
          buffersize = 1024;
          buffer     = malloc(buffersize);
          buffersize = MLEGetBuffer (&buffer);
          if (!buffer)
          {
            error ("out of memory");
            break;
          }

          strReplace (&buffer,
            &buffersize,
            "[NAME]",
            CMF_Name);
          strReplace (&buffer,&buffersize,"[TYP]",CMF_Type);
          strReplace (&buffer,&buffersize,"[FUNKTION]",CMF_Function);
          strReplace (&buffer,&buffersize,"[PARAMETER]",CMF_Parameter);
          strReplace (&buffer,&buffersize,"[VARIABLEN]",CMF_Variables);
          strReplace (&buffer,&buffersize,"[ERGEBNIS]",CMF_Result);
          strReplace (&buffer,&buffersize,"[AUTOR]",CMF_Author);
          strReplace (&buffer,&buffersize,"[BEMERKUNG]",CMF_Remark);
          strReplace (&buffer,&buffersize,"[DATUM]",szDate);
          CopyToClipboard (hab,buffer);
          free (buffer);
      break;

         case PB_CANCEL:
      WinQueryWindowPos (hwnd,&(Options.PosComment));
      WinDismissDlg (hwnd,PB_CANCEL);
                break;

              case PB_NEW:
                WinSetDlgItemText (hwnd,EF_NAME,"");
                WinSetDlgItemText (hwnd,CB_TYPE,"");
                WinSetDlgItemText (hwnd,EF_PARAMETER,"");
                WinSetDlgItemText (hwnd,EF_FUNCTION,"");
                WinSetDlgItemText (hwnd,EF_VARIABLES,"");
                WinSetDlgItemText (hwnd,EF_RESULT,"");
                WinSetDlgItemText (hwnd,EF_REMARK,"");

                WinSetFocus (HWND_DESKTOP,WinWindowFromID (hwnd,EF_NAME));
                break;
            }
       break;

   case WM_INITDLG:

             WinSetWindowPos(hwnd,HWND_BOTTOM,
                    Options.PosComment.x, Options.PosComment.y,
                    Options.PosComment.cx,Options.PosComment.cy,
                    SWP_MOVE | SWP_SHOW);

      /* Max no. of chars for EF_NAME is 60 */
      WinSendDlgItemMsg(hwnd,
              EF_NAME,
              EM_SETTEXTLIMIT,
              MPFROMSHORT(60),
              0L);

      /* Max no. of chars for EF_PARAMETER is 80 */
      WinSendDlgItemMsg(hwnd,
              EF_PARAMETER,
              EM_SETTEXTLIMIT,
              MPFROMSHORT(80),
              0L);

      /* Max no. of chars for EF_VARIABLES is 80 */
      WinSendDlgItemMsg(hwnd,
              EF_VARIABLES,
              EM_SETTEXTLIMIT,
              MPFROMSHORT(80),
              0L);

      /* Max no. of chars for EF_FUNCTION is 80 */
      WinSendDlgItemMsg(hwnd,
              EF_FUNCTION,
              EM_SETTEXTLIMIT,
              MPFROMSHORT(80),
              0L);

      /* Max no. of chars for EF_RESULT is 80 */
      WinSendDlgItemMsg(hwnd,
              EF_RESULT,
              EM_SETTEXTLIMIT,
              MPFROMSHORT(80),
              0L);

      /* Max no. of chars for EF_AUTHOR is 60 */
      WinSendDlgItemMsg(hwnd,
              EF_AUTHOR,
              EM_SETTEXTLIMIT,
              MPFROMSHORT(60),
              0L);

      /* Max no. of chars for EF_REMARK_1 is 80 */
      WinSendDlgItemMsg(hwnd,
              EF_REMARK,
              EM_SETTEXTLIMIT,
              MPFROMSHORT(80),
              0L);

      /* Max no. of chars for EF_RESULT_1 is 80 */
      WinSendDlgItemMsg(hwnd,
              EF_RESULT,
              EM_SETTEXTLIMIT,
              MPFROMSHORT(80),
              0L);
      CommentFillType (WinWindowFromID(hwnd,CB_TYPE));
     break;

   default:
       return(WinDefDlgProc(hwnd,msg,mp1,mp2));
    }
    return(FALSE);
}


/***********************************************************************
 * Name      : void CommentProcess
 * Funktion  : Umschalten der Fenster fÅr Komme
 * Parameter : HWND hwnd
 * Variablen :
 * Ergebnis  :
 * Bemerkung : Haupt- und Editorfenster werden ausgeblendet.
 *
 * Autor     : Patrick Haller [Samstag, 15.04.1994 00.44.34]
 ***********************************************************************/

void CommentProcess (HWND hwnd)
{
  WinShowWindow (hwndMain,FALSE);
  WinShowWindow (hwndTemp,FALSE);
  WinDlgBox (HWND_DESKTOP,hwnd,(PFNWP) CommentDlgProc,hModKommgen,
        ID_DLGCOMMENT,NULL);
  WinShowWindow (hwndMain,TRUE);
  WinShowWindow (hwndTemp,flTemplateEditorShown);
}


/***********************************************************************
 * Name      : MRESULT EXPENTRY AboutDlgProc
 * Funktion  : Dialogroutine fÅr ID_DLGABOUT
 * Parameter : HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag,16.11.93 - 16:39:32]
 ***********************************************************************/

MRESULT EXPENTRY AboutDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch(msg)
    {
   case WM_CONTROL:
       switch (SHORT1FROMMP(mp1))
       {
         case DID_OK:  break;
       }
       break;
   default:
       return(WinDefDlgProc(hwnd,msg,mp1,mp2));
    }
    return(FALSE);
}

/***********************************************************************
 * Name      : void ProfileRead
 * Funktion  : User-Profile îffnen und lesen
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 26.11.1993 - 16:10:23]
 ***********************************************************************/

void ProfileRead (void)
{
  ULONG buflen  = sizeof(Options);
  ULONG buflen2 = sizeof(TWINPARAMS);

  hInit = PrfOpenProfile (hab,APPININAME);
  if (!hInit)
  {
    error ("Can't use profile");
    return;
  }

  PrfQueryProfileData   (hInit,APPNAME,"Options",&Options,&buflen);
  PrfQueryProfileData   (hInit,APPNAME,"Editor",&MLEPARAMS,&buflen2);
  PrfQueryProfileData   (hInit,APPNAME,"Listbox",&MLBOXPARAMS,&buflen2);

  /* Mu·te die INI neu angelegt werden ? */
  if (!Options.PosMain.cx & !Options.PosMain.cy &
      !Options.PosTemp.cx & !Options.PosTemp.cy)
  {
    Options.PosMain.cx = 350;
    Options.PosMain.cy = 150;
    Options.PosMain.x  = 0;
    Options.PosMain.y  = 0;
    Options.PosTemp.cx = 350;
    Options.PosTemp.cy = 200;
    Options.PosTemp.x  = 0;
    Options.PosTemp.y  = 150;
  }
}


/***********************************************************************
 * Name      : void ProfileWrite
 * Funktion  : User-Profile schreiben und beenden
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 26.11.1993 - 16:10:23]
 ***********************************************************************/

void ProfileWrite (void)
{
  if (!hInit) return;

  /* Profile schreiben */
  PrfWriteProfileData   (hInit,APPNAME,"Options",&Options,sizeof(Options));
  PrfWriteProfileData   (hInit,APPNAME,"Editor",&MLEPARAMS,sizeof(MLEPARAMS));
  PrfWriteProfileData   (hInit,APPNAME,"Listbox",&MLBOXPARAMS,sizeof(MLBOXPARAMS));
  PrfCloseProfile       (hInit);
}


/***********************************************************************
 * Name      : void MainFillListbox
 * Funktion  : Alle *.KGVs im aktuellen Verzeichnis auflisten...
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung : Multithreaded
 *
 * Autor     : Patrick Haller [Freitag, 26.11.1993 - 16:10:23]
 ***********************************************************************/

void MainFillListbox (void)
{
  APIRET        rc;
  HDIR          dirHandle = HDIR_SYSTEM;
  FILEFINDBUF3  *fileData;
  char          *ffbuf;
  ULONG         count;
  char          nextpath[MAXPATHLEN+2];


  WinEnableWindowUpdate(hwndMainListbox,FALSE);
  WinSendMsg (hwndMainListbox,LM_DELETEALL,(MPARAM)0,(MPARAM)0);

  strcpy (nextpath,"*.KGV");

  ffbuf = malloc(FFBUF);
  if (!ffbuf)
  {
    error ("MailFillListbox: out of memory");
    return;
  }

  count = NFILES;
  fileData = (FILEFINDBUF3 *)ffbuf;
  rc = DosFindFirst(nextpath,&dirHandle,
          FILE_ARCHIVED |
          FILE_READONLY,
          fileData,FFBUF,
          &count,FIL_STANDARD);
  if (rc)
  {
    error ("DosFindFirst");
    return;
  }

  while (!rc)
  {
    while (count)
    {
      if (fileData->achName[0] != '.')
   WinInsertLboxItem (hwndMainListbox,LIT_END,fileData->achName);

      count--;
      fileData = (FILEFINDBUF3 *) ((BYTE*)fileData + fileData->oNextEntryOffset);
    }
    count    = NFILES;
    fileData = (FILEFINDBUF3 *)ffbuf;
    rc = DosFindNext (dirHandle,fileData,FFBUF,&count);
  }

  DosFindClose(dirHandle);
  free(ffbuf);
  WinEnableWindowUpdate(hwndMainListbox,TRUE);
}

/***********************************************************************
 * Name      : void MainProcessListbox
 * Funktion  : Datei lesen
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung : Multithreaded
 *
 * Autor     : Patrick Haller [Freitag, 26.11.1993 - 16:10:23]
 ***********************************************************************/

void MainProcessListbox (void)
{
  SHORT rc;
  char  fnamebuf[MAXPATHLEN];

  if ( (rc = WinQueryLboxSelectedItem (hwndMainListbox)) ==
      LIT_NONE) error ("Keine Vorlage selektiert.");
  else
  {
    WinQueryLboxItemText (hwndMainListbox, rc,
           fnamebuf,MAXPATHLEN);
    FileRead (fnamebuf);
  }
}

/***********************************************************************
 * Name      : void MLboxGetPresparams (char **ptarget,ULONG *psize)
 * Funktion  : Ermitteln der aktuelle Presentation Parameters
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 26.11.1993 - 16:10:23]
 ***********************************************************************/

void MLboxGetPresparams (void)
{

  WinQueryPresParam (hwndMainListbox,PP_FOREGROUNDCOLOR,0L,NULL,sizeof(RGB2),
            &MLBOXPARAMS.fgcolor,
            QPF_NOINHERIT | QPF_PURERGBCOLOR);
  WinQueryPresParam (hwndMainListbox,PP_BACKGROUNDCOLOR,0L,NULL,sizeof(RGB2),
            &MLBOXPARAMS.bkcolor,
            QPF_NOINHERIT | QPF_PURERGBCOLOR);

  WinQueryPresParam (hwndMainListbox,PP_FONTNAMESIZE,0L,NULL,sizeof(MLBOXPARAMS.fontname),
            &MLBOXPARAMS.fontname,
            QPF_NOINHERIT);
}

void MLboxSetPresparams (void)
{
  WinSetPresParam (hwndMainListbox,PP_FOREGROUNDCOLOR,sizeof(RGB2),&MLBOXPARAMS.fgcolor);
  WinSetPresParam (hwndMainListbox,PP_BACKGROUNDCOLOR,sizeof(RGB2),&MLBOXPARAMS.bkcolor);
  WinSetPresParam (hwndMainListbox,PP_FONTNAMESIZE,strlen(MLBOXPARAMS.fontname)+1,&MLBOXPARAMS.fontname);
}


/***********************************************************************
 * Name      : void CommentFillType
 * Funktion  : FÅllen der Type-Listbox CB_TYPE
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 26.11.1993 - 16:10:23]
 ***********************************************************************/

void CommentFillType (HWND hwnd)
{
  static char *types[] = { "Procedure",
            "Function",
            "Unit",
            "Library",
            "Program",
            "Interface",
            "Implementation",
            "void",
            "int",
            "char",
            "unsigned",
            "APIRET",
            "short",
            "unsigned short",
            "unsigneg char",
            "unsigned int",
            "unsigned short",
            "unsigned long",
            "ULONG",
            "USHORT",
            "UCHAR",
            "UINT",
            "long",
            "char *",
            "PSZ",
            "BOOL",
            "MRESULT EXPENTRY",
            "structure",
            "union",
            "record",
            "Constructor",
            "class",
            "template",
            "Destructor",
            "cdecl",
            "Constants",
            "Declaration",
            "Definition",
            "Object",
            "",
            NULL};
  char *p;

  p = types[0];

  WinEnableWindowUpdate(hwnd,FALSE);
  while (*p)
  {
    WinInsertLboxItem (hwnd,LIT_SORTASCENDING,p);
    p += strlen (p) + 1;
  }
  WinEnableWindowUpdate(hwnd,TRUE);
}
