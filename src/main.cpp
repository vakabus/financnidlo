#include <iostream>
#include "iterator.h"
#include "parser.h"

int main() {
    Iter::file_by_lines("/etc/pacman.conf")
            .filter(empty_list_filter)
            .filter(comment_filter)
            .map(token_splitter)
            .filter(empty_line_filter)
            .map(line_parser)
            .exhaust();
    return 0;
}