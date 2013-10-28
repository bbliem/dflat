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

#include "DecompositionNode.h"

DecompositionNode::DecompositionNode(const Hypergraph::Vertices& bag)
	: bag(bag)
{
	static unsigned int nextGlobalId = 1;
	globalId = nextGlobalId++;
}

const Hypergraph::Vertices& DecompositionNode::getBag() const
{
	return bag;
}

unsigned int DecompositionNode::getGlobalId() const
{
	return globalId;
}

std::ostream& operator<<(std::ostream& os, const DecompositionNode& node)
{
	os << node.globalId << ' ';
	// Print bag
	os << '{';
	Hypergraph::Vertices::const_iterator it = node.bag.begin();
	if(it != node.bag.end()) {
			os << *it;
		while(++it != node.bag.end())
			os << ',' << *it;
	}
	os << '}';
	return os;
}
