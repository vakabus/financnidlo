#pragma once

#include<gtest/gtest.h>
#include "parser.h"
#include <vector>
#include <string>

TEST(TokenSplitter, EmptyString) {
	auto e = std::vector<std::string>{};
	ASSERT_EQ(token_splitter(""), e);
	ASSERT_EQ(token_splitter("    "), e);
}

TEST(TokenSplitter, SingleToken) {
	auto r = std::vector<std::string>{"token"};
	ASSERT_EQ(token_splitter("token"), r);
	ASSERT_EQ(token_splitter("   token"), r);
	ASSERT_EQ(token_splitter("token   "), r);
	ASSERT_EQ(token_splitter("  token  "), r);
}

TEST(TokenSplitter, MultiToken) {
	auto r = std::vector<std::string>{"1", "2"};
	ASSERT_EQ(token_splitter("  1       2    "), r);
}
