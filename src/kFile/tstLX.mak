# $Id: tstLX.mak,v 1.1 2001/06/08 02:59:33 bird Exp $

#
# kFileLX Test program.
#

#
# Targets
#
TARGET_NAME = tstLX
MAKEFILE = $(TARGET_NAME).mak

TARGET_LIBS =\
$(LIB_OS)\
$(LIB_C_OBJ)\
$(PATH_LIB)\kFile.$(EXT_LIB)

TARGET_DEPS =\
$(PATH_LIB)\kFile.$(EXT_LIB)



#
# Load the build setup and process rules.
#
PATH_ROOT = ..\..
!include <$(PATH_ROOT)\make\setup.mak>

LINK_FLAGS_EXE=$(LINK_FLAGS) /EXE /STACK:$(TARGET_STACKSIZE) /EXEPACK

!include <$(MAKE_INCLUDE_PROCESS)>

