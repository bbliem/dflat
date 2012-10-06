#pragma once

#include "../Problem.h"

namespace cyclic_ordering {

class Problem : public ::Problem
{
public:
	typedef std::vector<std::string> StringTriple; // You may assume that instances of StringTriple always contain 3 elements
	typedef std::vector<StringTriple> Instance;

	Problem(std::istream& input);
	virtual ~Problem();

	const StringTriple& getOrdering(sharp::Vertex orderingVertex) const {
		return instance[orderingVertex-1];
	}

	virtual void declareVertex(std::ostream& out, sharp::Vertex v, const sharp::VertexSet& currentVertices) const;

protected:
	virtual void parse();
	virtual void preprocess();
	virtual sharp::Hypergraph* buildHypergraphRepresentation();

private:
	std::istream& input;
	Instance instance;

	typedef std::vector<sharp::Vertex> VertexTriple;
	typedef std::set<VertexTriple> VertexTripleSet;
	std::map<sharp::Vertex, VertexTripleSet> elementToTriples;
};

} // namespace cyclic_ordering
