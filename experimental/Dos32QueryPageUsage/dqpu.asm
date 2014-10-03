APIRET VR32QueryMemState (ULONG ulParam1,   /* ebp+50 */
                          ULONG ulParam2,   /* ebp+54 */
                          ULONG ulParam3,   /* ebp+58 */
                          ULONG ulParam4)   /* ebp+5c */
{
%fffa12d9 55                 push    ebp 
%fffa12da 8bec               mov     ebp,esp 
%fffa12dc 83ec10             sub     esp,+10 
  APIRET rc; /* ebp-04 */
  ULONG ul2; /* ebp-08 */
  ULONG ul3; /* ebp-0c */
  ULONG ul4; /* ebp-10 */
  
%fffa12df 6a00               push    +00 
%fffa12e1 6a04               push    +04 
%fffa12e3 ff7554             push    dword ptr [ebp+54] 
%fffa12e6 a1b4b2f1ff         mov     eax,dword ptr [fff1b2b4] 
%fffa12eb 8d4df8             lea     ecx,[ebp-08] 
%fffa12ee 03c1               add     eax,ecx 
%fffa12f0 50                 push    eax 
%fffa12f1 e8f5dffcff         call    _TKFuBuff 
  if ((TKFuBuff(&ul2,
                ulParam2,
                4,
                0) == 0)
  
%fffa12f6 0bc0               or      eax,eax 
%fffa12f8 7407               jz      %fffa1301 
%fffa12fa b857000000         mov     eax,00000057 
%fffa12ff c9                 leave 
%fffa1300 c3                 retd 
    return (ERROR_INVALID_PARAMETER);
    
%fffa1301 8b45f8             mov     eax,dword ptr [ebp-08] 
%fffa1304 8945f0             mov     dword ptr [ebp-10],eax 
  ul4 = *ul2;
  
%fffa1307 3d00e0ffff         cmp     eax,ffffe000 
%fffa130c 7607               jbe     %fffa1315 
%fffa130e c745f800e0ffff     mov     dword ptr [ebp-08],ffffe000 
  if (ul4 > 0xFFFFe000)
    ul4 = 0xFFFFe000;
    
%fffa1315 a1b4b2f1ff         mov     eax,dword ptr [fff1b2b4] 
%fffa131a 8d4df4             lea     ecx,[ebp-0c] 
%fffa131d 03c1               add     eax,ecx 
%fffa131f 50                 push    eax 
%fffa1320 a1b4b2f1ff         mov     eax,dword ptr [fff1b2b4] 
%fffa1325 8d4df8             lea     ecx,[ebp-08] 
%fffa1328 03c1               add     eax,ecx 
%fffa132a 50                 push    eax 
%fffa132b ff7550             push    dword ptr [ebp+50] 
%fffa132e e80e060000         call    _VM32ApiQueryMemState 
%fffa1333 8945fc             mov     dword ptr [ebp-04],eax 
  rc = VM32ApiQueryMemState(ulParam1,
                            TKStack1 -> &ul2,
                            TKStack2 -> &ul3);

%fffa1336 0bc0               or      eax,eax 
%fffa1338 751e               jnz     %fffa1358 
  if (rc == NO_ERROR)                         
  {
%fffa133a ff7554             push    dword ptr [ebp+54] 
%fffa133d ff75f8             push    dword ptr [ebp-08] 
%fffa1340 e8c0e3fcff         call    _TKSuDWordNF 
%fffa1345 0bc0               or      eax,eax 
%fffa1347 75b1               jnz     %fffa12fa 
    if (TKSuDWordNF(ul1,
                    ulParam2))
      return (ERROR_INVALID_PARAMETER);
      
%fffa1349 ff7558             push    dword ptr [ebp+58] 
%fffa134c ff75f4             push    dword ptr [ebp-0c] 
%fffa134f e8b1e3fcff         call    _TKSuDWordNF 
%fffa1354 0bc0               or      eax,eax 
%fffa1356 75a2               jnz     %fffa12fa 
    if (TKSuDWordNF(ul3,
                    ulParam2))
      return (ERROR_INVALID_PARAMETER);
  }
%fffa1358 8b45fc             mov     eax,dword ptr [ebp-04] 
%fffa135b c9                 leave 
%fffa135c c3                 retd
  return (rc);
}


APIRET VM32ApiQueryMemState (ULONG ulParam1, /* ebp+08 */
                             ULONG ulParam2, /* ebp+0c */
                             ULONG ulParam3) /* ebp+10 */
{
%fffa1941 55                 push    ebp 
%fffa1942 8bec               mov     ebp,esp 
%fffa1944 83ec08             sub     esp,+08 
  APIRET rc;     /* ebp-04 */

%fffa1947 c745f801000000     mov     dword ptr [ebp-08],00000001 
  ULONG ul2 = 1; /* ebp-08 */

%fffa194e 8b450c             mov     eax,dword ptr [ebp+0c] 
%fffa1951 833800             cmp     dword ptr [eax],+00 
%fffa1954 7509               jnz     %fffa195f 
%fffa1956 b857000000         mov     eax,00000057 
%fffa195b c9                 leave 
%fffa195c c20c00             retd    000c 
  if (ulParam2 == 0)
    return (ERROR_INVALID_PARAMETER);

%fffa195f a1b4b2f1ff         mov     eax,dword ptr [fff1b2b4] 
%fffa1964 8d4df8             lea     ecx,[ebp-08] 
%fffa1967 03c1               add     eax,ecx 
%fffa1969 50                 push    eax 
%fffa196a 6a00               push    +00 
%fffa196c ff750c             push    dword ptr [ebp+0c] 
%fffa196f ff7508             push    dword ptr [ebp+08] 
%fffa1972 e8dc2dfcff         call    _VMQueryMem 
%fffa1977 8945fc             mov     dword ptr [ebp-04],eax 
  rc = VMQueryMem(ulParam1,
                  ulParam2,
                  0,
                  eax); /* *fff1b2b4 + 1 -> thunk stack, &ul2 ? */
                  
%fffa197a 0bc0               or      eax,eax 
%fffa197c 7508               jnz     %fffa1986 
  if (rc == NO_ERROR)
  {
%fffa197e 8b4510             mov     eax,dword ptr [ebp+10] 
%fffa1981 8b4df8             mov     ecx,dword ptr [ebp-08] 
%fffa1984 8908               mov     dword ptr [eax],ecx 
    *ulParam3 = ul2;
  }
%fffa1986 8b45fc             mov     eax,dword ptr [ebp-04] 
%fffa1989 c9                 leave 
%fffa198a c20c00             retd    000c 
  return (rc);
}



OS2KRNL _VMQueryMem: 
%fff64753 55                 push    ebp 
%fff64754 8bec               mov     ebp,esp 
%fff64756 83ec20             sub     esp,+20          ;' ' 
%fff64759 53                 push    ebx 
%fff6475a 57                 push    edi 
%fff6475b 56                 push    esi 
%fff6475c 8b5d08             mov     ebx,dword ptr [ebp+08] 
%fff6475f 8b3db4b2f1ff       mov     edi,dword ptr [fff1b2b4] 
%fff64765 8d45e4             lea     eax,[ebp-1c] 
%fff64768 03f8               add     edi,eax 
%fff6476a 8b4d0c             mov     ecx,dword ptr [ebp+0c] 
%fff6476d 8bd3               mov     edx,ebx 
%fff6476f 81e2ff0f0000       and     edx,00000fff 
%fff64775 0311               add     edx,dword ptr [ecx] 
%fff64777 8d92ff0f0000       lea     edx,[edx+00000fff] 
%fff6477d 81e200f0ffff       and     edx,fffff000 
%fff64783 8911               mov     dword ptr [ecx],edx 
%fff64785 81e300f0ffff       and     ebx,fffff000 
%fff6478b 895de4             mov     dword ptr [ebp-1c],ebx 
%fff6478e 66837d1000         cmp     word ptr [ebp+10],+00 
%fff64793 7411               jz      %fff647a6 
%fff64795 0fb74510           movzx   eax,word ptr [ebp+10] 
%fff64799 c1e004             shl     eax,04 
%fff6479c 030580e0f1ff       add     eax,dword ptr [_pobvmZero] 
%fff647a2 8b00               mov     eax,dword ptr [eax] 
%fff647a4 eb05               jmp     %fff647ab 
%fff647a6 a1a4b2f1ff         mov     eax,dword ptr [fff1b2a4] 
%fff647ab 8945e8             mov     dword ptr [ebp-18],eax 
%fff647ae 391d1409f2ff       cmp     dword ptr [fff20914],ebx 
%fff647b4 7704               ja      %fff647ba 
%fff647b6 2bf6               sub     esi,esi 
%fff647b8 eb14               jmp     %fff647ce 
%fff647ba 8b45e8             mov     eax,dword ptr [ebp-18] 
%fff647bd 395864             cmp     dword ptr [eax+64],ebx 
%fff647c0 7707               ja      %fff647c9 
%fff647c2 be00000004         mov     esi,04000000 
%fff647c7 eb05               jmp     %fff647ce 
%fff647c9 be00000002         mov     esi,02000000 
%fff647ce a1b4b2f1ff         mov     eax,dword ptr [fff1b2b4] 
%fff647d3 8d4de4             lea     ecx,[ebp-1c] 
%fff647d6 03c1               add     eax,ecx 
%fff647d8 50                 push    eax 
%fff647d9 8bc6               mov     eax,esi 
%fff647db 660d0101           or      ax,0101 
%fff647df 50                 push    eax 
%fff647e0 e8f7d8ffff         call    _VMGetOd 
%fff647e5 8945fc             mov     dword ptr [ebp-04],eax 
%fff647e8 0bc0               or      eax,eax 
%fff647ea 0f8520010000       jnz     %fff64910 
%fff647f0 8b45f0             mov     eax,dword ptr [ebp-10] 
%fff647f3 8b4804             mov     ecx,dword ptr [eax+04] 
%fff647f6 8bd1               mov     edx,ecx 
%fff647f8 33cb               xor     ecx,ebx 
%fff647fa 81e100f0ffff       and     ecx,fffff000 
%fff64800 742b               jz      %fff6482d 
%fff64802 8bca               mov     ecx,edx 
%fff64804 81e200f0ffff       and     edx,fffff000 
%fff6480a 8bc3               mov     eax,ebx 
%fff6480c 2500f0ffff         and     eax,fffff000 
%fff64811 3bd0               cmp     edx,eax 
%fff64813 7767               ja      %fff6487c 
%fff64815 c1e90c             shr     ecx,0c 
%fff64818 8b45f0             mov     eax,dword ptr [ebp-10] 
%fff6481b 8b00               mov     eax,dword ptr [eax] 
%fff6481d 25ffff0f00         and     eax,000fffff 
%fff64822 03c8               add     ecx,eax 
%fff64824 8bc3               mov     eax,ebx 
%fff64826 c1e80c             shr     eax,0c 
%fff64829 3bc8               cmp     ecx,eax 
%fff6482b 764f               jbe     %fff6487c 
%fff6482d 391d1009f2ff       cmp     dword ptr [fff20910],ebx 
%fff64833 771d               ja      %fff64852 
%fff64835 391d1409f2ff       cmp     dword ptr [fff20914],ebx 
%fff6483b 7615               jbe     %fff64852 
%fff6483d 8b45f0             mov     eax,dword ptr [ebp-10] 
%fff64840 8b4004             mov     eax,dword ptr [eax+04] 
%fff64843 2506000000         and     eax,00000006 
%fff64848 83f802             cmp     eax,+02 
%fff6484b 742f               jz      %fff6487c 
%fff6484d 83f806             cmp     eax,+06 
%fff64850 742a               jz      %fff6487c 
%fff64852 ff7514             push    dword ptr [ebp+14] 
%fff64855 ff750c             push    dword ptr [ebp+0c] 
%fff64858 53                 push    ebx 
%fff64859 57                 push    edi 
%fff6485a e88db90100         call    _vmQueryObject 
%fff6485f 8945fc             mov     dword ptr [ebp-04],eax 
%fff64862 8b45f4             mov     eax,dword ptr [ebp-0c] 
%fff64865 2b0580e0f1ff       sub     eax,dword ptr [_pobvmZero] 
%fff6486b c1f804             sar     eax,04 
%fff6486e 50                 push    eax 
%fff6486f e8310a0000         call    _VMClearSem 
%fff64874 8b45fc             mov     eax,dword ptr [ebp-04] 
%fff64877 e976010000         jmp     %fff649f2  ; exit proc

%fff6487c 8b45f0             mov     eax,dword ptr [ebp-10] 
%fff6487f f6400402           test    byte ptr [eax+04],02 
%fff64883 7519               jnz     %fff6489e 
%fff64885 6683781000         cmp     word ptr [eax+10],+00 
%fff6488a 7412               jz      %fff6489e 
%fff6488c 8b45f4             mov     eax,dword ptr [ebp-0c] 
%fff6488f 2b0580e0f1ff       sub     eax,dword ptr [_pobvmZero] 
%fff64895 c1f804             sar     eax,04 
%fff64898 50                 push    eax 
%fff64899 e8070a0000         call    _VMClearSem 
%fff6489e 8b45f0             mov     eax,dword ptr [ebp-10] 
%fff648a1 0fb74008           movzx   eax,word ptr [eax+08] 
%fff648a5 6bc016             imul    eax,eax,16 
%fff648a8 0305bc0bf2ff       add     eax,dword ptr [fff20bbc] 
%fff648ae 8945f0             mov     dword ptr [ebp-10],eax 
%fff648b1 391d1009f2ff       cmp     dword ptr [fff20910],ebx 
%fff648b7 777e               ja      %fff64937 
%fff648b9 391d1409f2ff       cmp     dword ptr [fff20914],ebx 
%fff648bf 7676               jbe     %fff64937 
%fff648c1 8b4804             mov     ecx,dword ptr [eax+04] 
%fff648c4 8bd1               mov     edx,ecx 
%fff648c6 33cb               xor     ecx,ebx 
%fff648c8 81e100f0ffff       and     ecx,fffff000 
%fff648ce 742b               jz      %fff648fb 
%fff648d0 8bca               mov     ecx,edx 
%fff648d2 81e200f0ffff       and     edx,fffff000 
%fff648d8 8bc3               mov     eax,ebx 
%fff648da 2500f0ffff         and     eax,fffff000 
%fff648df 3bd0               cmp     edx,eax 
%fff648e1 7754               ja      %fff64937 
%fff648e3 c1e90c             shr     ecx,0c 
%fff648e6 8b45f0             mov     eax,dword ptr [ebp-10] 
%fff648e9 8b00               mov     eax,dword ptr [eax] 
%fff648eb 25ffff0f00         and     eax,000fffff 
%fff648f0 03c8               add     ecx,eax 
%fff648f2 8bc3               mov     eax,ebx 
%fff648f4 c1e80c             shr     eax,0c 
%fff648f7 3bc8               cmp     ecx,eax 
%fff648f9 763c               jbe     %fff64937 
%fff648fb 8b45f0             mov     eax,dword ptr [ebp-10] 
%fff648fe 0fb74008           movzx   eax,word ptr [eax+08] 
%fff64902 6bc016             imul    eax,eax,16 
%fff64905 0305bc0bf2ff       add     eax,dword ptr [fff20bbc] 
%fff6490b 8945f0             mov     dword ptr [ebp-10],eax 
%fff6490e eb27               jmp     %fff64937 
%fff64910 817dfc1a800000     cmp     dword ptr [ebp-04],0000801a 
%fff64917 0f8557ffffff       jnz     %fff64874 
%fff6491d 8b45f0             mov     eax,dword ptr [ebp-10] 
%fff64920 0fb74008           movzx   eax,word ptr [eax+08] 
%fff64924 6bc016             imul    eax,eax,16 
%fff64927 0305bc0bf2ff       add     eax,dword ptr [fff20bbc] 
%fff6492d 8945f0             mov     dword ptr [ebp-10],eax 
%fff64930 c745fc00000000     mov     dword ptr [ebp-04],00000000 

%fff64937 8b45f0             mov     eax,dword ptr [ebp-10] 
%fff6493a 6683781000         cmp     word ptr [eax+10],+00 
%fff6493f 7513               jnz     %fff64954 
%fff64941 8b4804             mov     ecx,dword ptr [eax+04] 
%fff64944 81e106000000       and     ecx,00000006 
%fff6494a 83f902             cmp     ecx,+02 
%fff6494d 7405               jz      %fff64954 
%fff6494f 83f906             cmp     ecx,+06 
%fff64952 75aa               jnz     %fff648fe 
%fff64954 8b45f0             mov     eax,dword ptr [ebp-10] 
%fff64957 8b4004             mov     eax,dword ptr [eax+04] 
%fff6495a 8bc8               mov     ecx,eax 
%fff6495c 2506000000         and     eax,00000006 
%fff64961 83f802             cmp     eax,+02 
%fff64964 742b               jz      %fff64991 
%fff64966 83f806             cmp     eax,+06 
%fff64969 7426               jz      %fff64991 
%fff6496b 81e100f0ffff       and     ecx,fffff000 
%fff64971 894de0             mov     dword ptr [ebp-20],ecx 

%fff64974 8b450c             mov     eax,dword ptr [ebp+0c] 
%fff64977 8b4de0             mov     ecx,dword ptr [ebp-20] 
%fff6497a 2bcb               sub     ecx,ebx 
%fff6497c 8908               mov     dword ptr [eax],ecx 
%fff6497e 8b4514             mov     eax,dword ptr [ebp+14] 
%fff64981 f60001             test    byte ptr [eax],01 
%fff64984 745e               jz      %fff649e4 
%fff64986 c70000000000       mov     dword ptr [eax],00000000 
%fff6498c e9e3feffff         jmp     %fff64874 

%fff64991 8b45e8             mov     eax,dword ptr [ebp-18] 
%fff64994 2b45ec             sub     eax,dword ptr [ebp-14] 
%fff64997 83f8c0             cmp     eax,-40 
%fff6499a 7521               jnz     %fff649bd 
%fff6499c a1f808f2ff         mov     eax,dword ptr [fff208f8] 
%fff649a1 0fb74008           movzx   eax,word ptr [eax+08] 
%fff649a5 6bc016             imul    eax,eax,16 
%fff649a8 0305bc0bf2ff       add     eax,dword ptr [fff20bbc] 
%fff649ae 8945f0             mov     dword ptr [ebp-10],eax 
%fff649b1 c745ecf008f2ff     mov     dword ptr [ebp-14],fff208f0 
%fff649b8 e97affffff         jmp     %fff64937 
%fff649bd 817decf008f2ff     cmp     dword ptr [ebp-14],fff208f0 
%fff649c4 7511               jnz     %fff649d7 
%fff649c6 c745ec5409f2ff     mov     dword ptr [ebp-14],fff20954 
%fff649cd a15c09f2ff         mov     eax,dword ptr [fff2095c] 
%fff649d2 e927ffffff         jmp     %fff648fe 

%fff649d7 a16c09f2ff         mov     eax,dword ptr [fff2096c] 
%fff649dc 8b400c             mov     eax,dword ptr [eax+0c] 
%fff649df 8945e0             mov     dword ptr [ebp-20],eax 
%fff649e2 eb90               jmp     %fff64974 

%fff649e4 8b4514             mov     eax,dword ptr [ebp+14] 
%fff649e7 c70000000040       mov     dword ptr [eax],40000000 
%fff649ed e982feffff         jmp     %fff64874 
%fff649f2 5e                 pop     esi 
%fff649f3 5f                 pop     edi 
%fff649f4 5b                 pop     ebx 
%fff649f5 c9                 leave 
%fff649f6 c21000             retd    0010 

