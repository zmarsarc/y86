from enum import Enum, auto
import string


class TokenType(Enum):
    Invalid = auto()
    ID = auto()
    Const = auto()
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


EOF = Token(TokenType.EOF, 'eof')
COMMA = Token(TokenType.Comma, ',')
LPAREN = Token(TokenType.Lparen, '(')
RPAREN = Token(TokenType.Rparen, ')')
PRESENT = Token(TokenType.Present, '%')
        

class Lexer:
    
    def __init__(self, src: str) -> None:
        self._source = src
        self._cur = 0
        self._la = None
        self._eof = False

    def lookahead(self) -> Token:
        if self._la is None:
            self.consume()
        return self._la

    def consume(self):
        if self._eof:
            self._la = EOF
            return
        self._la = self._next_token()

    def _next_token(self) -> Token:
        try:
            while self._source[self._cur] in ' \r\n\t':
                self._cur += 1
        except IndexError:
            self._eof = True
            return EOF
        
        c = self._source[self._cur]

        if c == ',':
            return COMMA
        if c == '(':
            return LPAREN
        if c == ')':
            return RPAREN
        if c == '%':
            return PRESENT
        if c in string.digits:
            return Token(TokenType.Const, self._read_digit_str())
        if c in string.ascii_letters:
            return Token(TokenType.ID, self._read_id_str())
        else:
            return Token(TokenType.Invalid, c)

    def _read_digit_str(self) -> str:
        s = ''
        try:
            while self._source[self._cur] in string.digits:
                s += self._source[self._cur]
                self._cur += 1
        except IndexError:
            self._eof = True

        if s == '':
            raise 'some error' # TODO

        return s

    def _read_id_str(self) -> str:
        s = ''
        try:
            while self._source[self._cur] in string.digits + string.ascii_letters:
                s += self._source[self._cur]
                self._cur += 1
        except IndexError:
            self._eof = True

        if s == '':
            raise 'some error' # TODO

        return s
