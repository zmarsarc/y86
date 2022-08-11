# -*- coding: utf-8 -*-

from .lexer import TokenStreamer, EOF, TokenType, Token
from .errors import MismatchError, UnsupportInstructionError, UnsupportRegisterError
from ..machine import opcode, reg


class Instruction:
    
    def __init__(self, op: opcode.Instruction,
                 src_reg: reg.Register = None, dst_reg: reg.Register = None,
                 var: Token = None) -> None:
        self._op = op
        self._src_register = src_reg
        self._dst_register = dst_reg
        self._appendix_var = var

    @property
    def operator(self) -> opcode.Instruction:
        return self._op

    @property
    def src_register(self) -> reg.Register:
        return self._src_register

    @property
    def dst_register(self) -> reg.Register:
        return self._dst_register

    @property
    def var(self) -> Token:
        return self._appendix_var


class TokenStreamParser:
    
    def __init__(self, lexer: TokenStreamer) -> None:
        self._lexer = lexer

    def parse(self) -> list[Instruction]:
        insts = []
        while self._lexer.lookahead() != EOF:
            insts.append(self._instruction())
        return insts

    def _instruction(self) -> Instruction:
        op = self._match_operator()

        # nop/halt/ret do not need argument
        if op in (opcode.nop, opcode.halt, opcode.ret):
            return Instruction(op)

        # pushl/popl need a register
        if op in (opcode.pushl, opcode.popl):
            return Instruction(op, src_reg=self._match_register(), dst_reg=reg.no_reg)

        # addl/subl/andl/xorl/rrmovl need two registers
        if op in (opcode.andl, opcode.subl, opcode.addl, opcode.xorl, opcode.rrmovl):
            src, dst = self._match_src_and_dst_register()
            return Instruction(op, src_reg=src, dst_reg=dst)

        # jmp/jl/je/jg/jle/jge/jne/call need a label as destination
        if op in (opcode.jmp, opcode.jl, opcode.je, opcode.jg,
                  opcode.jle, opcode.jge, opcode.jne, opcode.call):
            label = self._match(TokenType.ID)
            return Instruction(op, var=label)

        if op == opcode.irmovl:
            immediate = self._match_immediate()
            self._match(TokenType.Comma)
            dst = self._match_register()
            return Instruction(op, src_reg=reg.no_reg, dst_reg=dst, var=immediate)

        if op == opcode.rmmovl:
            src = self._match_register()
            self._match(TokenType.Comma)
            base, offset = self._match_offset_address()
            return Instruction(op, src_reg=src, dst_reg=base, var=offset)

        if op == opcode.mrmovl:
            base, offset = self._match_offset_address()
            self._match(TokenType.Comma)
            src = self._match_register()
            return Instruction(op, src_reg=src, dst_reg=base, var=offset)

    def _match(self, typ: TokenType) -> Token:
        if self._lexer.lookahead().token_type != typ:
            raise MismatchError(typ, self._lexer.lookahead())
        return self._lexer.consume()

    def _match_number(self):
        if self._lexer.lookahead().token_type not in (TokenType.Int, TokenType.Hex, TokenType.Bin, TokenType.Otc):
            raise MismatchError(TokenType.Number, self._lexer.lookahead())
        return self._lexer.consume()

    def _match_operator(self) -> opcode.Instruction:
        op_id = self._match(TokenType.ID)
        op = opcode.name_to_inst.get(op_id.value)
        if op is None:
            raise UnsupportInstructionError(op_id.value)
        return op

    def _match_register(self) -> reg.Register:
        self._match(TokenType.Present)

        reg_name = self._match(TokenType.ID)
        r = reg.name_to_reg.get(reg_name.value)
        if r is None:
            raise UnsupportRegisterError(reg_name.value)
        return r

    def _match_src_and_dst_register(self) -> tuple[reg.Register, reg.Register]:
        src = self._match_register()
        self._match(TokenType.Comma)
        dst = self._match_register()
        return src, dst

    def _match_offset_address(self) -> tuple[reg.Register, Token]:
        offset = self._match_number()
        self._match(TokenType.Lparen)
        base_reg = self._match_register()
        self._match(TokenType.Rparen)

        return base_reg, offset

    def _match_immediate(self) -> Token:
        self._match(TokenType.Dollar)
        return self._match_number()
