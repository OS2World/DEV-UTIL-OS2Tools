# $Id: makefile,v 1.3 2002/07/02 12:52:38 phaller Exp $

#
# IOStream
#
# History:
#
# $Log: makefile,v $
# Revision 1.3  2002/07/02 12:52:38  phaller
# Added capability to dynamically load TCP/IP functions (boot disk support)
#
# Revision 1.2  2002/06/17 17:02:52  phaller
# SPlit source
#
# Revision 1.1  2002/03/07 18:28:02  phaller
# Added IOStream tool
#
#
#

#
# Set the target
#

TARGET_NAME=iostream
TARGET_LIBS=$(PATH_LIB)\toolrt.lib cppom30o.lib os2386.lib
TARGET_OBJS=\
$(PATH_TARGET)\iostream.obj\
$(PATH_TARGET)\io_file.obj\
$(PATH_TARGET)\io_net.obj


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
