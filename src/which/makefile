# $Id: makefile,v 1.1 2001/06/20 11:49:32 phaller Exp $

#
# Which
#
# History:
#
# $Log: makefile,v $
# Revision 1.1  2001/06/20 11:49:32  phaller
# added which tool
#
#
#

#
# Set the target
#

TARGET_NAME=which


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
