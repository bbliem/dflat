#pragma once

#include <sharp/AbstractAlgorithm.hpp>
#include <clasp/clasp_facade.h>

class Problem;
class ModelProcessor;

class Algorithm : public sharp::AbstractSemiNormalizedHTDAlgorithm
{
	// XXX: IIRC this is just because there are protected methods in Algorithm's base class which do not belong there...
	friend class ModelProcessor;
public:
	enum ProblemType {
		ENUMERATION,
		COUNTING,
		DECISION
	};

	Algorithm(Problem& problem, ProblemType problemType = ENUMERATION);
	virtual ~Algorithm();

protected:
	virtual sharp::Solution* selectSolution(sharp::TupleSet* tuples, const sharp::ExtendedHypertree* root);
	virtual sharp::TupleSet* evaluateBranchNode(const sharp::ExtendedHypertree* node);
	virtual sharp::TupleSet* evaluatePermutationNode(const sharp::ExtendedHypertree* node);

	Problem& problem;

private:
	Clasp::ClaspFacade clasp;
	ProblemType problemType;

protected:
#ifndef NO_PROGRESS_REPORT
	int nodesProcessed; // For progress report
	void printProgressLine(const sharp::ExtendedHypertree* node);
	virtual sharp::TupleSet* evaluateNode(const sharp::ExtendedHypertree* node);
#endif
#ifdef VERBOSE
	void printBagContents(const sharp::VertexSet& vertices) const;
#endif
};
