#include <iostream>
#include "iterator.h"
#include "parser.h"
#include "balancer.h"
#include "simplifier.h"

int main() {
    BalancingState result = Iter::file_by_lines("../tests/inputs/01.txt")
            .filter(empty_list_filter)
            .filter(comment_filter)
            .map(token_splitter)
            .filter(empty_list_filter)
            .map(line_parser)
            .lazy_for_each(print_definitions)
            .fold(advance_state, BalancingState());

    std::cout << std::endl;
    auto people = move(result.people);

    Iter::from(move(result.currencies)).into([&people](auto &&cdv) {
        auto currency = move(cdv.first);
        auto debtVector = move(cdv.second);
        SimplifiedTransactionGenerator::create(move(debtVector))
                .map([&currency, &people](SimpleTransaction st) {
                    return st.to_full_transaction(people, currency);
                })
                .lazy_for_each([](auto const &transaction) { std::cout << transaction << std::endl; })
                .exhaust();
        return 0;
    });

    return 0;

}