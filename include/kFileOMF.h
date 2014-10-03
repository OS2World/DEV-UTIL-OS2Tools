/*
 * kFileDef - Definition files.
 *
 * Copyright (c) 2001 knut st. osmundsen
 *
 */
#ifndef _kFileOMF_h_
#define _kFileOMF_h_



class kFileOMF : public kFileFormatBase /*, public kFileObjectI, public kFileObjectWritableI */
{
public:
    kFileOMF(kFile *pFile) throw (kError);
    kFileOMF(void *pvFile, unsigned long cbFile) throw (kError);
    virtual ~kFileOMF();

    KBOOL dump(kFile *pOut);

private:
    void parse(void) throw (kError);


private:
    void *          pvFile;
    unsigned long   cbFile;

};












#endif
