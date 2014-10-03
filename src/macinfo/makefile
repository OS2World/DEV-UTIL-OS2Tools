# $Id: makefile,v 1.1 2002/03/04 13:31:39 phaller Exp $

#
# MacInfo
#
# History:
#
# $Log: makefile,v $
# Revision 1.1  2002/03/04 13:31:39  phaller
# Added MACInfo tool (display of protocol manager information
#
# Revision 1.1  2002/02/28 19:56:34  phaller
# Added Netbios tool
#
#
#

#
# Set the target
#

TARGET_NAME=macinfo
TARGET_LIBS=$(PATH_LIB)\toolrt.lib cppom30o.lib os2386.lib

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
