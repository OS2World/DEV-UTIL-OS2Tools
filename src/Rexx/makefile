# $Id: makefile,v 1.1 2002/07/29 15:32:04 phaller Exp $

#
# Rexx
#

#
# Set the target
#
TARGET=calc.cmd
TARGET2=popuplog.cmd
TARGET3=wpsdereg.cmd
TARGET4=wpsreg.cmd

#
# Load the build setup
#
PATH_ROOT=..\..
!INCLUDE <$(PATH_ROOT)\make\setup.mak>

#
# Override the rules
#

all:

lib:

dep:

install:
    if exist $(TARGET)  $(TOOL_COPY) $(TARGET)  $(PATH_BIN)
    if exist $(TARGET2) $(TOOL_COPY) $(TARGET2) $(PATH_BIN)
    if exist $(TARGET3) $(TOOL_COPY) $(TARGET3) $(PATH_BIN)
    if exist $(TARGET4) $(TOOL_COPY) $(TARGET4) $(PATH_BIN)


#
# Build the targets
# by loading the processing rules
#


# History:
#
# $Log: makefile,v $
# Revision 1.1  2002/07/29 15:32:04  phaller
# Added Rexx Scripts
#
#
#
