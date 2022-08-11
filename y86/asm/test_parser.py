import unittest
from .parser import TokenStreamParser
from ..machine import opcode, reg
from . import lexer


class FakeStreamer(lexer.TokenStreamer):

    def __init__(self, tokens: list[lexer.Token]):
        self._tokens = tokens
        self._p = -1

    def consume(self) -> lexer.Token:
        tk = self.lookahead()
        self._p += 1
        return tk

    def lookahead(self) -> lexer.Token:
        p = self._p + 1
        if p < len(self._tokens):
            return self._tokens[p]
        return lexer.EOF


def id_token(name: str) -> lexer.Token:
    return lexer.Token(lexer.TokenType.ID, name)


def make_inst_ra_rb(inst: opcode.Instruction, ra: reg.Register, rb: reg.Register) -> list[lexer.Token]:
    return [id_token(inst.name), lexer.PRESENT, id_token(ra.name), lexer.COMMA, lexer.PRESENT, id_token(rb.name)]


def make_inst_ra(inst: opcode.Instruction, ra: reg.Register) -> list[lexer.Token]:
    return [id_token(inst.name), lexer.PRESENT, id_token(ra.name)]


def make_inst_dst(inst: opcode.Instruction, dst: str) -> list[lexer.Token]:
    return [id_token(inst.name), id_token(dst)]


class TestTokenStreamParser(unittest.TestCase):

    def test_nop_halt_ret(self):
        tokens = [id_token(x.name) for x in (opcode.nop, opcode.halt, opcode.ret)]
        p = TokenStreamParser(FakeStreamer(tokens))
        insts = p.parse()
        self.assertEqual(len(insts), 3)
        self.assertEqual(insts[0].name, opcode.nop.name)
        self.assertEqual(insts[1].name, opcode.halt.name)
        self.assertEqual(insts[2].name, opcode.ret.name)

    def test_push_pop(self):
        tokens = make_inst_ra(opcode.pushl, reg.eax) + make_inst_ra(opcode.popl, reg.ecx)
        result = [
            {'name': opcode.pushl.name, 'ra': reg.eax.name},
            {'name': opcode.popl.name, 'ra': reg.ecx.name}
        ]

        p = TokenStreamParser(FakeStreamer(tokens))
        insts = p.parse()
        self.assertEqual(len(insts), 2)

        for i, r in enumerate(result):
            self.assertEqual(insts[i].name, r['name'])
            self.assertEqual(insts[i].ra, r['ra'])

    def test_add_sub_and_xor(self):
        tokens = make_inst_ra_rb(opcode.addl, reg.eax, reg.ecx) +\
                make_inst_ra_rb(opcode.subl, reg.eax, reg.ecx) +\
                make_inst_ra_rb(opcode.andl, reg.eax, reg.ecx) +\
                make_inst_ra_rb(opcode.xorl, reg.eax, reg.ecx)
        result = [
            {'name': opcode.addl.name, 'ra': reg.eax.name, 'rb': reg.ecx.name},
            {'name': opcode.subl.name, 'ra': reg.eax.name, 'rb': reg.ecx.name},
            {'name': opcode.andl.name, 'ra': reg.eax.name, 'rb': reg.ecx.name},
            {'name': opcode.xorl.name, 'ra': reg.eax.name, 'rb': reg.ecx.name},
        ]

        p = TokenStreamParser(FakeStreamer(tokens))
        instructions = p.parse()

        self.assertEqual(len(instructions), len(result))

        for i, r in enumerate(result):
            self.assertEqual(instructions[i].name, r['name'])
            self.assertEqual(instructions[i].ra, r['ra'])
            self.assertEqual(instructions[i].rb, r['rb'])

    def test_jmp_call(self):
        ops = (opcode.jmp, opcode.je, opcode.jg, opcode.jl, opcode.jge, opcode.jle, opcode.jne, opcode.call)
        tokens = []
        for op in ops:
            tokens += make_inst_dst(op, 'main')
        result = [{'name': x.name, 'dst': 'main'} for x in ops]

        instructions = TokenStreamParser(FakeStreamer(tokens)).parse()
        self.assertEqual(len(instructions), len(ops))

        for i, r in enumerate(result):
            self.assertEqual(instructions[i].name, r['name'])
            self.assertEqual(instructions[i].dst, r['dst'])

