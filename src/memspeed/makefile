# $Id: makefile,v 1.1 2002/08/07 16:14:09 phaller Exp $

#
# Memory Speed Testing Utility
#

#
# Set the target
#
TARGET_NAME=MEMSPEED
TARGET_MODE=EXE

TARGET_OBJS=\
$(PATH_TARGET)\memspeed.obj\
$(PATH_TARGET)\util.obj\
$(PATH_TARGET)\transgen.obj\
$(PATH_TARGET)\transmm2.obj\
$(PATH_TARGET)\transmmx.obj\
$(PATH_TARGET)\trcommon.obj


#
# Load the build setup
#
PATH_ROOT=..\..
!INCLUDE <$(PATH_ROOT)\make\setup.mak>


#
# Overrides
#


#
# Build the targets
# by loading the processing rules
#

!ifndef MAKE_INCLUDE_PROCESS
!error MAKE_INCLUDE_PROCESS should have been defined by parent makefile!
!else
!INCLUDE <$(MAKE_INCLUDE_PROCESS)>
!endif


# History:
#
# $Log: makefile,v $
# Revision 1.1  2002/08/07 16:14:09  phaller
# Added tiny memory benchmark
#
#
#
