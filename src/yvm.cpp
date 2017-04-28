#include <assert.h>
#include "yvm.h"

#define SET_OF (context->flag |= F_OVER)
#define CLR_OF (context->flag &= ~F_OVER)
#define TST_OF (uint)(context->flag & F_OVER)

#define SET_SF (context->flag |= F_SIGN)
#define CLR_SF (context->flag &= ~F_SIGN)
#define TST_SF (uint)(context->flag & F_SIGN)

#define SET_ZF (context->flag |= F_ZERO)
#define CLR_ZF (context->flag &= ~F_ZERO)
#define TST_ZF (uint)(context->flag & F_ZERO)

#define REGISTER_FILE ((unsigned int*)(context))
#define MEMORY_BASE ((unsigned char*)context->memory)
#define MEMORY32(X) (*((unsigned int*)(MEMORY_BASE + (X))))

// user define type
typedef unsigned int uint;
typedef unsigned char ubyte;
typedef RESULT(*pINSTRUCT)(vm_context, const uint, const uint, const uint);

static uint nog;  // F register never used

// const
static const uint uint_max = (uint)0xFFFFFFFF;

static RESULT select_reg(vm_context* context, const ubyte id, uint** reg) {
    assert(context);

    switch (id) {
        case 0x0:
            *reg = &(context->eax);
            break;
        case 0x1:
            *reg = &(context->ecx);
            break;
        case 0x2:
            *reg = &(context->edx);
            break;
        case 0x3:
            *reg = &(context->ebx);
            break;
        case 0x4:
            *reg = &(context->esp);
            break;
        case 0x5:
            *reg = &(context->ebp);
            break;
        case 0x6:
            *reg = &(context->esi);
            break;
        case 0x7:
            *reg = &(context->edi);
            break;
        case 0xF:
            *reg = &nog;
            break;
        default:
            return E_INVALID_REG_ID;
    }
    return S_OK;
}

static RESULT split_regs(vm_context* context, const ubyte reg_file, uint** reg_a, uint** reg_b) {
    if (select_reg(context, (ubyte)((reg_file & 0xF0) >> 4), reg_a)) return E_INVALID_REG_ID;
    if (select_reg(context, (ubyte)(reg_file & 0x0F), reg_b)) return E_INVALID_REG_ID;
    return S_OK;
}

extern "C" RESULT process(vm_context* context, const ubyte opt, const ubyte regs, const uint arg) {
    assert(context);
    if (context->stat == HLT) return S_HALT;
    uint *reg_a = 0;
    uint *reg_b = 0;
    uint tmp = 0;

    switch (opt) {
        case cmovg:
            if (split_regs(context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            *reg_b = (!TST_SF) ? *reg_a : *reg_b;
            break;
        case cmovge:
            if (split_regs(context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            *reg_b = (!TST_SF || TST_ZF) ? *reg_a : *reg_b;
            break;
        case cmovne:
            if (split_regs(context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            *reg_b = (!TST_ZF) ? *reg_a : *reg_b;
            break;
        case cmove:
            if (split_regs(context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            *reg_b = (TST_ZF) ? *reg_a : *reg_b;
            break;
        case cmovl:
            if (split_regs(context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            *reg_b = (TST_SF) ? *reg_a : *reg_b;
            break;
        case cmovle:
            if (split_regs(context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            // b <- a
            *reg_b = (TST_SF || TST_ZF) ? *reg_a : *reg_b;
            break;
        case jg:
            if (!TST_SF) context->pc = arg;
            break;
        case jge:
            if (!TST_SF || TST_ZF) context->pc = arg;
            break;
        case jne:
            if (!TST_ZF) context->pc = arg;
            break;
        case je:
            if (TST_ZF) context->pc = arg;
            break;
        case jl:
            if (TST_SF) context->pc = arg;
            break;
        case jle:
            if (TST_SF || TST_ZF) context->pc = arg;
            break;
        case jmp:
            context->pc = arg;
            break;
        case xorl:
            if (split_regs(context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            tmp = *reg_a ^ *reg_b;
            if ((int)tmp < 0) SET_SF; else CLR_SF;
            if (tmp == (uint)0) SET_ZF; else CLR_ZF;
            CLR_OF;
            *reg_a = tmp;
            break;
        case andl:
            if (split_regs(context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            tmp = *reg_a | *reg_b;
            if ((int)tmp < 0) SET_SF; else CLR_SF;
            if (tmp == (uint)0) SET_ZF; else CLR_ZF;
            CLR_OF;
            *reg_a = tmp;
            break;
        case subl:
            if (split_regs(context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            tmp = (uint)(*reg_a - *reg_b);
            if ((int)*reg_b > (int)*reg_a) SET_OF; else CLR_OF;
            if ((int)tmp < 0) SET_SF; else CLR_SF;
            if (tmp == (uint)0) SET_ZF; else CLR_ZF;
            *reg_a = tmp;
            break;
        case addl:
            if (split_regs(context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            tmp = (uint)(*reg_a + *reg_b);
            if ((*reg_a > uint_max - *reg_b) || (*reg_b > uint_max - *reg_a)) SET_OF; else CLR_OF;
            if ((int)tmp < 0) SET_SF; else CLR_SF;
            if (tmp == (uint)0) SET_ZF; else CLR_ZF;
            *reg_a = tmp;
            break;
        case call:
            context->pc = arg;
            break;
        case ret:
            context->pc = *(context->memory + context->esp);
            context->esp = (uint)(context->esp + sizeof(uint));
            break;
        case mrmovl:
            if (split_regs(context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            tmp = (uint)((arg + *reg_b) % context->m_size);
            *reg_a = *(uint*)(context->memory + tmp);
            break;
        case rmmovl:
            if (split_regs(context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            tmp = (uint)((arg + *reg_b) % context->m_size);
            *(uint*)(context->memory + tmp) = *reg_a;
            break;
        case irmovl:
            tmp = regs | 0xF0;
            if (split_regs(context, tmp, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            *reg_b = arg;
            break;
        case rrmovl:
            if (split_regs(context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            // b <- a
            *reg_b = *reg_a;
            break;
        case popl:
            if (split_regs(context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            *reg_a = *((uint*)(context->memory + context->esp));
            context->esp = (uint)((context->esp + (uint)sizeof(uint)) % context->m_size);
            break;
        case pushl:
			if (split_regs(context, regs, &reg_a, &reg_b)) return E_INVALID_REG_ID;
            context->esp = (uint)((context->esp - (uint)sizeof(uint)) % context->m_size);
            *(uint*)(context->memory + context->esp) = *reg_a;
            break;
        case nop:
            break;
        case halt:
            context->stat = HLT;
            return S_HALT;
        default:
            context->stat = INS;
            return E_INVALID_OPT;
    }
    context->stat = AOK;
    return S_OK;
}






static RESULT I_PUSHL(vm_context*, const uint, const uint, const uint);
static RESULT I_POPL(vm_context*, const uint, const uint, const uint);

// INS CODE 0x00
static RESULT I_HALT(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	context->stat = halt;
	return S_OK;
}

// INS CODE 0x10
static RESULT I_NOP(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	return S_OK;
}

// INS CODE 0x20
inline RESULT I_RRMOVL(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	// move the value stored in register b to register a
	REGISTER_FILE[reg_a] = REGISTER_FILE[reg_b];
	return S_OK;
}

// INS CODE 0x21
static RESULT I_CMOVLE(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	if (TST_SF || TST_ZF) return I_RRMOVL(context, reg_a, reg_b, arg);
	return S_FALSE_COND;
}

// INS CODE 0x22
static RESULT I_CMOVL(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	if (TST_SF) return I_RRMOVL(context, reg_a, reg_b, arg);
	return S_FALSE_COND;
}

// INS CODE 0x23
static RESULT I_CMOVE(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	if (TST_ZF) return I_RRMOVL(context, reg_a, reg_b, arg);
	return S_FALSE_COND;
}

// INS CODE 0x24
static RESULT I_CMOVNE(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	if (!TST_ZF) return I_RRMOVL(context, reg_a, reg_b, arg);
	return S_FALSE_COND;
}

// INS CODE 0x25
static RESULT I_CMOVGE(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	if (!TST_SF || TST_ZF) return I_RRMOVL(context, reg_a, reg_b, arg);
	return S_FALSE_COND;
}

// INS CODE 0x26
static RESULT I_CMOVG(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	if (!TST_SF) return I_RRMOVL(context, reg_a, reg_b, arg);
	return S_FALSE_COND;
}

// INS CODE 0x30
static RESULT I_IRMOVL(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	// register b <- arg
	REGISTER_FILE[reg_b] = arg;
	return S_OK;
}

// INS CODE 0x40
static RESULT I_RMMOVL(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	// arg[register b] <- register a
	uint offset = (arg + REGISTER_FILE[reg_b]) % context->m_size;
	MEMORY32(offset) = REGISTER_FILE[reg_a];
	return S_OK;
}

// INS CODE 0x50
static RESULT I_MRMOVL(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	// register a <- arg[register b]
	uint offset = (arg + REGISTER_FILE[reg_b]) % context->m_size;
	REGISTER_FILE[reg_a] = MEMORY32(offset);
	return S_OK;
}

// INS CODE 0x60
static RESULT I_ADDL(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	// register a <- register a + register b
	unsigned result = REGISTER_FILE[reg_a] + REGISTER_FILE[reg_b];
	if (result < REGISTER_FILE[reg_a] || result < REGISTER_FILE[reg_b]) SET_OF; else CLR_OF;
	if (result & (unsigned)0x80000000) SET_SF; else CLR_SF;
	if (result == (unsigned)0x0) SET_ZF; else CLR_ZF;
	REGISTER_FILE[reg_a] = result;
	return S_OK;
}

// INS CODE 0x61
static RESULT I_SUBL(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	// register a <- register a - register b
	// equval to register a + (- register b)
	REGISTER_FILE[reg_b] = ~REGISTER_FILE[reg_b] + (unsigned)0x1;
	RESULT result = I_ADDL(context, reg_a, reg_b, arg);
	REGISTER_FILE[reg_b] = ~REGISTER_FILE[reg_b] + (unsigned)0x1;
	return S_OK;
}

// INS CODE 0x62
static RESULT I_ANDL(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	// register a <- register a & register b
	unsigned result = REGISTER_FILE[reg_a] & REGISTER_FILE[reg_b];
	if (result & (unsigned)0x80000000) SET_SF; else CLR_SF;
	if (result == (unsigned)0x0) SET_ZF; else CLR_ZF;
	REGISTER_FILE[reg_a] = result;
	return S_OK;
}

// INS CODE 0x63
static RESULT I_XORL(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	// register a <- register a ^ register b
	unsigned result = REGISTER_FILE[reg_a] ^ REGISTER_FILE[reg_b];
	if (result & (unsigned)0x80000000) SET_SF; else CLR_SF;
	if (result == (unsigned)0x0) SET_ZF; else CLR_ZF;
	REGISTER_FILE[reg_a] = result;
	return S_OK;
}

// INS CODE 0x70
static RESULT I_JMP(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	if (arg >= context->m_size) return E_INVALID_ADDRESS;
	REGISTER_FILE[PC] = arg;
	return S_JMP;
}

// INS CODE 0x71
static RESULT I_JLE(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	if (TST_SF || TST_ZF) return I_JMP(context, reg_a, reg_b, arg);
	return S_FALSE_COND;
}

// INS CODE 0x72
static RESULT I_JL(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	if (TST_SF) return I_JMP(context, reg_a, reg_b, arg);
	return S_FALSE_COND;
}

// INS CODE 0x73
static RESULT I_JE(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	if (TST_ZF) return I_JMP(context, reg_a, reg_b, arg);
	return S_FALSE_COND;
}

// INS CODE 0x74
static RESULT I_JNE(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	if (!TST_ZF) return I_JMP(context, reg_a, reg_b, arg);
	return S_FALSE_COND;
}

// INS CODE 0x75
static RESULT I_JGE(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	if (!TST_SF || TST_ZF) return I_JMP(context, reg_a, reg_b, arg);
	return S_FALSE_COND;
}

// INS CODE 0x76
static RESULT I_JG(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	if (!TST_SF) return I_JMP(context, reg_a, reg_b, arg);
	return S_FALSE_COND;
}

// INS CODE 0x80
static RESULT I_CALL(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	RESULT result = I_PUSHL(context, PC, NREG, 0);
	if (S_OK != result) return result;
	result = I_JMP(context, NREG, NREG, arg);
	if (S_JMP != result) {
		I_POPL(context, NREG, NREG, 0);
		return result;
	}
	return S_JMP;
}

// INS CODE 0x90
static RESULT I_RET(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	return I_POPL(context, PC, NREG, 0);
}

// INS CODE 0xA0
static RESULT I_PUSHL(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	unsigned sp_address = (context->esp - 4) % context->m_size;
	MEMORY32(sp_address) = REGISTER_FILE[reg_a];
	context->esp = sp_address;
	return S_OK;
}

// INS CODE 0xB0
static RESULT I_POPL(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	unsigned sp_address = context->esp % context->m_size;
	REGISTER_FILE[reg_a] = MEMORY32(sp_address);
	context->esp = sp_address;
	return (PC == reg_a) ? S_JMP : S_OK;
}