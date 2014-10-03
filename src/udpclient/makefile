# $Id: makefile,v 1.2 2001/03/22 03:26:23 bird Exp $

#
# udpbench2
#
# History:
#
# $Log: makefile,v $
# Revision 1.2  2001/03/22 03:26:23  bird
# Updated to match latest build system.
#
# Revision 1.1  2001/03/02 11:23:34  phaller
# Broke UDPBench2 into it's separate directories and some C++ setup fixes for VAC308
#
# Revision 1.2  2001/03/01 16:08:37  phaller
# .
#
# Revision 1.1  2001/02/28 18:08:41  phaller
# Added all sub-makefiles
#
#
#
#

#
# Set the target
#

TARGET_NAME=udpclient
TARGET_LIBS=$(PATH_LIB)\toolrt.lib tcpip32.lib so32dll.lib tcp32dll.lib $(LIB_OS) $(LIB_C_RTDLL)

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
