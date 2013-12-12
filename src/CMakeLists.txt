cmake_minimum_required(VERSION 2.8)
project(dflat)
set(USE_STATIC_LIBS "${USE_STATIC_LIBS}" CACHE BOOL "Link statically")

# Set BOOST_ROOT.
set(ENV{BOOST_ROOT} /files/abseher/boost_1_55_0/)

# Set BISON_PKGDATADIR.
set(ENV{BISON_PKGDATADIR} /files/abseher/bison-3.0.1/data/)

# Add gcc to PATH.
set(ENV{PATH} /files/abseher/gcc-4.8.2/bin/:$ENV{PATH})

# Add bison to PATH.
set(ENV{PATH} /files/abseher/bison-3.0.1/bin/:$ENV{PATH})

# Add boost to PATH.
set(ENV{PATH} /files/abseher/boost_1_55_0/:$ENV{PATH})

# Add cmake to PATH.
set(ENV{PATH} /files/abseher/cmake-2.8.12.1-Linux-i386/bin/:$ENV{PATH})

# Enable linker to access the gcc-Libraries.
set(ENV{LD_LIBRARY_PATH} /files/abseher/gcc-4.8.2/lib:$ENV{LD_LIBRARY_PATH})

# Enable linker to access the mpc-Libraries.
set(ENV{LD_LIBRARY_PATH} /files/abseher/mpc-1.0.1/lib:$ENV{LD_LIBRARY_PATH})

# Enable linker to access the GNU-MP-Libraries.
set(ENV{LD_LIBRARY_PATH} /files/abseher/gmp-5.1.3/lib:$ENV{LD_LIBRARY_PATH})

# Enable linker to access the sharp-Libraries.
set(ENV{LD_LIBRARY_PATH} /files/abseher/sharp-1.1.1/lib:$ENV{LD_LIBRARY_PATH})

# Enable linker to access the boost-Libraries.
set(ENV{LD_LIBRARY_PATH} /files/abseher/boost_1_55_0/lib:$ENV{LD_LIBRARY_PATH})

set(ENV{CMAKE_ROOT} /files/abseher/cmake-2.8.12.1-Linux-i386)

add_definitions("-Wall -pedantic -std=c++11 -DDISABLE_MULTI_THREADING")

set(CMAKE_CXX_FLAGS_GPROF "-O3 -g -pg")
set(CMAKE_LINKER_FLAGS_GPROF "-pg")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -DDEBUG") # Without -D_DEBUG there will be voodoo in libclasp's pod_vector.h, since then the size of our Clasp::PodVector is different than when we built clasp's debug release

# Unfortunately libclasp does not compile with -pedantic.
# Using -isystem instead of -I; then they are treated as system headers,
# i.e., warnings are suppressed for the clasp headers.
include_directories(SYSTEM ${clasp_dir}/libclasp)
include_directories(${sharp_dir}/include ${gringo_dir}/libgringo)

if(USE_STATIC_LIBS)
	set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)
	set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS)
	set(Boost_USE_STATIC_LIBS TRUE)
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
# XXX This is a workaround because flex causes these warnings with GCC
set_source_files_properties(${FLEX_Scanner_OUTPUTS} PROPERTIES COMPILE_FLAGS -Wno-sign-compare)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

#ADDED
include_directories(/files/abseher/boost_1_55_0/include)
set(SOURCES /files/abseher/boost_1_55_0/include/boost/functional/hash/hash.hpp)

add_executable(dflat
        ${SOURCES} #ADDED
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
	solver/DefaultJoin.cpp
	Debugger.cpp
	debugger/Dummy.cpp
	debugger/HumanReadable.cpp
	debugger/MachineReadable.cpp
	ItemTreeNode.cpp
	ItemTree.cpp
	ExtensionIterator.cpp
	Application.cpp
	main.cpp)

target_link_libraries(dflat gringo clasp sharp gmpxx gmp ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
