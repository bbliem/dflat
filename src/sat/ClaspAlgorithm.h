#pragma once

#include "../ClaspAlgorithm.h"

namespace sat {

class Problem;

class ClaspAlgorithm : public ::ClaspAlgorithm
{
	friend class ClaspCallback; // FIXME: See note in ClaspCallback.cpp
public:
	ClaspAlgorithm(Problem& problem, const char* exchangeNodeProgram);

	virtual std::auto_ptr<Clasp::ClaspFacade::Callback> newClaspCallback(sharp::TupleSet& newTuples, const ::GringoOutputProcessor&) const;
	virtual std::auto_ptr< ::GringoOutputProcessor> newGringoOutputProcessor() const;

protected:
#if PROGRESS_REPORT > 1
	virtual void printProgressLine(const sharp::ExtendedHypertree* node, size_t numChildTuples = 0);
#endif
#ifdef VERBOSE
	virtual void printBagContents(const sharp::VertexSet& vertices) const;
#endif
};

} // namespace sat
