cmake_minimum_required(VERSION 3.10.0)
project(SocketClientServer VERSION 0.1.0 LANGUAGES CXX)

# set(CMAKE_CXX_STANDARD 17)
option(BUILD_TESTS "Build tests" ON)

# 添加子目录
add_subdirectory(Apps/SocketClient)
add_subdirectory(Apps/SocketServer)

if(BUILD_TESTS)
    add_subdirectory(Tests)
    # include(CTest)
    # enable_testing()
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
