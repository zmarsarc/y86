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

    private List<Token> tokens = new();
    private FakeSourceCodeBuilder() {}

    public void Label(string name)
    {
        tokens.Add(new IDToken(name));
        tokens.Add(Token.Colon);
    }

    public void Operator(Operator op)
    {
        tokens.Add(new IDToken(op.Name));
    }
}