# $Id: makefile,v 1.1 2002/04/23 13:55:55 phaller Exp $

#
# Whoami
#
# History:
#
# $Log: makefile,v $
# Revision 1.1  2002/04/23 13:55:55  phaller
# Added whoami tool
#
#
#

#
# Set the target
#

TARGET_NAME=whoami
TARGET_LIBS=$(PATH_LIB)\toolrt.lib $(LIB_OS) $(LIB_C_RTDLL) upm32.lib

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
