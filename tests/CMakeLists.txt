cmake_minimum_required(VERSION 3.21)
project(water_gun_tests LANGUAGES C)

enable_testing()

add_executable(gun_utils_tests
    test_gun_utils.c
    ../gun_utils.c
)
target_include_directories(gun_utils_tests PRIVATE ..)
target_compile_features(gun_utils_tests PRIVATE c_std_11)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(gun_utils_tests PRIVATE -Wall -Wextra -Werror)
endif()

foreach(suite IN ITEMS integer delimiter tenths command)
    add_test(NAME gun_utils.${suite} COMMAND gun_utils_tests ${suite})
endforeach()
