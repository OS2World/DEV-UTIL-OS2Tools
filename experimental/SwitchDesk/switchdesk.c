/***********************************************************************
 * Name      : Switchdesk
 * Funktion  : Umschaltung verschiedener Desktops
 * Bemerkung : PrfReset
 *
 * Autor     : Patrick Haller [Montag, 19.09.1994 15.25.02]
 ***********************************************************************/

#define INCL_WINSHELLDATA
#define INCL_WIN
#define INCL_WINDIALOGS
#define INCL_WINSHELLDATA
#include <OS2.H>

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define ININAME "SwDesk.Cfg"

#define ID_DLG_MAIN 1
#define ID_FIRST_USER 100

typedef struct {
  void* next;
  char* Username;
  char* Ininame;
  char* Sysininame;
  } CUser;

CUser U_System = {NULL,"System","D:\\OS2\\OS2.INI","D:\\OS2\\OS2SYS.INI"};
CUser *Userlist = &U_System;
HAB hab;
HWND hwndFrame;

MRESULT EXPENTRY SwitchdeskDlgProc (HWND,ULONG,MPARAM,MPARAM);
void AddButtons (HWND hwnd);
void ActivateUser (HWND hwnd, USHORT usUser);
void SwitchUser (HWND hwnd,CUser *User);

int main()
{
  HMQ hmq;
  int rc;

  InitRead ();

  hab = WinInitialize (0);
  if (hab)
  {
    hmq = WinCreateMsgQueue (hab,0);

    hwndFrame = WinLoadDlg(HWND_DESKTOP,HWND_DESKTOP,
			 SwitchdeskDlgProc,0,ID_DLG_MAIN,NULL);

    /* process the dialog */
    WinProcessDlg( hwndFrame );

    WinDestroyMsgQueue (hmq);
    WinTerminate       (hab);
  }

  DosExit            (EXIT_PROCESS,0);
  return 0;
}

MRESULT EXPENTRY SwitchdeskDlgProc (HWND hwnd, ULONG msgid, MPARAM mp1, MPARAM mp2)
{
  switch (msgid)
  {
    case WM_COMMAND:
      switch (SHORT1FROMMP(mp1))
      {
	default:
	  ActivateUser (hwnd,SHORT1FROMMP(mp1));
      }
      return FALSE;
      break;

    case WM_INITDLG:
      AddButtons (hwnd);
      break;

    case WM_MINMAXFRAME :
	{
	  /* FALSE: Dialog wird minimiert; TRUE: Dialog wird restored */
	  BOOL  fShow = ! (((PSWP) mp1)->fl & SWP_MINIMIZE);
	  HENUM henum; /* handle fr das Aufz„hlen aller Controls */
	  HWND  hwndChild;

	  WinEnableWindowUpdate ( hwnd, FALSE );

	  for (henum=WinBeginEnumWindows(hwnd);
	       (hwndChild = WinGetNextWindow (henum)) != 0; )
	  WinShowWindow ( hwndChild, fShow );

	  WinEndEnumWindows ( henum );
	  WinEnableWindowUpdate ( hwnd, TRUE );
	}
	break;
  }
  return (WinDefDlgProc(hwnd,msgid,mp1,mp2));
}

void AddButtons (HWND hwnd)
{
  CUser *p;
  ULONG y = 1;
  ULONG counter = ID_FIRST_USER; /* erste ID */
  ULONG height = 30;
  ULONG captionheight = 21;

  p=Userlist;
  while (p)
  {
   /* create button window */
   WinCreateWindow(hwnd,  /* parent window          */
		   WC_BUTTON,    /* class name             */
		   p->Username,    /* window text            */
		   WS_VISIBLE,    /* window style           */
		   1, y,          /* position (x,y)         */
		   158, height,      /* size (width,height)    */
		   NULLHANDLE,    /* owner window           */
		   HWND_TOP,      /* sibling window         */
		   counter,     /* window id              */
		   NULL,          /* control data           */
		   NULL);         /* presentation parms     */

    p=p->next;
    counter++;
    y+=height-2;
  }
  WinSetWindowPos(hwnd,HWND_TOP,0,0,160,captionheight+y,SWP_ACTIVATE|SWP_SIZE|SWP_MOVE|SWP_SHOW);
}

void SwitchUser (HWND hwnd, CUser *User)
{
  char szUser[CCHMAXPATH];
  char szSys[CCHMAXPATH];
  PRFPROFILE profile;

  profile.cchUserName = CCHMAXPATH;
  profile.pszUserName = szUser;
  profile.cchSysName  = CCHMAXPATH;
  profile.pszSysName  = szSys;

  if (PrfQueryProfile(hab, &profile) == FALSE) /* get the system profile name */
    WinMessageBox (HWND_DESKTOP,HWND_DESKTOP,"Profilequery failed.","Shit ...",0,MB_OK);
  else
  {
    profile.pszUserName = User->Ininame;
    profile.cchUserName = strlen(User->Ininame)+1;
    profile.pszSysName = User->Sysininame;
    profile.cchSysName = strlen(User->Sysininame)+1;

    if (PrfReset (hab, &profile) == TRUE)
      WinSetWindowText (hwnd,User->Username);
    else
      WinMessageBox (HWND_DESKTOP,HWND_DESKTOP,"Failed.","Profile",0,MB_OK);
  }
}

void trim (char* inistr)
{
  char *p;

  if (!p) return;

  /* process the string */
  p = strchr (inistr,';'); /* Remark ??? */
  if (p) *p = 0; /* truncate remark... */

  /* trim the string */
  if (inistr[0])
  {
    p = inistr + strlen(inistr) - 1;
    while ((*p == ' ') || (*p == '\n')) *p-- = 0;

    p = inistr;
    while (*p == ' ') p++;
    if (p != inistr) memcpy (inistr,p,strlen(p)+1);
  }
}


CUser* AddUser (char* Username, char* Ininame, char* Sysininame)
{
  CUser *p;

  trim (Ininame);
  trim (Sysininame);

  p = malloc (sizeof(CUser));
  if (p)
  {
    p->next = Userlist;

    p->Username = malloc (strlen(Username)+1);
    strcpy (p->Username,Username);
    p->Ininame = malloc (strlen(Ininame)+1);
    strcpy (p->Ininame,Ininame);
    p->Sysininame = malloc (strlen(Sysininame)+1);
    strcpy (p->Sysininame,Sysininame);    Userlist = p;
  }
  return p;
}

int InitRead (void)
{
  FILE *inifile;
#define BUFFERSIZE 256
  char buffer[BUFFERSIZE];
  char *p1, *p2, *p3;

  inifile = fopen (ININAME,"r");
  if (!inifile) return 1;

  while (!feof(inifile))
  {
    fgets (buffer,BUFFERSIZE,inifile);

    /* Username, INIname */
    p1 = strtok (buffer,",");
    if (p1)
    {
       p2 = strtok (NULL,",");
       if (p2)
       {
	 p3 = strtok (NULL,",");
	 if (p3)
	   AddUser (p1,p2,p3);
       }
    }
  }

  fclose (inifile);
  return (0);
}

void ActivateUser (HWND hwnd, USHORT usUser)
{
  USHORT us = ID_FIRST_USER;
  CUser *p = Userlist;

  while (p)
  {
    if (us == usUser)
    {
      SwitchUser (hwnd,p);
      break;
    }
    us++;
    p=p->next;
  }
  if (!p) WinAlarm(hwnd,WA_ERROR);
}
