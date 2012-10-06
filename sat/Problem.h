#pragma once

#include "../Problem.h"

namespace sat {

class Problem : public ::Problem
{
public:
	typedef std::string Identifier;
	typedef std::string Atom;
	typedef std::pair<bool,Atom> Literal; // first is true iff negative
	typedef std::vector<Atom> Head;
	typedef std::vector<Literal> Body;
	typedef std::pair<Head,Body> Rule;
	typedef std::vector<Rule> Program;

	struct VerticesInRule {
		sharp::VertexSet head;
		sharp::VertexSet pos;
		sharp::VertexSet neg;
	};

	Problem(std::istream& input);
	virtual ~Problem();

	//! Returns the TD-vertices corresponding to atoms in the given rule represented by ruleVertex. If ruleVertex is invalid, you are fscked.
	const VerticesInRule& getAtomsInRule(sharp::Vertex ruleVertex) const {
		return atomsInRules[ruleVertex-1];
	}

	bool vertexIsRule(sharp::Vertex v) const {
		return v <= lastRule;
	}

	virtual void declareVertex(std::ostream& out, sharp::Vertex v) const;

protected:
	virtual void parse();
	virtual void preprocess();
	virtual sharp::Hypergraph* buildHypergraphRepresentation();

private:
	std::istream& input;
	Program program;

	sharp::Vertex lastRule; // Vertices from 1 to this number correspond to rules, afterwards to atoms

	// For each rule R, stores (H,P,N) where H are the vertices representing atoms which occur in the head of R, etc.
	VerticesInRule* atomsInRules;
};

} // namespace sat
