; $Id: util.asm,v 1.1 2002/08/13 06:15:55 phaller Exp $

;/*
; * Execution Trace Profiler
; *
; * Copyright 2001 Patrick Haller <patrick.haller@innotek.de>
; *
; */

.586p
                NAME    profasm

CODE32          SEGMENT DWORD PUBLIC USE32 'CODE'
                ASSUME  DS:FLAT, SS:FLAT

		public ProfileGetTimestamp
ProfileGetTimestamp proc near

                push eax  ; save altered registers
                push edx
                push edi

                rdtsc     ; push the machine timestamp
                          ; (assuming at least Pentium CPU is available)
                mov edi, dword ptr [esp+14h]
                mov dword ptr [edi], eax
                mov edi, dword ptr [esp+10h]
                mov dword ptr [edi], edx

                pop edi
                pop edx
                pop eax

                ret
ProfileGetTimestamp	endp


CODE32          ENDS
                END
