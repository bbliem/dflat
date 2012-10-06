#pragma once

#include "../ClaspAlgorithm.h"

namespace asdp {

class Problem;

class ClaspAlgorithm : public ::ClaspAlgorithm
{
	friend class ClaspCallback; // FIXME: See note in ClaspCallback.cpp
public:
	ClaspAlgorithm(Problem& problem, const char* exchangeNodeProgram, const std::string& instanceFacts, sharp::NormalizationType normalizationType = sharp::SemiNormalization);

	virtual std::auto_ptr<Clasp::ClaspFacade::Callback> newClaspCallback(sharp::TupleSet& newTuples, const ::GringoOutputProcessor&, const sharp::VertexSet& vertices) const;
	virtual std::auto_ptr< ::GringoOutputProcessor> newGringoOutputProcessor() const;
};

} // namespace asdp
