# $Id: makefile,v 1.4 2001/03/22 03:26:22 bird Exp $

#
# ExeInfo
#
# History:
#
# $Log: makefile,v $
# Revision 1.4  2001/03/22 03:26:22  bird
# Updated to match latest build system.
#
# Revision 1.3  2001/03/02 11:10:06  phaller
# .
#
# Revision 1.2  2001/03/01 16:08:35  phaller
# .
#
# Revision 1.1  2001/02/28 18:08:32  phaller
# Added all sub-makefiles
#
#
#
#

#
# Set the target
#
TARGET_NAME=exeinfo
TARGET_OBJS=\
$(PATH_TARGET)\exeinfo.obj\
$(PATH_TARGET)\exe_le.obj\
$(PATH_TARGET)\exe_lx.obj\
$(PATH_TARGET)\exe_mz.obj\
$(PATH_TARGET)\exe_ne.obj\
$(PATH_TARGET)\exe_pe.obj


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
