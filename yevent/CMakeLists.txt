cmake_minimum_required(VERSION 2.8)
project(YEvent)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O0 -Wall")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

option(WITH_TESTS "enable the build of test" ON)
option(WITH_TESTS_HASH_RING "enable the build of hash ring test" ON)
option(WITH_TESTS_UEVENT "enable the build of uevent test" ON)
option(WITH_TESTS_GATE "enable the build of gate test" ON)

execute_process(COMMAND git describe --always WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_VARIABLE GITVER)

include_directories(${CMAKE_SOURCE_DIR}/uevent/base)

set(lib_common_files
    murmur_hash2.cc
    str_list.cc
    config_parser.cc
    umongo.cc
    flow_ctrl.cc
    md5.c
    crc32c.cc
    zk_name_container.cc
    gettime.cc)

add_library(common STATIC ${lib_common_files})
#target_link_libraries(common uevent_base zookeeper_mt message)
