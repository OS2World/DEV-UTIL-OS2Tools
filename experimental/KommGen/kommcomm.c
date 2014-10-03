/***********************************************************************
 * Name      : Kommentargenerator/2
 * Autor     : Patrick Haller [Freitag,01.04.94 - 16:39:32]
 ***********************************************************************/

#include "KommIncl.h"


/***********************************************************************
 * Name      : void strDate
 * Funktion  : Erzeugen eines Datumsstrings
 * Parameter : char *puf
 * Variablen :
 * Ergebnis  : z.B. "Freitag, 12.12.93, 01:02:03"
 * Bemerkung : Bentzt NLS von OS/2
 *
 * Autor     : Patrick Haller [Samstag, 15.04.1994 00.36.41]
 ***********************************************************************/

void strDate (char *puf)
{
  static char *dowG[] = {"Montag" ,"Dienstag","Mittwoch","Donnerstag",
                        "Freitag","Samstag" ,"Sonntag" ,NULL};
  static char *dowE[] = {"Monday" ,"Tuesday","Wednesday","Thursday",
                        "Friday","Saturday" ,"Sunday" ,NULL};

  COUNTRYCODE Country;
  COUNTRYINFO CtryBuffer;
  ULONG       Length;
  DATETIME    DT;
  APIRET      rc;                            /* Return code */
  char        **DOW;

  char szdate[14];
  char sztime[14];


  Country.country = 0; /* current country */
  Country.codepage = 850;

  rc = DosQueryCtryInfo(sizeof(CtryBuffer),  /* Length of data
                                                area provided */
                         &Country,        /* Input data structure */
                         &CtryBuffer,     /* Data area to be filled
                                               by function */
                         &Length);        /* Length of data
                                               returned */

  if (rc != 0)
  {
    error("DosQueryCtryInfo error");
    return;
  }

  rc = DosGetDateTime (&DT);
  if (rc != 0)
  {
    error("DosGetDateTime error");
    return;
  }

  /* nu den string zusammenbasteln */

  /* Datum */
  switch (CtryBuffer.fsDateFmt)
  {
    case 0: /* mm/dd/yy */
      sprintf (szdate,"%02u%s%02u%s%04u",
          (USHORT)DT.month,
          CtryBuffer.szDateSeparator,
          (USHORT)DT.day,
          CtryBuffer.szDateSeparator,
          (USHORT)DT.year);
      break;
    case 1: /* dd/mm/yy */
      sprintf (szdate,"%02u%s%02u%s%04u",
          (USHORT)DT.day,
          CtryBuffer.szDateSeparator,
          (USHORT)DT.month,
          CtryBuffer.szDateSeparator,
          (USHORT)DT.year);
      break;
    case 2: /* yy/mm/dd */
      sprintf (szdate,"%04u%s%02u%s%02u",
          (USHORT)DT.year,
          CtryBuffer.szDateSeparator,
          (USHORT)DT.month,
          CtryBuffer.szDateSeparator,
          (USHORT)DT.day);
      break;
  }

  /* Zeit */
  switch (CtryBuffer.fsTimeFmt)
  {
    case 0: /* 12-hour ap */
      if (DT.hours > 12)
   sprintf (sztime,"%02u%s%02u%s%02u pm",
         (USHORT)(DT.hours - 12),
         CtryBuffer.szTimeSeparator,
         (USHORT)DT.minutes,
         CtryBuffer.szTimeSeparator,
         (USHORT)DT.seconds);
      else
   sprintf (sztime,"%02u%s%02u%s%02u am",
         (USHORT)DT.hours,
         CtryBuffer.szTimeSeparator,
         (USHORT)DT.minutes,
         CtryBuffer.szTimeSeparator,
         (USHORT)DT.seconds);
      break;

    case 1: /* 24-hour */
   sprintf (sztime,"%02u%s%02u%s%02u",
         (USHORT)DT.hours,
         CtryBuffer.szTimeSeparator,
         (USHORT)DT.minutes,
         CtryBuffer.szTimeSeparator,
         (USHORT)DT.seconds);
      break;
  }

  DOW = dowG; /* german */

  sprintf (puf,"%s, %s %s",DOW[DT.weekday - 1],szdate,sztime);
}


/***********************************************************************
 * Name      : void CopyToClipboard
 * Funktion  : Kopiert String ins Clipboard
 * Parameter : HAB hab, PSZ szBuf
 * Variablen : PVOID clpTextOut, APIRET ulRC
 * Ergebnis  :
 * Bemerkung : Bentzt Shared Memory, evtl. "wackelige" Funktion :(
 *
 * Autor     : Patrick Haller [Samstag, 15.04.1994 00.38.29]
 ***********************************************************************/

void CopyToClipboard (HAB hab,PSZ szBuf)
{
  PVOID  clpTextOut;
  APIRET ulRC;

  if (WinOpenClipbrd (hab))
  {
      ulRC = DosAllocSharedMem ( &clpTextOut, NULL,
             strlen(szBuf) + 1,
             PAG_WRITE | PAG_COMMIT | OBJ_GIVEABLE);
    if (!ulRC)
    {
      strcpy ((char *)clpTextOut,szBuf);
      if (!WinSetClipbrdData (hab,(ULONG)clpTextOut,CF_TEXT,CFI_POINTER))
        error ("Can't put text into clipboard.");
    }
    else error ("Can't alloc shared memory.");
    WinCloseClipbrd (hab);
  }
  else error ("Can't use clipboard.");
}


/***********************************************************************
 * Name      : void strReplace
 * Funktion  : Search & Replace, bufferresizing
 * Parameter : char **buffer, ULONG *bufsize, char *replace, char *by
 * Variablen :
 * Ergebnis  : neuer String
 * Bemerkung : soll(te) automatisch den Puffer vergr”áern, wenn notwendig.
 *
 * Autor     : Patrick Haller [Samstag, 15.04.1994 00.39.42]
 ***********************************************************************/

void strReplace (char **buffer,ULONG *bufsize,char *replace, char *by)
{
  ULONG slen_rep;
  ULONG slen_by;
  ULONG slen_buf;
  int   delta;

  char *p;
  char *pt;

  p = *buffer;
  slen_rep = strlen(replace);
  slen_by  = strlen(by);
  slen_buf = strlen(*buffer);

  while ( (p = strstr (*buffer,replace)) != NULL )
  {
    delta = slen_by - slen_rep;
    pt = p + delta;

    if (slen_buf + delta >= *bufsize)
    {
      *bufsize+=1024; /* increase buffer by 1kB */
      *buffer = (char *)realloc(*buffer,*bufsize);
      if (!(*buffer))
      {
        *buffer = NULL; /* error */
        return;
      }
    }

    slen_buf += delta;

    /* replacement */

    if (!delta);
    else
      if (delta < 0)
        memmove (p,p-delta,strlen (p - delta) + 1);
      else
        memmove (pt,p, strlen (p) + 1);
    memcpy (p,by,slen_by);
  }
}
