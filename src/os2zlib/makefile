# $Id: makefile,v 1.1 2002/07/12 18:08:23 phaller Exp $

#
# ZLib
#

#
# Set the target
#
TARGET_NAME=OS2ZLIB
TARGET_MODE=DLL
TARGET_LIBS=$(PATH_LIB)\toolrt.lib rexx.lib $(LIB_OS) $(LIB_C_RTDLL)

TARGET_OBJS=\
$(PATH_TARGET)\adler32.obj \
$(PATH_TARGET)\compress.obj \
$(PATH_TARGET)\crc32.obj \
$(PATH_TARGET)\gzio.obj \
$(PATH_TARGET)\uncompr.obj \
$(PATH_TARGET)\deflate.obj \
$(PATH_TARGET)\trees.obj \
$(PATH_TARGET)\zutil.obj \
$(PATH_TARGET)\inflate.obj \
$(PATH_TARGET)\infblock.obj \
$(PATH_TARGET)\inftrees.obj \
$(PATH_TARGET)\infcodes.obj \
$(PATH_TARGET)\infutil.obj \
$(PATH_TARGET)\inffast.obj


#
# Load the build setup
#
PATH_ROOT=..\..
!INCLUDE <$(PATH_ROOT)\make\setup.mak>


#
# Overrides
#
PATH_INCLUDE=$(PATH_INCLUDE);..\..;..\..\..\openssl\openssl\include;


#
# Build the targets
# by loading the processing rules
#

!ifndef MAKE_INCLUDE_PROCESS
!error MAKE_INCLUDE_PROCESS should have been defined by parent makefile!
!else
!INCLUDE <$(MAKE_INCLUDE_PROCESS)>
!endif


# History:
#
# $Log: makefile,v $
# Revision 1.1  2002/07/12 18:08:23  phaller
# IOStream updates, OS2ZLib added
#
# Revision 1.2  2002/03/28 15:28:58  patrick
# Statically linked against CPPOM30
#
# Revision 1.1  2002/03/28 15:20:49  patrick
# Initial version of zlib 1.1.4 OS/2 port
#
#