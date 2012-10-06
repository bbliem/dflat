#include "ClaspAlgorithm.h"
#include "Problem.h"
#include "GringoOutputProcessor.h"
#include "ClaspCallback.h"
#include "ClaspCallbackNP.h"

namespace asdp {

ClaspAlgorithm::ClaspAlgorithm(Problem& problem, const char* exchangeNodeProgram, const std::string& instanceFacts, sharp::NormalizationType normalizationType, unsigned int level)
	: ::ClaspAlgorithm(problem, exchangeNodeProgram, instanceFacts, normalizationType), level(level)
{
}


std::auto_ptr<Clasp::ClaspFacade::Callback> ClaspAlgorithm::newClaspCallback(sharp::TupleSet& newTuples, const ::GringoOutputProcessor& gringoOutput, const sharp::VertexSet&) const
{
	if(level == 0)
		return std::auto_ptr<Clasp::ClaspFacade::Callback>(new ClaspCallbackNP(*this, newTuples, dynamic_cast<const GringoOutputProcessor&>(gringoOutput)));
	else
		return std::auto_ptr<Clasp::ClaspFacade::Callback>(new ClaspCallback(*this, newTuples, dynamic_cast<const GringoOutputProcessor&>(gringoOutput), level));
}

std::auto_ptr< ::GringoOutputProcessor> ClaspAlgorithm::newGringoOutputProcessor() const
{
	return std::auto_ptr< ::GringoOutputProcessor>(new GringoOutputProcessor(problem));
}

} // namespace asdp
