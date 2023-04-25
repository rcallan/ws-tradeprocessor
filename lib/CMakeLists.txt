cmake_minimum_required(VERSION 3.16)

set(SHARED_LIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SHARED_LIB_INCLUDE_DIR ${SHARED_LIB_INCLUDE_DIR} PARENT_SCOPE)

if(TARGET lib)
    
    message("lib is already defined")
    
else()
    
    include_directories(${Boost_INCLUDE_DIRS})
    include_directories(${SHARED_LIB_INCLUDE_DIR})

    include_directories("../curlpp/include/")
    include_directories("../thread-pool/include/")

    file(GLOB LIB_SRC_FILES "./src/*.cpp")

    set(LIB_SRCS ${LIB_SRC_FILES})
    
    add_library(lib STATIC ${LIB_SRCS})

    target_link_libraries(lib PRIVATE Boost::boost ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto)
    
endif()