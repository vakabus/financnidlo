#pragma once

#include "types.h"
#include "model.h"
#include <unordered_map>
#include <variant>
#include <vector>
#include <unordered_set>



namespace internal {
    using std::unordered_set;
    using person_id_t = usize;

    class IDRegister {
    private:
        person_id_t last_id = static_cast<person_id_t>(-1);
        unordered_map<string, person_id_t> registry;
        unordered_map<string, unordered_set <person_id_t>> groupRegistry;

        void add_person_name(string name, person_id_t id) {
            auto [col, success] = registry.insert({name, id});
            if (!success) {
                std::cerr << "Person with name \"" << name << "\" is defined twice!" << std::endl;
                std::cerr << "\tFirst time it was - " << std::get<0>(*col) << " with id " << std::get<1>(*col) << std::endl;
                throw "Person definition occured for the second time with the same name";
            }
        }

        person_id_t add_person_name_auto_id(string name) {
            last_id++;
            add_person_name(std::move(name), last_id);
            return last_id;
        }

        unordered_set<person_id_t>& create_group_record(string name) {
            auto [col, success] = groupRegistry.insert({name, unordered_set<person_id_t >{}});
            if (!success) {
                std::cerr << "Group with name \"" << name << "\" is defined twice!" << std::endl;
                throw "Person definition occured for the second time with the same name";
            } else {
                return groupRegistry.at(name);
            }
        }

    public:
        IDRegister():registry{},groupRegistry{}{}
        IDRegister(IDRegister& other) = delete;
        IDRegister(IDRegister&& old) :last_id{std::move(old.last_id)},registry{std::move(old.registry)}{}
        IDRegister& operator=(IDRegister&& old) {
            std::swap(registry, old.registry);
            std::swap(groupRegistry, old.groupRegistry);
            last_id = old.last_id;
            return *this;
        }
        void add_person(file_mapping::Person person) {
            auto id = add_person_name_auto_id(std::move(person.name));
            for (auto alias : person.aliases) add_person_name(std::move(alias), id);
        }

        bool is_group(string& name) {
            return groupRegistry.find(name) != groupRegistry.end();
        }

        unordered_set<person_id_t> const & get_group_members(string& name) {
            return groupRegistry.at(name);
        }

        void add_group(file_mapping::Group group) {
            //TODO What to do, when group contains itself?
            //TODO What to do, when member is defined multiple times? Currently we ignore it.
            auto& g = create_group_record(group.name);
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
}

class State {
    private:
    public:
        internal::IDRegister people;
        State():people{}{}
};

void handlePerson(State& state, file_mapping::Person p) {
    std::cout << "Registering person - " << p.name  << std::endl;
    state.people.add_person(std::move(p));
}

void handleGroup(State& state, file_mapping::Group g) {
    std::cout << "Registering group - " << g.name << std::endl;
    state.people.add_group(std::move(g));
}


auto constexpr advance_state = [](file_mapping::ConfigElement&& config, State state) {
    std::visit([&state](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, file_mapping::Person>) {
                handlePerson(state, std::move(arg));
            } else if constexpr (std::is_same_v<T, file_mapping::Group>){
                handleGroup(state, std::move(arg));
            } else
                std::cout << "Something else" << std::endl;
        }, config);
    return state;
};
