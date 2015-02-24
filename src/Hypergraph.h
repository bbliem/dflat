/*{{{
Copyright 2012-2015, Bernhard Bliem
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
#include <set>
#include <unordered_map>
#include <string>
#include <vector>
#ifdef DECOMPOSITION_COMPATIBILITY // Define this to generate the same decompositions as D-FLAT 0.2 when setting the same random seed
#include <algorithm>
#endif

class Hypergraph
{
public:
	// XXX Maybe using numbers instead of strings for vertices would be an improvement...
	typedef std::string Vertex;
#ifdef DECOMPOSITION_COMPATIBILITY // Define this to generate the same decompositions as D-FLAT 0.2 when setting the same random seed
	typedef std::vector<Vertex> Vertices;
#else
	typedef std::set<Vertex> Vertices;
#endif
	typedef std::vector<Vertex> Edge;
	typedef std::set<Edge> Edges;
	typedef std::unordered_map<std::string, Edges> EdgesOfKinds;

	const Vertices& getVertices() const { return vertices; }
	const Edges& getEdges() const { return edges; }
	const Edges& getEdgesOfKind(const std::string& kind) const
	{
		const EdgesOfKinds::const_iterator it = edgesOfKinds.find(kind);
		return it == edgesOfKinds.end() ? emptyEdgeSet : it->second;
	}

#ifdef DECOMPOSITION_COMPATIBILITY // Define this to generate the same decompositions as D-FLAT 0.2 when setting the same random seed
	void addVertex(const Vertex& v) { if(std::find(vertices.begin(), vertices.end(), v) == vertices.end()) vertices.push_back(v); }
#else
	void addVertex(const Vertex& v) { vertices.insert(v); }
#endif
	void addEdge(const Edge& e) { edges.insert(e); }
	void addEdgeOfKind(const Edge& e, const std::string& kind) { edges.insert(e); edgesOfKinds[kind].insert(e); }

private:
	Vertices vertices;
	Edges edges; // contains all edges of all kinds
	EdgesOfKinds edgesOfKinds;
	Edges emptyEdgeSet; // always empty, to be returned by getEdgesOfKind() for unknown kind
};
