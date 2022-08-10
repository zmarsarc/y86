# -*- coding: utf-8 -*-

from .lexer import Lexer, StringStream, EOF, TokenType
from . import opcode


class Instruction:
    
    def dump(self) -> bytes:
        pass


class Assembler:
    
    def assemble(self, src: str) -> bytes:
        l = Lexer(StringStream(src))
        code = bytes()
        for inst in self._instructions(l):
            code += inst.dump()
        return code

    def _instructions(self, l: Lexer) -> list[Instruction]:
        inst = []
        while l.lookahead() is not EOF:
            inst += self._instruction(l)
        return inst

    def _instruction(self, l: Lexer) -> Instruction:
        if l.lookahead().token_type != TokenType.ID:
            tid = l.consume()
            
            # should be inst
            if tid not in opcode.name_code.keys:
                pass

            inst = opcode.name_code[tid.value.lower()]
            if inst in (opcode.nop, opcode.halt, opcode.ret):
                pass
            