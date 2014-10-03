/* $Id: build.cmd,v 1.1 2002/01/11 22:13:03 bird Exp $
 *
 * Builds debug and release editions of OS2Tools.
 *
 * Copyright (c) 1999-2002 knut st. osmundsen (bird@anduin.net)
 *
 * GPL
 *
 */
    /* debug build */
    'SET BUILD_MODE=DEBUG';
    'nmake dep';
    if (RC <> 0) then call failure rc, 'Make failed (dep).';
    'nmake';
    if (RC <> 0) then call failure rc, 'Make debug failed.';
    'nmake install';
    if (RC <> 0) then call failure rc, 'Make install debug failed.';

    /* release build */
    'SET BUILD_MODE=RELEASE';
    'nmake dep';
    if (RC <> 0) then call failure rc, 'Make failed (dep).';
    'nmake ';
    if (RC <> 0) then call failure rc, 'Make release failed .';
    'nmake install';
    if (RC <> 0) then call failure rc, 'Make install release failed.';

    exit(0);

failure: procedure
parse arg rc, sText;
    say 'rc='rc sText
    exit(rc);

