# the warnings for this project's own code: link project::warnings_options to a target to turn them on
# (third-party code, such as sjasm and the glibc getopt copy in nwhostapp, stays without them)
add_library(define_project_warning_options INTERFACE)
add_library(project::warnings_options ALIAS define_project_warning_options)
add_library(define_project_warning_suppressions INTERFACE)
add_library(project::warning_suppressions ALIAS define_project_warning_suppressions)

if (MSVC)
  # replace /Zi and /ZI with /Z7 for tooling compatibility (Incredibuild and stashed.io)
  string(REGEX REPLACE "/Z[iI7]" "/Z7" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  string(REGEX REPLACE "/Z[iI7]" "/Z7" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

  string (REGEX REPLACE "/W3" "" CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}")
  string (REGEX REPLACE "/W3" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  string (REGEX REPLACE "/INCREMENTAL[:NO]*" "/INCREMENTAL:NO" CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}")

  set(PROJECT_WARNING_DEFAULTS
    /W4            # Baseline reasonable warnings
    /w14242        # 'identifier': conversion from 'type1' to 'type2', possible loss of data
    /w14254        # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible loss of data
    /w14263        # 'function': member function does not override any base class virtual member function
    /w14265        # 'classname': class has virtual functions, but destructor is not virtual instances of this class may not
                   # be destructed correctly
    /w14287        # 'operator': unsigned/negative constant mismatch
    /we4289        # nonstandard extension used: 'variable': loop control variable declared in the for-loop is used outside
                  # the for-loop scope
    /w14296        # 'operator': expression is always 'boolean_value'
    /w14311        # 'variable': pointer truncation from 'type1' to 'type2'
    /w14545        # expression before comma evaluates to a function which is missing an argument list
    /w14546        # function call before comma missing argument list
    /w14547        # 'operator': operator before comma has no effect; expected operator with side-effect
    /w14549        # 'operator': operator before comma has no effect; did you intend 'operator'?
    /w14555        # expression has no effect; expected expression with side- effect
    /w14619        # pragma warning: there is no warning number 'number'
    /w14640        # Enable warning on thread un-safe static member initialization
    /w14826        # Conversion from 'type1' to 'type_2' is sign-extended. This may cause unexpected runtime behavior.
    /w14905        # wide string literal cast to 'LPSTR'
    /w14906        # string literal cast to 'LPWSTR'
    /w14928        # illegal copy-initialization; more than one user-defined conversion has been implicitly applied
   )
   target_compile_options(define_project_warning_options INTERFACE "$<$<COMPILE_LANGUAGE:CXX>:${PROJECT_WARNING_DEFAULTS}>")

# investigate these flags:
# "CMAKE_CXX_FLAGS": "/sdl /guard:cf /utf-8 /diagnostics:caret /w14165 /w44242 /w44254 /w44263 /w34265 /w34287 /w44296 /w44365 /w44388 /w44464
#   /w14545 /w14546 /w14547 /w14549 /w14555 /w34619 /w34640 /w24826 /w14905 /w14906 /w14928 /w45038 /W4 /permissive- /volatile:iso /Zc:inline /Zc:preprocessor
#   /Zc:enumTypes /Zc:lambda /Zc:__cplusplus /Zc:externConstexpr /Zc:throwingNew /EHsc",
# "CMAKE_EXE_LINKER_FLAGS": "/machine:x64 /guard:cf",
# "CMAKE_SHARED_LINKER_FLAGS": "/machine:x64 /guard:cf"


  target_compile_options(define_project_warning_suppressions INTERFACE
    /wd4242
    /wd4244
   )
else()
  # Linux, Windows (MinGW), macOS
  if(NOT APPLE)
    # report unresolved symbols when linking a shared library too (the macOS linker has no --no-undefined, it
    # already does this by default)
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    -Wl,--no-undefined")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--no-undefined")
  endif()

  target_compile_options(define_project_warning_options INTERFACE
    -Wall
    -Wextra
    -Wpedantic
    -Wcast-align
    -Wshadow
    # -Wsign-conversion and -Wconversion are off: the legacy code (emuv1 from 2004, nwhost from 2009) mixes int,
    # unsigned, byte and word everywhere, which gives about a thousand warnings that hide the useful ones
    # -Wsign-conversion
    # -Wconversion
    -Wdouble-promotion
    -Wold-style-cast
    -Wformat=2
    -Wmisleading-indentation
    -Wmissing-include-dirs
    -Wnull-dereference
    # -Wswitch-enum is off: it wants a case for every enumerator even when there is a default, emuv1's key handling
    # switches over SDLKey (hundreds of keys) gave about 1300 warnings; -Wswitch (in -Wall) still reports a missing
    # case in a switch without a default
    # -Wswitch-enum
    -Wunused
    -Wunused-variable
    -Wunused-parameter
    -Wvla
    # test -Wreturn-type
    # Warnings that are not allowed in C compilation units
    $<$<COMPILE_LANGUAGE:CXX>:-Wnon-virtual-dtor>
    $<$<COMPILE_LANGUAGE:CXX>:-Woverloaded-virtual>
  )
endif()
