#pragma once

#include <sharp/main>
#include <clasp/clasp_facade.h>

class GringoOutputProcessor;

class Algorithm : public sharp::AbstractSemiNormalizedHTDAlgorithm
{
public:
	//! @param normalizationType either sharp::SemiNormalization or sharp::DefaultNormalization
	Algorithm(sharp::Problem& problem, const sharp::PlanFactory& planFactory, const std::string& instanceFacts, const char* exchangeNodeProgram, const char* joinNodeProgram = 0, sharp::NormalizationType normalizationType = sharp::SemiNormalization, unsigned int level = 0);

protected:
	virtual sharp::Plan* selectPlan(sharp::TupleTable* table, const sharp::ExtendedHypertree* root);
	virtual sharp::ExtendedHypertree* prepareHypertreeDecomposition(sharp::ExtendedHypertree* root);

	// If you want to derive from this class, do not override evaluate*Node, but rather exchange*/join.
	sharp::TupleTable* evaluatePermutationNode(const sharp::ExtendedHypertree* node);
	sharp::TupleTable* evaluateBranchNode(const sharp::ExtendedHypertree* node);

	virtual sharp::TupleTable* exchangeLeaf(const sharp::VertexSet& vertices, const sharp::VertexSet& introduced, const sharp::VertexSet& removed);
	virtual sharp::TupleTable* exchangeNonLeaf(const sharp::VertexSet& vertices, const sharp::VertexSet& introduced, const sharp::VertexSet& removed, const sharp::TupleTable& childTable);
	virtual sharp::TupleTable* join(const sharp::VertexSet& vertices, sharp::TupleTable& childTableLeft, sharp::TupleTable& childTableRight);

	virtual std::auto_ptr<Clasp::ClaspFacade::Callback> newClaspCallback(sharp::TupleTable& newTable, const GringoOutputProcessor&) const;
	virtual std::auto_ptr<GringoOutputProcessor> newGringoOutputProcessor() const;

	sharp::Problem& problem;
	sharp::NormalizationType normalizationType;
	const std::string& instanceFacts;
	const char* exchangeNodeProgram;
	const char* joinNodeProgram;
	unsigned int level;
	Clasp::ClaspFacade clasp;

#ifdef PROGRESS_REPORT
	int nodesProcessed; // For progress report
	virtual void printProgressLine(const sharp::ExtendedHypertree* node, size_t numChildTuples = 0);
	virtual sharp::TupleTable* evaluateNode(const sharp::ExtendedHypertree* node);
#endif
#ifdef VERBOSE
	virtual void printBagContents(const sharp::VertexSet& vertices) const;
#endif
};
