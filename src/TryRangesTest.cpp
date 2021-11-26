#include <doctest/doctest.h>

#include <fmt/format.h>
#include <range/v3/all.hpp>

using namespace std::literals;

namespace rg = ranges;
namespace rv = ranges::views;

TEST_CASE("The purpose of std::views::common in C++20 Ranges") {
  const auto v = {8, 7, 3};
  auto rng = v | rv::filter([](int x) { return x > 5; });
  const auto res = rg::accumulate(rng, 0);
  CHECK_EQ(res, 15);
}

TEST_CASE("Resistors in parallel, 1/R = 1/R1 + 1/R2 + 1/R3") {
  const auto v = {20, 10, 15};
  auto r_inv = v | rv::transform([](int x) { return 1.0 / x; });
  // r_inv = [0.05,0.1,0.0666667]
  const auto val = 1.0 / rg::accumulate(r_inv, 0.0);
  // v = 1.0 / 0.216667 = 4.61538
  CHECK_EQ(val, doctest::Approx(4.61538));
}

TEST_CASE("Binary to decimal conversion, 0b1110 = 14") {
  const std::vector<uint8_t> v = {1, 1, 1, 0};
  auto r_rev = v | rv::reverse;              // [0,1,1,1]
  auto r_int = rv::iota(0, rg::distance(v)); // [0,1,2,3];
  auto r_pow =
      r_int | rv::transform([](unsigned x) { return 1U << x; }); // [1,2,4,8]
  auto val = rg::inner_product(r_rev, r_pow, 0); // 0*1 + 1*2 + 1*4 + 1*8 = 14
  CHECK_EQ(val, 14);
}

TEST_CASE("snake_case to CamelCase") {
  const auto s = "feel_the_force"sv;
  auto words = s | rv::split('_'); // [[f,e,e,l],[t,h,e],[f,o,r,c,e]]
  auto words_cap = words | rv::transform([](auto w) {
                     auto head = w | rv::take(1) | rv::transform([](int c) {
                                   return std::toupper(c);
                                 });
                     return rv::concat(head, w | rv::tail);
                   }); // [[F,e,e,l],[T,h,e],[F,o,r,c,e]]
  auto s_camelcase = words_cap | rv::join | rg::to<std::string>();
  CHECK_EQ(s_camelcase, "FeelTheForce");
}

TEST_CASE("Fibonacci sequence") {
  auto rng = rv::generate([p = std::pair{0, 1}]() mutable {
    auto [a0, b0] = p;
    p = {b0, a0 + b0};
    return a0;
  });
  auto fib10 = rng | rv::take(10);
  CHECK_EQ(fmt::format("{}", fmt::join(fib10, ",")), "0,1,1,2,3,5,8,13,21,34");
}
