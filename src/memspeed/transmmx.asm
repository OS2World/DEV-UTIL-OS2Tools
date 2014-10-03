; transmmx.asm - Software Screen to Screen BitBlt Acceleration GRADD filter, release 0.0.6
;   Copyright (c) 2001 Takayuki 'January June' Suwa

  .586
  .mmx

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


; static VOID _Optlink TransferScanline_MMX(PBYTE pbDst,   /* eax */
;                                           PBYTE pbSrc,   /* edx */
;                                           ULONG ulBytes  /* ecx */);
  org  37h
TransferScanline_MMX  proc  near
  push  ebx
  push  esi
  push  edi

  lea  ebx, [ecx+edx+7]
  mov  esi, not 7
  and  esi, edx
  sub  ebx, esi

  shr  ebx, 3
  mov  edi, eax ; pbDst
BLBL0:
  cmp  al, [edi+32]
  add  edi, 8
  movq  mm0, [esi]
  add  esi, 8
  dec  ebx
  movq  [edi-8], mm0
  jnz  short BLBL0

  ; Note: non-8-byte-remainder is ignored!

  pop  edi
  pop  esi
  pop  ebx
  ret
TransferScanline_MMX  endp

CODE32  ends

  end

