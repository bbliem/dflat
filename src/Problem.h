#pragma once

#include <sharp/main>

class Problem : public sharp::Problem
{
public:
	virtual void declareVertex(std::ostream& out, sharp::Vertex v, const sharp::VertexSet& currentVertices) const = 0;
};
