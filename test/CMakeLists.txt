include_directories(SYSTEM ${gtest_dir}/include)
include_directories(${gtest_dir})
include_directories(${CMAKE_SOURCE_DIR})
add_executable(tests
	Decomposition.cpp
	DecompositionNode.cpp
	DirectedAcyclicGraph.cpp
	Hypergraph.cpp
	Module.cpp
	)
target_link_libraries(tests dflat-objects gringo clasp sharp gmpxx gmp ${CMAKE_THREAD_LIBS_INIT} gtest gtest_main)
add_test(tests tests)
