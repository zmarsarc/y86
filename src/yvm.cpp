#include <assert.h>
#include "yvm.h"

#define SET_OF (cur_context->flag |= 0x00000001)
#define CLR_OF (cur_context->flag &= 0xFFFFFFFE)
#define TST_OF (uint)(cur_context->flag & 0x00000001)

#define SET_SF (cur_context->flag |= 0x00000002)
#define CLR_SF (cur_context->flag &= 0xFFFFFFFD)
#define TST_SF (uint)((cur_context->flag & 0x00000002) >> 1)

#define SET_ZF (cur_context->flag |= 0x00000004)
#define CLR_ZF (cur_context->flag &= 0xFFFFFFFB)
#define TST_ZF (uint)((cur_context->flag & 0x00000004) >> 2)

// user define type
typedef unsigned int uint;
typedef unsigned char ubyte;

static uint nog;  // F register never used

// const
static const uint uint_max = (uint)0xFFFFFFFF;

static RESULT select_reg(vm_context* cur_context, const ubyte id, uint** reg) {
    assert(cur_context);

    switch (id) {
        case 0x0:
            *reg = &(cur_context->eax);
            break;
        case 0x1:
            *reg = &(cur_context->ecx);
            break;
        case 0x2:
            *reg = &(cur_context->edx);
            break;
        case 0x3:
            *reg = &(cur_context->ebx);
            break;
        case 0x4:
            *reg = &(cur_context->esp);
            break;
        case 0x5:
            *reg = &(cur_context->ebp);
            break;
        case 0x6:
            *reg = &(cur_context->esi);
            break;
        case 0x7:
            *reg = &(cur_context->edi);
            break;
        case 0xF:
            *reg = &nog;
            break;
        default:
            return E_INVALID_REG_ID;
    }
    return S_OK;
}

static RESULT split_regs(vm_context* cur_context, const ubyte reg_file, uint** reg_a, uint** reg_b) {
    if (select_reg(cur_context, (ubyte)((reg_file & 0xF0) >> 4), reg_a)) return E_INVALID_REG_ID;
    if (select_reg(cur_context, (ubyte)(reg_file & 0x0F), reg_b)) return E_INVALID_REG_ID;
    return S_OK;
}

extern "C" RESULT process(vm_context* cur_context, const ubyte opt, const ubyte regs, const uint arg) {
    assert(cur_context);
    if (cur_context->stat == HLT) return S_HALT;
    uint *reg_a = 0;
    uint *reg_b = 0;
    uint tmp = 0;

    switch (opt) {
        case cmovg:
            if (split_regs(cur_context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            *reg_b = (!TST_SF) ? *reg_a : *reg_b;
            break;
        case cmovge:
            if (split_regs(cur_context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            *reg_b = (!TST_SF || TST_ZF) ? *reg_a : *reg_b;
            break;
        case cmovne:
            if (split_regs(cur_context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            *reg_b = (!TST_ZF) ? *reg_a : *reg_b;
            break;
        case cmove:
            if (split_regs(cur_context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            *reg_b = (TST_ZF) ? *reg_a : *reg_b;
            break;
        case cmovl:
            if (split_regs(cur_context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            *reg_b = (TST_SF) ? *reg_a : *reg_b;
            break;
        case cmovle:
            if (split_regs(cur_context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            // b <- a
            *reg_b = (TST_SF || TST_ZF) ? *reg_a : *reg_b;
            break;
        case jg:
            if (!TST_SF) cur_context->pc = arg;
            break;
        case jge:
            if (!TST_SF || TST_ZF) cur_context->pc = arg;
            break;
        case jne:
            if (!TST_ZF) cur_context->pc = arg;
            break;
        case je:
            if (TST_ZF) cur_context->pc = arg;
            break;
        case jl:
            if (TST_SF) cur_context->pc = arg;
            break;
        case jle:
            if (TST_SF || TST_ZF) cur_context->pc = arg;
            break;
        case jmp:
            cur_context->pc = arg;
            break;
        case xorl:
            if (split_regs(cur_context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            tmp = *reg_a ^ *reg_b;
            if ((int)tmp < 0) SET_SF; else CLR_SF;
            if (tmp == (uint)0) SET_ZF; else CLR_ZF;
            CLR_OF;
            *reg_a = tmp;
            break;
        case andl:
            if (split_regs(cur_context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            tmp = *reg_a | *reg_b;
            if ((int)tmp < 0) SET_SF; else CLR_SF;
            if (tmp == (uint)0) SET_ZF; else CLR_ZF;
            CLR_OF;
            *reg_a = tmp;
            break;
        case subl:
            if (split_regs(cur_context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            tmp = (uint)(*reg_a - *reg_b);
            if ((int)*reg_b > (int)*reg_a) SET_OF; else CLR_OF;
            if ((int)tmp < 0) SET_SF; else CLR_SF;
            if (tmp == (uint)0) SET_ZF; else CLR_ZF;
            *reg_a = tmp;
            break;
        case addl:
            if (split_regs(cur_context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            tmp = (uint)(*reg_a + *reg_b);
            if ((*reg_a > uint_max - *reg_b) || (*reg_b > uint_max - *reg_a)) SET_OF; else CLR_OF;
            if ((int)tmp < 0) SET_SF; else CLR_SF;
            if (tmp == (uint)0) SET_ZF; else CLR_ZF;
            *reg_a = tmp;
            break;
        case call:
            cur_context->pc = arg;
            break;
        case ret:
            cur_context->pc = *(cur_context->memory + cur_context->esp);
            cur_context->esp = (uint)(cur_context->esp + sizeof(uint));
            break;
        case mrmovl:
            if (split_regs(cur_context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            tmp = (uint)((arg + *reg_b) % cur_context->m_size);
            *reg_a = *(uint*)(cur_context->memory + tmp);
            break;
        case rmmovl:
            if (split_regs(cur_context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            tmp = (uint)((arg + *reg_b) % cur_context->m_size);
            *(uint*)(cur_context->memory + tmp) = *reg_a;
            break;
        case irmovl:
            tmp = regs | 0xF0;
            if (split_regs(cur_context, tmp, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            *reg_b = arg;
            break;
        case rrmovl:
            if (split_regs(cur_context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            // b <- a
            *reg_b = *reg_a;
            break;
        case popl:
            if (split_regs(cur_context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            *reg_a = *(uint*)(cur_context->memory + cur_context->esp);
            cur_context->esp = (uint)((cur_context->esp + sizeof(uint)) % cur_context->m_size);
            break;
        case pushl:
            cur_context->esp = (uint)((cur_context->esp - sizeof(uint)) % cur_context->m_size);
            *(uint*)(cur_context->memory + cur_context->esp) = arg;
            break;
        case nop:
            break;
        case halt:
            cur_context->stat = HLT;
            return S_HALT;
        default:
            cur_context->stat = INS;
            return E_INVALID_OPT;
    }
    cur_context->stat = AOK;
    return S_OK;
}