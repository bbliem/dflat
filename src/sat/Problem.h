#pragma once

#include <sharp/main>

namespace sat {

class Problem : public sharp::Problem
{
public:
	typedef std::map<std::string, std::pair<std::set<std::string>,std::set<std::string> > > Instance; // Maps clauses to positive and negative atoms

	Problem(const std::string& input);

protected:
	virtual void parse();
	virtual void preprocess();
	virtual sharp::Hypergraph* buildHypergraphRepresentation();

private:
	const std::string& input;
	Instance instance;
};

} // namespace sat
