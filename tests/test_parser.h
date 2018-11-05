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

using std::vector;
using std::string;

TEST(ParserTest, PersonParser) {
    ASSERT_EQ(line_parser({"def", "person", "Pepik"}), file_mapping::Person("Pepik", {}));
    ASSERT_EQ(line_parser({"def", "person", "a", "b", "c"}), file_mapping::Person("a", {"b", "c"}));
    ASSERT_ANY_THROW(line_parser({"def", "person"}));
}

TEST(ParserTest, GroupParser) {
    ASSERT_EQ(line_parser({"def", "group", "Pepik"}), file_mapping::Group("Pepik", {}));
    ASSERT_EQ(line_parser({"def", "group", "a", "b", "c"}), file_mapping::Group("a", {"b", "c"}));
    ASSERT_ANY_THROW(line_parser({"def", "group"}));
}

TEST(ParserTest, CurrencyParser) {
    ASSERT_EQ(line_parser({"def", "currency", "usd"}), file_mapping::Currency("usd"));
    ASSERT_ANY_THROW(line_parser({"def", "currency"}));
    ASSERT_ANY_THROW(line_parser({"def", "currency", "usd", "lol"}));
}

TEST(ParserTest, TransactionParser0) {
    ASSERT_EQ(line_parser({"a", "paid", "5usd", "for", "b"}), file_mapping::Transaction({"a"}, {5, "usd"}, {"b"}));
}

TEST(ParserTest, TransactionParser1) {
    ASSERT_EQ(line_parser({"a", "b", "c", "paid", "5usd", "for", "b"}),
              file_mapping::Transaction({"a", "b", "c"}, {5, "usd"}, {"b"}));
}

TEST(ParserTest, TransactionParser2) {
    ASSERT_EQ(line_parser({"a", "paid", "5usd", "for", "b", "a", "c"}),
              file_mapping::Transaction({"a"}, {5, "usd"}, {"b", "a", "c"}));
}

TEST(ParserTest, TransactionParser3) { ASSERT_ANY_THROW(line_parser({"a", "5usd", "for", "b"})); }

TEST(ParserTest, TransactionParser4) { ASSERT_ANY_THROW(line_parser({"a", "paid", "5usd", "b"})); }

TEST(ParserTest, TransactionParser5) { ASSERT_ANY_THROW(line_parser({"a", "paid", "usd", "for", "b"})); }

TEST(ParserTest, TransactionParser6) { ASSERT_ANY_THROW(line_parser({"a", "paid", "5", "for", "b"})); }
