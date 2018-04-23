/*{{{
Copyright 2012-2016, Bernhard Bliem
WWW: <http://dbai.tuwien.ac.at/research/project/dflat/>.

This file is part of D-FLAT.

D-FLAT is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

D-FLAT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with D-FLAT.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
//}}}
#include <vector>
#include <unordered_map>

class Instance
{
public:
	typedef std::vector<std::vector<unsigned>> WeightedAdjacencyMatrix;

	// Call this before any other method.
	void setVertexNames(std::vector<unsigned>&& names);

	unsigned getNumVertices() const { return names.size(); }

	const std::vector<unsigned>& getVertexNames() const { return names; }

	unsigned getIndexOf(unsigned name) const { return indexOf.at(name); }

	const WeightedAdjacencyMatrix& getAdjacencyMatrix() const { return adj; }

	// Adds an undirected edge with given weight between the given vertices.
	// We assume that all weights are positive.
	// setNumVertices must have been set before and x,y must be valid indices.
	void addEdge(unsigned x, unsigned y, unsigned w);

	// Returns true iff the vertex with index i is a terminal.
	bool isTerminal(unsigned i) const;

	// setNumVertices must have been set accordingly before.
	// i must be a valid index.
	void setTerminal(unsigned i);

	// Induce a subinstance on the vertices with the given indices.
	Instance induce(const std::vector<unsigned>& indices) const;

	// Induce a subinstance on the vertices with the given names
	Instance induceByNames(const std::vector<unsigned>& names) const;

private:
	std::vector<bool> terminals; // terminals[i] is true iff i+1 is a terminal vertex
	WeightedAdjacencyMatrix adj; // 0 iff the vertices are not adjacent
	std::vector<unsigned> names; // Map indices to their names
	std::unordered_map<unsigned, unsigned> indexOf; // Map names to indices
};
