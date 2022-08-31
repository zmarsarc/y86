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

    public FakeSourceCodeBuilder Operator(Operator op)
    {
        tokens.Add(new IDToken(op.Name));
        return this;
    }
}