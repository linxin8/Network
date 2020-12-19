find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(serialization serialization.cpp)
target_link_libraries(serialization ${GTEST_LIBRARIES} net rpc)