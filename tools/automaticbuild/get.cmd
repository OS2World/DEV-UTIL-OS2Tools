/* $Id: get.cmd,v 1.1 2002/01/11 22:13:03 bird Exp $
 *
 * Gets the CVS tree from netlabs.
 *
 * Copyright (c) 1999-2002 knut st. osmundsen (bird@anduin.net)
 *
 * GPL
 *
 */

    'cvs checkout .'
    if (RC <> 0) then call failure rc, 'CVS checkout . failed';
    exit(0);


failure: procedure
parse arg rc, sText;
    say 'rc='rc sText
    exit(rc);

