#cmake_minimum_required(VERSION 3.16)

#set(CMAKE_SYSTEM_NAME Linux)

#set(CMAKE_SYSTEM_PROCESSOR armv7)

#SET(CMAKE_C_COMPILER "/usr/bin/arm-linux-gnueabihf-gcc-7")

#SET(CMAKE_CXX_COMPILER "/usr/bin/arm-linux-gnueabihf-g++-7")

message(STATUS "using ${CMAKE_CXX_COMPILER}")

set(CMAKE_VERBOSE_MAKEFILE ON)


#set(CMAKE_SYSTEM_NAME Linux)

#set(CMAKE_SYSTEM_PROCESSOR arm)


#SET(CMAKE_C_COMPILER "/usr/bin/arm-linux-gnueabihf-gcc")

#SET(CMAKE_CXX_COMPILER "/usr/bin/arm-linux-gnueabihf-g++")


message(STATUS "using ${CMAKE_CXX_COMPILER}")


set(CMAKE_VERBOSE_MAKEFILE ON)


# set the project name
project(opssat-segment)

#set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Os -std=c++11")

# tensorflow
set(TENSORFLOW_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/tensorflow" CACHE PATH
  "Directory that contains the TensorFlow project" )
#set(TENSORFLOW_SOURCE_DIR "/mnt/c/Users/shree/Documents/OPS-SAT/opssat-cloud-detection/tensorflow" CACHE PATH
  #"Directory that contains the TensorFlow project" )
#if(NOT TENSORFLOW_SOURCE_DIR)
  #get_filename_component(TENSORFLOW_SOURCE_DIR
    #"${CMAKE_CURRENT_LIST_DIR}/../../../../" ABSOLUTE)
#endif()

add_subdirectory(
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite"
  "${CMAKE_CURRENT_BINARY_DIR}/tensorflow-lite" EXCLUDE_FROM_ALL)

#ranger
set(RANGER_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/include/ranger")

add_subdirectory("${RANGER_SOURCE_DIR}/cpp_version")

# add includes
include_directories(
  include
  include/csv2/include)


#add_compile_options(-Wall -static -O0 -march=armv7-a -mfpu=neon-vfpv4 -funsafe-math-optimizations)

# add the executable
add_executable(opssat-segment.out src/main.cpp)

#add ranger executable
#add_executable(opssat-ranger ${RANGER_SOURCE_DIR}/cpp_version/src/main.cpp)

#add random forest executable
add_executable(opssat-rf-segment.out src/main_rf.cpp)

target_link_libraries(opssat-segment.out
  tensorflow-lite
)
