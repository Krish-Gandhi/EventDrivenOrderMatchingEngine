cmake_minimum_required(VERSION 3.15)
project(engine_py_module)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 REQUIRED)


pybind11_add_module(engine_py
    engine.cpp
    bindings.cpp
)

target_include_directories(engine_py PRIVATE ${Python3_INCLUDE_DIRS})
target_link_libraries(engine_py PRIVATE ${Python3_LIBRARIES})