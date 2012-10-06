# Unfortunately libclasp does not compile with -pedantic.
# Using -isystem instead of -I; then they are treated as system headers,
# i.e., warnings are suppressed for the clasp headers.

# debug build
#CXXFLAGS=-Wall -pedantic -ansi -I../sharp/include -isystem /home/bernhard/Informatik/clasp-2.0.3/libclasp -I/home/bernhard/Informatik/gringo-3.0.3-source/libgringo -DDISABLE_MULTI_THREADING -g
#LDFLAGS=-lgmpxx ../sharp/src/.libs/libsharp.a /home/bernhard/Informatik/clasp-2.0.3/build/release/libclasp/lib/libclasp.a /home/bernhard/Informatik/gringo-3.0.3-source/build/release/lib/libgringo.a -lboost_filesystem

# gprof build
#CXXFLAGS=-Wall -pedantic -ansi -I../sharp/include -isystem /home/bernhard/Informatik/clasp-2.0.3/libclasp -I/home/bernhard/Informatik/gringo-3.0.3-source/libgringo -DDISABLE_MULTI_THREADING -g -pg -DNDEBUG -O3
#LDFLAGS=-lgmpxx ../sharp/src/.libs/libsharp.a /home/bernhard/Informatik/clasp-2.0.3/build/gprof/libclasp/lib/libclasp.a /home/bernhard/Informatik/gringo-3.0.3-source/build/gprof/lib/libgringo.a -lboost_filesystem -pg

# release build
CXXFLAGS=-Wall -pedantic -ansi -I../sharp/include -isystem /home/bernhard/Informatik/clasp-2.0.3/libclasp -I/home/bernhard/Informatik/gringo-3.0.3-source/libgringo -DDISABLE_MULTI_THREADING -DNDEBUG -O3
LDFLAGS=-lgmpxx ../sharp/src/.libs/libsharp.a /home/bernhard/Informatik/clasp-2.0.3/build/release/libclasp/lib/libclasp.a /home/bernhard/Informatik/gringo-3.0.3-source/build/release/lib/libgringo.a -lboost_filesystem
# TODO: Remove -pg flags from gringo's build config again

CPP=gfilt -banner:N
#CPP=g++

all: Problem.o Algorithm.o ClaspInputReader.o GringoOutputProcessor.o Tuple.o ModelProcessor.o main.o
	${CPP} -o main $^ ${LDFLAGS}

main.o: main.cpp
	${CPP} -c main.cpp ${CXXFLAGS}

Problem.o: Problem.cpp Problem.h
	${CPP} -c Problem.cpp ${CXXFLAGS}

Algorithm.o: Algorithm.cpp Algorithm.h ClaspInputReader.h Problem.h Tuple.h ModelProcessor.h
	${CPP} -c Algorithm.cpp ${CXXFLAGS}

Tuple.o: Tuple.cpp Tuple.h Problem.h
	${CPP} -c Tuple.cpp ${CXXFLAGS}

GringoOutputProcessor.o: GringoOutputProcessor.cpp GringoOutputProcessor.h
	${CPP} -c GringoOutputProcessor.cpp ${CXXFLAGS}

ClaspInputReader.o: ClaspInputReader.cpp ClaspInputReader.h GringoOutputProcessor.h
	${CPP} -c ClaspInputReader.cpp ${CXXFLAGS}

ModelProcessor.o: ModelProcessor.cpp ModelProcessor.h GringoOutputProcessor.h Algorithm.h Tuple.h Problem.h
	${CPP} -c ModelProcessor.cpp ${CXXFLAGS}

clean:
	rm -f *.o main
