# CMakeList.txt : CMake project for KVBench, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# for fastkv
set(NO_CONDITIONS ON)

subdirs(Parsers/fastkv)

# Add source to this project's executable.
add_executable (KVBench "main.cpp" "Parsers/QuickKV/KVParser/quickkv.cpp" "Parsers/QuickKV/KVParser/quickkv.h")

target_link_libraries(KVBench libfastkv)

# TODO: Add tests and install targets if needed.
