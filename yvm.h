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
typedef enum {
    AOK = 1,
    HLT,
    ADR,
    INS
} STATUS;


// function

void process(const unsigned char opt, const unsigned char regs, const unsigned int arg);


#endif // _YVM_H_