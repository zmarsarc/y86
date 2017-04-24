#include <stdio.h>
#include <stdlib.h>

// user define type
typedef unsigned int uint;

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


// stat code
#define AOK 1
#define HLT 2  
#define ADR 3  // invalid memory address
#define INS 4  // invalid operation


// register file
static uint eax;
static uint ebx;
static uint ecx;
static uint edx;
static uint esi;
static uint edi;
static uint esp;
static uint ebp;

static uint pc;
static uint stat;
static uint flag;

static char* memory;
static uint  m_size;


#define OUT_OF_MEM 1

uint bin_loader(FILE* fp);

int main(const int argc, char* argv[]) {
    
    if (argc < 2) {
        printf("Usage: yvm binary_file_path");
        exit(0);
    }

    m_size = (uint)(0x01 << 16);  // 32K memory
    if (!(memory = (char*)malloc(sizeof(char) * m_size))) {
        printf("memory alloc error\n");
        exit(OUT_OF_MEM);
    }
    


    // clearup
    free(memory);
    memory = NULL;
    return 0;
}

uint bin_loader(FILE* fp, /*out*/ size_t* file_size) {
    size_t read_in = 1;

    while (read_in != m_size) {
        if (fread(memory, 1, 1, fp)) {
            read_in++;
            continue;
        }
        *file_size = read_in - 1;
        return (uint)0;
    }
    return (uint)OUT_OF_MEM;
}