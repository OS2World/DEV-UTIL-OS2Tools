# $Id: makefile,v 1.3 2001/11/28 14:34:06 phaller Exp $

#
# PMStart
#
# History:
#
# $Log: makefile,v $
# Revision 1.3  2001/11/28 14:34:06  phaller
# .
#
# Revision 1.2  2001/09/24 15:14:47  phaller
# .
#
# Revision 1.1  2001/09/24 15:11:00  phaller
# .
#
# Revision 1.2  2001/03/01 16:08:35  phaller
# .
#
# Revision 1.1  2001/02/28 18:08:33  phaller
# Added all sub-makefiles
#
#
#
#

#
# Set the target
#

TARGET_NAME=pmstart
TARGET_LIBS=$(LIB_OS) cppos30.lib

#
# Load the build setup
#
PATH_ROOT=..\..
!INCLUDE <$(PATH_ROOT)\make\setup.mak>

#
# Note:
# Compiler-dependend override of some compiler flags!
# - static linkage
# - PM program
#
TARGET_STACKSIZE=0x10000
CC_FLAGS_EXE = $(CC_FLAGS) /Gd- /Gm- /Ge+ /Gn+ /Gs+
LINK_FLAGS = /pmtype:pm /packd /packc /exepack /optfunc /nologo /Nod /NOE

#
# Build the targets
# by loading the processing rules
#

!ifndef MAKE_INCLUDE_PROCESS
!error MAKE_INCLUDE_PROCESS should have been defined by parent makefile!
!else
!INCLUDE <$(MAKE_INCLUDE_PROCESS)>
!endif
