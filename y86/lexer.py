from enum import Enum, auto
import string


class TokenType(Enum):
    Invalid = auto()
    ID = auto()
    Int = auto()
    Hex = auto()
    Otc = auto()
    Bin = auto()
    Comma = auto()
    Lparen = auto()
    Rparen = auto()
    Present = auto()
    EOF = auto()


class Token:
    
    def __init__(self, typ: TokenType, value: str) -> None:
        self._type = typ
        self._val = value

    @property
    def token_type(self) -> TokenType:
        return self._type

    @property
    def value(self) -> str:
        return self._val

EOS = '__eos__'
EOF = Token(TokenType.EOF, 'eof')
COMMA = Token(TokenType.Comma, ',')
LPAREN = Token(TokenType.Lparen, '(')
RPAREN = Token(TokenType.Rparen, ')')
PRESENT = Token(TokenType.Present, '%')


class StringStream:

    def __init__(self, src: str) -> None:
        self._src = src
        self._p = -1
        self._is_eos = False

    def lookahead(self, n: int = 1) -> str:
        if n < 1:
            raise IndexError()

        if self._is_eos:
            return EOS
        
        p = self._p + n
        if p < len(self._src):
            return self._src[p]
        return EOS

    def consume(self) -> str:
        p = self._p + 1
        if p < len(self._src):
            self._p = p
            return self._src[p]
        self._is_eos = True
        return EOS


class Lexer:
    
    def __init__(self, stream: StringStream) -> None:
        self._s: StringStream = stream
        self._la: Token = None

    def lookahead(self) -> Token:
        if self._la is None:
            self.consume()
        return self._la

    def consume(self):
        if self._s.lookahead() == EOS:
            self._la = EOF
            return
        self._la = self._next_token()

    def _next_token(self) -> Token:

        while self._s.lookahead() in string.whitespace:
            self._s.consume()
        if self._s.lookahead() == EOS:
            return EOF

        c = self._s.lookahead()

        if c == ',':
            self._s.consume()
            return COMMA
        if c == '(':
            self._s.consume()
            return LPAREN
        if c == ')':
            self._s.consume()
            return RPAREN
        if c == '%':
            self._s.consume()
            return PRESENT
        if c in string.digits:
            return self._match_number()
        if c in string.ascii_letters:
            return Token(TokenType.ID, self._read_until_not_in_set(string.digits + string.ascii_letters))
        else:
            return Token(TokenType.Invalid, c)

    def _match_number(self) -> Token:
        s = ''
        
        if self._s.lookahead() == '0':
            s += self._s.consume()

            if self._s.lookahead() in 'xX' and self._s.lookahead(2) in string.hexdigits:
                s += self._s.consume() + self._read_until_not_in_set(string.hexdigits)
                return Token(TokenType.Hex, s)

            if self._s.lookahead() in 'bB' and self._s.lookahead(2) in '01':
                s += self._s.consume() + self._read_until_not_in_set('01')
                return Token(TokenType.Bin, s)

            if self._s.lookahead() in 'oO' and self._s.lookahead(2) in '01234567':
                s += self._s.consume() + self._read_until_not_in_set('01234567')
                return Token(TokenType.Otc, s)
                
        else:
            return Token(TokenType.Int, self._read_until_not_in_set(string.digits))

    def _read_until_not_in_set(self, set: str) -> str:
        s = ''
        while self._s.lookahead() in set:
            s += self._s.consume()
        return s