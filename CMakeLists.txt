cmake_minimum_required(VERSION 3.16)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")
set(VCPKG_TARGET_TRIPLET x64-linux CACHE STRING "Vcpkg target triplet")

project(rtdprocessor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

set(CMAKE_CXX_FLAGS "-DWITH_JSONCPP=ON")

add_subdirectory(curlpp)
add_subdirectory(thread-pool)

find_package(jsoncpp CONFIG REQUIRED)
find_package(unofficial-concurrentqueue CONFIG REQUIRED)
find_package(Boost COMPONENTS program_options REQUIRED)
# find_package(unofficial-curlpp CONFIG REQUIRED)
find_package(websocketpp CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

# set(ThreadPool_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build/thread-pool)
# find_package(dp::thread-pool REQUIRED)

add_subdirectory(lib)
include_directories(${SHARED_LIB_INCLUDE_DIR})

add_executable(rtdprocessor rtdprocessor.cpp)

target_link_libraries(rtdprocessor PRIVATE curlpp JsonCpp::JsonCpp dp::thread-pool unofficial::concurrentqueue::concurrentqueue websocketpp::websocketpp lib)

