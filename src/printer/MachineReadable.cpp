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
#include <iostream>
#include <sstream>

#include "MachineReadable.h"
#include "../Decomposition.h"
#include "../Application.h"
#include "../solver/clasp/Solver.h"
#include "../asp_utils.h"

namespace {

void declareItemTreeNodeMemoryAddresses(std::ostream& out, const ItemTree* itemTree, const std::string& itemSetName)
{
	if(!itemTree)
		return;

	out << "itemTreeNodeHasAddress(" << itemSetName << ',' << itemTree->getNode().get() << ")." << std::endl;
	size_t i = 0;
	for(const auto& child : itemTree->getChildren()) {
		std::ostringstream childName;
		childName << itemSetName << '_' << i++;
		declareItemTreeNodeMemoryAddresses(out, child.get(), childName.str());
	}
}

void declareExtensionPointers(std::ostream& out, const ItemTree* itemTree, const std::string& itemSetName)
{
	if(!itemTree)
		return;

	for(const auto& tuple : itemTree->getNode()->getExtensionPointers()) {
		out << "itemTreeNodeExtends(" << itemSetName << ",tuple(";
		std::string sep;
		for(const auto& ep : tuple) {
			out << sep << ep.get();
			sep = ",";
		}
		out << "))." << std::endl;
	}

	size_t i = 0;
	for(const auto& child : itemTree->getChildren()) {
		std::ostringstream childName;
		childName << itemSetName << '_' << i++;
		declareExtensionPointers(out, child.get(), childName.str());
	}
}

void declareDerivedCosts(std::ostream& out, const ItemTree* itemTree, const std::string& itemSetName)
{
	// Only print the costs for non-leaf nodes, as leaves' costs have already been printed in asp_utils::declareItemTree().
	if(!itemTree || itemTree->getChildren().empty())
		return;

	out << "childCost(" << itemSetName << ',' << itemTree->getNode()->getCost() << ")." << std::endl;

	size_t i = 0;
	for(const auto& child : itemTree->getChildren()) {
		std::ostringstream childName;
		childName << itemSetName << '_' << i++;
		declareDerivedCosts(out, child.get(), childName.str());
	}
}

} // anonymous namespace

namespace printer {

MachineReadable::MachineReadable(Application& app, bool newDefault)
	: Printer(app, "machine", "Machine-readable debugging output", newDefault)
{
}

void MachineReadable::decomposerResult(const Decomposition& result)
{
}

void MachineReadable::solverInvocationInput(const Decomposition& decompositionNode, const std::string& input)
{
	std::cout << "% Input for solver at decomposition node " << decompositionNode.getNode().getGlobalId() << std::endl << input << std::endl;
}

void MachineReadable::solverInvocationResult(const Decomposition& decompositionNode, const ItemTree* result)
{
	const auto id = decompositionNode.getNode().getGlobalId();
	if(result) {
		std::cout << "% Facts describing the resulting item tree at node " << id << std::endl;
		std::ostringstream rootItemSetName;
		rootItemSetName << 'n' << id;
		asp_utils::declareItemTree(std::cout, result, false, id, rootItemSetName.str());
		std::cout << std::endl;

		std::cout << "% Memory locations of the item tree nodes at decomposition node " << id << " (not passed to ASP)" << std::endl;
		declareItemTreeNodeMemoryAddresses(std::cout, result, rootItemSetName.str());
		std::cout << std::endl;

		std::cout << "% Extension pointers at decomposition node " << id << " (not passed to ASP)" << std::endl;
		declareExtensionPointers(std::cout, result, rootItemSetName.str());
		std::cout << std::endl;

		std::cout << "% (Derived) costs of non-leaf nodes of the item tree at decomposition node " << id << " (not passed to ASP)" << std::endl;
		declareDerivedCosts(std::cout, result, rootItemSetName.str());
		std::cout << std::endl;
	}
	else
		std::cout << "% Item tree of node " << id << " is empty." << std::endl;
}

bool MachineReadable::listensForSolverEvents() const
{
	return false;
}

void MachineReadable::solverEvent(const std::string& msg)
{
}

} // namespace printer
