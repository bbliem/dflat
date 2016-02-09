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
#include "ClaspCallback.h"
#include "../../../Application.h"
#include "../../../Printer.h"

namespace solver { namespace clasp { namespace trees {

ClaspCallback::ClaspCallback(const GringoOutputProcessor& gringoOutput, const ChildItemTrees& childItemTrees, const Application& app, const Decomposition& decomposition)
	: asp_utils::ClaspCallback(app)
	, gringoOutput(gringoOutput)
	, childItemTrees(childItemTrees)
	, decomposition(decomposition)
{
}

bool ClaspCallback::onModel(const Clasp::Solver& s, const Clasp::Model& m)
{
	asp_utils::ClaspCallback::onModel(s, m);

	struct BranchNode
	{
		ItemTreeNode::Items items;
		ItemTreeNode::Items auxItems;
		ItemTreeNode::ExtensionPointerTuple extended;
		ItemTreeNode::Type type = ItemTreeNode::Type::UNDEFINED;
	};

	// Get number of levels in the branch corresponding to this answer set {{{
	ASP_CHECK(countTrue(m, lengthAtomInfos) != 0, "No true length/1 atom");
	ASP_CHECK(countTrue(m, lengthAtomInfos) <= 1, "Multiple true length/1 atoms");
	unsigned int numLevels = 0;
	forFirstTrue(m, lengthAtomInfos, [&numLevels](const GringoOutputProcessor::LengthAtomArguments& arguments) {
			numLevels = arguments.length+1;
	});
	assert(numLevels > 0);
	std::vector<BranchNode> branchData(numLevels);
	// }}}
	// Get items {{{
	forEachTrue(m, itemAtomInfos, [&branchData](const GringoOutputProcessor::ItemAtomArguments& arguments) {
			ASP_CHECK(arguments.level < branchData.size(), "Item at level higher than branch length");
			branchData[arguments.level].items.insert(arguments.item);
	});
	forEachTrue(m, auxItemAtomInfos, [&branchData](const GringoOutputProcessor::AuxItemAtomArguments& arguments) {
			ASP_CHECK(arguments.level < branchData.size(), "Auxiliary item at level higher than branch length");
			branchData[arguments.level].auxItems.insert(arguments.item);
	});
	// }}}
	// Get extension pointers {{{
	forEachTrue(m, extendAtomInfos, [&branchData](const GringoOutputProcessor::ExtendAtomArguments& arguments) {
			ASP_CHECK(arguments.level < branchData.size(), "Extension pointer at level higher than branch length");
			branchData[arguments.level].extended.push_back(ItemTreeNode::ExtensionPointer(arguments.extendedNode));
	});
	// }}}
	// Checks on extension pointers and item sets {{{
#ifndef DISABLE_CHECKS
	for(const BranchNode& node : branchData) {
		ASP_CHECK(node.extended.size() == childItemTrees.size(), "Not all extension pointer tuples within a branch have arity n, where n is the number of children in the decomposition");

		ASP_CHECK(std::find_if(node.items.begin(), node.items.end(), [&node](const String& item) {
				   return node.auxItems.find(item) != node.auxItems.end();
		}) == node.items.end(), "Items and auxiliary items not disjoint");
	}

	for(const auto& pointer : branchData[0].extended)
		ASP_CHECK(pointer->getParent() == nullptr, "Level 0 extension pointer does not point to the root of an item tree");

	auto curNode = branchData.begin();
	auto prevNode = curNode++;
	while(curNode != branchData.end()) {
//		ASP_CHECK(std::all_of(curNode->extended.begin(), curNode->extended.end(), [&prevNode](const ItemTreeNode::ExtensionPointerTuple::value_type& pair) {
//				return pair.second->getParent() == prevNode->extended.at(pair.first).get();
//		}), "Extension pointer at level n+1 does not point to a child of the extended level-n node");
		assert(curNode->extended.size() == prevNode->extended.size());
		for(unsigned int i = 0; i < curNode->extended.size(); ++i)
			ASP_CHECK(curNode->extended[i]->getParent() == prevNode->extended[i].get(), "Extension pointer at level n+1 does not point to a child of the extended level-n node");
		++prevNode;
		++curNode;
	}

	assert(!uncompressedItemTree || uncompressedItemTree->getNode()->getExtensionPointers().size() == 1);
	ASP_CHECK(!uncompressedItemTree ||
			(uncompressedItemTree->getNode()->getItems() == branchData.front().items &&
			 uncompressedItemTree->getNode()->getAuxItems() == branchData.front().auxItems &&
			 uncompressedItemTree->getNode()->getExtensionPointers().front() == branchData.front().extended),
			"Item tree branches specify different roots");
#endif
	// }}}
	// Set item tree node types (or, and, accept or reject) {{{
	forEachTrue(m, orAtomInfos, [&branchData](const GringoOutputProcessor::OrAtomArguments& arguments) {
			ASP_CHECK(arguments.level + 1 < branchData.size(), "Item tree node type specificiation 'or' at level higher than or equal to branch length");
			ASP_CHECK(branchData[arguments.level].type == ItemTreeNode::Type::UNDEFINED, "More than one type specified for an item tree node");
			branchData[arguments.level].type = ItemTreeNode::Type::OR;
	});
	forEachTrue(m, andAtomInfos, [&branchData](const GringoOutputProcessor::AndAtomArguments& arguments) {
			ASP_CHECK(arguments.level + 1 < branchData.size(), "Item tree node type specificiation 'and' at level higher than or equal to branch length");
			ASP_CHECK(branchData[arguments.level].type == ItemTreeNode::Type::UNDEFINED, "More than one type specified for an item tree node");
			branchData[arguments.level].type = ItemTreeNode::Type::AND;
	});
	if(acceptLiteral && m.isTrue(*acceptLiteral)) {
		ASP_CHECK(branchData.back().type == ItemTreeNode::Type::UNDEFINED, "More than one type specified for an item tree leaf");
		branchData.back().type = ItemTreeNode::Type::ACCEPT;
	}
	if(rejectLiteral && m.isTrue(*rejectLiteral)) {
		ASP_CHECK(branchData.back().type == ItemTreeNode::Type::UNDEFINED, "More than one type specified for an item tree leaf");
		branchData.back().type = ItemTreeNode::Type::REJECT;
	}
	// }}}
	// Convert branchData to UncompressedItemTree::Branch {{{
	UncompressedItemTree::Branch branch;
	branch.reserve(numLevels);
	if(uncompressedItemTree)
		branch.emplace_back(uncompressedItemTree->getNode());
	else
		branch.emplace_back(UncompressedItemTree::Node(new ItemTreeNode(std::move(branchData.front().items), std::move(branchData.front().auxItems), {std::move(branchData.front().extended)}, branchData.front().type)));

	for(size_t i = 1; i < branchData.size(); ++i)
		branch.emplace_back(UncompressedItemTree::Node(new ItemTreeNode(std::move(branchData[i].items), std::move(branchData[i].auxItems), {std::move(branchData[i].extended)}, branchData[i].type)));
	// }}}
	if(!app.isOptimizationDisabled()) {
		// Set cost {{{
		ASP_CHECK(countTrue(m, costAtomInfos) <= 1, "More than one true cost/1 atom");
		ASP_CHECK(countTrue(m, costAtomInfos) == 0 || std::all_of(branchData.begin(), branchData.end()-1, [](const BranchNode& node) {
				return node.type != ItemTreeNode::Type::UNDEFINED;
		}), "Cost specified but not all types of (non-leaf) nodes are defined");
		forFirstTrue(m, costAtomInfos, [&branch](const GringoOutputProcessor::CostAtomArguments& arguments) {
				branch.back()->setCost(arguments.cost);
		});
		// }}}
		// Set current cost {{{
		ASP_CHECK(countTrue(m, currentCostAtomInfos) <= 1, "More than one true currentCost/1 atom");
		ASP_CHECK(countTrue(m, currentCostAtomInfos) == 0 || countTrue(m, costAtomInfos) == 1, "True currentCost/1 atom without true cost/1 atom");
		forFirstTrue(m, currentCostAtomInfos, [&branch](const GringoOutputProcessor::CurrentCostAtomArguments& arguments) {
				branch.back()->setCurrentCost(arguments.currentCost);
		});
		// }}}
	}
	// Insert branch into tree {{{
	if(!uncompressedItemTree)
		uncompressedItemTree = UncompressedItemTreePtr(new UncompressedItemTree(std::move(branch.front())));

	uncompressedItemTree->addBranch(++branch.begin(), branch.end());
	// }}}
	return true;
}

void ClaspCallback::prepare(const Clasp::Asp::LogicProgram& prg)
{
	assert(prg.frozen()); // Ground program must be frozen
	for(const auto& atom : gringoOutput.getItemAtomInfos())
		itemAtomInfos.emplace_back(ItemAtomInfo(atom, prg));
	for(const auto& atom : gringoOutput.getAuxItemAtomInfos())
		auxItemAtomInfos.emplace_back(AuxItemAtomInfo(atom, prg));
	for(const auto& atom : gringoOutput.getExtendAtomInfos())
		extendAtomInfos.emplace_back(ExtendAtomInfo(atom, prg));
	for(const auto& atom : gringoOutput.getCurrentCostAtomInfos())
		currentCostAtomInfos.emplace_back(CurrentCostAtomInfo(atom, prg));
	for(const auto& atom : gringoOutput.getCostAtomInfos())
		costAtomInfos.emplace_back(CostAtomInfo(atom, prg));
	for(const auto& atom : gringoOutput.getLengthAtomInfos())
		lengthAtomInfos.emplace_back(LengthAtomInfo(atom, prg));
	for(const auto& atom : gringoOutput.getOrAtomInfos())
		orAtomInfos.emplace_back(OrAtomInfo(atom, prg));
	for(const auto& atom : gringoOutput.getAndAtomInfos())
		andAtomInfos.emplace_back(AndAtomInfo(atom, prg));

	if(gringoOutput.getAcceptAtomKey())
		acceptLiteral.reset(new Clasp::Literal(prg.getLiteral(*gringoOutput.getAcceptAtomKey())));
	if(gringoOutput.getRejectAtomKey())
		rejectLiteral.reset(new Clasp::Literal(prg.getLiteral(*gringoOutput.getRejectAtomKey())));
}

ItemTreePtr ClaspCallback::finalize(bool pruneUndefined, bool pruneRejecting)
{
	app.getPrinter().uncompressedSolverInvocationResult(decomposition, uncompressedItemTree.get());
	if(uncompressedItemTree)
		itemTree = uncompressedItemTree->compress(pruneUndefined);
	return asp_utils::ClaspCallback::finalize(pruneUndefined, pruneRejecting);
}

}}} // namespace solver::clasp::trees
