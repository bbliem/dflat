#pragma once

#include <clasp/clasp_facade.h>

#include "Algorithm.h"

class GringoOutputProcessor;

class ClaspAlgorithm : public Algorithm
{
public:
	ClaspAlgorithm(sharp::Problem& problem, const std::string& instanceFacts, const char* exchangeNodeProgram, const char* joinNodeProgram = 0, sharp::NormalizationType normalizationType = sharp::SemiNormalization);

protected:
	virtual sharp::TupleSet* exchangeLeaf(const sharp::VertexSet& vertices, const sharp::VertexSet& introduced, const sharp::VertexSet& removed);
	virtual sharp::TupleSet* exchangeNonLeaf(const sharp::VertexSet& vertices, const sharp::VertexSet& introduced, const sharp::VertexSet& removed, const sharp::TupleSet& childTuples);
	virtual sharp::TupleSet* join(const sharp::VertexSet& vertices, sharp::TupleSet& childTuplesLeft, sharp::TupleSet& childTuplesRight);
	virtual void setClaspConfig(Clasp::ClaspConfig& config) const;
	virtual std::auto_ptr<Clasp::ClaspFacade::Callback> newClaspCallback(sharp::TupleSet& newTuples, const GringoOutputProcessor&) const = 0;
	virtual std::auto_ptr<GringoOutputProcessor> newGringoOutputProcessor() const;

private:
	const std::string& instanceFacts;
	const char* exchangeNodeProgram;
	const char* joinNodeProgram;
	Clasp::ClaspFacade clasp;
};
