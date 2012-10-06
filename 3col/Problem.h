#pragma once

#include <string>
#include <vector>

#include "../Problem.h"

namespace threeCol {

class Problem : public ::Problem
{
public:
	Problem(std::istream& input);

	virtual void declareVertex(std::ostream& out, sharp::Vertex v) const;

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
	std::istream& input;
	Graph graph;
	std::map<sharp::Vertex, std::set<sharp::Vertex> > adjacent; // XXX: Access could be in O(1)
};

} // namespace threeCol
