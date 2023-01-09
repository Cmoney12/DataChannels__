cmake_minimum_required(VERSION 3.24)
project(DataChannels__)

set(CMAKE_CXX_STANDARD 17)

# openssl lib
find_package(OpenSSL REQUIRED)

include_directories(${OPENSSL_INCLUDE_DIR})

# libnice
find_package(PkgConfig REQUIRED)
pkg_search_module(LIBNICE REQUIRED nice)

# usrsctp
if (NOT TARGET SctpLab::UsrSCTP)
    set(USRSCTP_DEFINITIONS INET INET6)
    find_path(USRSCTP_INCLUDE_DIR usrsctp.h PATH_SUFFICES usrsctp)
    find_library(USRSCTP_LIBRARY NAMES usrsctp libusrsctp)

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(Usrsctp DEFAULT_MSG USRSCTP_LIBRARY USRSCTP_INCLUDE_DIR)

    mark_as_advanced(USRSCTP_INCLUDE_DIR USRSCTP_LIBRARY)

    set(USRSCTP_LIBRARIES ${USRSCTP_LIBRARY})
    set(USRSCTP_INCLUDE_DIRS ${USRSCTP_INCLUDE_DIR})

    if (USRSCTP_FOUND)
        message(usrsctp library found ${USRSCTP_LIBRARY})
        add_library(SctpLab::UsrSCTP UNKNOWN IMPORTED)
        set_target_properties(SctpLab::UsrSCTP PROPERTIES
                IMPORTED_LOCATION "${USRSCTP_LIBRARY}"
                INTERFACE_COMPILE_DEFINITIONS "${USRSCTP_DEFINITIONS}"
                INTERFACE_INCLUDE_DIRECTORIES "${USRSCTP_INCLUDE_DIRS}"
                IMPORTED_LINK_INTERFACE_LANGUAGES "C")
    endif ()
endif ()

add_executable(DataChannels__ main.cpp DataStructures/MemoryPool/MemoryPool.cpp DataStructures/MemoryPool/MemoryPool.h
        DataStructures/ThreadSafeMemoryPool/MemoryPool.h DataStructures/ThreadSafeMemoryPool/MemoryPool.cpp
        DataStructures/ThreadPool/ThreadPool.h DataStructures/ThreadPool/ThreadPool.cpp)

target_link_libraries(DataChannels__ ${OPENSSL_LIBRARIES} ${LIBNICE_LIBRARIES}  ${USRSCTP_LIBRARY})