PROJECT(nowind)
INCLUDE_DIRECTORIES(../general)
INCLUDE_DIRECTORIES(../nwhost)
INCLUDE_DIRECTORIES(../ftdx)
ADD_LIBRARY(nowind STATIC Attribute.cpp NwhostService.cpp nowind.cpp)

