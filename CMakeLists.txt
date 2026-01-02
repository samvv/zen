
cmake_minimum_required(VERSION 3.25)

project(Zen CXX)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
  set(is_debug_build ON)
else()
  set(is_debug_build OFF)
endif()

set(ZEN_NAMESPACE "" CACHE STRING "The namespace in which to embed Zen++")
set(ZEN_ENABLE_TESTS "${is_debug_build}" CACHE BOOL "Whether to generate the test infrastructure")
set(ZEN_ENABLE_EXAMPLES "${is_debug_build}" CACHE BOOL "Whether to generate build infrastructure for the examples")
set(ZEN_ENABLE_DEBUG_ASSERTIONS "${is_debug_build}" CACHE BOOL "Force the compiler to generate assertions for certain invariants")

if (NOT ZEN_NAMESPACE STREQUAL "")
  message("-- Placing Zen++ in custom namespace ${ZEN_NAMESPACE}")
endif()
if (ZEN_ENABLE_DEBUG_ASSERTIONS)
  message("-- Compiling Zen++ with debug assertions enabled")
endif()

#add_subdirectory(subprojects/hana EXCLUDE_FROM_ALL)

# This must be added before doing any other configuration so that
# changing something local doesn't trigger a GoogleTest rebuild
if (ZEN_ENABLE_TESTS)
  add_subdirectory(subprojects/googletest-1.14.0 EXCLUDE_FROM_ALL)
endif()

set(zen_sources
  src/json.cc
  src/fs_io.cc
  src/unicode.cc
  src/msgpack.cc
  src/po.cc
)

add_library(
  zen
  ${zen_sources}
)

target_compile_definitions(
  zen
  PUBLIC
  "ZEN_ENABLE_DEBUG_ASSERTIONS=$<BOOL:${ZEN_ENABLE_DEBUG_ASSERTIONS}>"
)

if (NOT ZEN_NAMESPACE STREQUAL "")
  string(REPLACE "::" ";" zen_namespace_chunks "${ZEN_NAMESPACE}")
  set(zen_namespace_start "")
  set(zen_namespace_end "")
  foreach(name IN LISTS zen_namespace_chunks)
    set(zen_namespace_start "${zen_namespace_start} namespace ${name} { ")
    set(zen_namespace_end "${zen_namespace_end} } ")
  endforeach()
  target_compile_definitions(
    zen
    PUBLIC
    ZEN_NAMESPACE=${ZEN_NAMESPACE}
    "ZEN_NAMESPACE_START=${zen_namespace_start}"
    "ZEN_NAMESPACE_END=${zen_namespace_end}"
  )
endif()

target_include_directories(
  zen
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCDIR}>
)

target_compile_features(
  zen
  PUBLIC
  cxx_std_20
)

install(
  DIRECTORY "include"
  DESTINATION "${CMAKE_INSTALL_INCDIR}"
  FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)
install(TARGETS zen EXPORT Zen)
install(EXPORT Zen NAMESPACE "${ZEN_NAMESPACE}::" DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Zen")

if (ZEN_ENABLE_EXAMPLES)
  file(GLOB example_sources examples/*.cc)
  add_custom_target(examples)
  foreach (filename IN LISTS example_sources)
    get_filename_component (name "${filename}" NAME_WE)
    add_executable("${name}" "${filename}")
    target_link_libraries("${name}" zen)
    add_dependencies(examples "${name}")
  endforeach()
endif()

if (ZEN_ENABLE_TESTS)
  add_executable(
    alltests
    test/bytestring.cc
    test/either.cc
    test/fs_io.cc
    test/graph.cc
    test/json.cc
    test/mapped_iterator.cc
    test/po.cc
    test/pool.cc
    test/iterator_range.cc
    test/unicode.cc
    test/zip_iterator.cc
  )
  target_link_libraries(alltests zen gtest gtest_main)
  add_custom_target(check COMMAND alltests --gtest_color=yes COMMENT "Running tests")
endif()

