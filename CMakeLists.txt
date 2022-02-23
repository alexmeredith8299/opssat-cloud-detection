cmake_minimum_required(VERSION 3.10)

# set the project name
project(opssat-segment)

# tensorflow

set(TENSORFLOW_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/tensorflow/tensorflow" CACHE PATH
  "Directory that contains the TensorFlow project" )
if(NOT TENSORFLOW_SOURCE_DIR)
  get_filename_component(TENSORFLOW_SOURCE_DIR
    "${CMAKE_CURRENT_LIST_DIR}/../../../../" ABSOLUTE)
endif()

add_subdirectory(
  "${TENSORFLOW_SOURCE_DIR}/tensorflow/lite"
  "${CMAKE_CURRENT_BINARY_DIR}/tensorflow-lite" EXCLUDE_FROM_ALL)

# add includes
include_directories(include)

add_compile_options(-Wall -static -Os)

# add the executable
add_executable(opssat-segment.out src/main.cpp)

target_link_libraries(opssat-segment.out
  tensorflow-lite
)