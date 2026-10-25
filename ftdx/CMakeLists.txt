# ftdx: the USB connection to the interface, FTD2XX on Windows, libftdi or the kernel's ftdi_sio elsewhere
add_library(ftdx STATIC ConFTD2XX.cpp UsbStream.cpp ftdx.cpp)

# the FTDI library, as an imported target
add_library(ftdi UNKNOWN IMPORTED)
if(WIN32)
  # static x64 FTD2XX library, the header is included relative to this directory (FTD2XX-msc/FTD2XX.H)
  set(FTDI_LIB ${CMAKE_CURRENT_SOURCE_DIR}/FTD2XX-msc/FTD2XX.lib)
  set_target_properties(ftdi PROPERTIES IMPORTED_LOCATION ${FTDI_LIB} INTERFACE_COMPILE_DEFINITIONS FTD2XX_STATIC)
else()
  # cached, nwhostapp installs it
  find_library(FTDI_LIB NAMES ftdi1 REQUIRED)
  find_path(FTDI_INCLUDE_DIR NAMES ftdi.h PATH_SUFFIXES libftdi1 REQUIRED)
  set_target_properties(ftdi PROPERTIES IMPORTED_LOCATION ${FTDI_LIB} INTERFACE_INCLUDE_DIRECTORIES ${FTDI_INCLUDE_DIR})
  # the linux kernel (ftdi_sio) and libftdi backends are not available on Windows
  target_sources(ftdx PRIVATE ConFtdiSio.cpp ConLibFtdi.cpp)
endif()
message(STATUS "FTDI library: ${FTDI_LIB}")

target_include_directories(ftdx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(ftdx PUBLIC NOWIND_STATIC_LIBS)
# public ftdi: the headers include FTD2XX.H (Windows), and the programs link the library
target_link_libraries(ftdx PUBLIC ftdi PRIVATE nwgeneral project::warnings_options)
