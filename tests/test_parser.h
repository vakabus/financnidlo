#pragma once

#include<gtest/gtest.h>
#include "parser.h"
#include <vector>
#include <string>

TEST(ParserTest, TokenSplitterEmptyString) {
	auto e = std::vector<std::string>{};
	ASSERT_EQ(token_splitter(""), e);
	ASSERT_EQ(token_splitter("    "), e);
}

TEST(ParserTest, TokenSplitterSingleToken) {
	auto r = std::vector<std::string>{"token"};
	ASSERT_EQ(token_splitter("token"), r);
	ASSERT_EQ(token_splitter("   token"), r);
	ASSERT_EQ(token_splitter("token   "), r);
	ASSERT_EQ(token_splitter("  token  "), r);
}

TEST(ParserTest, TokenSplitterMultiToken) {
	auto r = std::vector<std::string>{"1", "2"};
	ASSERT_EQ(token_splitter("  1       2    "), r);
}

TEST(ParserTest, EmptyListFilter) {
	ASSERT_FALSE(empty_list_filter(std::string{""}));
	ASSERT_TRUE(empty_list_filter(std::string{" "}));
	ASSERT_FALSE(empty_list_filter(std::vector<int>{}));
	ASSERT_TRUE(empty_list_filter(std::vector<int>{4}));
}


