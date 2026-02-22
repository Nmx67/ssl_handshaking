cmake_minimum_required(VERSION 3.25)


project(
    ssl-server
    LANGUAGES C
)
# set(CMAKE_VERBOSE_MAKEFILE True)

message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()


if(MSVC)
    set(CMAKE_PREFIX_PATH "c:/Libs/OpenSSL/x64-Release/lib/cmake")
    #set(OPENSSL_ROOT_DIR c:/Libs/OpenSSL/x64-Release)
endif()

find_package(OpenSSL 4.0 REQUIRED)

message("OPENSSL_VERSION = ${OPENSSL_VERSION}")
message("OPENSSL_APPLINK_SOURCE = ${OPENSSL_APPLINK_SOURCE}")
message("OPENSSL_RUNTIME_DIR = ${OPENSSL_RUNTIME_DIR}")
message("OPENSSL_PROGRAM = ${OPENSSL_PROGRAM}")
message("OPENSSL_LIBCRYPTO_SHARED = ${OPENSSL_LIBCRYPTO_SHARED}")
message("OPENSSL_LIBSSL_SHARED = ${OPENSSL_LIBSSL_SHARED}")


include(CMakePrintHelpers)
cmake_print_properties(
  TARGETS     OpenSSL::Crypto
              OpenSSL::SSL
  PROPERTIES  IMPORTED_IMPLIB
              IMPORTED_LOCATION
              # IMPORTED_LOCATION_DEBUG
              # IMPORTED_LOCATION_RELEASE
)


set(CMAKE_C_STANDARD 17)
set(CMAKE_C_FLAGS_RELEASE "-O3")

add_executable(server server.c win32_util.c ssl_util.c ${OPENSSL_APPLINK_SOURCE})
target_link_libraries(server OpenSSL::Crypto OpenSSL::SSL)

if(MSVC)

    add_custom_target(echo-TARGET_RUNTIME_DLLS
    COMMAND ${CMAKE_COMMAND} -E echo
          "$<TARGET_RUNTIME_DLLS:server>")

    set(my_target server)
    target_link_libraries(${my_target} ws2_32.lib) 
    add_custom_command(TARGET ${my_target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:${my_target}> $<TARGET_RUNTIME_DLLS:${my_target}>
        # COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_RUNTIME_DLLS:${my_target}> $<TARGET_FILE_DIR:${my_target}>
        COMMAND_EXPAND_LISTS
    )
endif()



add_executable(client client.c win32_util.c ssl_util.c ${OPENSSL_APPLINK_SOURCE})
target_link_libraries(client OpenSSL::Crypto OpenSSL::SSL)
if(MSVC)
    target_link_libraries(client ws2_32.lib) 
endif()