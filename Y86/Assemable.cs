// 上游 -> abstract instructions list (AIL)
// AIL 类型：指令 伪指令
// label边解析边定义，直接放进符号表
//
// parser解析完后产生符号表和AIL
// 使用符号表和AIL可以直接生成机器码
//
// 字节流 ----> token流 ----> 逻辑单元 ----> 抽象指令 ----> 机器码
//
// 直接生成指令:
// [前序处理] ----> 逻辑单元 ----> 抽象指令 ----> 机器码
//
// parser接受一个token流的输入，输出符号表和AIL
// token流是源程序中的逻辑碎片，每一个token都是一个独立的逻辑单元:
// label定义，label引用，指令名，寄存器，立即数，基址偏移寻址，伪指令
//
using Y86.Assemable.Instructions;
using Y86.Machine;

namespace Y86.Assemable
{

    namespace Errors
    {

        [Serializable]
        public class MismatchException : Exception
        {

            public Token? Token { get; }
            public Token.Types? Expect { get; }

            public MismatchException(Token.Types expect, Token tk) : base()
            {
                Token = tk;
                Expect = expect;
            }

            public MismatchException(string msg) : base(msg) { }

            public override string ToString()
            {
                if (Token != null && Expect != null)
                {
                    return $"expect a {Expect} token, but actuan is {Token.Type}";
                }
                return base.ToString();
            }
        }

        [Serializable]
        public class LabelRedefinedException : Exception
        {
            public string Label { get; }
            public LabelRedefinedException(string label) : base() => Label = label;
            public override string ToString()
            {
                return $"label \"{Label}\" is redefined";
            }
        }

        [Serializable]
        public class UnsupportedInstructionException : Exception
        {
            public string Name { get; }
            public UnsupportedInstructionException(string name) : base() => Name = name;
            public override string ToString()
            {
                return $"\"{Name}\" is not a instruction name";
            }
        }

        [Serializable]
        public class UnsupportedRegisterException : Exception
        {
            public string Name { get; }
            public UnsupportedRegisterException(string name) : base() => Name = name;
            public override string ToString()
            {
                return $"\"{Name}\" is not a valid register name";
            }
        }

        [Serializable]
        public class ReferenceOfLabelNotResolveException : Exception
        {
            public string Label { get; }
            public ReferenceOfLabelNotResolveException(string label) : base() => Label = label;
            public override string ToString()
            {
                return $"reference of label\"{Label}\" is not resolve, can not encode because address is unsolved.";
            }
        }

        [Serializable]
        public class UnsupportedAssemableCommandException : Exception
        {
            public string Name { get; }
            public UnsupportedAssemableCommandException(string name) : base() => Name = name;
            public override string ToString()
            {
                return $"assemable command \"{Name}\" is not supported.";
            }
        }
    }

    // Token 描述基本的符号信息
    public class Token
    {

        public enum Types
        {
            EOS,        // end of stream 代表符号流结束
            Whitespace, // 所有空白字元，用于分割两个token以避免二义性
            ID,         // 标识符如main/addl/eax等
            Integer,    // 整型数
            Dot,        // 符号 "."
            Comma,      // 符号 ","
            Present,    // 符号 "%"
            Dollar,     // 符号 "$"
            Colon,      // 符号 ":"
            Hashtag,    // 符号 “#”
            LeftParentheses, // 符号 "("
            RightParentheses, // 符号 ")"
        }

        public Types Type { get; }

        protected Token(Types type)
        {
            Type = type;
        }

        public static Token EOS = new(Types.EOS);
        public static Token Dot = new(Types.Dot);
        public static Token Comma = new(Types.Comma);
        public static Token Present = new(Types.Present);
        public static Token Dollar = new(Types.Dollar);
        public static Token Colon = new(Types.Colon);
        public static Token Hashtag = new(Types.Hashtag);
        public static Token Whitespace = new(Types.Whitespace);
        public static Token LeftParentheses = new(Types.LeftParentheses);
        public static Token RightParentheses = new(Types.RightParentheses);
    }

    // Token 子类
    namespace Tokens
    {
        // ID 类型的Token，携带id信息
        public class IDToken : Token
        {
            public string ID { get; }
            public IDToken(string id) : base(Types.ID) => ID = id;
        }

        // Int型token，携带一个整型数
        public class IntegerToken<T> : Token
        {
            public T Value { get; }

            public IntegerToken(T val) : base(Types.Integer) => Value = val;
        }

    }

    public interface ITokenStream
    {
        public Token Lookahead(int n = 1);
        public void Consume(int n = 1);
    }

    // AbstractInstruction 抽象指令类型，机器指令和伪指令的基类
    public class AbstractInstruction
    {
        // Address 为指令安排的地址
        public UInt32 Address { get; set; }

        // Size 指令长度
        public virtual int Size { get => 0; }

        public AbstractInstruction() { }
        public AbstractInstruction(UInt32 addr) => Address = addr;

        // 编码指令，子类需重写此指令
        public virtual byte[] Encode()
        {
            throw new NotImplementedException("subclass must implement encode method");
        }
    }

    // PesudoInstruction 伪指令类型
    public class PseudoInstruction : AbstractInstruction
    {

        public enum Commands
        {
            SetPosition, // .POS 指定下一条指令的地址
            Align,       // .align 指定下一条指令地址对齐到几byte
            DefineLong,  // .long 定义long型数据
            DefineWord,  // .word 定义word型数据
            DefineByte,  // .byte 定义byte型数据
        }

        public Commands Command { get; }

        public static readonly Dictionary<string, Commands> CommandNames; // 记录为指令名和对应的代码

        static PseudoInstruction()
        {
            CommandNames = new();
            CommandNames.Add("pos", Commands.SetPosition);
            CommandNames.Add("align", Commands.Align);
            CommandNames.Add("long", Commands.DefineLong);
            CommandNames.Add("word", Commands.DefineWord);
            CommandNames.Add("byte", Commands.DefineByte);
        }

        protected PseudoInstruction(Commands cmd) : base(0) => Command = cmd;
        protected PseudoInstruction(UInt32 addr, Commands cmd) : base(addr) => Command = cmd;

        public virtual void Apply(AbstractInstructionsList ail)
        {
            throw new NotImplementedException("subclass of PesudoInstruction must implement Apply method.");
        }
    }

    // 伪指令实现的集合
    namespace PseudoInstructions
    {
        // SetInstructionPosition 伪指令，重置下一条指令的地址
        public class SetInstructionPosition : PseudoInstruction
        {
            public UInt32 Position { get; }
            public SetInstructionPosition(UInt32 pos) : base(Commands.SetPosition) => Position = pos;

            public override void Apply(AbstractInstructionsList ail)
            {
                ail.Address.Reset(Position);
            }
        }

        // Align 指令对齐，对齐到1/2/4字节
        public class Align : PseudoInstruction
        {
            public int AlignByte { get; } // 对齐的字节数

            public Align(int bytes) : base(Commands.Align)
            {
                switch (bytes)
                {
                    case 1:
                    case 2:
                    case 4:
                        AlignByte = bytes;
                        break;
                    default:
                        throw new Errors.UnsupportedAssemableCommandException("align bytes must be 1/2/4");
                }
            }

            public override void Apply(AbstractInstructionsList ail)
            {
                int result = 0;
                int rem = Math.DivRem((int)ail.Address.Value, AlignByte, out result);
                if (rem == 0)
                {
                    return;
                }
                int newAddr = result * AlignByte + AlignByte;
                ail.Address.Reset((uint)newAddr);
            }
        }

        // DefineData 定义数据
        public class DefineData<T> : PseudoInstruction
        {
            public List<T> Data { get; set; } = new List<T>();

            public override int Size
            {
                get
                {
                    switch (Command)
                    {
                        case Commands.DefineByte:
                            return Data.Count();
                        case Commands.DefineWord:
                            return Data.Count() * sizeof(short);
                        case Commands.DefineLong:
                            return Data.Count() * sizeof(int);
                        default:
                            throw new Errors.UnsupportedAssemableCommandException("integer type not supported");
                    }
                }
            }

            public DefineData(Commands cmdType) : base(cmdType) { }

            public override byte[] Encode()
            {
                switch (Command)
                {
                    case Commands.DefineByte:
                        return (Data as List<byte> ?? new List<byte>()).ToArray();
                    case Commands.DefineWord:
                        {
                            List<byte> data = new();
                            foreach (var n in (Data as List<short> ?? new List<short>()))
                            {
                                byte[] code = BitConverter.GetBytes(n);
                                data.AddRange(BitConverter.IsLittleEndian ? code : code.Reverse());
                            }
                            return data.ToArray();
                        }
                    case Commands.DefineLong:
                        {
                            List<byte> data = new();
                            foreach (var n in (Data as List<int> ?? new List<int>()))
                            {
                                byte[] code = BitConverter.GetBytes(n);
                                data.AddRange(BitConverter.IsLittleEndian ? code : code.Reverse());
                            }
                            return data.ToArray();
                        }
                    default:
                        throw new Errors.UnsupportedAssemableCommandException("integer type are not supported");
                }
            }

            public override void Apply(AbstractInstructionsList ail)
            {
                ail.Add(this);
            }
        }
    }

    // Instruction 需要最终翻译为机器码的指令
    public class Instruction : AbstractInstruction
    {
        public Operator Operator { get; }

        public override int Size { get => Operator.ByteSize; } // 指令长度

        public Instruction(Operator op) : base() => Operator = op;

        // 辅助方法，确保输出的byte[]为小端
        protected static byte[] ToLittleEndian(byte[] arr)
        {
            if (!BitConverter.IsLittleEndian)
            {
                arr.Reverse();
            }
            return arr;
        }

        public override byte[] Encode()
        {
            return new byte[] { Operator.Code };
        }
    }

    // Instruction 子类
    namespace Instructions
    {
        // push 或 pop 类指令，拥有一个寄存器参数
        public class PushOrPop : Instruction
        {
            public Register Register { get; }

            public PushOrPop(Operator op, Register reg) : base(op) => Register = reg;

            public override byte[] Encode()
            {
                int regCode = (Register.Code << 4) | 0x8;
                return new byte[] { Operator.Code, (byte)regCode };
            }
        }

        // SourceAndDestinationRegisters 拥有两个寄存器作为参数的命令
        public class SourceAndDestinationRegisters : Instruction
        {
            public Register Source { get; }
            public Register Destination { get; }

            public SourceAndDestinationRegisters(Operator op, Register src, Register dst) : base(op)
            {
                Source = src;
                Destination = dst;
            }

            public override byte[] Encode()
            {
                int regCode = (Source.Code << 4) | Destination.Code;
                return new byte[] { Operator.Code, (byte)regCode };
            }
        }

        public class DestinationLabel : Instruction
        {
            public string Label { get; }
            private UInt32? address = null;

            public DestinationLabel(Operator op, string label) : base(op) => Label = label;

            // Resolve 设置label对应的地址（解引用）
            public void Resolve(UInt32 addr)
            {
                address = addr;
            }

            public override byte[] Encode()
            {
                if (address == null)
                {
                    throw new Errors.ReferenceOfLabelNotResolveException(Label);
                }
                List<byte> result = new();
                result.Add(Operator.Code);

                byte[] addr = BitConverter.GetBytes((uint)address!);
                result.AddRange(ToLittleEndian(addr));

                return result.ToArray();
            }
        }

        // irmovl，移动立即数到寄存器中
        public class MoveImmediateNumberToRegister : Instruction
        {
            public Int32 ImmediateNumber { get; }
            public Register Register { get; }

            public MoveImmediateNumberToRegister(Operator op, Int32 num, Register reg) : base(op)
            {
                ImmediateNumber = num;
                Register = reg;
            }

            public override byte[] Encode()
            {
                List<byte> result = new();
                result.Add(Operator.Code);

                int regCode = (0x8 << 4) | Register.Code;
                result.Add((byte)regCode);

                result.AddRange(ToLittleEndian(BitConverter.GetBytes(ImmediateNumber)));

                return result.ToArray();
            }
        }

        // MoveBetweenMemoryAndRegister 在寄存器和内存间（或反过来）移动数据
        public class MoveBetweenMemoryAndRegister : Instruction
        {
            public Register DataRegister { get; } // 数据寄存器
            public Register BaseAddressRegister { get; } // 源址基址寄存器
            public int AddressOffset { get; } // 源址偏移地址

            public MoveBetweenMemoryAndRegister(Operator op, Register addr, Register data, Int32 offset) : base(op)
            {
                DataRegister = data;
                BaseAddressRegister = addr;
                AddressOffset = offset;
            }

            public override byte[] Encode()
            {
                List<byte> result = new();
                result.Add(Operator.Code);

                int regCode = (DataRegister.Code << 4) | BaseAddressRegister.Code;
                result.Add((byte)regCode);

                result.AddRange(ToLittleEndian(BitConverter.GetBytes(AddressOffset)));

                return result.ToArray();
            }
        }
    }

    // AIL Abstract Instructoins List 抽象指令列表
    // 包含指令和符号表
    public class AbstractInstructionsList : List<AbstractInstruction>
    {
        public class CodeAddress
        {
            public uint Value { get; private set; } = 0;

            public void Reset(uint addr = 0)
            {
                Value = addr;
            }

            public static CodeAddress operator +(CodeAddress address, uint addr)
            {
                address.Reset(address.Value + addr);
                return address;
            }
        }

        public CodeAddress Address { get; private set; } // 跟踪指令地址

        private Dictionary<string, uint> symbols = new();

        public AbstractInstructionsList() : base()
        {
            Address = new();
        }

        public new void Add(AbstractInstruction inst)
        {
            inst.Address = Address.Value;
            base.Add(inst);
            Address += (uint)inst.Size;
        }

        public void AddSymbol(string name)
        {
            if (symbols.ContainsKey(name))
            {
                throw new Errors.LabelRedefinedException(name);
            }
            symbols.Add(name, Address.Value);
        }

        public void AddSymbol(string name, uint address)
        {
            if (symbols.ContainsKey(name))
            {
                throw new Errors.LabelRedefinedException(name);
            }
            symbols.Add(name, address);
        }

        public bool TryGetSymbolAddress(string syb, out uint addr)
        {
            return symbols.TryGetValue(syb, out addr);
        }

        public byte[] Encode()
        {
            // resolve label address
            foreach (var inst in this)
            {
                var dest = inst as DestinationLabel;
                if (dest != null)
                {
                    uint addr;
                    if (!symbols.TryGetValue(dest.Label, out addr))
                    {
                        throw new Errors.ReferenceOfLabelNotResolveException(dest.Label);
                    }
                    dest.Resolve(addr);
                }
            }

            List<byte> result = new();
            foreach (var inst in this)
            {
                result.AddRange(inst.Encode());
            }
            return result.ToArray();
        }
    }

    public class Parser
    {
        public static AbstractInstructionsList Parse(ITokenStream s)
        {
            AbstractInstructionsList ail = new();    // 存放解析结果

            // 当token流未结束时连续的解析token
            // 允许出现在语句头的有whitespace/label/伪指令/opcode，这些token可以作为先导
            // 其它类型的token不能作为先导，出现在语句头部时表示输入为非法源代码
            //
            // 每轮解析，向前看一个token指导
            while (s.Lookahead() != Token.EOS)
            {
                Token tk = s.Lookahead();

                // 伪指令形如".POS 0x100",以一个"."开始，向前看到Dot符号则尝试匹配一个伪指令
                // 伪指令指导汇编器的行为，成功识别到一个伪指令后需要立刻执行伪指令的操作
                if (tk == Token.Dot)
                {
                    MatchPseudoInstruction(s).Apply(ail);
                    continue;
                }

                // label/opcode都以一个id作为先导
                if (tk.Type == Token.Types.ID)
                {
                    // 向前再看一个符号，如果是":"则识别为label，否则尝试匹配一个opcode
                    if (s.Lookahead(2) == Token.Colon)
                    {
                        ail.AddSymbol(MatchLabel(s));
                    }
                    else
                    {
                        ail.Add(MatchInstruction(s));
                    }
                    continue;
                }

                // 对于所有可以用于识别的先导token前的空白符号，都可以跳过
                if (tk == Token.Whitespace)
                {
                    s.Consume();
                    continue;
                }

                throw new Errors.MismatchException(
                    "expect a label/pseudo instruction/instruction name as the first element in line. invalid code.");
            }

            return ail;
        }

        // 匹配一个符号
        static string MatchLabel(ITokenStream s)
        {
            Tokens.IDToken? tk = s.Lookahead() as Tokens.IDToken;
            if (tk != null && s.Lookahead(2) == Token.Colon)
            {
                s.Consume(2); // 吃掉一个id和一个冒号
                return tk.ID;
            }
            throw new Errors.MismatchException(Token.Types.ID, s.Lookahead());
        }

        // 匹配一条伪指令
        // 伪指令格式为.cmdname
        static PseudoInstruction MatchPseudoInstruction(ITokenStream s)
        {
            ParseHelper helper = new(s);

            helper.Match(Token.Dot);
            string cmdName = helper.MatchID().ID;

            if (!PseudoInstruction.CommandNames.ContainsKey(cmdName.ToLower()))
            {
                throw new Errors.UnsupportedAssemableCommandException(cmdName);
            }

            switch (PseudoInstruction.CommandNames[cmdName.ToLower()])
            {
                case PseudoInstruction.Commands.SetPosition:
                    return new PseudoInstructions.SetInstructionPosition(helper.MatchInteger<uint>().Value);
                case PseudoInstruction.Commands.Align:
                    return new PseudoInstructions.Align(helper.MatchInteger<int>().Value);
                case PseudoInstruction.Commands.DefineByte:
                    {
                        PseudoInstructions.DefineData<byte> inst = new(PseudoInstruction.Commands.DefineByte);
                        inst.Data = helper.MatchDataList<byte>();
                        return inst;
                    }
                case PseudoInstruction.Commands.DefineLong:
                    {
                        PseudoInstructions.DefineData<int> inst = new(PseudoInstruction.Commands.DefineWord);
                        inst.Data = helper.MatchDataList<int>();
                        return inst;
                    }
                case PseudoInstruction.Commands.DefineWord:
                    {
                        PseudoInstructions.DefineData<short> inst = new(PseudoInstruction.Commands.DefineWord);
                        inst.Data = helper.MatchDataList<short>();
                        return inst;
                    }
            }

            throw new NotImplementedException("end of match pseudo instruction, check code.");
        }

        // 匹配一条指令
        static Instruction MatchInstruction(ITokenStream s)
        {
            ParseHelper helper = new(s); // 解析token流的辅助类

            // 解析指令
            // 根据指令判断类型，不同类型的指令拥有不同的参数，决定接下来的解析过程
            Operator op = helper.MatchOperator();

            // nop/hale/ret指令不需要参数
            if (op == Operator.NOP || op == Operator.HALT || op == Operator.RET)
            {
                return new(op);
            }

            // push/pop需要一个寄存器作为参数
            if (op == Operator.PUSHL || op == Operator.POPL)
            {
                // 继续向前看，尝试命中一个register
                Register register = helper.MatchRegister();
                return new Instructions.PushOrPop(op, register);
            }

            // rrmovl/addl/subl/andl/xorl需要两个寄存器，两个寄存器中间通过逗号隔开
            if (op == Operator.RRMOVL || op.IsMathOperator)
            {
                Register a = helper.MatchRegister();
                helper.Match(Token.Comma);
                Register b = helper.MatchRegister();
                return new Instructions.SourceAndDestinationRegisters(op, a, b);
            }

            // 跳转指令和call指令需要一个label作为目标
            if (op.IsJumpOperator || op == Operator.CALL)
            {
                string label = helper.MatchID().ID;
                return new Instructions.DestinationLabel(op, label);
            }

            if (op == Operator.IRMOVL)
            {
                // 匹配一个立即数和一个寄存器
                helper.Match(Token.Dollar);
                Tokens.IntegerToken<int> immediateNumber = helper.MatchInteger<int>();
                helper.Match(Token.Comma);
                Register register = helper.MatchRegister();
                return new Instructions.MoveImmediateNumberToRegister(op, immediateNumber.Value, register);
            }

            // rmmovl将数据从寄存器移动到内存，采用基址位移寻址
            // 命令格式 rmmovl %ra, offset(%rb)
            if (op == Operator.RMMOVL)
            {
                Register dataRegister = helper.MatchRegister();
                helper.Match(Token.Comma);
                Tokens.IntegerToken<int> offset = helper.MatchInteger<int>();
                helper.Match(Token.LeftParentheses);
                Register baseRegister = helper.MatchRegister();
                helper.Match(Token.RightParentheses);
                return new Instructions.MoveBetweenMemoryAndRegister(op, baseRegister, dataRegister, offset.Value);
            }

            // mrmovl 将数据从内存移动到寄存器，此阿勇基址位移寻址
            // 命令格式 mrmovl offset(%rb), %ra
            if (op == Operator.MRMOVL)
            {
                int offset = helper.MatchInteger<int>().Value;
                helper.Match(Token.LeftParentheses);
                Register baseRegister = helper.MatchRegister();
                helper.Match(Token.RightParentheses);
                helper.Match(Token.Comma);
                Register dataRegister = helper.MatchRegister();
                return new Instructions.MoveBetweenMemoryAndRegister(op, baseRegister, dataRegister, offset);
            }

            // 命令全部处理，绝对不会运行到这里
            throw new NotImplementedException();
        }
    }

    // 一些解析过程的辅助方法
    public class ParseHelper
    {
        private ITokenStream stream;

        public ParseHelper(ITokenStream s) => stream = s;

        // Match 尝试匹配下一个token
        // 向前看一个token，若此token于指定token相同，则接受并返回此token，否则引发异常
        public Token Match(Token tk)
        {
            if (stream.Lookahead() != tk)
            {
                throw new Errors.MismatchException(tk.Type, stream.Lookahead());
            }
            else
            {
                stream.Consume();
            }
            return tk;
        }

        // Match 尝试匹配下一个token
        // 向前看一个token，若此token属于指定类型，则接受并返回此token，否则引发异常
        public Token Match(Token.Types type)
        {
            Token tk = stream.Lookahead();
            if (tk.Type != type)
            {
                throw new Errors.MismatchException(type, stream.Lookahead());
            }
            else
            {
                stream.Consume();
            }
            return tk;
        }

        // MatchID 尝试将下一个token作为ID匹配
        // 向前看一个token，若此token是id token则接受并返回一个IDToken，否则引发异常
        public Tokens.IDToken MatchID()
        {
            Tokens.IDToken? tk = stream.Lookahead() as Tokens.IDToken;
            if (tk == null)
            {
                throw new Errors.MismatchException(Token.Types.ID, stream.Lookahead());
            }
            else
            {
                stream.Consume();
            }
            return tk;
        }

        // MatchOperator 尝试匹配下一个ID并使用此id查找operator
        // 向前看一个token，若此token是id且id字面值命中了一个指令名，则接受此token并返回对于的operator，否则引发异常
        public Operator MatchOperator()
        {
            Tokens.IDToken? tk = stream.Lookahead() as Tokens.IDToken;
            Operator? op = null;
            if (tk != null && (op = Operator.FindByName(tk.ID.ToLower())) != null)
            {
                stream.Consume();
                return op;
            }
            throw new Errors.MismatchException(Token.Types.ID, stream.Lookahead());
        }

        // MatchRegister 尝试将从token流的头部匹配一个寄存器名称模式
        // 向前看两个token，第一个token必须为"%"，第二个token为id且id字面值为一个寄存器名称
        // 如果满足上述条件，则接受这两个token并返回对应的寄存器，否则引发异常
        public Register MatchRegister()
        {
            if (stream.Lookahead() != Token.Present)
            {
                throw new Errors.MismatchException(Token.Types.Present, stream.Lookahead());
            }

            Tokens.IDToken? tk = stream.Lookahead(2) as Tokens.IDToken;
            Register? register = null;
            if (tk != null && (register = Register.FindByName(tk.ID.ToLower())) != null)
            {
                stream.Consume(2);
                return register;
            }
            throw new Errors.MismatchException(Token.Types.ID, stream.Lookahead(2));
        }

        // MatchInteger 尝试从输入的符号流头部匹配一个整形数，可以是int/short/byte
        // 向前看一个token，如果此token是整型token则接受并返回此token，否则引发异常
        public Tokens.IntegerToken<T> MatchInteger<T>()
        {
            Token tk = stream.Lookahead();
            Tokens.IntegerToken<T>? num = null;
            if (tk.Type == Token.Types.Integer && (num = tk as Tokens.IntegerToken<T>) != null)
            {
                stream.Consume();
                return num;
            }
            else
            {
                throw new Errors.MismatchException(Token.Types.Integer, tk);
            }
        }

        // MatchDataList 匹配数据列表
        public List<T> MatchDataList<T>()
        {
            List<T> data = new();
            try
            {
                while (true)
                {
                    data.Add(MatchInteger<T>().Value);
                }
            }
            catch (Errors.MismatchException) { }
            return data;
        }
    }
}