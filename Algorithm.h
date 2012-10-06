#pragma once

#include <sharp/AbstractAlgorithm.hpp>
#include <clasp/clasp_facade.h>

class ModelProcessor;

class Algorithm : public sharp::AbstractSemiNormalizedHTDAlgorithm
{
	// XXX: IIRC this is just because there are protected methods in Algorithm's super class which do not belong there...
	friend class ModelProcessor;
public:
	Algorithm(sharp::Problem& problem);
	virtual ~Algorithm();

protected:
	virtual sharp::Solution* selectSolution(sharp::TupleSet* tuples, const sharp::ExtendedHypertree* root);
	virtual sharp::TupleSet* evaluateBranchNode(const sharp::ExtendedHypertree* node);
	virtual sharp::TupleSet* evaluatePermutationNode(const sharp::ExtendedHypertree* node);

private:
	Clasp::ClaspFacade clasp;
};
