# nowind: ties nwhost and ftdx together, the library nwhostapp is built on
add_library(nowind STATIC Attribute.cpp NwhostService.cpp nowind.cpp)
target_include_directories(nowind PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(nowind PUBLIC NOWIND_STATIC_LIBS)
# public: the headers include libnwhost.h and libftdx.h
target_link_libraries(nowind PUBLIC nwhost ftdx PRIVATE project::warnings_options)
