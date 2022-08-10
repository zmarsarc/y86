# -*- coding: utf-8 -*-

from .lexer import Token, TokenType


class MismatchError(Exception):

    def __init__(self, expect: TokenType, actual: Token, *args: object) -> None:
        super().__init__(*args)
        self._expect = expect
        self._actual = actual

    def __str__(self) -> str:
        return 'mismatch error, expect type {e}, but actual {a}({val})'.format(
            e=self._expect, a=self._actual.token_type, val=self._actual.value
        )


class UnsupportInstructionError(Exception):

    def __init__(self, name: str, *args: object) -> None:
        super().__init__(*args)
        self._name = name

    def __str__(self) -> str:
        return '\'{0}\' is not a instruction name'.format(self._name)


class UnsupportRegisterError(Exception):

    def __init__(self, name: str, *args: object) -> None:
        super().__init__(*args)
        self._name = name

    def __str__(self) -> str:
        return '\'{0}\' is not a valid register name'.format(self._name)
