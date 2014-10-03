# $Id: makefile,v 1.1 2002/01/10 16:24:48 phaller Exp $

#
# DLLRNAME
#
# History:
#
# $Log: makefile,v $
# Revision 1.1  2002/01/10 16:24:48  phaller
# Added DLLRNAME tool
#
#
#

#
# Set the target
#
TARGET_NAME=dllrname
TARGET_OBJS=\
$(PATH_TARGET)\dllrname.obj\
$(PATH_TARGET)\ccollection.obj\
$(PATH_TARGET)\binaryfile.obj\
$(PATH_TARGET)\moduleimage.obj\
$(PATH_TARGET)\moduleimagelx.obj\
$(PATH_TARGET)\moduleimagene.obj


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
