#pragma once

#include <optional>
#include <fstream>
#include <cassert>
#include <vector>
#include <iostream>
#include <functional>
#include <type_traits>
#include "types.h"
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

    template<typename I>
    struct is_iterator {
        constexpr static bool value =
                std::is_same<decltype(std::declval<I>().next()), std::optional<typename I::value_type>>::value &&
                // return value of next and optional<value_type> match
                !std::is_reference<typename I::value_type>::value; // value_type is not a reference
    };


    template<typename Iter, typename Func>
    class MapIterator {
    private:
        Iter iter;
        Func func;
    public:
        using value_type = decltype(func(std::declval<typename Iter::value_type>()));
        static_assert(is_iterator<Iter>::value);
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
        static_assert(is_iterator<Iter>::value);
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

        explicit FileLineIterator(std::string filename) : in{filename} {
            assert(in.good() && "Can't read the supplied file");
        }

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

    template<typename ObsoleteIter>
    class ObsoleteIteratorConverter {
    private:
        ObsoleteIter _start;
        ObsoleteIter _end;
    public:
        using value_type = typename std::iterator_traits<ObsoleteIter>::value_type;
        static_assert(!std::is_reference<value_type>::value);
        static_assert(std::is_same<decltype(_start), decltype(_start++)>::value);

        ObsoleteIteratorConverter(ObsoleteIter &&start, ObsoleteIter &&ennd) : _start{std::move(start)},
                                                                               _end{std::move(ennd)} {}

        optional<value_type> next() {
            return _start == _end ?
                   std::nullopt :
                   [&]() {
                       auto retVal = std::move(*_start);
                       _start++;
                       return std::optional{std::move(retVal)};
                   }();
        }
    };

    template<typename T>
    class IncrementIter {
    private:
        T state;
    public:
        //TODO fixme check for T's validity
        IncrementIter(T init) : state{std::move(init)} {}

        using value_type = T;

        optional<T> next() {
            return state++;
        }
    };

    template<typename Iter>
    class LimitIterator {
    private:
        usize count = 0;
        usize limit;
        Iter iter;
    public:
        using value_type = typename Iter::value_type;
        static_assert(is_iterator<Iter>::value);
        LimitIterator(Iter iter, usize limit) : limit{limit},iter{iter}{}

        optional<typename Iter::value_type> next() {
            if (count >= limit)
                return {};
            count++;
            return iter.next();
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
public:
    using value_type = typename Iter::value_type;
private:
    std::optional<Iter> iter;
    std::optional<value_type> last_value_for_oldschool_iter = {};

public:
    I(Iter &&iter) : iter{std::move(iter)} {}

    static_assert(internal::is_iterator<Iter>::value);

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
    auto lazy_for_each(Func f) {
        assert(iter);
        internal::MapIterator mi(std::move(*iter), [=](value_type p) {
            value_type const &r = p;
            f(r);
            return p;
        });
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

    auto take(usize count) {
        assert(iter);
        internal::LimitIterator li(std::move(*iter), count);
        iter.reset();
        return wrap_iter(std::move(li));
    }

    template<typename Func, typename State>
    State fold(Func f, State s) {
        assert(iter);
        while (auto a = iter->next()) {
            State ss = f(std::move(*a), std::move(s));
            s = std::move(ss);
        }
        return s;
    }

    I &begin() {
        last_value_for_oldschool_iter = next();
        return *this;
    }

    value_type &operator*() {
        if (last_value_for_oldschool_iter)
            return *last_value_for_oldschool_iter;
        else
            throw std::range_error("Iterator range overrun!");
    }

    I& end() {
        // cause the comparison operator will report ending when comparing with anything, we can just return any garbage
        return *this;
    }

    void operator++() {
        last_value_for_oldschool_iter = next();
    }

    /**
     * Returns whether we run out of values. The comparison is otherwise useless, it's here just for compatibility
     * reasons with the archaic C++ iterators.
     */
    template <typename T>
    bool operator==(T & other) const {
        return !last_value_for_oldschool_iter.has_value();
    }

    /**
     * See the operator==
     */
    template <typename T>
    bool operator!=(T& other) const {
        return !(*this == other);
    }
};

namespace Iter {
    auto file_by_lines(std::string file) {
        return wrap_iter(internal::FileLineIterator(file));
    }

    template<typename T>
    auto from_vector(std::vector<T> &&vec) {
        return wrap_iter(internal::ObsoleteIteratorConverter(vec.begin(), vec.end()));
    }

    template<typename T>
    auto from_vector(std::vector<T> &vec) {
        return wrap_iter(internal::ObsoleteIteratorConverter(vec.begin(), vec.end()));
    }

    template<typename T>
    auto count_from(T init) {
        return wrap_iter(internal::IncrementIter(init));
    }

    auto range(usize fromInclusive, usize toExclusive) {
        return count_from(fromInclusive).take(toExclusive - fromInclusive);
    }

    auto range(usize toExclusive) {
        return count_from((usize)0).take(toExclusive);
    }
}