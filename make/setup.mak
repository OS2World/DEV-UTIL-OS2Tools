# $Id: setup.mak,v 1.18 2001/06/18 16:40:49 bird Exp $

#
# Unix-like tools for OS/2
#
#   Setting up the build environment variables
#
# History:
#
# $Log: setup.mak,v $
# Revision 1.18  2001/06/18 16:40:49  bird
# TARGET_DOCS
#
# Revision 1.17  2001/05/16 01:29:05  bird
# Renamed all compiler setup.* files to setup.*.mk. Reason: Syntax highlighting.
#
# Revision 1.16  2001/05/16 01:21:06  bird
# setup.tools.mak -> setup.tools.mk
#
# Revision 1.15  2001/05/16 01:19:50  bird
# Synced with PHP - lot's of improovments. Added colors.
#
# Revision 1.14  2001/03/22 03:28:31  bird
# Enhanced the build system.
#
# Revision 1.13  2001/03/02 11:10:05  phaller
# .
#
# Revision 1.12  2001/03/01 15:36:32  phaller
# Improved linker support
#
# Revision 1.11  2001/03/01 10:53:33  phaller
# .
#
# Revision 1.10  2001/03/01 10:41:09  phaller
# Now also creating target directories for the tools like /obj/os2debvac308/xxx.exe
#
# Revision 1.9  2001/03/01 10:34:48  phaller
# Added capability of automatic creation of target paths
#
# Revision 1.8  2001/02/28 15:18:01  phaller
# OK, things start to work. The basic skeleton is not in place and scanning the src\* directories or subsequent makefiles to process.
#
# Revision 1.7  2001/02/28 14:44:17  phaller
# Damned stupid make program. No !include with fully-qualified path names, no nested !ifs ... :[
#
# Revision 1.6  2001/02/28 13:51:43  phaller
# Workaround for nmake5 not supporting nested !if statments correctly
#
# Revision 1.5  2001/02/27 15:35:43  phaller
# .
#
# Revision 1.4  2001/02/27 15:04:46  phaller
# Build system skeleton matures ...
#
# Revision 1.3  2001/02/27 14:33:04  phaller
# Added some settings and variables
#
# Revision 1.2  2001/02/27 14:13:52  phaller
# Fixes missing quotes in setup.mak
#
# Revision 1.1  2001/02/27 13:49:47  phaller
# Added first shot of build system
#
#


!ifndef MAKE_SETUP_INCLUDED
MAKE_SETUP_INCLUDED=YES



# -----------
# Directories
# -----------

# Note: external makefiles are supposed to set the
# correct *RELATIVE* path to the projects root.
# PATH_ROOT= .., ..\.., whatever
!ifndef PATH_ROOT
!error fatal error: PATH_ROOT undefined.
!endif

PATH_CURRENT=$(MAKEDIR)
PATH_MAKE=$(PATH_ROOT)\make
PATH_TOOLS=$(PATH_ROOT)\tools\bin
PATH_INCLUDE=$(PATH_ROOT)\include
PATH_BIN=$(PATH_ROOT)\bin
PATH_LIB=$(PATH_ROOT)\lib
PATH_DLL=$(PATH_ROOT)\bin
PATH_DOC=$(PATH_ROOT)\bin

BUILD_TIMESTAMP=$(TIMESTAMP)


# -----------------------------------------------------------------------
# Common variables
# We provide some variables that can be overridden by the specific setups
# -----------------------------------------------------------------------

TOOL_MAKE=$(MAKE) -nologo


# ----------------------
# Build the environments
# ----------------------

# BUILD_PLATFORM: OS2, WIN32, ...
# BUILD_ENV: VAC308, VAC365, VAC4, EMX, MSC6, WATCOM
# BUILD_MODE: RELEASE, PROFILE, DEBUG

!ifndef BUILD_PLATFORM
!error Please set BUILD_PLATFORM to OS2, WIN32, ...
!endif

!ifndef BUILD_MODE
!error Please set BUILD_MODE to RELEASE, PROFILE, or DEBUG
!endif

!ifndef BUILD_ENV
!error Please set BUILD_ENV to VAC308, VAC365, VAC4, EMX, MSVC6, ...
!endif


# ----------------------
# Build the environments
# ----------------------

# Build the appropriate setup.xxx name from the BUILD environment variables.

!if "$(BUILD_PLATFORM)" == "OS2"
NAME_1=os2
!endif
!if "$(BUILD_PLATFORM)" == "WIN32"
NAME_1=win32
!endif
!if "$(NAME_1)" == ""
! error The current setting of BUILD_PLATFORM is unknown or incorrect ($(BUILD_PLATFORM)) (1)
!endif


!if "$(BUILD_MODE)" == "RELEASE"
NAME_2=rel
!endif
!if "$(BUILD_MODE)" == "DEBUG"
NAME_2=deb
!endif
!if "$(BUILD_MODE)" == "PROFILE"
NAME_2=prf
!endif
!if "$(NAME_2)" == ""
! error The current setting of BUILD_MODE is unknown or incorrect ($(BUILD_MODE)) (2)
!endif


!if "$(BUILD_ENV)" == "VAC308"
NAME_3=vac308
!endif
!if "$(BUILD_ENV)" == "VAC365"
NAME_3=vac365
!endif
!if "$(BUILD_ENV)" == "VAC4"
NAME_3=vac4
!endif
!if "$(BUILD_ENV)" == "EMX"
NAME_3=emx
!endif
!if "$(BUILD_ENV)" == "MSVC6"
NAME_3=mscv6
!endif
!if "$(BUILD_ENV)" == "WATCOM"
NAME_3=wat11
!endif
!if "$(NAME_3)" == ""
! error The current setting of BUILD_ENV is unknown or incorrect ($(BUILD_ENV)) (3)
!endif


# build name from the fragments
!include <$(PATH_MAKE)\setup.tools.mk>
MAKE_INCLUDE_SETUP=$(PATH_MAKE)\setup.$(NAME_1)$(NAME_2)$(NAME_3).mk
!if [$(ECHO) Including platform setup file $(CLRFIL)"$(MAKE_INCLUDE_SETUP)"$(CLRRST)]
!endif
!include <$(MAKE_INCLUDE_SETUP)>

# set the path name for platform-specific generated files.
PATH_OBJ=$(PATH_ROOT)\obj\$(NAME_1)$(NAME_2)$(NAME_3)

# set the path name for platform-specific definitino files.
PATH_DEF=$(BUILD_PLATFORM)

# ensure this path exists
!if "$(PATH_OBJ)" != ""
! if [$(TOOL_EXISTS) $(PATH_OBJ)] != 0
!  if [$(ECHO) Target path $(PATH_OBJ) does NOT exist. Creating. $(CLRRST)]
!  endif
!  if [$(TOOL_CREATEPATH) $(PATH_OBJ)]
!   error Could not create $(PATH_OBJ)
!  endif
! endif
!endif


# -----------------------
# Verify the environments
# Note: "foreach" is not used to provide compatibility with older NMAKEs.
# -----------------------

!ifndef ENV_NAME
!error No environment signature has been defined ($(NAME_COMPLETE))
!endif

!ifndef MAKE_INCLUDE_SETUP
!error No setup to include has been determined (MAKE_INCLUDE_SETUP)
!endif

!if "$(ENV_STATUS)" != "OK"
!error Environment $(ENV_NAME) does work yet (ENV_STATUS is not OK).
!endif

!ifndef CC
!error Environment $(ENV_NAME) does not define variable (CC).
!endif

!ifndef CD
!error Environment $(ENV_NAME) does not define variable (CD).
!endif

!ifndef CC_FLAGS_EXE
!error Environment $(ENV_NAME) does not define variable (CC_FLAGS_EXE).
!endif

!ifndef LINK
!error Environment $(ENV_NAME) does not define variable (LINK).
!endif

!ifndef LINK_FLAGS_EXE
!error Environment $(ENV_NAME) does not define variable (LINK_FLAGS_EXE).
!endif


# -----------------------------------------
# Now we setup some final, common variables
# -----------------------------------------
MAKE_INCLUDE_PROCESS=$(PATH_MAKE)\process.mak


!endif MAKE_SETUP_INCLUDED
