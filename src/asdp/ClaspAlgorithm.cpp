#include "ClaspAlgorithm.h"
#include "Problem.h"
#include "GringoOutputProcessor.h"
#include "ClaspCallback.h"
#include "ClaspCallbackNP.h"

namespace asdp {

ClaspAlgorithm::ClaspAlgorithm(Problem& problem, const sharp::PlanFactory& planFactory, const std::string& instanceFacts, const char* exchangeNodeProgram, const char* joinNodeProgram, sharp::NormalizationType normalizationType, unsigned int level)
	: ::ClaspAlgorithm(problem, planFactory, instanceFacts, exchangeNodeProgram, joinNodeProgram, normalizationType), level(level)
{
}


std::auto_ptr<Clasp::ClaspFacade::Callback> ClaspAlgorithm::newClaspCallback(sharp::TupleTable& newTable, const ::GringoOutputProcessor& gringoOutput) const
{
	if(level == 0)
		return std::auto_ptr<Clasp::ClaspFacade::Callback>(new ClaspCallbackNP(*this, newTable, dynamic_cast<const GringoOutputProcessor&>(gringoOutput)));
	else
		return std::auto_ptr<Clasp::ClaspFacade::Callback>(new ClaspCallback(*this, newTable, dynamic_cast<const GringoOutputProcessor&>(gringoOutput), level));
}

std::auto_ptr< ::GringoOutputProcessor> ClaspAlgorithm::newGringoOutputProcessor() const
{
	return std::auto_ptr< ::GringoOutputProcessor>(new GringoOutputProcessor(problem));
}

} // namespace asdp
