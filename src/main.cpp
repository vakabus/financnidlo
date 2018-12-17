#include <iostream>
#include "iterator.h"
#include "parser.h"
#include "balancer.h"
#include "simplifier.h"

using std::optional;
using std::make_optional;
using std::nullopt;
using std::move;

int main(int argc, char ** argv) {
    if (argc > 1) {
        std::cout << "This program takes all its input through stdin. No arguments are allowed." << std::endl;
        return 0;
    }

    //BalancingState result = Iter::file_by_lines("./tests/inputs/bigga.txt")
    BalancingState result = Iter::stdin_by_lines()
            .filter(empty_filter)
            .filter(comment_filter)
            .map(token_splitter)
            .filter(empty_filter)
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