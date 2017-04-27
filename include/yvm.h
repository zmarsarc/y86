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

// status code
#define AOK 1
#define HLT 2
#define ADR 3
#define INS 4

typedef unsigned int RESULT;
#define S_OK              (RESULT)0x0000
#define S_HALT            (RESULT)(S_OK | 0x0001)
#define E_ERROR           (RESULT) 0x1000
#define E_INVALID_REG_ID  (RESULT)(E_ERROR | 0x0001)
#define E_INVALID_OPT     (RESULT)(E_ERROR | 0x0002)


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
    unsigned int pc;
    unsigned int flag;
    unsigned int stat;
    unsigned int* memory;
    unsigned int m_size;
} vm_context;


// function
extern "C" void set_context(vm_context*);
extern "C" RESULT process(const unsigned char opt, const unsigned char regs, const unsigned int arg);


#endif // _YVM_H_