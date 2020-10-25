cmake_minimum_required(VERSION 2.6)

project(Network)
 
# add_subdirectory(src) 
# add_subdirectory(test) 


# set(CMAKE_CXX_COMPILER "/usr/bin/g++-10")
# set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 20)
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS}) 
set(CXX_FLAGS
#  -g
 # -DVALGRIND
 -DCHECK_PTHREAD_RETURN_VALUE
 -D_FILE_OFFSET_BITS=64
 -Wall
 -Wextra
 -Werror
 -Wconversion
 -Wno-unused-parameter
 -Wold-style-cast
 -Woverloaded-virtual
 -Wpointer-arith
 -Wshadow
 -Wwrite-strings
 -march=native
 # -MMD
 -std=c++20
 -rdynamic
 )


add_subdirectory(src) 
add_subdirectory(test) 
add_subdirectory(example) 
