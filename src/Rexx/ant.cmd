/*
 * ANT Launcher Rexx Script for OS/2
 *
 * Copyright (c) 2002 Patrick Haller
 *
 * Copyright (c) 2001-2002 The Apache Software Foundation.  All rights
 * reserved.
 */

PARSE ARG szCmdLine
PARSE SOURCE szCmdSource

/* load rexxutils */
call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs'
call SysLoadFuncs


/* check for the pre-invokation hook */
envHOME=VALUE('HOME',,'OS2ENVIRONMENT')

scriptAntPre=envHome||'\antrc_pre.cmd'
IF STREAM(scriptAntPre,'c','QUERY EXISTS')\="" THEN
DO
    /* execute the pre-ant hook */
    '@'||scriptAntPre
END


/* determine certain system settings */
envANT_HOME=VALUE('ANT_HOME',,'OS2ENVIRONMENT')
IF envANT_HOME='' THEN
DO
    /* extract the path from the szCmdSource variable */
    iPathPos=WORDINDEX(szCmdSource,3)
    szPath=SUBSTR(szCmdSource,iPathPos)
    envANT_HOME=FILESPEC('drive',szPath)||FILESPEC('path',szPath)
    envANT_HOME=LEFT(envANT_HOME, LENGTH(envANT_HOME)-1)
    
    /* remove the \bin part from the end of the path */
    if RIGHT(envANT_HOME,4)="\bin" THEN
        envANT_HOME=LEFT(envANT_HOME, LENGTH(envANT_HOME)-4)
        
    IF envANT_HOME='' THEN
    DO
        /* try to determine the ANT location automatically */
        /* @@@PH */
        say 'ANT_HOME is not set and ant could not be located. Please set ANT_HOME.'
        exit
    END
END

envJAVA_HOME=VALUE('JAVA_HOME',,'OS2ENVIRONMENT')
IF envJAVA_HOME='' THEN
DO
    /* try to determine the JAVA location automatically */
    /* @@@PH */
    say 'JAVA_HOME is not set and ant could not be located. Please set JAVA_HOME.'
    exit
END

envANT_OPTS=VALUE('ANT_OPTS',,'OS2ENVIRONMENT')
envANT_ARGS=VALUE('ANT_ARGS',,'OS2ENVIRONMENT')


/* build a local classpath */
envCLASSPATH=VALUE('CLASSPATH',,'OS2ENVIRONMENT')

/* insert all .jar files found in %ant%\lib to the classpath */
rc=SysFileTree(envANT_HOME||'\lib\*.jar','arrJARs','FO')
do i = 1 to arrJARs.0
    envCLASSPATH=arrJARs.i||';'||envCLASSPATH
end

IF STREAM(envJAVA_HOME||'\lib\tools.jar','c','QUERY EXISTS')\="" THEN
    envCLASSPATH=envJAVA_HOME||'\lib\tools.jar;'||envCLASSPATH

IF STREAM(envJAVA_HOME||'\lib\classes.zip','c','QUERY EXISTS')\="" THEN
    envCLASSPATH=envJAVA_HOME||'\lib\classes.zip;'||envCLASSPATH

/* check how to call the java frontend */
if STREAM(envJAVA_HOME||'\bin\java.exe','c','query exists')\="" THEN
    /* call it where we found it */
    cmdJAVA=envJAVA_HOME||'\bin\java.exe'
else
if STREAM(envJAVA_HOME||'\jre\bin\java.exe','c','query exists')\="" THEN
    /* call it where we found it */
    cmdJAVA=envJAVA_HOME||'\jre\bin\java.exe'
else
    /* trust it's in the path */
    cmdJAVA='java.exe'

/* check for the jikes compiler */
envJIKES=VALUE('JIKES_PATH',,'OS2ENVIRONMENT')
if STREAM(envJIKES||'\jikes.exe','c','query exists')\="" then
    cmdJIKES='-Djikes.class.path='||envJikes
else
    cmdJIKES=''

/* finally call the java environment to execute ant */
dbgCmd=cmdJAVA||' -classpath '||envCLASSPATH||' -Dant.home='||envANT_HOME||' '||cmdJikes||envANT_OPTS||' org.apache.tools.ant.Main '||envANT_ARGS||' '||szCmdLine

/* print debug output */
/*
say 'ANT_PATH:['||envANT_HOME||']'
say 'JAVA_PATH:['||envJAVA_HOME||']'
say 'CMD:['||dbgCmd||']'
*/

'@'dbgCmd

/* check for the post-invokation hook */
scriptAntPost=envHome||'\antrc_post.cmd'
IF STREAM(scriptAntPost,'c','QUERY EXISTS')\="" THEN
DO
    /* execute the post-ant hook */
    '@'||scriptAntPost
END
