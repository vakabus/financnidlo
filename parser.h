#pragma once

#include "types.h"
#include <string>
#include <set>
#include <vector>
#include <cassert>

auto constexpr token_splitter = [](std::string&& line){
    std::string::iterator it = line.begin();
    std::vector<std::string> result;

    std::string word;
    while (it != line.end()) {
        if (*it != ' ') {
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

auto constexpr empty_line_filter = [](std::string const & line) {
    return !line.empty();
};

auto constexpr comment_filter = [](std::string const & line) {
    assert(line.size() > 0);
    return line[0] != '#';
};