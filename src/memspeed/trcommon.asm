; trcommon.asm - Software Screen to Screen BitBlt Acceleration GRADD filter, release 0.0.6
;   Copyright (c) 2001 Takayuki 'January June' Suwa

  .586
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

; extern ULONG _Optlink IdentifyCpuFeature(VOID);
  public  IdentifyCpuFeature
  org  0
IdentifyCpuFeature  proc  near
  pushfd
  pop  edx
  mov  eax, 1 shl 21
  xor  eax, edx
  push  eax
  popfd
  pushfd
  pop  eax
  xor  eax, edx
  jz  short BLBL_0
  mov  eax, 1
  cpuid
  xchg  eax, edx
BLBL_0:
  ret
IdentifyCpuFeature  endp

CODE32  ends

  end

