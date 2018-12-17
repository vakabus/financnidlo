#pragma once

#include <optional>
#include <fstream>
#include <cassert>
#include <vector>
#include <iostream>
#include <functional>
#include <type_traits>
#include "types.h"


/***
 *  Helper for compile-time checks determining if an iterator matches required criteria and behaves as expected
 * @tparam I Type to run the check against
 */
template<typename I>
struct is_iterator {
    constexpr static bool value =
            std::is_same<decltype(std::declval<I>().next()), std::optional<typename I::value_type>>::value &&
            // return value of next and std::optional<value_type> match
            !std::is_reference<typename I::value_type>::value; // value_type is not a reference
};

namespace {

//    This unused class represents an interface, that generic Iterator must implement.
//
//    template<typename T>
//    class Iterator {
//    public:
//        using value_type = T;
//        std::optional<T> next() = 0;
//    };


    /**
     * This is NOT meant to be used DIRECTLY. Use method `Iter::map` instead.
     *
     * MetaIterator - transforms one iterator into another by calling the supplied function on every element of
     * the supplied one. Evaluates lazily.
     * @tparam Iter Type of the source iterator
     * @tparam Func Function run against values returned by the Iter iterator
     */
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

        MapIterator(Iter &&a, Func &&b) : iter{std::move(a)}, func{std::move(b)} {}

        auto next() {
            auto a = iter.next();
            return a ? std::make_optional(func(std::move(*a))) : std::nullopt;
        }
    };

    /**
     * This is NOT meant to be used DIRECTLY. Use method `Iter::filter` instead.
     *
     * MetaIterator - given an iterator of one type and a validation function, return an iterator yielding only those
     * values, that match the validation function
     * @tparam Iter Type of source iterator
     * @tparam Func Function taking `Iter::value_type` as an input and returing `bool` representing wheter to keep the data or not.
     */
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

        FilterIterator(Iter &&a, Func &&b) : iter{std::move(a)}, func{std::move(b)} {}

        std::optional<value_type> next() {
            auto a = iter.next();
            return !a.has_value() ? std::nullopt :
                   func(*a) ? std::make_optional(*a) :
                   next();
        }
    };

    /**
     * This is NOT meant to be used DIRECTLY. Use method `Iter::file_by_lines` or `Iter::stdin_by_lines` instead.
     *
     * Given a stream, iterate over it by lines...
     * @tparam stream Type of the data stream
     */
    template<class stream>
    class StreamLineIterator {
    private:
        stream in;
    public:
        using value_type = std::string;

        explicit StreamLineIterator(stream &&st) : in{std::move(st)} {
            assert(in.good() && "Broken stream...");
        }

        explicit StreamLineIterator(stream st, bool _copy) : in{st} {
            assert(in.good() && "Broken stream...");
        };

        StreamLineIterator(const StreamLineIterator &old) = delete;

        StreamLineIterator(StreamLineIterator &&old) = default;

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

    /**
     * This is NOT meant to be used DIRECTLY. Use method `Iter::from` instead.
     *
     * Iterator that takes native C++ iterator and transforms it into iterator compatible with this library.
     * @tparam ObsoleteIter
     */
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

        ObsoleteIteratorConverter(ObsoleteIter &&start, ObsoleteIter &&ennd) : _start{std::move(start)},
                                                                               _end{std::move(ennd)} {}

        std::optional<value_type> next() {
            return _start == _end ?
                   std::nullopt :
                   [&]() {
                       auto retVal = std::move(*_start);
                       _start++;
                       return std::make_optional(std::move(retVal));
                   }();
        }
    };

    /**
     * This is NOT meant to be used DIRECTLY. Use method `Iter::range` or `Iter::count` instead.
     *
     * Iterator yielding values obtained by incrementing initial supplied value by calling `value++`
     * @tparam T Type of the incremented value
     */
    template<typename T>
    class IncrementIter {
    private:
        T state;
    public:

        IncrementIter(const IncrementIter &other) = delete;

        IncrementIter(IncrementIter &&old) = default;

        static_assert(std::is_same_v<decltype((*(T *) nullptr)++), T>, "Template type does not implement ++ operator.");

        explicit IncrementIter(T &&init) : state{std::move(init)} {}

        using value_type = T;

        std::optional<T> next() {
            return std::make_optional(state++);
        }
    };

    /**
     * This is NOT meant to be used DIRECTLY. Use method `Iter::take` instead.
     *
     * Takes an iterator as an input and limits the number of output values to a certain number. Can be used to transform
     * infinite iterator into a finite one.
     * @tparam Iter Iterator whose output should be limited.
     */
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

        LimitIterator(Iter &&iter, usize limit) : limit{limit}, iter{std::move(iter)} {}

        std::optional<typename Iter::value_type> next() {
            if (count >= limit)
                return {};
            count++;
            return iter.next();
        }
    };


    /**
     * This is NOT meant to be used DIRECTLY. Use method `Iter::zip` instead.
     *
     * Given two iterators, merge them and return `std::pair` of the values yielded by both of them. There will always be
     * valid values in the pair. The iterator stops, when there are not enough data to zip.
     * @tparam Iter1 Type of first iterator to zip
     * @tparam Iter2 Type of second iterator to zip
     */
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

        ZipIterator(Iter1 &&iter1, Iter2 &&iter2) : iter1{std::move(iter1)}, iter2{std::move(iter2)} {}

        std::optional<value_type> next() {
            auto a = iter1.next();
            auto b = iter2.next();

            if (a && b) {
                return std::optional(std::pair{*a, *b});
            } else {
                return std::nullopt;
            }
        }
    };


}

/**
 * This class IS MEANT to be used directly.
 *
 * This is a helper class used for wrapping the basic iterators into a more powerful versions of themselves. In itself,
 * it has all the required properties of an iterator and it just mirrors the behaviour of the iterator it was supplied
 * with in the beginning. However, it provides extra methods for transforming itself to other versions of itself with
 * different behaviour. The abstraction of this wrapper iterator is meant to be compiled away and should not lead to any
 * performance penalties.
 *
 *
 * @tparam Iter Iterator type to wrap
 */
template<typename Iter>
class I {
public:
    using value_type = typename Iter::value_type;
private:
    std::optional<Iter> iter;
    std::optional<value_type> last_value_for_oldschool_iter = {};

    template<typename OtherIter>
    I<OtherIter> wrap_iter(OtherIter &&iter) {
        return I<OtherIter>(std::forward<OtherIter>(iter));
    }

public:

    I(I &&old) = default;

    I(I &other) = delete;

    I(Iter &&iter) : iter{std::move(iter)} {}

    static_assert(is_iterator<Iter>::value);

    /**
     * Return differently templated instance of `I` returning data of this iterator after they have been transformed by
     * the supplied function
     * @tparam Func Transformation function, which will take ownership of the data and return it again
     * @param f the function
     * @return instance of Self
     */
    template<typename Func>
    auto map(Func &&f) {
        assert(iter);
        MapIterator<Iter, Func> mi(std::move(*iter), std::forward<Func>(f));
        iter.reset();
        return wrap_iter(std::move(mi));
    }

    /**
     * Add function which will run for every element of the iterator to the iterator pipeline and return the new iterator.
     * The function is evaluated lazily, so it will not run, when `next()` is not called.
     * @tparam Func
     * @param f
     * @return
     */
    template<typename Func>
    auto lazy_for_each(Func &&f) {
        assert(iter);
        MapIterator mi(std::move(*iter), [=](value_type p) {
            value_type const &r = p;
            f(r);
            return p;
        });
        iter.reset();
        return wrap_iter(std::move(mi));
    }

    /**
     * Exhaust the iterator and std::move every resulting item into the supplied function.
     * @tparam Func
     * @param f
     */
    template<typename Func>
    void into(Func &&f) {
        assert(iter);
        MapIterator mi(std::move(*iter), [=](value_type p) {
            f(std::move(p));
            return 0;
        });
        iter.reset();
        wrap_iter(std::move(mi)).exhaust();
    }

    /**
     * Return new iterator with filtered out elements by the supplied filter function.
     * @tparam Func
     * @param f
     * @return Instance of I with filtered data
     */
    template<typename Func>
    auto filter(Func &&f) {
        assert(iter);
        FilterIterator<Iter, Func> fi(std::move(*iter), f);
        iter.reset();
        return wrap_iter(std::move(fi));
    }

    /**
     * Get next element of the iterator...
     * @return Next element of the iterator as an option
     */
    std::optional<value_type> next() {
        assert(iter);
        return (*iter).next();
    }

    /**
     * Collect the values of the iterator into a `std::vector`
     * @return Vector with the values...
     */
    auto collect() {
        assert(iter);
        std::vector<value_type> result;
        while (auto a = (*iter).next()) result.push_back(*a);
        return result;
    }

    /**
     * Exhausts the iterator. Calls the `next()` function for as long as it returns something.
     */
    void exhaust() {
        assert(iter);
        while (iter->next()) {}
    }

    /**
     * Returns an iterator with limited number of items to the supplied amount.
     * @param count Limiting amount of resulting items.
     * @return
     */
    auto take(usize count) {
        assert(iter);
        LimitIterator li(std::move(*iter), count);
        iter.reset();
        return wrap_iter(std::move(li));
    }

    /**
     * Collapses the iterator into the resulting state using the supplied fold function. For more explanation, see
     * (Wikipedia)[https://en.wikipedia.org/wiki/Fold_%28higher-order_function%29].
     *
     * @tparam Func
     * @tparam State
     * @param f the folding function
     * @param s initial state
     * @return state after aggregation with all the items in the iterator
     */
    template<typename Func, typename State>
    State fold(Func &&f, State &&s) {
        static_assert(
                !std::is_reference_v<decltype(f(std::declval<typename Iter::value_type>(), std::declval<State>()))>,
                "Function returns a reference, but it should return by value.");
        static_assert(!std::is_reference_v<State>, "State in fold can't be reference.");

        assert(iter);
        while (auto a = iter->next()) {
            State ss = f(std::move(*a), std::forward<State>(s));
            s = std::move(ss);
        }
        return std::forward<State>(s);
    }

    /**
     * Same as fold, takes the first value of the iterator as the initial. Returns the resulting state in an option due
     * to the possibility of the iterator being empty.
     *
     * @tparam Func
     * @param f the folding function
     * @return
     */
    template<typename Func>
    std::optional<typename Iter::value_type> reduce(Func &&f) {
        static_assert(std::is_same_v<typename Iter::value_type, decltype(f(std::declval<typename Iter::value_type>(),
                                                                           std::declval<typename Iter::value_type>()))>,
                      "Return type is not the same as iterator type!");
        assert(iter);
        auto first = iter->next();
        if (!first) {
            return std::nullopt;
        }

        return std::make_optional(fold(std::forward<Func>(f), std::move(*first)));
    }

    using iterator = I &;

    /**
     * For compatibility reasons with classical C++ code. Simulates the behaviour of C++ classical "iterators". Behaves as
     * expected as would other classical C++ "iterators" do.
     */
    std::reference_wrapper<I> begin() {
        last_value_for_oldschool_iter = next();
        return std::reference_wrapper(*this);
    }

    /**
     * For compatibility reasons with classical C++ code. Simulates the behaviour of C++ classical "iterators". Behaves as
     * expected as would other classical C++ "iterators" do.
     */
    friend value_type &operator*(std::reference_wrapper<I> &me) {
        if (me.get().last_value_for_oldschool_iter)
            return *(me.get().last_value_for_oldschool_iter);
        else
            throw std::range_error("Iterator range overrun!");
    }


    /**
     * For compatibility reasons with classical C++ code. Simulates the behaviour of C++ classical "iterators". Behaves as
     * expected as would other classical C++ "iterators" do.
     *
     * We actualy don't care about the data in this. We return ourselves just because it has to has the same type as
     * the return type of the `begin()` function.
     */
    std::reference_wrapper<I> end() {
        // cause the comparison operator will report ending when comparing with anything, we can just return any garbage
        return std::reference_wrapper(*this);
    }

    /**
     * For compatibility reasons with classical C++ code. Simulates the behaviour of C++ classical "iterators". Behaves as
     * expected as would other classical C++ "iterators" do.
     */
    friend void operator++(std::reference_wrapper<I> &me) {
        me.get().last_value_for_oldschool_iter = me.get().next();
    }

    /**
     * For compatibility reasons with classical C++ code. Simulates the behaviour of C++ classical "iterators". Behaves as
     * expected as would other classical C++ "iterators" do.
     *
     * Returns whether we run out of values. The comparison is otherwise useless, it's here just for compatibility
     * reasons with the archaic C++ iterators.
     */
    friend bool operator==(std::reference_wrapper<I> &me, std::reference_wrapper<I> &_) {
        return !me.get().last_value_for_oldschool_iter.has_value();
    }

    /**
     * See the operator==
     */
    friend bool operator!=(std::reference_wrapper<I> &me, std::reference_wrapper<I> &other) {
        return !(me == other);
    }

    /**
     * Transform the iterator into an iterator of pairs where the first pair is the index of the item being processed.
     * @return
     */
    auto enumerate() {
        assert(iter);
        return wrap_iter(ZipIterator(IncrementIter((usize) 0), std::move(*iter)));
    }

    /**
     * Sums the values in the iterator and returns the result.
     * @return the sum of all values
     */
    auto sum() {
        assert(iter);
        return fold([](auto v, auto s) { return v + s; }, (usize) 0);
    }

    /**
     * Sums the values in the iterator and returns the result. Takes an inital value as an argument...
     * @param initialValue initial value
     * @return the resulting sum
     */
    auto sum(typename Iter::value_type initialValue) {
        assert(iter);
        return fold([](auto v, auto s) { return v + s; }, initialValue);
    }

    /**
     * Returns the maximal item of the iterator. When there are multiple with the same size, returns the first.
     * @return
     */
    std::optional<typename Iter::value_type> max() {
        assert(iter);
        return reduce([](auto a, auto b) { return a > b ? a : b; });
    }

    /**
     * Returns the maximal item of the iterator. When there are multiple with the same size, returns the first.
     * The comparison is done on values returned by the supplied function
     * @tparam Func
     * @param f function transforming the item into a comparable element
     * @return
     */
    template<typename Func>
    std::optional<typename Iter::value_type> max_by(Func &&f) {
        assert(iter);
        return reduce([=](auto a, auto b) { return f(a) > f(b) ? a : b; });
    }

    /**
     * See `max()`
     * @return
     */
    std::optional<typename Iter::value_type> min() {
        assert(iter);
        return reduce([](auto a, auto b) { return a < b ? a : b; });
    }

    /**
     * See `max_by()`
     * @tparam Func
     * @param f
     * @return
     */
    template<typename Func>
    std::optional<typename Iter::value_type> min_by(Func &&f) {
        assert(iter);
        return reduce([=](auto a, auto b) { return f(a) < f(b) ? a : b; });
    }
};

namespace Iter {

    /**
     * Given an initial value, continue by incrementing the value to infinity and return the resulting iterator.
     * @tparam T
     * @param init
     * @return
     */
    template<typename T>
    auto count_from(T init) {
        return I(IncrementIter(std::move(init)));
    }

    /**
     * Return an iterator yielding numbers in the configured range.
     * @param fromInclusive
     * @param toExclusive
     * @return
     */
    auto range(usize fromInclusive, usize toExclusive) {
        return count_from(fromInclusive).take(toExclusive - fromInclusive);
    }

    /**
     * Returns an iterator yielding numbers from zero to the specified limit
     * @param toExclusive
     * @return
     */
    auto range(usize toExclusive) {
        return count_from((usize) 0).take(toExclusive);
    }

    /**
     * Return line by line iterator of a file specified by its name.
     * @param file filename
     * @return
     */
    auto file_by_lines(const std::string &file) {
        return I(StreamLineIterator<std::ifstream>(std::ifstream(file)));
    }

    /**
     * Return iterator of the lines in the standard input.
     * @return
     */
    auto stdin_by_lines() {
        return I(StreamLineIterator<std::istream &>(std::cin, false));
    }

    /**
     * Zip two iterators together returning an iterator of pairs
     *
     * @tparam Iter1
     * @tparam Iter2
     * @param iter1
     * @param iter2
     * @return
     */
    template<typename Iter1, typename Iter2>
    auto zip(Iter1 &&iter1, Iter2 &&iter2) {
        static_assert(is_iterator<Iter1>::value);
        static_assert(is_iterator<Iter2>::value);
        return I(std::move(ZipIterator(std::move(iter1), std::move(iter2))));
    }

    /**
     * Return an iterator of a iterable container. Targeted mainly at containers from STL
     * @tparam Container
     * @param vec
     * @return
     */
    template<typename Container>
    auto from(Container &&vec) {
        return I(std::move(ObsoleteIteratorConverter(vec.begin(), vec.end())));
    }

    /**
     * Return an iterator of a iterable container. Targeted mainly at containers from STL
     * @tparam Container
     * @param vec
     * @return
     */
    template<typename Container>
    auto from(Container &vec) {
        return I(std::move(ObsoleteIteratorConverter(vec.begin(), vec.end())));
    }
}