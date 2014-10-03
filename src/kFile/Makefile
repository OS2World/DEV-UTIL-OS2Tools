# $Id: Makefile,v 1.6 2001/12/14 21:33:11 bird Exp $

#
# kFile - Generalized interfaces on fileformats.
#


#
# Tell buildenvironmet to build an object library named kFile.
#
TARGET_MODE = PUBLIB
TARGET_NAME = kFile

POSTMAKEFILES = tstOMF.mak tstLX.mak


#
# Object files.
#
TARGET_OBJS =\
$(PATH_TARGET)\kAssert.$(EXT_OBJ)\
$(PATH_TARGET)\kError.$(EXT_OBJ)\
$(PATH_TARGET)\kFile.$(EXT_OBJ)\
$(PATH_TARGET)\kFileBND.$(EXT_OBJ)\
$(PATH_TARGET)\kFileDEF.$(EXT_OBJ)\
$(PATH_TARGET)\kFileFormatBase.$(EXT_OBJ)\
$(PATH_TARGET)\kFileInterfaces.$(EXT_OBJ)\
$(PATH_TARGET)\kFileLX.$(EXT_OBJ)\
$(PATH_TARGET)\kFileOMF.$(EXT_OBJ)\
$(PATH_TARGET)\kFileOMFLib.$(EXT_OBJ)\
$(PATH_TARGET)\kFilePE.$(EXT_OBJ)\
$(PATH_TARGET)\kFileSDF.$(EXT_OBJ)\


#
# Load the build setup and process rules.
#
PATH_ROOT = ..\..
!include <$(PATH_ROOT)\make\setup.mak>
!include <$(MAKE_INCLUDE_PROCESS)>

