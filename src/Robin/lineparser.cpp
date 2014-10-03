/***********************************************************************
 * Name      : Module Parser
 * Funktion  : Kommandozeilen / Eingabeanalyse
 * Autor     : Patrick Haller [2002-03-23]
 ***********************************************************************/

/***********************************************************************
 * Includes
 ***********************************************************************/

#include <strings.h>
#include <stdio.h>
#include "lineparser.h"


// #define DEBUG(a) printf a
#define DEBUG(a)


/***********************************************************************
 * Name      :
 * Funktion  :
 * Parameter :
 * Variablen :
 * Ergebnis  :
 * Bemerkung :
 *
 * Autor     : Patrick Haller [2002-03-23]
 ***********************************************************************/

LineParser::LineParser(char* pszLine)
{
  DEBUG(("LineParser::LineParser(%s)\n",
         pszLine));
  
  arrTokens       = NULL;
  iNumberOfTokens = 0;
  iCurrentToken   = 0;
  
  parse( pszLine );
}


void LineParser::init( char* pszLine )
{
  DEBUG(("LineParser::init(%s)\n",
         pszLine));
  
  clearTokens();
  
  arrTokens       = NULL;
  iNumberOfTokens = 0;
  iCurrentToken   = 0;
  
  parse( pszLine );
}



LineParser::~LineParser()
{
  DEBUG(("LineParser::~LineParser(%s)"));
  
  clearTokens();
}


int LineParser::getNumberOfTokens()
{
  DEBUG(("LineParser::getNumberOfTokens()\n"));
  
  return iNumberOfTokens;
}


char* LineParser::getToken( int iTokenNr )
{
  DEBUG(("LineParser::getTokens(%d)\n",
        iTokenNr));
  
  if (iTokenNr < iNumberOfTokens)
    return arrTokens[ iTokenNr ];
  else
    return NULL;
}


int LineParser::hasMoreTokens( )
{
  DEBUG(("LineParser::hasMoreTokens\n"));
  
  return iNumberOfTokens > iCurrentToken;
}


char* LineParser::nextToken( )
{
  DEBUG(("LineParser::nextToken\n"));
  
  return getToken( iCurrentToken++ );
}


void LineParser::parse( char* pszLine )
{
  DEBUG(("LineParser::prase(%s)\n",
        pszLine));
  
  if (NULL == pszLine)
    return;
  
  int   flagIsInQuotes = 0;
  int   flagIsEscaped  = 0;
  char* pszLast        = pszLine;
  char* pszCursor      = pszLine;
  
  // parser loop
  while (*pszCursor != '\0')
  {
    if (flagIsEscaped)
      flagIsEscaped = 0;
    else
    {
      switch(*pszCursor)
      {
        case ' ':
          if (!flagIsInQuotes)
          {
            // extract this token
            createToken(pszLast, pszCursor);
            
            // store the new last-position marker
            pszLast = pszCursor + 1;
          }
          break;

        case '\"':
          flagIsInQuotes = !flagIsInQuotes;
          break;
        
        case '\\':
          flagIsEscaped = !flagIsEscaped;
          break;
      }
    }
    
    
    // advance to the next character
    pszCursor++;
  }
  
  // add the last token of the string to the list
  createToken(pszLast, pszCursor);
}


void LineParser::createToken(char* pszLast,
                             char* pszCursor)
{
  DEBUG(("LineParser::createToken(%08xh,%08xh)\n",
         pszLast,
         pszCursor));
  
  // determine length of cleaned token
  int iLength = cleanToken(pszLast,
                           pszCursor,
                           NULL);
  
  
  // Note: iLength == 1 means we need space
  // for the terminating zero character only.
  // That's nonsense :)
  if (iLength > 1)
  {
    char* pszToken = new char[ iLength ];
    if (NULL != pszToken)
    {
      cleanToken(pszLast,
                 pszCursor,
                 pszToken);

      // add the token to the list
      addToken( pszToken );
    }
  }
}


// clear out meta-characters such as backslash
// and quote characters.
int  LineParser::cleanToken(char *pszLast,
                            char *pszCursor,
                            char *pszCleanToken)
{
  DEBUG(("LineParser::cleanToken(%08xh,%08xh,%08xh)\n",
         pszLast,
         pszCursor,
         pszCleanToken));
  
  if (NULL == pszLast)
    return 0;
  
  int iNewLength     = 0;
  int flagIsInQuotes = 0;
  int flagIsEscaped  = 0;
  
  
  while (pszLast < pszCursor)
  {
    if (flagIsEscaped)
    {
      // add the character and count the length
      if (NULL != pszCleanToken)
        *pszCleanToken++ = *pszLast;
      iNewLength++;
      flagIsEscaped    = 0;
    }
    else
    {
      switch(*pszLast)
      {
        case '\"':
          flagIsInQuotes = !flagIsInQuotes;
          break;

        case '\\':
          flagIsEscaped = !flagIsEscaped;
          break;
        
        default:
          // add the character and count the length
          if (NULL != pszCleanToken)
            *pszCleanToken++ = *pszLast;
          iNewLength++;
          break;
      }
    }
    
    pszLast++;
  }
  
  // terminate the token string
  if (NULL != pszCleanToken)
    *pszCleanToken = 0;
  iNewLength++;
  
  return iNewLength;
}


void LineParser::addToken(char* pszToken)
{
  DEBUG(("LineParser::addToken(%s)\n",
         pszToken));
  
  char **newarrTokens = new char*[ ++iNumberOfTokens ];
  
  // check if the array could be allocated
  if (NULL == newarrTokens)
    return;
  
  // copy old content
  if (NULL != arrTokens)
  {
    memcpy(newarrTokens,
           arrTokens,
           sizeof( char* ) * (iNumberOfTokens - 1) );
           
    clearTokens();
  }
           
  arrTokens = newarrTokens;
  arrTokens[ iNumberOfTokens - 1] = pszToken;
}


void LineParser::clearTokens()
{
  DEBUG(("LineParser::clearTokens()\n"));
  
  if (NULL != arrTokens)
  {
    for(int i = 0;
        i < iNumberOfTokens;
        i++)
    {
      // @@@PH
      // is this necessary?
      // delete[] arrTokens[ i ];
    }

    delete[] arrTokens;
  }
}