#pragma once

#include "types.h"
#include "iterator.h"
#include "model.h"
#include "balancer.h"
#include "people.h"
#include <limits.h>
#include <float.h>
#include <vector>
#include <algorithm>
#include <optional>


struct SimpleTransaction {
    person_id_t paidBy;
    double amount;
    person_id_t paidTo;

    model::Transaction to_full_transaction(IDRegister &idRegister, std::string currency) {
        return model::Transaction(std::vector{idRegister.get_canonical_person_name(paidBy)},
                                  std::pair{amount, std::move(currency)},
                                  std::vector{idRegister.get_canonical_person_name(paidTo)});
    }
};

/**
 * When supplied with debt vector, it provides an iterator for transactions representing that debt vector. Number of
 * them should be minimal.
 *
 * This generator knows only about the numbers, it has no idea about currencies and people, who hold the debt.
 * That means, that the simplified transaction has to be converted to full transaction before other usage.
 */
class SimplifiedTransactionGenerator {
private:
    std::vector<double> debtVector;

    SimplifiedTransactionGenerator(std::vector<double> &&debtVector) : debtVector(std::move(debtVector)) {
        if (this->debtVector.size() < 2) {
            throw std::logic_error("Does not make sense to generate transactions for so few people!");
        }
    }

public:
    using value_type = SimpleTransaction;

    SimplifiedTransactionGenerator(SimplifiedTransactionGenerator &other) = delete;

    SimplifiedTransactionGenerator(SimplifiedTransactionGenerator &&old) {
        std::swap(this->debtVector, old.debtVector);
    }

    std::optional<SimpleTransaction> next() {
        // check if we should do something
        if (Iter::from(debtVector).map([](auto d) { return std::abs(d); }).max() < 0.001)
            return std::nullopt;

        // greedy algo, take the maximal debtor and maximal loaner and create a transaction between them
        auto[loaner, loan] = *(Iter::from(debtVector).enumerate().max_by([](auto p) { return p.second; }));
        auto[debtor, debt] = *(Iter::from(debtVector).enumerate().min_by([](auto p) { return p.second; }));

        double transactionVal = std::min(-debt, loan);

        // modify debt vector according to the new transaction
        debtVector[loaner] -= transactionVal;
        debtVector[debtor] += transactionVal;

        // save new transaction
        SimpleTransaction trans;
        trans.paidBy = debtor;
        trans.amount = transactionVal;
        trans.paidTo = loaner;

        return trans;
    }

    static I<SimplifiedTransactionGenerator> create(std::vector<double> &&debtVector) {
        return I(SimplifiedTransactionGenerator(std::move(debtVector)));
    }
};
