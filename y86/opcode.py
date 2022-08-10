# -*- coding: utf-8 -*-

nop: int = 0x00
halt: int = 0x10
rrmovl: int = 0x20
irmovl: int = 0x30
rmmovl: int = 0x40
mrmovl: int = 0x50
addl: int = 0x60
subl: int = 0x61
andl: int = 0x62
xorl: int = 0x63
jmp: int = 0x70
jle: int = 0x71
jl: int = 0x72
je: int = 0x73
jne: int = 0x74
jge: int = 0x75
jg: int = 0x76
call: int = 0x80
ret: int = 0x90
pushl: int = 0xa0
popl: int = 0xb0


code_name: dict[int, str] = {
    nop: 'nop',
    halt: 'halt',
    rrmovl: 'rrmovl',
    irmovl: 'irmovl',
    rmmovl: 'rmmovl',
    mrmovl: 'mrmovl',
    addl: 'addl',
    subl: 'subl',
    andl: 'andl',
    xorl: 'xorl',
    jmp: 'jmp',
    jle: 'jle',
    jl: 'jl',
    je: 'je',
    jne: 'jne',
    jge: 'jge',
    jg: 'jg',
    call: 'call',
    ret: 'ret',
    pushl: 'pushl',
    popl: 'popl'
}


name_code: dict[str, int] = {
    'nop': nop,
    'halt': halt,
    'rrmovl': rrmovl,
    'irmovl': irmovl,
    'rmmovl': rmmovl,
    'mrmovl': mrmovl,
    'addl': addl,
    'subl': subl,
    'andl': andl,
    'xorl': xorl,
    'jmp': jmp,
    'jle': jle,
    'jl': jl,
    'je': je,
    'jne': jne,
    'jge': jge,
    'jg': jg,
    'call': call,
    'ret': ret,
    'pushl': pushl,
    'popl': popl
}