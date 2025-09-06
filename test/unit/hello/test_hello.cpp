#include <gtest/gtest.h>

#include <iostream>

TEST(HelloTest, PrintMessage) {
  std::cout << "Hello, World!" << std::endl;
  EXPECT_EQ(1, 1);
}
