cmake_minimum_required(VERSION 3.23)

# set the project name
project(MySocket)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(EXPORT_COMPILE_COMMANDS ON)

# add the executable
add_executable(MySocket sqlite3.c connection.cpp json.hpp account.cpp mysocket.cpp mysqlpool.cpp myssl.cpp logger.cpp thread.cpp service.cpp main.cpp)

set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-static -static-libstdc++ -static-libgcc")

if(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    set(OPENSSL_ROOT_DIR "${CMAKE_SOURCE_DIR}/requireDebug")
    find_package(OpenSSL REQUIRED)

    # set curl path
    set(CURL_LIBRARY "${CMAKE_SOURCE_DIR}/requireDebug/lib")
    set(CURL_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/requireDebug/include")
    add_library(curl STATIC IMPORTED)
    target_compile_definitions(curl INTERFACE CURL_STATICLIB)
    target_include_directories(curl INTERFACE ${CURL_INCLUDE_DIR})
    target_link_directories(curl INTERFACE ${CURL_LIBRARY})
    target_link_libraries(curl INTERFACE bcrypt crypt32 crypto ssl advapi32 wldap32 z gdi32)
    set_target_properties(curl PROPERTIES IMPORTED_LOCATION ${CURL_LIBRARY}/libcurl.a)
    
    target_compile_options(MySocket PRIVATE -fdiagnostics-color=always -pedantic)
    target_include_directories(MySocket PRIVATE ${CURL_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR})
    target_link_directories(MySocket PRIVATE ${CURL_LIBRARY})
    target_link_libraries(MySocket PRIVATE curl ${OPENSSL_LIBRARIES} gcc winmm ws2_32 pthread)
    
elseif(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
    set(OPENSSL_ROOT_DIR "${CMAKE_SOURCE_DIR}/requireRelease")
    find_package(OpenSSL REQUIRED)

    # set curl path
    set(CURL_LIBRARY "${CMAKE_SOURCE_DIR}/requireRelease/lib")
    set(CURL_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/requireRelease/include")

    set(OPENSSL_USE_STATIC_LIBS TRUE)
    set(CURL_USE_STATIC_LIBS TRUE)
    set(CMAKE_AR gcc-ar)

    target_compile_options(MySocket PRIVATE -fdiagnostics-color=always -pedantic -flto -s)
    target_link_options(MySocket PRIVATE -lcurl -lbcrypt -ladvapi32 -lcrypt32 -lssl -lcrypto -lgdi32 -lwldap32 -lz -lws2_32)
    target_include_directories(MySocket PRIVATE ${CURL_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR})
    target_link_directories(MySocket PRIVATE ${CURL_LIBRARY})
    target_link_libraries(MySocket PRIVATE curl Crypt32 ${OPENSSL_LIBRARIES} gcc winmm ws2_32 pthread)
endif()
