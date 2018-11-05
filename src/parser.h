#pragma once

#include "types.h"
#include <string>
#include <set>
#include <vector>
#include <cassert>
#include "model.h"
#include <exception>

auto constexpr token_splitter = [](std::string &&line) -> std::vector<std::string> {
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

auto constexpr empty_list_filter = [](auto const &line) {
    return !line.empty();
};

auto constexpr comment_filter = [](std::string const &line) {
    assert(line.size() > 0);
    return line[0] != '#';
};

std::pair<double, string> parseValue(string &&val) {
    string amount;
    string currency;
    for (auto c : val) {
        if (isdigit(c) || c == '.' || c == ',') {
            amount.push_back(c);
        } else {
            currency.push_back(c);
        }
    }
    if (amount.empty() || currency.empty()) throw std::logic_error("Invalid value in transaction");
    return std::pair{std::atof(amount.c_str()), currency};
}

auto constexpr line_parser = [](std::vector<std::string> line) -> file_mapping::ConfigElement {
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
            auto it = line.begin();
            std::advance(it, 3);
            auto entities = vector<string>{it, line.end()};
            auto g = file_mapping::Group{std::move(name), std::move(entities)};
            return std::move(g);
        } else if (line.at(1) == "currency") {
            if (line.size() > 3)
                throw std::logic_error("Currency definition contains too many tokens...");
            auto name = line.at(2);
            auto c = file_mapping::Currency{std::move(name)};
            return std::move(c);
        } else {
            throw std::logic_error("Invalid definition!");
        }
    } else {
        vector<string> paidBy;
        for (auto &a : line) {
            if (a == "paid")
                break;
            paidBy.push_back(a);
        }

        if (paidBy.empty())
            throw std::logic_error("not enough payees");

        if (line.size() - paidBy.size() <= 3) {
            throw std::logic_error("transaction does not have enough tokens");
        }

        if (line.at(paidBy.size()) != "paid") {
            throw std::logic_error("invalid transaction - missing paid keyword");
        }

        auto value = parseValue(std::move(line.at(paidBy.size()+1)));

        if (line.at(paidBy.size() + 2) != "for") {
            throw std::logic_error("invalid transaction - missing for keyword");
        }

        auto it = line.begin();
        std::advance(it, paidBy.size() + 3);
        vector<string> paidFor{it, line.end()};
        file_mapping::Transaction t{std::move(paidBy), std::move(value), std::move(paidFor)};
        return std::move(t);
    };
};
