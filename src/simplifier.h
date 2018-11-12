#pragma once

#include "types.h"
#include "iterator.h"
#include "model.h"
#include "balancer.h"
#include <limits.h>
#include <float.h>
#include <vector>
#include <algorithm>
#include <optional>


struct SimpleTransaction {
    internal::person_id_t paidBy;
    double amount;
    internal::person_id_t paidTo;

    model::Transaction to_full_transaction(internal::IDRegister &idRegister, string currency) {
        return model::Transaction(std::vector{idRegister.get_canonical_person_name(paidBy)},
                                         std::pair{amount, std::move(currency)},
                                         std::vector{idRegister.get_canonical_person_name(paidTo)});
    }
};

class SimplifiedTransactionGenerator {
private:
    std::vector<double> debtVector;
public:
    using value_type = SimpleTransaction;
    SimplifiedTransactionGenerator(vector<double> debtVector) : debtVector(std::move(debtVector)) {
        if (this->debtVector.size() < 2) {
            throw std::logic_error("Does not make sense to generate transactions for so few people!");
        }
    }

    optional<SimpleTransaction> next() {
        // check if we should do something
        bool shouldGenerateSomething = false;
        for (auto d : debtVector) {
            if (std::abs(d) > 0.00001) {
                shouldGenerateSomething = true;
                break;
            }
        }
        if (!shouldGenerateSomething)
            return std::nullopt;

        // greedy algo, take the maximal debtor and maximal loaner and create a transaction between them
        auto[loaner, loan] = Iter::from(debtVector)
                .enumerate()
                .fold([](auto p, auto m) { return m.second > p.second ? m : p; }, std::make_pair((usize)0, DBL_MIN));
        auto[debtor, debt] = Iter::from(debtVector)
                .enumerate()
                .fold([](auto p, auto m) { return m.second < p.second ? m : p; }, std::make_pair((usize)0, DBL_MAX));

        double transactionVal = std::min(-debt, loan);

        // modify debt vector according to the new transaction
        debtVector[loaner] -= transactionVal;
        debtVector[debtor] += transactionVal;

        // save new transaction
        SimpleTransaction trans;
        trans.paidBy = loaner;
        trans.amount = transactionVal;
        trans.paidTo = debtor;

        return trans;
    }
};
