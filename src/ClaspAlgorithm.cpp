#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH
#include <sstream>
#include <sharp/main>
#include <gringo/streams.h>

#include "ClaspAlgorithm.h"
#include "ClaspInputReader.h"
#include "Tuple.h"

using sharp::TupleTable;

namespace {
	inline void declareBagContents(std::ostream& bagContents, const sharp::Problem& problem, const std::string& instanceFacts, const sharp::VertexSet& vertices, const sharp::VertexSet& introduced, const sharp::VertexSet& removed)
	{
		bagContents << instanceFacts << std::endl;

		foreach(sharp::Vertex v, vertices)
			bagContents << "current(" << const_cast<sharp::Problem&>(problem).getVertexName(v) << ")." << std::endl;
		foreach(sharp::Vertex v, introduced)
			bagContents << "introduced(" << const_cast<sharp::Problem&>(problem).getVertexName(v) << ")." << std::endl;
		foreach(sharp::Vertex v, removed)
			bagContents << "removed(" << const_cast<sharp::Problem&>(problem).getVertexName(v) << ")." << std::endl;
	}
}

ClaspAlgorithm::ClaspAlgorithm(sharp::Problem& problem, const sharp::PlanFactory& planFactory, const std::string& instanceFacts, const char* exchangeNodeProgram, const char* joinNodeProgram, sharp::NormalizationType normalizationType)
	: Algorithm(problem, planFactory, normalizationType), instanceFacts(instanceFacts), exchangeNodeProgram(exchangeNodeProgram), joinNodeProgram(joinNodeProgram)
{
}

TupleTable* ClaspAlgorithm::exchangeLeaf(const sharp::VertexSet& vertices, const sharp::VertexSet& introduced, const sharp::VertexSet& removed)
{
	std::stringstream* bagContents = new std::stringstream;
	declareBagContents(*bagContents, problem, instanceFacts, vertices, introduced, removed);

	Streams inputStreams;
	inputStreams.addFile(exchangeNodeProgram, false); // Second parameter: "relative" here means relative to the file added previously, which does not exist yet
	// Remember: "Streams" deletes the appended streams -_-
	inputStreams.appendStream(Streams::StreamPtr(bagContents), "<bag_contents>");

	std::auto_ptr<GringoOutputProcessor> outputProcessor = newGringoOutputProcessor();
	ClaspInputReader inputReader(inputStreams, *outputProcessor);

	TupleTable* newTable = new TupleTable;
	std::auto_ptr<Clasp::ClaspFacade::Callback> cb = newClaspCallback(*newTable, *outputProcessor);
	Clasp::ClaspConfig config;
	setClaspConfig(config);
	clasp.solve(inputReader, config, cb.get());

	return newTable;
}

TupleTable* ClaspAlgorithm::exchangeNonLeaf(const sharp::VertexSet& vertices, const sharp::VertexSet& introduced, const sharp::VertexSet& removed, const sharp::TupleTable& childTable)
{
	TupleTable* newTable = new TupleTable;
	// There might be no child tuples, consider as a child e.g. a join node without matches.
	// If we were to run the program without child tuples, it would consider the current node as a leaf node and wrongly generate new tuples.
	if(childTable.empty())
		return newTable;

	std::stringstream* bagContents = new std::stringstream;
	declareBagContents(*bagContents, problem, instanceFacts, vertices, introduced, removed);

	std::stringstream* childTableInput = new std::stringstream;
	// Declare child tuples
	foreach(const TupleTable::value_type& tupleAndSolution, childTable)
		dynamic_cast<Tuple*>(tupleAndSolution.first)->declare(*childTableInput, tupleAndSolution);
#ifdef VERBOSE
	std::cout << std::endl << "Child tuple input:" << std::endl << childTableInput->str() << std::endl;
#endif

	Streams inputStreams;
	inputStreams.addFile(exchangeNodeProgram, false); // Second parameter: "relative" here means relative to the file added previously, which does not exist yet
	// Remember: "Streams" deletes the appended streams -_-
	inputStreams.appendStream(Streams::StreamPtr(bagContents), "<bag_contents>");
	inputStreams.appendStream(Streams::StreamPtr(childTableInput), "<child_tuples>");

	std::auto_ptr<GringoOutputProcessor> outputProcessor = newGringoOutputProcessor();
	ClaspInputReader inputReader(inputStreams, *outputProcessor);
	std::auto_ptr<Clasp::ClaspFacade::Callback> cb = newClaspCallback(*newTable, *outputProcessor);
	Clasp::ClaspConfig config;
	setClaspConfig(config);
	clasp.solve(inputReader, config, cb.get());

	return newTable;
}

TupleTable* ClaspAlgorithm::join(const sharp::VertexSet& vertices, sharp::TupleTable& childTableLeft, sharp::TupleTable& childTableRight)
{
	if(!joinNodeProgram)
		return Algorithm::join(vertices, childTableLeft, childTableRight);

	std::stringstream* bagContents = new std::stringstream;
	declareBagContents(*bagContents, problem, instanceFacts, vertices, sharp::VertexSet(), sharp::VertexSet());

	std::stringstream* childTableInput = new std::stringstream;
	// Declare child tuples
	foreach(const TupleTable::value_type& tupleAndSolution, childTableLeft)
		dynamic_cast<Tuple*>(tupleAndSolution.first)->declare(*childTableInput, tupleAndSolution, "L");
	foreach(const TupleTable::value_type& tupleAndSolution, childTableRight)
		dynamic_cast<Tuple*>(tupleAndSolution.first)->declare(*childTableInput, tupleAndSolution, "R");
#ifdef VERBOSE
	std::cout << std::endl << "Child tuple input:" << std::endl << childTableInput->str() << std::endl;
#endif

	Streams inputStreams;
	inputStreams.addFile(joinNodeProgram, false); // Second parameter: "relative" here means relative to the file added previously, which does not exist yet
	// Remember: "Streams" deletes the appended streams -_-
	inputStreams.appendStream(Streams::StreamPtr(bagContents), "<bag_contents>");
	inputStreams.appendStream(Streams::StreamPtr(childTableInput), "<child_tuples>");

	std::auto_ptr<GringoOutputProcessor> outputProcessor = newGringoOutputProcessor();
	ClaspInputReader inputReader(inputStreams, *outputProcessor);

	TupleTable* newTable = new TupleTable;
	std::auto_ptr<Clasp::ClaspFacade::Callback> cb = newClaspCallback(*newTable, *outputProcessor);
	Clasp::ClaspConfig config;
	setClaspConfig(config);
	clasp.solve(inputReader, config, cb.get());

	return newTable;
}

void ClaspAlgorithm::setClaspConfig(Clasp::ClaspConfig& config) const
{
	config.enumerate.numModels = 0;
//	config.master()->heuristic().name = "none";
}

std::auto_ptr<GringoOutputProcessor> ClaspAlgorithm::newGringoOutputProcessor() const
{
	return std::auto_ptr<GringoOutputProcessor>(new GringoOutputProcessor);
}
