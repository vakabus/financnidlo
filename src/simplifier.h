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
                                  std::pair{amount, move(currency)},
                                  std::vector{idRegister.get_canonical_person_name(paidTo)});
    }
};

class SimplifiedTransactionGenerator {
private:
    std::vector<double> debtVector;
public:
    using value_type = SimpleTransaction;

    SimplifiedTransactionGenerator(vector<double> debtVector) : debtVector(move(debtVector)) {
        if (this->debtVector.size() < 2) {
            throw std::logic_error("Does not make sense to generate transactions for so few people!");
        }
    }

    SimplifiedTransactionGenerator(SimplifiedTransactionGenerator &other) = delete;

    SimplifiedTransactionGenerator(SimplifiedTransactionGenerator &&old) {
        std::swap(this->debtVector, old.debtVector);
    }

    optional<SimpleTransaction> next() {
        // check if we should do something
        if (Iter::from(debtVector).map([](auto d) { return std::abs(d); }).sum() < 0.001)
            return nullopt;

        // greedy algo, take the maximal debtor and maximal loaner and create a transaction between them
        auto[loaner, loan] = *(Iter::from(debtVector).enumerate().max_by([](auto p) { return p.second; }));
        auto[debtor, debt] = *(Iter::from(debtVector).enumerate().min_by([](auto p) { return p.second; }));

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
