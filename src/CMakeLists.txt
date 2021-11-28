add_library(doctest-main DoctestMain.cpp)
target_link_libraries(doctest-main PUBLIC project_defaults doctest::doctest)

find_package(fmt REQUIRED)

add_executable(try-ranges TryRangesTest.cpp)
target_link_libraries(try-ranges PRIVATE doctest-main fmt::fmt)
target_compile_definitions(try-ranges PRIVATE FMT_ENFORCE_COMPILE_STRING)
doctest_discover_tests(try-ranges ADD_LABELS 0)

install(TARGETS try-ranges DESTINATION bin)
