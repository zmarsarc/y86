using Y86.Assemable;
using Y86.Machine;
using Y86.Assemable.Tokens;


namespace Y86.Tests;

public class TestParse
{
    [Fact]
    public void ParseShouldReturnAbstractInstructionList()
    {
        // Given
        ITokenStream fakeStream = FakeSourceCodeBuilder.NewSingleOperator(Operator.NOP);

        // When
        AbstractInstructionsList result = Parser.Parse(fakeStream);

        // Then
        Assert.NotEmpty(result);
    }

    [Fact]
    public void ParseShouldStopWhenEndOfStream()
    {
        // Given
        ITokenStream fakeStream = FakeSourceCodeBuilder.NewSingleOperator(Operator.NOP);

        // When
        AbstractInstructionsList result = Parser.Parse(fakeStream);

        // Then
        Assert.Single(result);
    }

    [Fact]
    public void ParseShouldMatchInstruction()
    {
        // Given
        ITokenStream fakeStream = FakeSourceCodeBuilder.NewSingleOperator(Operator.NOP);
        // When
        AbstractInstructionsList result = Parser.Parse(fakeStream);

        // Then
        Assert.IsType<Instruction>(result.First());
    }

    [Fact]
    public void ParseShouldMatchNopInstructionWhenInputNopOperator()
    {
        ITokenStream fakeStream = FakeSourceCodeBuilder.NewSingleOperator(Operator.NOP);

        // When
        AbstractInstructionsList result = Parser.Parse(fakeStream);

        // Then
        Assert.Equal(result.Encode(), new byte[] { Operator.NOP.Code });
    }

    [Fact]
    public void ParseShouldMatchHaltInstructionWhenInputHalt()
    {
        // Given
        ITokenStream fakeStream = FakeSourceCodeBuilder.NewSingleOperator(Operator.HALT);

        // When
        AbstractInstructionsList result = Parser.Parse(fakeStream);

        // Then
        Assert.Equal(result.Encode(), new byte[] { Operator.HALT.Code });
    }

    [Fact]
    public void ParseShouldMatchReturnInstructionWhenInputRet()
    {
        // Given
        ITokenStream fakeStream = FakeSourceCodeBuilder.NewSingleOperator(Operator.RET);

        // When
        AbstractInstructionsList result = Parser.Parse(fakeStream);

        // Then
        Assert.Equal(result.Encode(), new byte[] { Operator.RET.Code });
    }

    [Fact]
    public void ParseShouldMatchIrmovlInstruction()
    {
        // Given
        ITokenStream fakeStream = FakeSourceCodeBuilder.NewIrmovl(0x1234, Register.EAX);

        // When
        AbstractInstructionsList result = Parser.Parse(fakeStream);

        // Then
        Assert.Equal(result.Encode(), new byte[] { Operator.IRMOVL.Code, 0x80, 0x34, 0x12, 0x00, 0x00 });
    }

    [Fact]
    public void ParseShouldMatchRrmovlInstruction()
    {
        // Given
        ITokenStream fakeStream = FakeSourceCodeBuilder.NewOperationWithTwoRegisters(Operator.RRMOVL, Register.EAX, Register.EBX);

        // When
        AbstractInstructionsList result = Parser.Parse(fakeStream);

        // Then
        Assert.Equal(result.Encode(), new byte[] { Operator.RRMOVL.Code, 0x03 });
    }

    [Fact]
    public void ParseShouldMatchRmmvolInstruction()
    {
        // Given
        ITokenStream fakeStream = FakeSourceCodeBuilder.NewRmmovl(Register.EAX, Register.EBX, 0x12345);

        // When
        AbstractInstructionsList result = Parser.Parse(fakeStream);

        // Then
        Assert.Equal(result.Encode(), new byte[] { Operator.RMMOVL.Code, 0x03, 0x45, 0x23, 0x01, 0x00 });
    }

    [Fact]
    public void ParseShouldMatchMrmovlInstruction()
    {
        // Given
        ITokenStream fakeStream = FakeSourceCodeBuilder.NewMrmovl(Register.EAX, 0x12345, Register.EBX);

        // When
        AbstractInstructionsList result = Parser.Parse(fakeStream);

        // Then
        Assert.Equal(result.Encode(), new byte[] { Operator.MRMOVL.Code, 0x30, 0x45, 0x23, 0x01, 0x00 });
    }

    public static List<object[]> Opl = new List<object[]>{
        new object[] { Operator.ADDL, Register.EAX, Register.EBX },
        new object[] { Operator.SUBL, Register.EAX, Register.EBX },
        new object[] { Operator.ANDL, Register.EAX, Register.EBX },
        new object[] { Operator.XORL, Register.EAX, Register.EBX },
    };

    [Theory]
    [MemberData(nameof(Opl))]
    public void ParseShouldMatchOpl(Operator op, Register ra, Register rb)
    {
        // Given
        ITokenStream fakeStream = FakeSourceCodeBuilder.NewSource().Operator(op).Register(ra).Raw(Token.Comma).Register(rb).TokenStream();

        // When
        AbstractInstructionsList result = Parser.Parse(fakeStream);

        // Then
        Assert.Equal(result.Encode(), new byte[] { op.Code, (byte)(ra.Code << 4 | rb.Code) });
    }

    public static List<object[]> JumpInstructions = new List<object[]>
    {
        new object[] {Operator.JMP, "main", (uint)0x12345},
        new object[] {Operator.CALL, "func", (uint)0x12345 },
        new object[] {Operator.JE, "foo", (uint)0x12345 },
        new object[] {Operator.JNE, "main", (uint)0x12345 },
        new object[] {Operator.JL, "main", (uint)0x12345 },
        new object[] {Operator.JLE, "main", (uint)0x56789 },
        new object[] {Operator.JG, "main", (uint)0xaabbcc },
        new object[] {Operator.JGE, "main", (uint)0x12345 }
    };

    [Theory]
    [MemberData(nameof(JumpInstructions))]
    public void ParseShouldMatchJumpInstruction(Operator op, string dst, uint address)
    {
        // Given
        ITokenStream fakeStream = FakeSourceCodeBuilder.NewSource()
            .Operator(op).Raw(new IDToken(dst))
            .TokenStream();

        // When
        AbstractInstructionsList result = Parser.Parse(fakeStream);
        result.AddSymbol(dst, address);

        // Then 
        Assert.Equal(result.Encode(), new byte[] { op.Code }.Concat(BitConverter.GetBytes(address)));
    }

    [Fact]
    public void ParseShouldMatchLabel()
    {
        // Given
        ITokenStream fakeStream = FakeSourceCodeBuilder.NewSource().Label("main").TokenStream();

        // When
        var result = Parser.Parse(fakeStream);

        // Then
        Assert.True(result.TryGetSymbolAddress("main", out _));
    }

    [Fact]
    public void ParseShouldSetCurrectAddressOfLabel()
    {
        // Given
        var fakeStream = FakeSourceCodeBuilder.NewSource().PseudoInstruction("POS")
            .Integer<uint>(0x100)
            .Label("main")
            .TokenStream();

        // When
        var result = Parser.Parse(fakeStream);
        uint addr;
        result.TryGetSymbolAddress("main", out addr);

        // Then
        Assert.Equal((uint)0x100, addr);
    }
}