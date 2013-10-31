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

#include <iostream>
#include <sstream>

#include "MachineReadable.h"
#include "../Decomposition.h"
#include "../Application.h"
#include "../solver/Asp.h"

namespace {

void declareItemTreeNodeMemoryAddresses(std::ostream& out, const ItemTree* itemTree, const std::string& itemSetName)
{
	if(!itemTree)
		return;

	out << "itemTreeNodeHasAddress(" << itemSetName << ',' << itemTree->getRoot().get() << ")." << std::endl;
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

	for(const auto& tuple : itemTree->getRoot()->getExtensionPointers()) {
		out << "itemTreeNodeExtends(" << itemSetName << ",tuple(";
		std::string sep;
		for(const auto& ep : tuple) {
			out << sep << ep.second.get();
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

} // anonymous namespace

namespace debugger {

MachineReadable::MachineReadable(Application& app, bool newDefault)
	: Debugger(app, "machine", "Machine-readable debugging output", newDefault)
{
}

void MachineReadable::decomposerResult(const Decomposition& result) const
{
}

void MachineReadable::solverInvocationInput(const DecompositionNode& decompositionNode, const std::string& input) const
{
	std::cout << "% Input for solver at decomposition node " << decompositionNode.getGlobalId() << std::endl << input << std::endl;
}

void MachineReadable::solverInvocationResult(const DecompositionNode& decompositionNode, const ItemTree* result) const
{
	const auto id = decompositionNode.getGlobalId();
	if(result) {
		std::cout << "% Facts describing the resulting item tree at node " << id << std::endl;
		std::ostringstream rootItemSetName;
		rootItemSetName << 'n' << id;
		solver::Asp::declareItemTree(std::cout, result, false, id, rootItemSetName.str());
		std::cout << std::endl;

		std::cout << "% Memory locations of the item tree nodes at decomposition node " << id << " (not passed to ASP)" << std::endl;
		declareItemTreeNodeMemoryAddresses(std::cout, result, rootItemSetName.str());
		std::cout << std::endl;

		std::cout << "% Extension pointers at decomposition node " << id << " (not passed to ASP)" << std::endl;
		declareExtensionPointers(std::cout, result, rootItemSetName.str());
		std::cout << std::endl;
	}
	else
		std::cout << "% Item tree of node " << id << " is empty." << std::endl;
}

bool MachineReadable::listensForSolverEvents() const
{
	return false;
}

void MachineReadable::solverEvent(const std::string& msg) const
{
}

} // namespace debugger
