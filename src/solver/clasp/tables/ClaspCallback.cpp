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

namespace solver { namespace clasp { namespace tables {

ClaspCallback::ClaspCallback(const GringoOutputProcessor& gringoOutput, const ChildItemTrees& childItemTrees, const Application& app, bool root, bool cardinalityCost)
	: asp_utils::ClaspCallback(app)
	, gringoOutput(gringoOutput)
	, childItemTrees(childItemTrees)
	, rowType(root ? ItemTreeNode::Type::ACCEPT : ItemTreeNode::Type::UNDEFINED)
	, cardinalityCost(cardinalityCost)
{
	unsigned int i = 0;
	for(const auto& pair : childItemTrees)
		indexOfChildItemTreeRoot[pair.second->getNode().get()] = i++;
}

bool ClaspCallback::onModel(const Clasp::Solver& s, const Clasp::Model& m)
{
	asp_utils::ClaspCallback::onModel(s, m);

	// Check if counters are used correctly {{{
#ifndef DISABLE_CHECKS
	for(const auto& counterAtomInfos : allCounterAtomInfos) {
        if(counterAtomInfos.first.compare("cost") == 0) {
			ASP_CHECK(countTrue(m, allCounterIncAtomInfos[counterAtomInfos.first]) == 0 || countTrue(m, counterAtomInfos.second) == 0,
				"Both 'counter'/'cost' and 'counterInc' predicates used for setting the cost");
			ASP_CHECK(countTrue(m, counterAtomInfos.second) <= 1, "More than one true atom for setting the cost");
		} else {
			ASP_CHECK(countTrue(m, allCounterIncAtomInfos[counterAtomInfos.first]) == 0 || countTrue(m, counterAtomInfos.second) == 0,
                "Both 'counter' and 'counterInc' predicates used for setting the " + counterAtomInfos.first + " counter");
			ASP_CHECK(countTrue(m, counterAtomInfos.second) <= 1, "More than one true atom for setting the " + counterAtomInfos.first + " counter");
		}
	}

	for(const auto& currentCounterAtomInfos : allCurrentCounterAtomInfos) {
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
#endif // }}}
	// Get items {{{
	ItemTreeNode::Items items;
	forEachTrue(m, itemAtomInfos, [&items](const GringoOutputProcessor::ItemAtomArguments& arguments) {
			items.insert(arguments.item);
	});
	ItemTreeNode::Items auxItems;
	forEachTrue(m, auxItemAtomInfos, [&auxItems](const GringoOutputProcessor::AuxItemAtomArguments& arguments) {
			auxItems.insert(arguments.item);
	});

	ASP_CHECK(std::find_if(items.begin(), items.end(), [&auxItems](const String& item) {
				return auxItems.find(item) != auxItems.end();
	}) == items.end(), "Items and auxiliary items not disjoint");
	// }}}
	// Get extension pointers {{{
	ItemTreeNode::ExtensionPointerTuple extendedRows(childItemTrees.size());
	ASP_CHECK(countTrue(m, extendAtomInfos) == childItemTrees.size(), "Not as many extension pointers as there are child item trees");
	unsigned int numExtended = 0;
	forEachTrueLimited(m, extendAtomInfos, [&](const GringoOutputProcessor::ExtendAtomArguments& arguments) {
			ItemTreeNode::ExtensionPointer pointer(arguments.extendedRow);
			assert(indexOfChildItemTreeRoot.find(pointer->getParent()) != indexOfChildItemTreeRoot.end());
			extendedRows[indexOfChildItemTreeRoot[pointer->getParent()]] = std::move(pointer);
			++numExtended;
			return numExtended != childItemTrees.size();
	});
	// }}}
	// Create item tree root if it doesn't exist yet {{{
	if(!itemTree) {
		ItemTreeNode::ExtensionPointerTuple rootExtensionPointers(childItemTrees.size());
		for(const auto& childItemTree : childItemTrees) {
			ItemTreeNode::ExtensionPointer pointer(childItemTree.second->getNode());
			assert(indexOfChildItemTreeRoot.find(pointer.get()) != indexOfChildItemTreeRoot.end());
			rootExtensionPointers[indexOfChildItemTreeRoot[pointer.get()]] = std::move(pointer);
		}
		itemTree = ItemTreePtr(new ItemTree(std::shared_ptr<ItemTreeNode>(new ItemTreeNode({}, {}, {std::move(rootExtensionPointers)}, ItemTreeNode::Type::OR))));
		// Set cost to "infinity"
		if(!app.isOptimizationDisabled())
			itemTree->getNode()->setCost(std::numeric_limits<decltype(itemTree->getNode()->getCost())>::max());
	}
	// }}}

	// Create item tree node
	std::shared_ptr<ItemTreeNode> node(new ItemTreeNode(std::move(items), std::move(auxItems), {std::move(extendedRows)}, rowType));

	long cost = 0;
	long currentCost = 0;

	// Set (current) counters and compute (if optimization is not disabled) cost {{{
    std::map<std::string,long> counterValues;
	for(const auto& counterIncAtomInfos : allCounterIncAtomInfos) {
		// Note that here forFirstTrue is not sufficient
		forEachTrue(m, counterIncAtomInfos.second, [&counterValues](const GringoOutputProcessor::CounterIncAtomArguments& arguments) {
				counterValues[arguments.counterName] += arguments.counterInc;
		});
	}

	for(const auto& extPtr : node->getExtensionPointers().back()) {
		for(const auto& counter : extPtr->getCounters())
			counterValues[counter.first] += counter.second;
		counterValues["cost"] += extPtr->getCost();
	}

	for(const auto& counterAtomInfos : allCounterAtomInfos) {
		forFirstTrue(m, counterAtomInfos.second, [&counterValues](const GringoOutputProcessor::CounterAtomArguments& arguments) {
				counterValues[arguments.counterName] = arguments.counter;
		});
	}
	forFirstTrue(m, costAtomInfos, [&counterValues](const GringoOutputProcessor::CostAtomArguments& arguments) {
			counterValues["cost"] = arguments.cost;
	});

	for(const auto& counterValue : counterValues) {
		if(counterRemAtomInfos.find(counterValue.first) == counterRemAtomInfos.end() || !m.isTrue(counterRemAtomInfos[counterValue.first])) {
			if(counterValue.first == "cost") {
				if(!app.isOptimizationDisabled())
					cost = counterValue.second;
			}
			else
				node->setCounter(counterValue.first, counterValue.second);
		}
	}

	// Possibly update counters of root [XXX necessary?]
//	for(const auto& counter : itemTree->getNode()->getCounters())
//		itemTree->getNode()->setCounter(counter.first, std::min(itemTree->getNode()->getCounter(counter.first), counterValues[counter.first]));

    std::map<std::string,long> currentCounterValues;
	for(const auto& currentCounterIncAtomInfos : allCurrentCounterIncAtomInfos) {
		// Note that here forFirstTrue is not sufficient
		forEachTrue(m, currentCounterIncAtomInfos.second, [&currentCounterValues](const GringoOutputProcessor::CurrentCounterIncAtomArguments& arguments) {
				currentCounterValues[arguments.currentCounterName] += arguments.currentCounterInc;
		});
	}

	for(const auto& extPtr : node->getExtensionPointers().back()) {
		for(const auto& currentCounter : extPtr->getCurrentCounters())
			currentCounterValues[currentCounter.first] += extPtr->getCurrentCounter(currentCounter.first);
		currentCounterValues["cost"] += extPtr->getCurrentCost();
	}

	for(const auto& currentCounterAtomInfos : allCurrentCounterAtomInfos) {
		forFirstTrue(m, currentCounterAtomInfos.second, [&currentCounterValues](const GringoOutputProcessor::CurrentCounterAtomArguments& arguments) {
				currentCounterValues[arguments.currentCounterName] = arguments.currentCounter;
		});
	}
	forFirstTrue(m, currentCostAtomInfos, [&currentCounterValues](const GringoOutputProcessor::CurrentCostAtomArguments& arguments) {
			currentCounterValues["cost"] = arguments.currentCost;
	});

	for(const auto& currentCounterValue : currentCounterValues) {
		if(counterRemAtomInfos.find(currentCounterValue.first) == counterRemAtomInfos.end() || !m.isTrue(counterRemAtomInfos[currentCounterValue.first])) {
			if(currentCounterValue.first == "cost") {
				if(!app.isOptimizationDisabled())
					currentCost = currentCounterValue.second;
			}
			else
				node->setCurrentCounter(currentCounterValue.first, currentCounterValue.second);
		}
	}
	// }}}

	if(cardinalityCost && !app.isOptimizationDisabled()) {
		cost = node->getItems().size();
		for(const auto& row : node->getExtensionPointers().front()) {
			const auto& oldItems = row->getItems();
			ItemTreeNode::Items intersection;
			std::set_intersection(node->getItems().begin(), node->getItems().end(), oldItems.begin(), oldItems.end(), std::inserter(intersection, intersection.begin()));
			cost += row->getCost() - intersection.size();
		}

		currentCost = node->getItems().size();
	}

	node->setCost(cost);
	node->setCurrentCost(currentCost);

	// Possibly update cost of root
	itemTree->getNode()->setCost(std::min(itemTree->getNode()->getCost(), cost));

	// Add node to item tree
	itemTree->addChildAndMerge(ItemTree::ChildPtr(new ItemTree(std::move(node))));

	return true;
}

void ClaspCallback::prepare(const Clasp::Asp::LogicProgram& prg)
{
	assert(prg.frozen());  // Ground program must be frozen
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

}}} // namespace solver::clasp::tables
