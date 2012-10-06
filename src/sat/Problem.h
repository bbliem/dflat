#pragma once

#include "../Problem.h"

namespace sat {

class Problem : public ::Problem
{
public:
	typedef std::map<std::string, std::pair<std::set<std::string>,std::set<std::string> > > Instance; // Maps clauses to positive and negative atoms

	struct VerticesInClause {
		sharp::VertexSet pos;
		sharp::VertexSet neg;
	};

	Problem(std::istream& input);
	virtual ~Problem();

	//! Returns the TD-vertices corresponding to atoms in the given clause represented by clauseVertex. If clauseVertex is invalid, you are fscked.
	const VerticesInClause& getAtomsInClause(sharp::Vertex clauseVertex) const {
		return atomsInClauses[clauseVertex-1];
	}

	bool vertexIsClause(sharp::Vertex v) const {
		return v <= lastClause;
	}

	virtual void declareVertex(std::ostream& out, sharp::Vertex v, const sharp::VertexSet& currentVertices) const;

protected:
	virtual void parse();
	virtual void preprocess();
	virtual sharp::Hypergraph* buildHypergraphRepresentation();

private:
	std::istream& input;
	Instance instance;

	sharp::Vertex lastClause; // Vertices from 1 to this number correspond to clauses, afterwards to atoms

	// For each clause C, stores (P,N) where P are the vertices representing atoms which occur positively in C, etc.
	VerticesInClause* atomsInClauses;
};

} // namespace sat
