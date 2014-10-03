# $Id: setup.os2prfvac308.mk,v 1.5 2002/08/07 16:15:48 phaller Exp $

# ---OS2, PROFILE, VAC308-------------------------
ENV_NAME="OS/2, Profile, IBM VisualAge for C++ 3.08"
ENV_STATUS=OK

#
# The tools
#
AR=ilib.exe
AS=alp.exe
CC=icc.exe
CXX=icc.exe
LINK=ilink.exe
IMPLIB=implib.exe
RC=rc.exe
RL=rc.exe
EXEPACK=lxlite.exe


#
# Extentions.
#
EXT_OBJ = obj
EXT_LIB = lib
EXT_ILIB= lib
EXT_EXE = exe
EXT_DLL = dll
EXT_RES = res


#
# The flags
#
AR_FLAGS=/nologo
AR_CMD=$(AR) $(AR_FLAGS) $@ @$(TARGET_LNK)
AR_LNK1= $(TARGET_OBJS: =&^
)
AR_LNK2= $(@R).lst
AS_FLAGS=-Mb -Sc -Sv:ALP +Od -D:DEBUG -D:OS2 -i:$(PATH_INCLUDE) $(AS_DEFINES) $(AS_INCLUDES)
AS_OBJ_OUT=-Fdo:

CC_FLAGS=/Q /DDEBUG /DOS2 /Ti+ /O- /Gh+ /Ss+ /C+ /I$(PATH_INCLUDE) $(CC_DEFINES) $(C_INCLUDES)
CC_FLAGS_EXE=$(CC_FLAGS) /Gm+ /Ge+ /Gn+
CC_FLAGS_DLL=$(CC_FLAGS) /Gm+ /Ge- /Gn-
CC_FLAGS_CRT=$(CC_FLAGS) /Gm+ /Ge-
CC_OBJ_OUT=/Fo
CC_PC_2_STDOUT=/Pd+ /P+

CXX_FLAGS=/Q /DDEBUG /DOS2 /Ti+ /O- /Gh+ /Ss+ /C+ /I$(PATH_INCLUDE) $(CXX_DEFINES) $(C_INCLUDES)
CXX_FLAGS_EXE=$(CXX_FLAGS) /Gm+ /Ge+ /Gn+
CXX_FLAGS_DLL=$(CXX_FLAGS) /Gm+ /Ge- /Gn-
CXX_FLAGS_CRT=$(CXX_FLAGS) /Gm+ /Ge-
CXX_OBJ_OUT=/Fo
CXX_PC_2_STDOUT=/Pd+ /P+

IMPLIB_FLAGS=/NOI /Nologo

LINK_FLAGS=/nologo /de /map /NOE /NOD /Optfunc
LINK_FLAGS_EXE=$(LINK_FLAGS) /EXECutable /STACK:$(TARGET_STACKSIZE)
LINK_FLAGS_DLL=$(LINK_FLAGS) /DLL
LINK_CMD_EXE=$(LINK) $(LINK_FLAGS_EXE) @$(TARGET_LNK)
LINK_CMD_DLL=$(LINK) $(LINK_FLAGS_DLL) @$(TARGET_LNK)
LINK_LNK1=$(TARGET_OBJS: =^
) $(OBJ_PROFILE)
LINK_LNK2=/OUT:$(TARGET)
LINK_LNK3=/MAP:$(TARGET_MAP)
LINK_LNK4=$(TARGET_LIBS: =^
)
LINK_LNK5=$(TARGET_DEF)

RC_FLAGS=-r -n -i $(PATH_INCLUDE:;= -i ) $(RC_DEFINES) $(RC_INCLUDES)
RL_FLAGS=-x2 -n

#
# Libraries and object files.
#
LIB_OS=_DOSCALL.LIB _PMGPI.LIB _PMWIN.LIB os2386.lib
LIB_C_OBJ=cppom30.lib
LIB_C_DLL=cppom30i.lib
LIB_C_RTDLL=cppom30o.lib
OBJ_PROFILE=cppopa3.obj

# ---OS2, PROFILE, VAC308-------------------------

