/* $Id: pack.cmd,v 1.1 2002/01/11 22:13:04 bird Exp $
 *
 * Make the two zip files.
 *
 * NOTE! This requires 4OS/2 for the DEL commands.
 *
 * Copyright (c) 1999-2000 knut st. osmundsen (knut.stange.osmundsen@mynd.no)
 *
 * Project Odin Software License can be found in LICENSE.TXT
 *
 */
    sStartDir = directory();

    if (/*DATE('B')//7 = 3*/ 1) then  /* weekly on Thursdays */
        sType = '-Weekly';
    else
        sType = '-Daily';

    /*
     * Make .WPI files.
     */
    /*
    call ChDir 'tools\install';
    'call odin.cmd 'sType' debug'
    if (RC <> 0) then call failure rc, 'odin.cmd debug failed.';
    'call odin.cmd 'sType' release'
    if (RC <> 0) then call failure rc, 'odin.cmd release failed.';
    'move *.wpi' sStartDir;
    if (RC <> 0) then call failure rc, 'failed to move the *.wpi ->' sStartDir;
    call ChDir '..\..';
    */

    /*
     * Make .ZIP files.
     */
    call packdir 'bin\debug', 'debug'
    call packdir 'bin\release', 'release'

    /*
     * Make copy.
     */
    if (/*DATE('B')//7 = 3*/ 0) then
        'copy *.wpi e:\DailyBuildArchive\'
    else
        'copy *.zip e:\DailyBuildArchive\'

    /* return successfully */
    exit(0);


packdir: procedure expose sStartDir;
parse arg sDir, sType;

    sZipFile = directory() || '\os2toolsbin-' || DATE(S) || '-' || sType || '.zip';

    /*
     * Change into the directory we're to pack and do some fixups
     */
    sRoot = directory();
    call ChDir sDir
    'del /Q /Y /Z *.cmd > nul 2>&1'
    'del /Q /Y /Z /X CVS > nul 2>&1'

    /* Copy root files into the pack directory. */
    call copy sRoot'\LICENSE.txt';
    call copy sRoot'\ChangeLog';
    call copy sRoot'\doc\Readme.txt';

    /*
     * Move (=rename) the /bin/<release|debug> dir into /pack/system32
     * and pack it.
     */
    say 'zip -9 -R' sZipFile '* -xCVS';
    'zip -9 -R' sZipFile '* -xCVS';
    if (RC <> 0) then
    do
        rc2 = rc;
        call failure rc2, 'zip...';
    end

    /* restore directory */
    call directory(sRoot);
    return;


/*
 * Changes the directory.
 * On error we will call failure.
 */
ChDir: procedure expose sStartDir;
    parse arg sDir

    'cd' sDir
    if (rc <> 0) then
    do
        call failure rc, 'Failed to ChDir into' sDir '(CWD='directory()').'
        return rc;
    end
    return 0;


/*
 * Copy a file.
 * On error we will call failure.
 */
Copy: procedure expose sStartDir
    parse arg sSrc, sDst, fNotFatal

    /* if no sDst set default */
    if (sDst = '') then sDst='.';
    if (fNotFatal = '') then fNotFatal = 0;

    'copy' sSrc sDst
    if (rc <> 0 & \fNotFatal) then
    do
        call failure rc, 'Copying' sSrc 'to' sDst 'failed.'
        return rc;
    end
    return 0;


/*
 * Complain about a failure and exit the script.
 * Note. Uses the global variable sStartDir to restore the current
 *       directory where the script was started from.
 * @param rc    Error code to write. (usually RC)
 * @param sText Description.
 */
failure: procedure expose sStartDir;
parse arg rc, sText;
    say 'rc='rc sText
    call directory sStartDir;
    exit(rc);

