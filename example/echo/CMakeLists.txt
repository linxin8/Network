add_executable(echo echo.cpp)
target_link_libraries(echo ${GTEST_LIBRARIES} net)