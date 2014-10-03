# $Id: makefile,v 1.3 2001/06/04 22:24:02 bird Exp $

#
# FastDep
#
# History:
#
# $Log: makefile,v $
# Revision 1.3  2001/06/04 22:24:02  bird
# Tried to speed it up slightly by not searching very old files which we have info on.
#
# Revision 1.2  2001/03/22 03:26:22  bird
# Updated to match latest build system.
#
# Revision 1.1  2001/03/02 11:12:29  phaller
# .
#
#
#

#
# Set the target
#
TARGET_NAME = fastdep

TARGET_OBJS =\
$(PATH_TARGET)\fastdep.$(EXT_OBJ)\
$(PATH_TARGET)\avl.$(EXT_OBJ)

TARGET_LIBS =\
$(LIB_OS)\
$(LIB_C_OBJ)


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
