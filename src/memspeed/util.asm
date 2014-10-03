; $Id: util.asm,v 1.1 2002/08/07 16:14:09 phaller Exp $

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


;; 267         BlitMemset(cAccu,                             = eax
;                         &pBuffer8[ iY * m_size.cx + cx],   = edx
;                         cxBytes);                          = ecx
                public BlitMemset
BlitMemset      proc near

                push  edi
                push  esi
                push  ecx

                mov   edi, edx    ; fill everything that fits into 32-bit
                mov   esi, ecx
                shr   ecx, 2
                repnz stosd

                mov   ecx, esi    ; fill last 16-bit value
                test  ecx, 2
                jz _l2
                stosw
                rol eax, 16       ; shift the eax register so a subsequent
                                  ; stosb will use the correct byte

_l2:
                test  ecx, 1      ; fill last 8-bit value
                jz _l3
                stosb

_l3:
                pop ecx
                pop esi
                pop edi

                ret
BlitMemset      endp

CODE32          ENDS
                END
