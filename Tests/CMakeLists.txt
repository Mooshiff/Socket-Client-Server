# include(GoogleTest)
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip    
)
FetchContent_MakeAvailable(googletest)

add_executable(TestUtils TestUtils.cpp)
target_link_libraries(TestUtils PRIVATE GTest::gtest_main)
add_test(NAME TestUtils COMMAND TestUtils)
