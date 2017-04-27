#include "yvm.h"

#define SET_OF (flag |= 0x00000001)
#define CLR_OF (flag &= 0xFFFFFFFE)
#define TST_OF (uint)(flag & 0x00000001)

#define SET_SF (flag |= 0x00000002)
#define CLR_SF (flag &= 0xFFFFFFFD)
#define TST_SF (uint)((flag & 0x00000002) >> 1)

#define SET_ZF (flag |= 0x00000004)
#define CLR_ZF (flag &= 0xFFFFFFFB)
#define TST_ZF (uint)((flag & 0x00000004) >> 2)

// user define type
typedef unsigned int uint;
typedef unsigned char ubyte;

// register file
static uint eax;  // 0
static uint ecx;  // 1
static uint edx;  // 2
static uint ebx;  // 3
static uint esp;  // 4
static uint ebp;  // 5
static uint esi;  // 6
static uint edi;  // 7
static uint nog;  // F

static uint pc;
static uint stat;
static uint flag;

static char* memory;
static uint  m_size;

// const
static const uint uint_max = (uint)0xFFFFFFFF;

static RESULT select_reg(const ubyte id, uint** reg) {
    switch (id) {
        case 0x0:
            *reg = &eax;
            break;
        case 0x1:
            *reg = &ecx;
            break;
        case 0x2:
            *reg = &edx;
            break;
        case 0x3:
            *reg = &ebx;
            break;
        case 0x4:
            *reg = &esp;
            break;
        case 0x5:
            *reg = &ebp;
            break;
        case 0x6:
            *reg = &esi;
            break;
        case 0x7:
            *reg = &edi;
            break;
        case 0xF:
            *reg = &nog;
            break;
        default:
            return E_INVALID_REG_ID;
    }
    return S_OK;
}

static RESULT split_regs(const ubyte reg_file, uint** reg_a, uint** reg_b) {
    if (!select_reg((ubyte)((reg_file & 0xF0) >> 4), reg_a)) return E_INVALID_REG_ID;
    if (!select_reg((ubyte)(reg_file & 0x0F), reg_b)) return E_INVALID_REG_ID;
    return S_OK;
}

extern "C" RESULT process(const ubyte opt, const ubyte regs, const uint arg) {
    if (stat == HLT) return S_HALT;
    uint *reg_a = 0;
    uint *reg_b = 0;
    uint tmp = 0;

    switch (opt) {
        case cmovg:
            if (!split_regs(regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            *reg_b = (!TST_SF) ? *reg_a : *reg_b;
            break;
        case cmovge:
            if (!split_regs(regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            *reg_b = (!TST_SF || TST_ZF) ? *reg_a : *reg_b;
            break;
        case cmovne:
            if (!split_regs(regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            *reg_b = (!TST_ZF) ? *reg_a : *reg_b;
            break;
        case cmove:
            if (!split_regs(regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            *reg_b = (TST_ZF) ? *reg_a : *reg_b;
            break;
        case cmovl:
            if (!split_regs(regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            *reg_b = (TST_SF) ? *reg_a : *reg_b;
            break;
        case cmovle:
            if (!split_regs(regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            // b <- a
            *reg_b = (TST_SF || TST_ZF) ? *reg_a : *reg_b;
            break;
        case jg:
            if (!TST_SF) pc = arg;
            break;
        case jge:
            if (!TST_SF || TST_ZF) pc = arg;
            break;
        case jne:
            if (!TST_ZF) pc = arg;
            break;
        case je:
            if (TST_ZF) pc = arg;
            break;
        case jl:
            if (TST_SF) pc = arg;
            break;
        case jle:
            if (TST_SF || TST_ZF) pc = arg;
            break;
        case jmp:
            pc = arg;
            break;
        case xorl:
            if (!split_regs(regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            tmp = *reg_a ^ *reg_b;
            if ((int)tmp < 0) SET_SF; else CLR_SF;
            if (tmp == (uint)0) SET_ZF; else CLR_ZF;
            CLR_OF;
            *reg_a = tmp;
            break;
        case andl:
            if (!split_regs(regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            tmp = *reg_a | *reg_b;
            if ((int)tmp < 0) SET_SF; else CLR_SF;
            if (tmp == (uint)0) SET_ZF; else CLR_ZF;
            CLR_OF;
            *reg_a = tmp;
            break;
        case subl:
            if (!split_regs(regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            tmp = (uint)(*reg_a - *reg_b);
            if ((int)*reg_b > (int)*reg_a) SET_OF; else CLR_OF;
            if ((int)tmp < 0) SET_SF; else CLR_SF;
            if (tmp == (uint)0) SET_ZF; else CLR_ZF;
            *reg_a = tmp;
            break;
        case addl:
            if (!split_regs(regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            tmp = (uint)(*reg_a + *reg_b);
            if ((*reg_a > uint_max - *reg_b) || (*reg_b > uint_max - *reg_a)) SET_OF; else CLR_OF;
            if ((int)tmp < 0) SET_SF; else CLR_SF;
            if (tmp == (uint)0) SET_ZF; else CLR_ZF;
            *reg_a = tmp;
            break;
        case call:
            pc = arg;
            break;
        case ret:
            pc = *(memory + esp);
            esp = (uint)(esp + sizeof(uint));
            break;
        case mrmovl:
            if (!split_regs(regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            tmp = (uint)(arg + *reg_b);
            *reg_a = *(memory + tmp);
            break;
        case rmmovl:
            if (!split_regs(regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            tmp = (uint)(arg + *reg_b);
            *(memory + tmp) = *reg_a;
            break;
        case irmovl:
            tmp = regs | 0xF0;
            if (!split_regs(tmp, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            *reg_b = arg;
            break;
        case rrmovl:
            if (!split_regs(regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            // b <- a
            *reg_b = *reg_a;
            break;
        case popl:
            if (!split_regs(regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            *reg_a = *(memory + esp);
            esp = (uint)(esp + sizeof(uint));
            break;
        case pushl:
            esp = (uint)(esp - sizeof(uint));
            *(memory + esp) = arg;
            break;
        case nop:
            break;
        case halt:
            stat = HLT;
            return S_HALT;
        default:
            stat = INS;
            return E_INVALID_OPT;
    }
    stat = AOK;
    return S_OK;
}