find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS}) 
 
add_executable(log log.cpp)

target_link_libraries(log ${GTEST_LIBRARIES} net pthread )


add_executable(file file.cpp)

target_link_libraries(file ${GTEST_LIBRARIES} net pthread )

