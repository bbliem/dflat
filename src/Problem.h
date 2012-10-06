#pragma once

#include <sharp/main>

class Problem : public sharp::Problem
{
public:
	Problem(const std::string& input, const std::set<std::string>& hyperedgePredicateNames);

	typedef std::pair<std::string, std::vector<std::string> > Fact; // Predicate name and (optional) arguments

	// To be used by the parser. Do not call directly.
	void parsedFact(const Fact&);

protected:
	virtual void parse();
	virtual void preprocess();
	virtual sharp::Hypergraph* buildHypergraphRepresentation();

private:
	const std::string& input;
	const std::set<std::string>& hyperedgePredicateNames;
	sharp::VertexSet vertices;
	sharp::HyperedgeSet hyperedges;
};
