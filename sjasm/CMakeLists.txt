# Sjasm 0.42c, the Z80 assembler the nowind ROM (msxsrc) is written for, built as a host tool
add_executable(sjasm
    datadir.cpp datastructures.cpp directives.cpp errors.cpp expressions.cpp fileio.cpp output.cpp pimsx.cpp
    preprocessor.cpp rawsource.cpp reader.cpp sjasm.cpp source.cpp)

# the 2011 sources use 'using namespace std' and their own 'byte', which clashes with C++17's std::byte
set_target_properties(sjasm PROPERTIES CXX_STANDARD 14)
