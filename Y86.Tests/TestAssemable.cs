using Y86.Assemable;
using Y86.Machine;

namespace Y86.Tests;

class FakeTokenStream: ITokenStream {
    
    private List<Token> Tokens;
    private int Pos = -1;

    public FakeTokenStream(List<Token> tokens) {
        Tokens = tokens;
    }

    public Token Lookahead(int n = 1) {
        int p = Pos + n;
        if (p < Tokens.Count) {
            return Tokens[p];
        }
        return Token.EOF;
    }

    public void Consume(int n = 1) {
        int p = Pos + n;
        if (p < Tokens.Count) {
            Pos = p;
            return;
        }
        Pos = Tokens.Count - 1;
    }
}

class TestTokensMaker {

    private List<Token> tokens;

    public TestTokensMaker() {
        tokens = new List<Token>();
    }

    public static TestTokensMaker Make() {
        return new();
    }

    public TestTokensMaker ID(string name) {
        tokens.Add(new(name, TokenType.ID));
        return this;
    }

    public TestTokensMaker Registers(Register ra, Register rb) {
        // %ra,%rb
        tokens.Add(Token.Present);
        tokens.Add(new(ra.Name, TokenType.ID));
        tokens.Add(Token.Comma);
        tokens.Add(Token.Present);
        tokens.Add(new(rb.Name, TokenType.ID));
        return this;
    }

    public TestTokensMaker Register(Register r) {
        tokens.Add(Token.Present);
        tokens.Add(new(r.Name, TokenType.ID));
        return this;
    }

    public TestTokensMaker Immediate(int num) {
        tokens.Add(Token.Dollar);
        tokens.Add(new(num.ToString(), TokenType.Number, (int)NumberType.Decimal));
        return this;
    }

    public TestTokensMaker BaseOffsetAddress(Register r, int offset) {
        tokens.Add(new(offset.ToString(), TokenType.Number, (int)NumberType.Decimal));
        tokens.Add(Token.LeftParenthesis);
        Register(r);
        tokens.Add(Token.RightParenthesis);
        return this;
    }

    public TestTokensMaker Add(Token tk) {
        tokens.Add(tk);
        return this;
    }

    public List<Token> Tokens() {
        return tokens;
    }

    public ITokenStream TokenStream() {
        return new FakeTokenStream(tokens);
    }
}

public class TestParser
{

    void AssertFragmentHaveOneInstruction(AST frag, Operator op) {
        Assert.Equal(ASTType.Fragment, frag.Type);
        Assert.Equal(1, frag.Children.Count);
        
        AST inst = frag.Children[0];
        Assert.Equal(ASTType.Instruction, inst.Type);
        Assert.Equal((Operator)inst.Value!, op);
    }

    void AssertInstructionHaveRegisters(AST inst, Register ra, Register rb) {
        Assert.Equal(ASTType.Instruction, inst.Type);
        Assert.True(inst.Children.Count >= 2);

        AST regA = inst.Children[0];
        Assert.Equal(ASTType.Register, regA.Type);
        Assert.Equal((Register)regA.Value!, ra);

        AST regB = inst.Children[1];
        Assert.Equal(ASTType.Register, regB.Type);
        Assert.Equal((Register)regB.Value!, rb);
    }

    void AssertIsFragment(AST frag) {
        Assert.Equal(ASTType.Fragment, frag.Type);
    }

    void AssertIsNumber(AST inst, int num) {
        Assert.Equal(ASTType.Integer, inst.Type);
        Assert.Equal((int)inst.Value!, num);
    }

    void AssertIsLabel(AST inst, string id) {
        Assert.Equal(ASTType.Label, inst.Type);
        Assert.Equal(id, (string)inst.Value!);
    }

    [Fact]
    public void TestNop()
    {   
        ITokenStream stream = TestTokensMaker.Make().ID("nop").TokenStream();
        AssertFragmentHaveOneInstruction(new Parser(stream).Parse(), Operator.NOP);
    }

    [Fact]
    public void TestHalt() {
        ITokenStream stream = TestTokensMaker.Make().ID("halt").TokenStream();
        AssertFragmentHaveOneInstruction(new Parser(stream).Parse(), Operator.HALT);
    }

    [Fact]
    public void TestRrmovl() {
        ITokenStream s = TestTokensMaker.Make().ID("rrmovl").Registers(Register.EAX, Register.ECX).TokenStream();
        AST root = new Parser(s).Parse();

        AssertFragmentHaveOneInstruction(root, Operator.RRMOVL);
        AssertInstructionHaveRegisters(root.Children[0], Register.EAX, Register.ECX);
    }

    [Fact]
    public void TestIrmovl() {
        ITokenStream s = TestTokensMaker.Make().ID("irmovl")
            .Immediate(100).Add(Token.Comma).Register(Register.EAX).TokenStream();

        AST root = new Parser(s).Parse();

        AssertFragmentHaveOneInstruction(root, Operator.IRMOVL);
        AssertInstructionHaveRegisters(root.Children[0], Register.NoneRegister, Register.EAX);
        AssertIsNumber(root.Children[0].Children[2], 100);
    }

    [Fact]
    public void TestRmmovl() {
        ITokenStream s = TestTokensMaker.Make().ID("rmmovl").Register(Register.EAX).Add(Token.Comma)
            .BaseOffsetAddress(Register.EBX, 100).TokenStream();
        AST root = new Parser(s).Parse();

        AssertFragmentHaveOneInstruction(root, Operator.RMMOVL);
        AssertInstructionHaveRegisters(root.Children[0], Register.EAX, Register.EBX);
        AssertIsNumber(root.Children[0].Children[2], 100);
    }

    [Fact]
    public void TestMrmovl() {
        ITokenStream s = TestTokensMaker.Make().ID("mrmovl").BaseOffsetAddress(Register.EAX, 100)
            .Add(Token.Comma).Register(Register.EBX).TokenStream();
        AST root = new Parser(s).Parse();

        AssertFragmentHaveOneInstruction(root, Operator.MRMOVL);
        AssertInstructionHaveRegisters(root.Children[0], Register.EBX, Register.EAX);
        AssertIsNumber(root.Children[0].Children[2], 100);
    }

    [Fact]
    public void TestMathOperators() {
        Operator[] ops = {Operator.ADDL, Operator.SUBL, Operator.ANDL, Operator.XORL};
        foreach (var op in ops)
        {
            ITokenStream s = TestTokensMaker.Make().ID(op.Name).Registers(Register.EAX, Register.EBX).TokenStream();
            AST root = new Parser(s).Parse();
            AssertFragmentHaveOneInstruction(root, op);
            AssertInstructionHaveRegisters(root.Children[0], Register.EAX, Register.EBX);
        }
    }

    [Fact]
    public void TestJump() {
        Operator[] ops = {Operator.CALL, Operator.JMP, Operator.JE, Operator.JG, Operator.JGE, Operator.JL, Operator.JLE, Operator.JNE};
        foreach (var op in ops) {
            ITokenStream s = TestTokensMaker.Make().ID(op.Name).ID("main").TokenStream();
            AST root = new Parser(s).Parse();
            AssertFragmentHaveOneInstruction(root, op);
            AssertIsLabel(root.Children[0].Children[0], "main");
        }
    }

    [Fact]
    public void TestRet() {
        ITokenStream stream = TestTokensMaker.Make().ID("ret").TokenStream();
        AssertFragmentHaveOneInstruction(new Parser(stream).Parse(), Operator.RET);
    }

    [Fact]
    public void TestPushPop() {
        Operator[] ops = {Operator.POPL, Operator.PUSHL};
        foreach (var op in ops) {
            ITokenStream s = TestTokensMaker.Make().ID(op.Name).Register(Register.EAX).TokenStream();
            AST root = new Parser(s).Parse();
            AssertFragmentHaveOneInstruction(root, op);
            AssertInstructionHaveRegisters(root.Children[0], Register.EAX, Register.NoneRegister);
        }
    }

    [Fact]
    public void TestLabel() {
        ITokenStream s = TestTokensMaker.Make().ID("main").Add(Token.Colon).TokenStream();
        AST root = new Parser(s).Parse();

        AssertIsFragment(root);
        Assert.Single(root.Children);
        AssertIsLabel(root.Children[0], "main");
    }
}