# $Id: Makefile,v 1.4 2002/01/24 18:21:23 bird Exp $
#
# Testcases for BindTools.
#
#

#
# Load the build setup
#
PATH_ROOT=..\..\..
!include $(PATH_ROOT)\make\setup.mak

#
# Main rule
#
testcase: \
    BindSet-1 \
    BindSet-2 \
    BindSet-3 \
    BindSet-4


#
#  The BindSet.exe Testcases.
#
BindSet-1: copy-exefiles BindSet-1-set comp-exefiles
    @$(ECHO) $(CLRFIL)BindSet.Exe $(CLRTXT) - Testcase 1: Completed.$(CLRRST)
BindSet-1-set:
    @$(ECHO) $(CLRFIL)BindSet.Exe $(CLRTXT) - Testcase 1: The -k option.$(CLRRST)
!if "$(BUILD_MODE)" != "RELEASE"    # doesn't support packed executables yet.
    $(PATH_BIN)\BindSet.exe -v0 -kABCDEFGH Bind*.exe
    $(PATH_BIN)\BindSet.exe -v0 -kBINDTOOL Bind*.exe
!endif
    $(PATH_BIN)\BindSet.exe -v0 -kABCDEFGH MF*.exe
    $(PATH_BIN)\BindSet.exe -v0 -kUSERID   MF*.exe


BindSet-2: copy-bindfiles BindSet-2-set comp-bindfiles
    @$(ECHO) $(CLRFIL)BindSet.Exe $(CLRTXT) - Testcase 2: Completed.$(CLRRST)
BindSet-2-set:
    @$(ECHO) $(CLRFIL)BindSet.Exe $(CLRTXT) - Testcase 2: The -c option.$(CLRRST)
    $(PATH_BIN)\BindSet.exe -v0 -cABCDEFGH *.bnd
    $(PATH_BIN)\BindSet.exe -v0 -cBINDTOOL *UDB*.bnd
    $(PATH_BIN)\BindSet.exe -v0 -c- *DB2*.bnd *MVS*.bnd MF*.bnd


BindSet-3: copy-bindfiles BindSet-3-set comp-bindfiles
    @$(ECHO) $(CLRFIL)BindSet.Exe $(CLRTXT) - Testcase 3: Completed.$(CLRRST)
BindSet-3-set:
    @$(ECHO) $(CLRFIL)BindSet.Exe $(CLRTXT) - Testcase 3: The -c, -d, -k, -o, -q and -i options.$(CLRRST)
    $(PATH_BIN)\BindSet.exe -v0 -cCOLLID -dEUR  -kCREATOR -iRS -oOWNER -qQUALIFY *.bnd
    $(PATH_BIN)\BindSet.exe -v0 -cBINDTOOL -dISO -kBINDTOOL -o- -q- -iUR *.bnd
    $(PATH_BIN)\BindSet.exe -v0 -c- *DB2*.bnd *MVS*.bnd


BindSet-4: copy-bindfiles BindSet-4-set comp-bindfiles
    @$(ECHO) $(CLRFIL)BindSet.Exe $(CLRTXT) - Testcase 4: Completed.$(CLRRST)
BindSet-4-set:
    @$(ECHO) $(CLRFIL)BindSet.Exe $(CLRTXT) - Testcase 4: The -c, -d, -k, -o, -q and -i options in testmode and showmode.$(CLRRST)
    $(PATH_BIN)\BindSet.exe -v0 -cCOLLID -dEUR  -kCREATOR -iRS -oOWNER -qQUALIFY -t *.bnd
    $(PATH_BIN)\BindSet.exe -v0 -cCOLLID -dEUR  -kCREATOR -iRS -oOWNER -qQUALIFY -x *.bnd





#
# Copy testfiles - the executable portion.
#
copy-exefiles:
    @$(TOOL_COPY) $(PATH_BIN)\BindChk.exe .
    @$(TOOL_COPY) $(PATH_BIN)\BindIt.exe .
    @$(TOOL_COPY) $(PATH_BIN)\BindPrep.exe .
    @$(TOOL_COPY) $(PATH_BIN)\BindSet.exe .
    @$(TOOL_COPY) .\MFTest1.exe.org MFTest1.exe

#
# Compare exe files.
#
comp-exefiles:
    $(TOOL_CMP) Bind*.exe $(PATH_BIN)
    $(TOOL_CMP) MF*.exe MF*.exe.org


#
# Copy testfiles - the bind portion.
#
copy-bindfiles:
    @$(TOOL_COPY) ..\*.bnd .
    @$(TOOL_COPY) .\MFTest1.bnd.org MFTest1.bnd

#
# Compare bind files.
#
comp-bindfiles:
    $(TOOL_CMP) Q*.bnd ..
    $(TOOL_CMP) MF*.bnd MF*.bnd.org



#
# Compile the cobol hello world
#
MFTest1.exe: MFTest1.sqb
    sqlprep MFTest1.sqb $(DATABASE_71UDB) /BMFTest1.bnd OUTPUT MFTest1.cbl
    cobol MFTest1.cbl, MFTest1.obj NOCOBOLDIR NOSQL;
    cbllink -O$@ MFTest1.obj


#
# Cleanup.
#
clean:
    $(TOOL_RM) *.exe *.bnd *.log *.obj *.int *.cbl
