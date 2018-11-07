#include <gtest/gtest.h>
#include "test_parser.h"
#include "test_iterator.h"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
