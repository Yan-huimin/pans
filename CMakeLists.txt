cmake_minimum_required(VERSION 3.25)

# 设置为缓存值
set(CMAKE_C_COMPILER "/usr/bin/gcc" CACHE FILEPATH "C compile" FORCE)
set(CMAKE_CXX_COMPILER "/usr/bin/g++" CACHE FILEPATH "C++ compile" FORCE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

project(pans VERSION 0.1.0 LANGUAGES C CXX)

# 生成 compile_commands.json 供 clangd 等工具使用
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs) # 引入安装目录变量
include(CMakePackageConfigHelpers)  # 创建配置文件工具集

option(PANS_BUILD_EXAMPLES "Build example" ON)
option(PANS_BUILD_TESTS    "Build tests"    ON)
option(PANS_BUILD_TOOLS    "Build tools"   ON)


# 编译类型
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "choos the type of build." FORCE)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MiniSizeRel)
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

# 性能优化
cmake_host_system_information(RESULT CPU_NUMS QUERY NUMBER_OF_LOGICAL_CORES)
message(STATUS "CPU NUMS on current server is: ${CPU_NUMS}")
math(EXPR JOBS "${CPU_NUMS} + ${CPU_NUMS} / 2")
set(CMAKE_BUILD_PARALLEL_LEVEL ${JOBS} CACHE STRING "并行编译线程数")
message(STATUS "Compiler default work jobs: ${CMAKE_BUILD_PARALLEL_LEVEL}")

include(cmake/pansOptions.cmake)

add_subdirectory(pans)

if(PANS_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
if(PANS_BUILD_TOOLS)
    add_subdirectory(tools)
endif()