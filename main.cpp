#include <iostream>
#include "iterator.h"
#include "parser.h"

int main() {
    Iter::file_by_lines("/etc/pacman.conf")
            .filter(empty_line_filter)
            .filter(comment_filter)
            .map(token_splitter)
            .lazyForEach([](std::vector<std::string> const & l) {
                auto cl = l;
                std::cout << "[line]\n";
                Iter::from_vector(cl).lazyForEach([](std::string const & s) {
                    std::cout << "    [token] " << s << "\n";
                }).exhaust();
            }).exhaust();
    return 0;
}