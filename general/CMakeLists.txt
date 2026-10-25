# general: platform helpers (Util, BasicType), used by all other libraries
add_library(nwgeneral STATIC BasicType.cpp Util.cpp general.cpp)
target_include_directories(nwgeneral PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# all libraries are static, so no dllimport/dllexport decoration (GeneralExports.h)
target_compile_definitions(nwgeneral PUBLIC NOWIND_STATIC_LIBS)
target_link_libraries(nwgeneral PRIVATE project::warnings_options)
