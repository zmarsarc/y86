# -*- coding: utf-8 -*-

class Instruction:
    
    def __init__(self, name: str, code: int) -> None:
        self._name = name
        self._code = code

    @property
    def name(self) -> str:
        return self._name

    @property
    def code(self) -> int:
        return self._code


nop = Instruction('nop', 0x00)
halt = Instruction('halt', 0x10)
rrmovl = Instruction('rrmovl', 0x20)
irmovl = Instruction('irmovl', 0x30)
rmmovl = Instruction('rmmovl', 0x40)
mrmovl = Instruction('mrmovl', 0x50)
addl = Instruction('addl', 0x60)
subl = Instruction('subl', 0x61)
andl = Instruction('andl', 0x62)
xorl = Instruction('xorl', 0x63)
jmp = Instruction('jmp', 0x70)
jle = Instruction('jle', 0x71)
jl = Instruction('jl', 0x72)
je = Instruction('je', 0x73)
jne = Instruction('jne', 0x74)
jge = Instruction('jge', 0x75)
jg = Instruction('jg', 0x76)
call = Instruction('call', 0x80)
ret = Instruction('ret', 0x90)
pushl = Instruction('pushl', 0xa0)
popl = Instruction('popl', 0xb0)


name_to_inst: dict[str, Instruction] = {
    nop.name: nop,
    halt.name: halt,
    rrmovl.name: rrmovl,
    irmovl.name: irmovl,
    rmmovl.name: rmmovl,
    mrmovl.name: mrmovl,
    addl.name: addl,
    subl.name: subl,
    andl.name: andl,
    xorl.name: xorl,
    jmp.name: jmp,
    jle.name: jle,
    jl.name: jl,
    je.name: je,
    jne.name: jne,
    jge.name: jge,
    jg.name: jg,
    call.name: call,
    ret.name: ret,
    pushl.name: pushl,
    popl.name: popl
}