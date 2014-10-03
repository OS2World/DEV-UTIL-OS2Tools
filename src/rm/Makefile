# $Id: Makefile,v 1.4 2002/08/23 03:29:58 bird Exp $

#
# mini rm clone
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
TARGET_NAME = rm
TARGET_STACKSIZE=0x60000
TARGET_LIBS = \
$(LIB_OS)\
$(LIB_C_OBJ)

#
# Rules config.
#
!include $(MAKE_INCLUDE_PROCESS)
