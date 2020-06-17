# CMakeList.txt : CMake project for KVBench, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)


# Add source to this project's executable.
add_executable (KVBench "main.cpp" "Parsers/fastkv/fastkv.c" "Parsers/fastkv/fastkv.h" "Parsers/QuickKV/KVParser/quickkv.cpp" "Parsers/QuickKV/KVParser/quickkv.h")

# TODO: Add tests and install targets if needed.
