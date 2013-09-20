/*
Copyright 2012-2013, Bernhard Bliem
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

#include <set>
#include <string>

class Hypergraph
{
public:
	// XXX Maybe using numbers instead of strings for vertices would be an improvement...
	typedef std::string Vertex;
	typedef std::set<Vertex> Vertices;
	typedef Vertices Edge;
	typedef std::set<Edge> Edges;

	const Vertices& getVertices() const { return vertices; }
	const Edges& getEdges() const { return edges; }

	void addVertex(const Vertex& v) { vertices.insert(v); }
	void addEdge(const Edge& e) { edges.insert(e); }

private:
	Vertices vertices;
	Edges edges;
};
