#ifndef _YVM_H_
#define _YVM_H_

// operation code define 

#define halt     0x00
#define nop      0x10

// condition move
#define cmv_mask 0x20
#define rrmovl   (cmv_mask | 0x00)
#define cmovle   (cmv_mask | 0x01)
#define cmovl    (cmv_mask | 0x02)
#define cmove    (cmv_mask | 0x03)
#define cmovne   (cmv_mask | 0x04)
#define cmovge   (cmv_mask | 0x05)
#define cmovg    (cmv_mask | 0x06)

#define irmovl   0x30
#define rmmovl   0x40
#define mrmovl   0x50

// numric and logic operation
#define opl_mask 0x60
#define addl     (opl_mask | 0x00)
#define subl     (opl_mask | 0x01)
#define andl     (opl_mask | 0x02)
#define xorl     (opl_mask | 0x03)

// condition jump
#define jmp_mask 0x70
#define jmp      (jmp_mask | 0x00)
#define jle      (jmp_mask | 0x01)
#define jl       (jmp_mask | 0x02)
#define je       (jmp_mask | 0x03)
#define jne      (jmp_mask | 0x04)
#define jge      (jmp_mask | 0x05)
#define jg       (jmp_mask | 0x06)

#define call     0x80
#define ret      0x90

#define pushl    0xa0
#define popl     0xb0

// register if
#define EAX (unsigned)0x0
#define ECX (unsigned)0x1
#define EDX (unsigned)0x2
#define EBX (unsigned)0x3
#define ESP (unsigned)0x4
#define EBP (unsigned)0x5
#define ESI (unsigned)0x6
#define EDI (unsigned)0x7
#define PC  (unsigned)0x8
#define FLAG (unsigned)0x9
#define STAT (unsigned)0xA
#define NREG (unsigned)0xF

// flag mask
#define F_OVER (unsigned int)0x00000001
#define F_SIGN (unsigned int)0x00000002
#define F_ZERO (unsigned int)0x00000004

// status code
#define AOK 1
#define HLT 2
#define ADR 3
#define INS 4

typedef unsigned int RESULT;
#define S_OK              (RESULT)0x0000
#define S_HALT            (RESULT)(S_OK | 0x0001)
#define S_JMP             (RESULT)(S_OK | 0x0002)
#define S_FALSE_COND      (RESULT)(S_OK | 0x0003)
#define E_ERROR           (RESULT) 0x1000
#define E_INVALID_REG_ID  (RESULT)(E_ERROR | 0x0001)
#define E_INVALID_OPT     (RESULT)(E_ERROR | 0x0002)
#define E_INVALID_ADDRESS (RESULT)(E_ERROR | 0x0003)


// context file
typedef struct __vm_contetxt {
    unsigned int eax;  // 0
    unsigned int ecx;  // 1
    unsigned int edx;  // 2
    unsigned int ebx;  // 3
    unsigned int esp;  // 4
    unsigned int ebp;  // 5
    unsigned int esi;  // 6
    unsigned int edi;  // 7
    unsigned int pc;   // A
    unsigned int flag; // B
    unsigned int stat; // C
	
	// reserved register address
	unsigned int __r1; // D
	unsigned int __r2; // E
	unsigned int __r3; // F

	// memory pointer
    unsigned char* memory;
    unsigned int m_size;
} vm_context;


// function
extern "C" RESULT process(vm_context*, const unsigned char opt, const unsigned char regs, const unsigned int arg);


#endif // _YVM_H_