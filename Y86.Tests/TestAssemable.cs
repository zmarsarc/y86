using Y86.Assemable;
using Y86.Machine;

namespace Y86.Tests;

public class TestParse
{
    [Fact]
    public void ParseShouldReturnAbstractInstructionList()
    {
        // Given
        ITokenStream fakeStream = FakeSourceCodeBuilder.NewSingleOperator(Operator.NOP);

        // When
        AIL result = Parser.Parse(fakeStream);

        // Then
        Assert.NotEmpty(result.Instructions);
    }

    [Fact]
    public void ParseShouldStopWhenEndOfStream()
    {
        // Given
        ITokenStream fakeStream =FakeSourceCodeBuilder.NewSingleOperator(Operator.NOP);

        // When
        AIL result = Parser.Parse(fakeStream);

        // Then
        Assert.Single(result.Instructions);
    }

    [Fact]
    public void ParseShouldMatchInstruction()
    {
        // Given
        ITokenStream fakeStream = FakeSourceCodeBuilder.NewSingleOperator(Operator.NOP);
        // When
        AIL result = Parser.Parse(fakeStream);

        // Then
        Assert.IsType<Instruction>(result.Instructions.First());
    }

    [Fact]
    public void ParseShouldMatchNopInstructionWhenInputNopOperator()
    {
        ITokenStream fakeStream = FakeSourceCodeBuilder.NewSingleOperator(Operator.NOP);

        // When
        AIL result = Parser.Parse(fakeStream);

        // Then
        Assert.Equal(result.Encode(), new byte[] { Operator.NOP.Code });
    }

    [Fact]
    public void ParseShouldMatchHaltInstructionWhenInputHalt()
    {
        // Given
        ITokenStream fakeStream = FakeSourceCodeBuilder.NewSingleOperator(Operator.HALT);

        // When
        AIL result = Parser.Parse(fakeStream);
    
        // Then
        Assert.Equal(result.Encode(), new byte[] {Operator.HALT.Code});
    }

    [Fact]
     public void ParseShouldMatchReturnInstructionWhenInputRet()
    {
        // Given
        ITokenStream fakeStream = FakeSourceCodeBuilder.NewSingleOperator(Operator.RET);

        // When
        AIL result = Parser.Parse(fakeStream);
    
        // Then
        Assert.Equal(result.Encode(), new byte[] {Operator.RET.Code});
    }
}