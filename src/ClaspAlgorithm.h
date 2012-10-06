#pragma once

#include <clasp/clasp_facade.h>

#include "Algorithm.h"

class GringoOutputProcessor;

class ClaspAlgorithm : public Algorithm
{
public:
	ClaspAlgorithm(sharp::Problem& problem, const sharp::PlanFactory& planFactory, const std::string& instanceFacts, const char* exchangeNodeProgram, const char* joinNodeProgram = 0, sharp::NormalizationType normalizationType = sharp::SemiNormalization);

protected:
	virtual sharp::TupleTable* exchangeLeaf(const sharp::VertexSet& vertices, const sharp::VertexSet& introduced, const sharp::VertexSet& removed);
	virtual sharp::TupleTable* exchangeNonLeaf(const sharp::VertexSet& vertices, const sharp::VertexSet& introduced, const sharp::VertexSet& removed, const sharp::TupleTable& childTuples);
	virtual sharp::TupleTable* join(const sharp::VertexSet& vertices, sharp::TupleTable& childTableLeft, sharp::TupleTable& childTableRight);
	virtual void setClaspConfig(Clasp::ClaspConfig& config) const;
	virtual std::auto_ptr<Clasp::ClaspFacade::Callback> newClaspCallback(sharp::TupleTable& newTable, const GringoOutputProcessor&) const = 0;
	virtual std::auto_ptr<GringoOutputProcessor> newGringoOutputProcessor() const;

private:
	const std::string& instanceFacts;
	const char* exchangeNodeProgram;
	const char* joinNodeProgram;
	Clasp::ClaspFacade clasp;
};
