cmake_minimum_required(VERSION 2.8)
project(dflat)
set(USE_STATIC_LIBS "${USE_STATIC_LIBS}" CACHE BOOL "Link statically")

add_definitions("-Wall -pedantic -std=c++11")
add_definitions("-Wno-zero-length-array") # XXX Workaround because clasp generates these warnings
add_definitions("-Wno-pessimizing-move") # XXX Workaround because gringo generates these warnings

set(CMAKE_CXX_FLAGS_GPROF "-O3 -g -pg")
set(CMAKE_CXX_FLAGS_PROFILER "-O3 -g")
# If there is voodoo in liblasp's pod_vector.h, it might be that you compiled it with -D_DEBUG. Consider compiling D-FLAT with this flag, too (uncomment next line). Otherwise the size of our Clasp::PodVector is different than when we built clasp's debug release.
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

include_directories(${gringo_dir}/libgringo)
include_directories(SYSTEM ${clasp_dir}/libclasp ${htd_dir}/include) # declared as system header to suppress warnings due to clasp and htd

if(USE_STATIC_LIBS)
	set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)
	set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
	if(WIN32)
		set(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
	else()
		set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
	endif()
endif()

add_library(gringo STATIC IMPORTED)
set_target_properties(gringo PROPERTIES IMPORTED_LOCATION ${gringo_lib})
add_library(clasp STATIC IMPORTED)
set_target_properties(clasp PROPERTIES IMPORTED_LOCATION ${clasp_lib})
add_library(htd STATIC IMPORTED)
set_target_properties(htd PROPERTIES IMPORTED_LOCATION ${htd_lib})

find_library(gmpxx gmpxx)
find_library(gmp gmp)
find_package(FLEX)
find_package(BISON)
if(CMAKE_BUILD_TYPE STREQUAL "profiler")
	find_package(Gperftools)
endif()

FLEX_TARGET(Scanner ${CMAKE_CURRENT_SOURCE_DIR}/parser/scanner.ll  ${CMAKE_CURRENT_BINARY_DIR}/scanner.cpp)
BISON_TARGET(Parser ${CMAKE_CURRENT_SOURCE_DIR}/parser/parser.yy ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)
ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)
# XXX This is a workaround because flex causes these warnings with GCC
set_source_files_properties(${FLEX_Scanner_OUTPUTS} PROPERTIES COMPILE_FLAGS -Wno-sign-compare)

include(CheckIncludeFiles)
check_include_files(wordexp.h HAVE_WORDEXP_H)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

# To build tests, call cmake with -Dbuild_tests=BOOL:ON
option(build_tests "Build tests" OFF)
if(build_tests)
	enable_testing()
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../test ${CMAKE_CURRENT_BINARY_DIR}/dflat-tests)
	add_subdirectory(${gtest_dir} ${CMAKE_CURRENT_BINARY_DIR}/gtest)
endif()

# We put everything into a static lib (except main.cpp) because we can then
# re-use this for the tests.
add_library(dflat-objects
	${FLEX_Scanner_OUTPUTS}
	${BISON_Parser_OUTPUTS}
	parser/Driver.cpp
	options/Option.cpp
	options/ValueOption.cpp
	options/SingleValueOption.cpp
	options/MultiValueOption.cpp
	options/Choice.cpp
	options/OptionHandler.cpp
	options/HelpObserver.cpp
	options/VersionObserver.cpp
	DecompositionNode.cpp
	Decomposition.cpp
	Module.cpp
	Decomposer.cpp
	decomposer/Dummy.cpp
	decomposer/TreeDecomposer.cpp
	decomposer/GraphMl.cpp
	SolverFactory.cpp
	Solver.cpp
	LazySolver.cpp
	solver/asp/AtomIntroductionSolver.cpp
	solver/asp/AtomRemovalSolver.cpp
	solver/asp/LeafSolver.cpp
	solver/asp/RuleIntroductionSolver.cpp
	solver/asp/RuleRemovalSolver.cpp
	solver/asp/SolverBase.cpp
	solver/asp/SolverFactory.cpp
	solver/clasp/Solver.cpp
	solver/clasp/SolverFactory.cpp
	solver/clasp/tables/ClaspCallback.cpp
	solver/clasp/tables/GringoOutputProcessor.cpp
	solver/clasp/trees/ClaspCallback.cpp
	solver/clasp/trees/EncodingChecker.cpp
	solver/clasp/trees/GringoOutputProcessor.cpp
	solver/default_join/Solver.cpp
	solver/dummy/Solver.cpp
	solver/dummy/SolverFactory.cpp
	solver/lazy_clasp/GringoOutputProcessor.cpp
	solver/lazy_clasp/Solver.cpp
	solver/lazy_clasp/SolveIter.cpp
	solver/lazy_default_join/Solver.cpp
	Printer.cpp
	printer/Quiet.cpp
	printer/Progress.cpp
	printer/Debug.cpp
	printer/MachineReadable.cpp
	printer/CountRows.cpp
	asp_utils.cpp
	asp_utils/GringoOutputProcessor.cpp
	asp_utils/ClaspCallback.cpp
	String.cpp
	ItemTreeNode.cpp
	ItemTree.cpp
	UncompressedItemTree.cpp
	ExtensionIterator.cpp
	Application.cpp
	util.cpp
	tinyxml2.cpp
	)
add_executable(dflat main.cpp)

# GPERFTOOLS_LIBRARIES is only set when find_package(Gperftools) has been called, which is only called if CMAKE_BUILD_TYPE is "profiler".
target_link_libraries(dflat dflat-objects gringo clasp htd gmpxx gmp ${GPERFTOOLS_LIBRARIES})
