; transgen.asm - Software Screen to Screen BitBlt Acceleration GRADD filter, release 0.0.6
;   Copyright (c) 2001 Takayuki 'January June' Suwa

  .386
  .387

CODE32  segment  para use32 public 'CODE'
CODE32  ends
DATA32  segment  para use32 public 'DATA'
DATA32  ends
CONST32_RO  segment  para use32 public 'CONST'
CONST32_RO  ends
BSS32  segment  para use32 public 'BSS'
BSS32  ends
DGROUP  group  BSS32, DATA32
  assume  cs:FLAT, ds:FLAT, ss:FLAT, es:FLAT, fs:nothing, gs:nothing


CODE32  segment


; static VOID _Optlink TransferScanline_Generic(PBYTE pbDst,   /* eax */
;                                               PBYTE pbSrc,   /* edx */
;                                               ULONG ulBytes  /* ecx */);
  org  37h
TransferScanline_Generic  proc  near
  push  ebx
  push  esi
  push  edi

  lea  ebx, [ecx+edx+7]
  mov  esi, not 7
  and  esi, edx
  sub  ebx, esi

  shr  ebx, 4
  mov  edi, eax              ; pbDst
  jnc  short BLBL0
  fild  qword ptr [esi]
  add  esi, 8
  fistp  qword ptr [edi]
  add  edi, 8
  test  ebx, ebx
  jz  short BLBL1
BLBL0:
  cmp  al, [edi+32]
  add  edi, 16
  fild  qword ptr [esi+8]
  fild  qword ptr [esi]
  add  esi, 16
  dec  ebx
  fistp  qword ptr [edi-16]
  fistp  qword ptr [edi-8]
  jnz  short BLBL0
BLBL1:

  ; Note: non-8-byte-remainder is ignored!

  pop  edi
  pop  esi
  pop  ebx
  ret
TransferScanline_Generic  endp

CODE32  ends

  end

