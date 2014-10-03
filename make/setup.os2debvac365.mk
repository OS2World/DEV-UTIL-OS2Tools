# $Id: setup.os2debvac365.mk,v 1.1 2001/05/16 01:29:11 bird Exp $

# ---OS2, DEBUG, VAC365-------------------------
ENV_NAME="OS/2, Debug, IBM VisualAge for C++ 3.6.5"
ENV_STATUS=OK

#
# The tools
#
CC=ICC.EXE
RC=RC.EXE
DEPEND=fastdep.exe
RM=deldir.exe
EXEPACK=lxlite.exe
LINK=ilink.exe
CD=%cd
CD_ENTER=cd
CD_LEAVE=cd ..
ECHO=%echo
# MAKE=@$(MAKE)

#
# The flags
#
CC_FLAGS=/DDEBUG /DOS2 /Ti+ /O- /Ss+ /C+
CC_FLAGS_EXE=$(CC_FLAGS)
CC_FLAGS_DLL=$(CC_FLAGS)
CXX_FLAGS=/DDEBUG /DOS2 /Ti+ /O- /Ss+ /C+ 
RC_FLAGS=
LINK_FLAGS=/de /map
LINK_FLAGS_EXE=$(LINK_FLAGS) /EXE
LINK_FLAGS_DLL=$(LINK_FLAGS) /DLL

# 
# Extra Settings
#
OBJ_PROFILE=


# ---OS2, DEBUG, VAC365-------------------------

