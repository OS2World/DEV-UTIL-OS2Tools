/* BuildVer
 * (c) 2000 Patrick Haller <patrick.haller@innotek.de>
 *
 * This script does automatically increase build version numbers
 * for build systems.
 *
 * the following macros are currently supported:
 *
 * #define VERSION_MAJOR "0"
 * #define VERSION_MINOR "0"
 * #define VERSION_BUILD "0.0.0"
 */

parse arg szFilename


/* verify command line arguments */
if szFilename="" then 
do
   say "ERROR: no filename specified"
   exit 1
end


/* check if the build version file does exist at all, create it if not */
fileVersion=STREAM(szFilename,'c','QUERY EXISTS')
if fileVersion="" then
do
   /* just setup a default array of lines */
   arrLines.0=3
   arrLines.1='#define VERSION_MAJOR "0"'
   arrLines.2='#define VERSION_MINOR "0"'
   arrLines.3='#define VERSION_BUILD "0.0.0"'
end
else
do
   /* read in the whole file */
   call LINEIN szFilename,1,0 /* open and set position to begin */
   lineCount=0
   do while LINES(szFilename)
      szLine = LINEIN(szFilename)
      if szLine\="" then
      do
        lineCount=lineCount+1
        arrLines.lineCount=szLine
      end
   end

   arrLines.0=lineCount /* close the array */
end

/* label */
szBuild="(unmodified)"

/* parse the array for known lines */
do i = 0 to arrLines.0
  szLine=arrLines.i
  
   /* scan for known tokens */
   /* switched off
   if POS('VERSION_MAJOR',szLine)\=0 then
     arrLines.i=increaseVersion(szLine)
   else
   if POS('VERSION_MINOR',szLine)\=0 then
     arrLines.i=increaseVersion(szLine)
   else
   */
  if POS('VERSION_BUILD',szLine)\=0 then
  do
    arrLines.i=increaseVersion(szLine)
    szBuild=WORD(arrLines.i, WORDS(arrLines.i))
  end

end /* do */

/* write out the scanned lines to the build version file */
say 'Updating build version file '||szFilename||": "||szBuild

call LINEOUT szFilename,,1 /* open for write and reset */

do i = 1 to arrLines.0
   call LINEOUT szFilename,arrLines.i
end /* do */

call LINEOUT szFilename /* close the file */


/* OK, Done */
exit 0


/**************
 * Procedures *
 **************/

increaseVersion:
  parse arg szVersion

  /* #define MACRO VALUE */
  szDefine=WORD(szVersion,1)
  szMacro=WORD(szVersion,2)
  szValue=WORD(szVersion,3)

  /* verify what we got */
  if szDefine="" OR szMacro="" OR szValue="" then
    return szVersion

  /* OK, fiddle with the Value string */
  szQuote1=LEFT(szValue,1)
  szQuote2=RIGHT(szValue,1)

  /* filter quotes from the value string */
  szValue=SPACE(TRANSLATE(szValue,'  ', '."'||"'"))
  arrNumbers.0=WORDS(szValue)

  do i=1 to arrNumbers.0
     arrNumbers.i=WORD(szValue,i)
  end /* do */

  do i=arrNumbers.0 to 1 by -1
     arrNumbers.i=arrNumbers.i+1
     if arrNumbers.i>9 then
     do
        if i>1 then
          arrNumbers.i=0
     end
     else
       leave
  end /* do */

  szReturn=szDefine||' '||szMacro||' '||szQuote1

  do i=1 to arrNumbers.0
     szReturn=szReturn||arrNumbers.i
     if i<arrNumbers.0 then
       szReturn=szReturn||'.'
  end

  szReturn=szReturn||szQuote2
  return szReturn


outoforder:
  select
     when szMacro="VERSION_MAJOR" then
        do
           PARSE VALUE szValue WITH iMajor
           iMajor=iMajor+1
           szVersion=szDefine||' '||szMacro||' '||szQuote1||iMajor||szQuote2
        end

     when szMacro="VERSION_MINOR" then
        do
           PARSE VALUE szValue WITH iMinor
           iMinor=iMinor+1
           szVersion=szDefine||' '||szMacro||' '||szQuote1||iMinor||szQuote2
        end

     when szMacro="VERSION_BUILD" then
        do
           PARSE VALUE szValue WITH iA '.' iB '.' iC
           iC=iC+1
           if iC>9 then
           do
              iC=0
              iB=iB+1
              if iB>9 then
              do
                 iA=iA+1
                 iB=0
              end
           end

           szVersion=szDefine||' '||szMacro||' '||szQuote1||iA||'.'||iB||'.'||iC||szQuote2
        end
     otherwise
  end  /* select */

  return szVersion

