/***********************************************************************
 * Name      : Kommentargenerator/2 - Vorlageneditormodul
 * Autor     : Patrick Haller [Freitag,01.04.94 - 16:39:32]
 ***********************************************************************/

#include "KommIncl.h"


/***********************************************************************
 * Name      : Globale Variablen
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Donnerstag,16.11.93 - 16:39:32]
 ***********************************************************************/

HWND    hwndClientTemp        = NULLHANDLE;
HWND    hwndMLE               = NULLHANDLE;
HWND    hwndTemp              = NULLHANDLE;
BOOL    flTemplateEditorShown = FALSE;
BOOL    flTemplateNamed       = FALSE;
PSZ     szTemplateName        = NULL;
PSZ     TTextBuffer           = NULL;    /* Puffer fÅr den Vorlagentext */
PSZ     TemplateBuffer        = NULL;    /* Puffer fÅr die Vorlage      */
char    szBuf[MAXPATHLEN];

TWINPARAMS MLEPARAMS;


/***********************************************************************
 * Name      : MRESULT EXPENTRY TemplateWinProc
 * Funktion  : Windowprocedure fÅr den Editor
 * Parameter : HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2
 * Variablen : HPS hps, RECTL rectl,
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 15.04.1994 00.41.06]
 ***********************************************************************/


MRESULT EXPENTRY TemplateWinProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
//@@@PH    HPS hps;                 /* Handle to Presentation Space */
    RECTL rectl;             /* Dimensions of window         */

    switch(msg)
    {
      case WM_COMMAND:
        switch (SHORT1FROMMP(mp1))
        {
          /*flTemplateEditorShown = FALSE;*/
          case CM_CMIN_NAME:   MLEInsert ("[NAME]");      break; /* ~Name      */
          case CM_CMIN_TYP:    MLEInsert ("[TYP]");       break; /* ~Typ       */
          case CM_CMIN_FUNC:   MLEInsert ("[FUNKTION]");  break; /* ~Funktion  */
          case CM_CMIN_PARAMS: MLEInsert ("[PARAMETER]"); break; /* ~Parameter */
          case CM_CMIN_VARS:   MLEInsert ("[VARIABLEN]"); break; /* ~Variablen */
          case CM_CMIN_RESULT: MLEInsert ("[ERGEBNIS]");  break; /* ~Ergebnis  */
          case CM_CMIN_REMARK: MLEInsert ("[BEMERKUNG]"); break; /* ~Bemerkung */
          case CM_CMIN_AUTHOR: MLEInsert ("[AUTOR]");     break; /* ~Autor     */
          case CM_CMIN_DATE:   MLEInsert ("[DATUM]");     break; /* ~Datum     */
          case CM_CM_HELP:                                break; /* ~Hilfe     */

          case CM_CM_UNDO:
            WinSendMsg (hwndMLE,MLM_UNDO,(MPARAM)0L,(MPARAM)0L);
            break;
          case CM_CM_CUT:
            WinSendMsg (hwndMLE,MLM_CUT,(MPARAM)0L,(MPARAM)0L);
            break;

          case CM_CM_COPY:
            WinSendMsg (hwndMLE,MLM_COPY,(MPARAM)0L,(MPARAM)0L);
            break;

          case CM_CM_PASTE:
            WinSendMsg (hwndMLE,MLM_PASTE,(MPARAM)0L,(MPARAM)0L);
            break;

          case CM_CM_DELETE:
            WinSendMsg (hwndMLE,MLM_DELETE,(MPARAM)-1,(MPARAM)0L);
            break;
   }
   break;

    case WM_CREATE:

         hwndMLE = WinCreateWindow (
            hwnd,
            WC_MLE,
            "",
            MLS_HSCROLL | MLS_VSCROLL | MLS_BORDER,
            0, 0, 0, 0,   /* will set size & position later */
            hwnd,
            HWND_TOP,
            ID_MLE,
            NULL, NULL);
         WinSetFocus (HWND_DESKTOP,hwndMLE);
         MLESetPresparams ();
      break;

    case WM_SIZE:
        WinQueryWindowRect (hwndClientTemp,&rectl);
        WinSetWindowPos (hwndMLE,hwndTemp,
                         rectl.xLeft,rectl.yBottom,
                         rectl.xRight,rectl.yTop,
                         SWP_MOVE | SWP_SIZE | SWP_SHOW);
      break;

    case WM_ERASEBACKGROUND:
        return (MPARAM)FALSE;

    case WM_CLOSE:
           WinShowWindow (hwndTemp,FALSE);
           flTemplateEditorShown = FALSE;
        return (MPARAM)TRUE; /* Fenster NICHT schlie·en */

    default:
        return(WinDefWindowProc(hwnd,msg,mp1,mp2));
  }
  return(FALSE);
}


/***********************************************************************
 * Name      : void TemplateProcess
 * Funktion  : Schaltet die Fenster um.
 * Parameter :
 * Variablen : flTemplateEditorShown
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Samstag, 15.04.1994 00.41.53]
 ***********************************************************************/

void TemplateProcess (void)
{
  /* Prepare all for the editing */
  /* Copy the asciiz text into the editor window */

  if (flTemplateEditorShown == TRUE)
  {
    WinShowWindow (hwndTemp,FALSE);
    flTemplateEditorShown = FALSE;
    //WinSetActiveWindow (HWND_DESKTOP,hwndMain);
  }
  else
  {
    WinShowWindow (hwndTemp,TRUE);
    flTemplateEditorShown = TRUE;
    WinSetActiveWindow (HWND_DESKTOP,hwndTemp);
  }
}


/***********************************************************************
 * Name      : void MLEInsert (char *text)
 * Funktion  : EinfÅgen eines Tokens in den MLE an die aktuelle Position
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 26.11.1993 - 16:10:23]
 ***********************************************************************/

void MLEInsert (char *text)
{
  WinSendMsg (hwndMLE,MLM_INSERT,(MPARAM) text,NULL);
}


/***********************************************************************
 * Name      : void MLEClear (char *text)
 * Funktion  : Leeren des MLEs
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 26.11.1993 - 16:10:23]
 ***********************************************************************/

void MLEClear (void)
{
  IPT textsize;

  textsize = (IPT)WinSendMsg (hwndMLE,  MLM_QUERYTEXTLENGTH,0,0);
  WinSendMsg (hwndMLE,MLM_DELETE,(IPT)0,(MPARAM)textsize);
  MLEReset ();
}


/***********************************************************************
 * Name      : void MLEReset (void)
 * Funktion  : MLE auf UNCHANGED setzen
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 26.11.1993 - 16:10:23]
 ***********************************************************************/

void MLEReset (void)
{
  WinSendMsg (hwndMLE,MLM_SETCHANGED,(MPARAM)FALSE,NULL);
}


/***********************************************************************
 * Name      : void FileRead (char *fname)
 * Funktion  : Lesen einer Kommentarvorlage in den Puffer
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 26.11.1993 - 16:10:23]
 ***********************************************************************/

void FileRead (char *fname)
{
  HFILE       infile;
  ULONG       infilesize;
  ULONG       action;
  FILESTATUS3 ffbuf;
  APIRET      rc;

  char *p;

  if (MLEChanged() == TRUE)
  {
    MLEClear ();

    rc = DosOpen ((char *)fname, &infile, &action, 0L, 0L,
                  OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                  OPEN_FLAGS_WRITE_THROUGH | OPEN_FLAGS_FAIL_ON_ERROR |
                  OPEN_FLAGS_NO_CACHE | OPEN_FLAGS_NO_LOCALITY |
                  OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYNONE |
                  OPEN_ACCESS_READONLY, 0L);
    if (rc)
    {
      error ("DosOpen");
      return;
    }

    switch (action)
    {
      case FILE_EXISTED:
          /* Get Filesize */
          rc = DosQueryFileInfo (infile,FIL_STANDARD,&ffbuf,sizeof(ffbuf));
          if (rc)
          {
            error ("DosQueryFileInfo");
            return;
          }
          infilesize = ffbuf.cbFile;

          /* check if file is zero-sized */
          if (!infilesize)
            goto recreate;

          if (infilesize > 65536)
          {
            error ("File will be cut to 64k");
            infilesize = 65536;
          }

          TTextBuffer = (char *)realloc (TTextBuffer,infilesize+1);
          p = TTextBuffer + infilesize;
          *p = '\0';
          if (!TTextBuffer)
          {
            error ("Out of memory");
            return;
          }

          rc = DosRead (infile,TTextBuffer,infilesize,&action);
          if (rc)
          {
            error ("DosRead");
            return;
          }
          MLEInsert (TTextBuffer);
          MLEReset ();
          /* Kopieren des Vorlagenpuffers */
          action = strlen(TTextBuffer);
          TemplateBuffer = (char *)realloc (TemplateBuffer,action);

          if (!TemplateBuffer)
            error ("TempBuf: out of memory");

          memcpy (TemplateBuffer,TTextBuffer,action);
        break;

      case FILE_CREATED:
recreate:
        error ("File was (re)created.");
        MLEClear ();
        if (TemplateBuffer)
          free (TemplateBuffer);
        TemplateBuffer = NULL;
        break;
    }

    rc = DosClose (infile);
    if (rc)
    {
      error ("DosClose");
      return;
    }

  }
  else return; /* Abbrechen ! */

  TemplateName ((char *)fname);
}


/***********************************************************************
 * Name      : void FileWrite (char *fname)
 * Funktion  : Schreiben einer Kommentarvorlage auf Disk
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 26.11.1993 - 16:10:23]
 ***********************************************************************/

void FileWrite (char *fname)
{
  HFILE  infile;
  ULONG  action;
  ULONG  textlen;
  APIRET rc;

  textlen = MLEGetBuffer ((char **)&TTextBuffer);

  rc = DosOpen ((char *)fname, &infile, &action, textlen, FILE_NORMAL,
                  OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS,
                  OPEN_FLAGS_WRITE_THROUGH | OPEN_FLAGS_FAIL_ON_ERROR |
                  OPEN_FLAGS_NOINHERIT | OPEN_SHARE_DENYWRITE |
                  OPEN_ACCESS_WRITEONLY, 0L);
  if (rc)
  {
    error ("DosOpen");
    return;
  }

  rc = DosWrite (infile,TTextBuffer,textlen,&action);
  if (rc)
  {
    error ("DosWrite");
    return;
  }

  rc = DosClose (infile);
  if (rc)
  {
    error ("DosClose");
    return;
  }

  MLEReset ();
}


/***********************************************************************
 * Name      : void FileNew (void)
 * Funktion  : Erstellen einer neuen Kommentarvorlage
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 26.11.1993 - 16:10:23]
 ***********************************************************************/

void FileNew (void)
{
  /* alte Vorlage Speichern ??? */
  if (MLEChanged() == TRUE)
    MLEClear ();
  else
    return; /* Abbrechen ! */

  TemplateName (NULL);
  WinShowWindow (hwndTemp,TRUE);
  flTemplateEditorShown = TRUE;
  WinSetFocus (HWND_DESKTOP,hwndMLE);
}


/***********************************************************************
 * Name      : BOOL MLEChanged (void)
 * Funktion  : Wurde Vorlage verÑndert ? -> Sichern ?
 * Parameter :
 * Variablen :
 * Ergebnis  : TRUE -> weitermachen, FALSE -> abbrechen
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 26.11.1993 - 16:10:23]
 ***********************************************************************/

BOOL MLEChanged (void)
{
  ULONG res;

  if ((BOOL)WinSendMsg (hwndMLE,MLM_QUERYCHANGED,NULL,NULL) == TRUE)
  {
    res = WinMessageBox (HWND_DESKTOP,hwndTemp,
                      "Die momentane Vorlage wurde verÑndert"
                      " und noch nicht gesichert.\n"
                      "Wollen Sie die bestehende Vorlage sichern ?",
                      "Vorlage",
                      0, /* will be passed to WM_HELP !!! */
                      MB_YESNOCANCEL | MB_HELP | MB_WARNING);
    switch (res)
    {
      case MBID_NO: /* Sichern ? */
        MLEReset ();
   break;

      case MBID_YES:
   /* ... FILESave ... */
   if (flTemplateNamed)
     FileWrite (szTemplateName);
   else
     EditSaveAs (hwndMain);

   MLEReset ();
   break;

      case MBID_CANCEL:
        return (FALSE); /* Weitere Verarbeitung abbrechen */
    }
  }
  return (TRUE); /* OK, weitermachen */
}


/***********************************************************************
 * Name      : BOOL MLEChanged (void)
 * Funktion  : Wurde Vorlage verÑndert ? -> Sichern ?
 * Parameter :
 * Variablen :
 * Ergebnis  : TRUE -> weitermachen, FALSE -> abbrechen
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 26.11.1993 - 16:10:23]
 ***********************************************************************/

void TemplateName (char *name)
{

  if (name == NULL)
  {
    flTemplateNamed = FALSE;
    WinSetWindowText (hwndTemp,"Vorlageneditor");
    if (szTemplateName)
      free (szTemplateName);
  }
  else
  {
    szTemplateName = (char *)realloc (szTemplateName,strlen(name)+1);

    if (szTemplateName)
      strcpy (szTemplateName,name);

    strcpy (szBuf,"Vorlageneditor - ");
    strcat (szBuf,name);
    WinSetWindowText (hwndTemp,szBuf);
    flTemplateNamed = TRUE;
  }
}


/***********************************************************************
 * Name      : void EditRead (HWND hwnd)
 * Funktion  : Filedialog - ôffnen
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 26.11.1993 - 16:10:23]
 ***********************************************************************/

void EditRead (HWND hwnd)
{
  FILEDLG pfDlg;       /* FileDialog structure */
  char pszTitle[]    = "Kommentarvorlage îffnen";
  char pszFullFile[] = "*.KGV";
  HWND hwndFDlg;

  memset (&pfDlg,0,sizeof(pfDlg));
  pfDlg.cbSize   = sizeof(pfDlg);
  pfDlg.fl       = FDS_CENTER | FDS_HELPBUTTON | FDS_OPEN_DIALOG;
  pfDlg.pszTitle = pszTitle;
  strcpy (pfDlg.szFullFile,pszFullFile);

  hwndFDlg = WinFileDlg (HWND_DESKTOP,hwnd,&pfDlg);
  if (hwndFDlg && (pfDlg.lReturn == DID_OK))
    FileRead (pfDlg.szFullFile);
}


/***********************************************************************
 * Name      : void EditSaveAs (HWND hwnd)
 * Funktion  : Filedialog - Speichern unter
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 26.11.1993 - 16:10:23]
 ***********************************************************************/

void EditSaveAs (HWND hwnd)
{
  FILEDLG pfDlg;       /* FileDialog structure */
  char pszTitle[]    = "Kommentarvorlage speichern";
  HWND hwndFDlg;

  memset (&pfDlg,0,sizeof(pfDlg));
  pfDlg.cbSize   = sizeof(pfDlg);
  pfDlg.fl       = FDS_CENTER | FDS_HELPBUTTON | FDS_SAVEAS_DIALOG |
                   FDS_ENABLEFILELB;
  pfDlg.pszTitle = pszTitle;
  if (flTemplateNamed)
    strcpy (pfDlg.szFullFile,szTemplateName);
  else
    strcpy (pfDlg.szFullFile,"*.KGV");

  hwndFDlg = WinFileDlg (HWND_DESKTOP,hwnd,&pfDlg);
  if (hwndFDlg && (pfDlg.lReturn == DID_OK))
    FileWrite (pfDlg.szFullFile);
}


/***********************************************************************
 * Name      : ULONG MLEGetBuffer (char *target)
 * Funktion  : Auslesen des MLE_Puffers
 * Parameter :
 * Variablen :
 * Ergebnis  : LÑnge des Puffers in Bytes.
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 26.11.1993 - 16:10:23]
 ***********************************************************************/

ULONG MLEGetBuffer (char **target)
{
  ULONG  dummy;
  ULONG  copied;
  ULONG  textlen;
  char   *p;

  textlen = LONGFROMMR(WinSendMsg (hwndMLE,MLM_QUERYFORMATTEXTLENGTH,
              (MPARAM)0,MPFROMLONG(-1)));
  *target = (char *)realloc (*target,textlen+1);
  if (!(*target))
    error ("Out of memory");

  /* Mit Nullen auffÅllen */
  memset (*target,0,textlen+1);

  WinSendMsg (hwndMLE,MLM_SETIMPORTEXPORT,*target,&textlen);
  copied = textlen;
  dummy  = (ULONG)0L;

  textlen = (ULONG)WinSendMsg (hwndMLE,MLM_EXPORT,&dummy,&copied);
  *(*target + textlen) = 0;

  return ((ULONG)textlen);
}


/***********************************************************************
 * Name      : void MLEGetPresparams (char **ptarget,ULONG *psize)
 * Funktion  : Ermitteln der aktuelle Presentation Parameters
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [Freitag, 26.11.1993 - 16:10:23]
 ***********************************************************************/

void MLEGetPresparams (void)
{
  WinQueryPresParam (hwndMLE,PP_FOREGROUNDCOLOR,0L,NULL,sizeof(RGB2),
            &MLEPARAMS.fgcolor,
            QPF_NOINHERIT | QPF_PURERGBCOLOR);
  WinQueryPresParam (hwndMLE,PP_BACKGROUNDCOLOR,0L,NULL,sizeof(RGB2),
            &MLEPARAMS.bkcolor,
            QPF_NOINHERIT | QPF_PURERGBCOLOR);
  WinQueryPresParam (hwndMLE,PP_FONTNAMESIZE,0L,NULL,sizeof(MLEPARAMS.fontname),
                      &MLEPARAMS.fontname,
                      QPF_NOINHERIT);
}


/***********************************************************************
 * Name      : void MLESetPresparams
 * Funktion  : Setzen der Presentation Parameter
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung : Fonts funktionieren, Farben (noch) nicht
 *
 * Autor     : Patrick Haller [Samstag, 15.04.1994 00.43.14]
 ***********************************************************************/

void MLESetPresparams (void)
{
  WinSetPresParam (hwndMLE,PP_FOREGROUNDCOLOR,sizeof(RGB2),&MLEPARAMS.fgcolor);
  WinSetPresParam (hwndMLE,PP_BACKGROUNDCOLOR,sizeof(RGB2),&MLEPARAMS.bkcolor);
  WinSetPresParam (hwndMLE,PP_FONTNAMESIZE,strlen(MLEPARAMS.fontname)+1,&MLEPARAMS.fontname);
}
