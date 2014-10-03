# $Id: makefile,v 1.1 2001/08/09 01:31:40 bird Exp $

#
# Command Queue Daemon (and client)
#

#
# Set the target
#

TARGET_NAME=CmdQd


#
# Load the build setup
#
PATH_ROOT=..\..
!INCLUDE <$(PATH_ROOT)\make\setup.mak>

#
# Make it statically linked
#
TARGET_LIBS =\
$(LIB_OS)\
$(LIB_C_OBJ)


#
# Build the targets
# by loading the processing rules
#
!ifndef MAKE_INCLUDE_PROCESS
!error MAKE_INCLUDE_PROCESS should have been defined by parent makefile!
!else
!INCLUDE <$(MAKE_INCLUDE_PROCESS)>
!endif
