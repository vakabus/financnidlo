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
    using Conversions = std::unordered_map<std::string, model::Value>;
}

class BalancingState {
private:
public:
    CurrencyDebts currencies;
    IDRegister people;
    Conversions conversions;

    BalancingState() : currencies{}, people{}, conversions{} {}

    BalancingState(BalancingState &other) = delete;

    BalancingState(BalancingState &&old) /*:currencies(std::move(old.currencies)), people(std::move(old.people))*/ {
        // TODO wtf
        // WTF!!! This fails when I get rid of the swaps and put the initialization up into the constructor definition,
        // It causes state passing in iterators to stop working
        std::swap(currencies, old.currencies);
        std::swap(people, old.people);
        std::swap(conversions, old.conversions);

    }

    BalancingState &operator=(BalancingState &&old) {
        std::swap(currencies, old.currencies);
        std::swap(people, old.people);
        std::swap(conversions, old.conversions);
        return *this;
    }
};

void handle_def_person(BalancingState &state, model::Person p) {
    state.people.add_person(std::move(p));

    // add the person to each currency debt vector
    for (auto&[curr, debtVector] : state.currencies) {
        debtVector.push_back(0.0);
    }
}

void handle_def_group(BalancingState &state, model::Group g) {
    state.people.add_group(std::move(g));
}

void handle_def_currency(BalancingState &state, model::Currency c) {
    auto n = state.people.get_number_of_people();
    auto p = DebtVector(n);
    auto[col, success] = state.currencies.insert({c.name, std::move(p)});
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

    auto currency = t.value.second;
    DebtVector *debtVector = &state.currencies.at(t.value.second);  //FIXME can we do it without using pointers and only
                                                                    // using references? I don't know how
    double conversionRate = 1.;

    // check if we should perform conversion
    auto conversion = state.conversions.find(currency);
    if (conversion != state.conversions.end()) {
        debtVector = &state.currencies.at(conversion->second.second.name);
        conversionRate = conversion->second.first;
    }

    auto paidByIndividual = t.value.first / payees.size();
    auto receivedByIndividual = t.value.first / receivers.size();

    // add debt to each receiver
    for (auto id : receivers)
        debtVector->at(id) = debtVector->at(id) + receivedByIndividual * conversionRate;

    // remove debt from each payee
    for (auto id : payees)
        debtVector->at(id) = debtVector->at(id) - paidByIndividual * conversionRate;
}

void handle_currency_transformation(BalancingState &state, model::CurrencyTransformation transformation) {
    const auto& from = transformation.first;
    const auto& to = transformation.second;

    const auto sourceCurrencyName = from.second.name;
    if (state.conversions.find(sourceCurrencyName) != state.conversions.end()) {
        std::cerr << "Duplicate currency conversion found! Aborting!" << std::endl;
        abort();
    }
    const auto targetCurrencyName = to.second.name;
    const double conversionRate = to.first / from.first;

    // save the conversion rate
    state.conversions.insert({sourceCurrencyName, std::make_pair(conversionRate, targetCurrencyName)});

    // convert all existing balance
    auto res = state.currencies.find(sourceCurrencyName);
    if (res != state.currencies.end()) {
        const auto sourceDebtVector = res->second;
        const auto targetDebtVector = state.currencies.at(targetCurrencyName);
        DebtVector nTarget = Iter::zip(Iter::from(sourceDebtVector), Iter::from(targetDebtVector))
                .map([=](auto vals){ return vals.second + vals.first * conversionRate;})
                .collect();
        state.currencies[targetCurrencyName] = nTarget;
        state.currencies[sourceCurrencyName] = DebtVector(state.people.get_number_of_people());
    }
}


auto constexpr advance_state = [](model::ConfigElement &&config, BalancingState &&state) -> BalancingState {
    std::visit(overloaded {
            [&state](model::Person &&arg) { handle_def_person(state, std::move(arg)); },
            [&state](model::Group &&arg) { handle_def_group(state, std::move(arg)); },
            [&state](model::Currency &&arg) { handle_def_currency(state, std::move(arg)); },
            [&state](model::Transaction &&arg) { handle_transaction(state, std::move(arg)); },
            [&state](model::CurrencyTransformation &&arg) { handle_currency_transformation(state, std::move(arg)); },
            [](auto &&_) {
                std::cerr << "Unimplemented config element appeared! No idea what to do!" << std::endl;
                abort();
            }
    }, std::move(config));
    return std::move(state);
};
