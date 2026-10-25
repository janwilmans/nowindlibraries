PROJECT(nwgeneral)

ADD_LIBRARY(nwgeneral STATIC BasicType.cpp Util.cpp general.cpp)
target_link_libraries(nwgeneral PRIVATE project::warnings_options)

