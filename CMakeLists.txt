cmake_minimum_required(VERSION 3.11)

project(
  CDRCompatLoader
  VERSION 0.1
  DESCRIPTION "Plugin that loads CheatDevice Remastered ONLY when you start GTA LCS / VCS."
  HOMEPAGE_URL "http://cheatdeviceremastered.com"
  LANGUAGES C
)

set(PROJECT_AUTHOR "daniemun")

add_subdirectory(src)

include("$ENV{PSPDEV}/psp/share/pspdev.cmake")

add_prx_module(${PROJECT_NAME} ${MAIN_SOURCES})

# Get version info from cmake to project
configure_file(
  src/versioning.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/versioning.h
  @ONLY # Only replace vars with this "@syntax@"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/archive"
)

target_compile_options(${PROJECT_NAME} PRIVATE
  -Os
  -fno-pic
  -G0

  -Wall
  -Wextra
  -Wpedantic
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  ansic
  pspkubridge
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_options(${PROJECT_NAME} PRIVATE
  -specs=${PSPDEV}/psp/sdk/lib/prxspecs
  -L
  -Wl,-zmax-page-size=128
  -flto
)