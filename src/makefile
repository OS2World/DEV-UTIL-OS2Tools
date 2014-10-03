# $Id: makefile,v 1.11 2002/07/29 15:32:03 phaller Exp $

#
# Enter the subdirectories and start the build processes
#
# History:
#
# $Log: makefile,v $
# Revision 1.11  2002/07/29 15:32:03  phaller
# Added Rexx Scripts
#
# Revision 1.10  2002/07/25 00:08:49  phaller
# Enabled the os2info tool
#
# Revision 1.9  2001/03/22 03:26:22  bird
# Updated to match latest build system.
#
# Revision 1.8  2001/03/02 13:02:09  phaller
# .
#
# Revision 1.7  2001/03/02 11:28:30  phaller
# Removed udpbench2
#
# Revision 1.6  2001/03/02 11:10:05  phaller
# .
#
# Revision 1.5  2001/02/28 18:08:29  phaller
# Added all sub-makefiles
#
# Revision 1.4  2001/02/28 15:18:01  phaller
# OK, things start to work. The basic skeleton is not in place and scanning the src\* directories or subsequent makefiles to process.
#
# Revision 1.3  2001/02/28 14:44:17  phaller
# Damned stupid make program. No !include with fully-qualified path names, no nested !ifs ... :[
#
# Revision 1.2  2001/02/28 13:51:43  phaller
# Workaround for nmake5 not supporting nested !if statments correctly
#
# Revision 1.1  2001/02/27 15:05:35  phaller
# Build system skeleton matures ...
#
#
#

#
# Load the build setup
#
PATH_ROOT=..
!INCLUDE <$(PATH_ROOT)\make\setup.mak>


#
# Special setting for massive parallel build
#
!if "$(BUILD_PARALLEL)" == "MASSIVE"
!if [echo Switching to massive parallel build mode!]
!endif
TOOL_MAKE=start /b /win /min $(TOOL_MAKE)
!endif

#
# Build the targets
#

# Directories to skip at build
# Note:
#   robin dto.
#   rstart is broken currently
#   udpbench2\* needs to be put "on root" to comply with build system
#   userXXX is not ready for LanMan 1.x, only NT
NOTTOBEMADE=CVS;robin;rstart;useradd;userinfo;userdel;udpbench2;

_all: all
all lib install clean dep:
    $(TOOL_DOWITHDIRS) -e$(NOTTOBEMADE) -lbld -c $(TOOL_MAKE) $@

