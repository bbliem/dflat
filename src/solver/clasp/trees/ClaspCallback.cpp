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

		for(auto const &counterAtomInfos : allCounterAtomInfos) {
		    if(counterAtomInfos.first.compare("cost") == 0) {
		        ASP_CHECK(countTrue(m, allCounterIncAtomInfos[counterAtomInfos.first]) == 0 || countTrue(m, counterAtomInfos.second) == 0,
		            "Both 'counter'/'cost' and 'counterInc' predicates used for setting the cost");
		        ASP_CHECK(countTrue(m, counterAtomInfos.second) <= 1, "More than one true atom for setting the cost");
		        ASP_CHECK(countTrue(m, counterAtomInfos.second) == 0 || std::all_of(branchData.begin(), branchData.end()-1, [](const BranchNode& node) {
		                return node.type != ItemTreeNode::Type::UNDEFINED;
		        }), "Cost specified but not all types of (non-leaf) nodes are defined");
		    } else {
		        ASP_CHECK(countTrue(m, allCounterIncAtomInfos[counterAtomInfos.first]) == 0 || countTrue(m, counterAtomInfos.second) == 0,
		            "Both 'counter' and 'counterInc' predicates used for setting the " + counterAtomInfos.first + " counter");
		        ASP_CHECK(countTrue(m, counterAtomInfos.second) <= 1, "More than one true atom for setting the " + counterAtomInfos.first + " counter");
		    }
		}

		for(auto const &currentCounterAtomInfos : allCurrentCounterAtomInfos) {
		    if(currentCounterAtomInfos.first.compare("cost") == 0){
		        ASP_CHECK(countTrue(m, currentCounterAtomInfos.second) == 0 || countTrue(m, allCounterAtomInfos[currentCounterAtomInfos.first]) == 1,
		                  "True current cost atom without true cost atom");
		        ASP_CHECK(countTrue(m, allCurrentCounterIncAtomInfos[currentCounterAtomInfos.first]) == 0 || countTrue(m, currentCounterAtomInfos.second) == 0,
		            "Both 'currentCounter'/'currentCost' and 'currentCounterInc' predicates used for setting the current cost");
		        ASP_CHECK(countTrue(m, currentCounterAtomInfos.second) <= 1, "More than one true atom for setting the current cost");
		    } else {
		        ASP_CHECK(countTrue(m, currentCounterAtomInfos.second) == 0 || countTrue(m, allCounterAtomInfos[currentCounterAtomInfos.first]) == 1,
		                  "True " + currentCounterAtomInfos.first + " current counter atom without true " + currentCounterAtomInfos.first + " counter atom");
		        ASP_CHECK(countTrue(m, allCurrentCounterIncAtomInfos[currentCounterAtomInfos.first]) == 0 || countTrue(m, currentCounterAtomInfos.second) == 0,
		            "Both 'currentCounter' and 'currentCounterInc' predicates used for setting the " + currentCounterAtomInfos.first + " current counter");
		        ASP_CHECK(countTrue(m, currentCounterAtomInfos.second) <= 1, "More than one true atom for setting the " + currentCounterAtomInfos.first + " current counter");
		    }
		}

		// Set cost {{{   
 
        std::map<std::string,long> counterValues;
        for(auto const &counterIncAtomInfos : allCounterIncAtomInfos) {
            forEachTrue(m, counterIncAtomInfos.second, [&counterValues](const GringoOutputProcessor::CounterIncAtomArguments& arguments) {
                            counterValues[arguments.counterName] += arguments.counterInc;
            });
        }

        for(const ItemTreeNode::ExtensionPointer& extPtr : branch.back()->getExtensionPointers().back()) {
            for(const auto& counter : extPtr->getCounters()) {
                counterValues[counter.first] += extPtr->getCounter(counter.first);
            }
        }

        for(auto const &counterAtomInfos : allCounterAtomInfos) {
            forEachTrue(m, counterAtomInfos.second, [&counterValues](const GringoOutputProcessor::CounterAtomArguments& arguments) {
                            counterValues[arguments.counterName] = arguments.counter;
            });
        }

        for(const auto& counterValue : counterValues) {
            if(counterRemAtomInfos.find(counterValue.first) == counterRemAtomInfos.end() || !m.isTrue(counterRemAtomInfos[counterValue.first])) {
                branch.back()->setCounter(counterValue.first, counterValue.second);
                //if(counterValue.first.compare("cost") == 0)
                //    cost = counterValue.second;
            }
        }
        // Possibly update cost of root {{{

        for(const auto& counter : branch.back()->getCounters())
           branch.back()->setCounter(counter.first, std::min(branch.back()->getCounter(counter.first), counterValues[counter.first]));
        // }}}

		// }}}

    	// Set current cost {{{

        std::map<std::string,long> currentCounterValues;
        for(auto const &currentCounterIncAtomInfos : allCurrentCounterIncAtomInfos) {
            forEachTrue(m, currentCounterIncAtomInfos.second, [&currentCounterValues](const GringoOutputProcessor::CurrentCounterIncAtomArguments& arguments) {
                            currentCounterValues[arguments.currentCounterName] += arguments.currentCounterInc;

            });
        }

        for(const ItemTreeNode::ExtensionPointer& extPtr : branch.back()->getExtensionPointers().back()) {
            for(const auto& currentCounter : extPtr->getCurrentCounters()) {
                currentCounterValues[currentCounter.first] += extPtr->getCurrentCounter(currentCounter.first);
            }
        }

        for(auto const &currentCounterAtomInfos : allCurrentCounterAtomInfos) {
            forEachTrue(m, currentCounterAtomInfos.second, [&currentCounterValues](const GringoOutputProcessor::CurrentCounterAtomArguments& arguments) {
                            currentCounterValues[arguments.currentCounterName] = arguments.currentCounter;
            });
        }

        for(const auto& currentCounterValue : currentCounterValues) {
            if(counterRemAtomInfos.find(currentCounterValue.first) == counterRemAtomInfos.end() || !m.isTrue(counterRemAtomInfos[currentCounterValue.first]))
                branch.back()->setCurrentCounter(currentCounterValue.first, currentCounterValue.second);
        }

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

    for(const auto& atom : gringoOutput.getCounterRemAtomInfos())
        counterRemAtomInfos.insert(std::pair<std::string, Clasp::Literal>(atom.arguments.counterName, Clasp::Literal(prg.getLiteral(atom.atomId))));
    for(const auto& counterIncAtomInfos : gringoOutput.getAllCounterIncAtomInfos())
        for(const auto& atom : counterIncAtomInfos.second)
            allCounterIncAtomInfos[counterIncAtomInfos.first].emplace_back(CounterIncAtomInfo(atom, prg));
    for(const auto& currentCounterIncAtomInfos : gringoOutput.getAllCurrentCounterIncAtomInfos())
        for(const auto& atom : currentCounterIncAtomInfos.second)
            allCurrentCounterIncAtomInfos[currentCounterIncAtomInfos.first].emplace_back(CurrentCounterIncAtomInfo(atom, prg));
    for(const auto& counterAtomInfos : gringoOutput.getAllCounterAtomInfos())
        for(const auto& atom : counterAtomInfos.second)
            allCounterAtomInfos[counterAtomInfos.first].emplace_back(CounterAtomInfo(atom, prg));
    for(const auto& currentCounterAtomInfos : gringoOutput.getAllCurrentCounterAtomInfos())
        for(const auto& atom : currentCounterAtomInfos.second)
            allCurrentCounterAtomInfos[currentCounterAtomInfos.first].emplace_back(CurrentCounterAtomInfo(atom, prg));
}

ItemTreePtr ClaspCallback::finalize(bool pruneUndefined, bool pruneRejecting)
{
	app.getPrinter().uncompressedSolverInvocationResult(decomposition, uncompressedItemTree.get());
	if(uncompressedItemTree)
		itemTree = uncompressedItemTree->compress(pruneUndefined);
	return asp_utils::ClaspCallback::finalize(pruneUndefined, pruneRejecting);
}

}}} // namespace solver::clasp::trees
