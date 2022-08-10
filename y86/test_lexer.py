import unittest

from .lexer import Lexer, StringStream, EOF, TokenType

class TestLexer(unittest.TestCase):

    def test_empty(self):
        l = Lexer(StringStream(''))
        self.assertEqual(l.lookahead(), EOF)

    def test_all_whitespace(self):
        l = Lexer(StringStream('      \r\n \t    \t\r '))
        self.assertEqual(l.lookahead(), EOF)

    def test_end_with_whitespace(self):
        l = Lexer(StringStream('abc      \r\n'))
        self.assertEqual(l.lookahead().token_type, TokenType.ID)
        l.consume()
        self.assertEqual(l.lookahead(), EOF)

    def test_always_eof(self):
        l = Lexer(StringStream(''))
        self.assertEqual(l.lookahead(), EOF)
        l.consume()
        self.assertEqual(l.lookahead(), EOF)

    def test_parse_id(self):
        l = Lexer(StringStream('abc def'))
        
        tk = l.lookahead()
        self.assertTupleEqual((tk.token_type, tk.value), (TokenType.ID, 'abc'))
        l.consume()

        tk = l.lookahead()
        self.assertTupleEqual((tk.token_type, tk.value), (TokenType.ID, 'def'))
        l.consume()

        self.assertEqual(l.lookahead(), EOF)

        l.consume()
        self.assertEqual(l.lookahead(), EOF)

    def test_eof(self):
        l = Lexer(StringStream(''))
        tk = l.lookahead()
        self.assertTupleEqual((tk.token_type, tk.value), (TokenType.EOF, 'eof'))

    def test_digit(self):
        l = Lexer(StringStream('1235'))
        tk = l.lookahead()
        self.assertTupleEqual((tk.token_type, tk.value), (TokenType.Int, '1235'))
        l.consume()
        self.assertEqual(l.lookahead(), EOF)

    def test_hex(self):
        l = Lexer(StringStream('0xa1234f'))
        tk = l.lookahead()
        self.assertTupleEqual((tk.token_type, tk.value), (TokenType.Hex, '0xa1234f'))

    def test_bin(self):
        l = Lexer(StringStream('0b11001'))
        tk = l.lookahead()
        self.assertTupleEqual((tk.token_type, tk.value), (TokenType.Bin, '0b11001'))

    def tset_oct(self):
        l = Lexer(StringStream('0o1234567'))
        tk = l.lookahead()
        self.assertTupleEqual((tk.token_type, tk.value), (TokenType.Otc, '0o1234567'))