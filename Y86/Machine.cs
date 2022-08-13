namespace Y86.Machine;

public class Operator {

    public static readonly Operator NOP = new("nop", 0x00, 1);
    public static readonly Operator HALT = new("halt", 0x10, 1);
    public static readonly Operator RRMOVL = new("rrmovl", 0x20, 2);
    public static readonly Operator IRMOVL = new("irmovl", 0x30, 6);
    public static readonly Operator RMMOVL = new("rmmovl", 0x40, 6);
    public static readonly Operator MRMOVL = new("mrmovl", 0x50, 6);
    public static readonly Operator ADDL = new("addl", 0x60, 2);
    public static readonly Operator SUBL = new("subl", 0x61, 2);
    public static readonly Operator ANDL = new("andl", 0x62, 2);
    public static readonly Operator XORL = new("xorl", 0x63, 2);
    public static readonly Operator JMP = new("jmp", 0x70, 5);
    public static readonly Operator JLE = new("jle", 0x71, 5);
    public static readonly Operator JL = new("jl", 0x72, 5);
    public static readonly Operator JE = new("je", 0x73, 5);
    public static readonly Operator JNE = new("jne", 0x74, 5);
    public static readonly Operator JGE = new("jge", 0x75, 5);
    public static readonly Operator JG = new("jg", 0x76, 5);
    public static readonly Operator CALL = new("call", 0x80, 5);
    public static readonly Operator RET = new("ret", 0x90, 1);
    public static readonly Operator PUSHL = new("pushl", 0xa0, 2);
    public static readonly Operator POPL = new("popl", 0xb0, 2);

    private static Dictionary<string, Operator> NameToOperator;

    public string Name {get; init;}
    public byte Code {get; init;}

    public int ByteSize {get; init;}

    public bool IsJumpOperator {
        get {
            return (Code >> 4 & 0x7) == 0x7;
        }
    }

    public bool IsMathOperator {
        get {
            return (Code >> 4 & 0x6) == 0x6;
        }
    }

    private Operator(string name, byte code, int bitSize) {
        Name = name;
        Code = code;
        ByteSize = bitSize;
    }

    static Operator() {
        NameToOperator = new Dictionary<string, Operator>();
        NameToOperator.Add(NOP.Name, NOP);
        NameToOperator.Add(HALT.Name, HALT);
        NameToOperator.Add(RRMOVL.Name, RRMOVL);
        NameToOperator.Add(IRMOVL.Name, IRMOVL);
        NameToOperator.Add(RMMOVL.Name, RMMOVL);
        NameToOperator.Add(MRMOVL.Name, MRMOVL);
        NameToOperator.Add(ADDL.Name, ADDL);
        NameToOperator.Add(SUBL.Name, SUBL);
        NameToOperator.Add(ANDL.Name, ANDL);
        NameToOperator.Add(XORL.Name, XORL);
        NameToOperator.Add(JMP.Name, JMP);
        NameToOperator.Add(JLE.Name, JLE);
        NameToOperator.Add(JL.Name, JL);
        NameToOperator.Add(JE.Name, JE);
        NameToOperator.Add(JNE.Name, JNE);
        NameToOperator.Add(JGE.Name, JGE);
        NameToOperator.Add(JG.Name, JG);
        NameToOperator.Add(CALL.Name, CALL);
        NameToOperator.Add(RET.Name, RET);
        NameToOperator.Add(PUSHL.Name, PUSHL);
        NameToOperator.Add(POPL.Name, POPL);
    }

    public static Operator? FindByName(string name) {
        return NameToOperator.GetValueOrDefault(name);
    }
}

public class Register {

    public static readonly Register EAX = new("eax", 0);
    public static readonly Register ECX = new("ecx", 1);
    public static readonly Register EDX = new("edx", 2);
    public static readonly Register EBX = new("ebx", 3);
    public static readonly Register ESP = new("esp", 4);
    public static readonly Register EBP = new("ebp", 5);
    public static readonly Register ESI = new("esi", 6);
    public static readonly Register EDI = new("edi", 7);
    public static readonly Register NoneRegister = new("no_reg", 8);

    private static Dictionary<string, Register> NameToRegister;

    static Register() {
        NameToRegister = new Dictionary<string, Register>();
        NameToRegister.Add(EAX.Name, EAX);
        NameToRegister.Add(ECX.Name, ECX);
        NameToRegister.Add(EDX.Name, EDX);
        NameToRegister.Add(EBX.Name, EBX);
        NameToRegister.Add(ESP.Name, ESP);
        NameToRegister.Add(EBP.Name, EBP);
        NameToRegister.Add(ESI.Name, ESI);
        NameToRegister.Add(EDI.Name, EDI);
    }

    public string Name {get; init;}
    public byte Code {get; init;}

    private Register(string name, byte code) {
        Name = name;
        Code = code;
    }

    public static Register? FindByName(string name) {
        return NameToRegister.GetValueOrDefault(name);
    }
}