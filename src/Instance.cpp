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
//}}}
#include <cassert>

#include "Instance.h"

void Instance::setVertexNames(std::vector<unsigned>&& n)
{
	names = std::move(n);
	const unsigned numVertices = names.size();
	terminals.resize(numVertices, false);
	adj.reserve(numVertices);
	names.reserve(numVertices);
	indexOf.reserve(numVertices);
	for(unsigned i = 0; i < numVertices; ++i) {
		adj.emplace_back(numVertices, 0);
		indexOf[names[i]] = i;
	}
}

void Instance::addEdge(unsigned x, unsigned y, unsigned w)
{
	assert(w > 0);
	assert(x < adj.size());
	assert(y < adj.size());
	assert(adj[x].size() == adj.size());
	assert(adj[y].size() == adj.size());
	adj[x][y] = w;
	adj[y][x] = w;
}

bool Instance::isTerminal(unsigned i) const
{
	assert(i < terminals.size());
	return terminals[i];
}

void Instance::setTerminal(unsigned i)
{
	assert(i < terminals.size());
	terminals[i] = true;
}

Instance Instance::induce(const std::vector<unsigned>& indices) const
{
	Instance newInst;
	std::vector<unsigned> newNames;
	newNames.reserve(indices.size());
	for(auto i : indices)
		newNames.push_back(names[i]);
	newInst.setVertexNames(std::move(newNames));
	for(unsigned i = 0; i < indices.size(); ++i) {
		newInst.terminals[i] = terminals[indices[i]];
		for(unsigned j = 0; j < indices.size(); ++j) {
			newInst.adj[i][j] = adj[indices[i]][indices[j]];
		}
	}
	return newInst;
}

Instance Instance::induceByNames(const std::vector<unsigned>& names) const
{
	std::vector<unsigned> indices;
	indices.reserve(names.size());
	for(unsigned name : names)
		indices.push_back(indexOf.at(name));
	return induce(indices);
}
