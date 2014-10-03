2001/03/01

Project OS/2 Tools
------------------


1. The idea of the project
--------------------------

The idea of the project is to provide a set of tools which are not part
of the shrink wrapped OS/2 package, very much like the GNU Utilities
package available for Unix systems. Some of the provided tools and
utilities are OS/2 specific.

The tools are not only meant as simple OS/2 ports of those available
unix tools, but integrate OS/2 specific extensions and enhancements.

Basically, I started to write those tools back in 1997 (maybe partially
even earlier) and collected them over time. Finally in 2001 the project
makes it to Netlabs and is converted to an Open Source project.


2. Required development tools to build
--------------------------------------

The build system is currently bound to IBM VisualAge C++ 3.08.
Also the resulting runtime library (TOOLRT.DLL) is bound to that
environment since it inculdes some parts of the C-runtime environment
and compatibility has to be ensured with existing executables.

Some backwards compatibility to Borland C++ for OS/2 is still there,
but the build process is not expected to succeed anymore.

Whereas compilers are not considered a problem, someone would have to
volunteer to adapt the build system to such an environment.

Some of the tools actually have a Win32 build with Microsoft Visual
C++. The tools runtime library is supposed to cover up for all platform
differences between OS/2 and Win32 and ... probably other platforms.


2.1 Setting up the build environment
------------------------------------

As the first shot of build system seems to be basically working as of
now, the following options are only a subset of what is intented to be
supported by the full-featured system.

You need IBM VisualAge C++ for OS/2 3.08.
Support for 3.6.5, 4.0, EMX for OS/2, and Microsoft Visual C++ 6 on
Win32-platforms is not functional yet.

The following environment variables have to be set:

SET BUILD_PLATFORM=[OS2,WIN32]
SET BUILD_MODE=[DEBUG,RELEASE,PROFILE]
SET BUILD_ENV=[VAC308,VAC365,VAC4,EMX,MSVC6]

The settings are pretty much self explanatory.

If you've got a lot of ram, a nice SMP-machine, pretty dark sunglasses,
or a souped up mouse-grey Mercedes 190-D, you might want to

SET BUILD_PARALLEL=MASSIVE

This causes the build process to spawn dozens of processes, one for
each build target.


3.0 Project participation
-------------------------

As OS2TOOLS became an open source project, everybody is kindly invited
to contribute his/her share to the progress of the project. May it be
active coding, fixing bugs or just providing detailed information about
examined problems.

In case you are interested in participating, every member of the
project will be happy to give you direction to the right places and to
give a personal introduction to further development of the particular
modules.


4.0 Warranty
------------

EXCEPT AS OTHERWISE RESTRICTED BY LAW, THIS WORK IS PROVIDED
WITHOUT ANY EXPRESSED OR IMPLIED WARRANTIES OF ANY KIND, INCLUDING
BUT NOT LIMITED TO, ANY IMPLIED WARRANTIES OF FITNESS FOR A
PARTICULAR PURPOSE, MERCHANTABILITY OR TITLE.  EXCEPT AS
OTHERWISE PROVIDED BY LAW, NO AUTHOR, COPYRIGHT HOLDER OR
LICENSOR SHALL BE LIABLE TO YOU FOR DAMAGES OF ANY KIND, EVEN IF
THEY HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
