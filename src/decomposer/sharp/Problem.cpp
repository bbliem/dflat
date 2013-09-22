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

#include "Problem.h"

namespace decomposer {
namespace sharp {

Problem::Problem(const Hypergraph& inst)
	: instance(instance)
{
}

void Problem::parse()
{
}

::sharp::Hypergraph* Problem::buildHypergraphRepresentation()
{
	::sharp::VertexSet vertices;
	::sharp::HyperedgeSet hyperedges;

	for(auto v : instance.getVertices())
		vertices.insert(storeVertexName(v));

	for(auto e : instance.getEdges()) {
		::sharp::VertexSet vs;
		for(auto v : e)
			vs.insert(getVertexId(v));
		hyperedges.insert(vs);
	}

	return createHypergraphFromSets(vertices, hyperedges);
}

} // namespace sharp
} // namespace decomposer
