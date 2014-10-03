/* RexxTools/2
 * (c) 1997 Patrick Haller Systemtechnik
 *
 * Testscript
 */

call RxFuncAdd 'RtLoadFuncs','RexxTool','RtLoadFuncs'
call RtLoadFuncs

say 'RexxTool DLL loaded.'

say
say 'RtVersion()'
say RtVersion()

say
say 'RtStringSplit(stem,string,number of delimiters,delimiter'
szString="This is the string to split into single words."
rc=RtStringSplit(arrWords,szString,0," ");
say '['||szString||']'
say 'rc='rc
do i=1 to arrWords.0
  say i||'. '||arrWords.i
end

say
say 'RtWordWrap(stem,string,width)'
szString="This is a sample of a very long text which shall be broken into shorter lines."
rc=RtWordWrap(arrLines,szString,30)
say 'rc='rc
do i=1 to arrLines.0
  say i||'. '||arrLines.i
end

say
say 'RtQuerySysInfo(stem,start,end)'
rc=RtQuerySysInfo(arrInfo,1,5)
say 'rc='rc
do i=1 to arrInfo.0
  say i||'. '||arrInfo.i
end

say
say 'RtQueryPathInfo(stem,pathname)'
rc=RtQueryPathInfo(sFile,'RXTEST.CMD')
say 'rc='rc
do i=1 to sFile.0
  say i||'. '||sFile.i
end

say
say 'RtQueryPathInfo(stem,pathname)'
rc=RtQueryPathInfo(sFile,'D:\os2\dll\..\install\..\..\os2krnl')
say 'rc='rc
do i=1 to sFile.0
  say i||'. '||sFile.i
end


say
say 'RtQueryCtryInfo(stem,<country,codepage>)'
rc=RtQueryCtryInfo(sCountry)
say 'rc='rc
do i=1 to sCountry.0
  say i||'. '||sCountry.i
end

say
say 'RtFileChecksum(filename)'
rc=RtFileChecksum("D:\OS2\CMD.EXE","MD5")
say 'MD5='rc
rc=RtFileChecksum("D:\OS2\CMD.EXE","CRC32")
say 'CRC32='rc

say
say 'RtQueryModule(stem,module)'
rc=RtQueryModule(sModule,"PMSHELL.EXE")
say 'rc='rc
do i=1 to sModule.0
  say i||'. '||sModule.i
end

say
say 'RtReplaceModule(module, [new name], [backup name])'
rc=RtReplaceModule("REXXTOOL.DLL")
say 'rc='rc

/* RtFileDelete */

say
say 'Unloading RexxTool DLL'
rc=RtDropFuncs()
say 'RtDropFuncs='rc

