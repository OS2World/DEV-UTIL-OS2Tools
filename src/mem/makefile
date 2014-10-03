# $Id: makefile,v 1.1 2001/03/15 08:54:28 phaller Exp $

#
# Mem    
#
# History:
#
# $Log: makefile,v $
# Revision 1.1  2001/03/15 08:54:28  phaller
# Added MEM tool
#
#
#

#
# Set the target
#

TARGET_NAME=mem


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
