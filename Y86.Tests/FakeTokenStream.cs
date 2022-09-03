using Y86.Assemable;
using Y86.Assemable.Tokens;
using Y86.Machine;

class FakeTokenStream : ITokenStream
{
    private List<Token> tokens;
    private int pos = -1;

    public FakeTokenStream(List<Token> tokens) => this.tokens = tokens;

    public Token Lookahead(int n = 1)
    {
        int pos = this.pos + n;
        if (pos >= 0 && pos < tokens.Count)
        {
            return tokens[pos];
        }
        return Token.EOS;
    }

    public void Consume(int n = 1)
    {
        pos += n;
    }
}

class FakeSourceCodeBuilder
{
    public static FakeSourceCodeBuilder NewSource()
    {
        return new FakeSourceCodeBuilder();
    }

    public static ITokenStream NewSingleOperator(Operator op)
    {
        return NewSource().Operator(op).TokenStream();
    }

    public static ITokenStream NewIrmovl(int immediate, Register reg)
    {
        return NewSource()
            .Operator(Y86.Machine.Operator.IRMOVL)
            .ImmediateNumber(immediate).Raw(Token.Comma).Register(reg)
            .TokenStream();
    }

    public static ITokenStream NewRmmovl(Register src, Register dst, int offset)
    {
        return NewSource()
            .Operator(Y86.Machine.Operator.RMMOVL)
            .Register(src)
            .Raw(Token.Comma)
            .Raw(new IntegerToken<int>(offset))
            .Raw(Token.LeftParentheses).Register(dst).Raw(Token.RightParentheses)
            .TokenStream();
    }

    public static ITokenStream NewMrmovl(Register src, int offset, Register dst)
    {
        return NewSource()
            .Operator(Y86.Machine.Operator.MRMOVL)
            .Raw(new IntegerToken<int>(offset))
            .Raw(Token.LeftParentheses).Register(src).Raw(Token.RightParentheses)
            .Raw(Token.Comma)
            .Register(dst)
            .TokenStream();
    }

    public static ITokenStream NewOperationWithTwoRegisters(Operator op, Register ra, Register rb)
    {
        return NewSource()
            .Operator(op)
            .Register(ra).Raw(Token.Comma).Register(rb)
            .TokenStream();
    }

    private List<Token> tokens = new();
    private FakeSourceCodeBuilder() {}

    public ITokenStream TokenStream()
    {
        return new FakeTokenStream(tokens);
    }

    public FakeSourceCodeBuilder Label(string name)
    {
        tokens.Add(new IDToken(name));
        tokens.Add(Token.Colon);
        return this;
    }

    public  FakeSourceCodeBuilder Raw(Token tk)
    {
        tokens.Add(tk);
        return this;
    }

    public FakeSourceCodeBuilder Operator(Operator op)
    {
        tokens.Add(new IDToken(op.Name));
        return this;
    }

    public FakeSourceCodeBuilder ImmediateNumber(int n)
    {
        tokens.Add(Token.Dollar);
        tokens.Add(new IntegerToken<int>(n));
        return this;
    }

    public FakeSourceCodeBuilder Register(Register reg)
    {
        tokens.Add(Token.Present);
        tokens.Add(new IDToken(reg.Name));
        return this;
    }

    public FakeSourceCodeBuilder PseudoInstruction(string instName)
    {
        tokens.Add(Token.Dot);
        tokens.Add(new IDToken(instName));
        return this;
    }

    public FakeSourceCodeBuilder Integer<T>(T num)
    {
        tokens.Add(new IntegerToken<T>(num));
        return this;
    }
}