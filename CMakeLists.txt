cmake_minimum_required(VERSION 3.20)
project(js)

set(CMAKE_CXX_STANDARD 20)

add_executable(js main.cpp ast.cpp ast.h interpreter.cpp interpreter.h)
