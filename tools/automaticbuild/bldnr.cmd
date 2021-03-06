/* $Id: bldnr.cmd,v 1.1 2002/01/11 22:13:02 bird Exp $
 *
 * Build number update script.
 *
 * Two operations:
 *      1. Increment the build number.
 *      2. Commit the build number file.
 *
 * Assumes that current directory is the root.
 *
 * Copyright (c) 2001-2002 knut st. osmundsen (bird@anduin.net)
 *
 * GPL
 *
 */

call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs';
call SysLoadFuncs;


/*
 * Parse parameters.
 */
parse arg sOperation sIgnore

/*
 * Save and change directory.
 */
sTree = directory();
'cd include';
if (rc <> 0) then call failure rc, 'cd include failed.';


/*
 * Do operation.
 */
if (substr(sOperation, 1, 3) = 'inc') then
do
    /*
     * Scan the build.h file for BUILD_NR.
     */
    sOut = 'build.h';
    sIn = 'build.h.backup';
    call SysFileDelete('build.h.backup');
    'copy' sOut sIn;
    if (rc) then call failure rc, 'backup copy failed';
    call SysFileDelete('build.h');

    rcIn = stream(sIn, 'c', 'open read');
    rcOut = stream(sOut, 'c', 'open write');
    if (pos('READY', rcIn) <> 1 | pos('READY', rcOut) <> 1) then
    do
        call stream(sIn, 'c', 'close');
        call stream(sOut, 'c', 'close');
        call failure 5, 'failed to open in or/and out file. rcIn='rcIn 'rcOut='rcOut;
    end

    /*
     * Copy loop which updates BUILD_NR when found.
     */
    fFound = 0;
    do while (lines(sIn))
        sLine = linein(sIn);
        if (\fFound & substr(strip(sLine), 1, 24) = '#define BUILD_NR ') then
        do
            parse var sLine '#define BUILD_NR' iBuildNr sComment;
            iBuildNr = strip(iBuildNr);
            sComment = strip(sComment);
            iBuildNr = iBuildNr + 1;
            sLine = '#define BUILD_NR       '||iBuildNr||'        '||sComment;
            say 'newln:' sLine;
            fFound = 1;
        end
        call lineout sOut, sLine;
    end
    call lineout sOut, '';
    call stream sIn, 'c', 'close';
    call stream sOut, 'c', 'close';
    rc = 0;
end
else if (sOperation = 'commit') then
do
    /*
     * Commit the build nr. file.
     */
    'cvs commit -m "Daily build - 'Date('S')'" build.h';
    if (rc <> 0) then call failed rc, 'failed to commit build.h'
end
else
do
    say 'invalid operation' sOperation'. Should be commit or inc.'
    rc = 87;
end


/*
 * Restore directory.
 */
call directory(sTree);

exit(rc);


/*
 * Fatal failures terminates here!.
 */
failure: procedure
parse arg rc, sText;
    say 'rc='rc sText
    exit(rc);

