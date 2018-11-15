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


namespace {
    std::pair<double, std::string> parse_value(std::string &&val) {
        std::string amount;
        std::string currency;
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

    model::Person parse_person(std::vector<std::string> &line) {
        auto name = line.at(2);
        auto it = line.begin();
        std::advance(it, 3);
        auto aliases = std::vector<std::string>{it, line.end()};
        auto p = model::Person{move(name), move(aliases)};
        return p;
    }

    model::Group parse_group(std::vector<std::string> &line) {
        auto name = line.at(2);
        auto it = line.begin();
        std::advance(it, 3);
        auto entities = std::vector<std::string>{it, line.end()};
        auto g = model::Group{move(name), move(entities)};
        return g;
    }

    model::Currency parse_currency(std::vector<std::string> &line) {
        if (line.size() > 3)
            throw std::logic_error("Currency definition contains too many tokens...");
        auto name = line.at(2);
        auto c = model::Currency{move(name)};
        return c;
    }

    model::Transaction parse_transaction(std::vector<std::string> &line) {
        std::vector<std::string> paidBy;
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

        auto value = parse_value(move(line.at(paidBy.size() + 1)));

        if (line.at(paidBy.size() + 2) != "for") {
            throw std::logic_error("invalid transaction - missing for keyword");
        }

        auto it = line.begin();
        std::advance(it, paidBy.size() + 3);
        std::vector<std::string> paidFor{it, line.end()};
        model::Transaction t{move(paidBy), move(value), move(paidFor)};
        return t;
    }
}

auto constexpr line_parser = [](std::vector<std::string> line) -> model::ConfigElement {
    assert(line.size() > 0);
    if (line.at(0) == "def") {
        if (line.at(1) == "person") {
            return parse_person(line);
        } else if (line.at(1) == "group") {
            return parse_group(line);
        } else if (line.at(1) == "currency") {
            return parse_currency(line);
        } else {
            throw std::logic_error("Invalid definition!");
        }
    } else {
        return parse_transaction(line);
    };
};

auto constexpr print_definitions = [](const model::ConfigElement& element) {
    std::visit(overloaded {
            [](const model::Person& arg) { std::cout << arg << std::endl; },
            [](const model::Currency& arg) { std::cout << arg << std::endl; },
            [](const model::Group& arg) { std::cout << arg << std::endl; },
            [](const auto& _) {}
    }, element);
};
