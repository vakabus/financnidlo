# Iterators

I didn't like how the default C++ iterators work. So I implemented my own using a ton of templates 
and few C++17 features.

## Main concepts

* simple interface for iterators - iterators return `optional` via method `next()` and they define `value_type`
* can be wrapped with a wrapper from the library, which will implement all the other nice things
    * functional `map()`, `filter()`, `reduce()`, `fold()` and more
    * run lambda when a value passes by in the iterator pipeline via `lazy_for_each()`
    * run lambda for each value by `into()`
    * all this without virtual dispatch

```c++
struct YourAwesomeIter {
    using value_type = bool;
    optional<bool> next();
}


// to use nice methods with your iterator, wrap it like this
auto iter = I(YourAwesomeIter());
```

* everything is **moved**, all library copy constructors were deleted

## Implementation

The whole library sits in `iterators.h` file. There is nothing more to it than that.

## Tests

Tests can be run via `make test`. Not everything is covered by them.