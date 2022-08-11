# -*- coding: utf-8 -*-

from .lexer import TokenStreamer, EOF, TokenType, Token
from .errors import MismatchError, UnsupportInstructionError, UnsupportRegisterError
from ..machine import opcode, reg


class Instruction:
    
    def __init__(self, name: str) -> None:
        self.name = name
        self._ra = None
        self._rb = None
        self._dst = ''

    @property
    def name(self) -> str:
        return self._opcode.name

    @name.setter
    def name(self, name: str):
        self._opcode = opcode.name_to_inst[name]

    @property
    def ra(self) -> str:
        return self._ra.name

    @ra.setter
    def ra(self, name: str):
        self._ra = reg.name_to_reg[name]

    @property
    def rb(self) -> str:
        return self._rb.name

    @rb.setter
    def rb(self, name: str):
        self._rb = reg.name_to_reg[name]

    @property
    def dst(self) -> str:
        return self._dst

    @dst.setter
    def dst(self, d: str):
        self._dst = d


class TokenStreamParser:
    
    def __init__(self, lexer: TokenStreamer) -> None:
        self._lexer = lexer

    def parse(self) -> list[Instruction]:
        return self._instructions()

    def _instructions(self) -> list[Instruction]:
        insts = []
        while self._lexer.lookahead() != EOF:
            insts.append(self._instruction())
        return insts

    def _instruction(self) -> Instruction:
        inst = Instruction(name=self._match_inst_name().value)

        # nop/halt/ret do not need argument
        if inst.name in (opcode.nop.name, opcode.halt.name, opcode.ret.name):
            return inst

        # pushl/popl need a register
        if inst.name in (opcode.pushl.name, opcode.popl.name):
            inst.ra = self._match_reg_name().value
            inst.rb = reg.no_reg.name
            return inst

        # addl/subl/andl/xorl need two registers
        if inst.name in (opcode.andl.name, opcode.subl.name, opcode.addl.name, opcode.xorl.name):
            inst.ra = self._match_reg_name().value
            if self._lexer.lookahead().token_type != TokenType.Comma:
                raise MismatchError(TokenType.Comma, self._lexer.lookahead())
            self._lexer.consume()
            inst.rb = self._match_reg_name().value
            return inst

        # jmp/jl/je/jg/jle/jge/jne/call need a label as destination
        jmp_inst = (opcode.jmp.name, opcode.jl.name, opcode.je.name, opcode.jg.name,
                    opcode.jle.name, opcode.jge.name, opcode.jne.name, opcode.call.name)
        if inst.name in jmp_inst:
            if self._lexer.lookahead().token_type != TokenType.ID:
                raise MismatchError(TokenType.ID, self._lexer.lookahead())
            inst.dst = self._lexer.consume().value
            return inst

    def _match_inst_name(self) -> Token:
        tk = self._lexer.lookahead()
        if tk.token_type != TokenType.ID:
            raise MismatchError(TokenType.ID, tk)

        # find instruction
        if tk.value not in opcode.name_to_inst.keys():
            raise UnsupportInstructionError(tk.value)

        return self._lexer.consume()

    def _match_reg_name(self) -> Token:
        tk = self._lexer.lookahead()

        if tk.token_type != TokenType.Present:
            raise MismatchError(TokenType.Present, tk)

        self._lexer.consume()
        tk = self._lexer.lookahead()

        if tk.token_type != TokenType.ID:
            raise MismatchError(TokenType.ID, tk)

        # find reg
        if tk.value not in reg.name_to_reg.keys():
            raise UnsupportRegisterError(tk.value)

        return self._lexer.consume()
