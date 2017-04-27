#include <gtest/gtest.h>
#include "yvm.h"

TEST(test_yvm, OPT_HALT) {
    EXPECT_EQ(S_HALT, process(halt, 0x00, 0x0));
}

TEST(test_yvm, OPT_ERROR) {
    EXPECT_EQ(E_INVALID_OPT, process(0xFF, 0x00, 0x00));
}
