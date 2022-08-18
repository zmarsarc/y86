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

using Y86.Machine;

namespace Y86.Assemable
{

    namespace Errors
    {

        [Serializable]
        public class MismatchException : Exception
        {

            public Token Token { get; }
            public Token.Types Expect { get; }

            public MismatchException(Token.Types expect, Token tk) : base()
            {
                Token = tk;
                Expect = expect;
            }

            public override string ToString()
            {
                return $"expect a {Expect} token, but actuan is {Token.Type}";
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
        public class ReferenceOfLabelNotResolveException: Exception
        {
            public string Label {get;}
            public ReferenceOfLabelNotResolveException(string label) : base() => Label = label;
            public override string ToString()
            {
                return $"reference of label\"{Label}\" is not resolve, can not encode because address is unsolved.";
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
            Int32,      // 32位整型数
            Dot,        // 符号 "."
            Comma,      // 符号 ","
            Present,    // 符号 "%"
            Dollar,     // 符号 "$"
            Colon,      // 符号 ":"
            Hashtag     // 符号 “#”
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
    }

    // ID 类型的Token，携带id信息
    public class IDToken : Token
    {
        public string ID { get; }
        public IDToken(string id) : base(Types.ID) => ID = id;
    }

    // Int32类型的Token，携带数值
    public class Int32Token : Token
    {
        public Int32 Value { get; }
        public Int32Token(Int32 val) : base(Types.Int32) => Value = val;
    }

    public interface ITokenStream
    {
        public Token Lookahead(int n = 1);
        public void Consume(int n = 1);
    }

    public class AbstractInstruction
    {
        public UInt32 Address { get; set; }
        public AbstractInstruction() { }
        public AbstractInstruction(UInt32 addr) => Address = addr;
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
    }

    // 伪指令实现的集合
    namespace PesudoInstructions
    {
        // SetInstructionPosition 伪指令，重置下一条指令的地址
        public class SetInstructionPosition : PseudoInstruction
        {
            public UInt32 Position { get; }
            public SetInstructionPosition(UInt32 pos) : base(Commands.SetPosition) => Position = pos;
        }
    }

    // Instruction 需要最终翻译为机器码的指令
    public class Instruction : AbstractInstruction
    {
        public Operator Operator { get; }

        public Instruction(Operator op) : base() => Operator = op;

        // 编码指令，子类需重写此指令
        public virtual byte[] Encode()
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
                if (!BitConverter.IsLittleEndian)
                {
                    addr.Reverse();
                }
                result.AddRange(addr);
                
                return result.ToArray();
            }
        }
    }

    public class AIL
    {
        public List<AbstractInstruction> Instructions = new(); // 记录指令
        public Dictionary<string, UInt32> Symbols = new(); // 记录符号
        private UInt32 address = 0; // 跟踪指令地址

        // 记录symbol到符号表
        // 注意，symbol不允许重复定义，已经定义过的symbol再次定义会引发异常
        public void AddSymbol(string name)
        {
            if (Symbols.ContainsKey(name))
            {
                throw new Errors.LabelRedefinedException(name);
            }
            Symbols.Add(name, address);
        }
    }

    public class Parser
    {
        public static AIL Parse(ITokenStream s)
        {
            AIL ail = new();    // 存放解析结果

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

                }

                // label/opcode都以一个id作为先导
                if (tk.Type == Token.Types.ID)
                {
                    // 向前再看一个符号，如果是":"则识别为label，否则尝试匹配一个opcode
                    if (s.Lookahead(2) == Token.Colon)
                    {
                        ail.AddSymbol(MatchLabel(s));
                        s.Consume(2); // 吃掉id和一个冒号
                    }
                    else
                    {

                    }
                    continue;
                }

                // 对于所有可以用于识别的先导token前的空白符号，都可以跳过
                if (tk == Token.Whitespace)
                {
                    s.Consume();
                    continue;
                }

                throw new ApplicationException();
            }

            return ail;
        }

        static string MatchLabel(ITokenStream s)
        {
            IDToken? tk = s.Lookahead() as IDToken;
            if (tk != null)
            {
                s.Consume(2); // 吃掉一个id和一个冒号
                return tk.ID;
            }
            throw new Errors.MismatchException(Token.Types.ID, s.Lookahead());
        }

        // 匹配一条伪指令
        static void MatchPseudoInstruction(ITokenStream s)
        {
            if (s.Lookahead() == Token.Dot && s.Lookahead(2).Type == Token.Types.ID)
            {
                IDToken? tk = s.Lookahead(2) as IDToken;
                if (tk == null) throw new ApplicationException("some code bug exists, this token must be id");

                // TODO: implement me!
                throw new NotImplementedException();
            }

            throw new Errors.MismatchException(Token.Types.ID, s.Lookahead(2));
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

            // TODO: implement me!
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
        public IDToken MatchID()
        {
            IDToken? tk = stream.Lookahead() as IDToken;
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
            IDToken? tk = stream.Lookahead() as IDToken;
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

            IDToken? tk = stream.Lookahead(2) as IDToken;
            Register? register = null;
            if (tk != null && (register = Register.FindByName(tk.ID.ToLower())) != null)
            {
                stream.Consume(2);
                return register;
            }
            throw new Errors.MismatchException(Token.Types.ID, stream.Lookahead(2));
        }
    }
}