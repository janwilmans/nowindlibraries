cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

set(BOOST_USE_STATIC_LIBS   ON)
set(BOOST_USE_MULTITHREADED ON)

set(BOOST_INCLUDEDIR /usr/include)
find_package( Boost REQUIRED)

link_directories ( ${BOOST_LIBRARY_DIRS} )
include_directories ( ${BOOST_INCLUDE_DIRS} )

PROJECT(nowindlibraries)

add_subdirectory(general)
add_subdirectory(nwhost)
add_subdirectory(ftdx)
add_subdirectory(nowind)
add_subdirectory(nwhostapp)

