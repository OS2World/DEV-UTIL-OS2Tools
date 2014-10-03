# $Id: makefile,v 1.6 2001/04/27 17:58:24 phaller Exp $

#
# Build alternate import libraries - none currently.
#
# History:
#
# $Log: makefile,v $
# Revision 1.6  2001/04/27 17:58:24  phaller
# .
#
# Revision 1.5  2001/03/22 03:30:27  bird
# Not very usefull any longer.
#
# Revision 1.4  2001/03/02 11:10:05  phaller
# .
#
# Revision 1.3  2001/02/28 15:18:01  phaller
# OK, things start to work. The basic skeleton is not in place and scanning the src\* directories or subsequent makefiles to process.
#
# Revision 1.2  2001/02/28 14:44:17  phaller
# Damned stupid make program. No !include with fully-qualified path names, no nested !ifs ... :[
#
# Revision 1.1  2001/02/27 15:05:35  phaller
# Build system skeleton matures ...
#
#
#

#
# Load the build setup
#
PATH_ROOT=..
!INCLUDE <$(PATH_ROOT)\make\setup.mak>

#
# Build the targets
#
_all: all

clean:
    $(TOOL_DELETE) *.lib

lib all dep install:
    @$(ECHO) .

