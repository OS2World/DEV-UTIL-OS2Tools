/***********************************************************************
 * Name      : Headerdatei CHKPATH.H
 * Funktion  : Definitionen fÅr CHKPATH.C
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung : Braucht PSZ-Definition !
 *
 * Autor     : Patrick Haller [Freitag, 12.01.1996 09.54.09]
 ***********************************************************************/

#ifndef MODULE_CHKPATH
#define MODULE_CHKPATH


typedef enum {SF_NONE,                            /* Kein Status angegeben */
              SF_FILE,                                       /* EINE Datei */
              SF_DIR,                                          /* EIN Pfad */
              SF_FILES,                                 /* Mehrere Dateien */
              SF_DIRS} STATEMENTFLAG;                     /* Mehrere Pfade */

typedef enum
{
    SFFN_NONE,
    SFFN_LIBPATH
} STATEMENTFUNCTION;

typedef struct
{
   STATEMENTFLAG sFlag;                                    /* Special flag */
   BOOL          bMustBeSet;             /* This token must be specified ! */
   PSZ           pszValue;            /* Value of the environment variable */
   PSZ           pszToken;                        /* The environment token */
   PSZ           pszExtensions;               /* Any specific extensions ? */
   PSZ           pszDescription;             /* What does this statement ? */
   STATEMENTFUNCTION sffn;           /* call special function to determine */
} STATEMENT, *PSTATEMENT;

typedef struct
{
   PSZ pszPath;                            /* Complete Path to this module */
   PSZ pszName;                                     /* Name of this module */
   PSZ pszDescription;                        /* Description of the module */
                                     /* INF/HLP-Header, EXE32-Header, etc. */
   PSZ pszLastWriteDate;      /* Date of the last write/creation of module */
} MODULE, *PMODULE;

typedef struct
{
  PSZ         pszName;                                       /* Dateiname */
  PSZ         pszFullName;                          /* Dateiname mit Pfad */
  PVOID       pLeft;                             /* Linker Ast des Baumes */
  PVOID       pRight;                           /* Rechter Ast des Baumes */
  PVOID       pSame;                                /* Gleiche Dateinamen */

  ULONG       cbFile;                                       /* Dateigrî·e */
  FDATE       fdateLastWrite;                 /* Datum letzten Schreibens */
  FTIME       ftimeLastWrite;                  /* Zeit letzten Schreibens */
} PATHLIST, *PPATHLIST;

#endif /* MODULE_CHKPATH */

