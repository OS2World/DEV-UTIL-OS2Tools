/***********************************************************************
 * Name      : Module LineHistory
 * Funktion  : Kommandozeilensupport
 * Autor     : Patrick Haller [2002-03-23]
 ***********************************************************************/

#ifndef _LINEHISTORY_H_
#define _LINEHISTORY_H_

/***********************************************************************
 * Includes
 ***********************************************************************/

class LineHistory
{
  private:
    char** arrHistory;
    int iUsedSize;
    int iSize;
    int iCursor;
    int iLastLine;
  
    void eraseLine(char* pszLine);
    void eraseBack(char *pBuffer,
                   int iPos,
                   int iLen);
    void moveBack(char *pBuffer,
                  int iPos,
                  int iLen);
    void moveForward(char *pBuffer,
                     int iPos,
                     int iLen);
    void moveToEnd(char *pBuffer,
                   int iPos);
  
  
  
  public:
    LineHistory   (int iSize = 50);
    ~LineHistory  ();

    int  findLine (char* pBuffer,
                   int iPos,
                   int iDirection);
    int   addLine (char *pszLine);
    char* getLine (char *pBuffer,
                   int iBufferSize);
};


#endif /* _LINEHISTORY_H_ */
