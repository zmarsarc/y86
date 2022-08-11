import unittest
from .parser import TokenStreamParser, Instruction
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


def make_inst_ver_reg(inst: opcode.Instruction, ver: int, reg: reg.Register) -> list[lexer.Token]:
    return [id_token(inst.name), lexer.DOLLAR, lexer.Token(lexer.TokenType.Int, str(ver)),
            lexer.COMMA, lexer.PRESENT, id_token(reg.name)]


def make_d_reg(hex_var: str, r: reg.Register) -> list[lexer.Token]:
    return [lexer.Token(lexer.TokenType.Hex, hex_var), lexer.LPAREN, lexer.PRESENT, id_token(r.name), lexer.RPAREN]


class TestTokenStreamParser(unittest.TestCase):

    def test_nop_halt_ret(self):
        tokens = [id_token(x.name) for x in (opcode.nop, opcode.halt, opcode.ret)]
        p = TokenStreamParser(FakeStreamer(tokens))
        insts = p.parse()
        self.assertEqual(len(insts), 3)
        self.assertEqual(insts[0].operator, opcode.nop)
        self.assertEqual(insts[1].operator, opcode.halt)
        self.assertEqual(insts[2].operator, opcode.ret)

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
            self.assertEqual(insts[i].operator.name, r['name'])
            self.assertEqual(insts[i].src_register.name, r['ra'])

    def test_add_sub_and_xor_rrmovl(self):
        tokens = make_inst_ra_rb(opcode.addl, reg.eax, reg.ecx) +\
                make_inst_ra_rb(opcode.subl, reg.eax, reg.ecx) +\
                make_inst_ra_rb(opcode.andl, reg.eax, reg.ecx) +\
                make_inst_ra_rb(opcode.xorl, reg.eax, reg.ecx) +\
                make_inst_ra_rb(opcode.rrmovl, reg.eax, reg.ecx)
        result = [
            {'name': opcode.addl.name, 'ra': reg.eax.name, 'rb': reg.ecx.name},
            {'name': opcode.subl.name, 'ra': reg.eax.name, 'rb': reg.ecx.name},
            {'name': opcode.andl.name, 'ra': reg.eax.name, 'rb': reg.ecx.name},
            {'name': opcode.xorl.name, 'ra': reg.eax.name, 'rb': reg.ecx.name},
            {'name': opcode.rrmovl.name, 'ra': reg.eax.name, 'rb': reg.ecx.name}
        ]

        p = TokenStreamParser(FakeStreamer(tokens))
        instructions = p.parse()

        self.assertEqual(len(instructions), len(result))

        for i, r in enumerate(result):
            self.assertEqual(instructions[i].operator.name, r['name'])
            self.assertEqual(instructions[i].src_register.name, r['ra'])
            self.assertEqual(instructions[i].dst_register.name, r['rb'])

    def test_jmp_call(self):
        ops = (opcode.jmp, opcode.je, opcode.jg, opcode.jl, opcode.jge, opcode.jle, opcode.jne, opcode.call)
        tokens = []
        for op in ops:
            tokens += make_inst_dst(op, 'main')
        result = [{'name': x.name, 'dst': 'main'} for x in ops]

        instructions = TokenStreamParser(FakeStreamer(tokens)).parse()
        self.assertEqual(len(instructions), len(ops))

        for i, r in enumerate(result):
            self.assertEqual(instructions[i].operator.name, r['name'])
            self.assertEqual(instructions[i].var.value, r['dst'])

    def test_irmovl(self):
        tokens = make_inst_ver_reg(opcode.irmovl, 12345, reg.eax)
        instructions = TokenStreamParser(FakeStreamer(tokens)).parse()

        self.assertEqual(len(instructions), 1)
        inst = instructions[0]
        self.assertEqual(inst.operator.name, 'irmovl')
        self.assertEqual(inst.src_register, reg.no_reg)
        self.assertEqual(inst.dst_register, reg.eax)
        self.assertEqual(inst.var.value, '12345')

    def test_rmmovl(self):
        tokens = [id_token(opcode.rmmovl.name), lexer.PRESENT, id_token(reg.eax.name), lexer.COMMA] +\
                make_d_reg('0x123', reg.ecx)
        instructions = TokenStreamParser(FakeStreamer(tokens)).parse()

        self.assertEqual(len(instructions), 1)
        inst = instructions[0]

        self.assertEqual(inst.operator.name, 'rmmovl')
        self.assertEqual(inst.src_register, reg.eax)
        self.assertEqual(inst.dst_register, reg.ecx)
        self.assertEqual(inst.var.value, '0x123')

    def test_mrmovl(self):
        tokens = [id_token(opcode.mrmovl.name)] + make_d_reg('0x123', reg.ecx) +\
            [lexer.COMMA, lexer.PRESENT, id_token(reg.eax.name)]

        instructions = TokenStreamParser(FakeStreamer(tokens)).parse()

        self.assertEqual(len(instructions), 1)
        inst = instructions[0]

        self.assertEqual(inst.operator.name, 'mrmovl')
        self.assertEqual(inst.src_register, reg.eax)
        self.assertEqual(inst.dst_register, reg.ecx)
        self.assertEqual(inst.var.value, '0x123')


class TestInstruction(unittest.TestCase):

    def test_nop_halt_ret(self):
        operators = (opcode.nop, opcode.halt, opcode.ret)
        result = (b'\x00', b'\x10', b'\x90')
        for i, op in enumerate(operators):
            code = Instruction(op).encode()
            self.assertEqual(code, result[i])

    def test_rrmovl(self):
        op = Instruction(opcode.rrmovl, src_reg=reg.eax, dst_reg=reg.ecx)
        self.assertEqual(op.encode(), b'\x20\x01')

    def test_irmovl(self):
        var = lexer.Token(lexer.TokenType.Hex, "0x12345678")
        op = Instruction(opcode.irmovl, src_reg=reg.no_reg, dst_reg=reg.eax, var=var)
        self.assertEqual(op.encode(), b'\x30\x80\x78\x56\x34\x12')

    def test_rmmovl(self):
        var = lexer.Token(lexer.TokenType.Int, "1")
        op = Instruction(opcode.rmmovl, src_reg=reg.edx, dst_reg=reg.ecx, var=var)
        self.assertEqual(op.encode(), b'\x40\x21\x01\x00\x00\x00')

    def test_mrmovl(self):
        var = lexer.Token(lexer.TokenType.Otc, "0o1")
        op = Instruction(opcode.mrmovl, src_reg=reg.eax, dst_reg=reg.ebx, var=var)
        self.assertEqual(op.encode(), b'\x50\x03\x01\x00\x00\x00')

    def test_opl(self):
        operators = (opcode.andl, opcode.xorl, opcode.addl, opcode.subl)
        result = (b'\x62', b'\x63', b'\x60', b'\x61')
        for i, op in enumerate(operators):
            code = Instruction(op, src_reg=reg.eax, dst_reg=reg.ebx)
            self.assertEqual(code.encode(), result[i] + b'\x03')
