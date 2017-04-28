#include <gtest/gtest.h>
#include <iostream>
#include "yvm.h"

TEST(test_yvm, SET_AND_READ_RIGISTER) {
    vm_context* context = new vm_context;

    unsigned int expect_a = 0x0000ABCD;
    unsigned int expect_b = 0xABCD0000;
    EXPECT_EQ(S_OK, process(context, irmovl, 0xF0, expect_a));
    EXPECT_EQ(S_OK, process(context, irmovl, 0xF3, expect_b));

    EXPECT_EQ(expect_a, context->eax);
    EXPECT_EQ(expect_b, context->ebx);

    delete context;
}

TEST(test_yvm, ACCESS_MEMORY) {
    // init context
    vm_context* context = nullptr;
    context = new vm_context;
    ASSERT_TRUE(context);
    
    // init memory
    context->m_size = (unsigned int)(0x1 << 16); // 32K memory
    unsigned char* memory = new unsigned char[context->m_size];
    ASSERT_TRUE(memory);
    context->memory = memory;

    unsigned int base_address = (unsigned int)(0x1 << 15); // base address 16k
    int* ptr_write = (int*)(context->memory + base_address);
    for (int i = 0; i < 10; ++i) {
        ptr_write[i] = i + 1;
    }

    EXPECT_EQ(S_OK, process(context, irmovl, 0xF3, (unsigned int)(5 * sizeof(int))));
    EXPECT_EQ(S_OK, process(context, mrmovl, 0x03, base_address));
    EXPECT_EQ(ptr_write[5], context->eax);

    int* ptr_start = (int*)context->memory;
    ptr_start[0] = (int)0xFFFF;

    context->eax = 0x0;
    context->ecx = 0x0;
    context->esi = 0x0;
    EXPECT_EQ(S_OK, process(context, mrmovl, 0x06, 0x0));
    EXPECT_EQ(S_OK, process(context, mrmovl, 0x16, (unsigned int)context->m_size));
    EXPECT_EQ(0xFFFF, context->eax);
    EXPECT_EQ(0xFFFF, context->ecx);

    EXPECT_EQ(E_INVALID_REG_ID, process(context, mrmovl, 0x0C, 0x0));

    // clean up
    delete [] memory;
    delete context;
}

TEST(test_yvm, OPERATORS) {
    vm_context* context = nullptr;
    context = new vm_context;
    ASSERT_TRUE(context);

    context->eax = 0x1200;
    context->ebx = 0x0034;

    EXPECT_EQ(S_OK, process(context, addl, 0x03, 0));
    EXPECT_EQ((unsigned int)0x1234, context->eax);
    EXPECT_FALSE(context->flag & F_OVER);
    EXPECT_FALSE(context->flag & F_SIGN);
    EXPECT_FALSE(context->flag & F_ZERO);

    EXPECT_EQ(S_OK, process(context, subl, 0x03, 0));
    EXPECT_EQ((unsigned int)0x1200, context->eax);
    EXPECT_FALSE(context->flag & F_OVER);
    EXPECT_FALSE(context->flag & F_SIGN);
    EXPECT_FALSE(context->flag & F_ZERO);

    context->eax = 0xFF00;
    context->ebx = 0x00FF;

    EXPECT_EQ(S_OK, process(context, andl, 0x03, 0));
    EXPECT_EQ((unsigned int)0xFFFF, context->eax);
    EXPECT_FALSE(context->flag & F_OVER);
    EXPECT_FALSE(context->flag & F_SIGN);
    EXPECT_FALSE(context->flag & F_ZERO);

    EXPECT_EQ(S_OK, process(context, xorl, 0x03, 0));
    EXPECT_EQ((unsigned int)0xFF00, context->eax);
    EXPECT_FALSE(context->flag & F_OVER);
    EXPECT_FALSE(context->flag & F_SIGN);
    EXPECT_FALSE(context->flag & F_ZERO);

    context->eax = 0;
    context->ebx = 0;

    EXPECT_EQ(S_OK, process(context, xorl, 0x00, 0));
    EXPECT_FALSE(context->flag & F_OVER);
    EXPECT_FALSE(context->flag & F_SIGN);
    EXPECT_TRUE(context->flag & F_ZERO);

    context->ebx = 1;
    EXPECT_EQ(S_OK, process(context, subl, 0x03, 0));
    EXPECT_TRUE(context->flag & F_OVER);
    EXPECT_TRUE(context->flag & F_SIGN);
    EXPECT_FALSE(context->flag & F_ZERO);

    context->eax = 0xFFFFF;
    context->ebx = 0xFFFFF000;
    EXPECT_EQ(S_OK, process(context, addl, 0x03, 0));
    EXPECT_TRUE(context->flag & F_OVER);
    EXPECT_FALSE(context->flag & F_SIGN);
    EXPECT_FALSE(context->flag & F_ZERO);

    delete context;
}

TEST(test_yvm, OPT_ERROR) {
    vm_context* context = nullptr;
    context = new vm_context;
    ASSERT_TRUE(context);

    EXPECT_EQ(E_INVALID_OPT, process(context, 0xFF, 0x00, 0x00));

    delete context;
}

TEST(test_yvm, PUSH_POP) {
    vm_context* context = nullptr;
    context = new vm_context;
    ASSERT_TRUE(context);

    unsigned int m_size = (unsigned int)(0x1 << 16); // 32K
    unsigned char* memory = new unsigned char[m_size];
    ASSERT_TRUE(memory);

    context->memory = memory;
    context->m_size = m_size;

    // setup stack pointer
    unsigned int stack_base = (unsigned int)(0x1 << 15); // 16K
    context->esp = stack_base;
    context->ebp = stack_base;

    EXPECT_EQ(S_OK, process(context, irmovl, 0x00, 0x12345678));
    EXPECT_EQ(S_OK, process(context, pushl, 0x00, 0x0));
    EXPECT_EQ(stack_base - sizeof(unsigned), context->esp);

    EXPECT_EQ(S_OK, process(context, popl, 0x30, 0x0));
    EXPECT_EQ((unsigned)0x12345678, context->ebx);
    EXPECT_EQ(stack_base, context->esp);

    delete [] memory;
    delete context;
}

TEST(test_yvm, MEMORY_WRITE) {
    vm_context* context = nullptr;
    context = new vm_context;
    ASSERT_TRUE(context);

    size_t m_size = (size_t)(0x1 << 16);
    unsigned char* memory = new unsigned char[m_size];
    ASSERT_TRUE(memory);
    context->memory = memory;
    context->m_size = (unsigned int)m_size;

    context->eax = 0xCC;
    context->ecx = 0x0;
    for (size_t i = 0; i != m_size - 4; i += 4) {
        context->ecx += (unsigned int)i;
        EXPECT_EQ(S_OK, process(context, rmmovl, 0x01, 0x0)) << "current ecx is: " << context->ecx;
    } 

    delete [] memory;
    delete context;
}

TEST(test_yvm, OPT_HALT) {
    vm_context* context = nullptr;
    context = new vm_context;
    ASSERT_TRUE(context);

    EXPECT_EQ(S_HALT, process(context, halt, 0x00, 0x0));

    delete context;
}
