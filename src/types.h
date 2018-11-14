#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

using usize = std::size_t;
using isize = std::ptrdiff_t;
using u32 = std::uint_fast32_t;
using i32 = std::int_fast32_t;
using u64 = std::uint_fast64_t;
using i64 = std::int_fast64_t;

using std::optional;
using std::make_optional;
using std::nullopt;
using std::move;

// helper type for the visitor #4
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
