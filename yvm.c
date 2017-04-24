#include <stdio.h>
#include <stdlib.h>
#include "yvm.h"

// user define type
typedef unsigned int uint;

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