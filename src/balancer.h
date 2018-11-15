#pragma once

#include "types.h"
#include "model.h"
#include <unordered_map>
#include <variant>
#include <vector>
#include <unordered_set>
#include <ostream>
#include "iterator.h"
#include "people.h"


namespace {
    using DebtVector = std::vector<double>;
    using CurrencyDebts = std::unordered_map<std::string, DebtVector>;
}

class BalancingState {
private:
public:
    CurrencyDebts currencies;
    IDRegister people;

    BalancingState() : currencies{}, people{} {}

    BalancingState(BalancingState &other) = delete;

    BalancingState(BalancingState &&old) /*:currencies(move(old.currencies)), people(move(old.people))*/ {
        // TODO
        // WTF!!! This when get rid of the swaps and put the initialization up into the construction definition,
        // the state passing in iterators stops working
        std::swap(currencies, old.currencies);
        std::swap(people, old.people);

    }

    BalancingState &operator=(BalancingState &&old) {
        std::swap(currencies, old.currencies);
        std::swap(people, old.people);
        return *this;
    }
};

void handle_def_person(BalancingState &state, model::Person p) {
    state.people.add_person(move(p));

    // add the person to each currency debt vector
    for (auto&[curr, debtVector] : state.currencies) {
        debtVector.push_back(0.0);
    }
}

void handle_def_group(BalancingState &state, model::Group g) {
    state.people.add_group(move(g));
}

void handle_def_currency(BalancingState &state, model::Currency c) {
    auto n = state.people.get_number_of_people();
    auto p = DebtVector(n);
    auto[col, success] = state.currencies.insert({c.name, move(p)});
    if (!success) {
        std::cerr << "Currency \"" << c.name << "\" is defined twice!" << std::endl;
        throw "Currency definition occured for the second time with the same name";
    }
}

std::unordered_set<person_id_t> get_all_people(BalancingState const &state, std::vector<std::string> const &paidBy) {
    std::unordered_set<person_id_t> payees;
    for (auto name : paidBy) {
        if (state.people.is_person(name)) {
            payees.insert(state.people.get_id(name));
        } else if (state.people.is_group(name)) {
            auto r = state.people.get_group_members(name);
            payees.insert(r.begin(), r.end());
        } else {
            throw std::logic_error("No group or person with name \"" + name + "\" exists...");
        }
    }
    return payees;
}

void handle_transaction(BalancingState &state, model::Transaction t) {
    auto payees = get_all_people(state, t.paidBy);
    auto receivers = get_all_people(state, t.paidFor);
    auto &debtVector = state.currencies.at(t.value.second);

    auto paidByIndividual = t.value.first / payees.size();
    auto receivedByIndividual = t.value.first / receivers.size();

    // add debt to each receiver
    for (auto id : receivers)
        debtVector.at(id) = debtVector.at(id) + receivedByIndividual;

    // remove debt from each payee
    for (auto id : payees)
        debtVector.at(id) = debtVector.at(id) - paidByIndividual;
}


auto constexpr advance_state = [](model::ConfigElement &&config, BalancingState &&state) -> BalancingState {
    std::visit(overloaded {
            [&state](model::Person &&arg) { handle_def_person(state, move(arg)); },
            [&state](model::Group &&arg) { handle_def_group(state, move(arg)); },
            [&state](model::Currency &&arg) { handle_def_currency(state, move(arg)); },
            [&state](model::Transaction &&arg) { handle_transaction(state, move(arg)); },
            [](auto &&_) {
                std::cerr << "Unimplemented config element appeared! No idea what to do!" << std::endl;
                abort();
            }
    }, move(config));
    return move(state);
};
