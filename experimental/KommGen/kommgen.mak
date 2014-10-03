#=============================================================
#
#	KOMMGEN.MAK - Makefile for project E:\Dev\Own\KommGen\KommGen.prj
#		Created on 05/06/95 at 15:25
#
#=============================================================

.AUTODEPEND

#=============================================================
#		Translator Definitions
#=============================================================
CC = bcc +KOMMGEN.CFG
TASM = tasm.exe
TLIB = tlib.exe
TLINK = tlink
RC = brcc.exe
RB = rc.exe
LIBPATH = .;E:\BCOS2\LIB
INCLUDEPATH = .;E:\BCOS2\INCLUDE


#=============================================================
#		Implicit Rules
#=============================================================
.c.obj:
  $(CC) -c {$< }

.cpp.obj:
  $(CC) -c {$< }

.asm.obj:
  $(TASM) -Mx $*.asm,$*.obj

.rc.res:
  $(RC) -r $*.rc

#=============================================================
#		List Macros
#=============================================================
LINK_EXCLUDE =  \
 kommgen.res

LINK_INCLUDE =  \
 kommgen.def \
 kommgen.obj \
 kommedit.obj \
 kommcomm.obj

#=============================================================
#		Explicit Rules
#=============================================================
kommgen.exe: kommgen.cfg $(LINK_INCLUDE) $(LINK_EXCLUDE)
  $(TLINK) /B:0x10000 /s /wdef /Toe /aa /L$(LIBPATH) @&&|
E:\BCOS2\LIB\C02.OBJ+
kommgen.obj+
kommedit.obj+
kommcomm.obj
kommgen,kommgen
E:\BCOS2\LIB\C2.LIB+
E:\BCOS2\LIB\OS2.LIB
kommgen.def
|
  rc.exe kommgen.res kommgen.exe

#=============================================================
#		Individual File Dependencies
#=============================================================
kommgen.res: kommgen.cfg KOMMGEN.RC 
	brcc.exe -R -I$(INCLUDEPATH) -FO kommgen.res KOMMGEN.RC

kommgen.obj: kommgen.cfg kommgen.c 

kommedit.obj: kommgen.cfg kommedit.c 

kommcomm.obj: kommgen.cfg kommcomm.c 

#=============================================================
#		Compiler Configuration File
#=============================================================
kommgen.cfg: kommgen.mak
  copy &&|
-RT-
-xd-
-x-
-R
-Oz
-Ob
-Oe
-Oc
-L$(LIBPATH)
-I$(INCLUDEPATH)
-H=E:\Dev\Own\KommGen\KommGen.CSM
-vi-
-d
-k-
-pc
-O
-v
-a
-w-aus
-w-par
| kommgen.cfg


