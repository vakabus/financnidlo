#pragma once

#include "types.h"
#include <string>
#include <set>
#include <vector>
#include <cassert>

auto constexpr token_splitter = [](std::string line){
    std::string::iterator it = line.begin();
    std::vector<std::string> result;

    std::string word;
    while (it != line.end()) {
        if (*it != ' ') {
            // this is not delimiter
            word.push_back(*it);
        } else {
            // this is a delimiter
            if (word.size() != 0) {
                result.push_back(word);
                word.clear();
            }
        }
        ++it;
    }

    if (word.size() != 0) {
        result.push_back(word);
        word.clear();
    }

    return result;
};

auto constexpr empty_line_filter = [](std::string line) {
    return line.size() != 0;
};

auto constexpr comment_filter = [](std::string line) {
    assert(line.size() > 0);
    return line[0] != '#';
};

