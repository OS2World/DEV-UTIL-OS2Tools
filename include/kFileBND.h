/* $Id: kFileBND.h,v 1.13 2002/02/07 09:57:55 bird Exp $
 *
 * kFileBND - DB2 Bind Files.
 *
 * Copyright (c) 2001 knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 *
 */


#ifndef _kFileBND_h_
#define _kFileBND_h_



/**
 * DB2 Bind Files.
 * @author      knut st. osmundsen
 */
class kFileBND : public kFileFormatBase
{
public:
    /**@cat Functions which opens a bnd as the correct object type. */
    static kFileBND *Open(const char *pszFilename, KBOOL fReadOnly = TRUE);
    static kFileBND *Open(kFile *pFile);


    /**@cat Constructor/Destructor */
    kFileBND(kFile *pFile) throw(kError);
    virtual ~kFileBND();
    int  Commit(void);


    /**@cat queries... */
    virtual unsigned long   queryRelNo(void) const = 0;
    virtual char *          queryApplication(char *pszBuffer) const = 0;
    virtual char *          queryTimeStamp(char *pszBuffer) const = 0;
    virtual char *          queryCreator(char *pszBuffer) const = 0;
    virtual char *          queryPrepId(char *pszBuffer) const = 0;
    virtual char *          queryColid(char *pszBuffer) const = 0;
    virtual char *          queryOwner(char *pszBuffer) const = 0;
    virtual char *          queryQual(char *pszBuffer) const = 0;
    virtual char *          queryVrsn(char *pszBuffer) const = 0;
    virtual unsigned long   queryIsol(void) const = 0;
    virtual unsigned long   queryBlck(void) const = 0;
    virtual unsigned long   queryDate(void) const = 0;
    virtual unsigned long   queryNumHostvars(void) const = 0;
    virtual unsigned long   queryMaxSect(void) const = 0;
    virtual unsigned long   queryNumStmt(void) const = 0;
    virtual unsigned long   queryStatements(void) const = 0;
    virtual unsigned long   queryDeclarel(void) const = 0;
    virtual unsigned long   queryDeclare(void) const = 0;
    const char *            queryClassNm() const {return pszClassNm;}

    /**@cat set... */
    virtual int             setApplication(const char *pszApplication) = 0;
    virtual int             setTimeStamp(const char *pszTimeStamp) = 0;
    virtual int             setCreator(const char *pszCreator) = 0;
    virtual int             setPrepId(const char *pszPrepId) = 0;
    virtual int             setColid(const char *pszColid) = 0;
    virtual int             setOwner(const char *pszOwner) = 0;
    virtual int             setQual(const char *pszQual) = 0;
    virtual int             setVrsn(const char *pszVrsn) = 0;
    virtual int             setIsol(unsigned long Isol) = 0;
    virtual int             setBlck(unsigned long Blck) = 0;
    virtual int             setDate(unsigned long Date) = 0;

    /**@cat usful static helpers */
    static char *           queryIsoShortName(unsigned long lIsolation, char *pszShortName);
    static char *           queryDateShortName(unsigned long lDateFmt, char *pszShortName);

    /**@cat constants */
    enum {notset = -1};                                 /* Generic */
    enum {iso_rr = 0, iso_cs = 1, iso_ur = 2,           /* ISOLATION */
          iso_rs = 3, iso_nc = 4};
    enum {blck_unambig = 0, blck_all = 1, blck_no = 2}; /* BLOCKING */
    enum {date_default = 0x30, date_usa = 0x31,         /* DATETIME */
          date_eur = 0x32, date_iso = 0x33,
          date_jis = 0x34, date_local = 0x35};

protected:
    int                     setField(const char *pszNew, char *pszField, int cchField);
    int                     setField2(const char *pszNew, char *pszField, int cchField, int cchSpaces = 0);
    int                     setField(const char *pszNew, PBINDOPTSTR pOpt, int cchValue, int cchSpaces = 0, KBOOL fDefaultOnBlank = TRUE, KBOOL fOneSpaceOnBlank =FALSE);
    int                     setField1Blank(const char *pszNew, PBINDOPTSTR pOpt, int cchValue, int cchSpace = 0);
    int                     setField(unsigned long ulValue, PBINDOPTUL pOpt);

    /**@cat Variables for commiting the header contents. */
    void *          pvHdr;
    int             cbHdr;
    KBOOL           fDirty;
    const char *    pszClassNm;
};



class kFileLX;
/**
 * DB2 Bind Program Ids.
 * @author      knut st. osmundsen
 */
class kFileBNDPrgId
{
public:
    /** Constructor. Initiates fDirty. */
    kFileBNDPrgId() : fDirty(FALSE) { }
    virtual ~kFileBNDPrgId()        { }

    /**@cat Compare operators for bind files. */
    KBOOL operator ==(const kFileBND &BndFile) const;
    KBOOL operator !=(const kFileBND &BndFile) const;

    /**@cat queries... */
    virtual char *          queryPlanname(char *pszBuffer) const = 0;
    virtual char *          queryContoken(char *pszBuffer) const = 0;
    virtual char *          querySqlUser(char *pszBuffer) const = 0;
    KBOOL                   isDirty() const {return fDirty;}
    virtual int             querySize() const = 0;
    const char *            queryClassNm() const {return pszClassNm;}

    /**@cat set... */
    virtual int             setPlanname(char *pszPlanname) = 0;
    virtual int             setContoken(char *pszContoken) = 0;
    virtual int             setSqlUser(char *pszSqlUser) = 0;

    /**@cat Executable scanner. */
    typedef int            (callback)(kFileBNDPrgId *pPrgId, unsigned ulAddress, unsigned long ulUser);
    static  int             scanExecutable(kFileLX *pFileLX, KBOOL fDontWrite, callback pfnCallback, unsigned long ulUser); /* TODO: kFileLX should be changed to kPageI+kSegmentI. */

protected:
    int                     setField(const char *pszNew, char *pszField, int cchField);

    KBOOL                   fDirty;
    const char *            pszClassNm;

};


class kFileBNDV10 : public kFileBND
{
public:
    /**@cat Constructor/Destructor */
    kFileBNDV10(kFile *pFile) throw(kError);

    /**@cat queries... */
    unsigned long   queryRelNo(void) const;
    char *          queryApplication(char *pszBuffer) const;
    char *          queryTimeStamp(char *pszBuffer) const;
    char *          queryCreator(char *pszBuffer) const;
    char *          queryPrepId(char *pszBuffer) const;
    char *          queryColid(char *pszBuffer) const;
    char *          queryOwner(char *pszBuffer) const;
    char *          queryQual(char *pszBuffer) const;
    char *          queryVrsn(char *pszBuffer) const;
    unsigned long   queryIsol(void) const;
    unsigned long   queryBlck(void) const;
    unsigned long   queryDate(void) const;
    unsigned long   queryNumHostvars(void) const;
    unsigned long   queryMaxSect(void) const;
    unsigned long   queryNumStmt(void) const;
    unsigned long   queryStatements(void) const;
    unsigned long   queryDeclarel(void) const;
    unsigned long   queryDeclare(void) const;

    /**@cat set... */
    int             setApplication(const char *pszApplication);
    int             setTimeStamp(const char *pszTimeStamp);
    int             setCreator(const char *pszCreator);
    int             setPrepId(const char *pszPrepId);
    int             setColid(const char *pszColid);
    int             setOwner(const char *pszOwner);
    int             setQual(const char *pszQual);
    int             setVrsn(const char *pszVrsn);
    int             setIsol(unsigned long Isol);
    int             setBlck(unsigned long Blck);
    int             setDate(unsigned long Date);

private:
    BINDHDRV10  hdr;

};



class kFileBNDV20 : public kFileBND
{
public:
    /**@cat Constructor/Destructor */
    kFileBNDV20(kFile *pFile) throw(kError);

    /**@cat queries... */
    unsigned long   queryRelNo(void) const;
    char *          queryApplication(char *pszBuffer) const;
    char *          queryTimeStamp(char *pszBuffer) const;
    char *          queryCreator(char *pszBuffer) const;
    char *          queryPrepId(char *pszBuffer) const;
    char *          queryColid(char *pszBuffer) const;
    char *          queryOwner(char *pszBuffer) const;
    char *          queryQual(char *pszBuffer) const;
    char *          queryVrsn(char *pszBuffer) const;
    unsigned long   queryIsol(void) const;
    unsigned long   queryBlck(void) const;
    unsigned long   queryDate(void) const;
    unsigned long   queryNumHostvars(void) const;
    unsigned long   queryMaxSect(void) const;
    unsigned long   queryNumStmt(void) const;
    unsigned long   queryStatements(void) const;
    unsigned long   queryDeclarel(void) const;
    unsigned long   queryDeclare(void) const;

    /**@cat set... */
    int             setApplication(const char *pszApplication);
    int             setTimeStamp(const char *pszTimeStamp);
    int             setCreator(const char *pszCreator);
    int             setPrepId(const char *pszPrepId);
    int             setColid(const char *pszColid);
    int             setOwner(const char *pszOwner);
    int             setQual(const char *pszQual);
    int             setVrsn(const char *pszVrsn);
    int             setIsol(unsigned long Isol);
    int             setBlck(unsigned long Blck);
    int             setDate(unsigned long Date);

private:
    BINDHDRV20  hdr;

};


class kFileBNDV52 : public kFileBND
{
public:
    /**@cat Constructor/Destructor */
    kFileBNDV52(kFile *pFile) throw(kError);

    /**@cat queries... */
    unsigned long   queryRelNo(void) const;
    char *          queryApplication(char *pszBuffer) const;
    char *          queryTimeStamp(char *pszBuffer) const;
    char *          queryCreator(char *pszBuffer) const;
    char *          queryPrepId(char *pszBuffer) const;
    char *          queryColid(char *pszBuffer) const;
    char *          queryOwner(char *pszBuffer) const;
    char *          queryQual(char *pszBuffer) const;
    char *          queryVrsn(char *pszBuffer) const;
    unsigned long   queryIsol(void) const;
    unsigned long   queryBlck(void) const;
    unsigned long   queryDate(void) const;
    unsigned long   queryNumHostvars(void) const;
    unsigned long   queryMaxSect(void) const;
    unsigned long   queryNumStmt(void) const;
    unsigned long   queryStatements(void) const;
    unsigned long   queryDeclarel(void) const;
    unsigned long   queryDeclare(void) const;

    /**@cat set... */
    int             setApplication(const char *pszApplication);
    int             setTimeStamp(const char *pszTimeStamp);
    int             setCreator(const char *pszCreator);
    int             setPrepId(const char *pszPrepId);
    int             setColid(const char *pszColid);
    int             setOwner(const char *pszOwner);
    int             setQual(const char *pszQual);
    int             setVrsn(const char *pszVrsn);
    int             setIsol(unsigned long Isol);
    int             setBlck(unsigned long Blck);
    int             setDate(unsigned long Date);

private:
    BINDHDRV52  hdr;

};


class kFileBNDV61 : public kFileBND
{
public:
    /**@cat Constructor/Destructor */
    kFileBNDV61(kFile *pFile) throw(kError);

    /**@cat queries... */
    unsigned long   queryRelNo(void) const;
    char *          queryApplication(char *pszBuffer) const;
    char *          queryTimeStamp(char *pszBuffer) const;
    char *          queryCreator(char *pszBuffer) const;
    char *          queryPrepId(char *pszBuffer) const;
    char *          queryColid(char *pszBuffer) const;
    char *          queryOwner(char *pszBuffer) const;
    char *          queryQual(char *pszBuffer) const;
    char *          queryVrsn(char *pszBuffer) const;
    unsigned long   queryIsol(void) const;
    unsigned long   queryBlck(void) const;
    unsigned long   queryDate(void) const;
    unsigned long   queryNumHostvars(void) const;
    unsigned long   queryMaxSect(void) const;
    unsigned long   queryNumStmt(void) const;
    unsigned long   queryStatements(void) const;
    unsigned long   queryDeclarel(void) const;
    unsigned long   queryDeclare(void) const;

    /**@cat set... */
    int             setApplication(const char *pszApplication);
    int             setTimeStamp(const char *pszTimeStamp);
    int             setCreator(const char *pszCreator);
    int             setPrepId(const char *pszPrepId);
    int             setColid(const char *pszColid);
    int             setOwner(const char *pszOwner);
    int             setQual(const char *pszQual);
    int             setVrsn(const char *pszVrsn);
    int             setIsol(unsigned long Isol);
    int             setBlck(unsigned long Blck);
    int             setDate(unsigned long Date);

private:
    BINDHDRV61  hdr;

};



class kFileBNDV71 : public kFileBND
{
public:
    /**@cat Constructor/Destructor */
    kFileBNDV71(kFile *pFile) throw(kError);

    /**@cat queries... */
    unsigned long   queryRelNo(void) const;
    char *          queryApplication(char *pszBuffer) const;
    char *          queryTimeStamp(char *pszBuffer) const;
    char *          queryCreator(char *pszBuffer) const;
    char *          queryPrepId(char *pszBuffer) const;
    char *          queryColid(char *pszBuffer) const;
    char *          queryOwner(char *pszBuffer) const;
    char *          queryQual(char *pszBuffer) const;
    char *          queryVrsn(char *pszBuffer) const;
    unsigned long   queryIsol(void) const;
    unsigned long   queryBlck(void) const;
    unsigned long   queryDate(void) const;
    unsigned long   queryNumHostvars(void) const;
    unsigned long   queryMaxSect(void) const;
    unsigned long   queryNumStmt(void) const;
    unsigned long   queryStatements(void) const;
    unsigned long   queryDeclarel(void) const;
    unsigned long   queryDeclare(void) const;

    /**@cat set... */
    int             setApplication(const char *pszApplication);
    int             setTimeStamp(const char *pszTimeStamp);
    int             setCreator(const char *pszCreator);
    int             setPrepId(const char *pszPrepId);
    int             setColid(const char *pszColid);
    int             setOwner(const char *pszOwner);
    int             setQual(const char *pszQual);
    int             setVrsn(const char *pszVrsn);
    int             setIsol(unsigned long Isol);
    int             setBlck(unsigned long Blck);
    int             setDate(unsigned long Date);

private:
    BINDHDRV71  hdr;

};




class kFileBNDPrgIdPre70 : public kFileBNDPrgId
{
public:
    kFileBNDPrgIdPre70(void *pv);

    /**@cat set... */
    char *          queryPlanname(char *pszBuffer) const;
    char *          queryContoken(char *pszBuffer) const;
    char *          querySqlUser(char *pszBuffer) const;
    int             querySize() const { return sizeof(*pProgId); }

    /**@cat queries... */
    int             setPlanname(char *pszPlanname);
    int             setContoken(char *pszContoken);
    int             setSqlUser(char *pszSqlUser);

private:
    PBINDPROGID pProgId;

};


class kFileBNDPrgIdV70 : public kFileBNDPrgId
{
public:
    kFileBNDPrgIdV70(void *pv);

    /**@cat set... */
    char *          queryPlanname(char *pszBuffer) const;
    char *          queryContoken(char *pszBuffer) const;
    char *          querySqlUser(char *pszBuffer) const;
    int             querySize() const { return sizeof(*pProgId); }

    /**@cat queries... */
    int             setPlanname(char *pszPlanname);
    int             setContoken(char *pszContoken);
    int             setSqlUser(char *pszSqlUser);

private:
    PBINDPROGIDV70 pProgId;
};


class kFileBNDPrgIdV71 : public kFileBNDPrgId
{
public:
    kFileBNDPrgIdV71(void *pv);

    /**@cat set... */
    char *          queryPlanname(char *pszBuffer) const;
    char *          queryContoken(char *pszBuffer) const;
    char *          querySqlUser(char *pszBuffer) const;
    int             querySize() const { return (int)(&((PBINDPROGIDV71)0)->sqluser) + pProgId->sqluser_len; }

    /**@cat queries... */
    int             setPlanname(char *pszPlanname);
    int             setContoken(char *pszContoken);
    int             setSqlUser(char *pszSqlUser);

private:
    PBINDPROGIDV71 pProgId;
};


class kFileBNDPrgIdV71MF : public kFileBNDPrgId
{
public:
    kFileBNDPrgIdV71MF(void *pv, kFileLX *pFileLX, int iObj, int offObj);
    ~kFileBNDPrgIdV71MF(void);

    /**@cat set... */
    char *          queryPlanname(char *pszBuffer) const;
    char *          queryContoken(char *pszBuffer) const;
    char *          querySqlUser(char *pszBuffer) const;
    int             querySize() const;

    /**@cat queries... */
    int             setPlanname(char *pszPlanname);
    int             setContoken(char *pszContoken);
    int             setSqlUser(char *pszSqlUser);

private:
    /** @cat data */
    PBINDPROGIDV71MF    pProgIdMF;
    PBINDPROGIDV71      pProgId71;      /* enmEncoding2: Pointer to prog id. Only some parts are valid. */
    PBINDPROGIDV71MFCODE pProgIdCode;   /* enmEncoding2: Pointer to the code to patch. */
    unsigned short *    pussqluser_len; /* Pointer to the sqluser_len in the code */
    char *              pachsqluser1;   /* Pointer to the sqluser part1 (4bytes) in the code */
    char *              pachsqluser2;   /* Pointer to the sqluser part2 (4bytes) in the code */
    kFileLX *           pExec;          /* Pointer to executable image. Fixme: kExecutableI. */
    enum
    {   enmEncoding1,                   /* just funny structure (pProgIdMF) */
        enmEncoding2                    /* need to patch code (pProgId71 + pExec) */
    }                   enmType;
    char *              pachPages;      /* Pointer to one or two pages. */
    unsigned long       cbPages;        /* Size of the pages. */
    unsigned long       ulSegment;      /* The segment containing the pachPages. */
    unsigned long       offSegment;     /* The offset into that segment of pachPages. */
    KBOOL               fDirtyPages;    /* Flags if the pages are drity or not. */
};


#endif

