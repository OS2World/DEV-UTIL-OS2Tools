# $Id: makefile,v 1.6 2001/03/01 16:08:34 phaller Exp $

#
# CDROM
#
# History:
#
# $Log: makefile,v $
# Revision 1.6  2001/03/01 16:08:34  phaller
# .
#
# Revision 1.5  2001/03/01 15:36:32  phaller
# Improved linker support
#
# Revision 1.4  2001/02/28 18:08:30  phaller
# Added all sub-makefiles
#
# Revision 1.3  2001/02/28 15:18:01  phaller
# OK, things start to work. The basic skeleton is not in place and scanning the src\* directories or subsequent makefiles to process.
#
# Revision 1.2  2001/02/28 14:44:17  phaller
# Damned stupid make program. No !include with fully-qualified path names, no nested !ifs ... :[
#
# Revision 1.1  2001/02/28 13:55:25  phaller
# Added first makefile for the subsystems
#
#
#

#
# Set the target
#

TARGET_NAME=cdrom


#
# Load the build setup
#
PATH_ROOT=..\..
!INCLUDE <$(PATH_ROOT)\make\setup.mak>

#
# Build the targets
# by loading the processing rules
#

!ifndef MAKE_INCLUDE_PROCESS
!error MAKE_INCLUDE_PROCESS should have been defined by parent makefile!
!else
!INCLUDE <$(MAKE_INCLUDE_PROCESS)>
!endif
