# $Id: makefile,v 1.6 2002/08/13 06:11:45 phaller Exp $

#
# toolrt
#
# History:
#
# $Log: makefile,v $
# Revision 1.6  2002/08/13 06:11:45  phaller
# Added RDTSC support
#
# Revision 1.5  2001/03/22 03:26:23  bird
# Updated to match latest build system.
#
# Revision 1.4  2001/03/02 11:10:08  phaller
# .
#
# Revision 1.3  2001/03/01 16:08:37  phaller
# .
#
# Revision 1.2  2001/02/28 18:08:41  phaller
# Added all sub-makefiles
#
#
#
#

#
# Set the target
#

TARGET_NAME = toolrt
TARGET_MODE = CRT

TARGET_OBJS =\
$(PATH_TARGET)\toolarg.$(EXT_OBJ)\
$(PATH_TARGET)\toolstack.$(EXT_OBJ)\
$(PATH_TARGET)\toollog.$(EXT_OBJ)\
$(PATH_TARGET)\util.$(EXT_OBJ)\
$(PATH_TARGET)\toolrt.$(EXT_OBJ)

TARGET_LIBS = $(LIB_C_OBJ) $(LIB_OS)

# we need to override the compiler flags so
# the runtime is bound statically!



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

