#pragma once

#include "../ClaspAlgorithm.h"

namespace sat {

class Problem;

class ClaspAlgorithm : public ::ClaspAlgorithm
{
	friend class ClaspCallback; // FIXME: See note in ClaspCallback.cpp
public:
	ClaspAlgorithm(Problem& problem, const char* exchangeNodeProgram, const std::string& instanceFacts, sharp::NormalizationType normalizationType = sharp::SemiNormalization);

	virtual std::auto_ptr<Clasp::ClaspFacade::Callback> newClaspCallback(sharp::TupleSet& newTuples, const ::GringoOutputProcessor&, const sharp::VertexSet& vertices) const;
	virtual std::auto_ptr< ::GringoOutputProcessor> newGringoOutputProcessor() const;

protected:
	// This doesn't work anymore since I threw out Problem::vertexIsClause(). Maybe add something similar?
//#if PROGRESS_REPORT > 1
//	virtual void printProgressLine(const sharp::ExtendedHypertree* node, size_t numChildTuples = 0);
//#endif
//#ifdef VERBOSE
//	virtual void printBagContents(const sharp::VertexSet& vertices) const;
//#endif
};

} // namespace sat
