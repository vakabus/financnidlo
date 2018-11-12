#include <iostream>
#include "iterator.h"
#include "parser.h"
#include "balancer.h"
#include "simplifier.h"

int main() {
    State result = Iter::file_by_lines("../tests/inputs/01.txt")
            .filter(empty_list_filter)
            .filter(comment_filter)
            .map(token_splitter)
            .filter(empty_list_filter)
            .map(line_parser)
            .fold(advance_state, State());

    for (auto&[currency, debtVector] : result.currencies) {
        wrap_iter(SimplifiedTransactionGenerator(std::move(debtVector)))
                .map([&currency,&result](SimpleTransaction st) { return st.to_full_transaction(result.people, currency); })
                .lazy_for_each([](auto const &transaction) { std::cout << transaction << std::endl; })
                .exhaust();
    }
    return 0;
}