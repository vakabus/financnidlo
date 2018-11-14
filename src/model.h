#pragma once

#include "types.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <sstream>
#include <variant>
#include <iostream>


namespace model {
    using std::unordered_map;
    using std::vector;
    using std::string;

    struct Person {
        string name;
        vector<string> aliases;

        Person(string name, vector<string> aliases) : name{name}, aliases{aliases} {}
        Person(Person &other) = delete;
        Person(Person &&old) : name{move(old.name)}, aliases{move(old.aliases)} {}

        friend std::ostream &operator<<(std::ostream &os, const Person &person) {
            os << "def person " << person.name;
            for (auto al : person.aliases) os << " " << al;
            return os;
        }
    };

    struct Group {
        string name;
        vector<string> mapsTo;

        Group(string &&name, vector<string> &&mapsTo) : name{move(name)}, mapsTo{move(mapsTo)} {}
        Group(Group &other) = delete;
        Group(Group &&old) : name{move(old.name)}, mapsTo{move(old.mapsTo)} {}

        friend std::ostream &operator<<(std::ostream &os, const Group &group) {
            os << "def group " << group.name;
            for (auto mt : group.mapsTo) os << " " << mt;
            return os;
        }
    };
    struct Currency {
        string name;

        Currency(string&& name):name{name}{}
        Currency(Currency&& old): name{old.name}{}
        Currency(Currency& other) = delete;

        friend std::ostream &operator<<(std::ostream &os, const Currency &currency) {
            os << "def currency " << currency.name;
            return os;
        }
    };

    using Value = std::pair<double, Currency>;
    using CurrencyTransformation = std::pair<Value, Value>;

    struct Transaction {
        vector<string> paidBy;
        std::pair<double, string> value;
        vector<string> paidFor;

        Transaction(vector<string> paidBy, std::pair<double, string> value, vector<string> paidFor):paidBy{move(paidBy)}, value{move(value)}, paidFor{move(paidFor)} {}
        Transaction(Transaction& other) = delete;
        Transaction(Transaction&& old): paidBy{move(old.paidBy)}, value{move(old.value)}, paidFor{move(old.paidFor)}{}

        friend std::ostream &operator<<(std::ostream &out, const Transaction &transaction) {
            for (auto p : transaction.paidBy)
                out << p << " ";
            out << "paid " << transaction.value.first << transaction.value.second << " for";
            for (auto p : transaction.paidFor)
                out << " " << p;
            return out;
        }
    };

    using ConfigElement = std::variant<Person, Group, Currency, Transaction>;

    bool operator==(const ConfigElement& ce, const Person& p) {
        return std::holds_alternative<Person>(ce) && std::get<Person>(ce).name == p.name && std::get<Person>(ce).aliases == p.aliases;
    }

    bool operator==(const ConfigElement& ce, const Group& p) {
        return std::holds_alternative<Group>(ce) && std::get<Group>(ce).name == p.name && std::get<Group>(ce).mapsTo == p.mapsTo;
    }

    bool operator==(const ConfigElement& ce, const Currency& p) {
        return std::holds_alternative<Currency>(ce) && std::get<Currency>(ce).name == p.name;
    }

    bool operator==(const ConfigElement& ce, const Transaction& p) {
        return std::holds_alternative<Transaction>(ce) && std::get<Transaction>(ce).paidBy == p.paidBy && std::get<Transaction>(ce).value == p.value && std::get<Transaction>(ce).paidFor == p.paidFor;
    }

}