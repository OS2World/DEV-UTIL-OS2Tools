/* Pointer means offset from beginning of file or beginning of struct */
#pragma pack(1)
typedef struct {
  unsigned short int ppNextMap;     /* paragraph pointer to next map         */
  unsigned char      bFlags;        /* symbol types                          */
  unsigned char      bReserved1;    /* reserved                              */
  unsigned short int pSegEntry;     /* segment entry point value             */
  unsigned short int cConsts;       /* count of constants in map             */
  unsigned short int pConstDef;     /* pointer to constant chain             */
  unsigned short int cSegs;         /* count of segments in map              */
  unsigned short int ppSegDef;      /* paragraph pointer to first segment    */
  unsigned char      cbMaxSym;      /* maximum symbol-name length            */
  unsigned char      cbModName;     /* length of module name                 */
  char               achModName[1]; /* cbModName Bytes of module-name member */
} MAPDEF;

typedef struct {
  unsigned short int ppNextMap;     /* always zero                           */
  unsigned char      release;       /* release number (minor version number) */
  unsigned char      version;       /* major version number                  */
} LAST_MAPDEF;

typedef struct {
  unsigned short int ppNextSeg;     /* paragraph pointer to next segment     */
  unsigned short int cSymbols;      /* count of symbols in list              */
  unsigned short int pSymDef;       /* offset of symbol chain                */
  unsigned short int wReserved1;    /* reserved                              */
  unsigned short int wReserved2;    /* reserved                              */
  unsigned short int wReserved3;    /* reserved                              */
  unsigned short int wReserved4;    /* reserved                              */
  unsigned char      bFlags;        /* symbol types                          */
  unsigned char      bReserved1;    /* reserved                              */
  unsigned short int ppLineDef;     /* offset of line number record          */
  unsigned char      bReserved2;    /* reserved                              */
  unsigned char      bReserved3;    /* reserved                              */
  unsigned char      cbSegName;     /* length of segment name                */
  char               achSegName[1]; /* cbSegName Bytes of segment-name member*/
} SEGDEF;

typedef struct {
  unsigned short int wSymVal;       /* symbol address or constant            */
  unsigned char      cbSymName;     /* length of symbol name                 */
  char               achSymName[1]; /* cbSymName Bytes of symbol-name member */
} SYMDEF16;

typedef struct {
  unsigned       int wSymVal;       /* symbol address or constant            */
  unsigned char      cbSymName;     /* length of symbol name                 */
  char               achSymName[1]; /* cbSymName Bytes of symbol-name member */
} SYMDEF32;

typedef struct {
  unsigned short int ppNextLine;    /* ptr to next linedef (0 if last)       */
  unsigned short int wReserved1;    /* reserved                              */
  unsigned short int pLines;        /* pointer to line numbers               */
  unsigned short int cLines;        /* reserved                              */
  unsigned char      cbFileName;    /* length of filename                    */
  char               achFileName[1];/* cbFileName Bytes of filename          */
} LINEDEF;

typedef struct {
  unsigned short int wCodeOffset;   /* executable offset                     */
  unsigned short int dwFileOffset;  /* source offset                         */
} LINEINF;

#define SEGDEFOFFSET(MapDef)     (MapDef.ppSegDef*16)
#define NEXTSEGDEFOFFSET(SegDef)  (SegDef.ppNextSeg*16)

#define ASYMPTROFFSET(SegDefOffset,Segdef) (SegDefOffset+SegDef.pSymDef)
#define SYMDEFOFFSET(SegDefOffset,SegDef,n) (ASYMPTROFFSET(SegDefOffset,SegDef)+(n)*(sizeof(unsigned short int)))

#define ACONSTPTROFFSET(MapDef) (MapDef.ppConstDef)
#define CONSTDEFOFFSET(MapDef,n) ((MapDef.ppConstDef)+(n)*(sizeof(unsigned short int)))

#define LINEDEFOFFSET(SegDef) (SegDef.ppLineDef*16))
#define NEXTLINEDEFOFFSET(LineDef) (LineDef.ppNextLine*16)
#define LINESOFFSET(LinedefOffset,LineDef) ((LinedefOffset)+LineDef.pLines)

#pragma pack()
