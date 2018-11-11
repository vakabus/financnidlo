#pragma once

#include "types.h"
#include "model.h"
#include <unordered_map>
#include <variant>
#include <vector>
#include <unordered_set>
#include <ostream>
#include "iterator.h"


namespace internal {
    using std::unordered_set;
    using person_id_t = usize;

    class IDRegister {
    private:
        person_id_t last_id = static_cast<person_id_t>(-1);
        unordered_map<string, person_id_t> registry;
        unordered_map<string, unordered_set<person_id_t>> groupRegistry;

        void add_person_name(string name, person_id_t id) {
            auto[col, success] = registry.insert({name, id});
            if (!success) {
                std::cerr << "Person with name \"" << name << "\" is defined twice!" << std::endl;
                std::cerr << "\tFirst time it was - " << std::get<0>(*col) << " with id " << std::get<1>(*col)
                          << std::endl;
                throw "Person definition occured for the second time with the same name";
            }
        }

        person_id_t add_person_name_auto_id(string name) {
            last_id++;
            add_person_name(std::move(name), last_id);
            return last_id;
        }

        unordered_set<person_id_t> &create_group_record(string name) {
            auto[col, success] = groupRegistry.insert({name, unordered_set<person_id_t>{}});
            if (!success) {
                std::cerr << "Group with name \"" << name << "\" is defined twice!" << std::endl;
                throw "Person definition occured for the second time with the same name";
            } else {
                return groupRegistry.at(name);
            }
        }

    public:
        IDRegister() : registry{}, groupRegistry{} {}

        IDRegister(IDRegister &other) = delete;

        IDRegister(IDRegister &&old) : last_id{std::move(old.last_id)}, registry{std::move(old.registry)} {}

        IDRegister &operator=(IDRegister &&old) {
            std::swap(registry, old.registry);
            std::swap(groupRegistry, old.groupRegistry);
            last_id = old.last_id;
            return *this;
        }

        void add_person(file_mapping::Person person) {
            auto id = add_person_name_auto_id(std::move(person.name));
            for (auto alias : person.aliases) add_person_name(std::move(alias), id);
        }

        usize get_number_of_people() const {
            return (usize) last_id + 1;
        }

        bool is_group(string &name) const {
            return groupRegistry.find(name) != groupRegistry.end();
        }

        bool is_person(string &name) const {
            return registry.find(name) != registry.end();
        }

        unordered_set<person_id_t> const &get_group_members(string &name) const {
            return groupRegistry.at(name);
        }

        void add_group(file_mapping::Group group) {
            //TODO What to do, when group contains itself?
            //TODO What to do, when member is defined multiple times? Currently we ignore it.
            auto &g = create_group_record(group.name);
            for (string a : group.mapsTo) {
                if (is_group(a))
                    for (person_id_t id : get_group_members(a))
                        g.insert(id);
                else
                    g.insert(get_id(a));
            }
        }

        person_id_t get_id(string &name) const {
            return registry.at(name);
        }
    };

    struct Pays {
        person_id_t to;
        double amount;

        Pays(person_id_t id, double a) : to{id}, amount{a} {}
    };

    struct Receives {
        vector<person_id_t> from;
    };

    using DebtVector = vector<double>;
    using CurrencyDebts = unordered_map<string, DebtVector>;
};

class State {
private:
public:
    internal::CurrencyDebts currencies;
    internal::IDRegister people;

    State() : people{}, currencies{} {}

    State(State& other) = delete;

    State(State&& old) {
        // TODO
        // WTF!!! This when get rid of the swaps and put the initialization up into the construction definition,
        // the state passing in iterators stops working
        std::swap(currencies, old.currencies);
        std::swap(people, old.people);
    }
    State &operator=(State &&old) {
        std::swap(currencies, old.currencies);
        std::swap(people, old.people);
        return *this;
    }
};

void handlePerson(State &state, file_mapping::Person p) {
    std::cout << "Registering person - " << p.name << std::endl;
    state.people.add_person(std::move(p));

    // add the person to each currency debt vector
    for (auto&[curr, debtVector] : state.currencies) {
        debtVector.push_back(0.0);
    }
}

void handleGroup(State &state, file_mapping::Group g) {
    std::cout << "Registering group - " << g.name << std::endl;
    state.people.add_group(std::move(g));
}

void handleCurrency(State &state, file_mapping::Currency c) {
    std::cout << "Registering currency - " << c.name << std::endl;
    auto n = state.people.get_number_of_people();
    auto p = internal::DebtVector(n);
    auto[col, success] = state.currencies.insert({c.name, std::move(p)});
    if (!success) {
        std::cerr << "Currency \"" << c.name << "\" is defined twice!" << std::endl;
        throw "Currency definition occured for the second time with the same name";
    }
}

std::unordered_set<internal::person_id_t> get_all_people(State const &state, vector<string> const &paidBy) {
    std::unordered_set<internal::person_id_t> payees;
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

void handleTransaction(State &state, file_mapping::Transaction t) {
    std::cout << "Transaction" << std::endl;
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


auto constexpr advance_state = [](file_mapping::ConfigElement &&config, State state) -> State {
    std::cout << "Advance state..." << std::endl;
    std::visit([&state](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, file_mapping::Person>) {
            handlePerson(state, std::move(arg));
        } else if constexpr (std::is_same_v<T, file_mapping::Group>) {
            handleGroup(state, std::move(arg));
        } else if constexpr (std::is_same_v<T, file_mapping::Currency>) {
            handleCurrency(state, std::move(arg));
        } else if constexpr (std::is_same_v<T, file_mapping::Transaction>) {
            handleTransaction(state, std::move(arg));
        } else {
            std::cerr << "Unimplemented config element appeared! No idea what to do!" << std::endl;
            abort();
        }
    }, config);
    return state;
};
