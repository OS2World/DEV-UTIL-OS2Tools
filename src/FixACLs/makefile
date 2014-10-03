# $Id: makefile,v 1.4 2001/03/22 03:26:22 bird Exp $

#
# FixACLs
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

TARGET_NAME=fixacls
TARGET_LIBS=$(PATH_LIB)\toolrt.lib netapi32.lib $(LIB_OS) $(LIB_C_RTDLL)

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
