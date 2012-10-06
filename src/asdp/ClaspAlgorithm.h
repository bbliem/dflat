#pragma once

#include "../ClaspAlgorithm.h"

namespace asdp {

class Problem;

class ClaspAlgorithm : public ::ClaspAlgorithm
{
	friend class ClaspCallback; // FIXME: See ClaspCallback.cpp
	friend class ClaspCallbackNP; // FIXME: See ClaspCallbackNP.cpp
public:
	ClaspAlgorithm(Problem& problem, const sharp::PlanFactory& planFactory, const std::string& instanceFacts, const char* exchangeNodeProgram, const char* joinNodeProgram = 0, sharp::NormalizationType normalizationType = sharp::SemiNormalization, unsigned int level = 0);

	virtual std::auto_ptr<Clasp::ClaspFacade::Callback> newClaspCallback(sharp::TupleTable& newTable, const ::GringoOutputProcessor&) const;
	virtual std::auto_ptr< ::GringoOutputProcessor> newGringoOutputProcessor() const;

private:
	unsigned int level;
};

} // namespace asdp
