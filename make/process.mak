# $Id: process.mak,v 1.20 2002/08/07 16:15:47 phaller Exp $

#
# Unix-like tools for OS/2
#
#   The common build process rules
#
# Note: this makefile is supposed to be included from the
# current source path.
#
# History:
#
# $Log: process.mak,v $
# Revision 1.20  2002/08/07 16:15:47  phaller
# ALP fixes
#
# Revision 1.19  2002/07/15 13:26:15  phaller
# Updated build system
#
# Revision 1.18  2002/01/22 17:41:14  bird
# Corrected the clean rule.
#
# Revision 1.17  2002/01/17 16:01:37  bird
# Added testcase rule.
#
# Revision 1.16  2001/06/18 16:40:49  bird
# TARGET_DOCS
#
# Revision 1.15  2001/06/07 22:53:01  bird
# TARGET_DEP -> TARGET_DEPS.
#
# Revision 1.14  2001/06/03 20:03:35  bird
# Added TARGET_DEP for adding main target dependencies. like libraries.
#
# Revision 1.13  2001/06/03 18:46:28  bird
# Added PUBLIB and POSTMAKEFILES.
#
# Revision 1.12  2001/05/17 01:02:29  bird
# Minor corrections.
#
# Revision 1.11  2001/05/16 01:19:50  bird
# Synced with PHP - lot's of improovments. Added colors.
#
# Revision 1.10  2001/03/22 03:28:31  bird
# Enhanced the build system.
#
# Revision 1.9  2001/03/02 11:10:05  phaller
# .
#
# Revision 1.8  2001/03/01 16:08:34  phaller
# .
#
# Revision 1.7  2001/03/01 15:36:31  phaller
# Improved linker support
#
# Revision 1.6  2001/03/01 13:20:09  phaller
# Files get compiled, no liker support yet
#
# Revision 1.5  2001/03/01 10:53:33  phaller
# .
#
# Revision 1.4  2001/03/01 10:41:09  phaller
# Now also creating target directories for the tools like /obj/os2debvac308/xxx.exe
#
# Revision 1.3  2001/02/28 18:08:29  phaller
# Added all sub-makefiles
#
# Revision 1.2  2001/02/28 15:18:01  phaller
# OK, things start to work. The basic skeleton is not in place and scanning the src\* directories or subsequent makefiles to process.
#
# Revision 1.1  2001/02/27 15:36:42  phaller
# .
#
#
#

#
# This makefile expects setup.mak and the specific setup to be included
# already. Plus there are several prerequisite environment variables
# subsystem makefiles need to set:
# TARGET_NAME is obligatory

!ifndef TARGET_NAME
!error fatal error: TARGET_NAME is not defined!
!endif

# provide overridable defaults
!ifndef TARGET
TARGET=$(PATH_TARGET)\$(TARGET_NAME).$(TARGET_EXT)
!endif

!ifndef TARGET_OBJS
TARGET_OBJS=$(PATH_TARGET)\$(TARGET_NAME).$(EXT_OBJ)
!endif

!ifndef TARGET_LIBS
TARGET_LIBS=$(PATH_LIB)\toolrt.$(EXT_LIB) $(LIB_OS) $(LIB_C_RTDLL)
!endif

!ifndef TARGET_DEF
TARGET_DEF=$(MAKEDIR)\$(PATH_DEF)\$(TARGET_NAME).def
!endif

!ifndef TARGET_MAP
TARGET_MAP=$(PATH_TARGET)\$(TARGET_NAME).map
!endif

!ifndef TARGET_LNK
TARGET_LNK=$(PATH_TARGET)\$(TARGET_NAME).lnk
!endif

!ifndef TARGET_MODE
TARGET_MODE=EXE
!endif

!ifndef TARGET_EXT
!if "$(TARGET_MODE)" == "CRT" || "$(TARGET_MODE)" == "DLL"
TARGET_EXT=$(EXT_DLL)
!endif
!if "$(TARGET_MODE)" == "EXE"
TARGET_EXT=$(EXT_EXE)
!endif
!if "$(TARGET_MODE)" == "LIB" || "$(TARGET_MODE)" == "PUBLIB"
TARGET_EXT=$(EXT_LIB)
!endif
!ifndef TARGET_EXT
!error Error: TARGET_EXT not set
!endif
!endif

!ifndef TARGET_ILIB
!if "$(TARGET_MODE)" == "CRT" || "$(TARGET_MODE)" == "DLL"
TARGET_ILIB=$(PATH_LIB)\$(TARGET_NAME).$(EXT_ILIB)
!endif
!endif

!if "$(TARGET_MODE)" == "PUBLIB"
TARGET_PUBLIB=$(PATH_LIB)\$(TARGET_NAME).$(TARGET_EXT)
!else
TARGET_PUBLIB=
!endif


!ifndef TARGET_STACKSIZE
TARGET_STACKSIZE=0x10000
!endif

!ifndef MAKEFILE
MAKEFILE = makefile
!endif


# ensure the platform-specific target path exists
PATH_TARGET=$(PATH_OBJ)\$(TARGET_NAME).$(TARGET_EXT)
!if "$(PATH_TARGET)" != ""
! if [$(TOOL_EXISTS) $(PATH_TARGET)] != 0
!  if [$(ECHO) Target path $(CLRFIL)$(PATH_TARGET)$(CLRTXT) does NOT exist. Creating. $(CLRRST)]
!  endif
!  if [$(TOOL_CREATEPATH) $(PATH_TARGET)]
!   error Could not create $(PATH_TARGET)
!  endif
! endif
!endif


# Tell user what we're building.
!if [$(ECHO) Target is $(CLRFIL)$(TARGET)$(CLRRST)]
!endif


# build the target filenames
BUILD_TARGET_DEPEND=$(PATH_TARGET)\.depend


# ----------------------
# common inference rules
# ----------------------

.SUFFIXES:
.SUFFIXES: .dll .exe .$(EXT_OBJ) .c .cpp .asm .res .rc .pre-c .pre-cpp # .h .def


# Assembling assembly source.
.asm{$(PATH_TARGET)}.$(EXT_OBJ):
    @$(ECHO) Assembling $(CLRFIL)$< $(CLRRST)
    $(AS) $(AS_FLAGS) $(AS_OBJ_OUT)$(PATH_TARGET) $<

.asm.$(EXT_OBJ):
    @$(ECHO) Assembling $(CLRFIL)$< $(CLRRST)
    @$(AS) $(AS_FLAGS) $(AS_OBJ_OUT)$(PATH_TARGET)\$(@F) $<


# Compiling C++ source.
.cpp{$(PATH_TARGET)}.$(EXT_OBJ):
    @$(ECHO) C++ Compiler $(CLRFIL)$< $(CLRRST)
    @$(CXX) \
!if "$(TARGET_MODE)" == "EXE" || "$(TARGET_MODE)" == "LIB"  || "$(TARGET_MODE)" == "PUBLIB"
        $(CXX_FLAGS_EXE) \
!endif
!if "$(TARGET_MODE)" == "CRT"
        $(CXX_FLAGS_CRT) \
!endif
!if "$(TARGET_MODE)" == "DLL"
        $(CXX_FLAGS_DLL) \
!endif
        $(CXX_OBJ_OUT)$@ $<

.cpp.$(EXT_OBJ):
    @$(ECHO) C++ Compiler $(CLRFIL)$< $(CLRRST)
    @$(CXX) \
!if "$(TARGET_MODE)" == "EXE" || "$(TARGET_MODE)" == "LIB" || "$(TARGET_MODE)" == "PUBLIB"
        $(CXX_FLAGS_EXE) \
!endif
!if "$(TARGET_MODE)" == "CRT"
        $(CXX_FLAGS_CRT) \
!endif
!if "$(TARGET_MODE)" == "DLL"
        $(CXX_FLAGS_DLL) \
!endif
        $(CXX_OBJ_OUT)$(PATH_TARGET)\$(@F) $<


# Pre-Compiling C++ source.
.cpp.pre-cpp:
    @$(ECHO) C++ Compiler $(CLRFIL)$< $(CLRRST)
    @$(CXX) \
!if "$(TARGET_MODE)" == "EXE" || "$(TARGET_MODE)" == "LIB" || "$(TARGET_MODE)" == "PUBLIB"
        $(CXX_FLAGS_EXE) \
!endif
!if "$(TARGET_MODE)" == "CRT"
        $(CXX_FLAGS_CRT) \
!endif
!if "$(TARGET_MODE)" == "DLL"
        $(CXX_FLAGS_DLL) \
!endif
        $(CXX_PC_2_STDOUT) $< > $@


# Compiling C source.
.c{$(PATH_TARGET)}.$(EXT_OBJ):
    @$(ECHO) C Compiler $(CLRFIL)$< $(CLRRST)
    @$(CC) \
!if "$(TARGET_MODE)" == "EXE" || "$(TARGET_MODE)" == "LIB" || "$(TARGET_MODE)" == "PUBLIB"
        $(CC_FLAGS_EXE) \
!endif
!if "$(TARGET_MODE)" == "CRT"
        $(CC_FLAGS_CRT) \
!endif
!if "$(TARGET_MODE)" == "DLL"
        $(CC_FLAGS_DLL) \
!endif
        $(CC_OBJ_OUT)$@ $<

.c.$(EXT_OBJ):
    @$(ECHO) C Compiler $(CLRFIL)$< $(CLRRST)
    @$(CC) \
!if "$(TARGET_MODE)" == "EXE" || "$(TARGET_MODE)" == "LIB" || "$(TARGET_MODE)" == "PUBLIB"
        $(CC_FLAGS_EXE) \
!endif
!if "$(TARGET_MODE)" == "CRT"
        $(CC_FLAGS_CRT) \
!endif
!if "$(TARGET_MODE)" == "DLL"
        $(CC_FLAGS_DLL) \
!endif
        $(CC_OBJ_OUT)$(PATH_TARGET)\$(@F) $<


# Pre-Compiling C source.
.c.pre-c:
    @$(ECHO) C PreCompiler $(CLRFIL)$< $(CLRRST)
    @$(CC) \
!if "$(TARGET_MODE)" == "EXE" || "$(TARGET_MODE)" == "LIB" || "$(TARGET_MODE)" == "PUBLIB"
        $(CC_FLAGS_EXE) \
!endif
!if "$(TARGET_MODE)" == "CRT"
        $(CC_FLAGS_CRT) \
!endif
!if "$(TARGET_MODE)" == "DLL"
        $(CC_FLAGS_DLL) \
!endif
        $(CC_PC_2_STDOUT) $< > $@


# Compiling resources.
.rc{$(PATH_TARGET)}.res:
    @$(ECHO) RC Compiler $(CLRFIL)$< $(CLRRST)
    @$(RC) $(RC_FLAGS) $< $@

.rc.res:
    @$(ECHO) RC Compiler $(CLRFIL)$< $(CLRRST)
    @$(RC) $(RC_FLAGS) $< $(PATH_TARGET)\$(@F)


#
# establish root dependency
# by removing the extension from the BUILD_TARGET
# and replacing it with .obj
#
all: build


#
# Build the main target.
#
!ifdef SUBDIRS
SUBDIRS_BUILD = subbuild
$(SUBDIRS_BUILD): $(MAKEFILE)
    @$(TOOL_DODIRS) "$(SUBDIRS)" $(TOOL_MAKE) build
!endif
build: $(SUBDIRS_BUILD) $(TARGET) $(TARGET_ILIB) $(TARGET_PUBLIB)
    @$(ECHO) Successfully Built $(CLRFIL)$(TARGET) $(TARGET_ILIB)$(CLRRST)
!ifdef POSTMAKEFILES
    @$(TOOL_DOMAKES) "$(POSTMAKEFILES)" $(TOOL_MAKE) $@
!endif


#
# Make Public libraries.
#
!ifdef SUBDIRS
SUBDIRS_LIB = subdir_lib
$(SUBDIRS_LIB): $(MAKEFILE)
    @$(TOOL_DODIRS) "$(SUBDIRS)" $(TOOL_MAKE) lib
!endif

lib: $(SUBDIRS_LIB) $(TARGET_ILIB) $(TARGET_PUBLIB)
!ifdef POSTMAKEFILES
    @$(TOOL_DOMAKES) "$(POSTMAKEFILES)" $(TOOL_MAKE) $@
!endif


#
# Copies target to main binary directory.
#
install:
!if "$(TARGET_MODE)" == "EXE"
    if exist $(TARGET) $(TOOL_COPY) $(TARGET) $(PATH_BIN)
!endif
!if "$(TARGET_MODE)" == "DLL" || "$(TARGET_MODE)" == "CRT"
    if exist $(TARGET) $(TOOL_COPY) $(TARGET) $(PATH_DLL)
!endif
!if "$(TARGET_MODE)" == "LIB" || "$(TARGET_MODE)" == "PUBLIB"
#    if exist $(TARGET) $(TOOL_COPY) $(TARGET) $(PATH_LIB)
!endif
!if "$(TARGET_DOCS)" != ""
    $(TOOL_COPY) $(TARGET_DOCS) $(PATH_DOC)
!endif
!ifdef SUBDIRS
    @$(TOOL_DODIRS) "$(SUBDIRS)" $(TOOL_MAKE) $@
!endif
!ifdef POSTMAKEFILES
    @$(TOOL_DOMAKES) "$(POSTMAKEFILES)" $(TOOL_MAKE) $@
!endif


#
# Run evt. testcase
#
testcase: install
!if [$(TOOL_EXISTS) testcase] == 0
    @$(TOOL_DODIRS) "testcase" $(TOOL_MAKE) $@
!endif
!if [$(TOOL_EXISTS) testcase.mak] == 0
    @$(TOOL_DOMAKES) "testcase.mak" $(TOOL_MAKE) $@
!endif


#
# Make dependencies.
#
dep:
    @$(ECHO) Building dependencies $(CLRRST)
    $(TOOL_DEP) $(TOOL_DEP_FLAGS) -o$$(PATH_TARGET) -d$(BUILD_TARGET_DEPEND) $(TOOL_DEP_FILES)
!ifdef SUBDIRS
    @$(TOOL_DODIRS) "$(SUBDIRS)" $(TOOL_MAKE) $@
!endif
!ifdef POSTMAKEFILES
    @$(TOOL_DOMAKES) "$(POSTMAKEFILES)" $(TOOL_MAKE) $@
!endif


#
# Clean up output files (not the installed ones).
#
clean:
!if "$(PATH_TARGET)" != ""              # paranoia
!if "$(PATH_TARGET)" == "."
    $(TOOL_RM) \
        $(PATH_TARGET)\*.$(EXT_OBJ) \
        $(PATH_TARGET)\*.$(EXT_ILIB) \
        $(PATH_TARGET)\*.$(EXT_EXE) \
        $(PATH_TARGET)\*.$(EXT_DLL) \
        $(PATH_TARGET)\*.$(EXT_RES) \
        $(PATH_TARGET)\*.$(EXT_LIB) \
        $(PATH_TARGET)\*.map \
        $(PATH_TARGET)\*.lnk \
        $(PATH_TARGET)\*.pre-c \
        $(PATH_TARGET)\*.pre-cpp \
        $(PATH_TARGET)\*.lst
!else
    $(TOOL_RM) \
        $(PATH_TARGET)\*.$(EXT_OBJ) \
        $(PATH_TARGET)\*.$(EXT_ILIB) \
        $(PATH_TARGET)\*.$(EXT_EXE) \
        $(PATH_TARGET)\*.$(EXT_DLL) \
        $(PATH_TARGET)\*.$(EXT_RES) \
        $(PATH_TARGET)\*.$(EXT_LIB) \
        $(PATH_TARGET)\*.map \
        $(PATH_TARGET)\*.lnk \
        $(PATH_TARGET)\*.lst
!endif
!endif
!ifdef SUBDIRS
    @$(TOOL_DODIRS) "$(SUBDIRS)" $(TOOL_MAKE) $@
!endif
!ifdef POSTMAKEFILES
    @$(TOOL_DOMAKES) "$(POSTMAKEFILES)" $(TOOL_MAKE) $@
!endif


#
# EXE and DLL Targets
#
!if "$(TARGET_MODE)" == "EXE" || "$(TARGET_MODE)" == "DLL" || "$(TARGET_MODE)" == "CRT"
$(TARGET): $(TARGET_OBJS) $(TARGET_RES) $(TARGET_DEF) $(TARGET_LNK) $(TARGET_DEPS)
    @$(ECHO) Linking $(TARGET_MODE) $(CLRFIL)$@ $(CLRRST)
!if "$(TARGET_MODE)" == "EXE"
    @$(LINK_CMD_EXE)
!endif
!if "$(TARGET_MODE)" == "DLL" || "$(TARGET_MODE)" == "CRT"
    -4 @$(LINK_CMD_DLL)
!endif
!if "$(TARGET_RES)" != "" && "$(RL)" != ""
    @$(ECHO) Linking Resources $(CLRRST)
    @$(RL) $(RL_FLAGS) $(TARGET_RES) $@
!endif


# call mapsym to generate a sym file for the corresponding executable
!if "$(TARGET_MODE)" == "EXE" || "$(TARGET_MODE)" == "DLL" || "$(TARGET_MODE)" == "CRT" || "$(TARGET_MODE)" == "PDD" || "$(TARGET_MODE)" == "IFS"
  @$(ECHO) Creating debug symbols $(CLRFIL)$@ $(CLRRST)
  $(TOOL_MAPSYM) $*.map
  $(TOOL_COPY) $(@B).sym $*.sym
  $(TOOL_RM) $(@B).sym
!endif

# call LXLITE to compress release build executables
!if "$(TARGET_MODE)" == "EXE" || "$(TARGET_MODE)" == "DLL" || "$(TARGET_MODE)" == "CRT"
!if "$(BUILD_MODE)" == "RELEASE"
  $(TOOL_LXLITE) $@
!endif
!endif


#
# Linker parameter file.
#
$(TARGET_LNK): $(MAKE_INCLUDE_PROCESS) $(MAKE_INCLUDE_SETUP) $(PATH_MAKE)\setup.mak $(MAKEFILE)
    @$(ECHO) Creating Linker Input File $(CLRRST)<<$@
$(LINK_LNK1)
$(LINK_LNK2)
$(LINK_LNK3)
$(LINK_LNK4)
$(LINK_LNK5)
<<KEEP


#
# DLL Import library
#
!ifdef TARGET_ILIB
$(TARGET_ILIB): $(TARGET_DEF)
    @$(ECHO) Creating Import Library $(CLRFIL)$@ $(CLRRST)
    $(IMPLIB) $(IMPLIB_FLAGS) $@ $(TARGET_DEF)
!endif
!endif


#
# Lib Targets.
#
!if "$(TARGET_MODE)" == "LIB" || "$(TARGET_MODE)" == "PUBLIB"
$(TARGET): $(TARGET_OBJS) $(TARGET_LNK) $(TARGET_DEPS)
    @$(ECHO) Creating Library $(CLRFIL)$@ $(CLRRST)
    $(TOOL_RM) $@
    $(AR_CMD)


#
# Lib parameter file.
#
$(TARGET_LNK): $(MAKE_INCLUDE_PROCESS) $(MAKE_INCLUDE_SETUP) $(PATH_MAKE)\setup.mak $(MAKEFILE)
    @$(ECHO) Creating Lib Input File $(CLRRST)<<$@
$(AR_LNK1)
$(AR_LNK2)
$(AR_LNK3)
$(AR_LNK4)
$(AR_LNK5)
<<KEEP
!endif


#
# Copy rule for public libraries.
#
!if "$(TARGET_MODE)" == "PUBLIB"
$(TARGET_PUBLIB): $(TARGET)
    @$(ECHO) Copying $(CLRFIL)$(TARGET)$(CLRTXT) to the Library Directory $(CLRRST)
    @$(TOOL_COPY) $** $@
!endif


#
# read dependency file from current directory
#
!if [$(TOOL_EXISTS) $(BUILD_TARGET_DEPEND)] == 0
! if [$(ECHO) Including dependency $(CLRFIL)$(BUILD_TARGET_DEPEND)$(CLRRST)]
! endif
! include $(BUILD_TARGET_DEPEND)
!else
! if [$(ECHO) $(CLRERR)WARNING: Please make dependencies first. $(BUILD_TARGET_DEPEND) is missing.$(CLRRST)]
! endif
!endif


#
# Force rule.
#
.force:
    $(ECHO) .

