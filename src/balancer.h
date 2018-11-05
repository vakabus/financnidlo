#pragma once

#include "types.h"
#include "model.h"
#include "unordered_map"
#include "variant"



class State {
    private:
    public:
        internal::PersonIDRegister people;
        State():people{}{}
};

void handlePerson(State& state, file_mapping::Person&& p) {
    std::cout << "Registering person - " << p.name  << std::endl;
    state.people.add_person(std::move(p));
}

auto constexpr advance_state = [](file_mapping::ConfigElement&& config, State& prevState) {
    State& state = prevState;
    std::visit([&state](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, file_mapping::Person>) {
                file_mapping::Person p = std::move(arg);
                handlePerson(state, std::move(p));
            } else
                std::cout << "Something else" << std::endl;
        }, config);
};