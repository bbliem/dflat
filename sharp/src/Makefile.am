AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/include/sharp -ansi --pedantic-error -Wall
AM_CTAGSFLAGS = --extra=+f

lib_LTLIBRARIES = libsharp.la
libsharp_la_SOURCES = ./AbstractAlgorithm.cpp \
./Global.cpp \
./Problem.cpp \
./htree/AbstractEliminationOrdering.cpp \
./htree/AbstractHypertreeDecompositionAlgorithm.cpp \
./htree/BucketEliminationAlgorithm.cpp \
./htree/Component.cpp \
./htree/CompSet.cpp \
./htree/ExtendedHypertree.cpp \
./htree/Globals.cpp \
./htree/H_BucketElim.cpp \
./htree/Hyperedge.cpp \
./htree/Hypergraph.cpp \
./htree/Hypertree.cpp \
./htree/MaximumCardinalitySearchOrdering.cpp \
./htree/MinimumDegreeOrdering.cpp \
./htree/MinimumFillOrdering.cpp \
./htree/Node.cpp \
./htree/Parser.cpp \
./htree/probSol.cpp \
./htree/SetCover.cpp \
./htree/SetCoverLS.cpp \
./htree/SpecialHEdge.cpp \
./htree/TabuSearch.cpp \
./support/support.cpp

libsharp_la_LDFLAGS = -version-info 0:0:0
