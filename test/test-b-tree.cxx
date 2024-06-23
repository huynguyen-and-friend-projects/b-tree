#include <gtest/gtest.h>

TEST(my_suite,check_gtest){
    std::cout << "This should compile\n";
    ASSERT_EQ(1, 1);
}
