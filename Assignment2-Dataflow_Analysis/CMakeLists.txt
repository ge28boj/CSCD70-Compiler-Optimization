cmake_minimum_required(VERSION 3.12.0)
project(DFA)

set(LLVM_VERSION "12" CACHE STRING "LLVM Version")
set(LLVM_VERSION_SUFFIX "-${LLVM_VERSION}")

execute_process(COMMAND llvm-config${LLVM_VERSION_SUFFIX} --cxxflags
                OUTPUT_VARIABLE LLVM_CXXFLAGS
                OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND llvm-config${LLVM_VERSION_SUFFIX} --includedir
                OUTPUT_VARIABLE LLVM_INCLUDEDIR
                OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CMAKE_CXX_FLAGS "${LLVM_CXXFLAGS} -isystem ${LLVM_INCLUDEDIR} -Wall -Werror")
set(CMAKE_CXX_CLANG_TIDY clang-tidy${LLVM_VERSION_SUFFIX} -header-filter=.*)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(lib)

include(CTest)
enable_testing()
add_subdirectory(test)

file(GLOB_RECURSE SRC_FILEs "*.h" "*.cpp")
add_custom_target(format WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                         COMMAND clang-format${LLVM_VERSION_SUFFIX} -i ${SRC_FILEs})
