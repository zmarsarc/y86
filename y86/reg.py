# -*- coding: utf-8 -*-

eax: int = 0
ecx: int = 1
edx: int = 2
ebx: int = 3
esp: int = 4
ebp: int = 5
esi: int = 6
edi: int = 7
no_reg: int = 8

reg_name: dict[int, str] = {
    eax: 'eax',
    ecx: 'ecx',
    edx: 'edx',
    ebx: 'ebx',
    esp: 'esp',
    ebp: 'ebp',
    esi: 'esi',
    edi: 'edi',
    no_reg: 'no_reg'
}

name_reg: dict[str, int] = {
    'eax': eax,
    'ecx': ecx,
    'edx': edx,
    'ebx': ebx,
    'esp': esp,
    'ebp': ebp,
    'esi': esi,
    'edi': edi
}