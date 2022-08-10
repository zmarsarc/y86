# -*- coding: utf-8 -*-

class Register:

    def __init__(self, name: str, code: int) -> None:
        self._name = name
        self._code = code

    @property
    def name(self) -> str:
        return self._name

    @property
    def code(self) -> int:
        return self._code


eax = Register('eax', 0)
ecx = Register('ecx', 1)
edx = Register('edx', 2)
ebx = Register('ebx', 3)
esp = Register('esp', 4)
ebp = Register('ebp', 5)
esi = Register('esi', 6)
edi = Register('edi', 7)
no_reg = Register('no_reg', 8)

name_to_reg: dict[str, Register] = {
    eax.name: eax,
    ecx.name: ecx,
    edx.name: edx,
    ebx.name: ebx,
    esp.name: esp,
    ebp.name: ebp,
    esi.name: esi,
    edi.name: edi,
    no_reg.name: no_reg
}
