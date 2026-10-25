PROJECT(nowind)
INCLUDE_DIRECTORIES(../general)
INCLUDE_DIRECTORIES(../nwhost)
INCLUDE_DIRECTORIES(../ftdx)
ADD_LIBRARY(nowind STATIC Attribute.cpp NwhostService.cpp nowind.cpp)
target_link_libraries(nowind PRIVATE project::warnings_options)

