/* $Id: kFileBND.cpp,v 1.22 2002/03/14 15:28:19 bird Exp $
 *
 * kFileBND - DB2 Bind Files.
 *            Sorry, but this code isn't as tidy as I'd like it to be.
 *
 * Copyright (c) 2001-2002 knut st. osmundsen (bird@anduin.net)
 *
 * GPL
 */




/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include <string.h>
#include <stdio.h>

#include "LXexe.h"
#include "Bind.h"

#include "kTypes.h"
#include "kError.h"
#include "kFile.h"
#include "kFileInterfaces.h"
#include "kFileFormatBase.h"
#include "kFileLX.h"
#include "kFileBND.h"


/*******************************************************************************
*   Internal Functions                                                         *
*******************************************************************************/
inline char *trimR(char *psz);


/**
 * Right trims a string, ie. removing spaces (and tabs) from the end of the stri
 * @returns   Pointer to the string passed in.
 * @param     psz   Pointer to the string which is to be right trimmed.
 * @status    completely implmented.
 * @author    knut st. osmundsen (knut.stange.osmundsen@pmsc.no)
 */
inline char *trimR(char *psz)
{
    int i;
    if (psz == NULL)
        return NULL;
    i = strlen(psz) - 1;
    while (i >= 0 && (psz[i] == ' ' || *psz == '\t'))
        i--;
    psz[i+1] = '\0';
    return psz;
}


kFileBND *kFileBND::Open(const char *pszFilename, KBOOL fReadOnly/* = TRUE*/)
{
    kFile *     pFile = NULL;
    kFileBND *  pBnd = NULL;
    try
    {
        pFile = new kFile(pszFilename, fReadOnly);
        pBnd = Open(pFile);
    }
    catch (kError err)
    {
        throw (err);
    }
    return pBnd;
}


kFileBND *kFileBND::Open(kFile *pFile)
{
    kFileBND *pBND = NULL;
    char    achBindId[BINDID_LENGTH];

    pFile->setThrowOnErrors();
    pFile->readAt(&achBindId[0], sizeof(achBindId), 0);
    if (!memcmp(BINDID_V10, achBindId, BINDID_LENGTH))
        pBND = new kFileBNDV10(pFile);
    else if (   !memcmp(BINDID_V20, achBindId, BINDID_LENGTH))
        pBND = new kFileBNDV20(pFile);
    else if (   !memcmp(BINDID_V50, achBindId, BINDID_LENGTH)
             || !memcmp(BINDID_V51, achBindId, BINDID_LENGTH)
             || !memcmp(BINDID_V52, achBindId, BINDID_LENGTH))
        pBND = new kFileBNDV52(pFile);
    else if (   !memcmp(BINDID_V60, achBindId, BINDID_LENGTH)
             || !memcmp(BINDID_V61, achBindId, BINDID_LENGTH))
        pBND = new kFileBNDV61(pFile);
    else if (   !memcmp(BINDID_V70, achBindId, BINDID_LENGTH)
             || !memcmp(BINDID_V71, achBindId, BINDID_LENGTH))
        pBND = new kFileBNDV71(pFile);
    else
    {
        delete pFile;
        throw(kError(kError::INVALID_SIGNATURE));
    }
    return pBND;
}


/**
 * Constructor.
 * @param   pFile   Pointer to file object.
 * @status  completely implemented.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
kFileBND::kFileBND(kFile *pFile) throw(kError) :
    kFileFormatBase(pFile), pvHdr(NULL), fDirty(FALSE)

{
    pszClassNm = "kFileBND";
}


/**
 * Destructor.
 * @status  completely implemented.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
kFileBND::~kFileBND()
{
}


/**
 * Commits the changes done to the header to the file.
 * @returns 0 on success. kError error number.
 * @status  completely implemented.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
int  kFileBND::Commit(void)
{
    if (!fDirty)
        return 0;
    pFile->setFailOnErrors();
    return pFile->writeAt(pvHdr, cbHdr, 0);
}


/**
 * Internal worker which updates a field.
 * Doing all the necessary work to maintain fDirty.
 * This is for field which are padded with spaces.
 * @return  0 on success. kError on failure.
 * @parma   pszNew      New value.
 * @parma   pszField    Pointer to field.
 * @parma   cchField    Size of the field.
 * @remark  might set fDirty.
 */
int kFileBND::setField(const char *pszNew, char *pszField, int cchField)
{
    return setField2(pszNew, pszField, cchField, -1);
}


/**
 * Internal worker which updates a field.
 * Doing all the necessary work to maintain fDirty.
 * This is for field which are padded with '\0'.
 * @return  0 on success. kError on failure.
 * @param   pszNew      New value.
 * @param   pszField    Pointer to field.
 * @param   cchField    Size of the field.
 * @param   cchSpaces   How many positions to fill with spaces. Default is 0. (meaning all is zeroed)
 *                      -1 here means that the entire string should be padded with spaces.
 *                      This is required while some versions of DB2 expects fields like creator and collid
 *                      to be 8 chars padded with spaces.
 * @remark  might set fDirty.
 */
int kFileBND::setField2(const char *pszNew, char *pszField, int cchField, int cchSpaces/* = 0*/)
{
    int     i;
    int     cchNew = strlen(pszNew);

    if (cchNew > cchField)
        return kError::STRING_TO_LONG;
    if (cchSpaces == -1)
        cchSpaces = cchField;

    if (!memcmp(pszField, pszNew, cchNew))
    {   /* value matched. check padding. */
        char *  psz = pszField + cchNew;
        while (cchNew < cchField && cchNew < cchSpaces && *psz == ' ')
            cchNew++, psz++;
        if (cchNew >= cchSpaces)
        {
            while (cchNew < cchField && *psz == '\0')
                cchNew++, psz++;
        }

        /* if match do nothing. */
        if (cchNew == cchField)
            return 0;
    }
    else
    {
        /* no match.. update the field and do required padding. */
        memcpy(pszField, pszNew, cchNew);
    }

    /* do the appropriate paddings. */
    if (cchNew < cchSpaces && cchNew < cchField)
    {
        cchSpaces = KMIN(cchSpaces, cchField) - cchNew;
        memset(pszField + cchNew, ' ', cchSpaces);
        cchNew += cchSpaces;
    }
    kASSERT(cchNew <= cchField);
    if (cchNew < cchField)
        memset(pszField + cchNew, '\0', cchField - cchNew);
    fDirty = TRUE;
    return 0;
}



/**
 * Internal worker which updates a field.
 * Doing all the necessary work to maintain fDirty.
 * This is for field which are padded with '\0'.
 * @return  0 on success. kError on failure.
 * @param   pszNew          New value.
 * @param   pOpt            Pointer to an string option struct.
 * @param   cchValue        Max value size.
 * @oaram   fDefaultOnBlank Set if we should use 'default' on empty string.
 *                          Clear if we should use 'defined' on empty string.
 * @remark  might set fDirty.
 */
int kFileBND::setField(const char *pszNew, PBINDOPTSTR pOpt, int cchValue, int cchSpaces/*=0*/, KBOOL fDefaultOnBlank /*=TRUE*/, KBOOL fOneSpaceOnBlank /*=FALSE*/)
{
    int cchNew = strlen(pszNew);
    if (cchNew > cchValue)
        return kError::STRING_TO_LONG;

    if (cchSpaces == -1)
        cchSpaces = cchValue;
    if (cchNew == 0)
        cchSpaces = fOneSpaceOnBlank ? 1 : 0;

    if (    pOpt->length == cchNew
        &&  pOpt->_default == (cchNew == 0 && fDefaultOnBlank ? 1 : 0)
        &&  !memcmp(&pOpt->value[0], pszNew, cchNew))
    {
        /* value matched. check padding. */
        char *  psz = &pOpt->value[cchNew];
        while (cchNew < cchValue && cchNew < cchSpaces && *psz == ' ')
            cchNew++, psz++;
        if (cchNew >= cchSpaces)
        {
            while (cchNew < cchValue && *psz == '\0')
                cchNew++, psz++;
        }

        /* if match do nothing. */
        if (cchNew == cchValue)
            return 0;
    }
    else
    {
        if (cchNew)
        {   /* value */
            pOpt->length = cchNew;
            pOpt->_default = 0;
            memcpy(&pOpt->value[0], pszNew, cchNew);
        }
        else
        {   /* no value */
            pOpt->length = fOneSpaceOnBlank ? 1 : 0;
            pOpt->_default = fDefaultOnBlank ? 1 : 0;
        }
    }

    /* the the appropriate paddings */
    if (cchNew < cchSpaces && cchNew < cchValue)
    {
        cchSpaces = KMIN(cchSpaces, cchValue) - cchNew;
        memset(&pOpt->value[cchNew], ' ', cchSpaces);
        cchNew += cchSpaces;
    }
    kASSERT(cchNew <= cchValue);
    if (cchNew < cchValue)
        memset(&pOpt->value[cchNew], '\0', cchValue - cchNew);
    fDirty = TRUE;
    return 0;
}


/**
 * Internal worker which updates a field.
 * Doing all the necessary work to maintain fDirty.
 * This is for field which are padded with '\0'.
 *
 * It differs from SetField in that when empty field a value of ' ' is insterted
 * while _default is set to 'default' (=1).
 *
 * @return  0 on success. kError on failure.
 * @param   pszNew      New value.
 * @param   pOpt        Pointer to an string option struct.
 * @param   cchValue    Max value size.
 * @param   cchSpaces   How many positions to fill with spaces. Default is 0. (meaning all is zeroed)
 *                      -1 here means that the entire string should be padded with spaces.
 *                      This is required while some versions of DB2 expects fields like creator and collid
 *                      to be 8 chars padded with spaces.
 * @remark  might set fDirty.
 */
int kFileBND::setField1Blank(const char *pszNew, PBINDOPTSTR pOpt, int cchValue, int cchSpaces/*=0*/)
{
    return setField(pszNew, pOpt, cchValue, cchSpaces, TRUE, TRUE);
}


/**
 * Sets an unsigned long option value.
 * @return  0 on success.
 * @param   ulValue     New value.
 * @param   pOpt        Pointer to the unsigned long option struct to change.
 * @remark  might set fDirty.
 */
int  kFileBND::setField(unsigned long ulValue, PBINDOPTUL pOpt)
{
    if (ulValue == kFileBND::notset)
    {
        if (pOpt->_default == 1 && pOpt->value == 0)
            return 0;
        pOpt->_default = 1;
        pOpt->value = 0;
    }
    else
    {
        if (pOpt->_default == 0 && pOpt->value == ulValue)
            return 0;
        pOpt->_default = 0;
        pOpt->value = ulValue;
    }
    fDirty = TRUE;
    return 0;
}



/**
 * Gets the short name for a given isolation level.
 * @returns pszShortName.
 * @param   lIsolation      The isolation level.
 * @param   pszShortName    Buffer. (>= 8 bytes)
 * @author  knut st. osmundsen (kosmunds@csc.com)
 */
char * kFileBND::queryIsoShortName(unsigned long lIsolation, char *pszShortName)
{
    switch (lIsolation)
    {
        case kFileBND::iso_rr:  strcpy(pszShortName, "RR"); break;
        case kFileBND::iso_cs:  strcpy(pszShortName, "CS"); break;
        case kFileBND::iso_ur:  strcpy(pszShortName, "UR"); break;
        case kFileBND::iso_rs:  strcpy(pszShortName, "RS"); break;
        case kFileBND::iso_nc:  strcpy(pszShortName, "NC"); break;
        case kFileBND::notset:  strcpy(pszShortName, "<Undef>"); break;
        default:                strcpy(pszShortName, "<Unknw>"); break;
    }

    return pszShortName;
}



/**
 * Gets the short name for a given isolation level.
 * @returns pszShortName.
 * @param   lIsolation      The date format.
 * @param   pszShortName    Buffer. (>= 8 bytes)
 * @author  knut st. osmundsen (kosmunds@csc.com)
 */
char * kFileBND::queryDateShortName(unsigned long lDateFmt, char *pszShortName)
{
    switch (lDateFmt)
    {
        case kFileBND::date_default:strcpy(pszShortName, "DEF"); break;
        case kFileBND::date_usa:    strcpy(pszShortName, "USA"); break;
        case kFileBND::date_eur:    strcpy(pszShortName, "EUR"); break;
        case kFileBND::date_iso:    strcpy(pszShortName, "ISO"); break;
        case kFileBND::date_jis:    strcpy(pszShortName, "JIS"); break;
        case kFileBND::date_local:  strcpy(pszShortName, "LOC"); break;
        default:                    strcpy(pszShortName, "<Unknw>"); break;
    }

    return pszShortName;
}












/**
 * Bind files for DB2 version 1.0 and perhaps 1.1.
 * @param   pFile   Pointer to file object. This is not deleted if
 *                  the constructor failes. Elsewise it's deleted
 *                  when the constructed object is destructed.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
kFileBNDV10::kFileBNDV10(kFile *pFile) :
    kFileBND(pFile)
{
    pszClassNm = "kFileBNDV10";
    pvHdr = &hdr;
    cbHdr = sizeof(hdr);

    pFile->setThrowOnErrors();
    pFile->readAt(&hdr, sizeof(hdr), 0);
    if (memcmp(hdr.bind_id, BINDID_V10, sizeof(hdr.bind_id)))
        throw(kError(kError::INVALID_SIGNATURE));

    if (    hdr.relno != BINDREL_V1
        &&  hdr.relno != BINDREL_V12)
        throw(kError(kError::INCORRECT_VERSION));
}

unsigned long   kFileBNDV10::queryRelNo(void) const
{   return hdr.relno; }

char *          kFileBNDV10::queryApplication(char *pszBuffer) const
{
    memcpy(pszBuffer, hdr.application, sizeof(hdr.application));
    pszBuffer[sizeof(hdr.application)] = '\0';
    return trimR(pszBuffer);
}

char *          kFileBNDV10::queryTimeStamp(char *pszBuffer) const
{
    memcpy(pszBuffer, hdr.timestamp, sizeof(hdr.timestamp));
    pszBuffer[sizeof(hdr.timestamp)] = '\0';
    return pszBuffer;
}

char *          kFileBNDV10::queryCreator(char *pszBuffer) const
{
    memcpy(pszBuffer, hdr.creator, sizeof(hdr.creator));
    pszBuffer[sizeof(hdr.creator)] = '\0';
    return trimR(pszBuffer);
}

char *          kFileBNDV10::queryPrepId(char *pszBuffer) const
{   pszBuffer = pszBuffer; return NULL; }

char *          kFileBNDV10::queryColid(char *pszBuffer) const
{   pszBuffer = pszBuffer; return NULL; }

char *          kFileBNDV10::queryOwner(char *pszBuffer) const
{   pszBuffer = pszBuffer; return NULL; }

char *          kFileBNDV10::queryQual(char *pszBuffer) const
{   pszBuffer = pszBuffer; return NULL; }

char *          kFileBNDV10::queryVrsn(char *pszBuffer) const
{   pszBuffer = pszBuffer; return NULL; }

unsigned long   kFileBNDV10::queryIsol(void) const
{   return kFileBND::notset; }

unsigned long   kFileBNDV10::queryBlck(void) const
{   return kFileBND::notset; }

unsigned long   kFileBNDV10::queryDate(void) const
{   return kFileBND::notset; }

unsigned long   kFileBNDV10::queryNumHostvars(void) const
{   return kFileBND::notset; }

unsigned long   kFileBNDV10::queryMaxSect(void) const
{   return kFileBND::notset; }

unsigned long   kFileBNDV10::queryNumStmt(void) const
{   return kFileBND::notset; }

unsigned long   kFileBNDV10::queryStatements(void) const
{   return hdr.statements; }

unsigned long   kFileBNDV10::queryDeclarel(void) const
{   return hdr.declarel; }

unsigned long   kFileBNDV10::queryDeclare(void) const
{   return hdr.declare; }


int kFileBNDV10::setApplication(const char *pszApplication)
{   return setField(pszApplication, &hdr.application[0], sizeof(hdr.application)); }

int kFileBNDV10::setTimeStamp(const char *pszTimeStamp)
{   return setField(pszTimeStamp, &hdr.timestamp[0], sizeof(hdr.timestamp)); }

int kFileBNDV10::setCreator(const char *pszCreator)
{   return setField(pszCreator, &hdr.creator[0], sizeof(hdr.creator)); }

int kFileBNDV10::setPrepId(const char *pszPrepId)
{   pszPrepId = pszPrepId;  return kError::NOT_SUPPORTED; }

int kFileBNDV10::setColid(const char *pszColid)
{   pszColid = pszColid;    return kError::NOT_SUPPORTED; }

int kFileBNDV10::setOwner(const char *pszOwner)
{   pszOwner = pszOwner;    return kError::NOT_SUPPORTED; }

int kFileBNDV10::setQual(const char *pszQual)
{   pszQual = pszQual;      return kError::NOT_SUPPORTED; }

int kFileBNDV10::setVrsn(const char *pszVrsn)
{   pszVrsn = pszVrsn;      return kError::NOT_SUPPORTED; }

int kFileBNDV10::setIsol(unsigned long Isol)
{   Isol = Isol;            return kError::NOT_SUPPORTED; }

int kFileBNDV10::setBlck(unsigned long Blck)
{   Blck = Blck;            return kError::NOT_SUPPORTED; }

int kFileBNDV10::setDate(unsigned long Date)
{   Date = Date;            return kError::NOT_SUPPORTED; }






/**
 * Bind files for DB2 version 1.2.
 * @param   pFile   Pointer to file object. This is not deleted if
 *                  the constructor failes. Elsewise it's deleted
 *                  when the constructed object is destructed.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
kFileBNDV20::kFileBNDV20(kFile *pFile) :
    kFileBND(pFile)
{
    pszClassNm = "kFileBNDV20";
    pvHdr = &hdr;
    cbHdr = sizeof(hdr);

    pFile->setThrowOnErrors();
    pFile->readAt(&hdr, sizeof(hdr), 0);
    if (memcmp(hdr.bind_id, BINDID_V20, sizeof(hdr.bind_id)))
        throw(kError(kError::INVALID_SIGNATURE));

    if (hdr.endian != ENDIAN_LITTLE)
        throw (kError(kError::UNSUPPORTED_ENDIAN));

    if (hdr.relno != BINDREL_V2)
        throw(kError(kError::INCORRECT_VERSION));
}

unsigned long   kFileBNDV20::queryRelNo(void) const
{   return hdr.relno; }

char *          kFileBNDV20::queryApplication(char *pszBuffer) const
{
    memcpy(pszBuffer, hdr.application, sizeof(hdr.application));
    pszBuffer[sizeof(hdr.application)] = '\0';
    return trimR(pszBuffer);
}

char *          kFileBNDV20::queryTimeStamp(char *pszBuffer) const
{
    memcpy(pszBuffer, hdr.timestamp, sizeof(hdr.timestamp));
    pszBuffer[sizeof(hdr.timestamp)] = '\0';
    return pszBuffer;
}

char *          kFileBNDV20::queryCreator(char *pszBuffer) const
{
    memcpy(pszBuffer, hdr.creator, sizeof(hdr.creator));
    pszBuffer[sizeof(hdr.creator)] = '\0';
    return trimR(pszBuffer);
}

char *          kFileBNDV20::queryPrepId(char *pszBuffer) const
{
    memcpy(pszBuffer, hdr.prep_id, sizeof(hdr.prep_id));
    pszBuffer[sizeof(hdr.prep_id)] = '\0';
    return trimR(pszBuffer);
}

char *          kFileBNDV20::queryColid(char *pszBuffer) const
{
    *pszBuffer = '\0';
    if (hdr.colid_default || hdr.colid_length == 0 || hdr.colid_value[0] == ' ')
        return NULL;
    return strncat(pszBuffer, hdr.colid_value, hdr.colid_length);
}

char *          kFileBNDV20::queryOwner(char *pszBuffer) const
{
    *pszBuffer = '\0';
    if (hdr.owner_default || hdr.owner_length == 0 || hdr.owner_value[0] == ' ')
        return NULL;
    return strncat(pszBuffer, hdr.owner_value, hdr.owner_length);
}

char *          kFileBNDV20::queryQual(char *pszBuffer) const
{
    *pszBuffer = '\0';
    if (hdr.qual_default || hdr.qual_length == 0 || hdr.qual_value[0] == ' ')
        return NULL;
    return strncat(pszBuffer, hdr.qual_value, hdr.qual_length);
}

char *          kFileBNDV20::queryVrsn(char *pszBuffer) const
{
    *pszBuffer = '\0';
    if (hdr.vrsn_default || hdr.vrsn_length == 0 || hdr.vrsn_value[0] == ' ')
        return NULL;
    return strncat(pszBuffer, hdr.vrsn_value, hdr.vrsn_length);
}

unsigned long   kFileBNDV20::queryIsol(void) const
{   return hdr.isol_default ? kFileBND::notset : hdr.isol_value; }

unsigned long   kFileBNDV20::queryBlck(void) const
{   return hdr.blck_default ? kFileBND::notset : hdr.blck_value; }

unsigned long   kFileBNDV20::queryDate(void) const
{   return hdr.date_default ? kFileBND::notset : hdr.date_value; }

unsigned long   kFileBNDV20::queryNumHostvars(void) const
{   return hdr.num_hostvars; }

unsigned long   kFileBNDV20::queryMaxSect(void) const
{   return hdr.max_sect; }

unsigned long   kFileBNDV20::queryNumStmt(void) const
{   return hdr.num_stmt; }

unsigned long   kFileBNDV20::queryStatements(void) const
{   return hdr.statements; }

unsigned long   kFileBNDV20::queryDeclarel(void) const
{   return hdr.declarel; }

unsigned long   kFileBNDV20::queryDeclare(void) const
{   return hdr.declare; }


int kFileBNDV20::setApplication(const char *pszApplication)
{   return setField(pszApplication, &hdr.application[0], sizeof(hdr.application)); }

int kFileBNDV20::setTimeStamp(const char *pszTimeStamp)
{   return setField(pszTimeStamp, &hdr.timestamp[0], sizeof(hdr.timestamp)); }

int kFileBNDV20::setCreator(const char *pszCreator)
{   return setField(pszCreator, &hdr.creator[0], sizeof(hdr.creator)); }

int kFileBNDV20::setPrepId(const char *pszPrepId)
{   return setField(pszPrepId, &hdr.prep_id[0], sizeof(hdr.prep_id)); }

int kFileBNDV20::setColid(const char *pszColid)
{   return setField(pszColid, (PBINDOPTSTR)&hdr.colid_reserved, sizeof(hdr.colid_value), 8, FALSE); }

int kFileBNDV20::setOwner(const char *pszOwner)
{   return setField1Blank(pszOwner, (PBINDOPTSTR)&hdr.owner_reserved, sizeof(hdr.owner_value)); }

int kFileBNDV20::setQual(const char *pszQual)
{   return setField(pszQual, (PBINDOPTSTR)&hdr.qual_reserved, sizeof(hdr.qual_value)); }

int kFileBNDV20::setVrsn(const char *pszVrsn)
{   return setField(pszVrsn, (PBINDOPTSTR)&hdr.vrsn_reserved, sizeof(hdr.vrsn_value)); }

int kFileBNDV20::setIsol(unsigned long Isol)
{
    if ((Isol >= kFileBND::iso_rr && Isol <= kFileBND::iso_nc) || Isol == kFileBND::notset)
        return setField(Isol, (PBINDOPTUL)&hdr.isol_reserved);
    return kError::INVALID_PARAMETER;
}

int kFileBNDV20::setBlck(unsigned long Blck)
{
    if ((Blck >= kFileBND::blck_unambig && Blck <= kFileBND::blck_no) || Blck == kFileBND::notset)
        return setField(Blck, (PBINDOPTUL)&hdr.blck_reserved);
    return kError::INVALID_PARAMETER;
}

int kFileBNDV20::setDate(unsigned long Date)
{
    if ((Date >= kFileBND::date_default && Date <= kFileBND::date_local)/* || Blck == kFileBND::notset*/)
        return setField(Date, (PBINDOPTUL)&hdr.date_reverved);
    return kError::INVALID_PARAMETER;
}





/**
 * Bind files for DB2 version 5.2.
 * @param   pFile   Pointer to file object. This is not deleted if
 *                  the constructor failes. Elsewise it's deleted
 *                  when the constructed object is destructed.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
kFileBNDV52::kFileBNDV52(kFile *pFile) :
    kFileBND(pFile)
{
    pszClassNm = "kFileBNDV52";
    pvHdr = &hdr;
    cbHdr = sizeof(hdr);

    pFile->setThrowOnErrors();
    pFile->readAt(&hdr, sizeof(hdr), 0);
    if (    memcmp(hdr.bind_id, BINDID_V50, sizeof(hdr.bind_id))
        &&  memcmp(hdr.bind_id, BINDID_V51, sizeof(hdr.bind_id))
        &&  memcmp(hdr.bind_id, BINDID_V52, sizeof(hdr.bind_id)))
        throw(kError(kError::INVALID_SIGNATURE));

    if (hdr.endian != ENDIAN_LITTLE)
        throw (kError(kError::UNSUPPORTED_ENDIAN));

    if (hdr.relno != BINDREL_V5)
        throw(kError(kError::INCORRECT_VERSION));
}

unsigned long   kFileBNDV52::queryRelNo(void) const
{   return hdr.relno; }

char *          kFileBNDV52::queryApplication(char *pszBuffer) const
{
    memcpy(pszBuffer, hdr.application, sizeof(hdr.application));
    pszBuffer[sizeof(hdr.application)] = '\0';
    return trimR(pszBuffer);
}

char *          kFileBNDV52::queryTimeStamp(char *pszBuffer)  const
{
    memcpy(pszBuffer, hdr.timestamp, sizeof(hdr.timestamp));
    pszBuffer[sizeof(hdr.timestamp)] = '\0';
    return pszBuffer;
}

char *          kFileBNDV52::queryCreator(char *pszBuffer) const
{
    memcpy(pszBuffer, hdr.creator, sizeof(hdr.creator));
    pszBuffer[sizeof(hdr.creator)] = '\0';
    return trimR(pszBuffer);
}

char *          kFileBNDV52::queryPrepId(char *pszBuffer) const
{
    memcpy(pszBuffer, hdr.prep_id, sizeof(hdr.prep_id));
    pszBuffer[sizeof(hdr.prep_id)] = '\0';
    return trimR(pszBuffer);
}

char *          kFileBNDV52::queryColid(char *pszBuffer) const
{
    *pszBuffer = '\0';
    if (hdr.colid_default || hdr.colid_length == 0 || hdr.colid_value[0] == ' ')
        return NULL;
    return strncat(pszBuffer, hdr.colid_value, hdr.colid_length);
}

char *          kFileBNDV52::queryOwner(char *pszBuffer) const
{
    *pszBuffer = '\0';
    if (hdr.owner_default || hdr.owner_length == 0 || hdr.owner_value[0] == ' ')
        return NULL;
    return strncat(pszBuffer, hdr.owner_value, hdr.owner_length);
}

char *          kFileBNDV52::queryQual(char *pszBuffer) const
{
    *pszBuffer = '\0';
    if (hdr.qual_default || hdr.qual_length == 0 || hdr.qual_value[0] == ' ')
        return NULL;
    return strncat(pszBuffer, hdr.qual_value, hdr.qual_length);
}

char *          kFileBNDV52::queryVrsn(char *pszBuffer) const
{
    *pszBuffer = '\0';
    if (hdr.vrsn_default || hdr.vrsn_length == 0 || hdr.vrsn_value[0] == ' ')
        return NULL;
    return strncat(pszBuffer, hdr.vrsn_value, hdr.vrsn_length);
}

unsigned long   kFileBNDV52::queryIsol(void) const
{   return hdr.isol_default ? kFileBND::notset : hdr.isol_value; }

unsigned long   kFileBNDV52::queryBlck(void) const
{   return hdr.blck_default ? kFileBND::notset : hdr.blck_value; }

unsigned long   kFileBNDV52::queryDate(void) const
{   return hdr.date_default ? kFileBND::notset : hdr.date_value; }

unsigned long   kFileBNDV52::queryNumHostvars(void) const
{   return hdr.num_hostvars; }

unsigned long   kFileBNDV52::queryMaxSect(void) const
{   return hdr.max_sect; }

unsigned long   kFileBNDV52::queryNumStmt(void) const
{   return hdr.num_stmt; }

unsigned long   kFileBNDV52::queryStatements(void) const
{   return hdr.statements; }

unsigned long   kFileBNDV52::queryDeclarel(void) const
{   return hdr.declarel; }

unsigned long   kFileBNDV52::queryDeclare(void) const
{   return hdr.declare; }


int kFileBNDV52::setApplication(const char *pszApplication)
{   return setField(pszApplication, &hdr.application[0], sizeof(hdr.application)); }

int kFileBNDV52::setTimeStamp(const char *pszTimeStamp)
{   return setField(pszTimeStamp, &hdr.timestamp[0], sizeof(hdr.timestamp)); }

int kFileBNDV52::setCreator(const char *pszCreator)
{   return setField(pszCreator, &hdr.creator[0], sizeof(hdr.creator)); }

int kFileBNDV52::setPrepId(const char *pszPrepId)
{   return setField(pszPrepId, &hdr.prep_id[0], sizeof(hdr.prep_id)); }

int kFileBNDV52::setColid(const char *pszColid)
{   return setField(pszColid, (PBINDOPTSTR)&hdr.colid_reserved, sizeof(hdr.colid_value), 8, FALSE); }

int kFileBNDV52::setOwner(const char *pszOwner)
{   return setField1Blank(pszOwner, (PBINDOPTSTR)&hdr.owner_reserved, sizeof(hdr.owner_value)); }

int kFileBNDV52::setQual(const char *pszQual)
{   return setField(pszQual, (PBINDOPTSTR)&hdr.qual_reserved, sizeof(hdr.qual_value)); }

int kFileBNDV52::setVrsn(const char *pszVrsn)
{   return setField(pszVrsn, (PBINDOPTSTR)&hdr.vrsn_reserved, sizeof(hdr.vrsn_value)); }

int kFileBNDV52::setIsol(unsigned long Isol)
{
    if ((Isol >= kFileBND::iso_rr && Isol <= kFileBND::iso_nc) || Isol == kFileBND::notset)
        return setField(Isol, (PBINDOPTUL)&hdr.isol_reserved);
    return kError::INVALID_PARAMETER;
}

int kFileBNDV52::setBlck(unsigned long Blck)
{
    if ((Blck >= kFileBND::blck_unambig && Blck <= kFileBND::blck_no) || Blck == kFileBND::notset)
        return setField(Blck, (PBINDOPTUL)&hdr.blck_reserved);
    return kError::INVALID_PARAMETER;
}

int kFileBNDV52::setDate(unsigned long Date)
{
    if ((Date >= kFileBND::date_default && Date <= kFileBND::date_local)/* || Blck == kFileBND::notset*/)
        return setField(Date, (PBINDOPTUL)&hdr.date_reverved);
    return kError::INVALID_PARAMETER;
}






/**
 * Bind files for DB2 version 6.1.
 * @param   pFile   Pointer to file object. This is not deleted if
 *                  the constructor failes. Elsewise it's deleted
 *                  when the constructed object is destructed.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
kFileBNDV61::kFileBNDV61(kFile *pFile) :
    kFileBND(pFile)
{
    pszClassNm = "kFileBNDV61";
    pvHdr = &hdr;
    cbHdr = sizeof(hdr);

    pFile->setThrowOnErrors();
    pFile->readAt(&hdr, sizeof(hdr), 0);
    if (    memcmp(hdr.bind_id, BINDID_V60, sizeof(hdr.bind_id))
        &&  memcmp(hdr.bind_id, BINDID_V61, sizeof(hdr.bind_id)))
        throw(kError(kError::INVALID_SIGNATURE));

    if (hdr.endian != ENDIAN_LITTLE)
        throw (kError(kError::UNSUPPORTED_ENDIAN));

    if (hdr.relno != BINDREL_V6)
        throw(kError(kError::INCORRECT_VERSION));
}

unsigned long   kFileBNDV61::queryRelNo(void) const
{   return hdr.relno; }

char *          kFileBNDV61::queryApplication(char *pszBuffer) const
{
    memcpy(pszBuffer, hdr.application, sizeof(hdr.application));
    pszBuffer[sizeof(hdr.application)] = '\0';
    return trimR(pszBuffer);
}

char *          kFileBNDV61::queryTimeStamp(char *pszBuffer) const
{
    memcpy(pszBuffer, hdr.timestamp, sizeof(hdr.timestamp));
    pszBuffer[sizeof(hdr.timestamp)] = '\0';
    return trimR(pszBuffer);
}

char *          kFileBNDV61::queryCreator(char *pszBuffer) const
{
    memcpy(pszBuffer, hdr.creator, sizeof(hdr.creator));
    pszBuffer[sizeof(hdr.creator)] = '\0';
    return trimR(pszBuffer);
}

char *          kFileBNDV61::queryPrepId(char *pszBuffer) const
{
    memcpy(pszBuffer, hdr.prep_id, sizeof(hdr.prep_id));
    pszBuffer[sizeof(hdr.prep_id)] = '\0';
    return trimR(pszBuffer);
}

char *          kFileBNDV61::queryColid(char *pszBuffer) const
{
    *pszBuffer = '\0';
    if (hdr.colid_default || hdr.colid_length == 0 || hdr.colid_value[0] == ' ')
        return NULL;
    return strncat(pszBuffer, hdr.colid_value, hdr.colid_length);
}

char *          kFileBNDV61::queryOwner(char *pszBuffer) const
{
    *pszBuffer = '\0';
    if (hdr.owner_default || hdr.owner_length == 0 || hdr.owner_value[0] == ' ')
        return NULL;
    return strncat(pszBuffer, hdr.owner_value, hdr.owner_length);
}

char *          kFileBNDV61::queryQual(char *pszBuffer) const
{
    *pszBuffer = '\0';
    if (hdr.qual_default || hdr.qual_length == 0 || hdr.qual_value[0] == ' ')
        return NULL;
    return strncat(pszBuffer, hdr.qual_value, hdr.qual_length);
}

char *          kFileBNDV61::queryVrsn(char *pszBuffer) const
{
    *pszBuffer = '\0';
    if (hdr.vrsn_default || hdr.vrsn_length == 0 || hdr.vrsn_value[0] == ' ')
        return NULL;
    return strncat(pszBuffer, hdr.vrsn_value, hdr.vrsn_length);
}

unsigned long   kFileBNDV61::queryIsol(void) const
{   return hdr.isol_default ? kFileBND::notset : hdr.isol_value; }

unsigned long   kFileBNDV61::queryBlck(void) const
{   return hdr.blck_default ? kFileBND::notset : hdr.blck_value; }

unsigned long   kFileBNDV61::queryDate(void) const
{   return hdr.date_default ? kFileBND::notset : hdr.date_value; }

unsigned long   kFileBNDV61::queryNumHostvars(void) const
{   return hdr.num_hostvars; }

unsigned long   kFileBNDV61::queryMaxSect(void) const
{   return hdr.max_sect; }

unsigned long   kFileBNDV61::queryNumStmt(void) const
{   return hdr.num_stmt; }

unsigned long   kFileBNDV61::queryStatements(void) const
{   return hdr.statements; }

unsigned long   kFileBNDV61::queryDeclarel(void) const
{   return hdr.declarel; }

unsigned long   kFileBNDV61::queryDeclare(void) const
{   return hdr.declare; }


int kFileBNDV61::setApplication(const char *pszApplication)
{   return setField(pszApplication, &hdr.application[0], sizeof(hdr.application)); }

int kFileBNDV61::setTimeStamp(const char *pszTimeStamp)
{   return setField(pszTimeStamp, &hdr.timestamp[0], sizeof(hdr.timestamp)); }

int kFileBNDV61::setCreator(const char *pszCreator)
{   return setField(pszCreator, &hdr.creator[0], sizeof(hdr.creator)); }

int kFileBNDV61::setPrepId(const char *pszPrepId)
{   return setField(pszPrepId, &hdr.prep_id[0], sizeof(hdr.prep_id)); }

int kFileBNDV61::setColid(const char *pszColid)
{   return setField(pszColid, (PBINDOPTSTR)&hdr.colid_reserved, sizeof(hdr.colid_value), 8, FALSE); }

int kFileBNDV61::setOwner(const char *pszOwner)
{   return setField1Blank(pszOwner, (PBINDOPTSTR)&hdr.owner_reserved, sizeof(hdr.owner_value)); }

int kFileBNDV61::setQual(const char *pszQual)
{   return setField(pszQual, (PBINDOPTSTR)&hdr.qual_reserved, sizeof(hdr.qual_value)); }

int kFileBNDV61::setVrsn(const char *pszVrsn)
{   return setField(pszVrsn, (PBINDOPTSTR)&hdr.vrsn_reserved, sizeof(hdr.vrsn_value)); }

int kFileBNDV61::setIsol(unsigned long Isol)
{
    if ((Isol >= kFileBND::iso_rr && Isol <= kFileBND::iso_nc) || Isol == kFileBND::notset)
        return setField(Isol, (PBINDOPTUL)&hdr.isol_reserved);
    return kError::INVALID_PARAMETER;
}

int kFileBNDV61::setBlck(unsigned long Blck)
{
    if ((Blck >= kFileBND::blck_unambig && Blck <= kFileBND::blck_no) || Blck == kFileBND::notset)
        return setField(Blck, (PBINDOPTUL)&hdr.blck_reserved);
    return kError::INVALID_PARAMETER;
}

int kFileBNDV61::setDate(unsigned long Date)
{
    if ((Date >= kFileBND::date_default && Date <= kFileBND::date_local)/* || Blck == kFileBND::notset*/)
        return setField(Date, (PBINDOPTUL)&hdr.date_reverved);
    return kError::INVALID_PARAMETER;
}








/**
 * Bind files for DB2 version 7.1.
 * @param   pFile   Pointer to file object. This is not deleted if
 *                  the constructor failes. Elsewise it's deleted
 *                  when the constructed object is destructed.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
kFileBNDV71::kFileBNDV71(kFile *pFile) :
    kFileBND(pFile)
{
    pszClassNm = "kFileBNDV71";
    pvHdr = &hdr;
    cbHdr = sizeof(hdr);

    pFile->setThrowOnErrors();
    pFile->readAt(&hdr, sizeof(hdr), 0);
    if (    memcmp(hdr.bind_id, BINDID_V70, sizeof(hdr.bind_id))
        &&  memcmp(hdr.bind_id, BINDID_V71, sizeof(hdr.bind_id)))
        throw(kError(kError::INVALID_SIGNATURE));

    if (hdr.endian != ENDIAN_LITTLE)
        throw (kError(kError::UNSUPPORTED_ENDIAN));

    if (hdr.relno != BINDREL_V7)
        throw(kError(kError::INCORRECT_VERSION));
}

unsigned long   kFileBNDV71::queryRelNo(void) const
{   return hdr.relno; }

char *          kFileBNDV71::queryApplication(char *pszBuffer) const
{
    memcpy(pszBuffer, hdr.application, sizeof(hdr.application));
    pszBuffer[sizeof(hdr.application)] = '\0';
    return trimR(pszBuffer);
}

char *          kFileBNDV71::queryTimeStamp(char *pszBuffer) const
{
    memcpy(pszBuffer, hdr.timestamp, sizeof(hdr.timestamp));
    pszBuffer[sizeof(hdr.timestamp)] = '\0';
    return trimR(pszBuffer);
}

char *          kFileBNDV71::queryCreator(char *pszBuffer) const
{
    memcpy(pszBuffer, hdr.creator, sizeof(hdr.creator));
    pszBuffer[sizeof(hdr.creator)] = '\0';
    return trimR(pszBuffer);
}

char *          kFileBNDV71::queryPrepId(char *pszBuffer) const
{
    memcpy(pszBuffer, hdr.prep_id, sizeof(hdr.prep_id));
    pszBuffer[sizeof(hdr.prep_id)] = '\0';
    return trimR(pszBuffer);
}

char *          kFileBNDV71::queryColid(char *pszBuffer) const
{
    *pszBuffer = '\0';
    if (hdr.colid_default || hdr.colid_length == 0 || hdr.colid_value[0] == ' ')
        return NULL;
    return strncat(pszBuffer, hdr.colid_value, hdr.colid_length);
}

char *          kFileBNDV71::queryOwner(char *pszBuffer) const
{
    *pszBuffer = '\0';
    if (hdr.owner_default || hdr.owner_length == 0 || hdr.owner_value[0] == ' ')
        return NULL;
    return strncat(pszBuffer, hdr.owner_value, hdr.owner_length);
}

char *          kFileBNDV71::queryQual(char *pszBuffer) const
{
    *pszBuffer = '\0';
    if (hdr.qual_default || hdr.qual_length == 0 || hdr.qual_value[0] == ' ')
        return NULL;
    return strncat(pszBuffer, hdr.qual_value, hdr.qual_length);
}

char *          kFileBNDV71::queryVrsn(char *pszBuffer) const
{
    *pszBuffer = '\0';
    if (hdr.vrsn_default || hdr.vrsn_length == 0 || hdr.vrsn_value[0] == ' ')
        return NULL;
    return strncat(pszBuffer, hdr.vrsn_value, hdr.vrsn_length);
}

unsigned long   kFileBNDV71::queryIsol(void) const
{   return hdr.isol_default ? kFileBND::notset : hdr.isol_value; }

unsigned long   kFileBNDV71::queryBlck(void) const
{   return hdr.blck_default ? kFileBND::notset : hdr.blck_value; }

unsigned long   kFileBNDV71::queryDate(void) const
{   return hdr.date_default ? kFileBND::notset : hdr.date_value; }

unsigned long   kFileBNDV71::queryNumHostvars(void) const
{   return hdr.num_hostvars; }

unsigned long   kFileBNDV71::queryMaxSect(void) const
{   return hdr.max_sect; }

unsigned long   kFileBNDV71::queryNumStmt(void) const
{   return hdr.num_stmt; }

unsigned long   kFileBNDV71::queryStatements(void) const
{   return hdr.statements; }

unsigned long   kFileBNDV71::queryDeclarel(void) const
{   return hdr.declarel; }

unsigned long   kFileBNDV71::queryDeclare(void) const
{   return hdr.declare; }


int kFileBNDV71::setApplication(const char *pszApplication)
{   return setField2(pszApplication, &hdr.application[0], sizeof(hdr.application)); }

int kFileBNDV71::setTimeStamp(const char *pszTimeStamp)
{   return setField2(pszTimeStamp, &hdr.timestamp[0], sizeof(hdr.timestamp)); }

int kFileBNDV71::setCreator(const char *pszCreator)
{   return setField2(pszCreator, &hdr.creator[0], sizeof(hdr.creator)); }

int kFileBNDV71::setPrepId(const char *pszPrepId)
{   return setField2(pszPrepId, &hdr.prep_id[0], sizeof(hdr.prep_id)); }

int kFileBNDV71::setColid(const char *pszColid)
{   return setField(pszColid, (PBINDOPTSTR)&hdr.colid_reserved, sizeof(hdr.colid_value), 8, FALSE); }

int kFileBNDV71::setOwner(const char *pszOwner)
{   return setField1Blank(pszOwner, (PBINDOPTSTR)&hdr.owner_reserved, sizeof(hdr.owner_value)); }

int kFileBNDV71::setQual(const char *pszQual)
{   return setField(pszQual, (PBINDOPTSTR)&hdr.qual_reserved, sizeof(hdr.qual_value)); }

int kFileBNDV71::setVrsn(const char *pszVrsn)
{   return setField(pszVrsn, (PBINDOPTSTR)&hdr.vrsn_reserved, sizeof(hdr.vrsn_value)); }

int kFileBNDV71::setIsol(unsigned long Isol)
{
    if ((Isol >= kFileBND::iso_rr && Isol <= kFileBND::iso_nc) || Isol == kFileBND::notset)
        return setField(Isol, (PBINDOPTUL)&hdr.isol_reserved);
    return kError::INVALID_PARAMETER;
}

int kFileBNDV71::setBlck(unsigned long Blck)
{
    if ((Blck >= kFileBND::blck_unambig && Blck <= kFileBND::blck_no) || Blck == kFileBND::notset)
        return setField(Blck, (PBINDOPTUL)&hdr.blck_reserved);
    return kError::INVALID_PARAMETER;
}

int kFileBNDV71::setDate(unsigned long Date)
{
    if ((Date >= kFileBND::date_default && Date <= kFileBND::date_local)/* || Blck == kFileBND::notset*/)
        return setField(Date, (PBINDOPTUL)&hdr.date_reverved);
    return kError::INVALID_PARAMETER;
}










/**
 * Operator for comparing a bind program id and a bind packet.
 * @returns TRUE if equal. FALSE if not.
 * @param   BndFile     Reference to a bind file.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
KBOOL kFileBNDPrgId::operator ==(const kFileBND &BndFile) const
{
    char    szBuffer1[257];
    char    szBuffer2[134];

    if (    !BndFile.queryApplication(szBuffer1)
        ||  !this->queryPlanname(szBuffer2)
        ||  strcmp(szBuffer1, szBuffer2))
        return FALSE;

    if (    !BndFile.queryTimeStamp(szBuffer1)
        ||  !this->queryContoken(szBuffer2)
        ||  strcmp(szBuffer1, szBuffer2))
        return FALSE;

    if (    !(   BndFile.queryColid(szBuffer1)
              || BndFile.queryCreator(szBuffer1))
        ||  !this->querySqlUser(szBuffer2)
        ||  strcmp(szBuffer1, szBuffer2))
        return FALSE;

    return TRUE;
}


/**
 * Operator for comparing a bind program id and a bind packet.
 * @returns TRUE if not equal. FALSE if equal.
 * @param   BndFile     Reference to a bind file.
 */
KBOOL kFileBNDPrgId::operator !=(const kFileBND &BndFile) const
{
    return !operator==(BndFile);
}


/**
 * This function will scan the given executable file for program id
 * structs. For each an kFileBNDPrgId object will be made and the
 * pfnCallback function will be called with this as a parameter.
 * @returns 0 on success. return from pageGet, pagePut or callback function on error.
 * @param   pFileLX         Pointer to file object to scan and write.
 * @param   fDontWrite      If set we'll not write the changes back to the executable.
 * @param   pfnCallBack     Pointer to the callback function. This is called with a
 *                          PrgId object and the passed in user parameter (ulUser).
 *                          If it failes (i.e. rc != 0) we'll fail.
 *                          If the PrgId object is dirty after the call (it was clean before!)
 *                          and !fDontWrite then an attempt to write the changes will be done.
 *                          If this write attempt failes, then we'll fail too.
 * @param   ulUser          User parameter to the callback function.
 * @sketch
 * @status
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 * @remark  TODO: kFileLX should be changed to kPageI+kSegmentI.
 */
int kFileBNDPrgId::scanExecutable(kFileLX *pFileLX, KBOOL fDontWrite, callback pfnCallback, unsigned long ulUser)
{
    /*
     * Scan the LX file.
     */
    int cObj = pFileLX->getObjectCount();
    for (int iObj = 0; iObj < cObj; iObj++)
    {
        struct o32_obj * pObj = pFileLX->getObject(iObj);
        if (!pObj)
            continue; //urg!

        /*
         * Scan looking for progid structs.
         */
        if (    (pObj->o32_flags & (OBJREAD | OBJBIGDEF | OBJEXEC | OBJRSRC))
                                == (OBJREAD | OBJBIGDEF)
            &&  pObj->o32_mapsize != 0)
        {
            char        pachPages[2*4096];
            int         iPage = 0;
            int         fCrossPage = 0;
            int         offObj = 0;
            int         rc;

            /*
             * Get the first page.
             */
            rc = pFileLX->pageGet(&pachPages[0x1000], iObj, offObj);
            if (rc)
            {
                fprintf(stdout, "Error: Failed to get page at address 0x%08x. err = %d\n",
                        pObj->o32_base + offObj, rc);
                return rc;
            }


            /*
             * Scan a segment.
             */
            while (offObj / 0x1000 < pObj->o32_mapsize)
            {
                /*
                 * Read next page so we allways have two pages in the buffer.
                 * This is done so due to possible cross page stuff.
                 */
                memcpy(&pachPages[0], &pachPages[0x1000], 0x1000);
                int fDirty = fCrossPage;
                fCrossPage = 0;
                if (offObj + 0x1000 < pObj->o32_size)
                {
                    rc = pFileLX->pageGet(&pachPages[0x1000], iObj, offObj + 0x1000);
                    if (rc)
                    {
                        fprintf(stdout, "Error: Failed to get page at address 0x%08x. err = %d\n",
                                pObj->o32_base + offObj, rc);
                        return rc;
                    }
                }
                else
                    memset(&pachPages[0x1000], 0, 0x1000);


                /*
                 * Scan a Page.
                 */
                char *          pch = &pachPages[0];
                kFileBNDPrgId * pPrgId = NULL;
                while (pch < &pachPages[0x1000])
                {
                    /* Typical sequences.       decoded
                     * bind relno
                     * 0x0201:  oA AB AC BC (DB 1.1?)
                     *      oA - length      40 bytes
                     *      AB - rp_rel_num  1.0
                     *      AC - db_rel_num  2.0
                     *      BC - bf_rel_num  2.1
                     * 0x0202:  oA AB AE CC (DB2 1.2?)
                     *      oA - length      40 bytes
                     *      AB - rp_rel_num  1.0
                     *      AE - db_rel_num  4.0
                     *      CC - bf_rel_num  2.2
                     * 0x0300:  oA AB AG AD (DB2 2.x)
                     *      oA - length      40 bytes
                     *      AB - rp_rel_num  1.0
                     *      AG - db_rel_num  6.0
                     *      AD - bf_rel_num  3.0
                     * 0x0500:  oA AB AI AF (DB2 5.x)
                     *      oA - length      40 bytes
                     *      AB - rp_rel_num  1.0
                     *      AI - db_rel_num  8.0
                     *      AF - bf_rel_num  5.0
                     * 0x0600:  oA AB AJ AG (DB2 6.x)
                     *      AB - rp_rel_num  1.0
                     *      AJ - db_rel_num  9.0
                     *      AG - bf_rel_num  6.0
                     * 0x0700:  ?? AD AJ AH (DB2 7.0)
                     *      ?? - length       bytes
                     *      AC - rp_rel_num  2.0
                     *      AJ - db_rel_num  9.0
                     *      AH - bf_rel_num  7.0
                     *          *. AD AJ AH (DB2 7.1)
                     *      *. - length       bytes
                     *      AD - rp_rel_num  3.0
                     *      AJ - db_rel_num  9.0
                     *      AH - bf_rel_num  7.0
                     *
                     */

                    if (!memcmp(pch, "oAAB", 4)) /* This covers 0x0201 -> 0x0600 */
                    {   /* We have an old program header - maybe.
                         * Check the db and bind file release numbers.
                         * Rule - database release number:
                         *      minor number is 0 for all.
                         *      major number range is from 2 to 9 (inclusive).
                         * Rule - bind file release number:
                         *      minor number is 0 for all majors above 2.
                         *          for major=2 it must be less or equal to 2.
                         *      major number range is from 2 to 6 (inclusive).
                         */
                        if (    pch[4] == 'A' && pch[5] >= 'B' && pch[5] <= 'J'
                            &&  (pch[6] == 'A'
                                 ? pch[7] >= 'C' && pch[7] <= 'G'
                                 : pch[7] == 'C' && pch[6] >= 'A' && pch[6] <= 'D')
                            )
                        {
                            try
                            {
                                pPrgId = new kFileBNDPrgIdPre70(pch);
                            }
                            catch (kError err)
                            {
                                fprintf(stdout, "Internal error: Failed to create kFileBNDPrgIdPre70 object. err=%d\n"
                                                "                Object=0x%x(0-based) Offset=0x%08x\n",
                                        err.getErrno(), iObj, offObj + pch - &pachPages[0]);
                                if (err.getErrno() != kError::BAD_STRUCTURE)
                                    return kError::INTERNAL_ERROR;
                            }
                        }
                    }
                    else if (   !memcmp(pch + 2, "ACAJAH", 6)
                             && (unsigned short)*pch >= 42   /* BUGBUG/TODO/FIXME: need corrected header size!!! */
                             && (unsigned short)*pch <= 512  /* BUGBUG/TODO/FIXME: need corrected header size!!! */)
                    { /* 7.0 */
                        try
                        {
                            pPrgId = new kFileBNDPrgIdV70(pch);
                        }
                        catch (kError err)
                        {
                            fprintf(stdout, "Internal error: Failed to create kFileBNDPrgIdV70 object. err=%d\n"
                                            "                Object=0x%x(0-based) Offset=0x%08x\n",
                                    err.getErrno(), iObj, offObj + pch - &pachPages[0]);
                            if (err != kError::BAD_STRUCTURE)
                                return kError::INTERNAL_ERROR;
                        }
                    }
                    else if (!memcmp(pch + 2, "ADAJAH", 6))
                    { /* 7.1 */
                        if (*(unsigned short*)pch == 42)
                        {
                            try
                            {
                                pPrgId = new kFileBNDPrgIdV71(pch);
                            }
                            catch (kError err)
                            {
                                fprintf(stdout, "Internal error: Failed to create kFileBNDPrgIdV71 object. err=%d\n"
                                                "                Object=0x%x(0-based) Offset=0x%08x\n",
                                        err.getErrno(), iObj, offObj + pch - &pachPages[0]);
                                if (err != kError::BAD_STRUCTURE)
                                    return kError::INTERNAL_ERROR;
                            }
                        }
                        else
                        {
                            try
                            {
                                pPrgId = new kFileBNDPrgIdV71MF(pch, pFileLX, iObj, offObj + pch - &pachPages[0]);
                            }
                            catch (kError err)
                            {
                                fprintf(stdout, "Internal error: Failed to create kFileBNDPrgIdV71MF object. err=%d\n"
                                                "                Object=0x%x(0-based) Offset=0x%08x\n",
                                        err.getErrno(), iObj, offObj + pch - &pachPages[0]);
                                if (err != kError::BAD_STRUCTURE)
                                    return kError::INTERNAL_ERROR;
                            }
                        }
                    }


                    /*
                     * If we found a program id pPrgId will point to an program id object.
                     */
                    if (pPrgId)
                    {
                        /*
                         * Call callback.
                         * Work the dirty flag.
                         */
                        rc = pfnCallback(pPrgId, offObj + pObj->o32_base + pch - &pachPages[0], ulUser);
                        if (rc)
                            return rc;
                        int cchPrgId = pPrgId->querySize();
                        if (pPrgId->isDirty())
                        {
                            fDirty = TRUE;
                            if (cchPrgId + pch > &pachPages[0x1000])
                                fCrossPage = TRUE;
                        }

                        /*
                         * Cleanup. Make sure pPrgId is NULL (important).
                         */
                        delete pPrgId;
                        pPrgId = NULL;
                        pch += cchPrgId;
                    }
                    else
                        pch++;

                } /* loop - Scan a page */


                /*
                 * Write page?
                 */
                if (fDirty && !fDontWrite)
                {
                    rc = pFileLX->pagePut(&pachPages[0], iObj, offObj);
                    if (rc)
                    {
                        fprintf(stdout, "Error: Failed to put page at address 0x%08x. err = %d\n",
                                pObj->o32_base + offObj, rc);
                        return rc;
                    }
                }

                /*
                 * Next page
                 */
                offObj += 0x1000;

            } /* loop - scan a segment */


        } /* loop - scan an executable */


    } /* for loop */

    return 0;
}

/**
 * Internal worker which updates a field.
 * Doing all the necessary work to maintain fDirty.
 * This is for field which are padded with spaces.
 * @return  0 on success. kError on failure.
 * @param   pszNew      New value.
 * @param   pszField    Pointer to field.
 * @param   cchField    Size of the field.
 */
int kFileBNDPrgId::setField(const char *pszNew, char *pszField, int cchField)
{
    int cchNew = strlen(pszNew);
    if (cchNew > cchField)
        return kError::STRING_TO_LONG;
    if (!memcmp(pszField, pszNew, cchNew))
    {
        char *  psz = pszField + cchNew;
        int     cch = cchField - cchNew;
        while (cch && *psz == ' ')
            cch--, psz++;
        if (cch != 0)
        {
            memset(psz, ' ', cch);
            fDirty = TRUE;
        }
        return 0;
    }
    memcpy(pszField, pszNew, cchNew);
    memset(pszField + cchNew, ' ', cchField - cchNew);
    fDirty = TRUE;
    return 0;
}







/**
 * Constructor.
 * @param   pv  Pointer to bind program id struct.
 *              This is the one we should operate on.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
kFileBNDPrgIdPre70::kFileBNDPrgIdPre70(void *pv)
{
    pszClassNm = "kFileBNDPrgIdPre70";
    pProgId = (PBINDPROGID)pv;
}

char *kFileBNDPrgIdPre70::queryPlanname(char *pszBuffer) const
{
    *pszBuffer = '\0';
    return trimR(strncat(pszBuffer, pProgId->planname, sizeof(pProgId->planname)));
}

char *kFileBNDPrgIdPre70::queryContoken(char *pszBuffer) const
{
    *pszBuffer = '\0';
    return strncat(pszBuffer, pProgId->contoken, sizeof(pProgId->contoken));
}

char *kFileBNDPrgIdPre70::querySqlUser(char *pszBuffer) const
{
    *pszBuffer = '\0';
    return trimR(strncat(pszBuffer, pProgId->sqluser, sizeof(pProgId->sqluser)));
}

int   kFileBNDPrgIdPre70::setPlanname(char *pszPlanname)
{   return setField(pszPlanname, &pProgId->planname[0], sizeof(pProgId->planname)); }

int   kFileBNDPrgIdPre70::setContoken(char *pszContoken)
{   return setField(pszContoken, &pProgId->contoken[0], sizeof(pProgId->contoken)); }

int   kFileBNDPrgIdPre70::setSqlUser(char *pszSqlUser)
{   return setField(pszSqlUser, &pProgId->sqluser[0], sizeof(pProgId->sqluser)); }




/**
 * Constructor.
 * @param   pv  Pointer to bind program id struct.
 *              This is the one we should operate on.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
kFileBNDPrgIdV70::kFileBNDPrgIdV70(void *pv)
{
    pszClassNm = "kFileBNDPrgIdV70";
    pProgId = (PBINDPROGIDV70)pv;
}

char *kFileBNDPrgIdV70::queryPlanname(char *pszBuffer) const
{
    *pszBuffer = '\0';
    return trimR(strncat(pszBuffer, pProgId->planname, sizeof(pProgId->planname)));
}

char *kFileBNDPrgIdV70::queryContoken(char *pszBuffer) const
{
    *pszBuffer = '\0';
    return strncat(pszBuffer, pProgId->contoken, sizeof(pProgId->contoken));
}

char *kFileBNDPrgIdV70::querySqlUser(char *pszBuffer) const
{
    *pszBuffer = '\0';
    if (pProgId->sqluser_len > sizeof(pProgId->sqluser))
        return trimR(strncat(pszBuffer, pProgId->sqluser, sizeof(pProgId->sqluser)));
    return trimR(strncat(pszBuffer, pProgId->sqluser, pProgId->sqluser_len));
}


int   kFileBNDPrgIdV70::setPlanname(char *pszPlanname)
{   return setField(pszPlanname, &pProgId->planname[0], sizeof(pProgId->planname)); }

int   kFileBNDPrgIdV70::setContoken(char *pszContoken)
{   return setField(pszContoken, &pProgId->contoken[0], sizeof(pProgId->contoken)); }

int   kFileBNDPrgIdV70::setSqlUser(char *pszSqlUser)
{
    int rc = setField(pszSqlUser, &pProgId->sqluser[0], sizeof(pProgId->sqluser));
    if (!rc)
        pProgId->sqluser_len = strlen(pszSqlUser);
    return rc;
}




/**
 * Constructor.
 * @param   pv  Pointer to bind program id struct.
 *              This is the one we should operate on.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
kFileBNDPrgIdV71::kFileBNDPrgIdV71(void *pv)
{
    pszClassNm = "kFileBNDPrgIdV71";
    pProgId = (PBINDPROGIDV71)pv;

    if (pProgId->length != 42)
    {
        fprintf(stderr, "length: 0x%x02 (!= 42)\n", pProgId->length);
        throw(kError(kError::INTERNAL_ERROR));
    }

    if (pProgId->sqluser_len > sizeof(pProgId->sqluser))
    {
        fprintf(stderr, "length: 0x%x02 (!= %d)\n", pProgId->length, sizeof(pProgId->sqluser));
        throw(kError(kError::INTERNAL_ERROR));
    }
}


char *kFileBNDPrgIdV71::queryPlanname(char *pszBuffer) const
{
    *pszBuffer = '\0';
    return trimR(strncat(pszBuffer, pProgId->planname, sizeof(pProgId->planname)));
}

char *kFileBNDPrgIdV71::queryContoken(char *pszBuffer) const
{
    *pszBuffer = '\0';
    return strncat(pszBuffer, pProgId->contoken, sizeof(pProgId->contoken));
}

char *kFileBNDPrgIdV71::querySqlUser(char *pszBuffer) const
{
    *pszBuffer = '\0';
    if (pProgId->sqluser_len > sizeof(pProgId->sqluser))
        return trimR(strncat(pszBuffer, pProgId->sqluser, sizeof(pProgId->sqluser)));
    return trimR(strncat(pszBuffer, pProgId->sqluser, pProgId->sqluser_len));
}


int   kFileBNDPrgIdV71::setPlanname(char *pszPlanname)
{   return setField(pszPlanname, &pProgId->planname[0], sizeof(pProgId->planname)); }

int   kFileBNDPrgIdV71::setContoken(char *pszContoken)
{   return setField(pszContoken, &pProgId->contoken[0], sizeof(pProgId->contoken)); }


int   kFileBNDPrgIdV71::setSqlUser(char *pszSqlUser)
{
    /*
     * This is a bit spooky....
     *
     * We limit our selfs to the size of the existing sqluser.
     * Perhaps we may find some extra room, but I don't know how yet.
     * We'll pad this field with blanks and hope that DB2 will ignore them in any
     * query for a packet like it usually does with trailing blanks...
     */
    return setField(pszSqlUser, &pProgId->sqluser[0], pProgId->sqluser_len);
}



/**
 * Constructor.
 * @param   pv          Pointer to bind program id struct.
 *                      This is the one we should operate on.
 * @param   pFileExe    Currently kFileLX pointer, should later
 *                      be a kExecutableI pointer. We need this
 *                      to apply changes to the working storage
 *                      unpack code in some cases.
 * @author  knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 */
kFileBNDPrgIdV71MF::kFileBNDPrgIdV71MF(void *pv, kFileLX *pFileExe, int iObj, int offObj)
    : pExec(NULL), fDirtyPages(FALSE)
{
    pszClassNm = "kFileBNDPrgIdV71MF";
    pProgIdMF = (PBINDPROGIDV71MF)(void*)((int)pv - 1);
    pProgId71 = (PBINDPROGIDV71)(void*)((int)pv);

    if (*(unsigned char*)pv != 0)
    {
        /* This encoding is quite easy to change, we simply access it
         * using another structure.
         */
        enmType = enmEncoding1;
        if (pProgIdMF->length != 42)
        {
            fprintf(stderr, "length: 0x%02x (!= 42)\n", pProgIdMF->length);
            throw(kError(kError::BAD_STRUCTURE));
        }

        if (pProgIdMF->high_sqluser_len != 0)
        {
            fprintf(stderr, "high_sqluser_len: 0x%x02 (!= 0)\n", pProgIdMF->high_sqluser_len);
            throw(kError(kError::BAD_STRUCTURE));
        }

        if (pProgIdMF->sqluser_len > sizeof(pProgIdMF->sqluser))
        {
            fprintf(stderr, "length: 0x%02x (!= %d)\n", pProgIdMF->length, sizeof(pProgIdMF->sqluser));
            throw(kError(kError::BAD_STRUCTURE));
        }
    }
    else
    {   /* This is the hardest encoding.
         * We have to cross reference fixups to find the code which contains the
         * sqluser and sqluser_len.
         */
        enmType = enmEncoding2;
        if (pProgId71->length != 0)
        {
            fprintf(stderr, "length: 0x%02x (!= 0)\n", pProgId71->length);
            throw(kError(kError::BAD_STRUCTURE));
        }

        /*
         * Find reference to the data.
         */
        kRelocEntry reloc;

        if (pFileExe->relocXRefFindFirst(iObj, offObj + 2, &reloc))
        {

            if (reloc.isInternal())
            {
                int rc;
                cbPages = pFileExe->pageGetPageSize();
                ulSegment = reloc.ulSegment;
                offSegment = KALIGNDOWN(cbPages, reloc.offSegment - 1);
                if (reloc.offSegment + sizeof(BINDPROGIDV71MFCODE) - 1 >= KALIGNUP(cbPages, reloc.offSegment))
                    cbPages *= 2;

                pachPages = new char[cbPages];
                rc = pFileExe->pageGet(pachPages, ulSegment, offSegment);
                if (!rc)
                {
                    if (reloc.offSegment + sizeof(BINDPROGIDV71MFCODE) - 1 >= KALIGNUP(cbPages, reloc.offSegment))
                        rc = pFileExe->pageGet(pachPages, ulSegment, offSegment + pFileExe->pageGetPageSize());
                    if (!rc)
                    {
                        pProgIdCode = (PBINDPROGIDV71MFCODE)&pachPages[(reloc.offSegment - 1) % pFileExe->pageGetPageSize()];
                        if (    pProgIdCode->code0.achMovEsi[0] == 0xbe
                            &&  pProgIdCode->code0.achMovEdi[0] == 0xbf
                            &&  pProgIdCode->code0.achMovsw[0] == 0x66
                            &&  pProgIdCode->code0.achMovsw[1] == 0xa5
                            &&  pProgIdCode->code0.achMovCl7[0] == 0xb1
                            &&  pProgIdCode->code0.achMovCl7[1] == 0x07
                            &&  pProgIdCode->code0.achRepMovsd[0] == 0xf3
                            &&  pProgIdCode->code0.achRepMovsd[1] == 0xa5
                            &&  pProgIdCode->code0.achMovSqlUserLen[0] == 0x66
                            &&  pProgIdCode->code0.achMovSqlUserLen[1] == 0xc7
                            &&  pProgIdCode->code0.achMovSqlUserLen[2] == 0x05
                            &&  pProgIdCode->code0.achMovSqlUser1[0] == 0xc7
                            &&  pProgIdCode->code0.achMovSqlUser1[1] == 0x05
                            &&  pProgIdCode->code0.achMovSqlUser2[0] == 0xc7
                            &&  pProgIdCode->code0.achMovSqlUser2[1] == 0x47
                            &&  pProgIdCode->code0.achMovSqlUser2[2] == 0x06
                                )
                        {
                            pFileExe->relocXRefFindClose(&reloc);
                            pExec = pFileExe;
                            pussqluser_len = &pProgIdCode->code0.sqluser_len;
                            pachsqluser1 = &pProgIdCode->code0.sqluser1[0];
                            pachsqluser2 = &pProgIdCode->code0.sqluser2[0];
                            return;
                        }


                        if (    pProgIdCode->code1.achMovEsi[0] == 0xbe
                            &&  pProgIdCode->code1.achMovEdi[0] == 0xbf
                            &&  pProgIdCode->code1.achMovAxEsi[0] == 0x66
                            &&  pProgIdCode->code1.achMovAxEsi[1] == 0x8b
                            &&  pProgIdCode->code1.achMovAxEsi[2] == 0x6
                            &&  pProgIdCode->code1.achMovEdiAx[0] == 0x66
                            &&  pProgIdCode->code1.achMovEdiAx[1] == 0x89
                            &&  pProgIdCode->code1.achMovEdiAx[2] == 0x7
                            &&  pProgIdCode->code1.achAddEsi2[0] == 0x83
                            &&  pProgIdCode->code1.achAddEsi2[1] == 0xc6
                            &&  pProgIdCode->code1.achAddEsi2[2] == 0x2
                            &&  pProgIdCode->code1.achAddEdi2[0] == 0x83
                            &&  pProgIdCode->code1.achAddEdi2[1] == 0xc7
                            &&  pProgIdCode->code1.achAddEdi2[2] == 0x2
                            &&  pProgIdCode->code1.achMovCl7[0] == 0xb1
                            &&  pProgIdCode->code1.achMovCl7[1] == 0x07
                            &&  pProgIdCode->code1.achRepMovsd[0] == 0xf3
                            &&  pProgIdCode->code1.achRepMovsd[1] == 0xa5
                            &&  pProgIdCode->code1.achMovSqlUserLen[0] == 0x66
                            &&  pProgIdCode->code1.achMovSqlUserLen[1] == 0xc7
                            &&  pProgIdCode->code1.achMovSqlUserLen[2] == 0x05
                            &&  pProgIdCode->code1.achMovSqlUser1[0] == 0xc7
                            &&  pProgIdCode->code1.achMovSqlUser1[1] == 0x05
                            &&  pProgIdCode->code1.achMovSqlUser2[0] == 0xc7
                            &&  pProgIdCode->code1.achMovSqlUser2[1] == 0x47
                            &&  pProgIdCode->code1.achMovSqlUser2[2] == 0x06
                                )
                        {
                            pFileExe->relocXRefFindClose(&reloc);
                            pExec = pFileExe;
                            pussqluser_len = &pProgIdCode->code1.sqluser_len;
                            pachsqluser1 = &pProgIdCode->code1.sqluser1[0];
                            pachsqluser2 = &pProgIdCode->code1.sqluser2[0];
                            return;
                        }
                        else
                            fprintf(stderr, "invalid opcodes rc=%d\n", rc);
                    }
                    else
                        fprintf(stderr, "pageGet failed with rc=%d\n", rc);
                }
                else
                    fprintf(stderr, "pageGet failed with rc=%d\n", rc);
            }
            else
            {
                fprintf(stderr, "not internal\n");
                pFileExe->relocXRefFindClose(&reloc);
                throw(kError(kError::BAD_STRUCTURE));
            }

            pFileExe->relocXRefFindClose(&reloc);
        }
        else
            fprintf(stderr, "no Xref\n");
        throw(kError(kError::BAD_STRUCTURE));
    }
}


/**
 * Destructor needed to flush extra code pages and free memory for them.
 */
kFileBNDPrgIdV71MF::~kFileBNDPrgIdV71MF(void)
{
    if (fDirtyPages)
    {
        int rc = pExec->pagePut(&pachPages[0], ulSegment, offSegment);
        if (rc)
            fprintf(stderr, "pagePut failed with rc=%d in ~kFileBNDPrgIdV71MF\n", rc);
        if (cbPages > pExec->pageGetPageSize())
        {
            rc = pExec->pagePut(&pachPages[0], ulSegment, offSegment);
            if (rc)
                fprintf(stderr, "pagePut failed with rc=%d in ~kFileBNDPrgIdV71MF\n", rc);
        }

        delete pachPages;
        pachPages = NULL;
    }
}

char *kFileBNDPrgIdV71MF::queryPlanname(char *pszBuffer) const
{
    *pszBuffer = '\0';
    if (enmType == enmEncoding1)
        return trimR(strncat(pszBuffer, pProgIdMF->planname, sizeof(pProgIdMF->planname)));
    return trimR(strncat(pszBuffer, pProgId71->planname, sizeof(pProgId71->planname)));
}

char *kFileBNDPrgIdV71MF::queryContoken(char *pszBuffer) const
{
    *pszBuffer = '\0';
    if (enmType == enmEncoding1)
        return strncat(pszBuffer, pProgIdMF->contoken, sizeof(pProgIdMF->contoken));
    return strncat(pszBuffer, pProgId71->contoken, sizeof(pProgId71->contoken));
}

char *kFileBNDPrgIdV71MF::querySqlUser(char *pszBuffer) const
{
    *pszBuffer = '\0';
    if (enmType == enmEncoding1)
    {
        if (pProgIdMF->sqluser_len > sizeof(pProgIdMF->sqluser))
            return trimR(strncat(pszBuffer, pProgIdMF->sqluser, sizeof(pProgIdMF->sqluser)));
        return trimR(strncat(pszBuffer, pProgIdMF->sqluser, pProgIdMF->sqluser_len));
    }

    /* enmEncoding2 */
    memcpy(pszBuffer,     &pachsqluser1[0], 4);
    memcpy(pszBuffer + 4, &pachsqluser2[0], 4);
    pszBuffer[8] = '\0';
    return trimR(pszBuffer);
}

int kFileBNDPrgIdV71MF::querySize() const
{
    if (enmType == enmEncoding1)
        return (int)(&((PBINDPROGIDV71)0)->sqluser) + pProgIdMF->sqluser_len;
    return (int)(&((PBINDPROGIDV71)0)->sqluser) + 8;
}



int   kFileBNDPrgIdV71MF::setPlanname(char *pszPlanname)
{
    if (enmType == enmEncoding1)
        return setField(pszPlanname, &pProgIdMF->planname[0], sizeof(pProgIdMF->planname));
    return setField(pszPlanname, &pProgId71->planname[0], sizeof(pProgId71->planname));
}

int   kFileBNDPrgIdV71MF::setContoken(char *pszContoken)
{
    if (enmType == enmEncoding1)
        return setField(pszContoken, &pProgIdMF->contoken[0], sizeof(pProgIdMF->contoken));
    return setField(pszContoken, &pProgId71->contoken[0], sizeof(pProgId71->contoken));
}

/**
 * @remark  Use of this function implies that you should not create more than one instance
 *          of the class. If you do you might end up overwriting changes made by the
 *          different instances.
 */
int   kFileBNDPrgIdV71MF::setSqlUser(char *pszSqlUser)
{
    /* enmEncoding1:
     * This is a bit spooky....
     *
     * We limit our selfs to the size of the existing sqluser.
     * Perhaps we may find some extra room, but I don't know how yet.
     * We'll pad this field with blanks and hope that DB2 will ignore them in any
     * query for a packet like it usually does with trailing blanks...
     */
    if (enmType == enmEncoding1)
        return setField(pszSqlUser, &pProgIdMF->sqluser[0], pProgIdMF->sqluser_len);

    /* enmEncoding2:
     *
     * The sql user is encoded in the code segment so we'll have to set it there.
     */
    int cch = strlen(pszSqlUser);
    if (cch > 8)
        return kError::STRING_TO_LONG;
    memset(pachsqluser1, ' ', 4);
    memcpy(pachsqluser1, pszSqlUser, KMIN(4, cch));
    memset(pachsqluser2, ' ', 4);
    if (cch > 4)
        memcpy(pachsqluser2, pszSqlUser + 4, KMIN(4, cch - 4));
    *pussqluser_len = 8;

    /*
     * Mark pages dirty.
     */
    fDirtyPages = TRUE;

    return 0;
}
