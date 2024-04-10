cmake_minimum_required(VERSION 3.1)
project(hr4c-main)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS "on") # compile_commands.json作成

# force colored output
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  add_compile_options(-fdiagnostics-color=always)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_compile_options(-fcolor-diagnostics)
endif()

include(cmake/config.cmake)

add_subdirectory(hr4c)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/
  ${HR4C_INCLUDES}
)

link_directories(
  /opt/homebrew/lib/pkgconfig
  /opt/homebrew/lib
  /usr/local/lib
)

file(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/app/*.cpp)
foreach(path IN LISTS sources)
  get_filename_component(target ${path} NAME_WE)
  message("new target : ${target} ${path}")
  add_executable(${target} ${path})
  target_link_libraries(${target} ${ALL_LINKS} hr4c-core)
endforeach()

