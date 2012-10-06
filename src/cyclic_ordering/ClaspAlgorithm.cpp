#include "ClaspAlgorithm.h"
#include "Problem.h"
#include "GringoOutputProcessor.h"
#include "ClaspCallback.h"
#if PROGRESS_REPORT > 1
#include <iomanip>
#endif

namespace cyclic_ordering {

ClaspAlgorithm::ClaspAlgorithm(Problem& problem, const char* exchangeNodeProgram, sharp::NormalizationType normalizationType)
	: ::ClaspAlgorithm(problem, exchangeNodeProgram, normalizationType)
{
}


std::auto_ptr<Clasp::ClaspFacade::Callback> ClaspAlgorithm::newClaspCallback(sharp::TupleSet& newTuples, const ::GringoOutputProcessor& gringoOutput) const
{
	return std::auto_ptr<Clasp::ClaspFacade::Callback>(new ClaspCallback(*this, newTuples, dynamic_cast<const GringoOutputProcessor&>(gringoOutput)));
}

std::auto_ptr< ::GringoOutputProcessor> ClaspAlgorithm::newGringoOutputProcessor() const
{
	return std::auto_ptr< ::GringoOutputProcessor>(new GringoOutputProcessor);
}

} // namespace cyclic_ordering
