# $Id: makefile,v 1.2 2001/03/01 16:08:36 phaller Exp $

#
# testcom
#
# History:
#
# $Log: makefile,v $
# Revision 1.2  2001/03/01 16:08:36  phaller
# .
#
# Revision 1.1  2001/02/28 18:08:37  phaller
# Added all sub-makefiles
#
#
#
#

#
# Set the target
#

TARGET_NAME=testcom


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
