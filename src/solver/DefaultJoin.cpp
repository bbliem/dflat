/*{{{
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
//}}}
#include "DefaultJoin.h"
#include "../Decomposition.h"
#include "../Application.h"
#include "../Debugger.h"

namespace {

bool isJoinable(const ItemTreeNode& left, const ItemTreeNode& right)
{
	return left.getItems() == right.getItems() &&
		(left.getType() == ItemTreeNode::Type::UNDEFINED || right.getType() == ItemTreeNode::Type::UNDEFINED || left.getType() == right.getType());
}

ItemTreePtr join(unsigned int leftNodeIndex, const ItemTreePtr& left, unsigned int rightNodeIndex, const ItemTreePtr& right)
{
	assert(left);
	assert(right);
	assert(left->getRoot());
	assert(right->getRoot());
	ItemTreePtr result;

	if(isJoinable(*left->getRoot(), *right->getRoot())) {
		// Join left and right
		ItemTreeNode::Items items = left->getRoot()->getItems();
		ItemTreeNode::Items auxItems = left->getRoot()->getAuxItems();
		auxItems.insert(right->getRoot()->getAuxItems().begin(), right->getRoot()->getAuxItems().end());
		ItemTreeNode::ExtensionPointers extensionPointers = {{{leftNodeIndex, left->getRoot()}, {rightNodeIndex, right->getRoot()}}};
		ItemTreeNode::Type type = left->getRoot()->getType() == ItemTreeNode::Type::UNDEFINED ? right->getRoot()->getType() : left->getRoot()->getType();
		result.reset(new ItemTree(ItemTree::Node(new ItemTreeNode(std::move(items), std::move(auxItems), std::move(extensionPointers), type))));

		// Join children recursively
		auto lit = left->getChildren().begin();
		auto rit = right->getChildren().begin();
		while(lit != left->getChildren().end() && rit != right->getChildren().end()) {
			ItemTreePtr childResult = join(leftNodeIndex, *lit, rightNodeIndex, *rit);
			if(childResult) {
				// lit and rit match
				// Remember position of rit. We will later advance rit until is doesn't match with lit anymore.
				auto mark = rit;
				do {
					// Join lit will all partners starting at rit
					do {
						result->addChildAndMerge(std::move(childResult));
						++rit;
						if(rit == right->getChildren().end())
							break;
						childResult = join(leftNodeIndex, *lit, rightNodeIndex, *rit);
					} while(childResult);

					// lit and rit don't match anymore (or rit is past the end)
					// Advance lit. If it joins with mark, reset rit to mark.
					++lit;
					if(lit == left->getChildren().end())
						break;
					childResult = join(leftNodeIndex, *lit, rightNodeIndex, *mark);
					if(childResult) {
						rit = mark;
						continue;
					}
				} while(false);
			}
			else {
				// lit and rit don't match
				// Advance iterator pointing to smaller value
				if((*lit)->getRoot()->getItems() < (*rit)->getRoot()->getItems())
					++lit;
				else
					++rit;
			}
		}
	}

	// In leafs, set cost and make sure two branches can only be joined if they have the same length
	if(result && result->getChildren().empty()) {
		if(!left->getChildren().empty() || !right->getChildren().empty())
			result.reset();
		else
			result->getRoot()->setCost(left->getRoot()->getCost() - left->getRoot()->getCurrentCost() + right->getRoot()->getCost());
	}

	return result;
}

} // anonymous namespace

namespace solver {

DefaultJoin::DefaultJoin(const Decomposition& decomposition, const Application& app)
	: Solver(decomposition, app)
{
}

ItemTreePtr DefaultJoin::compute()
{
	assert(decomposition.getChildren().size() > 1);
	// Compute item trees of child nodes
	// When at least two have been computed, join them with the result so far
	// TODO Use a balanced join tree (with smaller "tables" further down)
	auto it = decomposition.getChildren().begin();
	ItemTreePtr result = (*it)->getSolver().compute();
	unsigned int leftChildIndex = (*it)->getRoot().getGlobalId();
	for(++it; it != decomposition.getChildren().end(); ++it) {
		if(!result)
			return ItemTreePtr();
		ItemTreePtr itree = (*it)->getSolver().compute();
		if(!itree)
			return ItemTreePtr();
		result = join(leftChildIndex, result, (*it)->getRoot().getGlobalId(), itree);
		leftChildIndex = (*it)->getRoot().getGlobalId();
	}

	if(result)
		result->finalize();

	app.getDebugger().solverInvocationResult(decomposition.getRoot(), result.get());

	return result;
}

} // namespace solver
