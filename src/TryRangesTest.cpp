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

  SUBCASE("dot product") {
    CHECK_EQ(val,
             rg::accumulate(rv::zip_with(rg::multiplies(), r_rev, r_pow), .0));
  }
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
  auto rng = rv::generate([a = 0, b = 1]() mutable {
    const auto at = a;
    a = std::exchange(b, a + b);
    return at;
  });
  auto fib10 = rng | rv::take(10);
  CHECK_UNARY(rg::equal(fib10, std::array{0, 1, 1, 2, 3, 5, 8, 13, 21, 34}));
}

TEST_CASE("Caesar cipher") {
  const int shift = 11;
  const auto s = "apple"s;

  auto alphabet = rv::closed_iota('a', 'z') | rv::cycle;
  auto shifted_alphabet = alphabet | rv::drop(shift);

  auto encrypted = s | rv::for_each([shifted_alphabet](char c) {
                     return shifted_alphabet | rv::drop(c - 'a') | rv::take(1);
                   });
  auto s_encrypted = encrypted | rg::to<std::string>();
  CHECK_EQ(s_encrypted, "laawp");
}

TEST_CASE("Triangular sequence") {
  auto r_int = rv::iota(1);                            // [1,2,3,4,5...]
  auto r_triseq = r_int | rv::partial_sum(rg::plus()); // [1,3,6,10,15...]
  auto tri5 = r_triseq | rv::take(5);
  CHECK_UNARY(rg::equal(tri5, std::array{1, 3, 6, 10, 15}));
}

TEST_CASE("accumulate, foldl") {
  // foldl (+) 0.0 [1,5,3.7,3.8,4.2]
  const std::vector v = {1.5, 2.7, 3.8, 4.2};
  const double val = rg::accumulate(v, .0);
  CHECK_EQ(val, doctest::Approx(12.2));

  // foldl (*) 1 [1,2,3,4]
  const std::vector m = {1, 2, 3, 4};
  const int mal = rg::accumulate(m, 1, rg::multiplies());
  CHECK_EQ(mal, 24);
}

TEST_CASE("copy") {
  const std::vector v = {6, 4, 1, 8, 3};
  std::vector<int> v2;
  rg::copy(v, rg::back_inserter(v2));
  CHECK_EQ(v, v2);

  const auto v3 = rg::copy(v);
  CHECK_EQ(v, v3);
}

TEST_CASE("count") { CHECK_EQ(rg::count(std::array{1, 2, 7, 4, 1, 7}, 7), 2); }

TEST_CASE("distance") {
  const std::vector v1 = {'a', 'b', 'c'};
  const std::vector v2 = {'c', 'd', 'e'};
  const auto v3 = rv::set_union(v1, v2);
  CHECK_EQ(rg::distance(v3), 5);
}

TEST_CASE("equal") {
  const std::array v1 = {1, 2, 3};
  const std::array v2 = {3, 2, 1};
  CHECK_UNARY(!rg::equal(v1, v2));
}

TEST_CASE("find") {
  constexpr std::array v = {5, 6, 7, 8, 7};
  const auto *const it = rg::find(v, 7);
  CHECK_EQ(it, v.cbegin() + 2);
}

TEST_CASE("front") {
  constexpr std::array v = {5, 6, 7, 8};
  CHECK_EQ(rg::front(v), 5);
}

TEST_CASE("inner_product") {
  constexpr std::array v1 = {1, 3, -5};
  constexpr std::array v2 = {4, -2, -1};
  CHECK_EQ(rg::inner_product(v1, v2, 0), 3); // 1*4+3*(-2)+(-5)*(-1)+0=3
}

TEST_CASE("partition_copy") {
  constexpr std::array v = {6, 4, 1, 8, 3};
  std::vector<int> even;
  std::vector<int> odd;

  rg::partition_copy(v, rg::back_inserter(even), rg::back_inserter(odd),
                     [](int i) { return i % 2 == 0; });
  CHECK_UNARY(rg::equal(even, std::array{6, 4, 8}));
  CHECK_UNARY(rg::equal(odd, std::array{1, 3}));
}

TEST_CASE("sort") {
  std::array v = {6, 7, 1, 3};
  rg::sort(v);
  CHECK_UNARY(rg::equal(v, std::array{1, 3, 6, 7}));

  std::array v1 = {6, 7, 1, 3};
  rg::sort(v1, rg::greater());
  CHECK_UNARY(rg::equal(v1, std::array{7, 6, 3, 1}));

  struct Elem {
    std::string name;
    double density;
  };
  auto v2 = std::array{Elem{"Au", 19.3}, Elem{"Cu", 8.96}, Elem{"Ag", 10.5}};
  rg::sort(v2, rg::less(), &Elem::density);
  CHECK_UNARY(rg::equal(v2, std::array{"Cu", "Ag", "Au"}, {}, &Elem::name));
}

TEST_CASE("views::all, stl container -> view") {
  constexpr std::array v = {1, 2, 3, 4, 5};
  auto rng = v | rv::all;
  CHECK_UNARY(rg::equal(rng, v));
}

TEST_CASE("views::chunk") {
  constexpr std::array v = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  auto rng = v | rv::chunk(4); // [[1,2,3,4], [5,6,7,8], [9]]
  CHECK_EQ(rng.size(), 3);
}

TEST_CASE("views::chunk_by replaces views::group_by") {
  std::array v = {7, 4, 2, 6, 9};
  rg::sort(v);
  [[maybe_unused]] auto rng = v | rv::group_by([](int a, int b) {
                                return a % 2 == b % 2;
                              }); // [[2,4,6],[7,9]]
  CHECK_EQ(2, 2);

  std::string s = "radar";
  rg::sort(s);
  [[maybe_unused]] auto rng1 = s | rv::group_by([](char a, char b) {
                                 return a == b;
                               }); // [[a,a],[d],[r,r]]
  CHECK_EQ(3, 3);
}

TEST_CASE("views::common to work with std algo") {
  constexpr std::array v = {8, 7, 3};
  auto rng = v | rv::take_while([](int a) { return a > 5; }) | rv::common;
  const int res = std::accumulate(rng.begin(), rng.end(), 0);
  CHECK_EQ(res, 15);
}

TEST_CASE("views::concat") {
  auto v1 = rv::iota(3, 6);
  auto v2 = rv::iota(7, 9);
  auto v3 = rv::iota(11, 14);
  auto rng = rv::concat(v1, v2, v3);
  CHECK_UNARY(rg::equal(rng, std::array{3, 4, 5, 7, 8, 11, 12, 13}));
}

TEST_CASE("views::counted, get a subrange") {
  auto v = rv::closed_iota(6, 10);
  auto rng = rv::counted(v.begin() + 2, 3); // subrange [2,2+3)
  CHECK_UNARY(rg::equal(rng, std::array{8, 9, 10}));
}

TEST_CASE("views::slice") {
  auto v = rv::closed_iota(6, 10);
  CHECK_UNARY(
      rg::equal(v | rv::slice(2, 2 + 3), rv::counted(v.begin() + 2, 3)));
}

TEST_CASE("views::cycle") {
  const std::array v = {1, 3, 9};
  auto rng = v | rv::cycle;
  CHECK_UNARY(
      rg::equal(rv::counted(rng.begin(), 7), std::array{1, 3, 9, 1, 3, 9, 1}));
}

TEST_CASE("views::delimit") {
  constexpr std::array v = {5, 8, 9, 13, 10, 9};
  auto rng = v | rv::delimit(9);
  CHECK_UNARY(rg::equal(rng, std::array{5, 8}));
}

TEST_CASE("views::drop") {
  constexpr std::array v = {4, 9, 3, 1, 7};
  auto rng = v | rv::drop(2);
  CHECK_UNARY(rg::equal(rng, std::array{3, 1, 7}));
}

TEST_CASE("views::drop_while") {
  constexpr std::array v = {2, 3, 5, 6, 7};
  auto rng = v | rv::drop_while([](int a) { return a < 5; });
  CHECK_UNARY(rg::equal(rng, std::array{5, 6, 7}));
}

TEST_CASE("views::take") {
  CHECK_UNARY(
      rg::equal(rv::closed_iota(1, 5) | rv::take(3), std::array{1, 2, 3}));
}

TEST_CASE("views::take_while") {
  constexpr std::array v = {2, 3, 5, 6, 7};
  auto rng = v | rv::take_while([](int a) { return a < 5; });
  CHECK_UNARY(rg::equal(rng, std::array{2, 3}));
}

TEST_CASE("views::enumerate") {
  constexpr std::array v = {"apple"sv, "banana"sv, "kiwi"sv};
  for (auto [first, second] : v | rv::enumerate) {
    CHECK_EQ(second, v[first]);
  }
}

TEST_CASE("views::exclusive_scan") {
  constexpr std::array v = {1, 2, 3, 4};
  auto rng = v | rv::exclusive_scan(10); // [10,10+1,10+1+2,10+1+2+3]
  CHECK_UNARY(rg::equal(rng, std::array{10, 11, 13, 16}));
}

TEST_CASE("views::filter") {
  constexpr std::array v = {7, 4, 2, 6, 9};
  auto rng = v | rv::filter([](int a) { return a > 6; });
  CHECK_UNARY(rg::equal(rng, std::array{7, 9}));
}

TEST_CASE("views::remove_if") {
  auto rng =
      rv::closed_iota(1, 5) | rv::remove_if([](int x) { return x % 2 == 0; });
  CHECK_UNARY(rg::equal(rng, std::array{1, 3, 5}));
}

TEST_CASE("views::for_each, generate subranges -> flatten") {
  const auto v = {1, 10, 100};
  auto rng = rv::for_each(
      v, [](int i) { return rv::linear_distribute(i - 1, i + 1, 2); });
  CHECK_UNARY(rg::equal(
      rng, std::array{1 - 1, 1 + 1, 10 - 1, 10 + 1, 100 - 1, 100 + 1}));
}

TEST_CASE("views::generate") {
  // infinite list of power of 2
  auto rng = rv::generate([n = 1U]() mutable { return (n <<= 1U) >> 1U; }) |
             rv::take(5);
  CHECK_UNARY(rg::equal(rng, std::array{1, 2, 4, 8, 16}));
}

TEST_CASE("views::intersperse") {
  const auto s = "London"s;
  const auto s2 = s | rv::intersperse('_') | rg::to<std::string>();
  CHECK_EQ(s2, "L_o_n_d_o_n");
}

TEST_CASE("views::join") {
  const std::vector<std::vector<int>> v = {{
                                               1,
                                               3,
                                           },
                                           {11, 13, 15},
                                           {25}};
  auto rng = v | rv::join;
  CHECK_UNARY(rg::equal(rng, std::array{1, 3, 11, 13, 15, 25}));
}

TEST_CASE("views::keys") {
  const std::map<std::string_view, int> m = {{"London"sv, 6},
                                             {"New York"sv, 7}};
  auto rng = m | rv::keys;
  CHECK_UNARY(rg::equal(rng, std::array{"London"sv, "New York"sv}));
}

TEST_CASE("views::values") {
  const std::map<std::string_view, int> m = {{"London"sv, 6},
                                             {"New York"sv, 7}};
  auto rng = m | rv::values;
  CHECK_UNARY(rg::equal(rng, std::array{6, 7}));
}

TEST_CASE("views::linear_distribute") {
  auto rng = rv::linear_distribute(1, 10, 3);
  CHECK_EQ(fmt::format("{}", fmt::join(rng, ",")), "1,5,10");
}

TEST_CASE("views::partial_sum, scanl") {
  constexpr std::array v = {1, 2, 3, 4};
  auto rng = v | rv::partial_sum(rg::plus());
  CHECK_UNARY(rg::equal(rng, std::array{1, 3, 6, 10}));
}

TEST_CASE("views::repeat") {
  CHECK_UNARY(rg::equal(rv::repeat(4) | rv::take(3), std::array{4, 4, 4}));
}

TEST_CASE("views::reverse") {
  CHECK_UNARY(
      rg::equal(rv::closed_iota(1, 4) | rv::reverse, std::array{4, 3, 2, 1}));
}

TEST_CASE("views::set_difference") {
  auto rng = rv::set_difference(rv::closed_iota(3, 7), rv::closed_iota(4, 5));
  CHECK_UNARY(rg::equal(rng, std::array{3, 6, 7}));
}

TEST_CASE("views::set_intersection") {
  auto rng = rv::set_intersection(rv::closed_iota(3, 6), rv::closed_iota(5, 8));
  CHECK_UNARY(rg::equal(rng, std::array{5, 6}));
}

TEST_CASE("views::set_union") {
  auto rng = rv::set_union(rv::closed_iota(1, 3), rv::closed_iota(4, 6));
  CHECK_UNARY(rg::equal(rng, rv::closed_iota(1, 6)));
}

TEST_CASE("views::single, make sth a range") {
  CHECK_UNARY(rg::equal(rv::single(3), std::array{3})); // a -> [a]

  auto rng = rv::single(rv::iota(1, 5)); // [1,2,3,4] -> [[1,2,3,4]]
  CHECK_EQ(rng.size(), 1U);
}

TEST_CASE("views::sliding") {
  auto v = rv::iota(1, 6);
  auto rng = v | rv::sliding(2); // [[1,2],[2,3],[3,4],[4,5]]
  CHECK_EQ(rng.size(), 4);
}

TEST_CASE("views::stride") {
  auto rng = rv::closed_iota(0, 6) | rv::stride(3);
  CHECK_UNARY(rg::equal(rng, std::array{0, 3, 6}));
}

TEST_CASE("views::split") {
  const std::string_view s = "hello  world"sv;
  auto rng = s | rv::split(' ') | rg::to<std::vector<std::string>>();
  CHECK_UNARY(rg::equal(rng, std::array{"hello"sv, ""sv, "world"sv}));
}

TEST_CASE("views::tokenize") {
  const std::string_view s = "Have a nice   day!"sv;
  const std::regex r{"[\\w]+"};
  auto rng = s | rv::tokenize(r); // [Have,a,nice,day(!)?]
  CHECK_EQ(4, 4); // tokenize_view doesn't work with common stuff
}

TEST_CASE("views::tail") {
  auto rng = rv::closed_iota(1, 4) | rv::tail;
  CHECK_UNARY(rg::equal(rng, std::array{2, 3, 4}));
}

TEST_CASE("views::transform") {
  auto rng = rv::closed_iota(1, 3) | rv::transform([](int x) { return 2 * x; });
  CHECK_UNARY(rg::equal(rng, std::array{2, 4, 6}));
}

TEST_CASE("views::unique, discard neighbouring duplicates") {
  const std::array v = {1, 2, 2, 3, 1, 1, 2, 2};
  auto rng = v | rv::unique;
  CHECK_UNARY(rg::equal(rng, std::array{1, 2, 3, 1, 2}));
}

TEST_CASE("views::zip") {
  constexpr std::array v_ar = {1, 10, 100, 1000, 10000};
  constexpr auto v_cn = std::array{"一"sv, "十"sv, "百"sv, "千"sv, "万"sv};
  auto rng = rv::zip(v_ar, v_cn);
  for (const auto &[i, p] : rng | rv::enumerate) {
    const auto &[ar, cn] = p;
    CHECK_EQ(ar, v_ar[i]);
    CHECK_EQ(cn, v_cn[i]);
  }
}

TEST_CASE("views::zip_with") {
  const auto v1 = std::array{1, 3, 5};
  const auto v2 = std::array{2, 4, 6};
  auto rng = rv::zip_with(rg::plus(), v1, v2);
  CHECK_UNARY(rg::equal(rng, std::array{3, 7, 11}));
}

TEST_CASE("All-digit magic") {
  auto all_pairs = rv::closed_iota(100, 999) | rv::for_each([](int i) {
                     const auto contain_digits = [](std::string s) {
                       rg::sort(s);
                       auto r = s | rv::unique | rg::to<std::string>();
                       return r.size() == 9 && r.find('0') == std::string::npos;
                     };

                     return rg::yield_if(contain_digits(std::to_string(i) +
                                                        std::to_string(i * i)),
                                         std::make_pair(i, i * i));
                   });
  CHECK_UNARY(rg::equal(all_pairs, std::array{std::make_pair(567, 321489),
                                              std::make_pair(854, 729316)}));
}

TEST_CASE("triangles 1") {
  auto a = rv::closed_iota(1, 10);
  auto b = rv::closed_iota(1, 10);
  auto c = rv::closed_iota(1, 10);
  auto result = rv::cartesian_product(a, b, c) | rv::for_each([](auto p) {
                  auto [x, y, z] = p;
                  return rg::yield_if(z > y && y >= x && x * x + y * y == z * z,
                                      std::make_tuple(x, y, z));
                });
  CHECK_UNARY(rg::equal(
      result, std::array{std::make_tuple(3, 4, 5), std::make_tuple(6, 8, 10)}));
}

TEST_CASE("triangles 2") {
  auto result =
      rv::closed_iota(1, 10) | rv::for_each([](int c) {
        return rv::closed_iota(1, c) | rv::for_each([c](int b) {
                 return rv::closed_iota(1, b) | rv::for_each([c, b](int a) {
                          return rg::yield_if(a * a + b * b == c * c,
                                              std::make_tuple(a, b, c));
                        });
               });
      });
  CHECK_UNARY(rg::equal(
      result, std::array{std::make_tuple(3, 4, 5), std::make_tuple(6, 8, 10)}));
}
