/***********************************************************************
 * Name      : Module LineHistory
 * Funktion  : Kommandozeilensupport
 * Autor     : Patrick Haller [2002-03-23]
 ***********************************************************************/


/***********************************************************************
 * Includes
 ***********************************************************************/

#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "linehistory.h"


// #define DEBUG(a) printf a
#define DEBUG(a)


/*
 * To Do
 *
 * - support for insert mode
 * - support for CHAR_DELETE
 * - fix CHAR_BACKSPACE if cursor is in mid of string
 */

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

LineHistory::LineHistory  (int _iSize)
{
  iSize     = _iSize;
  iCursor   = 0;
  iUsedSize = 0;
  iLastLine = -1;
  
  arrHistory = new char*[ iSize ];
  memset(arrHistory,
         0,
         sizeof( char*) * iSize );
}


LineHistory::~LineHistory ()
{
  // @@@PH possible bug
  // check if child strings are deleted properly
  
  if (NULL != arrHistory)
    delete[] arrHistory;
}


int LineHistory::addLine(char *pszLine)
{
  if (arrHistory[ iCursor ] != NULL)
    delete arrHistory[ iCursor ];
  
  int iLength = strlen( pszLine ) + 1;
  arrHistory[ iCursor ] = new char[ iLength ];
  memcpy(arrHistory[ iCursor ],
         pszLine,
         iLength + 1);
  
  // setup the last recorded line
  iLastLine = iCursor;
  
  // advance cursor to next history slot
  iCursor++;
  if (iCursor >= iSize)
    iCursor = 0;
  else
    // remember how many slots are filled
    iUsedSize = iCursor;
  
  
  return iCursor;
}


#define CHAR_EXTENDED   0x0100
#define CHAR_ESCAPE     27
#define CHAR_BELL       7
#define CHAR_TAB        9
#define CHAR_SHIFT_TAB  CHAR_EXTENDED | 15
#define CHAR_RETURN     13
#define CHAR_LINEFEED   10
#define CHAR_BACKSPACE  8
#define CHAR_RIGHT      CHAR_EXTENDED | 'M'
#define CHAR_LEFT       CHAR_EXTENDED | 'K'
#define CHAR_UP         CHAR_EXTENDED | 'H'
#define CHAR_DOWN       CHAR_EXTENDED | 'P'
#define CHAR_HOME       CHAR_EXTENDED | 'G'
#define CHAR_END        CHAR_EXTENDED | 'O'
#define CHAR_DELETE     CHAR_EXTENDED | 'S'
#define CHAR_INSERT     CHAR_EXTENDED | 'R'
#define CHAR_F3         CHAR_EXTENDED | '='

#define CHAR_CTRL_LEFT  CHAR_EXTENDED | 's'
#define CHAR_CTRL_RIGHT CHAR_EXTENDED | 't'

#define CHAR_MOVE_RIGHT 12


int LineHistory::findLine(char* pBuffer,
                          int iPos,
                          int iDirection)
{
  int iMatch;
  
  for (iMatch = 1;
       iMatch <= iUsedSize;
       iMatch++)
  {
    int iSlot = iDirection * iMatch + iCursor;
    
    if (iSlot > 0)
      iSlot = iSlot % iUsedSize;
    else
      if (iSlot < 0)
        iSlot = (iUsedSize + iSlot);
    
    // get the previous line from the history buffer
    char* pszLine = arrHistory[ iSlot ];
    
#if 0
    printf("\nComparing [%s, %d, %s, length %d]\n",
           pBuffer,
           iSlot,
           pszLine,
           iPos);
#endif
    
    if (iPos == 0)
      return iSlot;
    
    if (strnicmp(pszLine,
                 pBuffer,
                 iPos) == 0)
      return iSlot;
  }
  
  // no match found
  return -1;
}


void LineHistory::moveBack(char *pBuffer,
                           int iPos,
                           int iLen)
{
  for (int i = iLen;
       i > iPos;
       i--)
  {
    putchar( CHAR_BACKSPACE );
  }
}


void LineHistory::moveForward(char *pBuffer,
                              int iPos,
                              int iLen)
{
  for (int i = iPos;
       i < iLen;
       i++)
  {
    putchar( pBuffer[ i ] );
  }
}


void LineHistory::moveToEnd(char *pBuffer,
                            int iPos)
{
  moveForward(pBuffer,
              iPos,
              strlen( pBuffer ));
}


void LineHistory::eraseBack(char *pBuffer,
                            int iPos,
                            int iLen)
{
  for (int i = iLen;
       i > iPos;
       i--)
  {
    putchar( CHAR_BACKSPACE );
    putchar( ' ' );
    putchar( CHAR_BACKSPACE );
  }
}



void LineHistory::eraseLine(char* pszLine)
{
  int iLength = strlen( pszLine );
  
  eraseBack(pszLine,
            0,
            iLength);
}


char* LineHistory::getLine(char *pBuffer,
                           int iBufferSize)
{
  int  ch;
  int  iPos = 0;
  int  flagExtended;
  
  // clear the buffer
  memset(pBuffer,
         0,
         iBufferSize);
  
  for(;;)
  {
    // read from keyboard
    ch = getch();
    if (0 == ch)
      ch = getch() | CHAR_EXTENDED;
    
    switch(ch)
    {
      case CHAR_ESCAPE:
        // clear input line
        eraseLine( pBuffer );
        iPos = 0;
        pBuffer[0] = 0;
        break;
      
      
      case CHAR_F3:
        if ( (iUsedSize == 0) ||
             (iLastLine == -1) )
          break;
      
        moveToEnd( pBuffer,
                   iPos );
        eraseLine( pBuffer );
      
        strncpy(pBuffer,
                arrHistory[ iLastLine ],
                iBufferSize);
        pBuffer[ iBufferSize - 1] = 0;
      
        printf( pBuffer );
      
        iPos = strlen(pBuffer);
        break;
      
      
      case CHAR_LINEFEED:
        break;
      
      
      case CHAR_RETURN:
        // accept current input
        addLine( pBuffer );
        return pBuffer;
      
      
      case CHAR_HOME:
        moveBack(pBuffer,
                 0,
                 iPos);
        iPos = 0;
        break;
      
      
      case CHAR_END:
      {
        int iLength = strlen( pBuffer );
        moveForward(pBuffer,
                    iPos,
                    iLength);
        iPos = iLength;
        break;
      }
             
      
      case CHAR_TAB:
      {
        // scan for the first likely match
        if (iUsedSize == 0)
          break;
      
        int iMatch = findLine( pBuffer, iPos, 1 );
        if (iMatch == -1)
        {
          // no match found
          putchar( CHAR_BELL );
          break;
        }
        
        moveToEnd( pBuffer,
                   iPos );
        eraseLine( pBuffer );
      
        iCursor = iMatch;
        strncpy(pBuffer,
                arrHistory[ iCursor ],
                iBufferSize);
        pBuffer[ iBufferSize - 1] = 0;
      
        printf( pBuffer );
      
        // move back to current iPos!
        int iLen = strlen( pBuffer );
        moveBack(pBuffer,
                 iPos,
                 iLen);
      
        break;
      }
      
      
      case CHAR_SHIFT_TAB:
      {
        // scan for the first likely match
        if (iUsedSize == 0)
          break;
      
        int iMatch = findLine( pBuffer, iPos, -1 );
        if (iMatch == -1)
        {
          // no match found
          putchar( CHAR_BELL );
          break;
        }
      
        moveToEnd( pBuffer,
                   iPos );
        eraseLine( pBuffer );
      
        iCursor = iMatch;
        strncpy(pBuffer,
                arrHistory[ iCursor ],
                iBufferSize);
        pBuffer[ iBufferSize - 1] = 0;
      
        printf( pBuffer );
        
        // move back to current iPos!
        int iLen = strlen( pBuffer );
        moveBack(pBuffer,
                 iPos,
                 iLen);
      
        break;
      }
      
      
      case CHAR_UP:
        if (iUsedSize == 0)
          break;
      
        iCursor--;
        if (iCursor < 0)
          iCursor = iUsedSize - 1;
      
        moveToEnd( pBuffer,
                   iPos );
        eraseLine( pBuffer );
      
        strncpy(pBuffer,
                arrHistory[ iCursor ],
                iBufferSize);
        pBuffer[ iBufferSize - 1] = 0;
      
        printf( pBuffer );
      
        iPos = strlen(pBuffer);
      
        break;
      
      
      case CHAR_DOWN:
        if (iUsedSize == 0)
          break;
      
        iCursor++;
        if (iCursor >= iUsedSize)
          iCursor = 0;
      
        moveToEnd( pBuffer,
                   iPos );
        eraseLine( pBuffer );

        strncpy(pBuffer,
                arrHistory[ iCursor ],
                iBufferSize);
        pBuffer[ iBufferSize - 1] = 0;
      
        printf( pBuffer );
      
        iPos = strlen(pBuffer);
      
        break;

    
      case CHAR_LEFT:
        if (iPos <= 0)
        {
          // buffer is empty
          putchar( CHAR_BELL );
          break;
        }
      
        putchar( CHAR_BACKSPACE );
        iPos--;
        break;
      
      
      case CHAR_RIGHT:
        if ( (iPos >= iBufferSize - 1) ||
             (pBuffer[ iPos ] == 0) )
        {
          // buffer is full
          putchar( CHAR_BELL );
          break;
        }
      
        putchar( pBuffer[ iPos++ ] );
        break;

      
      case CHAR_BACKSPACE:
        if (iPos <= 0)
        {
          // buffer is empty
          putchar( CHAR_BELL );
          break;
        }
      
        putchar( CHAR_BACKSPACE );
        putchar(' ');
        putchar( CHAR_BACKSPACE );
        iPos--;
        pBuffer[ iPos ] = 0;
        break;
      
      case CHAR_CTRL_LEFT:
        while (iPos > 0)
        {
          putchar( CHAR_BACKSPACE );
          iPos--;
          
          if (pBuffer[ iPos ] == ' ')
            break;
        }
        break;
    
    
      case CHAR_CTRL_RIGHT:
      {
        int iLength = strlen( pBuffer );
        while (iPos < iLength)
        {
          putchar( pBuffer[ iPos ] );
          iPos++;
          
          if (pBuffer[ iPos ] == ' ')
            break;
        }
        break;
      }
      
      case CHAR_DELETE:
      
        break;
      
      
      default:
        if (iPos >= iBufferSize - 1)
        {
          // buffer is full
          putchar( CHAR_BELL );
          break;
        }
      
        putchar( ch );
        pBuffer[ iPos ] = ch;
        iPos++;
        break;
    }
  }
}
