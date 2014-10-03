# makefile
# Created by IBM WorkFrame/2 MakeMake at 4:43:15 on 12 Feb 1996
#
# The actions included in this make file are:
#  Compile::Resource Compiler
#  Compile::C++ Compiler
#  Link::Linker
#  Bind::Resource Bind

.SUFFIXES: .c .obj .rc .res 

.all: \
    .\kommgen.exe

.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s %|dpfF.RES

{E:\dev\own\kommgen}.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s %|dpfF.RES

.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /Tl20 /Ss /Q /Wcmpcndcnscnvdcleffenugengnrgotinilanobsordparporprorearettrdtruunduse /Fi /Si /Ti /Om /Fb /C %s

{E:\dev\own\kommgen}.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /Tl20 /Ss /Q /Wcmpcndcnscnvdcleffenugengnrgotinilanobsordparporprorearettrdtruunduse /Fi /Si /Ti /Om /Fb /C %s

.\kommgen.exe: \
    .\kommgen.obj \
    .\log.obj \
    .\error.obj \
    .\kommcomm.obj \
    .\kommedit.obj \
    .\kommgen.res
    @echo " Link::Linker "
    @echo " Bind::Resource Bind "
    icc.exe @<<
     /B" /de /exepack:2 /pmtype:pm /packd /optfunc /nologo"
     /Fekommgen.exe 
     .\kommgen.obj
     .\log.obj
     .\error.obj
     .\kommcomm.obj
     .\kommedit.obj
<<
    rc.exe .\kommgen.res kommgen.exe

.\kommgen.res: \
    E:\dev\own\kommgen\kommgen.rc \
    {$(INCLUDE)}kommgen.dlg \
    {$(INCLUDE)}KOMMTEMP.ICO \
    {$(INCLUDE)}KOMMGEN.ICO \
    {$(INCLUDE)}kommgen.h

.\kommgen.obj: \
    E:\dev\own\kommgen\kommgen.c \
    {E:\dev\own\kommgen;$(INCLUDE);}KommIncl.h \
    {E:\dev\own\kommgen;$(INCLUDE);}kommgen.h

.\kommedit.obj: \
    E:\dev\own\kommgen\kommedit.c \
    {E:\dev\own\kommgen;$(INCLUDE);}KommIncl.h \
    {E:\dev\own\kommgen;$(INCLUDE);}kommgen.h

.\kommcomm.obj: \
    E:\dev\own\kommgen\kommcomm.c \
    {E:\dev\own\kommgen;$(INCLUDE);}KommIncl.h \
    {E:\dev\own\kommgen;$(INCLUDE);}kommgen.h

.\error.obj: \
    E:\dev\own\kommgen\error.c \
    {E:\dev\own\kommgen;$(INCLUDE);}log.h \
    {E:\dev\own\kommgen;$(INCLUDE);}error.h

.\log.obj: \
    E:\dev\own\kommgen\log.c \
    {E:\dev\own\kommgen;$(INCLUDE);}log.h
