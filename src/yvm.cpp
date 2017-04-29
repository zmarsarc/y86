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
#define MEMORY32(X) (*((unsigned int*)(mem(context, (X)))))

#define HASSAMESIGN(X, Y) (((X) < 0 && (Y) < 0)||((X) >= 0 && (Y) >= 0))

// user define type
typedef unsigned int uint;
typedef unsigned char ubyte;
typedef RESULT(*pINSTRUCT)(vm_context*, const uint, const uint, const uint);

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

static bool isValidRegister[] = {
	true, true, true, true, true, true, true, true,
	true, true, true, false, false, false, false, true
};

static pINSTRUCT instructs[16][16] {
	{(pINSTRUCT)I_HALT,},
	{(pINSTRUCT)I_NOP,},
	{(pINSTRUCT)I_RRMOVL, (pINSTRUCT)I_CMOVLE, (pINSTRUCT)I_CMOVL, (pINSTRUCT)I_CMOVE, (pINSTRUCT)I_CMOVNE, (pINSTRUCT)I_CMOVGE, (pINSTRUCT)I_CMOVG,},
	{(pINSTRUCT)I_IRMOVL,},
	{(pINSTRUCT)I_RMMOVL,},
	{(pINSTRUCT)I_MRMOVL,},
	{(pINSTRUCT)I_ADDL, (pINSTRUCT)I_SUBL, (pINSTRUCT)I_ANDL, (pINSTRUCT)I_XORL,},
	{(pINSTRUCT)I_JMP, (pINSTRUCT)I_JLE, (pINSTRUCT)I_JL, (pINSTRUCT)I_JE, (pINSTRUCT)I_JNE, (pINSTRUCT)I_JGE, (pINSTRUCT)I_JG,},
	{(pINSTRUCT)I_CALL,},
	{(pINSTRUCT)I_RET,},
	{(pINSTRUCT)I_PUSHL,},
	{(pINSTRUCT)I_POPL,},
	{nullptr},
	{nullptr},
	{nullptr},
	{nullptr},
};


extern "C" RESULT process(vm_context* context, const ubyte opt, const ubyte regs, const uint arg) {
	if (context->stat == HLT) return S_HALT;
	if (context->stat == INS) return E_INVALID_OPT;

	uint reg_a_id = regs >> 4;
	uint reg_b_id = regs & 0xF;
	if (!isValidRegister[reg_a_id] || !isValidRegister[reg_b_id]) return E_INVALID_REG_ID;

	pINSTRUCT ins = ((pINSTRUCT*)instructs)[opt];
	if (ins == nullptr) {
		context->stat = INS;
		return E_INVALID_OPT;
	}
	uint result = ins(context, reg_a_id, reg_b_id, arg);

	if (result & S_OK) {
		context->stat = AOK;
		return result;
	}
	return result;
}

static void* mem(vm_context* context, uint offset) {
	return (void*)(context->memory + (offset % context->m_size));
}

#pragma region INSTRUCTS
// ALL INSTRUCTS DEFINED AS FELLOWING

// INS CODE 0x00
static RESULT I_HALT(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	context->stat = HLT;
	return S_HALT;
}

// INS CODE 0x10
static RESULT I_NOP(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	return S_OK;
}

// INS CODE 0x20
static RESULT I_RRMOVL(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	// move the value stored in register b to register a
	REGISTER_FILE[reg_a] = REGISTER_FILE[reg_b];
	return (reg_a == PC) ? S_JMP : S_OK;  // write register_pc will trigger jmp
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
	return (reg_b == PC) ? S_JMP : S_OK;
}

// INS CODE 0x40
static RESULT I_RMMOVL(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	// arg[register b] <- register a
	uint offset = arg + REGISTER_FILE[reg_b];
	MEMORY32(offset) = REGISTER_FILE[reg_a];
	return S_OK;
}

// INS CODE 0x50
static RESULT I_MRMOVL(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	// register a <- arg[register b]
	uint offset = arg + REGISTER_FILE[reg_b];
	REGISTER_FILE[reg_a] = MEMORY32(offset);
	return (reg_a == PC) ? S_JMP : S_OK;
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
	return (reg_a == PC) ? S_JMP : S_OK;
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
	return (reg_a == PC) ? S_JMP : S_OK;
}

// INS CODE 0x62
static RESULT I_ANDL(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	// register a <- register a & register b
	int result = (int)REGISTER_FILE[reg_a] & (int)REGISTER_FILE[reg_b];
	if (result < (int)0) SET_SF; else CLR_SF;
	if (result == (int)0) SET_ZF; else CLR_ZF;
	REGISTER_FILE[reg_a] = (uint)result;
	return (PC == reg_a) ? S_JMP : S_OK;
}

// INS CODE 0x63
static RESULT I_XORL(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	// register a <- register a ^ register b
	int result = (int)REGISTER_FILE[reg_a] ^ (int)REGISTER_FILE[reg_b];
	if (result < (int)0) SET_SF; else CLR_SF;
	if (result == (int)0) SET_ZF; else CLR_ZF;
	REGISTER_FILE[reg_a] = (uint)result;
	return (PC == reg_a) ? S_JMP : S_OK;
}

// INS CODE 0x70
static RESULT I_JMP(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
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
	return I_JMP(context, NREG, NREG, arg);
}

// INS CODE 0x90
static RESULT I_RET(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	return I_POPL(context, PC, NREG, 0);
}

// INS CODE 0xA0
static RESULT I_PUSHL(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	unsigned sp_address = (context->esp - 4);
	MEMORY32(sp_address) = REGISTER_FILE[reg_a];
	context->esp = sp_address;
	return S_OK;
}

// INS CODE 0xB0
static RESULT I_POPL(vm_context* context, const uint reg_a, const uint reg_b, const uint arg) {
	unsigned sp_address = context->esp;
	REGISTER_FILE[reg_a] = MEMORY32(context->esp);
	context->esp += (uint)sizeof(uint);
	return (PC == reg_a) ? S_JMP : S_OK;
}

// END OF INSTRUCTS DEFINIATIONS

#pragma endregion