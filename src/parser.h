#pragma once

#include "types.h"
#include <string>
#include <set>
#include <vector>
#include <cassert>
#include "model.h"

auto constexpr token_splitter = [](std::string&& line){
    std::string::iterator it = line.begin();
    std::vector<std::string> result;

    std::string word;
    while (it != line.end()) {
        if (!isspace(*it)) {
            // this is not delimiter
            word.push_back(*it);
        } else {
            // this is a delimiter
            if (!word.empty()) {
                result.push_back(word);
                word.clear();
            }
        }
        ++it;
    }

    if (!word.empty()) {
        result.push_back(word);
        word.clear();
    }

    return result;
};

auto constexpr empty_list_filter = [](auto const & line) {
    return !line.empty();
};

auto constexpr comment_filter = [](std::string const & line) {
    assert(line.size() > 0);
    return line[0] != '#';
};

std::pair<double,string> parseValue(string&& val) {
    string amount;
    string currency;
    for (auto c : val) {
        if (isdigit(c) || c=='.' || c==',') {
            amount.push_back(c);
        } else {
            currency.push_back(c);
        }
    }
    return std::pair{std::atof(amount.c_str()), currency};
}

auto constexpr line_parser = [](std::vector<std::string>&& line) -> file_mapping::ConfigElement {
    assert(line.size() > 0);
    if (line.at(0) == "def") {
        if (line.at(1) == "person") {
            auto name = line.at(2);
            auto it = line.begin();
            std::advance(it, 3);
            auto aliases = vector<string>{it, line.end()};
            auto p = file_mapping::Person{std::move(name), std::move(aliases)};
            return std::move(p);
        } else if (line.at(1) == "group") {
            auto name = line.at(2);
            auto entities = vector<string>{((line.begin()++)++)++, line.end()};
            auto g = file_mapping::Group{std::move(name), std::move(entities)};
            return std::move(g);
        } else if (line.at(1) == "currency") {
            auto name = line.at(2);
            auto c = file_mapping::Currency{std::move(name)};
            return std::move(c);
        } else {
            assert(false && "Invalid definition!");
        }
    } else {
        assert(line.size() >= 3 && "invalid transaction definition");
        auto paidBy = line.at(0);
        assert(line.at(1) == "->" && "invalid transaction - missing -> sign");
        auto value = parseValue(std::move(line.at(2)));
        vector<string> paidFor{((line.begin()++)++)++, line.end()};
        file_mapping::Transaction t{std::move(paidBy), std::move(value), std::move(paidFor)};
        return std::move(t);
    };
};