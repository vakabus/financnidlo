#include <iostream>
#include "iterator.h"
#include "parser.h"
#include "balancer.h"

int main() {
    State result = Iter::file_by_lines("tests/inputs/01.txt")
            .filter(empty_list_filter)
            .filter(comment_filter)
            .map(token_splitter)
            .filter(empty_list_filter)
            .map(line_parser)
            .fold(advance_state, State());
    return 0;
}