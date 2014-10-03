# $Id: makefile,v 1.11 2001/03/22 03:28:57 bird Exp $

#
# Unix-like tools for OS/2
#
#   Top of the tree makefile
#
#
#   Usage: nmake ( debug release | all | dep | clean )
#
#            debug: Change to a debug build.
#            release: Change to a release build.
#            all: Build the entire tree.
#            dep: Make dependencies for the entire tree.
#            clean: Bring tree back to a "virgin" state.
#
# History:
#
# $Log: makefile,v $
# Revision 1.11  2001/03/22 03:28:57  bird
# Enhanced the build system.
#
# Revision 1.10  2001/03/02 11:10:05  phaller
# .
#
# Revision 1.9  2001/03/01 15:36:31  phaller
# Improved linker support
#
# Revision 1.8  2001/02/28 15:18:01  phaller
# OK, things start to work. The basic skeleton is not in place and scanning the src\* directories or subsequent makefiles to process.
#
# Revision 1.7  2001/02/28 14:44:17  phaller
# Damned stupid make program. No !include with fully-qualified path names, no nested !ifs ... :[
#
# Revision 1.6  2001/02/28 13:55:08  phaller
# Fixed excess /make path fragment for the setup include file reference
#
# Revision 1.5  2001/02/27 15:35:43  phaller
# .
#
# Revision 1.4  2001/02/27 15:04:46  phaller
# Build system skeleton matures ...
#
# Revision 1.3  2001/02/27 14:32:50  phaller
# Improvements of the structure
#
# Revision 1.2  2001/02/27 14:15:31  phaller
# Fixes missing quotes in setup.mak
#
# Revision 1.1  2001/02/27 13:49:33  phaller
# Added first shot of build system
#
# Revision 1.1  2001/02/15 11:25:38  phaller
# Initial check-in of build system
#
#

# --------------------------
# Get a few common variables
# --------------------------

BUILD_TIMESTAMP=$(TIMESTAMP)

PATH_ROOT=.


# ------------------------------------------------
# Check environment for the appropriate variables.
# Eventually replace them with safe defaults.
# ------------------------------------------------

# BUILD_PLATFORM: OS2, WIN32, ...
# BUILD_ENV: VAC308, VAC365, VAC4, EMXOS2, MSC6
# BUILD_MODE: RELEASE, PROFILE, DEBUG
!INCLUDE <make\setup.mak>


# ---------------------------------------------------
# Build the variables to pass on to the sub-processes
# ---------------------------------------------------

TEMP_DMIS=BUILD_TIMESTAMP="$(BUILD_TIMESTAMP)"


#  [work in progress]
_all: all

dep lib all install clean:
    $(TOOL_DODIRS) "lib src" $(TOOL_MAKE) $@ $(TEMP_DMIS)

