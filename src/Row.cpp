/*{{{
Copyright 2012-2016, Bernhard Bliem, Marius Moldovan
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
#include <memory>
#include <limits>

#include "Row.h"
#include "Decomposition.h"

Row::Row(AdjacencyMatrix&& selectedEdges, Row::VertexList&& selectedVertices, AdjacencyMatrix&& connectedViaSelectedEdges, bool hasForgottenComponent, ExtensionPointers&& extensionPointers)
	: extensionPointers(std::move(extensionPointers))
	, selectedEdges(std::move(selectedEdges))
	, selectedVertices(selectedVertices)
	, connectedViaSelectedEdges(connectedViaSelectedEdges)
	, hasForgottenComponent(hasForgottenComponent)
{
	assert(!this->extensionPointers.empty());
	count = 0;
	for(const ExtensionPointerTuple& tuple : this->extensionPointers) {
		mpz_class product = 1;
		for(const auto& predecessor : tuple)
			product *= predecessor->getCount();
		count += product;
	}

	cost = 0;
	currentCost = 0;
}

void Row::setCost(unsigned long cost)
{
	this->cost = cost;
}

void Row::setCurrentCost(unsigned long currentCost)
{
	this->currentCost = currentCost;
}

void Row::merge(Row&& other)
{
	//assert(items == other.items);
	//assert(auxItems == other.auxItems);
	// TODO add replacements for the above assertions
	assert(cost == other.cost);
	assert(currentCost == other.currentCost);

	// Merge other row's data into this
	extensionPointers.insert(extensionPointers.end(), other.extensionPointers.begin(), other.extensionPointers.end());
	count += other.count;
}

Row::EdgeSet Row::firstExtension(const Decomposition& node) const
{
	EdgeSet result;
	assert(extensionPointers.size() > 0);
	ExtensionPointerTuple ept = extensionPointers.front();
	//for(const ExtensionPointer& ep : ept)
	assert(ept.size() == node.getChildren().size());
	for(unsigned i = 0; i < ept.size(); ++i) {
		const ExtensionPointer& ep = ept[i];
		const EdgeSet childResult = ep->firstExtension(*node.getChildren()[i]);
		result.insert(childResult.begin(), childResult.end());
	}

	const std::vector<unsigned>& names = node.getNode().getInducedInstance().getVertexNames();
	for(unsigned i = 0; i < selectedEdges.getNumRows(); ++i) {
		for(unsigned j = 0; j < i; ++j) {
			if(selectedEdges(i,j)) {
				if(names[i] < names[j])
					result.emplace(names[i], names[j]);
				else
					result.emplace(names[j], names[i]);
			}
		}
	}
	return result;
}

void Row::printWithNames(std::ostream& os, const std::vector<unsigned>& names) const
{
	assert(names.empty() || names.size() == selectedEdges.getNumRows());
	// Print count
	//os << '[' << row.count << "] ";

	selectedEdges.printWithNames(os, names);

	//os << "; extend: {";
	//std::string tupleSep;
	//for(const auto& tuple : row.extensionPointers) {
	//    os << tupleSep << '(';
	//    std::string ptrSep;
	//    for(const auto& extended : tuple) {
	//        os << ptrSep << extended.first << ':' << extended.second.get();
	//        ptrSep = ", ";
	//    }
	//    os << ')';
	//    tupleSep = ", ";
	//}
	//os << "}, this: " << &row << ", parent: " << row.parent;

	os << "; selected vertices:";
	assert(selectedVertices.size() == selectedEdges.getNumRows());
	for(unsigned i = 0; i < selectedVertices.size(); ++i) {
		if(selectedVertices[i])
			os << ' ' << (names.empty() ? i : names[i]);
	}

	os << "; connected:";
	connectedViaSelectedEdges.printWithNames(os, names);

	if(cost != 0) {
		os << "; cost: " << cost;
		if(currentCost != 0)
			os << "; current cost: " << currentCost;
	}

	os << "; forgotten component: " << hasForgottenComponent;
}

std::ostream& operator<<(std::ostream& os, const Row& row)
{
	row.printWithNames(os, {});
	return os;
}

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Row>& row)
{
	return os << *row;
}
