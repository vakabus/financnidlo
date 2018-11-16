#pragma once

#include <unordered_set>
#include <unordered_map>
#include <string>
#include "types.h"
#include "iterator.h"
#include "model.h"

using person_id_t = usize;

/**
 * This class stores mapping between people's names and their internal numerial identifiers. The same also for groups.
 */
class IDRegister {
private:
    std::vector<std::string> canonicalPersonNames;
    std::unordered_map <std::string, person_id_t> registry;
    std::unordered_map <std::string, std::unordered_set<person_id_t>> groupRegistry;

    void add_person_alias(std::string name, person_id_t id) {
        auto[col, success] = registry.insert({name, id});
        if (!success) {
            std::cerr << "Person with name \"" << name << "\" is defined twice!" << std::endl;
            std::cerr << "\tFirst time it was - " << std::get<0>(*col) << " with id " << std::get<1>(*col)
                      << std::endl;
            throw "Person definition occured for the second time with the same name";
        }
    }

    person_id_t register_person(std::string name) {
        canonicalPersonNames.push_back(name);
        usize id = canonicalPersonNames.size() - 1;
        add_person_alias(move(name), id);
        return id;
    }

    std::unordered_set<person_id_t> &create_group_record(std::string name) {
        auto[col, success] = groupRegistry.insert({name, std::unordered_set<person_id_t>{}});
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

    IDRegister(IDRegister &&old) : canonicalPersonNames{move(old.canonicalPersonNames)}, registry{move(old.registry)} {}

    IDRegister &operator=(IDRegister &&old) {
        std::swap(registry, old.registry);
        std::swap(groupRegistry, old.groupRegistry);
        std::swap(canonicalPersonNames, old.canonicalPersonNames);
        return *this;
    }

    void add_person(model::Person person) {
        auto id = register_person(move(person.name));
        for (auto alias : person.aliases) add_person_alias(move(alias), id);
    }

    usize get_number_of_people() const {
        return canonicalPersonNames.size();
    }

    bool is_group(std::string &name) const {
        return groupRegistry.find(name) != groupRegistry.end();
    }

    bool is_person(std::string &name) const {
        return registry.find(name) != registry.end();
    }

    std::unordered_set<person_id_t> const &get_group_members(std::string &name) const {
        return groupRegistry.at(name);
    }

    std::string const &get_canonical_person_name(const person_id_t id) const {
        return canonicalPersonNames.at(id);
    }

    void add_group(model::Group group) {
        auto &g = create_group_record(group.name);
        for (std::string a : group.mapsTo) {
            if (is_group(a))
                for (person_id_t id : get_group_members(a))
                    g.insert(id);
            else
                g.insert(get_id(a));
        }
    }

    person_id_t get_id(std::string &name) const {
        return registry.at(name);
    }
};