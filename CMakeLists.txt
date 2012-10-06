cmake_minimum_required(VERSION 2.8)
project(sat)

add_definitions("-Wall -pedantic -ansi -DDISABLE_MULTI_THREADING")
set(CMAKE_CXX_FLAGS_GPROF "-O3 -g -pg")
set(CMAKE_LINKER_FLAGS_GPROF "-pg")

# Unfortunately libclasp does not compile with -pedantic.
# Using -isystem instead of -I; then they are treated as system headers,
# i.e., warnings are suppressed for the clasp headers.
include_directories(SYSTEM ${clasp_dir}/libclasp)
include_directories(${sharp_dir}/include ${gringo_dir}/libgringo)
link_directories(${sharp_dir}/src/.libs)

add_library(gringo STATIC IMPORTED)
#set_target_properties(gringo PROPERTIES IMPORTED_LOCATION ${gringo_dir}/build/release/lib/libgringo.a IMPORTED_LOCATION_GPROF ${gringo_dir}/build/gprof/lib/libgringo.a)
set_target_properties(gringo PROPERTIES IMPORTED_LOCATION ${gringo_lib})
add_library(clasp STATIC IMPORTED)
#set_target_properties(clasp PROPERTIES IMPORTED_LOCATION ${clasp_dir}/build/release/libclasp/lib/libclasp.a IMPORTED_LOCATION_GPROF ${clasp_dir}/build/gprof/libclasp/lib/libclasp.a)
set_target_properties(clasp PROPERTIES IMPORTED_LOCATION ${clasp_lib})

find_library(gmpxx gmpxx)
find_package(Boost COMPONENTS system filesystem REQUIRED)

add_executable(sat main.cpp Problem.cpp Algorithm.cpp ClaspInputReader.cpp GringoOutputProcessor.cpp ModelProcessor.cpp Tuple.cpp)
target_link_libraries(sat libsharp.a gringo clasp gmpxx ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
