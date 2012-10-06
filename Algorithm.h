#pragma once

#include <sharp/main>

class Algorithm : public sharp::AbstractSemiNormalizedHTDAlgorithm
{
public:
	Algorithm(sharp::Problem& problem);

protected:
	virtual sharp::Solution* selectSolution(sharp::TupleSet* tuples, const sharp::ExtendedHypertree* root);
	virtual sharp::TupleSet* evaluateBranchNode(const sharp::ExtendedHypertree* node);

	/**
	 * Do not override this! Rather override exchangeLeaf() and
	 * exchangeNonLeaf(). This method calls them, deletes the child tuple set
	 * afterwards, and optionally outputs debug information.
	 */
	sharp::TupleSet* evaluatePermutationNode(const sharp::ExtendedHypertree* node);

	// Override these methods to process exchange nodes. Potential child tuples are passed to exchangeNonLeaf().
	virtual sharp::TupleSet* exchangeLeaf(const sharp::VertexSet& vertices, const sharp::VertexSet& introduced, const sharp::VertexSet& removed) = 0;
	virtual sharp::TupleSet* exchangeNonLeaf(const sharp::VertexSet& vertices, const sharp::VertexSet& introduced, const sharp::VertexSet& removed, const sharp::TupleSet& childTuples) = 0;

	sharp::Problem& problem;

protected:
#if PROGRESS_REPORT > 0
	int nodesProcessed; // For progress report
	virtual void printProgressLine(const sharp::ExtendedHypertree* node, size_t numChildTuples = 0);
	virtual sharp::TupleSet* evaluateNode(const sharp::ExtendedHypertree* node);
#endif
#ifdef VERBOSE
	virtual void printBagContents(const sharp::VertexSet& vertices) const;
#endif
};
