/*****************************************
 * Popuplog.OS2 Formatting Utility       *
 * (c) 1998 Patrick Haller Systemtechnik *
 *****************************************/

/* load libraries */
call RxFuncAdd "SysLoadFuncs","RexxUtil","SysLoadFuncs"
call SysLoadFuncs

/* setup some constants */
szPopup = SysBootDrive()||'\POPUPLOG.OS2'

/* command line arguments present ? */
parse arg szFile

if szFile\='' THEN szPopup=szFile

/* OK, here we go */
say 'Parsing '||szPopup

/* parser loop */
iState=0
iLineCount=0

do while lines(szPopup)
  szLine=LINEIN(szPopup)
  
  /* skip empty lines */
  IF (szLine \= '') & (LEFT(szLine,1)\='-') THEN
  DO
    /* DEBUG say '['||szLine||']' */
    
    SELECT
      WHEN iState=0 THEN szDate=SUBWORD(szLine,1,3)
      WHEN iState=1 THEN szProcess=szLine
      /* WHEN iState=2 THEN szException=szLine */
      WHEN iState=3 THEN szLocation=szLine
      OTHERWISE
    END
    
    /* skip to next state */
    iState=iState+1
  END
  ELSE
    iLineCount=iLineCount+1
  
  /* reset state machine */
  IF iLineCount>=3 THEN
  DO
    /* yield some output */
    say szDate' 'OVERLAY(szLocation,'        ')' 'szProcess
      
    iState=0
    iLineCount=0
  END
  
  IF LEFT(szLine,1)='-' THEN iLineCount=0
end

