#pragma once

#include <gtest/gtest.h>
#include "iterator.h"
#include "types.h"

TEST(IteratorTest, RangeFilterMapFold) {
    auto const filter = [](usize a) { return a % 7 == 0; };
    auto const map = [](usize a) { return a * a; };
    auto const fold = [](usize a, usize b) { return a + b; };

    ASSERT_EQ(Iter::range(0, 100)
                      .filter(filter)
                      .map(map)
                      .fold(fold, 0), 49735);
}

TEST(IteratorTest, LazyForEach) {
    usize sum = 0;
    ASSERT_NO_FATAL_FAILURE(Iter::range(100).lazyForEach([&sum](auto const &i) { sum += i; }).exhaust());
    ASSERT_EQ(sum, 4950);
}

TEST(IteratorTest, FoldStatePassing) {
    auto state_passer = [](usize a, auto state) { return state; };
    auto initial_data =std::vector<usize>{1, 2, 3, 4, 5, 6, 7, 8, 9, 111};
    ASSERT_EQ(
            Iter::range(1000).fold(state_passer, initial_data),
            (std::vector<usize>{1, 2, 3, 4, 5, 6, 7, 8, 9, 111})
    );
}