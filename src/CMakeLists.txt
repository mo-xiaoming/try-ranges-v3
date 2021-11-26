add_library(doctest-main DoctestMain.cpp)
target_link_libraries(doctest-main PUBLIC project_defaults doctest::doctest)

add_library(mx-lib Utils.cpp)
target_link_libraries(mx-lib PUBLIC project_defaults)

add_executable(mx-lib-test UtilsTest.cpp)
target_link_libraries(mx-lib-test PRIVATE doctest-main mx-lib)
# https://github.com/onqtam/doctest/pull/464 requires ADD_LABELS
doctest_discover_tests(mx-lib-test ADD_LABELS 0)

find_package(fmt REQUIRED)

add_executable(try-ranges TryRangesTest.cpp)
target_link_libraries(try-ranges PRIVATE doctest-main fmt::fmt)
doctest_discover_tests(try-ranges ADD_LABELS 0)

install(TARGETS try-ranges DESTINATION bin)
