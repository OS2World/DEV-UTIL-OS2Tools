# $Id: BindSet.mak,v 1.1 2001/06/09 04:22:42 bird Exp $

#
# BindTools - BindSet!
#


#
# Set the targets
#
TARGET_NAME = BindSet
MAKEFILE    = $(TARGET_NAME).mak

TARGET_OBJS =\
$(PATH_TARGET)\BindSet.$(EXT_OBJ)

TARGET_LIBS=\
$(PATH_LIB)\kFile.lib\
$(PATH_OBJ)\BindLib.$(EXT_LIB)\BindLib.$(EXT_LIB)\
db2api.lib\
$(LIB_OS)\
$(LIB_C_OBJ)

TARGET_DEPS=\
$(PATH_LIB)\kFile.lib\
$(PATH_OBJ)\BindLib.$(EXT_LIB)\BindLib.$(EXT_LIB)


#
# Load the build setup
#
PATH_ROOT=..\..
!include <$(PATH_ROOT)\make\setup.mak>


#
# Load the processing rules
#
!include <$(MAKE_INCLUDE_PROCESS)>

