#pragma once

#include <string>
#include <vector>

namespace threeCol {

class Problem : public sharp::Problem
{
public:
	Problem(const std::string& input);

	// XXX: As of now, we only accept graphs where each node has at least one neighbor, since the tree decomposition heuristic only works this way. Isolated nodes extend the 3col solutions only in a trivial way anyway.
	typedef std::string Node;
	typedef std::pair<Node,Node> Edge;
//	typedef std::pair<std::vector<Node>,std::vector<Edge> > Graph;
	typedef std::vector<Edge> Graph;

protected:
	virtual void parse();
	virtual void preprocess();
	virtual sharp::Hypergraph* buildHypergraphRepresentation();

private:
	const std::string& input;
	Graph graph;
	std::map<sharp::Vertex, std::set<sharp::Vertex> > adjacent; // XXX: Access could be in O(1)
};

} // namespace threeCol
