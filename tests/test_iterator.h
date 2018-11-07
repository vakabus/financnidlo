#pragma once

#include <gtest/gtest.h>
#include "iterator.h"
#include "types.h"

TEST(IteratorTest, RangeFilterMapFold) {
    auto const filter = [](usize a) { return a % 7 == 0; };
    auto const map = [](usize a) { return a * a; };
    auto const fold = [](usize a, usize b) { return a +b; };

    ASSERT_EQ(Iter::range(0, 100)
                      .filter(filter)
                      .map(map)
                      .fold(fold, 0), 49735);
}