#pragma once

#include <optional>
#include <fstream>
#include <cassert>
#include <vector>
#include <iostream>
#include <functional>
#include <type_traits>
//#include <concepts>


using std::optional;

//template<typename T>
//concept bool Iterator = requires(T a) {
//typename T::value_type;
//
//{ a.next() } -> std::optional<typename T::value_type>;
//};

namespace internal {
//
//    This unused class represents an interface, that generic Iterator must implement.
//
//    template<typename T>
//    class Iterator {
//    public:
//        using value_type = T;
//        std::optional<T> next() = 0;
//    };


    template<typename Iter, typename Func>
    class MapIterator {
    private:
        Iter iter;
        Func func;
    public:
        using value_type = decltype(func(std::declval<typename Iter::value_type>()));
        static_assert(!std::is_reference<value_type>::value);
        static_assert(std::is_invocable<Func, typename Iter::value_type>());
        MapIterator(Iter &&a, Func b) : iter{std::move(a)}, func{b} {}

        auto next() {
            auto a = iter.next();
            return a ? std::optional{func(std::move(*a))} : std::nullopt;
        }
    };

    template<typename Iter, typename Func>
    class FilterIterator {
    private:
        Iter iter;
        Func func;
    public:
        using value_type = typename Iter::value_type;
        static_assert(!std::is_reference<value_type>::value);
        static_assert(std::is_invocable_r<bool, Func, value_type>::value);
        FilterIterator(Iter &&a, Func b) : iter{std::move(a)}, func{b} {}

        optional<value_type> next() {
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
        using value_type = std::string;
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
        using value_type = typename std::iterator_traits<ObsoleteIter>::value_type;
        static_assert(!std::is_reference<value_type>::value);
        ObsoleteIteratorConverter(ObsoleteIter&& start, ObsoleteIter&& ennd):_start{std::move(start)}, _end{std::move(ennd)}{}
        optional<value_type> next() {
            return _start == _end ?
                std::nullopt :
                [&](){auto retVal = std::move(*_start); _start++; return std::optional{std::move(retVal)};}();
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

public:
    I(Iter &&iter) : iter{std::move(iter)} {}

    using value_type = typename Iter::value_type;
    static_assert(!std::is_reference<value_type>::value);

    /**
     * The function takes ownership of the data and returns it again.
     * @tparam Func Function, which will take ownership of the data and return it again
     * @param f
     * @return
     */
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
        internal::MapIterator mi(std::move(*iter), [&f](value_type&& p){value_type const & r = p; f(r); return std::move(p);});
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

    optional<value_type> next() {
        assert(iter);
        return (*iter).next();
    }

    auto collect() {
        assert(iter);
        std::vector<value_type> result;
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