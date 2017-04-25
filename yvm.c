#include <stdio.h>
#include <stdlib.h>
#include "yvm.h"

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


static void select_reg(const ubyte id, uint** reg) {
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
            break;
    }
    return;
}

static void split_regs(const ubyte reg_file, uint** reg_a, uint** reg_b) {
    ubyte id_b = (ubyte)(reg_file & 0x0F);
    ubyte id_a = (ubyte)((reg_file & 0xF0) >> 4);
    select_reg(id_a, reg_a);
    select_reg(id_b, reg_b);
}


// void process(const ubyte opt, const ubyte regs, const uint arg) {
//     if (stat == STATUS.HLT) return;
//     switch (opt) {
//         case pushl:
//             *(--esp) = arg;
//             stat = STATUS.AOK;
//             break;
//         case nop:
//             break;
//         case halt:
//             stat = STATUS.HLT;
//             break
//         default:
//             stat = STATUS.INS;
//             break;
//     }
// }