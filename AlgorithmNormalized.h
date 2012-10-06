#pragma once

#include "Algorithm.h"

// XXX: Note that this isn't derived from sharp::AbstractNormalizedHTDAlgorithm because we re-use code from the semi-normalized algorithm. Maybe I can think of something better?
class AlgorithmNormalized : public Algorithm
{
public:
	AlgorithmNormalized(Problem& problem, ProblemType problemType = ENUMERATION, AlgorithmType algorithmType = NORMALIZED);
	virtual ~AlgorithmNormalized();

protected:
	virtual sharp::ExtendedHypertree* prepareHypertreeDecomposition(sharp::ExtendedHypertree* root);

	virtual sharp::TupleSet* evaluatePermutationNode(const sharp::ExtendedHypertree* node);
	virtual sharp::TupleSet* evaluateIntroductionNode(const sharp::ExtendedHypertree* node);
	virtual sharp::TupleSet* evaluateRemovalNode(const sharp::ExtendedHypertree* node);

private:
	bool useSemiNormalizedAlgorithm;
};
