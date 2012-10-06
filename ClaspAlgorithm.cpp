#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
#include <sstream>
#include <sharp/main>
#include <gringo/streams.h>

#include "ClaspAlgorithm.h"
#include "Problem.h"
#include "ClaspInputReader.h"
#include "Tuple.h"

using sharp::TupleSet;

namespace {
	inline void declareBagContents(std::ostream& bagContents, const Problem& problem, const sharp::VertexSet& vertices, const sharp::VertexSet& introduced, const sharp::VertexSet& removed)
	{
		foreach(sharp::Vertex v, vertices)
			problem.declareVertex(bagContents, v);
		foreach(sharp::Vertex v, removed)
			problem.declareVertex(bagContents, v);

		foreach(sharp::Vertex v, vertices)
			bagContents << "current(v" << v << ")." << std::endl;
		foreach(sharp::Vertex v, introduced)
			bagContents << "introduced(v" << v << ")." << std::endl;
		foreach(sharp::Vertex v, removed)
			bagContents << "removed(v" << v << ")." << std::endl;
	}
}

ClaspAlgorithm::ClaspAlgorithm(Problem& problem, const char* exchangeNodeProgram)
	: Algorithm(problem), exchangeNodeProgram(exchangeNodeProgram)
{
}

TupleSet* ClaspAlgorithm::exchangeLeaf(const sharp::VertexSet& vertices, const sharp::VertexSet& introduced, const sharp::VertexSet& removed)
{
	std::stringstream* bagContents = new std::stringstream;
	declareBagContents(*bagContents, dynamic_cast<Problem&>(problem), vertices, introduced, removed);

	Streams inputStreams;
	inputStreams.addFile(exchangeNodeProgram, false); // Second parameter: "relative" here means relative to the file added previously, which does not exist yet
	// Remember: "Streams" deletes the appended streams -_-
	inputStreams.appendStream(Streams::StreamPtr(bagContents), "<bag_contents>");

	std::auto_ptr<GringoOutputProcessor> outputProcessor = newGringoOutputProcessor();
	ClaspInputReader inputReader(inputStreams, *outputProcessor);

	TupleSet* newTuples = new TupleSet;
	std::auto_ptr<Clasp::ClaspFacade::Callback> cb = newClaspCallback(*newTuples, *outputProcessor);
	Clasp::ClaspConfig config;
	setClaspConfig(config);
	clasp.solve(inputReader, config, cb.get());

	return newTuples;
}

TupleSet* ClaspAlgorithm::exchangeNonLeaf(const sharp::VertexSet& vertices, const sharp::VertexSet& introduced, const sharp::VertexSet& removed, const sharp::TupleSet& childTuples)
{
	TupleSet* newTuples = new TupleSet;
	// There might be no child tuples, consider as a child e.g. a join node without matches.
	// If we were to run the program without child tuples, it would consider the current node as a leaf node and wrongly generate new tuples.
	if(childTuples.empty())
		return newTuples;

	std::stringstream* bagContents = new std::stringstream;
	declareBagContents(*bagContents, dynamic_cast<Problem&>(problem), vertices, introduced, removed);

	std::stringstream* childTuplesInput = new std::stringstream;
	// Declare child tuples
	foreach(const TupleSet::value_type& tupleAndSolution, childTuples)
		dynamic_cast<Tuple*>(tupleAndSolution.first)->declare(*childTuplesInput, tupleAndSolution);
//#ifdef VERBOSE
//	std::cout << "Child tuple input:" << std::endl << childTuplesInput->str() << std::endl;
//#endif

	Streams inputStreams;
	inputStreams.addFile(exchangeNodeProgram, false); // Second parameter: "relative" here means relative to the file added previously, which does not exist yet
	// Remember: "Streams" deletes the appended streams -_-
	inputStreams.appendStream(Streams::StreamPtr(bagContents), "<bag_contents>");
	inputStreams.appendStream(Streams::StreamPtr(childTuplesInput), "<child_tuples>");

	std::auto_ptr<GringoOutputProcessor> outputProcessor = newGringoOutputProcessor();
	ClaspInputReader inputReader(inputStreams, *outputProcessor);
	std::auto_ptr<Clasp::ClaspFacade::Callback> cb = newClaspCallback(*newTuples, *outputProcessor);
	Clasp::ClaspConfig config;
	setClaspConfig(config);
	clasp.solve(inputReader, config, cb.get());

	return newTuples;
}

void ClaspAlgorithm::setClaspConfig(Clasp::ClaspConfig& config) const
{
	config.enumerate.numModels = 0;
	config.master()->heuristic().name = "none";
}

std::auto_ptr<GringoOutputProcessor> ClaspAlgorithm::newGringoOutputProcessor() const
{
	return std::auto_ptr<GringoOutputProcessor>(new GringoOutputProcessor);
}
