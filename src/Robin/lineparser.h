/***********************************************************************
 * Name      : Module Parser
 * Funktion  : Kommandozeilen / Eingabeanalyse
 * Autor     : Patrick Haller [2002-03-23]
 ***********************************************************************/

#ifndef _LINEPARSER_H_
#define _LINEPARSER_H_

/***********************************************************************
 * Includes
 ***********************************************************************/

class LineParser
{
  private:
    char** arrTokens;
    int iNumberOfTokens;
    int iCurrentToken;
  
    void parse             ( char* pszLine );
    void createToken       ( char* pszLast,
                             char* pszCursor );
    void addToken          ( char* pszToken );
    void clearTokens       ( void );
    int   cleanToken       ( char *pszLast,
                             char *pszCursor,
                             char *pszCleanToken );
  
  
  public:
    LineParser             ( char* pszLine = NULL );
    ~LineParser            ();
    void init              ( char* pszLine );
  
    int   getNumberOfTokens( void );
    char* getToken         ( int iTokenNumber );
    int   hasMoreTokens    ( void );
    char* nextToken        ( void );
};


#endif /* _LINEPARSER_H_ */
