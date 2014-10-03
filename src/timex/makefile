# $Id: makefile,v 1.1 2002/07/28 12:10:20 phaller Exp $

#
# Timex
#
# History:
#
# $Log: makefile,v $
# Revision 1.1  2002/07/28 12:10:20  phaller
# Added TIMEX tool
#
#
#
#

#
# Set the target
#

TARGET_NAME=timex


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
