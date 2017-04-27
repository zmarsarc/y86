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

TEST(test_yvm, OPT_ERROR) {
    EXPECT_EQ(E_INVALID_OPT, process(0xFF, 0x00, 0x00));
}

TEST(test_yvm, OPT_HALT) {
    EXPECT_EQ(S_HALT, process(halt, 0x00, 0x0));
}
