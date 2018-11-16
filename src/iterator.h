#pragma once

#include <optional>
#include <fstream>
#include <cassert>
#include <vector>
#include <iostream>
#include <functional>
#include <type_traits>
#include "types.h"


//template<typename T>
//concept bool Iterator = requires(T a) {
//typename T::value_type;
//
//{ a.next() } -> optional<typename T::value_type>;
//};

namespace {

//
//    This unused class represents an interface, that generic Iterator must implement.
//
//    template<typename T>
//    class Iterator {
//    public:
//        using value_type = T;
//        optional<T> next() = 0;
//    };

    template<typename I>
    struct is_iterator {
        constexpr static bool value =
                std::is_same<decltype(std::declval<I>().next()), optional<typename I::value_type>>::value &&
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

        MapIterator(const MapIterator &other) = delete;

        MapIterator(MapIterator &&old) = default;

        MapIterator(Iter &&a, Func &&b) : iter{move(a)}, func{move(b)} {}

        auto next() {
            auto a = iter.next();
            return a ? make_optional(func(move(*a))) : nullopt;
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

        FilterIterator(const FilterIterator &other) = delete;

        FilterIterator(FilterIterator &&old) = default;

        FilterIterator(Iter &&a, Func &&b) : iter{move(a)}, func{std::move(b)} {}

        optional<value_type> next() {
            auto a = iter.next();
            return !a.has_value() ? nullopt :
                   func(*a) ? make_optional(*a) :
                   next();
        }
    };

    template <class stream>
    class StreamLineIterator {
    private:
        stream in;
    public:
        using value_type = std::string;

        explicit StreamLineIterator(stream &&st) :in{move(st)} {
            assert(in.good() && "Broken stream...");
        }

        explicit StreamLineIterator(stream st, bool _copy): in{st} {
            assert(in.good() && "Broken stream...");
        };

        StreamLineIterator(const StreamLineIterator &old) = delete;

        StreamLineIterator(StreamLineIterator &&old) = default;

        optional<std::string> next() {
            if (in.eof() || in.fail() || in.bad()) {
                return nullopt;
            } else {
                std::string line;
                std::getline(in, line);
                return optional<std::string>{line};
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

        ObsoleteIteratorConverter(const ObsoleteIteratorConverter &other) = delete;

        ObsoleteIteratorConverter(ObsoleteIteratorConverter &&old) = default;

        ObsoleteIteratorConverter(ObsoleteIter &&start, ObsoleteIter &&ennd) : _start{move(start)},
                                                                               _end{move(ennd)} {}

        optional<value_type> next() {
            return _start == _end ?
                   nullopt :
                   [&]() {
                       auto retVal = move(*_start);
                       _start++;
                       return make_optional(move(retVal));
                   }();
        }
    };

    template<typename T>
    class IncrementIter {
    private:
        T state;
    public:
        IncrementIter(const IncrementIter &other) = delete;

        IncrementIter(IncrementIter &&old) = default;

        //TODO fixme check for T's validity
        explicit IncrementIter(T &&init) : state{move(init)} {}

        using value_type = T;

        optional<T> next() {
            return make_optional(state++);
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

        LimitIterator(const LimitIterator &other) = delete;

        LimitIterator(LimitIterator &&old) = default;

        LimitIterator(Iter &&iter, usize limit) : limit{limit}, iter{move(iter)} {}

        optional<typename Iter::value_type> next() {
            if (count >= limit)
                return {};
            count++;
            return iter.next();
        }
    };

    template<typename Iter1, typename Iter2>
    class ZipIterator {
    private:
        Iter1 iter1;
        Iter2 iter2;
    public:
        static_assert(is_iterator<Iter1>::value);
        static_assert(is_iterator<Iter2>::value);
        using value_type = std::pair<typename Iter1::value_type, typename Iter2::value_type>;

        ZipIterator(const ZipIterator &other) = delete;

        ZipIterator(ZipIterator &&old) = default;

        ZipIterator(Iter1 &&iter1, Iter2 &&iter2) : iter1{move(iter1)}, iter2{move(iter2)} {}

        optional<value_type> next() {
            auto a = iter1.next();
            auto b = iter2.next();

            if (a && b) {
                return optional(std::pair{*a, *b});
            } else {
                return nullopt;
            }
        }
    };


}



struct IOldIteratorEnd {
};

template<typename Iter>
class I {
public:
    using value_type = typename Iter::value_type;
private:
    optional<Iter> iter;
    optional<value_type> last_value_for_oldschool_iter = {};

    template <typename OtherIter>
    I<OtherIter> wrap_iter(OtherIter&& iter) {
        return I<OtherIter>(move(iter));
    }

public:

    I(I &&old) = default;

    I(I &other) = delete;

    I(Iter &&iter) : iter{move(iter)} {}

    static_assert(is_iterator<Iter>::value);

    /**
     * The function takes ownership of the data and returns it again.
     * @tparam Func Function, which will take ownership of the data and return it again
     * @param f
     * @return
     */
    template<typename Func>
    auto map(Func &&f) {
        assert(iter);
        MapIterator<Iter, Func> mi(move(*iter), move(f));
        iter.reset();
        return wrap_iter(move(mi));
    }

    template<typename Func>
    auto lazy_for_each(Func &&f) {
        assert(iter);
        MapIterator mi(move(*iter), [=](value_type p) {
            value_type const &r = p;
            f(r);
            return p;
        });
        iter.reset();
        return wrap_iter(move(mi));
    }

    template<typename Func>
    void into(Func &&f) {
        assert(iter);
        MapIterator mi(move(*iter), [=](value_type p) {
            f(move(p));
            return 0;
        });
        iter.reset();
        wrap_iter(move(mi)).exhaust();
    }

    template<typename Func>
    auto filter(Func &&f) {
        assert(iter);
        FilterIterator<Iter, Func> fi(move(*iter), f);
        iter.reset();
        return wrap_iter(move(fi));
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
        LimitIterator li(move(*iter), count);
        iter.reset();
        return wrap_iter(move(li));
    }

    template<typename Func, typename State>
    State fold(Func &&f, State &&s) {
        static_assert(!std::is_reference_v<decltype(f(std::declval<typename Iter::value_type>(), std::declval<State>()))>,
                      "Function returns a reference, but it should return by value.");
        static_assert(!std::is_reference_v<State>, "State in fold can't be reference.");

        assert(iter);
        while (auto a = iter->next()) {
            State ss = f(move(*a), move(s));
            s = move(ss);
        }
        return move(s);
    }

    template<typename Func>
    optional<typename Iter::value_type> reduce(Func &&f) {
        static_assert(std::is_same_v<typename Iter::value_type, decltype(f(std::declval<typename Iter::value_type>(),
                                                                           std::declval<typename Iter::value_type>()))>,
                      "Return type is not the same as iterator type!");
        assert(iter);
        auto first = iter->next();
        if (!first) {
            return nullopt;
        }

        return make_optional(fold(move(f), move(*first)));
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

    IOldIteratorEnd end() {
        // cause the comparison operator will report ending when comparing with anything, we can just return any garbage
        return IOldIteratorEnd{};
    }

    void operator++() {
        last_value_for_oldschool_iter = next();
    }

    /**
     * Returns whether we run out of values. The comparison is otherwise useless, it's here just for compatibility
     * reasons with the archaic C++ iterators.
     */
    bool operator==(IOldIteratorEnd &_) const {
        return !last_value_for_oldschool_iter.has_value();
    }

    /**
     * See the operator==
     */
    bool operator!=(IOldIteratorEnd &other) const {
        return !(*this == other);
    }

    auto enumerate() {
        assert(iter);
        return wrap_iter(ZipIterator(IncrementIter((usize) 0), move(*iter)));
    }

    auto sum() {
        assert(iter);
        return fold([](auto v, auto s) { return v + s; }, (usize) 0);
    }

    auto sum(typename Iter::value_type initialValue) {
        assert(iter);
        return fold([](auto v, auto s) { return v + s; }, initialValue);
    }

    optional<typename Iter::value_type> max() {
        assert(iter);
        return reduce([](auto a, auto b) { return a > b ? a : b; });
    }

    template<typename Func>
    optional<typename Iter::value_type> max_by(Func &&f) {
        assert(iter);
        return reduce([=](auto a, auto b) { return f(a) > f(b) ? a : b; });
    }

    optional<typename Iter::value_type> min() {
        assert(iter);
        return reduce([](auto a, auto b) { return a < b ? a : b; });
    }

    template<typename Func>
    optional<typename Iter::value_type> min_by(Func &&f) {
        assert(iter);
        return reduce([=](auto a, auto b) { return f(a) < f(b) ? a : b; });
    }
};

namespace Iter {

    template<typename T>
    auto count_from(T init) {
        return I(IncrementIter(move(init)));
    }

    auto range(usize fromInclusive, usize toExclusive) {
        return count_from(fromInclusive).take(toExclusive - fromInclusive);
    }

    auto range(usize toExclusive) {
        return count_from((usize) 0).take(toExclusive);
    }

    auto file_by_lines(const std::string &file) {
        return I(StreamLineIterator<std::ifstream>(std::ifstream(file)));
    }

    auto stdin_by_lines() {
        return I(StreamLineIterator<std::istream&>(std::cin, false));
    }

    template<typename Iter1, typename Iter2>
    auto zip(Iter1 &&iter1, Iter2 &&iter2) {
        static_assert(is_iterator<Iter1>::value);
        static_assert(is_iterator<Iter2>::value);
        return I(move(ZipIterator(move(iter1), move(iter2))));
    }

    template<typename Container>
    auto from(Container &&vec) {
        return I(move(ObsoleteIteratorConverter(vec.begin(), vec.end())));
    }

    template<typename Container>
    auto from(Container &vec) {
        return I(move(ObsoleteIteratorConverter(vec.begin(), vec.end())));
    }
}