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

#define HASSAMESIGN(X, Y) (((X) < 0 && (Y) < 0)||((X) >= 0 && (Y) >= 0))

// user define type
typedef unsigned int uint;
typedef unsigned char ubyte;
typedef RESULT(*pINSTRUCT)(vm_context, const uint, const uint, const uint);


static bool isValidRegister[] = {
	true, true, true, true, true, true, true, true,
	true, true, true, false, false, false, false, true
};

static RESULT I_HALT(vm_context*, const uint, const uint, const uint);
static RESULT I_NOP(vm_context*, const uint, const uint, const uint);
static RESULT I_RRMOVL(vm_context*, const uint, const uint, const uint);
static RESULT I_CMOVLE(vm_context*, const uint, const uint, const uint);
static RESULT I_CMOVL(vm_context*, const uint, const uint, const uint);
static RESULT I_CMOVE(vm_context*, const uint, const uint, const uint);
static RESULT I_CMOVNE(vm_context*, const uint, const uint, const uint);
static RESULT I_CMOVGE(vm_context*, const uint, const uint, const uint);
static RESULT I_CMOVG(vm_context*, const uint, const uint, const uint);
static RESULT I_IRMOVL(vm_context*, const uint, const uint, const uint);
static RESULT I_RMMOVL(vm_context*, const uint, const uint, const uint);
static RESULT I_MRMOVL(vm_context*, const uint, const uint, const uint);
static RESULT I_ADDL(vm_context*, const uint, const uint, const uint);
static RESULT I_SUBL(vm_context*, const uint, const uint, const uint);
static RESULT I_ANDL(vm_context*, const uint, const uint, const uint);
static RESULT I_XORL(vm_context*, const uint, const uint, const uint);
static RESULT I_JMP(vm_context*, const uint, const uint, const uint);
static RESULT I_JLE(vm_context*, const uint, const uint, const uint);
static RESULT I_JL(vm_context*, const uint, const uint, const uint);
static RESULT I_JE(vm_context*, const uint, const uint, const uint);
static RESULT I_JNE(vm_context*, const uint, const uint, const uint);
static RESULT I_JGE(vm_context*, const uint, const uint, const uint);
static RESULT I_JG(vm_context*, const uint, const uint, const uint);
static RESULT I_CALL(vm_context*, const uint, const uint, const uint);
static RESULT I_RET(vm_context*, const uint, const uint, const uint);
static RESULT I_PUSHL(vm_context*, const uint, const uint, const uint);
static RESULT I_POPL(vm_context*, const uint, const uint, const uint);

extern "C" RESULT process(vm_context* context, const ubyte opt, const ubyte regs, const uint arg) {
	if (context->stat == HLT) return S_HALT;

	uint reg_a_id = regs >> 4;
	uint reg_b_id = regs & 0xF;
	if (!isValidRegister[reg_a_id] || !isValidRegister[reg_b_id]) return E_INVALID_REG_ID;

	switch (opt) {
	case cmovg: return I_CMOVG(context, reg_a_id, reg_b_id, arg);
	case cmovge: return I_CMOVGE(context, reg_a_id, reg_b_id, arg);
	case cmovne: return I_CMOVNE(context, reg_a_id, reg_b_id, arg);
	case cmove: return I_CMOVE(context, reg_a_id, reg_b_id, arg);
	case cmovl: return I_CMOVL(context, reg_a_id, reg_b_id, arg);
	case cmovle: return I_CMOVLE(context, reg_a_id, reg_b_id, arg);
	case jg: return I_JG(context, reg_a_id, reg_b_id, arg);
	case jge: return I_JGE(context, reg_a_id, reg_b_id, arg);
	case jne: return I_JNE(context, reg_a_id, reg_b_id, arg);
	case je: return I_JE(context, reg_a_id, reg_b_id, arg);
	case jl: return I_JL(context, reg_a_id, reg_b_id, arg);
	case jle: return I_JLE(context, reg_a_id, reg_b_id, arg);
	case jmp: return I_JMP(context, reg_a_id, reg_b_id, arg);
	case xorl: return I_XORL(context, reg_a_id, reg_b_id, arg);
	case andl: return I_ANDL(context, reg_a_id, reg_b_id, arg);
	case subl: return I_SUBL(context, reg_a_id, reg_b_id, arg);
	case addl: return I_ADDL(context, reg_a_id, reg_b_id, arg);
	case call: return I_CALL(context, reg_a_id, reg_b_id, arg);
	case ret: return I_RET(context, reg_a_id, reg_b_id, arg);
	case mrmovl: return I_MRMOVL(context, reg_a_id, reg_b_id, arg);
	case rmmovl: return I_IRMOVL(context, reg_a_id, reg_b_id, arg);
	case irmovl: return I_IRMOVL(context, reg_a_id, reg_b_id, arg);
	case rrmovl: return I_RRMOVL(context, reg_a_id, reg_b_id, arg);
	case popl: return I_POPL(context, reg_a_id, reg_b_id, arg);
	case pushl: return I_PUSHL(context, reg_a_id, reg_b_id, arg);
	case nop: return I_NOP(context, reg_a_id, reg_b_id, arg);
	case halt: return I_HALT(context, reg_a_id, reg_b_id, arg);
	default:
		context->stat = INS;
		return E_INVALID_OPT;
	}
	context->stat = AOK;
	return S_OK;
}

// INS CODE 0x00
static RESULT I_HALT(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	context->stat = HLT;
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
	register int a = (int)REGISTER_FILE[reg_a];
	register int b = (int)REGISTER_FILE[reg_b];
	register int result = a + b;
	
	// overflow ?
	if (HASSAMESIGN(a, b)) {
		if ((a >= (int)0 && b >= (int)0 && result < (int)0) ||
			(a < (int)0 && b < (int)0 && result >= (int)0)) SET_OF;
		else CLR_OF;
	}
	else {
		CLR_OF;
	}

	if (result < (int)0) SET_SF; else CLR_SF;
	if (result == (int)0x0) SET_ZF; else CLR_ZF;
	REGISTER_FILE[reg_a] = (uint)result;
	return S_OK;
}

// INS CODE 0x61
static RESULT I_SUBL(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	// register a <- register a - register b
	
	register int a = (int)REGISTER_FILE[reg_a];
	register int b = (int)REGISTER_FILE[reg_b];
	register int result = a - b;

	// overflow ?
	if (!HASSAMESIGN(a, b)) {
		if ((a >= (int)0 && b < (int)0 && result < (int)0) ||
			(a < (int)0 && a >= (int)0 && result >= (int)0)) SET_OF;
	}
	else {
		CLR_OF;
	}

	if (result < (int)0) SET_SF; else CLR_SF;
	if (result == (int)0) SET_ZF; else CLR_ZF;
	REGISTER_FILE[reg_a] = (uint)result;
	return S_OK;
}

// INS CODE 0x62
static RESULT I_ANDL(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	// register a <- register a & register b
	int result = (int)REGISTER_FILE[reg_a] & (int)REGISTER_FILE[reg_b];
	if (result < (int)0) SET_SF; else CLR_SF;
	if (result == (int)0) SET_ZF; else CLR_ZF;
	REGISTER_FILE[reg_a] = (uint)result;
	return S_OK;
}

// INS CODE 0x63
static RESULT I_XORL(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	// register a <- register a ^ register b
	int result = (int)REGISTER_FILE[reg_a] ^ (int)REGISTER_FILE[reg_b];
	if (result < (int)0) SET_SF; else CLR_SF;
	if (result == (int)0) SET_ZF; else CLR_ZF;
	REGISTER_FILE[reg_a] = (uint)result;
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
	context->esp = sp_address + (uint)sizeof(uint);
	return (PC == reg_a) ? S_JMP : S_OK;
}