cmake_minimum_required(VERSION 3.23 FATAL_ERROR)

PROJECT(nowindlibraries)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "installation path" FORCE)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# all libraries are built as static libraries, so no dllimport/dllexport decoration
add_compile_definitions(NOWIND_STATIC_LIBS)
if(WIN32)
  # link the FTD2XX library statically
  add_compile_definitions(FTD2XX_STATIC)
endif()

# Setting the rpath to '$ORIGIN' ensures our application looks for
# libraries in its own directory first.
set(CMAKE_INSTALL_RPATH "\$ORIGIN")

# lets ctest in the build directory find the unit tests
enable_testing()

add_subdirectory(sjasm)
add_subdirectory(msxsrc)

# finds leaks, buffer overruns, use after free and undefined behaviour at run time, also in the tests; after sjasm
# (third-party, a short-lived build tool with harmless leaks) and msxsrc, so they are built without it
option(NOWIND_SANITIZE "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
if(NOWIND_SANITIZE)
  if(MSVC)
    # MSVC has no UndefinedBehaviorSanitizer
    add_compile_options(/fsanitize=address)
  else()
    # undefined behaviour stops the program, so a test with undefined behaviour fails
    add_compile_options(-fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
    # the leaks that are known and not fixed yet
    set(NOWIND_TEST_ENVIRONMENT "LSAN_OPTIONS=suppressions=${CMAKE_SOURCE_DIR}/cmake/lsan-suppressions.txt")
  endif()
endif()

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
