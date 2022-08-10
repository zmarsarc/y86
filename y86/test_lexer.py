import unittest

from . import lexer

class TestLexer(unittest.TestCase):

    def test_empty(self):
        l = lexer.Lexer('')
        self.assertEqual(l.lookahead(), lexer.EOF)

    def test_all_whitespace(self):
        l = lexer.Lexer('      \r\n \t    \t\r ')
        self.assertEqual(l.lookahead(), lexer.EOF)

    def test_end_with_whitespace(self):
        l = lexer.Lexer('abc      \r\n')
        self.assertEqual(l.lookahead().token_type, lexer.TokenType.ID)
        l.consume()
        self.assertEqual(l.lookahead(), lexer.EOF)

    def test_always_eof(self):
        l = lexer.Lexer('')
        self.assertEqual(l.lookahead(), lexer.EOF)
        l.consume()
        self.assertEqual(l.lookahead(), lexer.EOF)

    def test_parse_id(self):
        l = lexer.Lexer('abc def')
        
        tk = l.lookahead()
        self.assertTupleEqual((tk.token_type, tk.value), (lexer.TokenType.ID, 'abc'))
        l.consume()

        tk = l.lookahead()
        self.assertTupleEqual((tk.token_type, tk.value), (lexer.TokenType.ID, 'def'))
        l.consume()

        self.assertEqual(l.lookahead(), lexer.EOF)

        l.consume()
        self.assertEqual(l.lookahead(), lexer.EOF)

    def test_eof(self):
        l = lexer.Lexer('')
        tk = l.lookahead()
        self.assertTupleEqual((tk.token_type, tk.value), (lexer.TokenType.EOF, 'eof'))

    def test_digit(self):
        l = lexer.Lexer('1235')
        tk = l.lookahead()
        self.assertTupleEqual((tk.token_type, tk.value), (lexer.TokenType.Const, '1235'))
        l.consume()
        self.assertEqual(l.lookahead(), lexer.EOF)