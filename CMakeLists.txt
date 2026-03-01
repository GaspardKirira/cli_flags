cmake_minimum_required(VERSION 3.20)
project(cli_flags LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(cli_flags INTERFACE)
add_library(cli_flags::cli_flags ALIAS cli_flags)

target_include_directories(cli_flags INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

if (MSVC)
  target_compile_options(cli_flags INTERFACE /W4 /permissive-)
else()
  target_compile_options(cli_flags INTERFACE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
enable_testing()

add_executable(cli_flags_basic_test tests/test_basic.cpp)
target_link_libraries(cli_flags_basic_test PRIVATE cli_flags::cli_flags)
add_test(NAME cli_flags.basic COMMAND cli_flags_basic_test)

# -------------------------
# Examples
# -------------------------

add_executable(cli_flags_example_basic examples/basic_parse.cpp)
target_link_libraries(cli_flags_example_basic PRIVATE cli_flags::cli_flags)

add_executable(cli_flags_example_subcommands examples/subcommands.cpp)
target_link_libraries(cli_flags_example_subcommands PRIVATE cli_flags::cli_flags)

add_executable(cli_flags_example_help examples/help_only.cpp)
target_link_libraries(cli_flags_example_help PRIVATE cli_flags::cli_flags)

add_executable(cli_flags_example_minimist examples/minimist_style.cpp)
target_link_libraries(cli_flags_example_minimist PRIVATE cli_flags::cli_flags)
