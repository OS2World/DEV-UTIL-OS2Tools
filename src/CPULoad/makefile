# $Id: makefile,v 1.3 2002/08/07 16:13:22 phaller Exp $

#
# CPULoad
#
# History:
#
# $Log: makefile,v $
# Revision 1.3  2002/08/07 16:13:22  phaller
# Improved CPU info
#
# Revision 1.2  2001/03/01 16:08:34  phaller
# .
#
# Revision 1.1  2001/02/28 18:08:30  phaller
# Added all sub-makefiles
#
#
#
#

#
# Set the target
#

TARGET_NAME=cpuload
TARGET_OBJS=\
$(PATH_TARGET)\cpuload.$(EXT_OBJ)\
$(PATH_TARGET)\cpuhlp.$(EXT_OBJ)


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
