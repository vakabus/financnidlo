#pragma once

#include <optional>
#include <fstream>
#include <cassert>
#include <vector>
#include <iostream>


namespace internal {
/**
 * This unused class represents an interface, that generic Iterator must implement.
 */
//TODO replace with concepts
    template<typename T>
    class Iterator {
    public:
        std::optional<T> next() = 0;
    };

    template<typename Iter, typename Func>
    class MapIterator {
    private:
        Iter iter;
        Func func;
    public:
        MapIterator(Iter &&a, Func b) : iter{std::move(a)}, func{b} {}

        auto next() {
            auto a = iter.next();
            return a ? std::optional{func(*a)} : std::nullopt;
        }
    };

    template<typename Iter, typename Func>
    class FilterIterator {
    private:
        Iter iter;
        Func func;
    public:
        FilterIterator(Iter &&a, Func b) : iter{std::move(a)}, func{b} {}

        decltype(iter.next()) next() {
            auto a = iter.next();
            return !a.has_value() ? std::nullopt :
                   func(*a) ? std::optional{*a} :
                   next();
        }
    };

    class FileLineIterator {
    private:
        std::ifstream in;
    public:
        explicit FileLineIterator(std::string filename) : in{filename} {}

        FileLineIterator(FileLineIterator &&old) : in{std::move(old.in)} {}

        FileLineIterator(const FileLineIterator &old) = delete;

        std::optional<std::string> next() {
            if (in.eof() || in.fail() || in.bad()) {
                return std::nullopt;
            } else {
                std::string line;
                std::getline(in, line);
                return std::optional<std::string>{line};
            }
        }
    };

    template <typename ObsoleteIter>
    class  ObsoleteIteratorConverter {
    private:
        ObsoleteIter _start;
        ObsoleteIter _end;
    public:
        ObsoleteIteratorConverter(ObsoleteIter&& start, ObsoleteIter&& ennd):_start{std::move(start)}, _end{std::move(ennd)}{}
        auto next() {
            return _start == _end ?
                std::nullopt :
                [&](){auto retVal = *_start; _start++; return std::optional{retVal};}();
        }
    };
}

template<typename Iter>
class I;

template<typename Iter>
I<Iter> wrap_iter(Iter &&iter) {
    return I(std::move(iter));
}

template<typename Iter>
class I {
private:
    std::optional<Iter> iter;

    /**
     * This function is hopefully pretty much useless, however it can be used to get the type of the iterator via decltype.
     * Throws an exception, when the iterator is empty.
     * @return first element of the iterator
     */
    auto first() {
        assert(iter);
        auto a = (*iter).next();
        if (!a) {
            throw "The iterator is empty!";
        }
        return *a;
    }

public:
    I(Iter &&iter) : iter{std::move(iter)} {}

    template<typename Func>
    auto map(Func f) {
        assert(iter);
        internal::MapIterator<Iter, Func> mi(std::move(*iter), f);
        iter.reset();
        return wrap_iter(std::move(mi));
    }

    template<typename Func>
    auto lazyForEach(Func f) {
        assert(iter);
        internal::MapIterator mi(std::move(*iter), [&f](auto p){f(p); return p;});
        iter.reset();
        return wrap_iter(std::move(mi));
    }

    template<typename Func>
    auto filter(Func f) {
        assert(iter);
        internal::FilterIterator<Iter, Func> fi(std::move(*iter), f);
        iter.reset();
        return wrap_iter(std::move(fi));
    }

    auto next() {
        assert(iter);
        return (*iter).next();
    }

    auto collect() {
        assert(iter);
        std::vector<decltype(first())> result;
        while (auto a = (*iter).next()) result.push_back(*a);
        return result;
    }

    void exhaust() {
        assert(iter);
        while (iter->next()) {}
    }

    template<typename Func, typename State>
    State fold(Func f, State s) {
        assert(iter);
        while (auto a = iter->next()) s = f(a, s);
        return s;
    }
};

namespace Iter {
    auto file_by_lines(std::string file) {
        return wrap_iter(internal::FileLineIterator(file));
    }

    template <typename T>
    auto from_vector(std::vector<T>&& vec) {
        return wrap_iter(internal::ObsoleteIteratorConverter(vec.begin(), vec.end()));
    }

    template <typename T>
    auto from_vector(std::vector<T>& vec) {
        return wrap_iter(internal::ObsoleteIteratorConverter(vec.begin(), vec.end()));
    }
}