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

    def consume(self):
        p = self._p + 1
        if p < len(self._src):
            self._p = p
            return
        self._is_eos = True


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

        while self._s.lookahead() in ' \r\n\t':
            self._s.consume()
        if self._s.lookahead() == EOS:
            return EOF

        c = self._s.lookahead()

        if c == ',':
            return COMMA
        if c == '(':
            return LPAREN
        if c == ')':
            return RPAREN
        if c == '%':
            return PRESENT
        if c in string.digits:
            return Token(TokenType.Int, self._read_digit_str())
        if c in string.ascii_letters:
            return Token(TokenType.ID, self._read_id_str())
        else:
            return Token(TokenType.Invalid, c)

    def _read_digit_str(self) -> str:
        s = ''
        while self._s.lookahead() in string.digits:
            s += self._s.lookahead()
            self._s.consume()
        return s

    def _read_id_str(self) -> str:
        s = ''
        while self._s.lookahead() in string.digits + string.ascii_letters:
            s += self._s.lookahead()
            self._s.consume()
        return s
