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

#include "Decomposition.h"

Decomposition::Decomposition(const Hypergraph::Vertices& bag)
	: bag(bag)
{
	static int nextGlobalId = 1;
	globalId = nextGlobalId++;
}

const Hypergraph::Vertices& Decomposition::getBag() const
{
	return bag;
}

void Decomposition::addChild(Decomposition* child)
{
	children.push_back(DecompositionPtr(child));
}

void Decomposition::printNode(std::ostream& os, bool root, bool last, std::string indent) const
{
	os << indent;

	if(!root) {
		if(last) {
#ifndef NO_UNICODE
			os << "┗━ ";
			indent += "   ";
#else
			os << "\\-";
			indent += "  ";
#endif
		}
		else {
#ifndef NO_UNICODE
			os << "┣━ ";
			indent += "┃  ";
#else
			os << "|-";
			indent += "| ";
#endif
		}
	}

	os << globalId << ' ';

	// Print bag
	os << '{';
	Hypergraph::Vertices::const_iterator it = bag.begin();
	if(it != bag.end()) {
			os << *it;
		while(++it != bag.end())
			os << ',' << *it;
	}
	os << '}' << std::endl;

	for(size_t i = 0; i < children.size(); ++i)
		children[i]->printNode(os, false, i + 1 == children.size(), indent);
}

std::ostream& operator<<(std::ostream& os, const Decomposition& decomposition)
{
	decomposition.printNode(os, true);
	return os;
}
