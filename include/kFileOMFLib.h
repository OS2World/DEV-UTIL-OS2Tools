/*
 * kFileOMFLib - OMF Library files.
 *
 * Copyright (c) 2001 knut st. osmundsen
 *
 */
#ifndef _kFileOMFLib_h_
#define _kFileOMFLib_h_


class kFileOMFLib_Member : public kListEntry
{
public:
    ~kFileOMFLib_Member();

public:
    char *          pszName;
    void *          pvFile;
    unsigned long   cbFile;
    kFileOMF *      pOBJ;
};


class kFileOMFLib : public kFileFormatBase /*, public kFileLibraryI, public kFileLibraryWritableI */
{
public:
    kFileOMFLib(kFile *pFile) throw (kError);
    virtual ~kFileOMFLib();

    KBOOL dump(kFile *pOut);

private:
    void parse(void) throw (kError);
    void parseNew(void) throw (kError);


private:
    void *          pvFile;
    unsigned long   cbFile;

    class kList<kFileOMFLib_Member> Members;
};


#endif
