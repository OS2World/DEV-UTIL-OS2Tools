static ULONG fRegistered;    
static ULONG cbRIB;
static PVOID pResInfBuf;
static PVOID MonApp;
static PVOID MonSys;
static ULONG pgListCount;
static PVOID pPTDACur;


APIRET APIENTRY Dos32QueryPageUsage(ULONG ulFunc,
                                    ULONG ulParam2,
                                    ULONG ulParam3)
{
  /* OS2KRNL _VR32QueryPageUsage */

  APIRET rc = NO_ERROR; /* [ebp-08] */

%fffa135d 55                 push    ebp 
%fffa135e 8bec               mov     ebp,esp 
%fffa1360 83ec30             sub     esp,+30          ;'0' 

%fffa1363 c745f800000000     mov     rc,00000000 

%fffa136a 833dd8bdf1ff00     cmp     fRegistered,+00 
%fffa1371 7457               jz      %fffa13ca 
%fffa1373 a1d8bdf1ff         mov     eax,fRegistered 
%fffa1378 8945dc             mov     dword ptr [ebp-24],eax 
%fffa137b a1b4b2f1ff         mov     eax,dword ptr [fff1b2b4] 
%fffa1380 8d4dfc             lea     ecx,[ebp-04] 
%fffa1383 03c1               add     eax,ecx 
%fffa1385 50                 push    eax 
%fffa1386 0fb705debdf1ff     movzx   eax,(USHORT)[ApuOwner + 06] 
%fffa138d 50                 push    eax 
%fffa138e e841cafcff         call    TKiSlotTo_pTCB 
%fffa1393 0bc0               or      eax,eax 
%fffa1395 7529               jnz     %fffa13c0 
%fffa1397 8b45fc             mov     eax,dword ptr [ebp-04] 
%fffa139a 8b4ddc             mov     ecx,dword ptr [ebp-24] 
%fffa139d 394808             cmp     dword ptr [eax+08],ecx 
%fffa13a0 751e               jnz     %fffa13c0 
%fffa13a2 66a1dcbdf1ff       mov     ax,(USHORT)[ApuOwner + 04] 
%fffa13a8 663981ce050000     cmp     word ptr [ecx+000005ce],ax 
%fffa13af 7519               jnz     %fffa13ca 
%fffa13b1 8b0da4b2f1ff       mov     ecx,pPTDACur 
%fffa13b7 663981ce050000     cmp     word ptr [ecx+000005ce],ax 
%fffa13be 740a               jz      %fffa13ca 
%fffa13c0 b805000000         mov     eax,ERROR_ACCESS_DENIED 
%fffa13c5 e978040000         jmp    _exit; /* leave procedure */ 

%fffa13ca 0fb74550           movzx   eax,(USHORT)ulFunc 
%fffa13ce e946040000         jmp     %fffa1819 

/* func 0,2 */
%fffa13d3 66833df8bdf1ff00   cmp     (USHORT)fRegistered,+00 
%fffa13db 756c               jnz     %fffa1449 
%fffa13dd 66c705f8bdf1ff0100 mov     (USHORT)fRegistered,0001 
%fffa13e6 a1a4b2f1ff         mov     eax,pPTDACur 
%fffa13eb a3d8bdf1ff         fRegistered = eax; 
%fffa13f0 668b88ce050000     mov     cx,word ptr [eax+000005ce] 
%fffa13f7 66890ddcbdf1ff     mov     (USHORT)[ApuOwner + 04],cx 
%fffa13fe 8b4020             mov     eax,dword ptr [eax+20] 
%fffa1401 668b4002           mov     ax,word ptr [eax+02] 
%fffa1405 66a3debdf1ff       mov     (USHORT)[ApuOwner + 06],ax 
%fffa140b a1e0e7f1ff         mov     eax,pgListCount 
%fffa1410 8945d8             mov     dword ptr [ebp-28],eax 
%fffa1413 6878bdf1ff         push    &pgAgeRateTable 
%fffa1418 e8889bfbff         call    pgAgeRate 
%fffa141d 800d9ce1f1ff01     or      &(UCHAR)pgIdleList.[+0c],01 
%fffa1424 6a00               push    +00 
%fffa1426 6a00               push    +00 
%fffa1428 6aff               push    -01 
%fffa142a 6890e1f1ff         push    &pgIdleList 
%fffa142f e85451fdff         call    TKSleep 
%fffa1434 8945f8             mov     rc,eax 
%fffa1437 0bc0               or      eax,eax 
%fffa1439 0f8503040000       if NZ goto _exit; 
%fffa143f a1e0e7f1ff         mov     eax,pgListCount 
%fffa1444 3945d8             cmp     dword ptr [ebp-28],eax 
%fffa1447 75c2               jnz     %fffa140b 

%fffa1449 66837d5002         cmp     (USHORT)ulFunc,+02 
%fffa144e 7529               jnz     %fffa1479 
%fffa1450 a1a4b2f1ff         mov     eax,pPTDACur 
%fffa1455 a3e8bdf1ff         mov     MonSys,eax 
%fffa145a 668b88ce050000     mov     cx,word ptr [eax+000005ce] 
%fffa1461 66890decbdf1ff     mov     (USHORT)MonSys.[+04],cx 
%fffa1468 8b4020             mov     eax,dword ptr [eax+20] 
%fffa146b 668b4002           mov     ax,word ptr [eax+02] 
%fffa146f 66a3eebdf1ff       mov     word ptr [(USHORT)MonSys.[+06]],ax 

%fffa1475 2bc0               eax = 0; 
%fffa1477 c9                 leave 
%fffa1478 c3                 retd 
                             return (NO_ERROR);

%fffa1479 837d5400           cmp     ulParam2,+00 
%fffa147d 751b               jnz     %fffa149a 
%fffa147f c705e0bdf1ffffffffff mov     MonApp,ffffffff 
%fffa1489 662bc0             ax = 0; 
%fffa148c 66a3e6bdf1ff       mov     word ptr [fff1bde6],ax 
%fffa1492 66a3e4bdf1ff       mov     word ptr [fff1bde4],ax 
%fffa1498 ebdb               return(NO_ERROR); 

%fffa149a a1b4b2f1ff         mov     eax,dword ptr [fff1b2b4] 
%fffa149f 8d4dd0             lea     ecx,[ebp-30] 
%fffa14a2 03c1               add     eax,ecx 
%fffa14a4 50                 push    eax 
%fffa14a5 6a00               push    +00 
%fffa14a7 688bff0000         push    0000ff8b /* owner=pusetmp */ 
%fffa14ac 6800010000         push    00000100 
%fffa14b1 6a01               push    +01 
%fffa14b3 e8bc47fcff         call    VMAllocKHB 
%fffa14b8 8945f8             mov     rc,eax 
%fffa14bb 0bc0               or      eax,eax 
%fffa14bd 753c               jnz     %fffa14fb 
%fffa14bf 50                 push    eax 
%fffa14c0 6800010000         push    00000100 
%fffa14c5 ff7554             push    ulParam2 
%fffa14c8 ff75d0             push    dword ptr [ebp-30] 
%fffa14cb 50                 push    eax 
%fffa14cc e8e8defcff         call    TKFuBuffz 
%fffa14d1 0bc0               or      eax,eax 
%fffa14d3 7411               jz      %fffa14e6 
%fffa14d5 ff75d0             push    dword ptr [ebp-30] 
%fffa14d8 6a01               push    +01 
%fffa14da e83152fcff         call    VMFreeKHB 

%fffa14df b857000000         mov     eax,ERROR_INVALID_PARAMETER 
%fffa14e4 c9                 leave 
%fffa14e5 c3                 retd 
                             return (ERROR_INVALID_PARAMETER);

%fffa14e6 ff75d0             push    dword ptr [ebp-30] 
%fffa14e9 e8abdefbff         call    qpuFindApp 
%fffa14ee 8945f8             mov     rc,eax 
%fffa14f1 ff75d0             push    dword ptr [ebp-30] 
%fffa14f4 6a01               push    +01 
%fffa14f6 e81552fcff         call    VMFreeKHB 
%fffa14fb 837df800           cmp     rc,+00 
%fffa14ff 740c               jz      %fffa150d 
%fffa1501 2bc0               eax = 0; 
%fffa1503 a3e0bdf1ff         mov     MonApp,eax 
%fffa1508 a3d8bdf1ff         fRegistered = eax; 

%fffa150d 8b45f8             mov     eax,rc 
%fffa1510 c9                 leave 
%fffa1511 c3                 retd  
                             return (rc);


/* func 1,3,5 */
%fffa1512 66833df8bdf1ff00   cmp     (USHORT)fRegistered,+00 
%fffa151a 7507               jnz     %fffa1523 
%fffa151c b87c000000         mov     eax,ERROR_INVALID_LEVEL 
%fffa1521 c9                 leave 
%fffa1522 c3                 retd 
                             return (ERROR_INVALID_LEVEL);

%fffa1523 6a00               push    +00 
%fffa1525 6a04               push    +04 
%fffa1527 ff7558             push    ulParam3 
%fffa152a a1b4b2f1ff         mov     eax,dword ptr [fff1b2b4] 
%fffa152f 8d4de0             lea     ecx,[ebp-20] 
%fffa1532 03c1               add     eax,ecx 
%fffa1534 50                 push    eax 
%fffa1535 e8b1ddfcff         call    TKFuBuff 
%fffa153a 0bc0               or      eax,eax 
%fffa153c 75a1               jnz     %fffa14df 
                             if (rc != NO_ERROR)
                               return (ERROR_INVALID_PARAMETER);

%fffa153e 8b45e0             mov     eax,dword ptr [ebp-20] 
%fffa1541 8945e8             mov     dword ptr [ebp-18],eax 
%fffa1544 8945d4             mov     dword ptr [ebp-2c],eax 
%fffa1547 833df0bdf1ff00     cmp     pRefInfBuf,+00 
%fffa154e 7516               jnz     %fffa1566 
%fffa1550 68f0bdf1ff         push    fff1bdf0 
%fffa1555 6a00               push    +00 
%fffa1557 688aff0000         push    0000ff8a  /* owner=puse */ 
%fffa155c 50                 push    eax 
%fffa155d 6a01               push    +01 
%fffa155f e81047fcff         call    VMAllocKHB 
%fffa1564 eb1a               jmp     %fffa1580 
%fffa1566 8b45e0             mov     eax,dword ptr [ebp-20] 
%fffa1569 3905f4bdf1ff       cmp     cbRIB,eax 
%fffa156f 7312               jnc     %fffa1583 
%fffa1571 6a00               push    +00 
%fffa1573 50                 push    eax 
%fffa1574 68f0bdf1ff         push    fff1bdf0 
%fffa1579 6a01               push    +01 
%fffa157b e89d4efcff         call    VMReallocKHB 
%fffa1580 8945f8             mov     rc,eax 
%fffa1583 837df800           cmp     rc,+00 
%fffa1587 7419               jz      %fffa15a2 
%fffa1589 c705f0bdf1ff00000000 mov     pRefInfBuf,00000000 
%fffa1593 c705f4bdf1ff00000000 mov     cbRIB,00000000 
%fffa159d e96bffffff         jmp     %fffa150d 
%fffa15a2 a1f4bdf1ff         mov     eax,cbRIB 
%fffa15a7 3b45e0             cmp     eax,dword ptr [ebp-20] 
%fffa15aa 7303               jnc     %fffa15af 
%fffa15ac 8b45e0             mov     eax,dword ptr [ebp-20] 
%fffa15af a3f4bdf1ff         mov     cbRIB,eax 
%fffa15b4 a1f0bdf1ff         mov     eax,pRefInfBuf 
%fffa15b9 8945e4             mov     dword ptr [ebp-1c],eax 
%fffa15bc 66837d5001         cmp     (USHORT)ulFunc,+01 
%fffa15c1 7507               jnz     %fffa15ca 
%fffa15c3 a1e0bdf1ff         mov     eax,MonApp 
%fffa15c8 eb05               jmp     %fffa15cf 
%fffa15ca a1e8bdf1ff         mov     eax,MonSys 
%fffa15cf 8945dc             mov     dword ptr [ebp-24],eax 
%fffa15d2 0bc0               or      eax,eax 
%fffa15d4 7507               jnz     %fffa15dd 

%fffa15d6 b808000000         mov     eax,ERROR_NOT_ENOUGH_MEMORY 
%fffa15db c9                 leave 
%fffa15dc c3                 retd 
                             return (ERROR_NOT_ENOUGH_MEMORY);

%fffa15dd a1b4b2f1ff         mov     eax,dword ptr [fff1b2b4] 
%fffa15e2 8d4de4             lea     ecx,[ebp-1c] 
%fffa15e5 03c1               add     eax,ecx 
%fffa15e7 8945ec             mov     dword ptr [ebp-14],eax 
%fffa15ea a1b4b2f1ff         mov     eax,dword ptr [fff1b2b4] 
%fffa15ef 8d4de0             lea     ecx,[ebp-20] 
%fffa15f2 03c1               add     eax,ecx 
%fffa15f4 8945f0             mov     dword ptr [ebp-10],eax 
%fffa15f7 66837d5003         cmp     (USHORT)ulFunc,+03 
%fffa15fc 7528               jnz     %fffa1626 
%fffa15fe c745f400000600     mov     dword ptr [ebp-0c],00060000 
%fffa1605 a1b4b2f1ff         mov     eax,dword ptr [fff1b2b4] 
%fffa160a 8d4dec             lea     ecx,[ebp-14] 
%fffa160d 03c1               add     eax,ecx 
%fffa160f 50                 push    eax 
%fffa1610 a1e8bdf1ff         mov     eax,MonSys 
%fffa1615 8945dc             mov     dword ptr [ebp-24],eax 
%fffa1618 50                 push    eax 
%fffa1619 e8c0defbff         call    qpuGetPageInfo 
%fffa161e 8945f8             mov     rc,eax 
%fffa1621 e938010000         jmp     %fffa175e 
%fffa1626 a1e0bdf1ff         mov     eax,MonApp 
%fffa162b 8945dc             mov     dword ptr [ebp-24],eax 
%fffa162e c745f400000000     mov     dword ptr [ebp-0c],00000000 
%fffa1635 66837d5005         cmp     (USHORT)ulFunc,+05 
%fffa163a 7507               jnz     %fffa1643 
%fffa163c c745f400800100     mov     dword ptr [ebp-0c],00018000 
%fffa1643 837ddcff           cmp     dword ptr [ebp-24],-01 
%fffa1647 751e               jnz     %fffa1667 
%fffa1649 a1b4b2f1ff         mov     eax,dword ptr [fff1b2b4] 
%fffa164e 8d4dec             lea     ecx,[ebp-14] 
%fffa1651 03c1               add     eax,ecx 
%fffa1653 50                 push    eax 
%fffa1654 68def4f5ff         push    &qpuGetPageInfo
%fffa1659 6a00               push    +00 
%fffa165b 6804030000         push    00000304 
%fffa1660 e8efcafcff         call    TKScanTasks 
%fffa1665 ebb7               jmp     %fffa161e 
%fffa1667 a1b4b2f1ff         mov     eax,dword ptr [fff1b2b4] 
%fffa166c 8d4dfc             lea     ecx,[ebp-04] 
%fffa166f 03c1               add     eax,ecx 
%fffa1671 50                 push    eax 
%fffa1672 0fb705e6bdf1ff     movzx   eax,word ptr [fff1bde6] 
%fffa1679 50                 push    eax 
%fffa167a e855c7fcff         call    TKiSlotTo_pTCB 
%fffa167f 0bc0               or      eax,eax 
%fffa1681 7521               jnz     %fffa16a4 
%fffa1683 8b45fc             mov     eax,dword ptr [ebp-04] 
%fffa1686 8b4008             mov     eax,dword ptr [eax+08] 
%fffa1689 8945dc             mov     dword ptr [ebp-24],eax 
%fffa168c 3b05e0bdf1ff       cmp     eax,MonApp 
%fffa1692 7510               jnz     %fffa16a4 
%fffa1694 668b0de4bdf1ff     mov     cx,word ptr [fff1bde4] 
%fffa169b 663988ce050000     cmp     word ptr [eax+000005ce],cx 
%fffa16a2 7418               jz      %fffa16bc 
%fffa16a4 c705e0bdf1ff00000000 mov     MonApp,00000000 
%fffa16ae 66c705e6bdf1ff0000 mov     word ptr [fff1bde6],0000 
%fffa16b7 e960feffff         jmp     %fffa151c 
%fffa16bc a1b4b2f1ff         mov     eax,dword ptr [fff1b2b4] 
%fffa16c1 8d4ddc             lea     ecx,[ebp-24] 
%fffa16c4 03c1               add     eax,ecx 
%fffa16c6 50                 push    eax 
%fffa16c7 68c1000000         push    000000c1 
%fffa16cc 8b45dc             mov     eax,dword ptr [ebp-24] 
%fffa16cf 668b80ac050000     mov     ax,word ptr [eax+000005ac] 
%fffa16d6 50                 push    eax 
%fffa16d7 e87435fcff         call    VMGetSem 
%fffa16dc 8945f8             mov     rc,eax 
%fffa16df 833de0bdf1ff00     cmp     MonApp,+00 
%fffa16e6 0f84eafeffff       jz      %fffa15d6 
%fffa16ec a1b4b2f1ff         mov     eax,dword ptr [fff1b2b4] 
%fffa16f1 8d4dfc             lea     ecx,[ebp-04] 
%fffa16f4 03c1               add     eax,ecx 
%fffa16f6 50                 push    eax 
%fffa16f7 0fb705e6bdf1ff     movzx   eax,word ptr [fff1bde6] 
%fffa16fe 50                 push    eax 
%fffa16ff e8d0c6fcff         call    TKiSlotTo_pTCB 
%fffa1704 0bc0               or      eax,eax 
%fffa1706 7521               jnz     %fffa1729 
%fffa1708 8b45fc             mov     eax,dword ptr [ebp-04] 
%fffa170b 8b4008             mov     eax,dword ptr [eax+08] 
%fffa170e 8945dc             mov     dword ptr [ebp-24],eax 
%fffa1711 3b05e0bdf1ff       cmp     eax,MonApp 
%fffa1717 7510               jnz     %fffa1729 
%fffa1719 668b0de4bdf1ff     mov     cx,word ptr [fff1bde4] 
%fffa1720 663988ce050000     cmp     word ptr [eax+000005ce],cx 
%fffa1727 740f               jz      %fffa1738 
%fffa1729 c705e0bdf1ff00000000 mov     MonApp,00000000 
%fffa1733 e9e4fdffff         jmp     %fffa151c 
%fffa1738 a1b4b2f1ff         mov     eax,dword ptr [fff1b2b4] 
%fffa173d 8d4dec             lea     ecx,[ebp-14] 
%fffa1740 03c1               add     eax,ecx 
%fffa1742 50                 push    eax 
%fffa1743 ff75dc             push    dword ptr [ebp-24] 
%fffa1746 e893ddfbff         call    qpuGetPageInfo 
%fffa174b 8945f8             mov     rc,eax 
%fffa174e 8b45dc             mov     eax,dword ptr [ebp-24] 
%fffa1751 668b80ac050000     mov     ax,word ptr [eax+000005ac] 
%fffa1758 50                 push    eax 
%fffa1759 e8473bfcff         call    VMClearSem 
%fffa175e 837df800           cmp     rc,+00 
%fffa1762 0f85a5fdffff       jnz     %fffa150d 

%fffa1768 ff7558             push    ulParam3 
%fffa176b 8b45e0             mov     eax,dword ptr [ebp-20] 
%fffa176e 2945d4             sub     dword ptr [ebp-2c],eax 
%fffa1771 ff75d4             push    dword ptr [ebp-2c] 
%fffa1774 e88cdffcff         call    TKSuDWordNF 
%fffa1779 0bc0               or      eax,eax 
%fffa177b 0f855efdffff       jnz     %fffa14df 
%fffa1781 50                 push    eax 
%fffa1782 ff75d4             push    dword ptr [ebp-2c] 
%fffa1785 ff35f0bdf1ff       push    pRefInfBuf 
%fffa178b ff7554             push    ulParam2 
%fffa178e e822dffcff         call    TKSuBuff 
%fffa1793 0bc0               or      eax,eax 
%fffa1795 0f8472fdffff       jz      %fffa150d 
%fffa179b ff35f0bdf1ff       push    pRefInfBuf 
%fffa17a1 6a01               push    +01 
%fffa17a3 e8684ffcff         call    VMFreeKHB 
%fffa17a8 c705f0bdf1ff00000000 mov     pRefInfBuf,00000000 
%fffa17b2 c705f4bdf1ff00000000 mov     cbRIB,00000000 
%fffa17bc e91efdffff         jmp     %fffa14df 

/* func 4 */
%fffa17c1 662bc0             ax = 0; 
%fffa17c4 66a3f8bdf1ff       mov     (USHORT)fRegistered,ax 
%fffa17ca 66a3debdf1ff       mov     (USHORT)[ApuOwner + 06],ax 
%fffa17d0 66a3dcbdf1ff       mov     (USHORT)[ApuOwner + 04],ax 
%fffa17d6 2bc0               eax = 0; 
%fffa17d8 a3d8bdf1ff         fRegistered = eax; 
%fffa17dd a3e0bdf1ff         mov     MonApp,eax 
%fffa17e2 a3e8bdf1ff         mov     MonSys,eax 
%fffa17e7 3905f0bdf1ff       cmp     pRefInfBuf,eax 
%fffa17ed 0f8482fcffff       jz      %fffa1475 
%fffa17f3 ff35f0bdf1ff       push    pRefInfBuf 
%fffa17f9 6a01               push    +01 
%fffa17fb e8104ffcff         call    VMFreeKHB 
%fffa1800 c705f0bdf1ff00000000 mov     pRefInfBuf,00000000 
%fffa180a c705f4bdf1ff00000000 mov     cbRIB,00000000 
%fffa1814 e95cfcffff         return(NO_ERROR); 
  { /* func 4 */
    (USHORT)fRegistered = 0;
    ApuOwner.[+06] = 0;
    ApuOwner.[+04] = 0;    
    MonApp = NULL;
    MonSys = NULL;
    if (pRefInfBuf)
    {
      VMFreeKHB(pRefInfBuf);
      pRefInfBuf = NULL;
      cbRIB = 0;
    }
    return (NO_ERROR);
  }


%fffa1819 83f805             cmp     eax,+05 
%fffa181c 0f87fafcffff       ja      %fffa151c 
%fffa1822 2eff24852a18faff   jmp     dword ptr cs:[eax*4+fffa182a] 

JumpTable:
%fffa182a  fffa13d3 /* 0 */
           fffa1512 /* 1 */ 
           fffa13d3 /* 2 */
           fffa1512 /* 3 */  
%fffa183a  fffa17c1 /* 4 unregister */
           fffa1512 /* 5 */

_exit:
%fffa1842 c9                 leave 
%fffa1843 c3                 retd 

