# $Id: tstOMF.mak,v 1.2 2001/06/07 22:52:51 bird Exp $

#
# kFileOMF and kFileOMFLib Test program.
#

#
# Targets
#
TARGET_NAME = tstOMF
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
!include <$(MAKE_INCLUDE_PROCESS)>

