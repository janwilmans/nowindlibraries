cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

# important: declare the project before anything else
PROJECT(nowindlibraries)

#set(BOOST_USE_STATIC_LIBS   ON)
#set(BOOST_USE_MULTITHREADED ON)

find_package(Boost COMPONENTS system filesystem REQUIRED)

include_directories ( ${Boost_INCLUDE_DIRS} )
link_directories ( ${Boost_LIBRARY_DIRS} )

MESSAGE("find_package Boost: ${Boost_INCLUDE_DIRS} <end> ")
MESSAGE("find_package Boost: ${Boost_LIBRARY_DIRS} <end> ")

add_subdirectory(general)
add_subdirectory(nwhost)
add_subdirectory(ftdx)
add_subdirectory(nowind)
add_subdirectory(nwhostapp)

