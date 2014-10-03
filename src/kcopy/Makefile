# $Id: Makefile,v 1.1 2002/09/19 05:10:12 bird Exp $

#
# kCopy - funny little file copying program.
#
# Copyright (C) 2000 knut st. osmundsen
#


#
# Setup config
#
PATH_ROOT = ..\..
!include $(PATH_ROOT)\make\setup.mak

#
# Target config
#
CC_DEFINES = -Tm+
TARGET_NAME = kCopy
TARGET_STACKSIZE=0x60000
TARGET_LIBS = \
$(LIB_OS)\
$(LIB_C_OBJ)

#
# Rules config.
#
!include $(MAKE_INCLUDE_PROCESS)
