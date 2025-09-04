cmake_minimum_required(VERSION 3.10)
project(ZnetServer VERSION 1.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置编译选项
set(CMAKE_VERBOSE_MAKEFILE ON)
option(BUILD_TESTS "Build test cases" ON)

# 设置通用编译标志
add_compile_options(-Wall -Wextra)

# 分别设置Debug和Release编译选项
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

# 收集源文件
file(GLOB_RECURSE SERVER_SRC 
    "server/*.cpp"
    "server/*.h"
)

# 添加库目标
add_library(${PROJECT_NAME} SHARED ${SERVER_SRC})

# 设置包含目录
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/server>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

find_package(yaml-cpp REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE yaml-cpp)

# 构建测试
if(BUILD_TESTS)
    enable_testing()
    # add_executable(test_logger tests/test.cpp)
    # target_link_libraries(test_logger PRIVATE ${PROJECT_NAME})
    # add_executable(test_config tests/test_config.cpp)
    # target_link_libraries(test_config PRIVATE ${PROJECT_NAME} yaml-cpp)
    # add_executable(test_thread tests/test_thread.cpp)
    # target_link_libraries(test_thread PRIVATE ${PROJECT_NAME} yaml-cpp)
    add_executable(test_fiber tests/test_fiber.cpp)
    target_link_libraries(test_fiber PRIVATE ${PROJECT_NAME})
    add_executable(test_scheduler tests/test_scheduler.cpp)
    target_link_libraries(test_scheduler PRIVATE ${PROJECT_NAME})
endif()
