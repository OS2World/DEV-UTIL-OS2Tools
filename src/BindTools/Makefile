# $Id: Makefile,v 1.15 2001/12/04 17:04:49 bird Exp $

#
# BindTools - BindLib.
#


#
# Set the targets
#
TARGET_MODE = LIB

TARGET_NAME = BindLib

TARGET_OBJS =\
$(PATH_TARGET)\BindTS.$(EXT_OBJ)\
$(PATH_TARGET)\BindArgs.$(EXT_OBJ)\
$(PATH_TARGET)\BindMsg.$(EXT_OBJ)\
$(PATH_TARGET)\BindTmpF.$(EXT_OBJ)\
!if "$(DB2PATH)" != ""
$(PATH_TARGET)\BindRes.$(EXT_OBJ)\
$(PATH_TARGET)\Q21DB2_1.$(EXT_OBJ)\
$(PATH_TARGET)\Q61MVS_1.$(EXT_OBJ)\
$(PATH_TARGET)\Q71UDB_1.$(EXT_OBJ)\
$(PATH_TARGET)\BindCnct.$(EXT_OBJ)\
!endif

TARGET_DOCS = BindTool.txt


#
# Invoke makefiles after this target has been made.
#
!if "$(DB2PATH)" != ""
POSTMAKEFILES = BindPrep.mak BindChk.mak BindIt.mak BindSet.mak
!endif


#
# Load the build setup
#
PATH_ROOT=..\..
!include <$(PATH_ROOT)\make\setup.mak>


#
# Load the processing rules
#
!include <$(MAKE_INCLUDE_PROCESS)>


#
# Additions.
#
Q71UDB_1.c: $(@B).sqc
!if "$(DATABASE_71UDB)" != ""
    @$(ECHO) Precompiling C PC: $(CLRFIL)$(@R).sqc $(CLRRST)
    sqlprep $(@R).sqc $(DATABASE_71UDB) BINDFILE USING $(@B).bnd OUTPUT $(@B).c \
        DATETIME ISO ISOLATION UR /P COLLECTION BINDTOOL
!endif


Q21DB2_1.c: $(@B).sqc
!if "$(DATABASE_21DB2)" != ""
    @$(ECHO) Precompiling C PC: $(CLRFIL)$(@R).sqc $(CLRRST)
    sqlprep $(@R).sqc $(DATABASE_21DB2) BINDFILE USING $(@B).bnd OUTPUT $(@B).c \
        DATETIME ISO ISOLATION UR /P COLLECTION BINDTOOL
!endif


Q61MVS_1.c: $(@B).sqc
!if "$(DATABASE_61MVS)" != ""
    @$(ECHO) Precompiling C MVS: $(CLRFIL)$(@R).sqc $(CLRRST)
    sqlprep $(@R).sqc $(DATABASE_61MVS) BINDFILE USING $(@B).bnd OUTPUT $(@B).c \
        DATETIME ISO ISOLATION UR /P COLLECTION BINDTOOL VALIDATE BIND
!endif



