cmake_minimum_required(VERSION 3.14)
project(lab1_trees LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(lab1_core
  src/elements.cpp src/document.cpp src/rules.cpp
  src/session.cpp src/shell.cpp src/demo.cpp)
target_include_directories(lab1_core PUBLIC include)

add_executable(lab1 src/main.cpp)
target_link_libraries(lab1 PRIVATE lab1_core)

enable_testing()
add_executable(lab1_tests tests/tests.cpp)
target_link_libraries(lab1_tests PRIVATE lab1_core)
add_test(NAME lab1_tests COMMAND lab1_tests)
