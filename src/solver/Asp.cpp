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

#include <sstream>
#include <cassert>

#include "Asp.h"
#include "../Application.h"
#include "../ItemTree.h"
#include "../Decomposition.h"

#include <iostream> // XXX

namespace solver {

ItemTree Asp::compute()
{
	// Input: Child item trees
	std::unique_ptr<std::stringstream> childItemTrees(new std::stringstream);
	declareChildItemTrees(*childItemTrees);
	std::cout << "Child item trees:" << std::endl << childItemTrees->str() << std::endl;

	// Input: Original problem instance
	std::unique_ptr<std::stringstream> instance(new std::stringstream);
	*instance << app.getInputString();
	std::cout << "Instance:" << std::endl << instance->str() << std::endl;

	// Input: Decomposition
	std::unique_ptr<std::stringstream> decompositionInput(new std::stringstream);
	declareDecomposition(*decompositionInput);
	std::cout << "Decomposition:" << std::endl << decompositionInput->str() << std::endl;

//	// Put these inputs together
//	Streams inputStreams;
//	inputStreams.addFile(getUserProgram(node), false); // Second parameter: "relative" here means relative to the file added previously, which does not exist yet
//	// Remember: "Streams" deletes the appended streams -_-
//	inputStreams.appendStream(Streams::StreamPtr(instance.release()), "<instance>");
//	inputStreams.appendStream(Streams::StreamPtr(bag.release()), "<bag>");
//	inputStreams.appendStream(Streams::StreamPtr(childTableInput.release()), "<child_rows>");
//
//	// Call the ASP solver
//	std::auto_ptr<GringoOutputProcessor> outputProcessor = newGringoOutputProcessor();
//	ClaspInputReader inputReader(inputStreams, *outputProcessor);
//	std::auto_ptr<Clasp::ClaspFacade::Callback> cb = newClaspCallback(*newTable, *outputProcessor, childTables, node.getVertices());
//	Clasp::ClaspConfig config;
//	// config.master()->heuristic().name = "none"; // before clasp 2.1.1
//	// config.master()->solver->strategies().heuId = Clasp::ClaspConfig::heu_none; // for clasp 2.1.1, but it seems switching the heuristic off does not pay off anymore...?
//	config.enumerate.numModels = 0;
//	clasp.solve(inputReader, config, cb.get());
//
//	for(const Decomposition::ChildPtr& child : decomposition.getChildren())
//		itree.addChild(ItemTreePtr(new ItemTree(child->getSolver().compute())));

	ItemTree itree = ItemTreeNode({}); // empty root
	itree.addChild(ItemTreePtr(new ItemTree(ItemTreeNode({std::to_string(decomposition.getRoot().getGlobalId())}))));
	itree.addChild(ItemTreePtr(new ItemTree(ItemTreeNode({"foo"}))));

	return itree;
}

void Asp::declareDecomposition(std::ostream& out) const
{
	out << "currentNode(" << decomposition.getRoot().getGlobalId() << ")." << std::endl;

	for(const auto& child : decomposition.getChildren())
		out << "childNode(" << child->getRoot().getGlobalId() << ")." << std::endl;

	for(const auto& parent : decomposition.getParents())
		out << "parentNode(" << parent->getRoot().getGlobalId() << ")." << std::endl;

	for(const auto& v : decomposition.getRoot().getBag()) {
		out << "bag(" << decomposition.getRoot().getGlobalId() << ',' << v << "). ";
		out << "current(" << v << ")." << std::endl;
	}

	for(const auto& child : decomposition.getChildren())
		for(const auto& v : child->getRoot().getBag())
			out << "bag(" << child->getRoot().getGlobalId() << ',' << v << ")." << std::endl;

	for(const auto& parent : decomposition.getParents())
		for(const auto& v : parent->getRoot().getBag())
			out << "bag(" << parent->getRoot().getGlobalId() << ',' << v << ")." << std::endl;

	// Redundant predicates for convenience...
	out << "-introduced(X) :- childBag(_,X)." << std::endl;
	out << "introduced(X) :- current(X), not -introduced(X)." << std::endl;
	out << "removed(X) :- childBag(_,X), not current(X)." << std::endl;
	out << "-root :- parentNode(_)." << std::endl;
	out << "root :- not -root." << std::endl;
}

void Asp::declareChildItemTrees(std::ostream& out) const
{
	for(const auto& child : decomposition.getChildren()) {
		ItemTree itree = child->getSolver().compute();
		assert(itree.getRoot().getItems().empty());

		size_t rowIndex = 0;
		for(const ItemTreePtr& row : itree.getChildren()) {
			assert(row->getChildren().empty());
			std::ostringstream rowName;
			rowName << 'r' << child->getRoot().getGlobalId() << '_' << (rowIndex++);
			out << "childRow(" << rowName.str() << ',' << child->getRoot().getGlobalId() << ")." << std::endl;
			for(const auto& item : row->getRoot().getItems())
				out << "childItem(" << rowName.str() << ',' << item << ")." << std::endl;
		}
	}
}

} // namespace solver
