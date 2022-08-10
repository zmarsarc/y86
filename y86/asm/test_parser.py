import unittest
from .parser import TokenStreamParser
from .lexer import Lexer, StringStream
from ..machine import opcode, reg


class TestTokenStreamParser(unittest.TestCase):
    
    def test_nop_halt_ret(self):
        for op in (opcode.nop, opcode.halt, opcode.ret):
            l = Lexer(StringStream(op.name))
            p = TokenStreamParser(l)
            insts = p.parse()
            self.assertEqual(len(insts), 1)
            
            inst = insts[0]
            self.assertEqual(inst.name, op.name)

    def test_push_pop(self):
        for op in (opcode.pushl, opcode.popl):
            p = TokenStreamParser(Lexer(StringStream(op.name + ' %eax')))
            insts = p.parse()
            self.assertEqual(len(insts), 1)
            inst = insts[0]
            self.assertEqual(inst.name, op.name)
            self.assertEqual(inst.ra, reg.eax.name)

    def test_add_sub_and_xor(self):
        for op in (opcode.addl, opcode.subl, opcode.andl, opcode.xorl):
            p = TokenStreamParser(Lexer(StringStream(op.name + ' %eax,%ecx')))
            instructions = p.parse()

            self.assertEqual(len(instructions), 1)

            inst = instructions[0]

            self.assertEqual(inst.name, op.name)
            self.assertEqual(inst.ra, reg.eax.name)
            self.assertEqual(inst.rb, reg.ecx.name)

