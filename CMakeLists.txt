cmake_minimum_required(VERSION 3.23)

# set the project name
project(MySocket)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(EXPORT_COMPILE_COMMANDS ON)

# add the executable
add_executable(MySocket account.cpp mysocket.cpp connection.cpp mysqlpool.cpp myssl.cpp logger.cpp thread.cpp service.cpp main.cpp)

if(MINGW)
	find_path(MYSQL_INCLUDE_DIR
		NAMES "mysql.h"
		PATHS "$ENV{PROGRAMFILES}/MySQL/*/include"
              "$ENV{PROGRAMFILES\(x86\)}/MySQL/*/include"
			  "$ENV{SYSTEMDRIVE}/MySQL/*/include")
	set(CMAKE_FIND_LIBRARY_SUFFIXES .dll)
	find_library(MYSQL_LIBRARY
		NAMES "mysql" "mysqlclient" "mysqlclient_r"
		PATHS "$ENV{PROGRAMFILES}/MySQL/*/lib"
              "$ENV{PROGRAMFILES\(x86\)}/MySQL/*/lib"
			  "$ENV{SYSTEMDRIVE}/MySQL/*/lib")
	unset(CMAKE_FIND_LIBRARY_SUFFIXES)
else()
	find_path(MYSQL_INCLUDE_DIR
		NAMES "mysql.h"
		PATHS "/usr/include/mysql"
			  "/usr/local/include/mysql"
			  "/usr/mysql/include/mysql" )
	
	find_library(MYSQL_LIBRARY
		NAMES "mysql" "mysqlclient" "mysqlclient_r"
		PATHS "/lib/mysql"
			  "/lib64/mysql"
			  "/usr/lib/mysql"
			  "/usr/lib64/mysql"
			  "/usr/local/lib/mysql"
			  "/usr/local/lib64/mysql"
			  "/usr/mysql/lib/mysql"
			  "/usr/mysql/lib64/mysql" )
endif()

if( MYSQL_INCLUDE_DIR AND EXISTS "${MYSQL_INCLUDE_DIRS}/mysql_version.h" )
	file( STRINGS "${MYSQL_INCLUDE_DIRS}/mysql_version.h"
		MYSQL_VERSION_H REGEX "^#define[ \t]+MYSQL_SERVER_VERSION[ \t]+\"[^\"]+\".*$" )
	string( REGEX REPLACE
		"^.*MYSQL_SERVER_VERSION[ \t]+\"([^\"]+)\".*$" "\\1" MYSQL_VERSION_STRING
		"${MYSQL_VERSION_H}" )
endif()

# handle the QUIETLY and REQUIRED arguments and set MYSQL_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( MYSQL DEFAULT_MSG
	MYSQL_LIBRARY MYSQL_INCLUDE_DIR)

set(MYSQL_INCLUDE_DIRS ${MYSQL_INCLUDE_DIR})
set(MYSQL_LIBRARIES ${MYSQL_LIBRARY})

set(OPENSSL_ROOT_DIR "${CMAKE_SOURCE_DIR}/openssl")
set(OPENSSL_USE_STATIC_LIBS TRUE)
set(CMAKE_AR gcc-ar)
find_package(OpenSSL REQUIRED)
target_compile_options(MySocket PRIVATE -fdiagnostics-color=always -lcrypto -pedantic -lssl -ldl)
# target_link_options(MySocket PRIVATE -flto -s)

if(OpenSSL_FOUND)
target_include_directories(MySocket PRIVATE ${OPENSSL_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR})
target_link_libraries(MySocket PRIVATE ${OPENSSL_LIBRARIES} ${MYSQL_LIBRARIES} winmm ws2_32 Crypt32 pthread)
else(OpenSSL_FOUND)
    message(FATAL_ERROR "OpenSSL not found")
endif(OpenSSL_FOUND)