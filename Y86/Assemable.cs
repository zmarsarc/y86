using Y86.Machine;

namespace Y86.Assemable;

public enum TokenType {
    EOF,
    ID,
    Dot,
    Present,
    Comma,
    Number,
    LeftParenthesis,
    RightParenthesis,
    Colon,
    Dollar
}

public enum NumberType {
    Binary,
    Octal,
    Decimal,
    Hexadecimal
}

public class Token {

    public static readonly Token EOF = new("EOF", TokenType.EOF);
    public static readonly Token Dot = new(".", TokenType.Dot);
    public static readonly Token Present = new("%", TokenType.Present);
    public static readonly Token Comma = new(",", TokenType.Comma);
    public static readonly Token LeftParenthesis = new("(", TokenType.LeftParenthesis);
    public static readonly Token RightParenthesis = new(")", TokenType.RightParenthesis);
    public static readonly Token Colon = new(":", TokenType.Colon);
    public static readonly Token Dollar = new("$", TokenType.Dollar);

    public TokenType Type {get; init;}
    public int SubType {get; init;}

    public string Value {get; init;}

    public Token(string value, TokenType type, int subType = 0) {
        Value = value;
        Type = type;
        SubType = subType;
    }
}

public enum ASTType {
    Fragment,
    Instruction,
    Register,
    Integer,
    Label
}

public class AST {
    public ASTType Type {get; init;}
    public object? Value {get; init;}

    public List<AST> Children = new List<AST>();

    public AST(ASTType type, object? value = null) {
        Type = type;
        Value = value;
    }
}

public class ASTBuilder {

    public static AST SingleOperatorInstruction(Operator op) {
        return new(ASTType.Instruction, op);
    }

    public static AST OperatorWithRegisters(Operator op, Register src, Register dst) {
        AST inst = new(ASTType.Instruction, op);
        inst.Children.Add(new(ASTType.Register, src));
        inst.Children.Add(new(ASTType.Register, dst));
        return inst;
    }

    public static AST OperatorWithLabel(Operator op, string label) {
        AST inst = new(ASTType.Instruction, op);
        inst.Children.Add(new(ASTType.Label, label));
        return inst;
    }
}

public interface ITokenStream {
    Token Lookahead(int n = 1);

    void Consume(int n = 1);
}

public class Parser {

    private ITokenStream Stream;

    public Parser(ITokenStream stream) {
        Stream = stream;
    }

    public AST Parse() {
        AST root = new(ASTType.Fragment);
        while (Stream.Lookahead() != Token.EOF) {
            if (Stream.Lookahead().Type == TokenType.ID) {
                if (Stream.Lookahead(2) == Token.Colon) {
                    root.Children.Add(MatchLabel());
                } else {
                    root.Children.Add(MatchFullInstruction());
                }
                continue;
            }
            
            // TODO: use custom exception
            throw new ApplicationException();
        }
        return root;
    }

    private AST MatchLabel() {
        Token idToken = MatchTokenByType(TokenType.ID);
        MatchTokenByType(TokenType.Colon);
        return new(ASTType.Label, idToken.Value);
    }

    private Token MatchTokenByType(TokenType type) {
        Token tk = Stream.Lookahead();
        if (tk.Type != type) {
            // TODO: use custom exception
            throw new ApplicationException();
        }
        Stream.Consume();
        return tk;
    }

    private AST MatchFullInstruction() {
        AST inst = MatchInstruction();

        Operator op = (Operator)inst.Value!;

        if (op == Operator.NOP || op == Operator.HALT || op == Operator.RET) {
            return inst;
        }

        if (op == Operator.PUSHL || op == Operator.POPL) {
            inst.Children.Add(MatchRegister());
            inst.Children.Add(new(ASTType.Register, Register.NoneRegister));
            return inst;
        }

        if (op.IsJumpOperator || op == Operator.CALL) {
            AST label = new(ASTType.Label, MatchTokenByType(TokenType.ID).Value);
            inst.Children.Add(label);
            return inst;
        }

        if (op.IsMathOperator || op == Operator.RRMOVL) {
            AST ra = MatchRegister();
            MatchTokenByType(TokenType.Comma);
            AST rb = MatchRegister();
            inst.Children.Add(ra);
            inst.Children.Add(rb);
            return inst;
        }

        if (op == Operator.IRMOVL) {
            MatchTokenByType(TokenType.Dollar);
            AST num = MatchNumber();
            MatchTokenByType(TokenType.Comma);
            AST reg = MatchRegister();

            inst.Children.Add(new(ASTType.Register, Register.NoneRegister));
            inst.Children.Add(reg);
            inst.Children.Add(num);
            return inst;
        }

        if (op == Operator.RMMOVL) {
            AST ra = MatchRegister();
            MatchTokenByType(TokenType.Comma);
            AST offset = MatchNumber();
            MatchTokenByType(TokenType.LeftParenthesis);
            AST rb = MatchRegister();
            MatchTokenByType(TokenType.RightParenthesis);

            inst.Children.Add(ra);
            inst.Children.Add(rb);
            inst.Children.Add(offset);
            return inst;
        }

        if (op == Operator.MRMOVL) {
            AST offset = MatchNumber();
            MatchTokenByType(TokenType.LeftParenthesis);
            AST rb = MatchRegister();
            MatchTokenByType(TokenType.RightParenthesis);
            MatchTokenByType(TokenType.Comma);
            AST ra = MatchRegister();

            inst.Children.Add(ra);
            inst.Children.Add(rb);
            inst.Children.Add(offset);
            return inst;
        }

        // TODO: use custome exception
        throw new ApplicationException();
    }

    private AST MatchRegister() {
        MatchTokenByType(TokenType.Present);
        Token tk = MatchTokenByType(TokenType.ID);

        Register? reg = Register.FindByName(tk.Value);
        if (reg == null) {
            // TODO: use custom exception
            throw new ApplicationException();
        }

        return new(ASTType.Register, reg);
    }

    private AST MatchInstruction() {
        Token instToken = MatchTokenByType(TokenType.ID);
        Operator? op = Operator.FindByName(instToken.Value);
        
        if (op == null) {
            // TODO: use custom exception
            throw new ApplicationException();
        }

        return new(ASTType.Instruction, op);
    }

    private AST MatchNumber() {
        Token tk = MatchTokenByType(TokenType.Number);
        int num = int.Parse(tk.Value);
        return new(ASTType.Integer, num);
    }

}

interface IValue {
    int Value {get;}
}

class ImmediateValue: IValue {
    private int value;

    public ImmediateValue(int val) {
        value = val;
    }

    public int Value { get => value;}
}

class AddressValue: IValue {

    private string labelName;
    private Dictionary<string, int> symbolTable;

    public AddressValue(string label, Dictionary<string, int> symbols) {
        labelName = label;
        symbolTable = symbols;
    }

    public int Value {get => symbolTable.GetValueOrDefault(labelName);}
}

class Instruction {

    public Operator Operator {get;}
    public Register? Source {get;}
    public Register? Destnation {get;}
    public IValue? Variable {get;}

    public Instruction(Operator op, Register? ra = null, Register? rb = null, IValue? var = null) {
        Operator = op;
        Source = ra;
        Destnation = rb;
        Variable = var;
    }
}

public class CodeGenerator {

    public byte[] Generate(AST root) {
        Dictionary<string, int> symbolTable = new();
        List<Instruction> instructions = new();
        int address = 0;

        foreach (var ast in root.Children) {
            switch (ast.Type) {
                case ASTType.Instruction:
                    Instruction inst = MakeInstruction(ast, symbolTable);
                    address += inst.Operator.ByteSize;
                    instructions.Add(inst);
                    break;
                case ASTType.Label:
                    string label = (string)ast.Value!;
                    symbolTable[label] = address;
                    break;
                default:
                    throw new ApplicationException();
            }
        }

        List<byte> code = new();
        foreach (var inst in instructions) {
            code.AddRange(Encode(inst));
        }

        return code.ToArray();
    }

    private List<byte> Encode(Instruction inst) {
        List<byte> code = new();
        code.Add(inst.Operator.Code);
        if (inst.Source != null && inst.Destnation != null) {
            int reg = (inst.Source.Code << 4) | inst.Destnation.Code;
            code.Add((byte)reg);
        }
        if (inst.Variable != null) {
            byte[] bytes = BitConverter.GetBytes(inst.Variable.Value);
            if (!BitConverter.IsLittleEndian) {
                Array.Reverse(bytes);
            }
            code.AddRange(bytes);
        }

        return code;
    }

    private Instruction MakeInstruction(AST inst, Dictionary<string, int> symbols) {
        Operator op = (Operator)inst.Value!;

        if (op == Operator.NOP || op == Operator.HALT || op == Operator.RET) {
            return new(op);
        }
        if (op.IsMathOperator || op == Operator.RRMOVL || op == Operator.PUSHL || op == Operator.POPL) {
            return new(op, (Register)inst.Children[0].Value!, (Register)inst.Children[1].Value!);
        }
        if (op == Operator.IRMOVL || op == Operator.MRMOVL || op == Operator.RMMOVL) {
            return new(op,
                (Register)inst.Children[0].Value!,
                (Register)inst.Children[1].Value!,
                new ImmediateValue((int)inst.Children[2].Value!));
        }
        if (op.IsJumpOperator || op == Operator.CALL) {
            return new Instruction(op, var: new AddressValue((string)inst.Children[0].Value!, symbols));
        }

        throw new ApplicationException();
    }
}