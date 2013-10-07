cmake_minimum_required(VERSION 2.8)
project(dflat)

add_definitions("-Wall -pedantic -std=c++11 -DDISABLE_MULTI_THREADING")

# XXX This is a workaround because SHARP generates these warnings
#add_definitions("-Wno-write-strings")

# XXX This is a workaround because flex generates these warnings
add_definitions("-Wno-sign-compare")

set(CMAKE_CXX_FLAGS_GPROF "-O3 -g -pg")
set(CMAKE_LINKER_FLAGS_GPROF "-pg")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -DDEBUG") # Without -D_DEBUG there will be voodoo in libclasp's pod_vector.h, since then the size of our Clasp::PodVector is different than when we built clasp's debug release

# Unfortunately libclasp does not compile with -pedantic.
# Using -isystem instead of -I; then they are treated as system headers,
# i.e., warnings are suppressed for the clasp headers.
include_directories(SYSTEM ${clasp_dir}/libclasp)
include_directories(${sharp_dir}/include ${gringo_dir}/libgringo)

add_library(gringo STATIC IMPORTED)
set_target_properties(gringo PROPERTIES IMPORTED_LOCATION ${gringo_lib})
add_library(clasp STATIC IMPORTED)
set_target_properties(clasp PROPERTIES IMPORTED_LOCATION ${clasp_lib})
add_library(sharp STATIC IMPORTED)
set_target_properties(sharp PROPERTIES IMPORTED_LOCATION ${sharp_lib})

find_library(gmpxx gmpxx)
find_library(gmp gmp)
find_package(Boost COMPONENTS system filesystem REQUIRED) # XXX possibly no longer required with newer Gringo
find_package(FLEX)
find_package(BISON)

FLEX_TARGET(Scanner ${CMAKE_CURRENT_SOURCE_DIR}/parser/scanner.ll  ${CMAKE_CURRENT_BINARY_DIR}/scanner.cpp)
BISON_TARGET(Parser ${CMAKE_CURRENT_SOURCE_DIR}/parser/parser.yy ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)
ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(dflat
	${FLEX_Scanner_OUTPUTS}
	${BISON_Parser_OUTPUTS}
	parser/Driver.cpp
	parser/Terms.cpp
	options/Option.cpp
	options/ValueOption.cpp
	options/SingleValueOption.cpp
	options/MultiValueOption.cpp
	options/Choice.cpp
	options/OptionHandler.cpp
	options/HelpObserver.cpp
	DecompositionNode.cpp
	Decomposition.cpp
	Module.cpp
	Decomposer.cpp
	decomposer/Dummy.cpp
	decomposer/TreeDecomposer.cpp
	SolverFactory.cpp
	Solver.cpp
	solver/DummyFactory.cpp
	solver/Dummy.cpp
	solver/asp/ClaspCallback.cpp
	solver/asp/ClaspInputReader.cpp
	solver/asp/GringoOutputProcessor.cpp
	solver/AspFactory.cpp
	solver/Asp.cpp
	solver/asp/tables/ClaspCallback.cpp
	solver/asp/tables/GringoOutputProcessor.cpp
	solver/asp/trees/ClaspCallback.cpp
	solver/asp/trees/GringoOutputProcessor.cpp
	solver/asp/trees/UncompressedItemTree.cpp
	ItemTreeNode.cpp
	ItemTree.cpp
	ExtensionIterator.cpp
	Application.cpp
	main.cpp)

target_link_libraries(dflat sharp gringo clasp gmp gmpxx ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
