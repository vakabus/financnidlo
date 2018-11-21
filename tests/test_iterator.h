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
    ASSERT_NO_FATAL_FAILURE(Iter::range(100).lazy_for_each([&sum](auto const &i) { sum += i; }).exhaust());
    ASSERT_EQ(sum, 4950);
}

TEST(IteratorTest, FoldStatePassing) {
    auto constexpr state_passer = [](usize a, auto state) { return state; };
    auto initial_data = std::vector<usize>{1, 2, 3, 4, 5, 6, 7, 8, 9, 111};

    auto result = Iter::range(1000).fold(state_passer, move(initial_data));
    ASSERT_EQ(
            result,
            (std::vector<usize>{1, 2, 3, 4, 5, 6, 7, 8, 9, 111})
    );
}

TEST(IteratorTest, ZipIterator) {
    auto constexpr mapper = [](auto pair) { return pair.first; };
    ASSERT_EQ(Iter::range(100).enumerate().map(mapper).sum(), 4950);
}

TEST(IteratorTest, ClassicalIteratorCompatibility1) {
    usize accum = 0;
    auto iter = Iter::range(10);
    auto start = iter.begin();
    auto en = iter.end();
    while (start != en) {
        accum += *start;
        ++start;
    }
    ASSERT_EQ(accum, 45);
}

TEST(IteratorTest, ClassicalIteratorCompatibility2) {
    usize accum = 0;
    for (auto && i : Iter::range(10)) {
        accum += i;
    }
    ASSERT_EQ(accum, 45);
}