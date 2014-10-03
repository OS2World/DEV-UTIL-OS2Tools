# $Id: BindIt.mak,v 1.5 2001/06/07 22:51:49 bird Exp $

#
# BindTools - BindIt!
#


#
# Set the targets
#
TARGET_NAME = BindIt
MAKEFILE    = $(TARGET_NAME).mak

TARGET_RES=\
$(PATH_TARGET)\BindIt.$(EXT_RES)

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

