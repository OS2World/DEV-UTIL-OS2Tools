# $Id: BindPrep.mak,v 1.1 2001/06/16 22:51:50 bird Exp $

#
# BindTools - BindPrep!
#


#
# Set the targets
#
TARGET_NAME = BindPrep
MAKEFILE    = $(TARGET_NAME).mak

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

