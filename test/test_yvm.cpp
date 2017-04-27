#include <gtest/gtest.h>
#include "yvm.h"

TEST(test_yvm, SET_CONTEXT) {
    vm_context* context = nullptr;
    context = new vm_context;
    ASSERT_TRUE(context);
    set_context(context);
    delete context;
}

TEST(test_yvm, SET_AND_READ_RIGISTER) {
    vm_context* context = new vm_context;
    set_context(context);

    unsigned int expect_a = 0x0000ABCD;
    unsigned int expect_b = 0xABCD0000;
    EXPECT_EQ(S_OK, process(irmovl, 0xF0, expect_a));
    EXPECT_EQ(S_OK, process(irmovl, 0xF3, expect_b));

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
    set_context(context);

    unsigned int base_address = (unsigned int)(0x1 << 15); // base address 16k
    int* ptr_write = (int*)(context->memory + base_address);
    for (int i = 0; i < 10; ++i) {
        ptr_write[i] = i + 1;
    }

    EXPECT_EQ(S_OK, process(irmovl, 0xF3, (unsigned int)(5 * sizeof(int))));
    EXPECT_EQ(S_OK, process(mrmovl, 0x03, base_address));
    EXPECT_EQ(ptr_write[5], context->eax);

    int* ptr_start = (int*)context->memory;
    ptr_start[0] = (int)0xFFFF;

    context->eax = 0x0;
    context->ecx = 0x0;
    context->esi = 0x0;
    EXPECT_EQ(S_OK, process(mrmovl, 0x06, 0x0));
    EXPECT_EQ(S_OK, process(mrmovl, 0x16, (unsigned int)context->m_size));
    EXPECT_EQ(0xFFFF, context->eax);
    EXPECT_EQ(0xFFFF, context->ecx);

    EXPECT_EQ(E_INVALID_REG_ID, process(mrmovl, 0x0C, 0x0));

    // clean up
    delete [] memory;
    delete context;
}

TEST(test_yvm, OPT_ERROR) {
    vm_context* context = nullptr;
    context = new vm_context;
    ASSERT_TRUE(context);
    set_context(context);

    EXPECT_EQ(E_INVALID_OPT, process(0xFF, 0x00, 0x00));

    delete context;
}

TEST(test_yvm, OPT_HALT) {
    vm_context* context = nullptr;
    context = new vm_context;
    ASSERT_TRUE(context);
    set_context(context);

    EXPECT_EQ(S_HALT, process(halt, 0x00, 0x0));

    delete context;
}
