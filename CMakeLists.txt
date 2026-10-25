cmake_minimum_required(VERSION 3.23 FATAL_ERROR)

project(nowindlibraries LANGUAGES C CXX)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "installation path" FORCE)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Setting the rpath to '$ORIGIN' ensures our application looks for
# libraries in its own directory first.
set(CMAKE_INSTALL_RPATH "\$ORIGIN")

# fails the build when a source file has trailing whitespace or a C/C++ file does not end with a newline
find_package(Python3 COMPONENTS Interpreter)
if(Python3_FOUND)
  add_custom_target(whitespacecheck ALL
      COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/script/whitespace.py ${CMAKE_SOURCE_DIR}
      COMMENT "Checking the sources for trailing whitespace (fix: python script/whitespace.py --apply)"
      VERBATIM)
else()
  message(STATUS "Python 3 not found, the sources are not checked for trailing whitespace")
endif()

# lets ctest in the build directory find the unit tests
enable_testing()

add_subdirectory(sjasm)
add_subdirectory(msxsrc)

# finds leaks, buffer overruns, use after free and undefined behaviour at run time, also in the tests; after sjasm
# (third-party, a short-lived build tool that leaks) and msxsrc, so they are built without it
option(NOWIND_SANITIZE "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
if(NOWIND_SANITIZE)
  if(MSVC)
    # MSVC has no UndefinedBehaviorSanitizer
    add_compile_options(/fsanitize=address)
  else()
    # undefined behaviour stops the program, so a test with undefined behaviour fails
    add_compile_options(-fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
  endif()
endif()

# project::warnings_options, linked by this project's own targets; it includes -Wformat=2, which checks the
# arguments of the printf-style logging functions (NW_PRINTF_FORMAT in general/PrintfFormat.h)
include(cmake/CMakeLists.warnings.cmake)

# SDL 1.2, SDL_ttf and googletest: downloaded on Windows, the installed packages elsewhere (NOWIND_FETCH_DEPENDENCIES)
include(cmake/Dependencies.cmake)

add_subdirectory(general)
add_subdirectory(nwhost)
add_subdirectory(ftdx)
add_subdirectory(nowind)
add_subdirectory(nwhostapp)
add_subdirectory(emuv1)

if(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
   set(TARGETBITS 32)
else()
   set(TARGETBITS 64)
endif()

message("")
message("${PROJECT_NAME} configured to run on:  ${CMAKE_SYSTEM_NAME} ${TARGETBITS} bit, ${CMAKE_BUILD_TYPE} Mode")
message("${PROJECT_NAME} will be built in:      ${CMAKE_BINARY_DIR}")
message("${PROJECT_NAME} will be installed in:  ${CMAKE_INSTALL_PREFIX}")
message("
")
