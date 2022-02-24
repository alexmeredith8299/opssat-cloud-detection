cmake_minimum_required(VERSION 2.0)

# set the project name
project(opssat-segment)

#set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Os -std=c++11")

# tensorflow

set(TENSORFLOW_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/tensorflow" CACHE PATH
  "Directory that contains the TensorFlow project" )
if(NOT TENSORFLOW_SOURCE_DIR)
  get_filename_component(TENSORFLOW_SOURCE_DIR
    "${CMAKE_CURRENT_LIST_DIR}/../../../../" ABSOLUTE)
endif()

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



# add the executable
add_executable(opssat-segment.out src/main.cpp)

#add ranger executable
#add_executable(opssat-ranger ${RANGER_SOURCE_DIR}/cpp_version/src/main.cpp)

#add random forest executable
add_executable(opssat-rf-segment.out src/main_rf.cpp)

target_link_libraries(opssat-segment.out
  tensorflow-lite
)
