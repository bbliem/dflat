#pragma once

namespace cyclic_ordering {

class Problem : public sharp::Problem
{
public:
	Problem(const std::string& input);

	// To be used by the parser. Do not call directly.
	void parsedOrdering(const std::vector<std::string>& args);

protected:
	virtual void parse();
	virtual void preprocess();
	virtual sharp::Hypergraph* buildHypergraphRepresentation();

private:
	const std::string& input;
	sharp::VertexSet vertices;
	sharp::HyperedgeSet hyperedges;
};

} // namespace cyclic_ordering
