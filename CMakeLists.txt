cmake_minimum_required(VERSION 3.23)

# set the project name
project(MySocket)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(EXPORT_COMPILE_COMMANDS ON)

# add the executable
add_executable(MySocket sqlite3.c Connection.cpp json.hpp account.cpp mysocket.cpp mysqlpool.cpp myssl.cpp logger.cpp thread.cpp service.cpp main.cpp)

set(OPENSSL_ROOT_DIR "${CMAKE_SOURCE_DIR}/require")
set(OPENSSL_USE_STATIC_LIBS TRUE)
set(CMAKE_AR gcc-ar)
find_package(OpenSSL REQUIRED)
target_compile_options(MySocket PRIVATE -fdiagnostics-color=always -pedantic -fexec-charset=GBK -O2)
target_link_options(MySocket PRIVATE -lcurl -lbcrypt -ladvapi32 -lcrypt32 -lssl -lcrypto -lgdi32 -lwldap32 -lz -lws2_32 -flto=auto -s)

# set curl path
set(CURL_LIBRARY "${CMAKE_SOURCE_DIR}/require/lib")
set(CURL_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/require/include")
set(CURL_ROOT_DIR "${CMAKE_SOURCE_DIR}/require")
set(CURL_USE_STATIC_LIBS TRUE)

set(CMAKE_AR gcc-ar)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-static -static-libstdc++ -static-libgcc")
target_include_directories(MySocket PRIVATE ${CURL_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR})
target_link_directories(MySocket PRIVATE ${CURL_LIBRARY})
target_link_libraries(MySocket PRIVATE curl Crypt32  ${OPENSSL_LIBRARIES} gcc winmm ws2_32 pthread)
