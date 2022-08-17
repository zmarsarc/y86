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
        public UInt32 Address { get; }
        public AbstractInstruction(UInt32 addr) => Address = addr;
    }

    public class AIL
    {
        public List<AbstractInstruction> Instructions = new();
        public Dictionary<string, UInt32> Symbols = new();
    }

    public class Parser
    {
        public static AIL Parse(ITokenStream s)
        {
            AIL ail = new();    // 存放解析结果
            UInt32 address = 0; // 记录指令地址

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
                    // 注意，不允许label的重复定义
                    if (s.Lookahead(2) == Token.Colon)
                    {
                        string label = MatchLabel(s);
                        if (ail.Symbols.ContainsKey(label))
                        {
                            throw new Errors.LabelRedefinedException(label);
                        }
                        ail.Symbols.Add(label, address);
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
    }
}