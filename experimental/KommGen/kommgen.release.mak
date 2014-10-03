#=============================================================
#
#	KOMMGEN.MAK - Makefile for project E:\dev\kommgen\KommGen.prj
#		Created on 04/11/94 at 09:15
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
  $(TLINK) /s /wdef /Toe /aa /L$(LIBPATH) @&&|
E:\BCOS2\LIB\C02.OBJ+
kommgen.obj+
kommedit.obj+
kommcomm.obj
kommgen,kommgen
E:\BCOS2\LIB\C2MT.LIB+
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
-Ob
-Oc
-Ox
-O1
-O2
-L$(LIBPATH)
-I$(INCLUDEPATH)
-H=E:\dev\kommgen\KommGen.CSM
-vi-
-k-
-O
#-v
-a
-w-aus
-w-par
| kommgen.cfg
