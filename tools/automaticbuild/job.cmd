/* $Id: job.cmd,v 1.1 2002/01/11 22:13:03 bird Exp $
 *
 * Main job for building OS/2.
 *
 * Copyright (c) 1999-2002 knut st. osmundsen (bird@anduin.net)
 *
 * GPL
 *
 */

    /* Load rexxutils functions */
    call RxFuncAdd 'SysLoadFuncs', 'RexxUtil', 'SysLoadFuncs';
    call SysloadFuncs;

    /*
     * Get source directory of this script
     */
    parse source sd1 sd2 sScript
    sScriptDir = filespec('drive', sScript) || filespec('path', sScript);
    sLogFile = sScriptDir || '\logs\' || Date('S') || '.log';
    sTree    = sScriptDir || '..\tree' || Date('S');

    /*
     * Clean tree, get it and build it.
     */
    'mkdir' sTree
    filespec('drive', sScript);
    'cd' sTree;
    if (rc <> 0) then call failure rc, 'cd ..\'sTree 'failed.';
    'call' sScriptDir || 'env.cmd'
    if (rc <> 0) then call failure rc, 'Env failed.';
    'call' sScriptDir || 'clean.cmd'
    if (rc <> 0) then call failure rc, 'Clean failed.';
    'call' sScriptDir || 'get.cmd'
    if (rc <> 0) then call failure rc, 'Get failed.';
    'call' sScriptDir || 'bldnr.cmd inc'
    if (rc <> 0) then call failure rc, 'Build Nr inc failed.';
    'call' sScriptDir || 'build.cmd 2>&1 | tee /a ' || sLogFile; /* 4OS/2 tee command. */
    if (rc <> 0) then call failure rc, 'Build failed.';
    'call' sScriptDir || 'bldnr.cmd commit'
    if (rc <> 0) then call failure rc, 'Build Nr commit failed.';

    /*
     * Pack and upload it.
     */
    'call' sScriptDir || 'pack.cmd  2>&1 | tee /a ' || sLogFile; /* 4OS/2 tee command. */
    if (rc <> 0) then call failure rc, 'Packing failed.';
    'start /BG "Uploading OS2Tools..." nice -f cmd /C' sScriptDir || 'ftp.cmd';
    if (rc <> 0) then say 'rc='rc' FTPing failed. i = ' || i;

    /* successfull exit */
    exit(0);


/*
 * fatal failures terminates here!.
 */
failure: procedure
parse arg rc, sText;
    say 'rc='rc sText
    exit(rc);

